/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.5"

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

/* Line 268 of yacc.c  */
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

  _p->onExpStatement(out, call);
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


/* Line 268 of yacc.c  */
#line 660 "hphp.tab.cpp"

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
     T_TUPLE = 413,
     T_FROM = 414,
     T_WHERE = 415,
     T_JOIN = 416,
     T_IN = 417,
     T_ON = 418,
     T_EQUALS = 419,
     T_INTO = 420,
     T_LET = 421,
     T_ORDERBY = 422,
     T_ASCENDING = 423,
     T_DESCENDING = 424,
     T_SELECT = 425,
     T_GROUP = 426,
     T_BY = 427,
     T_LAMBDA_OP = 428,
     T_LAMBDA_CP = 429,
     T_UNRESOLVED_OP = 430
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


/* Line 343 of yacc.c  */
#line 890 "hphp.tab.cpp"

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
# if defined YYENABLE_NLS && YYENABLE_NLS
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
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
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
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

# define YYCOPY_NEEDED 1

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

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
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
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   14617

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  205
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  246
/* YYNRULES -- Number of rules.  */
#define YYNRULES  827
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1537

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   430

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    49,   203,     2,   200,    48,    32,   204,
     195,   196,    46,    43,     9,    44,    45,    47,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    27,   197,
      37,    14,    38,    26,    52,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    62,     2,   202,    31,     2,   201,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   198,    30,   199,    51,     2,     2,     2,
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
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    11,    13,    15,    17,
      19,    21,    26,    30,    31,    38,    39,    45,    49,    52,
      54,    56,    58,    60,    62,    64,    66,    68,    70,    72,
      74,    76,    78,    80,    82,    84,    86,    88,    90,    92,
      96,    98,   100,   103,   107,   112,   114,   118,   120,   124,
     127,   129,   132,   135,   141,   146,   149,   150,   152,   154,
     156,   158,   162,   168,   177,   178,   183,   184,   191,   192,
     203,   204,   209,   212,   216,   219,   223,   226,   230,   234,
     238,   242,   246,   252,   254,   256,   257,   267,   273,   274,
     288,   289,   295,   299,   303,   306,   309,   312,   315,   318,
     321,   325,   328,   331,   335,   338,   339,   344,   354,   355,
     356,   361,   364,   365,   367,   368,   370,   371,   381,   382,
     393,   394,   406,   407,   416,   417,   427,   428,   436,   437,
     446,   447,   455,   456,   465,   467,   469,   471,   473,   475,
     478,   481,   484,   485,   488,   489,   492,   493,   495,   499,
     501,   505,   508,   509,   511,   514,   519,   521,   526,   528,
     533,   535,   540,   542,   547,   551,   557,   561,   566,   571,
     577,   583,   588,   589,   591,   593,   598,   599,   605,   606,
     609,   610,   614,   615,   623,   630,   633,   639,   644,   645,
     650,   656,   664,   671,   678,   686,   696,   705,   712,   718,
     721,   726,   730,   731,   735,   740,   747,   753,   759,   766,
     775,   783,   786,   787,   789,   792,   796,   801,   805,   807,
     809,   812,   817,   821,   827,   829,   833,   836,   837,   838,
     843,   844,   850,   853,   854,   865,   866,   878,   882,   886,
     890,   895,   900,   904,   910,   913,   916,   917,   924,   930,
     935,   939,   941,   943,   947,   952,   954,   956,   958,   960,
     965,   967,   971,   974,   975,   978,   979,   981,   985,   987,
     989,   991,   993,   997,  1002,  1007,  1012,  1014,  1016,  1019,
    1022,  1025,  1029,  1033,  1035,  1037,  1039,  1041,  1045,  1047,
    1051,  1053,  1055,  1057,  1058,  1060,  1063,  1065,  1067,  1069,
    1071,  1073,  1075,  1077,  1079,  1080,  1082,  1084,  1086,  1090,
    1096,  1098,  1102,  1108,  1113,  1117,  1121,  1124,  1126,  1128,
    1132,  1136,  1138,  1140,  1141,  1144,  1149,  1153,  1160,  1163,
    1167,  1174,  1176,  1178,  1180,  1187,  1191,  1196,  1203,  1207,
    1211,  1215,  1219,  1223,  1227,  1231,  1235,  1239,  1243,  1247,
    1250,  1253,  1256,  1259,  1263,  1267,  1271,  1275,  1279,  1283,
    1287,  1291,  1295,  1299,  1303,  1307,  1311,  1315,  1319,  1323,
    1326,  1329,  1332,  1335,  1339,  1343,  1347,  1351,  1355,  1359,
    1363,  1367,  1371,  1375,  1381,  1386,  1388,  1391,  1394,  1397,
    1400,  1403,  1406,  1409,  1412,  1415,  1417,  1419,  1421,  1425,
    1428,  1430,  1432,  1434,  1440,  1441,  1442,  1454,  1455,  1468,
    1469,  1473,  1474,  1481,  1484,  1489,  1491,  1497,  1501,  1507,
    1511,  1514,  1515,  1518,  1519,  1524,  1529,  1533,  1538,  1543,
    1548,  1553,  1555,  1557,  1561,  1564,  1568,  1573,  1576,  1580,
    1582,  1585,  1587,  1590,  1592,  1594,  1596,  1598,  1600,  1602,
    1607,  1612,  1615,  1624,  1635,  1638,  1640,  1644,  1646,  1649,
    1651,  1653,  1655,  1657,  1660,  1665,  1669,  1673,  1678,  1680,
    1683,  1688,  1691,  1698,  1699,  1701,  1706,  1707,  1710,  1711,
    1713,  1715,  1719,  1721,  1725,  1727,  1729,  1733,  1737,  1739,
    1741,  1743,  1745,  1747,  1749,  1751,  1753,  1755,  1757,  1759,
    1761,  1763,  1765,  1767,  1769,  1771,  1773,  1775,  1777,  1779,
    1781,  1783,  1785,  1787,  1789,  1791,  1793,  1795,  1797,  1799,
    1801,  1803,  1805,  1807,  1809,  1811,  1813,  1815,  1817,  1819,
    1821,  1823,  1825,  1827,  1829,  1831,  1833,  1835,  1837,  1839,
    1841,  1843,  1845,  1847,  1849,  1851,  1853,  1855,  1857,  1859,
    1861,  1863,  1865,  1867,  1869,  1871,  1873,  1875,  1877,  1879,
    1881,  1883,  1885,  1887,  1889,  1891,  1893,  1895,  1897,  1902,
    1904,  1906,  1908,  1910,  1912,  1914,  1916,  1918,  1921,  1923,
    1924,  1925,  1927,  1929,  1933,  1934,  1936,  1938,  1940,  1942,
    1944,  1946,  1948,  1950,  1952,  1954,  1956,  1958,  1960,  1964,
    1967,  1969,  1971,  1974,  1977,  1982,  1987,  1991,  1996,  1998,
    2000,  2004,  2008,  2012,  2014,  2016,  2018,  2020,  2024,  2028,
    2032,  2035,  2036,  2038,  2039,  2041,  2042,  2048,  2052,  2056,
    2058,  2060,  2062,  2064,  2066,  2070,  2073,  2075,  2077,  2079,
    2081,  2083,  2085,  2088,  2091,  2096,  2101,  2105,  2110,  2113,
    2114,  2120,  2124,  2128,  2130,  2134,  2136,  2139,  2140,  2146,
    2150,  2153,  2154,  2158,  2159,  2164,  2167,  2168,  2172,  2176,
    2178,  2179,  2181,  2184,  2187,  2192,  2196,  2200,  2203,  2208,
    2211,  2216,  2218,  2220,  2222,  2224,  2226,  2229,  2234,  2238,
    2243,  2247,  2249,  2251,  2253,  2255,  2258,  2263,  2268,  2272,
    2274,  2276,  2280,  2288,  2295,  2304,  2314,  2323,  2334,  2342,
    2349,  2358,  2360,  2363,  2368,  2373,  2375,  2377,  2382,  2384,
    2385,  2387,  2390,  2392,  2394,  2397,  2402,  2406,  2410,  2411,
    2413,  2416,  2421,  2425,  2428,  2432,  2439,  2440,  2442,  2447,
    2450,  2451,  2457,  2461,  2465,  2467,  2474,  2479,  2484,  2487,
    2490,  2491,  2497,  2501,  2505,  2507,  2510,  2511,  2517,  2521,
    2525,  2527,  2530,  2533,  2535,  2538,  2540,  2545,  2549,  2553,
    2560,  2564,  2566,  2568,  2570,  2575,  2580,  2585,  2590,  2593,
    2596,  2601,  2604,  2607,  2609,  2613,  2617,  2621,  2622,  2625,
    2631,  2638,  2640,  2643,  2645,  2650,  2654,  2655,  2657,  2661,
    2665,  2667,  2669,  2670,  2671,  2674,  2678,  2680,  2686,  2690,
    2694,  2698,  2700,  2703,  2704,  2709,  2712,  2715,  2717,  2719,
    2721,  2723,  2728,  2735,  2737,  2746,  2752,  2754
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     206,     0,    -1,    -1,   207,   208,    -1,   208,   209,    -1,
      -1,   223,    -1,   239,    -1,   243,    -1,   248,    -1,   437,
      -1,   118,   195,   196,   197,    -1,   144,   215,   197,    -1,
      -1,   144,   215,   198,   210,   208,   199,    -1,    -1,   144,
     198,   211,   208,   199,    -1,   106,   213,   197,    -1,   220,
     197,    -1,    73,    -1,   151,    -1,   152,    -1,   154,    -1,
     156,    -1,   155,    -1,   177,    -1,   179,    -1,   180,    -1,
     182,    -1,   181,    -1,   183,    -1,   184,    -1,   185,    -1,
     186,    -1,   187,    -1,   188,    -1,   189,    -1,   190,    -1,
     191,    -1,   213,     9,   214,    -1,   214,    -1,   215,    -1,
     147,   215,    -1,   215,    92,   212,    -1,   147,   215,    92,
     212,    -1,   212,    -1,   215,   147,   212,    -1,   215,    -1,
     144,   147,   215,    -1,   147,   215,    -1,   216,    -1,   216,
     440,    -1,   216,   440,    -1,   220,     9,   438,    14,   384,
      -1,   101,   438,    14,   384,    -1,   221,   222,    -1,    -1,
     223,    -1,   239,    -1,   243,    -1,   248,    -1,   198,   221,
     199,    -1,    66,   316,   223,   270,   272,    -1,    66,   316,
      27,   221,   271,   273,    69,   197,    -1,    -1,    84,   316,
     224,   264,    -1,    -1,    83,   225,   223,    84,   316,   197,
      -1,    -1,    86,   195,   318,   197,   318,   197,   318,   196,
     226,   262,    -1,    -1,    93,   316,   227,   267,    -1,    97,
     197,    -1,    97,   325,   197,    -1,    99,   197,    -1,    99,
     325,   197,    -1,   102,   197,    -1,   102,   325,   197,    -1,
     148,    97,   197,    -1,   107,   280,   197,    -1,   113,   282,
     197,    -1,    82,   317,   197,    -1,   115,   195,   434,   196,
     197,    -1,   197,    -1,    77,    -1,    -1,    88,   195,   325,
      92,   261,   260,   196,   228,   263,    -1,    90,   195,   266,
     196,   265,    -1,    -1,   103,   231,   104,   195,   377,    75,
     196,   198,   221,   199,   233,   229,   236,    -1,    -1,   103,
     231,   162,   230,   234,    -1,   105,   325,   197,    -1,    98,
     212,   197,    -1,   325,   197,    -1,   319,   197,    -1,   320,
     197,    -1,   321,   197,    -1,   322,   197,    -1,   323,   197,
      -1,   102,   322,   197,    -1,   324,   197,    -1,   347,   197,
      -1,   102,   346,   197,    -1,   212,    27,    -1,    -1,   198,
     232,   221,   199,    -1,   233,   104,   195,   377,    75,   196,
     198,   221,   199,    -1,    -1,    -1,   198,   235,   221,   199,
      -1,   162,   234,    -1,    -1,    32,    -1,    -1,   100,    -1,
      -1,   238,   237,   439,   240,   195,   276,   196,   443,   305,
      -1,    -1,   309,   238,   237,   439,   241,   195,   276,   196,
     443,   305,    -1,    -1,   404,   308,   238,   237,   439,   242,
     195,   276,   196,   443,   305,    -1,    -1,   254,   251,   244,
     255,   256,   198,   283,   199,    -1,    -1,   404,   254,   251,
     245,   255,   256,   198,   283,   199,    -1,    -1,   120,   252,
     246,   257,   198,   283,   199,    -1,    -1,   404,   120,   252,
     247,   257,   198,   283,   199,    -1,    -1,   157,   253,   249,
     256,   198,   283,   199,    -1,    -1,   404,   157,   253,   250,
     256,   198,   283,   199,    -1,   439,    -1,   149,    -1,   439,
      -1,   439,    -1,   119,    -1,   112,   119,    -1,   111,   119,
      -1,   121,   377,    -1,    -1,   122,   258,    -1,    -1,   121,
     258,    -1,    -1,   377,    -1,   258,     9,   377,    -1,   377,
      -1,   259,     9,   377,    -1,   124,   261,    -1,    -1,   411,
      -1,    32,   411,    -1,   125,   195,   423,   196,    -1,   223,
      -1,    27,   221,    87,   197,    -1,   223,    -1,    27,   221,
      89,   197,    -1,   223,    -1,    27,   221,    85,   197,    -1,
     223,    -1,    27,   221,    91,   197,    -1,   212,    14,   384,
      -1,   266,     9,   212,    14,   384,    -1,   198,   268,   199,
      -1,   198,   197,   268,   199,    -1,    27,   268,    94,   197,
      -1,    27,   197,   268,    94,   197,    -1,   268,    95,   325,
     269,   221,    -1,   268,    96,   269,   221,    -1,    -1,    27,
      -1,   197,    -1,   270,    67,   316,   223,    -1,    -1,   271,
      67,   316,    27,   221,    -1,    -1,    68,   223,    -1,    -1,
      68,    27,   221,    -1,    -1,   275,     9,   405,   311,   450,
     158,    75,    -1,   275,     9,   405,   311,   450,   158,    -1,
     275,   389,    -1,   405,   311,   450,   158,    75,    -1,   405,
     311,   450,   158,    -1,    -1,   405,   311,   450,    75,    -1,
     405,   311,   450,    32,    75,    -1,   405,   311,   450,    32,
      75,    14,   384,    -1,   405,   311,   450,    75,    14,   384,
      -1,   275,     9,   405,   311,   450,    75,    -1,   275,     9,
     405,   311,   450,    32,    75,    -1,   275,     9,   405,   311,
     450,    32,    75,    14,   384,    -1,   275,     9,   405,   311,
     450,    75,    14,   384,    -1,   277,     9,   405,   450,   158,
      75,    -1,   277,     9,   405,   450,   158,    -1,   277,   389,
      -1,   405,   450,   158,    75,    -1,   405,   450,   158,    -1,
      -1,   405,   450,    75,    -1,   405,   450,    32,    75,    -1,
     405,   450,    32,    75,    14,   384,    -1,   405,   450,    75,
      14,   384,    -1,   277,     9,   405,   450,    75,    -1,   277,
       9,   405,   450,    32,    75,    -1,   277,     9,   405,   450,
      32,    75,    14,   384,    -1,   277,     9,   405,   450,    75,
      14,   384,    -1,   279,   389,    -1,    -1,   325,    -1,    32,
     411,    -1,   279,     9,   325,    -1,   279,     9,    32,   411,
      -1,   280,     9,   281,    -1,   281,    -1,    75,    -1,   200,
     411,    -1,   200,   198,   325,   199,    -1,   282,     9,    75,
      -1,   282,     9,    75,    14,   384,    -1,    75,    -1,    75,
      14,   384,    -1,   283,   284,    -1,    -1,    -1,   307,   285,
     313,   197,    -1,    -1,   309,   449,   286,   313,   197,    -1,
     314,   197,    -1,    -1,   308,   238,   237,   439,   195,   287,
     274,   196,   443,   306,    -1,    -1,   404,   308,   238,   237,
     439,   195,   288,   274,   196,   443,   306,    -1,   151,   293,
     197,    -1,   152,   299,   197,    -1,   154,   301,   197,    -1,
       4,   121,   377,   197,    -1,     4,   122,   377,   197,    -1,
     106,   259,   197,    -1,   106,   259,   198,   289,   199,    -1,
     289,   290,    -1,   289,   291,    -1,    -1,   219,   143,   212,
     159,   259,   197,    -1,   292,    92,   308,   212,   197,    -1,
     292,    92,   309,   197,    -1,   219,   143,   212,    -1,   212,
      -1,   294,    -1,   293,     9,   294,    -1,   295,   374,   297,
     298,    -1,   149,    -1,   126,    -1,   377,    -1,   114,    -1,
     155,   198,   296,   199,    -1,   383,    -1,   296,     9,   383,
      -1,    14,   384,    -1,    -1,    52,   156,    -1,    -1,   300,
      -1,   299,     9,   300,    -1,   153,    -1,   302,    -1,   212,
      -1,   117,    -1,   195,   303,   196,    -1,   195,   303,   196,
      46,    -1,   195,   303,   196,    26,    -1,   195,   303,   196,
      43,    -1,   302,    -1,   304,    -1,   304,    46,    -1,   304,
      26,    -1,   304,    43,    -1,   303,     9,   303,    -1,   303,
      30,   303,    -1,   212,    -1,   149,    -1,   153,    -1,   197,
      -1,   198,   221,   199,    -1,   197,    -1,   198,   221,   199,
      -1,   309,    -1,   114,    -1,   309,    -1,    -1,   310,    -1,
     309,   310,    -1,   108,    -1,   109,    -1,   110,    -1,   113,
      -1,   112,    -1,   111,    -1,   176,    -1,   312,    -1,    -1,
     108,    -1,   109,    -1,   110,    -1,   313,     9,    75,    -1,
     313,     9,    75,    14,   384,    -1,    75,    -1,    75,    14,
     384,    -1,   314,     9,   438,    14,   384,    -1,   101,   438,
      14,   384,    -1,   195,   315,   196,    -1,    64,   379,   382,
      -1,    63,   325,    -1,   366,    -1,   342,    -1,   195,   325,
     196,    -1,   317,     9,   325,    -1,   325,    -1,   317,    -1,
      -1,   148,   325,    -1,   148,   325,   124,   325,    -1,   411,
      14,   319,    -1,   125,   195,   423,   196,    14,   319,    -1,
     175,   325,    -1,   411,    14,   322,    -1,   125,   195,   423,
     196,    14,   322,    -1,   326,    -1,   411,    -1,   315,    -1,
     125,   195,   423,   196,    14,   325,    -1,   411,    14,   325,
      -1,   411,    14,    32,   411,    -1,   411,    14,    32,    64,
     379,   382,    -1,   411,    25,   325,    -1,   411,    24,   325,
      -1,   411,    23,   325,    -1,   411,    22,   325,    -1,   411,
      21,   325,    -1,   411,    20,   325,    -1,   411,    19,   325,
      -1,   411,    18,   325,    -1,   411,    17,   325,    -1,   411,
      16,   325,    -1,   411,    15,   325,    -1,   411,    61,    -1,
      61,   411,    -1,   411,    60,    -1,    60,   411,    -1,   325,
      28,   325,    -1,   325,    29,   325,    -1,   325,    10,   325,
      -1,   325,    12,   325,    -1,   325,    11,   325,    -1,   325,
      30,   325,    -1,   325,    32,   325,    -1,   325,    31,   325,
      -1,   325,    45,   325,    -1,   325,    43,   325,    -1,   325,
      44,   325,    -1,   325,    46,   325,    -1,   325,    47,   325,
      -1,   325,    48,   325,    -1,   325,    42,   325,    -1,   325,
      41,   325,    -1,    43,   325,    -1,    44,   325,    -1,    49,
     325,    -1,    51,   325,    -1,   325,    34,   325,    -1,   325,
      33,   325,    -1,   325,    36,   325,    -1,   325,    35,   325,
      -1,   325,    37,   325,    -1,   325,    40,   325,    -1,   325,
      38,   325,    -1,   325,    39,   325,    -1,   325,    50,   379,
      -1,   195,   326,   196,    -1,   325,    26,   325,    27,   325,
      -1,   325,    26,    27,   325,    -1,   433,    -1,    59,   325,
      -1,    58,   325,    -1,    57,   325,    -1,    56,   325,    -1,
      55,   325,    -1,    54,   325,    -1,    53,   325,    -1,    65,
     380,    -1,    52,   325,    -1,   386,    -1,   341,    -1,   340,
      -1,   201,   381,   201,    -1,    13,   325,    -1,   328,    -1,
     331,    -1,   344,    -1,   106,   195,   365,   389,   196,    -1,
      -1,    -1,   238,   237,   195,   329,   276,   196,   443,   327,
     198,   221,   199,    -1,    -1,   309,   238,   237,   195,   330,
     276,   196,   443,   327,   198,   221,   199,    -1,    -1,    75,
     332,   334,    -1,    -1,   192,   333,   276,   193,   443,   334,
      -1,     8,   325,    -1,     8,   198,   221,   199,    -1,    81,
      -1,   336,     9,   335,   124,   325,    -1,   335,   124,   325,
      -1,   337,     9,   335,   124,   384,    -1,   335,   124,   384,
      -1,   336,   388,    -1,    -1,   337,   388,    -1,    -1,   169,
     195,   338,   196,    -1,   126,   195,   424,   196,    -1,    62,
     424,   202,    -1,   377,   198,   426,   199,    -1,   377,   198,
     428,   199,    -1,   344,    62,   419,   202,    -1,   345,    62,
     419,   202,    -1,   341,    -1,   435,    -1,   195,   326,   196,
      -1,   348,   349,    -1,   411,    14,   346,    -1,   178,    75,
     181,   325,    -1,   350,   361,    -1,   350,   361,   364,    -1,
     361,    -1,   361,   364,    -1,   351,    -1,   350,   351,    -1,
     352,    -1,   353,    -1,   354,    -1,   355,    -1,   356,    -1,
     357,    -1,   178,    75,   181,   325,    -1,   185,    75,    14,
     325,    -1,   179,   325,    -1,   180,    75,   181,   325,   182,
     325,   183,   325,    -1,   180,    75,   181,   325,   182,   325,
     183,   325,   184,    75,    -1,   186,   358,    -1,   359,    -1,
     358,     9,   359,    -1,   325,    -1,   325,   360,    -1,   187,
      -1,   188,    -1,   362,    -1,   363,    -1,   189,   325,    -1,
     190,   325,   191,   325,    -1,   184,    75,   349,    -1,   365,
       9,    75,    -1,   365,     9,    32,    75,    -1,    75,    -1,
      32,    75,    -1,   163,   149,   367,   164,    -1,   369,    47,
      -1,   369,   164,   370,   163,    47,   368,    -1,    -1,   149,
      -1,   369,   371,    14,   372,    -1,    -1,   370,   373,    -1,
      -1,   149,    -1,   150,    -1,   198,   325,   199,    -1,   150,
      -1,   198,   325,   199,    -1,   366,    -1,   375,    -1,   374,
      27,   375,    -1,   374,    44,   375,    -1,   212,    -1,    65,
      -1,   100,    -1,   101,    -1,   102,    -1,   148,    -1,   175,
      -1,   103,    -1,   104,    -1,   162,    -1,   105,    -1,    66,
      -1,    67,    -1,    69,    -1,    68,    -1,    84,    -1,    85,
      -1,    83,    -1,    86,    -1,    87,    -1,    88,    -1,    89,
      -1,    90,    -1,    91,    -1,    50,    -1,    92,    -1,    93,
      -1,    94,    -1,    95,    -1,    96,    -1,    97,    -1,    99,
      -1,    98,    -1,    82,    -1,    13,    -1,   119,    -1,   120,
      -1,   121,    -1,   122,    -1,    64,    -1,    63,    -1,   114,
      -1,     5,    -1,     7,    -1,     6,    -1,     4,    -1,     3,
      -1,   144,    -1,   106,    -1,   107,    -1,   116,    -1,   117,
      -1,   118,    -1,   113,    -1,   112,    -1,   111,    -1,   110,
      -1,   109,    -1,   108,    -1,   176,    -1,   115,    -1,   125,
      -1,   126,    -1,    10,    -1,    12,    -1,    11,    -1,   128,
      -1,   130,    -1,   129,    -1,   131,    -1,   132,    -1,   146,
      -1,   145,    -1,   174,    -1,   157,    -1,   160,    -1,   159,
      -1,   170,    -1,   172,    -1,   169,    -1,   218,   195,   278,
     196,    -1,   219,    -1,   149,    -1,   377,    -1,   113,    -1,
     417,    -1,   377,    -1,   113,    -1,   421,    -1,   195,   196,
      -1,   316,    -1,    -1,    -1,    80,    -1,   430,    -1,   195,
     278,   196,    -1,    -1,    70,    -1,    71,    -1,    72,    -1,
      81,    -1,   131,    -1,   132,    -1,   146,    -1,   128,    -1,
     160,    -1,   129,    -1,   130,    -1,   145,    -1,   174,    -1,
     139,    80,   140,    -1,   139,   140,    -1,   383,    -1,   217,
      -1,    43,   384,    -1,    44,   384,    -1,   126,   195,   387,
     196,    -1,   177,   195,   387,   196,    -1,    62,   387,   202,
      -1,   169,   195,   339,   196,    -1,   385,    -1,   343,    -1,
     219,   143,   212,    -1,   149,   143,   212,    -1,   219,   143,
     119,    -1,   217,    -1,    74,    -1,   435,    -1,   383,    -1,
     203,   430,   203,    -1,   204,   430,   204,    -1,   139,   430,
     140,    -1,   390,   388,    -1,    -1,     9,    -1,    -1,     9,
      -1,    -1,   390,     9,   384,   124,   384,    -1,   390,     9,
     384,    -1,   384,   124,   384,    -1,   384,    -1,    70,    -1,
      71,    -1,    72,    -1,    81,    -1,   139,    80,   140,    -1,
     139,   140,    -1,    70,    -1,    71,    -1,    72,    -1,   212,
      -1,   391,    -1,   212,    -1,    43,   392,    -1,    44,   392,
      -1,   126,   195,   394,   196,    -1,   177,   195,   394,   196,
      -1,    62,   394,   202,    -1,   169,   195,   397,   196,    -1,
     395,   388,    -1,    -1,   395,     9,   393,   124,   393,    -1,
     395,     9,   393,    -1,   393,   124,   393,    -1,   393,    -1,
     396,     9,   393,    -1,   393,    -1,   398,   388,    -1,    -1,
     398,     9,   335,   124,   393,    -1,   335,   124,   393,    -1,
     396,   388,    -1,    -1,   195,   399,   196,    -1,    -1,   401,
       9,   212,   400,    -1,   212,   400,    -1,    -1,   403,   401,
     388,    -1,    42,   402,    41,    -1,   404,    -1,    -1,   407,
      -1,   123,   416,    -1,   123,   212,    -1,   123,   198,   325,
     199,    -1,    62,   419,   202,    -1,   198,   325,   199,    -1,
     412,   408,    -1,   195,   315,   196,   408,    -1,   422,   408,
      -1,   195,   315,   196,   408,    -1,   416,    -1,   376,    -1,
     414,    -1,   415,    -1,   409,    -1,   411,   406,    -1,   195,
     315,   196,   406,    -1,   378,   143,   416,    -1,   413,   195,
     278,   196,    -1,   195,   411,   196,    -1,   376,    -1,   414,
      -1,   415,    -1,   409,    -1,   411,   407,    -1,   195,   315,
     196,   407,    -1,   413,   195,   278,   196,    -1,   195,   411,
     196,    -1,   416,    -1,   409,    -1,   195,   411,   196,    -1,
     411,   123,   212,   440,   195,   278,   196,    -1,   411,   123,
     416,   195,   278,   196,    -1,   411,   123,   198,   325,   199,
     195,   278,   196,    -1,   195,   315,   196,   123,   212,   440,
     195,   278,   196,    -1,   195,   315,   196,   123,   416,   195,
     278,   196,    -1,   195,   315,   196,   123,   198,   325,   199,
     195,   278,   196,    -1,   378,   143,   212,   440,   195,   278,
     196,    -1,   378,   143,   416,   195,   278,   196,    -1,   378,
     143,   198,   325,   199,   195,   278,   196,    -1,   417,    -1,
     420,   417,    -1,   417,    62,   419,   202,    -1,   417,   198,
     325,   199,    -1,   418,    -1,    75,    -1,   200,   198,   325,
     199,    -1,   325,    -1,    -1,   200,    -1,   420,   200,    -1,
     416,    -1,   410,    -1,   421,   406,    -1,   195,   315,   196,
     406,    -1,   378,   143,   416,    -1,   195,   411,   196,    -1,
      -1,   410,    -1,   421,   407,    -1,   195,   315,   196,   407,
      -1,   195,   411,   196,    -1,   423,     9,    -1,   423,     9,
     411,    -1,   423,     9,   125,   195,   423,   196,    -1,    -1,
     411,    -1,   125,   195,   423,   196,    -1,   425,   388,    -1,
      -1,   425,     9,   325,   124,   325,    -1,   425,     9,   325,
      -1,   325,   124,   325,    -1,   325,    -1,   425,     9,   325,
     124,    32,   411,    -1,   425,     9,    32,   411,    -1,   325,
     124,    32,   411,    -1,    32,   411,    -1,   427,   388,    -1,
      -1,   427,     9,   325,   124,   325,    -1,   427,     9,   325,
      -1,   325,   124,   325,    -1,   325,    -1,   429,   388,    -1,
      -1,   429,     9,   384,   124,   384,    -1,   429,     9,   384,
      -1,   384,   124,   384,    -1,   384,    -1,   430,   431,    -1,
     430,    80,    -1,   431,    -1,    80,   431,    -1,    75,    -1,
      75,    62,   432,   202,    -1,    75,   123,   212,    -1,   141,
     325,   199,    -1,   141,    74,    62,   325,   202,   199,    -1,
     142,   411,   199,    -1,   212,    -1,    76,    -1,    75,    -1,
     116,   195,   434,   196,    -1,   117,   195,   411,   196,    -1,
     117,   195,   326,   196,    -1,   117,   195,   315,   196,    -1,
       7,   325,    -1,     6,   325,    -1,     5,   195,   325,   196,
      -1,     4,   325,    -1,     3,   325,    -1,   411,    -1,   434,
       9,   411,    -1,   378,   143,   212,    -1,   378,   143,   119,
      -1,    -1,    92,   449,    -1,   170,   439,    14,   449,   197,
      -1,   172,   439,   436,    14,   449,   197,    -1,   212,    -1,
     449,   212,    -1,   212,    -1,   212,   165,   444,   166,    -1,
     165,   441,   166,    -1,    -1,   449,    -1,   441,     9,   449,
      -1,   441,     9,   158,    -1,   441,    -1,   158,    -1,    -1,
      -1,    27,   449,    -1,   444,     9,   212,    -1,   212,    -1,
     444,     9,   212,    92,   449,    -1,   212,    92,   449,    -1,
      81,   124,   449,    -1,   446,     9,   445,    -1,   445,    -1,
     446,   388,    -1,    -1,   169,   195,   447,   196,    -1,    26,
     449,    -1,    52,   449,    -1,   219,    -1,   126,    -1,   127,
      -1,   448,    -1,   126,   165,   449,   166,    -1,   126,   165,
     449,     9,   449,   166,    -1,   149,    -1,   195,   100,   195,
     442,   196,    27,   449,   196,    -1,   195,   441,     9,   449,
     196,    -1,   449,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   749,   749,   749,   758,   760,   763,   764,   765,   766,
     767,   768,   771,   773,   773,   775,   775,   777,   778,   783,
     784,   785,   786,   787,   788,   789,   790,   791,   792,   793,
     794,   795,   796,   797,   798,   799,   800,   801,   802,   806,
     808,   811,   812,   813,   814,   819,   820,   824,   825,   827,
     830,   836,   843,   850,   854,   860,   862,   865,   866,   867,
     868,   871,   872,   876,   881,   881,   887,   887,   894,   893,
     899,   899,   904,   905,   906,   907,   908,   909,   910,   911,
     912,   913,   914,   915,   916,   919,   917,   924,   932,   926,
     936,   934,   938,   939,   943,   944,   945,   946,   947,   948,
     949,   950,   951,   952,   953,   961,   961,   966,   972,   976,
     976,   984,   985,   989,   990,   994,   999,   998,  1011,  1009,
    1023,  1021,  1037,  1036,  1055,  1053,  1072,  1071,  1080,  1078,
    1090,  1089,  1101,  1099,  1112,  1113,  1117,  1120,  1123,  1124,
    1125,  1128,  1130,  1133,  1134,  1137,  1138,  1141,  1142,  1146,
    1147,  1152,  1153,  1156,  1157,  1158,  1162,  1163,  1167,  1168,
    1172,  1173,  1177,  1178,  1183,  1184,  1189,  1190,  1191,  1192,
    1195,  1198,  1200,  1203,  1204,  1208,  1210,  1213,  1216,  1219,
    1220,  1223,  1224,  1228,  1234,  1241,  1243,  1248,  1254,  1258,
    1262,  1266,  1271,  1276,  1281,  1286,  1292,  1301,  1306,  1312,
    1314,  1318,  1323,  1327,  1330,  1333,  1337,  1341,  1345,  1349,
    1354,  1362,  1364,  1367,  1368,  1369,  1371,  1376,  1377,  1380,
    1381,  1382,  1386,  1387,  1389,  1390,  1394,  1396,  1399,  1399,
    1403,  1402,  1406,  1410,  1408,  1423,  1420,  1433,  1435,  1437,
    1439,  1441,  1443,  1445,  1449,  1450,  1451,  1454,  1460,  1463,
    1469,  1472,  1477,  1479,  1484,  1489,  1493,  1494,  1500,  1501,
    1506,  1507,  1512,  1513,  1517,  1518,  1522,  1524,  1530,  1535,
    1536,  1538,  1542,  1543,  1544,  1545,  1549,  1550,  1551,  1552,
    1553,  1554,  1556,  1561,  1564,  1565,  1569,  1570,  1574,  1575,
    1578,  1579,  1582,  1583,  1586,  1587,  1591,  1592,  1593,  1594,
    1595,  1596,  1597,  1601,  1602,  1605,  1606,  1607,  1610,  1612,
    1614,  1615,  1618,  1620,  1624,  1625,  1627,  1628,  1629,  1632,
    1636,  1637,  1641,  1642,  1646,  1647,  1651,  1655,  1660,  1664,
    1668,  1673,  1674,  1675,  1678,  1680,  1681,  1682,  1685,  1686,
    1687,  1688,  1689,  1690,  1691,  1692,  1693,  1694,  1695,  1696,
    1697,  1698,  1699,  1700,  1701,  1702,  1703,  1704,  1705,  1706,
    1707,  1708,  1709,  1710,  1711,  1712,  1713,  1714,  1715,  1716,
    1717,  1718,  1719,  1720,  1721,  1722,  1723,  1724,  1725,  1727,
    1728,  1730,  1732,  1733,  1734,  1735,  1736,  1737,  1738,  1739,
    1740,  1741,  1742,  1743,  1744,  1745,  1746,  1747,  1748,  1749,
    1750,  1751,  1752,  1756,  1760,  1765,  1764,  1779,  1777,  1794,
    1794,  1809,  1809,  1827,  1828,  1833,  1838,  1842,  1848,  1852,
    1858,  1860,  1864,  1866,  1870,  1874,  1875,  1879,  1886,  1893,
    1895,  1900,  1901,  1902,  1906,  1910,  1914,  1918,  1920,  1922,
    1924,  1929,  1930,  1935,  1936,  1937,  1938,  1939,  1940,  1944,
    1948,  1952,  1956,  1961,  1966,  1970,  1971,  1975,  1976,  1980,
    1981,  1985,  1986,  1990,  1994,  1998,  2002,  2003,  2004,  2005,
    2009,  2015,  2024,  2037,  2038,  2041,  2044,  2047,  2048,  2051,
    2055,  2058,  2061,  2068,  2069,  2073,  2074,  2076,  2080,  2081,
    2082,  2083,  2084,  2085,  2086,  2087,  2088,  2089,  2090,  2091,
    2092,  2093,  2094,  2095,  2096,  2097,  2098,  2099,  2100,  2101,
    2102,  2103,  2104,  2105,  2106,  2107,  2108,  2109,  2110,  2111,
    2112,  2113,  2114,  2115,  2116,  2117,  2118,  2119,  2120,  2121,
    2122,  2123,  2124,  2125,  2126,  2127,  2128,  2129,  2130,  2131,
    2132,  2133,  2134,  2135,  2136,  2137,  2138,  2139,  2140,  2141,
    2142,  2143,  2144,  2145,  2146,  2147,  2148,  2149,  2150,  2151,
    2152,  2153,  2154,  2155,  2156,  2157,  2158,  2159,  2163,  2168,
    2169,  2172,  2173,  2174,  2178,  2179,  2180,  2184,  2185,  2186,
    2190,  2191,  2192,  2195,  2197,  2201,  2202,  2203,  2204,  2206,
    2207,  2208,  2209,  2210,  2211,  2212,  2213,  2214,  2215,  2218,
    2223,  2224,  2225,  2226,  2227,  2229,  2231,  2232,  2234,  2235,
    2239,  2242,  2245,  2251,  2252,  2253,  2254,  2255,  2256,  2257,
    2262,  2264,  2268,  2269,  2272,  2273,  2277,  2280,  2282,  2284,
    2288,  2289,  2290,  2291,  2293,  2296,  2300,  2301,  2302,  2303,
    2306,  2307,  2308,  2309,  2310,  2312,  2314,  2315,  2320,  2322,
    2325,  2328,  2330,  2332,  2335,  2337,  2341,  2343,  2346,  2349,
    2355,  2357,  2360,  2361,  2366,  2369,  2373,  2373,  2378,  2381,
    2382,  2386,  2387,  2392,  2393,  2397,  2398,  2402,  2403,  2408,
    2410,  2415,  2416,  2417,  2418,  2419,  2420,  2421,  2423,  2426,
    2428,  2432,  2433,  2434,  2435,  2436,  2438,  2440,  2442,  2446,
    2447,  2448,  2452,  2455,  2458,  2461,  2465,  2469,  2476,  2480,
    2484,  2491,  2492,  2497,  2499,  2500,  2503,  2504,  2507,  2508,
    2512,  2513,  2517,  2518,  2519,  2520,  2522,  2525,  2528,  2529,
    2530,  2532,  2534,  2538,  2539,  2540,  2542,  2543,  2544,  2548,
    2550,  2553,  2555,  2556,  2557,  2558,  2561,  2563,  2564,  2568,
    2570,  2573,  2575,  2576,  2577,  2581,  2583,  2586,  2589,  2591,
    2593,  2597,  2598,  2600,  2601,  2607,  2608,  2610,  2612,  2614,
    2616,  2619,  2620,  2621,  2625,  2626,  2627,  2628,  2629,  2630,
    2631,  2632,  2633,  2637,  2638,  2642,  2644,  2652,  2654,  2658,
    2662,  2669,  2670,  2676,  2677,  2684,  2687,  2691,  2694,  2699,
    2700,  2701,  2702,  2706,  2707,  2711,  2713,  2714,  2716,  2720,
    2726,  2728,  2732,  2735,  2738,  2746,  2749,  2752,  2753,  2756,
    2759,  2760,  2763,  2767,  2771,  2777,  2785,  2786
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
  "T_COMPILER_HALT_OFFSET", "T_AWAIT", "T_ASYNC", "T_TUPLE", "T_FROM",
  "T_WHERE", "T_JOIN", "T_IN", "T_ON", "T_EQUALS", "T_INTO", "T_LET",
  "T_ORDERBY", "T_ASCENDING", "T_DESCENDING", "T_SELECT", "T_GROUP",
  "T_BY", "T_LAMBDA_OP", "T_LAMBDA_CP", "T_UNRESOLVED_OP", "'('", "')'",
  "';'", "'{'", "'}'", "'$'", "'`'", "']'", "'\"'", "'\\''", "$accept",
  "start", "$@1", "top_statement_list", "top_statement", "$@2", "$@3",
  "ident", "use_declarations", "use_declaration", "namespace_name",
  "namespace_string_base", "namespace_string", "namespace_string_typeargs",
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
     426,   427,   428,   429,   430,    40,    41,    59,   123,   125,
      36,    96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   205,   207,   206,   208,   208,   209,   209,   209,   209,
     209,   209,   209,   210,   209,   211,   209,   209,   209,   212,
     212,   212,   212,   212,   212,   212,   212,   212,   212,   212,
     212,   212,   212,   212,   212,   212,   212,   212,   212,   213,
     213,   214,   214,   214,   214,   215,   215,   216,   216,   216,
     217,   218,   219,   220,   220,   221,   221,   222,   222,   222,
     222,   223,   223,   223,   224,   223,   225,   223,   226,   223,
     227,   223,   223,   223,   223,   223,   223,   223,   223,   223,
     223,   223,   223,   223,   223,   228,   223,   223,   229,   223,
     230,   223,   223,   223,   223,   223,   223,   223,   223,   223,
     223,   223,   223,   223,   223,   232,   231,   233,   233,   235,
     234,   236,   236,   237,   237,   238,   240,   239,   241,   239,
     242,   239,   244,   243,   245,   243,   246,   243,   247,   243,
     249,   248,   250,   248,   251,   251,   252,   253,   254,   254,
     254,   255,   255,   256,   256,   257,   257,   258,   258,   259,
     259,   260,   260,   261,   261,   261,   262,   262,   263,   263,
     264,   264,   265,   265,   266,   266,   267,   267,   267,   267,
     268,   268,   268,   269,   269,   270,   270,   271,   271,   272,
     272,   273,   273,   274,   274,   274,   274,   274,   274,   275,
     275,   275,   275,   275,   275,   275,   275,   276,   276,   276,
     276,   276,   276,   277,   277,   277,   277,   277,   277,   277,
     277,   278,   278,   279,   279,   279,   279,   280,   280,   281,
     281,   281,   282,   282,   282,   282,   283,   283,   285,   284,
     286,   284,   284,   287,   284,   288,   284,   284,   284,   284,
     284,   284,   284,   284,   289,   289,   289,   290,   291,   291,
     292,   292,   293,   293,   294,   294,   295,   295,   295,   295,
     296,   296,   297,   297,   298,   298,   299,   299,   300,   301,
     301,   301,   302,   302,   302,   302,   303,   303,   303,   303,
     303,   303,   303,   304,   304,   304,   305,   305,   306,   306,
     307,   307,   308,   308,   309,   309,   310,   310,   310,   310,
     310,   310,   310,   311,   311,   312,   312,   312,   313,   313,
     313,   313,   314,   314,   315,   315,   315,   315,   315,   316,
     317,   317,   318,   318,   319,   319,   320,   321,   322,   323,
     324,   325,   325,   325,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   327,   327,   329,   328,   330,   328,   332,
     331,   333,   331,   334,   334,   335,   336,   336,   337,   337,
     338,   338,   339,   339,   340,   341,   341,   342,   343,   344,
     344,   345,   345,   345,   346,   347,   348,   349,   349,   349,
     349,   350,   350,   351,   351,   351,   351,   351,   351,   352,
     353,   354,   355,   356,   357,   358,   358,   359,   359,   360,
     360,   361,   361,   362,   363,   364,   365,   365,   365,   365,
     366,   367,   367,   368,   368,   369,   369,   370,   370,   371,
     372,   372,   373,   373,   373,   374,   374,   374,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   376,   377,
     377,   378,   378,   378,   379,   379,   379,   380,   380,   380,
     381,   381,   381,   382,   382,   383,   383,   383,   383,   383,
     383,   383,   383,   383,   383,   383,   383,   383,   383,   383,
     384,   384,   384,   384,   384,   384,   384,   384,   384,   384,
     385,   385,   385,   386,   386,   386,   386,   386,   386,   386,
     387,   387,   388,   388,   389,   389,   390,   390,   390,   390,
     391,   391,   391,   391,   391,   391,   392,   392,   392,   392,
     393,   393,   393,   393,   393,   393,   393,   393,   394,   394,
     395,   395,   395,   395,   396,   396,   397,   397,   398,   398,
     399,   399,   400,   400,   401,   401,   403,   402,   404,   405,
     405,   406,   406,   407,   407,   408,   408,   409,   409,   410,
     410,   411,   411,   411,   411,   411,   411,   411,   411,   411,
     411,   412,   412,   412,   412,   412,   412,   412,   412,   413,
     413,   413,   414,   414,   414,   414,   414,   414,   415,   415,
     415,   416,   416,   417,   417,   417,   418,   418,   419,   419,
     420,   420,   421,   421,   421,   421,   421,   421,   422,   422,
     422,   422,   422,   423,   423,   423,   423,   423,   423,   424,
     424,   425,   425,   425,   425,   425,   425,   425,   425,   426,
     426,   427,   427,   427,   427,   428,   428,   429,   429,   429,
     429,   430,   430,   430,   430,   431,   431,   431,   431,   431,
     431,   432,   432,   432,   433,   433,   433,   433,   433,   433,
     433,   433,   433,   434,   434,   435,   435,   436,   436,   437,
     437,   438,   438,   439,   439,   440,   440,   441,   441,   442,
     442,   442,   442,   443,   443,   444,   444,   444,   444,   445,
     446,   446,   447,   447,   448,   449,   449,   449,   449,   449,
     449,   449,   449,   449,   449,   449,   450,   450
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     4,     3,     0,     6,     0,     5,     3,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     1,     2,     3,     4,     1,     3,     1,     3,     2,
       1,     2,     2,     5,     4,     2,     0,     1,     1,     1,
       1,     3,     5,     8,     0,     4,     0,     6,     0,    10,
       0,     4,     2,     3,     2,     3,     2,     3,     3,     3,
       3,     3,     5,     1,     1,     0,     9,     5,     0,    13,
       0,     5,     3,     3,     2,     2,     2,     2,     2,     2,
       3,     2,     2,     3,     2,     0,     4,     9,     0,     0,
       4,     2,     0,     1,     0,     1,     0,     9,     0,    10,
       0,    11,     0,     8,     0,     9,     0,     7,     0,     8,
       0,     7,     0,     8,     1,     1,     1,     1,     1,     2,
       2,     2,     0,     2,     0,     2,     0,     1,     3,     1,
       3,     2,     0,     1,     2,     4,     1,     4,     1,     4,
       1,     4,     1,     4,     3,     5,     3,     4,     4,     5,
       5,     4,     0,     1,     1,     4,     0,     5,     0,     2,
       0,     3,     0,     7,     6,     2,     5,     4,     0,     4,
       5,     7,     6,     6,     7,     9,     8,     6,     5,     2,
       4,     3,     0,     3,     4,     6,     5,     5,     6,     8,
       7,     2,     0,     1,     2,     3,     4,     3,     1,     1,
       2,     4,     3,     5,     1,     3,     2,     0,     0,     4,
       0,     5,     2,     0,    10,     0,    11,     3,     3,     3,
       4,     4,     3,     5,     2,     2,     0,     6,     5,     4,
       3,     1,     1,     3,     4,     1,     1,     1,     1,     4,
       1,     3,     2,     0,     2,     0,     1,     3,     1,     1,
       1,     1,     3,     4,     4,     4,     1,     1,     2,     2,
       2,     3,     3,     1,     1,     1,     1,     3,     1,     3,
       1,     1,     1,     0,     1,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     0,     1,     1,     1,     3,     5,
       1,     3,     5,     4,     3,     3,     2,     1,     1,     3,
       3,     1,     1,     0,     2,     4,     3,     6,     2,     3,
       6,     1,     1,     1,     6,     3,     4,     6,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     5,     4,     1,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     1,     1,     1,     3,     2,
       1,     1,     1,     5,     0,     0,    11,     0,    12,     0,
       3,     0,     6,     2,     4,     1,     5,     3,     5,     3,
       2,     0,     2,     0,     4,     4,     3,     4,     4,     4,
       4,     1,     1,     3,     2,     3,     4,     2,     3,     1,
       2,     1,     2,     1,     1,     1,     1,     1,     1,     4,
       4,     2,     8,    10,     2,     1,     3,     1,     2,     1,
       1,     1,     1,     2,     4,     3,     3,     4,     1,     2,
       4,     2,     6,     0,     1,     4,     0,     2,     0,     1,
       1,     3,     1,     3,     1,     1,     3,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     4,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     1,     0,
       0,     1,     1,     3,     0,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     2,
       1,     1,     2,     2,     4,     4,     3,     4,     1,     1,
       3,     3,     3,     1,     1,     1,     1,     3,     3,     3,
       2,     0,     1,     0,     1,     0,     5,     3,     3,     1,
       1,     1,     1,     1,     3,     2,     1,     1,     1,     1,
       1,     1,     2,     2,     4,     4,     3,     4,     2,     0,
       5,     3,     3,     1,     3,     1,     2,     0,     5,     3,
       2,     0,     3,     0,     4,     2,     0,     3,     3,     1,
       0,     1,     2,     2,     4,     3,     3,     2,     4,     2,
       4,     1,     1,     1,     1,     1,     2,     4,     3,     4,
       3,     1,     1,     1,     1,     2,     4,     4,     3,     1,
       1,     3,     7,     6,     8,     9,     8,    10,     7,     6,
       8,     1,     2,     4,     4,     1,     1,     4,     1,     0,
       1,     2,     1,     1,     2,     4,     3,     3,     0,     1,
       2,     4,     3,     2,     3,     6,     0,     1,     4,     2,
       0,     5,     3,     3,     1,     6,     4,     4,     2,     2,
       0,     5,     3,     3,     1,     2,     0,     5,     3,     3,
       1,     2,     2,     1,     2,     1,     4,     3,     3,     6,
       3,     1,     1,     1,     4,     4,     4,     4,     2,     2,
       4,     2,     2,     1,     3,     3,     3,     0,     2,     5,
       6,     1,     2,     1,     4,     3,     0,     1,     3,     3,
       1,     1,     0,     0,     2,     3,     1,     5,     3,     3,
       3,     1,     2,     0,     4,     2,     2,     1,     1,     1,
       1,     4,     6,     1,     8,     5,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   666,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   740,     0,   728,   579,
       0,   585,   586,   587,    19,   614,   716,    84,   588,     0,
      66,     0,     0,     0,     0,     0,     0,     0,     0,   115,
       0,     0,     0,     0,     0,     0,   296,   297,   298,   301,
     300,   299,     0,     0,     0,     0,   138,     0,     0,     0,
     592,   594,   595,   589,   590,     0,     0,   596,   591,     0,
       0,   570,    20,    21,    22,    24,    23,     0,   593,     0,
       0,     0,     0,   597,     0,   302,    25,    26,    27,    29,
      28,    30,    31,    32,    33,    34,    35,    36,    37,    38,
     411,     0,    83,    56,   720,   580,     0,     0,     4,    45,
      47,    50,   613,     0,   569,     0,     6,   114,     7,     8,
       9,     0,     0,   294,   333,     0,     0,     0,     0,     0,
       0,     0,   331,   400,   401,   397,   396,   318,   402,     0,
       0,   317,   682,   571,     0,   616,   395,   293,   685,   332,
       0,     0,   683,   684,   681,   711,   715,     0,   385,   615,
      10,   301,   300,   299,     0,     0,    45,   114,     0,   782,
     332,   781,     0,   779,   778,   399,     0,     0,   369,   370,
     371,   372,   394,   392,   391,   390,   389,   388,   387,   386,
     716,   572,     0,   796,   571,     0,   352,   350,     0,   744,
       0,   623,   316,   575,     0,   796,   574,     0,   584,   723,
     722,   576,     0,     0,   578,   393,     0,     0,     0,     0,
     321,     0,    64,   323,     0,     0,    70,    72,     0,     0,
      74,     0,     0,     0,   818,   819,   823,     0,     0,    45,
     817,     0,   820,     0,     0,    76,     0,     0,     0,     0,
     105,     0,     0,     0,     0,    40,    41,   219,     0,     0,
     218,   140,   139,   224,     0,     0,     0,     0,     0,   793,
     126,   136,   736,   740,   765,     0,   599,     0,     0,     0,
     763,     0,    15,     0,    49,     0,   324,   130,   137,   476,
     421,     0,   787,   328,   670,   333,     0,   331,   332,     0,
       0,   581,     0,   582,     0,     0,     0,   104,     0,     0,
      52,   212,     0,    18,   113,     0,   135,   122,   134,   299,
     114,   295,    95,    96,    97,    98,    99,   101,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   728,    94,   719,   719,   102,   750,     0,
       0,     0,     0,     0,   292,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   351,   349,     0,
     686,   671,   719,     0,   677,   212,   719,     0,   721,   712,
     736,     0,   114,     0,     0,   668,   663,   623,     0,     0,
       0,     0,   748,     0,   426,   622,   739,     0,     0,    52,
       0,   212,   315,     0,   724,   671,   679,   577,     0,    56,
     176,     0,   410,     0,    81,     0,     0,   322,     0,     0,
       0,     0,     0,    73,    93,    75,   815,   816,     0,   813,
       0,     0,   797,     0,   792,     0,   100,    77,   103,     0,
       0,     0,     0,     0,     0,     0,   434,     0,   441,   443,
     444,   445,   446,   447,   448,   439,   461,   462,    56,     0,
      90,    92,    42,     0,    17,     0,     0,   220,     0,    79,
       0,     0,    80,   783,     0,     0,   333,   331,   332,     0,
       0,   146,     0,   737,     0,     0,     0,     0,   598,   764,
     614,     0,     0,   762,   619,   761,    48,     5,    12,    13,
      78,     0,   144,     0,     0,   415,     0,   623,     0,     0,
       0,     0,     0,   625,   669,   827,   314,   382,   690,    61,
      55,    57,    58,    59,    60,     0,   398,   617,   618,    46,
       0,     0,     0,   625,   213,     0,   405,   116,   142,     0,
     355,   357,   356,     0,     0,   353,   354,   358,   360,   359,
     374,   373,   376,   375,   377,   379,   380,   378,   368,   367,
     362,   363,   361,   364,   365,   366,   381,   718,     0,     0,
     754,     0,   623,   786,     0,   785,   688,   711,   128,   132,
     124,   114,     0,     0,   326,   329,   335,   435,   348,   347,
     346,   345,   344,   343,   342,   341,   340,   339,   338,     0,
     673,   672,     0,     0,     0,     0,     0,     0,     0,   780,
     661,   665,   622,   667,     0,     0,   796,     0,   743,     0,
     742,     0,   727,   726,     0,     0,   673,   672,   319,   178,
     180,    56,   413,   320,     0,    56,   160,    65,   323,     0,
       0,     0,     0,   172,   172,    71,     0,     0,   811,   623,
       0,   802,     0,     0,     0,   621,     0,     0,   570,     0,
      25,    50,   601,   569,   609,     0,   600,    54,   608,     0,
       0,   451,     0,     0,   457,   454,   455,   463,     0,   442,
     437,     0,   440,     0,     0,     0,     0,    39,    43,     0,
     217,   225,   222,     0,     0,   774,   777,   776,   775,    11,
     806,     0,     0,     0,   736,   733,     0,   425,   773,   772,
     771,     0,   767,     0,   768,   770,     0,     5,   325,     0,
       0,   470,   471,   479,   478,     0,     0,   622,   420,   424,
       0,   788,     0,   803,   670,   199,   826,     0,     0,   687,
     671,   678,   717,     0,   795,   214,   568,   624,   211,     0,
     670,     0,     0,   144,   407,   118,   384,     0,   429,   430,
       0,   427,   622,   749,     0,     0,   212,   146,   144,   142,
       0,   728,   336,     0,     0,   212,   675,   676,   689,   713,
     714,     0,     0,     0,   649,   630,   631,   632,   633,     0,
       0,     0,    25,   641,   640,   655,   623,     0,   663,   747,
     746,     0,   725,   671,   680,   583,     0,   182,     0,     0,
      62,     0,     0,     0,     0,     0,     0,   152,   153,   164,
       0,    56,   162,    87,   172,     0,   172,     0,     0,   821,
       0,   622,   812,   814,   801,   800,     0,   798,   602,   603,
     629,     0,   623,   621,     0,     0,   423,   621,     0,   756,
     436,     0,     0,     0,   459,   460,   458,     0,     0,   438,
       0,   106,     0,   109,    91,    44,   221,     0,   784,    82,
       0,     0,   794,   145,   147,   227,     0,     0,   734,     0,
     766,     0,    16,     0,   143,   227,     0,     0,   417,     0,
     789,     0,     0,     0,   827,     0,   203,   201,     0,   673,
     672,   798,     0,   215,    53,     0,   670,   141,     0,   670,
       0,   383,   753,   752,     0,   212,     0,     0,     0,   144,
     120,   584,   674,   212,     0,     0,   636,   637,   638,   639,
     642,   643,   653,     0,   623,   649,     0,   635,   657,   649,
     622,   660,   662,   664,     0,   741,   674,     0,     0,     0,
       0,   179,   414,    67,     0,   323,   154,   736,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   166,     0,   809,
     810,     0,     0,   825,     0,   606,   622,   620,     0,   611,
       0,   623,     0,     0,   612,   610,   760,     0,   623,   449,
       0,   450,   456,   464,   465,     0,    56,   223,   808,   805,
       0,   293,   738,   736,   327,   330,   334,     0,    14,   293,
     482,     0,     0,   484,   477,   480,     0,   475,     0,   790,
     804,   412,     0,   204,     0,   200,     0,     0,   212,   216,
     803,     0,   227,     0,   670,     0,   212,     0,   709,   227,
     227,     0,     0,   337,   212,     0,   703,     0,   646,   622,
     648,     0,   634,     0,     0,   623,     0,   654,   745,     0,
      56,     0,   175,   161,     0,     0,   151,    85,   165,     0,
       0,   168,     0,   173,   174,    56,   167,   822,   799,     0,
     628,   627,   604,     0,   622,   422,   607,   605,     0,   428,
     622,   755,     0,     0,     0,     0,   148,     0,     0,     0,
     291,     0,     0,     0,   127,   226,   228,     0,   290,     0,
     293,     0,   769,   131,   473,     0,     0,   416,     0,   207,
     198,     0,   206,   674,   212,     0,   404,   803,   293,   803,
       0,   751,     0,   708,   293,   293,   227,   670,     0,   702,
     652,   651,   644,     0,   647,   622,   656,   645,    56,   181,
      63,    68,   155,     0,   163,   169,    56,   171,     0,     0,
     419,     0,   759,   758,     0,    56,   110,   807,     0,     0,
       0,     0,   149,   258,   256,   570,    24,     0,   252,     0,
     257,   268,     0,   266,   271,     0,   270,     0,   269,     0,
     114,   230,     0,   232,     0,   735,   474,   472,   483,   481,
     208,     0,   197,   205,   212,     0,   706,     0,     0,     0,
     123,   404,   803,   710,   129,   133,   293,     0,   704,     0,
     659,     0,   177,     0,    56,   158,    86,   170,   824,   626,
       0,     0,     0,     0,     0,     0,     0,     0,   242,   246,
       0,     0,   237,   534,   533,   530,   532,   531,   551,   553,
     552,   522,   512,   528,   527,   489,   499,   500,   502,   501,
     521,   505,   503,   504,   506,   507,   508,   509,   510,   511,
     513,   514,   515,   516,   517,   518,   520,   519,   490,   491,
     492,   495,   496,   498,   536,   537,   546,   545,   544,   543,
     542,   541,   529,   548,   538,   539,   540,   523,   524,   525,
     526,   549,   550,   554,   556,   555,   557,   558,   535,   560,
     559,   493,   562,   564,   563,   497,   567,   565,   566,   561,
     494,   547,   488,   263,   485,     0,   238,   284,   285,   283,
     276,     0,   277,   239,   310,     0,     0,     0,     0,   114,
       0,   210,     0,   705,     0,    56,   286,    56,   117,     0,
       0,   125,   803,   650,     0,    56,   156,    69,     0,   418,
     757,   452,   108,   240,   241,   313,   150,     0,     0,   260,
     253,     0,     0,     0,   265,   267,     0,     0,   272,   279,
     280,   278,     0,     0,   229,     0,     0,     0,     0,   209,
     707,     0,   468,   625,     0,     0,    56,   119,     0,   658,
       0,     0,     0,    88,   243,    45,     0,   244,   245,     0,
       0,   259,   262,   486,   487,     0,   254,   281,   282,   274,
     275,   273,   311,   308,   233,   231,   312,     0,   469,   624,
       0,   406,   287,     0,   121,     0,   159,   453,     0,   112,
       0,   293,   261,   264,     0,   670,   235,     0,   466,   403,
     408,   157,     0,     0,    89,   250,     0,   292,   309,     0,
     625,   304,   670,   467,     0,   111,     0,     0,   249,   803,
     670,   185,   305,   306,   307,   827,   303,     0,     0,     0,
     248,     0,   304,     0,   803,     0,   247,   288,    56,   234,
     827,     0,   189,   187,     0,    56,     0,     0,   190,     0,
     186,   236,     0,   289,     0,   193,   184,     0,   192,   107,
     194,     0,   183,   191,     0,   196,   195
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   118,   737,   517,   176,   264,   265,
     120,   121,   122,   123,   124,   125,   309,   540,   541,   436,
     231,  1243,   442,  1173,  1459,   705,   261,   478,  1423,   884,
    1016,  1474,   325,   177,   542,   771,   930,  1062,   543,   558,
     789,   501,   787,   544,   522,   788,   327,   280,   297,   131,
     773,   740,   723,   893,  1191,   979,   837,  1377,  1246,   657,
     843,   441,   665,   845,  1095,   650,   827,   830,   969,  1479,
    1480,   532,   533,   552,   553,   269,   270,   274,  1021,  1125,
    1209,  1357,  1465,  1482,  1387,  1427,  1428,  1429,  1197,  1198,
    1199,  1388,  1394,  1436,  1202,  1203,  1207,  1350,  1351,  1352,
    1368,  1509,  1126,  1127,   178,   133,  1495,  1496,  1355,  1129,
     134,   224,   437,   438,   135,   136,   137,   138,   139,   140,
     141,   142,  1228,   143,   770,   929,   144,   228,   304,   432,
     526,   527,  1001,   528,  1002,   145,   146,   147,   684,   148,
     149,   258,   150,   259,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   695,   696,   876,   475,   476,   477,   702,
    1413,   151,   523,  1217,   524,   906,   745,  1037,  1034,  1343,
    1344,   152,   153,   154,   218,   225,   312,   422,   155,   860,
     688,   156,   861,   416,   755,   862,   814,   950,   952,   953,
     954,   816,  1074,  1075,   817,   631,   407,   186,   187,   157,
     535,   390,   391,   761,   158,   219,   180,   160,   161,   162,
     163,   164,   165,   166,   588,   167,   221,   222,   504,   210,
     211,   591,   592,  1007,  1008,   289,   290,   731,   168,   494,
     169,   531,   170,   251,   281,   320,   451,   856,   913,   721,
     668,   669,   670,   252,   253,   757
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1219
static const yytype_int16 yypact[] =
{
   -1219,   161, -1219, -1219,  4161, 11635, 11635,     8, 11635, 11635,
   11635, -1219, 11635, 11635, 11635, 11635, 11635, 11635, 11635, 11635,
   11635, 11635, 11635, 11635, 13475, 13475,  9009, 11635, 13540,    29,
     169, -1219, -1219, -1219, -1219, -1219,   200, -1219, -1219, 11635,
   -1219,   169,   174,   180,   206,   169,  9211, 10263,  9413, -1219,
   13059,  8605,   125, 11635, 14024,    93, -1219, -1219, -1219,   237,
     295,    43,   221,   226,   234,   270, -1219, 10263,   280,   282,
   -1219, -1219, -1219, -1219, -1219,   264, 13822, -1219, -1219, 10263,
    9615, -1219, -1219, -1219, -1219, -1219, -1219, 10263, -1219,   331,
     287, 10263, 10263, -1219, 11635, -1219, -1219, -1219, -1219, -1219,
   -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219,
   -1219, 11635, -1219, -1219,   300,   315,   337,   337, -1219,   467,
     359,   235, -1219,   309, -1219,    36, -1219,   482, -1219, -1219,
   -1219, 14067,   512, -1219, -1219,   319,   350,   352,   360,   367,
     369, 12108, -1219, -1219, -1219, -1219,   492, -1219,   507,   517,
     377, -1219,    94,   384,   441, -1219, -1219,   528,    92,  2322,
      96,   391,   112,   113,   392,   126, -1219,   101, -1219,   529,
   -1219, -1219, -1219,   447,   398,   448, -1219,   482,   512, 14527,
    3470, 14527, 11635, 14527, 14527,  4349,   553, 10263, -1219, -1219,
     547, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219,
   -1219, -1219,  2494,   434, -1219,   457,   479,   479, 13475, 13311,
     401,   595, -1219,   447,  2494,   434,   462,   463,   413,   118,
   -1219,   486,    96,  9817, -1219, -1219, 11635,  3262,   602,    45,
   14527,  8201, -1219, 11635, 11635, 10263, -1219, -1219, 12364,   419,
   -1219, 12405, 13059, 13059,   454, -1219, -1219,   418, 12833,   604,
   -1219,   613, -1219, 10263,   555, -1219,   436, 12446,   438,   591,
   -1219,    35, 12487, 10263,    53, -1219,    49, -1219, 13287,    58,
   -1219, -1219, -1219,   629,    59, 13475, 13475, 11635,   464,   484,
   -1219, -1219, 13337,  9009,    84,   252, -1219, 11837, 13475,   304,
   -1219, 10263, -1219,   -19,   359,   470, 14249, -1219, -1219, -1219,
     563,   636,   567, 14527,    79,   478, 14527,   481,  1865,  4363,
   11635,   351,   474,   348,   351,   296,   283, -1219, 10263, 13059,
     483, 10019, 13059, -1219, -1219,  8849, -1219, -1219, -1219, -1219,
     482, -1219, -1219, -1219, -1219, -1219, -1219, -1219, 11635, 11635,
   11635, 10221, 11635, 11635, 11635, 11635, 11635, 11635, 11635, 11635,
   11635, 11635, 11635, 11635, 11635, 11635, 11635, 11635, 11635, 11635,
   11635, 11635, 11635, 13540, -1219, 11635, 11635, -1219, 11635, 12648,
   10263, 10263, 14067,   579,   543,  8403, 11635, 11635, 11635, 11635,
   11635, 11635, 11635, 11635, 11635, 11635, 11635, -1219, -1219, 13594,
   -1219,   119, 11635, 11635, -1219, 10019, 11635, 11635,   300,   127,
   13337,   485,   482, 10423, 12534, -1219,   487,   674,  2494,   488,
      17, 13635,   479, 10625, -1219, 10827, -1219,   491,    19, -1219,
     145, 10019, -1219, 13685, -1219,   128, -1219, -1219, 12575, -1219,
   -1219, 11029, -1219, 11635, -1219,   606,  7393,   682,   495, 14443,
     681,    44,    14, -1219, -1219, -1219, -1219, -1219, 13059,   615,
     502,   689, -1219, 13239, -1219,   519, -1219, -1219, -1219,   630,
   11635,   631,   632, 11635, 11635, 11635, -1219,   591, -1219, -1219,
   -1219, -1219, -1219, -1219, -1219,   530, -1219, -1219, -1219,   518,
   -1219, -1219,   263, 14024, -1219, 10263, 11635,   479,    93, -1219,
   13239,   637, -1219,   479,    51,    55,   524,   525,  2139,   527,
   10263,   601,   531,   479,    80,   532, 14008, 10263, -1219, -1219,
     667,    66,   -48, -1219, -1219, -1219,   359, -1219, -1219, -1219,
   -1219, 11635,   609,   570,   170, -1219,   617,   728,   546, 13059,
   13059,   730,   552,   738, -1219, 13059,    86,   686,   102, -1219,
   -1219, -1219, -1219, -1219, -1219,  1781, -1219, -1219, -1219, -1219,
      48, 13475,   554,   744, 14527,   741, -1219, -1219,   640,  9051,
   14567,  4149,  4349, 11635, 14486,  4752,  4953,  5154,  5355,  3940,
    5556,  5556,  5556,  5556,  1319,  1319,  1319,  1319,   790,   790,
     473,   473,   473,   547,   547,   547, -1219, 14527,   560,   562,
   14290,   558,   756, -1219, 11635,   -40,   571,   127, -1219, -1219,
   -1219,   482,  3443, 11635, -1219, -1219,  4349, -1219,  4349,  4349,
    4349,  4349,  4349,  4349,  4349,  4349,  4349,  4349,  4349, 11635,
     -40,   583,   572,  2280,   586,   581,  2849,    81,   589, -1219,
    2395, -1219, 10263, -1219,   478,    86,   434, 13475, 14527, 13475,
   14346,    88,   132, -1219,   592, 11635, -1219, -1219, -1219,  7191,
     104, -1219, 14527, 14527,   169, -1219, -1219, -1219, 11635, 13108,
   13239, 10263,  7595,   588,   590, -1219,    56,   668, -1219,   782,
     597, 12913, 13059, 13239, 13239, 13239,   599,   233,   652,   607,
     608,   253, -1219,   653, -1219,   603, -1219, -1219, -1219, 11635,
     616, 14527,   624,   792, 12657,   800, -1219, 14527, 12616, -1219,
     530,   736, -1219,  4565, 13967,   614, 10263, -1219, -1219,  2954,
   -1219, -1219,   799, 13475,   618, -1219, -1219, -1219, -1219, -1219,
     722,   114, 13967,   621, 13337, 13421,   802, -1219, -1219, -1219,
   -1219,   619, -1219, 11635, -1219, -1219,  3754, -1219, 14527, 13967,
     622, -1219, -1219, -1219, -1219,   809, 11635,   563, -1219, -1219,
     633, -1219, 13059,   801,    95, -1219, -1219,   150, 13726, -1219,
     133, -1219, -1219, 13059, -1219,   479, -1219, 11231, -1219, 13239,
      75,   644, 13967,   609, -1219, -1219,  4551, 11635, -1219, -1219,
   11635, -1219, 11635, -1219,  3198,   646, 10019,   601,   609,   640,
   10263, 13540,   479,  3531,   647, 10019, -1219, -1219,   137, -1219,
   -1219,   812,  2880,  2880,  2395, -1219, -1219, -1219, -1219,   650,
     240,   654,   655, -1219, -1219, -1219,   822,   651,   487,   479,
     479, 11433, -1219,   139, -1219, -1219,  3659,   293,   169,  8201,
   -1219,  4767,   657,  4969,   662, 13475,   671,   724,   479, -1219,
     838, -1219, -1219, -1219, -1219,   353, -1219,    64, 13059, -1219,
   13059,   615, -1219, -1219, -1219,   858,   676,   677, -1219, -1219,
     746,   672,   866, 13239,   737, 10263,   563, 13239,  9657, 13239,
   14527, 11635, 11635, 11635, -1219, -1219, -1219, 11635, 11635, -1219,
     591, -1219,   803, -1219, -1219, -1219, -1219, 13239,   479, -1219,
   13059, 10263, -1219,   867, -1219, -1219,    82,   684,   479,  8807,
   -1219,  2207, -1219,  3959,   867, -1219,     7,   199, 14527,   757,
   -1219,   685, 13059,   602, 13059,   808,   870,   810, 11635,   -40,
     691, -1219, 13475, 14527, -1219,   692,    75, -1219,   693,    75,
     694,  4551, 14527, 14387,   695, 10019,   696,   697,   698,   609,
   -1219,   413,   699, 10019,   701, 11635, -1219, -1219, -1219, -1219,
   -1219, -1219,   774,   702,   891,  2395,   767, -1219,   563,  2395,
    2395, -1219, -1219, -1219, 13475, 14527, -1219,   169,   881,   840,
    8201, -1219, -1219, -1219,   713, 11635,   479, 13337, 13108,   715,
   13239,  5171,   444,   716, 11635,    46,    67, -1219,   748, -1219,
   -1219, 12979,   888, -1219, 13239, -1219, 13239, -1219,   721, -1219,
     794,   910,   729,   731, -1219, -1219,   805,   725,   917, 14527,
   12744, 14527, -1219, 14527, -1219,   740, -1219, -1219, -1219,   841,
   13967,   399, -1219, 13337, -1219, -1219,  4349,   735, -1219,   416,
   -1219,   179, 11635, -1219, -1219, -1219, 11635, -1219, 11635, -1219,
   -1219, -1219,   189,   924, 13239, -1219,  3705,   745, 10019,   479,
     801,   749, -1219,   752,    75, 11635, 10019,   754, -1219, -1219,
   -1219,   755,   747, -1219, 10019,   758, -1219,  2395, -1219,  2395,
   -1219,   761, -1219,   828,   762,   950,   764, -1219,   479,   934,
   -1219,   765, -1219, -1219,   771,   110, -1219, -1219, -1219,   766,
     772, -1219,  3145, -1219, -1219, -1219, -1219, -1219, -1219, 13059,
   -1219,   847, -1219, 13239,   563, -1219, -1219, -1219, 13239, -1219,
   13239, -1219, 11635,   777,  5373, 13059, -1219,    22, 13059, 13967,
   -1219, 13919,   819,  8445, -1219, -1219, -1219,   579, 12767,    60,
     543,   111, -1219, -1219,   834, 12023, 12064, 14527,   909,   971,
     912, 13239, -1219,   793, 10019,   796,   884,   801,   972,   801,
     797, 14527,   804, -1219,  1444,  1461, -1219,    75,   806, -1219,
   -1219,   875, -1219,  2395, -1219,   563, -1219, -1219, -1219,  7191,
   -1219, -1219, -1219,  7797, -1219, -1219, -1219,  7191,   807, 13239,
   -1219,   882, -1219,   883, 12703, -1219, -1219, -1219, 13967, 13967,
     995,    41, -1219, -1219, -1219,    62,   813,    63, -1219, 12182,
   -1219, -1219,    65, -1219, -1219, 13870, -1219,   815, -1219,   938,
     482, -1219, 13059, -1219,   579, -1219, -1219, -1219, -1219, -1219,
    1001, 13239, -1219, -1219, 10019,   820, -1219,   823,   821,   245,
   -1219,   884,   801, -1219, -1219, -1219,  1589,   824, -1219,  2395,
   -1219,   893,  7191,  7999, -1219, -1219, -1219,  7191, -1219, -1219,
   13239, 13239, 11635,  5575,   825,   826, 13239, 13967, -1219, -1219,
     850, 13919, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219,
   -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219,
   -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219,
   -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219,
   -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219,
   -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219,
   -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219,
   -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219,
   -1219, -1219, -1219,   504, -1219,   819, -1219, -1219, -1219, -1219,
   -1219,    40,   123, -1219,  1007,    70, 10263,   938,  1012,   482,
   13239, -1219,   831, -1219,   338, -1219, -1219, -1219, -1219,   830,
     245, -1219,   801, -1219,  2395, -1219, -1219, -1219,  5777, -1219,
   -1219, 12171, -1219, -1219, -1219, -1219, -1219, 13776,    33, -1219,
   -1219, 13239, 12182, 12182,   977, -1219, 13870, 13870,   489, -1219,
   -1219, -1219, 13239,   955, -1219,   836,    76, 13239, 10263, -1219,
   -1219,   957, -1219,  1026,  5979,  6181, -1219, -1219,   245, -1219,
    6383,   839,   965,   939, -1219,   952,   903, -1219, -1219,   956,
     850, -1219, -1219, -1219, -1219,   895, -1219,  1017, -1219, -1219,
   -1219, -1219, -1219,  1038, -1219, -1219, -1219,   859, -1219,   422,
     857, -1219, -1219,  6585, -1219,   860, -1219, -1219,   861,   896,
   10263,   543, -1219, -1219, 13239,    87, -1219,   980, -1219, -1219,
   -1219, -1219, 13967,   614, -1219,   902, 10263,   361, -1219,   868,
    1057,   453,    87, -1219,   992, -1219, 13967,   871, -1219,   801,
      89, -1219, -1219, -1219, -1219, 13059, -1219,   873,   876,    77,
   -1219,   336,   453,   306,   801,   877, -1219, -1219, -1219, -1219,
   13059,   996,  1062,  1002,   336, -1219,  6787,   308,  1073, 13239,
   -1219, -1219,  6989, -1219,  1013,  1075,  1015, 13239, -1219, -1219,
    1077, 13239, -1219, -1219, 13239, -1219, -1219
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1219, -1219, -1219,  -484, -1219, -1219, -1219,    -4, -1219,   611,
       5,  1009,  -286, -1219,  1511, -1219,  -305, -1219,     3, -1219,
   -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219,  -381,
   -1219, -1219,  -165,    13,    -1, -1219, -1219, -1219,     0, -1219,
   -1219, -1219, -1219,     2, -1219, -1219,   726,   727,   732,   942,
     307,  -658,   313,   362,  -384, -1219,   129, -1219, -1219, -1219,
   -1219, -1219, -1219,  -628,    12, -1219, -1219, -1219, -1219,  -376,
   -1219,  -742, -1219,  -355, -1219, -1219,   620, -1219,  -867, -1219,
   -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219,  -152,
   -1219, -1219, -1219, -1219, -1219,  -233, -1219,   -10,  -855, -1219,
   -1218,  -399, -1219,  -156,    20,  -130,  -385, -1219,  -239, -1219,
     -67,   -20,  1081,  -623,  -353, -1219, -1219,   -42, -1219, -1219,
    2737,   -31,  -109, -1219, -1219, -1219, -1219, -1219, -1219,   212,
    -713, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219,
   -1219,   753, -1219, -1219,   249, -1219,   663, -1219, -1219, -1219,
   -1219, -1219, -1219, -1219,   254, -1219,   665, -1219, -1219,   433,
   -1219,   229, -1219, -1219, -1219, -1219, -1219, -1219, -1219, -1219,
    -848, -1219,  1309,  1014,  -336, -1219, -1219,   195,  1412,    99,
   -1219, -1219,  -741,  -394,  -548, -1219, -1219,   334,  -607,  -753,
   -1219, -1219, -1219, -1219, -1219,   320, -1219, -1219, -1219,  -211,
    -722,  -182,  -174,  -140, -1219, -1219,    27, -1219, -1219, -1219,
   -1219,   -12,  -141, -1219,     6, -1219, -1219, -1219,  -392,   856,
   -1219, -1219, -1219, -1219, -1219,   456,   357, -1219, -1219,   869,
   -1219, -1219, -1219,  -311,   -73,  -186,  -289, -1219, -1013, -1219,
     291, -1219, -1219, -1219,  -187,  -899
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -797
static const yytype_int16 yytable[] =
{
     119,   373,   331,   128,   129,   768,   130,   126,   627,   256,
     227,   555,   401,   633,   298,  1042,   220,   127,   301,   302,
     394,   232,   604,   815,   132,   236,   399,   586,   925,   419,
     550,   159,   914,   736,   909,   834,   847,  1146,  1029,   424,
     624,   663,  1430,   239,   305,   322,   249,   425,   331,  1396,
    1257,   206,   207,   661,   433,   446,   447,   763,   328,   266,
     713,   452,   483,   279,   713,   848,   644,   488,   491,  1212,
    1397,  -255,  1261,  1093,  1345,   389,   338,   339,   340,  1403,
     307,   293,   426,   279,   294,  1403,  1257,   279,   279,   725,
     725,   725,   341,   534,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   928,   363,    11,   273,   725,
     725,    11,   998,   891,   649,   319,  1003,   279,   318,    11,
     938,    11,   452,   748,  1229,   409,  1231,    11,   308,   479,
     389,   485,   389,  1188,  1189,   330,   506,   417,   392,  1399,
     392,   735,  1417,  1000,  -694,  -796,  -691,  1030,   392,   984,
     985,     3,   984,   985,  -698,   559,  1400,   682,   267,  1401,
    1031,   828,   829,   703,  -692,  -693,   200,   374,   518,   519,
    -729,  -695,   915,   406,  1051,  1148,  -572,  1053,   396,   396,
    -730,   402,  1154,  1155,  -732,  -696,   318,   480,   783,  -697,
    1454,  -731,  1071,   182,   682,  1032,  1076,   507,  -409,   758,
     496,   423,   664,   538,   764,   642,   982,   742,   986,  1370,
     200,  1138,   849,   119,   223,   916,  1134,   119,   597,   410,
     430,   440,  1431,   323,   435,   412,  1398,   628,  1258,  1259,
     662,   418,   434,  1094,   331,  1073,   497,   714,   597,   454,
     484,   715,   557,   903,   159,   489,   492,  1213,   159,  -255,
    1262,   666,  1346,   987,  1139,   734,  1096,  1404,   482,  -573,
     597,  -202,  -202,  1445,  1506,   852,   726,   801,  1022,   597,
     892,  1061,   597,  -188,   393,  -624,   393,  -700,  -624,  1236,
    -694,  -624,  -691,   268,   393,   487,   516,  -701,   298,   328,
    -698,   398,   493,   493,   498,   119,  1172,  1215,   917,   503,
    -692,  -693,  1150,   864,   549,   512,  -729,  -695,   249,   743,
     956,   279,   127,   260,   397,   397,  -730,   284,   299,   132,
    -732,  -696,   896,   605,   744,  -697,   159,  -731,  1511,   284,
    1524,   634,   750,   751,   285,   114,   831,  1140,   756,  1035,
     833,   220,  1084,  1077,   759,   706,   271,   596,   284,  1418,
     967,   968,   760,   513,   226,   595,   279,   279,   279,   233,
    1411,   284,   589,   286,   682,   234,   513,   621,  -796,   284,
     957,  1512,   855,  1525,   513,   620,   601,   682,   682,   682,
     284,  1181,   508,   287,   288,   311,  -796,  1036,   622,   596,
     319,   235,   625,  1117,   286,   287,   288,   636,   643,   785,
     318,   647,   284,  1412,   272,  1237,   275,   314,   319,   646,
    1117,   276,   961,   284,   287,   288,   284,   503,   513,   277,
    -796,   936,   119,  -796,   794,   410,   790,   287,   288,   656,
     944,    11,  1366,  1367,   514,   287,   288,   983,   984,   985,
     785,  -796,  1241,   759,  1467,   941,   287,   288,    11,   822,
    1160,   760,  1161,   159,  1513,   278,  1526,   823,   997,    56,
      57,    58,   171,   172,   329,   282,  1501,   283,   287,   288,
     299,   708,   300,   682,   452,   857,   775,   548,   266,   287,
     288,  1514,   287,   288,   317,   419,   720,  1468,   310,   547,
    1118,   824,   730,   732,   321,  1119,   318,    56,    57,    58,
     171,   172,   329,  1120,   324,  1439,   332,  1118,  1391,   360,
     361,   362,  1119,   363,    56,    57,    58,   171,   172,   329,
    1120,  1392,  1440,  1507,  1508,  1441,   981,    95,  1090,   984,
     985,  1437,  1438,   534,  1433,  1434,  1024,   333,  1393,   334,
    1121,  1122,   687,  1123,  -431,   279,  1240,   335,  1488,   534,
    1070,  1492,  1493,  1494,   336,   911,   337,  1121,  1122,   365,
    1123,   313,   315,   316,   367,    95,   921,   682,   765,   366,
    1057,   682,   368,   682,   369,  1085,   395,  -699,  1065,   711,
    -572,  -432,    95,   400,   405,   291,  1503,   363,  1124,   319,
     411,   682,   389,   414,   415,  -571,   420,  1105,   421,   423,
     431,  1517,    49,   449,  1111,  1133,   444,   597,  -791,   448,
      56,    57,    58,   171,   172,   329,   813,   453,   818,   792,
     455,  1131,  1373,   456,   832,   458,    56,    57,    58,    59,
      60,   329,   509,   490,   525,   119,   515,    66,   370,   500,
     529,    56,    57,    58,   171,   172,   329,   840,   119,   530,
     499,   988,   127,   989,   819,   842,   820,   520,   509,   132,
     515,   509,   515,   515,   536,   546,   159,   537,   -51,    49,
     556,  1166,   630,   632,   635,   371,   838,   641,    95,   159,
     654,   433,   658,  1145,   682,   660,   667,   671,   672,   119,
     689,  1152,   885,  1018,    95,   690,   692,   693,   682,  1158,
     682,  1114,   712,   704,   701,   534,   127,   940,   534,    95,
     716,   717,   722,   132,   719,  1040,   724,   756,   727,   733,
     159,   739,   119,  1047,   741,   128,   129,   747,   130,   126,
     888,   746,   749,  1481,   752,   753,   920,   754,  -433,   127,
     766,   503,   898,   767,   919,   769,   132,   781,   682,   839,
    1481,   772,   778,   159,   779,   782,   786,  1419,  1502,   459,
     460,   461,   858,   859,   796,  1169,   462,   463,   795,   220,
     464,   465,   798,   799,   774,   844,   279,   846,   825,  1225,
    1177,   851,   850,   853,   863,   865,   868,   871,   949,   949,
     813,   869,   866,   867,   921,   872,   873,  1190,   970,   877,
    1130,   880,   883,   887,   890,   889,   899,   682,  1130,   895,
     905,   900,   682,   907,   682,   119,   945,   119,   912,   119,
     910,   960,   971,   357,   358,   359,   360,   361,   362,   926,
     363,   935,   943,   534,   127,   955,   127,   962,   978,   958,
     959,   132,   980,   132,   973,   682,   159,  1025,   159,   975,
     159,   999,   976,  1242,  1005,  1450,   977,   991,   924,  1362,
     994,  1247,   992,   993,   995,   996,  1020,   508,  1015,  1023,
    1253,  1038,  1039,  1043,  1044,  1045,  1048,  1019,  1050,  1054,
    1056,  1052,  1058,   682,  1064,  1059,  1060,  1066,  1067,   119,
    1069,  1358,   128,   129,  1068,   130,   126,  1072,  1080,  1081,
    1083,  1087,  1178,  1091,  1097,  1099,   127,  1102,  1103,  1104,
      31,    32,    33,   132,  1109,  1106,  1110,  1107,  1187,  1108,
     159,    38,  1491,  1115,  1132,   682,  1113,  1130,  1141,  1378,
    1144,  1211,  1157,  1130,  1130,  1147,   534,  1079,  1149,  1049,
    1153,   813,  1163,  1156,  1159,   813,   813,  1162,  1164,  1165,
    1167,  1168,  1170,  1174,   682,   682,   119,  1171,  1006,  1175,
     682,  1179,  1201,  1082,  1214,  1185,  1117,   119,    70,    71,
      72,    73,    74,  1216,  1220,  1221,  1017,  1222,  1224,   677,
    1227,  1078,  1226,  1232,   127,    77,    78,   159,   331,  1239,
    1233,   132,  1238,  1248,   503,   838,  1250,  1251,   159,  1256,
      88,  1260,  1353,  1354,    11,  1360,  1363,  1374,  1364,  1365,
    1372,  1402,  1383,  1384,    93,  1130,  1407,  1410,  1416,  1435,
    1443,  1444,  1448,   203,   203,  1449,  1456,   215,   205,   205,
    1457,  1128,   217,  1458,  -251,  1356,  1460,  1397,  1461,  1128,
     503,  1463,  1464,  1469,  1466,  1483,  1472,  1471,  1473,   215,
    1414,  1486,  1415,   813,  1489,   813,  1490,  1498,  1500,  1504,
    1420,  1518,  1505,  1118,   682,  1515,  1519,  1520,  1119,  1088,
      56,    57,    58,   171,   172,   329,  1120,  1527,  1530,  1531,
    1532,  1534,  1485,  1100,   707,  1101,   939,   598,   600,   372,
     937,   904,  1499,   599,  1176,   682,  1497,  1086,   710,  1390,
     119,  1453,  1395,  1208,   249,  1521,   682,  1510,  1406,  1206,
     229,   682,  1369,  1121,  1122,  1041,  1123,   127,   607,  1014,
     699,  1012,   700,   879,   132,  1033,  1063,   951,   963,   505,
    1210,   159,   990,  1142,     0,   495,     0,     0,    95,     0,
     374,     0,     0,     0,     0,     0,     0,     0,     0,   813,
       0,     0,     0,     0,     0,   119,     0,     0,  1128,   119,
       0,  1230,     0,   119,  1128,  1128,  1245,     0,   682,     0,
       0,     0,   127,     0,     0,     0,     0,     0,     0,   132,
     127,     0,     0,     0,  1408,  1342,   159,   132,     0,     0,
     159,  1349,  1180,  1516,   159,     0,     0,  1182,   249,  1183,
    1522,   203,     0,     0,     0,     0,   205,   203,     0,     0,
       0,     0,   205,   203,     0,     0,     0,  1359,   205,     0,
       0,     0,     0,   682,     0,   813,     0,     0,   119,   119,
    1223,   682,     0,   119,     0,   682,  1376,     0,   682,   119,
       0,   215,   215,     0,   534,   127,  1128,   215,     0,     0,
     127,     0,   132,     0,     0,     0,   127,   132,     0,   159,
     159,   534,     0,   132,   159,     0,     0,   203,  1249,   534,
     159,     0,   205,  1405,   203,   203,     0,     0,     0,   205,
     205,   203,     0,     0,     0,     0,   205,   203,     0,     0,
       0,     0,   205,     0,     0,  1476,     0,     0,   756,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1361,     0,     0,   756,     0,     0,     0,     0,   215,     0,
       0,   215,     0,   204,   204,  1447,     0,   216,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   331,     0,  1379,
    1380,     0,   279,     0,     0,  1385,  -797,  -797,  -797,  -797,
     355,   356,   357,   358,   359,   360,   361,   362,     0,   363,
     813,     0,   215,     0,   119,     0,     0,   217,     0,     0,
       0,     0,     0,  1425,     0,     0,     0,     0,  1342,  1342,
       0,   127,  1349,  1349,     0,     0,     0,     0,   132,     0,
       0,     0,     0,     0,   279,   159,     0,     0,     0,   203,
     119,   119,     0,     0,   205,     0,   119,   203,     0,     0,
       0,     0,   205,     0,     0,     0,     0,   127,   127,     0,
       0,     0,     0,   127,   132,   132,     0,     0,     0,     0,
     132,   159,   159,     0,     0,     0,     0,   159,  1117,   119,
       0,     0,     0,     0,     0,     0,  1475,   215,     0,  1409,
       0,     0,   681,     0,     0,  1117,   127,     0,     0,     0,
       0,     0,  1487,   132,     0,     0,     0,     0,     0,     0,
     159,  1477,     0,     0,     0,     0,    11,     0,     0,     0,
    1432,     0,     0,     0,     0,     0,     0,     0,     0,   681,
       0,  1442,     0,    11,     0,     0,  1446,     0,     0,     0,
       0,     0,   119,     0,     0,     0,     0,   204,   119,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   127,
       0,     0,     0,     0,     0,   127,   132,     0,   215,   215,
       0,     0,   132,   159,   215,  1118,     0,     0,     0,   159,
    1119,     0,    56,    57,    58,   171,   172,   329,  1120,     0,
     203,   250,  1118,  1478,     0,   205,     0,  1119,     0,    56,
      57,    58,   171,   172,   329,  1120,     0,   204,     0,     0,
       0,     0,     0,     0,   204,   204,     0,     0,     0,     0,
       0,   204,     0,  1117,     0,  1121,  1122,   204,  1123,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   203,  1121,  1122,     0,  1123,   205,     0,  1528,     0,
      95,     0,     0,     0,     0,     0,  1533,     0,     0,     0,
    1535,    11,     0,  1536,     0,     0,     0,    95,     0,     0,
       0,     0,     0,  1234,     0,     0,   203,     0,   203,     0,
       0,   205,     0,   205,     0,     0,     0,     0,     0,     0,
    1235,     0,     0,     0,     0,     0,     0,     0,   203,   681,
       0,     0,   216,   205,     0,     0,     0,     0,     0,     0,
     215,   215,   681,   681,   681,     0,     0,     0,     0,     0,
    1118,     0,     0,     0,     0,  1119,     0,    56,    57,    58,
     171,   172,   329,  1120,     0,     0,     0,     0,     0,   204,
       0,     0,     0,   215,     0,     0,     0,     0,     0,     0,
       0,     0,   203,     0,     0,     0,     0,   205,     0,     0,
       0,   215,     0,   203,   203,     0,     0,     0,   205,   205,
    1121,  1122,     0,  1123,     0,     0,     0,     0,   215,     0,
       0,     0,     0,   250,   250,     0,     0,     0,     0,   250,
       0,   215,   685,     0,     0,    95,     0,     0,     0,     0,
       0,     0,   215,     0,     0,     0,     0,     0,   681,     0,
       0,   215,     0,     0,     0,     0,     0,     0,  1371,     0,
       0,   338,   339,   340,     0,     0,     0,     0,     0,   685,
     215,     0,     0,     0,     0,   217,     0,   341,     0,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     250,   363,     0,   250,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   203,     0,     0,     0,     0,   205,
       0,     0,     0,     0,     0,     0,     0,   215,     0,   215,
     204,     0,     0,     0,     0,   686,     0,     0,     0,     0,
       0,     0,   681,     0,     0,     0,   681,     0,   681,   403,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,     0,     0,     0,     0,     0,   681,     0,     0,   215,
       0,     0,   686,     0,     0,     0,     0,     0,     0,     0,
       0,   204,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   215,     0,   215,     0,   387,   388,     0,     0,     0,
       0,   203,     0,     0,     0,     0,   205,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   204,     0,   204,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   250,
       0,     0,     0,     0,   683,     0,     0,     0,   204,   685,
       0,     0,     0,   203,     0,     0,     0,     0,   205,     0,
     762,     0,   685,   685,   685,     0,   203,   203,   389,   681,
       0,   205,   205,     0,     0,     0,     0,     0,     0,     0,
     215,   683,     0,   681,     0,   681,     0,     0,     0,     0,
       0,     0,     0,   882,     0,     0,     0,     0,     0,     0,
       0,     0,   204,     0,     0,     0,     0,     0,     0,   215,
       0,   894,   203,   204,   204,     0,     0,   205,     0,     0,
     250,   250,     0,     0,     0,     0,   250,     0,   894,     0,
       0,     0,     0,   681,     0,     0,     0,     0,     0,     0,
       0,   538,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   686,     0,     0,     0,     0,     0,   685,     0,
       0,   927,     0,     0,     0,   686,   686,   686,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     216,     0,     0,     0,     0,     0,     0,     0,   215,     0,
       0,     0,   681,     0,     0,     0,     0,   681,     0,   681,
       0,     0,     0,     0,   215,     0,     0,   215,   215,     0,
     215,     0,     0,     0,     0,     0,     0,   215,     0,     0,
       0,     0,     0,     0,   204,     0,     0,     0,     0,     0,
     681,     0,     0,   403,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,   386,     0,     0,     0,     0,     0,
       0,   683,   685,     0,     0,     0,   685,     0,   685,     0,
       0,   686,   250,   250,   683,   683,   683,     0,   681,     0,
       0,     0,     0,     0,     0,     0,   685,   215,   215,   387,
     388,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   338,   339,   340,
       0,   215,     0,     0,     0,     0,     0,     0,     0,     0,
     681,   204,     0,   341,     0,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,     0,   363,     0,   681,
     681,     0,   389,   250,     0,   681,   215,     0,     0,     0,
     215,     0,     0,   204,   250,   686,     0,     0,     0,   686,
     683,   686,     0,     0,     0,     0,   204,   204,     0,   685,
     338,   339,   340,     0,     0,     0,     0,     0,     0,   686,
       0,     0,     0,   685,     0,   685,   341,     0,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,  1116,
     363,     0,   204,     0,     0,   718,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,     0,     0,
       0,     0,     0,   685,     0,     0,     0,     0,     0,   250,
       0,   250,     0,     0,     0,     0,     0,     0,     0,   681,
       0,     0,     0,     0,   683,     0,     0,     0,   683,     0,
     683,     0,   387,   388,     0,     0,     0,     0,     0,     0,
       0,     0,   686,     0,     0,     0,   215,     0,   683,     0,
     681,   250,     0,     0,     0,     0,   686,     0,   686,  1027,
       0,   681,   685,     0,     0,     0,   681,   685,     0,   685,
       0,     0,     0,   250,     0,   250,     0,     0,  1192,     0,
    1200,     0,     0,     0,     0,     0,     0,     0,   802,   803,
       0,     0,     0,     0,     0,   389,     0,     0,     0,     0,
     685,     0,     0,     0,     0,     0,   686,   804,     0,     0,
       0,     0,     0,     0,     0,   805,   806,   807,    34,     0,
       0,     0,     0,   681,     0,     0,   808,     0,     0,   797,
       0,   215,     0,     0,     0,     0,     0,     0,   685,     0,
       0,   683,     0,     0,     0,   215,     0,  1254,  1255,     0,
       0,     0,   250,     0,   215,   683,     0,   683,     0,     0,
       0,     0,     0,     0,     0,   686,     0,     0,     0,   215,
     686,   809,   686,     0,     0,     0,     0,     0,   681,     0,
     685,     0,     0,     0,   810,     0,   681,     0,     0,     0,
     681,     0,     0,   681,     0,     0,    82,    83,     0,    84,
      85,    86,     0,   686,     0,   683,     0,    27,    28,   685,
     685,     0,     0,     0,   811,   685,  1386,    34,     0,   200,
    1200,     0,   812,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,     0,
       0,   686,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   201,     0,     0,
     250,     0,     0,     0,   683,     0,     0,     0,     0,   683,
       0,   683,     0,     0,     0,     0,   250,     0,     0,   250,
       0,     0,     0,   686,     0,     0,     0,     0,   175,   250,
       0,    79,     0,    81,     0,    82,    83,     0,    84,    85,
      86,     0,   683,     0,     0,     0,     0,    89,     0,     0,
       0,     0,   686,   686,     0,     0,     0,     0,   686,   685,
       0,    96,  1389,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,     0,   408,
     683,     0,     0,     0,   114,     0,     0,     0,     0,     0,
     685,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   685,     0,     0,     0,     0,   685,     0,     0,     0,
       0,     0,     0,   250,     0,     0,     0,     0,     0,     0,
       0,     0,   683,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   179,   181,     0,   183,   184,   185,     0,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   683,   683,   209,   212,     0,     0,   683,     0,     0,
       0,     0,   686,   685,     0,     0,   230,     0,     0,     0,
       0,  1484,     0,   238,     0,   241,     0,     0,   257,     0,
     262,     0,     0,     0,     0,  1192,     0,     0,     0,     0,
       0,     0,     0,   686,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   686,     0,     0,   296,     0,   686,
       0,     0,     0,     0,     0,     0,     0,     0,   685,     0,
       0,   303,     0,     0,     0,     0,   685,     0,     0,     0,
     685,     0,  1462,   685,     0,     0,     0,     0,   306,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   338,
     339,   340,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   683,     0,     0,     0,   341,   686,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,  1426,   363,
       0,     0,   683,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   683,     0,     0,     0,     0,   683,   404,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   686,     0,     0,     0,     0,     0,     0,     0,   686,
       0,     0,     0,   686,     0,     0,   686,     0,     0,     0,
     946,   947,   948,    34,     0,     0,     0,     0,     0,     0,
     428,     0,     0,   428,   338,   339,   340,     0,     0,     0,
     230,   439,     0,     0,     0,   683,     0,     0,     0,     0,
     341,     0,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,     0,   363,     0,   250,     0,     0,     0,
       0,     0,     0,     0,   306,     0,     0,     0,     0,     0,
     209,   250,     0,     0,   511,     0,     0,     0,     0,     0,
     683,    82,    83,     0,    84,    85,    86,     0,   683,     0,
       0,     0,   683,     0,     0,   683,     0,   545,   800,     0,
       0,     0,     0,     0,     0,     0,     0,    96,   554,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,     0,   560,   561,   562,   564,   565,
     566,   567,   568,   569,   570,   571,   572,   573,   574,   575,
     576,   577,   578,   579,   580,   581,   582,   583,   584,   585,
       0,     0,   587,   587,     0,   590,     0,     0,     0,     0,
       0,     0,   606,   608,   609,   610,   611,   612,   613,   614,
     615,   616,   617,   618,     0,     0,     0,     0,     0,   587,
     623,     0,   554,   587,   626,     0,     0,     0,     0,     0,
     606,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     638,     0,   640,   886,     0,   338,   339,   340,   554,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   652,     0,
     653,   341,  1093,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,     0,   363,     0,   691,     0,     0,
     694,   697,   698,     0,     0,     0,     0,     0,   338,   339,
     340,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   709,   341,     0,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,     0,   363,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   738,     0,
       0,     0,     0,     0,     0,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   429,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     776,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,   784,    31,    32,    33,    34,    35,    36,     0,    37,
     296,     0,  1094,    38,    39,    40,    41,     0,    42,     0,
      43,     0,    44,     0,     0,    45,   793,     0,     0,    46,
      47,    48,    49,     0,    51,    52,     0,    53,     0,    55,
      56,    57,    58,   171,   172,    61,     0,    62,    63,    64,
       0,     0,   826,     0,     0,     0,     0,    68,    69,     0,
      70,    71,    72,    73,    74,   230,     0,   934,     0,     0,
       0,    75,     0,     0,     0,     0,   175,    77,    78,    79,
      80,    81,     0,    82,    83,     0,    84,    85,    86,     0,
       0,     0,    88,     0,     0,    89,   870,     0,     0,     0,
       0,    90,     0,     0,     0,     0,    93,    94,    95,    96,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,   111,     0,   112,
     113,     0,   114,   115,     0,   116,   117,     0,     0,     0,
     901,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   908,   403,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,     0,     0,     0,     0,
       0,     0,     0,     0,   923,     0,     0,   791,     0,     0,
       0,     0,     0,     0,   931,     0,    34,   932,   200,   933,
       0,     0,     0,   554,     0,     0,     0,     0,     0,     0,
     387,   388,   554,     0,     0,     0,     0,     0,     0,     0,
       0,   338,   339,   340,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   201,   341,   965,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
       0,   363,     0,     0,     0,     0,     0,   175,     0,     0,
      79,     0,    81,   389,    82,    83,     0,    84,    85,    86,
       0,     0,     0,     0,     0,     0,     0,     0,  1009,  1010,
    1011,     0,     0,     0,   694,  1013,     0,     0,     0,     0,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,  1026,     0,   202,     0,
       0,     0,     0,   114,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1046,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   338,
     339,   340,   554,     0,     0,     0,     0,     0,     0,     0,
     554,     0,  1026,     0,     0,   341,     0,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,     0,   363,
       0,     0,   230,     0,     0,   338,   339,   340,     0,     0,
       0,  1092,     0,     0,     0,     0,     0,     0,     0,     0,
     942,   341,     0,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,     0,   363,     0,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,  1135,
       0,     0,     0,  1136,     0,  1137,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   554,     0,     0,     0,     0,
       0,     0,  1151,   554,     0,     0,    11,    12,    13,     0,
       0,   554,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,    36,
       0,    37,     0,     0,     0,    38,    39,    40,    41,     0,
      42,     0,    43,     0,    44,     0,     0,    45,     0,  1184,
       0,    46,    47,    48,    49,    50,    51,    52,   966,    53,
      54,    55,    56,    57,    58,    59,    60,    61,     0,    62,
      63,    64,    65,    66,    67,     0,     0,     0,     0,    68,
      69,   554,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,    75,     0,     0,     0,     0,    76,    77,
      78,    79,    80,    81,  1143,    82,    83,     0,    84,    85,
      86,    87,     0,     0,    88,     0,     0,    89,     0,     0,
       0,     0,     0,    90,    91,     0,    92,     0,    93,    94,
      95,    96,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,     0,     0,   111,
       0,   112,   113,   902,   114,   115,     0,   116,   117,     0,
       0,   554,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,  1381,
     363,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,    36,     0,    37,     0,     0,     0,
      38,    39,    40,    41,     0,    42,     0,    43,     0,    44,
       0,     0,    45,     0,     0,     0,    46,    47,    48,    49,
      50,    51,    52,     0,    53,    54,    55,    56,    57,    58,
      59,    60,    61,     0,    62,    63,    64,    65,    66,    67,
       0,     0,     0,     0,    68,    69,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,    76,    77,    78,    79,    80,    81,     0,
      82,    83,     0,    84,    85,    86,    87,     0,     0,    88,
       0,     0,    89,     0,     0,     0,     0,     0,    90,    91,
       0,    92,     0,    93,    94,    95,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,   111,     0,   112,   113,  1028,   114,
     115,   340,   116,   117,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   341,     0,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,     0,   363,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,    37,     0,
       0,     0,    38,    39,    40,    41,     0,    42,     0,    43,
       0,    44,     0,     0,    45,     0,     0,     0,    46,    47,
      48,    49,    50,    51,    52,     0,    53,    54,    55,    56,
      57,    58,    59,    60,    61,     0,    62,    63,    64,    65,
      66,    67,     0,     0,     0,     0,    68,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,    76,    77,    78,    79,    80,
      81,     0,    82,    83,     0,    84,    85,    86,    87,     0,
       0,    88,     0,     0,    89,     0,     0,     0,     0,     0,
      90,    91,     0,    92,     0,    93,    94,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,   341,    10,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,     0,   363,
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
       0,     0,    75,     0,     0,     0,     0,   175,    77,    78,
      79,    80,    81,     0,    82,    83,     0,    84,    85,    86,
      87,     0,     0,    88,     0,     0,    89,     0,     0,     0,
       0,     0,    90,     0,     0,     0,     0,    93,    94,    95,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,     0,   111,     0,
     112,   113,   539,   114,   115,     0,   116,   117,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
       0,   363,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,    37,     0,     0,     0,    38,    39,    40,    41,
       0,    42,     0,    43,     0,    44,     0,     0,    45,     0,
       0,     0,    46,    47,    48,    49,     0,    51,    52,     0,
      53,     0,    55,    56,    57,    58,    59,    60,    61,     0,
      62,    63,    64,     0,    66,    67,     0,     0,     0,     0,
      68,    69,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,    75,     0,     0,     0,     0,   175,
      77,    78,    79,    80,    81,     0,    82,    83,     0,    84,
      85,    86,    87,     0,     0,    88,     0,     0,    89,     0,
       0,     0,     0,     0,    90,     0,     0,     0,     0,    93,
      94,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,   881,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,     0,   363,     0,     0,     0,     0,     0,     0,    11,
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
       0,   175,    77,    78,    79,    80,    81,     0,    82,    83,
       0,    84,    85,    86,    87,     0,     0,    88,     0,     0,
      89,     0,     0,     0,     0,     0,    90,     0,     0,     0,
       0,    93,    94,    95,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,   111,     0,   112,   113,   972,   114,   115,     0,
     116,   117,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,     0,   363,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,    36,     0,    37,     0,     0,     0,
      38,    39,    40,    41,   974,    42,     0,    43,     0,    44,
       0,     0,    45,     0,     0,     0,    46,    47,    48,    49,
       0,    51,    52,     0,    53,     0,    55,    56,    57,    58,
      59,    60,    61,     0,    62,    63,    64,     0,    66,    67,
       0,     0,     0,     0,    68,    69,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,   175,    77,    78,    79,    80,    81,     0,
      82,    83,     0,    84,    85,    86,    87,     0,     0,    88,
       0,     0,    89,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,    93,    94,    95,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,   111,     0,   112,   113,     0,   114,
     115,     0,   116,   117,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,     0,   363,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,    37,     0,
       0,     0,    38,    39,    40,    41,     0,    42,     0,    43,
       0,    44,  1089,     0,    45,     0,     0,     0,    46,    47,
      48,    49,     0,    51,    52,     0,    53,     0,    55,    56,
      57,    58,    59,    60,    61,     0,    62,    63,    64,     0,
      66,    67,     0,     0,     0,     0,    68,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,   175,    77,    78,    79,    80,
      81,     0,    82,    83,     0,    84,    85,    86,    87,     0,
       0,    88,     0,     0,    89,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,    93,    94,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,     0,   363,     0,     0,     0,     0,
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
       0,     0,    75,     0,     0,     0,     0,   175,    77,    78,
      79,    80,    81,     0,    82,    83,     0,    84,    85,    86,
      87,     0,     0,    88,     0,     0,    89,     0,     0,     0,
       0,     0,    90,     0,     0,     0,     0,    93,    94,    95,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,     0,   111,     0,
     112,   113,  1186,   114,   115,     0,   116,   117,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,  -797,
    -797,  -797,  -797,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,     0,   363,     0,     0,     0,
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
       0,     0,     0,     0,    75,     0,     0,     0,     0,   175,
      77,    78,    79,    80,    81,     0,    82,    83,     0,    84,
      85,    86,    87,     0,     0,    88,     0,     0,    89,     0,
       0,     0,     0,     0,    90,     0,     0,     0,     0,    93,
      94,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,  1382,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,    36,     0,    37,     0,     0,     0,    38,    39,
      40,    41,     0,    42,     0,    43,  1421,    44,     0,     0,
      45,     0,     0,     0,    46,    47,    48,    49,     0,    51,
      52,     0,    53,     0,    55,    56,    57,    58,    59,    60,
      61,     0,    62,    63,    64,     0,    66,    67,     0,     0,
       0,     0,    68,    69,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
       0,   175,    77,    78,    79,    80,    81,     0,    82,    83,
       0,    84,    85,    86,    87,     0,     0,    88,     0,     0,
      89,     0,     0,     0,     0,     0,    90,     0,     0,     0,
       0,    93,    94,    95,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,   111,     0,   112,   113,     0,   114,   115,     0,
     116,   117,     5,     6,     7,     8,     9,     0,     0,     0,
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
       0,     0,     0,   175,    77,    78,    79,    80,    81,     0,
      82,    83,     0,    84,    85,    86,    87,     0,     0,    88,
       0,     0,    89,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,    93,    94,    95,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,   111,     0,   112,   113,  1451,   114,
     115,     0,   116,   117,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,    37,     0,
       0,     0,    38,    39,    40,    41,     0,    42,     0,    43,
       0,    44,     0,     0,    45,     0,     0,     0,    46,    47,
      48,    49,     0,    51,    52,     0,    53,     0,    55,    56,
      57,    58,    59,    60,    61,     0,    62,    63,    64,     0,
      66,    67,     0,     0,     0,     0,    68,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,   175,    77,    78,    79,    80,
      81,     0,    82,    83,     0,    84,    85,    86,    87,     0,
       0,    88,     0,     0,    89,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,    93,    94,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
    1452,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,    36,     0,
      37,     0,     0,     0,    38,    39,    40,    41,     0,    42,
    1455,    43,     0,    44,     0,     0,    45,     0,     0,     0,
      46,    47,    48,    49,     0,    51,    52,     0,    53,     0,
      55,    56,    57,    58,    59,    60,    61,     0,    62,    63,
      64,     0,    66,    67,     0,     0,     0,     0,    68,    69,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,     0,   175,    77,    78,
      79,    80,    81,     0,    82,    83,     0,    84,    85,    86,
      87,     0,     0,    88,     0,     0,    89,     0,     0,     0,
       0,     0,    90,     0,     0,     0,     0,    93,    94,    95,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,     0,   111,     0,
     112,   113,     0,   114,   115,     0,   116,   117,     5,     6,
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
       0,     0,     0,     0,    75,     0,     0,     0,     0,   175,
      77,    78,    79,    80,    81,     0,    82,    83,     0,    84,
      85,    86,    87,     0,     0,    88,     0,     0,    89,     0,
       0,     0,     0,     0,    90,     0,     0,     0,     0,    93,
      94,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,  1470,   114,   115,     0,   116,   117,
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
       0,   175,    77,    78,    79,    80,    81,     0,    82,    83,
       0,    84,    85,    86,    87,     0,     0,    88,     0,     0,
      89,     0,     0,     0,     0,     0,    90,     0,     0,     0,
       0,    93,    94,    95,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,   111,     0,   112,   113,  1523,   114,   115,     0,
     116,   117,     5,     6,     7,     8,     9,     0,     0,     0,
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
       0,     0,     0,   175,    77,    78,    79,    80,    81,     0,
      82,    83,     0,    84,    85,    86,    87,     0,     0,    88,
       0,     0,    89,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,    93,    94,    95,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,   111,     0,   112,   113,  1529,   114,
     115,     0,   116,   117,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,    37,     0,
       0,     0,    38,    39,    40,    41,     0,    42,     0,    43,
       0,    44,     0,     0,    45,     0,     0,     0,    46,    47,
      48,    49,     0,    51,    52,     0,    53,     0,    55,    56,
      57,    58,    59,    60,    61,     0,    62,    63,    64,     0,
      66,    67,     0,     0,     0,     0,    68,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,   175,    77,    78,    79,    80,
      81,     0,    82,    83,     0,    84,    85,    86,    87,     0,
       0,    88,     0,     0,    89,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,    93,    94,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     655,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,    36,     0,
      37,     0,     0,     0,    38,    39,    40,    41,     0,    42,
       0,    43,     0,    44,     0,     0,    45,     0,     0,     0,
      46,    47,    48,    49,     0,    51,    52,     0,    53,     0,
      55,    56,    57,    58,   171,   172,    61,     0,    62,    63,
      64,     0,     0,     0,     0,     0,     0,     0,    68,    69,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,     0,   175,    77,    78,
      79,    80,    81,     0,    82,    83,     0,    84,    85,    86,
       0,     0,     0,    88,     0,     0,    89,     0,     0,     0,
       0,     0,    90,     0,     0,     0,     0,    93,    94,    95,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,     0,   111,     0,
     112,   113,     0,   114,   115,     0,   116,   117,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   841,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,    37,     0,     0,     0,    38,    39,    40,    41,
       0,    42,     0,    43,     0,    44,     0,     0,    45,     0,
       0,     0,    46,    47,    48,    49,     0,    51,    52,     0,
      53,     0,    55,    56,    57,    58,   171,   172,    61,     0,
      62,    63,    64,     0,     0,     0,     0,     0,     0,     0,
      68,    69,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,    75,     0,     0,     0,     0,   175,
      77,    78,    79,    80,    81,     0,    82,    83,     0,    84,
      85,    86,     0,     0,     0,    88,     0,     0,    89,     0,
       0,     0,     0,     0,    90,     0,     0,     0,     0,    93,
      94,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,     0,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1244,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,    36,     0,    37,     0,     0,     0,    38,    39,
      40,    41,     0,    42,     0,    43,     0,    44,     0,     0,
      45,     0,     0,     0,    46,    47,    48,    49,     0,    51,
      52,     0,    53,     0,    55,    56,    57,    58,   171,   172,
      61,     0,    62,    63,    64,     0,     0,     0,     0,     0,
       0,     0,    68,    69,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
       0,   175,    77,    78,    79,    80,    81,     0,    82,    83,
       0,    84,    85,    86,     0,     0,     0,    88,     0,     0,
      89,     0,     0,     0,     0,     0,    90,     0,     0,     0,
       0,    93,    94,    95,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,   111,     0,   112,   113,     0,   114,   115,     0,
     116,   117,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1375,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,    36,     0,    37,     0,     0,     0,
      38,    39,    40,    41,     0,    42,     0,    43,     0,    44,
       0,     0,    45,     0,     0,     0,    46,    47,    48,    49,
       0,    51,    52,     0,    53,     0,    55,    56,    57,    58,
     171,   172,    61,     0,    62,    63,    64,     0,     0,     0,
       0,     0,     0,     0,    68,    69,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,   175,    77,    78,    79,    80,    81,     0,
      82,    83,     0,    84,    85,    86,     0,     0,     0,    88,
       0,     0,    89,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,    93,    94,    95,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,   111,     0,   112,   113,     0,   114,
     115,     0,   116,   117,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,    37,     0,
       0,     0,    38,    39,    40,    41,     0,    42,     0,    43,
       0,    44,     0,     0,    45,     0,     0,     0,    46,    47,
      48,    49,     0,    51,    52,     0,    53,     0,    55,    56,
      57,    58,   171,   172,    61,     0,    62,    63,    64,     0,
       0,     0,     0,     0,     0,     0,    68,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,   175,    77,    78,    79,    80,
      81,     0,    82,    83,     0,    84,    85,    86,     0,     0,
       0,    88,     0,     0,    89,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,    93,    94,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   602,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,    33,    34,    35,    36,     0,
       0,     0,     0,     0,    38,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    49,     0,     0,     0,     0,     0,     0,
       0,    56,    57,    58,   171,   172,   173,     0,    34,    63,
      64,     0,     0,     0,     0,     0,     0,     0,   174,    69,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,     0,   175,    77,    78,
      79,   603,    81,     0,    82,    83,     0,    84,    85,    86,
       0,     0,  1204,    88,     0,     0,    89,     0,     0,     0,
       0,     0,    90,     0,     0,     0,     0,    93,    94,    95,
      96,   254,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,    82,    83,   111,    84,
      85,    86,     0,   114,   115,     0,   116,   117,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,     0,
    1205,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,     0,     0,     0,     0,    38,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,     0,
       0,     0,     0,    56,    57,    58,   171,   172,   173,     0,
       0,    63,    64,     0,     0,     0,     0,     0,     0,     0,
     174,    69,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,    75,     0,     0,     0,     0,   175,
      77,    78,    79,     0,    81,     0,    82,    83,     0,    84,
      85,    86,     0,     0,     0,    88,     0,     0,    89,     0,
       0,     0,     0,     0,    90,     0,     0,     0,     0,    93,
      94,    95,    96,   254,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   255,     0,     0,   114,   115,     0,   116,   117,
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
       0,     0,     0,     0,     0,    56,    57,    58,   171,   172,
     173,     0,    34,    63,    64,     0,     0,     0,     0,     0,
       0,     0,   174,    69,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
       0,   175,    77,    78,    79,   603,    81,     0,    82,    83,
       0,    84,    85,    86,     0,     0,     0,    88,     0,     0,
      89,     0,     0,     0,     0,     0,    90,     0,     0,     0,
       0,    93,    94,    95,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
      82,    83,   111,    84,    85,    86,     0,   114,   115,     0,
     116,   117,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   208,     0,     0,   556,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,    35,    36,     0,     0,     0,     0,     0,
      38,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    56,    57,    58,
     171,   172,   173,     0,    34,    63,    64,     0,     0,     0,
       0,     0,     0,     0,   174,    69,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,   175,    77,    78,    79,     0,    81,     0,
      82,    83,     0,    84,    85,    86,     0,     0,     0,    88,
       0,     0,    89,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,    93,     0,    95,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,    82,    83,   111,    84,    85,    86,     0,   114,
     115,     0,   116,   117,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,     0,   774,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,     0,     0,
       0,     0,    38,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    49,     0,     0,     0,     0,     0,     0,     0,    56,
      57,    58,   171,   172,   173,     0,     0,    63,    64,     0,
       0,     0,     0,     0,     0,     0,   174,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,   175,    77,    78,    79,     0,
      81,     0,    82,    83,     0,    84,    85,    86,     0,     0,
       0,    88,     0,     0,    89,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,    93,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   237,     0,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,    33,    34,    35,    36,     0,
       0,     0,     0,     0,    38,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    49,     0,     0,     0,     0,     0,     0,
       0,    56,    57,    58,   171,   172,   173,     0,     0,    63,
      64,     0,     0,     0,     0,     0,     0,     0,   174,    69,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,     0,   175,    77,    78,
      79,     0,    81,     0,    82,    83,     0,    84,    85,    86,
       0,     0,     0,    88,     0,     0,    89,     0,     0,     0,
       0,     0,    90,     0,     0,     0,     0,    93,     0,    95,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,     0,   111,     0,
     240,     0,     0,   114,   115,     0,   116,   117,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,     0,     0,     0,     0,    38,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   295,     0,     0,    49,     0,     0,     0,     0,
       0,     0,     0,    56,    57,    58,   171,   172,   173,     0,
      34,    63,    64,     0,     0,     0,     0,     0,     0,     0,
     174,    69,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,    75,     0,     0,     0,     0,   175,
      77,    78,    79,     0,    81,     0,    82,    83,     0,    84,
      85,    86,     0,     0,     0,    88,  1004,     0,    89,     0,
       0,     0,     0,     0,    90,     0,     0,     0,     0,    93,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,    82,    83,
     111,    84,    85,    86,     0,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,    35,    36,     0,     0,     0,     0,     0,    38,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    49,     0,     0,
       0,     0,     0,     0,     0,    56,    57,    58,   171,   172,
     173,     0,     0,    63,    64,     0,     0,     0,     0,     0,
       0,     0,   174,    69,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
       0,   175,    77,    78,    79,     0,    81,     0,    82,    83,
       0,    84,    85,    86,     0,     0,     0,    88,     0,     0,
      89,     0,     0,     0,     0,     0,    90,     0,     0,     0,
       0,    93,     0,    95,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,   111,   427,     0,     0,     0,   114,   115,     0,
     116,   117,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   551,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,    35,    36,     0,     0,     0,     0,     0,
      38,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    56,    57,    58,
     171,   172,   173,     0,     0,    63,    64,     0,     0,     0,
       0,     0,     0,     0,   174,    69,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,   175,    77,    78,    79,     0,    81,     0,
      82,    83,     0,    84,    85,    86,     0,     0,     0,    88,
       0,     0,    89,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,    93,     0,    95,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,   111,     0,     0,     0,     0,   114,
     115,     0,   116,   117,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   563,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,     0,     0,
       0,     0,    38,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    49,     0,     0,     0,     0,     0,     0,     0,    56,
      57,    58,   171,   172,   173,     0,    34,    63,    64,     0,
       0,     0,     0,     0,     0,     0,   174,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,   175,    77,    78,    79,     0,
      81,     0,    82,    83,     0,    84,    85,    86,     0,     0,
       0,    88,     0,     0,    89,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,    93,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,    82,    83,   111,    84,    85,    86,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   602,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,    33,    34,    35,    36,     0,
       0,     0,     0,     0,    38,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    49,     0,     0,     0,     0,     0,     0,
       0,    56,    57,    58,   171,   172,   173,     0,     0,    63,
      64,     0,     0,     0,     0,     0,     0,     0,   174,    69,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,     0,   175,    77,    78,
      79,     0,    81,     0,    82,    83,     0,    84,    85,    86,
       0,     0,     0,    88,     0,     0,    89,     0,     0,     0,
       0,     0,    90,     0,     0,     0,     0,    93,     0,    95,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,     0,   111,     0,
       0,     0,     0,   114,   115,     0,   116,   117,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   637,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,     0,     0,     0,     0,    38,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,     0,
       0,     0,     0,    56,    57,    58,   171,   172,   173,     0,
       0,    63,    64,     0,     0,     0,     0,     0,     0,     0,
     174,    69,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,    75,     0,     0,     0,     0,   175,
      77,    78,    79,     0,    81,     0,    82,    83,     0,    84,
      85,    86,     0,     0,     0,    88,     0,     0,    89,     0,
       0,     0,     0,     0,    90,     0,     0,     0,     0,    93,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,     0,     0,     0,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   639,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,    35,    36,     0,     0,     0,     0,     0,    38,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    49,     0,     0,
       0,     0,     0,     0,     0,    56,    57,    58,   171,   172,
     173,     0,     0,    63,    64,     0,     0,     0,     0,     0,
       0,     0,   174,    69,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
       0,   175,    77,    78,    79,     0,    81,     0,    82,    83,
       0,    84,    85,    86,     0,     0,     0,    88,     0,     0,
      89,     0,     0,     0,     0,     0,    90,     0,     0,     0,
       0,    93,     0,    95,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,   111,     0,     0,     0,     0,   114,   115,     0,
     116,   117,     5,     6,     7,     8,     9,     0,     0,     0,
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
     171,   172,   173,     0,     0,    63,    64,     0,     0,     0,
       0,     0,     0,     0,   174,    69,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,   175,    77,    78,    79,     0,    81,     0,
      82,    83,     0,    84,    85,    86,     0,     0,     0,    88,
       0,     0,    89,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,    93,     0,    95,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,   111,     0,     0,   651,     0,   114,
     115,     0,   116,   117,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   922,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,     0,     0,
       0,     0,    38,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    49,     0,     0,     0,     0,     0,     0,     0,    56,
      57,    58,   171,   172,   173,     0,     0,    63,    64,     0,
       0,     0,     0,     0,     0,     0,   174,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,   175,    77,    78,    79,     0,
      81,     0,    82,    83,     0,    84,    85,    86,     0,     0,
       0,    88,     0,     0,    89,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,    93,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,     0,     0,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   964,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,    33,    34,    35,    36,     0,
       0,     0,     0,     0,    38,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    49,     0,     0,     0,     0,     0,     0,
       0,    56,    57,    58,   171,   172,   173,     0,     0,    63,
      64,     0,     0,     0,     0,     0,     0,     0,   174,    69,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,     0,   175,    77,    78,
      79,     0,    81,     0,    82,    83,     0,    84,    85,    86,
       0,     0,     0,    88,     0,     0,    89,     0,     0,     0,
       0,     0,    90,     0,     0,     0,     0,    93,     0,    95,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,     0,   111,     0,
       0,     0,     0,   114,   115,     0,   116,   117,     5,     6,
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
       0,     0,     0,    56,    57,    58,   171,   172,   173,     0,
       0,    63,    64,     0,     0,     0,     0,     0,     0,     0,
     174,    69,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,    75,     0,     0,     0,     0,   175,
      77,    78,    79,     0,    81,     0,    82,    83,     0,    84,
      85,    86,     0,     0,     0,    88,     0,     0,    89,     0,
       0,     0,     0,     0,    90,     0,     0,     0,     0,    93,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,     0,     0,     0,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,   510,    36,     0,     0,     0,     0,     0,    38,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    49,     0,     0,
       0,     0,     0,     0,     0,    56,    57,    58,   171,   172,
     173,     0,     0,    63,    64,     0,     0,     0,     0,     0,
       0,     0,   174,    69,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
       0,   175,    77,    78,    79,     0,    81,     0,    82,    83,
       0,    84,    85,    86,     0,     0,     0,    88,     0,     0,
      89,     0,     0,     0,     0,     0,    90,     0,     0,     0,
       0,    93,     0,    95,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,   111,   338,   339,   340,     0,   114,   115,     0,
     116,   117,     0,     0,     0,     0,     0,     0,     0,   341,
       0,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,     0,   363,   338,   339,   340,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     341,     0,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,     0,   363,     0,     0,     0,   338,   339,
     340,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   341,     0,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,     0,   363,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   338,   339,   340,     0,  1263,  1264,  1265,  1266,  1267,
       0,     0,  1268,  1269,  1270,  1271,     0,   341,     0,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
       0,   363,  1218,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1272,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1273,  1274,  1275,  1276,  1277,
    1278,  1279,     0,     0,     0,    34,     0,     0,     0,     0,
       0,     0,     0,  1219,  1280,  1281,  1282,  1283,  1284,  1285,
    1286,  1287,  1288,  1289,  1290,  1291,  1292,  1293,  1294,  1295,
    1296,  1297,  1298,  1299,  1300,  1301,  1302,  1303,  1304,  1305,
    1306,  1307,  1308,  1309,  1310,  1311,  1312,  1313,  1314,  1315,
    1316,  1317,  1318,  1319,  1320,   364,     0,  1321,  1322,     0,
    1323,  1324,  1325,  1326,  1327,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1328,  1329,  1330,     0,
    1331,     0,     0,    82,    83,     0,    84,    85,    86,  1332,
       0,  1333,  1334,     0,  1335,     0,     0,     0,     0,     0,
       0,  1336,  1337,     0,  1338,  1422,  1339,  1340,  1341,    96,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   338,   339,   340,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     341,     0,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,     0,   363,   338,   339,   340,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   341,     0,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,     0,   363,   338,   339,   340,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   341,     0,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,     0,   363,   338,   339,   340,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   341,     0,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,     0,   363,     0,     0,
       0,     0,     0,     0,   338,   339,   340,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     341,   443,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,     0,   363,   338,   339,   340,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   341,   445,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,     0,   363,   338,   339,   340,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   341,   457,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,     0,   363,   338,   339,   340,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   341,   481,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,     0,   363,     0,     0,
       0,     0,     0,   338,   339,   340,     0,     0,     0,     0,
       0,    34,     0,   200,     0,     0,     0,     0,     0,   341,
     629,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,     0,   363,   338,   339,   340,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   593,     0,     0,
     341,   648,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   242,   363,     0,     0,     0,     0,    82,
      83,     0,    84,    85,    86,     0,     0,   878,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   243,
       0,     0,     0,     0,     0,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
      34,     0,     0,     0,   874,   875,   594,     0,   114,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   242,
       0,     0,     0,     0,     0,     0,     0,  -292,     0,     0,
       0,     0,     0,     0,     0,    56,    57,    58,   171,   172,
     329,     0,     0,     0,     0,   243,  1252,     0,     0,     0,
       0,     0,     0,   244,   245,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    34,     0,     0,     0,
       0,   175,     0,     0,    79,     0,   246,     0,    82,    83,
       0,    84,    85,    86,     0,     0,  1112,     0,     0,     0,
       0,     0,     0,   450,     0,     0,   247,     0,     0,   242,
       0,     0,     0,    95,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   244,
     245,     0,   248,     0,     0,   243,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   175,     0,     0,
      79,     0,   246,     0,    82,    83,    34,    84,    85,    86,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   247,     0,     0,   242,     0,     0,     0,     0,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,     0,   248,     0,
       0,   243,     0,     0,     0,     0,     0,     0,     0,   244,
     245,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    34,     0,     0,     0,     0,   175,     0,     0,
      79,     0,   246,     0,    82,    83,     0,    84,    85,    86,
       0,   854,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   247,     0,     0,   242,     0,     0,     0,     0,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   244,   245,     0,   248,     0,
       0,   243,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   175,     0,     0,    79,     0,   246,     0,
      82,    83,    34,    84,    85,    86,     0,  1098,     0,     0,
     835,     0,     0,     0,     0,     0,     0,     0,   247,     0,
       0,     0,     0,     0,     0,     0,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,     0,   248,     0,     0,     0,     0,     0,
       0,    34,     0,   200,     0,   244,   245,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   175,     0,     0,    79,     0,   246,     0,
      82,    83,     0,    84,    85,    86,     0,     0,     0,     0,
       0,   201,     0,     0,     0,     0,     0,     0,   247,     0,
       0,     0,     0,   836,     0,     0,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,   175,     0,   248,    79,     0,    81,     0,    82,
      83,     0,    84,    85,    86,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   673,   674,     0,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,   675,     0,   202,     0,     0,     0,     0,   114,    31,
      32,    33,    34,     0,     0,     0,     0,     0,     0,     0,
      38,   338,   339,   340,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   341,     0,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
      34,   363,   200,     0,     0,   676,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,   677,     0,
       0,     0,     0,   175,    77,    78,    79,     0,   678,     0,
      82,    83,     0,    84,    85,    86,     0,     0,     0,    88,
     201,     0,     0,     0,     0,     0,     0,     0,   679,     0,
      34,     0,   200,    93,     0,     0,   680,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   175,     0,     0,    79,   413,    81,     0,    82,    83,
       0,    84,    85,    86,     0,     0,     0,     0,     0,     0,
     201,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   502,     0,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,   175,   202,     0,    79,   486,    81,   114,    82,    83,
       0,    84,    85,    86,    34,     0,   200,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,     0,   202,     0,   201,     0,     0,   114,     0,     0,
       0,     0,     0,     0,     0,     0,   897,     0,    34,     0,
     200,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   175,     0,     0,    79,     0,
      81,     0,    82,    83,     0,    84,    85,    86,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   201,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,    34,     0,   200,   202,     0,     0,   175,
       0,   114,    79,     0,    81,     0,    82,    83,     0,    84,
      85,    86,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    96,   213,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,    34,     0,   200,
     202,     0,     0,     0,     0,   114,     0,     0,     0,     0,
       0,     0,     0,     0,   175,     0,     0,    79,     0,    81,
       0,    82,    83,     0,    84,    85,    86,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    34,     0,
     200,     0,     0,     0,     0,     0,     0,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,     0,   214,     0,     0,     0,     0,
     114,     0,     0,     0,     0,    82,    83,     0,    84,    85,
      86,     0,     0,     0,     0,     0,     0,     0,    34,     0,
     200,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    96,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,    82,    83,     0,    84,
      85,    86,   619,     0,   114,     0,     0,     0,     0,    34,
       0,   200,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,     0,
       0,     0,     0,   594,     0,   114,    82,    83,     0,    84,
      85,    86,     0,     0,     0,     0,     0,     0,     0,    34,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,    82,    83,     0,
      84,    85,    86,   645,     0,   114,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    34,     0,     0,     0,     0,
       0,     0,     0,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
     175,     0,     0,    79,   918,     0,   114,    82,    83,     0,
      84,    85,    86,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    34,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,   291,
       0,     0,     0,    82,    83,  1424,    84,    85,    86,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    34,     0,     0,     0,     0,     0,     0,    96,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,     0,     0,     0,  1347,
     292,    82,    83,  1348,    84,    85,    86,     0,     0,     0,
       0,     0,     0,  1193,     0,     0,     0,     0,     0,     0,
      34,     0,     0,     0,     0,  1194,     0,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,   175,     0,  1205,    79,     0,  1195,     0,
      82,    83,     0,    84,  1196,    86,     0,     0,     0,     0,
       0,    34,     0,   728,   729,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    96,    34,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   175,     0,     0,    79,     0,    81,     0,    82,    83,
       0,    84,    85,    86,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      34,     0,     0,     0,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,    82,
      83,     0,    84,    85,    86,     0,     0,     0,     0,     0,
       0,   263,     0,     0,     0,    82,    83,     0,    84,    85,
      86,     0,     0,     0,     0,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,    96,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   326,     0,    82,    83,
       0,    84,    85,    86,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   338,
     339,   340,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   341,     0,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,     0,   363,
     338,   339,   340,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   341,     0,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,     0,
     363,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   338,   339,   340,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   341,   521,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,     0,   363,   338,   339,   340,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   341,   780,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,     0,   363,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   338,   339,   340,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   341,
     821,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,     0,   363,     0,     0,   338,   339,   340,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1055,   341,   777,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   659,   363,   338,   339,   340,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   341,     0,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,     0,   363,   339,   340,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   341,     0,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,     0,   363
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-1219))

