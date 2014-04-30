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
#define yyparse         Compilerparse
#define yylex           Compilerlex
#define yyerror         Compilererror
#define yylval          Compilerlval
#define yychar          Compilerchar
#define yydebug         Compilerdebug
#define yynerrs         Compilernerrs
#define yylloc          Compilerlloc

/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "hphp.y"

#ifdef XHPAST2_PARSER
#include "hphp/parser/xhpast2/parser.h"
#else
#include "hphp/compiler/parser/parser.h"
#endif
#include <boost/lexical_cast.hpp>
#include "hphp/util/text-util.h"
#include "hphp/util/logger.h"

// macros for bison
#define YYSTYPE HPHP::HPHP_PARSER_NS::Token
#define YYSTYPE_IS_TRIVIAL 1
#define YYLTYPE HPHP::Location
#define YYLTYPE_IS_TRIVIAL 1
#define YYERROR_VERBOSE
#define YYINITDEPTH 500
#define YYLEX_PARAM _p

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
    if (YYID (N)) {                                                     \
      (Current).first(YYRHSLOC (Rhs, 1));                               \
      (Current).last (YYRHSLOC (Rhs, N));                               \
    } else {                                                            \
      (Current).line0 = (Current).line1 = YYRHSLOC (Rhs, 0).line1;      \
      (Current).char0 = (Current).char1 = YYRHSLOC (Rhs, 0).char1;      \
    }                                                                   \
  while (YYID (0));                                                     \
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
  while (YYID (0))

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
  while (YYID (0))

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
  while (YYID (0))

# define YYSTACK_RELOCATE_RESET(Stack_alloc, Stack)                     \
  do                                                                    \
    {                                                                   \
      YYSIZE_T yynewbytes;                                              \
      YYCOPY_RESET (&yyptr->Stack_alloc, Stack, yysize);                \
      Stack = &yyptr->Stack_alloc;                                      \
      yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
      yyptr += yynewbytes / sizeof (*yyptr);                            \
    }                                                                   \
  while (YYID (0))

#define YYSTACK_CLEANUP                         \
  YYTOKEN_RESET (yyvs, yystacksize);            \
  if (yyvs != yyvsa) {                          \
    YYSTACK_FREE (yyvs);                        \
  }                                             \
  if (yyls != yylsa) {                          \
    YYSTACK_FREE (yyls);                        \
  }                                             \


// macros for rules
#define BEXP(e...) _p->onBinaryOpExp(e);
#define UEXP(e...) _p->onUnaryOpExp(e);

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
  t.setText(boost::lexical_cast<std::string>(num));
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
// converting constant declaration to "define(name, value);"
// TODO: get rid of this, or pass in more info, task 3491019.

static void on_constant(Parser *_p, Token &out, Token &name, Token &value) {
  Token sname;   _p->onScalar(sname, T_CONSTANT_ENCAPSED_STRING, name);

  Token fname;   fname.setText("define");
  Token params1; _p->onCallParam(params1, NULL, sname, 0);
  Token params2; _p->onCallParam(params2, &params1, value, 0);
  Token call;    _p->onCall(call, 0, fname, params2, 0);
  Token expr;    _p->onExpStatement(expr, call);

  _p->addTopStatement(expr);
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
  HPHP::split(':', attributes.text().c_str(), classes, true);
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
    Token params1; _p->onCallParam(params1, NULL, param1, 0);

    for (unsigned int i = 0; i < classes.size(); i++) {
      Token parent;  parent.set(T_STRING, classes[i]);
      Token cls;     _p->onName(cls, parent, Parser::StringName);
      Token fname;   fname.setText("__xhpAttributeDeclaration");
      Token param;   _p->onCall(param, 0, fname, dummy, &cls);

      Token params; _p->onCallParam(params, &params1, param, 0);
      params1 = params;
    }

    Token params2; _p->onCallParam(params2, &params1, arrAttributes, 0);

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
    Token params, ret, ref; ref = 1;
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
    Token params, ret, ref; ref = 1;
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
    Token params, ret, ref; ref = 1;
    _p->onMethod(out, modifiers, ret, ref, fname, params, stmt, nullptr, false);
  }
}

