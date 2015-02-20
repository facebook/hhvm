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

// macros for bison
#define YYSTYPE HPHP::HPHP_PARSER_NS::Token
#define YYSTYPE_IS_TRIVIAL false
#define YYLTYPE HPHP::Location
#define YYLTYPE_IS_TRIVIAL true
#define YYERROR_VERBOSE
#define YYINITDEPTH 500
#define YYLEX_PARAM _p

#include "hphp/compiler/parser/parser.h"
#include <folly/Conv.h>
#include "hphp/util/text-util.h"
#include "hphp/util/logger.h"

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
      (Current).line0 = (Current).line1 = YYRHSLOC (Rhs, 0).line1;      \
      (Current).char0 = (Current).char1 = YYRHSLOC (Rhs, 0).char1;      \
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

static int yylex(YYSTYPE *token, HPHP::Location *loc, Parser *_p) {
  return _p->scan(token, loc);
}


/* Line 189 of yacc.c  */
#line 646 "hphp.tab.cpp"

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
     T_BOOLEAN_OR = 282,
     T_BOOLEAN_AND = 283,
     T_IS_NOT_IDENTICAL = 284,
     T_IS_IDENTICAL = 285,
     T_IS_NOT_EQUAL = 286,
     T_IS_EQUAL = 287,
     T_IS_GREATER_OR_EQUAL = 288,
     T_IS_SMALLER_OR_EQUAL = 289,
     T_SR = 290,
     T_SL = 291,
     T_INSTANCEOF = 292,
     T_UNSET_CAST = 293,
     T_BOOL_CAST = 294,
     T_OBJECT_CAST = 295,
     T_ARRAY_CAST = 296,
     T_STRING_CAST = 297,
     T_DOUBLE_CAST = 298,
     T_INT_CAST = 299,
     T_DEC = 300,
     T_INC = 301,
     T_POW = 302,
     T_CLONE = 303,
     T_NEW = 304,
     T_EXIT = 305,
     T_IF = 306,
     T_ELSEIF = 307,
     T_ELSE = 308,
     T_ENDIF = 309,
     T_LNUMBER = 310,
     T_DNUMBER = 311,
     T_ONUMBER = 312,
     T_STRING = 313,
     T_STRING_VARNAME = 314,
     T_VARIABLE = 315,
     T_NUM_STRING = 316,
     T_INLINE_HTML = 317,
     T_HASHBANG = 318,
     T_CHARACTER = 319,
     T_BAD_CHARACTER = 320,
     T_ENCAPSED_AND_WHITESPACE = 321,
     T_CONSTANT_ENCAPSED_STRING = 322,
     T_ECHO = 323,
     T_DO = 324,
     T_WHILE = 325,
     T_ENDWHILE = 326,
     T_FOR = 327,
     T_ENDFOR = 328,
     T_FOREACH = 329,
     T_ENDFOREACH = 330,
     T_DECLARE = 331,
     T_ENDDECLARE = 332,
     T_AS = 333,
     T_SWITCH = 334,
     T_ENDSWITCH = 335,
     T_CASE = 336,
     T_DEFAULT = 337,
     T_BREAK = 338,
     T_GOTO = 339,
     T_CONTINUE = 340,
     T_FUNCTION = 341,
     T_CONST = 342,
     T_RETURN = 343,
     T_TRY = 344,
     T_CATCH = 345,
     T_THROW = 346,
     T_USE = 347,
     T_GLOBAL = 348,
     T_PUBLIC = 349,
     T_PROTECTED = 350,
     T_PRIVATE = 351,
     T_FINAL = 352,
     T_ABSTRACT = 353,
     T_STATIC = 354,
     T_VAR = 355,
     T_UNSET = 356,
     T_ISSET = 357,
     T_EMPTY = 358,
     T_HALT_COMPILER = 359,
     T_CLASS = 360,
     T_INTERFACE = 361,
     T_EXTENDS = 362,
     T_IMPLEMENTS = 363,
     T_OBJECT_OPERATOR = 364,
     T_NULLSAFE_OBJECT_OPERATOR = 365,
     T_DOUBLE_ARROW = 366,
     T_LIST = 367,
     T_ARRAY = 368,
     T_CALLABLE = 369,
     T_CLASS_C = 370,
     T_METHOD_C = 371,
     T_FUNC_C = 372,
     T_LINE = 373,
     T_FILE = 374,
     T_COMMENT = 375,
     T_DOC_COMMENT = 376,
     T_OPEN_TAG = 377,
     T_OPEN_TAG_WITH_ECHO = 378,
     T_CLOSE_TAG = 379,
     T_WHITESPACE = 380,
     T_START_HEREDOC = 381,
     T_END_HEREDOC = 382,
     T_DOLLAR_OPEN_CURLY_BRACES = 383,
     T_CURLY_OPEN = 384,
     T_DOUBLE_COLON = 385,
     T_NAMESPACE = 386,
     T_NS_C = 387,
     T_DIR = 388,
     T_NS_SEPARATOR = 389,
     T_XHP_LABEL = 390,
     T_XHP_TEXT = 391,
     T_XHP_ATTRIBUTE = 392,
     T_XHP_CATEGORY = 393,
     T_XHP_CATEGORY_LABEL = 394,
     T_XHP_CHILDREN = 395,
     T_ENUM = 396,
     T_XHP_REQUIRED = 397,
     T_TRAIT = 398,
     T_ELLIPSIS = 399,
     T_INSTEADOF = 400,
     T_TRAIT_C = 401,
     T_HH_ERROR = 402,
     T_FINALLY = 403,
     T_XHP_TAG_LT = 404,
     T_XHP_TAG_GT = 405,
     T_TYPELIST_LT = 406,
     T_TYPELIST_GT = 407,
     T_UNRESOLVED_LT = 408,
     T_COLLECTION = 409,
     T_SHAPE = 410,
     T_TYPE = 411,
     T_UNRESOLVED_TYPE = 412,
     T_NEWTYPE = 413,
     T_UNRESOLVED_NEWTYPE = 414,
     T_COMPILER_HALT_OFFSET = 415,
     T_ASYNC = 416,
     T_FROM = 417,
     T_WHERE = 418,
     T_JOIN = 419,
     T_IN = 420,
     T_ON = 421,
     T_EQUALS = 422,
     T_INTO = 423,
     T_LET = 424,
     T_ORDERBY = 425,
     T_ASCENDING = 426,
     T_DESCENDING = 427,
     T_SELECT = 428,
     T_GROUP = 429,
     T_BY = 430,
     T_LAMBDA_OP = 431,
     T_LAMBDA_CP = 432,
     T_UNRESOLVED_OP = 433
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
#line 879 "hphp.tab.cpp"

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
#define YYLAST   16459

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  208
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  273
/* YYNRULES -- Number of rules.  */
#define YYNRULES  931
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1743

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   433

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    52,   206,     2,   203,    51,    35,   207,
     198,   199,    49,    46,     9,    47,    48,    50,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    30,   200,
      40,    14,    41,    29,    55,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    66,     2,   205,    34,     2,   204,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   201,    33,   202,    54,     2,     2,     2,
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
      27,    28,    31,    32,    36,    37,    38,    39,    42,    43,
      44,    45,    53,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    67,    68,    69,    70,    71,    72,    73,
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
     194,   195,   196,   197
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    11,    13,    15,    17,
      19,    21,    23,    28,    32,    33,    40,    41,    47,    51,
      56,    61,    64,    66,    68,    70,    72,    74,    76,    78,
      80,    82,    84,    86,    88,    90,    92,    94,    96,    98,
     100,   102,   106,   108,   112,   114,   118,   120,   122,   125,
     129,   134,   136,   139,   143,   148,   150,   153,   157,   162,
     164,   168,   170,   174,   177,   179,   182,   185,   191,   196,
     199,   200,   202,   204,   206,   208,   212,   218,   227,   228,
     233,   234,   241,   242,   253,   254,   259,   262,   266,   269,
     273,   276,   280,   284,   288,   292,   296,   302,   304,   306,
     308,   309,   319,   320,   331,   337,   338,   352,   353,   359,
     363,   367,   370,   373,   376,   379,   382,   385,   389,   392,
     395,   399,   402,   403,   408,   418,   419,   420,   425,   428,
     429,   431,   432,   434,   435,   445,   446,   457,   458,   470,
     471,   481,   482,   493,   494,   503,   504,   514,   515,   523,
     524,   533,   534,   542,   543,   552,   554,   556,   558,   560,
     562,   565,   569,   573,   576,   579,   580,   583,   584,   587,
     588,   590,   594,   596,   600,   603,   604,   606,   609,   614,
     616,   621,   623,   628,   630,   635,   637,   642,   646,   652,
     656,   661,   666,   672,   678,   683,   684,   686,   688,   693,
     694,   700,   701,   704,   705,   709,   710,   718,   727,   734,
     737,   743,   750,   755,   756,   761,   767,   775,   782,   789,
     797,   807,   816,   823,   831,   837,   840,   845,   851,   855,
     856,   860,   865,   872,   878,   884,   891,   900,   908,   911,
     912,   914,   917,   920,   924,   929,   934,   938,   940,   942,
     945,   950,   954,   960,   962,   966,   969,   970,   973,   977,
     980,   981,   982,   987,   988,   994,   997,  1000,  1003,  1004,
    1015,  1016,  1028,  1032,  1036,  1040,  1045,  1050,  1054,  1060,
    1063,  1066,  1067,  1074,  1080,  1085,  1089,  1091,  1093,  1097,
    1102,  1104,  1106,  1111,  1118,  1120,  1122,  1127,  1129,  1131,
    1135,  1138,  1139,  1142,  1143,  1145,  1149,  1151,  1153,  1155,
    1157,  1161,  1166,  1171,  1176,  1178,  1180,  1183,  1186,  1189,
    1193,  1197,  1199,  1201,  1203,  1205,  1209,  1211,  1215,  1217,
    1219,  1221,  1222,  1224,  1227,  1229,  1231,  1233,  1235,  1237,
    1239,  1241,  1243,  1244,  1246,  1248,  1250,  1254,  1260,  1262,
    1266,  1272,  1277,  1281,  1285,  1289,  1294,  1298,  1302,  1306,
    1309,  1311,  1313,  1317,  1321,  1323,  1325,  1326,  1328,  1331,
    1336,  1340,  1347,  1350,  1354,  1361,  1363,  1365,  1367,  1369,
    1371,  1378,  1382,  1387,  1394,  1398,  1402,  1406,  1410,  1414,
    1418,  1422,  1426,  1430,  1434,  1438,  1442,  1445,  1448,  1451,
    1454,  1458,  1462,  1466,  1470,  1474,  1478,  1482,  1486,  1490,
    1494,  1498,  1502,  1506,  1510,  1514,  1518,  1522,  1525,  1528,
    1531,  1534,  1538,  1542,  1546,  1550,  1554,  1558,  1562,  1566,
    1570,  1574,  1580,  1585,  1587,  1590,  1593,  1596,  1599,  1602,
    1605,  1608,  1611,  1614,  1616,  1618,  1620,  1624,  1627,  1629,
    1635,  1636,  1637,  1649,  1650,  1663,  1664,  1668,  1669,  1674,
    1675,  1682,  1683,  1691,  1692,  1698,  1701,  1704,  1709,  1711,
    1713,  1719,  1723,  1729,  1733,  1736,  1737,  1740,  1741,  1746,
    1751,  1755,  1760,  1765,  1770,  1775,  1777,  1779,  1781,  1783,
    1787,  1790,  1794,  1799,  1802,  1806,  1808,  1811,  1813,  1816,
    1818,  1820,  1822,  1824,  1826,  1828,  1833,  1838,  1841,  1850,
    1861,  1864,  1866,  1870,  1872,  1875,  1877,  1879,  1881,  1883,
    1886,  1891,  1895,  1899,  1904,  1906,  1909,  1914,  1917,  1924,
    1925,  1927,  1932,  1933,  1936,  1937,  1939,  1941,  1945,  1947,
    1951,  1953,  1955,  1959,  1963,  1965,  1967,  1969,  1971,  1973,
    1975,  1977,  1979,  1981,  1983,  1985,  1987,  1989,  1991,  1993,
    1995,  1997,  1999,  2001,  2003,  2005,  2007,  2009,  2011,  2013,
    2015,  2017,  2019,  2021,  2023,  2025,  2027,  2029,  2031,  2033,
    2035,  2037,  2039,  2041,  2043,  2045,  2047,  2049,  2051,  2053,
    2055,  2057,  2059,  2061,  2063,  2065,  2067,  2069,  2071,  2073,
    2075,  2077,  2079,  2081,  2083,  2085,  2087,  2089,  2091,  2093,
    2095,  2097,  2099,  2101,  2103,  2105,  2107,  2109,  2111,  2113,
    2115,  2117,  2119,  2121,  2123,  2128,  2130,  2132,  2134,  2136,
    2138,  2140,  2142,  2144,  2147,  2149,  2150,  2151,  2153,  2155,
    2159,  2160,  2162,  2164,  2166,  2168,  2170,  2172,  2174,  2176,
    2178,  2180,  2182,  2184,  2186,  2190,  2193,  2195,  2197,  2202,
    2206,  2211,  2213,  2215,  2219,  2223,  2227,  2231,  2235,  2239,
    2243,  2247,  2251,  2255,  2259,  2263,  2267,  2271,  2275,  2279,
    2283,  2287,  2290,  2293,  2296,  2299,  2303,  2307,  2311,  2315,
    2319,  2323,  2327,  2331,  2337,  2342,  2346,  2350,  2354,  2356,
    2358,  2360,  2362,  2366,  2370,  2374,  2377,  2378,  2380,  2381,
    2383,  2384,  2390,  2394,  2398,  2400,  2402,  2404,  2406,  2410,
    2413,  2415,  2417,  2419,  2421,  2423,  2427,  2429,  2431,  2433,
    2436,  2439,  2444,  2448,  2453,  2456,  2457,  2463,  2467,  2471,
    2473,  2477,  2479,  2482,  2483,  2489,  2493,  2496,  2497,  2501,
    2502,  2507,  2510,  2511,  2515,  2519,  2521,  2522,  2524,  2526,
    2528,  2530,  2534,  2536,  2538,  2540,  2544,  2546,  2548,  2552,
    2556,  2559,  2564,  2567,  2572,  2574,  2576,  2578,  2580,  2582,
    2586,  2592,  2596,  2601,  2606,  2610,  2612,  2614,  2616,  2618,
    2622,  2628,  2633,  2637,  2639,  2641,  2645,  2649,  2651,  2653,
    2661,  2671,  2679,  2686,  2695,  2697,  2700,  2705,  2710,  2712,
    2714,  2719,  2721,  2722,  2724,  2727,  2729,  2731,  2735,  2741,
    2745,  2749,  2750,  2752,  2756,  2762,  2766,  2769,  2773,  2780,
    2781,  2783,  2788,  2791,  2792,  2798,  2802,  2806,  2808,  2815,
    2820,  2825,  2828,  2831,  2832,  2838,  2842,  2846,  2848,  2851,
    2852,  2858,  2862,  2866,  2868,  2871,  2874,  2876,  2879,  2881,
    2886,  2890,  2894,  2901,  2905,  2907,  2909,  2911,  2916,  2921,
    2926,  2931,  2936,  2941,  2944,  2947,  2952,  2955,  2958,  2960,
    2964,  2968,  2972,  2973,  2976,  2982,  2989,  2991,  2994,  2996,
    3001,  3005,  3006,  3008,  3012,  3015,  3019,  3021,  3023,  3024,
    3025,  3028,  3033,  3036,  3043,  3048,  3050,  3052,  3053,  3057,
    3063,  3067,  3069,  3072,  3073,  3078,  3080,  3084,  3087,  3090,
    3093,  3095,  3097,  3099,  3101,  3105,  3110,  3117,  3119,  3128,
    3135,  3137
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     209,     0,    -1,    -1,   210,   211,    -1,   211,   212,    -1,
      -1,   230,    -1,   247,    -1,   254,    -1,   251,    -1,   259,
      -1,   463,    -1,   123,   198,   199,   200,    -1,   150,   222,
     200,    -1,    -1,   150,   222,   201,   213,   211,   202,    -1,
      -1,   150,   201,   214,   211,   202,    -1,   111,   216,   200,
      -1,   111,   105,   217,   200,    -1,   111,   106,   218,   200,
      -1,   227,   200,    -1,    77,    -1,   156,    -1,   157,    -1,
     159,    -1,   161,    -1,   160,    -1,   182,    -1,   183,    -1,
     185,    -1,   184,    -1,   186,    -1,   187,    -1,   188,    -1,
     189,    -1,   190,    -1,   191,    -1,   192,    -1,   193,    -1,
     194,    -1,   216,     9,   219,    -1,   219,    -1,   220,     9,
     220,    -1,   220,    -1,   221,     9,   221,    -1,   221,    -1,
     222,    -1,   153,   222,    -1,   222,    97,   215,    -1,   153,
     222,    97,   215,    -1,   222,    -1,   153,   222,    -1,   222,
      97,   215,    -1,   153,   222,    97,   215,    -1,   222,    -1,
     153,   222,    -1,   222,    97,   215,    -1,   153,   222,    97,
     215,    -1,   215,    -1,   222,   153,   215,    -1,   222,    -1,
     150,   153,   222,    -1,   153,   222,    -1,   223,    -1,   223,
     466,    -1,   223,   466,    -1,   227,     9,   464,    14,   404,
      -1,   106,   464,    14,   404,    -1,   228,   229,    -1,    -1,
     230,    -1,   247,    -1,   254,    -1,   259,    -1,   201,   228,
     202,    -1,    70,   333,   230,   281,   283,    -1,    70,   333,
      30,   228,   282,   284,    73,   200,    -1,    -1,    89,   333,
     231,   275,    -1,    -1,    88,   232,   230,    89,   333,   200,
      -1,    -1,    91,   198,   335,   200,   335,   200,   335,   199,
     233,   273,    -1,    -1,    98,   333,   234,   278,    -1,   102,
     200,    -1,   102,   342,   200,    -1,   104,   200,    -1,   104,
     342,   200,    -1,   107,   200,    -1,   107,   342,   200,    -1,
      27,   102,   200,    -1,   112,   291,   200,    -1,   118,   293,
     200,    -1,    87,   334,   200,    -1,   120,   198,   460,   199,
     200,    -1,   200,    -1,    81,    -1,    82,    -1,    -1,    93,
     198,   342,    97,   272,   271,   199,   235,   274,    -1,    -1,
      93,   198,   342,    28,    97,   272,   271,   199,   236,   274,
      -1,    95,   198,   277,   199,   276,    -1,    -1,   108,   239,
     109,   198,   397,    79,   199,   201,   228,   202,   241,   237,
     244,    -1,    -1,   108,   239,   167,   238,   242,    -1,   110,
     342,   200,    -1,   103,   215,   200,    -1,   342,   200,    -1,
     336,   200,    -1,   337,   200,    -1,   338,   200,    -1,   339,
     200,    -1,   340,   200,    -1,   107,   339,   200,    -1,   341,
     200,    -1,   367,   200,    -1,   107,   366,   200,    -1,   215,
      30,    -1,    -1,   201,   240,   228,   202,    -1,   241,   109,
     198,   397,    79,   199,   201,   228,   202,    -1,    -1,    -1,
     201,   243,   228,   202,    -1,   167,   242,    -1,    -1,    35,
      -1,    -1,   105,    -1,    -1,   246,   245,   465,   248,   198,
     287,   199,   470,   319,    -1,    -1,   323,   246,   245,   465,
     249,   198,   287,   199,   470,   319,    -1,    -1,   425,   322,
     246,   245,   465,   250,   198,   287,   199,   470,   319,    -1,
      -1,   160,   215,   252,    30,   479,   462,   201,   294,   202,
      -1,    -1,   425,   160,   215,   253,    30,   479,   462,   201,
     294,   202,    -1,    -1,   265,   262,   255,   266,   267,   201,
     297,   202,    -1,    -1,   425,   265,   262,   256,   266,   267,
     201,   297,   202,    -1,    -1,   125,   263,   257,   268,   201,
     297,   202,    -1,    -1,   425,   125,   263,   258,   268,   201,
     297,   202,    -1,    -1,   162,   264,   260,   267,   201,   297,
     202,    -1,    -1,   425,   162,   264,   261,   267,   201,   297,
     202,    -1,   465,    -1,   154,    -1,   465,    -1,   465,    -1,
     124,    -1,   117,   124,    -1,   117,   116,   124,    -1,   116,
     117,   124,    -1,   116,   124,    -1,   126,   397,    -1,    -1,
     127,   269,    -1,    -1,   126,   269,    -1,    -1,   397,    -1,
     269,     9,   397,    -1,   397,    -1,   270,     9,   397,    -1,
     130,   272,    -1,    -1,   435,    -1,    35,   435,    -1,   131,
     198,   449,   199,    -1,   230,    -1,    30,   228,    92,   200,
      -1,   230,    -1,    30,   228,    94,   200,    -1,   230,    -1,
      30,   228,    90,   200,    -1,   230,    -1,    30,   228,    96,
     200,    -1,   215,    14,   404,    -1,   277,     9,   215,    14,
     404,    -1,   201,   279,   202,    -1,   201,   200,   279,   202,
      -1,    30,   279,    99,   200,    -1,    30,   200,   279,    99,
     200,    -1,   279,   100,   342,   280,   228,    -1,   279,   101,
     280,   228,    -1,    -1,    30,    -1,   200,    -1,   281,    71,
     333,   230,    -1,    -1,   282,    71,   333,    30,   228,    -1,
      -1,    72,   230,    -1,    -1,    72,    30,   228,    -1,    -1,
     286,     9,   426,   325,   480,   163,    79,    -1,   286,     9,
     426,   325,   480,    35,   163,    79,    -1,   286,     9,   426,
     325,   480,   163,    -1,   286,   409,    -1,   426,   325,   480,
     163,    79,    -1,   426,   325,   480,    35,   163,    79,    -1,
     426,   325,   480,   163,    -1,    -1,   426,   325,   480,    79,
      -1,   426,   325,   480,    35,    79,    -1,   426,   325,   480,
      35,    79,    14,   342,    -1,   426,   325,   480,    79,    14,
     342,    -1,   286,     9,   426,   325,   480,    79,    -1,   286,
       9,   426,   325,   480,    35,    79,    -1,   286,     9,   426,
     325,   480,    35,    79,    14,   342,    -1,   286,     9,   426,
     325,   480,    79,    14,   342,    -1,   288,     9,   426,   480,
     163,    79,    -1,   288,     9,   426,   480,    35,   163,    79,
      -1,   288,     9,   426,   480,   163,    -1,   288,   409,    -1,
     426,   480,   163,    79,    -1,   426,   480,    35,   163,    79,
      -1,   426,   480,   163,    -1,    -1,   426,   480,    79,    -1,
     426,   480,    35,    79,    -1,   426,   480,    35,    79,    14,
     342,    -1,   426,   480,    79,    14,   342,    -1,   288,     9,
     426,   480,    79,    -1,   288,     9,   426,   480,    35,    79,
      -1,   288,     9,   426,   480,    35,    79,    14,   342,    -1,
     288,     9,   426,   480,    79,    14,   342,    -1,   290,   409,
      -1,    -1,   342,    -1,    35,   435,    -1,   163,   342,    -1,
     290,     9,   342,    -1,   290,     9,   163,   342,    -1,   290,
       9,    35,   435,    -1,   291,     9,   292,    -1,   292,    -1,
      79,    -1,   203,   435,    -1,   203,   201,   342,   202,    -1,
     293,     9,    79,    -1,   293,     9,    79,    14,   404,    -1,
      79,    -1,    79,    14,   404,    -1,   294,   295,    -1,    -1,
     296,   200,    -1,   464,    14,   404,    -1,   297,   298,    -1,
      -1,    -1,   321,   299,   327,   200,    -1,    -1,   323,   479,
     300,   327,   200,    -1,   328,   200,    -1,   329,   200,    -1,
     330,   200,    -1,    -1,   322,   246,   245,   465,   198,   301,
     285,   199,   470,   320,    -1,    -1,   425,   322,   246,   245,
     465,   198,   302,   285,   199,   470,   320,    -1,   156,   307,
     200,    -1,   157,   313,   200,    -1,   159,   315,   200,    -1,
       4,   126,   397,   200,    -1,     4,   127,   397,   200,    -1,
     111,   270,   200,    -1,   111,   270,   201,   303,   202,    -1,
     303,   304,    -1,   303,   305,    -1,    -1,   226,   149,   215,
     164,   270,   200,    -1,   306,    97,   322,   215,   200,    -1,
     306,    97,   323,   200,    -1,   226,   149,   215,    -1,   215,
      -1,   308,    -1,   307,     9,   308,    -1,   309,   394,   311,
     312,    -1,   154,    -1,   132,    -1,   132,   170,   479,   171,
      -1,   132,   170,   479,     9,   479,   171,    -1,   397,    -1,
     119,    -1,   160,   201,   310,   202,    -1,   133,    -1,   403,
      -1,   310,     9,   403,    -1,    14,   404,    -1,    -1,    55,
     161,    -1,    -1,   314,    -1,   313,     9,   314,    -1,   158,
      -1,   316,    -1,   215,    -1,   122,    -1,   198,   317,   199,
      -1,   198,   317,   199,    49,    -1,   198,   317,   199,    29,
      -1,   198,   317,   199,    46,    -1,   316,    -1,   318,    -1,
     318,    49,    -1,   318,    29,    -1,   318,    46,    -1,   317,
       9,   317,    -1,   317,    33,   317,    -1,   215,    -1,   154,
      -1,   158,    -1,   200,    -1,   201,   228,   202,    -1,   200,
      -1,   201,   228,   202,    -1,   323,    -1,   119,    -1,   323,
      -1,    -1,   324,    -1,   323,   324,    -1,   113,    -1,   114,
      -1,   115,    -1,   118,    -1,   117,    -1,   116,    -1,   180,
      -1,   326,    -1,    -1,   113,    -1,   114,    -1,   115,    -1,
     327,     9,    79,    -1,   327,     9,    79,    14,   404,    -1,
      79,    -1,    79,    14,   404,    -1,   328,     9,   464,    14,
     404,    -1,   106,   464,    14,   404,    -1,   329,     9,   464,
      -1,   117,   106,   464,    -1,   117,   331,   462,    -1,   331,
     462,    14,   479,    -1,   106,   175,   465,    -1,   198,   332,
     199,    -1,    68,   399,   402,    -1,    67,   342,    -1,   386,
      -1,   362,    -1,   198,   342,   199,    -1,   334,     9,   342,
      -1,   342,    -1,   334,    -1,    -1,    27,    -1,    27,   342,
      -1,    27,   342,   130,   342,    -1,   435,    14,   336,    -1,
     131,   198,   449,   199,    14,   336,    -1,    28,   342,    -1,
     435,    14,   339,    -1,   131,   198,   449,   199,    14,   339,
      -1,   343,    -1,   435,    -1,   332,    -1,   439,    -1,   438,
      -1,   131,   198,   449,   199,    14,   342,    -1,   435,    14,
     342,    -1,   435,    14,    35,   435,    -1,   435,    14,    35,
      68,   399,   402,    -1,   435,    26,   342,    -1,   435,    25,
     342,    -1,   435,    24,   342,    -1,   435,    23,   342,    -1,
     435,    22,   342,    -1,   435,    21,   342,    -1,   435,    20,
     342,    -1,   435,    19,   342,    -1,   435,    18,   342,    -1,
     435,    17,   342,    -1,   435,    16,   342,    -1,   435,    15,
     342,    -1,   435,    64,    -1,    64,   435,    -1,   435,    63,
      -1,    63,   435,    -1,   342,    31,   342,    -1,   342,    32,
     342,    -1,   342,    10,   342,    -1,   342,    12,   342,    -1,
     342,    11,   342,    -1,   342,    33,   342,    -1,   342,    35,
     342,    -1,   342,    34,   342,    -1,   342,    48,   342,    -1,
     342,    46,   342,    -1,   342,    47,   342,    -1,   342,    49,
     342,    -1,   342,    50,   342,    -1,   342,    65,   342,    -1,
     342,    51,   342,    -1,   342,    45,   342,    -1,   342,    44,
     342,    -1,    46,   342,    -1,    47,   342,    -1,    52,   342,
      -1,    54,   342,    -1,   342,    37,   342,    -1,   342,    36,
     342,    -1,   342,    39,   342,    -1,   342,    38,   342,    -1,
     342,    40,   342,    -1,   342,    43,   342,    -1,   342,    41,
     342,    -1,   342,    42,   342,    -1,   342,    53,   399,    -1,
     198,   343,   199,    -1,   342,    29,   342,    30,   342,    -1,
     342,    29,    30,   342,    -1,   459,    -1,    62,   342,    -1,
      61,   342,    -1,    60,   342,    -1,    59,   342,    -1,    58,
     342,    -1,    57,   342,    -1,    56,   342,    -1,    69,   400,
      -1,    55,   342,    -1,   406,    -1,   361,    -1,   360,    -1,
     204,   401,   204,    -1,    13,   342,    -1,   364,    -1,   111,
     198,   385,   409,   199,    -1,    -1,    -1,   246,   245,   198,
     346,   287,   199,   470,   344,   201,   228,   202,    -1,    -1,
     323,   246,   245,   198,   347,   287,   199,   470,   344,   201,
     228,   202,    -1,    -1,    79,   349,   354,    -1,    -1,   180,
      79,   350,   354,    -1,    -1,   195,   351,   287,   196,   470,
     354,    -1,    -1,   180,   195,   352,   287,   196,   470,   354,
      -1,    -1,   180,   201,   353,   228,   202,    -1,     8,   342,
      -1,     8,   339,    -1,     8,   201,   228,   202,    -1,    86,
      -1,   461,    -1,   356,     9,   355,   130,   342,    -1,   355,
     130,   342,    -1,   357,     9,   355,   130,   404,    -1,   355,
     130,   404,    -1,   356,   408,    -1,    -1,   357,   408,    -1,
      -1,   174,   198,   358,   199,    -1,   132,   198,   450,   199,
      -1,    66,   450,   205,    -1,   397,   201,   452,   202,    -1,
     397,   201,   454,   202,    -1,   364,    66,   445,   205,    -1,
     365,    66,   445,   205,    -1,   361,    -1,   461,    -1,   438,
      -1,    86,    -1,   198,   343,   199,    -1,   368,   369,    -1,
     435,    14,   366,    -1,   181,    79,   184,   342,    -1,   370,
     381,    -1,   370,   381,   384,    -1,   381,    -1,   381,   384,
      -1,   371,    -1,   370,   371,    -1,   372,    -1,   373,    -1,
     374,    -1,   375,    -1,   376,    -1,   377,    -1,   181,    79,
     184,   342,    -1,   188,    79,    14,   342,    -1,   182,   342,
      -1,   183,    79,   184,   342,   185,   342,   186,   342,    -1,
     183,    79,   184,   342,   185,   342,   186,   342,   187,    79,
      -1,   189,   378,    -1,   379,    -1,   378,     9,   379,    -1,
     342,    -1,   342,   380,    -1,   190,    -1,   191,    -1,   382,
      -1,   383,    -1,   192,   342,    -1,   193,   342,   194,   342,
      -1,   187,    79,   369,    -1,   385,     9,    79,    -1,   385,
       9,    35,    79,    -1,    79,    -1,    35,    79,    -1,   168,
     154,   387,   169,    -1,   389,    50,    -1,   389,   169,   390,
     168,    50,   388,    -1,    -1,   154,    -1,   389,   391,    14,
     392,    -1,    -1,   390,   393,    -1,    -1,   154,    -1,   155,
      -1,   201,   342,   202,    -1,   155,    -1,   201,   342,   202,
      -1,   386,    -1,   395,    -1,   394,    30,   395,    -1,   394,
      47,   395,    -1,   215,    -1,    69,    -1,   105,    -1,   106,
      -1,   107,    -1,    27,    -1,    28,    -1,   108,    -1,   109,
      -1,   167,    -1,   110,    -1,    70,    -1,    71,    -1,    73,
      -1,    72,    -1,    89,    -1,    90,    -1,    88,    -1,    91,
      -1,    92,    -1,    93,    -1,    94,    -1,    95,    -1,    96,
      -1,    53,    -1,    97,    -1,    98,    -1,    99,    -1,   100,
      -1,   101,    -1,   102,    -1,   104,    -1,   103,    -1,    87,
      -1,    13,    -1,   124,    -1,   125,    -1,   126,    -1,   127,
      -1,    68,    -1,    67,    -1,   119,    -1,     5,    -1,     7,
      -1,     6,    -1,     4,    -1,     3,    -1,   150,    -1,   111,
      -1,   112,    -1,   121,    -1,   122,    -1,   123,    -1,   118,
      -1,   117,    -1,   116,    -1,   115,    -1,   114,    -1,   113,
      -1,   180,    -1,   120,    -1,   131,    -1,   132,    -1,    10,
      -1,    12,    -1,    11,    -1,   134,    -1,   136,    -1,   135,
      -1,   137,    -1,   138,    -1,   152,    -1,   151,    -1,   179,
      -1,   162,    -1,   165,    -1,   164,    -1,   175,    -1,   177,
      -1,   174,    -1,   225,   198,   289,   199,    -1,   226,    -1,
     154,    -1,   397,    -1,   118,    -1,   443,    -1,   397,    -1,
     118,    -1,   447,    -1,   198,   199,    -1,   333,    -1,    -1,
      -1,    85,    -1,   456,    -1,   198,   289,   199,    -1,    -1,
      74,    -1,    75,    -1,    76,    -1,    86,    -1,   137,    -1,
     138,    -1,   152,    -1,   134,    -1,   165,    -1,   135,    -1,
     136,    -1,   151,    -1,   179,    -1,   145,    85,   146,    -1,
     145,   146,    -1,   403,    -1,   224,    -1,   132,   198,   407,
     199,    -1,    66,   407,   205,    -1,   174,   198,   359,   199,
      -1,   405,    -1,   363,    -1,   198,   404,   199,    -1,   404,
      31,   404,    -1,   404,    32,   404,    -1,   404,    10,   404,
      -1,   404,    12,   404,    -1,   404,    11,   404,    -1,   404,
      33,   404,    -1,   404,    35,   404,    -1,   404,    34,   404,
      -1,   404,    48,   404,    -1,   404,    46,   404,    -1,   404,
      47,   404,    -1,   404,    49,   404,    -1,   404,    50,   404,
      -1,   404,    51,   404,    -1,   404,    45,   404,    -1,   404,
      44,   404,    -1,   404,    65,   404,    -1,    52,   404,    -1,
      54,   404,    -1,    46,   404,    -1,    47,   404,    -1,   404,
      37,   404,    -1,   404,    36,   404,    -1,   404,    39,   404,
      -1,   404,    38,   404,    -1,   404,    40,   404,    -1,   404,
      43,   404,    -1,   404,    41,   404,    -1,   404,    42,   404,
      -1,   404,    29,   404,    30,   404,    -1,   404,    29,    30,
     404,    -1,   226,   149,   215,    -1,   154,   149,   215,    -1,
     226,   149,   124,    -1,   224,    -1,    78,    -1,   461,    -1,
     403,    -1,   206,   456,   206,    -1,   207,   456,   207,    -1,
     145,   456,   146,    -1,   410,   408,    -1,    -1,     9,    -1,
      -1,     9,    -1,    -1,   410,     9,   404,   130,   404,    -1,
     410,     9,   404,    -1,   404,   130,   404,    -1,   404,    -1,
      74,    -1,    75,    -1,    76,    -1,   145,    85,   146,    -1,
     145,   146,    -1,    74,    -1,    75,    -1,    76,    -1,   215,
      -1,    86,    -1,    86,    48,   413,    -1,   411,    -1,   413,
      -1,   215,    -1,    46,   412,    -1,    47,   412,    -1,   132,
     198,   415,   199,    -1,    66,   415,   205,    -1,   174,   198,
     418,   199,    -1,   416,   408,    -1,    -1,   416,     9,   414,
     130,   414,    -1,   416,     9,   414,    -1,   414,   130,   414,
      -1,   414,    -1,   417,     9,   414,    -1,   414,    -1,   419,
     408,    -1,    -1,   419,     9,   355,   130,   414,    -1,   355,
     130,   414,    -1,   417,   408,    -1,    -1,   198,   420,   199,
      -1,    -1,   422,     9,   215,   421,    -1,   215,   421,    -1,
      -1,   424,   422,   408,    -1,    45,   423,    44,    -1,   425,
      -1,    -1,   128,    -1,   129,    -1,   215,    -1,   154,    -1,
     201,   342,   202,    -1,   428,    -1,   442,    -1,   215,    -1,
     201,   342,   202,    -1,   430,    -1,   442,    -1,    66,   445,
     205,    -1,   201,   342,   202,    -1,   436,   432,    -1,   198,
     332,   199,   432,    -1,   448,   432,    -1,   198,   332,   199,
     432,    -1,   442,    -1,   396,    -1,   440,    -1,   441,    -1,
     433,    -1,   435,   427,   429,    -1,   198,   332,   199,   427,
     429,    -1,   398,   149,   442,    -1,   437,   198,   289,   199,
      -1,   438,   198,   289,   199,    -1,   198,   435,   199,    -1,
     396,    -1,   440,    -1,   441,    -1,   433,    -1,   435,   427,
     428,    -1,   198,   332,   199,   427,   428,    -1,   437,   198,
     289,   199,    -1,   198,   435,   199,    -1,   442,    -1,   433,
      -1,   198,   435,   199,    -1,   198,   439,   199,    -1,   345,
      -1,   348,    -1,   435,   427,   431,   466,   198,   289,   199,
      -1,   198,   332,   199,   427,   431,   466,   198,   289,   199,
      -1,   398,   149,   215,   466,   198,   289,   199,    -1,   398,
     149,   442,   198,   289,   199,    -1,   398,   149,   201,   342,
     202,   198,   289,   199,    -1,   443,    -1,   446,   443,    -1,
     443,    66,   445,   205,    -1,   443,   201,   342,   202,    -1,
     444,    -1,    79,    -1,   203,   201,   342,   202,    -1,   342,
      -1,    -1,   203,    -1,   446,   203,    -1,   442,    -1,   434,
      -1,   447,   427,   429,    -1,   198,   332,   199,   427,   429,
      -1,   398,   149,   442,    -1,   198,   435,   199,    -1,    -1,
     434,    -1,   447,   427,   428,    -1,   198,   332,   199,   427,
     428,    -1,   198,   435,   199,    -1,   449,     9,    -1,   449,
       9,   435,    -1,   449,     9,   131,   198,   449,   199,    -1,
      -1,   435,    -1,   131,   198,   449,   199,    -1,   451,   408,
      -1,    -1,   451,     9,   342,   130,   342,    -1,   451,     9,
     342,    -1,   342,   130,   342,    -1,   342,    -1,   451,     9,
     342,   130,    35,   435,    -1,   451,     9,    35,   435,    -1,
     342,   130,    35,   435,    -1,    35,   435,    -1,   453,   408,
      -1,    -1,   453,     9,   342,   130,   342,    -1,   453,     9,
     342,    -1,   342,   130,   342,    -1,   342,    -1,   455,   408,
      -1,    -1,   455,     9,   404,   130,   404,    -1,   455,     9,
     404,    -1,   404,   130,   404,    -1,   404,    -1,   456,   457,
      -1,   456,    85,    -1,   457,    -1,    85,   457,    -1,    79,
      -1,    79,    66,   458,   205,    -1,    79,   427,   215,    -1,
     147,   342,   202,    -1,   147,    78,    66,   342,   205,   202,
      -1,   148,   435,   202,    -1,   215,    -1,    80,    -1,    79,
      -1,   121,   198,   460,   199,    -1,   122,   198,   435,   199,
      -1,   122,   198,   343,   199,    -1,   122,   198,   439,   199,
      -1,   122,   198,   438,   199,    -1,   122,   198,   332,   199,
      -1,     7,   342,    -1,     6,   342,    -1,     5,   198,   342,
     199,    -1,     4,   342,    -1,     3,   342,    -1,   435,    -1,
     460,     9,   435,    -1,   398,   149,   215,    -1,   398,   149,
     124,    -1,    -1,    97,   479,    -1,   175,   465,    14,   479,
     200,    -1,   177,   465,   462,    14,   479,   200,    -1,   215,
      -1,   479,   215,    -1,   215,    -1,   215,   170,   471,   171,
      -1,   170,   468,   171,    -1,    -1,   479,    -1,   467,     9,
     479,    -1,   467,   408,    -1,   467,     9,   163,    -1,   468,
      -1,   163,    -1,    -1,    -1,    30,   479,    -1,   471,     9,
     472,   215,    -1,   472,   215,    -1,   471,     9,   472,   215,
      97,   479,    -1,   472,   215,    97,   479,    -1,    46,    -1,
      47,    -1,    -1,    86,   130,   479,    -1,   226,   149,   215,
     130,   479,    -1,   474,     9,   473,    -1,   473,    -1,   474,
     408,    -1,    -1,   174,   198,   475,   199,    -1,   226,    -1,
     215,   149,   478,    -1,   215,   466,    -1,    29,   479,    -1,
      55,   479,    -1,   226,    -1,   132,    -1,   133,    -1,   476,
      -1,   477,   149,   478,    -1,   132,   170,   479,   171,    -1,
     132,   170,   479,     9,   479,   171,    -1,   154,    -1,   198,
     105,   198,   469,   199,    30,   479,   199,    -1,   198,   479,
       9,   467,   408,   199,    -1,   479,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   734,   734,   734,   743,   745,   748,   749,   750,   751,
     752,   753,   754,   757,   759,   759,   761,   761,   763,   764,
     766,   768,   773,   774,   775,   776,   777,   778,   779,   780,
     781,   782,   783,   784,   785,   786,   787,   788,   789,   790,
     791,   795,   797,   801,   803,   807,   809,   813,   814,   815,
     816,   821,   822,   823,   824,   829,   830,   831,   832,   837,
     838,   842,   843,   845,   848,   854,   861,   868,   872,   878,
     880,   883,   884,   885,   886,   889,   890,   894,   899,   899,
     905,   905,   912,   911,   917,   917,   922,   923,   924,   925,
     926,   927,   928,   929,   930,   931,   932,   933,   934,   935,
     939,   937,   946,   944,   951,   959,   953,   963,   961,   965,
     966,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   980,   988,   988,   993,   999,  1003,  1003,  1011,  1012,
    1016,  1017,  1021,  1026,  1025,  1038,  1036,  1050,  1048,  1064,
    1063,  1072,  1070,  1082,  1081,  1100,  1098,  1117,  1116,  1125,
    1123,  1135,  1134,  1146,  1144,  1157,  1158,  1162,  1165,  1168,
    1169,  1170,  1173,  1174,  1177,  1179,  1182,  1183,  1186,  1187,
    1190,  1191,  1195,  1196,  1201,  1202,  1205,  1206,  1207,  1211,
    1212,  1216,  1217,  1221,  1222,  1226,  1227,  1232,  1233,  1238,
    1239,  1240,  1241,  1244,  1247,  1249,  1252,  1253,  1257,  1259,
    1262,  1265,  1268,  1269,  1272,  1273,  1277,  1283,  1289,  1296,
    1298,  1303,  1308,  1314,  1318,  1322,  1326,  1331,  1336,  1341,
    1346,  1352,  1361,  1366,  1371,  1377,  1379,  1383,  1387,  1392,
    1396,  1399,  1402,  1406,  1410,  1414,  1418,  1423,  1431,  1433,
    1436,  1437,  1438,  1439,  1441,  1443,  1448,  1449,  1452,  1453,
    1454,  1458,  1459,  1461,  1462,  1466,  1468,  1471,  1475,  1481,
    1483,  1486,  1486,  1490,  1489,  1493,  1495,  1498,  1501,  1499,
    1514,  1511,  1524,  1526,  1528,  1530,  1532,  1534,  1536,  1540,
    1541,  1542,  1545,  1551,  1554,  1560,  1563,  1568,  1570,  1575,
    1580,  1584,  1585,  1587,  1589,  1595,  1596,  1598,  1602,  1603,
    1608,  1609,  1613,  1614,  1618,  1620,  1626,  1631,  1632,  1634,
    1638,  1639,  1640,  1641,  1645,  1646,  1647,  1648,  1649,  1650,
    1652,  1657,  1660,  1661,  1665,  1666,  1670,  1671,  1674,  1675,
    1678,  1679,  1682,  1683,  1687,  1688,  1689,  1690,  1691,  1692,
    1693,  1697,  1698,  1701,  1702,  1703,  1706,  1708,  1710,  1711,
    1714,  1716,  1720,  1722,  1726,  1730,  1734,  1738,  1739,  1741,
    1742,  1743,  1746,  1750,  1751,  1755,  1756,  1760,  1761,  1762,
    1766,  1770,  1775,  1779,  1783,  1788,  1789,  1790,  1791,  1792,
    1796,  1798,  1799,  1800,  1803,  1804,  1805,  1806,  1807,  1808,
    1809,  1810,  1811,  1812,  1813,  1814,  1815,  1816,  1817,  1818,
    1819,  1820,  1821,  1822,  1823,  1824,  1825,  1826,  1827,  1828,
    1829,  1830,  1831,  1832,  1833,  1834,  1835,  1836,  1837,  1838,
    1839,  1840,  1841,  1842,  1843,  1844,  1845,  1847,  1848,  1850,
    1852,  1853,  1854,  1855,  1856,  1857,  1858,  1859,  1860,  1861,
    1862,  1863,  1864,  1865,  1866,  1867,  1868,  1869,  1870,  1874,
    1878,  1883,  1882,  1897,  1895,  1912,  1912,  1928,  1927,  1945,
    1945,  1961,  1960,  1979,  1978,  1999,  2000,  2001,  2006,  2008,
    2012,  2016,  2022,  2026,  2032,  2034,  2038,  2040,  2044,  2048,
    2049,  2053,  2060,  2067,  2069,  2074,  2075,  2076,  2077,  2079,
    2083,  2087,  2091,  2095,  2097,  2099,  2101,  2106,  2107,  2112,
    2113,  2114,  2115,  2116,  2117,  2121,  2125,  2129,  2133,  2138,
    2143,  2147,  2148,  2152,  2153,  2157,  2158,  2162,  2163,  2167,
    2171,  2175,  2179,  2180,  2181,  2182,  2186,  2192,  2201,  2214,
    2215,  2218,  2221,  2224,  2225,  2228,  2232,  2235,  2238,  2245,
    2246,  2250,  2251,  2253,  2257,  2258,  2259,  2260,  2261,  2262,
    2263,  2264,  2265,  2266,  2267,  2268,  2269,  2270,  2271,  2272,
    2273,  2274,  2275,  2276,  2277,  2278,  2279,  2280,  2281,  2282,
    2283,  2284,  2285,  2286,  2287,  2288,  2289,  2290,  2291,  2292,
    2293,  2294,  2295,  2296,  2297,  2298,  2299,  2300,  2301,  2302,
    2303,  2304,  2305,  2306,  2307,  2308,  2309,  2310,  2311,  2312,
    2313,  2314,  2315,  2316,  2317,  2318,  2319,  2320,  2321,  2322,
    2323,  2324,  2325,  2326,  2327,  2328,  2329,  2330,  2331,  2332,
    2333,  2334,  2335,  2336,  2340,  2345,  2346,  2349,  2350,  2351,
    2355,  2356,  2357,  2361,  2362,  2363,  2367,  2368,  2369,  2372,
    2374,  2378,  2379,  2380,  2381,  2383,  2384,  2385,  2386,  2387,
    2388,  2389,  2390,  2391,  2392,  2395,  2400,  2401,  2402,  2404,
    2405,  2407,  2408,  2409,  2410,  2412,  2414,  2416,  2418,  2420,
    2421,  2422,  2423,  2424,  2425,  2426,  2427,  2428,  2429,  2430,
    2431,  2432,  2433,  2434,  2435,  2436,  2438,  2440,  2442,  2444,
    2445,  2448,  2449,  2453,  2455,  2459,  2462,  2465,  2471,  2472,
    2473,  2474,  2475,  2476,  2477,  2482,  2484,  2488,  2489,  2492,
    2493,  2497,  2500,  2502,  2504,  2508,  2509,  2510,  2511,  2514,
    2518,  2519,  2520,  2521,  2525,  2527,  2534,  2535,  2536,  2537,
    2538,  2539,  2541,  2542,  2547,  2549,  2552,  2555,  2557,  2559,
    2562,  2564,  2568,  2570,  2573,  2576,  2582,  2584,  2587,  2588,
    2593,  2596,  2600,  2600,  2605,  2608,  2609,  2613,  2614,  2618,
    2619,  2620,  2624,  2625,  2629,  2630,  2634,  2635,  2639,  2640,
    2644,  2645,  2650,  2652,  2657,  2658,  2659,  2660,  2661,  2662,
    2664,  2667,  2670,  2672,  2674,  2678,  2679,  2680,  2681,  2682,
    2685,  2689,  2691,  2695,  2696,  2697,  2701,  2705,  2706,  2710,
    2713,  2720,  2724,  2728,  2735,  2736,  2741,  2743,  2744,  2747,
    2748,  2751,  2752,  2756,  2757,  2761,  2762,  2763,  2766,  2769,
    2772,  2775,  2776,  2777,  2780,  2784,  2788,  2789,  2790,  2792,
    2793,  2794,  2798,  2800,  2803,  2805,  2806,  2807,  2808,  2811,
    2813,  2814,  2818,  2820,  2823,  2825,  2826,  2827,  2831,  2833,
    2836,  2839,  2841,  2843,  2847,  2848,  2850,  2851,  2857,  2858,
    2860,  2862,  2864,  2866,  2869,  2870,  2871,  2875,  2876,  2877,
    2878,  2879,  2880,  2881,  2882,  2883,  2884,  2885,  2889,  2890,
    2894,  2896,  2904,  2906,  2910,  2914,  2921,  2922,  2928,  2929,
    2936,  2939,  2943,  2946,  2951,  2956,  2958,  2959,  2960,  2964,
    2965,  2969,  2971,  2972,  2975,  2980,  2981,  2982,  2986,  2989,
    2998,  3000,  3004,  3007,  3010,  3015,  3018,  3021,  3028,  3031,
    3034,  3035,  3038,  3041,  3042,  3047,  3050,  3054,  3058,  3064,
    3074,  3075
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
  "T_MINUS_EQUAL", "T_PLUS_EQUAL", "T_YIELD", "T_AWAIT", "'?'", "':'",
  "T_BOOLEAN_OR", "T_BOOLEAN_AND", "'|'", "'^'", "'&'",
  "T_IS_NOT_IDENTICAL", "T_IS_IDENTICAL", "T_IS_NOT_EQUAL", "T_IS_EQUAL",
  "'<'", "'>'", "T_IS_GREATER_OR_EQUAL", "T_IS_SMALLER_OR_EQUAL", "T_SR",
  "T_SL", "'+'", "'-'", "'.'", "'*'", "'/'", "'%'", "'!'", "T_INSTANCEOF",
  "'~'", "'@'", "T_UNSET_CAST", "T_BOOL_CAST", "T_OBJECT_CAST",
  "T_ARRAY_CAST", "T_STRING_CAST", "T_DOUBLE_CAST", "T_INT_CAST", "T_DEC",
  "T_INC", "T_POW", "'['", "T_CLONE", "T_NEW", "T_EXIT", "T_IF",
  "T_ELSEIF", "T_ELSE", "T_ENDIF", "T_LNUMBER", "T_DNUMBER", "T_ONUMBER",
  "T_STRING", "T_STRING_VARNAME", "T_VARIABLE", "T_NUM_STRING",
  "T_INLINE_HTML", "T_HASHBANG", "T_CHARACTER", "T_BAD_CHARACTER",
  "T_ENCAPSED_AND_WHITESPACE", "T_CONSTANT_ENCAPSED_STRING", "T_ECHO",
  "T_DO", "T_WHILE", "T_ENDWHILE", "T_FOR", "T_ENDFOR", "T_FOREACH",
  "T_ENDFOREACH", "T_DECLARE", "T_ENDDECLARE", "T_AS", "T_SWITCH",
  "T_ENDSWITCH", "T_CASE", "T_DEFAULT", "T_BREAK", "T_GOTO", "T_CONTINUE",
  "T_FUNCTION", "T_CONST", "T_RETURN", "T_TRY", "T_CATCH", "T_THROW",
  "T_USE", "T_GLOBAL", "T_PUBLIC", "T_PROTECTED", "T_PRIVATE", "T_FINAL",
  "T_ABSTRACT", "T_STATIC", "T_VAR", "T_UNSET", "T_ISSET", "T_EMPTY",
  "T_HALT_COMPILER", "T_CLASS", "T_INTERFACE", "T_EXTENDS", "T_IMPLEMENTS",
  "T_OBJECT_OPERATOR", "T_NULLSAFE_OBJECT_OPERATOR", "T_DOUBLE_ARROW",
  "T_LIST", "T_ARRAY", "T_CALLABLE", "T_CLASS_C", "T_METHOD_C", "T_FUNC_C",
  "T_LINE", "T_FILE", "T_COMMENT", "T_DOC_COMMENT", "T_OPEN_TAG",
  "T_OPEN_TAG_WITH_ECHO", "T_CLOSE_TAG", "T_WHITESPACE", "T_START_HEREDOC",
  "T_END_HEREDOC", "T_DOLLAR_OPEN_CURLY_BRACES", "T_CURLY_OPEN",
  "T_DOUBLE_COLON", "T_NAMESPACE", "T_NS_C", "T_DIR", "T_NS_SEPARATOR",
  "T_XHP_LABEL", "T_XHP_TEXT", "T_XHP_ATTRIBUTE", "T_XHP_CATEGORY",
  "T_XHP_CATEGORY_LABEL", "T_XHP_CHILDREN", "T_ENUM", "T_XHP_REQUIRED",
  "T_TRAIT", "\"...\"", "T_INSTEADOF", "T_TRAIT_C", "T_HH_ERROR",
  "T_FINALLY", "T_XHP_TAG_LT", "T_XHP_TAG_GT", "T_TYPELIST_LT",
  "T_TYPELIST_GT", "T_UNRESOLVED_LT", "T_COLLECTION", "T_SHAPE", "T_TYPE",
  "T_UNRESOLVED_TYPE", "T_NEWTYPE", "T_UNRESOLVED_NEWTYPE",
  "T_COMPILER_HALT_OFFSET", "T_ASYNC", "T_FROM", "T_WHERE", "T_JOIN",
  "T_IN", "T_ON", "T_EQUALS", "T_INTO", "T_LET", "T_ORDERBY",
  "T_ASCENDING", "T_DESCENDING", "T_SELECT", "T_GROUP", "T_BY",
  "T_LAMBDA_OP", "T_LAMBDA_CP", "T_UNRESOLVED_OP", "'('", "')'", "';'",
  "'{'", "'}'", "'$'", "'`'", "']'", "'\"'", "'\\''", "$accept", "start",
  "$@1", "top_statement_list", "top_statement", "$@2", "$@3", "ident",
  "use_declarations", "use_fn_declarations", "use_const_declarations",
  "use_declaration", "use_fn_declaration", "use_const_declaration",
  "namespace_name", "namespace_string_base", "namespace_string",
  "namespace_string_typeargs", "class_namespace_string_typeargs",
  "constant_declaration", "inner_statement_list", "inner_statement",
  "statement", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9", "$@10", "$@11",
  "try_statement_list", "$@12", "additional_catches",
  "finally_statement_list", "$@13", "optional_finally", "is_reference",
  "function_loc", "function_declaration_statement", "$@14", "$@15", "$@16",
  "enum_declaration_statement", "$@17", "$@18",
  "class_declaration_statement", "$@19", "$@20", "$@21", "$@22",
  "trait_declaration_statement", "$@23", "$@24", "class_decl_name",
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
  "enum_statement_list", "enum_statement", "enum_constant_declaration",
  "class_statement_list", "class_statement", "$@25", "$@26", "$@27",
  "$@28", "trait_rules", "trait_precedence_rule", "trait_alias_rule",
  "trait_alias_rule_method", "xhp_attribute_stmt", "xhp_attribute_decl",
  "xhp_attribute_decl_type", "xhp_attribute_enum", "xhp_attribute_default",
  "xhp_attribute_is_required", "xhp_category_stmt", "xhp_category_decl",
  "xhp_children_stmt", "xhp_children_paren_expr", "xhp_children_decl_expr",
  "xhp_children_decl_tag", "function_body", "method_body",
  "variable_modifiers", "method_modifiers", "non_empty_member_modifiers",
  "member_modifier", "parameter_modifiers", "parameter_modifier",
  "class_variable_declaration", "class_constant_declaration",
  "class_abstract_constant_declaration", "class_type_constant_declaration",
  "class_type_constant", "expr_with_parens", "parenthesis_expr",
  "expr_list", "for_expr", "yield_expr", "yield_assign_expr",
  "yield_list_assign_expr", "await_expr", "await_assign_expr",
  "await_list_assign_expr", "expr", "expr_no_variable", "lambda_use_vars",
  "closure_expression", "$@29", "$@30", "lambda_expression", "$@31",
  "$@32", "$@33", "$@34", "$@35", "lambda_body", "shape_keyname",
  "non_empty_shape_pair_list", "non_empty_static_shape_pair_list",
  "shape_pair_list", "static_shape_pair_list", "shape_literal",
  "array_literal", "collection_literal", "static_collection_literal",
  "dim_expr", "dim_expr_base", "query_expr", "query_assign_expr",
  "query_head", "query_body", "query_body_clauses", "query_body_clause",
  "from_clause", "let_clause", "where_clause", "join_clause",
  "join_into_clause", "orderby_clause", "orderings", "ordering",
  "ordering_direction", "select_or_group_clause", "select_clause",
  "group_clause", "query_continuation", "lexical_var_list", "xhp_tag",
  "xhp_tag_body", "xhp_opt_end_label", "xhp_attributes", "xhp_children",
  "xhp_attribute_name", "xhp_attribute_value", "xhp_child", "xhp_label_ws",
  "xhp_bareword", "simple_function_call", "fully_qualified_class_name",
  "static_class_name", "class_name_reference", "exit_expr",
  "backticks_expr", "ctor_arguments", "common_scalar", "static_expr",
  "static_class_constant", "scalar", "static_array_pair_list",
  "possible_comma", "hh_possible_comma",
  "non_empty_static_array_pair_list", "common_scalar_ae",
  "static_numeric_scalar_ae", "static_string_expr_ae", "static_scalar_ae",
  "static_array_pair_list_ae", "non_empty_static_array_pair_list_ae",
  "non_empty_static_scalar_list_ae", "static_shape_pair_list_ae",
  "non_empty_static_shape_pair_list_ae", "static_scalar_list_ae",
  "attribute_static_scalar_list", "non_empty_user_attribute_list",
  "user_attribute_list", "$@36", "non_empty_user_attributes",
  "optional_user_attributes", "object_operator",
  "object_property_name_no_variables", "object_property_name",
  "object_method_name_no_variables", "object_method_name", "array_access",
  "dimmable_variable_access", "dimmable_variable_no_calls_access",
  "variable", "dimmable_variable", "callable_variable",
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
  "hh_name_with_type", "hh_name_with_typevar", "hh_typeargs_opt",
  "hh_non_empty_type_list", "hh_type_list", "hh_func_type_list",
  "hh_opt_return_type", "hh_typevar_list", "hh_typevar_variance",
  "hh_shape_member_type", "hh_non_empty_shape_member_list",
  "hh_shape_member_list", "hh_shape_type", "hh_access_type_start",
  "hh_access_type", "hh_type", "hh_type_opt", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,    44,
     264,   265,   266,   267,    61,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,    63,
      58,   282,   283,   124,    94,    38,   284,   285,   286,   287,
      60,    62,   288,   289,   290,   291,    43,    45,    46,    42,
      47,    37,    33,   292,   126,    64,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,    91,   303,   304,   305,
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
     426,   427,   428,   429,   430,   431,   432,   433,    40,    41,
      59,   123,   125,    36,    96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   208,   210,   209,   211,   211,   212,   212,   212,   212,
     212,   212,   212,   212,   213,   212,   214,   212,   212,   212,
     212,   212,   215,   215,   215,   215,   215,   215,   215,   215,
     215,   215,   215,   215,   215,   215,   215,   215,   215,   215,
     215,   216,   216,   217,   217,   218,   218,   219,   219,   219,
     219,   220,   220,   220,   220,   221,   221,   221,   221,   222,
     222,   223,   223,   223,   224,   225,   226,   227,   227,   228,
     228,   229,   229,   229,   229,   230,   230,   230,   231,   230,
     232,   230,   233,   230,   234,   230,   230,   230,   230,   230,
     230,   230,   230,   230,   230,   230,   230,   230,   230,   230,
     235,   230,   236,   230,   230,   237,   230,   238,   230,   230,
     230,   230,   230,   230,   230,   230,   230,   230,   230,   230,
     230,   230,   240,   239,   241,   241,   243,   242,   244,   244,
     245,   245,   246,   248,   247,   249,   247,   250,   247,   252,
     251,   253,   251,   255,   254,   256,   254,   257,   254,   258,
     254,   260,   259,   261,   259,   262,   262,   263,   264,   265,
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
     308,   309,   309,   309,   309,   309,   309,   309,   310,   310,
     311,   311,   312,   312,   313,   313,   314,   315,   315,   315,
     316,   316,   316,   316,   317,   317,   317,   317,   317,   317,
     317,   318,   318,   318,   319,   319,   320,   320,   321,   321,
     322,   322,   323,   323,   324,   324,   324,   324,   324,   324,
     324,   325,   325,   326,   326,   326,   327,   327,   327,   327,
     328,   328,   329,   329,   330,   330,   331,   332,   332,   332,
     332,   332,   333,   334,   334,   335,   335,   336,   336,   336,
     337,   338,   339,   340,   341,   342,   342,   342,   342,   342,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   344,
     344,   346,   345,   347,   345,   349,   348,   350,   348,   351,
     348,   352,   348,   353,   348,   354,   354,   354,   355,   355,
     356,   356,   357,   357,   358,   358,   359,   359,   360,   361,
     361,   362,   363,   364,   364,   365,   365,   365,   365,   365,
     366,   367,   368,   369,   369,   369,   369,   370,   370,   371,
     371,   371,   371,   371,   371,   372,   373,   374,   375,   376,
     377,   378,   378,   379,   379,   380,   380,   381,   381,   382,
     383,   384,   385,   385,   385,   385,   386,   387,   387,   388,
     388,   389,   389,   390,   390,   391,   392,   392,   393,   393,
     393,   394,   394,   394,   395,   395,   395,   395,   395,   395,
     395,   395,   395,   395,   395,   395,   395,   395,   395,   395,
     395,   395,   395,   395,   395,   395,   395,   395,   395,   395,
     395,   395,   395,   395,   395,   395,   395,   395,   395,   395,
     395,   395,   395,   395,   395,   395,   395,   395,   395,   395,
     395,   395,   395,   395,   395,   395,   395,   395,   395,   395,
     395,   395,   395,   395,   395,   395,   395,   395,   395,   395,
     395,   395,   395,   395,   395,   395,   395,   395,   395,   395,
     395,   395,   395,   395,   396,   397,   397,   398,   398,   398,
     399,   399,   399,   400,   400,   400,   401,   401,   401,   402,
     402,   403,   403,   403,   403,   403,   403,   403,   403,   403,
     403,   403,   403,   403,   403,   403,   404,   404,   404,   404,
     404,   404,   404,   404,   404,   404,   404,   404,   404,   404,
     404,   404,   404,   404,   404,   404,   404,   404,   404,   404,
     404,   404,   404,   404,   404,   404,   404,   404,   404,   404,
     404,   404,   404,   404,   404,   405,   405,   405,   406,   406,
     406,   406,   406,   406,   406,   407,   407,   408,   408,   409,
     409,   410,   410,   410,   410,   411,   411,   411,   411,   411,
     412,   412,   412,   412,   413,   413,   414,   414,   414,   414,
     414,   414,   414,   414,   415,   415,   416,   416,   416,   416,
     417,   417,   418,   418,   419,   419,   420,   420,   421,   421,
     422,   422,   424,   423,   425,   426,   426,   427,   427,   428,
     428,   428,   429,   429,   430,   430,   431,   431,   432,   432,
     433,   433,   434,   434,   435,   435,   435,   435,   435,   435,
     435,   435,   435,   435,   435,   436,   436,   436,   436,   436,
     436,   436,   436,   437,   437,   437,   438,   439,   439,   440,
     440,   441,   441,   441,   442,   442,   443,   443,   443,   444,
     444,   445,   445,   446,   446,   447,   447,   447,   447,   447,
     447,   448,   448,   448,   448,   448,   449,   449,   449,   449,
     449,   449,   450,   450,   451,   451,   451,   451,   451,   451,
     451,   451,   452,   452,   453,   453,   453,   453,   454,   454,
     455,   455,   455,   455,   456,   456,   456,   456,   457,   457,
     457,   457,   457,   457,   458,   458,   458,   459,   459,   459,
     459,   459,   459,   459,   459,   459,   459,   459,   460,   460,
     461,   461,   462,   462,   463,   463,   464,   464,   465,   465,
     466,   466,   467,   467,   468,   469,   469,   469,   469,   470,
     470,   471,   471,   471,   471,   472,   472,   472,   473,   473,
     474,   474,   475,   475,   476,   477,   478,   478,   479,   479,
     479,   479,   479,   479,   479,   479,   479,   479,   479,   479,
     480,   480
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     1,     4,     3,     0,     6,     0,     5,     3,     4,
       4,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     1,     3,     1,     3,     1,     1,     2,     3,
       4,     1,     2,     3,     4,     1,     2,     3,     4,     1,
       3,     1,     3,     2,     1,     2,     2,     5,     4,     2,
       0,     1,     1,     1,     1,     3,     5,     8,     0,     4,
       0,     6,     0,    10,     0,     4,     2,     3,     2,     3,
       2,     3,     3,     3,     3,     3,     5,     1,     1,     1,
       0,     9,     0,    10,     5,     0,    13,     0,     5,     3,
       3,     2,     2,     2,     2,     2,     2,     3,     2,     2,
       3,     2,     0,     4,     9,     0,     0,     4,     2,     0,
       1,     0,     1,     0,     9,     0,    10,     0,    11,     0,
       9,     0,    10,     0,     8,     0,     9,     0,     7,     0,
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
       0,     0,     4,     0,     5,     2,     2,     2,     0,    10,
       0,    11,     3,     3,     3,     4,     4,     3,     5,     2,
       2,     0,     6,     5,     4,     3,     1,     1,     3,     4,
       1,     1,     4,     6,     1,     1,     4,     1,     1,     3,
       2,     0,     2,     0,     1,     3,     1,     1,     1,     1,
       3,     4,     4,     4,     1,     1,     2,     2,     2,     3,
       3,     1,     1,     1,     1,     3,     1,     3,     1,     1,
       1,     0,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     1,     1,     1,     3,     5,     1,     3,
       5,     4,     3,     3,     3,     4,     3,     3,     3,     2,
       1,     1,     3,     3,     1,     1,     0,     1,     2,     4,
       3,     6,     2,     3,     6,     1,     1,     1,     1,     1,
       6,     3,     4,     6,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     5,     4,     1,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     1,     1,     1,     3,     2,     1,     5,
       0,     0,    11,     0,    12,     0,     3,     0,     4,     0,
       6,     0,     7,     0,     5,     2,     2,     4,     1,     1,
       5,     3,     5,     3,     2,     0,     2,     0,     4,     4,
       3,     4,     4,     4,     4,     1,     1,     1,     1,     3,
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
       1,     1,     1,     1,     3,     2,     1,     1,     4,     3,
       4,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     2,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     5,     4,     3,     3,     3,     1,     1,
       1,     1,     3,     3,     3,     2,     0,     1,     0,     1,
       0,     5,     3,     3,     1,     1,     1,     1,     3,     2,
       1,     1,     1,     1,     1,     3,     1,     1,     1,     2,
       2,     4,     3,     4,     2,     0,     5,     3,     3,     1,
       3,     1,     2,     0,     5,     3,     2,     0,     3,     0,
       4,     2,     0,     3,     3,     1,     0,     1,     1,     1,
       1,     3,     1,     1,     1,     3,     1,     1,     3,     3,
       2,     4,     2,     4,     1,     1,     1,     1,     1,     3,
       5,     3,     4,     4,     3,     1,     1,     1,     1,     3,
       5,     4,     3,     1,     1,     3,     3,     1,     1,     7,
       9,     7,     6,     8,     1,     2,     4,     4,     1,     1,
       4,     1,     0,     1,     2,     1,     1,     3,     5,     3,
       3,     0,     1,     3,     5,     3,     2,     3,     6,     0,
       1,     4,     2,     0,     5,     3,     3,     1,     6,     4,
       4,     2,     2,     0,     5,     3,     3,     1,     2,     0,
       5,     3,     3,     1,     2,     2,     1,     2,     1,     4,
       3,     3,     6,     3,     1,     1,     1,     4,     4,     4,
       4,     4,     4,     2,     2,     4,     2,     2,     1,     3,
       3,     3,     0,     2,     5,     6,     1,     2,     1,     4,
       3,     0,     1,     3,     2,     3,     1,     1,     0,     0,
       2,     4,     2,     6,     4,     1,     1,     0,     3,     5,
       3,     1,     2,     0,     4,     1,     3,     2,     2,     2,
       1,     1,     1,     1,     3,     4,     6,     1,     8,     6,
       1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   367,     0,   752,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   833,     0,
     821,   635,     0,   641,   642,   643,    22,   699,   809,    98,
      99,   644,     0,    80,     0,     0,     0,     0,     0,     0,
       0,     0,   132,     0,     0,     0,     0,     0,     0,   334,
     335,   336,   339,   338,   337,     0,     0,     0,     0,   159,
       0,     0,     0,   648,   650,   651,   645,   646,     0,     0,
     652,   647,     0,   626,    23,    24,    25,    27,    26,     0,
     649,     0,     0,     0,     0,   653,   340,    28,    29,    31,
      30,    32,    33,    34,    35,    36,    37,    38,    39,    40,
     459,     0,    97,    70,   813,   636,     0,     0,     4,    59,
      61,    64,   698,     0,   625,     0,     6,   131,     7,     9,
       8,    10,     0,     0,   332,   377,     0,     0,     0,     0,
       0,     0,     0,   375,   797,   798,   445,   444,   361,   448,
       0,     0,   360,   775,   627,     0,   701,   443,   331,   778,
     376,     0,     0,   379,   378,   776,   777,   774,   804,   808,
       0,   433,   700,    11,   339,   338,   337,     0,     0,    27,
      59,   131,     0,   877,   376,   876,     0,   874,   873,   447,
       0,   368,   372,     0,     0,   417,   418,   419,   420,   442,
     440,   439,   438,   437,   436,   435,   434,   809,   628,     0,
     891,   627,     0,   399,     0,   397,     0,   837,     0,   708,
     359,   631,     0,   891,   630,     0,   640,   816,   815,   632,
       0,     0,   634,   441,     0,     0,     0,     0,   364,     0,
      78,   366,     0,     0,    84,    86,     0,     0,    88,     0,
       0,     0,   921,   922,   927,     0,     0,    59,   920,     0,
     923,     0,     0,     0,    90,     0,     0,     0,     0,   122,
       0,     0,     0,     0,     0,     0,    42,    47,   248,     0,
       0,   247,     0,   163,     0,   160,   253,     0,     0,     0,
       0,     0,   888,   147,   157,   829,   833,   858,     0,   655,
       0,     0,     0,   856,     0,    16,     0,    63,   139,   151,
     158,   532,   475,     0,   882,   457,   461,   463,   756,   377,
       0,   375,   376,   378,     0,     0,   637,     0,   638,     0,
       0,     0,   121,     0,     0,    66,   239,     0,    21,   130,
       0,   156,   143,   155,   337,   340,   131,   333,   112,   113,
     114,   115,   116,   118,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   821,
       0,   111,   812,   812,   119,   843,     0,     0,     0,     0,
       0,     0,   330,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   398,   396,   757,   758,
       0,   812,     0,   770,   239,   239,   812,     0,   814,   805,
     829,     0,   131,     0,     0,    92,     0,   754,   749,   708,
       0,     0,     0,     0,     0,   841,     0,   480,   707,   832,
       0,     0,    66,     0,   239,   358,     0,   772,   633,     0,
      70,   199,     0,   456,     0,    95,     0,     0,   365,     0,
       0,     0,     0,     0,    87,   110,    89,   918,   919,     0,
     913,     0,     0,     0,     0,   887,     0,   117,    91,   120,
       0,     0,     0,     0,     0,     0,     0,   490,     0,   497,
     499,   500,   501,   502,   503,   504,   495,   517,   518,    70,
       0,   107,   109,     0,     0,    44,    51,     0,     0,    46,
      55,    48,     0,    18,     0,     0,   249,     0,    93,   162,
     161,     0,     0,    94,   878,     0,     0,   377,   375,   376,
     379,   378,     0,   907,   169,     0,   830,     0,     0,     0,
       0,   654,   857,   699,     0,     0,   855,   704,   854,    62,
       5,    13,    14,     0,   167,     0,     0,   468,     0,     0,
     708,     0,     0,   629,   469,     0,     0,     0,     0,   756,
      70,     0,   710,   755,   931,   357,   430,   784,   796,    75,
      69,    71,    72,    73,    74,   331,     0,   446,   702,   703,
      60,   708,     0,   892,     0,     0,     0,   710,   240,     0,
     451,   133,   165,     0,   402,   404,   403,     0,     0,   400,
     401,   405,   407,   406,   422,   421,   424,   423,   425,   427,
     428,   426,   416,   415,   409,   410,   408,   411,   412,   414,
     429,   413,   811,     0,     0,   847,     0,   708,   881,     0,
     880,   781,   804,   149,   141,   153,   145,   131,   367,     0,
     370,   373,   381,   491,   395,   394,   393,   392,   391,   390,
     389,   388,   387,   386,   385,   384,   760,     0,   759,   762,
     779,   766,   891,   763,     0,     0,     0,     0,     0,     0,
       0,     0,   875,   369,   747,   751,   707,   753,     0,     0,
     891,     0,   836,     0,   835,     0,   820,   819,     0,     0,
     759,   762,   817,   763,   362,   201,   203,    70,   466,   465,
     363,     0,    70,   183,    79,   366,     0,     0,     0,     0,
       0,   195,   195,    85,     0,     0,     0,   911,   708,     0,
     898,     0,     0,     0,     0,     0,   706,   644,     0,     0,
     626,     0,     0,    64,   657,   625,   662,     0,   656,    68,
     661,   891,   924,     0,     0,   507,     0,     0,   513,   510,
     511,   519,     0,   498,   493,     0,   496,     0,     0,     0,
      52,    19,     0,     0,    56,    20,     0,     0,     0,    41,
      49,     0,   246,   254,   251,     0,     0,   867,   872,   869,
     868,   871,   870,    12,   905,   906,     0,     0,     0,     0,
     829,   826,     0,   479,   866,   865,   864,     0,   860,     0,
     861,   863,     0,     5,     0,     0,     0,   526,   527,   535,
     534,     0,     0,   707,   474,   478,     0,     0,   883,     0,
     458,     0,     0,   899,   756,   225,   930,     0,     0,   771,
     810,   707,   894,   890,   241,   242,   624,   709,   238,     0,
     756,     0,     0,   167,   453,   135,   432,     0,   483,   484,
       0,   481,   707,   842,     0,     0,   239,   169,     0,   167,
     165,     0,   821,   382,     0,     0,   768,   769,   782,   783,
     806,   807,     0,     0,     0,   735,   715,   716,   717,   724,
       0,     0,     0,   728,   726,   727,   741,   708,     0,   749,
     840,   839,     0,     0,   773,   639,     0,   205,     0,     0,
      76,     0,     0,     0,     0,     0,     0,     0,   175,   176,
     187,     0,    70,   185,   104,   195,     0,   195,     0,     0,
     925,     0,     0,   707,   912,   914,   897,   708,   896,     0,
     708,   683,   684,   681,   682,   714,     0,   708,   706,     0,
       0,   477,     0,     0,   849,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   917,   492,     0,     0,     0,   515,   516,   514,
       0,     0,   494,     0,   123,     0,   126,   108,     0,    43,
      53,     0,    45,    57,    50,   250,     0,   879,    96,   907,
     889,   902,   168,   170,   260,     0,     0,   827,     0,   859,
       0,    17,     0,   882,   166,   260,     0,     0,   471,     0,
     880,   884,     0,   899,   464,     0,     0,   931,     0,   230,
     228,   762,   780,   891,   893,     0,     0,   243,    67,     0,
     756,   164,     0,   756,     0,   431,   846,   845,     0,   239,
       0,     0,     0,     0,   167,   137,   640,   761,   239,     0,
     720,   721,   722,   723,   729,   730,   739,     0,   708,     0,
     735,     0,   719,   743,   707,   746,   748,   750,     0,   834,
     762,   818,   761,     0,     0,     0,     0,   202,   467,    81,
       0,   366,   175,   177,   829,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   189,     0,   908,     0,   910,   707,
       0,     0,     0,   659,   707,   705,     0,   696,     0,   708,
       0,   663,   697,   695,   853,     0,   708,   666,   668,   667,
       0,     0,   664,   665,   669,   671,   670,   686,   685,   688,
     687,   689,   691,   692,   690,   679,   678,   673,   674,   672,
     675,   676,   677,   680,   916,   505,     0,   506,   512,   520,
     521,     0,    70,    54,    58,   252,     0,     0,     0,   331,
     831,   829,   371,   374,   380,     0,    15,     0,   331,   538,
       0,     0,   540,   533,   536,     0,   531,     0,   885,     0,
     900,   460,     0,   231,     0,     0,   226,     0,   245,   244,
     899,     0,   260,     0,   756,     0,   239,     0,   802,   260,
     882,   260,     0,     0,   383,     0,     0,   732,   707,   734,
     725,     0,   718,     0,     0,   708,   740,   838,     0,    70,
       0,   198,   184,     0,     0,     0,   174,   100,   188,     0,
       0,   191,     0,   196,   197,    70,   190,   926,     0,   895,
       0,   929,   713,   712,   658,     0,   707,   476,   660,     0,
     482,   707,   848,   694,     0,     0,     0,     0,   901,   904,
     171,     0,     0,     0,   338,   329,     0,     0,     0,   148,
     259,   261,     0,   328,     0,     0,     0,   882,   331,     0,
     862,   256,   152,   529,     0,     0,   470,   462,     0,   234,
     224,     0,   227,   233,   239,   450,   899,   331,   899,     0,
     844,     0,   801,   331,     0,   331,   260,   756,   799,   738,
     737,   731,     0,   733,   707,   742,    70,   204,    77,    82,
     102,   178,     0,   186,   192,    70,   194,   909,     0,     0,
     473,     0,   852,   851,   693,     0,    70,   127,     0,     0,
       0,     0,     0,     0,   172,     0,   882,   295,   291,   297,
     626,    27,     0,   287,     0,   294,   306,     0,   304,   309,
       0,   308,     0,   307,     0,   131,   263,     0,   265,     0,
     266,   267,     0,     0,   828,     0,   530,   528,   539,   537,
     235,     0,     0,   222,   232,     0,     0,     0,     0,   144,
     450,   899,   803,   150,   256,   154,   331,     0,     0,   745,
       0,   200,     0,     0,    70,   181,   101,   193,   928,   711,
       0,     0,     0,     0,   903,     0,     0,   356,     0,     0,
     277,   281,   353,   354,     0,     0,     0,   272,   590,   589,
     586,   588,   587,   607,   609,   608,   578,   549,   550,   568,
     584,   583,   545,   555,   556,   558,   557,   577,   561,   559,
     560,   562,   563,   564,   565,   566,   567,   569,   570,   571,
     572,   573,   574,   576,   575,   546,   547,   548,   551,   552,
     554,   592,   593,   602,   601,   600,   599,   598,   597,   585,
     604,   594,   595,   596,   579,   580,   581,   582,   605,   606,
     610,   612,   611,   613,   614,   591,   616,   615,   618,   620,
     619,   553,   623,   621,   622,   617,   603,   544,   301,   541,
       0,   273,   322,   323,   321,   314,     0,   315,   274,   348,
       0,     0,     0,     0,   352,     0,   131,   140,   255,     0,
       0,     0,   223,   237,   800,     0,    70,   324,    70,   134,
       0,     0,     0,   146,   899,   736,     0,    70,   179,    83,
     103,     0,   472,   850,   508,   125,   275,   276,   351,   173,
       0,     0,     0,   298,   288,     0,     0,     0,   303,   305,
       0,     0,   310,   317,   318,   316,     0,     0,   262,     0,
       0,     0,   355,     0,   257,     0,   236,     0,   524,   710,
       0,     0,    70,   136,   142,     0,   744,     0,     0,     0,
     105,   278,    59,     0,   279,   280,     0,     0,   292,     0,
     296,   300,   542,   543,     0,   289,   319,   320,   312,   313,
     311,   349,   346,   268,   264,   350,     0,   258,   525,   709,
       0,   452,   325,     0,   138,     0,   182,   509,     0,   129,
       0,   331,     0,   299,   302,     0,   756,   270,     0,   522,
     449,   454,   180,     0,     0,   106,   285,     0,   330,   293,
     347,     0,   710,   342,   756,   523,     0,   128,     0,     0,
     284,   899,   756,   209,   343,   344,   345,   931,   341,     0,
       0,     0,   283,     0,   342,     0,   899,     0,   282,   326,
      70,   269,   931,     0,   214,   212,     0,    70,     0,     0,
     215,     0,     0,   210,   271,     0,   327,     0,   218,   208,
       0,   211,   217,   124,   219,     0,     0,   206,   216,     0,
     207,   221,   220
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   118,   813,   550,   180,   275,   504,
     508,   276,   505,   509,   120,   121,   122,   123,   124,   125,
     324,   580,   581,   457,   239,  1422,   463,  1342,  1423,  1659,
     769,   270,   499,  1620,   997,  1172,  1675,   340,   181,   582,
     851,  1054,  1223,   129,   553,   868,   583,   602,   870,   534,
     867,   584,   554,   869,   342,   293,   309,   132,   853,   816,
     799,  1012,  1363,  1106,   918,  1569,  1426,   714,   924,   462,
     723,   926,  1255,   706,   907,   910,  1095,  1681,  1682,   571,
     572,   596,   597,   280,   281,   287,  1395,  1548,  1549,  1179,
    1290,  1384,  1542,  1666,  1684,  1580,  1624,  1625,  1626,  1372,
    1373,  1374,  1582,  1588,  1635,  1377,  1378,  1382,  1535,  1536,
    1537,  1559,  1711,  1291,  1292,   182,   134,  1697,  1698,  1540,
    1294,  1295,  1296,  1297,   135,   232,   458,   459,   136,   137,
     138,   139,   140,   141,   142,   143,  1407,   144,   850,  1053,
     145,   236,   568,   318,   569,   570,   453,   559,   560,  1129,
     561,  1130,   146,   147,   148,   746,   149,   150,   267,   151,
     268,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     759,   760,   989,   496,   497,   498,   766,  1609,   152,   555,
    1397,   556,  1026,   821,  1196,  1193,  1528,  1529,   153,   154,
     155,   226,   233,   327,   445,   156,   945,   750,   157,   946,
     842,   835,   947,   894,  1074,   895,  1076,  1077,  1078,   897,
    1234,  1235,   898,   685,   429,   193,   194,   585,   574,   410,
     669,   670,   671,   672,   839,   159,   227,   184,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   633,   170,   229,
     230,   537,   218,   219,   636,   637,  1135,  1136,   302,   303,
     807,   171,   525,   172,   567,   173,  1550,   294,   335,   591,
     592,   939,  1036,   796,   797,   727,   728,   729,   260,   261,
     752,   262,   837
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1432
static const yytype_int16 yypact[] =
{
   -1432,   151, -1432, -1432,  5657, 13447, 13447,   -36, 13447, 13447,
   13447, 11397, 13447, -1432, 13447, 13447, 13447, 13447, 13447, 13447,
   13447, 13447, 13447, 13447, 13447, 13447,  4188,  4188, 11602, 13447,
   15318,   -33,   -18, -1432, -1432, -1432, -1432, -1432,   183, -1432,
   -1432,   273, 13447, -1432,   -18,   148,   185,   205,   -18, 11807,
    4830, 12012, -1432, 14439, 10372,   180, 13447, 14318,   141, -1432,
   -1432, -1432,    22,   385,    93,   219,   232,   246,   248, -1432,
    4830,   279,   294, -1432, -1432, -1432, -1432, -1432,   466,  4289,
   -1432, -1432,  4830, -1432, -1432, -1432, -1432,  4830, -1432,  4830,
   -1432,   308,   305,  4830,  4830, -1432,   257, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432, 13447, -1432, -1432,   265,   374,   499,   499, -1432,   478,
     364,   483, -1432,   330, -1432,    65, -1432,   509, -1432, -1432,
   -1432, -1432, 14651,   762, -1432, -1432,   353,   367,   401,   411,
     415,   433,  5175, -1432, -1432, -1432, -1432,   569, -1432,   582,
     585,   455, -1432,    43,   458,   521, -1432, -1432,   880,   128,
    1460,    86,   463,   140, -1432,   126,   142,   480,    24, -1432,
     340, -1432,   621, -1432, -1432, -1432,   551,   505,   556, -1432,
   -1432,   509,   762, 16243,  1683, 16243, 13447, 16243, 16243, 10560,
     514, 15503, 10560,   668,  4830,   655,   655,    79,   655,   655,
     655,   655,   655,   655,   655,   655,   655, -1432, -1432, 14905,
     563, -1432,   581,   469,   552,   469,  4188, 15547,   554,   748,
   -1432,   551, 14957,   563,   612,   613,   565,   144, -1432,   469,
      86, 12217, -1432, -1432, 13447,  8937,   757,    71, 16243,  9962,
   -1432, 13447, 13447,  4830, -1432, -1432, 13841,   570, -1432, 13885,
   14439, 14439,   604, -1432, -1432,   580, 14176,    63,   630,   767,
   -1432,   631,  4830,   708, -1432,   588, 13929,   590,   766, -1432,
      -9, 13973, 15895, 15911,  4830,    88, -1432,    66, -1432, 15095,
      90, -1432,   670, -1432,   671, -1432,   782,    94,  4188,  4188,
   13447,   598,   628, -1432, -1432, 15181, 11602,    57,   409, -1432,
   13652,  4188,   490, -1432,  4830, -1432,   -13,   364, -1432, -1432,
   -1432, -1432, 14664,   791,   718, -1432, -1432, -1432,    38,   620,
   16243,   622,   603,   629,  4696, 13447,   348,   616,   510,   348,
     343,   461, -1432,  4830, 14439,   632, 10577, 14439, -1432, -1432,
    4351, -1432, -1432, -1432, -1432, -1432,   509, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, 13447, 13447, 13447, 12422, 13447, 13447,
   13447, 13447, 13447, 13447, 13447, 13447, 13447, 13447, 13447, 13447,
   13447, 13447, 13447, 13447, 13447, 13447, 13447, 13447, 13447, 15318,
   13447, -1432, 13447, 13447, -1432, 13447,  1448,  4830,  4830,  4830,
   14651,   724,   694, 10167, 13447, 13447, 13447, 13447, 13447, 13447,
   13447, 13447, 13447, 13447, 13447, 13447, -1432, -1432, -1432, -1432,
    2673, 13447, 13447, -1432, 10577, 10577, 13447, 13447,   265,   259,
   15181,   633,   509, 12627, 14017, -1432, 13447, -1432,   635,   825,
   14905,   637,    25,   629,  3835,   469, 12832, -1432, 13037, -1432,
     641,    32, -1432,   345, 10577, -1432,  2904, -1432, -1432, 14061,
   -1432, -1432, 10782, -1432, 13447, -1432,   746,  9142,   828,   644,
   16198,   831,    60,    45, -1432, -1432, -1432, -1432, -1432, 14439,
   15167,   651,   841, 14727,  4830, -1432,   669, -1432, -1432, -1432,
     775, 13447,   777,   780, 13447, 13447, 13447, -1432,   766, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432,   673, -1432, -1432, -1432,
     664, -1432, -1432,  4830,   663,   856,    70,  4830,   672,   859,
     304,   311, 15953, -1432,  4830, 13447,   469,   141, -1432, -1432,
   -1432, 14727,   792, -1432,   469,    80,   113,   682,   684,  1055,
      29,   688,   689,   560,   743,   690,   469,   115,   691, 14997,
    4830, -1432, -1432,   829,  2269,   314, -1432, -1432, -1432,   364,
   -1432, -1432, -1432,   861,   769,   729,   365, -1432,   265,   770,
     892,   712,   768,   259, -1432, 14439, 14439,   902,   757,    38,
   -1432,   722,   910, -1432, 14439,    21,   854,   139, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432,   790,  2351, -1432, -1432, -1432,
   -1432,   921,   760, -1432,  4188, 13447,   733,   925, 16243,   924,
   -1432, -1432,   809, 14459, 10970, 11997, 10560, 13447, 14916, 13020,
    2102, 15365,  4123,  3245,  3780,  3780,  3780,  3780,  1913,  1913,
    1913,  1913,   693,   693,   646,   646,   646,    79,    79,    79,
   -1432,   655, 16243,   734,   735, 15603,   739,   935, -1432, 13447,
     -28,   747,   259, -1432, -1432, -1432, -1432,   509, 13447, 15044,
   -1432, -1432, 10560, -1432, 10560, 10560, 10560, 10560, 10560, 10560,
   10560, 10560, 10560, 10560, 10560, 10560, -1432, 13447,    -2,   262,
   -1432, -1432,   563,   388,   751,  2491,   752,   758,   761,  2620,
     116,   765, -1432, 16243, 14856, -1432,  4830, -1432,   620,    21,
     563,  4188, 16243,  4188, 15647,    21,   264, -1432,   772, 13447,
   -1432,   266, -1432, -1432, -1432,  8732,   573, -1432, -1432, 16243,
   16243,   -18, -1432, -1432, -1432, 13447,   868, 14577, 14727,  4830,
    9347,   774,   778, -1432,    85,   837,   819, -1432,   960,   773,
   14223, 14439, 14727, 14727, 14727, 14727, 14727, -1432,   784,    30,
     834,   789, 14727,   482, -1432,   839, -1432,   788, -1432, 16329,
   -1432,   -11, -1432, 13447,   806, 16243,   808,   985, 11382,   991,
   -1432, 16243, 14105, -1432,   673,   923, -1432,  5862, 15833,   802,
     358, -1432, 15895,  4830,   359, -1432, 15911,  4830,  4830, -1432,
   -1432,  2759, -1432, 16329,   993,  4188,   810, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432,   111,  4830, 15833,   811,
   15181, 15267,   995, -1432, -1432, -1432, -1432,   812, -1432, 13447,
   -1432, -1432,  5247, -1432, 14439, 15833,   813, -1432, -1432, -1432,
   -1432,  1001, 13447, 14664, -1432, -1432, 15992,   816, -1432, 14439,
   -1432,   822,  6067,   989,    34, -1432, -1432,    69,  2673, -1432,
   -1432, 14439, -1432, -1432,   469, 16243, -1432, 10987, -1432, 14727,
      39,   823, 15833,   769, -1432, -1432,  4633, 13447, -1432, -1432,
   13447, -1432, 13447, -1432,  3120,   824, 10577,   743,   994,   769,
     809,  4830, 15318,   469,  3389,   830, -1432, -1432,   267, -1432,
   -1432, -1432,  1012,  3923,  3923, 14856, -1432, -1432, -1432,   979,
     832,    33,   833, -1432, -1432, -1432, -1432,  1020,   838,   635,
     469,   469, 13242,  2904, -1432, -1432,  3561,   605,   -18,  9962,
   -1432,  6272,   836,  6477,   843, 14577,  4188,   835,   908,   469,
   16329,  1030, -1432, -1432, -1432, -1432,   465, -1432,   334, 14439,
   -1432, 14439,  4830, 15167, -1432, -1432, -1432,  1036, -1432,   847,
     921,   106,   106,   982,   982, 15747,   850,  1039, 14727,   915,
    4830, 14664,  3436, 16031, 14727, 14727, 14727, 14727, 14531, 14727,
   14727, 14727, 14727, 14727, 14727, 14727, 14727, 14727, 14727, 14727,
   14727, 14727, 14727, 14727, 14727, 14727, 14727, 14727, 14727, 14727,
   14727,  4830, -1432, 16243, 13447, 13447, 13447, -1432, -1432, -1432,
   13447, 13447, -1432,   766, -1432,   983, -1432, -1432,  4830, -1432,
   -1432,  4830, -1432, -1432, -1432, -1432, 14727,   469, -1432,   560,
   -1432,   968,  1057, -1432, -1432,   117,   869,   469, 11192, -1432,
    2208, -1432,  5452,   718,  1057, -1432,   317,   277, 16243,   953,
   -1432, -1432,   884,   989, -1432, 14439,   757, 14439,    19,  1071,
    1007,   268, -1432,   563, -1432,  4188, 13447, 16243, 16329,   888,
      39, -1432,   889,    39,   891,  4633, 16243, 15703,   893, 10577,
     894,   897, 14439,   898,   769, -1432,   565,   390, 10577, 13447,
   -1432, -1432, -1432, -1432, -1432, -1432,   962,   890,  1085,  1015,
   14856,   956, -1432, 14664, 14856, -1432, -1432, -1432,  4188, 16243,
     282, -1432, -1432,   -18,  1073,  1031,  9962, -1432, -1432, -1432,
     905, 13447,   908,   469, 15181, 14577,   909, 14727,  6682,   492,
     911, 13447,    48,   336, -1432,   939, -1432,   984, -1432, 14298,
    1082,   914, 14727, -1432, 14727, -1432,   918, -1432,   996,  1112,
     926, -1432, -1432, -1432, 15803,   922,  1119, 11585, 12202, 12610,
   14727, 16287, 13225, 16362, 16394,  3073,  4923,  5131,  5131,  5131,
    5131,  2299,  2299,  2299,  2299,   878,   878,   106,   106,   106,
     982,   982,   982,   982, -1432, 16243, 13637, 16243, -1432, 16243,
   -1432,   931, -1432, -1432, -1432, 16329,  4830, 14439, 15833,    84,
   -1432, 15181, -1432, -1432, 10560,   929, -1432,   937,    98, -1432,
      81, 13447, -1432, -1432, -1432, 13447, -1432, 13447, -1432,   757,
   -1432, -1432,   172,  1118,  1060, 13447, -1432,   938,   469, 16243,
     989,   944, -1432,   946,    39, 13447, 10577,   947, -1432, -1432,
     718, -1432,   949,   950, -1432,   952, 14856, -1432, 14856, -1432,
   -1432,   954, -1432,  1022,   955,  1148, -1432,   469,  1128, -1432,
     959, -1432, -1432,   961,   963,   118, -1432, -1432, 16329,   964,
     965, -1432,  4189, -1432, -1432, -1432, -1432, -1432, 14439, -1432,
   14439, -1432, 16329, 15847, -1432, 14727, 14664, -1432, -1432, 14727,
   -1432, 14727, -1432, 12815, 14727, 13447,   969,  6887,  1066, -1432,
   -1432,   579, 14365, 15833,  1065, -1432, 15852,  1008,  4752, -1432,
   -1432, -1432,   724, 14130,    99,   101,   973,   718,   694,   119,
   -1432, -1432, -1432,  1021,  3674,  4092, 16243, -1432,    26,  1160,
    1097, 13447, -1432, 16243, 10577,  1067,   989,  1197,   989,   978,
   16243,   980, -1432,  1216,   981,  1722, -1432,    39, -1432, -1432,
    1050, -1432, 14856, -1432, 14664, -1432, -1432,  8732, -1432, -1432,
   -1432, -1432,  9552, -1432, -1432, -1432,  8732, -1432,   986, 14727,
   16329,  1051, 16329, 15903, 12815, 13432, -1432, -1432, 14439, 15833,
   15833,  4830,  1173,    92, -1432, 14365,   718, -1432,  1018, -1432,
     102,   990,   103, -1432,  4972, -1432, -1432,   104, -1432, -1432,
    4425, -1432,   998, -1432,  1114,   509, -1432, 14439, -1432, 14439,
   -1432, -1432,  1180,   724, -1432,  2857, -1432, -1432, -1432, -1432,
    1181,  1117, 13447, -1432, 16243,  1000,  1004,  1003,   518, -1432,
    1067,   989, -1432, -1432, -1432, -1432,  2015,  1006, 14856, -1432,
    1076,  8732,  9757,  9552, -1432, -1432, -1432,  8732, -1432, 16329,
   14727, 14727, 13447,  7092, -1432,  1016,  1017, -1432, 14727, 15833,
   -1432, -1432, -1432, -1432, 14439,   648, 15852, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432,   512, -1432,
    1008, -1432, -1432, -1432, -1432, -1432,   112,   533, -1432,  1193,
     105,  4830,  1114,  1194, -1432, 14439,   509, -1432, -1432,  1019,
    1198, 13447, -1432, 16243, -1432,   403, -1432, -1432, -1432, -1432,
    1014,   518,  3786, -1432,   989, -1432, 14856, -1432, -1432, -1432,
   -1432,  7297, 16329, 16329, 11792, -1432, -1432, -1432, 16329, -1432,
   14007,   127,    50, -1432, -1432, 14727,  4972,  4972,  1163, -1432,
    4425,  4425,   636, -1432, -1432, -1432, 14727,  1142, -1432,  1024,
     107, 14727, -1432,  4830, -1432, 14727, 16243,  1144, -1432,  1218,
    7502,  7707, -1432, -1432, -1432,   518, -1432,  7912,  1025,  1150,
    1126, -1432,  1139,  1088, -1432, -1432,  1141, 14439, -1432,   648,
   -1432, 16329, -1432, -1432,  1078, -1432,  1207, -1432, -1432, -1432,
   -1432, 16329,  1227, -1432, -1432, 16329,  1045, 16329, -1432,   419,
    1046, -1432, -1432,  8117, -1432,  1044, -1432, -1432,  1048,  1080,
    4830,   694,  1079, -1432, -1432, 14727,    40, -1432,  1174, -1432,
   -1432, -1432, -1432, 15833,   802, -1432,  1091,  4830,   638, -1432,
   16329,  1053,  1247,   527,    40, -1432,  1178, -1432, 15833,  1059,
   -1432,   989,    51, -1432, -1432, -1432, -1432, 14439, -1432,  1061,
    1064,   108, -1432,   525,   527,   341,   989,  1063, -1432, -1432,
   -1432, -1432, 14439,    55,  1251,  1188,   525, -1432,  8322,   344,
    1255,  1192, 13447, -1432, -1432,  8527, -1432,    58,  1258,  1202,
   13447, -1432, 16243, -1432,  1259,  1203, 13447, -1432, 16243, 13447,
   -1432, 16243, 16243
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1432, -1432, -1432,  -489, -1432, -1432, -1432,   436, -1432, -1432,
   -1432,   771,   513,   508,   201,  1639,  3598, -1432,  2924, -1432,
    -423, -1432,    27, -1432, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432,  -386, -1432, -1432,  -153,   467,    31,
   -1432, -1432, -1432, -1432, -1432, -1432,    35, -1432, -1432, -1432,
   -1432,    36, -1432, -1432,   896,   903,   900,   -98,   421,  -783,
     425,   481,  -393,   195,  -782, -1432,  -121, -1432, -1432, -1432,
   -1432,  -651,    46, -1432, -1432, -1432, -1432,  -384, -1432,  -476,
   -1432,  -352, -1432, -1432,   787, -1432,  -109, -1432, -1432,  -944,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432,
    -129, -1432, -1432, -1432, -1432, -1432,  -212, -1432,    52,  -863,
   -1432, -1431,  -392, -1432,  -156,   256,  -125,  -381, -1432,  -217,
   -1432, -1432, -1432,    42,   -45,     4,  1286,  -650,  -338, -1432,
   -1432,    -7, -1432, -1432,    -5,   -29,   -74, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432,  -536,  -773, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432,   945, -1432,
   -1432,   349, -1432,   855, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432,   354, -1432,   858, -1432, -1432,   584, -1432,   324, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432,  -850, -1432,  2408,
    1250,  -325, -1432, -1432,   285,  2826,  3683, -1432, -1432,   404,
    -176,  -575, -1432, -1432,   474,   280,  -643,   281, -1432, -1432,
   -1432, -1432, -1432,   468, -1432, -1432, -1432,    87,  -800,  -162,
    -390,  -388, -1432,   524,  -123, -1432, -1432,   315, -1432, -1432,
    1766,   -43, -1432, -1432,   153,  -137, -1432,  -242, -1432, -1432,
   -1432,  -375,  1070, -1432, -1432, -1432, -1432, -1432,   557,   362,
   -1432, -1432,  1081,  -282,  -950, -1432,   -24,   -68,  -170,    37,
     649, -1432,  -914, -1432,   372,   438, -1432, -1432, -1432, -1432,
     393,   320,  -995
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -916
static const yytype_int16 yytable[] =
{
     183,   185,   391,   187,   188,   189,   191,   192,   347,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   310,   848,   217,   220,   313,   314,   705,   421,   259,
     564,   126,   830,   419,  1037,   128,   235,   238,   413,   130,
     131,   896,  1202,   439,   246,   680,   249,   265,   240,   266,
    1029,   271,   244,   442,   630,   650,   701,   347,   702,  1629,
     390,   812,   676,   677,   343,   914,   319,   446,   323,   719,
    1052,   928,  -886,  1187,   337,   721,   767,  -886,  1253,    13,
     454,  1188,   321,    13,    13,    13,  1063,   411,  1281,   785,
     416,   158,   698,   831,   929,  -487,    13,   512,  1203,   517,
     500,  1439,  1281,   522,  1038,  1400,   320,   447,  1387,  -785,
    1389,  -290,  1446,  1530,  1597,   949,  1597,  1439,  1081,  1199,
    1009,  1590,   785,   539,   801,   801,   801,   801,   801,    13,
    1613,  1303,   379,  1102,  1720,   540,  1627,  1734,   981,   282,
     333,   634,   334,    13,   380,  1591,   283,   832,  1039,   408,
     409,     3,   411,   408,   409,   977,   978,   979,   501,   334,
     408,   409,   186,   514,   431,   231,   433,   773,  -764,   674,
    -891,   980,   286,  -629,   678,   563,   299,   440,  1128,  1082,
     234,   424,  1204,   228,  1654,   408,   409,   551,   552,  1401,
    1282,  -455,  -786,   603,  -788,  1283,  -764,    59,    60,    61,
     174,  1284,   344,  1285,  1282,  -792,  -487,  1308,  -787,  1283,
    -822,    59,    60,    61,   174,  1284,   344,  1285,  1721,   333,
     278,  1735,   412,   333,   577,   417,   449,   415,   791,   449,
    -709,   696,  1040,  -709,  -229,   311,   238,   460,  -229,  -213,
    1286,  1287,  -628,  1288,  -785,   527,   722,   531,  1254,   642,
    -709,  1309,  1630,   687,  1286,  1287,   930,  1288,   277,   720,
     133,   528,   451,  -886,   345,   338,   456,   347,  1317,   681,
    1324,   455,   601,   642,  1109,  1323,  1113,  1325,   345,   786,
     306,  1222,  1010,   307,   911,   320,  1289,   412,   513,   913,
     518,   217,  1440,  1441,   523,   544,  1315,   642,  1628,  1388,
    1302,  1390,  -290,  1447,  1531,  1598,   642,  1644,  1708,   642,
    1233,  1592,   787,   599,   802,   882,  1180,  1341,  1394,   160,
     586,   310,   343,  1246,  1022,   416,  -794,  -786,  -789,  -788,
    -825,   598,  -823,  -791,  -790,  1310,   315,  -795,   415,  -488,
    -792,   213,   215,  -787,   279,  -822,   241,  1392,  -824,   604,
     605,   606,   608,   609,   610,   611,   612,   613,   614,   615,
     616,   617,   618,   619,   620,   621,   622,   623,   624,   625,
     626,   627,   628,   629,  1049,   631,  1713,   632,   632,  1727,
     635,   269,  1416,   242,   824,   688,   651,   433,   652,   654,
     655,   656,   657,   658,   659,   660,   661,   662,   663,   664,
     665,   777,  1408,   243,  1410,   573,   632,   675,   778,   598,
     598,   632,   679,   838,   392,   818,  1443,   288,   652,   207,
    1714,   683,   297,  1728,   207,  1015,   322,   297,   546,   391,
     289,   692,  1194,   694,  1111,  1112,  1111,  1112,  1607,   598,
     119,  1236,   408,   409,   290,   708,   291,   709,  1041,   710,
    1042,  1243,   316,   297,  1668,   998,  1001,   333,   317,   326,
     417,   863,   311,  -789,   333,  -825,   325,  -823,  -791,  -790,
     865,   127,  1189,   506,   510,   511,   755,   295,  1195,   758,
     761,   762,  1608,  -824,   713,  1190,   247,   390,   297,   257,
     300,   301,   296,  1351,   871,   300,   301,  1561,  1669,  1108,
    1201,   284,   875,   312,  1715,   549,   292,  1729,   332,   285,
     781,   333,   333,  1090,  1060,  1091,   811,   333,  1191,   819,
     865,   300,   301,   308,   432,   292,  1585,   838,   336,   292,
     292,   435,   228,   903,   820,   855,  1114,   441,  1256,   641,
     297,   564,  1586,   418,   339,   297,   546,  1066,   114,   588,
     160,   298,   934,   348,   160,   541,   300,   301,  -767,  1587,
    -765,  1420,  1593,   673,  1110,  1111,  1112,   349,   292,   297,
     467,   468,   904,   442,  1211,   546,   472,  1213,   297,  1594,
     133,   982,  1595,  1329,   329,  1330,  -767,   641,  -765,   297,
     845,  1250,  1111,  1112,   516,   546,   697,   408,   409,   703,
     346,   350,   856,   524,   524,   529,   794,   795,   300,   301,
     536,   351,   299,   300,   301,   352,   545,   423,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,   405,
     428,  -891,  -891,   353,   864,  -485,   547,   300,   301,   160,
    1694,  1695,  1696,   191,   908,   909,   300,   301,   382,   422,
    1615,   383,   334,   334,   593,   384,   573,   300,   301,   385,
     542,   414,   874,  1307,   548,  1638,   406,   407,   589,   564,
     386,   119,   328,   330,   331,   119,  1093,  1094,  -793,   461,
    1182,  -891,  1639,  -891,  -891,  1640,   563,  -486,   542,  1419,
     548,   542,   548,   548,   906,   376,   377,   378,   475,   379,
    -628,   642,  1705,   420,   770,  1359,  1360,  1217,   774,   304,
     238,   380,   427,   277,   425,   912,  1225,  1719,  1557,  1558,
     380,  1085,    33,    34,    35,  1709,  1710,  1636,  1637,  1245,
     434,   408,   409,   334,   737,   536,  1632,  1633,  1319,   373,
     374,   375,   376,   377,   378,   432,   379,   923,   983,  1277,
     415,    59,    60,    61,   174,   175,   344,   438,   380,   437,
     119,  -627,   443,   444,  1121,   452,   642,   937,   940,   590,
     465,  1125,   160,   257,   469,  1565,   292,  1703,   470,  -915,
     474,   473,    73,    74,    75,    76,    77,   476,   477,   724,
     479,   127,  1716,   739,   519,   520,   521,   532,   533,    80,
      81,   564,   577,  1065,  1020,   565,  1299,    59,    60,    61,
     174,   175,   344,    90,   563,   566,  1337,  1028,   345,   575,
     587,   576,   640,   292,   644,   292,   292,    95,   578,    52,
     -65,   600,  1346,   684,   686,   711,   689,   454,  1690,   126,
     695,   392,  1047,   128,   715,   718,   668,   130,   131,   730,
     731,  1417,  1055,   753,   754,  1056,   756,  1057,   647,   757,
     765,   598,   768,   771,  1321,   772,  1683,    52,   776,   798,
     690,   784,   775,  1207,   345,    59,    60,    61,   174,   175,
     344,   788,   700,   789,  1683,   827,   828,   792,   800,   793,
     803,   814,  1704,   119,   836,   809,   815,  1089,   817,   158,
     822,   823,  1229,    59,    60,    61,    62,    63,   344,   844,
     751,   825,  1096,  1421,    69,   387,   829,   826,   833,   834,
    -489,   573,  1427,  1616,   974,   975,   976,   977,   978,   979,
     841,   843,   846,  1433,   847,   852,  1097,   573,   849,   858,
     859,   861,   345,   980,   862,   866,   563,   480,   481,   482,
     780,   878,   389,  1267,   483,   484,   876,   879,   485,   486,
    1272,   133,  1405,   854,   873,   915,   880,   931,   932,   933,
     345,   905,   935,   506,   925,   806,   808,   510,   927,  1165,
    1166,  1167,   948,   950,   564,   758,  1169,   951,   953,   954,
     984,   673,   985,    59,    60,    61,    62,    63,   344,   986,
     990,  1571,   993,   996,    69,   387,   900,  1006,   901,  1018,
    1008,  1183,  1014,  1184,  1025,  1027,  1031,  1019,  1033,  1035,
     160,  1050,  1059,   133,  1062,   228,  1069,  1079,  1068,  1084,
    1080,  1083,   919,  1104,  1650,   160,  1099,  1086,  1105,   292,
     388,  1209,   389,  1101,  1107,  1119,  1120,   980,  1124,   126,
     593,   593,   564,   128,   598,  1123,   703,   130,   131,  1335,
     345,   541,  1171,   598,  1184,  1177,  1178,  1181,   133,   423,
     394,   395,   396,   397,   398,   399,   400,   401,   402,   403,
     404,   405,   160,  1197,  1198,  1205,  1206,  1210,   133,  1214,
    1212,  1216,  1226,  1218,  1228,  1227,   238,  1238,  1219,  1221,
    1007,   889,  1232,  1239,  1240,  1242,  1252,  1693,  1247,   158,
    1257,  1251,  1260,  1261,  1258,   536,  1017,  1264,   406,   407,
     893,  1266,   899,  1241,  1270,  1268,  1265,   160,  1271,   563,
    1276,  1300,  1311,  1610,  1023,  1611,  1314,   573,  1301,  1312,
     573,   119,  1393,  1316,  1617,  1318,  1322,   160,  1327,  1032,
    1326,  1328,  1332,  1331,  1333,   921,   119,  1334,  1336,  1338,
    1339,  1044,  1340,  1358,  1343,  1344,  1376,   133,   347,   133,
    1356,  1365,   127,  1391,  1402,  1396,  1403,  1411,  1406,  1412,
    1418,  1430,  1414,   408,   409,  1428,  1304,  1438,  1444,  1653,
    1305,  1445,  1306,  1539,  1545,  1551,  1552,   563,  1538,  1554,
    1313,  1281,  1555,   119,  1556,  1564,  1566,  1596,  1601,  1000,
    1320,   598,  1605,  1003,  1004,  1612,  1576,  1577,  1634,  1604,
    1281,  1642,  1643,  1648,   160,  1656,   160,  1649,   160,  1657,
     919,  1103,  1541,  1011,   127,  1658,  -286,  1660,  1661,  1664,
    1591,  1665,    13,  1667,  1672,  1670,  1673,  1674,   119,  1115,
    1679,  1116,  1691,  1685,   790,  1688,  1692,  1700,  1362,  1702,
    1706,    13,  1030,  1707,  1717,  1722,  1298,  1723,   119,  1730,
    1355,  1731,  1736,  1739,   668,  1298,   212,   212,   133,   127,
     225,  1737,  1740,   779,  1002,   999,   646,  1718,  1687,   645,
     643,  1064,  1061,  1437,  1725,  1701,  1024,  1244,  1345,   127,
    1699,   573,  1570,  1282,   782,  1562,  1404,   292,  1283,   598,
      59,    60,    61,   174,  1284,   344,  1285,  1584,  1589,  1073,
    1073,   893,  1282,  1712,  1724,  1600,  1366,  1283,   237,    59,
      60,    61,   174,  1284,   344,  1285,  1560,   160,   653,   700,
    1383,  1442,  1170,   763,  1168,   119,   764,   119,   992,   119,
    1192,  1224,  1126,  1286,  1287,  1200,  1288,   836,  1075,  1230,
    1208,  1231,  1043,  1543,   133,  1544,   538,  1087,  1117,  1425,
     526,  1118,  1286,  1287,  1164,  1288,     0,   345,   127,   938,
     127,  1176,  1220,     0,     0,     0,  1127,     0,     0,  1133,
       0,     0,     0,  1603,     0,     0,   345,  1553,     0,  1409,
       0,     0,     0,  1237,  1298,     0,     0,     0,     0,     0,
    1298,   160,  1298,     0,   573,     0,     0,   751,  1413,   536,
     919,     0,     0,   160,     0,     0,     0,  1574,     0,     0,
       0,     0,     0,     0,  1173,  1293,     0,  1174,     0,  1044,
       0,     0,     0,     0,  1293,     0,     0,     0,     0,  1568,
    1425,     0,     0,     0,     0,     0,     0,     0,   119,   212,
       0,     0,     0,     0,     0,     0,   212,     0,     0,     0,
       0,     0,   212,  1599,   393,   394,   395,   396,   397,   398,
     399,   400,   401,   402,   403,   404,   405,     0,     0,   127,
       0,     0,     0,     0,     0,     0,   536,  1279,     0,     0,
       0,     0,     0,  1298,     0,  1677,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   893,     0,     0,     0,
     893,     0,     0,   406,   407,    36,     0,   207,     0,   212,
       0,     0,   119,   133,     0,  1646,     0,     0,   212,   212,
       0,     0,     0,     0,   119,   212,  1606,     0,     0,     0,
       0,   212,     0,   347,   392,     0,     0,     0,     0,     0,
       0,     0,   562,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   638,  1293,     0,   127,     0,     0,  1347,  1293,
    1348,  1293,     0,     0,     0,     0,     0,     0,   408,   409,
       0,     0,   160,   133,     0,     0,     0,     0,     0,     0,
       0,     0,   133,     0,    84,    85,     0,    86,   179,    88,
       0,     0,  1278,  1386,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   225,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,     0,     0,     0,     0,   639,
       0,   114,   160,     0,     0,     0,     0,   160,     0,     0,
       0,   160,   893,     0,   893,   210,   210,     0,     0,   223,
     212,     0,  1293,     0,     0,     0,     0,   133,  1434,     0,
     212,     0,     0,   133,     0,     0,     0,     0,     0,   133,
       0,     0,   223,     0,     0,     0,     0,   423,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,   405,
       0,     0,     0,   119,     0,     0,     0,  1732,   257,     0,
       0,     0,     0,     0,  1381,  1738,  1281,     0,     0,     0,
       0,  1741,     0,     0,  1742,     0,   160,   160,   160,     0,
       0,     0,   160,     0,   127,     0,   406,   407,   160,     0,
       0,     0,     0,   573,     0,     0,     0,     0,     0,  1385,
       0,     0,     0,     0,  1581,     0,     0,    13,   893,     0,
       0,   573,     0,   119,     0,     0,     0,     0,   119,   573,
       0,     0,   119,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   214,   214,     0,     0,     0,   292,     0,     0,
       0,   257,     0,     0,   127,     0,     0,     0,     0,     0,
    1527,   408,   409,   127,     0,     0,  1534,     0,     0,     0,
       0,     0,     0,   257,     0,   257,     0,   133,  1282,     0,
       0,   257,     0,  1283,     0,    59,    60,    61,   174,  1284,
     344,  1285,     0,     0,   212,     0,     0,     0,   210,     0,
       0,     0,     0,     0,   893,   210,     0,   119,   119,   119,
    1546,   210,     0,   119,     0,  1602,   133,   133,     0,   119,
       0,     0,     0,   133,     0,     0,     0,     0,  1286,  1287,
       0,  1288,     0,     0,     0,     0,   160,     0,   127,   223,
     223,     0,     0,     0,   127,   223,     0,     0,     0,   212,
     127,     0,   345,     0,     0,     0,     0,     0,     0,   133,
       0,     0,     0,     0,     0,     0,     0,  1678,   210,     0,
       0,     0,     0,     0,  1415,   160,   160,   210,   210,     0,
       0,     0,   160,     0,   210,     0,     0,     0,     0,     0,
     210,   212,     0,   212,     0,     0,     0,  1662,     0,     0,
       0,   223,     0,  -916,  -916,  -916,  -916,   371,   372,   373,
     374,   375,   376,   377,   378,     0,   379,   212,   160,     0,
       0,     0,     0,   223,   133,   214,   223,   292,   380,     0,
       0,   133,   214,     0,     0,     0,     0,     0,   214,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   257,     0,
       0,     0,   893,     0,     0,     0,     0,   119,     0,     0,
       0,     0,     0,     0,     0,     0,  1622,   836,   223,  1281,
       0,     0,  1527,  1527,     0,     0,  1534,  1534,     0,     0,
       0,     0,   836,   160,     0,   212,     0,     0,   127,   292,
     160,     0,     0,     0,     0,   214,   119,   119,     0,     0,
     212,   212,     0,   119,   214,   214,   530,     0,     0,   210,
      13,   214,     0,     0,     0,     0,     0,   214,     0,   210,
       0,     0,     0,   562,     0,     0,     0,   127,   127,     0,
       0,     0,     0,     0,   127,     0,     0,     0,     0,   119,
       0,     0,     0,     0,     0,     0,  1676,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   223,   223,
       0,     0,   743,  1689,     0,     0,     0,     0,     0,     0,
     127,  1282,   225,     0,     0,     0,  1283,     0,    59,    60,
      61,   174,  1284,   344,  1285,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   119,   379,     0,     0,     0,     0,
     743,   119,     0,     0,     0,   212,   212,   380,     0,     0,
       0,  1286,  1287,     0,  1288,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   127,   214,     0,     0,     0,
       0,     0,   127,     0,     0,   345,   214,     0,     0,     0,
       0,   562,     0,     0,   223,   223,     0,     0,     0,     0,
       0,     0,     0,   223,     0,     0,     0,  1563,   354,   355,
     356,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   210,     0,     0,     0,   357,     0,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
       0,   379,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   380,     0,     0,     0,     0,     0,   354,
     355,   356,     0,     0,     0,     0,     0,     0,   210,     0,
       0,     0,     0,     0,     0,   212,     0,     0,   357,     0,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,     0,   379,     0,     0,     0,     0,     0,     0,     0,
     210,     0,   210,   562,   380,     0,     0,     0,   212,  -916,
    -916,  -916,  -916,   972,   973,   974,   975,   976,   977,   978,
     979,     0,     0,     0,   212,   212,   210,   743,     0,     0,
     214,   354,   355,   356,   980,     0,     0,     0,     0,   223,
     223,   743,   743,   743,   743,   743,     0,     0,     0,     0,
     357,   743,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,     0,   379,     0,     0,   223,     0,     0,
       0,     0,     0,  1185,     0,   214,   380,     0,     0,     0,
       0,     0,     0,     0,   210,     0,     0,     0,     0,     0,
       0,   212,     0,     0,   211,   211,     0,   223,   224,   210,
     210,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   223,   223,     0,     0,   214,     0,   214,
       0,     0,   223,     0,     0,     0,     0,     0,   223,     0,
       0,   810,     0,     0,     0,     0,     0,     0,     0,     0,
     223,     0,     0,   214,     0,     0,     0,     0,   743,     0,
       0,   223,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   354,   355,   356,     0,     0,     0,     0,     0,     0,
       0,   223,     0,     0,     0,     0,   562,     0,     0,     0,
     357,     0,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,     0,   379,     0,     0,     0,     0,     0,
       0,   214,     0,   840,   210,   210,   380,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   214,   214,   223,     0,
     223,     0,   223,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   562,     0,     0,   743,     0,     0,
     223,     0,     0,   743,   743,   743,   743,   743,   743,   743,
     743,   743,   743,   743,   743,   743,   743,   743,   743,   743,
     743,   743,   743,   743,   743,   743,   743,   743,   743,   743,
       0,     0,     0,     0,   211,     0,     0,     0,     0,     0,
     354,   355,   356,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   743,     0,     0,     0,   357,
       0,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,     0,   379,   223,     0,   223,     0,     0,     0,
       0,   214,   214,     0,   210,   380,     0,   211,     0,     0,
       0,     0,     0,   877,     0,     0,   211,   211,     0,     0,
       0,   223,     0,   211,     0,     0,     0,     0,     0,   211,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     211,     0,   223,     0,     0,     0,     0,   210,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   210,   210,     0,   743,     0,     0,     0,
      36,     0,   207,     0,     0,     0,     0,     0,   223,     0,
       0,   743,     0,   743,     0,     0,     0,     0,     0,   354,
     355,   356,     0,     0,     0,     0,     0,     0,     0,   743,
       0,     0,     0,     0,     0,     0,     0,   224,   357,     0,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   214,   379,     0,     0,     0,   223,   223,     0,     0,
     210,     0,   881,     0,   380,     0,     0,   666,   211,    84,
      85,     0,    86,   179,    88,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   214,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
     214,   214,     0,     0,   667,     0,   114,     0,     0,     0,
       0,   747,     0,     0,     0,     0,   250,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   223,     0,   223,
       0,     0,     0,     0,   743,   223,     0,     0,   743,     0,
     743,     0,   251,   743,     0,     0,     0,     0,     0,     0,
       0,   223,   223,     0,     0,   223,     0,     0,     0,   747,
       0,     0,   223,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   214,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1005,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   223,     0,     0,     0,   258,     0,     0,
       0,    36,     0,   207,     0,     0,     0,     0,   743,   252,
     253,     0,     0,     0,     0,     0,     0,   223,   223,   223,
       0,     0,   211,     0,   223,     0,     0,   178,     0,     0,
      82,   254,     0,    84,    85,     0,    86,   179,    88,     0,
       0,     0,     0,     0,     0,     0,   223,     0,   223,     0,
       0,   255,     0,     0,   223,     0,     0,     0,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,     0,   256,     0,   211,   666,  1547,
      84,    85,     0,    86,   179,    88,     0,     0,     0,   743,
     743,     0,     0,     0,     0,     0,     0,   743,   223,     0,
       0,     0,     0,   223,     0,   223,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   211,
       0,   211,     0,     0,     0,   699,     0,   114,   963,   964,
     965,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   978,   979,   211,   747,     0,     0,     0,
     354,   355,   356,     0,     0,     0,     0,     0,   980,     0,
     747,   747,   747,   747,   747,     0,     0,     0,     0,   357,
     747,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,     0,   379,   258,   258,   995,     0,     0,     0,
     258,     0,     0,     0,   223,   380,     0,     0,     0,     0,
       0,     0,     0,   211,     0,     0,     0,     0,     0,     0,
       0,   223,     0,     0,     0,     0,  1013,     0,   211,   211,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   223,
       0,     0,     0,  1013,   743,     0,     0,     0,     0,     0,
       0,   211,     0,     0,     0,   743,     0,     0,     0,     0,
     743,     0,     0,     0,   743,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   747,   258,     0,
    1051,   258,     0,     0,     0,     0,   223,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     224,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,     0,   379,   748,
       0,     0,     0,     0,   743,     0,     0,     0,     0,     0,
     380,     0,   223,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1058,   211,   211,     0,     0,   223,     0,     0,
       0,     0,     0,     0,     0,     0,   223,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   748,     0,     0,
       0,   223,     0,     0,     0,     0,   747,     0,     0,   211,
       0,     0,   747,   747,   747,   747,   747,   747,   747,   747,
     747,   747,   747,   747,   747,   747,   747,   747,   747,   747,
     747,   747,   747,   747,   747,   747,   747,   747,   747,     0,
       0,     0,     0,   258,   726,     0,     0,   745,     0,   354,
     355,   356,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   747,     0,     0,     0,   357,     0,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,     0,   379,     0,     0,   745,   955,   956,   957,     0,
       0,     0,     0,   211,   380,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   958,     0,   959,   960,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,     0,   258,
     258,   211,     0,     0,     0,     0,   211,     0,   258,     0,
       0,   980,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   211,   211,     0,   747,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     747,     0,   747,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   748,     0,     0,     0,   747,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   748,   748,
     748,   748,   748,     0,     0,     0,     0,     0,   748,     0,
       0,   354,   355,   356,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1280,     0,     0,   211,
     357,  1067,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,     0,   379,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   380,     0,     0,     0,
       0,     0,     0,     0,     0,  1131,     0,     0,     0,     0,
       0,     0,   745,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   258,   258,   745,   745,   745,   745,
     745,     0,     0,     0,     0,     0,   745,     0,     0,     0,
       0,     0,     0,   747,   211,   748,     0,   747,     0,   747,
       0,     0,   747,     0,   354,   355,   356,     0,     0,     0,
       0,  1364,     0,     0,  1375,     0,     0,     0,     0,     0,
       0,     0,     0,   357,     0,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,     0,   379,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   258,   380,
       0,     0,   211,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   258,     0,     0,     0,   747,     0,     0,
       0,     0,     0,  1092,     0,   258,     0,  1435,  1436,     0,
       0,     0,     0,   745,   748,     0,     0,     0,     0,     0,
     748,   748,   748,   748,   748,   748,   748,   748,   748,   748,
     748,   748,   748,   748,   748,   748,   748,   748,   748,   748,
     748,   748,   748,   748,   748,   748,   748,     0,     0,     0,
       0,     0,     0,     0,     0,   250,  -916,  -916,  -916,  -916,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   748,   379,     0,     0,     0,     0,   747,   747,
       0,   251,     0,     0,     0,   380,   747,  1579,     0,     0,
       0,     0,     0,   258,  1375,   258,     0,   726,     0,     0,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,     0,   745,     0,     0,     0,  1398,     0,   745,   745,
     745,   745,   745,   745,   745,   745,   745,   745,   745,   745,
     745,   745,   745,   745,   745,   745,   745,   745,   745,   745,
     745,   745,   745,   745,   745,     0,     0,     0,     0,     0,
       0,     0,    36,     0,   207,     0,     0,     0,   252,   253,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     745,     0,     0,   748,     0,     0,   178,     0,     0,    82,
     254,     0,    84,    85,     0,    86,   179,    88,   748,     0,
     748,     0,     0,     0,     0,     0,     0,     0,     0,   258,
     255,   258,     0,     0,     0,     0,   748,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,     0,   256,     0,   258,     0,  1614,     0,
       0,    84,    85,   747,    86,   179,    88,  1070,  1071,  1072,
      36,     0,     0,     0,   747,     0,     0,     0,     0,   747,
       0,     0,     0,   747,     0,     0,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,   745,     0,     0,     0,     0,   639,     0,   114,     0,
       0,     0,     0,   258,     0,     0,   745,     0,   745,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   745,     0,     0,     0,     0,     0,
       0,   744,     0,   747,     0,     0,     0,     0,     0,    84,
      85,  1686,    86,   179,    88,     0,     0,     0,     0,     0,
       0,   748,     0,     0,     0,   748,  1364,   748,     0,     0,
     748,   258,   354,   355,   356,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,   744,
       0,   357,     0,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,     0,   379,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   749,   380,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   748,   379,     0,     0,     0,
       0,     0,   258,     0,   258,     0,     0,     0,   380,   745,
       0,     0,     0,   745,     0,   745,     0,     0,   745,   354,
     355,   356,     0,     0,   783,     0,   258,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   258,   357,  1253,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,     0,   379,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   380,     0,   748,   748,     0,     0,
       0,     0,     0,     0,   748,    36,     0,   207,     0,     0,
       0,  1583,     0,   745,     0,     0,     0,     0,     0,     0,
       0,     0,   258,     0,     0,     0,     0,     0,     0,   258,
       0,     0,     0,     0,  1399,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   208,     0,     0,     0,
       0,   258,     0,   258,     0,     0,   744,     0,     0,   258,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     744,   744,   744,   744,   744,     0,     0,     0,   178,     0,
     744,    82,    83,     0,    84,    85,     0,    86,   179,    88,
       0,     0,     0,     0,   745,   745,     0,     0,     0,     0,
       0,     0,   745,     0,     0,     0,    36,     0,   258,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,     0,   209,     0,     0,  1254,
       0,   114,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   920,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   748,     0,     0,     0,   941,   942,   943,   944,     0,
       0,     0,   748,     0,     0,   952,     0,   748,    36,     0,
       0,   748,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   304,     0,     0,    84,    85,   744,    86,   179,
      88,     0,     0,     0,     0,  1663,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   258,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,   258,     0,     0,     0,
     305,   748,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,  1623,     0,     0,    84,    85,   745,
      86,   179,    88,     0,     0,     0,     0,     0,     0,     0,
     745,     0,     0,     0,     0,   745,     0,     0,     0,   745,
       0,     0,  1048,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   744,     0,     0,   600,
       0,   258,   744,   744,   744,   744,   744,   744,   744,   744,
     744,   744,   744,   744,   744,   744,   744,   744,   744,   744,
     744,   744,   744,   744,   744,   744,   744,   744,   744,  1532,
       0,    84,    85,  1533,    86,   179,    88,     0,     0,   745,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   744,     0,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,   258,     0,  1380,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   258,  1134,  1137,  1138,
    1139,  1141,  1142,  1143,  1144,  1145,  1146,  1147,  1148,  1149,
    1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,
    1160,  1161,  1162,  1163,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,     0,   379,     0,     0,  1175,
       0,     0,     0,     0,     0,     0,     0,     0,   380,     5,
       6,     7,     8,     9,     0,   744,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     744,     0,   744,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   744,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
    1248,    47,     0,     0,    48,     0,     0,     0,    49,    50,
      51,    52,     0,    54,    55,  1262,    56,  1263,    58,    59,
      60,    61,    62,    63,    64,     0,    65,    66,    67,     0,
      69,    70,     0,  1273,     0,     0,     0,    71,    72,    36,
      73,    74,    75,    76,    77,     0,     0,     0,     0,     0,
       0,    78,     0,     0,     0,     0,   178,    80,    81,    82,
      83,     0,    84,    85,     0,    86,   179,    88,    89,     0,
       0,    90,     0,   744,    91,     0,     0,   744,     0,   744,
      92,     0,   744,     0,  1379,    95,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,   111,     0,   112,   113,   579,   114,
     115,     0,   116,   117,     0,     0,     0,    36,    84,    85,
       0,    86,   179,    88,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   744,  1350,     0,
    1380,     0,  1352,     0,  1353,     0,     0,  1354,     0,   964,
     965,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   978,   979,  1448,  1449,  1450,  1451,  1452,
       0,     0,  1453,  1454,  1455,  1456,    84,    85,   980,    86,
     179,    88,     0,     0,     0,     0,     0,     0,     0,  1457,
    1458,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,  1459,     0,     0,   744,   744,
       0,     0,  1429,     0,     0,     0,   744,     0,     0,  1460,
    1461,  1462,  1463,  1464,  1465,  1466,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1467,
    1468,  1469,  1470,  1471,  1472,  1473,  1474,  1475,  1476,  1477,
    1478,  1479,  1480,  1481,  1482,  1483,  1484,  1485,  1486,  1487,
    1488,  1489,  1490,  1491,  1492,  1493,  1494,  1495,  1496,  1497,
    1498,  1499,  1500,  1501,  1502,  1503,  1504,  1505,  1506,  1507,
       0,     0,     0,  1508,  1509,     0,  1510,  1511,  1512,  1513,
    1514,     0,     0,  1572,  1573,     0,     0,     0,     0,     0,
       0,  1578,  1515,  1516,  1517,     0,     0,     0,    84,    85,
       0,    86,   179,    88,  1518,     0,  1519,  1520,     0,  1521,
       0,     0,     0,     0,     0,     0,  1522,  1523,     0,  1524,
       0,  1525,  1526,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,  -916,  -916,  -916,
    -916,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   744,     0,   354,   355,   356,     0,     0,
       0,     0,     0,     0,   744,     0,   980,     0,     0,   744,
       0,     0,     0,   744,   357,     0,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,     0,   379,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     380,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,   744,     0,     0,     0,     0,  1631,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,  1641,
       0,     0,     0,     0,  1645,     0,     0,     0,  1647,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,     0,     0,  1680,    49,
      50,    51,    52,    53,    54,    55,     0,    56,    57,    58,
      59,    60,    61,    62,    63,    64,     0,    65,    66,    67,
      68,    69,    70,     0,     0,   381,     0,     0,    71,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,    79,    80,    81,
      82,    83,     0,    84,    85,     0,    86,    87,    88,    89,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,    93,     0,    94,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,   112,   113,  1021,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,     0,     0,     0,    49,    50,    51,    52,    53,    54,
      55,     0,    56,    57,    58,    59,    60,    61,    62,    63,
      64,     0,    65,    66,    67,    68,    69,    70,     0,     0,
       0,     0,     0,    71,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,    79,    80,    81,    82,    83,     0,    84,    85,
       0,    86,    87,    88,    89,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,    93,     0,    94,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,  1186,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,     0,     0,     0,    49,
      50,    51,    52,    53,    54,    55,     0,    56,    57,    58,
      59,    60,    61,    62,    63,    64,     0,    65,    66,    67,
      68,    69,    70,     0,     0,     0,     0,     0,    71,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,    79,    80,    81,
      82,    83,     0,    84,    85,     0,    86,    87,    88,    89,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,    93,     0,    94,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,   112,   113,     0,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,     0,     0,     0,    49,    50,    51,    52,     0,    54,
      55,     0,    56,     0,    58,    59,    60,    61,    62,    63,
      64,     0,    65,    66,    67,     0,    69,    70,     0,     0,
       0,     0,     0,    71,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   178,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   179,    88,    89,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,     0,     0,     0,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,   994,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,     0,     0,     0,    49,
      50,    51,    52,     0,    54,    55,     0,    56,     0,    58,
      59,    60,    61,    62,    63,    64,     0,    65,    66,    67,
       0,    69,    70,     0,     0,     0,     0,     0,    71,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   178,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   179,    88,    89,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,     0,     0,     0,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,   112,   113,  1034,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,     0,     0,     0,    49,    50,    51,    52,     0,    54,
      55,     0,    56,     0,    58,    59,    60,    61,    62,    63,
      64,     0,    65,    66,    67,     0,    69,    70,     0,     0,
       0,     0,     0,    71,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   178,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   179,    88,    89,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,     0,     0,     0,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,  1098,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,  1100,    45,     0,
      46,     0,    47,     0,     0,    48,     0,     0,     0,    49,
      50,    51,    52,     0,    54,    55,     0,    56,     0,    58,
      59,    60,    61,    62,    63,    64,     0,    65,    66,    67,
       0,    69,    70,     0,     0,     0,     0,     0,    71,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   178,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   179,    88,    89,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,     0,     0,     0,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,   112,   113,     0,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,  1249,     0,
      48,     0,     0,     0,    49,    50,    51,    52,     0,    54,
      55,     0,    56,     0,    58,    59,    60,    61,    62,    63,
      64,     0,    65,    66,    67,     0,    69,    70,     0,     0,
       0,     0,     0,    71,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   178,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   179,    88,    89,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,     0,     0,     0,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,     0,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,     0,     0,     0,    49,
      50,    51,    52,     0,    54,    55,     0,    56,     0,    58,
      59,    60,    61,    62,    63,    64,     0,    65,    66,    67,
       0,    69,    70,     0,     0,     0,     0,     0,    71,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   178,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   179,    88,    89,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,     0,     0,     0,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,   112,   113,  1357,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,     0,     0,     0,    49,    50,    51,    52,     0,    54,
      55,     0,    56,     0,    58,    59,    60,    61,    62,    63,
      64,     0,    65,    66,    67,     0,    69,    70,     0,     0,
       0,     0,     0,    71,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   178,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   179,    88,    89,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,     0,     0,     0,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,  1575,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,  1618,    47,     0,     0,    48,     0,     0,     0,    49,
      50,    51,    52,     0,    54,    55,     0,    56,     0,    58,
      59,    60,    61,    62,    63,    64,     0,    65,    66,    67,
       0,    69,    70,     0,     0,     0,     0,     0,    71,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   178,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   179,    88,    89,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,     0,     0,     0,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,   112,   113,     0,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,     0,     0,     0,    49,    50,    51,    52,     0,    54,
      55,     0,    56,     0,    58,    59,    60,    61,    62,    63,
      64,     0,    65,    66,    67,     0,    69,    70,     0,     0,
       0,     0,     0,    71,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   178,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   179,    88,    89,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,     0,     0,     0,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,  1651,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,     0,     0,     0,    49,
      50,    51,    52,     0,    54,    55,     0,    56,     0,    58,
      59,    60,    61,    62,    63,    64,     0,    65,    66,    67,
       0,    69,    70,     0,     0,     0,     0,     0,    71,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   178,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   179,    88,    89,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,     0,     0,     0,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,   112,   113,  1652,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,  1655,    46,     0,    47,     0,     0,
      48,     0,     0,     0,    49,    50,    51,    52,     0,    54,
      55,     0,    56,     0,    58,    59,    60,    61,    62,    63,
      64,     0,    65,    66,    67,     0,    69,    70,     0,     0,
       0,     0,     0,    71,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   178,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   179,    88,    89,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,     0,     0,     0,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,     0,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,     0,     0,     0,    49,
      50,    51,    52,     0,    54,    55,     0,    56,     0,    58,
      59,    60,    61,    62,    63,    64,     0,    65,    66,    67,
       0,    69,    70,     0,     0,     0,     0,     0,    71,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   178,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   179,    88,    89,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,     0,     0,     0,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,   112,   113,  1671,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,     0,     0,     0,    49,    50,    51,    52,     0,    54,
      55,     0,    56,     0,    58,    59,    60,    61,    62,    63,
      64,     0,    65,    66,    67,     0,    69,    70,     0,     0,
       0,     0,     0,    71,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   178,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   179,    88,    89,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,     0,     0,     0,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,  1726,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,     0,     0,     0,    49,
      50,    51,    52,     0,    54,    55,     0,    56,     0,    58,
      59,    60,    61,    62,    63,    64,     0,    65,    66,    67,
       0,    69,    70,     0,     0,     0,     0,     0,    71,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   178,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   179,    88,    89,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,     0,     0,     0,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,   112,   113,  1733,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,     0,     0,     0,    49,    50,    51,    52,     0,    54,
      55,     0,    56,     0,    58,    59,    60,    61,    62,    63,
      64,     0,    65,    66,    67,     0,    69,    70,     0,     0,
       0,     0,     0,    71,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   178,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   179,    88,    89,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,     0,     0,     0,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,     0,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,   450,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,     0,     0,     0,    49,
      50,    51,    52,     0,    54,    55,     0,    56,     0,    58,
      59,    60,    61,   174,   175,    64,     0,    65,    66,    67,
       0,     0,     0,     0,     0,     0,     0,     0,    71,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   178,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   179,    88,     0,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,     0,     0,     0,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,   112,   113,     0,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,   712,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,     0,     0,     0,    49,    50,    51,    52,     0,    54,
      55,     0,    56,     0,    58,    59,    60,    61,   174,   175,
      64,     0,    65,    66,    67,     0,     0,     0,     0,     0,
       0,     0,     0,    71,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   178,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   179,    88,     0,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,     0,     0,     0,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,     0,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,   922,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,     0,     0,     0,    49,
      50,    51,    52,     0,    54,    55,     0,    56,     0,    58,
      59,    60,    61,   174,   175,    64,     0,    65,    66,    67,
       0,     0,     0,     0,     0,     0,     0,     0,    71,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   178,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   179,    88,     0,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,     0,     0,     0,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,   112,   113,     0,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,  1424,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,     0,     0,     0,    49,    50,    51,    52,     0,    54,
      55,     0,    56,     0,    58,    59,    60,    61,   174,   175,
      64,     0,    65,    66,    67,     0,     0,     0,     0,     0,
       0,     0,     0,    71,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   178,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   179,    88,     0,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,     0,     0,     0,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,     0,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,  1567,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,     0,    47,     0,     0,    48,     0,     0,     0,    49,
      50,    51,    52,     0,    54,    55,     0,    56,     0,    58,
      59,    60,    61,   174,   175,    64,     0,    65,    66,    67,
       0,     0,     0,     0,     0,     0,     0,     0,    71,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   178,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   179,    88,     0,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,     0,     0,     0,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,   112,   113,     0,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,     0,     0,     0,    49,    50,    51,    52,     0,    54,
      55,     0,    56,     0,    58,    59,    60,    61,   174,   175,
      64,     0,    65,    66,    67,     0,     0,     0,     0,     0,
       0,     0,     0,    71,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   178,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   179,    88,     0,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,     0,     0,     0,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,     0,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   648,    12,     0,     0,     0,     0,
       0,     0,   649,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    52,     0,     0,     0,     0,     0,     0,     0,
      59,    60,    61,   174,   175,   176,     0,     0,    66,    67,
       0,     0,     0,     0,     0,     0,     0,     0,   177,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   178,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   179,    88,     0,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,     0,     0,     0,     0,    95,    96,   263,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,     0,     0,     0,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    52,     0,     0,
       0,     0,     0,     0,     0,    59,    60,    61,   174,   175,
     176,     0,     0,    66,    67,     0,     0,     0,     0,     0,
       0,     0,     0,   177,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   178,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   179,    88,     0,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,     0,     0,     0,
       0,    95,    96,   263,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   264,     0,     0,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,   357,
      10,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   594,   379,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,   380,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    52,     0,     0,     0,     0,     0,     0,     0,
      59,    60,    61,   174,   175,   176,     0,     0,    66,    67,
       0,     0,     0,     0,     0,     0,     0,     0,   177,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   178,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   179,    88,     0,
     595,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,     0,     0,     0,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,     0,     0,     0,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    52,     0,     0,
       0,     0,     0,     0,     0,    59,    60,    61,   174,   175,
     176,     0,     0,    66,    67,     0,     0,     0,     0,     0,
       0,     0,     0,   177,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   178,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   179,    88,     0,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,     0,     0,     0,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,   355,   356,   707,     0,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,   357,
      10,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,  1045,   379,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,   380,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    52,     0,     0,     0,     0,     0,     0,     0,
      59,    60,    61,   174,   175,   176,     0,     0,    66,    67,
       0,     0,     0,     0,     0,     0,     0,     0,   177,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   178,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   179,    88,     0,
    1046,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,     0,     0,     0,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,     0,     0,     0,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   648,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    52,     0,     0,
       0,     0,     0,     0,     0,    59,    60,    61,   174,   175,
     176,     0,     0,    66,    67,     0,     0,     0,     0,     0,
       0,     0,     0,   177,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   178,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   179,    88,     0,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,     0,     0,     0,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   354,   355,   356,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   357,     0,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,     0,   379,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,   380,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   190,
       0,     0,    52,     0,     0,     0,     0,     0,     0,     0,
      59,    60,    61,   174,   175,   176,     0,     0,    66,    67,
       0,     0,     0,     0,     0,     0,     0,     0,   177,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   178,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   179,    88,     0,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,   987,   988,     0,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,   956,   957,     0,     0,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,   958,    10,   959,   960,   961,   962,
     963,   964,   965,   966,   967,   968,   969,   970,   971,   972,
     973,   974,   975,   976,   977,   978,   979,   216,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
     980,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    52,     0,     0,
       0,     0,     0,     0,     0,    59,    60,    61,   174,   175,
     176,     0,     0,    66,    67,     0,     0,     0,     0,     0,
       0,     0,     0,   177,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   178,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   179,    88,     0,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,     0,     0,     0,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   354,   355,   356,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   357,     0,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,     0,   379,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,   380,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    52,     0,     0,     0,     0,     0,     0,     0,
      59,    60,    61,   174,   175,   176,     0,     0,    66,    67,
       0,     0,     0,     0,     0,     0,     0,     0,   177,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   178,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   179,    88,     0,
       0,     0,    90,     0,     0,    91,     0,     0,     0,  1619,
       0,    92,     0,     0,     0,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,   245,     0,   356,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   357,     0,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,     0,
     379,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,   380,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    52,     0,     0,
       0,     0,     0,     0,     0,    59,    60,    61,   174,   175,
     176,     0,     0,    66,    67,     0,     0,     0,     0,     0,
       0,     0,     0,   177,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   178,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   179,    88,     0,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,     0,     0,     0,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   248,     0,   957,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   958,     0,   959,   960,   961,   962,   963,   964,   965,
     966,   967,   968,   969,   970,   971,   972,   973,   974,   975,
     976,   977,   978,   979,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,   980,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    52,     0,     0,     0,     0,     0,     0,     0,
      59,    60,    61,   174,   175,   176,     0,     0,    66,    67,
       0,     0,     0,     0,     0,     0,     0,     0,   177,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   178,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   179,    88,     0,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,     0,     0,     0,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,   448,     0,     0,     0,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   607,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    52,     0,     0,
       0,     0,     0,     0,     0,    59,    60,    61,   174,   175,
     176,     0,     0,    66,    67,     0,     0,     0,     0,     0,
       0,     0,     0,   177,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   178,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   179,    88,     0,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,     0,     0,     0,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,     0,     0,     0,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,   958,
      10,   959,   960,   961,   962,   963,   964,   965,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     978,   979,   649,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,   980,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    52,     0,     0,     0,     0,     0,     0,     0,
      59,    60,    61,   174,   175,   176,     0,     0,    66,    67,
       0,     0,     0,     0,     0,     0,     0,     0,   177,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   178,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   179,    88,     0,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,     0,     0,     0,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,     0,     0,     0,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   959,   960,   961,   962,
     963,   964,   965,   966,   967,   968,   969,   970,   971,   972,
     973,   974,   975,   976,   977,   978,   979,   691,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
     980,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    52,     0,     0,
       0,     0,     0,     0,     0,    59,    60,    61,   174,   175,
     176,     0,     0,    66,    67,     0,     0,     0,     0,     0,
       0,     0,     0,   177,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   178,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   179,    88,     0,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,     0,     0,     0,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,     0,     0,     0,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   693,   379,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,   380,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    52,     0,     0,     0,     0,     0,     0,     0,
      59,    60,    61,   174,   175,   176,     0,     0,    66,    67,
       0,     0,     0,     0,     0,     0,     0,     0,   177,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   178,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   179,    88,     0,
       0,     0,    90,     0,     0,    91,     0,     0,     0,     0,
       0,    92,     0,     0,     0,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,     0,     0,     0,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,   960,   961,   962,
     963,   964,   965,   966,   967,   968,   969,   970,   971,   972,
     973,   974,   975,   976,   977,   978,   979,  1088,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
     980,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    52,     0,     0,
       0,     0,     0,     0,     0,    59,    60,    61,   174,   175,
     176,     0,     0,    66,    67,     0,     0,     0,     0,     0,
       0,     0,     0,   177,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   178,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   179,    88,     0,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,     0,     0,     0,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   354,   355,   356,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   357,     0,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,     0,   379,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,   380,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,     0,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    52,     0,     0,     0,     0,     0,     0,     0,
      59,    60,    61,   174,   175,   176,     0,     0,    66,    67,
       0,     0,     0,     0,     0,     0,     0,     0,   177,    72,
       0,    73,    74,    75,    76,    77,     0,     0,     0,     0,
       0,     0,    78,     0,     0,     0,     0,   178,    80,    81,
      82,    83,     0,    84,    85,     0,    86,   179,    88,     0,
       0,     0,    90,     0,     0,    91,     0,     0,  1432,     0,
       0,    92,     0,     0,     0,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,   354,   355,   356,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   357,     0,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,     0,
     379,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,   380,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
     543,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    52,     0,     0,
       0,     0,     0,     0,     0,    59,    60,    61,   174,   175,
     176,     0,     0,    66,    67,     0,     0,     0,     0,     0,
       0,     0,     0,   177,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   178,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   179,    88,     0,     0,     0,    90,     0,     0,
      91,     0,  1275,     0,     0,     0,    92,     0,     0,     0,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,   354,   355,   356,     0,   114,   115,     0,   116,   117,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     357,     0,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,     0,   379,   354,   355,   356,     0,     0,
       0,     0,     0,     0,     0,     0,   380,     0,     0,     0,
       0,     0,     0,     0,   357,     0,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,     0,   379,   354,
     355,   356,     0,     0,     0,     0,     0,     0,     0,     0,
     380,     0,     0,     0,     0,     0,     0,     0,   357,     0,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,     0,   379,   354,   355,   356,     0,     0,     0,     0,
       0,     0,     0,     0,   380,     0,     0,     0,     0,     0,
       0,     0,   357,     0,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,     0,   379,   354,   355,   356,
       0,     0,     0,     0,     0,     0,     0,     0,   380,     0,
       0,   464,     0,     0,     0,     0,   357,     0,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,     0,
     379,   354,   355,   356,     0,     0,     0,     0,     0,     0,
       0,     0,   380,     0,    36,   466,     0,     0,     0,     0,
     357,     0,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,     0,   379,   354,   355,   356,     0,     0,
       0,     0,     0,     0,     0,     0,   380,     0,     0,   478,
       0,     0,     0,     0,   357,     0,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   178,   379,   250,
      82,     0,     0,    84,    85,     0,    86,   179,    88,     0,
     380,     0,     0,   502,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   251,     0,     0,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,     0,   250,     0,    36,     0,  1621,
       0,     0,     0,     0,     0,     0,   682,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   251,     0,     0,     0,  -330,     0,     0,     0,     0,
       0,     0,     0,    59,    60,    61,   174,   175,   344,     0,
       0,     0,   250,    36,     0,     0,     0,     0,     0,     0,
     704,     0,   252,   253,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   251,     0,
     178,   471,     0,    82,   254,     0,    84,    85,     0,    86,
     179,    88,     0,     0,     0,     0,     0,     0,     0,   991,
      36,     0,     0,     0,   255,     0,     0,     0,   252,   253,
     345,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,   178,   250,   256,    82,
     254,     0,    84,    85,     0,    86,   179,    88,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     255,     0,     0,   251,     0,   252,   253,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   178,   256,    36,    82,   254,     0,    84,
      85,     0,    86,   179,    88,     0,   936,     0,     0,     0,
       0,     0,     0,     0,   250,    36,     0,   255,     0,     0,
       0,     0,     0,     0,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
     251,   256,     0,   272,   273,     0,     0,     0,     0,     0,
     252,   253,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,     0,     0,     0,     0,   178,     0,
       0,    82,   254,     0,    84,    85,     0,    86,   179,    88,
       0,  1259,     0,     0,     0,     0,     0,     0,   250,     0,
       0,   274,   255,     0,    84,    85,     0,    86,   179,    88,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,   251,     0,   256,   252,   253,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,   178,    36,     0,    82,   254,
       0,    84,    85,     0,    86,   179,    88,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,     0,   255,
    1361,     0,     0,     0,     0,     0,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,  1140,     0,   256,     0,     0,     0,     0,     0,     0,
       0,   252,   253,     0,     0,     0,     0,   732,   733,     0,
       0,     0,     0,   734,     0,   735,     0,     0,     0,   178,
       0,     0,    82,   254,     0,    84,    85,   736,    86,   179,
      88,     0,     0,     0,     0,    33,    34,    35,    36,     0,
       0,     0,   916,   255,     0,    84,    85,   737,    86,   179,
      88,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,     0,   256,     0,     0,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,    36,     0,   207,   854,     0,     0,
       0,     0,     0,   738,     0,    73,    74,    75,    76,    77,
       0,     0,     0,     0,     0,     0,   739,     0,     0,     0,
       0,   178,    80,    81,    82,   740,     0,    84,    85,     0,
      86,   179,    88,     0,     0,   208,    90,     0,     0,     0,
       0,     0,     0,     0,     0,   741,     0,     0,   917,     0,
      95,     0,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,   178,    36,   742,
      82,    83,     0,    84,    85,     0,    86,   179,    88,     0,
       0,    36,     0,   207,     0,     0,     0,     0,     0,     0,
     557,     0,     0,     0,     0,     0,     0,     0,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,   732,   733,   209,     0,     0,     0,   734,
     114,   735,   208,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   736,     0,     0,     0,     0,     0,     0,
       0,    33,    34,    35,    36,   341,     0,    84,    85,     0,
      86,   179,    88,   737,   178,     0,     0,    82,    83,     0,
      84,    85,     0,    86,   179,    88,     0,     0,     0,     0,
       0,     0,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   738,
       0,    73,    74,    75,    76,    77,     0,   558,     0,     0,
       0,     0,   739,     0,     0,     0,     0,   178,    80,    81,
      82,   740,     0,    84,    85,     0,    86,   179,    88,     0,
       0,     0,    90,     0,     0,     0,     0,     0,     0,     0,
       0,   741,   883,   884,     0,     0,    95,     0,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   885,     0,     0,   742,   354,   355,   356,     0,
     886,   887,   888,    36,     0,     0,     0,     0,     0,     0,
       0,     0,   889,     0,     0,   357,   857,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,     0,   379,
       0,     0,    29,    30,     0,     0,     0,     0,     0,     0,
       0,   380,    36,     0,    38,     0,     0,     0,   890,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   891,     0,     0,     0,     0,     0,     0,     0,     0,
      52,     0,    84,    85,     0,    86,   179,    88,    59,    60,
      61,   174,   175,   176,    29,    30,     0,     0,     0,     0,
     892,     0,     0,     0,    36,     0,   207,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,     0,     0,   178,     0,     0,    82,    83,
       0,    84,    85,     0,    86,   179,    88,     0,     0,     0,
       0,     0,     0,    91,    36,   208,   804,   805,     0,     0,
       0,     0,     0,     0,     0,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,     0,   430,     0,     0,     0,   178,   114,     0,
      82,    83,   872,    84,    85,     0,    86,   179,    88,     0,
       0,    36,     0,   207,     0,    91,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,    84,    85,   430,    86,   179,    88,     0,
     114,     0,   208,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,   207,     0,     0,     0,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,   178,     0,     0,    82,    83,     0,
      84,    85,     0,    86,   179,    88,     0,     0,     0,     0,
       0,     0,     0,   208,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,     0,   209,     0,    36,   178,     0,   114,    82,    83,
       0,    84,    85,   725,    86,   179,    88,     0,    36,     0,
     207,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,     0,   209,     0,     0,   515,     0,   114,   208,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   535,     0,     0,     0,     0,   178,     0,     0,
      82,     0,     0,    84,    85,     0,    86,   179,    88,     0,
       0,   178,     0,     0,    82,    83,     0,    84,    85,     0,
      86,   179,    88,     0,    36,     0,   207,     0,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,     0,   209,
       0,     0,     0,     0,   114,   208,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    36,     0,   207,  1016,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   178,   379,     0,
      82,    83,     0,    84,    85,     0,    86,   179,    88,     0,
     380,     0,     0,     0,     0,     0,   221,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,     0,   209,     0,     0,   178,     0,
     114,    82,    83,     0,    84,    85,     0,    86,   179,    88,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   354,   355,   356,   222,     0,     0,     0,
       0,   114,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   357,     0,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,     0,   379,   354,   355,   356,
       0,     0,     0,     0,     0,     0,     0,     0,   380,     0,
       0,     0,     0,     0,     0,     0,   357,     0,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,     0,
     379,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   380,   354,   355,   356,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   357,   426,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,     0,   379,   354,   355,   356,
       0,     0,     0,     0,     0,     0,     0,     0,   380,     0,
       0,     0,     0,     0,     0,     0,   357,   436,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,     0,
     379,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   380,   354,   355,   356,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   357,   860,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,     0,   379,   955,   956,   957,
       0,     0,     0,     0,     0,     0,     0,     0,   380,     0,
       0,     0,     0,     0,     0,     0,   958,   902,   959,   960,
     961,   962,   963,   964,   965,   966,   967,   968,   969,   970,
     971,   972,   973,   974,   975,   976,   977,   978,   979,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   980,   955,   956,   957,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   958,  1215,   959,   960,   961,   962,   963,   964,
     965,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   978,   979,     0,     0,   955,   956,   957,
       0,     0,     0,     0,     0,     0,     0,     0,   980,     0,
       0,     0,     0,     0,     0,     0,   958,  1122,   959,   960,
     961,   962,   963,   964,   965,   966,   967,   968,   969,   970,
     971,   972,   973,   974,   975,   976,   977,   978,   979,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      36,     0,   980,   955,   956,   957,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    36,
       0,     0,   958,  1269,   959,   960,   961,   962,   963,   964,
     965,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   978,   979,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   980,     0,
       0,  1367,    36,     0,     0,     0,     0,  1349,     0,     0,
       0,     0,     0,   178,  1368,  1369,    82,    83,    36,    84,
      85,     0,    86,   179,    88,     0,     0,     0,     0,     0,
       0,     0,   178,     0,     0,    82,  1370,     0,    84,    85,
       0,    86,  1371,    88,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
      36,     0,     0,  1431,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,   503,     0,
       0,    84,    85,     0,    86,   179,    88,     0,     0,     0,
       0,     0,     0,     0,   507,     0,     0,    84,    85,    36,
      86,   179,    88,     0,     0,     0,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   274,     0,    36,    84,
      85,     0,    86,   179,    88,     0,   638,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,    84,    85,
       0,    86,   179,    88,     0,  1132,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,    84,    85,     0,
      86,   179,    88,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   354,   355,
     356,     0,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   716,   357,     0,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
       0,   379,     0,   354,   355,   356,     0,     0,     0,     0,
       0,     0,     0,   380,     0,     0,     0,     0,     0,     0,
       0,     0,   357,     0,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   717,   379,   955,   956,   957,
       0,     0,     0,     0,     0,     0,     0,     0,   380,     0,
       0,     0,     0,     0,     0,     0,   958,  1274,   959,   960,
     961,   962,   963,   964,   965,   966,   967,   968,   969,   970,
     971,   972,   973,   974,   975,   976,   977,   978,   979,   955,
     956,   957,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   980,     0,     0,     0,     0,     0,   958,     0,
     959,   960,   961,   962,   963,   964,   965,   966,   967,   968,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   980,   961,   962,   963,   964,   965,
     966,   967,   968,   969,   970,   971,   972,   973,   974,   975,
     976,   977,   978,   979,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   980,   962,   963,
     964,   965,   966,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,   978,   979,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   980
};

static const yytype_int16 yycheck[] =
{
       5,     6,   158,     8,     9,    10,    11,    12,   133,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    89,   597,    28,    29,    93,    94,   450,   181,    53,
     312,     4,   568,   170,   834,     4,    32,    42,   161,     4,
       4,   684,  1037,   219,    49,   420,    51,    54,    44,    54,
     823,    56,    48,   223,   379,   393,   446,   182,   446,     9,
     158,   550,   414,   415,   132,   715,   111,   229,   111,     9,
     853,   722,     9,  1023,     9,    30,   499,    14,    30,    45,
       9,  1025,   111,    45,    45,    45,   869,    66,     4,     9,
      66,     4,   444,   569,     9,    66,    45,     9,    79,     9,
     109,     9,     4,     9,    35,    79,   111,   230,     9,    66,
       9,     9,     9,     9,     9,    85,     9,     9,    85,  1033,
       9,     9,     9,    66,     9,     9,     9,     9,     9,    45,
    1561,    50,    53,   915,    79,   297,     9,    79,   149,   117,
     153,   383,   170,    45,    65,    33,   124,   570,    79,   128,
     129,     0,    66,   128,   129,    49,    50,    51,   167,   170,
     128,   129,   198,    97,   209,   198,   209,    97,   170,   411,
     198,    65,    79,   149,   416,   312,   146,   222,   951,   146,
     198,   186,   163,    30,  1615,   128,   129,   200,   201,   163,
     106,     8,    66,   346,    66,   111,   198,   113,   114,   115,
     116,   117,   118,   119,   106,    66,    66,    35,    66,   111,
      66,   113,   114,   115,   116,   117,   118,   119,   163,   153,
      79,   163,   201,   153,   199,   201,   231,   198,   199,   234,
     196,   199,   163,   199,   196,   154,   241,   242,   199,   199,
     156,   157,   149,   159,   201,   290,   201,   290,   200,   386,
     199,    79,   202,   429,   156,   157,   171,   159,    57,   199,
       4,   290,   235,   200,   180,   200,   239,   392,  1212,   422,
    1220,   200,   340,   410,   925,  1219,   927,  1221,   180,   199,
      79,  1064,   171,    82,   707,   290,   202,   201,   200,   712,
     200,   296,   200,   201,   200,   300,  1210,   434,   171,   200,
     202,   200,   200,   200,   200,   200,   443,   200,   200,   446,
    1083,   199,   199,   337,   199,   199,   199,   199,   199,     4,
     325,   389,   390,  1105,   813,    66,   198,   201,    66,   201,
      66,   336,    66,    66,    66,   163,    79,   198,   198,    66,
     201,    26,    27,   201,   203,   201,   198,  1297,    66,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   850,   380,    35,   382,   383,    35,
     385,   201,  1326,   198,   560,   430,   393,   430,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,    97,  1316,   198,  1318,   318,   411,   412,    97,   414,
     415,   416,   417,   575,   158,    50,  1366,   198,   423,    79,
      79,   426,    79,    79,    79,   800,   111,    79,    85,   585,
     198,   436,   155,   438,   100,   101,   100,   101,    35,   444,
       4,  1084,   128,   129,   198,   452,   198,   452,   838,   454,
     838,  1101,   195,    79,    35,    97,    97,   153,   201,    85,
     201,   637,   154,   201,   153,   201,   201,   201,   201,   201,
     640,     4,   155,   272,   273,   274,   481,   198,   201,   484,
     485,   486,    79,   201,   457,   168,    50,   585,    79,    53,
     147,   148,   198,  1266,   647,   147,   148,  1411,    79,   922,
    1036,   116,   672,   198,   163,   304,    70,   163,    30,   124,
     515,   153,   153,   903,   866,   903,   202,   153,   201,   154,
     690,   147,   148,    87,   209,    89,    14,   689,   198,    93,
      94,   216,   379,   695,   169,   603,   202,   222,   202,   386,
      79,   823,    30,   203,    35,    79,    85,   872,   203,   206,
     235,    85,   728,   200,   239,   146,   147,   148,   170,    47,
     170,  1334,    29,   410,    99,   100,   101,   200,   132,    79,
     250,   251,   695,   743,  1050,    85,   256,  1053,    79,    46,
     324,   751,    49,  1226,    85,  1228,   198,   434,   198,    79,
     595,    99,   100,   101,   279,    85,   443,   128,   129,   446,
     133,   200,   607,   288,   289,   290,    46,    47,   147,   148,
     295,   200,   146,   147,   148,   200,   301,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     194,   149,   149,   200,   639,    66,   146,   147,   148,   324,
     113,   114,   115,   648,    71,    72,   147,   148,    66,   182,
    1564,    66,   170,   170,   334,   200,   569,   147,   148,   201,
     298,   198,   667,  1199,   302,    29,    63,    64,   207,   951,
     149,   235,   115,   116,   117,   239,    71,    72,   198,   243,
    1018,   198,    46,   201,   201,    49,   823,    66,   326,  1332,
     328,   329,   330,   331,   699,    49,    50,    51,   262,    53,
     149,   838,  1697,   198,   503,   126,   127,  1059,   507,   153,
     715,    65,    44,   512,   200,   711,  1068,  1712,   200,   201,
      65,   897,    74,    75,    76,   200,   201,  1590,  1591,  1104,
     149,   128,   129,   170,    86,   420,  1586,  1587,  1214,    46,
      47,    48,    49,    50,    51,   430,    53,   720,   753,  1172,
     198,   113,   114,   115,   116,   117,   118,     9,    65,   205,
     324,   149,   149,   198,   940,     8,   903,   730,   731,   333,
     200,   947,   457,   337,   170,  1418,   340,  1691,   198,   149,
     149,    14,   134,   135,   136,   137,   138,    79,   200,   469,
     200,   324,  1706,   145,   124,   124,    14,   199,   170,   151,
     152,  1083,   199,   871,   809,    14,  1181,   113,   114,   115,
     116,   117,   118,   165,   951,    97,  1239,   822,   180,   199,
     204,   199,   386,   387,   388,   389,   390,   179,   199,   105,
     198,   198,  1255,   198,     9,    89,   199,     9,   200,   812,
     199,   585,   847,   812,   200,    14,   410,   812,   812,   198,
       9,  1327,   857,   184,    79,   860,    79,   862,   391,    79,
     187,   866,   198,   200,  1216,     9,  1666,   105,     9,   126,
     434,    79,   200,  1043,   180,   113,   114,   115,   116,   117,
     118,   199,   446,   199,  1684,   565,   566,   199,   198,   200,
     199,    30,  1692,   457,   574,    66,   127,   902,   169,   812,
     130,     9,  1078,   113,   114,   115,   116,   117,   118,   594,
     474,   199,   908,  1336,   124,   125,    14,   149,   196,     9,
      66,   834,  1345,  1566,    46,    47,    48,    49,    50,    51,
       9,   171,   199,  1356,     9,   126,   909,   850,    14,   205,
     205,   202,   180,    65,     9,   198,  1083,   181,   182,   183,
     514,   199,   162,  1129,   188,   189,   205,   199,   192,   193,
    1136,   705,  1314,   198,   649,    97,   205,   130,   149,     9,
     180,   199,   199,   772,   200,   539,   540,   776,   200,   984,
     985,   986,   198,   149,  1266,   990,   991,   198,   149,   201,
     184,   838,   184,   113,   114,   115,   116,   117,   118,    14,
       9,  1424,    79,   201,   124,   125,   691,    14,   693,    14,
     200,  1018,   201,  1018,   201,    14,   200,   205,   196,    30,
     705,   198,   198,   767,    30,   872,    14,    48,   198,     9,
     198,   198,   717,   198,  1609,   720,   200,   199,   130,   603,
     160,  1046,   162,   200,    14,     9,   199,    65,     9,  1022,
     730,   731,  1334,  1022,  1059,   205,   903,  1022,  1022,  1235,
     180,   146,    79,  1068,  1069,    97,     9,   198,   812,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,   767,   130,   200,    14,    79,   199,   832,   198,
     201,   198,   130,   199,     9,   205,  1101,  1093,   201,   201,
     785,    86,   146,    30,    73,   200,  1111,  1682,   199,  1022,
     171,   200,    30,   199,   130,   800,   801,   199,    63,    64,
     684,     9,   686,  1096,   202,   199,   130,   812,     9,  1266,
     199,   202,    14,  1556,   814,  1558,   198,  1050,   201,    79,
    1053,   705,  1298,   199,  1567,   199,   199,   832,   198,   829,
     201,   199,   130,   199,   199,   719,   720,     9,    30,   200,
     199,   841,   199,    97,   200,   200,   158,   911,  1293,   913,
     201,   106,   705,   200,    14,   154,    79,   199,   111,   199,
     130,   130,   201,   128,   129,   199,  1191,    14,   170,  1612,
    1195,   201,  1197,    79,    14,    14,    79,  1334,   200,   199,
    1205,     4,   198,   767,   201,   199,   130,    14,    14,   773,
    1215,  1216,    14,   777,   778,   201,   200,   200,    55,   200,
       4,    79,   198,    79,   909,   200,   911,     9,   913,    79,
     915,   916,  1385,   797,   767,   109,    97,   149,    97,   161,
      33,    14,    45,   198,   200,   199,   198,   167,   812,   929,
     171,   931,   199,    79,   199,   164,     9,    79,  1282,   200,
     199,    45,   826,   199,   201,    14,  1179,    79,   832,    14,
    1275,    79,    14,    14,   838,  1188,    26,    27,  1022,   812,
      30,    79,    79,   512,   776,   772,   390,  1710,  1674,   389,
     387,   870,   867,  1361,  1717,  1688,   815,  1102,  1252,   832,
    1684,  1214,  1423,   106,   517,  1414,  1311,   871,   111,  1314,
     113,   114,   115,   116,   117,   118,   119,  1446,  1530,   883,
     884,   885,   106,  1704,  1716,  1542,  1284,   111,    42,   113,
     114,   115,   116,   117,   118,   119,  1410,  1022,   393,   903,
    1288,  1365,   993,   488,   990,   909,   488,   911,   764,   913,
    1026,  1066,   948,   156,   157,  1035,   159,  1037,   884,  1079,
    1045,  1080,   838,  1387,  1108,  1389,   296,   899,   932,  1342,
     289,   933,   156,   157,   981,   159,    -1,   180,   911,   730,
     913,  1009,  1062,    -1,    -1,    -1,   950,    -1,    -1,   953,
      -1,    -1,    -1,  1546,    -1,    -1,   180,  1402,    -1,   202,
      -1,    -1,    -1,  1088,  1317,    -1,    -1,    -1,    -1,    -1,
    1323,  1096,  1325,    -1,  1327,    -1,    -1,   981,   202,  1104,
    1105,    -1,    -1,  1108,    -1,    -1,    -1,  1432,    -1,    -1,
      -1,    -1,    -1,    -1,   998,  1179,    -1,  1001,    -1,  1119,
      -1,    -1,    -1,    -1,  1188,    -1,    -1,    -1,    -1,  1422,
    1423,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1022,   209,
      -1,    -1,    -1,    -1,    -1,    -1,   216,    -1,    -1,    -1,
      -1,    -1,   222,  1541,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,  1022,
      -1,    -1,    -1,    -1,    -1,    -1,  1181,  1177,    -1,    -1,
      -1,    -1,    -1,  1416,    -1,  1661,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1080,    -1,    -1,    -1,
    1084,    -1,    -1,    63,    64,    77,    -1,    79,    -1,   279,
      -1,    -1,  1096,  1277,    -1,  1603,    -1,    -1,   288,   289,
      -1,    -1,    -1,    -1,  1108,   295,  1551,    -1,    -1,    -1,
      -1,   301,    -1,  1678,  1298,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   312,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   124,  1317,    -1,  1108,    -1,    -1,  1258,  1323,
    1260,  1325,    -1,    -1,    -1,    -1,    -1,    -1,   128,   129,
      -1,    -1,  1277,  1337,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1346,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    -1,  1176,  1293,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   379,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    -1,    -1,    -1,    -1,    -1,    -1,   201,
      -1,   203,  1337,    -1,    -1,    -1,    -1,  1342,    -1,    -1,
      -1,  1346,  1226,    -1,  1228,    26,    27,    -1,    -1,    30,
     420,    -1,  1416,    -1,    -1,    -1,    -1,  1421,  1358,    -1,
     430,    -1,    -1,  1427,    -1,    -1,    -1,    -1,    -1,  1433,
      -1,    -1,    53,    -1,    -1,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,  1277,    -1,    -1,    -1,  1722,  1282,    -1,
      -1,    -1,    -1,    -1,  1288,  1730,     4,    -1,    -1,    -1,
      -1,  1736,    -1,    -1,  1739,    -1,  1421,  1422,  1423,    -1,
      -1,    -1,  1427,    -1,  1277,    -1,    63,    64,  1433,    -1,
      -1,    -1,    -1,  1666,    -1,    -1,    -1,    -1,    -1,  1292,
      -1,    -1,    -1,    -1,  1444,    -1,    -1,    45,  1332,    -1,
      -1,  1684,    -1,  1337,    -1,    -1,    -1,    -1,  1342,  1692,
      -1,    -1,  1346,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,    27,    -1,    -1,    -1,  1361,    -1,    -1,
      -1,  1365,    -1,    -1,  1337,    -1,    -1,    -1,    -1,    -1,
    1374,   128,   129,  1346,    -1,    -1,  1380,    -1,    -1,    -1,
      -1,    -1,    -1,  1387,    -1,  1389,    -1,  1571,   106,    -1,
      -1,  1395,    -1,   111,    -1,   113,   114,   115,   116,   117,
     118,   119,    -1,    -1,   594,    -1,    -1,    -1,   209,    -1,
      -1,    -1,    -1,    -1,  1418,   216,    -1,  1421,  1422,  1423,
    1393,   222,    -1,  1427,    -1,  1545,  1610,  1611,    -1,  1433,
      -1,    -1,    -1,  1617,    -1,    -1,    -1,    -1,   156,   157,
      -1,   159,    -1,    -1,    -1,    -1,  1571,    -1,  1421,   250,
     251,    -1,    -1,    -1,  1427,   256,    -1,    -1,    -1,   649,
    1433,    -1,   180,    -1,    -1,    -1,    -1,    -1,    -1,  1653,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1661,   279,    -1,
      -1,    -1,    -1,    -1,   202,  1610,  1611,   288,   289,    -1,
      -1,    -1,  1617,    -1,   295,    -1,    -1,    -1,    -1,    -1,
     301,   691,    -1,   693,    -1,    -1,    -1,  1627,    -1,    -1,
      -1,   312,    -1,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,   717,  1653,    -1,
      -1,    -1,    -1,   334,  1718,   209,   337,  1541,    65,    -1,
      -1,  1725,   216,    -1,    -1,    -1,    -1,    -1,   222,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1562,    -1,
      -1,    -1,  1566,    -1,    -1,    -1,    -1,  1571,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1580,  1697,   379,     4,
      -1,    -1,  1586,  1587,    -1,    -1,  1590,  1591,    -1,    -1,
      -1,    -1,  1712,  1718,    -1,   785,    -1,    -1,  1571,  1603,
    1725,    -1,    -1,    -1,    -1,   279,  1610,  1611,    -1,    -1,
     800,   801,    -1,  1617,   288,   289,   290,    -1,    -1,   420,
      45,   295,    -1,    -1,    -1,    -1,    -1,   301,    -1,   430,
      -1,    -1,    -1,   823,    -1,    -1,    -1,  1610,  1611,    -1,
      -1,    -1,    -1,    -1,  1617,    -1,    -1,    -1,    -1,  1653,
      -1,    -1,    -1,    -1,    -1,    -1,  1660,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   469,   470,
      -1,    -1,   473,  1677,    -1,    -1,    -1,    -1,    -1,    -1,
    1653,   106,   872,    -1,    -1,    -1,   111,    -1,   113,   114,
     115,   116,   117,   118,   119,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,  1718,    53,    -1,    -1,    -1,    -1,
     521,  1725,    -1,    -1,    -1,   915,   916,    65,    -1,    -1,
      -1,   156,   157,    -1,   159,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1718,   420,    -1,    -1,    -1,
      -1,    -1,  1725,    -1,    -1,   180,   430,    -1,    -1,    -1,
      -1,   951,    -1,    -1,   565,   566,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   574,    -1,    -1,    -1,   202,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   594,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,   649,    -1,
      -1,    -1,    -1,    -1,    -1,  1045,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     691,    -1,   693,  1083,    65,    -1,    -1,    -1,  1088,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    -1,    -1,  1104,  1105,   717,   718,    -1,    -1,
     594,    10,    11,    12,    65,    -1,    -1,    -1,    -1,   730,
     731,   732,   733,   734,   735,   736,    -1,    -1,    -1,    -1,
      29,   742,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,   768,    -1,    -1,
      -1,    -1,    -1,   205,    -1,   649,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   785,    -1,    -1,    -1,    -1,    -1,
      -1,  1181,    -1,    -1,    26,    27,    -1,   798,    30,   800,
     801,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   814,   815,    -1,    -1,   691,    -1,   693,
      -1,    -1,   823,    -1,    -1,    -1,    -1,    -1,   829,    -1,
      -1,   202,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     841,    -1,    -1,   717,    -1,    -1,    -1,    -1,   849,    -1,
      -1,   852,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   872,    -1,    -1,    -1,    -1,  1266,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,   785,    -1,   202,   915,   916,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   800,   801,   929,    -1,
     931,    -1,   933,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1334,    -1,    -1,   948,    -1,    -1,
     951,    -1,    -1,   954,   955,   956,   957,   958,   959,   960,
     961,   962,   963,   964,   965,   966,   967,   968,   969,   970,
     971,   972,   973,   974,   975,   976,   977,   978,   979,   980,
      -1,    -1,    -1,    -1,   216,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1006,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,  1035,    -1,  1037,    -1,    -1,    -1,
      -1,   915,   916,    -1,  1045,    65,    -1,   279,    -1,    -1,
      -1,    -1,    -1,   202,    -1,    -1,   288,   289,    -1,    -1,
      -1,  1062,    -1,   295,    -1,    -1,    -1,    -1,    -1,   301,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     312,    -1,  1083,    -1,    -1,    -1,    -1,  1088,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1104,  1105,    -1,  1107,    -1,    -1,    -1,
      77,    -1,    79,    -1,    -1,    -1,    -1,    -1,  1119,    -1,
      -1,  1122,    -1,  1124,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1140,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   379,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,  1045,    53,    -1,    -1,    -1,  1177,  1178,    -1,    -1,
    1181,    -1,   202,    -1,    65,    -1,    -1,   154,   420,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1088,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,    -1,
    1104,  1105,    -1,    -1,   201,    -1,   203,    -1,    -1,    -1,
      -1,   473,    -1,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1258,    -1,  1260,
      -1,    -1,    -1,    -1,  1265,  1266,    -1,    -1,  1269,    -1,
    1271,    -1,    55,  1274,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1282,  1283,    -1,    -1,  1286,    -1,    -1,    -1,   521,
      -1,    -1,  1293,    -1,    77,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1181,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   202,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1334,    -1,    -1,    -1,    53,    -1,    -1,
      -1,    77,    -1,    79,    -1,    -1,    -1,    -1,  1349,   132,
     133,    -1,    -1,    -1,    -1,    -1,    -1,  1358,  1359,  1360,
      -1,    -1,   594,    -1,  1365,    -1,    -1,   150,    -1,    -1,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1387,    -1,  1389,    -1,
      -1,   174,    -1,    -1,  1395,    -1,    -1,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,    -1,    -1,   198,    -1,   649,   154,   202,
     156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,  1430,
    1431,    -1,    -1,    -1,    -1,    -1,    -1,  1438,  1439,    -1,
      -1,    -1,    -1,  1444,    -1,  1446,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   691,
      -1,   693,    -1,    -1,    -1,   201,    -1,   203,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,   717,   718,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    65,    -1,
     732,   733,   734,   735,   736,    -1,    -1,    -1,    -1,    29,
     742,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,   250,   251,   768,    -1,    -1,    -1,
     256,    -1,    -1,    -1,  1545,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   785,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1562,    -1,    -1,    -1,    -1,   798,    -1,   800,   801,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1580,
      -1,    -1,    -1,   815,  1585,    -1,    -1,    -1,    -1,    -1,
      -1,   823,    -1,    -1,    -1,  1596,    -1,    -1,    -1,    -1,
    1601,    -1,    -1,    -1,  1605,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   849,   334,    -1,
     852,   337,    -1,    -1,    -1,    -1,  1627,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     872,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,   473,
      -1,    -1,    -1,    -1,  1665,    -1,    -1,    -1,    -1,    -1,
      65,    -1,  1673,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   202,   915,   916,    -1,    -1,  1688,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1697,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   521,    -1,    -1,
      -1,  1712,    -1,    -1,    -1,    -1,   948,    -1,    -1,   951,
      -1,    -1,   954,   955,   956,   957,   958,   959,   960,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,   980,    -1,
      -1,    -1,    -1,   469,   470,    -1,    -1,   473,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1006,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,   521,    10,    11,    12,    -1,
      -1,    -1,    -1,  1045,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,   565,
     566,  1083,    -1,    -1,    -1,    -1,  1088,    -1,   574,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1104,  1105,    -1,  1107,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1122,    -1,  1124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   718,    -1,    -1,    -1,  1140,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   732,   733,
     734,   735,   736,    -1,    -1,    -1,    -1,    -1,   742,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1178,    -1,    -1,  1181,
      29,   202,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   199,    -1,    -1,    -1,    -1,
      -1,    -1,   718,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   730,   731,   732,   733,   734,   735,
     736,    -1,    -1,    -1,    -1,    -1,   742,    -1,    -1,    -1,
      -1,    -1,    -1,  1265,  1266,   849,    -1,  1269,    -1,  1271,
      -1,    -1,  1274,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,  1283,    -1,    -1,  1286,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   814,    65,
      -1,    -1,  1334,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   829,    -1,    -1,    -1,  1349,    -1,    -1,
      -1,    -1,    -1,   202,    -1,   841,    -1,  1359,  1360,    -1,
      -1,    -1,    -1,   849,   948,    -1,    -1,    -1,    -1,    -1,
     954,   955,   956,   957,   958,   959,   960,   961,   962,   963,
     964,   965,   966,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,   978,   979,   980,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,  1006,    53,    -1,    -1,    -1,    -1,  1430,  1431,
      -1,    55,    -1,    -1,    -1,    65,  1438,  1439,    -1,    -1,
      -1,    -1,    -1,   929,  1446,   931,    -1,   933,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   948,    -1,    -1,    -1,   202,    -1,   954,   955,
     956,   957,   958,   959,   960,   961,   962,   963,   964,   965,
     966,   967,   968,   969,   970,   971,   972,   973,   974,   975,
     976,   977,   978,   979,   980,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    79,    -1,    -1,    -1,   132,   133,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1006,    -1,    -1,  1107,    -1,    -1,   150,    -1,    -1,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,  1122,    -1,
    1124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1035,
     174,  1037,    -1,    -1,    -1,    -1,  1140,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,    -1,    -1,    -1,   198,    -1,  1062,    -1,   202,    -1,
      -1,   156,   157,  1585,   159,   160,   161,    74,    75,    76,
      77,    -1,    -1,    -1,  1596,    -1,    -1,    -1,    -1,  1601,
      -1,    -1,    -1,  1605,    -1,    -1,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      -1,  1107,    -1,    -1,    -1,    -1,   201,    -1,   203,    -1,
      -1,    -1,    -1,  1119,    -1,    -1,  1122,    -1,  1124,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1140,    -1,    -1,    -1,    -1,    -1,
      -1,   473,    -1,  1665,    -1,    -1,    -1,    -1,    -1,   156,
     157,  1673,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,
      -1,  1265,    -1,    -1,    -1,  1269,  1688,  1271,    -1,    -1,
    1274,  1177,    10,    11,    12,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,   521,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   473,    65,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,  1349,    53,    -1,    -1,    -1,
      -1,    -1,  1258,    -1,  1260,    -1,    -1,    -1,    65,  1265,
      -1,    -1,    -1,  1269,    -1,  1271,    -1,    -1,  1274,    10,
      11,    12,    -1,    -1,   521,    -1,  1282,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1293,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,  1430,  1431,    -1,    -1,
      -1,    -1,    -1,    -1,  1438,    77,    -1,    79,    -1,    -1,
      -1,  1445,    -1,  1349,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1358,    -1,    -1,    -1,    -1,    -1,    -1,  1365,
      -1,    -1,    -1,    -1,   202,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   118,    -1,    -1,    -1,
      -1,  1387,    -1,  1389,    -1,    -1,   718,    -1,    -1,  1395,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     732,   733,   734,   735,   736,    -1,    -1,    -1,   150,    -1,
     742,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    -1,    -1,  1430,  1431,    -1,    -1,    -1,    -1,
      -1,    -1,  1438,    -1,    -1,    -1,    77,    -1,  1444,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    -1,    -1,    -1,   198,    -1,    -1,   200,
      -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   718,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1585,    -1,    -1,    -1,   732,   733,   734,   735,    -1,
      -1,    -1,  1596,    -1,    -1,   742,    -1,  1601,    77,    -1,
      -1,  1605,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   153,    -1,    -1,   156,   157,   849,   159,   160,
     161,    -1,    -1,    -1,    -1,  1629,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1545,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    -1,    -1,  1562,    -1,    -1,    -1,
     201,  1665,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,  1580,    -1,    -1,   156,   157,  1585,
     159,   160,   161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1596,    -1,    -1,    -1,    -1,  1601,    -1,    -1,    -1,  1605,
      -1,    -1,   849,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   948,    -1,    -1,   198,
      -1,  1627,   954,   955,   956,   957,   958,   959,   960,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,   980,   154,
      -1,   156,   157,   158,   159,   160,   161,    -1,    -1,  1665,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1006,    -1,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      -1,  1697,    -1,   198,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1712,   954,   955,   956,
     957,   958,   959,   960,   961,   962,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,  1006,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,     3,
       4,     5,     6,     7,    -1,  1107,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1122,    -1,  1124,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1140,    -1,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    82,    -1,
      -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,    93,
    1107,    95,    -1,    -1,    98,    -1,    -1,    -1,   102,   103,
     104,   105,    -1,   107,   108,  1122,   110,  1124,   112,   113,
     114,   115,   116,   117,   118,    -1,   120,   121,   122,    -1,
     124,   125,    -1,  1140,    -1,    -1,    -1,   131,   132,    77,
     134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,
      -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,   162,    -1,
      -1,   165,    -1,  1265,   168,    -1,    -1,  1269,    -1,  1271,
     174,    -1,  1274,    -1,   122,   179,   180,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,   198,    -1,   200,   201,   202,   203,
     204,    -1,   206,   207,    -1,    -1,    -1,    77,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,  1349,  1265,    -1,
     198,    -1,  1269,    -1,  1271,    -1,    -1,  1274,    -1,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,     3,     4,     5,     6,     7,
      -1,    -1,    10,    11,    12,    13,   156,   157,    65,   159,
     160,   161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    53,    -1,    -1,  1430,  1431,
      -1,    -1,  1349,    -1,    -1,    -1,  1438,    -1,    -1,    67,
      68,    69,    70,    71,    72,    73,    -1,    -1,    -1,    77,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,  1430,  1431,    -1,    -1,    -1,    -1,    -1,
      -1,  1438,   150,   151,   152,    -1,    -1,    -1,   156,   157,
      -1,   159,   160,   161,   162,    -1,   164,   165,    -1,   167,
      -1,    -1,    -1,    -1,    -1,    -1,   174,   175,    -1,   177,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,  1585,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,  1596,    -1,    65,    -1,    -1,  1601,
      -1,    -1,    -1,  1605,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,  1665,    -1,    -1,    -1,    -1,  1585,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,  1596,
      -1,    -1,    -1,    -1,  1601,    -1,    -1,    -1,  1605,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    95,    -1,    -1,    98,    -1,    -1,  1665,   102,
     103,   104,   105,   106,   107,   108,    -1,   110,   111,   112,
     113,   114,   115,   116,   117,   118,    -1,   120,   121,   122,
     123,   124,   125,    -1,    -1,   200,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,   162,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,   175,    -1,   177,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    -1,   200,   201,   202,
     203,   204,    -1,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    -1,    -1,    -1,   102,   103,   104,   105,   106,   107,
     108,    -1,   110,   111,   112,   113,   114,   115,   116,   117,
     118,    -1,   120,   121,   122,   123,   124,   125,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,   175,    -1,   177,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,   200,   201,   202,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,   102,
     103,   104,   105,   106,   107,   108,    -1,   110,   111,   112,
     113,   114,   115,   116,   117,   118,    -1,   120,   121,   122,
     123,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,   162,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,   175,    -1,   177,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    -1,   200,   201,    -1,
     203,   204,    -1,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,   107,
     108,    -1,   110,    -1,   112,   113,   114,   115,   116,   117,
     118,    -1,   120,   121,   122,    -1,   124,   125,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,   200,   201,   202,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,   102,
     103,   104,   105,    -1,   107,   108,    -1,   110,    -1,   112,
     113,   114,   115,   116,   117,   118,    -1,   120,   121,   122,
      -1,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,   162,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    -1,   200,   201,   202,
     203,   204,    -1,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,   107,
     108,    -1,   110,    -1,   112,   113,   114,   115,   116,   117,
     118,    -1,   120,   121,   122,    -1,   124,   125,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,   200,   201,   202,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    90,    91,    -1,
      93,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,   102,
     103,   104,   105,    -1,   107,   108,    -1,   110,    -1,   112,
     113,   114,   115,   116,   117,   118,    -1,   120,   121,   122,
      -1,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,   162,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    -1,   200,   201,    -1,
     203,   204,    -1,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    96,    -1,
      98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,   107,
     108,    -1,   110,    -1,   112,   113,   114,   115,   116,   117,
     118,    -1,   120,   121,   122,    -1,   124,   125,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,   200,   201,    -1,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,   102,
     103,   104,   105,    -1,   107,   108,    -1,   110,    -1,   112,
     113,   114,   115,   116,   117,   118,    -1,   120,   121,   122,
      -1,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,   162,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    -1,   200,   201,   202,
     203,   204,    -1,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,   107,
     108,    -1,   110,    -1,   112,   113,   114,   115,   116,   117,
     118,    -1,   120,   121,   122,    -1,   124,   125,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,   200,   201,   202,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    94,    95,    -1,    -1,    98,    -1,    -1,    -1,   102,
     103,   104,   105,    -1,   107,   108,    -1,   110,    -1,   112,
     113,   114,   115,   116,   117,   118,    -1,   120,   121,   122,
      -1,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,   162,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    -1,   200,   201,    -1,
     203,   204,    -1,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,   107,
     108,    -1,   110,    -1,   112,   113,   114,   115,   116,   117,
     118,    -1,   120,   121,   122,    -1,   124,   125,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,   200,   201,   202,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,   102,
     103,   104,   105,    -1,   107,   108,    -1,   110,    -1,   112,
     113,   114,   115,   116,   117,   118,    -1,   120,   121,   122,
      -1,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,   162,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    -1,   200,   201,   202,
     203,   204,    -1,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    92,    93,    -1,    95,    -1,    -1,
      98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,   107,
     108,    -1,   110,    -1,   112,   113,   114,   115,   116,   117,
     118,    -1,   120,   121,   122,    -1,   124,   125,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,   200,   201,    -1,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,   102,
     103,   104,   105,    -1,   107,   108,    -1,   110,    -1,   112,
     113,   114,   115,   116,   117,   118,    -1,   120,   121,   122,
      -1,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,   162,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    -1,   200,   201,   202,
     203,   204,    -1,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,   107,
     108,    -1,   110,    -1,   112,   113,   114,   115,   116,   117,
     118,    -1,   120,   121,   122,    -1,   124,   125,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,   200,   201,   202,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,   102,
     103,   104,   105,    -1,   107,   108,    -1,   110,    -1,   112,
     113,   114,   115,   116,   117,   118,    -1,   120,   121,   122,
      -1,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,   162,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    -1,   200,   201,   202,
     203,   204,    -1,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,   107,
     108,    -1,   110,    -1,   112,   113,   114,   115,   116,   117,
     118,    -1,   120,   121,   122,    -1,   124,   125,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,   200,   201,    -1,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    30,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,   102,
     103,   104,   105,    -1,   107,   108,    -1,   110,    -1,   112,
     113,   114,   115,   116,   117,   118,    -1,   120,   121,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    -1,   200,   201,    -1,
     203,   204,    -1,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,   107,
     108,    -1,   110,    -1,   112,   113,   114,   115,   116,   117,
     118,    -1,   120,   121,   122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,   200,   201,    -1,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    30,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,   102,
     103,   104,   105,    -1,   107,   108,    -1,   110,    -1,   112,
     113,   114,   115,   116,   117,   118,    -1,   120,   121,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    -1,   200,   201,    -1,
     203,   204,    -1,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,   107,
     108,    -1,   110,    -1,   112,   113,   114,   115,   116,   117,
     118,    -1,   120,   121,   122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,   200,   201,    -1,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    30,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    82,
      -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,
      93,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,   102,
     103,   104,   105,    -1,   107,   108,    -1,   110,    -1,   112,
     113,   114,   115,   116,   117,   118,    -1,   120,   121,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    -1,   200,   201,    -1,
     203,   204,    -1,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,   107,
     108,    -1,   110,    -1,   112,   113,   114,   115,   116,   117,
     118,    -1,   120,   121,   122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,   200,   201,    -1,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     113,   114,   115,   116,   117,   118,    -1,    -1,   121,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,    -1,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    -1,    -1,    -1,    -1,
     203,   204,    -1,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,   117,
     118,    -1,    -1,   121,   122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,   200,    -1,    -1,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    29,
      13,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    35,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    65,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     113,   114,   115,   116,   117,   118,    -1,    -1,   121,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
     163,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    -1,    -1,    -1,    -1,
     203,   204,    -1,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,   117,
     118,    -1,    -1,   121,   122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    11,    12,   201,    -1,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    29,
      13,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    35,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    65,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     113,   114,   115,   116,   117,   118,    -1,    -1,   121,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
     163,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    -1,    -1,    -1,    -1,
     203,   204,    -1,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,   117,
     118,    -1,    -1,   121,   122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,    10,    11,    12,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    65,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   102,
      -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     113,   114,   115,   116,   117,   118,    -1,    -1,   121,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,   190,   191,    -1,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    11,    12,    -1,    -1,
     203,   204,    -1,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    29,    13,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      65,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,   117,
     118,    -1,    -1,   121,   122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,    10,    11,    12,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    65,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     113,   114,   115,   116,   117,   118,    -1,    -1,   121,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,   187,
      -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    -1,   200,    -1,    12,
     203,   204,    -1,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    65,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,   117,
     118,    -1,    -1,   121,   122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,   200,    -1,    12,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    65,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     113,   114,   115,   116,   117,   118,    -1,    -1,   121,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,   199,    -1,    -1,    -1,
     203,   204,    -1,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,   117,
     118,    -1,    -1,   121,   122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,    -1,    -1,    -1,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    29,
      13,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    65,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     113,   114,   115,   116,   117,   118,    -1,    -1,   121,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    -1,    -1,    -1,    -1,
     203,   204,    -1,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      65,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,   117,
     118,    -1,    -1,   121,   122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,    -1,    -1,    -1,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    35,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    65,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     113,   114,   115,   116,   117,   118,    -1,    -1,   121,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    -1,    -1,    -1,    -1,
     203,   204,    -1,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      65,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,   117,
     118,    -1,    -1,   121,   122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,    10,    11,    12,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    65,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     113,   114,   115,   116,   117,   118,    -1,    -1,   121,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,   165,    -1,    -1,   168,    -1,    -1,   186,    -1,
      -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    -1,    10,    11,    12,
     203,   204,    -1,   206,   207,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    65,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,   117,
     118,    -1,    -1,   121,   122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,    -1,
     168,    -1,   185,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    10,    11,    12,    -1,   203,   204,    -1,   206,   207,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,   200,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    77,   200,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,   200,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,   150,    53,    29,
     153,    -1,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      65,    -1,    -1,   200,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,    -1,    -1,    29,    -1,    77,    -1,   202,
      -1,    -1,    -1,    -1,    -1,    -1,   199,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    55,    -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   113,   114,   115,   116,   117,   118,    -1,
      -1,    -1,    29,    77,    -1,    -1,    -1,    -1,    -1,    -1,
     199,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
     150,   105,    -1,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   194,
      77,    -1,    -1,    -1,   174,    -1,    -1,    -1,   132,   133,
     180,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    -1,   150,    29,   198,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     174,    -1,    -1,    55,    -1,   132,   133,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,    -1,    -1,   150,   198,    77,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,   163,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    77,    -1,   174,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,    -1,
      55,   198,    -1,   105,   106,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,   150,    -1,
      -1,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
      -1,   163,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      -1,   153,   174,    -1,   156,   157,    -1,   159,   160,   161,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    -1,    55,    -1,   198,   132,   133,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    -1,    -1,   150,    77,    -1,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,   174,
     175,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      -1,    30,    -1,   198,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   132,   133,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    -1,    -1,    -1,   150,
      -1,    -1,   153,   154,    -1,   156,   157,    66,   159,   160,
     161,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    -1,
      -1,    -1,    35,   174,    -1,   156,   157,    86,   159,   160,
     161,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    -1,    -1,    -1,   198,    -1,    -1,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    77,    -1,    79,   198,    -1,    -1,
      -1,    -1,    -1,   132,    -1,   134,   135,   136,   137,   138,
      -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,
      -1,   150,   151,   152,   153,   154,    -1,   156,   157,    -1,
     159,   160,   161,    -1,    -1,   118,   165,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,   131,    -1,
     179,    -1,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,    -1,   150,    77,   198,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,    46,    47,   198,    -1,    -1,    -1,    52,
     203,    54,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,   154,    -1,   156,   157,    -1,
     159,   160,   161,    86,   150,    -1,    -1,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   132,
      -1,   134,   135,   136,   137,   138,    -1,   203,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,   165,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   174,    46,    47,    -1,    -1,   179,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    66,    -1,    -1,   198,    10,    11,    12,    -1,
      74,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    67,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    77,    -1,    79,    -1,    -1,    -1,   132,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     105,    -1,   156,   157,    -1,   159,   160,   161,   113,   114,
     115,   116,   117,   118,    67,    68,    -1,    -1,    -1,    -1,
     174,    -1,    -1,    -1,    77,    -1,    79,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,    -1,    -1,    -1,    -1,   150,    -1,    -1,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,
      -1,    -1,    -1,   168,    77,   118,    79,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   180,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,   198,    -1,    -1,    -1,   150,   203,    -1,
     153,   154,    68,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    77,    -1,    79,    -1,   168,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,   156,   157,   198,   159,   160,   161,    -1,
     203,    -1,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    79,    -1,    -1,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,    -1,   150,    -1,    -1,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   118,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,    -1,
      -1,    -1,   198,    -1,    77,   150,    -1,   203,   153,   154,
      -1,   156,   157,    86,   159,   160,   161,    -1,    77,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      -1,    -1,    -1,   198,    -1,    -1,   201,    -1,   203,   118,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   131,    -1,    -1,    -1,    -1,   150,    -1,    -1,
     153,    -1,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,   150,    -1,    -1,   153,   154,    -1,   156,   157,    -1,
     159,   160,   161,    -1,    77,    -1,    79,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,    -1,    -1,    -1,   198,
      -1,    -1,    -1,    -1,   203,   118,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    77,    -1,    79,   131,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,   150,    53,    -1,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      65,    -1,    -1,    -1,    -1,    -1,   118,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,    -1,    -1,   198,    -1,    -1,   150,    -1,
     203,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    10,    11,    12,   198,    -1,    -1,    -1,
      -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,   130,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,   130,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,   130,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,   130,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,   130,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,   130,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      77,    -1,    65,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,
      -1,    -1,    29,   130,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,   119,    77,    -1,    -1,    -1,    -1,   130,    -1,    -1,
      -1,    -1,    -1,   150,   132,   133,   153,   154,    77,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   150,    -1,    -1,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,    -1,
      77,    -1,    -1,   130,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,    -1,   153,    -1,
      -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   153,    -1,    -1,   156,   157,    77,
     159,   160,   161,    -1,    -1,    -1,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      -1,    -1,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   153,    -1,    77,   156,
     157,    -1,   159,   160,   161,    -1,   124,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   156,   157,
      -1,   159,   160,   161,    -1,   124,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   156,   157,    -1,
     159,   160,   161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,    28,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    97,    53,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   209,   210,     0,   211,     3,     4,     5,     6,     7,
      13,    27,    28,    45,    46,    47,    52,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    66,    67,
      68,    69,    70,    74,    75,    76,    77,    78,    79,    81,
      82,    86,    87,    88,    89,    91,    93,    95,    98,   102,
     103,   104,   105,   106,   107,   108,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   120,   121,   122,   123,   124,
     125,   131,   132,   134,   135,   136,   137,   138,   145,   150,
     151,   152,   153,   154,   156,   157,   159,   160,   161,   162,
     165,   168,   174,   175,   177,   179,   180,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   198,   200,   201,   203,   204,   206,   207,   212,   215,
     222,   223,   224,   225,   226,   227,   230,   246,   247,   251,
     254,   259,   265,   323,   324,   332,   336,   337,   338,   339,
     340,   341,   342,   343,   345,   348,   360,   361,   362,   364,
     365,   367,   386,   396,   397,   398,   403,   406,   425,   433,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     446,   459,   461,   463,   116,   117,   118,   131,   150,   160,
     215,   246,   323,   342,   435,   342,   198,   342,   342,   342,
     102,   342,   342,   423,   424,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,    79,   118,   198,
     223,   397,   398,   435,   438,   435,    35,   342,   450,   451,
     342,   118,   198,   223,   397,   398,   399,   434,   442,   447,
     448,   198,   333,   400,   198,   333,   349,   334,   342,   232,
     333,   198,   198,   198,   333,   200,   342,   215,   200,   342,
      29,    55,   132,   133,   154,   174,   198,   215,   226,   464,
     476,   477,   479,   181,   200,   339,   342,   366,   368,   201,
     239,   342,   105,   106,   153,   216,   219,   222,    79,   203,
     291,   292,   117,   124,   116,   124,    79,   293,   198,   198,
     198,   198,   215,   263,   465,   198,   198,    79,    85,   146,
     147,   148,   456,   457,   153,   201,   222,   222,   215,   264,
     465,   154,   198,   465,   465,    79,   195,   201,   351,   332,
     342,   343,   435,   439,   228,   201,    85,   401,   456,    85,
     456,   456,    30,   153,   170,   466,   198,     9,   200,    35,
     245,   154,   262,   465,   118,   180,   246,   324,   200,   200,
     200,   200,   200,   200,    10,    11,    12,    29,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    53,
      65,   200,    66,    66,   200,   201,   149,   125,   160,   162,
     265,   322,   323,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    63,    64,   128,   129,
     427,    66,   201,   432,   198,   198,    66,   201,   203,   443,
     198,   245,   246,    14,   342,   200,   130,    44,   215,   422,
     198,   332,   435,   439,   149,   435,   130,   205,     9,   408,
     332,   435,   466,   149,   198,   402,   427,   432,   199,   342,
      30,   230,     8,   354,     9,   200,   230,   231,   334,   335,
     342,   215,   277,   234,   200,   200,   200,   479,   479,   170,
     198,   105,   479,    14,   149,   215,    79,   200,   200,   200,
     181,   182,   183,   188,   189,   192,   193,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   381,   382,   383,   240,
     109,   167,   200,   153,   217,   220,   222,   153,   218,   221,
     222,   222,     9,   200,    97,   201,   435,     9,   200,   124,
     124,    14,     9,   200,   435,   460,   460,   332,   343,   435,
     438,   439,   199,   170,   257,   131,   435,   449,   450,    66,
     427,   146,   457,    78,   342,   435,    85,   146,   457,   222,
     214,   200,   201,   252,   260,   387,   389,    86,   203,   355,
     356,   358,   398,   443,   461,    14,    97,   462,   350,   352,
     353,   287,   288,   425,   426,   199,   199,   199,   199,   202,
     229,   230,   247,   254,   259,   425,   342,   204,   206,   207,
     215,   467,   468,   479,    35,   163,   289,   290,   342,   464,
     198,   465,   255,   245,   342,   342,   342,    30,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     399,   342,   342,   445,   445,   342,   452,   453,   124,   201,
     215,   442,   443,   263,   215,   264,   262,   246,    27,    35,
     336,   339,   342,   366,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   154,   201,   215,   428,
     429,   430,   431,   442,   445,   342,   289,   289,   445,   342,
     449,   245,   199,   342,   198,   421,     9,   408,   332,   199,
     215,    35,   342,    35,   342,   199,   199,   442,   289,   201,
     215,   428,   429,   442,   199,   228,   281,   201,   339,   342,
     342,    89,    30,   230,   275,   200,    28,    97,    14,     9,
     199,    30,   201,   278,   479,    86,   226,   473,   474,   475,
     198,     9,    46,    47,    52,    54,    66,    86,   132,   145,
     154,   174,   198,   223,   224,   226,   363,   397,   403,   404,
     405,   215,   478,   184,    79,   342,    79,    79,   342,   378,
     379,   342,   342,   371,   381,   187,   384,   228,   198,   238,
     222,   200,     9,    97,   222,   200,     9,    97,    97,   219,
     215,   342,   292,   404,    79,     9,   199,   199,   199,   199,
     199,   199,   199,   200,    46,    47,   471,   472,   126,   268,
     198,     9,   199,   199,    79,    80,   215,   458,   215,    66,
     202,   202,   211,   213,    30,   127,   267,   169,    50,   154,
     169,   391,   130,     9,   408,   199,   149,   479,   479,    14,
     354,   287,   228,   196,     9,   409,   479,   480,   427,   432,
     202,     9,   408,   171,   435,   342,   199,     9,   409,    14,
     346,   248,   126,   266,   198,   465,   342,    30,   205,   205,
     130,   202,     9,   408,   342,   466,   198,   258,   253,   261,
     256,   245,    68,   435,   342,   466,   205,   202,   199,   199,
     205,   202,   199,    46,    47,    66,    74,    75,    76,    86,
     132,   145,   174,   215,   411,   413,   414,   417,   420,   215,
     435,   435,   130,   427,   432,   199,   342,   282,    71,    72,
     283,   228,   333,   228,   335,    97,    35,   131,   272,   435,
     404,   215,    30,   230,   276,   200,   279,   200,   279,     9,
     171,   130,   149,     9,   408,   199,   163,   467,   468,   469,
     467,   404,   404,   404,   404,   404,   407,   410,   198,    85,
     149,   198,   404,   149,   201,    10,    11,    12,    29,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      65,   149,   466,   342,   184,   184,    14,   190,   191,   380,
       9,   194,   384,    79,   202,   397,   201,   242,    97,   220,
     215,    97,   221,   215,   215,   202,    14,   435,   200,     9,
     171,   215,   269,   397,   201,   449,   131,   435,    14,   205,
     342,   202,   211,   479,   269,   201,   390,    14,   342,   355,
     215,   200,   479,   196,   202,    30,   470,   426,    35,    79,
     163,   428,   429,   431,   479,    35,   163,   342,   404,   287,
     198,   397,   267,   347,   249,   342,   342,   342,   202,   198,
     289,   268,    30,   267,   266,   465,   399,   202,   198,    14,
      74,    75,    76,   215,   412,   412,   414,   415,   416,    48,
     198,    85,   146,   198,     9,   408,   199,   421,    35,   342,
     428,   429,   202,    71,    72,   284,   333,   230,   202,   200,
      90,   200,   272,   435,   198,   130,   271,    14,   228,   279,
      99,   100,   101,   279,   202,   479,   479,   215,   473,     9,
     199,   408,   130,   205,     9,   408,   407,   215,   355,   357,
     359,   199,   124,   215,   404,   454,   455,   404,   404,   404,
      30,   404,   404,   404,   404,   404,   404,   404,   404,   404,
     404,   404,   404,   404,   404,   404,   404,   404,   404,   404,
     404,   404,   404,   404,   478,   342,   342,   342,   379,   342,
     369,    79,   243,   215,   215,   404,   472,    97,     9,   297,
     199,   198,   336,   339,   342,   205,   202,   462,   297,   155,
     168,   201,   386,   393,   155,   201,   392,   130,   200,   470,
     479,   354,   480,    79,   163,    14,    79,   466,   435,   342,
     199,   287,   201,   287,   198,   130,   198,   289,   199,   201,
     479,   201,   267,   250,   402,   289,   130,   205,     9,   408,
     413,   415,   146,   355,   418,   419,   414,   435,   333,    30,
      73,   230,   200,   335,   271,   449,   272,   199,   404,    96,
      99,   200,   342,    30,   200,   280,   202,   171,   130,   163,
      30,   199,   404,   404,   199,   130,     9,   408,   199,   130,
     202,     9,   408,   404,    30,   185,   199,   228,   215,   479,
     397,     4,   106,   111,   117,   119,   156,   157,   159,   202,
     298,   321,   322,   323,   328,   329,   330,   331,   425,   449,
     202,   201,   202,    50,   342,   342,   342,   354,    35,    79,
     163,    14,    79,   342,   198,   470,   199,   297,   199,   287,
     342,   289,   199,   297,   462,   297,   201,   198,   199,   414,
     414,   199,   130,   199,     9,   408,    30,   228,   200,   199,
     199,   199,   235,   200,   200,   280,   228,   479,   479,   130,
     404,   355,   404,   404,   404,   342,   201,   202,    97,   126,
     127,   175,   464,   270,   397,   106,   331,   119,   132,   133,
     154,   160,   307,   308,   309,   397,   158,   313,   314,   122,
     198,   215,   315,   316,   299,   246,   479,     9,   200,     9,
     200,   200,   462,   322,   199,   294,   154,   388,   202,   202,
      79,   163,    14,    79,   342,   289,   111,   344,   470,   202,
     470,   199,   199,   202,   201,   202,   297,   287,   130,   414,
     355,   228,   233,   236,    30,   230,   274,   228,   199,   404,
     130,   130,   186,   228,   479,   397,   397,   465,    14,     9,
     200,   201,   464,   462,   170,   201,     9,   200,     3,     4,
       5,     6,     7,    10,    11,    12,    13,    27,    28,    53,
      67,    68,    69,    70,    71,    72,    73,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   131,   132,
     134,   135,   136,   137,   138,   150,   151,   152,   162,   164,
     165,   167,   174,   175,   177,   179,   180,   215,   394,   395,
       9,   200,   154,   158,   215,   316,   317,   318,   200,    79,
     327,   245,   300,   464,   464,    14,   246,   202,   295,   296,
     464,    14,    79,   342,   199,   198,   201,   200,   201,   319,
     344,   470,   294,   202,   199,   414,   130,    30,   230,   273,
     274,   228,   404,   404,   342,   202,   200,   200,   404,   397,
     303,   479,   310,   403,   308,    14,    30,    47,   311,   314,
       9,    33,   199,    29,    46,    49,    14,     9,   200,   465,
     327,    14,   479,   245,   200,    14,   342,    35,    79,   385,
     228,   228,   201,   319,   202,   470,   414,   228,    94,   187,
     241,   202,   215,   226,   304,   305,   306,     9,   171,     9,
     202,   404,   395,   395,    55,   312,   317,   317,    29,    46,
      49,   404,    79,   198,   200,   404,   465,   404,    79,     9,
     409,   202,   202,   228,   319,    92,   200,    79,   109,   237,
     149,    97,   479,   403,   161,    14,   301,   198,    35,    79,
     199,   202,   200,   198,   167,   244,   215,   322,   323,   171,
     404,   285,   286,   426,   302,    79,   397,   242,   164,   215,
     200,   199,     9,   409,   113,   114,   115,   325,   326,   285,
      79,   270,   200,   470,   426,   480,   199,   199,   200,   200,
     201,   320,   325,    35,    79,   163,   470,   201,   228,   480,
      79,   163,    14,    79,   320,   228,   202,    35,    79,   163,
      14,    79,   342,   202,    79,   163,    14,    79,   342,    14,
      79,   342,   342
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
#line 734 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 737 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 744 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 745 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 748 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 749 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 750 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 751 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 752 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 753 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 754 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 757 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 759 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 760 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 761 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 762 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 763 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 765 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 767 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 768 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 778 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 780 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 781 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 782 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 783 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 796 "hphp.y"
    { ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 797 "hphp.y"
    { ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 802 "hphp.y"
    { ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 803 "hphp.y"
    { ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 808 "hphp.y"
    { ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 809 "hphp.y"
    { ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 813 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 814 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 815 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 817 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 821 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 822 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 823 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 825 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 829 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 830 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 831 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 833 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 842 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 844 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 845 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 848 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 855 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 862 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 870 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 873 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 879 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 880 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 883 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 884 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 885 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 886 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 889 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 893 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 898 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 899 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 901 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 905 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 908 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 912 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 914 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 917 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 919 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 922 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 923 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 924 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 925 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 926 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 928 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 929 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 930 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 931 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 933 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 934 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 935 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 939 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 941 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 946 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 948 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 952 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 959 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 960 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 963 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 964 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 965 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 966 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval));;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 970 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 971 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 972 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 973 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 974 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 976 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 977 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 978 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 979 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 980 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 988 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 989 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 998 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 999 "hphp.y"
    { (yyval).reset();;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1003 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1005 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1011 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { (yyval).reset();;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1016 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1017 "hphp.y"
    { (yyval).reset();;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1021 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1026 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1032 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1038 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1044 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1050 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1056 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1064 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1068 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1072 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1076 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1082 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1085 "hphp.y"
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

  case 145:

/* Line 1455 of yacc.c  */
#line 1100 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1103 "hphp.y"
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

  case 147:

/* Line 1455 of yacc.c  */
#line 1117 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1120 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1125 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1128 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1135 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1138 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1146 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1149 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1157 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1158 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1162 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1165 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1168 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1169 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1170 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1173 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1174 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1178 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1179 "hphp.y"
    { (yyval).reset();;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1182 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1183 "hphp.y"
    { (yyval).reset();;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1186 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1187 "hphp.y"
    { (yyval).reset();;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1190 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1192 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1195 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1197 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1201 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1202 "hphp.y"
    { (yyval).reset();;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1205 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1206 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1207 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1211 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1213 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1216 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1218 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1221 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1223 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1226 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1228 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1238 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1239 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1240 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1241 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1246 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1248 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1249 "hphp.y"
    { (yyval).reset();;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1252 "hphp.y"
    { (yyval).reset();;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1253 "hphp.y"
    { (yyval).reset();;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1258 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1259 "hphp.y"
    { (yyval).reset();;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1264 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1265 "hphp.y"
    { (yyval).reset();;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1268 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1269 "hphp.y"
    { (yyval).reset();;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1272 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { (yyval).reset();;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1281 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1287 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1293 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1297 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1301 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1311 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1314 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1320 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1324 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1329 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1334 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1339 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1344 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1350 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1356 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1364 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1369 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1374 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1378 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1381 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1385 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1389 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1392 "hphp.y"
    { (yyval).reset();;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1397 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1400 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1404 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1408 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1412 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1416 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1421 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1426 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1432 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1433 "hphp.y"
    { (yyval).reset();;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1436 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1437 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1438 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1440 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1442 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1444 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1448 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1449 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1452 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1453 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1454 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1458 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1460 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1461 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1462 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1467 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1468 "hphp.y"
    { (yyval).reset();;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1471 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1476 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1482 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1483 "hphp.y"
    { (yyval).reset();;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1486 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1487 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1490 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1491 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1493 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1498 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1501 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1507 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1514 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1520 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1525 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1527 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1529 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1531 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1533 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1534 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1537 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1540 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1542 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1552 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1555 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1563 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1568 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1578 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1580 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1584 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1586 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1588 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1589 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1595 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1597 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1598 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1604 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1608 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1618 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1621 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1626 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1631 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1632 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1634 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1638 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1639 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1640 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1641 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1645 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1646 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1647 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1649 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1651 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1657 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1660 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1661 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1665 "hphp.y"
    { (yyval).reset();;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1666 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1670 "hphp.y"
    { (yyval).reset();;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1671 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1675 "hphp.y"
    { (yyval).reset();;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1678 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { (yyval).reset();;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1682 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1684 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1687 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1688 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1689 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1690 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1693 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1697 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1698 "hphp.y"
    { (yyval).reset();;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1701 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1702 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1703 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1709 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1710 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1717 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1721 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1723 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1727 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1731 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1751 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { (yyval).reset();;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1761 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1852 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1858 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1864 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1867 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1868 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1869 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1877 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1878 "hphp.y"
    { (yyval).reset();;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(7) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1897 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(8) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
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

  case 459:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1969 "hphp.y"
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

  case 463:

/* Line 1455 of yacc.c  */
#line 1979 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1985 "hphp.y"
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

  case 465:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 2000 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 2002 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 2006 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 2008 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 2015 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 2018 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 2025 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 2028 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 2033 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 2034 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 2039 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 2040 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 2044 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 2048 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 2049 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2054 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2061 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2068 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2070 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2074 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2075 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2076 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2077 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2079 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2083 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2087 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2091 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2096 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2098 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2100 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2102 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2106 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2108 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2112 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2114 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2115 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2116 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2117 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2121 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2125 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2129 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2139 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2147 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2148 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2153 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2162 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2163 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2167 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2171 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2175 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2179 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2181 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2182 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
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

  case 528:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
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

  case 529:

/* Line 1455 of yacc.c  */
#line 2214 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2215 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2221 "hphp.y"
    { (yyval).reset();;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { (yyval).reset();;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2332 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2345 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2349 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2355 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { (yyval).reset();;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { (yyval).reset();;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { (yyval).reset();;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { (yyval).reset();;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2421 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2467 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2471 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2473 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2475 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2476 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { (yyval).reset();;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2488 "hphp.y"
    { (yyval).reset();;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2489 "hphp.y"
    { (yyval).reset();;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2492 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2493 "hphp.y"
    { (yyval).reset();;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2503 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2510 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2513 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2518 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2519 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2520 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2521 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2528 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2535 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2538 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2540 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2541 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2549 "hphp.y"
    { (yyval).reset();;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2558 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2559 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2564 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2569 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2570 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2578 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2583 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2584 "hphp.y"
    { (yyval).reset();;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2588 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2597 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2608 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { (yyval).reset();;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2613 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2618 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2619 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2624 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2629 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2630 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2634 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2635 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2639 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2640 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2644 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2646 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2653 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2657 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2659 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2660 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2661 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2663 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2666 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2669 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2671 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2673 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2674 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2678 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2679 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2680 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2681 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2684 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2688 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2690 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2691 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2695 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2696 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2697 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2701 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2705 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2706 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2712 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2716 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2723 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2727 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2731 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2735 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2737 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2742 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2743 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2744 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2747 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2748 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2751 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2752 "hphp.y"
    { (yyval).reset();;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2756 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2757 "hphp.y"
    { (yyval)++;;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2761 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2762 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2765 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2771 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2772 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2779 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2783 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2784 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2788 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2789 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2791 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2792 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2793 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2794 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2800 "hphp.y"
    { (yyval).reset();;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2804 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2805 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2806 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2807 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2810 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2812 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2813 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2814 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2819 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2820 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2824 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2825 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2826 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2827 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2832 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2833 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2840 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2842 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2843 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2847 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2849 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2850 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2852 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2857 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2859 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2861 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]).num(), (yyvsp[(3) - (3)]));;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2863 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2865 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2866 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2869 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2870 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2871 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2875 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2876 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2877 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2878 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2879 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2880 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2881 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2882 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2883 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2884 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2885 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2889 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2890 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2895 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2897 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2911 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2915 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2922 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2928 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2932 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2938 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2939 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2946 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2952 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2957 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2958 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2959 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2964 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2965 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2970 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2971 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 2974 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (6)]).text()); ;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 2976 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (4)]).text()); ;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 2980 "hphp.y"
    {;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 2981 "hphp.y"
    {;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 2982 "hphp.y"
    {;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 2993 "hphp.y"
    { ;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3005 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3007 "hphp.y"
    {;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3011 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3015 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3018 "hphp.y"
    { Token t; t.reset();
                                          _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                          _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3021 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3028 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3031 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3034 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3035 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3038 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3041 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3044 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3048 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3051 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3054 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3060 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3066 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3074 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3075 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13604 "hphp.tab.cpp"
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
#line 3078 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