#define yytable_value_is_error(yytable_value) \
  ((yytable_value) == (-797))

static const yytype_int16 yycheck[] =
{
       4,   157,   132,     4,     4,   553,     4,     4,   400,    51,
      30,   322,   177,   407,    87,   914,    28,     4,    91,    92,
     160,    41,   375,   630,     4,    45,   167,   363,   770,   215,
     319,     4,   754,   517,   747,   658,   664,  1050,   905,   221,
     395,    27,     9,    47,   111,     9,    50,   221,   178,     9,
       9,    24,    25,     9,     9,   242,   243,     9,   131,    54,
       9,   248,     9,    67,     9,     9,   421,     9,     9,     9,
      30,     9,     9,    27,     9,   123,    10,    11,    12,     9,
     111,    76,   222,    87,    79,     9,     9,    91,    92,     9,
       9,     9,    26,   304,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,   773,    50,    42,    75,     9,
       9,    42,   863,     9,   429,   165,   867,   131,   147,    42,
     788,    42,   319,   527,  1147,   202,  1149,    42,   111,   104,
     123,    92,   123,   121,   122,   132,    62,   214,    62,    26,
      62,   199,  1370,   866,    62,   195,    62,   150,    62,    95,
      96,     0,    95,    96,    62,   330,    43,   453,    75,    46,
     163,    67,    68,   478,    62,    62,    75,   157,   197,   198,
      62,    62,    32,   187,   926,  1052,   143,   929,    62,    62,
      62,   178,  1059,  1060,    62,    62,   147,   162,   592,    62,
    1418,    62,   955,   195,   490,   198,   959,   123,     8,   123,
     277,   123,   198,   196,   166,   196,   844,    47,   846,  1232,
      75,    32,   166,   227,   195,    75,    47,   231,   369,   202,
     227,   235,   199,   197,   231,   208,   196,   402,   197,   198,
     196,   214,   197,   197,   374,   958,   277,   196,   389,   253,
     197,   196,   325,   737,   227,   197,   197,   197,   231,   197,
     197,   448,   197,   199,    75,   199,   199,   197,   263,   143,
     411,   196,   193,   197,   197,   669,   196,   196,   196,   420,
     166,   939,   423,   196,   198,   196,   198,   195,   193,  1156,
     198,   196,   198,   200,   198,   268,   291,   195,   371,   372,
     198,   200,   275,   276,   277,   309,   196,   196,   158,   282,
     198,   198,  1054,    80,   318,   288,   198,   198,   322,   149,
      80,   325,   309,   198,   198,   198,   198,    75,   149,   309,
     198,   198,   724,   375,   164,   198,   309,   198,    32,    75,
      32,   408,   529,   530,    80,   200,   651,   158,   535,   150,
     655,   363,   975,   960,   536,    92,   119,   369,    75,  1372,
      67,    68,   536,    80,   195,   369,   370,   371,   372,   195,
      32,    75,   366,   140,   660,   195,    80,   389,   143,    75,
     140,    75,   671,    75,    80,   389,   373,   673,   674,   675,
      75,  1104,   140,   141,   142,    80,   143,   198,   392,   411,
     165,   195,   396,     4,   140,   141,   142,   411,   420,   595,
     147,   423,    75,    75,   119,  1157,   195,    80,   165,   423,
       4,   195,   816,    75,   141,   142,    75,   400,    80,   195,
     195,   786,   436,   198,   620,   408,   601,   141,   142,   436,
     795,    42,   197,   198,   140,   141,   142,    94,    95,    96,
     636,   198,  1165,   635,    32,   791,   141,   142,    42,   641,
    1067,   635,  1069,   436,   158,   195,   158,   641,   862,   108,
     109,   110,   111,   112,   113,   195,  1489,   195,   141,   142,
     149,   485,   195,   769,   671,   672,   559,   204,   483,   141,
     142,  1504,   141,   142,    27,   681,   500,    75,   198,   203,
     101,   641,   506,   507,   195,   106,   147,   108,   109,   110,
     111,   112,   113,   114,    32,    26,   197,   101,    14,    46,
      47,    48,   106,    50,   108,   109,   110,   111,   112,   113,
     114,    27,    43,   197,   198,    46,   841,   176,    94,    95,
      96,  1396,  1397,   754,  1392,  1393,   899,   197,    44,   197,
     151,   152,   453,   154,    62,   559,  1163,   197,   197,   770,
     954,   108,   109,   110,   197,   752,   197,   151,   152,    62,
     154,   115,   116,   117,   197,   176,   763,   863,   551,    62,
     935,   867,   198,   869,   143,   977,   195,   195,   943,   490,
     143,    62,   176,   195,    41,   147,  1495,    50,   199,   165,
     143,   887,   123,   202,     9,   143,   143,  1001,   195,   123,
       8,  1510,   100,   195,  1008,   199,   197,   758,    14,   165,
     108,   109,   110,   111,   112,   113,   630,    14,   632,   602,
      75,  1023,  1239,   197,   654,   197,   108,   109,   110,   111,
     112,   113,   285,    14,    81,   649,   289,   119,   120,   165,
      14,   108,   109,   110,   111,   112,   113,   661,   662,    92,
     196,   848,   649,   850,   637,   662,   639,   197,   311,   649,
     313,   314,   315,   316,   196,   201,   649,   196,   195,   100,
     195,  1075,   195,     9,   196,   157,   659,   196,   176,   662,
      84,     9,   197,  1048,   980,    14,    81,   195,     9,   703,
     181,  1056,   706,   890,   176,    75,    75,    75,   994,  1064,
     996,  1016,    75,   195,   184,   926,   703,   790,   929,   176,
     196,   196,   121,   703,   197,   912,   195,   914,   196,    62,
     703,   122,   736,   919,   164,   736,   736,     9,   736,   736,
     713,   124,   196,  1465,    14,   193,   758,     9,    62,   736,
     196,   724,   725,     9,   758,    14,   736,   199,  1044,   660,
    1482,   121,   202,   736,   202,     9,   195,  1374,  1490,   178,
     179,   180,   673,   674,   202,  1080,   185,   186,   195,   791,
     189,   190,   196,   202,   195,   197,   790,   197,   196,  1144,
    1095,     9,   124,   196,   195,   143,   143,   181,   802,   803,
     804,   198,   195,   195,   991,   181,    14,  1118,   828,     9,
    1021,    75,   198,    14,    92,   197,    14,  1103,  1029,   198,
     198,   202,  1108,    14,  1110,   829,    14,   831,    27,   833,
     197,     9,   829,    43,    44,    45,    46,    47,    48,   195,
      50,   195,   195,  1054,   831,   195,   833,   196,   124,   195,
     195,   831,    14,   833,   197,  1141,   829,   899,   831,   197,
     833,   865,   835,  1168,   868,  1413,   195,     9,   769,  1224,
     124,  1176,   196,   196,   202,     9,     9,   140,    75,   195,
    1185,   124,   197,    75,    14,    75,   195,   891,   196,   195,
     195,   198,   196,  1179,   195,   198,   198,   196,   124,   903,
       9,  1212,   903,   903,   202,   903,   903,   140,    27,    69,
     197,   196,  1099,   197,   166,    27,   903,   196,   124,     9,
      70,    71,    72,   903,   199,   196,     9,   196,  1115,   124,
     903,    81,  1480,    92,   199,  1221,   196,  1148,    14,  1244,
     195,  1128,   195,  1154,  1155,   196,  1157,   967,   196,   922,
     196,   955,   124,   198,   196,   959,   960,   196,   196,     9,
     196,    27,   197,   197,  1250,  1251,   970,   196,   869,   197,
    1256,   124,   153,   970,  1130,   198,     4,   981,   128,   129,
     130,   131,   132,   149,    75,    14,   887,    75,   195,   139,
     106,   964,   196,   196,   981,   145,   146,   970,  1128,   124,
     196,   981,   196,   196,   977,   978,   124,   124,   981,    14,
     160,   198,   197,    75,    42,    14,   196,   124,   195,   198,
     196,    14,   197,   197,   174,  1236,    14,   196,   198,    52,
      75,   195,    75,    24,    25,     9,   197,    28,    24,    25,
      75,  1021,    28,   104,    92,  1210,   143,    30,    92,  1029,
    1023,   156,    14,   196,   195,    75,   195,   197,   162,    50,
    1365,   159,  1367,  1067,   196,  1069,     9,    75,   197,   196,
    1375,    75,   196,   101,  1360,   198,    14,    75,   106,   980,
     108,   109,   110,   111,   112,   113,   114,    14,    75,    14,
      75,    14,  1473,   994,   483,   996,   789,   370,   372,   157,
     787,   739,  1486,   371,  1092,  1391,  1482,   978,   488,  1261,
    1114,  1416,  1345,  1123,  1118,  1514,  1402,  1502,  1357,  1123,
      39,  1407,  1231,   151,   152,   913,   154,  1114,   375,   880,
     467,   877,   467,   700,  1114,   906,   941,   803,   818,   283,
    1127,  1114,   851,  1044,    -1,   276,    -1,    -1,   176,    -1,
    1130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1163,
      -1,    -1,    -1,    -1,    -1,  1169,    -1,    -1,  1148,  1173,
      -1,   199,    -1,  1177,  1154,  1155,  1173,    -1,  1464,    -1,
      -1,    -1,  1169,    -1,    -1,    -1,    -1,    -1,    -1,  1169,
    1177,    -1,    -1,    -1,  1359,  1199,  1169,  1177,    -1,    -1,
    1173,  1205,  1103,  1508,  1177,    -1,    -1,  1108,  1212,  1110,
    1515,   202,    -1,    -1,    -1,    -1,   202,   208,    -1,    -1,
      -1,    -1,   208,   214,    -1,    -1,    -1,  1214,   214,    -1,
      -1,    -1,    -1,  1519,    -1,  1239,    -1,    -1,  1242,  1243,
    1141,  1527,    -1,  1247,    -1,  1531,  1243,    -1,  1534,  1253,
      -1,   242,   243,    -1,  1465,  1242,  1236,   248,    -1,    -1,
    1247,    -1,  1242,    -1,    -1,    -1,  1253,  1247,    -1,  1242,
    1243,  1482,    -1,  1253,  1247,    -1,    -1,   268,  1179,  1490,
    1253,    -1,   268,  1356,   275,   276,    -1,    -1,    -1,   275,
     276,   282,    -1,    -1,    -1,    -1,   282,   288,    -1,    -1,
      -1,    -1,   288,    -1,    -1,  1461,    -1,    -1,  1495,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1221,    -1,    -1,  1510,    -1,    -1,    -1,    -1,   319,    -1,
      -1,   322,    -1,    24,    25,  1408,    -1,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1477,    -1,  1250,
    1251,    -1,  1356,    -1,    -1,  1256,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
    1374,    -1,   363,    -1,  1378,    -1,    -1,   363,    -1,    -1,
      -1,    -1,    -1,  1387,    -1,    -1,    -1,    -1,  1392,  1393,
      -1,  1378,  1396,  1397,    -1,    -1,    -1,    -1,  1378,    -1,
      -1,    -1,    -1,    -1,  1408,  1378,    -1,    -1,    -1,   400,
    1414,  1415,    -1,    -1,   400,    -1,  1420,   408,    -1,    -1,
      -1,    -1,   408,    -1,    -1,    -1,    -1,  1414,  1415,    -1,
      -1,    -1,    -1,  1420,  1414,  1415,    -1,    -1,    -1,    -1,
    1420,  1414,  1415,    -1,    -1,    -1,    -1,  1420,     4,  1453,
      -1,    -1,    -1,    -1,    -1,    -1,  1460,   448,    -1,  1360,
      -1,    -1,   453,    -1,    -1,     4,  1453,    -1,    -1,    -1,
      -1,    -1,  1476,  1453,    -1,    -1,    -1,    -1,    -1,    -1,
    1453,  1461,    -1,    -1,    -1,    -1,    42,    -1,    -1,    -1,
    1391,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   490,
      -1,  1402,    -1,    42,    -1,    -1,  1407,    -1,    -1,    -1,
      -1,    -1,  1516,    -1,    -1,    -1,    -1,   208,  1522,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1516,
      -1,    -1,    -1,    -1,    -1,  1522,  1516,    -1,   529,   530,
      -1,    -1,  1522,  1516,   535,   101,    -1,    -1,    -1,  1522,
     106,    -1,   108,   109,   110,   111,   112,   113,   114,    -1,
     551,    50,   101,  1464,    -1,   551,    -1,   106,    -1,   108,
     109,   110,   111,   112,   113,   114,    -1,   268,    -1,    -1,
      -1,    -1,    -1,    -1,   275,   276,    -1,    -1,    -1,    -1,
      -1,   282,    -1,     4,    -1,   151,   152,   288,   154,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   602,   151,   152,    -1,   154,   602,    -1,  1519,    -1,
     176,    -1,    -1,    -1,    -1,    -1,  1527,    -1,    -1,    -1,
    1531,    42,    -1,  1534,    -1,    -1,    -1,   176,    -1,    -1,
      -1,    -1,    -1,   199,    -1,    -1,   637,    -1,   639,    -1,
      -1,   637,    -1,   639,    -1,    -1,    -1,    -1,    -1,    -1,
     199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   659,   660,
      -1,    -1,   363,   659,    -1,    -1,    -1,    -1,    -1,    -1,
     671,   672,   673,   674,   675,    -1,    -1,    -1,    -1,    -1,
     101,    -1,    -1,    -1,    -1,   106,    -1,   108,   109,   110,
     111,   112,   113,   114,    -1,    -1,    -1,    -1,    -1,   400,
      -1,    -1,    -1,   704,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   713,    -1,    -1,    -1,    -1,   713,    -1,    -1,
      -1,   722,    -1,   724,   725,    -1,    -1,    -1,   724,   725,
     151,   152,    -1,   154,    -1,    -1,    -1,    -1,   739,    -1,
      -1,    -1,    -1,   242,   243,    -1,    -1,    -1,    -1,   248,
      -1,   752,   453,    -1,    -1,   176,    -1,    -1,    -1,    -1,
      -1,    -1,   763,    -1,    -1,    -1,    -1,    -1,   769,    -1,
      -1,   772,    -1,    -1,    -1,    -1,    -1,    -1,   199,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,   490,
     791,    -1,    -1,    -1,    -1,   791,    -1,    26,    -1,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
     319,    50,    -1,   322,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   835,    -1,    -1,    -1,    -1,   835,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   848,    -1,   850,
     551,    -1,    -1,    -1,    -1,   453,    -1,    -1,    -1,    -1,
      -1,    -1,   863,    -1,    -1,    -1,   867,    -1,   869,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    -1,    -1,    -1,    -1,    -1,   887,    -1,    -1,   890,
      -1,    -1,   490,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   602,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   912,    -1,   914,    -1,    60,    61,    -1,    -1,    -1,
      -1,   922,    -1,    -1,    -1,    -1,   922,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   637,    -1,   639,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   448,
      -1,    -1,    -1,    -1,   453,    -1,    -1,    -1,   659,   660,
      -1,    -1,    -1,   964,    -1,    -1,    -1,    -1,   964,    -1,
     199,    -1,   673,   674,   675,    -1,   977,   978,   123,   980,
      -1,   977,   978,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     991,   490,    -1,   994,    -1,   996,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   704,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   713,    -1,    -1,    -1,    -1,    -1,    -1,  1020,
      -1,   722,  1023,   724,   725,    -1,    -1,  1023,    -1,    -1,
     529,   530,    -1,    -1,    -1,    -1,   535,    -1,   739,    -1,
      -1,    -1,    -1,  1044,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   660,    -1,    -1,    -1,    -1,    -1,   769,    -1,
      -1,   772,    -1,    -1,    -1,   673,   674,   675,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     791,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1099,    -1,
      -1,    -1,  1103,    -1,    -1,    -1,    -1,  1108,    -1,  1110,
      -1,    -1,    -1,    -1,  1115,    -1,    -1,  1118,  1119,    -1,
    1121,    -1,    -1,    -1,    -1,    -1,    -1,  1128,    -1,    -1,
      -1,    -1,    -1,    -1,   835,    -1,    -1,    -1,    -1,    -1,
    1141,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    -1,    -1,    -1,    -1,    -1,
      -1,   660,   863,    -1,    -1,    -1,   867,    -1,   869,    -1,
      -1,   769,   671,   672,   673,   674,   675,    -1,  1179,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   887,  1188,  1189,    60,
      61,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,  1212,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1221,   922,    -1,    26,    -1,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    -1,  1250,
    1251,    -1,   123,   752,    -1,  1256,  1257,    -1,    -1,    -1,
    1261,    -1,    -1,   964,   763,   863,    -1,    -1,    -1,   867,
     769,   869,    -1,    -1,    -1,    -1,   977,   978,    -1,   980,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,   887,
      -1,    -1,    -1,   994,    -1,   996,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,  1020,
      50,    -1,  1023,    -1,    -1,   196,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    -1,    -1,
      -1,    -1,    -1,  1044,    -1,    -1,    -1,    -1,    -1,   848,
      -1,   850,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1360,
      -1,    -1,    -1,    -1,   863,    -1,    -1,    -1,   867,    -1,
     869,    -1,    60,    61,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   980,    -1,    -1,    -1,  1387,    -1,   887,    -1,
    1391,   890,    -1,    -1,    -1,    -1,   994,    -1,   996,   202,
      -1,  1402,  1103,    -1,    -1,    -1,  1407,  1108,    -1,  1110,
      -1,    -1,    -1,   912,    -1,   914,    -1,    -1,  1119,    -1,
    1121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    -1,    -1,   123,    -1,    -1,    -1,    -1,
    1141,    -1,    -1,    -1,    -1,    -1,  1044,    62,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,  1464,    -1,    -1,    81,    -1,    -1,   199,
      -1,  1472,    -1,    -1,    -1,    -1,    -1,    -1,  1179,    -1,
      -1,   980,    -1,    -1,    -1,  1486,    -1,  1188,  1189,    -1,
      -1,    -1,   991,    -1,  1495,   994,    -1,   996,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1103,    -1,    -1,    -1,  1510,
    1108,   126,  1110,    -1,    -1,    -1,    -1,    -1,  1519,    -1,
    1221,    -1,    -1,    -1,   139,    -1,  1527,    -1,    -1,    -1,
    1531,    -1,    -1,  1534,    -1,    -1,   151,   152,    -1,   154,
     155,   156,    -1,  1141,    -1,  1044,    -1,    63,    64,  1250,
    1251,    -1,    -1,    -1,   169,  1256,  1257,    73,    -1,    75,
    1261,    -1,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,    -1,    -1,    -1,
      -1,  1179,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,
    1099,    -1,    -1,    -1,  1103,    -1,    -1,    -1,    -1,  1108,
      -1,  1110,    -1,    -1,    -1,    -1,  1115,    -1,    -1,  1118,
      -1,    -1,    -1,  1221,    -1,    -1,    -1,    -1,   144,  1128,
      -1,   147,    -1,   149,    -1,   151,   152,    -1,   154,   155,
     156,    -1,  1141,    -1,    -1,    -1,    -1,   163,    -1,    -1,
      -1,    -1,  1250,  1251,    -1,    -1,    -1,    -1,  1256,  1360,
      -1,   177,  1260,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,    -1,    -1,    -1,   195,
    1179,    -1,    -1,    -1,   200,    -1,    -1,    -1,    -1,    -1,
    1391,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1402,    -1,    -1,    -1,    -1,  1407,    -1,    -1,    -1,
      -1,    -1,    -1,  1212,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1221,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     5,     6,    -1,     8,     9,    10,    -1,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,  1250,  1251,    26,    27,    -1,    -1,  1256,    -1,    -1,
      -1,    -1,  1360,  1464,    -1,    -1,    39,    -1,    -1,    -1,
      -1,  1472,    -1,    46,    -1,    48,    -1,    -1,    51,    -1,
      53,    -1,    -1,    -1,    -1,  1486,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1391,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1402,    -1,    -1,    80,    -1,  1407,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1519,    -1,
      -1,    94,    -1,    -1,    -1,    -1,  1527,    -1,    -1,    -1,
    1531,    -1,  1430,  1534,    -1,    -1,    -1,    -1,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1360,    -1,    -1,    -1,    26,  1464,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,  1387,    50,
      -1,    -1,  1391,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1402,    -1,    -1,    -1,    -1,  1407,   182,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1519,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1527,
      -1,    -1,    -1,  1531,    -1,    -1,  1534,    -1,    -1,    -1,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,    -1,
     223,    -1,    -1,   226,    10,    11,    12,    -1,    -1,    -1,
     233,   234,    -1,    -1,    -1,  1464,    -1,    -1,    -1,    -1,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    -1,  1495,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   277,    -1,    -1,    -1,    -1,    -1,
     283,  1510,    -1,    -1,   287,    -1,    -1,    -1,    -1,    -1,
    1519,   151,   152,    -1,   154,   155,   156,    -1,  1527,    -1,
      -1,    -1,  1531,    -1,    -1,  1534,    -1,   310,   199,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   177,   321,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,    -1,    -1,    -1,   338,   339,   340,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
      -1,    -1,   365,   366,    -1,   368,    -1,    -1,    -1,    -1,
      -1,    -1,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   386,    -1,    -1,    -1,    -1,    -1,   392,
     393,    -1,   395,   396,   397,    -1,    -1,    -1,    -1,    -1,
     403,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     413,    -1,   415,   199,    -1,    10,    11,    12,   421,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   431,    -1,
     433,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    -1,   460,    -1,    -1,
     463,   464,   465,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   486,    26,    -1,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   521,    -1,
      -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     563,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      -1,   594,    70,    71,    72,    73,    74,    75,    -1,    77,
     603,    -1,   197,    81,    82,    83,    84,    -1,    86,    -1,
      88,    -1,    90,    -1,    -1,    93,   619,    -1,    -1,    97,
      98,    99,   100,    -1,   102,   103,    -1,   105,    -1,   107,
     108,   109,   110,   111,   112,   113,    -1,   115,   116,   117,
      -1,    -1,   645,    -1,    -1,    -1,    -1,   125,   126,    -1,
     128,   129,   130,   131,   132,   658,    -1,   199,    -1,    -1,
      -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,
     148,   149,    -1,   151,   152,    -1,   154,   155,   156,    -1,
      -1,    -1,   160,    -1,    -1,   163,   689,    -1,    -1,    -1,
      -1,   169,    -1,    -1,    -1,    -1,   174,   175,   176,   177,
      -1,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,    -1,    -1,   195,    -1,   197,
     198,    -1,   200,   201,    -1,   203,   204,    -1,    -1,    -1,
     733,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   746,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   767,    -1,    -1,    64,    -1,    -1,
      -1,    -1,    -1,    -1,   777,    -1,    73,   780,    75,   782,
      -1,    -1,    -1,   786,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    61,   795,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,    26,   821,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,    -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,
     147,    -1,   149,   123,   151,   152,    -1,   154,   155,   156,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   871,   872,
     873,    -1,    -1,    -1,   877,   878,    -1,    -1,    -1,    -1,
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,    -1,   899,    -1,   195,    -1,
      -1,    -1,    -1,   200,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   918,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,   935,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     943,    -1,   945,    -1,    -1,    26,    -1,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      -1,    -1,   975,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,   984,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     199,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    -1,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,  1032,
      -1,    -1,    -1,  1036,    -1,  1038,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1048,    -1,    -1,    -1,    -1,
      -1,    -1,  1055,  1056,    -1,    -1,    42,    43,    44,    -1,
      -1,  1064,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      -1,    77,    -1,    -1,    -1,    81,    82,    83,    84,    -1,
      86,    -1,    88,    -1,    90,    -1,    -1,    93,    -1,  1112,
      -1,    97,    98,    99,   100,   101,   102,   103,   199,   105,
     106,   107,   108,   109,   110,   111,   112,   113,    -1,   115,
     116,   117,   118,   119,   120,    -1,    -1,    -1,    -1,   125,
     126,  1144,   128,   129,   130,   131,   132,    -1,    -1,    -1,
      -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,
     146,   147,   148,   149,   199,   151,   152,    -1,   154,   155,
     156,   157,    -1,    -1,   160,    -1,    -1,   163,    -1,    -1,
      -1,    -1,    -1,   169,   170,    -1,   172,    -1,   174,   175,
     176,   177,    -1,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,    -1,    -1,   195,
      -1,   197,   198,   199,   200,   201,    -1,   203,   204,    -1,
      -1,  1224,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,  1252,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    75,    -1,    77,    -1,    -1,    -1,
      81,    82,    83,    84,    -1,    86,    -1,    88,    -1,    90,
      -1,    -1,    93,    -1,    -1,    -1,    97,    98,    99,   100,
     101,   102,   103,    -1,   105,   106,   107,   108,   109,   110,
     111,   112,   113,    -1,   115,   116,   117,   118,   119,   120,
      -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,
     131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,
      -1,    -1,    -1,   144,   145,   146,   147,   148,   149,    -1,
     151,   152,    -1,   154,   155,   156,   157,    -1,    -1,   160,
      -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,   170,
      -1,   172,    -1,   174,   175,   176,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,    -1,    -1,   195,    -1,   197,   198,   199,   200,
     201,    12,   203,   204,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    26,    -1,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      -1,    -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    75,    -1,    77,    -1,
      -1,    -1,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    90,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,
      99,   100,   101,   102,   103,    -1,   105,   106,   107,   108,
     109,   110,   111,   112,   113,    -1,   115,   116,   117,   118,
     119,   120,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,
     129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,   148,
     149,    -1,   151,   152,    -1,   154,   155,   156,   157,    -1,
      -1,   160,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,
     169,   170,    -1,   172,    -1,   174,   175,   176,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,    -1,    -1,   195,    -1,   197,   198,
      -1,   200,   201,    -1,   203,   204,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    26,    13,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
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
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,    -1,    -1,   195,    -1,
     197,   198,   199,   200,   201,    -1,   203,   204,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,
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
     175,   176,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    -1,    -1,
     195,    -1,   197,   198,   199,   200,   201,    -1,   203,   204,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    42,
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
      -1,   174,   175,   176,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
      -1,    -1,   195,    -1,   197,   198,   199,   200,   201,    -1,
     203,   204,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    75,    -1,    77,    -1,    -1,    -1,
      81,    82,    83,    84,    85,    86,    -1,    88,    -1,    90,
      -1,    -1,    93,    -1,    -1,    -1,    97,    98,    99,   100,
      -1,   102,   103,    -1,   105,    -1,   107,   108,   109,   110,
     111,   112,   113,    -1,   115,   116,   117,    -1,   119,   120,
      -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,
     131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,
      -1,    -1,    -1,   144,   145,   146,   147,   148,   149,    -1,
     151,   152,    -1,   154,   155,   156,   157,    -1,    -1,   160,
      -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,
      -1,    -1,    -1,   174,   175,   176,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,    -1,    -1,   195,    -1,   197,   198,    -1,   200,
     201,    -1,   203,   204,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    75,    -1,    77,    -1,
      -1,    -1,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    90,    91,    -1,    93,    -1,    -1,    -1,    97,    98,
      99,   100,    -1,   102,   103,    -1,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,    -1,   115,   116,   117,    -1,
     119,   120,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,
     129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,   148,
     149,    -1,   151,   152,    -1,   154,   155,   156,   157,    -1,
      -1,   160,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,
     169,    -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,    -1,    -1,   195,    -1,   197,   198,
      -1,   200,   201,    -1,   203,   204,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,
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
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,    -1,    -1,   195,    -1,
     197,   198,   199,   200,   201,    -1,   203,   204,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,
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
     175,   176,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    -1,    -1,
     195,    -1,   197,   198,   199,   200,   201,    -1,   203,   204,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    75,    -1,    77,    -1,    -1,    -1,    81,    82,
      83,    84,    -1,    86,    -1,    88,    89,    90,    -1,    -1,
      93,    -1,    -1,    -1,    97,    98,    99,   100,    -1,   102,
     103,    -1,   105,    -1,   107,   108,   109,   110,   111,   112,
     113,    -1,   115,   116,   117,    -1,   119,   120,    -1,    -1,
      -1,    -1,   125,   126,    -1,   128,   129,   130,   131,   132,
      -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,
      -1,   144,   145,   146,   147,   148,   149,    -1,   151,   152,
      -1,   154,   155,   156,   157,    -1,    -1,   160,    -1,    -1,
     163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,
      -1,   174,   175,   176,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
      -1,    -1,   195,    -1,   197,   198,    -1,   200,   201,    -1,
     203,   204,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
      -1,    -1,    -1,   174,   175,   176,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,    -1,    -1,   195,    -1,   197,   198,   199,   200,
     201,    -1,   203,   204,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    75,    -1,    77,    -1,
      -1,    -1,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    90,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,
      99,   100,    -1,   102,   103,    -1,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,    -1,   115,   116,   117,    -1,
     119,   120,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,
     129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,   148,
     149,    -1,   151,   152,    -1,   154,   155,   156,   157,    -1,
      -1,   160,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,
     169,    -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,    -1,    -1,   195,    -1,   197,   198,
     199,   200,   201,    -1,   203,   204,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,    44,    -1,    -1,
      -1,    -1,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    75,    -1,
      77,    -1,    -1,    -1,    81,    82,    83,    84,    -1,    86,
      87,    88,    -1,    90,    -1,    -1,    93,    -1,    -1,    -1,
      97,    98,    99,   100,    -1,   102,   103,    -1,   105,    -1,
     107,   108,   109,   110,   111,   112,   113,    -1,   115,   116,
     117,    -1,   119,   120,    -1,    -1,    -1,    -1,   125,   126,
      -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,
      -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,
     147,   148,   149,    -1,   151,   152,    -1,   154,   155,   156,
     157,    -1,    -1,   160,    -1,    -1,   163,    -1,    -1,    -1,
      -1,    -1,   169,    -1,    -1,    -1,    -1,   174,   175,   176,
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,    -1,    -1,   195,    -1,
     197,   198,    -1,   200,   201,    -1,   203,   204,     3,     4,
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
     175,   176,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    -1,    -1,
     195,    -1,   197,   198,   199,   200,   201,    -1,   203,   204,
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
      -1,   174,   175,   176,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
      -1,    -1,   195,    -1,   197,   198,   199,   200,   201,    -1,
     203,   204,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
      -1,    -1,    -1,   174,   175,   176,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,    -1,    -1,   195,    -1,   197,   198,   199,   200,
     201,    -1,   203,   204,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    75,    -1,    77,    -1,
      -1,    -1,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    90,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,
      99,   100,    -1,   102,   103,    -1,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,    -1,   115,   116,   117,    -1,
     119,   120,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,
     129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,   148,
     149,    -1,   151,   152,    -1,   154,   155,   156,   157,    -1,
      -1,   160,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,
     169,    -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,    -1,    -1,   195,    -1,   197,   198,
      -1,   200,   201,    -1,   203,   204,     3,     4,     5,     6,
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
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,    -1,    -1,   195,    -1,
     197,   198,    -1,   200,   201,    -1,   203,   204,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      75,    -1,    77,    -1,    -1,    -1,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    90,    -1,    -1,    93,    -1,
      -1,    -1,    97,    98,    99,   100,    -1,   102,   103,    -1,
     105,    -1,   107,   108,   109,   110,   111,   112,   113,    -1,
     115,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     125,   126,    -1,   128,   129,   130,   131,   132,    -1,    -1,
      -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,
     145,   146,   147,   148,   149,    -1,   151,   152,    -1,   154,
     155,   156,    -1,    -1,    -1,   160,    -1,    -1,   163,    -1,
      -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,
     175,   176,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    -1,    -1,
     195,    -1,   197,   198,    -1,   200,   201,    -1,   203,   204,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    75,    -1,    77,    -1,    -1,    -1,    81,    82,
      83,    84,    -1,    86,    -1,    88,    -1,    90,    -1,    -1,
      93,    -1,    -1,    -1,    97,    98,    99,   100,    -1,   102,
     103,    -1,   105,    -1,   107,   108,   109,   110,   111,   112,
     113,    -1,   115,   116,   117,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   125,   126,    -1,   128,   129,   130,   131,   132,
      -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,
      -1,   144,   145,   146,   147,   148,   149,    -1,   151,   152,
      -1,   154,   155,   156,    -1,    -1,    -1,   160,    -1,    -1,
     163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,
      -1,   174,   175,   176,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
      -1,    -1,   195,    -1,   197,   198,    -1,   200,   201,    -1,
     203,   204,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
      -1,    -1,    -1,   174,   175,   176,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,    -1,    -1,   195,    -1,   197,   198,    -1,   200,
     201,    -1,   203,   204,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     169,    -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,    -1,    -1,   195,    -1,   197,   198,
      -1,   200,   201,    -1,   203,   204,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,
      -1,    -1,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    75,    -1,
      -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,   110,   111,   112,   113,    -1,    73,   116,
     117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,   126,
      -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,
      -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,
     147,   148,   149,    -1,   151,   152,    -1,   154,   155,   156,
      -1,    -1,   117,   160,    -1,    -1,   163,    -1,    -1,    -1,
      -1,    -1,   169,    -1,    -1,    -1,    -1,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   151,   152,   195,   154,
     155,   156,    -1,   200,   201,    -1,   203,   204,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,    -1,    -1,    -1,
     195,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
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
     185,   186,   187,   188,   189,   190,   191,   192,    -1,    -1,
     195,    -1,   197,    -1,    -1,   200,   201,    -1,   203,   204,
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
     113,    -1,    73,   116,   117,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   125,   126,    -1,   128,   129,   130,   131,   132,
      -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,
      -1,   144,   145,   146,   147,   148,   149,    -1,   151,   152,
      -1,   154,   155,   156,    -1,    -1,    -1,   160,    -1,    -1,
     163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,
      -1,   174,   175,   176,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     151,   152,   195,   154,   155,   156,    -1,   200,   201,    -1,
     203,   204,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,    32,    -1,    -1,   195,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,
     111,   112,   113,    -1,    73,   116,   117,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,
     131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,
      -1,    -1,    -1,   144,   145,   146,   147,    -1,   149,    -1,
     151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,   160,
      -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,
      -1,    -1,    -1,   174,    -1,   176,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   151,   152,   195,   154,   155,   156,    -1,   200,
     201,    -1,   203,   204,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,    -1,    -1,    -1,   195,    -1,    -1,    -1,
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
     169,    -1,    -1,    -1,    -1,   174,    -1,   176,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,    -1,    -1,   195,    -1,   197,    -1,
      -1,   200,   201,    -1,   203,   204,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,    -1,    -1,   195,    -1,
     197,    -1,    -1,   200,   201,    -1,   203,   204,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      75,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    97,    -1,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,   110,   111,   112,   113,    -1,
      73,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     125,   126,    -1,   128,   129,   130,   131,   132,    -1,    -1,
      -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,
     145,   146,   147,    -1,   149,    -1,   151,   152,    -1,   154,
     155,   156,    -1,    -1,    -1,   160,   119,    -1,   163,    -1,
      -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,
      -1,   176,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   151,   152,
     195,   154,   155,   156,    -1,   200,   201,    -1,   203,   204,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,    -1,
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
      -1,   174,    -1,   176,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
      -1,    -1,   195,   196,    -1,    -1,    -1,   200,   201,    -1,
     203,   204,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
      -1,    -1,    -1,   174,    -1,   176,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,    -1,    -1,   195,    -1,    -1,    -1,    -1,   200,
     201,    -1,   203,   204,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    75,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,   110,   111,   112,   113,    -1,    73,   116,   117,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,
     129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,    -1,
     149,    -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,
      -1,   160,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,
     169,    -1,    -1,    -1,    -1,   174,    -1,   176,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   151,   152,   195,   154,   155,   156,
      -1,   200,   201,    -1,   203,   204,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,    32,    -1,    -1,    -1,    -1,
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
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,    -1,    -1,   195,    -1,
      -1,    -1,    -1,   200,   201,    -1,   203,   204,     3,     4,
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
      -1,   176,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    -1,    -1,
     195,    -1,    -1,    -1,    -1,   200,   201,    -1,   203,   204,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,
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
      -1,   174,    -1,   176,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
      -1,    -1,   195,    -1,    -1,    -1,    -1,   200,   201,    -1,
     203,   204,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
      -1,    -1,    -1,   174,    -1,   176,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,    -1,    -1,   195,    -1,    -1,   198,    -1,   200,
     201,    -1,   203,   204,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,
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
     169,    -1,    -1,    -1,    -1,   174,    -1,   176,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,    -1,    -1,   195,    -1,    -1,    -1,
      -1,   200,   201,    -1,   203,   204,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,    -1,
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
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,    -1,    -1,   195,    -1,
      -1,    -1,    -1,   200,   201,    -1,   203,   204,     3,     4,
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
      -1,   176,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    -1,    -1,
     195,    -1,    -1,    -1,    -1,   200,   201,    -1,   203,   204,
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
      -1,   174,    -1,   176,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
      -1,    -1,   195,    10,    11,    12,    -1,   200,   201,    -1,
     203,   204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
      -1,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    -1,    50,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    26,    -1,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,     3,     4,     5,     6,     7,
      -1,    -1,    10,    11,    12,    13,    -1,    26,    -1,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,   199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    63,    64,    65,    66,    67,
      68,    69,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   199,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   197,    -1,   125,   126,    -1,
     128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   144,   145,   146,    -1,
     148,    -1,    -1,   151,   152,    -1,   154,   155,   156,   157,
      -1,   159,   160,    -1,   162,    -1,    -1,    -1,    -1,    -1,
      -1,   169,   170,    -1,   172,   184,   174,   175,   176,   177,
      -1,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,    -1,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,    -1,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,   197,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,   197,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,   197,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,   197,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    73,    -1,    75,    -1,    -1,    -1,    -1,    -1,    26,
     196,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    -1,    50,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,
      26,   196,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    26,    50,    -1,    -1,    -1,    -1,   151,
     152,    -1,   154,   155,   156,    -1,    -1,   191,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    52,
      -1,    -1,    -1,    -1,    -1,   177,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
      73,    -1,    -1,    -1,   187,   188,   198,    -1,   200,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,   112,
     113,    -1,    -1,    -1,    -1,    52,   183,    -1,    -1,    -1,
      -1,    -1,    -1,   126,   127,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,
      -1,   144,    -1,    -1,   147,    -1,   149,    -1,   151,   152,
      -1,   154,   155,   156,    -1,    -1,   182,    -1,    -1,    -1,
      -1,    -1,    -1,   100,    -1,    -1,   169,    -1,    -1,    26,
      -1,    -1,    -1,   176,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   126,
     127,    -1,   195,    -1,    -1,    52,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,
     147,    -1,   149,    -1,   151,   152,    73,   154,   155,   156,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   169,    -1,    -1,    26,    -1,    -1,    -1,    -1,
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,    -1,    -1,    -1,   195,    -1,
      -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   126,
     127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    73,    -1,    -1,    -1,    -1,   144,    -1,    -1,
     147,    -1,   149,    -1,   151,   152,    -1,   154,   155,   156,
      -1,   158,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   169,    -1,    -1,    26,    -1,    -1,    -1,    -1,
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   126,   127,    -1,   195,    -1,
      -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   144,    -1,    -1,   147,    -1,   149,    -1,
     151,   152,    73,   154,   155,   156,    -1,   158,    -1,    -1,
      32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   169,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,    -1,    -1,    -1,   195,    -1,    -1,    -1,    -1,    -1,
      -1,    73,    -1,    75,    -1,   126,   127,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   144,    -1,    -1,   147,    -1,   149,    -1,
     151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,    -1,
      -1,   113,    -1,    -1,    -1,    -1,    -1,    -1,   169,    -1,
      -1,    -1,    -1,   125,    -1,    -1,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,    -1,   144,    -1,   195,   147,    -1,   149,    -1,   151,
     152,    -1,   154,   155,   156,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    -1,   177,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
      -1,    62,    -1,   195,    -1,    -1,    -1,    -1,   200,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      73,    50,    75,    -1,    -1,   126,    -1,   128,   129,   130,
     131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,
      -1,    -1,    -1,   144,   145,   146,   147,    -1,   149,    -1,
     151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,   160,
     113,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   169,    -1,
      73,    -1,    75,   174,    -1,    -1,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   144,    -1,    -1,   147,   124,   149,    -1,   151,   152,
      -1,   154,   155,   156,    -1,    -1,    -1,    -1,    -1,    -1,
     113,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   125,    -1,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,    -1,
      -1,   144,   195,    -1,   147,   198,   149,   200,   151,   152,
      -1,   154,   155,   156,    73,    -1,    75,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,    -1,
      -1,    -1,   195,    -1,   113,    -1,    -1,   200,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   125,    -1,    73,    -1,
      75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,
     149,    -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,    73,    -1,    75,   195,    -1,    -1,   144,
      -1,   200,   147,    -1,   149,    -1,   151,   152,    -1,   154,
     155,   156,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   177,   113,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,    73,    -1,    75,
     195,    -1,    -1,    -1,    -1,   200,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,   149,
      -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,
      75,    -1,    -1,    -1,    -1,    -1,    -1,   177,    -1,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,    -1,    -1,    -1,   195,    -1,    -1,    -1,    -1,
     200,    -1,    -1,    -1,    -1,   151,   152,    -1,   154,   155,
     156,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,
      75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   177,    -1,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   151,   152,    -1,   154,
     155,   156,   198,    -1,   200,    -1,    -1,    -1,    -1,    73,
      -1,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,    -1,    -1,    -1,
      -1,    -1,    -1,   198,    -1,   200,   151,   152,    -1,   154,
     155,   156,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   151,   152,    -1,
     154,   155,   156,   198,    -1,   200,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   177,    -1,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,    -1,    -1,
     144,    -1,    -1,   147,   198,    -1,   200,   151,   152,    -1,
     154,   155,   156,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   177,    -1,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,    -1,   147,
      -1,    -1,    -1,   151,   152,   199,   154,   155,   156,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,   177,
      -1,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,    -1,    -1,    -1,    -1,    -1,   149,
     198,   151,   152,   153,   154,   155,   156,    -1,    -1,    -1,
      -1,    -1,    -1,   114,    -1,    -1,    -1,    -1,    -1,    -1,
      73,    -1,    -1,    -1,    -1,   126,    -1,   177,    -1,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,    -1,   144,    -1,   195,   147,    -1,   149,    -1,
     151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,    -1,
      -1,    73,    -1,    75,    76,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   177,    73,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   144,    -1,    -1,   147,    -1,   149,    -1,   151,   152,
      -1,   154,   155,   156,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      73,    -1,    -1,    -1,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   151,
     152,    -1,   154,   155,   156,    -1,    -1,    -1,    -1,    -1,
      -1,   147,    -1,    -1,    -1,   151,   152,    -1,   154,   155,
     156,    -1,    -1,    -1,    -1,   177,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
      -1,   177,    -1,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   149,    -1,   151,   152,
      -1,   154,   155,   156,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,   124,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,   124,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
     124,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    -1,    50,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    92,    50,    10,    11,    12,
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
       0,   206,   207,     0,   208,     3,     4,     5,     6,     7,
      13,    42,    43,    44,    49,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    70,    71,    72,    73,    74,    75,    77,    81,    82,
      83,    84,    86,    88,    90,    93,    97,    98,    99,   100,
     101,   102,   103,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   115,   116,   117,   118,   119,   120,   125,   126,
     128,   129,   130,   131,   132,   139,   144,   145,   146,   147,
     148,   149,   151,   152,   154,   155,   156,   157,   160,   163,
     169,   170,   172,   174,   175,   176,   177,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   195,   197,   198,   200,   201,   203,   204,   209,   212,
     215,   216,   217,   218,   219,   220,   223,   238,   239,   243,
     248,   254,   309,   310,   315,   319,   320,   321,   322,   323,
     324,   325,   326,   328,   331,   340,   341,   342,   344,   345,
     347,   366,   376,   377,   378,   383,   386,   404,   409,   411,
     412,   413,   414,   415,   416,   417,   418,   420,   433,   435,
     437,   111,   112,   113,   125,   144,   212,   238,   309,   325,
     411,   325,   195,   325,   325,   325,   402,   403,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
      75,   113,   195,   216,   377,   378,   411,   411,    32,   325,
     424,   425,   325,   113,   195,   216,   377,   378,   379,   410,
     416,   421,   422,   195,   316,   380,   195,   316,   332,   317,
     325,   225,   316,   195,   195,   195,   316,   197,   325,   212,
     197,   325,    26,    52,   126,   127,   149,   169,   195,   212,
     219,   438,   448,   449,   178,   197,   322,   325,   346,   348,
     198,   231,   325,   147,   213,   214,   215,    75,   200,   280,
     281,   119,   119,    75,   282,   195,   195,   195,   195,   212,
     252,   439,   195,   195,    75,    80,   140,   141,   142,   430,
     431,   147,   198,   215,   215,    97,   325,   253,   439,   149,
     195,   439,   439,   325,   333,   315,   325,   326,   411,   221,
     198,    80,   381,   430,    80,   430,   430,    27,   147,   165,
     440,   195,     9,   197,    32,   237,   149,   251,   439,   113,
     238,   310,   197,   197,   197,   197,   197,   197,    10,    11,
      12,    26,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    50,   197,    62,    62,   197,   198,   143,
     120,   157,   254,   308,   309,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    60,    61,   123,
     406,   407,    62,   198,   408,   195,    62,   198,   200,   417,
     195,   237,   238,    14,   325,    41,   212,   401,   195,   315,
     411,   143,   411,   124,   202,     9,   388,   315,   411,   440,
     143,   195,   382,   123,   406,   407,   408,   196,   325,    27,
     223,     8,   334,     9,   197,   223,   224,   317,   318,   325,
     212,   266,   227,   197,   197,   197,   449,   449,   165,   195,
     100,   441,   449,    14,   212,    75,   197,   197,   197,   178,
     179,   180,   185,   186,   189,   190,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   361,   362,   363,   232,   104,
     162,   197,   215,     9,   197,    92,   198,   411,     9,   197,
      14,     9,   197,   411,   434,   434,   315,   326,   411,   196,
     165,   246,   125,   411,   423,   424,    62,   123,   140,   431,
      74,   325,   411,    80,   140,   431,   215,   211,   197,   198,
     197,   124,   249,   367,   369,    81,   335,   336,   338,    14,
      92,   436,   276,   277,   404,   405,   196,   196,   196,   199,
     222,   223,   239,   243,   248,   325,   201,   203,   204,   212,
     441,    32,   278,   279,   325,   438,   195,   439,   244,   237,
     325,   325,   325,    27,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   379,   325,   419,   419,
     325,   426,   427,   119,   198,   212,   416,   417,   252,   253,
     251,   238,    32,   148,   319,   322,   325,   346,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   198,
     212,   416,   419,   325,   278,   419,   325,   423,   237,   196,
     195,   400,     9,   388,   315,   196,   212,    32,   325,    32,
     325,   196,   196,   416,   278,   198,   212,   416,   196,   221,
     270,   198,   325,   325,    84,    27,   223,   264,   197,    92,
      14,     9,   196,    27,   198,   267,   449,    81,   445,   446,
     447,   195,     9,    43,    44,    62,   126,   139,   149,   169,
     177,   216,   217,   219,   343,   377,   383,   384,   385,   181,
      75,   325,    75,    75,   325,   358,   359,   325,   325,   351,
     361,   184,   364,   221,   195,   230,    92,   214,   212,   325,
     281,   384,    75,     9,   196,   196,   196,   196,   196,   197,
     212,   444,   121,   257,   195,     9,   196,   196,    75,    76,
     212,   432,   212,    62,   199,   199,   208,   210,   325,   122,
     256,   164,    47,   149,   164,   371,   124,     9,   388,   196,
     449,   449,    14,   193,     9,   389,   449,   450,   123,   406,
     407,   408,   199,     9,   166,   411,   196,     9,   389,    14,
     329,   240,   121,   255,   195,   439,   325,    27,   202,   202,
     124,   199,     9,   388,   325,   440,   195,   247,   250,   245,
     237,    64,   411,   325,   440,   195,   202,   199,   196,   202,
     199,   196,    43,    44,    62,    70,    71,    72,    81,   126,
     139,   169,   177,   212,   391,   393,   396,   399,   212,   411,
     411,   124,   406,   407,   408,   196,   325,   271,    67,    68,
     272,   221,   316,   221,   318,    32,   125,   261,   411,   384,
     212,    27,   223,   265,   197,   268,   197,   268,     9,   166,
     124,     9,   388,   196,   158,   441,   442,   449,   384,   384,
     384,   387,   390,   195,    80,   143,   195,   195,   143,   198,
     325,   181,   181,    14,   187,   188,   360,     9,   191,   364,
      75,   199,   377,   198,   234,   212,   199,    14,   411,   197,
      92,     9,   166,   258,   377,   198,   423,   125,   411,    14,
     202,   325,   199,   208,   258,   198,   370,    14,   325,   335,
     197,   449,    27,   443,   405,    32,    75,   158,   198,   212,
     416,   449,    32,   325,   384,   276,   195,   377,   256,   330,
     241,   325,   325,   325,   199,   195,   278,   257,   256,   255,
     439,   379,   199,   195,   278,    14,    70,    71,    72,   212,
     392,   392,   393,   394,   395,   195,    80,   140,   195,   195,
       9,   388,   196,   400,    32,   325,   199,    67,    68,   273,
     316,   223,   199,   197,    85,   197,   411,   195,   124,   260,
      14,   221,   268,    94,    95,    96,   268,   199,   449,   449,
     445,     9,   196,   196,   124,   202,     9,   388,   387,   212,
     335,   337,   339,   387,   119,   212,   384,   428,   429,   325,
     325,   325,   359,   325,   349,    75,   235,   384,   449,   212,
       9,   283,   196,   195,   319,   322,   325,   202,   199,   283,
     150,   163,   198,   366,   373,   150,   198,   372,   124,   197,
     449,   334,   450,    75,    14,    75,   325,   440,   195,   411,
     196,   276,   198,   276,   195,   124,   195,   278,   196,   198,
     198,   256,   242,   382,   195,   278,   196,   124,   202,     9,
     388,   394,   140,   335,   397,   398,   394,   393,   411,   316,
      27,    69,   223,   197,   318,   423,   261,   196,   384,    91,
      94,   197,   325,    27,   197,   269,   199,   166,   158,    27,
     384,   384,   196,   124,     9,   388,   196,   196,   124,   199,
       9,   388,   182,   196,   221,    92,   377,     4,   101,   106,
     114,   151,   152,   154,   199,   284,   307,   308,   309,   314,
     404,   423,   199,   199,    47,   325,   325,   325,    32,    75,
     158,    14,   384,   199,   195,   278,   443,   196,   283,   196,
     276,   325,   278,   196,   283,   283,   198,   195,   278,   196,
     393,   393,   196,   124,   196,     9,   388,   196,    27,   221,
     197,   196,   196,   228,   197,   197,   269,   221,   449,   124,
     384,   335,   384,   384,   325,   198,   199,   449,   121,   122,
     438,   259,   377,   114,   126,   149,   155,   293,   294,   295,
     377,   153,   299,   300,   117,   195,   212,   301,   302,   285,
     238,   449,     9,   197,   308,   196,   149,   368,   199,   199,
      75,    14,    75,   384,   195,   278,   196,   106,   327,   443,
     199,   443,   196,   196,   199,   199,   283,   276,   196,   124,
     393,   335,   221,   226,    27,   223,   263,   221,   196,   384,
     124,   124,   183,   221,   377,   377,    14,     9,   197,   198,
     198,     9,   197,     3,     4,     5,     6,     7,    10,    11,
      12,    13,    50,    63,    64,    65,    66,    67,    68,    69,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   125,   126,   128,   129,   130,   131,   132,   144,   145,
     146,   148,   157,   159,   160,   162,   169,   170,   172,   174,
     175,   176,   212,   374,   375,     9,   197,   149,   153,   212,
     302,   303,   304,   197,    75,   313,   237,   286,   438,   238,
      14,   384,   278,   196,   195,   198,   197,   198,   305,   327,
     443,   199,   196,   393,   124,    27,   223,   262,   221,   384,
     384,   325,   199,   197,   197,   384,   377,   289,   296,   383,
     294,    14,    27,    44,   297,   300,     9,    30,   196,    26,
      43,    46,    14,     9,   197,   439,   313,    14,   237,   384,
     196,    32,    75,   365,   221,   221,   198,   305,   443,   393,
     221,    89,   184,   233,   199,   212,   219,   290,   291,   292,
       9,   199,   384,   375,   375,    52,   298,   303,   303,    26,
      43,    46,   384,    75,   195,   197,   384,   439,    75,     9,
     389,   199,   199,   221,   305,    87,   197,    75,   104,   229,
     143,    92,   383,   156,    14,   287,   195,    32,    75,   196,
     199,   197,   195,   162,   236,   212,   308,   309,   384,   274,
     275,   405,   288,    75,   377,   234,   159,   212,   197,   196,
       9,   389,   108,   109,   110,   311,   312,   274,    75,   259,
     197,   443,   405,   450,   196,   196,   197,   197,   198,   306,
     311,    32,    75,   158,   443,   198,   221,   450,    75,    14,
      75,   306,   221,   199,    32,    75,   158,    14,   384,   199,
      75,    14,    75,   384,    14,   384,   384
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
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
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
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
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
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
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
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

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


/*----------.
| yyparse.  |
`----------*/

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
    YYLTYPE yyerror_range[3];

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

#if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
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
  if (yypact_value_is_default (yyn))
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

/* Line 1806 of yacc.c  */
#line 749 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 752 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 759 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));}
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 760 "hphp.y"
    { }
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 763 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 764 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 765 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 766 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 768 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 771 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();}
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 773 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());}
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 774 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);}
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 775 "hphp.y"
    { _p->onNamespaceStart("");}
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 776 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);}
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 777 "hphp.y"
    { _p->nns(); (yyval).reset();}
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 778 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;}
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 783 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 794 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 796 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 799 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 800 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 801 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 802 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 807 "hphp.y"
    { }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 808 "hphp.y"
    { }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 811 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");}
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 812 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");}
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 813 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());}
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 815 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());}
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 819 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 821 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;}
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 824 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;}
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 826 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));}
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 827 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;}
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 830 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 837 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 844 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 852 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 855 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 861 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 862 "hphp.y"
    { _p->onStatementListStart((yyval));}
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 865 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 866 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 867 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 868 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 871 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 875 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 880 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));}
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 881 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 883 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 887 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 890 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 894 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 896 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 899 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 901 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 904 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);}
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 905 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));}
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 906 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);}
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 907 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));}
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 908 "hphp.y"
    { _p->onReturn((yyval), NULL);}
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 909 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));}
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 910 "hphp.y"
    { _p->onYieldBreak((yyval));}
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 911 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 912 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 913 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);}
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 914 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));}
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 915 "hphp.y"
    { (yyval).reset(); (yyval) = ';';}
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 916 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);}
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 919 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 921 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 925 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;}
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 932 "hphp.y"
    { _p->onCompleteLabelScope(false);}
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 933 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));}
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 936 "hphp.y"
    { _p->onCompleteLabelScope(false);}
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 937 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));}
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 938 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 939 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval));}
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 943 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 944 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 945 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 946 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 947 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 948 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 949 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); }
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 950 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 951 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 952 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 953 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 961 "hphp.y"
    { _p->onNewLabelScope(false);}
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 962 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 971 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));}
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 972 "hphp.y"
    { (yyval).reset();}
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 976 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 978 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 984 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 985 "hphp.y"
    { (yyval).reset();}
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 989 "hphp.y"
    { (yyval) = 1;}
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 990 "hphp.y"
    { (yyval).reset();}
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 994 "hphp.y"
    { _p->pushFuncLocation(); }
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 999 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();}
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 1005 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 1011 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();}
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 1017 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 1023 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();}
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 1029 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 1037 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));}
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 1040 "hphp.y"
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
                                         _p->popTypeScope();}
    break;

  case 124:

/* Line 1806 of yacc.c  */
#line 1055 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));}
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 1058 "hphp.y"
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
                                         _p->popTypeScope();}
    break;

  case 126:

/* Line 1806 of yacc.c  */
#line 1072 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));}
    break;

  case 127:

/* Line 1806 of yacc.c  */
#line 1075 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 128:

/* Line 1806 of yacc.c  */
#line 1080 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));}
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 1083 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 130:

/* Line 1806 of yacc.c  */
#line 1090 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));}
    break;

  case 131:

/* Line 1806 of yacc.c  */
#line 1093 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0);
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 1101 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));}
    break;

  case 133:

/* Line 1806 of yacc.c  */
#line 1104 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 1112 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 1113 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 136:

/* Line 1806 of yacc.c  */
#line 1117 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 1120 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 1123 "hphp.y"
    { (yyval) = T_CLASS;}
    break;

  case 139:

/* Line 1806 of yacc.c  */
#line 1124 "hphp.y"
    { (yyval) = T_ABSTRACT;}
    break;

  case 140:

/* Line 1806 of yacc.c  */
#line 1125 "hphp.y"
    { (yyval) = T_FINAL;}
    break;

  case 141:

/* Line 1806 of yacc.c  */
#line 1129 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 142:

/* Line 1806 of yacc.c  */
#line 1130 "hphp.y"
    { (yyval).reset();}
    break;

  case 143:

/* Line 1806 of yacc.c  */
#line 1133 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 1134 "hphp.y"
    { (yyval).reset();}
    break;

  case 145:

/* Line 1806 of yacc.c  */
#line 1137 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 146:

/* Line 1806 of yacc.c  */
#line 1138 "hphp.y"
    { (yyval).reset();}
    break;

  case 147:

/* Line 1806 of yacc.c  */
#line 1141 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 148:

/* Line 1806 of yacc.c  */
#line 1143 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 149:

/* Line 1806 of yacc.c  */
#line 1146 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 150:

/* Line 1806 of yacc.c  */
#line 1148 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 151:

/* Line 1806 of yacc.c  */
#line 1152 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 152:

/* Line 1806 of yacc.c  */
#line 1153 "hphp.y"
    { (yyval).reset();}
    break;

  case 153:

/* Line 1806 of yacc.c  */
#line 1156 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 154:

/* Line 1806 of yacc.c  */
#line 1157 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;}
    break;

  case 155:

/* Line 1806 of yacc.c  */
#line 1158 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);}
    break;

  case 156:

/* Line 1806 of yacc.c  */
#line 1162 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 157:

/* Line 1806 of yacc.c  */
#line 1164 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 158:

/* Line 1806 of yacc.c  */
#line 1167 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 159:

/* Line 1806 of yacc.c  */
#line 1169 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 160:

/* Line 1806 of yacc.c  */
#line 1172 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 161:

/* Line 1806 of yacc.c  */
#line 1174 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 162:

/* Line 1806 of yacc.c  */
#line 1177 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 163:

/* Line 1806 of yacc.c  */
#line 1179 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 166:

/* Line 1806 of yacc.c  */
#line 1189 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 167:

/* Line 1806 of yacc.c  */
#line 1190 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 168:

/* Line 1806 of yacc.c  */
#line 1191 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 169:

/* Line 1806 of yacc.c  */
#line 1192 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);}
    break;

  case 170:

/* Line 1806 of yacc.c  */
#line 1197 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 171:

/* Line 1806 of yacc.c  */
#line 1199 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));}
    break;

  case 172:

/* Line 1806 of yacc.c  */
#line 1200 "hphp.y"
    { (yyval).reset();}
    break;

  case 173:

/* Line 1806 of yacc.c  */
#line 1203 "hphp.y"
    { (yyval).reset();}
    break;

  case 174:

/* Line 1806 of yacc.c  */
#line 1204 "hphp.y"
    { (yyval).reset();}
    break;

  case 175:

/* Line 1806 of yacc.c  */
#line 1209 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 176:

/* Line 1806 of yacc.c  */
#line 1210 "hphp.y"
    { (yyval).reset();}
    break;

  case 177:

/* Line 1806 of yacc.c  */
#line 1215 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 178:

/* Line 1806 of yacc.c  */
#line 1216 "hphp.y"
    { (yyval).reset();}
    break;

  case 179:

/* Line 1806 of yacc.c  */
#line 1219 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 180:

/* Line 1806 of yacc.c  */
#line 1220 "hphp.y"
    { (yyval).reset();}
    break;

  case 181:

/* Line 1806 of yacc.c  */
#line 1223 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);}
    break;

  case 182:

/* Line 1806 of yacc.c  */
#line 1224 "hphp.y"
    { (yyval).reset();}
    break;

  case 183:

/* Line 1806 of yacc.c  */
#line 1232 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); }
    break;

  case 184:

/* Line 1806 of yacc.c  */
#line 1238 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); }
    break;

  case 185:

/* Line 1806 of yacc.c  */
#line 1242 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 186:

/* Line 1806 of yacc.c  */
#line 1246 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); }
    break;

  case 187:

/* Line 1806 of yacc.c  */
#line 1251 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); }
    break;

  case 188:

/* Line 1806 of yacc.c  */
#line 1254 "hphp.y"
    { (yyval).reset(); }
    break;

  case 189:

/* Line 1806 of yacc.c  */
#line 1260 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));}
    break;

  case 190:

/* Line 1806 of yacc.c  */
#line 1264 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));}
    break;

  case 191:

/* Line 1806 of yacc.c  */
#line 1269 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));}
    break;

  case 192:

/* Line 1806 of yacc.c  */
#line 1274 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));}
    break;

  case 193:

/* Line 1806 of yacc.c  */
#line 1279 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));}
    break;

  case 194:

/* Line 1806 of yacc.c  */
#line 1284 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));}
    break;

  case 195:

/* Line 1806 of yacc.c  */
#line 1290 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));}
    break;

  case 196:

/* Line 1806 of yacc.c  */
#line 1296 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));}
    break;

  case 197:

/* Line 1806 of yacc.c  */
#line 1304 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); }
    break;

  case 198:

/* Line 1806 of yacc.c  */
#line 1309 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); }
    break;

  case 199:

/* Line 1806 of yacc.c  */
#line 1313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 200:

/* Line 1806 of yacc.c  */
#line 1316 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); }
    break;

  case 201:

/* Line 1806 of yacc.c  */
#line 1320 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); }
    break;

  case 202:

/* Line 1806 of yacc.c  */
#line 1323 "hphp.y"
    { (yyval).reset();}
    break;

  case 203:

/* Line 1806 of yacc.c  */
#line 1328 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); }
    break;

  case 204:

/* Line 1806 of yacc.c  */
#line 1331 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); }
    break;

  case 205:

/* Line 1806 of yacc.c  */
#line 1335 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); }
    break;

  case 206:

/* Line 1806 of yacc.c  */
#line 1339 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); }
    break;

  case 207:

/* Line 1806 of yacc.c  */
#line 1343 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); }
    break;

  case 208:

/* Line 1806 of yacc.c  */
#line 1347 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); }
    break;

  case 209:

/* Line 1806 of yacc.c  */
#line 1352 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); }
    break;

  case 210:

/* Line 1806 of yacc.c  */
#line 1357 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); }
    break;

  case 211:

/* Line 1806 of yacc.c  */
#line 1363 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 212:

/* Line 1806 of yacc.c  */
#line 1364 "hphp.y"
    { (yyval).reset();}
    break;

  case 213:

/* Line 1806 of yacc.c  */
#line 1367 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),0);}
    break;

  case 214:

/* Line 1806 of yacc.c  */
#line 1368 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),1);}
    break;

  case 215:

/* Line 1806 of yacc.c  */
#line 1370 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 216:

/* Line 1806 of yacc.c  */
#line 1372 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 217:

/* Line 1806 of yacc.c  */
#line 1376 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 218:

/* Line 1806 of yacc.c  */
#line 1377 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 219:

/* Line 1806 of yacc.c  */
#line 1380 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 220:

/* Line 1806 of yacc.c  */
#line 1381 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;}
    break;

  case 221:

/* Line 1806 of yacc.c  */
#line 1382 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;}
    break;

  case 222:

/* Line 1806 of yacc.c  */
#line 1386 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 223:

/* Line 1806 of yacc.c  */
#line 1388 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));}
    break;

  case 224:

/* Line 1806 of yacc.c  */
#line 1389 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);}
    break;

  case 225:

/* Line 1806 of yacc.c  */
#line 1390 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 226:

/* Line 1806 of yacc.c  */
#line 1395 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 227:

/* Line 1806 of yacc.c  */
#line 1396 "hphp.y"
    { (yyval).reset();}
    break;

  case 228:

/* Line 1806 of yacc.c  */
#line 1399 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));}
    break;

  case 229:

/* Line 1806 of yacc.c  */
#line 1400 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 230:

/* Line 1806 of yacc.c  */
#line 1403 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));}
    break;

  case 231:

/* Line 1806 of yacc.c  */
#line 1404 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));}
    break;

  case 232:

/* Line 1806 of yacc.c  */
#line 1406 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);}
    break;

  case 233:

/* Line 1806 of yacc.c  */
#line 1410 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();}
    break;

  case 234:

/* Line 1806 of yacc.c  */
#line 1416 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 235:

/* Line 1806 of yacc.c  */
#line 1423 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();}
    break;

  case 236:

/* Line 1806 of yacc.c  */
#line 1429 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 237:

/* Line 1806 of yacc.c  */
#line 1434 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));}
    break;

  case 238:

/* Line 1806 of yacc.c  */
#line 1436 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));}
    break;

  case 239:

/* Line 1806 of yacc.c  */
#line 1438 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));}
    break;

  case 240:

/* Line 1806 of yacc.c  */
#line 1440 "hphp.y"
    { _p->onTraitRequire((yyval), (yyvsp[(3) - (4)]), true); }
    break;

  case 241:

/* Line 1806 of yacc.c  */
#line 1442 "hphp.y"
    { _p->onTraitRequire((yyval), (yyvsp[(3) - (4)]), false); }
    break;

  case 242:

/* Line 1806 of yacc.c  */
#line 1443 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); }
    break;

  case 243:

/* Line 1806 of yacc.c  */
#line 1446 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); }
    break;

  case 244:

/* Line 1806 of yacc.c  */
#line 1449 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); }
    break;

  case 245:

/* Line 1806 of yacc.c  */
#line 1450 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); }
    break;

  case 246:

/* Line 1806 of yacc.c  */
#line 1451 "hphp.y"
    { (yyval).reset(); }
    break;

  case 247:

/* Line 1806 of yacc.c  */
#line 1457 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 248:

/* Line 1806 of yacc.c  */
#line 1461 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));}
    break;

  case 249:

/* Line 1806 of yacc.c  */
#line 1464 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);}
    break;

  case 250:

/* Line 1806 of yacc.c  */
#line 1471 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 251:

/* Line 1806 of yacc.c  */
#line 1472 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));}
    break;

  case 252:

/* Line 1806 of yacc.c  */
#line 1477 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));}
    break;

  case 253:

/* Line 1806 of yacc.c  */
#line 1480 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 254:

/* Line 1806 of yacc.c  */
#line 1487 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;}
    break;

  case 255:

/* Line 1806 of yacc.c  */
#line 1489 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;}
    break;

  case 256:

/* Line 1806 of yacc.c  */
#line 1493 "hphp.y"
    { (yyval) = 4;}
    break;

  case 257:

/* Line 1806 of yacc.c  */
#line 1494 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));}
    break;

  case 258:

/* Line 1806 of yacc.c  */
#line 1500 "hphp.y"
    { (yyval) = 6;}
    break;

  case 259:

/* Line 1806 of yacc.c  */
#line 1502 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;}
    break;

  case 260:

/* Line 1806 of yacc.c  */
#line 1506 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);}
    break;

  case 261:

/* Line 1806 of yacc.c  */
#line 1508 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);}
    break;

  case 262:

/* Line 1806 of yacc.c  */
#line 1512 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 263:

/* Line 1806 of yacc.c  */
#line 1513 "hphp.y"
    { scalar_null(_p, (yyval));}
    break;

  case 264:

/* Line 1806 of yacc.c  */
#line 1517 "hphp.y"
    { scalar_num(_p, (yyval), "1");}
    break;

  case 265:

/* Line 1806 of yacc.c  */
#line 1518 "hphp.y"
    { scalar_num(_p, (yyval), "0");}
    break;

  case 266:

/* Line 1806 of yacc.c  */
#line 1522 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);}
    break;

  case 267:

/* Line 1806 of yacc.c  */
#line 1525 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);}
    break;

  case 268:

/* Line 1806 of yacc.c  */
#line 1530 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 269:

/* Line 1806 of yacc.c  */
#line 1535 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;}
    break;

  case 270:

/* Line 1806 of yacc.c  */
#line 1536 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;}
    break;

  case 271:

/* Line 1806 of yacc.c  */
#line 1538 "hphp.y"
    { (yyval) = 0;}
    break;

  case 272:

/* Line 1806 of yacc.c  */
#line 1542 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);}
    break;

  case 273:

/* Line 1806 of yacc.c  */
#line 1543 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);}
    break;

  case 274:

/* Line 1806 of yacc.c  */
#line 1544 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);}
    break;

  case 275:

/* Line 1806 of yacc.c  */
#line 1545 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);}
    break;

  case 276:

/* Line 1806 of yacc.c  */
#line 1549 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 277:

/* Line 1806 of yacc.c  */
#line 1550 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);}
    break;

  case 278:

/* Line 1806 of yacc.c  */
#line 1551 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);}
    break;

  case 279:

/* Line 1806 of yacc.c  */
#line 1552 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);}
    break;

  case 280:

/* Line 1806 of yacc.c  */
#line 1553 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);}
    break;

  case 281:

/* Line 1806 of yacc.c  */
#line 1555 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));}
    break;

  case 282:

/* Line 1806 of yacc.c  */
#line 1557 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));}
    break;

  case 283:

/* Line 1806 of yacc.c  */
#line 1561 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;}
    break;

  case 284:

/* Line 1806 of yacc.c  */
#line 1564 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;}
    break;

  case 285:

/* Line 1806 of yacc.c  */
#line 1565 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;}
    break;

  case 286:

/* Line 1806 of yacc.c  */
#line 1569 "hphp.y"
    { (yyval).reset();}
    break;

  case 287:

/* Line 1806 of yacc.c  */
#line 1570 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;}
    break;

  case 288:

/* Line 1806 of yacc.c  */
#line 1574 "hphp.y"
    { (yyval).reset();}
    break;

  case 289:

/* Line 1806 of yacc.c  */
#line 1575 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;}
    break;

  case 290:

/* Line 1806 of yacc.c  */
#line 1578 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 291:

/* Line 1806 of yacc.c  */
#line 1579 "hphp.y"
    { (yyval).reset();}
    break;

  case 292:

/* Line 1806 of yacc.c  */
#line 1582 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 293:

/* Line 1806 of yacc.c  */
#line 1583 "hphp.y"
    { (yyval).reset();}
    break;

  case 294:

/* Line 1806 of yacc.c  */
#line 1586 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));}
    break;

  case 295:

/* Line 1806 of yacc.c  */
#line 1588 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 296:

/* Line 1806 of yacc.c  */
#line 1591 "hphp.y"
    { (yyval) = T_PUBLIC;}
    break;

  case 297:

/* Line 1806 of yacc.c  */
#line 1592 "hphp.y"
    { (yyval) = T_PROTECTED;}
    break;

  case 298:

/* Line 1806 of yacc.c  */
#line 1593 "hphp.y"
    { (yyval) = T_PRIVATE;}
    break;

  case 299:

/* Line 1806 of yacc.c  */
#line 1594 "hphp.y"
    { (yyval) = T_STATIC;}
    break;

  case 300:

/* Line 1806 of yacc.c  */
#line 1595 "hphp.y"
    { (yyval) = T_ABSTRACT;}
    break;

  case 301:

/* Line 1806 of yacc.c  */
#line 1596 "hphp.y"
    { (yyval) = T_FINAL;}
    break;

  case 302:

/* Line 1806 of yacc.c  */
#line 1597 "hphp.y"
    { (yyval) = T_ASYNC;}
    break;

  case 303:

/* Line 1806 of yacc.c  */
#line 1601 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 304:

/* Line 1806 of yacc.c  */
#line 1602 "hphp.y"
    { (yyval).reset();}
    break;

  case 305:

/* Line 1806 of yacc.c  */
#line 1605 "hphp.y"
    { (yyval) = T_PUBLIC;}
    break;

  case 306:

/* Line 1806 of yacc.c  */
#line 1606 "hphp.y"
    { (yyval) = T_PROTECTED;}
    break;

  case 307:

/* Line 1806 of yacc.c  */
#line 1607 "hphp.y"
    { (yyval) = T_PRIVATE;}
    break;

  case 308:

/* Line 1806 of yacc.c  */
#line 1611 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 309:

/* Line 1806 of yacc.c  */
#line 1613 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));}
    break;

  case 310:

/* Line 1806 of yacc.c  */
#line 1614 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);}
    break;

  case 311:

/* Line 1806 of yacc.c  */
#line 1615 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 312:

/* Line 1806 of yacc.c  */
#line 1619 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 313:

/* Line 1806 of yacc.c  */
#line 1620 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 314:

/* Line 1806 of yacc.c  */
#line 1624 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 315:

/* Line 1806 of yacc.c  */
#line 1626 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 316:

/* Line 1806 of yacc.c  */
#line 1627 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);}
    break;

  case 317:

/* Line 1806 of yacc.c  */
#line 1628 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 318:

/* Line 1806 of yacc.c  */
#line 1629 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 319:

/* Line 1806 of yacc.c  */
#line 1632 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 320:

/* Line 1806 of yacc.c  */
#line 1636 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 321:

/* Line 1806 of yacc.c  */
#line 1637 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 322:

/* Line 1806 of yacc.c  */
#line 1641 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 323:

/* Line 1806 of yacc.c  */
#line 1642 "hphp.y"
    { (yyval).reset();}
    break;

  case 324:

/* Line 1806 of yacc.c  */
#line 1646 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));}
    break;

  case 325:

/* Line 1806 of yacc.c  */
#line 1647 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 326:

/* Line 1806 of yacc.c  */
#line 1651 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 327:

/* Line 1806 of yacc.c  */
#line 1656 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);}
    break;

  case 328:

/* Line 1806 of yacc.c  */
#line 1660 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 329:

/* Line 1806 of yacc.c  */
#line 1664 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 330:

/* Line 1806 of yacc.c  */
#line 1669 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);}
    break;

  case 331:

/* Line 1806 of yacc.c  */
#line 1673 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 332:

/* Line 1806 of yacc.c  */
#line 1674 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 333:

/* Line 1806 of yacc.c  */
#line 1675 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 334:

/* Line 1806 of yacc.c  */
#line 1679 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));}
    break;

  case 335:

/* Line 1806 of yacc.c  */
#line 1680 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 336:

/* Line 1806 of yacc.c  */
#line 1681 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);}
    break;

  case 337:

/* Line 1806 of yacc.c  */
#line 1684 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));}
    break;

  case 338:

/* Line 1806 of yacc.c  */
#line 1685 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);}
    break;

  case 339:

/* Line 1806 of yacc.c  */
#line 1686 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);}
    break;

  case 340:

/* Line 1806 of yacc.c  */
#line 1687 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);}
    break;

  case 341:

/* Line 1806 of yacc.c  */
#line 1688 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);}
    break;

  case 342:

/* Line 1806 of yacc.c  */
#line 1689 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);}
    break;

  case 343:

/* Line 1806 of yacc.c  */
#line 1690 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);}
    break;

  case 344:

/* Line 1806 of yacc.c  */
#line 1691 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);}
    break;

  case 345:

/* Line 1806 of yacc.c  */
#line 1692 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);}
    break;

  case 346:

/* Line 1806 of yacc.c  */
#line 1693 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);}
    break;

  case 347:

/* Line 1806 of yacc.c  */
#line 1694 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);}
    break;

  case 348:

/* Line 1806 of yacc.c  */
#line 1695 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);}
    break;

  case 349:

/* Line 1806 of yacc.c  */
#line 1696 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);}
    break;

  case 350:

/* Line 1806 of yacc.c  */
#line 1697 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);}
    break;

  case 351:

/* Line 1806 of yacc.c  */
#line 1698 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);}
    break;

  case 352:

/* Line 1806 of yacc.c  */
#line 1699 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);}
    break;

  case 353:

/* Line 1806 of yacc.c  */
#line 1700 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);}
    break;

  case 354:

/* Line 1806 of yacc.c  */
#line 1701 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);}
    break;

  case 355:

/* Line 1806 of yacc.c  */
#line 1702 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);}
    break;

  case 356:

/* Line 1806 of yacc.c  */
#line 1703 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);}
    break;

  case 357:

/* Line 1806 of yacc.c  */
#line 1704 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);}
    break;

  case 358:

/* Line 1806 of yacc.c  */
#line 1705 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');}
    break;

  case 359:

/* Line 1806 of yacc.c  */
#line 1706 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');}
    break;

  case 360:

/* Line 1806 of yacc.c  */
#line 1707 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');}
    break;

  case 361:

/* Line 1806 of yacc.c  */
#line 1708 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');}
    break;

  case 362:

/* Line 1806 of yacc.c  */
#line 1709 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');}
    break;

  case 363:

/* Line 1806 of yacc.c  */
#line 1710 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');}
    break;

  case 364:

/* Line 1806 of yacc.c  */
#line 1711 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');}
    break;

  case 365:

/* Line 1806 of yacc.c  */
#line 1712 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');}
    break;

  case 366:

/* Line 1806 of yacc.c  */
#line 1713 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');}
    break;

  case 367:

/* Line 1806 of yacc.c  */
#line 1714 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);}
    break;

  case 368:

/* Line 1806 of yacc.c  */
#line 1715 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);}
    break;

  case 369:

/* Line 1806 of yacc.c  */
#line 1716 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 370:

/* Line 1806 of yacc.c  */
#line 1717 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 371:

/* Line 1806 of yacc.c  */
#line 1718 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);}
    break;

  case 372:

/* Line 1806 of yacc.c  */
#line 1719 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);}
    break;

  case 373:

/* Line 1806 of yacc.c  */
#line 1720 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);}
    break;

  case 374:

/* Line 1806 of yacc.c  */
#line 1721 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);}
    break;

  case 375:

/* Line 1806 of yacc.c  */
#line 1722 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);}
    break;

  case 376:

/* Line 1806 of yacc.c  */
#line 1723 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);}
    break;

  case 377:

/* Line 1806 of yacc.c  */
#line 1724 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');}
    break;

  case 378:

/* Line 1806 of yacc.c  */
#line 1725 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);}
    break;

  case 379:

/* Line 1806 of yacc.c  */
#line 1727 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');}
    break;

  case 380:

/* Line 1806 of yacc.c  */
#line 1728 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);}
    break;

  case 381:

/* Line 1806 of yacc.c  */
#line 1731 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);}
    break;

  case 382:

/* Line 1806 of yacc.c  */
#line 1732 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 383:

/* Line 1806 of yacc.c  */
#line 1733 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));}
    break;

  case 384:

/* Line 1806 of yacc.c  */
#line 1734 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));}
    break;

  case 385:

/* Line 1806 of yacc.c  */
#line 1735 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 386:

/* Line 1806 of yacc.c  */
#line 1736 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);}
    break;

  case 387:

/* Line 1806 of yacc.c  */
#line 1737 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);}
    break;

  case 388:

/* Line 1806 of yacc.c  */
#line 1738 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);}
    break;

  case 389:

/* Line 1806 of yacc.c  */
#line 1739 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);}
    break;

  case 390:

/* Line 1806 of yacc.c  */
#line 1740 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);}
    break;

  case 391:

/* Line 1806 of yacc.c  */
#line 1741 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);}
    break;

  case 392:

/* Line 1806 of yacc.c  */
#line 1742 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);}
    break;

  case 393:

/* Line 1806 of yacc.c  */
#line 1743 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);}
    break;

  case 394:

/* Line 1806 of yacc.c  */
#line 1744 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);}
    break;

  case 395:

/* Line 1806 of yacc.c  */
#line 1745 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 396:

/* Line 1806 of yacc.c  */
#line 1746 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 397:

/* Line 1806 of yacc.c  */
#line 1747 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 398:

/* Line 1806 of yacc.c  */
#line 1748 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));}
    break;

  case 399:

/* Line 1806 of yacc.c  */
#line 1749 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);}
    break;

  case 400:

/* Line 1806 of yacc.c  */
#line 1750 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 401:

/* Line 1806 of yacc.c  */
#line 1751 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 402:

/* Line 1806 of yacc.c  */
#line 1752 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 403:

/* Line 1806 of yacc.c  */
#line 1759 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);}
    break;

  case 404:

/* Line 1806 of yacc.c  */
#line 1760 "hphp.y"
    { (yyval).reset();}
    break;

  case 405:

/* Line 1806 of yacc.c  */
#line 1765 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
    break;

  case 406:

/* Line 1806 of yacc.c  */
#line 1771 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 407:

/* Line 1806 of yacc.c  */
#line 1779 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
    break;

  case 408:

/* Line 1806 of yacc.c  */
#line 1785 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 409:

/* Line 1806 of yacc.c  */
#line 1794 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);}
    break;

  case 410:

/* Line 1806 of yacc.c  */
#line 1802 "hphp.y"
    { Token v; Token w;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 411:

/* Line 1806 of yacc.c  */
#line 1809 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
    break;

  case 412:

/* Line 1806 of yacc.c  */
#line 1817 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 413:

/* Line 1806 of yacc.c  */
#line 1827 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));}
    break;

  case 414:

/* Line 1806 of yacc.c  */
#line 1829 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 415:

/* Line 1806 of yacc.c  */
#line 1833 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); }
    break;

  case 416:

/* Line 1806 of yacc.c  */
#line 1841 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 417:

/* Line 1806 of yacc.c  */
#line 1844 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 418:

/* Line 1806 of yacc.c  */
#line 1851 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 419:

/* Line 1806 of yacc.c  */
#line 1854 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 420:

/* Line 1806 of yacc.c  */
#line 1859 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 421:

/* Line 1806 of yacc.c  */
#line 1860 "hphp.y"
    { (yyval).reset(); }
    break;

  case 422:

/* Line 1806 of yacc.c  */
#line 1865 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 423:

/* Line 1806 of yacc.c  */
#line 1866 "hphp.y"
    { (yyval).reset(); }
    break;

  case 424:

/* Line 1806 of yacc.c  */
#line 1870 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);}
    break;

  case 425:

/* Line 1806 of yacc.c  */
#line 1874 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 426:

/* Line 1806 of yacc.c  */
#line 1875 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 427:

/* Line 1806 of yacc.c  */
#line 1880 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);}
    break;

  case 428:

/* Line 1806 of yacc.c  */
#line 1887 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);}
    break;

  case 429:

/* Line 1806 of yacc.c  */
#line 1894 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 430:

/* Line 1806 of yacc.c  */
#line 1896 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 431:

/* Line 1806 of yacc.c  */
#line 1900 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 432:

/* Line 1806 of yacc.c  */
#line 1901 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 433:

/* Line 1806 of yacc.c  */
#line 1902 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 434:

/* Line 1806 of yacc.c  */
#line 1906 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); }
    break;

  case 435:

/* Line 1806 of yacc.c  */
#line 1910 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 436:

/* Line 1806 of yacc.c  */
#line 1914 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 437:

/* Line 1806 of yacc.c  */
#line 1919 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); }
    break;

  case 438:

/* Line 1806 of yacc.c  */
#line 1921 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); }
    break;

  case 439:

/* Line 1806 of yacc.c  */
#line 1923 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); }
    break;

  case 440:

/* Line 1806 of yacc.c  */
#line 1925 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); }
    break;

  case 441:

/* Line 1806 of yacc.c  */
#line 1929 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); }
    break;

  case 442:

/* Line 1806 of yacc.c  */
#line 1931 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); }
    break;

  case 443:

/* Line 1806 of yacc.c  */
#line 1935 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 444:

/* Line 1806 of yacc.c  */
#line 1936 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 445:

/* Line 1806 of yacc.c  */
#line 1937 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 446:

/* Line 1806 of yacc.c  */
#line 1938 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 447:

/* Line 1806 of yacc.c  */
#line 1939 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 448:

/* Line 1806 of yacc.c  */
#line 1940 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 449:

/* Line 1806 of yacc.c  */
#line 1944 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 450:

/* Line 1806 of yacc.c  */
#line 1948 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 451:

/* Line 1806 of yacc.c  */
#line 1952 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 452:

/* Line 1806 of yacc.c  */
#line 1957 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); }
    break;

  case 453:

/* Line 1806 of yacc.c  */
#line 1962 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); }
    break;

  case 454:

/* Line 1806 of yacc.c  */
#line 1966 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 455:

/* Line 1806 of yacc.c  */
#line 1970 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); }
    break;

  case 456:

/* Line 1806 of yacc.c  */
#line 1971 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); }
    break;

  case 457:

/* Line 1806 of yacc.c  */
#line 1975 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); }
    break;

  case 458:

/* Line 1806 of yacc.c  */
#line 1976 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); }
    break;

  case 459:

/* Line 1806 of yacc.c  */
#line 1980 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 460:

/* Line 1806 of yacc.c  */
#line 1981 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 461:

/* Line 1806 of yacc.c  */
#line 1985 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 462:

/* Line 1806 of yacc.c  */
#line 1986 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 463:

/* Line 1806 of yacc.c  */
#line 1990 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 464:

/* Line 1806 of yacc.c  */
#line 1994 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 465:

/* Line 1806 of yacc.c  */
#line 1998 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); }
    break;

  case 466:

/* Line 1806 of yacc.c  */
#line 2002 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 467:

/* Line 1806 of yacc.c  */
#line 2003 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 468:

/* Line 1806 of yacc.c  */
#line 2004 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 469:

/* Line 1806 of yacc.c  */
#line 2005 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);}
    break;

  case 470:

/* Line 1806 of yacc.c  */
#line 2012 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));}
    break;

  case 471:

/* Line 1806 of yacc.c  */
#line 2015 "hphp.y"
    { Token t1; _p->onArray(t1,(yyvsp[(1) - (2)]));
                                         Token t2; _p->onArray(t2,(yyvsp[(2) - (2)]));
                                         Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onCallParam((yyvsp[(1) - (2)]),NULL,t1,0);
                                         _p->onCallParam((yyval), &(yyvsp[(1) - (2)]),t2,0);
                                         _p->onCallParam((yyvsp[(1) - (2)]), &(yyvsp[(1) - (2)]),file,0);
                                         _p->onCallParam((yyvsp[(1) - (2)]), &(yyvsp[(1) - (2)]),line,0);
                                         (yyval).setText("");}
    break;

  case 472:

/* Line 1806 of yacc.c  */
#line 2026 "hphp.y"
    { Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onArray((yyvsp[(4) - (6)]),(yyvsp[(1) - (6)]));
                                         _p->onArray((yyvsp[(5) - (6)]),(yyvsp[(3) - (6)]));
                                         _p->onCallParam((yyvsp[(2) - (6)]),NULL,(yyvsp[(4) - (6)]),0);
                                         _p->onCallParam((yyval), &(yyvsp[(2) - (6)]),(yyvsp[(5) - (6)]),0);
                                         _p->onCallParam((yyvsp[(2) - (6)]), &(yyvsp[(2) - (6)]),file,0);
                                         _p->onCallParam((yyvsp[(2) - (6)]), &(yyvsp[(2) - (6)]),line,0);
                                         (yyval).setText((yyvsp[(6) - (6)]).text());}
    break;

  case 473:

/* Line 1806 of yacc.c  */
#line 2037 "hphp.y"
    { (yyval).reset(); (yyval).setText("");}
    break;

  case 474:

/* Line 1806 of yacc.c  */
#line 2038 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));}
    break;

  case 475:

/* Line 1806 of yacc.c  */
#line 2043 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);}
    break;

  case 476:

/* Line 1806 of yacc.c  */
#line 2044 "hphp.y"
    { (yyval).reset();}
    break;

  case 477:

/* Line 1806 of yacc.c  */
#line 2047 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);}
    break;

  case 478:

/* Line 1806 of yacc.c  */
#line 2048 "hphp.y"
    { (yyval).reset();}
    break;

  case 479:

/* Line 1806 of yacc.c  */
#line 2051 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 480:

/* Line 1806 of yacc.c  */
#line 2055 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 481:

/* Line 1806 of yacc.c  */
#line 2058 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 482:

/* Line 1806 of yacc.c  */
#line 2061 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       }
    break;

  case 483:

/* Line 1806 of yacc.c  */
#line 2068 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 484:

/* Line 1806 of yacc.c  */
#line 2069 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 485:

/* Line 1806 of yacc.c  */
#line 2073 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 486:

/* Line 1806 of yacc.c  */
#line 2075 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);}
    break;

  case 487:

/* Line 1806 of yacc.c  */
#line 2077 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);}
    break;

  case 488:

/* Line 1806 of yacc.c  */
#line 2080 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 489:

/* Line 1806 of yacc.c  */
#line 2081 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 490:

/* Line 1806 of yacc.c  */
#line 2082 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 491:

/* Line 1806 of yacc.c  */
#line 2083 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 492:

/* Line 1806 of yacc.c  */
#line 2084 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 493:

/* Line 1806 of yacc.c  */
#line 2085 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 494:

/* Line 1806 of yacc.c  */
#line 2086 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 495:

/* Line 1806 of yacc.c  */
#line 2087 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 496:

/* Line 1806 of yacc.c  */
#line 2088 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 497:

/* Line 1806 of yacc.c  */
#line 2089 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 498:

/* Line 1806 of yacc.c  */
#line 2090 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 499:

/* Line 1806 of yacc.c  */
#line 2091 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 500:

/* Line 1806 of yacc.c  */
#line 2092 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 501:

/* Line 1806 of yacc.c  */
#line 2093 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 502:

/* Line 1806 of yacc.c  */
#line 2094 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 503:

/* Line 1806 of yacc.c  */
#line 2095 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 504:

/* Line 1806 of yacc.c  */
#line 2096 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 505:

/* Line 1806 of yacc.c  */
#line 2097 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 506:

/* Line 1806 of yacc.c  */
#line 2098 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 507:

/* Line 1806 of yacc.c  */
#line 2099 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 508:

/* Line 1806 of yacc.c  */
#line 2100 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 509:

/* Line 1806 of yacc.c  */
#line 2101 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 510:

/* Line 1806 of yacc.c  */
#line 2102 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 511:

/* Line 1806 of yacc.c  */
#line 2103 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 512:

/* Line 1806 of yacc.c  */
#line 2104 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 513:

/* Line 1806 of yacc.c  */
#line 2105 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 514:

/* Line 1806 of yacc.c  */
#line 2106 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 515:

/* Line 1806 of yacc.c  */
#line 2107 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 516:

/* Line 1806 of yacc.c  */
#line 2108 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 517:

/* Line 1806 of yacc.c  */
#line 2109 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 518:

/* Line 1806 of yacc.c  */
#line 2110 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 519:

/* Line 1806 of yacc.c  */
#line 2111 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 520:

/* Line 1806 of yacc.c  */
#line 2112 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 521:

/* Line 1806 of yacc.c  */
#line 2113 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 522:

/* Line 1806 of yacc.c  */
#line 2114 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 523:

/* Line 1806 of yacc.c  */
#line 2115 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 524:

/* Line 1806 of yacc.c  */
#line 2116 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 525:

/* Line 1806 of yacc.c  */
#line 2117 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 526:

/* Line 1806 of yacc.c  */
#line 2118 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 527:

/* Line 1806 of yacc.c  */
#line 2119 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 528:

/* Line 1806 of yacc.c  */
#line 2120 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 529:

/* Line 1806 of yacc.c  */
#line 2121 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 530:

/* Line 1806 of yacc.c  */
#line 2122 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 531:

/* Line 1806 of yacc.c  */
#line 2123 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 532:

/* Line 1806 of yacc.c  */
#line 2124 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 533:

/* Line 1806 of yacc.c  */
#line 2125 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 534:

/* Line 1806 of yacc.c  */
#line 2126 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 535:

/* Line 1806 of yacc.c  */
#line 2127 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 536:

/* Line 1806 of yacc.c  */
#line 2128 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 537:

/* Line 1806 of yacc.c  */
#line 2129 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 538:

/* Line 1806 of yacc.c  */
#line 2130 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 539:

/* Line 1806 of yacc.c  */
#line 2131 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 540:

/* Line 1806 of yacc.c  */
#line 2132 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 541:

/* Line 1806 of yacc.c  */
#line 2133 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 542:

/* Line 1806 of yacc.c  */
#line 2134 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 543:

/* Line 1806 of yacc.c  */
#line 2135 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 544:

/* Line 1806 of yacc.c  */
#line 2136 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 545:

/* Line 1806 of yacc.c  */
#line 2137 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 546:

/* Line 1806 of yacc.c  */
#line 2138 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 547:

/* Line 1806 of yacc.c  */
#line 2139 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 548:

/* Line 1806 of yacc.c  */
#line 2140 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 549:

/* Line 1806 of yacc.c  */
#line 2141 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 550:

/* Line 1806 of yacc.c  */
#line 2142 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 551:

/* Line 1806 of yacc.c  */
#line 2143 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 552:

/* Line 1806 of yacc.c  */
#line 2144 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 553:

/* Line 1806 of yacc.c  */
#line 2145 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 554:

/* Line 1806 of yacc.c  */
#line 2146 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 555:

/* Line 1806 of yacc.c  */
#line 2147 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 556:

/* Line 1806 of yacc.c  */
#line 2148 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 557:

/* Line 1806 of yacc.c  */
#line 2149 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 558:

/* Line 1806 of yacc.c  */
#line 2150 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 559:

/* Line 1806 of yacc.c  */
#line 2151 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 560:

/* Line 1806 of yacc.c  */
#line 2152 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 561:

/* Line 1806 of yacc.c  */
#line 2153 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 562:

/* Line 1806 of yacc.c  */
#line 2154 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 563:

/* Line 1806 of yacc.c  */
#line 2155 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 564:

/* Line 1806 of yacc.c  */
#line 2156 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 565:

/* Line 1806 of yacc.c  */
#line 2157 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 566:

/* Line 1806 of yacc.c  */
#line 2158 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 567:

/* Line 1806 of yacc.c  */
#line 2159 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 568:

/* Line 1806 of yacc.c  */
#line 2164 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 569:

/* Line 1806 of yacc.c  */
#line 2168 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 570:

/* Line 1806 of yacc.c  */
#line 2169 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 571:

/* Line 1806 of yacc.c  */
#line 2172 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);}
    break;

  case 572:

/* Line 1806 of yacc.c  */
#line 2173 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);}
    break;

  case 573:

/* Line 1806 of yacc.c  */
#line 2174 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);}
    break;

  case 574:

/* Line 1806 of yacc.c  */
#line 2178 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);}
    break;

  case 575:

/* Line 1806 of yacc.c  */
#line 2179 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);}
    break;

  case 576:

/* Line 1806 of yacc.c  */
#line 2180 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);}
    break;

  case 577:

/* Line 1806 of yacc.c  */
#line 2184 "hphp.y"
    { (yyval).reset();}
    break;

  case 578:

/* Line 1806 of yacc.c  */
#line 2185 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 579:

/* Line 1806 of yacc.c  */
#line 2186 "hphp.y"
    { (yyval).reset();}
    break;

  case 580:

/* Line 1806 of yacc.c  */
#line 2190 "hphp.y"
    { (yyval).reset();}
    break;

  case 581:

/* Line 1806 of yacc.c  */
#line 2191 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);}
    break;

  case 582:

/* Line 1806 of yacc.c  */
#line 2192 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 583:

/* Line 1806 of yacc.c  */
#line 2196 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 584:

/* Line 1806 of yacc.c  */
#line 2197 "hphp.y"
    { (yyval).reset();}
    break;

  case 585:

/* Line 1806 of yacc.c  */
#line 2201 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 586:

/* Line 1806 of yacc.c  */
#line 2202 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 587:

/* Line 1806 of yacc.c  */
#line 2203 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 588:

/* Line 1806 of yacc.c  */
#line 2204 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));}
    break;

  case 589:

/* Line 1806 of yacc.c  */
#line 2206 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));}
    break;

  case 590:

/* Line 1806 of yacc.c  */
#line 2207 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));}
    break;

  case 591:

/* Line 1806 of yacc.c  */
#line 2208 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));}
    break;

  case 592:

/* Line 1806 of yacc.c  */
#line 2209 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));}
    break;

  case 593:

/* Line 1806 of yacc.c  */
#line 2210 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));}
    break;

  case 594:

/* Line 1806 of yacc.c  */
#line 2211 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));}
    break;

  case 595:

/* Line 1806 of yacc.c  */
#line 2212 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));}
    break;

  case 596:

/* Line 1806 of yacc.c  */
#line 2213 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));}
    break;

  case 597:

/* Line 1806 of yacc.c  */
#line 2214 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));}
    break;

  case 598:

/* Line 1806 of yacc.c  */
#line 2217 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));}
    break;

  case 599:

/* Line 1806 of yacc.c  */
#line 2219 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
    break;

  case 600:

/* Line 1806 of yacc.c  */
#line 2223 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 601:

/* Line 1806 of yacc.c  */
#line 2224 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 602:

/* Line 1806 of yacc.c  */
#line 2225 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 603:

/* Line 1806 of yacc.c  */
#line 2226 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 604:

/* Line 1806 of yacc.c  */
#line 2228 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 605:

/* Line 1806 of yacc.c  */
#line 2230 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 606:

/* Line 1806 of yacc.c  */
#line 2231 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); }
    break;

  case 607:

/* Line 1806 of yacc.c  */
#line 2233 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 608:

/* Line 1806 of yacc.c  */
#line 2234 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 609:

/* Line 1806 of yacc.c  */
#line 2235 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 610:

/* Line 1806 of yacc.c  */
#line 2241 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 611:

/* Line 1806 of yacc.c  */
#line 2243 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 612:

/* Line 1806 of yacc.c  */
#line 2247 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 613:

/* Line 1806 of yacc.c  */
#line 2251 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 614:

/* Line 1806 of yacc.c  */
#line 2252 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 615:

/* Line 1806 of yacc.c  */
#line 2253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 616:

/* Line 1806 of yacc.c  */
#line 2254 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 617:

/* Line 1806 of yacc.c  */
#line 2255 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));}
    break;

  case 618:

/* Line 1806 of yacc.c  */
#line 2256 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));}
    break;

  case 619:

/* Line 1806 of yacc.c  */
#line 2258 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));}
    break;

  case 620:

/* Line 1806 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 621:

/* Line 1806 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval).reset();}
    break;

  case 622:

/* Line 1806 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval).reset();}
    break;

  case 623:

/* Line 1806 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval).reset();}
    break;

  case 624:

/* Line 1806 of yacc.c  */
#line 2272 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();}
    break;

  case 625:

/* Line 1806 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval).reset();}
    break;

  case 626:

/* Line 1806 of yacc.c  */
#line 2279 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 627:

/* Line 1806 of yacc.c  */
#line 2281 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 628:

/* Line 1806 of yacc.c  */
#line 2283 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 629:

/* Line 1806 of yacc.c  */
#line 2284 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 630:

/* Line 1806 of yacc.c  */
#line 2288 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 631:

/* Line 1806 of yacc.c  */
#line 2289 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 632:

/* Line 1806 of yacc.c  */
#line 2290 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 633:

/* Line 1806 of yacc.c  */
#line 2291 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));}
    break;

  case 634:

/* Line 1806 of yacc.c  */
#line 2295 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));}
    break;

  case 635:

/* Line 1806 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
    break;

  case 636:

/* Line 1806 of yacc.c  */
#line 2300 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));}
    break;

  case 637:

/* Line 1806 of yacc.c  */
#line 2301 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));}
    break;

  case 638:

/* Line 1806 of yacc.c  */
#line 2302 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));}
    break;

  case 639:

/* Line 1806 of yacc.c  */
#line 2303 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));}
    break;

  case 640:

/* Line 1806 of yacc.c  */
#line 2306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 641:

/* Line 1806 of yacc.c  */
#line 2307 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));}
    break;

  case 642:

/* Line 1806 of yacc.c  */
#line 2308 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 643:

/* Line 1806 of yacc.c  */
#line 2309 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 644:

/* Line 1806 of yacc.c  */
#line 2311 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 645:

/* Line 1806 of yacc.c  */
#line 2313 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 646:

/* Line 1806 of yacc.c  */
#line 2314 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 647:

/* Line 1806 of yacc.c  */
#line 2316 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 648:

/* Line 1806 of yacc.c  */
#line 2321 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 649:

/* Line 1806 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval).reset();}
    break;

  case 650:

/* Line 1806 of yacc.c  */
#line 2327 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 651:

/* Line 1806 of yacc.c  */
#line 2329 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 652:

/* Line 1806 of yacc.c  */
#line 2331 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 653:

/* Line 1806 of yacc.c  */
#line 2332 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 654:

/* Line 1806 of yacc.c  */
#line 2336 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 655:

/* Line 1806 of yacc.c  */
#line 2337 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 656:

/* Line 1806 of yacc.c  */
#line 2342 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 657:

/* Line 1806 of yacc.c  */
#line 2343 "hphp.y"
    { (yyval).reset(); }
    break;

  case 658:

/* Line 1806 of yacc.c  */
#line 2348 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 659:

/* Line 1806 of yacc.c  */
#line 2351 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 660:

/* Line 1806 of yacc.c  */
#line 2356 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 661:

/* Line 1806 of yacc.c  */
#line 2357 "hphp.y"
    { (yyval).reset();}
    break;

  case 662:

/* Line 1806 of yacc.c  */
#line 2360 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 663:

/* Line 1806 of yacc.c  */
#line 2361 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
    break;

  case 664:

/* Line 1806 of yacc.c  */
#line 2368 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 665:

/* Line 1806 of yacc.c  */
#line 2370 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 666:

/* Line 1806 of yacc.c  */
#line 2373 "hphp.y"
    { only_in_hh_syntax(_p);}
    break;

  case 667:

/* Line 1806 of yacc.c  */
#line 2375 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 668:

/* Line 1806 of yacc.c  */
#line 2378 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 669:

/* Line 1806 of yacc.c  */
#line 2381 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 670:

/* Line 1806 of yacc.c  */
#line 2382 "hphp.y"
    { (yyval).reset();}
    break;

  case 671:

/* Line 1806 of yacc.c  */
#line 2386 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 672:

/* Line 1806 of yacc.c  */
#line 2388 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 673:

/* Line 1806 of yacc.c  */
#line 2392 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 674:

/* Line 1806 of yacc.c  */
#line 2393 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 675:

/* Line 1806 of yacc.c  */
#line 2397 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 676:

/* Line 1806 of yacc.c  */
#line 2398 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 677:

/* Line 1806 of yacc.c  */
#line 2402 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 678:

/* Line 1806 of yacc.c  */
#line 2404 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 679:

/* Line 1806 of yacc.c  */
#line 2409 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 680:

/* Line 1806 of yacc.c  */
#line 2411 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 681:

/* Line 1806 of yacc.c  */
#line 2415 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 682:

/* Line 1806 of yacc.c  */
#line 2416 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 683:

/* Line 1806 of yacc.c  */
#line 2417 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 684:

/* Line 1806 of yacc.c  */
#line 2418 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 685:

/* Line 1806 of yacc.c  */
#line 2419 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 686:

/* Line 1806 of yacc.c  */
#line 2420 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 687:

/* Line 1806 of yacc.c  */
#line 2422 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 688:

/* Line 1806 of yacc.c  */
#line 2425 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 689:

/* Line 1806 of yacc.c  */
#line 2427 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 690:

/* Line 1806 of yacc.c  */
#line 2428 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 691:

/* Line 1806 of yacc.c  */
#line 2432 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 692:

/* Line 1806 of yacc.c  */
#line 2433 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 693:

/* Line 1806 of yacc.c  */
#line 2434 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 694:

/* Line 1806 of yacc.c  */
#line 2435 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 695:

/* Line 1806 of yacc.c  */
#line 2437 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 696:

/* Line 1806 of yacc.c  */
#line 2439 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 697:

/* Line 1806 of yacc.c  */
#line 2441 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 698:

/* Line 1806 of yacc.c  */
#line 2442 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 699:

/* Line 1806 of yacc.c  */
#line 2446 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 700:

/* Line 1806 of yacc.c  */
#line 2447 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 701:

/* Line 1806 of yacc.c  */
#line 2448 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 702:

/* Line 1806 of yacc.c  */
#line 2454 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));}
    break;

  case 703:

/* Line 1806 of yacc.c  */
#line 2457 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 704:

/* Line 1806 of yacc.c  */
#line 2460 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));}
    break;

  case 705:

/* Line 1806 of yacc.c  */
#line 2464 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));}
    break;

  case 706:

/* Line 1806 of yacc.c  */
#line 2468 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));}
    break;

  case 707:

/* Line 1806 of yacc.c  */
#line 2472 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));}
    break;

  case 708:

/* Line 1806 of yacc.c  */
#line 2479 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));}
    break;

  case 709:

/* Line 1806 of yacc.c  */
#line 2483 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));}
    break;

  case 710:

/* Line 1806 of yacc.c  */
#line 2487 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));}
    break;

  case 711:

/* Line 1806 of yacc.c  */
#line 2491 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 712:

/* Line 1806 of yacc.c  */
#line 2493 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 713:

/* Line 1806 of yacc.c  */
#line 2498 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 714:

/* Line 1806 of yacc.c  */
#line 2499 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 715:

/* Line 1806 of yacc.c  */
#line 2500 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 716:

/* Line 1806 of yacc.c  */
#line 2503 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 717:

/* Line 1806 of yacc.c  */
#line 2504 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);}
    break;

  case 718:

/* Line 1806 of yacc.c  */
#line 2507 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 719:

/* Line 1806 of yacc.c  */
#line 2508 "hphp.y"
    { (yyval).reset();}
    break;

  case 720:

/* Line 1806 of yacc.c  */
#line 2512 "hphp.y"
    { (yyval) = 1;}
    break;

  case 721:

/* Line 1806 of yacc.c  */
#line 2513 "hphp.y"
    { (yyval)++;}
    break;

  case 722:

/* Line 1806 of yacc.c  */
#line 2517 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 723:

/* Line 1806 of yacc.c  */
#line 2518 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 724:

/* Line 1806 of yacc.c  */
#line 2519 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 725:

/* Line 1806 of yacc.c  */
#line 2521 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 726:

/* Line 1806 of yacc.c  */
#line 2524 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 727:

/* Line 1806 of yacc.c  */
#line 2525 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 729:

/* Line 1806 of yacc.c  */
#line 2529 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 730:

/* Line 1806 of yacc.c  */
#line 2531 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 731:

/* Line 1806 of yacc.c  */
#line 2533 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 732:

/* Line 1806 of yacc.c  */
#line 2534 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 733:

/* Line 1806 of yacc.c  */
#line 2538 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);}
    break;

  case 734:

/* Line 1806 of yacc.c  */
#line 2539 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 735:

/* Line 1806 of yacc.c  */
#line 2541 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 736:

/* Line 1806 of yacc.c  */
#line 2542 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);}
    break;

  case 737:

/* Line 1806 of yacc.c  */
#line 2543 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));}
    break;

  case 738:

/* Line 1806 of yacc.c  */
#line 2544 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));}
    break;

  case 739:

/* Line 1806 of yacc.c  */
#line 2549 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 740:

/* Line 1806 of yacc.c  */
#line 2550 "hphp.y"
    { (yyval).reset();}
    break;

  case 741:

/* Line 1806 of yacc.c  */
#line 2554 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 742:

/* Line 1806 of yacc.c  */
#line 2555 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 743:

/* Line 1806 of yacc.c  */
#line 2556 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 744:

/* Line 1806 of yacc.c  */
#line 2557 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 745:

/* Line 1806 of yacc.c  */
#line 2560 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);}
    break;

  case 746:

/* Line 1806 of yacc.c  */
#line 2562 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);}
    break;

  case 747:

/* Line 1806 of yacc.c  */
#line 2563 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 748:

/* Line 1806 of yacc.c  */
#line 2564 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);}
    break;

  case 749:

/* Line 1806 of yacc.c  */
#line 2569 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 750:

/* Line 1806 of yacc.c  */
#line 2570 "hphp.y"
    { _p->onEmptyCollection((yyval));}
    break;

  case 751:

/* Line 1806 of yacc.c  */
#line 2574 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 752:

/* Line 1806 of yacc.c  */
#line 2575 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));}
    break;

  case 753:

/* Line 1806 of yacc.c  */
#line 2576 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 754:

/* Line 1806 of yacc.c  */
#line 2577 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));}
    break;

  case 755:

/* Line 1806 of yacc.c  */
#line 2582 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 756:

/* Line 1806 of yacc.c  */
#line 2583 "hphp.y"
    { _p->onEmptyCollection((yyval));}
    break;

  case 757:

/* Line 1806 of yacc.c  */
#line 2588 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 758:

/* Line 1806 of yacc.c  */
#line 2590 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));}
    break;

  case 759:

/* Line 1806 of yacc.c  */
#line 2592 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 760:

/* Line 1806 of yacc.c  */
#line 2593 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));}
    break;

  case 761:

/* Line 1806 of yacc.c  */
#line 2597 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);}
    break;

  case 762:

/* Line 1806 of yacc.c  */
#line 2599 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);}
    break;

  case 763:

/* Line 1806 of yacc.c  */
#line 2600 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);}
    break;

  case 764:

/* Line 1806 of yacc.c  */
#line 2602 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); }
    break;

  case 765:

/* Line 1806 of yacc.c  */
#line 2607 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 766:

/* Line 1806 of yacc.c  */
#line 2609 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 767:

/* Line 1806 of yacc.c  */
#line 2611 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 768:

/* Line 1806 of yacc.c  */
#line 2613 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);}
    break;

  case 769:

/* Line 1806 of yacc.c  */
#line 2615 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));}
    break;

  case 770:

/* Line 1806 of yacc.c  */
#line 2616 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 771:

/* Line 1806 of yacc.c  */
#line 2619 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;}
    break;

  case 772:

/* Line 1806 of yacc.c  */
#line 2620 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;}
    break;

  case 773:

/* Line 1806 of yacc.c  */
#line 2621 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;}
    break;

  case 774:

/* Line 1806 of yacc.c  */
#line 2625 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);}
    break;

  case 775:

/* Line 1806 of yacc.c  */
#line 2626 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);}
    break;

  case 776:

/* Line 1806 of yacc.c  */
#line 2627 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);}
    break;

  case 777:

/* Line 1806 of yacc.c  */
#line 2628 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);}
    break;

  case 778:

/* Line 1806 of yacc.c  */
#line 2629 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);}
    break;

  case 779:

/* Line 1806 of yacc.c  */
#line 2630 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);}
    break;

  case 780:

/* Line 1806 of yacc.c  */
#line 2631 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);}
    break;

  case 781:

/* Line 1806 of yacc.c  */
#line 2632 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);}
    break;

  case 782:

/* Line 1806 of yacc.c  */
#line 2633 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);}
    break;

  case 783:

/* Line 1806 of yacc.c  */
#line 2637 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 784:

/* Line 1806 of yacc.c  */
#line 2638 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 785:

/* Line 1806 of yacc.c  */
#line 2643 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 786:

/* Line 1806 of yacc.c  */
#line 2645 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 789:

/* Line 1806 of yacc.c  */
#line 2659 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); }
    break;

  case 790:

/* Line 1806 of yacc.c  */
#line 2663 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); }
    break;

  case 791:

/* Line 1806 of yacc.c  */
#line 2669 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 792:

/* Line 1806 of yacc.c  */
#line 2670 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 793:

/* Line 1806 of yacc.c  */
#line 2676 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 794:

/* Line 1806 of yacc.c  */
#line 2680 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); }
    break;

  case 795:

/* Line 1806 of yacc.c  */
#line 2686 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 796:

/* Line 1806 of yacc.c  */
#line 2687 "hphp.y"
    { (yyval).reset(); }
    break;

  case 797:

/* Line 1806 of yacc.c  */
#line 2691 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 798:

/* Line 1806 of yacc.c  */
#line 2694 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); }
    break;

  case 799:

/* Line 1806 of yacc.c  */
#line 2699 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); }
    break;

  case 800:

/* Line 1806 of yacc.c  */
#line 2700 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 801:

/* Line 1806 of yacc.c  */
#line 2701 "hphp.y"
    { (yyval).reset(); }
    break;

  case 802:

/* Line 1806 of yacc.c  */
#line 2702 "hphp.y"
    { (yyval).reset(); }
    break;

  case 803:

/* Line 1806 of yacc.c  */
#line 2706 "hphp.y"
    { (yyval).reset(); }
    break;

  case 804:

/* Line 1806 of yacc.c  */
#line 2707 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 805:

/* Line 1806 of yacc.c  */
#line 2712 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); }
    break;

  case 806:

/* Line 1806 of yacc.c  */
#line 2713 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); }
    break;

  case 807:

/* Line 1806 of yacc.c  */
#line 2715 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); }
    break;

  case 808:

/* Line 1806 of yacc.c  */
#line 2716 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); }
    break;

  case 809:

/* Line 1806 of yacc.c  */
#line 2722 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); }
    break;

  case 812:

/* Line 1806 of yacc.c  */
#line 2733 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 813:

/* Line 1806 of yacc.c  */
#line 2735 "hphp.y"
    {}
    break;

  case 814:

/* Line 1806 of yacc.c  */
#line 2739 "hphp.y"
    { (yyval).setText("array"); }
    break;

  case 815:

/* Line 1806 of yacc.c  */
#line 2746 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 816:

/* Line 1806 of yacc.c  */
#line 2749 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 817:

/* Line 1806 of yacc.c  */
#line 2752 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 818:

/* Line 1806 of yacc.c  */
#line 2753 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); }
    break;

  case 819:

/* Line 1806 of yacc.c  */
#line 2756 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); }
    break;

  case 820:

/* Line 1806 of yacc.c  */
#line 2759 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 821:

/* Line 1806 of yacc.c  */
#line 2761 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); }
    break;

  case 822:

/* Line 1806 of yacc.c  */
#line 2764 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); }
    break;

  case 823:

/* Line 1806 of yacc.c  */
#line 2767 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
    break;

  case 824:

/* Line 1806 of yacc.c  */
#line 2773 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
    break;

  case 825:

/* Line 1806 of yacc.c  */
#line 2777 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (5)]));
                                        _p->onTypeSpecialization((yyval), 't'); }
    break;

  case 826:

/* Line 1806 of yacc.c  */
#line 2785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 827:

/* Line 1806 of yacc.c  */
#line 2786 "hphp.y"
    { (yyval).reset(); }
    break;



/* Line 1806 of yacc.c  */
#line 12341 "hphp.tab.cpp"
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

  yyerror_range[1] = yylsp[1-yylen];
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

  *++yyvsp = yylval;

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
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, _p);
    }
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



/* Line 2067 of yacc.c  */
#line 2789 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