static void only_in_hh_syntax(Parser *_p) {
  if (!_p->scanner().isHHSyntaxEnabled()) {
    HPHP_PARSER_ERROR(
      "Syntax only allowed with -v Eval.EnableHipHopSyntax=true", _p);
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

static int yylex(YYSTYPE *token, HPHP::Location *loc, Parser *_p) {
  return _p->scan(token, loc);
}


/* Line 189 of yacc.c  */
#line 663 "hphp.tab.cpp"

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
     T_SR_EQUAL = 268,
     T_SL_EQUAL = 269,
     T_XOR_EQUAL = 270,
     T_OR_EQUAL = 271,
     T_AND_EQUAL = 272,
     T_MOD_EQUAL = 273,
     T_CONCAT_EQUAL = 274,
     T_DIV_EQUAL = 275,
     T_MUL_EQUAL = 276,
     T_MINUS_EQUAL = 277,
     T_PLUS_EQUAL = 278,
     T_BOOLEAN_OR = 279,
     T_BOOLEAN_AND = 280,
     T_IS_NOT_IDENTICAL = 281,
     T_IS_IDENTICAL = 282,
     T_IS_NOT_EQUAL = 283,
     T_IS_EQUAL = 284,
     T_IS_GREATER_OR_EQUAL = 285,
     T_IS_SMALLER_OR_EQUAL = 286,
     T_SR = 287,
     T_SL = 288,
     T_INSTANCEOF = 289,
     T_UNSET_CAST = 290,
     T_BOOL_CAST = 291,
     T_OBJECT_CAST = 292,
     T_ARRAY_CAST = 293,
     T_STRING_CAST = 294,
     T_DOUBLE_CAST = 295,
     T_INT_CAST = 296,
     T_DEC = 297,
     T_INC = 298,
     T_CLONE = 299,
     T_NEW = 300,
     T_EXIT = 301,
     T_IF = 302,
     T_ELSEIF = 303,
     T_ELSE = 304,
     T_ENDIF = 305,
     T_LNUMBER = 306,
     T_DNUMBER = 307,
     T_ONUMBER = 308,
     T_STRING = 309,
     T_STRING_VARNAME = 310,
     T_VARIABLE = 311,
     T_NUM_STRING = 312,
     T_INLINE_HTML = 313,
     T_CHARACTER = 314,
     T_BAD_CHARACTER = 315,
     T_ENCAPSED_AND_WHITESPACE = 316,
     T_CONSTANT_ENCAPSED_STRING = 317,
     T_ECHO = 318,
     T_DO = 319,
     T_WHILE = 320,
     T_ENDWHILE = 321,
     T_FOR = 322,
     T_ENDFOR = 323,
     T_FOREACH = 324,
     T_ENDFOREACH = 325,
     T_DECLARE = 326,
     T_ENDDECLARE = 327,
     T_AS = 328,
     T_SWITCH = 329,
     T_ENDSWITCH = 330,
     T_CASE = 331,
     T_DEFAULT = 332,
     T_BREAK = 333,
     T_GOTO = 334,
     T_CONTINUE = 335,
     T_FUNCTION = 336,
     T_CONST = 337,
     T_RETURN = 338,
     T_TRY = 339,
     T_CATCH = 340,
     T_THROW = 341,
     T_USE = 342,
     T_GLOBAL = 343,
     T_PUBLIC = 344,
     T_PROTECTED = 345,
     T_PRIVATE = 346,
     T_FINAL = 347,
     T_ABSTRACT = 348,
     T_STATIC = 349,
     T_VAR = 350,
     T_UNSET = 351,
     T_ISSET = 352,
     T_EMPTY = 353,
     T_HALT_COMPILER = 354,
     T_CLASS = 355,
     T_INTERFACE = 356,
     T_EXTENDS = 357,
     T_IMPLEMENTS = 358,
     T_OBJECT_OPERATOR = 359,
     T_DOUBLE_ARROW = 360,
     T_LIST = 361,
     T_ARRAY = 362,
     T_CALLABLE = 363,
     T_CLASS_C = 364,
     T_METHOD_C = 365,
     T_FUNC_C = 366,
     T_LINE = 367,
     T_FILE = 368,
     T_COMMENT = 369,
     T_DOC_COMMENT = 370,
     T_OPEN_TAG = 371,
     T_OPEN_TAG_WITH_ECHO = 372,
     T_CLOSE_TAG = 373,
     T_WHITESPACE = 374,
     T_START_HEREDOC = 375,
     T_END_HEREDOC = 376,
     T_DOLLAR_OPEN_CURLY_BRACES = 377,
     T_CURLY_OPEN = 378,
     T_DOUBLE_COLON = 379,
     T_NAMESPACE = 380,
     T_NS_C = 381,
     T_DIR = 382,
     T_NS_SEPARATOR = 383,
     T_YIELD = 384,
     T_XHP_LABEL = 385,
     T_XHP_TEXT = 386,
     T_XHP_ATTRIBUTE = 387,
     T_XHP_CATEGORY = 388,
     T_XHP_CATEGORY_LABEL = 389,
     T_XHP_CHILDREN = 390,
     T_XHP_ENUM = 391,
     T_XHP_REQUIRED = 392,
     T_TRAIT = 393,
     T_ELLIPSIS = 394,
     T_INSTEADOF = 395,
     T_TRAIT_C = 396,
     T_HH_ERROR = 397,
     T_FINALLY = 398,
     T_XHP_TAG_LT = 399,
     T_XHP_TAG_GT = 400,
     T_TYPELIST_LT = 401,
     T_TYPELIST_GT = 402,
     T_UNRESOLVED_LT = 403,
     T_COLLECTION = 404,
     T_SHAPE = 405,
     T_TYPE = 406,
     T_UNRESOLVED_TYPE = 407,
     T_NEWTYPE = 408,
     T_UNRESOLVED_NEWTYPE = 409,
     T_COMPILER_HALT_OFFSET = 410,
     T_AWAIT = 411,
     T_ASYNC = 412,
     T_FROM = 413,
     T_WHERE = 414,
     T_JOIN = 415,
     T_IN = 416,
     T_ON = 417,
     T_EQUALS = 418,
     T_INTO = 419,
     T_LET = 420,
     T_ORDERBY = 421,
     T_ASCENDING = 422,
     T_DESCENDING = 423,
     T_SELECT = 424,
     T_GROUP = 425,
     T_BY = 426,
     T_LAMBDA_OP = 427,
     T_LAMBDA_CP = 428,
     T_UNRESOLVED_OP = 429
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
#line 892 "hphp.tab.cpp"

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
#define YYLAST   14667

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  204
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  248
/* YYNRULES -- Number of rules.  */
#define YYNRULES  831
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1541

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
       2,     2,     2,    49,   202,     2,   199,    48,    32,   203,
     194,   195,    46,    43,     9,    44,    45,    47,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    27,   196,
      37,    14,    38,    26,    52,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    62,     2,   201,    31,     2,   200,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   197,    30,   198,    51,     2,     2,     2,
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
      17,    18,    19,    20,    21,    22,    23,    24,    25,    28,
      29,    33,    34,    35,    36,    39,    40,    41,    42,    50,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
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
      19,    21,    26,    30,    31,    38,    39,    45,    49,    54,
      57,    59,    61,    63,    65,    67,    69,    71,    73,    75,
      77,    79,    81,    83,    85,    87,    89,    91,    93,    95,
      99,   101,   105,   107,   109,   112,   116,   121,   123,   126,
     130,   135,   137,   141,   143,   147,   150,   152,   155,   158,
     164,   169,   172,   173,   175,   177,   179,   181,   185,   191,
     200,   201,   206,   207,   214,   215,   226,   227,   232,   235,
     239,   242,   246,   249,   253,   257,   261,   265,   269,   275,
     277,   279,   280,   290,   296,   297,   311,   312,   318,   322,
     326,   329,   332,   335,   338,   341,   344,   348,   351,   354,
     358,   361,   362,   367,   377,   378,   379,   384,   387,   388,
     390,   391,   393,   394,   404,   405,   416,   417,   429,   430,
     439,   440,   450,   451,   459,   460,   469,   470,   478,   479,
     488,   490,   492,   494,   496,   498,   501,   504,   507,   508,
     511,   512,   515,   516,   518,   522,   524,   528,   531,   532,
     534,   537,   542,   544,   549,   551,   556,   558,   563,   565,
     570,   574,   580,   584,   589,   594,   600,   606,   611,   612,
     614,   616,   621,   622,   628,   629,   632,   633,   637,   638,
     646,   653,   656,   662,   667,   668,   673,   679,   687,   694,
     701,   709,   719,   728,   735,   741,   744,   749,   753,   754,
     758,   763,   770,   776,   782,   789,   798,   806,   809,   810,
     812,   815,   819,   824,   828,   830,   832,   835,   840,   844,
     850,   852,   856,   859,   860,   861,   866,   867,   873,   876,
     877,   888,   889,   901,   905,   909,   913,   918,   923,   927,
     933,   936,   939,   940,   947,   953,   958,   962,   964,   966,
     970,   975,   977,   979,   981,   983,   988,   990,   994,   997,
     998,  1001,  1002,  1004,  1008,  1010,  1012,  1014,  1016,  1020,
    1025,  1030,  1035,  1037,  1039,  1042,  1045,  1048,  1052,  1056,
    1058,  1060,  1062,  1064,  1068,  1070,  1074,  1076,  1078,  1080,
    1081,  1083,  1086,  1088,  1090,  1092,  1094,  1096,  1098,  1100,
    1102,  1103,  1105,  1107,  1109,  1113,  1119,  1121,  1125,  1131,
    1136,  1140,  1144,  1147,  1149,  1151,  1155,  1159,  1161,  1163,
    1164,  1167,  1172,  1176,  1183,  1186,  1190,  1197,  1199,  1201,
    1203,  1210,  1214,  1219,  1226,  1230,  1234,  1238,  1242,  1246,
    1250,  1254,  1258,  1262,  1266,  1270,  1273,  1276,  1279,  1282,
    1286,  1290,  1294,  1298,  1302,  1306,  1310,  1314,  1318,  1322,
    1326,  1330,  1334,  1338,  1342,  1346,  1349,  1352,  1355,  1358,
    1362,  1366,  1370,  1374,  1378,  1382,  1386,  1390,  1394,  1398,
    1404,  1409,  1411,  1414,  1417,  1420,  1423,  1426,  1429,  1432,
    1435,  1438,  1440,  1442,  1444,  1448,  1451,  1453,  1455,  1457,
    1463,  1464,  1465,  1477,  1478,  1491,  1492,  1496,  1497,  1504,
    1507,  1512,  1514,  1520,  1524,  1530,  1534,  1537,  1538,  1541,
    1542,  1547,  1552,  1556,  1561,  1566,  1571,  1576,  1578,  1580,
    1584,  1587,  1591,  1596,  1599,  1603,  1605,  1608,  1610,  1613,
    1615,  1617,  1619,  1621,  1623,  1625,  1630,  1635,  1638,  1647,
    1658,  1661,  1663,  1667,  1669,  1672,  1674,  1676,  1678,  1680,
    1683,  1688,  1692,  1696,  1701,  1703,  1706,  1711,  1714,  1721,
    1722,  1724,  1729,  1730,  1733,  1734,  1736,  1738,  1742,  1744,
    1748,  1750,  1752,  1756,  1760,  1762,  1764,  1766,  1768,  1770,
    1772,  1774,  1776,  1778,  1780,  1782,  1784,  1786,  1788,  1790,
    1792,  1794,  1796,  1798,  1800,  1802,  1804,  1806,  1808,  1810,
    1812,  1814,  1816,  1818,  1820,  1822,  1824,  1826,  1828,  1830,
    1832,  1834,  1836,  1838,  1840,  1842,  1844,  1846,  1848,  1850,
    1852,  1854,  1856,  1858,  1860,  1862,  1864,  1866,  1868,  1870,
    1872,  1874,  1876,  1878,  1880,  1882,  1884,  1886,  1888,  1890,
    1892,  1894,  1896,  1898,  1900,  1902,  1904,  1906,  1908,  1910,
    1912,  1914,  1916,  1918,  1920,  1925,  1927,  1929,  1931,  1933,
    1935,  1937,  1939,  1941,  1944,  1946,  1947,  1948,  1950,  1952,
    1956,  1957,  1959,  1961,  1963,  1965,  1967,  1969,  1971,  1973,
    1975,  1977,  1979,  1981,  1983,  1987,  1990,  1992,  1994,  1997,
    2000,  2005,  2009,  2014,  2016,  2018,  2022,  2026,  2030,  2032,
    2034,  2036,  2038,  2042,  2046,  2050,  2053,  2054,  2056,  2057,
    2059,  2060,  2066,  2070,  2074,  2076,  2078,  2080,  2082,  2084,
    2088,  2091,  2093,  2095,  2097,  2099,  2101,  2103,  2106,  2109,
    2114,  2118,  2123,  2126,  2127,  2133,  2137,  2141,  2143,  2147,
    2149,  2152,  2153,  2159,  2163,  2166,  2167,  2171,  2172,  2177,
    2180,  2181,  2185,  2189,  2191,  2192,  2194,  2197,  2200,  2205,
    2209,  2213,  2216,  2221,  2224,  2229,  2231,  2233,  2235,  2237,
    2239,  2242,  2247,  2251,  2256,  2260,  2262,  2264,  2266,  2268,
    2271,  2276,  2281,  2285,  2287,  2289,  2293,  2301,  2308,  2317,
    2327,  2336,  2347,  2355,  2362,  2371,  2373,  2376,  2381,  2386,
    2388,  2390,  2395,  2397,  2398,  2400,  2403,  2405,  2407,  2410,
    2415,  2419,  2423,  2424,  2426,  2429,  2434,  2438,  2441,  2445,
    2452,  2453,  2455,  2460,  2463,  2464,  2470,  2474,  2478,  2480,
    2487,  2492,  2497,  2500,  2503,  2504,  2510,  2514,  2518,  2520,
    2523,  2524,  2530,  2534,  2538,  2540,  2543,  2546,  2548,  2551,
    2553,  2558,  2562,  2566,  2573,  2577,  2579,  2581,  2583,  2588,
    2593,  2598,  2603,  2606,  2609,  2614,  2617,  2620,  2622,  2626,
    2630,  2634,  2635,  2638,  2644,  2651,  2653,  2656,  2658,  2663,
    2667,  2668,  2670,  2674,  2678,  2680,  2682,  2683,  2684,  2687,
    2691,  2693,  2699,  2703,  2707,  2711,  2713,  2716,  2717,  2722,
    2725,  2728,  2730,  2732,  2734,  2736,  2741,  2748,  2750,  2759,
    2765,  2767
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     205,     0,    -1,    -1,   206,   207,    -1,   207,   208,    -1,
      -1,   224,    -1,   240,    -1,   244,    -1,   249,    -1,   438,
      -1,   118,   194,   195,   196,    -1,   144,   216,   196,    -1,
      -1,   144,   216,   197,   209,   207,   198,    -1,    -1,   144,
     197,   210,   207,   198,    -1,   106,   212,   196,    -1,   106,
     100,   213,   196,    -1,   221,   196,    -1,    73,    -1,   151,
      -1,   152,    -1,   154,    -1,   156,    -1,   155,    -1,   178,
      -1,   179,    -1,   181,    -1,   180,    -1,   182,    -1,   183,
      -1,   184,    -1,   185,    -1,   186,    -1,   187,    -1,   188,
      -1,   189,    -1,   190,    -1,   212,     9,   214,    -1,   214,
      -1,   215,     9,   215,    -1,   215,    -1,   216,    -1,   147,
     216,    -1,   216,    92,   211,    -1,   147,   216,    92,   211,
      -1,   216,    -1,   147,   216,    -1,   216,    92,   211,    -1,
     147,   216,    92,   211,    -1,   211,    -1,   216,   147,   211,
      -1,   216,    -1,   144,   147,   216,    -1,   147,   216,    -1,
     217,    -1,   217,   441,    -1,   217,   441,    -1,   221,     9,
     439,    14,   385,    -1,   101,   439,    14,   385,    -1,   222,
     223,    -1,    -1,   224,    -1,   240,    -1,   244,    -1,   249,
      -1,   197,   222,   198,    -1,    66,   317,   224,   271,   273,
      -1,    66,   317,    27,   222,   272,   274,    69,   196,    -1,
      -1,    84,   317,   225,   265,    -1,    -1,    83,   226,   224,
      84,   317,   196,    -1,    -1,    86,   194,   319,   196,   319,
     196,   319,   195,   227,   263,    -1,    -1,    93,   317,   228,
     268,    -1,    97,   196,    -1,    97,   326,   196,    -1,    99,
     196,    -1,    99,   326,   196,    -1,   102,   196,    -1,   102,
     326,   196,    -1,   148,    97,   196,    -1,   107,   281,   196,
      -1,   113,   283,   196,    -1,    82,   318,   196,    -1,   115,
     194,   435,   195,   196,    -1,   196,    -1,    77,    -1,    -1,
      88,   194,   326,    92,   262,   261,   195,   229,   264,    -1,
      90,   194,   267,   195,   266,    -1,    -1,   103,   232,   104,
     194,   378,    75,   195,   197,   222,   198,   234,   230,   237,
      -1,    -1,   103,   232,   162,   231,   235,    -1,   105,   326,
     196,    -1,    98,   211,   196,    -1,   326,   196,    -1,   320,
     196,    -1,   321,   196,    -1,   322,   196,    -1,   323,   196,
      -1,   324,   196,    -1,   102,   323,   196,    -1,   325,   196,
      -1,   348,   196,    -1,   102,   347,   196,    -1,   211,    27,
      -1,    -1,   197,   233,   222,   198,    -1,   234,   104,   194,
     378,    75,   195,   197,   222,   198,    -1,    -1,    -1,   197,
     236,   222,   198,    -1,   162,   235,    -1,    -1,    32,    -1,
      -1,   100,    -1,    -1,   239,   238,   440,   241,   194,   277,
     195,   444,   306,    -1,    -1,   310,   239,   238,   440,   242,
     194,   277,   195,   444,   306,    -1,    -1,   405,   309,   239,
     238,   440,   243,   194,   277,   195,   444,   306,    -1,    -1,
     255,   252,   245,   256,   257,   197,   284,   198,    -1,    -1,
     405,   255,   252,   246,   256,   257,   197,   284,   198,    -1,
      -1,   120,   253,   247,   258,   197,   284,   198,    -1,    -1,
     405,   120,   253,   248,   258,   197,   284,   198,    -1,    -1,
     157,   254,   250,   257,   197,   284,   198,    -1,    -1,   405,
     157,   254,   251,   257,   197,   284,   198,    -1,   440,    -1,
     149,    -1,   440,    -1,   440,    -1,   119,    -1,   112,   119,
      -1,   111,   119,    -1,   121,   378,    -1,    -1,   122,   259,
      -1,    -1,   121,   259,    -1,    -1,   378,    -1,   259,     9,
     378,    -1,   378,    -1,   260,     9,   378,    -1,   124,   262,
      -1,    -1,   412,    -1,    32,   412,    -1,   125,   194,   424,
     195,    -1,   224,    -1,    27,   222,    87,   196,    -1,   224,
      -1,    27,   222,    89,   196,    -1,   224,    -1,    27,   222,
      85,   196,    -1,   224,    -1,    27,   222,    91,   196,    -1,
     211,    14,   385,    -1,   267,     9,   211,    14,   385,    -1,
     197,   269,   198,    -1,   197,   196,   269,   198,    -1,    27,
     269,    94,   196,    -1,    27,   196,   269,    94,   196,    -1,
     269,    95,   326,   270,   222,    -1,   269,    96,   270,   222,
      -1,    -1,    27,    -1,   196,    -1,   271,    67,   317,   224,
      -1,    -1,   272,    67,   317,    27,   222,    -1,    -1,    68,
     224,    -1,    -1,    68,    27,   222,    -1,    -1,   276,     9,
     406,   312,   451,   158,    75,    -1,   276,     9,   406,   312,
     451,   158,    -1,   276,   390,    -1,   406,   312,   451,   158,
      75,    -1,   406,   312,   451,   158,    -1,    -1,   406,   312,
     451,    75,    -1,   406,   312,   451,    32,    75,    -1,   406,
     312,   451,    32,    75,    14,   385,    -1,   406,   312,   451,
      75,    14,   385,    -1,   276,     9,   406,   312,   451,    75,
      -1,   276,     9,   406,   312,   451,    32,    75,    -1,   276,
       9,   406,   312,   451,    32,    75,    14,   385,    -1,   276,
       9,   406,   312,   451,    75,    14,   385,    -1,   278,     9,
     406,   451,   158,    75,    -1,   278,     9,   406,   451,   158,
      -1,   278,   390,    -1,   406,   451,   158,    75,    -1,   406,
     451,   158,    -1,    -1,   406,   451,    75,    -1,   406,   451,
      32,    75,    -1,   406,   451,    32,    75,    14,   385,    -1,
     406,   451,    75,    14,   385,    -1,   278,     9,   406,   451,
      75,    -1,   278,     9,   406,   451,    32,    75,    -1,   278,
       9,   406,   451,    32,    75,    14,   385,    -1,   278,     9,
     406,   451,    75,    14,   385,    -1,   280,   390,    -1,    -1,
     326,    -1,    32,   412,    -1,   280,     9,   326,    -1,   280,
       9,    32,   412,    -1,   281,     9,   282,    -1,   282,    -1,
      75,    -1,   199,   412,    -1,   199,   197,   326,   198,    -1,
     283,     9,    75,    -1,   283,     9,    75,    14,   385,    -1,
      75,    -1,    75,    14,   385,    -1,   284,   285,    -1,    -1,
      -1,   308,   286,   314,   196,    -1,    -1,   310,   450,   287,
     314,   196,    -1,   315,   196,    -1,    -1,   309,   239,   238,
     440,   194,   288,   275,   195,   444,   307,    -1,    -1,   405,
     309,   239,   238,   440,   194,   289,   275,   195,   444,   307,
      -1,   151,   294,   196,    -1,   152,   300,   196,    -1,   154,
     302,   196,    -1,     4,   121,   378,   196,    -1,     4,   122,
     378,   196,    -1,   106,   260,   196,    -1,   106,   260,   197,
     290,   198,    -1,   290,   291,    -1,   290,   292,    -1,    -1,
     220,   143,   211,   159,   260,   196,    -1,   293,    92,   309,
     211,   196,    -1,   293,    92,   310,   196,    -1,   220,   143,
     211,    -1,   211,    -1,   295,    -1,   294,     9,   295,    -1,
     296,   375,   298,   299,    -1,   149,    -1,   126,    -1,   378,
      -1,   114,    -1,   155,   197,   297,   198,    -1,   384,    -1,
     297,     9,   384,    -1,    14,   385,    -1,    -1,    52,   156,
      -1,    -1,   301,    -1,   300,     9,   301,    -1,   153,    -1,
     303,    -1,   211,    -1,   117,    -1,   194,   304,   195,    -1,
     194,   304,   195,    46,    -1,   194,   304,   195,    26,    -1,
     194,   304,   195,    43,    -1,   303,    -1,   305,    -1,   305,
      46,    -1,   305,    26,    -1,   305,    43,    -1,   304,     9,
     304,    -1,   304,    30,   304,    -1,   211,    -1,   149,    -1,
     153,    -1,   196,    -1,   197,   222,   198,    -1,   196,    -1,
     197,   222,   198,    -1,   310,    -1,   114,    -1,   310,    -1,
      -1,   311,    -1,   310,   311,    -1,   108,    -1,   109,    -1,
     110,    -1,   113,    -1,   112,    -1,   111,    -1,   176,    -1,
     313,    -1,    -1,   108,    -1,   109,    -1,   110,    -1,   314,
       9,    75,    -1,   314,     9,    75,    14,   385,    -1,    75,
      -1,    75,    14,   385,    -1,   315,     9,   439,    14,   385,
      -1,   101,   439,    14,   385,    -1,   194,   316,   195,    -1,
      64,   380,   383,    -1,    63,   326,    -1,   367,    -1,   343,
      -1,   194,   326,   195,    -1,   318,     9,   326,    -1,   326,
      -1,   318,    -1,    -1,   148,   326,    -1,   148,   326,   124,
     326,    -1,   412,    14,   320,    -1,   125,   194,   424,   195,
      14,   320,    -1,   175,   326,    -1,   412,    14,   323,    -1,
     125,   194,   424,   195,    14,   323,    -1,   327,    -1,   412,
      -1,   316,    -1,   125,   194,   424,   195,    14,   326,    -1,
     412,    14,   326,    -1,   412,    14,    32,   412,    -1,   412,
      14,    32,    64,   380,   383,    -1,   412,    25,   326,    -1,
     412,    24,   326,    -1,   412,    23,   326,    -1,   412,    22,
     326,    -1,   412,    21,   326,    -1,   412,    20,   326,    -1,
     412,    19,   326,    -1,   412,    18,   326,    -1,   412,    17,
     326,    -1,   412,    16,   326,    -1,   412,    15,   326,    -1,
     412,    61,    -1,    61,   412,    -1,   412,    60,    -1,    60,
     412,    -1,   326,    28,   326,    -1,   326,    29,   326,    -1,
     326,    10,   326,    -1,   326,    12,   326,    -1,   326,    11,
     326,    -1,   326,    30,   326,    -1,   326,    32,   326,    -1,
     326,    31,   326,    -1,   326,    45,   326,    -1,   326,    43,
     326,    -1,   326,    44,   326,    -1,   326,    46,   326,    -1,
     326,    47,   326,    -1,   326,    48,   326,    -1,   326,    42,
     326,    -1,   326,    41,   326,    -1,    43,   326,    -1,    44,
     326,    -1,    49,   326,    -1,    51,   326,    -1,   326,    34,
     326,    -1,   326,    33,   326,    -1,   326,    36,   326,    -1,
     326,    35,   326,    -1,   326,    37,   326,    -1,   326,    40,
     326,    -1,   326,    38,   326,    -1,   326,    39,   326,    -1,
     326,    50,   380,    -1,   194,   327,   195,    -1,   326,    26,
     326,    27,   326,    -1,   326,    26,    27,   326,    -1,   434,
      -1,    59,   326,    -1,    58,   326,    -1,    57,   326,    -1,
      56,   326,    -1,    55,   326,    -1,    54,   326,    -1,    53,
     326,    -1,    65,   381,    -1,    52,   326,    -1,   387,    -1,
     342,    -1,   341,    -1,   200,   382,   200,    -1,    13,   326,
      -1,   329,    -1,   332,    -1,   345,    -1,   106,   194,   366,
     390,   195,    -1,    -1,    -1,   239,   238,   194,   330,   277,
     195,   444,   328,   197,   222,   198,    -1,    -1,   310,   239,
     238,   194,   331,   277,   195,   444,   328,   197,   222,   198,
      -1,    -1,    75,   333,   335,    -1,    -1,   191,   334,   277,
     192,   444,   335,    -1,     8,   326,    -1,     8,   197,   222,
     198,    -1,    81,    -1,   337,     9,   336,   124,   326,    -1,
     336,   124,   326,    -1,   338,     9,   336,   124,   385,    -1,
     336,   124,   385,    -1,   337,   389,    -1,    -1,   338,   389,
      -1,    -1,   169,   194,   339,   195,    -1,   126,   194,   425,
     195,    -1,    62,   425,   201,    -1,   378,   197,   427,   198,
      -1,   378,   197,   429,   198,    -1,   345,    62,   420,   201,
      -1,   346,    62,   420,   201,    -1,   342,    -1,   436,    -1,
     194,   327,   195,    -1,   349,   350,    -1,   412,    14,   347,
      -1,   177,    75,   180,   326,    -1,   351,   362,    -1,   351,
     362,   365,    -1,   362,    -1,   362,   365,    -1,   352,    -1,
     351,   352,    -1,   353,    -1,   354,    -1,   355,    -1,   356,
      -1,   357,    -1,   358,    -1,   177,    75,   180,   326,    -1,
     184,    75,    14,   326,    -1,   178,   326,    -1,   179,    75,
     180,   326,   181,   326,   182,   326,    -1,   179,    75,   180,
     326,   181,   326,   182,   326,   183,    75,    -1,   185,   359,
      -1,   360,    -1,   359,     9,   360,    -1,   326,    -1,   326,
     361,    -1,   186,    -1,   187,    -1,   363,    -1,   364,    -1,
     188,   326,    -1,   189,   326,   190,   326,    -1,   183,    75,
     350,    -1,   366,     9,    75,    -1,   366,     9,    32,    75,
      -1,    75,    -1,    32,    75,    -1,   163,   149,   368,   164,
      -1,   370,    47,    -1,   370,   164,   371,   163,    47,   369,
      -1,    -1,   149,    -1,   370,   372,    14,   373,    -1,    -1,
     371,   374,    -1,    -1,   149,    -1,   150,    -1,   197,   326,
     198,    -1,   150,    -1,   197,   326,   198,    -1,   367,    -1,
     376,    -1,   375,    27,   376,    -1,   375,    44,   376,    -1,
     211,    -1,    65,    -1,   100,    -1,   101,    -1,   102,    -1,
     148,    -1,   175,    -1,   103,    -1,   104,    -1,   162,    -1,
     105,    -1,    66,    -1,    67,    -1,    69,    -1,    68,    -1,
      84,    -1,    85,    -1,    83,    -1,    86,    -1,    87,    -1,
      88,    -1,    89,    -1,    90,    -1,    91,    -1,    50,    -1,
      92,    -1,    93,    -1,    94,    -1,    95,    -1,    96,    -1,
      97,    -1,    99,    -1,    98,    -1,    82,    -1,    13,    -1,
     119,    -1,   120,    -1,   121,    -1,   122,    -1,    64,    -1,
      63,    -1,   114,    -1,     5,    -1,     7,    -1,     6,    -1,
       4,    -1,     3,    -1,   144,    -1,   106,    -1,   107,    -1,
     116,    -1,   117,    -1,   118,    -1,   113,    -1,   112,    -1,
     111,    -1,   110,    -1,   109,    -1,   108,    -1,   176,    -1,
     115,    -1,   125,    -1,   126,    -1,    10,    -1,    12,    -1,
      11,    -1,   128,    -1,   130,    -1,   129,    -1,   131,    -1,
     132,    -1,   146,    -1,   145,    -1,   174,    -1,   157,    -1,
     160,    -1,   159,    -1,   170,    -1,   172,    -1,   169,    -1,
     219,   194,   279,   195,    -1,   220,    -1,   149,    -1,   378,
      -1,   113,    -1,   418,    -1,   378,    -1,   113,    -1,   422,
      -1,   194,   195,    -1,   317,    -1,    -1,    -1,    80,    -1,
     431,    -1,   194,   279,   195,    -1,    -1,    70,    -1,    71,
      -1,    72,    -1,    81,    -1,   131,    -1,   132,    -1,   146,
      -1,   128,    -1,   160,    -1,   129,    -1,   130,    -1,   145,
      -1,   174,    -1,   139,    80,   140,    -1,   139,   140,    -1,
     384,    -1,   218,    -1,    43,   385,    -1,    44,   385,    -1,
     126,   194,   388,   195,    -1,    62,   388,   201,    -1,   169,
     194,   340,   195,    -1,   386,    -1,   344,    -1,   220,   143,
     211,    -1,   149,   143,   211,    -1,   220,   143,   119,    -1,
     218,    -1,    74,    -1,   436,    -1,   384,    -1,   202,   431,
     202,    -1,   203,   431,   203,    -1,   139,   431,   140,    -1,
     391,   389,    -1,    -1,     9,    -1,    -1,     9,    -1,    -1,
     391,     9,   385,   124,   385,    -1,   391,     9,   385,    -1,
     385,   124,   385,    -1,   385,    -1,    70,    -1,    71,    -1,
      72,    -1,    81,    -1,   139,    80,   140,    -1,   139,   140,
      -1,    70,    -1,    71,    -1,    72,    -1,   211,    -1,   392,
      -1,   211,    -1,    43,   393,    -1,    44,   393,    -1,   126,
     194,   395,   195,    -1,    62,   395,   201,    -1,   169,   194,
     398,   195,    -1,   396,   389,    -1,    -1,   396,     9,   394,
     124,   394,    -1,   396,     9,   394,    -1,   394,   124,   394,
      -1,   394,    -1,   397,     9,   394,    -1,   394,    -1,   399,
     389,    -1,    -1,   399,     9,   336,   124,   394,    -1,   336,
     124,   394,    -1,   397,   389,    -1,    -1,   194,   400,   195,
      -1,    -1,   402,     9,   211,   401,    -1,   211,   401,    -1,
      -1,   404,   402,   389,    -1,    42,   403,    41,    -1,   405,
      -1,    -1,   408,    -1,   123,   417,    -1,   123,   211,    -1,
     123,   197,   326,   198,    -1,    62,   420,   201,    -1,   197,
     326,   198,    -1,   413,   409,    -1,   194,   316,   195,   409,
      -1,   423,   409,    -1,   194,   316,   195,   409,    -1,   417,
      -1,   377,    -1,   415,    -1,   416,    -1,   410,    -1,   412,
     407,    -1,   194,   316,   195,   407,    -1,   379,   143,   417,
      -1,   414,   194,   279,   195,    -1,   194,   412,   195,    -1,
     377,    -1,   415,    -1,   416,    -1,   410,    -1,   412,   408,
      -1,   194,   316,   195,   408,    -1,   414,   194,   279,   195,
      -1,   194,   412,   195,    -1,   417,    -1,   410,    -1,   194,
     412,   195,    -1,   412,   123,   211,   441,   194,   279,   195,
      -1,   412,   123,   417,   194,   279,   195,    -1,   412,   123,
     197,   326,   198,   194,   279,   195,    -1,   194,   316,   195,
     123,   211,   441,   194,   279,   195,    -1,   194,   316,   195,
     123,   417,   194,   279,   195,    -1,   194,   316,   195,   123,
     197,   326,   198,   194,   279,   195,    -1,   379,   143,   211,
     441,   194,   279,   195,    -1,   379,   143,   417,   194,   279,
     195,    -1,   379,   143,   197,   326,   198,   194,   279,   195,
      -1,   418,    -1,   421,   418,    -1,   418,    62,   420,   201,
      -1,   418,   197,   326,   198,    -1,   419,    -1,    75,    -1,
     199,   197,   326,   198,    -1,   326,    -1,    -1,   199,    -1,
     421,   199,    -1,   417,    -1,   411,    -1,   422,   407,    -1,
     194,   316,   195,   407,    -1,   379,   143,   417,    -1,   194,
     412,   195,    -1,    -1,   411,    -1,   422,   408,    -1,   194,
     316,   195,   408,    -1,   194,   412,   195,    -1,   424,     9,
      -1,   424,     9,   412,    -1,   424,     9,   125,   194,   424,
     195,    -1,    -1,   412,    -1,   125,   194,   424,   195,    -1,
     426,   389,    -1,    -1,   426,     9,   326,   124,   326,    -1,
     426,     9,   326,    -1,   326,   124,   326,    -1,   326,    -1,
     426,     9,   326,   124,    32,   412,    -1,   426,     9,    32,
     412,    -1,   326,   124,    32,   412,    -1,    32,   412,    -1,
     428,   389,    -1,    -1,   428,     9,   326,   124,   326,    -1,
     428,     9,   326,    -1,   326,   124,   326,    -1,   326,    -1,
     430,   389,    -1,    -1,   430,     9,   385,   124,   385,    -1,
     430,     9,   385,    -1,   385,   124,   385,    -1,   385,    -1,
     431,   432,    -1,   431,    80,    -1,   432,    -1,    80,   432,
      -1,    75,    -1,    75,    62,   433,   201,    -1,    75,   123,
     211,    -1,   141,   326,   198,    -1,   141,    74,    62,   326,
     201,   198,    -1,   142,   412,   198,    -1,   211,    -1,    76,
      -1,    75,    -1,   116,   194,   435,   195,    -1,   117,   194,
     412,   195,    -1,   117,   194,   327,   195,    -1,   117,   194,
     316,   195,    -1,     7,   326,    -1,     6,   326,    -1,     5,
     194,   326,   195,    -1,     4,   326,    -1,     3,   326,    -1,
     412,    -1,   435,     9,   412,    -1,   379,   143,   211,    -1,
     379,   143,   119,    -1,    -1,    92,   450,    -1,   170,   440,
      14,   450,   196,    -1,   172,   440,   437,    14,   450,   196,
      -1,   211,    -1,   450,   211,    -1,   211,    -1,   211,   165,
     445,   166,    -1,   165,   442,   166,    -1,    -1,   450,    -1,
     442,     9,   450,    -1,   442,     9,   158,    -1,   442,    -1,
     158,    -1,    -1,    -1,    27,   450,    -1,   445,     9,   211,
      -1,   211,    -1,   445,     9,   211,    92,   450,    -1,   211,
      92,   450,    -1,    81,   124,   450,    -1,   447,     9,   446,
      -1,   446,    -1,   447,   389,    -1,    -1,   169,   194,   448,
     195,    -1,    26,   450,    -1,    52,   450,    -1,   220,    -1,
     126,    -1,   127,    -1,   449,    -1,   126,   165,   450,   166,
      -1,   126,   165,   450,     9,   450,   166,    -1,   149,    -1,
     194,   100,   194,   443,   195,    27,   450,   195,    -1,   194,
     442,     9,   450,   195,    -1,   450,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   749,   749,   749,   758,   760,   763,   764,   765,   766,
     767,   768,   771,   773,   773,   775,   775,   777,   778,   780,
     785,   786,   787,   788,   789,   790,   791,   792,   793,   794,
     795,   796,   797,   798,   799,   800,   801,   802,   803,   807,
     809,   813,   815,   819,   820,   821,   822,   827,   828,   829,
     830,   835,   836,   840,   841,   843,   846,   852,   859,   866,
     870,   876,   878,   881,   882,   883,   884,   887,   888,   892,
     897,   897,   903,   903,   910,   909,   915,   915,   920,   921,
     922,   923,   924,   925,   926,   927,   928,   929,   930,   931,
     932,   935,   933,   940,   948,   942,   952,   950,   954,   955,
     959,   960,   961,   962,   963,   964,   965,   966,   967,   968,
     969,   977,   977,   982,   988,   992,   992,  1000,  1001,  1005,
    1006,  1010,  1015,  1014,  1027,  1025,  1039,  1037,  1053,  1052,
    1071,  1069,  1088,  1087,  1096,  1094,  1106,  1105,  1117,  1115,
    1128,  1129,  1133,  1136,  1139,  1140,  1141,  1144,  1146,  1149,
    1150,  1153,  1154,  1157,  1158,  1162,  1163,  1168,  1169,  1172,
    1173,  1174,  1178,  1179,  1183,  1184,  1188,  1189,  1193,  1194,
    1199,  1200,  1205,  1206,  1207,  1208,  1211,  1214,  1216,  1219,
    1220,  1224,  1226,  1229,  1232,  1235,  1236,  1239,  1240,  1244,
    1250,  1257,  1259,  1264,  1270,  1274,  1278,  1282,  1287,  1292,
    1297,  1302,  1308,  1317,  1322,  1328,  1330,  1334,  1339,  1343,
    1346,  1349,  1353,  1357,  1361,  1365,  1370,  1378,  1380,  1383,
    1384,  1385,  1387,  1392,  1393,  1396,  1397,  1398,  1402,  1403,
    1405,  1406,  1410,  1412,  1415,  1415,  1419,  1418,  1422,  1426,
    1424,  1439,  1436,  1449,  1451,  1453,  1455,  1457,  1459,  1461,
    1465,  1466,  1467,  1470,  1476,  1479,  1485,  1488,  1493,  1495,
    1500,  1505,  1509,  1510,  1516,  1517,  1522,  1523,  1528,  1529,
    1533,  1534,  1538,  1540,  1546,  1551,  1552,  1554,  1558,  1559,
    1560,  1561,  1565,  1566,  1567,  1568,  1569,  1570,  1572,  1577,
    1580,  1581,  1585,  1586,  1590,  1591,  1594,  1595,  1598,  1599,
    1602,  1603,  1607,  1608,  1609,  1610,  1611,  1612,  1613,  1617,
    1618,  1621,  1622,  1623,  1626,  1628,  1630,  1631,  1634,  1636,
    1640,  1641,  1643,  1644,  1645,  1648,  1652,  1653,  1657,  1658,
    1662,  1663,  1667,  1671,  1676,  1680,  1684,  1689,  1690,  1691,
    1694,  1696,  1697,  1698,  1701,  1702,  1703,  1704,  1705,  1706,
    1707,  1708,  1709,  1710,  1711,  1712,  1713,  1714,  1715,  1716,
    1717,  1718,  1719,  1720,  1721,  1722,  1723,  1724,  1725,  1726,
    1727,  1728,  1729,  1730,  1731,  1732,  1733,  1734,  1735,  1736,
    1737,  1738,  1739,  1740,  1741,  1743,  1744,  1746,  1748,  1749,
    1750,  1751,  1752,  1753,  1754,  1755,  1756,  1757,  1758,  1759,
    1760,  1761,  1762,  1763,  1764,  1765,  1766,  1767,  1768,  1772,
    1776,  1781,  1780,  1795,  1793,  1810,  1810,  1825,  1825,  1843,
    1844,  1849,  1854,  1858,  1864,  1868,  1874,  1876,  1880,  1882,
    1886,  1890,  1891,  1895,  1902,  1909,  1911,  1916,  1917,  1918,
    1922,  1926,  1930,  1934,  1936,  1938,  1940,  1945,  1946,  1951,
    1952,  1953,  1954,  1955,  1956,  1960,  1964,  1968,  1972,  1977,
    1982,  1986,  1987,  1991,  1992,  1996,  1997,  2001,  2002,  2006,
    2010,  2014,  2018,  2019,  2020,  2021,  2025,  2031,  2040,  2053,
    2054,  2057,  2060,  2063,  2064,  2067,  2071,  2074,  2077,  2084,
    2085,  2089,  2090,  2092,  2096,  2097,  2098,  2099,  2100,  2101,
    2102,  2103,  2104,  2105,  2106,  2107,  2108,  2109,  2110,  2111,
    2112,  2113,  2114,  2115,  2116,  2117,  2118,  2119,  2120,  2121,
    2122,  2123,  2124,  2125,  2126,  2127,  2128,  2129,  2130,  2131,
    2132,  2133,  2134,  2135,  2136,  2137,  2138,  2139,  2140,  2141,
    2142,  2143,  2144,  2145,  2146,  2147,  2148,  2149,  2150,  2151,
    2152,  2153,  2154,  2155,  2156,  2157,  2158,  2159,  2160,  2161,
    2162,  2163,  2164,  2165,  2166,  2167,  2168,  2169,  2170,  2171,
    2172,  2173,  2174,  2175,  2179,  2184,  2185,  2188,  2189,  2190,
    2194,  2195,  2196,  2200,  2201,  2202,  2206,  2207,  2208,  2211,
    2213,  2217,  2218,  2219,  2220,  2222,  2223,  2224,  2225,  2226,
    2227,  2228,  2229,  2230,  2231,  2234,  2239,  2240,  2241,  2242,
    2243,  2245,  2246,  2248,  2249,  2253,  2256,  2259,  2265,  2266,
    2267,  2268,  2269,  2270,  2271,  2276,  2278,  2282,  2283,  2286,
    2287,  2291,  2294,  2296,  2298,  2302,  2303,  2304,  2305,  2307,
    2310,  2314,  2315,  2316,  2317,  2320,  2321,  2322,  2323,  2324,
    2326,  2327,  2332,  2334,  2337,  2340,  2342,  2344,  2347,  2349,
    2353,  2355,  2358,  2361,  2367,  2369,  2372,  2373,  2378,  2381,
    2385,  2385,  2390,  2393,  2394,  2398,  2399,  2404,  2405,  2409,
    2410,  2414,  2415,  2420,  2422,  2427,  2428,  2429,  2430,  2431,
    2432,  2433,  2435,  2438,  2440,  2444,  2445,  2446,  2447,  2448,
    2450,  2452,  2454,  2458,  2459,  2460,  2464,  2467,  2470,  2473,
    2477,  2481,  2488,  2492,  2496,  2503,  2504,  2509,  2511,  2512,
    2515,  2516,  2519,  2520,  2524,  2525,  2529,  2530,  2531,  2532,
    2534,  2537,  2540,  2541,  2542,  2544,  2546,  2550,  2551,  2552,
    2554,  2555,  2556,  2560,  2562,  2565,  2567,  2568,  2569,  2570,
    2573,  2575,  2576,  2580,  2582,  2585,  2587,  2588,  2589,  2593,
    2595,  2598,  2601,  2603,  2605,  2609,  2610,  2612,  2613,  2619,
    2620,  2622,  2624,  2626,  2628,  2631,  2632,  2633,  2637,  2638,
    2639,  2640,  2641,  2642,  2643,  2644,  2645,  2649,  2650,  2654,
    2656,  2664,  2666,  2670,  2674,  2681,  2682,  2688,  2689,  2696,
    2699,  2703,  2706,  2711,  2712,  2713,  2714,  2718,  2719,  2723,
    2725,  2726,  2728,  2732,  2738,  2740,  2744,  2747,  2750,  2758,
    2761,  2764,  2765,  2768,  2771,  2772,  2775,  2779,  2783,  2789,
    2797,  2798
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_REQUIRE_ONCE", "T_REQUIRE", "T_EVAL",
  "T_INCLUDE_ONCE", "T_INCLUDE", "T_LAMBDA_ARROW", "','", "T_LOGICAL_OR",
  "T_LOGICAL_XOR", "T_LOGICAL_AND", "T_PRINT", "'='", "T_SR_EQUAL",
  "T_SL_EQUAL", "T_XOR_EQUAL", "T_OR_EQUAL", "T_AND_EQUAL", "T_MOD_EQUAL",
  "T_CONCAT_EQUAL", "T_DIV_EQUAL", "T_MUL_EQUAL", "T_MINUS_EQUAL",
  "T_PLUS_EQUAL", "'?'", "':'", "T_BOOLEAN_OR", "T_BOOLEAN_AND", "'|'",
  "'^'", "'&'", "T_IS_NOT_IDENTICAL", "T_IS_IDENTICAL", "T_IS_NOT_EQUAL",
  "T_IS_EQUAL", "'<'", "'>'", "T_IS_GREATER_OR_EQUAL",
  "T_IS_SMALLER_OR_EQUAL", "T_SR", "T_SL", "'+'", "'-'", "'.'", "'*'",
  "'/'", "'%'", "'!'", "T_INSTANCEOF", "'~'", "'@'", "T_UNSET_CAST",
  "T_BOOL_CAST", "T_OBJECT_CAST", "T_ARRAY_CAST", "T_STRING_CAST",
  "T_DOUBLE_CAST", "T_INT_CAST", "T_DEC", "T_INC", "'['", "T_CLONE",
  "T_NEW", "T_EXIT", "T_IF", "T_ELSEIF", "T_ELSE", "T_ENDIF", "T_LNUMBER",
  "T_DNUMBER", "T_ONUMBER", "T_STRING", "T_STRING_VARNAME", "T_VARIABLE",
  "T_NUM_STRING", "T_INLINE_HTML", "T_CHARACTER", "T_BAD_CHARACTER",
  "T_ENCAPSED_AND_WHITESPACE", "T_CONSTANT_ENCAPSED_STRING", "T_ECHO",
  "T_DO", "T_WHILE", "T_ENDWHILE", "T_FOR", "T_ENDFOR", "T_FOREACH",
  "T_ENDFOREACH", "T_DECLARE", "T_ENDDECLARE", "T_AS", "T_SWITCH",
  "T_ENDSWITCH", "T_CASE", "T_DEFAULT", "T_BREAK", "T_GOTO", "T_CONTINUE",
  "T_FUNCTION", "T_CONST", "T_RETURN", "T_TRY", "T_CATCH", "T_THROW",
  "T_USE", "T_GLOBAL", "T_PUBLIC", "T_PROTECTED", "T_PRIVATE", "T_FINAL",
  "T_ABSTRACT", "T_STATIC", "T_VAR", "T_UNSET", "T_ISSET", "T_EMPTY",
  "T_HALT_COMPILER", "T_CLASS", "T_INTERFACE", "T_EXTENDS", "T_IMPLEMENTS",
  "T_OBJECT_OPERATOR", "T_DOUBLE_ARROW", "T_LIST", "T_ARRAY", "T_CALLABLE",
  "T_CLASS_C", "T_METHOD_C", "T_FUNC_C", "T_LINE", "T_FILE", "T_COMMENT",
  "T_DOC_COMMENT", "T_OPEN_TAG", "T_OPEN_TAG_WITH_ECHO", "T_CLOSE_TAG",
  "T_WHITESPACE", "T_START_HEREDOC", "T_END_HEREDOC",
  "T_DOLLAR_OPEN_CURLY_BRACES", "T_CURLY_OPEN", "T_DOUBLE_COLON",
  "T_NAMESPACE", "T_NS_C", "T_DIR", "T_NS_SEPARATOR", "T_YIELD",
  "T_XHP_LABEL", "T_XHP_TEXT", "T_XHP_ATTRIBUTE", "T_XHP_CATEGORY",
  "T_XHP_CATEGORY_LABEL", "T_XHP_CHILDREN", "T_XHP_ENUM", "T_XHP_REQUIRED",
  "T_TRAIT", "\"...\"", "T_INSTEADOF", "T_TRAIT_C", "T_HH_ERROR",
  "T_FINALLY", "T_XHP_TAG_LT", "T_XHP_TAG_GT", "T_TYPELIST_LT",
  "T_TYPELIST_GT", "T_UNRESOLVED_LT", "T_COLLECTION", "T_SHAPE", "T_TYPE",
  "T_UNRESOLVED_TYPE", "T_NEWTYPE", "T_UNRESOLVED_NEWTYPE",
  "T_COMPILER_HALT_OFFSET", "T_AWAIT", "T_ASYNC", "T_FROM", "T_WHERE",
  "T_JOIN", "T_IN", "T_ON", "T_EQUALS", "T_INTO", "T_LET", "T_ORDERBY",
  "T_ASCENDING", "T_DESCENDING", "T_SELECT", "T_GROUP", "T_BY",
  "T_LAMBDA_OP", "T_LAMBDA_CP", "T_UNRESOLVED_OP", "'('", "')'", "';'",
  "'{'", "'}'", "'$'", "'`'", "']'", "'\"'", "'\\''", "$accept", "start",
  "$@1", "top_statement_list", "top_statement", "$@2", "$@3", "ident",
  "use_declarations", "use_fn_declarations", "use_declaration",
  "use_fn_declaration", "namespace_name", "namespace_string_base",
  "namespace_string", "namespace_string_typeargs",
  "class_namespace_string_typeargs", "constant_declaration",
  "inner_statement_list", "inner_statement", "statement", "$@4", "$@5",
  "$@6", "$@7", "$@8", "$@9", "$@10", "try_statement_list", "$@11",
  "additional_catches", "finally_statement_list", "$@12",
  "optional_finally", "is_reference", "function_loc",
  "function_declaration_statement", "$@13", "$@14", "$@15",
  "class_declaration_statement", "$@16", "$@17", "$@18", "$@19",
  "trait_declaration_statement", "$@20", "$@21", "class_decl_name",
  "interface_decl_name", "trait_decl_name", "class_entry_type",
  "extends_from", "implements_list", "interface_extends_list",
  "interface_list", "trait_list", "foreach_optional_arg",
  "foreach_variable", "for_statement", "foreach_statement",
  "while_statement", "declare_statement", "declare_list",
  "switch_case_list", "case_list", "case_separator", "elseif_list",
  "new_elseif_list", "else_single", "new_else_single",
  "method_parameter_list", "non_empty_method_parameter_list",
  "parameter_list", "non_empty_parameter_list",
  "function_call_parameter_list", "non_empty_fcall_parameter_list",
  "global_var_list", "global_var", "static_var_list",
  "class_statement_list", "class_statement", "$@22", "$@23", "$@24",
  "$@25", "trait_rules", "trait_precedence_rule", "trait_alias_rule",
  "trait_alias_rule_method", "xhp_attribute_stmt", "xhp_attribute_decl",
  "xhp_attribute_decl_type", "xhp_attribute_enum", "xhp_attribute_default",
  "xhp_attribute_is_required", "xhp_category_stmt", "xhp_category_decl",
  "xhp_children_stmt", "xhp_children_paren_expr", "xhp_children_decl_expr",
  "xhp_children_decl_tag", "function_body", "method_body",
  "variable_modifiers", "method_modifiers", "non_empty_member_modifiers",
  "member_modifier", "parameter_modifiers", "parameter_modifier",
  "class_variable_declaration", "class_constant_declaration",
  "expr_with_parens", "parenthesis_expr", "expr_list", "for_expr",
  "yield_expr", "yield_assign_expr", "yield_list_assign_expr",
  "await_expr", "await_assign_expr", "await_list_assign_expr", "expr",
  "expr_no_variable", "lambda_use_vars", "closure_expression", "$@26",
  "$@27", "lambda_expression", "$@28", "$@29", "lambda_body",
  "shape_keyname", "non_empty_shape_pair_list",
  "non_empty_static_shape_pair_list", "shape_pair_list",
  "static_shape_pair_list", "shape_literal", "array_literal",
  "collection_literal", "static_collection_literal", "dim_expr",
  "dim_expr_base", "query_expr", "query_assign_expr", "query_head",
  "query_body", "query_body_clauses", "query_body_clause", "from_clause",
  "let_clause", "where_clause", "join_clause", "join_into_clause",
  "orderby_clause", "orderings", "ordering", "ordering_direction",
  "select_or_group_clause", "select_clause", "group_clause",
  "query_continuation", "lexical_var_list", "xhp_tag", "xhp_tag_body",
  "xhp_opt_end_label", "xhp_attributes", "xhp_children",
  "xhp_attribute_name", "xhp_attribute_value", "xhp_child", "xhp_label_ws",
  "xhp_bareword", "simple_function_call", "fully_qualified_class_name",
  "static_class_name", "class_name_reference", "exit_expr",
  "backticks_expr", "ctor_arguments", "common_scalar", "static_scalar",
  "static_class_constant", "scalar", "static_array_pair_list",
  "possible_comma", "hh_possible_comma",
  "non_empty_static_array_pair_list", "common_scalar_ae",
  "static_numeric_scalar_ae", "static_scalar_ae",
  "static_array_pair_list_ae", "non_empty_static_array_pair_list_ae",
  "non_empty_static_scalar_list_ae", "static_shape_pair_list_ae",
  "non_empty_static_shape_pair_list_ae", "static_scalar_list_ae",
  "attribute_static_scalar_list", "non_empty_user_attribute_list",
  "user_attribute_list", "$@30", "non_empty_user_attributes",
  "optional_user_attributes", "property_access",
  "property_access_without_variables", "array_access",
  "dimmable_variable_access", "dimmable_variable_no_calls_access",
  "variable", "dimmable_variable", "callable_variable",
  "object_method_call", "class_method_call", "variable_without_objects",
  "reference_variable", "compound_variable", "dim_offset",
  "simple_indirect_reference", "variable_no_calls",
  "dimmable_variable_no_calls", "assignment_list", "array_pair_list",
  "non_empty_array_pair_list", "collection_init",
  "non_empty_collection_init", "static_collection_init",
  "non_empty_static_collection_init", "encaps_list", "encaps_var",
  "encaps_var_offset", "internal_functions", "variable_list",
  "class_constant", "hh_opt_constraint", "hh_type_alias_statement",
  "hh_name_with_type", "hh_name_with_typevar", "hh_typeargs_opt",
  "hh_type_list", "hh_func_type_list", "hh_opt_return_type",
  "hh_typevar_list", "hh_shape_member_type",
  "hh_non_empty_shape_member_list", "hh_shape_member_list",
  "hh_shape_type", "hh_type", "hh_type_opt", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,    44,
     264,   265,   266,   267,    61,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,    63,    58,   279,   280,
     124,    94,    38,   281,   282,   283,   284,    60,    62,   285,
     286,   287,   288,    43,    45,    46,    42,    47,    37,    33,
     289,   126,    64,   290,   291,   292,   293,   294,   295,   296,
     297,   298,    91,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
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
     208,   208,   208,   209,   208,   210,   208,   208,   208,   208,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   212,
     212,   213,   213,   214,   214,   214,   214,   215,   215,   215,
     215,   216,   216,   217,   217,   217,   218,   219,   220,   221,
     221,   222,   222,   223,   223,   223,   223,   224,   224,   224,
     225,   224,   226,   224,   227,   224,   228,   224,   224,   224,
     224,   224,   224,   224,   224,   224,   224,   224,   224,   224,
     224,   229,   224,   224,   230,   224,   231,   224,   224,   224,
     224,   224,   224,   224,   224,   224,   224,   224,   224,   224,
     224,   233,   232,   234,   234,   236,   235,   237,   237,   238,
     238,   239,   241,   240,   242,   240,   243,   240,   245,   244,
     246,   244,   247,   244,   248,   244,   250,   249,   251,   249,
     252,   252,   253,   254,   255,   255,   255,   256,   256,   257,
     257,   258,   258,   259,   259,   260,   260,   261,   261,   262,
     262,   262,   263,   263,   264,   264,   265,   265,   266,   266,
     267,   267,   268,   268,   268,   268,   269,   269,   269,   270,
     270,   271,   271,   272,   272,   273,   273,   274,   274,   275,
     275,   275,   275,   275,   275,   276,   276,   276,   276,   276,
     276,   276,   276,   277,   277,   277,   277,   277,   277,   278,
     278,   278,   278,   278,   278,   278,   278,   279,   279,   280,
     280,   280,   280,   281,   281,   282,   282,   282,   283,   283,
     283,   283,   284,   284,   286,   285,   287,   285,   285,   288,
     285,   289,   285,   285,   285,   285,   285,   285,   285,   285,
     290,   290,   290,   291,   292,   292,   293,   293,   294,   294,
     295,   295,   296,   296,   296,   296,   297,   297,   298,   298,
     299,   299,   300,   300,   301,   302,   302,   302,   303,   303,
     303,   303,   304,   304,   304,   304,   304,   304,   304,   305,
     305,   305,   306,   306,   307,   307,   308,   308,   309,   309,
     310,   310,   311,   311,   311,   311,   311,   311,   311,   312,
     312,   313,   313,   313,   314,   314,   314,   314,   315,   315,
     316,   316,   316,   316,   316,   317,   318,   318,   319,   319,
     320,   320,   321,   322,   323,   324,   325,   326,   326,   326,
     327,   327,   327,   327,   327,   327,   327,   327,   327,   327,
     327,   327,   327,   327,   327,   327,   327,   327,   327,   327,
     327,   327,   327,   327,   327,   327,   327,   327,   327,   327,
     327,   327,   327,   327,   327,   327,   327,   327,   327,   327,
     327,   327,   327,   327,   327,   327,   327,   327,   327,   327,
     327,   327,   327,   327,   327,   327,   327,   327,   327,   327,
     327,   327,   327,   327,   327,   327,   327,   327,   327,   328,
     328,   330,   329,   331,   329,   333,   332,   334,   332,   335,
     335,   336,   337,   337,   338,   338,   339,   339,   340,   340,
     341,   342,   342,   343,   344,   345,   345,   346,   346,   346,
     347,   348,   349,   350,   350,   350,   350,   351,   351,   352,
     352,   352,   352,   352,   352,   353,   354,   355,   356,   357,
     358,   359,   359,   360,   360,   361,   361,   362,   362,   363,
     364,   365,   366,   366,   366,   366,   367,   368,   368,   369,
     369,   370,   370,   371,   371,   372,   373,   373,   374,   374,
     374,   375,   375,   375,   376,   376,   376,   376,   376,   376,
     376,   376,   376,   376,   376,   376,   376,   376,   376,   376,
     376,   376,   376,   376,   376,   376,   376,   376,   376,   376,
     376,   376,   376,   376,   376,   376,   376,   376,   376,   376,
     376,   376,   376,   376,   376,   376,   376,   376,   376,   376,
     376,   376,   376,   376,   376,   376,   376,   376,   376,   376,
     376,   376,   376,   376,   376,   376,   376,   376,   376,   376,
     376,   376,   376,   376,   376,   376,   376,   376,   376,   376,
     376,   376,   376,   376,   377,   378,   378,   379,   379,   379,
     380,   380,   380,   381,   381,   381,   382,   382,   382,   383,
     383,   384,   384,   384,   384,   384,   384,   384,   384,   384,
     384,   384,   384,   384,   384,   384,   385,   385,   385,   385,
     385,   385,   385,   385,   385,   386,   386,   386,   387,   387,
     387,   387,   387,   387,   387,   388,   388,   389,   389,   390,
     390,   391,   391,   391,   391,   392,   392,   392,   392,   392,
     392,   393,   393,   393,   393,   394,   394,   394,   394,   394,
     394,   394,   395,   395,   396,   396,   396,   396,   397,   397,
     398,   398,   399,   399,   400,   400,   401,   401,   402,   402,
     404,   403,   405,   406,   406,   407,   407,   408,   408,   409,
     409,   410,   410,   411,   411,   412,   412,   412,   412,   412,
     412,   412,   412,   412,   412,   413,   413,   413,   413,   413,
     413,   413,   413,   414,   414,   414,   415,   415,   415,   415,
     415,   415,   416,   416,   416,   417,   417,   418,   418,   418,
     419,   419,   420,   420,   421,   421,   422,   422,   422,   422,
     422,   422,   423,   423,   423,   423,   423,   424,   424,   424,
     424,   424,   424,   425,   425,   426,   426,   426,   426,   426,
     426,   426,   426,   427,   427,   428,   428,   428,   428,   429,
     429,   430,   430,   430,   430,   431,   431,   431,   431,   432,
     432,   432,   432,   432,   432,   433,   433,   433,   434,   434,
     434,   434,   434,   434,   434,   434,   434,   435,   435,   436,
     436,   437,   437,   438,   438,   439,   439,   440,   440,   441,
     441,   442,   442,   443,   443,   443,   443,   444,   444,   445,
     445,   445,   445,   446,   447,   447,   448,   448,   449,   450,
     450,   450,   450,   450,   450,   450,   450,   450,   450,   450,
     451,   451
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     4,     3,     0,     6,     0,     5,     3,     4,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     3,     1,     1,     2,     3,     4,     1,     2,     3,
       4,     1,     3,     1,     3,     2,     1,     2,     2,     5,
       4,     2,     0,     1,     1,     1,     1,     3,     5,     8,
       0,     4,     0,     6,     0,    10,     0,     4,     2,     3,
       2,     3,     2,     3,     3,     3,     3,     3,     5,     1,
       1,     0,     9,     5,     0,    13,     0,     5,     3,     3,
       2,     2,     2,     2,     2,     2,     3,     2,     2,     3,
       2,     0,     4,     9,     0,     0,     4,     2,     0,     1,
       0,     1,     0,     9,     0,    10,     0,    11,     0,     8,
       0,     9,     0,     7,     0,     8,     0,     7,     0,     8,
       1,     1,     1,     1,     1,     2,     2,     2,     0,     2,
       0,     2,     0,     1,     3,     1,     3,     2,     0,     1,
       2,     4,     1,     4,     1,     4,     1,     4,     1,     4,
       3,     5,     3,     4,     4,     5,     5,     4,     0,     1,
       1,     4,     0,     5,     0,     2,     0,     3,     0,     7,
       6,     2,     5,     4,     0,     4,     5,     7,     6,     6,
       7,     9,     8,     6,     5,     2,     4,     3,     0,     3,
       4,     6,     5,     5,     6,     8,     7,     2,     0,     1,
       2,     3,     4,     3,     1,     1,     2,     4,     3,     5,
       1,     3,     2,     0,     0,     4,     0,     5,     2,     0,
      10,     0,    11,     3,     3,     3,     4,     4,     3,     5,
       2,     2,     0,     6,     5,     4,     3,     1,     1,     3,
       4,     1,     1,     1,     1,     4,     1,     3,     2,     0,
       2,     0,     1,     3,     1,     1,     1,     1,     3,     4,
       4,     4,     1,     1,     2,     2,     2,     3,     3,     1,
       1,     1,     1,     3,     1,     3,     1,     1,     1,     0,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     1,     1,     1,     3,     5,     1,     3,     5,     4,
       3,     3,     2,     1,     1,     3,     3,     1,     1,     0,
       2,     4,     3,     6,     2,     3,     6,     1,     1,     1,
       6,     3,     4,     6,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     5,
       4,     1,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     1,     1,     1,     3,     2,     1,     1,     1,     5,
       0,     0,    11,     0,    12,     0,     3,     0,     6,     2,
       4,     1,     5,     3,     5,     3,     2,     0,     2,     0,
       4,     4,     3,     4,     4,     4,     4,     1,     1,     3,
       2,     3,     4,     2,     3,     1,     2,     1,     2,     1,
       1,     1,     1,     1,     1,     4,     4,     2,     8,    10,
       2,     1,     3,     1,     2,     1,     1,     1,     1,     2,
       4,     3,     3,     4,     1,     2,     4,     2,     6,     0,
       1,     4,     0,     2,     0,     1,     1,     3,     1,     3,
       1,     1,     3,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     4,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     1,     0,     0,     1,     1,     3,
       0,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     2,     1,     1,     2,     2,
       4,     3,     4,     1,     1,     3,     3,     3,     1,     1,
       1,     1,     3,     3,     3,     2,     0,     1,     0,     1,
       0,     5,     3,     3,     1,     1,     1,     1,     1,     3,
       2,     1,     1,     1,     1,     1,     1,     2,     2,     4,
       3,     4,     2,     0,     5,     3,     3,     1,     3,     1,
       2,     0,     5,     3,     2,     0,     3,     0,     4,     2,
       0,     3,     3,     1,     0,     1,     2,     2,     4,     3,
       3,     2,     4,     2,     4,     1,     1,     1,     1,     1,
       2,     4,     3,     4,     3,     1,     1,     1,     1,     2,
       4,     4,     3,     1,     1,     3,     7,     6,     8,     9,
       8,    10,     7,     6,     8,     1,     2,     4,     4,     1,
       1,     4,     1,     0,     1,     2,     1,     1,     2,     4,
       3,     3,     0,     1,     2,     4,     3,     2,     3,     6,
       0,     1,     4,     2,     0,     5,     3,     3,     1,     6,
       4,     4,     2,     2,     0,     5,     3,     3,     1,     2,
       0,     5,     3,     3,     1,     2,     2,     1,     2,     1,
       4,     3,     3,     6,     3,     1,     1,     1,     4,     4,
       4,     4,     2,     2,     4,     2,     2,     1,     3,     3,
       3,     0,     2,     5,     6,     1,     2,     1,     4,     3,
       0,     1,     3,     3,     1,     1,     0,     0,     2,     3,
       1,     5,     3,     3,     3,     1,     2,     0,     4,     2,
       2,     1,     1,     1,     1,     4,     6,     1,     8,     5,
       1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   670,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   744,     0,   732,   585,
       0,   591,   592,   593,    20,   619,   720,    90,   594,     0,
      72,     0,     0,     0,     0,     0,     0,     0,     0,   121,
       0,     0,     0,     0,     0,     0,   302,   303,   304,   307,
     306,   305,     0,     0,     0,     0,   144,     0,     0,     0,
     598,   600,   601,   595,   596,     0,     0,   602,   597,     0,
       0,   576,    21,    22,    23,    25,    24,     0,   599,     0,
       0,     0,     0,   603,     0,   308,    26,    27,    29,    28,
      30,    31,    32,    33,    34,    35,    36,    37,    38,   417,
       0,    89,    62,   724,   586,     0,     0,     4,    51,    53,
      56,   618,     0,   575,     0,     6,   120,     7,     8,     9,
       0,     0,   300,   339,     0,     0,     0,     0,     0,     0,
       0,   337,   406,   407,   403,   402,   324,   408,     0,     0,
     323,   686,   577,     0,   621,   401,   299,   689,   338,     0,
       0,   687,   688,   685,   715,   719,     0,   391,   620,    10,
     307,   306,   305,     0,     0,    51,   120,     0,   786,   338,
     785,     0,   783,   782,   405,     0,     0,   375,   376,   377,
     378,   400,   398,   397,   396,   395,   394,   393,   392,   720,
     578,     0,   800,   577,     0,   358,   356,     0,   748,     0,
     628,   322,   581,     0,   800,   580,     0,   590,   727,   726,
     582,     0,     0,   584,   399,     0,     0,     0,     0,   327,
       0,    70,   329,     0,     0,    76,    78,     0,     0,    80,
       0,     0,     0,   822,   823,   827,     0,     0,    51,   821,
       0,   824,     0,     0,    82,     0,     0,     0,     0,   111,
       0,     0,     0,     0,     0,    40,    43,   225,     0,     0,
     224,   146,   145,   230,     0,     0,     0,     0,     0,   797,
     132,   142,   740,   744,   769,     0,   605,     0,     0,     0,
     767,     0,    15,     0,    55,     0,   330,   136,   143,   482,
     427,     0,   791,   334,   674,   339,     0,   337,   338,     0,
       0,   587,     0,   588,     0,     0,     0,   110,     0,     0,
      58,   218,     0,    19,   119,     0,   141,   128,   140,   305,
     120,   301,   101,   102,   103,   104,   105,   107,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   732,   100,   723,   723,   108,   754,     0,
       0,     0,     0,     0,   298,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   357,   355,     0,
     690,   675,   723,     0,   681,   218,   723,     0,   725,   716,
     740,     0,   120,     0,     0,   672,   667,   628,     0,     0,
       0,     0,   752,     0,   432,   627,   743,     0,     0,    58,
       0,   218,   321,     0,   728,   675,   683,   583,     0,    62,
     182,     0,   416,     0,    87,     0,     0,   328,     0,     0,
       0,     0,     0,    79,    99,    81,   819,   820,     0,   817,
       0,     0,   801,     0,   796,     0,   106,    83,   109,     0,
       0,     0,     0,     0,     0,     0,   440,     0,   447,   449,
     450,   451,   452,   453,   454,   445,   467,   468,    62,     0,
      96,    98,     0,     0,    42,    47,    44,     0,    17,     0,
       0,   226,     0,    85,     0,     0,    86,   787,     0,     0,
     339,   337,   338,     0,     0,   152,     0,   741,     0,     0,
       0,     0,   604,   768,   619,     0,     0,   766,   624,   765,
      54,     5,    12,    13,    84,     0,   150,     0,     0,   421,
       0,   628,     0,     0,     0,     0,     0,   630,   673,   831,
     320,   388,   694,    67,    61,    63,    64,    65,    66,     0,
     404,   622,   623,    52,     0,     0,     0,   630,   219,     0,
     411,   122,   148,     0,   361,   363,   362,     0,     0,   359,
     360,   364,   366,   365,   380,   379,   382,   381,   383,   385,
     386,   384,   374,   373,   368,   369,   367,   370,   371,   372,
     387,   722,     0,     0,   758,     0,   628,   790,     0,   789,
     692,   715,   134,   138,   130,   120,     0,     0,   332,   335,
     341,   441,   354,   353,   352,   351,   350,   349,   348,   347,
     346,   345,   344,     0,   677,   676,     0,     0,     0,     0,
       0,     0,     0,   784,   665,   669,   627,   671,     0,     0,
     800,     0,   747,     0,   746,     0,   731,   730,     0,     0,
     677,   676,   325,   184,   186,    62,   419,   326,     0,    62,
     166,    71,   329,     0,     0,     0,     0,   178,   178,    77,
       0,     0,   815,   628,     0,   806,     0,     0,     0,   626,
       0,     0,   576,     0,    56,   607,   575,   614,     0,   606,
      60,   613,     0,     0,   457,     0,     0,   463,   460,   461,
     469,     0,   448,   443,     0,   446,     0,     0,     0,    48,
      18,     0,     0,     0,    39,    45,     0,   223,   231,   228,
       0,     0,   778,   781,   780,   779,    11,   810,     0,     0,
       0,   740,   737,     0,   431,   777,   776,   775,     0,   771,
       0,   772,   774,     0,     5,   331,     0,     0,   476,   477,
     485,   484,     0,     0,   627,   426,   430,     0,   792,     0,
     807,   674,   205,   830,     0,     0,   691,   675,   682,   721,
       0,   799,   220,   574,   629,   217,     0,   674,     0,     0,
     150,   413,   124,   390,     0,   435,   436,     0,   433,   627,
     753,     0,     0,   218,   152,   150,   148,     0,   732,   342,
       0,     0,   218,   679,   680,   693,   717,   718,     0,     0,
       0,   653,   635,   636,   637,   638,     0,     0,     0,   646,
     645,   659,   628,     0,   667,   751,   750,     0,   729,   675,
     684,   589,     0,   188,     0,     0,    68,     0,     0,     0,
       0,     0,     0,   158,   159,   170,     0,    62,   168,    93,
     178,     0,   178,     0,     0,   825,     0,   627,   816,   818,
     805,   804,     0,   802,   608,   609,   634,     0,   628,   626,
       0,     0,   429,     0,   760,   442,     0,     0,     0,   465,
     466,   464,     0,     0,   444,     0,   112,     0,   115,    97,
       0,    41,    49,    46,   227,     0,   788,    88,     0,     0,
     798,   151,   153,   233,     0,     0,   738,     0,   770,     0,
      16,     0,   149,   233,     0,     0,   423,     0,   793,     0,
       0,     0,   831,     0,   209,   207,     0,   677,   676,   802,
       0,   221,    59,     0,   674,   147,     0,   674,     0,   389,
     757,   756,     0,   218,     0,     0,     0,   150,   126,   590,
     678,   218,     0,     0,   641,   642,   643,   644,   647,   648,
     657,     0,   628,   653,     0,   640,   661,   627,   664,   666,
     668,     0,   745,   678,     0,     0,     0,     0,   185,   420,
      73,     0,   329,   160,   740,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   172,     0,   813,   814,     0,     0,
     829,     0,   611,   627,   625,     0,   616,     0,   628,     0,
     617,   615,   764,     0,   628,   455,     0,   456,   462,   470,
     471,     0,    62,    50,   229,   812,   809,     0,   299,   742,
     740,   333,   336,   340,     0,    14,   299,   488,     0,     0,
     490,   483,   486,     0,   481,     0,   794,   808,   418,     0,
     210,     0,   206,     0,     0,   218,   222,   807,     0,   233,
       0,   674,     0,   218,     0,   713,   233,   233,     0,     0,
     343,   218,     0,   707,     0,   650,   627,   652,     0,   639,
       0,     0,   628,   658,   749,     0,    62,     0,   181,   167,
       0,     0,   157,    91,   171,     0,     0,   174,     0,   179,
     180,    62,   173,   826,   803,     0,   633,   632,   610,     0,
     627,   428,   612,     0,   434,   627,   759,     0,     0,     0,
       0,   154,     0,     0,     0,   297,     0,     0,     0,   133,
     232,   234,     0,   296,     0,   299,     0,   773,   137,   479,
       0,     0,   422,     0,   213,   204,     0,   212,   678,   218,
       0,   410,   807,   299,   807,     0,   755,     0,   712,   299,
     299,   233,   674,     0,   706,   656,   655,   649,     0,   651,
     627,   660,    62,   187,    69,    74,   161,     0,   169,   175,
      62,   177,     0,     0,   425,     0,   763,   762,     0,    62,
     116,   811,     0,     0,     0,     0,   155,   264,   262,   576,
      25,     0,   258,     0,   263,   274,     0,   272,   277,     0,
     276,     0,   275,     0,   120,   236,     0,   238,     0,   739,
     480,   478,   489,   487,   214,     0,   203,   211,   218,     0,
     710,     0,     0,     0,   129,   410,   807,   714,   135,   139,
     299,     0,   708,     0,   663,     0,   183,     0,    62,   164,
      92,   176,   828,   631,     0,     0,     0,     0,     0,     0,
       0,     0,   248,   252,     0,     0,   243,   540,   539,   536,
     538,   537,   557,   559,   558,   528,   518,   534,   533,   495,
     505,   506,   508,   507,   527,   511,   509,   510,   512,   513,
     514,   515,   516,   517,   519,   520,   521,   522,   523,   524,
     526,   525,   496,   497,   498,   501,   502,   504,   542,   543,
     552,   551,   550,   549,   548,   547,   535,   554,   544,   545,
     546,   529,   530,   531,   532,   555,   556,   560,   562,   561,
     563,   564,   541,   566,   565,   499,   568,   570,   569,   503,
     573,   571,   572,   567,   500,   553,   494,   269,   491,     0,
     244,   290,   291,   289,   282,     0,   283,   245,   316,     0,
       0,     0,     0,   120,     0,   216,     0,   709,     0,    62,
     292,    62,   123,     0,     0,   131,   807,   654,     0,    62,
     162,    75,     0,   424,   761,   458,   114,   246,   247,   319,
     156,     0,     0,   266,   259,     0,     0,     0,   271,   273,
       0,     0,   278,   285,   286,   284,     0,     0,   235,     0,
       0,     0,     0,   215,   711,     0,   474,   630,     0,     0,
      62,   125,     0,   662,     0,     0,     0,    94,   249,    51,
       0,   250,   251,     0,     0,   265,   268,   492,   493,     0,
     260,   287,   288,   280,   281,   279,   317,   314,   239,   237,
     318,     0,   475,   629,     0,   412,   293,     0,   127,     0,
     165,   459,     0,   118,     0,   299,   267,   270,     0,   674,
     241,     0,   472,   409,   414,   163,     0,     0,    95,   256,
       0,   298,   315,     0,   630,   310,   674,   473,     0,   117,
       0,     0,   255,   807,   674,   191,   311,   312,   313,   831,
     309,     0,     0,     0,   254,     0,   310,     0,   807,     0,
     253,   294,    62,   240,   831,     0,   195,   193,     0,    62,
       0,     0,   196,     0,   192,   242,     0,   295,     0,   199,
     190,     0,   198,   113,   200,     0,   189,   197,     0,   202,
     201
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   117,   744,   521,   175,   264,   483,
     265,   484,   119,   120,   121,   122,   123,   124,   309,   544,
     545,   436,   230,  1247,   442,  1177,  1463,   708,   260,   478,
    1427,   889,  1022,  1478,   325,   176,   546,   778,   938,  1069,
     547,   562,   796,   505,   794,   548,   526,   795,   327,   280,
     297,   130,   780,   747,   730,   901,  1195,   986,   843,  1381,
    1250,   661,   849,   441,   669,   851,  1101,   654,   833,   836,
     976,  1483,  1484,   536,   537,   556,   557,   269,   270,   274,
    1028,  1130,  1213,  1361,  1469,  1486,  1391,  1431,  1432,  1433,
    1201,  1202,  1203,  1392,  1398,  1440,  1206,  1207,  1211,  1354,
    1355,  1356,  1372,  1513,  1131,  1132,   177,   132,  1499,  1500,
    1359,  1134,   133,   223,   437,   438,   134,   135,   136,   137,
     138,   139,   140,   141,  1232,   142,   777,   937,   143,   227,
     304,   432,   530,   531,  1008,   532,  1009,   144,   145,   146,
     687,   147,   148,   257,   149,   258,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   698,   699,   881,   475,   476,
     477,   705,  1417,   150,   527,  1221,   528,   914,   752,  1044,
    1041,  1347,  1348,   151,   152,   153,   217,   224,   312,   422,
     154,   866,   691,   155,   867,   416,   762,   868,   820,   958,
     960,   961,   962,   822,  1081,  1082,   823,   635,   407,   185,
     186,   156,   539,   390,   391,   768,   157,   218,   179,   159,
     160,   161,   162,   163,   164,   165,   592,   166,   220,   221,
     508,   209,   210,   595,   596,  1013,  1014,   289,   290,   738,
     167,   498,   168,   535,   169,   250,   281,   320,   451,   862,
     921,   728,   672,   673,   674,   251,   252,   764
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1261
static const yytype_int16 yypact[] =
{
   -1261,   142, -1261, -1261,  4183, 11620, 11620,   -79, 11620, 11620,
   11620, -1261, 11620, 11620, 11620, 11620, 11620, 11620, 11620, 11620,
   11620, 11620, 11620, 11620, 13517, 13517,  9007, 11620, 13566,   -60,
     -54, -1261, -1261, -1261, -1261, -1261,   136, -1261, -1261, 11620,
   -1261,   -54,   -39,     4,   130,   -54,  9208,  9651,  9409, -1261,
   13161,  8605,   -11, 11620,  1995,   126, -1261, -1261, -1261,    74,
     234,    61,   166,   175,   182,   186, -1261,  9651,   189,   193,
   -1261, -1261, -1261, -1261, -1261,   579,   384, -1261, -1261,  9651,
    9610, -1261, -1261, -1261, -1261, -1261, -1261,  9651, -1261,   245,
     208,  9651,  9651, -1261, 11620, -1261, -1261, -1261, -1261, -1261,
   -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261,
   11620, -1261, -1261,   213,   283,   336,   336, -1261,   403,   303,
     290, -1261,   260, -1261,    39, -1261,   433, -1261, -1261, -1261,
   13941,   507, -1261, -1261,   272,   287,   294,   319,   321,   332,
   12427, -1261, -1261, -1261, -1261,   424, -1261,   434,   468,   337,
   -1261,   115,   302,   389, -1261, -1261,   437,    42,  1761,   116,
     348,   123,   125,   350,   146, -1261,   223, -1261,   489, -1261,
   -1261, -1261,   432,   385,   445, -1261,   433,   507, 14577,  1774,
   14577, 11620, 14577, 14577,  4169,   539,  9651, -1261, -1261,   543,
   -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261,
   -1261,  2097,   436, -1261,   462,   479,   479, 13517, 14234,   407,
     595, -1261,   432,  2097,   436,   467,   483,   428,   132, -1261,
     501,   116,  9811, -1261, -1261, 11620,  7399,   635,    47, 14577,
    8404, -1261, 11620, 11620,  9651, -1261, -1261, 12468,   448, -1261,
   12509, 13161, 13161,   482, -1261, -1261,   456, 12936,   637, -1261,
     642, -1261,  9651,   585, -1261,   470, 12550,   471,   457, -1261,
      -5, 12596, 13985,  9651,    55, -1261,    14, -1261, 13419,    58,
   -1261, -1261, -1261,   649,    59, 13517, 13517, 11620,   474,   508,
   -1261, -1261,  3275,  9007,    33,   304, -1261, 11821, 13517,   596,
   -1261,  9651, -1261,   -44,   303,   476, 14275, -1261, -1261, -1261,
     593,   663,   587, 14577,    29,   486, 14577,   487,   149,  4384,
   11620,   263,   491,   454,   263,   295,   267, -1261,  9651, 13161,
     503, 10012, 13161, -1261, -1261,  8847, -1261, -1261, -1261, -1261,
     433, -1261, -1261, -1261, -1261, -1261, -1261, -1261, 11620, 11620,
   11620, 10213, 11620, 11620, 11620, 11620, 11620, 11620, 11620, 11620,
   11620, 11620, 11620, 11620, 11620, 11620, 11620, 11620, 11620, 11620,
   11620, 11620, 11620, 13566, -1261, 11620, 11620, -1261, 11620, 12748,
    9651,  9651, 13941,   598,    37,  3554, 11620, 11620, 11620, 11620,
   11620, 11620, 11620, 11620, 11620, 11620, 11620, -1261, -1261, 13607,
   -1261,   133, 11620, 11620, -1261, 10012, 11620, 11620,   213,   134,
    3275,   505,   433, 10414, 12637, -1261,   510,   686,  2097,   506,
     -16, 13656,   479, 10615, -1261, 10816, -1261,   512,   -15, -1261,
     224, 10012, -1261, 13696, -1261,   135, -1261, -1261, 12678, -1261,
   -1261, 11017, -1261, 11620, -1261,   627,  7600,   704,   522, 14469,
     708,    52,    17, -1261, -1261, -1261, -1261, -1261, 13161,   643,
     531,   717, -1261, 13230, -1261,   548, -1261, -1261, -1261,   654,
   11620,   655,   660, 11620, 11620, 11620, -1261,   457, -1261, -1261,
   -1261, -1261, -1261, -1261, -1261,   557, -1261, -1261, -1261,   547,
   -1261, -1261,  9651,   549,   739,   253,   254, 14029, -1261,  9651,
   11620,   479,   126, -1261, 13230,   678, -1261,   479,    75,    84,
     565,   569,  1412,   570,  9651,   646,   574,   479,    87,   578,
   13928,  9651, -1261, -1261,   707,  2590,   -10, -1261, -1261, -1261,
     303, -1261, -1261, -1261, -1261, 11620,   656,   615,   264, -1261,
     657,   775,   590, 13161, 13161,   773,   597,   779, -1261, 13161,
      15,   729,   113, -1261, -1261, -1261, -1261, -1261, -1261,  2857,
   -1261, -1261, -1261, -1261,    50, 13517,   600,   783, 14577,   782,
   -1261, -1261,   676,  9048, 14617,  3970,  4169, 11620, 14536,  4570,
    4770,  4970,  5170,  2365,  3370,  3370,  3370,  3370,  1604,  1604,
    1604,  1604,   796,   796,   638,   638,   638,   543,   543,   543,
   -1261, 14577,   599,   603, 14331,   601,   789, -1261, 11620,   -53,
     607,   134, -1261, -1261, -1261,   433, 13368, 11620, -1261, -1261,
    4169, -1261,  4169,  4169,  4169,  4169,  4169,  4169,  4169,  4169,
    4169,  4169,  4169, 11620,   -53,   614,   609,  3112,   616,   611,
    3247,    89,   619, -1261, 13321, -1261,  9651, -1261,   486,    15,
     436, 13517, 14577, 13517, 14372,   255,   138, -1261,   622, 11620,
   -1261, -1261, -1261,  7198,   331, -1261, 14577, 14577,   -54, -1261,
   -1261, -1261, 11620,  3056, 13230,  9651,  7801,   618,   623, -1261,
      57,   694, -1261,   811,   626, 13016, 13161, 13230, 13230, 13230,
     629,    44,   681,   631,   221, -1261,   683, -1261,   632, -1261,
   -1261, -1261, 11620,   648, 14577,   652,   820, 12765,   828, -1261,
   14577, 12719, -1261,   557,   770, -1261,  4585, 13888,   651,   279,
   -1261, 13985,  9651,  9651, -1261, -1261,  3289, -1261, -1261,   836,
   13517,   658, -1261, -1261, -1261, -1261, -1261,   761,    96, 13888,
     659,  3275, 13468,   841, -1261, -1261, -1261, -1261,   662, -1261,
   11620, -1261, -1261,  3781, -1261, 14577, 13888,   661, -1261, -1261,
   -1261, -1261,   845, 11620,   593, -1261, -1261,   665, -1261, 13161,
     838,   101, -1261, -1261,    79, 13745, -1261,   140, -1261, -1261,
   13161, -1261,   479, -1261, 11218, -1261, 13230,    23,   677, 13888,
     656, -1261, -1261,  4370, 11620, -1261, -1261, 11620, -1261, 11620,
   -1261,  3332,   684, 10012,   646,   656,   676,  9651, 13566,   479,
    3460,   685, 10012, -1261, -1261,   143, -1261, -1261,   856, 13794,
   13794, 13321, -1261, -1261, -1261, -1261,   687,    80,   689, -1261,
   -1261, -1261,   868,   695,   510,   479,   479, 11419, -1261,   144,
   -1261, -1261, 12006,   372,   -54,  8404, -1261,  4786,   688,  4987,
     693, 13517,   697,   769,   479, -1261,   880, -1261, -1261, -1261,
   -1261,   427, -1261,    21, 13161, -1261, 13161,   643, -1261, -1261,
   -1261,   888,   703,   709, -1261, -1261,   778,   698,   894, 13230,
     765,  9651,   593, 14069, 13230, 14577, 11620, 11620, 11620, -1261,
   -1261, -1261, 11620, 11620, -1261,   457, -1261,   831, -1261, -1261,
    9651, -1261, -1261, -1261, -1261, 13230,   479, -1261, 13161,  9651,
   -1261,   899, -1261, -1261,    91,   718,   479,  8806, -1261,  2485,
   -1261,  3982,   899, -1261,   306,   232, 14577,   787, -1261,   719,
   13161,   635, 13161,   839,   902,   842, 11620,   -53,   726, -1261,
   13517, 14577, -1261,   732,    23, -1261,   716,    23,   727,  4370,
   14577, 14428,   735, 10012,   742,   733,   745,   656, -1261,   428,
     750, 10012,   744, 11620, -1261, -1261, -1261, -1261, -1261, -1261,
     822,   746,   939, 13321,   812, -1261,   593, 13321, -1261, -1261,
   -1261, 13517, 14577, -1261,   -54,   924,   884,  8404, -1261, -1261,
   -1261,   759, 11620,   479,  3275,  3056,   763, 13230,  5188,   459,
     764, 11620,    70,   300, -1261,   799, -1261, -1261, 13081,   935,
   -1261, 13230, -1261, 13230, -1261,   771, -1261,   843,   959,   774,
   -1261, -1261,   848,   777,   967, 14577, 12848, 14577, -1261, 14577,
   -1261,   784, -1261, -1261, -1261, -1261,   893, 13888,   477, -1261,
    3275, -1261, -1261,  4169,   788, -1261,   941, -1261,    62, 11620,
   -1261, -1261, -1261, 11620, -1261, 11620, -1261, -1261, -1261,   317,
     974, 13230, -1261, 12047,   797, 10012,   479,   838,   795, -1261,
     798,    23, 11620, 10012,   800, -1261, -1261, -1261,   802,   803,
   -1261, 10012,   805, -1261, 13321, -1261, 13321, -1261,   806, -1261,
     870,   807,   996, -1261,   479,   980, -1261,   814, -1261, -1261,
     818,    92, -1261, -1261, -1261,   821,   823, -1261, 12183, -1261,
   -1261, -1261, -1261, -1261, -1261, 13161, -1261,   890, -1261, 13230,
     593, -1261, -1261, 13230, -1261, 13230, -1261, 11620,   819,  5389,
   13161, -1261,    -3, 13161, 13888, -1261, 13841,   865,  3361, -1261,
   -1261, -1261,   598, 12871,    60,    37,    93, -1261, -1261,   871,
   12088, 12129, 14577,   948,  1007,   949, 13230, -1261,   832, 10012,
     830,   921,   838,  1215,   838,   833, 14577,   834, -1261,  1464,
    1575, -1261,    23,   835, -1261, -1261,   908, -1261, 13321, -1261,
     593, -1261, -1261,  7198, -1261, -1261, -1261,  8002, -1261, -1261,
   -1261,  7198,   844, 13230, -1261,   909, -1261,   910, 12807, -1261,
   -1261, -1261, 13888, 13888,  1023,    36, -1261, -1261, -1261,    63,
     847,    64, -1261, 12246, -1261, -1261,    69, -1261, -1261, 12083,
   -1261,   849, -1261,   963,   433, -1261, 13161, -1261,   598, -1261,
   -1261, -1261, -1261, -1261,  1026, 13230, -1261, -1261, 10012,   846,
   -1261,   852,   851,   -68, -1261,   921,   838, -1261, -1261, -1261,
    1652,   861, -1261, 13321, -1261,   934,  7198,  8203, -1261, -1261,
   -1261,  7198, -1261, -1261, 13230, 13230, 11620,  5590,   863,   864,
   13230, 13888, -1261, -1261,   804, 13841, -1261, -1261, -1261, -1261,
   -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261,
   -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261,
   -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261,
   -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261,
   -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261,
   -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261,
   -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261,
   -1261, -1261, -1261, -1261, -1261, -1261, -1261,   481, -1261,   865,
   -1261, -1261, -1261, -1261, -1261,    32,   395, -1261,  1047,    71,
    9651,   963,  1048,   433, 13230, -1261,   869, -1261,   316, -1261,
   -1261, -1261, -1261,   866,   -68, -1261,   838, -1261, 13321, -1261,
   -1261, -1261,  5791, -1261, -1261,  3771, -1261, -1261, -1261, -1261,
   -1261,  2583,    26, -1261, -1261, 13230, 12246, 12246,  1014, -1261,
   12083, 12083,   515, -1261, -1261, -1261, 13230,   992, -1261,   874,
      81, 13230,  9651, -1261, -1261,   994, -1261,  1064,  5992,  6193,
   -1261, -1261,   -68, -1261,  6394,   878,  1000,   972, -1261,   985,
     937, -1261, -1261,   986,   804, -1261, -1261, -1261, -1261,   926,
   -1261,  1053, -1261, -1261, -1261, -1261, -1261,  1070, -1261, -1261,
   -1261,   891, -1261,   345,   892, -1261, -1261,  6595, -1261,   895,
   -1261, -1261,   896,   927,  9651,    37, -1261, -1261, 13230,    78,
   -1261,  1011, -1261, -1261, -1261, -1261, 13888,   651, -1261,   929,
    9651,   401, -1261,   901,  1085,   529,    78, -1261,  1022, -1261,
   13888,   903, -1261,   838,    83, -1261, -1261, -1261, -1261, 13161,
   -1261,   906,   907,    85, -1261,   -65,   529,   318,   838,   912,
   -1261, -1261, -1261, -1261, 13161,  1028,  1090,  1030,   -65, -1261,
    6796,   322,  1092, 13230, -1261, -1261,  6997, -1261,  1032,  1096,
    1036, 13230, -1261, -1261,  1098, 13230, -1261, -1261, 13230, -1261,
   -1261
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1261, -1261, -1261,  -481, -1261, -1261, -1261,    -4, -1261, -1261,
     634,   402,     6,   946,  -403, -1261,  1695, -1261,  -271, -1261,
      12, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261,
   -1261,  -363, -1261, -1261,  -171,    -1,    11, -1261, -1261, -1261,
      13, -1261, -1261, -1261, -1261,    20, -1261, -1261,   751,   752,
     754,   960,   330,  -706,   333,   383,  -360, -1261,   148, -1261,
   -1261, -1261, -1261, -1261, -1261,  -558,    38, -1261, -1261, -1261,
   -1261,  -352, -1261,  -745, -1261,  -340, -1261, -1261,   645, -1261,
    -876, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261, -1261,
   -1261,  -127, -1261, -1261, -1261, -1261, -1261,  -209, -1261,    16,
    -899, -1261, -1260,  -377, -1261,  -155,    43,  -124,  -364, -1261,
    -218, -1261,   -74,   -18,  1106,  -623,  -347, -1261, -1261,   -47,
   -1261, -1261,  2649,   -52,   -87, -1261, -1261, -1261, -1261, -1261,
   -1261,   229,  -721, -1261, -1261, -1261, -1261, -1261, -1261, -1261,
   -1261, -1261, -1261,   776, -1261, -1261,   269, -1261,   690, -1261,
   -1261, -1261, -1261, -1261, -1261, -1261,   273, -1261,   691, -1261,
   -1261,   453, -1261,   246, -1261, -1261, -1261, -1261, -1261, -1261,
   -1261, -1261,  -892, -1261,  1317,    51,  -334, -1261, -1261,   212,
     859,   488, -1261, -1261,   298,  -350,  -544, -1261, -1261,   353,
    -616,   205, -1261, -1261, -1261, -1261, -1261,   346, -1261, -1261,
   -1261,  -234,  -742,  -178,  -166,  -129, -1261, -1261,    27, -1261,
   -1261, -1261, -1261,    -8,  -140, -1261,   128, -1261, -1261, -1261,
    -375,   898, -1261, -1261, -1261, -1261, -1261,     7,   461, -1261,
   -1261,   900, -1261, -1261, -1261,  -320,   -81,  -193,  -285, -1261,
   -1019, -1261,   314, -1261, -1261, -1261,  -233,  -900
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -801
static const yytype_int16 yytable[] =
{
     118,   373,   559,   126,   255,   401,   298,   331,   446,   447,
     301,   302,   226,   775,   452,   127,   125,   128,   821,   922,
     219,   419,  1049,   231,   129,   631,   399,   235,   608,   590,
     394,   158,   933,   917,   554,  1434,   305,  1036,  1151,   840,
     743,  1400,   424,   238,   667,  1261,   248,   131,   322,   328,
     685,   205,   206,   331,   425,   628,   433,   637,   307,   770,
     266,   665,  1401,   279,   487,    11,   854,   492,   495,  1216,
     538,    11,  -261,  1265,   936,   204,   204,   392,  1349,   216,
    1407,   648,   293,   279,   720,   294,   452,   279,   279,   946,
    1407,   685,   426,   720,  1261,   510,   732,  1099,   732,   479,
     732,   732,   732,   318,  -698,   899,   489,   389,   389,  1139,
     853,   923,   319,   389,  1421,   181,   991,   992,  1192,  1193,
      11,   313,   315,   316,   870,    11,   279,   409,  1370,  1371,
     330,  1511,  1512,  1233,   222,  1235,   273,   308,   765,   417,
     225,  -800,     3,    11,  -415,    56,    57,    58,   170,   171,
     329,  1007,   522,   523,   924,   232,   511,   480,   653,   563,
     964,   318,  1458,   403,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,   386,  -702,   402,  -695,   392,   542,
     646,   755,   406,  1153,   286,  -696,   259,  -697,   742,  1058,
    1159,  1160,  1060,   271,  -733,  -699,   396,  -734,   233,   374,
    -736,   267,  -700,   500,  -578,  -701,  -735,   706,   396,   387,
     388,   299,   393,    95,   668,   670,   771,  1374,  -208,   994,
     965,  -208,   118,   855,  1435,   501,   118,  1402,   410,   601,
     440,   632,  1262,  1263,   412,   323,  -704,   925,   430,  -698,
     418,  1068,   435,   434,   561,  1080,   790,   666,   454,   601,
     331,   488,   204,   158,   493,   496,  1217,   158,   204,  -261,
    1266,   685,   900,   911,   204,  1350,  1100,  1408,   485,   486,
     721,   601,   389,  -194,   685,   685,   685,  1449,  -629,   722,
     601,  1510,   733,   601,   808,  1240,  1029,  1176,  1219,  -579,
     298,   328,   989,  -629,   993,   491,  -629,   520,   199,   199,
     757,   758,   497,   497,   502,   118,   763,  -705,   126,   507,
    -702,   749,  -695,   393,   553,   516,  1155,   392,   248,   204,
    -696,   279,  -697,   858,   234,   268,   204,   204,   609,  -733,
    -699,   397,  -734,   204,   638,  -736,   158,  -700,   284,   204,
    -701,  -735,   284,   397,   542,   712,   713,   517,  1415,  1143,
    1515,  1083,   131,   272,  1528,   219,   904,  1422,   284,  1090,
     275,   600,   766,   311,  -800,   599,   279,   279,   279,   276,
     284,   890,   605,   685,   767,   517,   277,  1471,   423,   284,
     278,   625,  1042,   282,   837,   624,   319,   283,   839,  1185,
     861,  1416,  1144,  1516,   299,   991,   992,  1529,   834,   835,
     318,   318,   300,   600,   287,   288,   792,   640,   287,   288,
     310,   284,   647,   750,   216,   651,   314,  1241,  -800,   650,
    1472,  1403,   398,   113,   287,   288,   318,   507,   751,  1043,
     317,   801,   118,  -800,   797,   410,   287,   288,  1404,   974,
     975,  1405,   452,   863,   512,   287,   288,   792,   660,  1245,
     318,   204,   393,   944,   321,   319,  1037,    34,  1165,   204,
    1166,   766,   952,   158,   949,   324,   685,   828,   332,  1038,
     552,   685,   968,   767,  1505,  1145,  1517,   287,   288,   829,
    1530,  1122,   782,   333,  -800,   715,  -437,  -800,   709,  1518,
     334,   419,   685,   266,   593,  1395,   365,   551,  1102,   368,
     727,  1441,  1442,  1039,  1437,  1438,   737,   739,  1396,    56,
      57,    58,   170,   171,   329,   335,   830,   336,  1004,    11,
     626,   990,   991,   992,   629,  1397,   919,   538,   337,   284,
     366,   291,   369,   367,   517,    82,    83,   929,    84,    85,
      86,  1443,   395,   538,  -703,    56,    57,    58,    59,    60,
     329,  -438,  1244,  1096,   991,   992,    66,   370,  1444,   279,
    1031,  1445,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,  -578,   988,    95,  1123,   400,
     405,   292,   772,  1124,   685,    56,    57,    58,   170,   171,
     329,  1125,   291,   363,   371,   287,   288,  1492,   685,  1507,
     685,   319,   389,  1064,   415,   411,   204,    49,   414,  1091,
    -577,  1072,  1077,    95,  1521,    56,    57,    58,   170,   171,
     329,   995,   421,   996,   423,   601,   420,  1377,  1126,  1127,
     819,  1128,   824,   799,   459,   460,   461,  1496,  1497,  1498,
     838,   462,   463,   431,   444,   464,   465,   448,   685,   118,
     449,  -795,   126,    95,   284,  1136,   453,   204,  1111,   285,
     455,   846,   118,   494,  1116,  1025,   456,   458,   825,   503,
     826,   284,   524,   504,   529,  1129,   517,   533,   848,   534,
     158,   540,   541,    95,   360,   361,   362,  1047,   363,   763,
     844,   550,   204,   158,   204,   636,   131,   -57,    49,   560,
     538,   639,   118,   538,   634,   126,   685,   645,   892,   893,
     685,   658,   685,   433,   204,  1150,   948,   485,   662,   286,
     287,   288,   664,  1157,   671,   675,   676,  1485,   692,   693,
     695,  1163,  1171,   158,  1054,   696,   518,   287,   288,   118,
     704,   707,   126,   685,  1485,   710,   513,   896,   711,   131,
     519,  1119,  1506,   719,   127,   125,   128,   928,   507,   906,
     723,   927,  1423,   129,   724,   929,   726,   729,   731,   740,
     158,   204,   513,   734,   519,   513,   519,   519,   746,   748,
     685,   753,   204,   204,   754,   756,   131,   759,   761,   760,
     219,  -439,   774,   279,  1135,   773,   776,   779,   789,   788,
     785,   793,  1135,  1194,   786,   957,   957,   819,   802,  1229,
     803,   805,   806,   781,   850,  1173,   977,   831,   856,   852,
     857,   859,   685,   869,   871,   872,   873,   538,   876,   874,
    1181,   118,   877,   118,   878,   118,   126,   882,   126,   357,
     358,   359,   360,   361,   362,   885,   363,   978,   888,   216,
     895,   685,   685,   898,   897,   907,   903,   685,   913,   915,
    1032,   918,   158,   908,   158,   920,   158,  1006,   983,  1011,
     953,   934,  1182,  1454,    31,    32,    33,   967,   943,   951,
     131,   963,   131,   966,   980,    38,  1023,  1191,  1366,   982,
     969,   984,   204,   985,   987,  1026,  1362,   998,   999,  1002,
    1215,  1246,  1001,  1003,  1000,   512,  1021,   118,  1027,  1251,
     126,  1045,  1030,  1059,  1050,  1046,  1051,  1052,  1257,  1135,
    1055,  1061,   127,   125,   128,  1135,  1135,  1057,   538,  1063,
    1066,   129,    70,    71,    72,    73,    74,  1065,   158,  1073,
    1495,   690,  1067,   681,  1071,  1122,  1074,  1075,  1076,    77,
      78,  1086,  1079,  1087,   131,  1089,  1085,  1056,  1093,   819,
    1097,   685,  1105,   819,    88,  1103,  1108,  1109,  1110,  1112,
     202,   202,  1113,   118,   214,  1114,  1115,  1382,    93,  1118,
    1218,   204,   718,    11,   118,  1120,  1137,   126,  1146,  1088,
    1152,  1149,   685,  1154,  1168,  1158,   214,  1162,  1084,  1161,
    1164,  1167,  1169,   685,   158,  1170,  1135,  1172,   685,   331,
    1174,   507,   844,  1175,  1183,   158,  1189,  1178,  1205,  1179,
    1220,  1225,   204,  1224,  1226,  1230,  1228,  1231,  1236,  1237,
    1242,   131,  1243,  1254,  1255,   204,   204,  1260,  1358,  1252,
    1364,  1367,  1123,  1360,  1264,  1357,  1368,  1124,  1369,    56,
      57,    58,   170,   171,   329,  1125,  1376,   507,  1378,  1387,
    1388,  1406,  1411,  1420,  1414,   685,  1439,  1447,  1448,  1452,
     819,  1133,   819,  1453,  1460,  1461,  1462,  -257,  1465,  1133,
    1464,   204,  1467,  1401,  1468,  1470,  1487,  1473,  1490,  1477,
    1476,  1475,  1126,  1127,  1494,  1128,  1493,  1502,  1418,  1504,
    1419,  1508,  1509,  1522,  1523,  1524,  1531,  1534,  1424,  1519,
    1535,  1536,  1538,   891,  1489,   118,   372,    95,   126,   248,
     685,   714,   602,   604,  1210,   603,   947,   945,   685,   912,
    1503,  1214,   685,  1092,  1501,   685,  1180,   717,  1394,  1138,
    1399,  1525,  1514,  1410,  1212,   228,   158,   202,  1373,  1457,
    1048,   611,   845,   202,  1020,  1018,   884,   702,   703,   202,
    1040,  1070,   131,   959,   819,   864,   865,  1005,  1078,   118,
     970,   997,   126,   118,     0,     0,   499,   118,   374,     0,
     126,   509,     0,     0,     0,     0,     0,   214,   214,  1249,
       0,     0,  1412,   214,     0,     0,  1133,     0,     0,  1346,
     158,     0,  1133,  1133,   158,  1353,     0,     0,   158,     0,
       0,     0,   248,     0,   202,     0,   131,  1363,     0,  1122,
       0,   202,   202,     0,   131,     0,     0,     0,   202,     0,
       0,     0,     0,     0,   202,   538,     0,     0,     0,   819,
       0,  1520,   118,   118,     0,   126,     0,   118,  1526,     0,
     126,     0,   538,   118,     0,     0,   126,    11,     0,  1380,
     538,     0,     0,     0,   932,   214,   763,     0,   214,     0,
       0,     0,     0,   158,   158,     0,     0,     0,   158,  1409,
       0,   763,     0,  1133,   158,     0,     0,     0,     0,   131,
       0,     0,     0,     0,   131,     0,     0,     0,     0,     0,
     131,     0,     0,     0,     0,     0,     0,     0,     0,   214,
    1480,     0,   689,     0,     0,     0,  1123,     0,     0,     0,
       0,  1124,     0,    56,    57,    58,   170,   171,   329,  1125,
       0,  1451,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   203,   203,     0,     0,   215,   202,     0,     0,     0,
       0,     0,     0,   689,   202,     0,   279,   331,     0,     0,
       0,     0,  1012,     0,     0,     0,  1126,  1127,     0,  1128,
       0,     0,     0,     0,   819,     0,     0,     0,   118,     0,
       0,   126,     0,  1024,     0,     0,     0,  1429,     0,     0,
       0,    95,  1346,  1346,   214,     0,  1353,  1353,     0,   684,
       0,     0,     0,     0,     0,     0,     0,     0,   279,   158,
       0,     0,     0,  1234,   118,   118,     0,   126,   126,     0,
     118,     0,     0,   126,     0,   131,   403,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,     0,     0,
     684,     0,     0,     0,     0,   158,   158,     0,     0,     0,
       0,   158,     0,   118,     0,     0,   126,     0,     0,     0,
    1479,   131,   131,     0,     0,     0,     0,   131,  1122,     0,
       0,     0,   387,   388,     0,  1094,  1491,     0,     0,   214,
     214,     0,     0,     0,   158,   214,     0,     0,     0,  1106,
       0,  1107,     0,     0,     0,     0,     0,     0,     0,     0,
     131,   202,     0,     0,     0,     0,    11,     0,  1481,     0,
       0,     0,     0,     0,     0,     0,   118,     0,     0,   126,
       0,     0,   118,   689,   203,   126,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   389,   689,   689,   689,  1147,
       0,     0,     0,     0,     0,     0,     0,   158,     0,     0,
       0,     0,   202,   158,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   131,     0,  1123,     0,     0,     0,   131,
    1124,     0,    56,    57,    58,   170,   171,   329,  1125,  1122,
       0,     0,     0,     0,     0,   203,     0,   202,     0,   202,
       0,     0,   203,   203,     0,     0,     0,  1184,     0,   203,
       0,  1186,     0,  1187,     0,   203,     0,   725,     0,   202,
     684,     0,     0,     0,     0,  1126,  1127,    11,  1128,     0,
       0,   214,   214,   684,   684,   684,     0,     0,     0,     0,
       0,     0,     0,     0,  1227,   689,     0,     0,     0,     0,
      95,  -801,  -801,  -801,  -801,   355,   356,   357,   358,   359,
     360,   361,   362,   214,   363,     0,  1122,     0,     0,     0,
       0,     0,  1238,     0,     0,     0,   202,     0,     0,     0,
       0,  1253,     0,     0,     0,   214,  1123,   202,   202,     0,
     215,  1124,     0,    56,    57,    58,   170,   171,   329,  1125,
       0,     0,   214,     0,    11,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   214,     0,     0,     0,     0,
       0,     0,     0,  1365,     0,     0,   214,   203,     0,     0,
       0,     0,   684,     0,     0,   214,  1126,  1127,   689,  1128,
       0,     0,     0,   689,     0,     0,     0,     0,     0,     0,
       0,     0,  1383,  1384,   214,   249,     0,     0,  1389,     0,
       0,    95,     0,  1123,   689,     0,     0,     0,  1124,     0,
      56,    57,    58,   170,   171,   329,  1125,     0,     0,     0,
     688,     0,     0,  1239,     0,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,   386,   202,   403,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,   386,
     214,     0,   214,  1126,  1127,     0,  1128,     0,     0,     0,
       0,   688,     0,     0,     0,   684,     0,     0,     0,     0,
     684,   387,   388,     0,     0,     0,     0,     0,    95,     0,
       0,     0,     0,     0,   387,   388,     0,     0,     0,     0,
       0,   684,     0,     0,   214,     0,   689,     0,     0,     0,
    1375,     0,  1413,     0,     0,     0,     0,     0,     0,     0,
     689,     0,   689,     0,     0,     0,   214,     0,   214,     0,
       0,     0,   203,     0,     0,     0,   202,     0,     0,     0,
       0,     0,     0,  1436,   389,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1446,     0,     0,   389,     0,  1450,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     689,     0,     0,     0,     0,     0,     0,   202,     0,     0,
       0,     0,     0,   203,     0,     0,     0,     0,     0,     0,
     202,   202,     0,   684,     0,     0,   249,   249,     0,     0,
       0,     0,   249,     0,   214,     0,     0,   684,     0,   684,
       0,     0,     0,     0,     0,     0,  1482,     0,   203,     0,
     203,     0,     0,     0,     0,     0,     0,     0,   689,     0,
       0,     0,   689,   214,   689,     0,   202,     0,     0,     0,
     203,   688,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   688,   688,   688,   684,     0,     0,
       0,     0,     0,     0,     0,   689,     0,     0,     0,     0,
       0,  1532,     0,     0,   249,     0,     0,   249,     0,  1537,
       0,     0,     0,  1539,   887,     0,  1540,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   203,     0,     0,
       0,     0,   689,     0,     0,     0,   902,     0,   203,   203,
       0,   214,     0,     0,     0,   684,     0,     0,     0,   684,
       0,   684,     0,   902,     0,     0,   214,     0,    34,   214,
     214,     0,   214,     0,     0,     0,     0,     0,     0,   214,
       0,     0,     0,     0,   689,     0,     0,     0,     0,     0,
       0,     0,   684,   688,     0,   262,   935,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   689,   689,   215,     0,     0,     0,   689,
       0,     0,     0,  1393,     0,     0,     0,     0,     0,   684,
       0,     0,     0,     0,     0,     0,     0,     0,   214,   214,
       0,     0,   263,   249,     0,     0,    82,    83,   686,    84,
      85,    86,     0,     0,     0,     0,     0,     0,   203,     0,
      27,    28,   214,     0,     0,     0,     0,     0,     0,     0,
      34,   684,   199,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   688,     0,     0,   686,
       0,   688,     0,     0,     0,     0,     0,     0,     0,     0,
     684,   684,     0,     0,     0,     0,   684,   214,     0,     0,
     200,   214,   688,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   689,     0,     0,     0,     0,   249,   249,
       0,     0,     0,     0,   249,     0,     0,     0,     0,     0,
       0,   174,     0,     0,    79,     0,    81,   203,    82,    83,
       0,    84,    85,    86,   689,     0,     0,     0,     0,     0,
      89,     0,     0,     0,     0,   689,     0,     0,     0,     0,
     689,     0,     0,     0,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   203,     0,
       0,   408,     0,  1466,     0,     0,   113,     0,     0,     0,
       0,   203,   203,     0,   688,     0,     0,     0,     0,     0,
     684,     0,     0,     0,     0,     0,     0,     0,   688,     0,
     688,     0,     0,     0,     0,     0,     0,   689,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   214,     0,     0,
       0,   684,     0,     0,  1121,     0,     0,   203,     0,     0,
       0,     0,   684,     0,     0,     0,     0,   684,     0,   686,
       0,     0,     0,     0,     0,     0,     0,     0,   688,     0,
     249,   249,   686,   686,   686,     0,     0,     0,     0,     0,
       0,     0,   689,     0,     0,     0,     0,     0,     0,     0,
     689,     0,     0,     0,   689,     0,     0,   689,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   684,   363,     0,     0,     0,     0,
       0,     0,   214,     0,     0,     0,   688,     0,     0,     0,
     688,     0,   688,     0,     0,     0,   214,     0,     0,     0,
       0,  1196,     0,  1204,     0,   214,     0,     0,     0,     0,
       0,     0,     0,     0,   249,     0,     0,     0,     0,     0,
     214,     0,     0,   688,     0,   249,     0,     0,     0,   684,
       0,   686,     0,     0,     0,     0,     0,   684,     0,     0,
       0,   684,     0,     0,   684,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   338,   339,   340,     0,     0,
     688,     0,     0,     0,     0,     0,     0,     0,     0,  1258,
    1259,   341,     0,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,     0,   363,     0,     0,     0,     0,
       0,     0,   688,     0,     0,     0,     0,     0,     0,   249,
       0,   249,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   686,     0,     0,     0,     0,   686,
       0,   688,   688,     0,     0,     0,     0,   688,  1390,     0,
       0,     0,  1204,     0,     0,     0,     0,     0,     0,     0,
     686,     0,     0,   249,     0,     0,     0,     0,     0,     0,
     338,   339,   340,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   249,   341,   249,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,     0,
     363,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   178,   180,    34,   182,   183,   184,
       0,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,     0,     0,   208,   211,     0,     0,     0,
       0,   688,   686,     0,     0,     0,  1034,     0,   229,     0,
       0,     0,     0,   249,     0,   237,   686,   240,   686,     0,
     256,     0,   261,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   688,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   688,     0,     0,     0,   174,   688,   296,
      79,     0,     0,     0,    82,    83,     0,    84,    85,    86,
       0,     0,     0,   303,     0,     0,   686,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   306,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,     0,     0,     0,     0,     0,     0,
       0,  1428,     0,     0,     0,   688,     0,     0,   741,     0,
       0,     0,     0,  1488,     0,     0,     0,     0,     0,     0,
     249,     0,     0,     0,   686,     0,     0,  1196,   686,     0,
     686,     0,     0,     0,     0,   249,     0,     0,   249,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   249,     0,
     404,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     688,   686,     0,     0,     0,     0,     0,     0,   688,     0,
       0,     0,   688,     0,     0,   688,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   338,   339,   340,
       0,   428,     0,     0,   428,     0,     0,     0,   686,     0,
       0,   229,   439,   341,     0,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,     0,   363,     0,     0,
       0,   249,     0,     0,     0,     0,     0,     0,     0,     0,
     686,     0,     0,     0,     0,     0,   306,     0,     0,     0,
       0,     0,   208,     0,     0,     0,   515,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   686,
     686,     0,     0,     0,     0,   686,     0,     0,     0,   549,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     558,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   564,   565,   566,
     568,   569,   570,   571,   572,   573,   574,   575,   576,   577,
     578,   579,   580,   581,   582,   583,   584,   585,   586,   587,
     588,   589,     0,     0,   591,   591,     0,   594,     0,     0,
       0,     0,     0,     0,   610,   612,   613,   614,   615,   616,
     617,   618,   619,   620,   621,   622,     0,     0,     0,     0,
       0,   591,   627,     0,   558,   591,   630,     0,     0,     0,
       0,     0,   610,     0,     0,   769,     0,     0,     0,   686,
       0,     0,   642,     0,   644,     0,     0,     0,     0,     0,
     558,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     656,     0,   657,     0,     0,     0,  1430,     0,   841,     0,
     686,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   686,     0,     0,     0,     0,   686,     0,     0,   694,
       0,     0,   697,   700,   701,     0,     0,     0,     0,     0,
       0,     0,   338,   339,   340,     0,     0,     0,     0,    34,
       0,   199,     0,     0,     0,     0,     0,     0,   341,   716,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,     0,   363,   686,     0,     0,     0,     0,     0,   200,
       0,     0,     0,     0,   745,     0,     0,     0,     0,     0,
       0,   842,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   249,     0,     0,     0,     0,     0,
     174,     0,     0,    79,     0,    81,     0,    82,    83,   249,
      84,    85,    86,     0,     0,     0,   783,     0,   686,     0,
       0,     0,     0,     0,     0,     0,   686,     0,     0,     0,
     686,     0,     0,   686,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   791,     0,     0,
     201,     0,     0,     0,     0,   113,   296,   338,   339,   340,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   800,   341,     0,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,     0,   363,   832,   338,
     339,   340,     0,     0,     0,     0,     0,     0,     0,     0,
     804,   229,     0,     0,     0,   341,     0,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,     0,   363,
       0,   875,   338,   339,   340,     0,     0,     0,    34,     0,
     199,     0,     0,     0,     0,     0,     0,     0,   341,     0,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,     0,   363,     0,     0,     0,     0,     0,   200,   909,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     506,     0,   916,  -801,  -801,  -801,  -801,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   174,
     363,     0,    79,   931,    81,     0,    82,    83,     0,    84,
      85,    86,     0,   939,    34,     0,   940,     0,   941,     0,
       0,     0,   558,     0,     0,   807,     0,     0,     0,     0,
       0,   558,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,     0,     0,     0,   201,
     338,   339,   340,     0,   113,     0,   972,     0,  1208,     0,
       0,     0,     0,     0,     0,     0,   341,   894,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,     0,
     363,     0,    82,    83,     0,    84,    85,    86,     0,     0,
       0,     0,     0,     0,     0,  1015,  1016,  1017,     0,     0,
     942,   697,  1019,     0,     0,     0,     0,     0,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,     0,     0,     0,  1209,  1033,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,  1053,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   606,     0,     0,     0,
       0,     0,   558,     0,     0,     0,     0,    12,    13,     0,
     558,     0,  1033,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,    35,    36,
       0,   229,     0,     0,     0,    38,     0,     0,     0,     0,
    1098,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,   950,     0,
       0,     0,    56,    57,    58,   170,   171,   172,     0,     0,
      63,    64,     0,     0,     0,     0,     0,     0,     0,   173,
      69,     0,    70,    71,    72,    73,    74,     0,  1140,     0,
       0,     0,  1141,    75,  1142,     0,     0,     0,   174,    77,
      78,    79,   607,    81,   558,    82,    83,     0,    84,    85,
      86,  1156,   558,     0,    88,     0,     0,    89,     0,     0,
     558,     0,     0,    90,     0,     0,     0,     0,    93,    94,
      95,   253,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,   110,     0,
       0,     0,     0,   113,   114,     0,   115,   116,     0,     0,
       0,     0,     0,     0,     0,     0,  1188,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   338,   339,   340,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,   341,   558,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
       0,   363,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,    37,     0,
       0,     0,    38,    39,    40,    41,     0,    42,     0,    43,
       0,    44,     0,     0,    45,     0,     0,   558,    46,    47,
      48,    49,    50,    51,    52,     0,    53,    54,    55,    56,
      57,    58,    59,    60,    61,     0,    62,    63,    64,    65,
      66,    67,     0,     0,     0,  1385,    68,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,    76,    77,    78,    79,    80,
      81,     0,    82,    83,     0,    84,    85,    86,    87,     0,
       0,    88,     0,     0,    89,     0,     0,     0,     0,     0,
      90,    91,     0,    92,  1426,    93,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,   110,     0,   111,   112,   910,
     113,   114,   340,   115,   116,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   341,     0,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,     0,
     363,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,    36,     0,    37,
       0,     0,     0,    38,    39,    40,    41,     0,    42,     0,
      43,     0,    44,     0,     0,    45,     0,     0,     0,    46,
      47,    48,    49,    50,    51,    52,     0,    53,    54,    55,
      56,    57,    58,    59,    60,    61,     0,    62,    63,    64,
      65,    66,    67,     0,     0,     0,     0,    68,    69,     0,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,    75,     0,     0,     0,     0,    76,    77,    78,    79,
      80,    81,     0,    82,    83,     0,    84,    85,    86,    87,
       0,     0,    88,     0,     0,    89,     0,     0,     0,     0,
       0,    90,    91,     0,    92,     0,    93,    94,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,   110,     0,   111,   112,
    1035,   113,   114,     0,   115,   116,     5,     6,     7,     8,
       9,     0,     0,     0,     0,   341,    10,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,     0,   363,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,    36,     0,
      37,     0,     0,     0,    38,    39,    40,    41,     0,    42,
       0,    43,     0,    44,     0,     0,    45,     0,     0,     0,
      46,    47,    48,    49,    50,    51,    52,     0,    53,    54,
      55,    56,    57,    58,    59,    60,    61,     0,    62,    63,
      64,    65,    66,    67,     0,     0,     0,     0,    68,    69,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,     0,    76,    77,    78,
      79,    80,    81,     0,    82,    83,     0,    84,    85,    86,
      87,     0,     0,    88,     0,     0,    89,     0,     0,     0,
       0,     0,    90,    91,     0,    92,     0,    93,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,   110,     0,   111,
     112,     0,   113,   114,     0,   115,   116,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,     0,
     363,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,    36,
       0,    37,     0,     0,     0,    38,    39,    40,    41,     0,
      42,     0,    43,     0,    44,     0,     0,    45,     0,     0,
       0,    46,    47,    48,    49,     0,    51,    52,     0,    53,
       0,    55,    56,    57,    58,    59,    60,    61,     0,    62,
      63,    64,     0,    66,    67,     0,     0,     0,     0,    68,
      69,     0,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,    75,     0,     0,     0,     0,   174,    77,
      78,    79,    80,    81,     0,    82,    83,     0,    84,    85,
      86,    87,     0,     0,    88,     0,     0,    89,     0,     0,
       0,     0,     0,    90,     0,     0,     0,     0,    93,    94,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,   110,     0,
     111,   112,   543,   113,   114,     0,   115,   116,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,     0,
     363,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,    37,     0,     0,     0,    38,    39,    40,    41,
       0,    42,     0,    43,     0,    44,     0,     0,    45,     0,
       0,     0,    46,    47,    48,    49,     0,    51,    52,     0,
      53,     0,    55,    56,    57,    58,    59,    60,    61,     0,
      62,    63,    64,     0,    66,    67,     0,     0,     0,     0,
      68,    69,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,    75,     0,     0,     0,     0,   174,
      77,    78,    79,    80,    81,     0,    82,    83,     0,    84,
      85,    86,    87,     0,     0,    88,     0,     0,    89,     0,
       0,     0,     0,     0,    90,     0,     0,     0,     0,    93,
      94,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,   110,
       0,   111,   112,   886,   113,   114,     0,   115,   116,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,     0,
     363,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,    36,     0,    37,     0,     0,     0,    38,    39,    40,
      41,     0,    42,     0,    43,     0,    44,     0,     0,    45,
       0,     0,     0,    46,    47,    48,    49,     0,    51,    52,
       0,    53,     0,    55,    56,    57,    58,    59,    60,    61,
       0,    62,    63,    64,     0,    66,    67,     0,     0,     0,
       0,    68,    69,     0,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,    75,     0,     0,     0,     0,
     174,    77,    78,    79,    80,    81,     0,    82,    83,     0,
      84,    85,    86,    87,     0,     0,    88,     0,     0,    89,
       0,     0,     0,     0,     0,    90,     0,     0,     0,     0,
      93,    94,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
     110,     0,   111,   112,   979,   113,   114,     0,   115,   116,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,     0,
     363,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,    36,     0,    37,     0,     0,     0,    38,    39,
      40,    41,   981,    42,     0,    43,     0,    44,     0,     0,
      45,     0,     0,     0,    46,    47,    48,    49,     0,    51,
      52,     0,    53,     0,    55,    56,    57,    58,    59,    60,
      61,     0,    62,    63,    64,     0,    66,    67,     0,     0,
       0,     0,    68,    69,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
       0,   174,    77,    78,    79,    80,    81,     0,    82,    83,
       0,    84,    85,    86,    87,     0,     0,    88,     0,     0,
      89,     0,     0,     0,     0,     0,    90,     0,     0,     0,
       0,    93,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,   110,     0,   111,   112,     0,   113,   114,     0,   115,
     116,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,     0,
     363,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,    36,     0,    37,     0,     0,     0,    38,
      39,    40,    41,     0,    42,     0,    43,     0,    44,  1095,
       0,    45,     0,     0,     0,    46,    47,    48,    49,     0,
      51,    52,     0,    53,     0,    55,    56,    57,    58,    59,
      60,    61,     0,    62,    63,    64,     0,    66,    67,     0,
       0,     0,     0,    68,    69,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,    75,     0,     0,
       0,     0,   174,    77,    78,    79,    80,    81,     0,    82,
      83,     0,    84,    85,    86,    87,     0,     0,    88,     0,
       0,    89,     0,     0,     0,     0,     0,    90,     0,     0,
       0,     0,    93,    94,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,   110,     0,   111,   112,     0,   113,   114,     0,
     115,   116,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,    36,     0,    37,     0,     0,     0,
      38,    39,    40,    41,     0,    42,     0,    43,     0,    44,
       0,     0,    45,     0,     0,     0,    46,    47,    48,    49,
       0,    51,    52,     0,    53,     0,    55,    56,    57,    58,
      59,    60,    61,     0,    62,    63,    64,     0,    66,    67,
       0,     0,     0,     0,    68,    69,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,   174,    77,    78,    79,    80,    81,     0,
      82,    83,     0,    84,    85,    86,    87,     0,     0,    88,
       0,     0,    89,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,  1190,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,    36,     0,    37,     0,     0,
       0,    38,    39,    40,    41,     0,    42,     0,    43,     0,
      44,     0,     0,    45,     0,     0,     0,    46,    47,    48,
      49,     0,    51,    52,     0,    53,     0,    55,    56,    57,
      58,    59,    60,    61,     0,    62,    63,    64,     0,    66,
      67,     0,     0,     0,     0,    68,    69,     0,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,    75,
       0,     0,     0,     0,   174,    77,    78,    79,    80,    81,
       0,    82,    83,     0,    84,    85,    86,    87,     0,     0,
      88,     0,     0,    89,     0,     0,     0,     0,     0,    90,
       0,     0,     0,     0,    93,    94,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,   110,     0,   111,   112,  1386,   113,
     114,     0,   115,   116,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,    37,     0,
       0,     0,    38,    39,    40,    41,     0,    42,     0,    43,
    1425,    44,     0,     0,    45,     0,     0,     0,    46,    47,
      48,    49,     0,    51,    52,     0,    53,     0,    55,    56,
      57,    58,    59,    60,    61,     0,    62,    63,    64,     0,
      66,    67,     0,     0,     0,     0,    68,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,   174,    77,    78,    79,    80,
      81,     0,    82,    83,     0,    84,    85,    86,    87,     0,
       0,    88,     0,     0,    89,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,    93,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,   110,     0,   111,   112,     0,
     113,   114,     0,   115,   116,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,    36,     0,    37,
       0,     0,     0,    38,    39,    40,    41,     0,    42,     0,
      43,     0,    44,     0,     0,    45,     0,     0,     0,    46,
      47,    48,    49,     0,    51,    52,     0,    53,     0,    55,
      56,    57,    58,    59,    60,    61,     0,    62,    63,    64,
       0,    66,    67,     0,     0,     0,     0,    68,    69,     0,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,    75,     0,     0,     0,     0,   174,    77,    78,    79,
      80,    81,     0,    82,    83,     0,    84,    85,    86,    87,
       0,     0,    88,     0,     0,    89,     0,     0,     0,     0,
       0,    90,     0,     0,     0,     0,    93,    94,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,   110,     0,   111,   112,
    1455,   113,   114,     0,   115,   116,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,    36,     0,
      37,     0,     0,     0,    38,    39,    40,    41,     0,    42,
       0,    43,     0,    44,     0,     0,    45,     0,     0,     0,
      46,    47,    48,    49,     0,    51,    52,     0,    53,     0,
      55,    56,    57,    58,    59,    60,    61,     0,    62,    63,
      64,     0,    66,    67,     0,     0,     0,     0,    68,    69,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,     0,   174,    77,    78,
      79,    80,    81,     0,    82,    83,     0,    84,    85,    86,
      87,     0,     0,    88,     0,     0,    89,     0,     0,     0,
       0,     0,    90,     0,     0,     0,     0,    93,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,   110,     0,   111,
     112,  1456,   113,   114,     0,   115,   116,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,    36,
       0,    37,     0,     0,     0,    38,    39,    40,    41,     0,
      42,  1459,    43,     0,    44,     0,     0,    45,     0,     0,
       0,    46,    47,    48,    49,     0,    51,    52,     0,    53,
       0,    55,    56,    57,    58,    59,    60,    61,     0,    62,
      63,    64,     0,    66,    67,     0,     0,     0,     0,    68,
      69,     0,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,    75,     0,     0,     0,     0,   174,    77,
      78,    79,    80,    81,     0,    82,    83,     0,    84,    85,
      86,    87,     0,     0,    88,     0,     0,    89,     0,     0,
       0,     0,     0,    90,     0,     0,     0,     0,    93,    94,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,   110,     0,
     111,   112,     0,   113,   114,     0,   115,   116,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,    37,     0,     0,     0,    38,    39,    40,    41,
       0,    42,     0,    43,     0,    44,     0,     0,    45,     0,
       0,     0,    46,    47,    48,    49,     0,    51,    52,     0,
      53,     0,    55,    56,    57,    58,    59,    60,    61,     0,
      62,    63,    64,     0,    66,    67,     0,     0,     0,     0,
      68,    69,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,    75,     0,     0,     0,     0,   174,
      77,    78,    79,    80,    81,     0,    82,    83,     0,    84,
      85,    86,    87,     0,     0,    88,     0,     0,    89,     0,
       0,     0,     0,     0,    90,     0,     0,     0,     0,    93,
      94,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,   110,
       0,   111,   112,  1474,   113,   114,     0,   115,   116,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,    36,     0,    37,     0,     0,     0,    38,    39,    40,
      41,     0,    42,     0,    43,     0,    44,     0,     0,    45,
       0,     0,     0,    46,    47,    48,    49,     0,    51,    52,
       0,    53,     0,    55,    56,    57,    58,    59,    60,    61,
       0,    62,    63,    64,     0,    66,    67,     0,     0,     0,
       0,    68,    69,     0,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,    75,     0,     0,     0,     0,
     174,    77,    78,    79,    80,    81,     0,    82,    83,     0,
      84,    85,    86,    87,     0,     0,    88,     0,     0,    89,
       0,     0,     0,     0,     0,    90,     0,     0,     0,     0,
      93,    94,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
     110,     0,   111,   112,  1527,   113,   114,     0,   115,   116,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,    36,     0,    37,     0,     0,     0,    38,    39,
      40,    41,     0,    42,     0,    43,     0,    44,     0,     0,
      45,     0,     0,     0,    46,    47,    48,    49,     0,    51,
      52,     0,    53,     0,    55,    56,    57,    58,    59,    60,
      61,     0,    62,    63,    64,     0,    66,    67,     0,     0,
       0,     0,    68,    69,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
       0,   174,    77,    78,    79,    80,    81,     0,    82,    83,
       0,    84,    85,    86,    87,     0,     0,    88,     0,     0,
      89,     0,     0,     0,     0,     0,    90,     0,     0,     0,
       0,    93,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,   110,     0,   111,   112,  1533,   113,   114,     0,   115,
     116,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,    36,     0,    37,     0,     0,     0,    38,
      39,    40,    41,     0,    42,     0,    43,     0,    44,     0,
       0,    45,     0,     0,     0,    46,    47,    48,    49,     0,
      51,    52,     0,    53,     0,    55,    56,    57,    58,    59,
      60,    61,     0,    62,    63,    64,     0,    66,    67,     0,
       0,     0,     0,    68,    69,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,    75,     0,     0,
       0,     0,   174,    77,    78,    79,    80,    81,     0,    82,
      83,     0,    84,    85,    86,    87,     0,     0,    88,     0,
       0,    89,     0,     0,     0,     0,     0,    90,     0,     0,
       0,     0,    93,    94,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,   110,     0,   111,   112,     0,   113,   114,     0,
     115,   116,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   429,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,    36,     0,    37,     0,     0,     0,
      38,    39,    40,    41,     0,    42,     0,    43,     0,    44,
       0,     0,    45,     0,     0,     0,    46,    47,    48,    49,
       0,    51,    52,     0,    53,     0,    55,    56,    57,    58,
     170,   171,    61,     0,    62,    63,    64,     0,     0,     0,
       0,     0,     0,     0,    68,    69,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,   174,    77,    78,    79,    80,    81,     0,
      82,    83,     0,    84,    85,    86,     0,     0,     0,    88,
       0,     0,    89,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   659,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,    36,     0,    37,     0,     0,
       0,    38,    39,    40,    41,     0,    42,     0,    43,     0,
      44,     0,     0,    45,     0,     0,     0,    46,    47,    48,
      49,     0,    51,    52,     0,    53,     0,    55,    56,    57,
      58,   170,   171,    61,     0,    62,    63,    64,     0,     0,
       0,     0,     0,     0,     0,    68,    69,     0,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,    75,
       0,     0,     0,     0,   174,    77,    78,    79,    80,    81,
       0,    82,    83,     0,    84,    85,    86,     0,     0,     0,
      88,     0,     0,    89,     0,     0,     0,     0,     0,    90,
       0,     0,     0,     0,    93,    94,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,   110,     0,   111,   112,     0,   113,
     114,     0,   115,   116,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   847,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,    37,     0,
       0,     0,    38,    39,    40,    41,     0,    42,     0,    43,
       0,    44,     0,     0,    45,     0,     0,     0,    46,    47,
      48,    49,     0,    51,    52,     0,    53,     0,    55,    56,
      57,    58,   170,   171,    61,     0,    62,    63,    64,     0,
       0,     0,     0,     0,     0,     0,    68,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,   174,    77,    78,    79,    80,
      81,     0,    82,    83,     0,    84,    85,    86,     0,     0,
       0,    88,     0,     0,    89,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,    93,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,   110,     0,   111,   112,     0,
     113,   114,     0,   115,   116,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1248,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,    36,     0,    37,
       0,     0,     0,    38,    39,    40,    41,     0,    42,     0,
      43,     0,    44,     0,     0,    45,     0,     0,     0,    46,
      47,    48,    49,     0,    51,    52,     0,    53,     0,    55,
      56,    57,    58,   170,   171,    61,     0,    62,    63,    64,
       0,     0,     0,     0,     0,     0,     0,    68,    69,     0,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,    75,     0,     0,     0,     0,   174,    77,    78,    79,
      80,    81,     0,    82,    83,     0,    84,    85,    86,     0,
       0,     0,    88,     0,     0,    89,     0,     0,     0,     0,
       0,    90,     0,     0,     0,     0,    93,    94,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,   110,     0,   111,   112,
       0,   113,   114,     0,   115,   116,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1379,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,    36,     0,
      37,     0,     0,     0,    38,    39,    40,    41,     0,    42,
       0,    43,     0,    44,     0,     0,    45,     0,     0,     0,
      46,    47,    48,    49,     0,    51,    52,     0,    53,     0,
      55,    56,    57,    58,   170,   171,    61,     0,    62,    63,
      64,     0,     0,     0,     0,     0,     0,     0,    68,    69,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,     0,   174,    77,    78,
      79,    80,    81,     0,    82,    83,     0,    84,    85,    86,
       0,     0,     0,    88,     0,     0,    89,     0,     0,     0,
       0,     0,    90,     0,     0,     0,     0,    93,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,   110,     0,   111,
     112,     0,   113,   114,     0,   115,   116,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,    36,
       0,    37,     0,     0,     0,    38,    39,    40,    41,     0,
      42,     0,    43,     0,    44,     0,     0,    45,     0,     0,
       0,    46,    47,    48,    49,     0,    51,    52,     0,    53,
       0,    55,    56,    57,    58,   170,   171,    61,     0,    62,
      63,    64,     0,     0,     0,     0,     0,     0,     0,    68,
      69,     0,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,    75,     0,     0,     0,     0,   174,    77,
      78,    79,    80,    81,     0,    82,    83,     0,    84,    85,
      86,     0,     0,     0,    88,     0,     0,    89,     0,     0,
       0,     0,     0,    90,     0,     0,     0,     0,    93,    94,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,   110,     0,
     111,   112,     0,   113,   114,     0,   115,   116,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,     0,     0,     0,     0,    38,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,     0,
       0,     0,     0,    56,    57,    58,   170,   171,   172,     0,
       0,    63,    64,     0,     0,     0,     0,     0,     0,     0,
     173,    69,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,    75,     0,     0,     0,     0,   174,
      77,    78,    79,     0,    81,     0,    82,    83,     0,    84,
      85,    86,     0,     0,     0,    88,     0,     0,    89,     0,
       0,     0,     0,     0,    90,     0,     0,     0,     0,    93,
      94,    95,   253,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,   110,
       0,   254,     0,     0,   113,   114,     0,   115,   116,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,    36,     0,     0,     0,     0,     0,    38,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    49,     0,     0,     0,
       0,     0,     0,     0,    56,    57,    58,   170,   171,   172,
      34,     0,    63,    64,     0,     0,     0,     0,     0,     0,
       0,   173,    69,     0,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,    75,     0,     0,     0,     0,
     174,    77,    78,    79,   607,    81,     0,    82,    83,     0,
      84,    85,    86,     0,     0,     0,    88,     0,     0,    89,
       0,     0,     0,     0,     0,    90,     0,     0,     0,     0,
      93,    94,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,    82,    83,
     110,    84,    85,    86,     0,   113,   114,     0,   115,   116,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,     0,   207,
       0,   560,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,    35,    36,     0,     0,     0,     0,     0,    38,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    49,     0,     0,
       0,     0,     0,     0,     0,    56,    57,    58,   170,   171,
     172,    34,     0,    63,    64,     0,     0,     0,     0,     0,
       0,     0,   173,    69,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
       0,   174,    77,    78,    79,     0,    81,     0,    82,    83,
       0,    84,    85,    86,     0,     0,     0,    88,     0,     0,
      89,     0,     0,     0,     0,     0,    90,     0,     0,     0,
       0,    93,     0,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,    82,
      83,   110,    84,    85,    86,     0,   113,   114,     0,   115,
     116,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,     0,
       0,     0,   781,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,    36,     0,     0,     0,     0,     0,    38,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    49,     0,
       0,     0,     0,     0,     0,     0,    56,    57,    58,   170,
     171,   172,     0,     0,    63,    64,     0,     0,     0,     0,
       0,     0,     0,   173,    69,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,    75,     0,     0,
       0,     0,   174,    77,    78,    79,     0,    81,     0,    82,
      83,     0,    84,    85,    86,     0,     0,     0,    88,     0,
       0,    89,     0,     0,     0,     0,     0,    90,     0,     0,
       0,     0,    93,     0,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,   110,     0,   236,     0,     0,   113,   114,     0,
     115,   116,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,    35,    36,     0,     0,     0,     0,     0,
      38,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    56,    57,    58,
     170,   171,   172,     0,     0,    63,    64,     0,     0,     0,
       0,     0,     0,     0,   173,    69,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,   174,    77,    78,    79,     0,    81,     0,
      82,    83,     0,    84,    85,    86,     0,     0,     0,    88,
       0,     0,    89,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,    93,     0,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   239,     0,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,    36,     0,     0,     0,     0,
       0,    38,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   295,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,    56,    57,
      58,   170,   171,   172,    34,     0,    63,    64,     0,     0,
       0,     0,     0,     0,     0,   173,    69,     0,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,    75,
       0,     0,     0,     0,   174,    77,    78,    79,     0,    81,
       0,    82,    83,     0,    84,    85,    86,     0,     0,     0,
      88,     0,     0,    89,     0,     0,     0,     0,     0,    90,
       0,     0,     0,     0,    93,     0,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,    82,    83,   110,    84,    85,    86,     0,   113,
     114,     0,   115,   116,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,     0,     0,
       0,     0,    38,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    49,     0,     0,     0,     0,     0,     0,     0,    56,
      57,    58,   170,   171,   172,     0,     0,    63,    64,     0,
       0,     0,     0,     0,     0,     0,   173,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,   174,    77,    78,    79,     0,
      81,     0,    82,    83,     0,    84,    85,    86,     0,     0,
       0,    88,     0,     0,    89,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,    93,     0,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,   110,   427,     0,     0,     0,
     113,   114,     0,   115,   116,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   555,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,    34,    35,    36,     0,     0,
       0,     0,     0,    38,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    49,     0,     0,     0,     0,     0,     0,     0,
      56,    57,    58,   170,   171,   172,     0,     0,    63,    64,
       0,     0,     0,     0,     0,     0,     0,   173,    69,     0,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,    75,     0,     0,     0,     0,   174,    77,    78,    79,
       0,    81,     0,    82,    83,     0,    84,    85,    86,     0,
       0,     0,    88,     0,     0,    89,     0,     0,     0,     0,
       0,    90,     0,     0,     0,     0,    93,     0,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,   110,     0,     0,     0,
       0,   113,   114,     0,   115,   116,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     567,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,    33,    34,    35,    36,     0,
       0,     0,     0,     0,    38,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    49,     0,     0,     0,     0,     0,     0,
       0,    56,    57,    58,   170,   171,   172,     0,     0,    63,
      64,     0,     0,     0,     0,     0,     0,     0,   173,    69,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,     0,   174,    77,    78,
      79,     0,    81,     0,    82,    83,     0,    84,    85,    86,
       0,     0,     0,    88,     0,     0,    89,     0,     0,     0,
       0,     0,    90,     0,     0,     0,     0,    93,     0,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,   110,     0,     0,
       0,     0,   113,   114,     0,   115,   116,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   606,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,    35,    36,
       0,     0,     0,     0,     0,    38,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,    56,    57,    58,   170,   171,   172,     0,     0,
      63,    64,     0,     0,     0,     0,     0,     0,     0,   173,
      69,     0,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,    75,     0,     0,     0,     0,   174,    77,
      78,    79,     0,    81,     0,    82,    83,     0,    84,    85,
      86,     0,     0,     0,    88,     0,     0,    89,     0,     0,
       0,     0,     0,    90,     0,     0,     0,     0,    93,     0,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,   110,     0,
       0,     0,     0,   113,   114,     0,   115,   116,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   641,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,     0,     0,     0,     0,    38,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,     0,
       0,     0,     0,    56,    57,    58,   170,   171,   172,     0,
       0,    63,    64,     0,     0,     0,     0,     0,     0,     0,
     173,    69,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,    75,     0,     0,     0,     0,   174,
      77,    78,    79,     0,    81,     0,    82,    83,     0,    84,
      85,    86,     0,     0,     0,    88,     0,     0,    89,     0,
       0,     0,     0,     0,    90,     0,     0,     0,     0,    93,
       0,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,   110,
       0,     0,     0,     0,   113,   114,     0,   115,   116,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   643,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,    36,     0,     0,     0,     0,     0,    38,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    49,     0,     0,     0,
       0,     0,     0,     0,    56,    57,    58,   170,   171,   172,
       0,     0,    63,    64,     0,     0,     0,     0,     0,     0,
       0,   173,    69,     0,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,    75,     0,     0,     0,     0,
     174,    77,    78,    79,     0,    81,     0,    82,    83,     0,
      84,    85,    86,     0,     0,     0,    88,     0,     0,    89,
       0,     0,     0,     0,     0,    90,     0,     0,     0,     0,
      93,     0,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
     110,     0,     0,     0,     0,   113,   114,     0,   115,   116,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,    35,    36,     0,     0,     0,     0,     0,    38,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    49,     0,     0,
       0,     0,     0,     0,     0,    56,    57,    58,   170,   171,
     172,     0,     0,    63,    64,     0,     0,     0,     0,     0,
       0,     0,   173,    69,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
       0,   174,    77,    78,    79,     0,    81,     0,    82,    83,
       0,    84,    85,    86,     0,     0,     0,    88,     0,     0,
      89,     0,     0,     0,     0,     0,    90,     0,     0,     0,
       0,    93,     0,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,   110,     0,     0,   655,     0,   113,   114,     0,   115,
     116,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     930,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,    36,     0,     0,     0,     0,     0,    38,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    49,     0,
       0,     0,     0,     0,     0,     0,    56,    57,    58,   170,
     171,   172,     0,     0,    63,    64,     0,     0,     0,     0,
       0,     0,     0,   173,    69,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,    75,     0,     0,
       0,     0,   174,    77,    78,    79,     0,    81,     0,    82,
      83,     0,    84,    85,    86,     0,     0,     0,    88,     0,
       0,    89,     0,     0,     0,     0,     0,    90,     0,     0,
       0,     0,    93,     0,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,   110,     0,     0,     0,     0,   113,   114,     0,
     115,   116,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   971,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,    35,    36,     0,     0,     0,     0,     0,
      38,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    56,    57,    58,
     170,   171,   172,     0,     0,    63,    64,     0,     0,     0,
       0,     0,     0,     0,   173,    69,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,   174,    77,    78,    79,     0,    81,     0,
      82,    83,     0,    84,    85,    86,     0,     0,     0,    88,
       0,     0,    89,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,    93,     0,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,     0,     0,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,    36,     0,     0,     0,     0,
       0,    38,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,    56,    57,
      58,   170,   171,   172,     0,     0,    63,    64,     0,     0,
       0,     0,     0,     0,     0,   173,    69,     0,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,    75,
       0,     0,     0,     0,   174,    77,    78,    79,     0,    81,
       0,    82,    83,     0,    84,    85,    86,     0,     0,     0,
      88,     0,     0,    89,     0,     0,     0,     0,     0,    90,
       0,     0,     0,     0,    93,     0,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,   110,     0,     0,     0,     0,   113,
     114,     0,   115,   116,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,   514,    36,     0,     0,     0,
       0,     0,    38,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    49,     0,     0,     0,     0,     0,     0,     0,    56,
      57,    58,   170,   171,   172,     0,     0,    63,    64,     0,
       0,     0,     0,     0,     0,     0,   173,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,   174,    77,    78,    79,     0,
      81,     0,    82,    83,     0,    84,    85,    86,     0,     0,
       0,    88,     0,     0,    89,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,    93,     0,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,   110,   338,   339,   340,     0,
     113,   114,     0,   115,   116,     0,     0,     0,     0,     0,
       0,     0,   341,     0,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,     0,   363,   338,   339,   340,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   341,     0,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,     0,   363,   338,   339,
     340,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   341,     0,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,     0,   363,   338,
     339,   340,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   341,    34,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,     0,   363,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   338,   339,   340,     0,     0,     0,     0,
       0,     0,     0,     0,   973,     0,     0,     0,     0,   341,
    1099,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,  1351,   363,    82,    83,  1352,    84,    85,    86,
       0,     0,     0,     0,     0,  1148,     0,     0,     0,  1267,
    1268,  1269,  1270,  1271,     0,     0,  1272,  1273,  1274,  1275,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,     0,     0,     0,  1209,     0,     0,
       0,     0,     0,     0,     0,     0,  1222,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1276,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1277,
    1278,  1279,  1280,  1281,  1282,  1283,     0,     0,     0,    34,
       0,     0,     0,     0,     0,     0,     0,  1223,  1284,  1285,
    1286,  1287,  1288,  1289,  1290,  1291,  1292,  1293,  1294,  1295,
    1296,  1297,  1298,  1299,  1300,  1301,  1302,  1303,  1304,  1305,
    1306,  1307,  1308,  1309,  1310,  1311,  1312,  1313,  1314,  1315,
    1316,  1317,  1318,  1319,  1320,  1321,  1322,  1323,  1324,     0,
       0,  1325,  1326,     0,  1327,  1328,  1329,  1330,  1331,  1100,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1332,  1333,  1334,     0,  1335,     0,     0,    82,    83,     0,
      84,    85,    86,  1336,     0,  1337,  1338,     0,  1339,     0,
       0,     0,     0,     0,     0,  1340,  1341,     0,  1342,     0,
    1343,  1344,  1345,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   338,   339,   340,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   341,     0,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,     0,   363,   338,   339,
     340,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   341,     0,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,     0,   363,   338,
     339,   340,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   341,     0,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,     0,   363,
     338,   339,   340,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   341,     0,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,     0,
     363,     0,     0,     0,     0,     0,   338,   339,   340,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   341,   364,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,     0,   363,   338,   339,   340,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   341,   443,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,     0,   363,   338,   339,
     340,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   341,   445,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,     0,   363,   338,
     339,   340,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   341,   457,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,     0,   363,
       0,     0,     0,     0,     0,   338,   339,   340,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   341,   481,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,     0,   363,     0,   338,   339,   340,
       0,    34,     0,   199,     0,     0,     0,     0,     0,     0,
       0,     0,   633,   341,     0,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,     0,   363,   338,   339,
     340,     0,     0,     0,     0,     0,     0,   597,     0,     0,
       0,     0,     0,   652,   341,     0,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   241,   363,    82,
      83,     0,    84,    85,    86,     0,     0,     0,     0,   883,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   242,     0,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,     0,
       0,     0,     0,     0,    34,   598,     0,   113,     0,     0,
       0,   879,   880,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   241,     0,     0,     0,     0,     0,     0,     0,
       0,  -298,     0,     0,     0,     0,     0,     0,     0,    56,
      57,    58,   170,   171,   329,     0,     0,     0,   242,  1256,
       0,     0,     0,     0,     0,     0,     0,   243,   244,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    34,
       0,     0,     0,     0,     0,   174,     0,     0,    79,     0,
     245,     0,    82,    83,     0,    84,    85,    86,     0,  1117,
       0,     0,     0,     0,     0,     0,   450,     0,     0,     0,
     246,     0,   241,     0,     0,     0,     0,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   243,   244,     0,   247,     0,     0,   242,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     174,     0,     0,    79,     0,   245,     0,    82,    83,    34,
      84,    85,    86,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   246,     0,   241,     0,     0,
       0,     0,     0,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,     0,     0,     0,
     247,     0,     0,   242,     0,     0,     0,     0,     0,     0,
       0,     0,   243,   244,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    34,     0,     0,     0,     0,     0,
     174,     0,     0,    79,     0,   245,     0,    82,    83,     0,
      84,    85,    86,     0,   860,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   246,     0,   241,     0,     0,
       0,     0,     0,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   243,   244,     0,
     247,     0,     0,   242,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   174,     0,     0,    79,     0,
     245,     0,    82,    83,    34,    84,    85,    86,     0,  1104,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     246,     0,     0,     0,     0,     0,     0,     0,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,     0,   677,   678,   247,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   243,   244,     0,
       0,     0,   679,     0,     0,     0,     0,     0,     0,     0,
      31,    32,    33,    34,     0,   174,     0,     0,    79,     0,
     245,    38,    82,    83,     0,    84,    85,    86,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     246,     0,     0,     0,     0,     0,     0,     0,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,     0,     0,     0,   247,   680,     0,    70,    71,
      72,    73,    74,     0,   809,   810,     0,     0,     0,   681,
       0,     0,     0,     0,   174,    77,    78,    79,     0,   682,
       0,    82,    83,   811,    84,    85,    86,     0,     0,     0,
      88,   812,   813,   814,    34,     0,     0,     0,     0,   683,
       0,     0,   815,     0,    93,     0,     0,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   798,     0,     0,     0,     0,     0,     0,     0,
       0,    34,     0,   199,     0,     0,     0,   816,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     817,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    82,    83,     0,    84,    85,    86,     0,     0,
       0,   200,     0,     0,     0,     0,     0,     0,     0,     0,
     818,     0,    34,     0,   199,     0,     0,     0,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   174,     0,     0,    79,     0,    81,     0,    82,
      83,     0,    84,    85,    86,     0,     0,     0,     0,     0,
       0,     0,   200,     0,     0,     0,     0,     0,     0,     0,
       0,    34,     0,   199,     0,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,     0,
       0,     0,   201,   174,     0,     0,    79,   113,    81,     0,
      82,    83,     0,    84,    85,    86,     0,     0,     0,     0,
       0,   200,     0,     0,     0,     0,     0,     0,     0,     0,
      34,     0,   199,   905,     0,     0,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
       0,     0,   174,   201,     0,    79,   490,    81,   113,    82,
      83,     0,    84,    85,    86,     0,     0,     0,     0,     0,
     200,     0,     0,     0,     0,     0,     0,     0,     0,    34,
       0,   199,     0,     0,     0,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,     0,
       0,   174,   201,     0,    79,     0,    81,   113,    82,    83,
       0,    84,    85,    86,     0,     0,     0,     0,     0,   212,
      34,     0,   199,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,     0,     0,
     174,   201,     0,    79,     0,    81,   113,    82,    83,     0,
      84,    85,    86,     0,     0,     0,     0,     0,     0,    34,
       0,   199,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,     0,    82,    83,
     213,    84,    85,    86,     0,   113,     0,     0,     0,    34,
       0,   199,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,     0,     0,
       0,     0,     0,     0,   623,     0,   113,    82,    83,     0,
      84,    85,    86,     0,     0,     0,     0,     0,    34,     0,
     199,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,    82,    83,     0,
      84,    85,    86,   598,     0,   113,     0,     0,     0,     0,
       0,     0,     0,     0,   954,   955,   956,    34,     0,     0,
       0,     0,     0,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,     0,     0,     0,
       0,     0,     0,   649,     0,   113,    82,    83,     0,    84,
      85,    86,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    34,     0,     0,     0,     0,     0,
       0,     0,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,     0,     0,     0,     0,
       0,     0,   926,     0,   113,    82,    83,     0,    84,    85,
      86,     0,     0,     0,     0,  1197,     0,     0,     0,     0,
       0,    34,     0,     0,     0,     0,     0,  1198,     0,     0,
       0,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   174,     0,     0,    79,     0,
    1199,     0,    82,    83,     0,    84,  1200,    86,     0,     0,
       0,    34,     0,   735,   736,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    34,     0,     0,     0,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   174,     0,     0,    79,     0,    81,     0,    82,
      83,     0,    84,    85,    86,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    34,     0,
       0,     0,     0,     0,     0,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,    82,
      83,     0,    84,    85,    86,     0,     0,     0,     0,     0,
     326,     0,    82,    83,     0,    84,    85,    86,     0,     0,
       0,     0,    34,     0,     0,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   482,     0,     0,     0,    82,    83,     0,    84,
      85,    86,    34,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   263,     0,     0,     0,
      82,    83,     0,    84,    85,    86,     0,     0,  1010,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
      82,    83,     0,    84,    85,    86,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   338,   339,   340,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     341,     0,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,     0,   363,   338,   339,   340,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   341,     0,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,     0,   363,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   338,   339,   340,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   341,   413,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
       0,   363,   338,   339,   340,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   341,   525,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,     0,   363,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   338,   339,
     340,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   341,   787,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,     0,   363,   338,
     339,   340,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   341,   827,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,     0,   363,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   338,   339,   340,     0,
       0,     0,  1062,     0,     0,     0,     0,     0,     0,     0,
       0,   663,   341,   784,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,     0,   363,   338,   339,   340,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   341,     0,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,     0,   363,   339,   340,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   341,     0,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,     0,   363
};

static const yytype_int16 yycheck[] =
{
       4,   156,   322,     4,    51,   176,    87,   131,   241,   242,
      91,    92,    30,   557,   247,     4,     4,     4,   634,   761,
      28,   214,   922,    41,     4,   400,   166,    45,   375,   363,
     159,     4,   777,   754,   319,     9,   110,   913,  1057,   662,
     521,     9,   220,    47,    27,     9,    50,     4,     9,   130,
     453,    24,    25,   177,   220,   395,     9,   407,   110,     9,
      54,     9,    30,    67,     9,    42,     9,     9,     9,     9,
     304,    42,     9,     9,   780,    24,    25,    62,     9,    28,
       9,   421,    76,    87,     9,    79,   319,    91,    92,   795,
       9,   494,   221,     9,     9,    62,     9,    27,     9,   104,
       9,     9,     9,   147,    62,     9,    92,   123,   123,    47,
     668,    32,   165,   123,  1374,   194,    95,    96,   121,   122,
      42,   114,   115,   116,    80,    42,   130,   201,   196,   197,
     131,   196,   197,  1152,   194,  1154,    75,   110,   123,   213,
     194,   194,     0,    42,     8,   108,   109,   110,   111,   112,
     113,   872,   196,   197,    75,   194,   123,   162,   429,   330,
      80,   147,  1422,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    62,   177,    62,    62,   195,
     195,   531,   186,  1059,   140,    62,   197,    62,   198,   934,
    1066,  1067,   937,   119,    62,    62,    62,    62,   194,   156,
      62,    75,    62,   277,   143,    62,    62,   478,    62,    60,
      61,   149,   197,   176,   197,   448,   166,  1236,   195,   198,
     140,   192,   226,   166,   198,   277,   230,   195,   201,   369,
     234,   402,   196,   197,   207,   196,   194,   158,   226,   197,
     213,   947,   230,   196,   325,   966,   596,   195,   252,   389,
     374,   196,   201,   226,   196,   196,   196,   230,   207,   196,
     196,   664,   166,   744,   213,   196,   196,   196,   262,   263,
     195,   411,   123,   195,   677,   678,   679,   196,   195,   195,
     420,   196,   195,   423,   195,  1161,   195,   195,   195,   143,
     371,   372,   850,   192,   852,   268,   195,   291,    75,    75,
     533,   534,   275,   276,   277,   309,   539,   194,   309,   282,
     197,    47,   197,   197,   318,   288,  1061,    62,   322,   268,
     197,   325,   197,   673,   194,   199,   275,   276,   375,   197,
     197,   197,   197,   282,   408,   197,   309,   197,    75,   288,
     197,   197,    75,   197,   195,    92,    92,    80,    32,    32,
      32,   967,   309,   119,    32,   363,   731,  1376,    75,   982,
     194,   369,   540,    80,   143,   369,   370,   371,   372,   194,
      75,    92,   373,   776,   540,    80,   194,    32,   123,    75,
     194,   389,   150,   194,   655,   389,   165,   194,   659,  1110,
     675,    75,    75,    75,   149,    95,    96,    75,    67,    68,
     147,   147,   194,   411,   141,   142,   599,   411,   141,   142,
     197,    75,   420,   149,   363,   423,    80,  1162,   197,   423,
      75,    26,   199,   199,   141,   142,   147,   400,   164,   197,
      27,   624,   436,   143,   605,   408,   141,   142,    43,    67,
      68,    46,   675,   676,   140,   141,   142,   640,   436,  1170,
     147,   400,   197,   793,   194,   165,   150,    73,  1074,   408,
    1076,   639,   802,   436,   798,    32,   869,   645,   196,   163,
     203,   874,   822,   639,  1493,   158,   158,   141,   142,   645,
     158,     4,   563,   196,   194,   489,    62,   197,   482,  1508,
     196,   684,   895,   487,   366,    14,    62,   202,   198,   197,
     504,  1400,  1401,   197,  1396,  1397,   510,   511,    27,   108,
     109,   110,   111,   112,   113,   196,   645,   196,   868,    42,
     392,    94,    95,    96,   396,    44,   759,   761,   196,    75,
      62,   147,   143,   196,    80,   151,   152,   770,   154,   155,
     156,    26,   194,   777,   194,   108,   109,   110,   111,   112,
     113,    62,  1168,    94,    95,    96,   119,   120,    43,   563,
     907,    46,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   143,   847,   176,   101,   194,
      41,   197,   555,   106,   987,   108,   109,   110,   111,   112,
     113,   114,   147,    50,   157,   141,   142,   196,  1001,  1499,
    1003,   165,   123,   943,     9,   143,   555,   100,   201,   984,
     143,   951,   962,   176,  1514,   108,   109,   110,   111,   112,
     113,   854,   194,   856,   123,   765,   143,  1243,   151,   152,
     634,   154,   636,   606,   177,   178,   179,   108,   109,   110,
     658,   184,   185,     8,   196,   188,   189,   165,  1051,   653,
     194,    14,   653,   176,    75,  1030,    14,   606,  1008,    80,
      75,   665,   666,    14,  1014,   898,   196,   196,   641,   195,
     643,    75,   196,   165,    81,   198,    80,    14,   666,    92,
     653,   195,   195,   176,    46,    47,    48,   920,    50,   922,
     663,   200,   641,   666,   643,     9,   653,   194,   100,   194,
     934,   195,   706,   937,   194,   706,  1109,   195,   712,   713,
    1113,    84,  1115,     9,   663,  1055,   797,   711,   196,   140,
     141,   142,    14,  1063,    81,   194,     9,  1469,   180,    75,
      75,  1071,  1082,   706,   927,    75,   140,   141,   142,   743,
     183,   194,   743,  1146,  1486,   196,   285,   720,     9,   706,
     289,  1022,  1494,    75,   743,   743,   743,   765,   731,   732,
     195,   765,  1378,   743,   195,   998,   196,   121,   194,    62,
     743,   720,   311,   195,   313,   314,   315,   316,   122,   164,
    1183,   124,   731,   732,     9,   195,   743,    14,     9,   192,
     798,    62,     9,   797,  1028,   195,    14,   121,     9,   198,
     201,   194,  1036,  1123,   201,   809,   810,   811,   194,  1149,
     201,   195,   201,   194,   196,  1086,   834,   195,   124,   196,
       9,   195,  1225,   194,   143,   194,   143,  1061,   180,   197,
    1101,   835,   180,   837,    14,   839,   837,     9,   839,    43,
      44,    45,    46,    47,    48,    75,    50,   835,   197,   798,
      14,  1254,  1255,    92,   196,    14,   197,  1260,   197,    14,
     907,   196,   835,   201,   837,    27,   839,   871,   841,   873,
      14,   194,  1105,  1417,    70,    71,    72,     9,   194,   194,
     837,   194,   839,   194,   196,    81,   890,  1120,  1228,   196,
     195,   194,   841,   124,    14,   899,  1216,     9,   195,   201,
    1133,  1172,   124,     9,   195,   140,    75,   911,     9,  1180,
     911,   124,   194,   197,    75,   196,    14,    75,  1189,  1153,
     194,   194,   911,   911,   911,  1159,  1160,   195,  1162,   194,
     197,   911,   128,   129,   130,   131,   132,   195,   911,   195,
    1484,   453,   197,   139,   194,     4,   124,   201,     9,   145,
     146,    27,   140,    69,   911,   196,   974,   930,   195,   963,
     196,  1364,    27,   967,   160,   166,   195,   124,     9,   195,
      24,    25,   124,   977,    28,   198,     9,  1248,   174,   195,
    1135,   930,   494,    42,   988,    92,   198,   988,    14,   977,
     195,   194,  1395,   195,   124,   195,    50,   194,   971,   197,
     195,   195,   195,  1406,   977,     9,  1240,    27,  1411,  1133,
     196,   984,   985,   195,   124,   988,   197,   196,   153,   196,
     149,    14,   971,    75,    75,   195,   194,   106,   195,   195,
     195,   988,   124,   124,   124,   984,   985,    14,    75,   195,
      14,   195,   101,  1214,   197,   196,   194,   106,   197,   108,
     109,   110,   111,   112,   113,   114,   195,  1030,   124,   196,
     196,    14,    14,   197,   195,  1468,    52,    75,   194,    75,
    1074,  1028,  1076,     9,   196,    75,   104,    92,    92,  1036,
     143,  1030,   156,    30,    14,   194,    75,   195,   159,   162,
     194,   196,   151,   152,     9,   154,   195,    75,  1369,   196,
    1371,   195,   195,    75,    14,    75,    14,    75,  1379,   197,
      14,    75,    14,   711,  1477,  1119,   156,   176,  1119,  1123,
    1523,   487,   370,   372,  1128,   371,   796,   794,  1531,   746,
    1490,  1132,  1535,   985,  1486,  1538,  1098,   492,  1265,   198,
    1349,  1518,  1506,  1361,  1128,    39,  1119,   201,  1235,  1420,
     921,   375,   664,   207,   885,   882,   703,   467,   467,   213,
     914,   949,  1119,   810,  1168,   677,   678,   869,   963,  1173,
     824,   857,  1173,  1177,    -1,    -1,   276,  1181,  1135,    -1,
    1181,   283,    -1,    -1,    -1,    -1,    -1,   241,   242,  1177,
      -1,    -1,  1363,   247,    -1,    -1,  1153,    -1,    -1,  1203,
    1173,    -1,  1159,  1160,  1177,  1209,    -1,    -1,  1181,    -1,
      -1,    -1,  1216,    -1,   268,    -1,  1173,  1218,    -1,     4,
      -1,   275,   276,    -1,  1181,    -1,    -1,    -1,   282,    -1,
      -1,    -1,    -1,    -1,   288,  1469,    -1,    -1,    -1,  1243,
      -1,  1512,  1246,  1247,    -1,  1246,    -1,  1251,  1519,    -1,
    1251,    -1,  1486,  1257,    -1,    -1,  1257,    42,    -1,  1247,
    1494,    -1,    -1,    -1,   776,   319,  1499,    -1,   322,    -1,
      -1,    -1,    -1,  1246,  1247,    -1,    -1,    -1,  1251,  1360,
      -1,  1514,    -1,  1240,  1257,    -1,    -1,    -1,    -1,  1246,
      -1,    -1,    -1,    -1,  1251,    -1,    -1,    -1,    -1,    -1,
    1257,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   363,
    1465,    -1,   453,    -1,    -1,    -1,   101,    -1,    -1,    -1,
      -1,   106,    -1,   108,   109,   110,   111,   112,   113,   114,
      -1,  1412,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    24,    25,    -1,    -1,    28,   400,    -1,    -1,    -1,
      -1,    -1,    -1,   494,   408,    -1,  1360,  1481,    -1,    -1,
      -1,    -1,   874,    -1,    -1,    -1,   151,   152,    -1,   154,
      -1,    -1,    -1,    -1,  1378,    -1,    -1,    -1,  1382,    -1,
      -1,  1382,    -1,   895,    -1,    -1,    -1,  1391,    -1,    -1,
      -1,   176,  1396,  1397,   448,    -1,  1400,  1401,    -1,   453,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1412,  1382,
      -1,    -1,    -1,   198,  1418,  1419,    -1,  1418,  1419,    -1,
    1424,    -1,    -1,  1424,    -1,  1382,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    -1,    -1,
     494,    -1,    -1,    -1,    -1,  1418,  1419,    -1,    -1,    -1,
      -1,  1424,    -1,  1457,    -1,    -1,  1457,    -1,    -1,    -1,
    1464,  1418,  1419,    -1,    -1,    -1,    -1,  1424,     4,    -1,
      -1,    -1,    60,    61,    -1,   987,  1480,    -1,    -1,   533,
     534,    -1,    -1,    -1,  1457,   539,    -1,    -1,    -1,  1001,
      -1,  1003,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1457,   555,    -1,    -1,    -1,    -1,    42,    -1,  1465,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1520,    -1,    -1,  1520,
      -1,    -1,  1526,   664,   207,  1526,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   123,   677,   678,   679,  1051,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1520,    -1,    -1,
      -1,    -1,   606,  1526,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1520,    -1,   101,    -1,    -1,    -1,  1526,
     106,    -1,   108,   109,   110,   111,   112,   113,   114,     4,
      -1,    -1,    -1,    -1,    -1,   268,    -1,   641,    -1,   643,
      -1,    -1,   275,   276,    -1,    -1,    -1,  1109,    -1,   282,
      -1,  1113,    -1,  1115,    -1,   288,    -1,   195,    -1,   663,
     664,    -1,    -1,    -1,    -1,   151,   152,    42,   154,    -1,
      -1,   675,   676,   677,   678,   679,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1146,   776,    -1,    -1,    -1,    -1,
     176,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,   707,    50,    -1,     4,    -1,    -1,    -1,
      -1,    -1,   198,    -1,    -1,    -1,   720,    -1,    -1,    -1,
      -1,  1183,    -1,    -1,    -1,   729,   101,   731,   732,    -1,
     363,   106,    -1,   108,   109,   110,   111,   112,   113,   114,
      -1,    -1,   746,    -1,    42,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   759,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1225,    -1,    -1,   770,   400,    -1,    -1,
      -1,    -1,   776,    -1,    -1,   779,   151,   152,   869,   154,
      -1,    -1,    -1,   874,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1254,  1255,   798,    50,    -1,    -1,  1260,    -1,
      -1,   176,    -1,   101,   895,    -1,    -1,    -1,   106,    -1,
     108,   109,   110,   111,   112,   113,   114,    -1,    -1,    -1,
     453,    -1,    -1,   198,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,   841,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
     854,    -1,   856,   151,   152,    -1,   154,    -1,    -1,    -1,
      -1,   494,    -1,    -1,    -1,   869,    -1,    -1,    -1,    -1,
     874,    60,    61,    -1,    -1,    -1,    -1,    -1,   176,    -1,
      -1,    -1,    -1,    -1,    60,    61,    -1,    -1,    -1,    -1,
      -1,   895,    -1,    -1,   898,    -1,   987,    -1,    -1,    -1,
     198,    -1,  1364,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1001,    -1,  1003,    -1,    -1,    -1,   920,    -1,   922,    -1,
      -1,    -1,   555,    -1,    -1,    -1,   930,    -1,    -1,    -1,
      -1,    -1,    -1,  1395,   123,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1406,    -1,    -1,   123,    -1,  1411,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1051,    -1,    -1,    -1,    -1,    -1,    -1,   971,    -1,    -1,
      -1,    -1,    -1,   606,    -1,    -1,    -1,    -1,    -1,    -1,
     984,   985,    -1,   987,    -1,    -1,   241,   242,    -1,    -1,
      -1,    -1,   247,    -1,   998,    -1,    -1,  1001,    -1,  1003,
      -1,    -1,    -1,    -1,    -1,    -1,  1468,    -1,   641,    -1,
     643,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1109,    -1,
      -1,    -1,  1113,  1027,  1115,    -1,  1030,    -1,    -1,    -1,
     663,   664,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   677,   678,   679,  1051,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1146,    -1,    -1,    -1,    -1,
      -1,  1523,    -1,    -1,   319,    -1,    -1,   322,    -1,  1531,
      -1,    -1,    -1,  1535,   707,    -1,  1538,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   720,    -1,    -1,
      -1,    -1,  1183,    -1,    -1,    -1,   729,    -1,   731,   732,
      -1,  1105,    -1,    -1,    -1,  1109,    -1,    -1,    -1,  1113,
      -1,  1115,    -1,   746,    -1,    -1,  1120,    -1,    73,  1123,
    1124,    -1,  1126,    -1,    -1,    -1,    -1,    -1,    -1,  1133,
      -1,    -1,    -1,    -1,  1225,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1146,   776,    -1,   100,   779,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1254,  1255,   798,    -1,    -1,    -1,  1260,
      -1,    -1,    -1,  1264,    -1,    -1,    -1,    -1,    -1,  1183,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1192,  1193,
      -1,    -1,   147,   448,    -1,    -1,   151,   152,   453,   154,
     155,   156,    -1,    -1,    -1,    -1,    -1,    -1,   841,    -1,
      63,    64,  1216,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      73,  1225,    75,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   869,    -1,    -1,   494,
      -1,   874,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1254,  1255,    -1,    -1,    -1,    -1,  1260,  1261,    -1,    -1,
     113,  1265,   895,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1364,    -1,    -1,    -1,    -1,   533,   534,
      -1,    -1,    -1,    -1,   539,    -1,    -1,    -1,    -1,    -1,
      -1,   144,    -1,    -1,   147,    -1,   149,   930,   151,   152,
      -1,   154,   155,   156,  1395,    -1,    -1,    -1,    -1,    -1,
     163,    -1,    -1,    -1,    -1,  1406,    -1,    -1,    -1,    -1,
    1411,    -1,    -1,    -1,    -1,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   971,    -1,
      -1,   194,    -1,  1434,    -1,    -1,   199,    -1,    -1,    -1,
      -1,   984,   985,    -1,   987,    -1,    -1,    -1,    -1,    -1,
    1364,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1001,    -1,
    1003,    -1,    -1,    -1,    -1,    -1,    -1,  1468,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1391,    -1,    -1,
      -1,  1395,    -1,    -1,  1027,    -1,    -1,  1030,    -1,    -1,
      -1,    -1,  1406,    -1,    -1,    -1,    -1,  1411,    -1,   664,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1051,    -1,
     675,   676,   677,   678,   679,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1523,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1531,    -1,    -1,    -1,  1535,    -1,    -1,  1538,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,  1468,    50,    -1,    -1,    -1,    -1,
      -1,    -1,  1476,    -1,    -1,    -1,  1109,    -1,    -1,    -1,
    1113,    -1,  1115,    -1,    -1,    -1,  1490,    -1,    -1,    -1,
      -1,  1124,    -1,  1126,    -1,  1499,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   759,    -1,    -1,    -1,    -1,    -1,
    1514,    -1,    -1,  1146,    -1,   770,    -1,    -1,    -1,  1523,
      -1,   776,    -1,    -1,    -1,    -1,    -1,  1531,    -1,    -1,
      -1,  1535,    -1,    -1,  1538,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
    1183,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1192,
    1193,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,
      -1,    -1,  1225,    -1,    -1,    -1,    -1,    -1,    -1,   854,
      -1,   856,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   869,    -1,    -1,    -1,    -1,   874,
      -1,  1254,  1255,    -1,    -1,    -1,    -1,  1260,  1261,    -1,
      -1,    -1,  1265,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     895,    -1,    -1,   898,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   920,    26,   922,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     5,     6,    73,     8,     9,    10,
      -1,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    -1,    -1,    26,    27,    -1,    -1,    -1,
      -1,  1364,   987,    -1,    -1,    -1,   201,    -1,    39,    -1,
      -1,    -1,    -1,   998,    -1,    46,  1001,    48,  1003,    -1,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1395,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1406,    -1,    -1,    -1,   144,  1411,    80,
     147,    -1,    -1,    -1,   151,   152,    -1,   154,   155,   156,
      -1,    -1,    -1,    94,    -1,    -1,  1051,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   110,
      -1,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   198,    -1,    -1,    -1,  1468,    -1,    -1,   198,    -1,
      -1,    -1,    -1,  1476,    -1,    -1,    -1,    -1,    -1,    -1,
    1105,    -1,    -1,    -1,  1109,    -1,    -1,  1490,  1113,    -1,
    1115,    -1,    -1,    -1,    -1,  1120,    -1,    -1,  1123,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1133,    -1,
     181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1523,  1146,    -1,    -1,    -1,    -1,    -1,    -1,  1531,    -1,
      -1,    -1,  1535,    -1,    -1,  1538,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,   222,    -1,    -1,   225,    -1,    -1,    -1,  1183,    -1,
      -1,   232,   233,    26,    -1,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    -1,    -1,
      -1,  1216,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1225,    -1,    -1,    -1,    -1,    -1,   277,    -1,    -1,    -1,
      -1,    -1,   283,    -1,    -1,    -1,   287,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1254,
    1255,    -1,    -1,    -1,    -1,  1260,    -1,    -1,    -1,   310,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     321,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   338,   339,   340,
     341,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,    -1,    -1,   365,   366,    -1,   368,    -1,    -1,
      -1,    -1,    -1,    -1,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,    -1,    -1,    -1,    -1,
      -1,   392,   393,    -1,   395,   396,   397,    -1,    -1,    -1,
      -1,    -1,   403,    -1,    -1,   198,    -1,    -1,    -1,  1364,
      -1,    -1,   413,    -1,   415,    -1,    -1,    -1,    -1,    -1,
     421,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     431,    -1,   433,    -1,    -1,    -1,  1391,    -1,    32,    -1,
    1395,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1406,    -1,    -1,    -1,    -1,  1411,    -1,    -1,   460,
      -1,    -1,   463,   464,   465,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    73,
      -1,    75,    -1,    -1,    -1,    -1,    -1,    -1,    26,   490,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    50,  1468,    -1,    -1,    -1,    -1,    -1,   113,
      -1,    -1,    -1,    -1,   525,    -1,    -1,    -1,    -1,    -1,
      -1,   125,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1499,    -1,    -1,    -1,    -1,    -1,
     144,    -1,    -1,   147,    -1,   149,    -1,   151,   152,  1514,
     154,   155,   156,    -1,    -1,    -1,   567,    -1,  1523,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1531,    -1,    -1,    -1,
    1535,    -1,    -1,  1538,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   598,    -1,    -1,
     194,    -1,    -1,    -1,    -1,   199,   607,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   623,    26,    -1,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,   649,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     198,   662,    -1,    -1,    -1,    26,    -1,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      -1,   692,    10,    11,    12,    -1,    -1,    -1,    73,    -1,
      75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    50,    -1,    -1,    -1,    -1,    -1,   113,   740,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     125,    -1,   753,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,   144,
      50,    -1,   147,   774,   149,    -1,   151,   152,    -1,   154,
     155,   156,    -1,   784,    73,    -1,   787,    -1,   789,    -1,
      -1,    -1,   793,    -1,    -1,   198,    -1,    -1,    -1,    -1,
      -1,   802,    -1,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,    -1,    -1,    -1,   194,
      10,    11,    12,    -1,   199,    -1,   827,    -1,   117,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    26,   198,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   876,   877,   878,    -1,    -1,
     198,   882,   883,    -1,    -1,    -1,    -1,    -1,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,    -1,    -1,    -1,   194,   907,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   926,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,
      -1,    -1,   943,    -1,    -1,    -1,    -1,    43,    44,    -1,
     951,    -1,   953,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      -1,   982,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
     991,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,   198,    -1,
      -1,    -1,   108,   109,   110,   111,   112,   113,    -1,    -1,
     116,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,
     126,    -1,   128,   129,   130,   131,   132,    -1,  1039,    -1,
      -1,    -1,  1043,   139,  1045,    -1,    -1,    -1,   144,   145,
     146,   147,   148,   149,  1055,   151,   152,    -1,   154,   155,
     156,  1062,  1063,    -1,   160,    -1,    -1,   163,    -1,    -1,
    1071,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,    -1,    -1,   194,    -1,
      -1,    -1,    -1,   199,   200,    -1,   202,   203,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1117,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    26,  1149,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    75,    -1,    77,    -1,
      -1,    -1,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    90,    -1,    -1,    93,    -1,    -1,  1228,    97,    98,
      99,   100,   101,   102,   103,    -1,   105,   106,   107,   108,
     109,   110,   111,   112,   113,    -1,   115,   116,   117,   118,
     119,   120,    -1,    -1,    -1,  1256,   125,   126,    -1,   128,
     129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,   148,
     149,    -1,   151,   152,    -1,   154,   155,   156,   157,    -1,
      -1,   160,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,
     169,   170,    -1,   172,   183,   174,   175,   176,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,    -1,    -1,   194,    -1,   196,   197,   198,
     199,   200,    12,   202,   203,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    -1,    -1,    -1,    42,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    75,    -1,    77,
      -1,    -1,    -1,    81,    82,    83,    84,    -1,    86,    -1,
      88,    -1,    90,    -1,    -1,    93,    -1,    -1,    -1,    97,
      98,    99,   100,   101,   102,   103,    -1,   105,   106,   107,
     108,   109,   110,   111,   112,   113,    -1,   115,   116,   117,
     118,   119,   120,    -1,    -1,    -1,    -1,   125,   126,    -1,
     128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,
      -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,
     148,   149,    -1,   151,   152,    -1,   154,   155,   156,   157,
      -1,    -1,   160,    -1,    -1,   163,    -1,    -1,    -1,    -1,
      -1,   169,   170,    -1,   172,    -1,   174,   175,   176,    -1,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,    -1,    -1,   194,    -1,   196,   197,
     198,   199,   200,    -1,   202,   203,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    26,    13,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    42,    43,    44,    -1,    -1,
      -1,    -1,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    75,    -1,
      77,    -1,    -1,    -1,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    90,    -1,    -1,    93,    -1,    -1,    -1,
      97,    98,    99,   100,   101,   102,   103,    -1,   105,   106,
     107,   108,   109,   110,   111,   112,   113,    -1,   115,   116,
     117,   118,   119,   120,    -1,    -1,    -1,    -1,   125,   126,
      -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,
      -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,
     147,   148,   149,    -1,   151,   152,    -1,   154,   155,   156,
     157,    -1,    -1,   160,    -1,    -1,   163,    -1,    -1,    -1,
      -1,    -1,   169,   170,    -1,   172,    -1,   174,   175,   176,
      -1,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,    -1,    -1,   194,    -1,   196,
     197,    -1,   199,   200,    -1,   202,   203,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,    -1,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      -1,    77,    -1,    -1,    -1,    81,    82,    83,    84,    -1,
      86,    -1,    88,    -1,    90,    -1,    -1,    93,    -1,    -1,
      -1,    97,    98,    99,   100,    -1,   102,   103,    -1,   105,
      -1,   107,   108,   109,   110,   111,   112,   113,    -1,   115,
     116,   117,    -1,   119,   120,    -1,    -1,    -1,    -1,   125,
     126,    -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,
      -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,
     146,   147,   148,   149,    -1,   151,   152,    -1,   154,   155,
     156,   157,    -1,    -1,   160,    -1,    -1,   163,    -1,    -1,
      -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,   175,
     176,    -1,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,    -1,    -1,   194,    -1,
     196,   197,   198,   199,   200,    -1,   202,   203,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      75,    -1,    77,    -1,    -1,    -1,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    90,    -1,    -1,    93,    -1,
      -1,    -1,    97,    98,    99,   100,    -1,   102,   103,    -1,
     105,    -1,   107,   108,   109,   110,   111,   112,   113,    -1,
     115,   116,   117,    -1,   119,   120,    -1,    -1,    -1,    -1,
     125,   126,    -1,   128,   129,   130,   131,   132,    -1,    -1,
      -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,
     145,   146,   147,   148,   149,    -1,   151,   152,    -1,   154,
     155,   156,   157,    -1,    -1,   160,    -1,    -1,   163,    -1,
      -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,
     175,   176,    -1,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,    -1,    -1,   194,
      -1,   196,   197,   198,   199,   200,    -1,   202,   203,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    75,    -1,    77,    -1,    -1,    -1,    81,    82,    83,
      84,    -1,    86,    -1,    88,    -1,    90,    -1,    -1,    93,
      -1,    -1,    -1,    97,    98,    99,   100,    -1,   102,   103,
      -1,   105,    -1,   107,   108,   109,   110,   111,   112,   113,
      -1,   115,   116,   117,    -1,   119,   120,    -1,    -1,    -1,
      -1,   125,   126,    -1,   128,   129,   130,   131,   132,    -1,
      -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,
     144,   145,   146,   147,   148,   149,    -1,   151,   152,    -1,
     154,   155,   156,   157,    -1,    -1,   160,    -1,    -1,   163,
      -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,
     174,   175,   176,    -1,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,    -1,    -1,
     194,    -1,   196,   197,   198,   199,   200,    -1,   202,   203,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    75,    -1,    77,    -1,    -1,    -1,    81,    82,
      83,    84,    85,    86,    -1,    88,    -1,    90,    -1,    -1,
      93,    -1,    -1,    -1,    97,    98,    99,   100,    -1,   102,
     103,    -1,   105,    -1,   107,   108,   109,   110,   111,   112,
     113,    -1,   115,   116,   117,    -1,   119,   120,    -1,    -1,
      -1,    -1,   125,   126,    -1,   128,   129,   130,   131,   132,
      -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,
      -1,   144,   145,   146,   147,   148,   149,    -1,   151,   152,
      -1,   154,   155,   156,   157,    -1,    -1,   160,    -1,    -1,
     163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,
      -1,   174,   175,   176,    -1,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,    -1,
      -1,   194,    -1,   196,   197,    -1,   199,   200,    -1,   202,
     203,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    75,    -1,    77,    -1,    -1,    -1,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    90,    91,
      -1,    93,    -1,    -1,    -1,    97,    98,    99,   100,    -1,
     102,   103,    -1,   105,    -1,   107,   108,   109,   110,   111,
     112,   113,    -1,   115,   116,   117,    -1,   119,   120,    -1,
      -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,   131,
     132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,
      -1,    -1,   144,   145,   146,   147,   148,   149,    -1,   151,
     152,    -1,   154,   155,   156,   157,    -1,    -1,   160,    -1,
      -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,
      -1,    -1,   174,   175,   176,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
      -1,    -1,   194,    -1,   196,   197,    -1,   199,   200,    -1,
     202,   203,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    75,    -1,    77,    -1,    -1,    -1,
      81,    82,    83,    84,    -1,    86,    -1,    88,    -1,    90,
      -1,    -1,    93,    -1,    -1,    -1,    97,    98,    99,   100,
      -1,   102,   103,    -1,   105,    -1,   107,   108,   109,   110,
     111,   112,   113,    -1,   115,   116,   117,    -1,   119,   120,
      -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,
     131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,
      -1,    -1,    -1,   144,   145,   146,   147,   148,   149,    -1,
     151,   152,    -1,   154,   155,   156,   157,    -1,    -1,   160,
      -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,
      -1,    -1,    -1,   174,   175,   176,    -1,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,    -1,    -1,   194,    -1,   196,   197,   198,   199,   200,
      -1,   202,   203,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    75,    -1,    77,    -1,    -1,
      -1,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      90,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,    99,
     100,    -1,   102,   103,    -1,   105,    -1,   107,   108,   109,
     110,   111,   112,   113,    -1,   115,   116,   117,    -1,   119,
     120,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,
     130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,
      -1,    -1,    -1,    -1,   144,   145,   146,   147,   148,   149,
      -1,   151,   152,    -1,   154,   155,   156,   157,    -1,    -1,
     160,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,
      -1,    -1,    -1,    -1,   174,   175,   176,    -1,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,    -1,    -1,   194,    -1,   196,   197,   198,   199,
     200,    -1,   202,   203,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    75,    -1,    77,    -1,
      -1,    -1,    81,    82,    83,    84,    -1,    86,    -1,    88,
      89,    90,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,
      99,   100,    -1,   102,   103,    -1,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,    -1,   115,   116,   117,    -1,
     119,   120,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,
     129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,   148,
     149,    -1,   151,   152,    -1,   154,   155,   156,   157,    -1,
      -1,   160,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,
     169,    -1,    -1,    -1,    -1,   174,   175,   176,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,    -1,    -1,   194,    -1,   196,   197,    -1,
     199,   200,    -1,   202,   203,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    75,    -1,    77,
      -1,    -1,    -1,    81,    82,    83,    84,    -1,    86,    -1,
      88,    -1,    90,    -1,    -1,    93,    -1,    -1,    -1,    97,
      98,    99,   100,    -1,   102,   103,    -1,   105,    -1,   107,
     108,   109,   110,   111,   112,   113,    -1,   115,   116,   117,
      -1,   119,   120,    -1,    -1,    -1,    -1,   125,   126,    -1,
     128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,
      -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,
     148,   149,    -1,   151,   152,    -1,   154,   155,   156,   157,
      -1,    -1,   160,    -1,    -1,   163,    -1,    -1,    -1,    -1,
      -1,   169,    -1,    -1,    -1,    -1,   174,   175,   176,    -1,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,    -1,    -1,   194,    -1,   196,   197,
     198,   199,   200,    -1,   202,   203,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,    44,    -1,    -1,
      -1,    -1,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    75,    -1,
      77,    -1,    -1,    -1,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    90,    -1,    -1,    93,    -1,    -1,    -1,
      97,    98,    99,   100,    -1,   102,   103,    -1,   105,    -1,
     107,   108,   109,   110,   111,   112,   113,    -1,   115,   116,
     117,    -1,   119,   120,    -1,    -1,    -1,    -1,   125,   126,
      -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,
      -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,
     147,   148,   149,    -1,   151,   152,    -1,   154,   155,   156,
     157,    -1,    -1,   160,    -1,    -1,   163,    -1,    -1,    -1,
      -1,    -1,   169,    -1,    -1,    -1,    -1,   174,   175,   176,
      -1,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,    -1,    -1,   194,    -1,   196,
     197,   198,   199,   200,    -1,   202,   203,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,    -1,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      -1,    77,    -1,    -1,    -1,    81,    82,    83,    84,    -1,
      86,    87,    88,    -1,    90,    -1,    -1,    93,    -1,    -1,
      -1,    97,    98,    99,   100,    -1,   102,   103,    -1,   105,
      -1,   107,   108,   109,   110,   111,   112,   113,    -1,   115,
     116,   117,    -1,   119,   120,    -1,    -1,    -1,    -1,   125,
     126,    -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,
      -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,
     146,   147,   148,   149,    -1,   151,   152,    -1,   154,   155,
     156,   157,    -1,    -1,   160,    -1,    -1,   163,    -1,    -1,
      -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,   175,
     176,    -1,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,    -1,    -1,   194,    -1,
     196,   197,    -1,   199,   200,    -1,   202,   203,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      75,    -1,    77,    -1,    -1,    -1,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    90,    -1,    -1,    93,    -1,
      -1,    -1,    97,    98,    99,   100,    -1,   102,   103,    -1,
     105,    -1,   107,   108,   109,   110,   111,   112,   113,    -1,
     115,   116,   117,    -1,   119,   120,    -1,    -1,    -1,    -1,
     125,   126,    -1,   128,   129,   130,   131,   132,    -1,    -1,
      -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,
     145,   146,   147,   148,   149,    -1,   151,   152,    -1,   154,
     155,   156,   157,    -1,    -1,   160,    -1,    -1,   163,    -1,
      -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,
     175,   176,    -1,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,    -1,    -1,   194,
      -1,   196,   197,   198,   199,   200,    -1,   202,   203,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    75,    -1,    77,    -1,    -1,    -1,    81,    82,    83,
      84,    -1,    86,    -1,    88,    -1,    90,    -1,    -1,    93,
      -1,    -1,    -1,    97,    98,    99,   100,    -1,   102,   103,
      -1,   105,    -1,   107,   108,   109,   110,   111,   112,   113,
      -1,   115,   116,   117,    -1,   119,   120,    -1,    -1,    -1,
      -1,   125,   126,    -1,   128,   129,   130,   131,   132,    -1,
      -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,
     144,   145,   146,   147,   148,   149,    -1,   151,   152,    -1,
     154,   155,   156,   157,    -1,    -1,   160,    -1,    -1,   163,
      -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,
     174,   175,   176,    -1,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,    -1,    -1,
     194,    -1,   196,   197,   198,   199,   200,    -1,   202,   203,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    75,    -1,    77,    -1,    -1,    -1,    81,    82,
      83,    84,    -1,    86,    -1,    88,    -1,    90,    -1,    -1,
      93,    -1,    -1,    -1,    97,    98,    99,   100,    -1,   102,
     103,    -1,   105,    -1,   107,   108,   109,   110,   111,   112,
     113,    -1,   115,   116,   117,    -1,   119,   120,    -1,    -1,
      -1,    -1,   125,   126,    -1,   128,   129,   130,   131,   132,
      -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,
      -1,   144,   145,   146,   147,   148,   149,    -1,   151,   152,
      -1,   154,   155,   156,   157,    -1,    -1,   160,    -1,    -1,
     163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,
      -1,   174,   175,   176,    -1,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,    -1,
      -1,   194,    -1,   196,   197,   198,   199,   200,    -1,   202,
     203,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    75,    -1,    77,    -1,    -1,    -1,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    90,    -1,
      -1,    93,    -1,    -1,    -1,    97,    98,    99,   100,    -1,
     102,   103,    -1,   105,    -1,   107,   108,   109,   110,   111,
     112,   113,    -1,   115,   116,   117,    -1,   119,   120,    -1,
      -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,   131,
     132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,
      -1,    -1,   144,   145,   146,   147,   148,   149,    -1,   151,
     152,    -1,   154,   155,   156,   157,    -1,    -1,   160,    -1,
      -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,
      -1,    -1,   174,   175,   176,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
      -1,    -1,   194,    -1,   196,   197,    -1,   199,   200,    -1,
     202,   203,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    75,    -1,    77,    -1,    -1,    -1,
      81,    82,    83,    84,    -1,    86,    -1,    88,    -1,    90,
      -1,    -1,    93,    -1,    -1,    -1,    97,    98,    99,   100,
      -1,   102,   103,    -1,   105,    -1,   107,   108,   109,   110,
     111,   112,   113,    -1,   115,   116,   117,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,
     131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,
      -1,    -1,    -1,   144,   145,   146,   147,   148,   149,    -1,
     151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,   160,
      -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,
      -1,    -1,    -1,   174,   175,   176,    -1,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,    -1,    -1,   194,    -1,   196,   197,    -1,   199,   200,
      -1,   202,   203,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    75,    -1,    77,    -1,    -1,
      -1,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      90,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,    99,
     100,    -1,   102,   103,    -1,   105,    -1,   107,   108,   109,
     110,   111,   112,   113,    -1,   115,   116,   117,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,
     130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,
      -1,    -1,    -1,    -1,   144,   145,   146,   147,   148,   149,
      -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,
     160,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,
      -1,    -1,    -1,    -1,   174,   175,   176,    -1,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,    -1,    -1,   194,    -1,   196,   197,    -1,   199,
     200,    -1,   202,   203,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    75,    -1,    77,    -1,
      -1,    -1,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    90,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,
      99,   100,    -1,   102,   103,    -1,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,    -1,   115,   116,   117,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,
     129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,   148,
     149,    -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,
      -1,   160,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,
     169,    -1,    -1,    -1,    -1,   174,   175,   176,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,    -1,    -1,   194,    -1,   196,   197,    -1,
     199,   200,    -1,   202,   203,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    75,    -1,    77,
      -1,    -1,    -1,    81,    82,    83,    84,    -1,    86,    -1,
      88,    -1,    90,    -1,    -1,    93,    -1,    -1,    -1,    97,
      98,    99,   100,    -1,   102,   103,    -1,   105,    -1,   107,
     108,   109,   110,   111,   112,   113,    -1,   115,   116,   117,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,   126,    -1,
     128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,
      -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,
     148,   149,    -1,   151,   152,    -1,   154,   155,   156,    -1,
      -1,    -1,   160,    -1,    -1,   163,    -1,    -1,    -1,    -1,
      -1,   169,    -1,    -1,    -1,    -1,   174,   175,   176,    -1,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,    -1,    -1,   194,    -1,   196,   197,
      -1,   199,   200,    -1,   202,   203,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,
      -1,    -1,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    75,    -1,
      77,    -1,    -1,    -1,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    90,    -1,    -1,    93,    -1,    -1,    -1,
      97,    98,    99,   100,    -1,   102,   103,    -1,   105,    -1,
     107,   108,   109,   110,   111,   112,   113,    -1,   115,   116,
     117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,   126,
      -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,
      -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,
     147,   148,   149,    -1,   151,   152,    -1,   154,   155,   156,
      -1,    -1,    -1,   160,    -1,    -1,   163,    -1,    -1,    -1,
      -1,    -1,   169,    -1,    -1,    -1,    -1,   174,   175,   176,
      -1,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,    -1,    -1,   194,    -1,   196,
     197,    -1,   199,   200,    -1,   202,   203,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      -1,    77,    -1,    -1,    -1,    81,    82,    83,    84,    -1,
      86,    -1,    88,    -1,    90,    -1,    -1,    93,    -1,    -1,
      -1,    97,    98,    99,   100,    -1,   102,   103,    -1,   105,
      -1,   107,   108,   109,   110,   111,   112,   113,    -1,   115,
     116,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,
     126,    -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,
      -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,
     146,   147,   148,   149,    -1,   151,   152,    -1,   154,   155,
     156,    -1,    -1,    -1,   160,    -1,    -1,   163,    -1,    -1,
      -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,   175,
     176,    -1,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,    -1,    -1,   194,    -1,
     196,   197,    -1,   199,   200,    -1,   202,   203,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      75,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,   110,   111,   112,   113,    -1,
      -1,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     125,   126,    -1,   128,   129,   130,   131,   132,    -1,    -1,
      -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,
     145,   146,   147,    -1,   149,    -1,   151,   152,    -1,   154,
     155,   156,    -1,    -1,    -1,   160,    -1,    -1,   163,    -1,
      -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,    -1,    -1,   194,
      -1,   196,    -1,    -1,   199,   200,    -1,   202,   203,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    75,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,   109,   110,   111,   112,   113,
      73,    -1,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   125,   126,    -1,   128,   129,   130,   131,   132,    -1,
      -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,
     144,   145,   146,   147,   148,   149,    -1,   151,   152,    -1,
     154,   155,   156,    -1,    -1,    -1,   160,    -1,    -1,   163,
      -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,
     174,   175,   176,    -1,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   151,   152,
     194,   154,   155,   156,    -1,   199,   200,    -1,   202,   203,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,    -1,    32,
      -1,   194,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    75,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,   112,
     113,    73,    -1,   116,   117,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   125,   126,    -1,   128,   129,   130,   131,   132,
      -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,
      -1,   144,   145,   146,   147,    -1,   149,    -1,   151,   152,
      -1,   154,   155,   156,    -1,    -1,    -1,   160,    -1,    -1,
     163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,
      -1,   174,    -1,   176,    -1,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   151,
     152,   194,   154,   155,   156,    -1,   199,   200,    -1,   202,
     203,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,    -1,
      -1,    -1,   194,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,
     112,   113,    -1,    -1,   116,   117,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,   131,
     132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,
      -1,    -1,   144,   145,   146,   147,    -1,   149,    -1,   151,
     152,    -1,   154,   155,   156,    -1,    -1,    -1,   160,    -1,
      -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,
      -1,    -1,   174,    -1,   176,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
      -1,    -1,   194,    -1,   196,    -1,    -1,   199,   200,    -1,
     202,   203,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,
     111,   112,   113,    -1,    -1,   116,   117,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,
     131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,
      -1,    -1,    -1,   144,   145,   146,   147,    -1,   149,    -1,
     151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,   160,
      -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,
      -1,    -1,    -1,   174,    -1,   176,    -1,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,    -1,    -1,   194,    -1,   196,    -1,    -1,   199,   200,
      -1,   202,   203,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    75,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    97,    -1,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
     110,   111,   112,   113,    73,    -1,   116,   117,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,
     130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,
      -1,    -1,    -1,    -1,   144,   145,   146,   147,    -1,   149,
      -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,
     160,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,
      -1,    -1,    -1,    -1,   174,    -1,   176,    -1,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   151,   152,   194,   154,   155,   156,    -1,   199,
     200,    -1,   202,   203,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    75,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,   110,   111,   112,   113,    -1,    -1,   116,   117,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,
     129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,    -1,
     149,    -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,
      -1,   160,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,
     169,    -1,    -1,    -1,    -1,   174,    -1,   176,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,    -1,    -1,   194,   195,    -1,    -1,    -1,
     199,   200,    -1,   202,   203,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    75,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,   109,   110,   111,   112,   113,    -1,    -1,   116,   117,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,   126,    -1,
     128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,
      -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,
      -1,   149,    -1,   151,   152,    -1,   154,   155,   156,    -1,
      -1,    -1,   160,    -1,    -1,   163,    -1,    -1,    -1,    -1,
      -1,   169,    -1,    -1,    -1,    -1,   174,    -1,   176,    -1,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,    -1,    -1,   194,    -1,    -1,    -1,
      -1,   199,   200,    -1,   202,   203,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,
      -1,    -1,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    75,    -1,
      -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,   110,   111,   112,   113,    -1,    -1,   116,
     117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,   126,
      -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,
      -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,
     147,    -1,   149,    -1,   151,   152,    -1,   154,   155,   156,
      -1,    -1,    -1,   160,    -1,    -1,   163,    -1,    -1,    -1,
      -1,    -1,   169,    -1,    -1,    -1,    -1,   174,    -1,   176,
      -1,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,    -1,    -1,   194,    -1,    -1,
      -1,    -1,   199,   200,    -1,   202,   203,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,   109,   110,   111,   112,   113,    -1,    -1,
     116,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,
     126,    -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,
      -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,
     146,   147,    -1,   149,    -1,   151,   152,    -1,   154,   155,
     156,    -1,    -1,    -1,   160,    -1,    -1,   163,    -1,    -1,
      -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,    -1,
     176,    -1,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,    -1,    -1,   194,    -1,
      -1,    -1,    -1,   199,   200,    -1,   202,   203,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      75,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,   110,   111,   112,   113,    -1,
      -1,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     125,   126,    -1,   128,   129,   130,   131,   132,    -1,    -1,
      -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,
     145,   146,   147,    -1,   149,    -1,   151,   152,    -1,   154,
     155,   156,    -1,    -1,    -1,   160,    -1,    -1,   163,    -1,
      -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,
      -1,   176,    -1,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,    -1,    -1,   194,
      -1,    -1,    -1,    -1,   199,   200,    -1,   202,   203,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    75,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,   109,   110,   111,   112,   113,
      -1,    -1,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   125,   126,    -1,   128,   129,   130,   131,   132,    -1,
      -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,
     144,   145,   146,   147,    -1,   149,    -1,   151,   152,    -1,
     154,   155,   156,    -1,    -1,    -1,   160,    -1,    -1,   163,
      -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,
     174,    -1,   176,    -1,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,    -1,    -1,
     194,    -1,    -1,    -1,    -1,   199,   200,    -1,   202,   203,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    75,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,   112,
     113,    -1,    -1,   116,   117,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   125,   126,    -1,   128,   129,   130,   131,   132,
      -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,
      -1,   144,   145,   146,   147,    -1,   149,    -1,   151,   152,
      -1,   154,   155,   156,    -1,    -1,    -1,   160,    -1,    -1,
     163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,
      -1,   174,    -1,   176,    -1,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,    -1,
      -1,   194,    -1,    -1,   197,    -1,   199,   200,    -1,   202,
     203,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,
     112,   113,    -1,    -1,   116,   117,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,   131,
     132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,
      -1,    -1,   144,   145,   146,   147,    -1,   149,    -1,   151,
     152,    -1,   154,   155,   156,    -1,    -1,    -1,   160,    -1,
      -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,
      -1,    -1,   174,    -1,   176,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
      -1,    -1,   194,    -1,    -1,    -1,    -1,   199,   200,    -1,
     202,   203,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,
     111,   112,   113,    -1,    -1,   116,   117,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,
     131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,
      -1,    -1,    -1,   144,   145,   146,   147,    -1,   149,    -1,
     151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,   160,
      -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,
      -1,    -1,    -1,   174,    -1,   176,    -1,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,    -1,    -1,   194,    -1,    -1,    -1,    -1,   199,   200,
      -1,   202,   203,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    75,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
     110,   111,   112,   113,    -1,    -1,   116,   117,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,
     130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,
      -1,    -1,    -1,    -1,   144,   145,   146,   147,    -1,   149,
      -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,
     160,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,
      -1,    -1,    -1,    -1,   174,    -1,   176,    -1,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,    -1,    -1,   194,    -1,    -1,    -1,    -1,   199,
     200,    -1,   202,   203,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    75,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,   110,   111,   112,   113,    -1,    -1,   116,   117,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,
     129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,    -1,
     149,    -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,
      -1,   160,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,
     169,    -1,    -1,    -1,    -1,   174,    -1,   176,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,    -1,    -1,   194,    10,    11,    12,    -1,
     199,   200,    -1,   202,   203,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,    -1,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,    -1,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    26,    -1,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,    73,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,    -1,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,   149,    50,   151,   152,   153,   154,   155,   156,
      -1,    -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,     3,
       4,     5,     6,     7,    -1,    -1,    10,    11,    12,    13,
      -1,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,    -1,    -1,    -1,   194,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,
      64,    65,    66,    67,    68,    69,    -1,    -1,    -1,    73,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
      -1,   125,   126,    -1,   128,   129,   130,   131,   132,   196,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     144,   145,   146,    -1,   148,    -1,    -1,   151,   152,    -1,
     154,   155,   156,   157,    -1,   159,   160,    -1,   162,    -1,
      -1,    -1,    -1,    -1,    -1,   169,   170,    -1,   172,    -1,
     174,   175,   176,    -1,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,    -1,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    26,    -1,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,   196,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,   196,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    26,   196,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,   196,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,   196,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    -1,    10,    11,    12,
      -1,    73,    -1,    75,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   195,    26,    -1,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,
      -1,    -1,    -1,   195,    26,    -1,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    26,    50,   151,
     152,    -1,   154,   155,   156,    -1,    -1,    -1,    -1,   190,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    52,    -1,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,    -1,
      -1,    -1,    -1,    -1,    73,   197,    -1,   199,    -1,    -1,
      -1,   186,   187,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,   110,   111,   112,   113,    -1,    -1,    -1,    52,   182,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   126,   127,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,
      -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,
     149,    -1,   151,   152,    -1,   154,   155,   156,    -1,   181,
      -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,
     169,    -1,    26,    -1,    -1,    -1,    -1,   176,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   126,   127,    -1,   194,    -1,    -1,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     144,    -1,    -1,   147,    -1,   149,    -1,   151,   152,    73,
     154,   155,   156,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   169,    -1,    26,    -1,    -1,
      -1,    -1,    -1,    -1,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,    -1,    -1,    -1,
     194,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   126,   127,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,
     144,    -1,    -1,   147,    -1,   149,    -1,   151,   152,    -1,
     154,   155,   156,    -1,   158,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   169,    -1,    26,    -1,    -1,
      -1,    -1,    -1,    -1,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   126,   127,    -1,
     194,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,
     149,    -1,   151,   152,    73,   154,   155,   156,    -1,   158,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     169,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,    -1,    43,    44,   194,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   126,   127,    -1,
      -1,    -1,    62,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    -1,   144,    -1,    -1,   147,    -1,
     149,    81,   151,   152,    -1,   154,   155,   156,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     169,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,    -1,    -1,    -1,   194,   126,    -1,   128,   129,
     130,   131,   132,    -1,    43,    44,    -1,    -1,    -1,   139,
      -1,    -1,    -1,    -1,   144,   145,   146,   147,    -1,   149,
      -1,   151,   152,    62,   154,   155,   156,    -1,    -1,    -1,
     160,    70,    71,    72,    73,    -1,    -1,    -1,    -1,   169,
      -1,    -1,    81,    -1,   174,    -1,    -1,    -1,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    73,    -1,    75,    -1,    -1,    -1,   126,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,
      -1,   113,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     169,    -1,    73,    -1,    75,    -1,    -1,    -1,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   144,    -1,    -1,   147,    -1,   149,    -1,   151,
     152,    -1,   154,   155,   156,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   113,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    73,    -1,    75,    -1,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,    -1,
      -1,    -1,   194,   144,    -1,    -1,   147,   199,   149,    -1,
     151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,    -1,
      -1,   113,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      73,    -1,    75,   125,    -1,    -1,    -1,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
      -1,    -1,   144,   194,    -1,   147,   197,   149,   199,   151,
     152,    -1,   154,   155,   156,    -1,    -1,    -1,    -1,    -1,
     113,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,
      -1,    75,    -1,    -1,    -1,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,    -1,
      -1,   144,   194,    -1,   147,    -1,   149,   199,   151,   152,
      -1,   154,   155,   156,    -1,    -1,    -1,    -1,    -1,   113,
      73,    -1,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,    -1,    -1,
     144,   194,    -1,   147,    -1,   149,   199,   151,   152,    -1,
     154,   155,   156,    -1,    -1,    -1,    -1,    -1,    -1,    73,
      -1,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,    -1,   151,   152,
     194,   154,   155,   156,    -1,   199,    -1,    -1,    -1,    73,
      -1,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,    -1,    -1,
      -1,    -1,    -1,    -1,   197,    -1,   199,   151,   152,    -1,
     154,   155,   156,    -1,    -1,    -1,    -1,    -1,    73,    -1,
      75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   151,   152,    -1,
     154,   155,   156,   197,    -1,   199,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    -1,    -1,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,    -1,    -1,    -1,
      -1,    -1,    -1,   197,    -1,   199,   151,   152,    -1,   154,
     155,   156,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,    -1,    -1,    -1,    -1,
      -1,    -1,   197,    -1,   199,   151,   152,    -1,   154,   155,
     156,    -1,    -1,    -1,    -1,   114,    -1,    -1,    -1,    -1,
      -1,    73,    -1,    -1,    -1,    -1,    -1,   126,    -1,    -1,
      -1,    -1,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   144,    -1,    -1,   147,    -1,
     149,    -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,
      -1,    73,    -1,    75,    76,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   144,    -1,    -1,   147,    -1,   149,    -1,   151,
     152,    -1,   154,   155,   156,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   151,
     152,    -1,   154,   155,   156,    -1,    -1,    -1,    -1,    -1,
     149,    -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,
      -1,    -1,    73,    -1,    -1,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   147,    -1,    -1,    -1,   151,   152,    -1,   154,
     155,   156,    73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   147,    -1,    -1,    -1,
     151,   152,    -1,   154,   155,   156,    -1,    -1,   119,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,   124,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,   124,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    26,   124,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,   124,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    92,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,    -1,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,    -1,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   205,   206,     0,   207,     3,     4,     5,     6,     7,
      13,    42,    43,    44,    49,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    70,    71,    72,    73,    74,    75,    77,    81,    82,
      83,    84,    86,    88,    90,    93,    97,    98,    99,   100,
     101,   102,   103,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   115,   116,   117,   118,   119,   120,   125,   126,
     128,   129,   130,   131,   132,   139,   144,   145,   146,   147,
     148,   149,   151,   152,   154,   155,   156,   157,   160,   163,
     169,   170,   172,   174,   175,   176,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     194,   196,   197,   199,   200,   202,   203,   208,   211,   216,
     217,   218,   219,   220,   221,   224,   239,   240,   244,   249,
     255,   310,   311,   316,   320,   321,   322,   323,   324,   325,
     326,   327,   329,   332,   341,   342,   343,   345,   346,   348,
     367,   377,   378,   379,   384,   387,   405,   410,   412,   413,
     414,   415,   416,   417,   418,   419,   421,   434,   436,   438,
     111,   112,   113,   125,   144,   211,   239,   310,   326,   412,
     326,   194,   326,   326,   326,   403,   404,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,    75,
     113,   194,   217,   378,   379,   412,   412,    32,   326,   425,
     426,   326,   113,   194,   217,   378,   379,   380,   411,   417,
     422,   423,   194,   317,   381,   194,   317,   333,   318,   326,
     226,   317,   194,   194,   194,   317,   196,   326,   211,   196,
     326,    26,    52,   126,   127,   149,   169,   194,   211,   220,
     439,   449,   450,   177,   196,   323,   326,   347,   349,   197,
     232,   326,   100,   147,   212,   214,   216,    75,   199,   281,
     282,   119,   119,    75,   283,   194,   194,   194,   194,   211,
     253,   440,   194,   194,    75,    80,   140,   141,   142,   431,
     432,   147,   197,   216,   216,    97,   326,   254,   440,   149,
     194,   440,   440,   326,   334,   316,   326,   327,   412,   222,
     197,    80,   382,   431,    80,   431,   431,    27,   147,   165,
     441,   194,     9,   196,    32,   238,   149,   252,   440,   113,
     239,   311,   196,   196,   196,   196,   196,   196,    10,    11,
      12,    26,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    50,   196,    62,    62,   196,   197,   143,
     120,   157,   255,   309,   310,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    60,    61,   123,
     407,   408,    62,   197,   409,   194,    62,   197,   199,   418,
     194,   238,   239,    14,   326,    41,   211,   402,   194,   316,
     412,   143,   412,   124,   201,     9,   389,   316,   412,   441,
     143,   194,   383,   123,   407,   408,   409,   195,   326,    27,
     224,     8,   335,     9,   196,   224,   225,   318,   319,   326,
     211,   267,   228,   196,   196,   196,   450,   450,   165,   194,
     100,   442,   450,    14,   211,    75,   196,   196,   196,   177,
     178,   179,   184,   185,   188,   189,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   362,   363,   364,   233,   104,
     162,   196,   147,   213,   215,   216,   216,     9,   196,    92,
     197,   412,     9,   196,    14,     9,   196,   412,   435,   435,
     316,   327,   412,   195,   165,   247,   125,   412,   424,   425,
      62,   123,   140,   432,    74,   326,   412,    80,   140,   432,
     216,   210,   196,   197,   196,   124,   250,   368,   370,    81,
     336,   337,   339,    14,    92,   437,   277,   278,   405,   406,
     195,   195,   195,   198,   223,   224,   240,   244,   249,   326,
     200,   202,   203,   211,   442,    32,   279,   280,   326,   439,
     194,   440,   245,   238,   326,   326,   326,    27,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     380,   326,   420,   420,   326,   427,   428,   119,   197,   211,
     417,   418,   253,   254,   252,   239,    32,   148,   320,   323,
     326,   347,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   197,   211,   417,   420,   326,   279,   420,
     326,   424,   238,   195,   194,   401,     9,   389,   316,   195,
     211,    32,   326,    32,   326,   195,   195,   417,   279,   197,
     211,   417,   195,   222,   271,   197,   326,   326,    84,    27,
     224,   265,   196,    92,    14,     9,   195,    27,   197,   268,
     450,    81,   446,   447,   448,   194,     9,    43,    44,    62,
     126,   139,   149,   169,   217,   218,   220,   344,   378,   384,
     385,   386,   180,    75,   326,    75,    75,   326,   359,   360,
     326,   326,   352,   362,   183,   365,   222,   194,   231,   216,
     196,     9,    92,    92,   214,   211,   326,   282,   385,    75,
       9,   195,   195,   195,   195,   195,   196,   211,   445,   121,
     258,   194,     9,   195,   195,    75,    76,   211,   433,   211,
      62,   198,   198,   207,   209,   326,   122,   257,   164,    47,
     149,   164,   372,   124,     9,   389,   195,   450,   450,    14,
     192,     9,   390,   450,   451,   123,   407,   408,   409,   198,
       9,   166,   412,   195,     9,   390,    14,   330,   241,   121,
     256,   194,   440,   326,    27,   201,   201,   124,   198,     9,
     389,   326,   441,   194,   248,   251,   246,   238,    64,   412,
     326,   441,   194,   201,   198,   195,   201,   198,   195,    43,
      44,    62,    70,    71,    72,    81,   126,   139,   169,   211,
     392,   394,   397,   400,   211,   412,   412,   124,   407,   408,
     409,   195,   326,   272,    67,    68,   273,   222,   317,   222,
     319,    32,   125,   262,   412,   385,   211,    27,   224,   266,
     196,   269,   196,   269,     9,   166,   124,     9,   389,   195,
     158,   442,   443,   450,   385,   385,   385,   388,   391,   194,
      80,   143,   194,   143,   197,   326,   180,   180,    14,   186,
     187,   361,     9,   190,   365,    75,   198,   378,   197,   235,
      92,   215,   211,   211,   198,    14,   412,   196,    92,     9,
     166,   259,   378,   197,   424,   125,   412,    14,   201,   326,
     198,   207,   259,   197,   371,    14,   326,   336,   196,   450,
      27,   444,   406,    32,    75,   158,   197,   211,   417,   450,
      32,   326,   385,   277,   194,   378,   257,   331,   242,   326,
     326,   326,   198,   194,   279,   258,   257,   256,   440,   380,
     198,   194,   279,    14,    70,    71,    72,   211,   393,   393,
     394,   395,   396,   194,    80,   140,   194,     9,   389,   195,
     401,    32,   326,   198,    67,    68,   274,   317,   224,   198,
     196,    85,   196,   412,   194,   124,   261,    14,   222,   269,
      94,    95,    96,   269,   198,   450,   450,   446,     9,   195,
     195,   124,   201,     9,   389,   388,   211,   336,   338,   340,
     119,   211,   385,   429,   430,   326,   326,   326,   360,   326,
     350,    75,   236,   211,   385,   450,   211,     9,   284,   195,
     194,   320,   323,   326,   201,   198,   284,   150,   163,   197,
     367,   374,   150,   197,   373,   124,   196,   450,   335,   451,
      75,    14,    75,   326,   441,   194,   412,   195,   277,   197,
     277,   194,   124,   194,   279,   195,   197,   197,   257,   243,
     383,   194,   279,   195,   124,   201,     9,   389,   395,   140,
     336,   398,   399,   394,   412,   317,    27,    69,   224,   196,
     319,   424,   262,   195,   385,    91,    94,   196,   326,    27,
     196,   270,   198,   166,   158,    27,   385,   385,   195,   124,
       9,   389,   195,   124,   198,     9,   389,   181,   195,   222,
      92,   378,     4,   101,   106,   114,   151,   152,   154,   198,
     285,   308,   309,   310,   315,   405,   424,   198,   198,    47,
     326,   326,   326,    32,    75,   158,    14,   385,   198,   194,
     279,   444,   195,   284,   195,   277,   326,   279,   195,   284,
     284,   197,   194,   279,   195,   394,   394,   195,   124,   195,
       9,   389,    27,   222,   196,   195,   195,   229,   196,   196,
     270,   222,   450,   124,   385,   336,   385,   385,   326,   197,
     198,   450,   121,   122,   439,   260,   378,   114,   126,   149,
     155,   294,   295,   296,   378,   153,   300,   301,   117,   194,
     211,   302,   303,   286,   239,   450,     9,   196,   309,   195,
     149,   369,   198,   198,    75,    14,    75,   385,   194,   279,
     195,   106,   328,   444,   198,   444,   195,   195,   198,   198,
     284,   277,   195,   124,   394,   336,   222,   227,    27,   224,
     264,   222,   195,   385,   124,   124,   182,   222,   378,   378,
      14,     9,   196,   197,   197,     9,   196,     3,     4,     5,
       6,     7,    10,    11,    12,    13,    50,    63,    64,    65,
      66,    67,    68,    69,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   125,   126,   128,   129,   130,
     131,   132,   144,   145,   146,   148,   157,   159,   160,   162,
     169,   170,   172,   174,   175,   176,   211,   375,   376,     9,
     196,   149,   153,   211,   303,   304,   305,   196,    75,   314,
     238,   287,   439,   239,    14,   385,   279,   195,   194,   197,
     196,   197,   306,   328,   444,   198,   195,   394,   124,    27,
     224,   263,   222,   385,   385,   326,   198,   196,   196,   385,
     378,   290,   297,   384,   295,    14,    27,    44,   298,   301,
       9,    30,   195,    26,    43,    46,    14,     9,   196,   440,
     314,    14,   238,   385,   195,    32,    75,   366,   222,   222,
     197,   306,   444,   394,   222,    89,   183,   234,   198,   211,
     220,   291,   292,   293,     9,   198,   385,   376,   376,    52,
     299,   304,   304,    26,    43,    46,   385,    75,   194,   196,
     385,   440,    75,     9,   390,   198,   198,   222,   306,    87,
     196,    75,   104,   230,   143,    92,   384,   156,    14,   288,
     194,    32,    75,   195,   198,   196,   194,   162,   237,   211,
     309,   310,   385,   275,   276,   406,   289,    75,   378,   235,
     159,   211,   196,   195,     9,   390,   108,   109,   110,   312,
     313,   275,    75,   260,   196,   444,   406,   451,   195,   195,
     196,   196,   197,   307,   312,    32,    75,   158,   444,   197,
     222,   451,    75,    14,    75,   307,   222,   198,    32,    75,
     158,    14,   385,   198,    75,    14,    75,   385,    14,   385,
     385
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
# define YYLEX yylex (&yylval, &yylloc)
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
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 768 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 771 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 773 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 774 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 775 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 776 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 777 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 779 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 780 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 794 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 796 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 799 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 800 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 801 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 802 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 803 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 808 "hphp.y"
    { ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 809 "hphp.y"
    { ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 814 "hphp.y"
    { ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 815 "hphp.y"
    { ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 819 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 820 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 821 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 823 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 827 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 828 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 829 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 831 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 835 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 842 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 843 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 846 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 853 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 860 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 868 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 871 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 877 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 878 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 881 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 882 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 883 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 884 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 887 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 891 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 896 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 897 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 899 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 903 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 906 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 910 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 912 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 915 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 917 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 920 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 921 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 922 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 923 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 924 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 925 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 926 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 928 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 929 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 930 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 931 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 935 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 937 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 941 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 948 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 949 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 952 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 953 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 954 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 955 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval));;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 959 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 960 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 961 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 962 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 963 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 964 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 965 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 966 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 967 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 968 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 969 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 977 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 978 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 987 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 988 "hphp.y"
    { (yyval).reset();;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 992 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 994 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 1000 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 1001 "hphp.y"
    { (yyval).reset();;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 1005 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 1006 "hphp.y"
    { (yyval).reset();;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 1010 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 1015 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 1021 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1027 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1033 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1039 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1045 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1053 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1056 "hphp.y"
    { Token stmts;
                                         if (_p->peekClass()) {
                                           xhp_collect_attributes(_p,stmts,(yyvsp[(7) - (8)]));
                                         } else {
                                           stmts = (yyvsp[(7) - (8)]);
                                         }
                                         _p->onClass((yyval),(yyvsp[(1) - (8)]).num(),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),
                                                     stmts,0);
                                         if (_p->peekClass()) {
                                           _p->xhpResetAttributes();
                                         }
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1071 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1074 "hphp.y"
    { Token stmts;
                                         if (_p->peekClass()) {
                                           xhp_collect_attributes(_p,stmts,(yyvsp[(8) - (9)]));
                                         } else {
                                           stmts = (yyvsp[(8) - (9)]);
                                         }
                                         _p->onClass((yyval),(yyvsp[(2) - (9)]).num(),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),
                                                     stmts,&(yyvsp[(1) - (9)]));
                                         if (_p->peekClass()) {
                                           _p->xhpResetAttributes();
                                         }
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1088 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1091 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1096 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1099 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1106 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1109 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1117 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1120 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1128 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1129 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1133 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1136 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1139 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1140 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1141 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1145 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1146 "hphp.y"
    { (yyval).reset();;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1149 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1150 "hphp.y"
    { (yyval).reset();;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1153 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1154 "hphp.y"
    { (yyval).reset();;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1157 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1159 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1162 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1164 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1168 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1169 "hphp.y"
    { (yyval).reset();;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1172 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1173 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1174 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1178 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1180 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1183 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1185 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1188 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1190 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1193 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1195 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1205 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1206 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1207 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1208 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1213 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1215 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1216 "hphp.y"
    { (yyval).reset();;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1219 "hphp.y"
    { (yyval).reset();;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1220 "hphp.y"
    { (yyval).reset();;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1225 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1226 "hphp.y"
    { (yyval).reset();;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1231 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1232 "hphp.y"
    { (yyval).reset();;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1235 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1236 "hphp.y"
    { (yyval).reset();;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1239 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1240 "hphp.y"
    { (yyval).reset();;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1248 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1254 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1262 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1267 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1270 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1276 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1280 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1285 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1290 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1295 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1300 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1312 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1320 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1325 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1329 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1332 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1336 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1339 "hphp.y"
    { (yyval).reset();;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1344 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1347 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1351 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1355 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1359 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1363 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1368 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1373 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1379 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1380 "hphp.y"
    { (yyval).reset();;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1383 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),0);;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1384 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),1);;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1386 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1388 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1392 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1393 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1396 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1397 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1398 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1402 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1404 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1405 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1406 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1411 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1412 "hphp.y"
    { (yyval).reset();;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1415 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1416 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1419 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1420 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1422 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1426 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1432 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1439 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1445 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1450 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1452 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1454 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1456 "hphp.y"
    { _p->onTraitRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1458 "hphp.y"
    { _p->onTraitRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1459 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1462 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1465 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1466 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1467 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1473 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1477 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1480 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1487 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1488 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1493 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1503 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1505 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1510 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1516 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1518 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1522 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1524 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1528 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1529 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1533 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1534 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1538 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1546 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1552 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1558 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1559 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1560 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1566 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1567 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1568 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1573 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1580 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1581 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1585 "hphp.y"
    { (yyval).reset();;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1586 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1590 "hphp.y"
    { (yyval).reset();;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1591 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1594 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1595 "hphp.y"
    { (yyval).reset();;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1598 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { (yyval).reset();;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1604 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1607 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1608 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1610 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1617 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1618 "hphp.y"
    { (yyval).reset();;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1621 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1622 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1623 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1627 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1629 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1630 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1631 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1635 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1636 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1640 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1642 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1643 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1644 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1645 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1652 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1657 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1658 "hphp.y"
    { (yyval).reset();;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1662 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1663 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1667 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1672 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1676 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1680 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1685 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1689 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1690 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1695 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1696 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1697 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1700 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1701 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1702 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1703 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1704 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1708 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1709 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1710 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1713 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1714 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1716 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1717 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1718 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1719 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1721 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1722 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1723 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1724 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1725 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1726 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1727 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1728 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1729 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1730 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1731 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1732 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1735 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1736 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1739 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1744 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1748 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1749 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1751 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1752 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1761 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { (yyval).reset();;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { Token v; Token w;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1867 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1875 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1876 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1886 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1890 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1891 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1941 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1968 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1978 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1982 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1986 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1987 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1991 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1992 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1996 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1997 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 2001 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 2002 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 2006 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 2010 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 2014 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 2018 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 2019 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 2020 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 2021 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 2028 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 2031 "hphp.y"
    { Token t1; _p->onArray(t1,(yyvsp[(1) - (2)]));
                                         Token t2; _p->onArray(t2,(yyvsp[(2) - (2)]));
                                         Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onCallParam((yyvsp[(1) - (2)]),NULL,t1,0);
                                         _p->onCallParam((yyval), &(yyvsp[(1) - (2)]),t2,0);
                                         _p->onCallParam((yyvsp[(1) - (2)]), &(yyvsp[(1) - (2)]),file,0);
                                         _p->onCallParam((yyvsp[(1) - (2)]), &(yyvsp[(1) - (2)]),line,0);
                                         (yyval).setText("");;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 2042 "hphp.y"
    { Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onArray((yyvsp[(4) - (6)]),(yyvsp[(1) - (6)]));
                                         _p->onArray((yyvsp[(5) - (6)]),(yyvsp[(3) - (6)]));
                                         _p->onCallParam((yyvsp[(2) - (6)]),NULL,(yyvsp[(4) - (6)]),0);
                                         _p->onCallParam((yyval), &(yyvsp[(2) - (6)]),(yyvsp[(5) - (6)]),0);
                                         _p->onCallParam((yyvsp[(2) - (6)]), &(yyvsp[(2) - (6)]),file,0);
                                         _p->onCallParam((yyvsp[(2) - (6)]), &(yyvsp[(2) - (6)]),line,0);
                                         (yyval).setText((yyvsp[(6) - (6)]).text());;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 2053 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 2054 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2059 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2060 "hphp.y"
    { (yyval).reset();;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2063 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2064 "hphp.y"
    { (yyval).reset();;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2067 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2071 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2074 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2077 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2084 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2085 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2089 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2091 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2093 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2096 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2097 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2098 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2099 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2100 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2101 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2102 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2103 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2104 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2105 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2106 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2107 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2108 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2109 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2110 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2111 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2112 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2114 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2115 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2116 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2117 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2118 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2120 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2121 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2123 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2124 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2125 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2126 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2127 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2128 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2129 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2132 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2133 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2135 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2136 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2137 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2139 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2140 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2141 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2142 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2146 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2147 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2148 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2149 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2150 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2151 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2153 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2154 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2155 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2156 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2159 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2160 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2161 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2162 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2163 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2164 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2165 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2166 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2167 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2168 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2169 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2170 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2171 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2173 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2174 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2175 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2184 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2185 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2190 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2194 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2195 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2200 "hphp.y"
    { (yyval).reset();;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2201 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { (yyval).reset();;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { (yyval).reset();;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2207 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2213 "hphp.y"
    { (yyval).reset();;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2219 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2222 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2227 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval).reset();;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval).reset();;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval).reset();;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval).reset();;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { (yyval).reset();;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2343 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2344 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2348 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2349 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2355 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { (yyval).reset();;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { (yyval).reset();;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2421 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2459 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2466 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2476 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2503 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2505 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2510 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2511 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2512 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2519 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2520 "hphp.y"
    { (yyval).reset();;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2524 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { (yyval)++;;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2529 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2541 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2545 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2550 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2551 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2553 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2562 "hphp.y"
    { (yyval).reset();;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2567 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2569 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2572 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2574 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2576 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2581 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2582 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2588 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2589 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2594 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2619 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2621 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2627 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2628 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2631 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2632 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2633 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2638 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2639 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2640 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2643 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2644 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2645 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2649 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2650 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2655 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2657 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2671 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2675 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2681 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2682 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2688 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2692 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2698 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2699 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2703 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2706 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2711 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2712 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2713 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2718 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2724 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); ;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2725 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); ;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2727 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); ;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2728 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2745 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2747 "hphp.y"
    {;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2751 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2758 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2761 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2764 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2765 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2771 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2773 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2776 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2779 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2785 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2789 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (5)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2798 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 12348 "hphp.tab.cpp"
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
#line 2801 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

