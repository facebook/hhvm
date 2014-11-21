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
     T_VARRAY = 411,
     T_MIARRAY = 412,
     T_MSARRAY = 413,
     T_TYPE = 414,
     T_UNRESOLVED_TYPE = 415,
     T_NEWTYPE = 416,
     T_UNRESOLVED_NEWTYPE = 417,
     T_COMPILER_HALT_OFFSET = 418,
     T_ASYNC = 419,
     T_FROM = 420,
     T_WHERE = 421,
     T_JOIN = 422,
     T_IN = 423,
     T_ON = 424,
     T_EQUALS = 425,
     T_INTO = 426,
     T_LET = 427,
     T_ORDERBY = 428,
     T_ASCENDING = 429,
     T_DESCENDING = 430,
     T_SELECT = 431,
     T_GROUP = 432,
     T_BY = 433,
     T_LAMBDA_OP = 434,
     T_LAMBDA_CP = 435,
     T_UNRESOLVED_OP = 436
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
#line 882 "hphp.tab.cpp"

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
#define YYLAST   17030

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  211
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  279
/* YYNRULES -- Number of rules.  */
#define YYNRULES  944
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1772

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   436

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    52,   209,     2,   206,    51,    35,   210,
     201,   202,    49,    46,     9,    47,    48,    50,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    30,   203,
      40,    14,    41,    29,    55,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    66,     2,   208,    34,     2,   207,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   204,    33,   205,    54,     2,     2,     2,
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
     194,   195,   196,   197,   198,   199,   200
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
     980,   981,   982,   987,   988,   994,   997,   998,  1009,  1010,
    1022,  1026,  1030,  1034,  1039,  1044,  1048,  1054,  1057,  1060,
    1061,  1068,  1074,  1079,  1083,  1085,  1087,  1091,  1096,  1098,
    1100,  1105,  1112,  1114,  1116,  1121,  1123,  1125,  1129,  1132,
    1133,  1136,  1137,  1139,  1143,  1145,  1147,  1149,  1151,  1155,
    1160,  1165,  1170,  1172,  1174,  1177,  1180,  1183,  1187,  1191,
    1193,  1195,  1197,  1199,  1203,  1205,  1209,  1211,  1213,  1215,
    1216,  1218,  1221,  1223,  1225,  1227,  1229,  1231,  1233,  1235,
    1237,  1238,  1240,  1242,  1244,  1248,  1254,  1256,  1260,  1266,
    1271,  1275,  1279,  1282,  1284,  1286,  1290,  1294,  1296,  1298,
    1299,  1301,  1304,  1309,  1313,  1320,  1323,  1327,  1334,  1336,
    1338,  1340,  1342,  1344,  1351,  1355,  1360,  1367,  1371,  1375,
    1379,  1383,  1387,  1391,  1395,  1399,  1403,  1407,  1411,  1415,
    1418,  1421,  1424,  1427,  1431,  1435,  1439,  1443,  1447,  1451,
    1455,  1459,  1463,  1467,  1471,  1475,  1479,  1483,  1487,  1491,
    1495,  1498,  1501,  1504,  1507,  1511,  1515,  1519,  1523,  1527,
    1531,  1535,  1539,  1543,  1547,  1553,  1558,  1560,  1563,  1566,
    1569,  1572,  1575,  1578,  1581,  1584,  1587,  1589,  1591,  1593,
    1595,  1597,  1601,  1604,  1606,  1612,  1613,  1614,  1626,  1627,
    1640,  1641,  1645,  1646,  1651,  1652,  1659,  1660,  1668,  1669,
    1675,  1678,  1681,  1686,  1688,  1690,  1696,  1700,  1706,  1710,
    1713,  1714,  1717,  1718,  1723,  1728,  1732,  1737,  1742,  1747,
    1752,  1757,  1762,  1767,  1772,  1777,  1782,  1784,  1786,  1788,
    1790,  1794,  1797,  1801,  1806,  1809,  1813,  1815,  1818,  1820,
    1823,  1825,  1827,  1829,  1831,  1833,  1835,  1840,  1845,  1848,
    1857,  1868,  1871,  1873,  1877,  1879,  1882,  1884,  1886,  1888,
    1890,  1893,  1898,  1902,  1906,  1911,  1913,  1916,  1921,  1924,
    1931,  1932,  1934,  1939,  1940,  1943,  1944,  1946,  1948,  1952,
    1954,  1958,  1960,  1962,  1966,  1970,  1972,  1974,  1976,  1978,
    1980,  1982,  1984,  1986,  1988,  1990,  1992,  1994,  1996,  1998,
    2000,  2002,  2004,  2006,  2008,  2010,  2012,  2014,  2016,  2018,
    2020,  2022,  2024,  2026,  2028,  2030,  2032,  2034,  2036,  2038,
    2040,  2042,  2044,  2046,  2048,  2050,  2052,  2054,  2056,  2058,
    2060,  2062,  2064,  2066,  2068,  2070,  2072,  2074,  2076,  2078,
    2080,  2082,  2084,  2086,  2088,  2090,  2092,  2094,  2096,  2098,
    2100,  2102,  2104,  2106,  2108,  2110,  2112,  2114,  2116,  2118,
    2120,  2122,  2124,  2126,  2128,  2130,  2135,  2137,  2139,  2141,
    2143,  2145,  2147,  2149,  2151,  2154,  2156,  2157,  2158,  2160,
    2162,  2166,  2167,  2169,  2171,  2173,  2175,  2177,  2179,  2181,
    2183,  2185,  2187,  2189,  2191,  2193,  2197,  2200,  2202,  2204,
    2209,  2213,  2218,  2220,  2222,  2224,  2226,  2230,  2234,  2238,
    2242,  2246,  2250,  2254,  2258,  2262,  2266,  2270,  2274,  2278,
    2282,  2286,  2290,  2294,  2298,  2301,  2304,  2307,  2310,  2314,
    2318,  2322,  2326,  2330,  2334,  2338,  2342,  2348,  2353,  2357,
    2361,  2365,  2367,  2369,  2371,  2373,  2377,  2381,  2385,  2388,
    2389,  2391,  2392,  2394,  2395,  2401,  2405,  2409,  2411,  2413,
    2415,  2417,  2419,  2423,  2426,  2428,  2430,  2432,  2434,  2436,
    2438,  2441,  2444,  2449,  2453,  2458,  2461,  2462,  2468,  2472,
    2476,  2478,  2482,  2484,  2487,  2488,  2494,  2498,  2501,  2502,
    2506,  2507,  2512,  2515,  2516,  2520,  2524,  2526,  2527,  2529,
    2531,  2533,  2535,  2539,  2541,  2543,  2545,  2549,  2551,  2553,
    2557,  2561,  2564,  2569,  2572,  2577,  2579,  2581,  2583,  2585,
    2587,  2591,  2597,  2601,  2606,  2611,  2615,  2617,  2619,  2621,
    2623,  2627,  2633,  2638,  2642,  2644,  2646,  2650,  2654,  2656,
    2658,  2666,  2676,  2684,  2691,  2700,  2702,  2705,  2710,  2715,
    2717,  2719,  2724,  2726,  2727,  2729,  2732,  2734,  2736,  2740,
    2746,  2750,  2754,  2755,  2757,  2761,  2767,  2771,  2774,  2778,
    2785,  2786,  2788,  2793,  2796,  2797,  2803,  2807,  2811,  2813,
    2820,  2825,  2830,  2833,  2836,  2837,  2843,  2847,  2851,  2853,
    2856,  2857,  2863,  2867,  2871,  2873,  2876,  2877,  2880,  2881,
    2887,  2891,  2895,  2897,  2900,  2901,  2904,  2905,  2911,  2915,
    2919,  2921,  2924,  2927,  2929,  2932,  2934,  2939,  2943,  2947,
    2954,  2958,  2960,  2962,  2964,  2969,  2974,  2979,  2984,  2989,
    2994,  2997,  3000,  3005,  3008,  3011,  3013,  3017,  3021,  3025,
    3026,  3029,  3035,  3042,  3044,  3047,  3049,  3054,  3058,  3059,
    3061,  3065,  3068,  3072,  3074,  3076,  3077,  3078,  3081,  3086,
    3089,  3096,  3101,  3103,  3105,  3106,  3110,  3116,  3120,  3122,
    3125,  3126,  3131,  3134,  3137,  3139,  3141,  3143,  3145,  3150,
    3157,  3159,  3168,  3175,  3177
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     212,     0,    -1,    -1,   213,   214,    -1,   214,   215,    -1,
      -1,   233,    -1,   250,    -1,   257,    -1,   254,    -1,   262,
      -1,   474,    -1,   123,   201,   202,   203,    -1,   150,   225,
     203,    -1,    -1,   150,   225,   204,   216,   214,   205,    -1,
      -1,   150,   204,   217,   214,   205,    -1,   111,   219,   203,
      -1,   111,   105,   220,   203,    -1,   111,   106,   221,   203,
      -1,   230,   203,    -1,    77,    -1,   156,    -1,   157,    -1,
     159,    -1,   161,    -1,   160,    -1,   185,    -1,   186,    -1,
     188,    -1,   187,    -1,   189,    -1,   190,    -1,   191,    -1,
     192,    -1,   193,    -1,   194,    -1,   195,    -1,   196,    -1,
     197,    -1,   219,     9,   222,    -1,   222,    -1,   223,     9,
     223,    -1,   223,    -1,   224,     9,   224,    -1,   224,    -1,
     225,    -1,   153,   225,    -1,   225,    97,   218,    -1,   153,
     225,    97,   218,    -1,   225,    -1,   153,   225,    -1,   225,
      97,   218,    -1,   153,   225,    97,   218,    -1,   225,    -1,
     153,   225,    -1,   225,    97,   218,    -1,   153,   225,    97,
     218,    -1,   218,    -1,   225,   153,   218,    -1,   225,    -1,
     150,   153,   225,    -1,   153,   225,    -1,   226,    -1,   226,
     477,    -1,   226,   477,    -1,   230,     9,   475,    14,   408,
      -1,   106,   475,    14,   408,    -1,   231,   232,    -1,    -1,
     233,    -1,   250,    -1,   257,    -1,   262,    -1,   204,   231,
     205,    -1,    70,   333,   233,   284,   286,    -1,    70,   333,
      30,   231,   285,   287,    73,   203,    -1,    -1,    89,   333,
     234,   278,    -1,    -1,    88,   235,   233,    89,   333,   203,
      -1,    -1,    91,   201,   335,   203,   335,   203,   335,   202,
     236,   276,    -1,    -1,    98,   333,   237,   281,    -1,   102,
     203,    -1,   102,   342,   203,    -1,   104,   203,    -1,   104,
     342,   203,    -1,   107,   203,    -1,   107,   342,   203,    -1,
      27,   102,   203,    -1,   112,   294,   203,    -1,   118,   296,
     203,    -1,    87,   334,   203,    -1,   120,   201,   471,   202,
     203,    -1,   203,    -1,    81,    -1,    82,    -1,    -1,    93,
     201,   342,    97,   275,   274,   202,   238,   277,    -1,    -1,
      93,   201,   342,    28,    97,   275,   274,   202,   239,   277,
      -1,    95,   201,   280,   202,   279,    -1,    -1,   108,   242,
     109,   201,   401,    79,   202,   204,   231,   205,   244,   240,
     247,    -1,    -1,   108,   242,   167,   241,   245,    -1,   110,
     342,   203,    -1,   103,   218,   203,    -1,   342,   203,    -1,
     336,   203,    -1,   337,   203,    -1,   338,   203,    -1,   339,
     203,    -1,   340,   203,    -1,   107,   339,   203,    -1,   341,
     203,    -1,   371,   203,    -1,   107,   370,   203,    -1,   218,
      30,    -1,    -1,   204,   243,   231,   205,    -1,   244,   109,
     201,   401,    79,   202,   204,   231,   205,    -1,    -1,    -1,
     204,   246,   231,   205,    -1,   167,   245,    -1,    -1,    35,
      -1,    -1,   105,    -1,    -1,   249,   248,   476,   251,   201,
     290,   202,   481,   322,    -1,    -1,   326,   249,   248,   476,
     252,   201,   290,   202,   481,   322,    -1,    -1,   428,   325,
     249,   248,   476,   253,   201,   290,   202,   481,   322,    -1,
      -1,   160,   218,   255,    30,   488,   473,   204,   297,   205,
      -1,    -1,   428,   160,   218,   256,    30,   488,   473,   204,
     297,   205,    -1,    -1,   268,   265,   258,   269,   270,   204,
     300,   205,    -1,    -1,   428,   268,   265,   259,   269,   270,
     204,   300,   205,    -1,    -1,   125,   266,   260,   271,   204,
     300,   205,    -1,    -1,   428,   125,   266,   261,   271,   204,
     300,   205,    -1,    -1,   162,   267,   263,   270,   204,   300,
     205,    -1,    -1,   428,   162,   267,   264,   270,   204,   300,
     205,    -1,   476,    -1,   154,    -1,   476,    -1,   476,    -1,
     124,    -1,   117,   124,    -1,   117,   116,   124,    -1,   116,
     117,   124,    -1,   116,   124,    -1,   126,   401,    -1,    -1,
     127,   272,    -1,    -1,   126,   272,    -1,    -1,   401,    -1,
     272,     9,   401,    -1,   401,    -1,   273,     9,   401,    -1,
     130,   275,    -1,    -1,   438,    -1,    35,   438,    -1,   131,
     201,   452,   202,    -1,   233,    -1,    30,   231,    92,   203,
      -1,   233,    -1,    30,   231,    94,   203,    -1,   233,    -1,
      30,   231,    90,   203,    -1,   233,    -1,    30,   231,    96,
     203,    -1,   218,    14,   408,    -1,   280,     9,   218,    14,
     408,    -1,   204,   282,   205,    -1,   204,   203,   282,   205,
      -1,    30,   282,    99,   203,    -1,    30,   203,   282,    99,
     203,    -1,   282,   100,   342,   283,   231,    -1,   282,   101,
     283,   231,    -1,    -1,    30,    -1,   203,    -1,   284,    71,
     333,   233,    -1,    -1,   285,    71,   333,    30,   231,    -1,
      -1,    72,   233,    -1,    -1,    72,    30,   231,    -1,    -1,
     289,     9,   429,   328,   489,   163,    79,    -1,   289,     9,
     429,   328,   489,    35,   163,    79,    -1,   289,     9,   429,
     328,   489,   163,    -1,   289,   413,    -1,   429,   328,   489,
     163,    79,    -1,   429,   328,   489,    35,   163,    79,    -1,
     429,   328,   489,   163,    -1,    -1,   429,   328,   489,    79,
      -1,   429,   328,   489,    35,    79,    -1,   429,   328,   489,
      35,    79,    14,   342,    -1,   429,   328,   489,    79,    14,
     342,    -1,   289,     9,   429,   328,   489,    79,    -1,   289,
       9,   429,   328,   489,    35,    79,    -1,   289,     9,   429,
     328,   489,    35,    79,    14,   342,    -1,   289,     9,   429,
     328,   489,    79,    14,   342,    -1,   291,     9,   429,   489,
     163,    79,    -1,   291,     9,   429,   489,    35,   163,    79,
      -1,   291,     9,   429,   489,   163,    -1,   291,   413,    -1,
     429,   489,   163,    79,    -1,   429,   489,    35,   163,    79,
      -1,   429,   489,   163,    -1,    -1,   429,   489,    79,    -1,
     429,   489,    35,    79,    -1,   429,   489,    35,    79,    14,
     342,    -1,   429,   489,    79,    14,   342,    -1,   291,     9,
     429,   489,    79,    -1,   291,     9,   429,   489,    35,    79,
      -1,   291,     9,   429,   489,    35,    79,    14,   342,    -1,
     291,     9,   429,   489,    79,    14,   342,    -1,   293,   413,
      -1,    -1,   342,    -1,    35,   438,    -1,   163,   342,    -1,
     293,     9,   342,    -1,   293,     9,   163,   342,    -1,   293,
       9,    35,   438,    -1,   294,     9,   295,    -1,   295,    -1,
      79,    -1,   206,   438,    -1,   206,   204,   342,   205,    -1,
     296,     9,    79,    -1,   296,     9,    79,    14,   408,    -1,
      79,    -1,    79,    14,   408,    -1,   297,   298,    -1,    -1,
     299,   203,    -1,   475,    14,   408,    -1,   300,   301,    -1,
      -1,    -1,   324,   302,   330,   203,    -1,    -1,   326,   488,
     303,   330,   203,    -1,   331,   203,    -1,    -1,   325,   249,
     248,   476,   201,   304,   288,   202,   481,   323,    -1,    -1,
     428,   325,   249,   248,   476,   201,   305,   288,   202,   481,
     323,    -1,   156,   310,   203,    -1,   157,   316,   203,    -1,
     159,   318,   203,    -1,     4,   126,   401,   203,    -1,     4,
     127,   401,   203,    -1,   111,   273,   203,    -1,   111,   273,
     204,   306,   205,    -1,   306,   307,    -1,   306,   308,    -1,
      -1,   229,   149,   218,   164,   273,   203,    -1,   309,    97,
     325,   218,   203,    -1,   309,    97,   326,   203,    -1,   229,
     149,   218,    -1,   218,    -1,   311,    -1,   310,     9,   311,
      -1,   312,   398,   314,   315,    -1,   154,    -1,   132,    -1,
     132,   170,   488,   171,    -1,   132,   170,   488,     9,   488,
     171,    -1,   401,    -1,   119,    -1,   160,   204,   313,   205,
      -1,   133,    -1,   407,    -1,   313,     9,   407,    -1,    14,
     408,    -1,    -1,    55,   161,    -1,    -1,   317,    -1,   316,
       9,   317,    -1,   158,    -1,   319,    -1,   218,    -1,   122,
      -1,   201,   320,   202,    -1,   201,   320,   202,    49,    -1,
     201,   320,   202,    29,    -1,   201,   320,   202,    46,    -1,
     319,    -1,   321,    -1,   321,    49,    -1,   321,    29,    -1,
     321,    46,    -1,   320,     9,   320,    -1,   320,    33,   320,
      -1,   218,    -1,   154,    -1,   158,    -1,   203,    -1,   204,
     231,   205,    -1,   203,    -1,   204,   231,   205,    -1,   326,
      -1,   119,    -1,   326,    -1,    -1,   327,    -1,   326,   327,
      -1,   113,    -1,   114,    -1,   115,    -1,   118,    -1,   117,
      -1,   116,    -1,   183,    -1,   329,    -1,    -1,   113,    -1,
     114,    -1,   115,    -1,   330,     9,    79,    -1,   330,     9,
      79,    14,   408,    -1,    79,    -1,    79,    14,   408,    -1,
     331,     9,   475,    14,   408,    -1,   106,   475,    14,   408,
      -1,   201,   332,   202,    -1,    68,   403,   406,    -1,    67,
     342,    -1,   390,    -1,   362,    -1,   201,   342,   202,    -1,
     334,     9,   342,    -1,   342,    -1,   334,    -1,    -1,    27,
      -1,    27,   342,    -1,    27,   342,   130,   342,    -1,   438,
      14,   336,    -1,   131,   201,   452,   202,    14,   336,    -1,
      28,   342,    -1,   438,    14,   339,    -1,   131,   201,   452,
     202,    14,   339,    -1,   343,    -1,   438,    -1,   332,    -1,
     442,    -1,   441,    -1,   131,   201,   452,   202,    14,   342,
      -1,   438,    14,   342,    -1,   438,    14,    35,   438,    -1,
     438,    14,    35,    68,   403,   406,    -1,   438,    26,   342,
      -1,   438,    25,   342,    -1,   438,    24,   342,    -1,   438,
      23,   342,    -1,   438,    22,   342,    -1,   438,    21,   342,
      -1,   438,    20,   342,    -1,   438,    19,   342,    -1,   438,
      18,   342,    -1,   438,    17,   342,    -1,   438,    16,   342,
      -1,   438,    15,   342,    -1,   438,    64,    -1,    64,   438,
      -1,   438,    63,    -1,    63,   438,    -1,   342,    31,   342,
      -1,   342,    32,   342,    -1,   342,    10,   342,    -1,   342,
      12,   342,    -1,   342,    11,   342,    -1,   342,    33,   342,
      -1,   342,    35,   342,    -1,   342,    34,   342,    -1,   342,
      48,   342,    -1,   342,    46,   342,    -1,   342,    47,   342,
      -1,   342,    49,   342,    -1,   342,    50,   342,    -1,   342,
      65,   342,    -1,   342,    51,   342,    -1,   342,    45,   342,
      -1,   342,    44,   342,    -1,    46,   342,    -1,    47,   342,
      -1,    52,   342,    -1,    54,   342,    -1,   342,    37,   342,
      -1,   342,    36,   342,    -1,   342,    39,   342,    -1,   342,
      38,   342,    -1,   342,    40,   342,    -1,   342,    43,   342,
      -1,   342,    41,   342,    -1,   342,    42,   342,    -1,   342,
      53,   403,    -1,   201,   343,   202,    -1,   342,    29,   342,
      30,   342,    -1,   342,    29,    30,   342,    -1,   470,    -1,
      62,   342,    -1,    61,   342,    -1,    60,   342,    -1,    59,
     342,    -1,    58,   342,    -1,    57,   342,    -1,    56,   342,
      -1,    69,   404,    -1,    55,   342,    -1,   410,    -1,   361,
      -1,   360,    -1,   363,    -1,   364,    -1,   207,   405,   207,
      -1,    13,   342,    -1,   368,    -1,   111,   201,   389,   413,
     202,    -1,    -1,    -1,   249,   248,   201,   346,   290,   202,
     481,   344,   204,   231,   205,    -1,    -1,   326,   249,   248,
     201,   347,   290,   202,   481,   344,   204,   231,   205,    -1,
      -1,    79,   349,   354,    -1,    -1,   183,    79,   350,   354,
      -1,    -1,   198,   351,   290,   199,   481,   354,    -1,    -1,
     183,   198,   352,   290,   199,   481,   354,    -1,    -1,   183,
     204,   353,   231,   205,    -1,     8,   342,    -1,     8,   339,
      -1,     8,   204,   231,   205,    -1,    86,    -1,   472,    -1,
     356,     9,   355,   130,   342,    -1,   355,   130,   342,    -1,
     357,     9,   355,   130,   408,    -1,   355,   130,   408,    -1,
     356,   412,    -1,    -1,   357,   412,    -1,    -1,   174,   201,
     358,   202,    -1,   132,   201,   453,   202,    -1,    66,   453,
     208,    -1,   401,   204,   455,   205,    -1,   176,   201,   459,
     202,    -1,   177,   201,   459,   202,    -1,   175,   201,   460,
     202,    -1,   176,   201,   463,   202,    -1,   177,   201,   463,
     202,    -1,   175,   201,   464,   202,    -1,   401,   204,   457,
     205,    -1,   368,    66,   448,   208,    -1,   369,    66,   448,
     208,    -1,   361,    -1,   472,    -1,   441,    -1,    86,    -1,
     201,   343,   202,    -1,   372,   373,    -1,   438,    14,   370,
      -1,   184,    79,   187,   342,    -1,   374,   385,    -1,   374,
     385,   388,    -1,   385,    -1,   385,   388,    -1,   375,    -1,
     374,   375,    -1,   376,    -1,   377,    -1,   378,    -1,   379,
      -1,   380,    -1,   381,    -1,   184,    79,   187,   342,    -1,
     191,    79,    14,   342,    -1,   185,   342,    -1,   186,    79,
     187,   342,   188,   342,   189,   342,    -1,   186,    79,   187,
     342,   188,   342,   189,   342,   190,    79,    -1,   192,   382,
      -1,   383,    -1,   382,     9,   383,    -1,   342,    -1,   342,
     384,    -1,   193,    -1,   194,    -1,   386,    -1,   387,    -1,
     195,   342,    -1,   196,   342,   197,   342,    -1,   190,    79,
     373,    -1,   389,     9,    79,    -1,   389,     9,    35,    79,
      -1,    79,    -1,    35,    79,    -1,   168,   154,   391,   169,
      -1,   393,    50,    -1,   393,   169,   394,   168,    50,   392,
      -1,    -1,   154,    -1,   393,   395,    14,   396,    -1,    -1,
     394,   397,    -1,    -1,   154,    -1,   155,    -1,   204,   342,
     205,    -1,   155,    -1,   204,   342,   205,    -1,   390,    -1,
     399,    -1,   398,    30,   399,    -1,   398,    47,   399,    -1,
     218,    -1,    69,    -1,   105,    -1,   106,    -1,   107,    -1,
      27,    -1,    28,    -1,   108,    -1,   109,    -1,   167,    -1,
     110,    -1,    70,    -1,    71,    -1,    73,    -1,    72,    -1,
      89,    -1,    90,    -1,    88,    -1,    91,    -1,    92,    -1,
      93,    -1,    94,    -1,    95,    -1,    96,    -1,    53,    -1,
      97,    -1,    98,    -1,    99,    -1,   100,    -1,   101,    -1,
     102,    -1,   104,    -1,   103,    -1,    87,    -1,    13,    -1,
     124,    -1,   125,    -1,   126,    -1,   127,    -1,    68,    -1,
      67,    -1,   119,    -1,     5,    -1,     7,    -1,     6,    -1,
       4,    -1,     3,    -1,   150,    -1,   111,    -1,   112,    -1,
     121,    -1,   122,    -1,   123,    -1,   118,    -1,   117,    -1,
     116,    -1,   115,    -1,   114,    -1,   113,    -1,   183,    -1,
     120,    -1,   131,    -1,   132,    -1,    10,    -1,    12,    -1,
      11,    -1,   134,    -1,   136,    -1,   135,    -1,   137,    -1,
     138,    -1,   152,    -1,   151,    -1,   182,    -1,   162,    -1,
     165,    -1,   164,    -1,   178,    -1,   180,    -1,   174,    -1,
     228,   201,   292,   202,    -1,   229,    -1,   154,    -1,   401,
      -1,   118,    -1,   446,    -1,   401,    -1,   118,    -1,   450,
      -1,   201,   202,    -1,   333,    -1,    -1,    -1,    85,    -1,
     467,    -1,   201,   292,   202,    -1,    -1,    74,    -1,    75,
      -1,    76,    -1,    86,    -1,   137,    -1,   138,    -1,   152,
      -1,   134,    -1,   165,    -1,   135,    -1,   136,    -1,   151,
      -1,   182,    -1,   145,    85,   146,    -1,   145,   146,    -1,
     407,    -1,   227,    -1,   132,   201,   411,   202,    -1,    66,
     411,   208,    -1,   174,   201,   359,   202,    -1,   409,    -1,
     367,    -1,   365,    -1,   366,    -1,   201,   408,   202,    -1,
     408,    31,   408,    -1,   408,    32,   408,    -1,   408,    10,
     408,    -1,   408,    12,   408,    -1,   408,    11,   408,    -1,
     408,    33,   408,    -1,   408,    35,   408,    -1,   408,    34,
     408,    -1,   408,    48,   408,    -1,   408,    46,   408,    -1,
     408,    47,   408,    -1,   408,    49,   408,    -1,   408,    50,
     408,    -1,   408,    51,   408,    -1,   408,    45,   408,    -1,
     408,    44,   408,    -1,   408,    65,   408,    -1,    52,   408,
      -1,    54,   408,    -1,    46,   408,    -1,    47,   408,    -1,
     408,    37,   408,    -1,   408,    36,   408,    -1,   408,    39,
     408,    -1,   408,    38,   408,    -1,   408,    40,   408,    -1,
     408,    43,   408,    -1,   408,    41,   408,    -1,   408,    42,
     408,    -1,   408,    29,   408,    30,   408,    -1,   408,    29,
      30,   408,    -1,   229,   149,   218,    -1,   154,   149,   218,
      -1,   229,   149,   124,    -1,   227,    -1,    78,    -1,   472,
      -1,   407,    -1,   209,   467,   209,    -1,   210,   467,   210,
      -1,   145,   467,   146,    -1,   414,   412,    -1,    -1,     9,
      -1,    -1,     9,    -1,    -1,   414,     9,   408,   130,   408,
      -1,   414,     9,   408,    -1,   408,   130,   408,    -1,   408,
      -1,    74,    -1,    75,    -1,    76,    -1,    86,    -1,   145,
      85,   146,    -1,   145,   146,    -1,    74,    -1,    75,    -1,
      76,    -1,   218,    -1,   415,    -1,   218,    -1,    46,   416,
      -1,    47,   416,    -1,   132,   201,   418,   202,    -1,    66,
     418,   208,    -1,   174,   201,   421,   202,    -1,   419,   412,
      -1,    -1,   419,     9,   417,   130,   417,    -1,   419,     9,
     417,    -1,   417,   130,   417,    -1,   417,    -1,   420,     9,
     417,    -1,   417,    -1,   422,   412,    -1,    -1,   422,     9,
     355,   130,   417,    -1,   355,   130,   417,    -1,   420,   412,
      -1,    -1,   201,   423,   202,    -1,    -1,   425,     9,   218,
     424,    -1,   218,   424,    -1,    -1,   427,   425,   412,    -1,
      45,   426,    44,    -1,   428,    -1,    -1,   128,    -1,   129,
      -1,   218,    -1,   154,    -1,   204,   342,   205,    -1,   431,
      -1,   445,    -1,   218,    -1,   204,   342,   205,    -1,   433,
      -1,   445,    -1,    66,   448,   208,    -1,   204,   342,   205,
      -1,   439,   435,    -1,   201,   332,   202,   435,    -1,   451,
     435,    -1,   201,   332,   202,   435,    -1,   445,    -1,   400,
      -1,   443,    -1,   444,    -1,   436,    -1,   438,   430,   432,
      -1,   201,   332,   202,   430,   432,    -1,   402,   149,   445,
      -1,   440,   201,   292,   202,    -1,   441,   201,   292,   202,
      -1,   201,   438,   202,    -1,   400,    -1,   443,    -1,   444,
      -1,   436,    -1,   438,   430,   431,    -1,   201,   332,   202,
     430,   431,    -1,   440,   201,   292,   202,    -1,   201,   438,
     202,    -1,   445,    -1,   436,    -1,   201,   438,   202,    -1,
     201,   442,   202,    -1,   345,    -1,   348,    -1,   438,   430,
     434,   477,   201,   292,   202,    -1,   201,   332,   202,   430,
     434,   477,   201,   292,   202,    -1,   402,   149,   218,   477,
     201,   292,   202,    -1,   402,   149,   445,   201,   292,   202,
      -1,   402,   149,   204,   342,   205,   201,   292,   202,    -1,
     446,    -1,   449,   446,    -1,   446,    66,   448,   208,    -1,
     446,   204,   342,   205,    -1,   447,    -1,    79,    -1,   206,
     204,   342,   205,    -1,   342,    -1,    -1,   206,    -1,   449,
     206,    -1,   445,    -1,   437,    -1,   450,   430,   432,    -1,
     201,   332,   202,   430,   432,    -1,   402,   149,   445,    -1,
     201,   438,   202,    -1,    -1,   437,    -1,   450,   430,   431,
      -1,   201,   332,   202,   430,   431,    -1,   201,   438,   202,
      -1,   452,     9,    -1,   452,     9,   438,    -1,   452,     9,
     131,   201,   452,   202,    -1,    -1,   438,    -1,   131,   201,
     452,   202,    -1,   454,   412,    -1,    -1,   454,     9,   342,
     130,   342,    -1,   454,     9,   342,    -1,   342,   130,   342,
      -1,   342,    -1,   454,     9,   342,   130,    35,   438,    -1,
     454,     9,    35,   438,    -1,   342,   130,    35,   438,    -1,
      35,   438,    -1,   456,   412,    -1,    -1,   456,     9,   342,
     130,   342,    -1,   456,     9,   342,    -1,   342,   130,   342,
      -1,   342,    -1,   458,   412,    -1,    -1,   458,     9,   408,
     130,   408,    -1,   458,     9,   408,    -1,   408,   130,   408,
      -1,   408,    -1,   461,   412,    -1,    -1,   462,   412,    -1,
      -1,   461,     9,   342,   130,   342,    -1,   342,   130,   342,
      -1,   462,     9,   342,    -1,   342,    -1,   465,   412,    -1,
      -1,   466,   412,    -1,    -1,   465,     9,   408,   130,   408,
      -1,   408,   130,   408,    -1,   466,     9,   408,    -1,   408,
      -1,   467,   468,    -1,   467,    85,    -1,   468,    -1,    85,
     468,    -1,    79,    -1,    79,    66,   469,   208,    -1,    79,
     430,   218,    -1,   147,   342,   205,    -1,   147,    78,    66,
     342,   208,   205,    -1,   148,   438,   205,    -1,   218,    -1,
      80,    -1,    79,    -1,   121,   201,   471,   202,    -1,   122,
     201,   438,   202,    -1,   122,   201,   343,   202,    -1,   122,
     201,   442,   202,    -1,   122,   201,   441,   202,    -1,   122,
     201,   332,   202,    -1,     7,   342,    -1,     6,   342,    -1,
       5,   201,   342,   202,    -1,     4,   342,    -1,     3,   342,
      -1,   438,    -1,   471,     9,   438,    -1,   402,   149,   218,
      -1,   402,   149,   124,    -1,    -1,    97,   488,    -1,   178,
     476,    14,   488,   203,    -1,   180,   476,   473,    14,   488,
     203,    -1,   218,    -1,   488,   218,    -1,   218,    -1,   218,
     170,   482,   171,    -1,   170,   479,   171,    -1,    -1,   488,
      -1,   478,     9,   488,    -1,   478,   412,    -1,   478,     9,
     163,    -1,   479,    -1,   163,    -1,    -1,    -1,    30,   488,
      -1,   482,     9,   483,   218,    -1,   483,   218,    -1,   482,
       9,   483,   218,    97,   488,    -1,   483,   218,    97,   488,
      -1,    46,    -1,    47,    -1,    -1,    86,   130,   488,    -1,
     229,   149,   218,   130,   488,    -1,   485,     9,   484,    -1,
     484,    -1,   485,   412,    -1,    -1,   174,   201,   486,   202,
      -1,    29,   488,    -1,    55,   488,    -1,   229,    -1,   132,
      -1,   133,    -1,   487,    -1,   132,   170,   488,   171,    -1,
     132,   170,   488,     9,   488,   171,    -1,   154,    -1,   201,
     105,   201,   480,   202,    30,   488,   202,    -1,   201,   488,
       9,   478,   412,   202,    -1,   488,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   737,   737,   737,   746,   748,   751,   752,   753,   754,
     755,   756,   757,   760,   762,   762,   764,   764,   766,   767,
     769,   771,   776,   777,   778,   779,   780,   781,   782,   783,
     784,   785,   786,   787,   788,   789,   790,   791,   792,   793,
     794,   798,   800,   804,   806,   810,   812,   816,   817,   818,
     819,   824,   825,   826,   827,   832,   833,   834,   835,   840,
     841,   845,   846,   848,   851,   857,   864,   871,   875,   881,
     883,   886,   887,   888,   889,   892,   893,   897,   902,   902,
     908,   908,   915,   914,   920,   920,   925,   926,   927,   928,
     929,   930,   931,   932,   933,   934,   935,   936,   937,   938,
     942,   940,   949,   947,   954,   962,   956,   966,   964,   968,
     969,   973,   974,   975,   976,   977,   978,   979,   980,   981,
     982,   983,   991,   991,   996,  1002,  1006,  1006,  1014,  1015,
    1019,  1020,  1024,  1029,  1028,  1041,  1039,  1053,  1051,  1067,
    1066,  1075,  1073,  1085,  1084,  1103,  1101,  1120,  1119,  1128,
    1126,  1138,  1137,  1149,  1147,  1160,  1161,  1165,  1168,  1171,
    1172,  1173,  1176,  1177,  1180,  1182,  1185,  1186,  1189,  1190,
    1193,  1194,  1198,  1199,  1204,  1205,  1208,  1209,  1210,  1214,
    1215,  1219,  1220,  1224,  1225,  1229,  1230,  1235,  1236,  1241,
    1242,  1243,  1244,  1247,  1250,  1252,  1255,  1256,  1260,  1262,
    1265,  1268,  1271,  1272,  1275,  1276,  1280,  1286,  1292,  1299,
    1301,  1306,  1311,  1317,  1321,  1325,  1329,  1334,  1339,  1344,
    1349,  1355,  1364,  1369,  1374,  1380,  1382,  1386,  1390,  1395,
    1399,  1402,  1405,  1409,  1413,  1417,  1421,  1426,  1434,  1436,
    1439,  1440,  1441,  1442,  1444,  1446,  1451,  1452,  1455,  1456,
    1457,  1461,  1462,  1464,  1465,  1469,  1471,  1474,  1478,  1484,
    1486,  1489,  1489,  1493,  1492,  1496,  1500,  1498,  1513,  1510,
    1523,  1525,  1527,  1529,  1531,  1533,  1535,  1539,  1540,  1541,
    1544,  1550,  1553,  1559,  1562,  1567,  1569,  1574,  1579,  1583,
    1584,  1586,  1588,  1594,  1595,  1597,  1601,  1602,  1607,  1608,
    1612,  1613,  1617,  1619,  1625,  1630,  1631,  1633,  1637,  1638,
    1639,  1640,  1644,  1645,  1646,  1647,  1648,  1649,  1651,  1656,
    1659,  1660,  1664,  1665,  1669,  1670,  1673,  1674,  1677,  1678,
    1681,  1682,  1686,  1687,  1688,  1689,  1690,  1691,  1692,  1696,
    1697,  1700,  1701,  1702,  1705,  1707,  1709,  1710,  1713,  1715,
    1720,  1721,  1723,  1724,  1725,  1728,  1732,  1733,  1737,  1738,
    1742,  1743,  1744,  1748,  1752,  1757,  1761,  1765,  1770,  1771,
    1772,  1773,  1774,  1778,  1780,  1781,  1782,  1785,  1786,  1787,
    1788,  1789,  1790,  1791,  1792,  1793,  1794,  1795,  1796,  1797,
    1798,  1799,  1800,  1801,  1802,  1803,  1804,  1805,  1806,  1807,
    1808,  1809,  1810,  1811,  1812,  1813,  1814,  1815,  1816,  1817,
    1818,  1819,  1820,  1821,  1822,  1823,  1824,  1825,  1826,  1827,
    1829,  1830,  1832,  1834,  1835,  1836,  1837,  1838,  1839,  1840,
    1841,  1842,  1843,  1844,  1845,  1846,  1847,  1848,  1849,  1850,
    1851,  1852,  1853,  1854,  1858,  1862,  1867,  1866,  1881,  1879,
    1896,  1896,  1912,  1911,  1929,  1929,  1945,  1944,  1963,  1962,
    1983,  1984,  1985,  1990,  1992,  1996,  2000,  2006,  2010,  2016,
    2018,  2022,  2024,  2028,  2032,  2033,  2037,  2044,  2045,  2049,
    2053,  2055,  2060,  2065,  2072,  2074,  2079,  2080,  2081,  2082,
    2084,  2088,  2092,  2096,  2100,  2102,  2104,  2106,  2111,  2112,
    2117,  2118,  2119,  2120,  2121,  2122,  2126,  2130,  2134,  2138,
    2143,  2148,  2152,  2153,  2157,  2158,  2162,  2163,  2167,  2168,
    2172,  2176,  2180,  2184,  2185,  2186,  2187,  2191,  2197,  2206,
    2219,  2220,  2223,  2226,  2229,  2230,  2233,  2237,  2240,  2243,
    2250,  2251,  2255,  2256,  2258,  2262,  2263,  2264,  2265,  2266,
    2267,  2268,  2269,  2270,  2271,  2272,  2273,  2274,  2275,  2276,
    2277,  2278,  2279,  2280,  2281,  2282,  2283,  2284,  2285,  2286,
    2287,  2288,  2289,  2290,  2291,  2292,  2293,  2294,  2295,  2296,
    2297,  2298,  2299,  2300,  2301,  2302,  2303,  2304,  2305,  2306,
    2307,  2308,  2309,  2310,  2311,  2312,  2313,  2314,  2315,  2316,
    2317,  2318,  2319,  2320,  2321,  2322,  2323,  2324,  2325,  2326,
    2327,  2328,  2329,  2330,  2331,  2332,  2333,  2334,  2335,  2336,
    2337,  2338,  2339,  2340,  2341,  2345,  2350,  2351,  2354,  2355,
    2356,  2360,  2361,  2362,  2366,  2367,  2368,  2372,  2373,  2374,
    2377,  2379,  2383,  2384,  2385,  2386,  2388,  2389,  2390,  2391,
    2392,  2393,  2394,  2395,  2396,  2397,  2400,  2405,  2406,  2407,
    2409,  2410,  2412,  2413,  2414,  2415,  2416,  2417,  2419,  2421,
    2423,  2425,  2427,  2428,  2429,  2430,  2431,  2432,  2433,  2434,
    2435,  2436,  2437,  2438,  2439,  2440,  2441,  2442,  2443,  2445,
    2447,  2449,  2451,  2452,  2455,  2456,  2460,  2462,  2466,  2469,
    2472,  2478,  2479,  2480,  2481,  2482,  2483,  2484,  2489,  2491,
    2495,  2496,  2499,  2500,  2504,  2507,  2509,  2511,  2515,  2516,
    2517,  2518,  2520,  2523,  2527,  2528,  2529,  2530,  2533,  2534,
    2535,  2536,  2537,  2539,  2540,  2545,  2547,  2550,  2553,  2555,
    2557,  2560,  2562,  2566,  2568,  2571,  2574,  2580,  2582,  2585,
    2586,  2591,  2594,  2598,  2598,  2603,  2606,  2607,  2611,  2612,
    2616,  2617,  2618,  2622,  2623,  2627,  2628,  2632,  2633,  2637,
    2638,  2642,  2643,  2648,  2650,  2655,  2656,  2657,  2658,  2659,
    2660,  2662,  2665,  2668,  2670,  2672,  2676,  2677,  2678,  2679,
    2680,  2683,  2687,  2689,  2693,  2694,  2695,  2699,  2703,  2704,
    2708,  2711,  2718,  2722,  2726,  2733,  2734,  2739,  2741,  2742,
    2745,  2746,  2749,  2750,  2754,  2755,  2759,  2760,  2761,  2764,
    2767,  2770,  2773,  2774,  2775,  2778,  2782,  2786,  2787,  2788,
    2790,  2791,  2792,  2796,  2798,  2801,  2803,  2804,  2805,  2806,
    2809,  2811,  2812,  2816,  2818,  2821,  2823,  2824,  2825,  2829,
    2831,  2834,  2837,  2839,  2841,  2845,  2847,  2850,  2852,  2855,
    2857,  2860,  2861,  2865,  2867,  2870,  2872,  2875,  2878,  2882,
    2884,  2888,  2889,  2891,  2892,  2898,  2899,  2901,  2903,  2905,
    2907,  2910,  2911,  2912,  2916,  2917,  2918,  2919,  2920,  2921,
    2922,  2923,  2924,  2925,  2926,  2930,  2931,  2935,  2937,  2945,
    2947,  2951,  2955,  2962,  2963,  2969,  2970,  2977,  2980,  2984,
    2987,  2992,  2997,  2999,  3000,  3001,  3005,  3006,  3010,  3012,
    3013,  3016,  3021,  3022,  3023,  3027,  3030,  3039,  3041,  3045,
    3048,  3051,  3059,  3062,  3065,  3066,  3069,  3072,  3073,  3076,
    3080,  3084,  3090,  3100,  3101
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
  "T_TYPELIST_GT", "T_UNRESOLVED_LT", "T_COLLECTION", "T_SHAPE",
  "T_VARRAY", "T_MIARRAY", "T_MSARRAY", "T_TYPE", "T_UNRESOLVED_TYPE",
  "T_NEWTYPE", "T_UNRESOLVED_NEWTYPE", "T_COMPILER_HALT_OFFSET", "T_ASYNC",
  "T_FROM", "T_WHERE", "T_JOIN", "T_IN", "T_ON", "T_EQUALS", "T_INTO",
  "T_LET", "T_ORDERBY", "T_ASCENDING", "T_DESCENDING", "T_SELECT",
  "T_GROUP", "T_BY", "T_LAMBDA_OP", "T_LAMBDA_CP", "T_UNRESOLVED_OP",
  "'('", "')'", "';'", "'{'", "'}'", "'$'", "'`'", "']'", "'\"'", "'\\''",
  "$accept", "start", "$@1", "top_statement_list", "top_statement", "$@2",
  "$@3", "ident", "use_declarations", "use_fn_declarations",
  "use_const_declarations", "use_declaration", "use_fn_declaration",
  "use_const_declaration", "namespace_name", "namespace_string_base",
  "namespace_string", "namespace_string_typeargs",
  "class_namespace_string_typeargs", "constant_declaration",
  "inner_statement_list", "inner_statement", "statement", "$@4", "$@5",
  "$@6", "$@7", "$@8", "$@9", "$@10", "$@11", "try_statement_list", "$@12",
  "additional_catches", "finally_statement_list", "$@13",
  "optional_finally", "is_reference", "function_loc",
  "function_declaration_statement", "$@14", "$@15", "$@16",
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
  "expr_with_parens", "parenthesis_expr", "expr_list", "for_expr",
  "yield_expr", "yield_assign_expr", "yield_list_assign_expr",
  "await_expr", "await_assign_expr", "await_list_assign_expr", "expr",
  "expr_no_variable", "lambda_use_vars", "closure_expression", "$@29",
  "$@30", "lambda_expression", "$@31", "$@32", "$@33", "$@34", "$@35",
  "lambda_body", "shape_keyname", "non_empty_shape_pair_list",
  "non_empty_static_shape_pair_list", "shape_pair_list",
  "static_shape_pair_list", "shape_literal", "array_literal",
  "collection_literal", "map_array_literal", "varray_literal",
  "static_map_array_literal", "static_varray_literal",
  "static_collection_literal", "dim_expr", "dim_expr_base", "query_expr",
  "query_assign_expr", "query_head", "query_body", "query_body_clauses",
  "query_body_clause", "from_clause", "let_clause", "where_clause",
  "join_clause", "join_into_clause", "orderby_clause", "orderings",
  "ordering", "ordering_direction", "select_or_group_clause",
  "select_clause", "group_clause", "query_continuation",
  "lexical_var_list", "xhp_tag", "xhp_tag_body", "xhp_opt_end_label",
  "xhp_attributes", "xhp_children", "xhp_attribute_name",
  "xhp_attribute_value", "xhp_child", "xhp_label_ws", "xhp_bareword",
  "simple_function_call", "fully_qualified_class_name",
  "static_class_name", "class_name_reference", "exit_expr",
  "backticks_expr", "ctor_arguments", "common_scalar", "static_expr",
  "static_class_constant", "scalar", "static_array_pair_list",
  "possible_comma", "hh_possible_comma",
  "non_empty_static_array_pair_list", "common_scalar_ae",
  "static_numeric_scalar_ae", "static_scalar_ae",
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
  "non_empty_static_collection_init", "map_array_init", "varray_init",
  "non_empty_map_array_init", "non_empty_varray_init",
  "static_map_array_init", "static_varray_init",
  "non_empty_static_map_array_init", "non_empty_static_varray_init",
  "encaps_list", "encaps_var", "encaps_var_offset", "internal_functions",
  "variable_list", "class_constant", "hh_opt_constraint",
  "hh_type_alias_statement", "hh_name_with_type", "hh_name_with_typevar",
  "hh_typeargs_opt", "hh_non_empty_type_list", "hh_type_list",
  "hh_func_type_list", "hh_opt_return_type", "hh_typevar_list",
  "hh_typevar_variance", "hh_shape_member_type",
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
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,    40,    41,    59,   123,   125,    36,    96,    93,    34,
      39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   211,   213,   212,   214,   214,   215,   215,   215,   215,
     215,   215,   215,   215,   216,   215,   217,   215,   215,   215,
     215,   215,   218,   218,   218,   218,   218,   218,   218,   218,
     218,   218,   218,   218,   218,   218,   218,   218,   218,   218,
     218,   219,   219,   220,   220,   221,   221,   222,   222,   222,
     222,   223,   223,   223,   223,   224,   224,   224,   224,   225,
     225,   226,   226,   226,   227,   228,   229,   230,   230,   231,
     231,   232,   232,   232,   232,   233,   233,   233,   234,   233,
     235,   233,   236,   233,   237,   233,   233,   233,   233,   233,
     233,   233,   233,   233,   233,   233,   233,   233,   233,   233,
     238,   233,   239,   233,   233,   240,   233,   241,   233,   233,
     233,   233,   233,   233,   233,   233,   233,   233,   233,   233,
     233,   233,   243,   242,   244,   244,   246,   245,   247,   247,
     248,   248,   249,   251,   250,   252,   250,   253,   250,   255,
     254,   256,   254,   258,   257,   259,   257,   260,   257,   261,
     257,   263,   262,   264,   262,   265,   265,   266,   267,   268,
     268,   268,   268,   268,   269,   269,   270,   270,   271,   271,
     272,   272,   273,   273,   274,   274,   275,   275,   275,   276,
     276,   277,   277,   278,   278,   279,   279,   280,   280,   281,
     281,   281,   281,   282,   282,   282,   283,   283,   284,   284,
     285,   285,   286,   286,   287,   287,   288,   288,   288,   288,
     288,   288,   288,   288,   289,   289,   289,   289,   289,   289,
     289,   289,   290,   290,   290,   290,   290,   290,   290,   290,
     291,   291,   291,   291,   291,   291,   291,   291,   292,   292,
     293,   293,   293,   293,   293,   293,   294,   294,   295,   295,
     295,   296,   296,   296,   296,   297,   297,   298,   299,   300,
     300,   302,   301,   303,   301,   301,   304,   301,   305,   301,
     301,   301,   301,   301,   301,   301,   301,   306,   306,   306,
     307,   308,   308,   309,   309,   310,   310,   311,   311,   312,
     312,   312,   312,   312,   312,   312,   313,   313,   314,   314,
     315,   315,   316,   316,   317,   318,   318,   318,   319,   319,
     319,   319,   320,   320,   320,   320,   320,   320,   320,   321,
     321,   321,   322,   322,   323,   323,   324,   324,   325,   325,
     326,   326,   327,   327,   327,   327,   327,   327,   327,   328,
     328,   329,   329,   329,   330,   330,   330,   330,   331,   331,
     332,   332,   332,   332,   332,   333,   334,   334,   335,   335,
     336,   336,   336,   337,   338,   339,   340,   341,   342,   342,
     342,   342,   342,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   344,   344,   346,   345,   347,   345,
     349,   348,   350,   348,   351,   348,   352,   348,   353,   348,
     354,   354,   354,   355,   355,   356,   356,   357,   357,   358,
     358,   359,   359,   360,   361,   361,   362,   363,   363,   364,
     365,   365,   366,   367,   368,   368,   369,   369,   369,   369,
     369,   370,   371,   372,   373,   373,   373,   373,   374,   374,
     375,   375,   375,   375,   375,   375,   376,   377,   378,   379,
     380,   381,   382,   382,   383,   383,   384,   384,   385,   385,
     386,   387,   388,   389,   389,   389,   389,   390,   391,   391,
     392,   392,   393,   393,   394,   394,   395,   396,   396,   397,
     397,   397,   398,   398,   398,   399,   399,   399,   399,   399,
     399,   399,   399,   399,   399,   399,   399,   399,   399,   399,
     399,   399,   399,   399,   399,   399,   399,   399,   399,   399,
     399,   399,   399,   399,   399,   399,   399,   399,   399,   399,
     399,   399,   399,   399,   399,   399,   399,   399,   399,   399,
     399,   399,   399,   399,   399,   399,   399,   399,   399,   399,
     399,   399,   399,   399,   399,   399,   399,   399,   399,   399,
     399,   399,   399,   399,   399,   399,   399,   399,   399,   399,
     399,   399,   399,   399,   399,   400,   401,   401,   402,   402,
     402,   403,   403,   403,   404,   404,   404,   405,   405,   405,
     406,   406,   407,   407,   407,   407,   407,   407,   407,   407,
     407,   407,   407,   407,   407,   407,   407,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   409,   409,
     409,   410,   410,   410,   410,   410,   410,   410,   411,   411,
     412,   412,   413,   413,   414,   414,   414,   414,   415,   415,
     415,   415,   415,   415,   416,   416,   416,   416,   417,   417,
     417,   417,   417,   417,   417,   418,   418,   419,   419,   419,
     419,   420,   420,   421,   421,   422,   422,   423,   423,   424,
     424,   425,   425,   427,   426,   428,   429,   429,   430,   430,
     431,   431,   431,   432,   432,   433,   433,   434,   434,   435,
     435,   436,   436,   437,   437,   438,   438,   438,   438,   438,
     438,   438,   438,   438,   438,   438,   439,   439,   439,   439,
     439,   439,   439,   439,   440,   440,   440,   441,   442,   442,
     443,   443,   444,   444,   444,   445,   445,   446,   446,   446,
     447,   447,   448,   448,   449,   449,   450,   450,   450,   450,
     450,   450,   451,   451,   451,   451,   451,   452,   452,   452,
     452,   452,   452,   453,   453,   454,   454,   454,   454,   454,
     454,   454,   454,   455,   455,   456,   456,   456,   456,   457,
     457,   458,   458,   458,   458,   459,   459,   460,   460,   461,
     461,   462,   462,   463,   463,   464,   464,   465,   465,   466,
     466,   467,   467,   467,   467,   468,   468,   468,   468,   468,
     468,   469,   469,   469,   470,   470,   470,   470,   470,   470,
     470,   470,   470,   470,   470,   471,   471,   472,   472,   473,
     473,   474,   474,   475,   475,   476,   476,   477,   477,   478,
     478,   479,   480,   480,   480,   480,   481,   481,   482,   482,
     482,   482,   483,   483,   483,   484,   484,   485,   485,   486,
     486,   487,   488,   488,   488,   488,   488,   488,   488,   488,
     488,   488,   488,   489,   489
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
       0,     0,     4,     0,     5,     2,     0,    10,     0,    11,
       3,     3,     3,     4,     4,     3,     5,     2,     2,     0,
       6,     5,     4,     3,     1,     1,     3,     4,     1,     1,
       4,     6,     1,     1,     4,     1,     1,     3,     2,     0,
       2,     0,     1,     3,     1,     1,     1,     1,     3,     4,
       4,     4,     1,     1,     2,     2,     2,     3,     3,     1,
       1,     1,     1,     3,     1,     3,     1,     1,     1,     0,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     1,     1,     1,     3,     5,     1,     3,     5,     4,
       3,     3,     2,     1,     1,     3,     3,     1,     1,     0,
       1,     2,     4,     3,     6,     2,     3,     6,     1,     1,
       1,     1,     1,     6,     3,     4,     6,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     5,     4,     1,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     1,     1,     1,
       1,     3,     2,     1,     5,     0,     0,    11,     0,    12,
       0,     3,     0,     4,     0,     6,     0,     7,     0,     5,
       2,     2,     4,     1,     1,     5,     3,     5,     3,     2,
       0,     2,     0,     4,     4,     3,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     1,     1,     1,     1,
       3,     2,     3,     4,     2,     3,     1,     2,     1,     2,
       1,     1,     1,     1,     1,     1,     4,     4,     2,     8,
      10,     2,     1,     3,     1,     2,     1,     1,     1,     1,
       2,     4,     3,     3,     4,     1,     2,     4,     2,     6,
       0,     1,     4,     0,     2,     0,     1,     1,     3,     1,
       3,     1,     1,     3,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     4,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     1,     0,     0,     1,     1,
       3,     0,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     2,     1,     1,     4,
       3,     4,     1,     1,     1,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     5,     4,     3,     3,
       3,     1,     1,     1,     1,     3,     3,     3,     2,     0,
       1,     0,     1,     0,     5,     3,     3,     1,     1,     1,
       1,     1,     3,     2,     1,     1,     1,     1,     1,     1,
       2,     2,     4,     3,     4,     2,     0,     5,     3,     3,
       1,     3,     1,     2,     0,     5,     3,     2,     0,     3,
       0,     4,     2,     0,     3,     3,     1,     0,     1,     1,
       1,     1,     3,     1,     1,     1,     3,     1,     1,     3,
       3,     2,     4,     2,     4,     1,     1,     1,     1,     1,
       3,     5,     3,     4,     4,     3,     1,     1,     1,     1,
       3,     5,     4,     3,     1,     1,     3,     3,     1,     1,
       7,     9,     7,     6,     8,     1,     2,     4,     4,     1,
       1,     4,     1,     0,     1,     2,     1,     1,     3,     5,
       3,     3,     0,     1,     3,     5,     3,     2,     3,     6,
       0,     1,     4,     2,     0,     5,     3,     3,     1,     6,
       4,     4,     2,     2,     0,     5,     3,     3,     1,     2,
       0,     5,     3,     3,     1,     2,     0,     2,     0,     5,
       3,     3,     1,     2,     0,     2,     0,     5,     3,     3,
       1,     2,     2,     1,     2,     1,     4,     3,     3,     6,
       3,     1,     1,     1,     4,     4,     4,     4,     4,     4,
       2,     2,     4,     2,     2,     1,     3,     3,     3,     0,
       2,     5,     6,     1,     2,     1,     4,     3,     0,     1,
       3,     2,     3,     1,     1,     0,     0,     2,     4,     2,
       6,     4,     1,     1,     0,     3,     5,     3,     1,     2,
       0,     4,     2,     2,     1,     1,     1,     1,     4,     6,
       1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   360,     0,   753,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   834,     0,
     822,   636,     0,   642,   643,   644,    22,   702,   810,    98,
      99,   645,     0,    80,     0,     0,     0,     0,     0,     0,
       0,     0,   132,     0,     0,     0,     0,     0,     0,   332,
     333,   334,   337,   336,   335,     0,     0,     0,     0,   159,
       0,     0,     0,   649,   651,   652,   646,   647,     0,     0,
     653,   648,     0,   627,    23,    24,    25,    27,    26,     0,
     650,     0,     0,     0,     0,     0,     0,     0,   654,   338,
      28,    29,    31,    30,    32,    33,    34,    35,    36,    37,
      38,    39,    40,   454,     0,    97,    70,   814,   637,     0,
       0,     4,    59,    61,    64,   701,     0,   626,     0,     6,
     131,     7,     9,     8,    10,     0,     0,   330,   370,     0,
       0,     0,     0,     0,     0,     0,   368,   798,   799,   438,
     437,   354,   439,   440,   443,     0,     0,   353,   776,   628,
       0,   704,   436,   329,   779,   369,     0,     0,   372,   371,
     777,   778,   775,   805,   809,     0,   426,   703,    11,   337,
     336,   335,     0,     0,    27,    59,   131,     0,   894,   369,
     893,     0,   891,   890,   442,     0,   361,   365,     0,     0,
     410,   411,   412,   413,   435,   433,   432,   431,   430,   429,
     428,   427,   810,   629,     0,   908,   628,     0,   392,     0,
     390,     0,   838,     0,   711,   352,   632,     0,   908,   631,
       0,   641,   817,   816,   633,     0,     0,   635,   434,     0,
       0,     0,     0,   357,     0,    78,   359,     0,     0,    84,
      86,     0,     0,    88,     0,     0,     0,   935,   936,   940,
       0,     0,    59,   934,     0,   937,     0,     0,    90,     0,
       0,     0,     0,   122,     0,     0,     0,     0,     0,     0,
      42,    47,   248,     0,     0,   247,     0,   163,     0,   160,
     253,     0,     0,     0,     0,     0,   905,   147,   157,   830,
     834,   875,     0,   656,     0,     0,     0,   873,     0,    16,
       0,    63,   139,   151,   158,   533,   470,   858,   856,   856,
       0,   899,   452,   456,   458,   757,   370,     0,   368,   369,
     371,     0,     0,   638,     0,   639,     0,     0,     0,   121,
       0,     0,    66,   239,     0,    21,   130,     0,   156,   143,
     155,   335,   338,   131,   331,   112,   113,   114,   115,   116,
     118,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   822,     0,   111,   813,
     813,   119,   844,     0,     0,     0,     0,     0,     0,   328,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   391,   389,   758,   759,     0,   813,     0,
     771,   239,   239,   813,     0,   815,   806,   830,     0,   131,
       0,     0,    92,     0,   755,   750,   711,     0,     0,     0,
       0,     0,   842,     0,   475,   710,   833,     0,     0,    66,
       0,   239,   351,     0,   773,   634,     0,    70,   199,     0,
     451,     0,    95,     0,     0,   358,     0,     0,     0,     0,
       0,    87,   110,    89,   932,   933,     0,   930,     0,     0,
       0,   904,     0,   117,    91,   120,     0,     0,     0,     0,
       0,     0,     0,   491,     0,   498,   500,   501,   502,   503,
     504,   505,   496,   518,   519,    70,     0,   107,   109,     0,
       0,    44,    51,     0,     0,    46,    55,    48,     0,    18,
       0,     0,   249,     0,    93,   162,   161,     0,     0,    94,
     895,     0,     0,   370,   368,   369,   372,   371,     0,   924,
     169,     0,   831,     0,     0,     0,     0,   655,   874,   702,
       0,     0,   872,   707,   871,    62,     5,    13,    14,     0,
     167,     0,     0,   463,     0,     0,   711,     0,     0,   630,
     464,   862,     0,   711,     0,     0,   711,     0,     0,     0,
       0,     0,   757,    70,     0,   713,   756,   944,   350,   423,
     785,   797,    75,    69,    71,    72,    73,    74,   329,     0,
     441,   705,   706,    60,   711,     0,   909,     0,     0,     0,
     713,   240,     0,   446,   133,   165,     0,   395,   397,   396,
       0,     0,   393,   394,   398,   400,   399,   415,   414,   417,
     416,   418,   420,   421,   419,   409,   408,   402,   403,   401,
     404,   405,   407,   422,   406,   812,     0,     0,   848,     0,
     711,   898,     0,   897,   782,   805,   149,   141,   153,   145,
     131,   360,     0,   363,   366,   374,   492,   388,   387,   386,
     385,   384,   383,   382,   381,   380,   379,   378,   377,   761,
       0,   760,   763,   780,   767,   908,   764,     0,     0,     0,
       0,     0,     0,     0,     0,   892,   362,   748,   752,   710,
     754,     0,     0,   908,     0,   837,     0,   836,     0,   821,
     820,     0,     0,   760,   763,   818,   764,   355,   201,   203,
      70,   461,   460,   356,     0,    70,   183,    79,   359,     0,
       0,     0,     0,     0,   195,   195,    85,     0,     0,     0,
     928,   711,     0,   915,     0,     0,     0,     0,     0,   709,
     645,     0,     0,   627,     0,     0,     0,     0,     0,    64,
     658,   626,   664,   665,   663,     0,   657,    68,   662,     0,
       0,   508,     0,     0,   514,   511,   512,   520,     0,   499,
     494,     0,   497,     0,     0,     0,    52,    19,     0,     0,
      56,    20,     0,     0,     0,    41,    49,     0,   246,   254,
     251,     0,     0,   884,   889,   886,   885,   888,   887,    12,
     922,   923,     0,     0,     0,     0,   830,   827,     0,   474,
     883,   882,   881,     0,   877,     0,   878,   880,     0,     5,
       0,     0,     0,   527,   528,   536,   535,     0,     0,   710,
     469,   473,     0,   479,   710,   857,     0,   477,   710,   855,
     478,     0,   900,     0,   453,     0,     0,   916,   757,   225,
     943,     0,     0,   772,   811,   710,   911,   907,   241,   242,
     625,   712,   238,     0,   757,     0,     0,   167,   448,   135,
     425,     0,   484,   485,     0,   476,   710,   843,     0,     0,
     239,   169,     0,   167,   165,     0,   822,   375,     0,     0,
     769,   770,   783,   784,   807,   808,     0,     0,     0,   736,
     718,   719,   720,   721,     0,     0,     0,   729,   728,   742,
     711,     0,   750,   841,   840,     0,     0,   774,   640,     0,
     205,     0,     0,    76,     0,     0,     0,     0,     0,     0,
       0,   175,   176,   187,     0,    70,   185,   104,   195,     0,
     195,     0,     0,   938,     0,     0,   710,   929,   931,   914,
     711,   913,     0,   711,   686,   687,   684,   685,   717,     0,
     711,   709,     0,     0,   472,   866,   864,   864,     0,     0,
     850,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   493,     0,     0,
       0,   516,   517,   515,     0,     0,   495,     0,   123,     0,
     126,   108,     0,    43,    53,     0,    45,    57,    50,   250,
       0,   896,    96,   924,   906,   919,   168,   170,   260,     0,
       0,   828,     0,   876,     0,    17,     0,   899,   166,   260,
       0,     0,   466,     0,   897,   861,   860,     0,   901,     0,
     916,   459,     0,     0,   944,     0,   230,   228,   763,   781,
     908,   910,     0,     0,   243,    67,     0,   757,   164,     0,
     757,     0,   424,   847,   846,     0,   239,     0,     0,     0,
       0,   167,   137,   641,   762,   239,     0,   724,   725,   726,
     727,   730,   731,   740,     0,   711,   736,     0,   723,   744,
     710,   747,   749,   751,     0,   835,   763,   819,   762,     0,
       0,     0,     0,   202,   462,    81,     0,   359,   175,   177,
     830,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     189,     0,   925,     0,   927,   710,     0,     0,     0,   660,
     710,   708,     0,   699,     0,   711,     0,   870,     0,   711,
       0,     0,   711,     0,   666,   700,   698,   854,     0,   711,
     669,   671,   670,     0,     0,   667,   668,   672,   674,   673,
     689,   688,   691,   690,   692,   694,   695,   693,   682,   681,
     676,   677,   675,   678,   679,   680,   683,   506,     0,   507,
     513,   521,   522,     0,    70,    54,    58,   252,     0,     0,
       0,   329,   832,   830,   364,   367,   373,     0,    15,     0,
     329,   539,     0,     0,   541,   534,   537,     0,   532,     0,
       0,   902,     0,   917,   455,     0,   231,     0,     0,   226,
       0,   245,   244,   916,     0,   260,     0,   757,     0,   239,
       0,   803,   260,   899,   260,     0,     0,   376,     0,     0,
     733,   710,   735,     0,   722,     0,     0,   711,   741,   839,
       0,    70,     0,   198,   184,     0,     0,     0,   174,   100,
     188,     0,     0,   191,     0,   196,   197,    70,   190,   939,
       0,   912,     0,   942,   716,   715,   659,     0,   710,   471,
     661,   482,   710,   865,     0,   480,   710,   863,   481,     0,
     483,   710,   849,   697,     0,     0,     0,     0,   918,   921,
     171,     0,     0,     0,   327,     0,     0,     0,   148,   259,
     261,     0,   326,     0,   329,     0,   879,   256,   152,   530,
       0,     0,   465,   859,   457,     0,   234,   224,     0,   227,
     233,   239,   445,   916,   329,   916,     0,   845,     0,   802,
     329,     0,   329,   260,   757,   800,   739,   738,   732,     0,
     734,   710,   743,    70,   204,    77,    82,   102,   178,     0,
     186,   192,    70,   194,   926,     0,     0,   468,     0,   869,
     868,     0,   853,   852,   696,     0,    70,   127,     0,     0,
       0,     0,     0,   172,   293,   289,   295,   627,    27,     0,
     285,     0,   292,   304,     0,   302,   307,     0,   306,     0,
     305,     0,   131,   263,     0,   265,     0,   829,     0,   531,
     529,   540,   538,   235,     0,     0,   222,   232,     0,     0,
       0,     0,   144,   445,   916,   804,   150,   256,   154,   329,
       0,     0,   746,     0,   200,     0,     0,    70,   181,   101,
     193,   941,   714,     0,     0,     0,     0,     0,   920,     0,
       0,     0,     0,   275,   279,     0,     0,     0,   270,   591,
     590,   587,   589,   588,   608,   610,   609,   579,   550,   551,
     569,   585,   584,   546,   556,   557,   559,   558,   578,   562,
     560,   561,   563,   564,   565,   566,   567,   568,   570,   571,
     572,   573,   574,   575,   577,   576,   547,   548,   549,   552,
     553,   555,   593,   594,   603,   602,   601,   600,   599,   598,
     586,   605,   595,   596,   597,   580,   581,   582,   583,   606,
     607,   611,   613,   612,   614,   615,   592,   617,   616,   619,
     621,   620,   554,   624,   622,   623,   618,   604,   545,   299,
     542,     0,   271,   320,   321,   319,   312,     0,   313,   272,
     346,     0,     0,     0,     0,   131,   140,   255,     0,     0,
       0,   223,   237,   801,     0,    70,   322,    70,   134,     0,
       0,     0,   146,   916,   737,     0,    70,   179,    83,   103,
       0,   467,   867,   851,   509,   125,   273,   274,   349,   173,
       0,     0,     0,   296,   286,     0,     0,     0,   301,   303,
       0,     0,   308,   315,   316,   314,     0,     0,   262,     0,
       0,     0,     0,   257,     0,   236,     0,   525,   713,     0,
       0,    70,   136,   142,     0,   745,     0,     0,     0,   105,
     276,    59,     0,   277,   278,     0,     0,   290,     0,   294,
     298,   543,   544,     0,   287,   317,   318,   310,   311,   309,
     347,   344,   266,   264,   348,     0,   258,   526,   712,     0,
     447,   323,     0,   138,     0,   182,   510,     0,   129,     0,
     329,     0,   297,   300,     0,   757,   268,     0,   523,   444,
     449,   180,     0,     0,   106,   283,     0,   328,   291,   345,
       0,   713,   340,   757,   524,     0,   128,     0,     0,   282,
     916,   757,   209,   341,   342,   343,   944,   339,     0,     0,
       0,   281,     0,   340,     0,   916,     0,   280,   324,    70,
     267,   944,     0,   214,   212,     0,    70,     0,     0,   215,
       0,     0,   210,   269,     0,   325,     0,   218,   208,     0,
     211,   217,   124,   219,     0,     0,   206,   216,     0,   207,
     221,   220
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   121,   829,   556,   185,   279,   510,
     514,   280,   511,   515,   123,   124,   125,   126,   127,   128,
     331,   593,   594,   464,   244,  1455,   470,  1379,  1456,  1688,
     785,   274,   505,  1649,  1021,  1204,  1704,   347,   186,   595,
     875,  1081,  1256,   132,   559,   892,   596,   615,   894,   540,
     891,   597,   560,   893,   349,   297,   313,   135,   877,   832,
     815,  1036,  1402,  1132,   941,  1598,  1459,   727,   947,   469,
     736,   949,  1287,   719,   930,   933,  1121,  1710,  1711,   584,
     585,   609,   610,   284,   285,   291,  1428,  1577,  1578,  1211,
    1329,  1421,  1573,  1695,  1713,  1610,  1653,  1654,  1655,  1409,
    1410,  1411,  1612,  1618,  1664,  1414,  1415,  1419,  1566,  1567,
    1568,  1588,  1740,  1330,  1331,   187,   137,  1726,  1727,  1571,
    1333,   138,   237,   465,   466,   139,   140,   141,   142,   143,
     144,   145,   146,  1440,   147,   874,  1080,   148,   241,   581,
     325,   582,   583,   460,   565,   566,  1155,   567,  1156,   149,
     150,   151,   152,   153,   762,   763,   764,   154,   155,   271,
     156,   272,   493,   494,   495,   496,   497,   498,   499,   500,
     501,   775,   776,  1013,   502,   503,   504,   782,  1638,   157,
     561,  1430,   562,  1050,   837,  1228,  1225,  1559,  1560,   158,
     159,   160,   231,   238,   334,   452,   161,   968,   768,   162,
     969,   866,   859,   970,   918,  1101,  1103,  1104,  1105,   920,
    1266,  1267,   921,   698,   436,   198,   199,   598,   587,   417,
     682,   683,   684,   685,   863,   164,   232,   189,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   646,   175,   234,
     235,   543,   223,   224,   649,   650,  1168,  1169,   575,   572,
     576,   573,  1161,  1158,  1162,  1159,   306,   307,   823,   176,
     531,   177,   580,   178,  1579,   298,   342,   604,   605,   962,
    1063,   812,   813,   740,   741,   742,   265,   266,   861
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1491
static const yytype_int16 yypact[] =
{
   -1491,   125, -1491, -1491,  5138, 13040, 13040,   -87, 13040, 13040,
   13040, 11128, 13040, -1491, 13040, 13040, 13040, 13040, 13040, 13040,
   13040, 13040, 13040, 13040, 13040, 13040, 15136, 15136, 11336, 13040,
   15202,   -50,   -46, -1491, -1491, -1491, -1491, -1491,   154, -1491,
   -1491,   109, 13040, -1491,   -46,   -36,   -23,   228,   -46, 11502,
   12709, 11668, -1491, 14447, 10130,    60, 13040, 14895,    22, -1491,
   -1491, -1491,   436,    57,     0,   232,   247,   260,   290, -1491,
   12709,   307,   340, -1491, -1491, -1491, -1491, -1491,   466, 14976,
   -1491, -1491, 12709, -1491, -1491, -1491, -1491, 12709, -1491, 12709,
   -1491,   187,   377,   417,   478,   502, 12709, 12709, -1491,   268,
   -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491,
   -1491, -1491, -1491, -1491, 13040, -1491, -1491,   151,   457,   469,
     469, -1491,   200,   404,   394, -1491,   514, -1491,    67, -1491,
     641, -1491, -1491, -1491, -1491, 16136,   555, -1491, -1491,   407,
     517,   519,   521,   526,   533, 13602, -1491, -1491, -1491, -1491,
     679, -1491, -1491, -1491,   689,   694,   554, -1491,    49,   513,
     612, -1491, -1491,   816,   122,  2158,    97,   561,    46, -1491,
     138,   140,   562,   266, -1491,    26, -1491,   699, -1491, -1491,
   -1491,   617,   566,   615, -1491, -1491,   641,   555, 16502,  2658,
   16502, 13040, 16502, 16502,  3130,   577, 15527,  3130,   737, 12709,
     718,   718,   460,   718,   718,   718,   718,   718,   718,   718,
     718,   718, -1491, -1491, 14772,   619, -1491,   638,   324,   592,
     324, 15136, 15571,   586,   788, -1491,   617, 14582,   619,   650,
     656,   599,   141, -1491,   324,    97, 11834, -1491, -1491, 13040,
    8674,   806,    72, 16502,  9714, -1491, 13040, 13040, 12709, -1491,
   -1491, 13646,   613, -1491, 13690, 14447, 14447,   645, -1491, -1491,
     616, 14219,   805, -1491,   808, -1491, 12709,   744, -1491,   624,
   13734,   629,   600, -1491,   245, 13778, 16181, 16196, 12709,    74,
   -1491,   256, -1491, 14911,    79, -1491,   707, -1491,   710, -1491,
     821,    82, 15136, 15136, 13040,   634,   667, -1491, -1491, 14996,
   11336,    71,    48, -1491, 13206, 15136,   498, -1491, 12709, -1491,
     -24,   404, -1491, -1491, -1491, -1491,  2679, 13040, 13040, 13040,
     826,   745, -1491, -1491, -1491,    52,   642, 16502,   644,  1576,
     646,  5346, 13040,   310,   640,   495,   310,   411,   351, -1491,
   12709, 14447,   649, 10338, 14447, -1491, -1491, 11171, -1491, -1491,
   -1491, -1491, -1491,   641, -1491, -1491, -1491, -1491, -1491, -1491,
   -1491, 13040, 13040, 13040, 12042, 13040, 13040, 13040, 13040, 13040,
   13040, 13040, 13040, 13040, 13040, 13040, 13040, 13040, 13040, 13040,
   13040, 13040, 13040, 13040, 13040, 13040, 15202, 13040, -1491, 13040,
   13040, -1491, 13040,  3662, 12709, 12709, 12709, 16136,   746,   388,
    9922, 13040, 13040, 13040, 13040, 13040, 13040, 13040, 13040, 13040,
   13040, 13040, 13040, -1491, -1491, -1491, -1491, 15224, 13040, 13040,
   -1491, 10338, 10338, 13040, 13040,   151,   144, 14996,   652,   641,
   12250, 13822, -1491, 13040, -1491,   653,   843, 14772,   658,     6,
     646, 14028,   324, 12458, -1491, 12666, -1491,   659,    54, -1491,
      42, 10338, -1491, 15287, -1491, -1491, 13866, -1491, -1491, 10546,
   -1491, 13040, -1491,   775,  8882,   858,   668, 16413,   856,    66,
      39, -1491, -1491, -1491, -1491, -1491, 14447, 16055,   677,   871,
   14671, -1491,   695, -1491, -1491, -1491,   804, 13040,   807,   809,
   13040, 13040, 13040, -1491,   600, -1491, -1491, -1491, -1491, -1491,
   -1491, -1491,   697, -1491, -1491, -1491,   683, -1491, -1491, 12709,
     686,   882,   296, 12709,   693,   883,   354,   358, 16243, -1491,
   12709, 13040,   324,    22, -1491, -1491, -1491, 14671,   815, -1491,
     324,    85,   113,   698,   701,  2437,    37,   702,   696,   124,
     779,   700,   324,   115,   704,  1171, 12709, -1491, -1491,   841,
    2273,    24, -1491, -1491, -1491,   404, -1491, -1491, -1491,   879,
     783,   742,   393, -1491,   151,   782,   905,   711,   767,   144,
   -1491, 16502,   716,   910, 15627,   719,   915,   723, 14447, 14447,
     912,   806,    52, -1491,   729,   928, -1491, 14447,    29,   873,
     139, -1491, -1491, -1491, -1491, -1491, -1491, -1491,   904,  2499,
   -1491, -1491, -1491, -1491,   935,   776, -1491, 15136, 13040,   748,
     937, 16502,   934, -1491, -1491,   825, 12085, 10737, 16669,  3130,
   13040, 16458, 13411, 16776, 16841, 16904,  2308, 12020, 12020, 12020,
   12020,  2382,  2382,  2382,  2382,   725,   725,   660,   660,   660,
     460,   460,   460, -1491,   718, 16502,   750,   751, 15671,   747,
     944, -1491, 13040,   400,   753,   144, -1491, -1491, -1491, -1491,
     641, 13040, 14826, -1491, -1491,  3130, -1491,  3130,  3130,  3130,
    3130,  3130,  3130,  3130,  3130,  3130,  3130,  3130,  3130, -1491,
   13040,   405,   145, -1491, -1491,   619,   454,   752,  3095,   754,
     759,   756,  3602,   117,   761, -1491, 16502,  4224, -1491, 12709,
   -1491,   642,    29,   619, 15136, 16502, 15136, 15727,    29,   146,
   -1491,   766, 13040, -1491,   147, -1491, -1491, -1491,  8466,   120,
   -1491, -1491, 16502, 16502,   -46, -1491, -1491, -1491, 13040,   874,
    3301, 14671, 12709,  9090,   771,   774, -1491,    73,   849,   831,
   -1491,   972,   781, 14300, 14447, 14671, 14671, 14671, 14671, 14671,
   -1491,   784,    51,   835,   785,   789,   790,   791, 14671,    -2,
   -1491,   844, -1491, -1491, -1491,   792, -1491, 16588, -1491, 13040,
     810, 16502,   811,   981, 13997,   991, -1491, 16502, 13953, -1491,
     697,   922, -1491,  5554, 16122,   803,   362, -1491, 16181, 12709,
     371, -1491, 16196, 12709, 12709, -1491, -1491,  3685, -1491, 16588,
     994, 15136,   820, -1491, -1491, -1491, -1491, -1491, -1491, -1491,
   -1491, -1491,    83, 12709, 16122,   822, 14996, 15081,   997, -1491,
   -1491, -1491, -1491,   817, -1491, 13040, -1491, -1491,  4722, -1491,
   14447, 16122,   827, -1491, -1491, -1491, -1491,   999, 13040,  2679,
   -1491, -1491, 12293, -1491, 13040, -1491, 13040, -1491, 13040, -1491,
   -1491,   829, -1491, 14447, -1491,   834,  5762,  1004,    53, -1491,
   -1491,   355, 15224, -1491, -1491, 14447, -1491, -1491,   324, 16502,
   -1491, 10754, -1491, 14671,    35,   823, 16122,   783, -1491, -1491,
   10321, 13040, -1491, -1491, 13040, -1491, 13040, -1491,  4129,   837,
   10338,   779,  1006,   783,   825, 12709, 15202,   324,  4196,   838,
   -1491, -1491,   179, -1491, -1491, -1491,  1026, 15358, 15358,  4224,
   -1491, -1491, -1491, -1491,   842,    89,   845, -1491, -1491, -1491,
    1033,   846,   653,   324,   324, 12874, 15287, -1491, -1491,  4413,
     496,   -46,  9714, -1491,  5970,   848,  6178,   851,  3301, 15136,
     857,   914,   324, 16588,  1035, -1491, -1491, -1491, -1491,   522,
   -1491,    18, 14447, -1491, 14447, 12709, 16055, -1491, -1491, -1491,
    1038, -1491,   850,   935,   691,   691,   992,   992, 15871,   852,
    1052, 14671,   916, 12709,  2679, 14671, 14671, 14671, 13910, 12501,
   14671, 14671, 14671, 14671, 14519, 14671, 14671, 14671, 14671, 14671,
   14671, 14671, 14671, 14671, 14671, 14671, 14671, 14671, 14671, 14671,
   14671, 14671, 14671, 14671, 14671, 14671, 14671, 16502, 13040, 13040,
   13040, -1491, -1491, -1491, 13040, 13040, -1491,   600, -1491,   985,
   -1491, -1491, 12709, -1491, -1491, 12709, -1491, -1491, -1491, -1491,
   14671,   324, -1491,   124, -1491,   968,  1060, -1491, -1491,   131,
     869,   324, 10962, -1491,  1947, -1491,  4930,   745,  1060, -1491,
     395,   276, 16502,   941, -1491, 16502, 16502, 15771, -1491,   870,
    1004, -1491, 14447,   806, 14447,    38,  1058,   996,   218, -1491,
     619, -1491, 15136, 13040, 16502, 16588,   875,    35, -1491,   878,
      35,   877, 10321, 16502, 15827,   884, 10338,   881,   880, 14447,
     885,   783, -1491,   599,   463, 10338, 13040, -1491, -1491, -1491,
   -1491, -1491, -1491,   949,   886,  1077,  4224,   946, -1491,  2679,
    4224, -1491, -1491, -1491, 15136, 16502,   237, -1491, -1491,   -46,
    1063,  1022,  9714, -1491, -1491, -1491,   893, 13040,   914,   324,
   14996,  3301,   895, 14671,  6386,   528,   896, 13040,    47,    31,
   -1491,   927, -1491,   971, -1491, 14366,  1073,   902, 14671, -1491,
   14671, -1491,   907, -1491,   976,  1098,   908, 16588,   909,  1103,
   15927,   911,  1107,   917, -1491, -1491, -1491, 15971,   913,  1108,
   16629, 16709,  4031, 14671, 16546, 16743, 16809, 16873, 16935, 14870,
   16965, 16965, 16965, 16965,  2426,  2426,  2426,  2426,   651,   651,
     691,   691,   691,   992,   992,   992,   992, 16502, 14130, 16502,
   -1491, 16502, -1491,   918, -1491, -1491, -1491, 16588, 12709, 14447,
   16122,   103, -1491, 14996, -1491, -1491,  3130,   919, -1491,   921,
     475, -1491,    70, 13040, -1491, -1491, -1491, 13040, -1491, 13040,
   13040, -1491,   806, -1491, -1491,   356,  1113,  1042, 13040, -1491,
     929,   324, 16502,  1004,   926, -1491,   932,    35, 13040, 10338,
     936, -1491, -1491,   745, -1491,   933,   930, -1491,   938,  4224,
   -1491,  4224, -1491,   939, -1491,  1013,   943,  1127, -1491,   324,
    1117, -1491,   945, -1491, -1491,   948,   950,   134, -1491, -1491,
   16588,   954,   956, -1491,  4591, -1491, -1491, -1491, -1491, -1491,
   14447, -1491, 14447, -1491, 16588, 16027, -1491, 14671,  2679, -1491,
   -1491, -1491, 14671, -1491, 14671, -1491, 14671, -1491, -1491, 14671,
   -1491, 14671, -1491,  4539, 14671, 13040,   947,  6594,  1056, -1491,
   -1491,   481, 14447, 16122, -1491, 16074,  1002, 15309, -1491, -1491,
   -1491,   746, 14153,    93,   388,   135, -1491, -1491, -1491,  1007,
    4457,  4501, 16502, 16502, -1491,    62,  1149,  1085, 13040, -1491,
   16502, 10338,  1055,  1004,  1234,  1004,   965, 16502,   967, -1491,
    1624,   966,  1755, -1491,    35, -1491, -1491,  1044, -1491,  4224,
   -1491,  2679, -1491, -1491,  8466, -1491, -1491, -1491, -1491,  9298,
   -1491, -1491, -1491,  8466, -1491,   973, 14671, 16588,  1046, 16588,
   16588, 16071, 16588, 16127,  4539, 14086, -1491, -1491, 14447, 16122,
   16122,  1163,    90, -1491, -1491,  1009, -1491,    95,   977,    99,
   -1491, 13414, -1491, -1491,   101, -1491, -1491,  3251, -1491,   979,
   -1491,  1101,   641, -1491, 14447, -1491,   746, -1491,  1874, -1491,
   -1491, -1491, -1491,  1170,  1106, 13040, -1491, 16502,   986,   988,
     983,   459, -1491,  1055,  1004, -1491, -1491, -1491, -1491,  1769,
     989,  4224, -1491,  1062,  8466,  9506,  9298, -1491, -1491, -1491,
    8466, -1491, 16588, 14671, 14671, 14671, 13040,  6802, -1491,   990,
     993, 14671, 16122, -1491, -1491, 14447,   673, 16074, -1491, -1491,
   -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491,
   -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491,
   -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491,
   -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491,
   -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491,
   -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491,
   -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491,
   -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491,   508,
   -1491,  1002, -1491, -1491, -1491, -1491, -1491,    56,   536, -1491,
    1180,   102, 12709,  1101,  1184,   641, -1491, -1491,   998,  1185,
   13040, -1491, 16502, -1491,   110, -1491, -1491, -1491, -1491,  1001,
     459,  3759, -1491,  1004, -1491,  4224, -1491, -1491, -1491, -1491,
    7010, 16588, 16588, 16588, 14041, -1491, -1491, -1491, 16588, -1491,
   14605,   129,    58, -1491, -1491, 14671, 13414, 13414,  1145, -1491,
    3251,  3251,   628, -1491, -1491, -1491, 14671,  1124, -1491,  1005,
     104, 14671, 12709, -1491, 14671, 16502,  1128, -1491,  1199,  7218,
    7426, -1491, -1491, -1491,   459, -1491,  7634,  1008,  1130,  1104,
   -1491,  1115,  1061, -1491, -1491,  1118, 14447, -1491,   673, -1491,
   16588, -1491, -1491,  1059, -1491,  1183, -1491, -1491, -1491, -1491,
   16588,  1203, -1491, -1491, 16588,  1018, 16588, -1491,   415,  1021,
   -1491, -1491,  7842, -1491,  1023, -1491, -1491,  1027,  1064, 12709,
     388,  1065, -1491, -1491, 14671,   114, -1491,  1150, -1491, -1491,
   -1491, -1491, 16122,   803, -1491,  1066, 12709,   523, -1491, 16588,
    1032,  1226,   575,   114, -1491,  1161, -1491, 16122,  1039, -1491,
    1004,   119, -1491, -1491, -1491, -1491, 14447, -1491,  1045,  1047,
     107, -1491,   530,   575,   409,  1004,  1048, -1491, -1491, -1491,
   -1491, 14447,    63,  1232,  1174,   530, -1491,  8050,   410,  1242,
    1178, 13040, -1491, -1491,  8258, -1491,    77,  1244,  1181, 13040,
   -1491, 16502, -1491,  1245,  1182, 13040, -1491, 16502, 13040, -1491,
   16502, 16502
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1491, -1491, -1491,  -495, -1491, -1491, -1491,    80, -1491, -1491,
   -1491,   749,   476,   471,    14,  1271,  3053, -1491,  2367, -1491,
     458, -1491,    17, -1491, -1491, -1491, -1491, -1491, -1491, -1491,
   -1491, -1491, -1491, -1491,  -438, -1491, -1491,  -160,   199,    28,
   -1491, -1491, -1491, -1491, -1491, -1491,    30, -1491, -1491, -1491,
   -1491,    34, -1491, -1491,   872,   887,   876,  -103,   374,  -803,
     380,   442,  -443,   148,  -792, -1491,  -181, -1491, -1491, -1491,
   -1491,  -672,    -7, -1491, -1491, -1491, -1491,  -435, -1491,  -543,
   -1491,  -381, -1491, -1491,   760, -1491,  -165, -1491, -1491,  -971,
   -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491,
    -193, -1491, -1491, -1491, -1491, -1491,  -276, -1491,   -41,  -954,
   -1491, -1490,  -458, -1491,  -161,    21,  -128,  -442, -1491,  -281,
   -1491,   -29,     4,  1248,  -664,  -355, -1491, -1491,   -12, -1491,
   -1491,    -5,   -28,  -150, -1491, -1491, -1491, -1491, -1491, -1491,
   -1491, -1491, -1491,  -524,  -784, -1491, -1491, -1491, -1491, -1491,
   -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491,   899,
   -1491, -1491,   278, -1491,   812, -1491, -1491, -1491, -1491, -1491,
   -1491, -1491,   286, -1491,   813, -1491, -1491,   524, -1491,   252,
   -1491, -1491, -1491, -1491, -1491, -1491, -1491, -1491,  -865, -1491,
    2079,  2496,  -330, -1491, -1491,   215,  2280,  3129, -1491, -1491,
     334,     3,  -582, -1491, -1491,   401,  -650,   205, -1491, -1491,
   -1491, -1491, -1491,   390, -1491, -1491, -1491,    69,  -823,  -162,
    -395,  -391, -1491,   451,  -112, -1491, -1491,   438, -1491, -1491,
    3589,   -27, -1491, -1491,    76,  -144, -1491,   212, -1491, -1491,
   -1491,  -384,  1014, -1491, -1491, -1491, -1491, -1491,  1000, -1491,
   -1491, -1491,   338, -1491, -1491, -1491,   574,   348, -1491, -1491,
    1024,  -283,  -981, -1491,   -26,   -67,  -175,    10,   578, -1491,
    -932, -1491,   285,   364, -1491, -1491, -1491,   -95, -1014
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -909
static const yytype_int16 yytable[] =
{
     188,   190,   398,   192,   193,   194,   196,   197,   354,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   129,   314,   222,   225,   136,   428,   264,   872,   320,
     321,   426,   131,   570,   133,  1064,   240,   243,   134,   855,
     689,   690,   269,   693,   251,   663,   254,   919,   245,   270,
    1235,   275,   249,   449,   420,  1053,   643,   854,   714,   354,
     397,   828,   715,   951,   937,  1620,  1219,  1658,   350,   734,
     711,   281,   453,   163,  1079,   732,   344,  1285,  1220,   290,
      13,   461,   952,   518,   122,   326,   328,   330,   523,  1621,
    1090,   528,  1033,   310,   801,   418,   311,    13,    13,  1472,
    1642,   282,  1424,  -488,  -288,   212,   233,  1321,  1477,   327,
    1561,  1627,  -488,  1627,   191,  -786,  1472,  1236,  1137,  1138,
    1339,   212,   801,   454,   817,     3,   817,   301,  1232,   340,
     252,  1137,  1138,   262,   415,   416,   972,   545,  1656,   546,
     817,  1433,  1749,   817,   817,  1636,  1128,  -908,    13,  -629,
     296,   236,   415,   416,  1683,   239,  1763,   415,   416,    13,
     474,   475,  -450,   418,    13,   246,   479,   312,   341,   296,
     810,   811,   569,   288,  1107,  -489,   296,   296,   247,   557,
     558,   289,   415,   416,   399,   438,   431,   440,  -789,  1637,
    1154,   931,   932,   616,   547,   304,   305,   303,   447,   415,
     416,  1237,  -908,   130,  -787,  -793,  -788,  -823,   590,  1322,
     423,  -790,  -826,  -824,  1323,   296,    59,    60,    61,   179,
     180,   351,  1324,  1140,   315,  1434,  1750,   446,   283,   827,
     339,   456,   425,   419,   456,  1108,  1288,  -229,   422,   807,
    1764,   243,   467,   735,   953,  -792,   606,   422,   117,   655,
    1286,  -229,  -712,  -786,  1034,  -712,   709,   458,  1622,  1325,
    1326,   463,  1327,  1659,   273,   533,   534,   537,   733,   694,
     345,   354,  1361,   655,  1354,   462,  1135,   519,  1139,   435,
     614,  1360,   524,  1362,  -791,   529,   352,   802,  1255,   327,
     512,   516,   517,  1473,  1474,   222,  1425,   655,  -288,   550,
    1657,   419,  1478,  -825,  1562,  1628,   655,  1673,  1328,   655,
    1737,  1352,   571,   574,   574,   803,  -213,   818,   612,   906,
     122,  -712,   555,  -795,   122,  1265,  -789,   599,   468,   314,
     350,  1076,   423,  1212,  1046,   353,  1378,  1427,   611,  1278,
    -796,   315,  -787,  -793,  -788,  -823,   481,   322,   424,  -790,
    -826,  -824,   136,   520,   506,   332,   617,   618,   619,   621,
     622,   623,   624,   625,   626,   627,   628,   629,   630,   631,
     632,   633,   634,   635,   636,   637,   638,   639,   640,   641,
     642,   737,   644,  -792,   645,   645,   429,   648,   664,   301,
    1065,  1345,  1449,   789,   586,   665,   667,   668,   669,   670,
     671,   672,   673,   674,   675,   676,   677,   678,   701,   340,
     440,   122,   507,   645,   688,  -630,   611,   611,   645,   692,
     603,  1441,  -791,  1443,   262,   665,   862,   296,   696,   248,
     301,  1226,  1039,   292,  1066,  1346,   552,   398,   705,   700,
     707,  -825,   165,   834,  1742,  1756,   611,   721,   293,   340,
    1697,   793,   415,   416,   722,   794,   723,   304,   305,  1022,
    1268,   294,   233,  1275,   218,   220,   323,  1068,  1025,   654,
     424,  1069,   324,   653,   296,   657,   296,   296,   889,  1321,
    1227,   726,   771,   851,   852,   774,   777,   778,  1743,  1757,
     301,   295,   860,   686,  1698,   397,   552,   681,   304,   305,
     895,    59,    60,    61,   179,   180,   351,   340,   299,  1087,
     899,   340,  1590,   386,  1388,   340,   797,   654,  1067,  1347,
      13,   703,  1615,   786,   340,   387,   710,   790,   889,   716,
     130,  1116,   281,   713,  1244,  1117,   301,  1246,  1616,  1234,
     862,   300,   333,  -908,   122,   301,   926,   835,   301,   879,
    1221,   302,   329,   286,   336,  1617,   570,   340,   304,   305,
     287,   602,   836,  1222,   341,  1623,  1093,  1119,  1120,   840,
     341,   352,  1744,  1758,   301,  -765,   845,   301,   316,   849,
     552,  1322,  1624,   552,   449,  1625,  1323,  1453,    59,    60,
      61,   179,   180,   351,  1324,  -908,   927,   660,  -908,  1223,
     796,  -908,   647,   869,   304,   305,  -765,  1399,  1400,  1366,
     355,  1367,   303,   304,   305,   880,   304,   305,   317,   399,
     601,  1136,  1137,  1138,  -768,   822,   824,  1282,  1137,  1138,
     687,  1325,  1326,  -766,  1327,   691,    59,    60,    61,   179,
     180,   351,   304,   305,   553,   304,   305,   888,   606,   606,
     548,   586,   439,   887,   554,  -768,   196,  1667,   352,   442,
      52,  1644,  1586,  1587,  -766,   448,  1665,  1666,    59,    60,
      61,   179,   180,   351,  1668,   898,   346,  1669,   165,   318,
    1338,   548,   165,   554,   548,   554,   554,  1214,  1723,  1724,
    1725,   570,   335,   337,   338,   569,   296,  1000,  1001,  1002,
    1003,  1004,  1005,   319,  1356,  1250,   352,   929,  1344,   383,
     384,   385,  1734,   386,  1258,   343,  1006,   392,   655,  1452,
     356,   522,   357,   243,   358,   387,  1719,  1748,   935,   359,
     530,   530,   535,  1738,  1739,  1047,   360,   542,   352,   136,
    1003,  1004,  1005,   551,   957,  -486,  1277,    33,    34,    35,
     946,  1661,  1662,   960,   963,   389,  1006,   391,  1059,   750,
     390,   393,   421,  -794,  1007,  -487,  -629,   427,   308,   165,
    1071,   380,   381,   382,   383,   384,   385,   917,   386,   922,
     432,   434,   655,   387,   486,   487,   488,   441,  1732,   341,
     387,   489,   490,   422,   444,   491,   492,   445,   122,  -628,
     451,  1594,   512,  1745,   136,   450,   516,    73,    74,    75,
      76,    77,   944,   122,   459,   476,   472,   477,   752,  -903,
    1044,  1450,   480,   482,    80,    81,   570,   483,  1092,  1335,
     569,   525,   485,  1052,   526,   527,   538,   539,    90,  1055,
     578,  1056,   579,  1057,   588,   129,   589,   600,   591,   136,
     -65,    52,   699,   613,   697,    98,   131,  1141,   133,  1142,
     702,   708,   134,   122,   724,   542,  1074,   461,  1358,  1024,
     731,   728,  1712,  1027,  1028,   439,  1082,   136,   743,  1083,
     744,  1084,   769,   770,   784,   611,   772,   781,   773,   787,
    1712,   788,   792,  1035,   800,  1240,   791,   163,  1733,   809,
     804,   816,   165,   805,   808,   814,   819,   825,   122,   830,
     831,   833,   838,   841,   839,   718,   842,   130,   843,   844,
    1115,   847,  1054,  1111,   848,   850,   853,   586,   857,    59,
      60,    61,    62,    63,   351,  1122,   122,   858,   686,  -490,
      69,   394,   681,   586,   865,  1645,   871,   867,   873,  1123,
     870,   876,   885,   886,   890,   136,   902,   136,   882,   883,
     900,   903,   878,   783,   904,   569,  1147,  1233,   928,   860,
    1438,   938,   233,  1151,   948,   296,   395,   950,   396,   954,
     955,   956,   130,   958,   973,   971,   974,  1100,  1100,   917,
     975,   976,   977,   979,  1253,  1010,   980,  1008,  1009,   352,
    1014,  1017,   716,  1197,  1198,  1199,   713,  1020,  1030,   774,
    1201,  1042,   122,  1051,   122,   570,   122,    59,    60,    61,
      62,    63,   351,  1032,  1077,  1043,  1038,   130,    69,   394,
    1215,  1049,  1058,  1060,  1062,  1143,  1089,  1216,  1086,  1095,
    1096,   856,  1110,  1106,  1131,   868,  1109,  1145,  1112,  1133,
    1071,  1125,  1146,  1153,  1127,   130,  1679,  1006,  1130,  1166,
    1149,  1150,   547,   129,  1203,  1209,   396,   136,  1242,  1210,
    1213,  1229,  1238,  1231,   131,  1239,   133,  1243,  1247,  1259,
     134,   611,  1245,  1251,  1252,  1249,  1261,   352,   570,  1254,
     611,  1216,  1264,  1271,  1260,  1272,  1274,  1279,  1289,  1283,
     897,  1290,  1205,  1292,  1293,  1206,  1297,  1298,  1262,  1296,
    1300,  1301,  1302,  1305,  1319,   163,  1306,  1311,  1310,  1308,
    1316,  1349,   243,  1270,  1336,  1337,   122,  1348,  1353,  1722,
    1351,  1364,  1284,   130,  1355,   130,  1371,  1363,  1359,  1273,
    1365,  1368,   923,  1369,   924,  1370,   586,  1373,  1375,   586,
    1376,  1396,  1377,  1398,   569,   136,   165,  1380,  1299,  1381,
    1413,  1429,  1303,  1435,  1436,  1307,  1439,  1444,   942,  1445,
    1447,   165,  1312,  1426,  1451,  1461,  1463,  1471,   934,  1475,
    1570,  1476,  1569,   936,  1580,  1581,   917,  1585,  1583,  1584,
     917,  1593,  1595,  1606,  1626,  1384,  1607,  1385,  1631,  1634,
    1663,  1633,   122,  1671,   354,  1641,  1672,  1677,  1678,  1686,
    1689,  1685,  -284,  1687,   122,  1690,  1621,  1694,  1340,  1696,
    1693,   165,  1341,  1699,  1342,  1343,  1701,   569,  1702,  1714,
    1717,  1703,  1332,  1350,  1720,  1721,  1708,  1423,  1321,  1031,
    1729,  1332,  1731,  1357,   611,   130,  1751,  1735,    36,  1736,
     820,   821,  1746,  1752,   542,  1041,  1759,  1760,  1765,  1768,
    1766,  1769,  1572,  1026,  1023,  1716,   165,   795,  1091,   659,
    1372,  1088,   658,  1048,  1730,  1599,  1276,  1382,  1728,    13,
    1334,   656,  1591,   798,  1614,  1619,  1420,  1753,  1318,  1334,
     242,  1741,  1630,  1589,   165,  1202,  1401,   215,   215,   666,
    1200,   228,  1224,  1468,  1016,  1152,   779,   780,  1257,  1102,
    1395,  1263,  1113,  1070,   544,  1163,   586,   532,  1208,   577,
    1144,   961,     0,     0,   228,     0,     0,    84,    85,     0,
      86,   184,    88,   130,     0,     0,     0,     0,   136,   917,
    1322,   917,     0,  1437,     0,  1323,   611,    59,    60,    61,
     179,   180,   351,  1324,     0,   399,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
     165,     0,   165,     0,   165,  1332,   942,  1129,     0,     0,
    1611,  1332,     0,  1332,     0,     0,     0,     0,     0,     0,
    1325,  1326,     0,  1327,     0,   136,  1458,   122,  1574,     0,
       0,     0,   262,  1134,   136,     0,     0,  1418,     0,     0,
       0,     0,     0,     0,     0,  1632,     0,   352,     0,     0,
       0,     0,     0,  1334,     0,     0,     0,     0,     0,  1334,
    1582,  1334,     0,   586,     0,     0,     0,     0,     0,  1442,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   917,
       0,     0,     0,     0,   122,     0,     0,     0,     0,   122,
       0,  1604,     0,   122,     0,     0,     0,     0,     0,     0,
    1332,     0,  1597,  1458,     0,   136,     0,     0,     0,     0,
       0,   136,     0,     0,   165,   215,     0,     0,   136,     0,
       0,  1558,   215,     0,     0,     0,     0,  1565,   215,     0,
       0,     0,     0,     0,   262,  1629,     0,     0,   262,     0,
    1241,     0,     0,     0,     0,     0,   130,     0,  1334,     0,
       0,     0,     0,     0,     0,     0,   228,   228,     0,  1706,
    1422,   917,   228,     0,   122,   122,   122,     0,     0,     0,
     122,     0,     0,     0,     0,     0,     0,   122,     0,     0,
       0,     0,  1269,     0,   215,     0,     0,     0,     0,     0,
     165,  1691,     0,   215,   215,  1675,     0,     0,   542,   942,
     215,     0,   165,   130,     0,  1635,   215,     0,     0,   354,
       0,     0,   130,     0,     0,     0,     0,   228,     0,     0,
     430,   401,   402,   403,   404,   405,   406,   407,   408,   409,
     410,   411,   412,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   228,     0,     0,   228,     0,     0,     0,     0,
       0,   136,     0,     0,     0,  1575,     0,     0,  1321,     0,
       0,   860,     0,     0,     0,     0,     0,     0,     0,   413,
     414,     0,     0,     0,     0,     0,   860,     0,     0,     0,
       0,   542,   296,   130,     0,     0,     0,   228,     0,   130,
     136,   136,  1317,     0,     0,     0,   130,   136,     0,    13,
       0,   262,     0,     0,     0,   917,     0,     0,     0,     0,
     122,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1651,     0,     0,     0,     0,     0,  1558,  1558,   215,     0,
    1565,  1565,     0,   136,   415,   416,     0,     0,   215,     0,
       0,  1707,   296,     0,     0,     0,     0,     0,     0,   122,
     122,     0,     0,     0,     0,     0,   122,     0,     0,  1374,
    1322,     0,     0,     0,     0,  1323,     0,    59,    60,    61,
     179,   180,   351,  1324,     0,  1383,  1761,   228,   228,     0,
       0,   759,     0,     0,  1767,   165,     0,     0,     0,  1321,
    1770,     0,   122,  1771,   586,     0,     0,     0,   136,  1705,
       0,     0,     0,  1321,     0,   136,     0,     0,   590,     0,
    1325,  1326,   586,  1327,     0,     0,  1718,     0,     0,     0,
     586,     0,     0,     0,     0,     0,     0,     0,   759,   130,
      13,     0,     0,     0,     0,     0,     0,   352,     0,     0,
       0,     0,   165,     0,    13,     0,     0,   165,     0,     0,
       0,   165,     0,     0,     0,     0,     0,   122,     0,  1446,
       0,  1454,     0,     0,   122,     0,     0,     0,   130,   130,
    1460,     0,     0,     0,     0,   130,     0,     0,     0,   228,
     228,     0,     0,     0,  1467,     0,     0,     0,   228,     0,
       0,  1322,     0,     0,     0,     0,  1323,     0,    59,    60,
      61,   179,   180,   351,  1324,  1322,     0,     0,   215,     0,
    1323,   130,    59,    60,    61,   179,   180,   351,  1324,     0,
       0,     0,   165,   165,   165,     0,     0,     0,   165,     0,
       0,     0,     0,   255,     0,   165,     0,     0,     0,     0,
       0,  1325,  1326,     0,  1327,  1600,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1325,  1326,     0,  1327,   256,
       0,     0,     0,   215,     0,     0,     0,     0,   352,     0,
       0,     0,     0,     0,     0,     0,   130,     0,     0,     0,
       0,    36,   352,   130,     0,     0,     0,   361,   362,   363,
    1448,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1592,   215,   364,   215,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,     0,
     386,   215,   759,     0,     0,     0,   257,   258,     0,     0,
       0,     0,   387,     0,   228,   228,   759,   759,   759,   759,
     759,     0,     0,     0,   183,     0,     0,    82,   259,   759,
      84,    85,     0,    86,   184,    88,     0,     0,   165,     0,
       0,     0,     0,  1639,     0,  1640,     0,     0,   260,     0,
       0,     0,     0,     0,  1646,   228,     0,     0,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   215,     0,     0,   261,     0,   165,   165,  1576,
       0,     0,     0,     0,   165,   228,     0,   215,   215,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1682,
       0,   228,   228,     0,     0,   216,   216,     0,     0,   229,
     228,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     165,     0,     0,     0,   228,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   228,     0,     0,     0,
       0,     0,     0,     0,   759,     0,     0,   228,     0,     0,
       0,     0,     0,     0,     0,  1217,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   228,     0,     0,
       0,     0,   400,   401,   402,   403,   404,   405,   406,   407,
     408,   409,   410,   411,   412,   165,     0,     0,     0,     0,
       0,     0,   165,     0,     0,     0,     0,  1747,     0,     0,
       0,     0,     0,     0,  1754,     0,     0,     0,     0,   215,
     215,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   413,   414,   228,     0,   228,     0,   228,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   759,     0,     0,   228,   759,   759,   759,     0,
       0,   759,   759,   759,   759,   759,   759,   759,   759,   759,
     759,   759,   759,   759,   759,   759,   759,   759,   759,   759,
     759,   759,   759,   759,   759,   759,   759,   759,     0,     0,
       0,     0,     0,   361,   362,   363,   415,   416,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     216,   759,   364,     0,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,     0,   386,     0,     0,     0,
       0,     0,     0,   228,     0,   228,     0,     0,   387,     0,
       0,     0,     0,   215,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     228,   386,   216,     0,     0,     0,     0,     0,     0,     0,
       0,   216,   216,   387,     0,     0,     0,     0,   216,     0,
     228,     0,     0,     0,   216,   215,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   216,     0,     0,     0,     0,
       0,   215,   215,     0,   759,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   228,     0,     0,   759,
     263,   759,  -909,  -909,  -909,  -909,   378,   379,   380,   381,
     382,   383,   384,   385,     0,   386,     0,     0,     0,     0,
       0,     0,     0,     0,   759,     0,     0,   387,     0,     0,
       0,   430,   401,   402,   403,   404,   405,   406,   407,   408,
     409,   410,   411,   412,     0,   229,  -909,  -909,  -909,  -909,
     998,   999,  1000,  1001,  1002,  1003,  1004,  1005,   826,     0,
     228,   228,     0,     0,   215,     0,     0,     0,     0,     0,
       0,  1006,     0,     0,     0,     0,     0,     0,     0,     0,
     413,   414,     0,     0,     0,     0,   216,     0,     0,   361,
     362,   363,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   217,   217,     0,     0,   230,     0,   364,     0,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,     0,   386,     0,     0,     0,     0,     0,     0,   765,
       0,   228,     0,   228,   387,   415,   416,     0,   759,   228,
       0,     0,     0,   759,     0,   759,     0,   759,     0,     0,
     759,     0,   759,     0,     0,   759,     0,     0,     0,     0,
       0,     0,     0,   228,   228,     0,   228,     0,     0,     0,
       0,     0,     0,   228,     0,     0,   765,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   263,   263,     0,     0,     0,     0,   263,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   806,
       0,     0,   228,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   759,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   228,
     228,   228,   430,   401,   402,   403,   404,   405,   406,   407,
     408,   409,   410,   411,   412,     0,   216,     0,     0,     0,
       0,     0,     0,     0,     0,   228,     0,     0,     0,   228,
       0,     0,     0,     0,   864,     0,     0,     0,   263,     0,
     217,   263,     0,     0,     0,     0,     0,   217,     0,     0,
       0,   413,   414,   217,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   759,   759,   759,     0,     0,     0,
       0,   216,   759,   228,     0,     0,   228,     0,   228,     0,
       0,     0,     0,     0,     0,     0,    36,     0,   212,     0,
     766,     0,     0,     0,     0,   563,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   217,
       0,     0,     0,   216,     0,   216,   415,   416,   217,   217,
       0,     0,     0,     0,     0,   217,     0,   213,     0,     0,
       0,   217,     0,     0,     0,     0,     0,   766,     0,   216,
     765,     0,   568,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   765,   765,   765,   765,   765,   183,
       0,     0,    82,    83,     0,    84,    85,   765,    86,   184,
      88,     0,     0,   263,   739,     0,     0,   761,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   228,  1019,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,     0,
     216,   228,   230,     0,     0,   564,   759,     0,     0,     0,
       0,     0,     0,  1037,   761,   216,   216,   759,     0,     0,
       0,     0,   759,     0,     0,   759,     0,     0,     0,     0,
    1037,     0,     0,     0,     0,     0,     0,     0,   216,     0,
       0,     0,     0,   217,     0,     0,     0,   228,     0,     0,
       0,     0,     0,   217,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   263,   263,     0,     0,     0,
       0,     0,   765,     0,   263,  1078,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   759,     0,     0,     0,     0,
       0,     0,     0,   228,     0,   229,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   228,     0,
       0,     0,     0,     0,     0,     0,     0,   228,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   766,   228,     0,     0,     0,     0,   216,   216,     0,
       0,     0,     0,     0,     0,   766,   766,   766,   766,   766,
       0,     0,     0,     0,     0,     0,     0,     0,   766,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     765,     0,     0,   216,   765,   765,   765,     0,     0,   765,
     765,   765,   765,   765,   765,   765,   765,   765,   765,   765,
     765,   765,   765,   765,   765,   765,   765,   765,   765,   765,
     765,   765,   765,   765,   765,   765,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   761,     0,
       0,     0,     0,   217,     0,   361,   362,   363,     0,   765,
     263,   263,   761,   761,   761,   761,   761,     0,     0,     0,
       0,     0,     0,     0,   364,   761,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,     0,   386,     0,
       0,   216,     0,   766,     0,     0,     0,     0,   217,   364,
     387,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,     0,   386,     0,     0,     0,     0,   216,     0,
       0,     0,     0,   216,     0,   387,     0,   263,     0,     0,
     217,     0,   217,     0,     0,     0,     0,     0,     0,   216,
     216,     0,   765,     0,     0,     0,     0,     0,     0,     0,
     263,     0,     0,     0,     0,     0,   217,   765,     0,   765,
       0,     0,   263,     0,     0,     0,     0,     0,     0,     0,
     761,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   766,   765,     0,     0,   766,   766,   766,     0,     0,
     766,   766,   766,   766,   766,   766,   766,   766,   766,   766,
     766,   766,   766,   766,   766,   766,   766,   766,   766,   766,
     766,   766,   766,   766,   766,   766,   766,     0,     0,  1320,
       0,     0,   216,     0,     0,     0,     0,   217,     0,     0,
     901,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     766,     0,   217,   217,     0,     0,     0,     0,     0,   263,
       0,   263,     0,   739,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,   568,   939,     0,   761,     0,
       0,     0,   761,   761,   761,     0,     0,   761,   761,   761,
     761,   761,   761,   761,   761,   761,   761,   761,   761,   761,
     761,   761,   761,   761,   761,   761,   761,   761,   761,   761,
     761,   761,   761,   761,     0,     0,   765,   216,    36,     0,
     212,   765,     0,   765,     0,   765,     0,     0,   765,     0,
     765,     0,   230,   765,     0,     0,     0,   761,     0,     0,
       0,     0,  1403,     0,  1412,  1563,     0,    84,    85,  1564,
      86,   184,    88,   766,     0,     0,     0,     0,     0,   213,
       0,     0,     0,     0,     0,     0,     0,     0,   766,   263,
     766,   263,   940,     0,   217,   217,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
     216,   183,  1417,   766,    82,    83,   263,    84,    85,     0,
      86,   184,    88,     0,     0,   765,     0,     0,     0,     0,
     568,     0,     0,     0,     0,     0,     0,     0,  1469,  1470,
       0,     0,     0,     0,     0,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
     761,     0,   214,     0,     0,     0,     0,   117,     0,     0,
       0,     0,   263,     0,     0,   761,     0,   761,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   760,     0,     0,     0,     0,     0,     0,
     761,     0,   765,   765,   765,     0,     0,     0,     0,     0,
     765,  1609,     0,     0,     0,     0,  1412,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   217,     0,
       0,     0,     0,     0,     0,     0,   263,   766,     0,     0,
     760,     0,   766,     0,   766,     0,   766,     0,     0,   766,
       0,   766,     0,     0,   766,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   568,     0,     0,     0,   767,
     217,     0,   361,   362,   363,   219,   219,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   217,   217,     0,     0,
       0,   364,     0,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,     0,   386,   799,   263,     0,   263,
       0,     0,     0,     0,   761,     0,   766,   387,     0,   761,
       0,   761,     0,   761,     0,     0,   761,     0,   761,     0,
       0,   761,     0,     0,     0,     0,     0,     0,     0,   263,
       0,     0,     0,     0,   765,   361,   362,   363,     0,   263,
       0,     0,     0,     0,     0,   765,     0,     0,     0,   217,
     765,     0,     0,   765,   364,     0,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,     0,   386,    36,
       0,   212,     0,   766,   766,   766,     0,     0,     0,     0,
     387,   766,     0,   761,     0,     0,  1613,     0,     0,     0,
       0,     0,     0,     0,     0,   263,     0,     0,     0,     0,
       0,     0,     0,   765,     0,     0,     0,     0,     0,     0,
       0,  1715,     0,     0,   760,     0,   651,     0,   255,     0,
       0,   263,     0,     0,   568,   263,  1403,     0,   760,   760,
     760,   760,   760,   219,     0,     0,     0,   905,     0,     0,
     219,   760,     0,     0,   256,     0,   219,     0,    84,    85,
       0,    86,   184,    88,     0,     0,     0,     0,     0,     0,
     761,   761,   761,     0,     0,     0,    36,     0,   761,     0,
       0,     0,   263,     0,     0,     0,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     943,     0,     0,     0,     0,     0,   652,   568,   117,     0,
       0,     0,   219,     0,   964,   965,   966,   967,     0,     0,
       0,   219,   219,   536,     0,     0,     0,   978,   219,     0,
    1029,   257,   258,     0,   219,   766,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   766,     0,     0,   183,
       0,   766,    82,   259,   766,    84,    85,     0,    86,   184,
      88,     0,     0,     0,     0,     0,   760,     0,     0,     0,
       0,     0,     0,   260,     0,     0,     0,     0,  1692,     0,
       0,     0,     0,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,   263,     0,
     261,     0,     0,     0,  1643,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   766,     0,     0,  1652,     0,     0,
       0,     0,   761,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   761,     0,     0,     0,     0,   761,     0,
       0,   761,  1075,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   219,     0,     0,     0,
       0,     0,     0,   263,   760,     0,   219,     0,   760,   760,
     760,     0,     0,   760,   760,   760,   760,   760,   760,   760,
     760,   760,   760,   760,   760,   760,   760,   760,   760,   760,
     760,   760,   760,   760,   760,   760,   760,   760,   760,   760,
     984,   761,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,  1002,
    1003,  1004,  1005,   760,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   263,     0,     0,  1006,     0,     0,     0,
       0,     0,     0,     0,  1157,  1160,  1160,     0,   263,  1167,
    1170,  1171,  1172,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,  1189,  1190,
    1191,  1192,  1193,  1194,  1195,  1196,     0,     0,     0,   361,
     362,   363,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   364,  1207,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,     0,   386,     0,     0,     0,   760,     0,     0,     0,
       0,     0,     0,     0,   387,     0,   219,     0,     0,     0,
       0,   760,     0,   760,     0,     0,   361,   362,   363,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   364,   760,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,     0,   386,
       0,   219,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   387,  1280,     0,     0,     0,     0,     0,     0,     0,
     907,   908,     0,     0,     0,     0,     0,  1294,     0,  1295,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     909,     0,     0,   219,     0,   219,     0,     0,   910,   911,
     912,    36,  1313,     0,     0,     0,     0,     0,     0,     0,
     913,     0,     0,     0,     0,     0,     0,     0,     0,   219,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1085,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     760,     0,     0,     0,     0,   760,   914,   760,     0,   760,
       0,     0,   760,     0,   760,     0,     0,   760,     0,   915,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      84,    85,     0,    86,   184,    88,     0,     0,     0,     0,
     219,     0,     0,     0,     0,     0,     0,     0,   916,     0,
       0,  1094,     0,     0,     0,   219,   219,     0,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,   361,   362,   363,  1387,     0,     0,     0,
       0,  1389,     0,  1390,     0,  1391,     0,     0,  1392,   760,
    1393,     0,   364,  1394,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,     0,   386,   361,   362,   363,
       0,     0,     0,     0,     0,     0,     0,     0,   387,     0,
       0,     0,     0,     0,     0,     0,   364,     0,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,     0,
     386,   361,   362,   363,     0,  1462,   760,   760,   760,     0,
       0,     0,   387,     0,   760,     0,     0,   219,   219,     0,
     364,     0,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,     0,   386,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   387,     0,     0,     0,
     985,   986,   987,   988,   989,   990,   991,   992,   993,   994,
     995,   996,   997,   998,   999,  1000,  1001,  1002,  1003,  1004,
    1005,     0,  1601,  1602,  1603,     0,     0,     0,     0,     0,
    1608,   361,   362,   363,  1006,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1118,     0,
     364,  1285,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,     0,   386,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   387,     0,     0,     0,
       0,   219,  1431,     0,     0,     0,     0,     0,   760,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   760,
       0,     0,     0,     0,   760,     0,     0,   760,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   219,     0,     0,  1432,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   219,
     219,     0,     0,     0,     0,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,  1660,     0,     0,   760,     0,    11,
      12,     0,     0,     0,     0,  1670,     0,     0,     0,     0,
    1674,     0,     0,  1676,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,  1286,     0,    33,    34,    35,    36,
      37,    38,   219,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,     0,     0,  1709,    49,    50,    51,    52,    53,    54,
      55,     0,    56,    57,    58,    59,    60,    61,    62,    63,
      64,     0,    65,    66,    67,    68,    69,    70,     0,     0,
       0,     0,     0,    71,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,    79,    80,    81,    82,    83,     0,    84,    85,
       0,    86,    87,    88,    89,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,    93,    94,    95,
      96,     0,    97,     0,    98,    99,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,     0,     0,   114,     0,   115,   116,  1045,   117,   118,
       0,   119,   120,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,     0,
       0,     0,    49,    50,    51,    52,    53,    54,    55,     0,
      56,    57,    58,    59,    60,    61,    62,    63,    64,     0,
      65,    66,    67,    68,    69,    70,     0,     0,     0,     0,
       0,    71,    72,     0,    73,    74,    75,    76,    77,     0,
       0,     0,     0,     0,     0,    78,     0,     0,     0,     0,
      79,    80,    81,    82,    83,     0,    84,    85,     0,    86,
      87,    88,    89,     0,     0,    90,     0,     0,    91,     0,
       0,     0,     0,     0,    92,    93,    94,    95,    96,     0,
      97,     0,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,     0,
       0,   114,     0,   115,   116,  1218,   117,   118,     0,   119,
     120,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,     0,     0,     0,
      49,    50,    51,    52,    53,    54,    55,     0,    56,    57,
      58,    59,    60,    61,    62,    63,    64,     0,    65,    66,
      67,    68,    69,    70,     0,     0,     0,     0,     0,    71,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,    79,    80,
      81,    82,    83,     0,    84,    85,     0,    86,    87,    88,
      89,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,    93,    94,    95,    96,     0,    97,     0,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,     0,     0,   114,
       0,   115,   116,     0,   117,   118,     0,   119,   120,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,     0,     0,     0,    49,    50,
      51,    52,     0,    54,    55,     0,    56,     0,    58,    59,
      60,    61,    62,    63,    64,     0,    65,    66,    67,     0,
      69,    70,     0,     0,     0,     0,     0,    71,    72,     0,
      73,    74,    75,    76,    77,     0,     0,     0,     0,     0,
       0,    78,     0,     0,     0,     0,   183,    80,    81,    82,
      83,     0,    84,    85,     0,    86,   184,    88,    89,     0,
       0,    90,     0,     0,    91,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,     0,     0,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,     0,     0,   114,     0,   115,
     116,   592,   117,   118,     0,   119,   120,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,     0,    47,
       0,     0,    48,     0,     0,     0,    49,    50,    51,    52,
       0,    54,    55,     0,    56,     0,    58,    59,    60,    61,
      62,    63,    64,     0,    65,    66,    67,     0,    69,    70,
       0,     0,     0,     0,     0,    71,    72,     0,    73,    74,
      75,    76,    77,     0,     0,     0,     0,     0,     0,    78,
       0,     0,     0,     0,   183,    80,    81,    82,    83,     0,
      84,    85,     0,    86,   184,    88,    89,     0,     0,    90,
       0,     0,    91,     0,     0,     0,     0,     0,    92,    93,
      94,    95,     0,     0,     0,     0,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,     0,     0,   114,     0,   115,   116,  1018,
     117,   118,     0,   119,   120,     5,     6,     7,     8,     9,
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
       0,     0,   183,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   184,    88,    89,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,    93,    94,    95,
       0,     0,     0,     0,    98,    99,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,     0,     0,   114,     0,   115,   116,  1061,   117,   118,
       0,   119,   120,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,     0,
       0,     0,    49,    50,    51,    52,     0,    54,    55,     0,
      56,     0,    58,    59,    60,    61,    62,    63,    64,     0,
      65,    66,    67,     0,    69,    70,     0,     0,     0,     0,
       0,    71,    72,     0,    73,    74,    75,    76,    77,     0,
       0,     0,     0,     0,     0,    78,     0,     0,     0,     0,
     183,    80,    81,    82,    83,     0,    84,    85,     0,    86,
     184,    88,    89,     0,     0,    90,     0,     0,    91,     0,
       0,     0,     0,     0,    92,    93,    94,    95,     0,     0,
       0,     0,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,     0,
       0,   114,     0,   115,   116,  1124,   117,   118,     0,   119,
     120,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,  1126,    45,
       0,    46,     0,    47,     0,     0,    48,     0,     0,     0,
      49,    50,    51,    52,     0,    54,    55,     0,    56,     0,
      58,    59,    60,    61,    62,    63,    64,     0,    65,    66,
      67,     0,    69,    70,     0,     0,     0,     0,     0,    71,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   183,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   184,    88,
      89,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,    93,    94,    95,     0,     0,     0,     0,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,     0,     0,   114,
       0,   115,   116,     0,   117,   118,     0,   119,   120,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,  1281,     0,    48,     0,     0,     0,    49,    50,
      51,    52,     0,    54,    55,     0,    56,     0,    58,    59,
      60,    61,    62,    63,    64,     0,    65,    66,    67,     0,
      69,    70,     0,     0,     0,     0,     0,    71,    72,     0,
      73,    74,    75,    76,    77,     0,     0,     0,     0,     0,
       0,    78,     0,     0,     0,     0,   183,    80,    81,    82,
      83,     0,    84,    85,     0,    86,   184,    88,    89,     0,
       0,    90,     0,     0,    91,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,     0,     0,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,     0,     0,   114,     0,   115,
     116,     0,   117,   118,     0,   119,   120,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,     0,    47,
       0,     0,    48,     0,     0,     0,    49,    50,    51,    52,
       0,    54,    55,     0,    56,     0,    58,    59,    60,    61,
      62,    63,    64,     0,    65,    66,    67,     0,    69,    70,
       0,     0,     0,     0,     0,    71,    72,     0,    73,    74,
      75,    76,    77,     0,     0,     0,     0,     0,     0,    78,
       0,     0,     0,     0,   183,    80,    81,    82,    83,     0,
      84,    85,     0,    86,   184,    88,    89,     0,     0,    90,
       0,     0,    91,     0,     0,     0,     0,     0,    92,    93,
      94,    95,     0,     0,     0,     0,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,     0,     0,   114,     0,   115,   116,  1397,
     117,   118,     0,   119,   120,     5,     6,     7,     8,     9,
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
       0,     0,   183,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   184,    88,    89,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,    93,    94,    95,
       0,     0,     0,     0,    98,    99,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,     0,     0,   114,     0,   115,   116,  1605,   117,   118,
       0,   119,   120,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,  1647,    47,     0,     0,    48,     0,
       0,     0,    49,    50,    51,    52,     0,    54,    55,     0,
      56,     0,    58,    59,    60,    61,    62,    63,    64,     0,
      65,    66,    67,     0,    69,    70,     0,     0,     0,     0,
       0,    71,    72,     0,    73,    74,    75,    76,    77,     0,
       0,     0,     0,     0,     0,    78,     0,     0,     0,     0,
     183,    80,    81,    82,    83,     0,    84,    85,     0,    86,
     184,    88,    89,     0,     0,    90,     0,     0,    91,     0,
       0,     0,     0,     0,    92,    93,    94,    95,     0,     0,
       0,     0,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,     0,
       0,   114,     0,   115,   116,     0,   117,   118,     0,   119,
     120,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,     0,     0,     0,
      49,    50,    51,    52,     0,    54,    55,     0,    56,     0,
      58,    59,    60,    61,    62,    63,    64,     0,    65,    66,
      67,     0,    69,    70,     0,     0,     0,     0,     0,    71,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   183,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   184,    88,
      89,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,    93,    94,    95,     0,     0,     0,     0,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,     0,     0,   114,
       0,   115,   116,  1680,   117,   118,     0,   119,   120,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,     0,     0,     0,    49,    50,
      51,    52,     0,    54,    55,     0,    56,     0,    58,    59,
      60,    61,    62,    63,    64,     0,    65,    66,    67,     0,
      69,    70,     0,     0,     0,     0,     0,    71,    72,     0,
      73,    74,    75,    76,    77,     0,     0,     0,     0,     0,
       0,    78,     0,     0,     0,     0,   183,    80,    81,    82,
      83,     0,    84,    85,     0,    86,   184,    88,    89,     0,
       0,    90,     0,     0,    91,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,     0,     0,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,     0,     0,   114,     0,   115,
     116,  1681,   117,   118,     0,   119,   120,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,  1684,    46,     0,    47,
       0,     0,    48,     0,     0,     0,    49,    50,    51,    52,
       0,    54,    55,     0,    56,     0,    58,    59,    60,    61,
      62,    63,    64,     0,    65,    66,    67,     0,    69,    70,
       0,     0,     0,     0,     0,    71,    72,     0,    73,    74,
      75,    76,    77,     0,     0,     0,     0,     0,     0,    78,
       0,     0,     0,     0,   183,    80,    81,    82,    83,     0,
      84,    85,     0,    86,   184,    88,    89,     0,     0,    90,
       0,     0,    91,     0,     0,     0,     0,     0,    92,    93,
      94,    95,     0,     0,     0,     0,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,     0,     0,   114,     0,   115,   116,     0,
     117,   118,     0,   119,   120,     5,     6,     7,     8,     9,
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
       0,     0,   183,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   184,    88,    89,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,    93,    94,    95,
       0,     0,     0,     0,    98,    99,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,     0,     0,   114,     0,   115,   116,  1700,   117,   118,
       0,   119,   120,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,     0,
       0,     0,    49,    50,    51,    52,     0,    54,    55,     0,
      56,     0,    58,    59,    60,    61,    62,    63,    64,     0,
      65,    66,    67,     0,    69,    70,     0,     0,     0,     0,
       0,    71,    72,     0,    73,    74,    75,    76,    77,     0,
       0,     0,     0,     0,     0,    78,     0,     0,     0,     0,
     183,    80,    81,    82,    83,     0,    84,    85,     0,    86,
     184,    88,    89,     0,     0,    90,     0,     0,    91,     0,
       0,     0,     0,     0,    92,    93,    94,    95,     0,     0,
       0,     0,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,     0,
       0,   114,     0,   115,   116,  1755,   117,   118,     0,   119,
     120,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,     0,     0,     0,
      49,    50,    51,    52,     0,    54,    55,     0,    56,     0,
      58,    59,    60,    61,    62,    63,    64,     0,    65,    66,
      67,     0,    69,    70,     0,     0,     0,     0,     0,    71,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   183,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   184,    88,
      89,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,    93,    94,    95,     0,     0,     0,     0,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,     0,     0,   114,
       0,   115,   116,  1762,   117,   118,     0,   119,   120,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,     0,     0,     0,    49,    50,
      51,    52,     0,    54,    55,     0,    56,     0,    58,    59,
      60,    61,    62,    63,    64,     0,    65,    66,    67,     0,
      69,    70,     0,     0,     0,     0,     0,    71,    72,     0,
      73,    74,    75,    76,    77,     0,     0,     0,     0,     0,
       0,    78,     0,     0,     0,     0,   183,    80,    81,    82,
      83,     0,    84,    85,     0,    86,   184,    88,    89,     0,
       0,    90,     0,     0,    91,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,     0,     0,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,     0,     0,   114,     0,   115,
     116,     0,   117,   118,     0,   119,   120,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,   457,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,     0,    47,
       0,     0,    48,     0,     0,     0,    49,    50,    51,    52,
       0,    54,    55,     0,    56,     0,    58,    59,    60,    61,
     179,   180,    64,     0,    65,    66,    67,     0,     0,     0,
       0,     0,     0,     0,     0,    71,    72,     0,    73,    74,
      75,    76,    77,     0,     0,     0,     0,     0,     0,    78,
       0,     0,     0,     0,   183,    80,    81,    82,    83,     0,
      84,    85,     0,    86,   184,    88,     0,     0,     0,    90,
       0,     0,    91,     0,     0,     0,     0,     0,    92,    93,
      94,    95,     0,     0,     0,     0,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,     0,     0,   114,     0,   115,   116,     0,
     117,   118,     0,   119,   120,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,   725,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,     0,     0,     0,    49,    50,    51,    52,     0,    54,
      55,     0,    56,     0,    58,    59,    60,    61,   179,   180,
      64,     0,    65,    66,    67,     0,     0,     0,     0,     0,
       0,     0,     0,    71,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   183,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   184,    88,     0,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,    93,    94,    95,
       0,     0,     0,     0,    98,    99,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,     0,     0,   114,     0,   115,   116,     0,   117,   118,
       0,   119,   120,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
     945,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,     0,
       0,     0,    49,    50,    51,    52,     0,    54,    55,     0,
      56,     0,    58,    59,    60,    61,   179,   180,    64,     0,
      65,    66,    67,     0,     0,     0,     0,     0,     0,     0,
       0,    71,    72,     0,    73,    74,    75,    76,    77,     0,
       0,     0,     0,     0,     0,    78,     0,     0,     0,     0,
     183,    80,    81,    82,    83,     0,    84,    85,     0,    86,
     184,    88,     0,     0,     0,    90,     0,     0,    91,     0,
       0,     0,     0,     0,    92,    93,    94,    95,     0,     0,
       0,     0,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,     0,
       0,   114,     0,   115,   116,     0,   117,   118,     0,   119,
     120,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,  1457,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,     0,     0,     0,
      49,    50,    51,    52,     0,    54,    55,     0,    56,     0,
      58,    59,    60,    61,   179,   180,    64,     0,    65,    66,
      67,     0,     0,     0,     0,     0,     0,     0,     0,    71,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   183,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   184,    88,
       0,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,    93,    94,    95,     0,     0,     0,     0,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,     0,     0,   114,
       0,   115,   116,     0,   117,   118,     0,   119,   120,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,  1596,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,     0,     0,     0,    49,    50,
      51,    52,     0,    54,    55,     0,    56,     0,    58,    59,
      60,    61,   179,   180,    64,     0,    65,    66,    67,     0,
       0,     0,     0,     0,     0,     0,     0,    71,    72,     0,
      73,    74,    75,    76,    77,     0,     0,     0,     0,     0,
       0,    78,     0,     0,     0,     0,   183,    80,    81,    82,
      83,     0,    84,    85,     0,    86,   184,    88,     0,     0,
       0,    90,     0,     0,    91,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,     0,     0,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,     0,     0,   114,     0,   115,
     116,     0,   117,   118,     0,   119,   120,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,     0,    47,
       0,     0,    48,     0,     0,     0,    49,    50,    51,    52,
       0,    54,    55,     0,    56,     0,    58,    59,    60,    61,
     179,   180,    64,     0,    65,    66,    67,     0,     0,     0,
       0,     0,     0,     0,     0,    71,    72,     0,    73,    74,
      75,    76,    77,     0,     0,     0,     0,     0,     0,    78,
       0,     0,     0,     0,   183,    80,    81,    82,    83,     0,
      84,    85,     0,    86,   184,    88,     0,     0,     0,    90,
       0,     0,    91,     0,     0,     0,     0,     0,    92,    93,
      94,    95,     0,     0,     0,     0,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,     0,     0,   114,     0,   115,   116,     0,
     117,   118,     0,   119,   120,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   661,
      12,     0,     0,     0,     0,     0,     0,   662,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    52,     0,     0,
       0,     0,     0,     0,     0,    59,    60,    61,   179,   180,
     181,     0,     0,    66,    67,     0,     0,     0,     0,     0,
       0,     0,     0,   182,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   183,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   184,    88,     0,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,    93,    94,    95,
       0,     0,     0,     0,    98,    99,   267,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,     0,     0,   114,     0,     0,     0,     0,   117,   118,
       0,   119,   120,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    52,     0,     0,     0,     0,
       0,     0,     0,    59,    60,    61,   179,   180,   181,     0,
       0,    66,    67,     0,     0,     0,     0,     0,     0,     0,
       0,   182,    72,     0,    73,    74,    75,    76,    77,     0,
       0,     0,     0,     0,     0,    78,     0,     0,     0,     0,
     183,    80,    81,    82,    83,     0,    84,    85,     0,    86,
     184,    88,     0,     0,     0,    90,     0,     0,    91,     0,
       0,     0,     0,     0,    92,    93,    94,    95,     0,     0,
       0,     0,    98,    99,   267,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,     0,
       0,   114,     0,   268,     0,     0,   117,   118,     0,   119,
     120,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   607,   386,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,   387,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    52,     0,     0,     0,     0,     0,     0,
       0,    59,    60,    61,   179,   180,   181,     0,     0,    66,
      67,     0,     0,     0,     0,     0,     0,     0,     0,   182,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   183,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   184,    88,
       0,   608,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,    93,    94,    95,     0,     0,     0,     0,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,     0,     0,   114,
       0,     0,     0,     0,   117,   118,     0,   119,   120,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    52,     0,     0,     0,     0,     0,     0,     0,    59,
      60,    61,   179,   180,   181,     0,     0,    66,    67,     0,
       0,     0,     0,     0,     0,     0,     0,   182,    72,     0,
      73,    74,    75,    76,    77,     0,     0,     0,     0,     0,
       0,    78,     0,     0,     0,     0,   183,    80,    81,    82,
      83,     0,    84,    85,     0,    86,   184,    88,     0,     0,
       0,    90,     0,     0,    91,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,     0,     0,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,     0,     0,   114,   362,   363,
     720,     0,   117,   118,     0,   119,   120,     5,     6,     7,
       8,     9,     0,     0,     0,     0,   364,    10,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,  1072,
     386,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,   387,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    52,
       0,     0,     0,     0,     0,     0,     0,    59,    60,    61,
     179,   180,   181,     0,     0,    66,    67,     0,     0,     0,
       0,     0,     0,     0,     0,   182,    72,     0,    73,    74,
      75,    76,    77,     0,     0,     0,     0,     0,     0,    78,
       0,     0,     0,     0,   183,    80,    81,    82,    83,     0,
      84,    85,     0,    86,   184,    88,     0,  1073,     0,    90,
       0,     0,    91,     0,     0,     0,     0,     0,    92,    93,
      94,    95,     0,     0,     0,     0,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,     0,     0,   114,     0,     0,     0,     0,
     117,   118,     0,   119,   120,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   661,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    52,     0,     0,
       0,     0,     0,     0,     0,    59,    60,    61,   179,   180,
     181,     0,     0,    66,    67,     0,     0,     0,     0,     0,
       0,     0,     0,   182,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   183,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   184,    88,     0,     0,     0,    90,     0,     0,
      91,     5,     6,     7,     8,     9,    92,    93,    94,    95,
       0,    10,     0,     0,    98,    99,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,     0,     0,   114,     0,     0,     0,     0,   117,   118,
       0,   119,   120,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     195,     0,     0,    52,     0,     0,     0,     0,     0,     0,
       0,    59,    60,    61,   179,   180,   181,     0,    36,    66,
      67,     0,     0,     0,     0,     0,     0,     0,     0,   182,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   183,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   184,    88,
       0,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,    93,    94,    95,     0,     0,     0,     0,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,    84,    85,   114,
      86,   184,    88,     0,   117,   118,     0,   119,   120,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   221,   613,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    52,     0,     0,     0,     0,     0,     0,     0,    59,
      60,    61,   179,   180,   181,     0,     0,    66,    67,     0,
       0,     0,     0,     0,     0,     0,     0,   182,    72,     0,
      73,    74,    75,    76,    77,     0,     0,     0,     0,     0,
       0,    78,     0,     0,     0,     0,   183,    80,    81,    82,
      83,     0,    84,    85,     0,    86,   184,    88,     0,     0,
       0,    90,     0,     0,    91,     5,     6,     7,     8,     9,
      92,    93,    94,    95,     0,    10,     0,     0,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,     0,     0,   114,     0,     0,
       0,     0,   117,   118,     0,   119,   120,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    52,     0,     0,
       0,     0,     0,     0,     0,    59,    60,    61,   179,   180,
     181,     0,     0,    66,    67,     0,     0,     0,     0,     0,
       0,     0,     0,   182,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   183,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   184,    88,     0,     0,     0,    90,     0,     0,
      91,     5,     6,     7,     8,     9,    92,    93,    94,    95,
       0,    10,     0,     0,    98,    99,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,     0,     0,   114,     0,   250,     0,     0,   117,   118,
       0,   119,   120,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    52,     0,     0,     0,     0,     0,     0,
       0,    59,    60,    61,   179,   180,   181,     0,     0,    66,
      67,     0,     0,     0,     0,     0,     0,     0,     0,   182,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   183,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   184,    88,
       0,     0,     0,    90,     0,     0,    91,     5,     6,     7,
       8,     9,    92,    93,    94,    95,     0,    10,     0,     0,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,     0,     0,   114,
       0,   253,     0,     0,   117,   118,     0,   119,   120,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    52,
       0,     0,     0,     0,     0,     0,     0,    59,    60,    61,
     179,   180,   181,     0,     0,    66,    67,     0,     0,     0,
       0,     0,     0,     0,     0,   182,    72,     0,    73,    74,
      75,    76,    77,     0,     0,     0,     0,     0,     0,    78,
       0,     0,     0,     0,   183,    80,    81,    82,    83,     0,
      84,    85,     0,    86,   184,    88,     0,     0,     0,    90,
       0,     0,    91,     0,     0,     0,     0,     0,    92,    93,
      94,    95,     0,     0,     0,     0,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,     0,     0,   114,   455,     0,     0,     0,
     117,   118,     0,   119,   120,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,  -909,  -909,  -909,  -909,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   620,   386,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   387,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    52,     0,     0,
       0,     0,     0,     0,     0,    59,    60,    61,   179,   180,
     181,     0,    36,    66,    67,     0,     0,     0,     0,     0,
       0,     0,     0,   182,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   183,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   184,    88,     0,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,    93,    94,    95,
       0,     0,     0,     0,    98,    99,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,    84,    85,   114,    86,   184,    88,     0,   117,   118,
       0,   119,   120,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   662,   878,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    52,     0,     0,     0,     0,
       0,     0,     0,    59,    60,    61,   179,   180,   181,     0,
      36,    66,    67,     0,     0,     0,     0,     0,     0,     0,
       0,   182,    72,     0,    73,    74,    75,    76,    77,     0,
       0,     0,     0,     0,     0,    78,     0,     0,     0,     0,
     183,    80,    81,    82,    83,     0,    84,    85,     0,    86,
     184,    88,     0,     0,     0,    90,     0,   651,    91,     0,
       0,     0,     0,     0,    92,    93,    94,    95,     0,     0,
       0,     0,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,    84,
      85,   114,    86,   184,    88,     0,   117,   118,     0,   119,
     120,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   704,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    52,     0,     0,     0,     0,     0,     0,
       0,    59,    60,    61,   179,   180,   181,     0,    36,    66,
      67,     0,     0,     0,     0,     0,     0,     0,     0,   182,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   183,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   184,    88,
       0,     0,     0,    90,     0,  1165,    91,     0,     0,     0,
       0,     0,    92,    93,    94,    95,     0,     0,     0,     0,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,    84,    85,   114,
      86,   184,    88,     0,   117,   118,     0,   119,   120,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   706,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    52,     0,     0,     0,     0,     0,     0,     0,    59,
      60,    61,   179,   180,   181,     0,    36,    66,    67,     0,
       0,     0,     0,     0,     0,     0,     0,   182,    72,     0,
      73,    74,    75,    76,    77,     0,     0,     0,     0,     0,
       0,    78,     0,     0,     0,     0,   183,    80,    81,    82,
      83,     0,    84,    85,     0,    86,   184,    88,     0,     0,
       0,    90,     0,     0,    91,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,     0,     0,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,    84,    85,   114,    86,   184,
      88,     0,   117,   118,     0,   119,   120,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,  1114,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    52,
       0,     0,     0,     0,     0,     0,     0,    59,    60,    61,
     179,   180,   181,     0,     0,    66,    67,     0,     0,     0,
       0,     0,     0,     0,     0,   182,    72,     0,    73,    74,
      75,    76,    77,     0,     0,     0,     0,     0,     0,    78,
       0,     0,     0,     0,   183,    80,    81,    82,    83,     0,
      84,    85,     0,    86,   184,    88,     0,     0,     0,    90,
       0,     0,    91,     5,     6,     7,     8,     9,    92,    93,
      94,    95,     0,    10,     0,     0,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,     0,     0,   114,     0,     0,     0,     0,
     117,   118,     0,   119,   120,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    52,     0,     0,     0,     0,
       0,     0,     0,    59,    60,    61,   179,   180,   181,     0,
       0,    66,    67,     0,     0,     0,     0,     0,     0,     0,
       0,   182,    72,     0,    73,    74,    75,    76,    77,     0,
       0,     0,     0,     0,     0,    78,     0,     0,     0,     0,
     183,    80,    81,    82,    83,     0,    84,    85,     0,    86,
     184,    88,     0,     0,     0,    90,     0,     0,    91,     5,
       6,     7,     8,     9,    92,    93,    94,    95,     0,    10,
       0,     0,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,     0,
       0,   114,     0,     0,     0,     0,   117,   118,     0,   119,
     120,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,   549,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    52,     0,     0,     0,     0,     0,     0,     0,    59,
      60,    61,   179,   180,   181,     0,     0,    66,    67,     0,
       0,     0,     0,     0,     0,     0,     0,   182,    72,     0,
      73,    74,    75,    76,    77,     0,     0,     0,     0,     0,
       0,    78,     0,     0,     0,     0,   183,    80,    81,    82,
      83,     0,    84,    85,     0,    86,   184,    88,     0,     0,
       0,    90,     0,     0,    91,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,     0,     0,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,     0,     0,   114,     0,     0,
       0,     0,   117,   118,     0,   119,   120,  1479,  1480,  1481,
    1482,  1483,     0,     0,  1484,  1485,  1486,  1487,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1488,  1489,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,     0,   386,     0,     0,  1490,     0,     0,
       0,     0,     0,     0,     0,     0,   387,     0,     0,     0,
       0,  1491,  1492,  1493,  1494,  1495,  1496,  1497,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1498,  1499,  1500,  1501,  1502,  1503,  1504,  1505,  1506,
    1507,  1508,  1509,  1510,  1511,  1512,  1513,  1514,  1515,  1516,
    1517,  1518,  1519,  1520,  1521,  1522,  1523,  1524,  1525,  1526,
    1527,  1528,  1529,  1530,  1531,  1532,  1533,  1534,  1535,  1536,
    1537,  1538,     0,     0,     0,  1539,  1540,     0,  1541,  1542,
    1543,  1544,  1545,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1546,  1547,  1548,     0,     0,     0,
      84,    85,     0,    86,   184,    88,  1549,     0,  1550,  1551,
       0,  1552,     0,     0,     0,     0,     0,     0,  1553,     0,
       0,     0,  1554,     0,  1555,     0,  1556,  1557,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   361,   362,   363,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   364,     0,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,     0,   386,   361,   362,   363,     0,
       0,     0,     0,     0,     0,     0,     0,   387,     0,     0,
       0,     0,     0,     0,     0,   364,     0,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,     0,   386,
     361,   362,   363,     0,     0,     0,     0,     0,     0,     0,
       0,   387,     0,     0,     0,     0,     0,     0,     0,   364,
       0,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,     0,   386,   361,   362,   363,     0,     0,     0,
       0,     0,     0,     0,     0,   387,     0,     0,     0,     0,
       0,     0,     0,   364,     0,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,     0,   386,   361,   362,
     363,     0,     0,     0,     0,     0,     0,     0,     0,   387,
       0,     0,     0,     0,     0,   388,     0,   364,     0,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
       0,   386,   361,   362,   363,     0,     0,     0,     0,     0,
       0,     0,     0,   387,     0,     0,     0,     0,     0,   471,
       0,   364,     0,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,     0,   386,   361,   362,   363,     0,
       0,     0,     0,     0,     0,     0,     0,   387,     0,     0,
       0,     0,     0,   473,     0,   364,     0,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,     0,   386,
     981,   982,   983,     0,     0,     0,     0,     0,     0,     0,
       0,   387,     0,     0,     0,     0,     0,   484,     0,   984,
       0,   985,   986,   987,   988,   989,   990,   991,   992,   993,
     994,   995,   996,   997,   998,   999,  1000,  1001,  1002,  1003,
    1004,  1005,     0,   361,   362,   363,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1006,     0,     0,     0,     0,
       0,   508,   364,     0,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,     0,   386,   361,   362,   363,
       0,     0,     0,     0,     0,     0,     0,     0,   387,     0,
       0,     0,     0,     0,   695,     0,   364,     0,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,     0,
     386,   361,   362,   363,     0,     0,     0,     0,     0,     0,
       0,     0,   387,     0,     0,     0,     0,     0,   717,     0,
     364,     0,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,     0,   386,     0,   361,   362,   363,     0,
       0,     0,     0,     0,     0,    36,   387,   212,     0,     0,
       0,     0,  1164,     0,     0,   364,     0,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,     0,   386,
     361,   362,   363,     0,     0,     0,     0,     0,     0,     0,
    1015,   387,     0,     0,     0,     0,     0,     0,     0,   364,
       0,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   255,   386,    84,    85,     0,    86,   184,    88,
    1011,  1012,     0,     0,     0,   387,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   256,     0,
       0,     0,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,     0,     0,
      36,  1648,   652,     0,   117,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   255,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  -328,     0,
       0,     0,     0,     0,     0,     0,    59,    60,    61,   179,
     180,   351,     0,     0,   256,  1466,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   257,   258,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,   183,     0,     0,    82,   259,     0,    84,
      85,     0,    86,   184,    88,     0,     0,     0,  1315,     0,
       0,     0,     0,     0,   478,     0,     0,   260,     0,   255,
       0,     0,     0,     0,     0,     0,   352,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   257,   258,     0,   261,   256,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   183,
       0,     0,    82,   259,     0,    84,    85,    36,    86,   184,
      88,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   260,     0,   255,     0,     0,     0,     0,
       0,     0,     0,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,     0,
     261,   256,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   257,   258,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
     183,     0,     0,    82,   259,     0,    84,    85,     0,    86,
     184,    88,     0,   959,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   260,     0,   255,     0,     0,     0,
       0,     0,     0,     0,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   257,   258,
       0,   261,   256,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   183,     0,     0,    82,
     259,     0,    84,    85,    36,    86,   184,    88,     0,  1291,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     260,     0,     0,     0,     0,     0,     0,     0,     0,  1173,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,   745,   746,   261,     0,     0,
       0,   747,     0,   748,     0,     0,     0,     0,     0,   257,
     258,     0,     0,     0,     0,   749,     0,     0,     0,     0,
       0,     0,     0,    33,    34,    35,    36,   183,     0,     0,
      82,   259,     0,    84,    85,   750,    86,   184,    88,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   260,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,     0,   261,    29,
      30,   751,     0,    73,    74,    75,    76,    77,     0,    36,
       0,   212,     0,     0,   752,     0,     0,     0,     0,   183,
      80,    81,    82,   753,     0,    84,    85,     0,    86,   184,
      88,     0,    36,     0,    90,     0,     0,     0,     0,     0,
       0,     0,     0,   754,   755,   756,   757,     0,     0,     0,
     213,    98,     0,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   745,   746,     0,
     758,     0,     0,   747,     0,   748,     0,     0,     0,     0,
       0,     0,   183,     0,     0,    82,    83,   749,    84,    85,
       0,    86,   184,    88,     0,    33,    34,    35,    36,     0,
      91,     0,     0,     0,     0,   183,     0,   750,    82,     0,
       0,    84,    85,     0,    86,   184,    88,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,     0,   437,     0,     0,     0,     0,   117,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   751,     0,    73,    74,    75,    76,    77,
    1650,     0,     0,     0,     0,     0,   752,     0,     0,     0,
       0,   183,    80,    81,    82,   753,     0,    84,    85,     0,
      86,   184,    88,     0,     0,     0,    90,     0,     0,    29,
      30,     0,     0,     0,     0,   754,   755,   756,   757,    36,
       0,    38,     0,    98,     0,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,     0,   758,     0,     0,     0,     0,    52,     0,     0,
       0,     0,     0,     0,     0,    59,    60,    61,   179,   180,
     181,     0,     0,     0,   896,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,   212,   990,   991,   992,   993,
     994,   995,   996,   997,   998,   999,  1000,  1001,  1002,  1003,
    1004,  1005,   183,     0,     0,    82,    83,     0,    84,    85,
       0,    86,   184,    88,     0,  1006,     0,     0,     0,     0,
      91,     0,     0,     0,   213,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    99,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,     0,    36,   437,     0,     0,   183,     0,   117,    82,
      83,     0,    84,    85,     0,    86,   184,    88,    36,     0,
     212,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     276,   277,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,     0,   214,     0,   213,
       0,     0,   117,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   278,     0,
       0,    84,    85,    36,    86,   184,    88,     0,     0,     0,
       0,   183,     0,     0,    82,    83,     0,    84,    85,     0,
      86,   184,    88,    36,     0,   212,     0,     0,     0,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,     0,   214,     0,   213,   521,     0,   117,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   541,     0,   308,
       0,     0,    84,    85,     0,    86,   184,    88,     0,     0,
       0,     0,     0,     0,     0,     0,   183,     0,     0,    82,
      83,     0,    84,    85,     0,    86,   184,    88,    36,     0,
     212,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,     0,     0,     0,     0,
     309,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,     0,   214,     0,   213,
       0,     0,   117,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1040,    36,     0,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   183,     0,     0,    82,    83,     0,    84,    85,     0,
      86,   184,    88,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   213,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,    36,
       0,   212,   214,     0,     0,     0,   183,   117,     0,    82,
      83,     0,    84,    85,     0,    86,   184,    88,     0,     0,
       0,    36,     0,   212,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     226,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,     0,   214,     0,     0,
       0,     0,   117,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   183,     0,     0,    82,    83,     0,    84,    85,
       0,    86,   184,    88,    36,     0,   212,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   679,     0,
      84,    85,     0,    86,   184,    88,    36,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,     0,   227,     0,     0,     0,     0,   117,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,     0,     0,     0,     0,   680,     0,
     117,  1416,  1097,  1098,  1099,    36,     0,     0,     0,     0,
       0,   679,     0,    84,    85,     0,    86,   184,    88,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    84,    85,     0,    86,   184,
      88,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,     0,     0,     0,
       0,   712,     0,   117,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,     0,
    1417,     0,     0,     0,    84,    85,     0,    86,   184,    88,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   361,   362,   363,
       0,     0,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   364,     0,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,     0,
     386,   361,   362,   363,     0,     0,     0,     0,     0,     0,
       0,     0,   387,     0,     0,     0,     0,     0,     0,     0,
     364,     0,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,     0,   386,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   387,   361,   362,   363,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   364,   433,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,     0,
     386,   361,   362,   363,     0,     0,     0,     0,     0,     0,
       0,     0,   387,     0,     0,     0,     0,     0,     0,     0,
     364,   443,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,     0,   386,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   387,   361,   362,   363,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   364,   846,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,     0,
     386,   361,   362,   363,     0,     0,     0,     0,     0,     0,
       0,     0,   387,     0,     0,     0,     0,     0,     0,     0,
     364,   884,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,     0,   386,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   387,   361,   362,   363,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   364,   925,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,     0,
     386,   981,   982,   983,     0,     0,     0,     0,     0,     0,
       0,     0,   387,     0,     0,     0,     0,     0,     0,     0,
     984,  1230,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,  1002,
    1003,  1004,  1005,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1006,   981,   982,   983,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   984,  1248,   985,   986,
     987,   988,   989,   990,   991,   992,   993,   994,   995,   996,
     997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,     0,
       0,   981,   982,   983,     0,     0,     0,     0,     0,     0,
       0,     0,  1006,     0,     0,     0,     0,     0,     0,     0,
     984,  1148,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,  1002,
    1003,  1004,  1005,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1006,   981,   982,   983,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   984,  1304,   985,   986,
     987,   988,   989,   990,   991,   992,   993,   994,   995,   996,
     997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,     0,
       0,   981,   982,   983,     0,     0,     0,     0,     0,     0,
       0,     0,  1006,     0,     0,     0,     0,     0,     0,     0,
     984,  1309,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,  1002,
    1003,  1004,  1005,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,     0,     0,  1006,   981,   982,   983,
       0,   738,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    36,     0,     0,     0,     0,   984,  1386,   985,   986,
     987,   988,   989,   990,   991,   992,   993,   994,   995,   996,
     997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1006,  1404,     0,     0,     0,     0,     0,    36,
       0,  1464,     0,     0,     0,   183,  1405,  1406,    82,     0,
       0,    84,    85,    36,    86,   184,    88,     0,     0,     0,
       0,     0,     0,     0,   183,     0,     0,    82,  1407,     0,
      84,    85,     0,    86,  1408,    88,     0,     0,     0,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,     0,     0,  1465,    36,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   183,    36,     0,    82,    83,     0,    84,    85,
       0,    86,   184,    88,     0,     0,     0,     0,     0,     0,
     348,     0,    84,    85,     0,    86,   184,    88,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
      36,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   509,     0,     0,    84,    85,     0,
      86,   184,    88,     0,     0,     0,     0,     0,     0,   513,
       0,     0,    84,    85,     0,    86,   184,    88,     0,     0,
       0,     0,     0,     0,     0,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   278,     0,     0,    84,
      85,     0,    86,   184,    88,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   361,   362,   363,     0,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   729,   364,     0,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,     0,   386,     0,   361,   362,
     363,     0,     0,     0,     0,     0,     0,     0,   387,     0,
       0,     0,     0,     0,     0,     0,     0,   364,   881,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     730,   386,   361,   362,   363,     0,     0,     0,     0,     0,
       0,     0,     0,   387,     0,     0,     0,     0,     0,     0,
       0,   364,     0,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,     0,   386,   981,   982,   983,     0,
       0,     0,     0,     0,     0,     0,     0,   387,     0,     0,
       0,     0,     0,     0,     0,   984,  1314,   985,   986,   987,
     988,   989,   990,   991,   992,   993,   994,   995,   996,   997,
     998,   999,  1000,  1001,  1002,  1003,  1004,  1005,   981,   982,
     983,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1006,     0,     0,     0,     0,     0,   984,     0,   985,
     986,   987,   988,   989,   990,   991,   992,   993,   994,   995,
     996,   997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,
     982,   983,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1006,     0,     0,     0,     0,   984,     0,
     985,   986,   987,   988,   989,   990,   991,   992,   993,   994,
     995,   996,   997,   998,   999,  1000,  1001,  1002,  1003,  1004,
    1005,   363,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1006,     0,     0,     0,   364,     0,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   983,   386,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   387,     0,     0,     0,   984,     0,
     985,   986,   987,   988,   989,   990,   991,   992,   993,   994,
     995,   996,   997,   998,   999,  1000,  1001,  1002,  1003,  1004,
    1005,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1006,   986,   987,   988,   989,   990,
     991,   992,   993,   994,   995,   996,   997,   998,   999,  1000,
    1001,  1002,  1003,  1004,  1005,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1006,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,     0,   386,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   387,   987,   988,   989,   990,   991,   992,   993,   994,
     995,   996,   997,   998,   999,  1000,  1001,  1002,  1003,  1004,
    1005,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1006,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,     0,   386,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   387,   988,   989,   990,
     991,   992,   993,   994,   995,   996,   997,   998,   999,  1000,
    1001,  1002,  1003,  1004,  1005,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1006,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,     0,   386,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   387,
     989,   990,   991,   992,   993,   994,   995,   996,   997,   998,
     999,  1000,  1001,  1002,  1003,  1004,  1005,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1006,  -909,  -909,  -909,  -909,   994,   995,   996,   997,   998,
     999,  1000,  1001,  1002,  1003,  1004,  1005,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1006
};

static const yytype_int16 yycheck[] =
{
       5,     6,   163,     8,     9,    10,    11,    12,   136,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,     4,    89,    28,    29,     4,   186,    53,   610,    96,
      97,   175,     4,   316,     4,   858,    32,    42,     4,   582,
     421,   422,    54,   427,    49,   400,    51,   697,    44,    54,
    1064,    56,    48,   228,   166,   839,   386,   581,   453,   187,
     163,   556,   453,   735,   728,     9,  1047,     9,   135,    30,
     451,    57,   234,     4,   877,     9,     9,    30,  1049,    79,
      45,     9,     9,     9,     4,   114,   114,   114,     9,    33,
     893,     9,     9,    79,     9,    66,    82,    45,    45,     9,
    1590,    79,     9,    66,     9,    79,    30,     4,     9,   114,
       9,     9,    66,     9,   201,    66,     9,    79,   100,   101,
      50,    79,     9,   235,     9,     0,     9,    79,  1060,   153,
      50,   100,   101,    53,   128,   129,    85,    66,     9,   301,
       9,    79,    79,     9,     9,    35,   938,   149,    45,   149,
      70,   201,   128,   129,  1644,   201,    79,   128,   129,    45,
     255,   256,     8,    66,    45,   201,   261,    87,   170,    89,
      46,    47,   316,   116,    85,    66,    96,    97,   201,   203,
     204,   124,   128,   129,   163,   214,   191,   214,    66,    79,
     974,    71,    72,   353,   146,   147,   148,   146,   227,   128,
     129,   163,   204,     4,    66,    66,    66,    66,   202,   106,
      66,    66,    66,    66,   111,   135,   113,   114,   115,   116,
     117,   118,   119,   205,   154,   163,   163,   224,   206,   205,
      30,   236,   206,   204,   239,   146,   205,   202,   201,   202,
     163,   246,   247,   204,   171,    66,   341,   201,   206,   393,
     203,   199,   199,   204,   171,   202,   202,   240,   202,   156,
     157,   244,   159,   205,   204,   294,   294,   294,   202,   429,
     203,   399,  1253,   417,  1245,   203,   948,   203,   950,   199,
     347,  1252,   203,  1254,    66,   203,   183,   202,  1091,   294,
     276,   277,   278,   203,   204,   300,   203,   441,   203,   304,
     171,   204,   203,    66,   203,   203,   450,   203,   205,   453,
     203,  1243,   317,   318,   319,   202,   202,   202,   344,   202,
     240,   202,   308,   201,   244,  1109,   204,   332,   248,   396,
     397,   874,    66,   202,   829,   136,   202,   202,   343,  1131,
     201,   154,   204,   204,   204,   204,   266,    79,   204,   204,
     204,   204,   331,    97,   109,   204,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   476,   387,   204,   389,   390,   187,   392,   400,    79,
      35,    35,  1363,    97,   325,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   437,   153,
     437,   331,   167,   418,   419,   149,   421,   422,   423,   424,
     340,  1353,   204,  1355,   344,   430,   588,   347,   433,   201,
      79,   155,   816,   201,    79,    79,    85,   598,   443,   436,
     445,   204,     4,    50,    35,    35,   451,   459,   201,   153,
      35,    97,   128,   129,   459,    97,   461,   147,   148,    97,
    1110,   201,   386,  1127,    26,    27,   198,   862,    97,   393,
     204,   862,   204,   393,   394,   395,   396,   397,   653,     4,
     204,   464,   487,   578,   579,   490,   491,   492,    79,    79,
      79,   201,   587,   417,    79,   598,    85,   417,   147,   148,
     660,   113,   114,   115,   116,   117,   118,   153,   201,   890,
     685,   153,  1444,    53,  1298,   153,   521,   441,   163,   163,
      45,   441,    14,   509,   153,    65,   450,   513,   703,   453,
     331,   926,   518,   453,  1077,   926,    79,  1080,    30,  1063,
     702,   201,    85,   149,   464,    79,   708,   154,    79,   616,
     155,    85,   114,   117,    85,    47,   839,   153,   147,   148,
     124,   210,   169,   168,   170,    29,   896,    71,    72,   566,
     170,   183,   163,   163,    79,   170,   573,    79,   201,   576,
      85,   106,    46,    85,   759,    49,   111,  1371,   113,   114,
     115,   116,   117,   118,   119,   201,   708,   398,   204,   204,
     520,   201,   390,   608,   147,   148,   201,   126,   127,  1259,
     203,  1261,   146,   147,   148,   620,   147,   148,   201,   598,
     209,    99,   100,   101,   170,   545,   546,    99,   100,   101,
     418,   156,   157,   170,   159,   423,   113,   114,   115,   116,
     117,   118,   147,   148,   146,   147,   148,   652,   743,   744,
     302,   582,   214,   650,   306,   201,   661,    29,   183,   221,
     105,  1593,   203,   204,   201,   227,  1620,  1621,   113,   114,
     115,   116,   117,   118,    46,   680,    35,    49,   240,   201,
     205,   333,   244,   335,   336,   337,   338,  1042,   113,   114,
     115,   974,   118,   119,   120,   839,   616,    46,    47,    48,
      49,    50,    51,   201,  1247,  1086,   183,   712,  1232,    49,
      50,    51,  1726,    53,  1095,   201,    65,   204,   862,  1369,
     203,   283,   203,   728,   203,    65,   203,  1741,   724,   203,
     292,   293,   294,   203,   204,   830,   203,   299,   183,   718,
      49,    50,    51,   305,   741,    66,  1130,    74,    75,    76,
     733,  1616,  1617,   743,   744,    66,    65,   203,   853,    86,
      66,   149,   201,   201,   769,    66,   149,   201,   153,   331,
     865,    46,    47,    48,    49,    50,    51,   697,    53,   699,
     203,    44,   926,    65,   184,   185,   186,   149,  1720,   170,
      65,   191,   192,   201,   208,   195,   196,     9,   718,   149,
     201,  1451,   788,  1735,   783,   149,   792,   134,   135,   136,
     137,   138,   732,   733,     8,   170,   203,   201,   145,    14,
     825,  1364,    14,    79,   151,   152,  1109,   203,   895,  1213,
     974,   124,   203,   838,   124,    14,   202,   170,   165,   844,
      14,   846,    97,   848,   202,   828,   202,   207,   202,   828,
     201,   105,     9,   201,   201,   182,   828,   952,   828,   954,
     202,   202,   828,   783,    89,   427,   871,     9,  1249,   789,
      14,   203,  1695,   793,   794,   437,   881,   856,   201,   884,
       9,   886,   187,    79,   201,   890,    79,   190,    79,   203,
    1713,     9,     9,   813,    79,  1070,   203,   828,  1721,   203,
     202,   201,   464,   202,   202,   126,   202,    66,   828,    30,
     127,   169,   130,   202,     9,   457,   149,   718,   202,     9,
     925,   202,   842,   920,     9,   202,    14,   858,   199,   113,
     114,   115,   116,   117,   118,   931,   856,     9,   862,    66,
     124,   125,   862,   874,     9,  1595,     9,   171,    14,   932,
     202,   126,   205,     9,   201,   934,   202,   936,   208,   208,
     208,   202,   201,   505,   208,  1109,   963,  1062,   202,  1064,
    1351,    97,   896,   970,   203,   895,   160,   203,   162,   130,
     149,     9,   783,   202,   149,   201,   201,   907,   908,   909,
     201,   201,   201,   149,  1089,    14,   204,   187,   187,   183,
       9,    79,   926,  1008,  1009,  1010,   926,   204,    14,  1014,
    1015,    14,   932,    14,   934,  1298,   936,   113,   114,   115,
     116,   117,   118,   203,   201,   208,   204,   828,   124,   125,
    1042,   204,   203,   199,    30,   955,    30,  1042,   201,   201,
      14,   583,     9,   201,   130,   607,   201,     9,   202,    14,
    1145,   203,   202,   973,   203,   856,  1638,    65,   201,   979,
     208,     9,   146,  1046,    79,    97,   162,  1046,  1073,     9,
     201,   130,    14,   203,  1046,    79,  1046,   202,   201,   130,
    1046,  1086,   204,   202,   204,   201,     9,   183,  1371,   204,
    1095,  1096,   146,    30,   208,    73,   203,   202,   171,   203,
     662,   130,  1022,    30,   202,  1025,   130,     9,  1105,   202,
     202,   202,     9,   202,  1209,  1046,     9,     9,   205,   202,
     202,    79,  1127,  1119,   205,   204,  1046,    14,   202,  1711,
     201,   201,  1137,   934,   202,   936,     9,   204,   202,  1122,
     202,   202,   704,   130,   706,   202,  1077,    30,   203,  1080,
     202,   204,   202,    97,  1298,  1134,   718,   203,  1155,   203,
     158,   154,  1159,    14,    79,  1162,   111,   202,   730,   202,
     204,   733,  1169,  1334,   130,   202,   130,    14,   720,   170,
      79,   204,   203,   725,    14,    79,  1106,   204,   202,   201,
    1110,   202,   130,   203,    14,  1290,   203,  1292,    14,    14,
      55,   203,  1122,    79,  1332,   204,   201,    79,     9,    79,
     149,   203,    97,   109,  1134,    97,    33,    14,  1223,   201,
     161,   783,  1227,   202,  1229,  1230,   203,  1371,   201,    79,
     164,   167,  1211,  1238,   202,     9,   171,  1332,     4,   801,
      79,  1220,   203,  1248,  1249,  1046,    14,   202,    77,   202,
      79,    80,   204,    79,   816,   817,    14,    79,    14,    14,
      79,    79,  1422,   792,   788,  1703,   828,   518,   894,   397,
    1267,   891,   396,   831,  1717,  1456,  1128,  1284,  1713,    45,
    1211,   394,  1447,   523,  1477,  1561,  1327,  1745,  1208,  1220,
      42,  1733,  1573,  1443,   856,  1017,  1322,    26,    27,   400,
    1014,    30,  1050,  1398,   780,   971,   494,   494,  1093,   908,
    1315,  1106,   922,   862,   300,   977,  1247,   293,  1033,   319,
     956,   743,    -1,    -1,    53,    -1,    -1,   156,   157,    -1,
     159,   160,   161,  1134,    -1,    -1,    -1,    -1,  1317,  1259,
     106,  1261,    -1,  1348,    -1,   111,  1351,   113,   114,   115,
     116,   117,   118,   119,    -1,  1334,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
     932,    -1,   934,    -1,   936,  1354,   938,   939,    -1,    -1,
    1475,  1360,    -1,  1362,    -1,    -1,    -1,    -1,    -1,    -1,
     156,   157,    -1,   159,    -1,  1374,  1379,  1317,  1424,    -1,
      -1,    -1,  1322,   945,  1383,    -1,    -1,  1327,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1575,    -1,   183,    -1,    -1,
      -1,    -1,    -1,  1354,    -1,    -1,    -1,    -1,    -1,  1360,
    1435,  1362,    -1,  1364,    -1,    -1,    -1,    -1,    -1,   205,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1369,
      -1,    -1,    -1,    -1,  1374,    -1,    -1,    -1,    -1,  1379,
      -1,  1466,    -1,  1383,    -1,    -1,    -1,    -1,    -1,    -1,
    1449,    -1,  1455,  1456,    -1,  1454,    -1,    -1,    -1,    -1,
      -1,  1460,    -1,    -1,  1046,   214,    -1,    -1,  1467,    -1,
      -1,  1411,   221,    -1,    -1,    -1,    -1,  1417,   227,    -1,
      -1,    -1,    -1,    -1,  1424,  1572,    -1,    -1,  1428,    -1,
    1072,    -1,    -1,    -1,    -1,    -1,  1317,    -1,  1449,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   255,   256,    -1,  1690,
    1331,  1451,   261,    -1,  1454,  1455,  1456,    -1,    -1,    -1,
    1460,    -1,    -1,    -1,    -1,    -1,    -1,  1467,    -1,    -1,
      -1,    -1,  1114,    -1,   283,    -1,    -1,    -1,    -1,    -1,
    1122,  1656,    -1,   292,   293,  1632,    -1,    -1,  1130,  1131,
     299,    -1,  1134,  1374,    -1,  1580,   305,    -1,    -1,  1707,
      -1,    -1,  1383,    -1,    -1,    -1,    -1,   316,    -1,    -1,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   341,    -1,    -1,   344,    -1,    -1,    -1,    -1,
      -1,  1600,    -1,    -1,    -1,  1426,    -1,    -1,     4,    -1,
      -1,  1726,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,
      64,    -1,    -1,    -1,    -1,    -1,  1741,    -1,    -1,    -1,
      -1,  1213,  1572,  1454,    -1,    -1,    -1,   386,    -1,  1460,
    1639,  1640,  1204,    -1,    -1,    -1,  1467,  1646,    -1,    45,
      -1,  1591,    -1,    -1,    -1,  1595,    -1,    -1,    -1,    -1,
    1600,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1610,    -1,    -1,    -1,    -1,    -1,  1616,  1617,   427,    -1,
    1620,  1621,    -1,  1682,   128,   129,    -1,    -1,   437,    -1,
      -1,  1690,  1632,    -1,    -1,    -1,    -1,    -1,    -1,  1639,
    1640,    -1,    -1,    -1,    -1,    -1,  1646,    -1,    -1,  1271,
     106,    -1,    -1,    -1,    -1,   111,    -1,   113,   114,   115,
     116,   117,   118,   119,    -1,  1287,  1751,   476,   477,    -1,
      -1,   480,    -1,    -1,  1759,  1317,    -1,    -1,    -1,     4,
    1765,    -1,  1682,  1768,  1695,    -1,    -1,    -1,  1747,  1689,
      -1,    -1,    -1,     4,    -1,  1754,    -1,    -1,   202,    -1,
     156,   157,  1713,   159,    -1,    -1,  1706,    -1,    -1,    -1,
    1721,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   527,  1600,
      45,    -1,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,
      -1,    -1,  1374,    -1,    45,    -1,    -1,  1379,    -1,    -1,
      -1,  1383,    -1,    -1,    -1,    -1,    -1,  1747,    -1,   205,
      -1,  1373,    -1,    -1,  1754,    -1,    -1,    -1,  1639,  1640,
    1382,    -1,    -1,    -1,    -1,  1646,    -1,    -1,    -1,   578,
     579,    -1,    -1,    -1,  1396,    -1,    -1,    -1,   587,    -1,
      -1,   106,    -1,    -1,    -1,    -1,   111,    -1,   113,   114,
     115,   116,   117,   118,   119,   106,    -1,    -1,   607,    -1,
     111,  1682,   113,   114,   115,   116,   117,   118,   119,    -1,
      -1,    -1,  1454,  1455,  1456,    -1,    -1,    -1,  1460,    -1,
      -1,    -1,    -1,    29,    -1,  1467,    -1,    -1,    -1,    -1,
      -1,   156,   157,    -1,   159,  1457,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   156,   157,    -1,   159,    55,
      -1,    -1,    -1,   662,    -1,    -1,    -1,    -1,   183,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1747,    -1,    -1,    -1,
      -1,    77,   183,  1754,    -1,    -1,    -1,    10,    11,    12,
     205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   205,   704,    29,   706,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,   730,   731,    -1,    -1,    -1,   132,   133,    -1,    -1,
      -1,    -1,    65,    -1,   743,   744,   745,   746,   747,   748,
     749,    -1,    -1,    -1,   150,    -1,    -1,   153,   154,   758,
     156,   157,    -1,   159,   160,   161,    -1,    -1,  1600,    -1,
      -1,    -1,    -1,  1585,    -1,  1587,    -1,    -1,   174,    -1,
      -1,    -1,    -1,    -1,  1596,   784,    -1,    -1,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   801,    -1,    -1,   201,    -1,  1639,  1640,   205,
      -1,    -1,    -1,    -1,  1646,   814,    -1,   816,   817,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1641,
      -1,   830,   831,    -1,    -1,    26,    27,    -1,    -1,    30,
     839,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1682,    -1,    -1,    -1,   853,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   865,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   873,    -1,    -1,   876,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   208,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   896,    -1,    -1,
      -1,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,  1747,    -1,    -1,    -1,    -1,
      -1,    -1,  1754,    -1,    -1,    -1,    -1,  1739,    -1,    -1,
      -1,    -1,    -1,    -1,  1746,    -1,    -1,    -1,    -1,   938,
     939,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    63,    64,   952,    -1,   954,    -1,   956,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   971,    -1,    -1,   974,   975,   976,   977,    -1,
      -1,   980,   981,   982,   983,   984,   985,   986,   987,   988,
     989,   990,   991,   992,   993,   994,   995,   996,   997,   998,
     999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     221,  1030,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,  1062,    -1,  1064,    -1,    -1,    65,    -1,
      -1,    -1,    -1,  1072,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
    1089,    53,   283,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   292,   293,    65,    -1,    -1,    -1,    -1,   299,    -1,
    1109,    -1,    -1,    -1,   305,  1114,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   316,    -1,    -1,    -1,    -1,
      -1,  1130,  1131,    -1,  1133,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1145,    -1,    -1,  1148,
      53,  1150,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1173,    -1,    -1,    65,    -1,    -1,
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,   386,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,   205,    -1,
    1209,  1210,    -1,    -1,  1213,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      63,    64,    -1,    -1,    -1,    -1,   427,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,    27,    -1,    -1,    30,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,   480,
      -1,  1290,    -1,  1292,    65,   128,   129,    -1,  1297,  1298,
      -1,    -1,    -1,  1302,    -1,  1304,    -1,  1306,    -1,    -1,
    1309,    -1,  1311,    -1,    -1,  1314,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1322,  1323,    -1,  1325,    -1,    -1,    -1,
      -1,    -1,    -1,  1332,    -1,    -1,   527,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   255,   256,    -1,    -1,    -1,    -1,   261,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   202,
      -1,    -1,  1371,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1386,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1398,
    1399,  1400,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,   607,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1424,    -1,    -1,    -1,  1428,
      -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,   341,    -1,
     214,   344,    -1,    -1,    -1,    -1,    -1,   221,    -1,    -1,
      -1,    63,    64,   227,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1463,  1464,  1465,    -1,    -1,    -1,
      -1,   662,  1471,  1472,    -1,    -1,  1475,    -1,  1477,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    79,    -1,
     480,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   283,
      -1,    -1,    -1,   704,    -1,   706,   128,   129,   292,   293,
      -1,    -1,    -1,    -1,    -1,   299,    -1,   118,    -1,    -1,
      -1,   305,    -1,    -1,    -1,    -1,    -1,   527,    -1,   730,
     731,    -1,   316,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   745,   746,   747,   748,   749,   150,
      -1,    -1,   153,   154,    -1,   156,   157,   758,   159,   160,
     161,    -1,    -1,   476,   477,    -1,    -1,   480,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1591,   784,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,    -1,
     801,  1610,   386,    -1,    -1,   206,  1615,    -1,    -1,    -1,
      -1,    -1,    -1,   814,   527,   816,   817,  1626,    -1,    -1,
      -1,    -1,  1631,    -1,    -1,  1634,    -1,    -1,    -1,    -1,
     831,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   839,    -1,
      -1,    -1,    -1,   427,    -1,    -1,    -1,  1656,    -1,    -1,
      -1,    -1,    -1,   437,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   578,   579,    -1,    -1,    -1,
      -1,    -1,   873,    -1,   587,   876,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1694,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1702,    -1,   896,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1717,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1726,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   731,  1741,    -1,    -1,    -1,    -1,   938,   939,    -1,
      -1,    -1,    -1,    -1,    -1,   745,   746,   747,   748,   749,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   758,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     971,    -1,    -1,   974,   975,   976,   977,    -1,    -1,   980,
     981,   982,   983,   984,   985,   986,   987,   988,   989,   990,
     991,   992,   993,   994,   995,   996,   997,   998,   999,  1000,
    1001,  1002,  1003,  1004,  1005,  1006,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   731,    -1,
      -1,    -1,    -1,   607,    -1,    10,    11,    12,    -1,  1030,
     743,   744,   745,   746,   747,   748,   749,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,   758,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,  1072,    -1,   873,    -1,    -1,    -1,    -1,   662,    29,
      65,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,    -1,  1109,    -1,
      -1,    -1,    -1,  1114,    -1,    65,    -1,   830,    -1,    -1,
     704,    -1,   706,    -1,    -1,    -1,    -1,    -1,    -1,  1130,
    1131,    -1,  1133,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     853,    -1,    -1,    -1,    -1,    -1,   730,  1148,    -1,  1150,
      -1,    -1,   865,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     873,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   971,  1173,    -1,    -1,   975,   976,   977,    -1,    -1,
     980,   981,   982,   983,   984,   985,   986,   987,   988,   989,
     990,   991,   992,   993,   994,   995,   996,   997,   998,   999,
    1000,  1001,  1002,  1003,  1004,  1005,  1006,    -1,    -1,  1210,
      -1,    -1,  1213,    -1,    -1,    -1,    -1,   801,    -1,    -1,
     205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1030,    -1,   816,   817,    -1,    -1,    -1,    -1,    -1,   952,
      -1,   954,    -1,   956,    -1,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    -1,    -1,   839,    35,    -1,   971,    -1,
      -1,    -1,   975,   976,   977,    -1,    -1,   980,   981,   982,
     983,   984,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,  1002,
    1003,  1004,  1005,  1006,    -1,    -1,  1297,  1298,    77,    -1,
      79,  1302,    -1,  1304,    -1,  1306,    -1,    -1,  1309,    -1,
    1311,    -1,   896,  1314,    -1,    -1,    -1,  1030,    -1,    -1,
      -1,    -1,  1323,    -1,  1325,   154,    -1,   156,   157,   158,
     159,   160,   161,  1133,    -1,    -1,    -1,    -1,    -1,   118,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1148,  1062,
    1150,  1064,   131,    -1,   938,   939,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
    1371,   150,   201,  1173,   153,   154,  1089,   156,   157,    -1,
     159,   160,   161,    -1,    -1,  1386,    -1,    -1,    -1,    -1,
     974,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1399,  1400,
      -1,    -1,    -1,    -1,    -1,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
    1133,    -1,   201,    -1,    -1,    -1,    -1,   206,    -1,    -1,
      -1,    -1,  1145,    -1,    -1,  1148,    -1,  1150,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   480,    -1,    -1,    -1,    -1,    -1,    -1,
    1173,    -1,  1463,  1464,  1465,    -1,    -1,    -1,    -1,    -1,
    1471,  1472,    -1,    -1,    -1,    -1,  1477,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1072,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1209,  1297,    -1,    -1,
     527,    -1,  1302,    -1,  1304,    -1,  1306,    -1,    -1,  1309,
      -1,  1311,    -1,    -1,  1314,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1109,    -1,    -1,    -1,   480,
    1114,    -1,    10,    11,    12,    26,    27,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1130,  1131,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,   527,  1290,    -1,  1292,
      -1,    -1,    -1,    -1,  1297,    -1,  1386,    65,    -1,  1302,
      -1,  1304,    -1,  1306,    -1,    -1,  1309,    -1,  1311,    -1,
      -1,  1314,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1322,
      -1,    -1,    -1,    -1,  1615,    10,    11,    12,    -1,  1332,
      -1,    -1,    -1,    -1,    -1,  1626,    -1,    -1,    -1,  1213,
    1631,    -1,    -1,  1634,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    77,
      -1,    79,    -1,  1463,  1464,  1465,    -1,    -1,    -1,    -1,
      65,  1471,    -1,  1386,    -1,    -1,  1476,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1398,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1694,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1702,    -1,    -1,   731,    -1,   124,    -1,    29,    -1,
      -1,  1424,    -1,    -1,  1298,  1428,  1717,    -1,   745,   746,
     747,   748,   749,   214,    -1,    -1,    -1,   205,    -1,    -1,
     221,   758,    -1,    -1,    55,    -1,   227,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,    -1,
    1463,  1464,  1465,    -1,    -1,    -1,    77,    -1,  1471,    -1,
      -1,    -1,  1475,    -1,    -1,    -1,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     731,    -1,    -1,    -1,    -1,    -1,   204,  1371,   206,    -1,
      -1,    -1,   283,    -1,   745,   746,   747,   748,    -1,    -1,
      -1,   292,   293,   294,    -1,    -1,    -1,   758,   299,    -1,
     205,   132,   133,    -1,   305,  1615,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1626,    -1,    -1,   150,
      -1,  1631,   153,   154,  1634,   156,   157,    -1,   159,   160,
     161,    -1,    -1,    -1,    -1,    -1,   873,    -1,    -1,    -1,
      -1,    -1,    -1,   174,    -1,    -1,    -1,    -1,  1658,    -1,
      -1,    -1,    -1,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,  1591,    -1,
     201,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1694,    -1,    -1,  1610,    -1,    -1,
      -1,    -1,  1615,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1626,    -1,    -1,    -1,    -1,  1631,    -1,
      -1,  1634,   873,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   427,    -1,    -1,    -1,
      -1,    -1,    -1,  1656,   971,    -1,   437,    -1,   975,   976,
     977,    -1,    -1,   980,   981,   982,   983,   984,   985,   986,
     987,   988,   989,   990,   991,   992,   993,   994,   995,   996,
     997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,
      29,  1694,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,  1030,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1726,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   975,   976,   977,    -1,  1741,   980,
     981,   982,   983,   984,   985,   986,   987,   988,   989,   990,
     991,   992,   993,   994,   995,   996,   997,   998,   999,  1000,
    1001,  1002,  1003,  1004,  1005,  1006,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,  1030,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,  1133,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,   607,    -1,    -1,    -1,
      -1,  1148,    -1,  1150,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,  1173,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,   662,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,  1133,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    -1,  1148,    -1,  1150,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,   704,    -1,   706,    -1,    -1,    74,    75,
      76,    77,  1173,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   730,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1297,    -1,    -1,    -1,    -1,  1302,   132,  1304,    -1,  1306,
      -1,    -1,  1309,    -1,  1311,    -1,    -1,  1314,    -1,   145,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,
     801,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,
      -1,   205,    -1,    -1,    -1,   816,   817,    -1,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    10,    11,    12,  1297,    -1,    -1,    -1,
      -1,  1302,    -1,  1304,    -1,  1306,    -1,    -1,  1309,  1386,
    1311,    -1,    29,  1314,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    10,    11,    12,    -1,  1386,  1463,  1464,  1465,    -1,
      -1,    -1,    65,    -1,  1471,    -1,    -1,   938,   939,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,  1463,  1464,  1465,    -1,    -1,    -1,    -1,    -1,
    1471,    10,    11,    12,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   205,    -1,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,  1072,   205,    -1,    -1,    -1,    -1,    -1,  1615,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1626,
      -1,    -1,    -1,    -1,  1631,    -1,    -1,  1634,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1114,    -1,    -1,   205,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1130,
    1131,    -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1615,    -1,    -1,  1694,    -1,    27,
      28,    -1,    -1,    -1,    -1,  1626,    -1,    -1,    -1,    -1,
    1631,    -1,    -1,  1634,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,   203,    -1,    74,    75,    76,    77,
      78,    79,  1213,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    -1,    -1,  1694,   102,   103,   104,   105,   106,   107,
     108,    -1,   110,   111,   112,   113,   114,   115,   116,   117,
     118,    -1,   120,   121,   122,   123,   124,   125,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,
     178,    -1,   180,    -1,   182,   183,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,    -1,    -1,   201,    -1,   203,   204,   205,   206,   207,
      -1,   209,   210,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,    -1,
      -1,    -1,   102,   103,   104,   105,   106,   107,   108,    -1,
     110,   111,   112,   113,   114,   115,   116,   117,   118,    -1,
     120,   121,   122,   123,   124,   125,    -1,    -1,    -1,    -1,
      -1,   131,   132,    -1,   134,   135,   136,   137,   138,    -1,
      -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,
     150,   151,   152,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,   162,    -1,    -1,   165,    -1,    -1,   168,    -1,
      -1,    -1,    -1,    -1,   174,   175,   176,   177,   178,    -1,
     180,    -1,   182,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,    -1,
      -1,   201,    -1,   203,   204,   205,   206,   207,    -1,   209,
     210,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,
     102,   103,   104,   105,   106,   107,   108,    -1,   110,   111,
     112,   113,   114,   115,   116,   117,   118,    -1,   120,   121,
     122,   123,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,
     132,    -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,
      -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
     162,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,
      -1,    -1,   174,   175,   176,   177,   178,    -1,   180,    -1,
     182,   183,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,    -1,    -1,   201,
      -1,   203,   204,    -1,   206,   207,    -1,   209,   210,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    82,    -1,
      -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,    93,
      -1,    95,    -1,    -1,    98,    -1,    -1,    -1,   102,   103,
     104,   105,    -1,   107,   108,    -1,   110,    -1,   112,   113,
     114,   115,   116,   117,   118,    -1,   120,   121,   122,    -1,
     124,   125,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,
     134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,
      -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,   162,    -1,
      -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,
     174,   175,   176,   177,    -1,    -1,    -1,    -1,   182,   183,
      -1,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,    -1,    -1,   201,    -1,   203,
     204,   205,   206,   207,    -1,   209,   210,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,
      86,    87,    88,    89,    -1,    91,    -1,    93,    -1,    95,
      -1,    -1,    98,    -1,    -1,    -1,   102,   103,   104,   105,
      -1,   107,   108,    -1,   110,    -1,   112,   113,   114,   115,
     116,   117,   118,    -1,   120,   121,   122,    -1,   124,   125,
      -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,
     136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,
      -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,   162,    -1,    -1,   165,
      -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,
     176,   177,    -1,    -1,    -1,    -1,   182,   183,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,    -1,    -1,   201,    -1,   203,   204,   205,
     206,   207,    -1,   209,   210,     3,     4,     5,     6,     7,
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
     168,    -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,
      -1,    -1,    -1,    -1,   182,   183,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,    -1,    -1,   201,    -1,   203,   204,   205,   206,   207,
      -1,   209,   210,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,    -1,
      -1,    -1,   102,   103,   104,   105,    -1,   107,   108,    -1,
     110,    -1,   112,   113,   114,   115,   116,   117,   118,    -1,
     120,   121,   122,    -1,   124,   125,    -1,    -1,    -1,    -1,
      -1,   131,   132,    -1,   134,   135,   136,   137,   138,    -1,
      -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,
     150,   151,   152,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,   162,    -1,    -1,   165,    -1,    -1,   168,    -1,
      -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,    -1,
      -1,    -1,   182,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,    -1,
      -1,   201,    -1,   203,   204,   205,   206,   207,    -1,   209,
     210,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      82,    -1,    -1,    -1,    86,    87,    88,    89,    90,    91,
      -1,    93,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,
     102,   103,   104,   105,    -1,   107,   108,    -1,   110,    -1,
     112,   113,   114,   115,   116,   117,   118,    -1,   120,   121,
     122,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,
     132,    -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,
      -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
     162,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,
      -1,    -1,   174,   175,   176,   177,    -1,    -1,    -1,    -1,
     182,   183,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,    -1,    -1,   201,
      -1,   203,   204,    -1,   206,   207,    -1,   209,   210,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    82,    -1,
      -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,    93,
      -1,    95,    96,    -1,    98,    -1,    -1,    -1,   102,   103,
     104,   105,    -1,   107,   108,    -1,   110,    -1,   112,   113,
     114,   115,   116,   117,   118,    -1,   120,   121,   122,    -1,
     124,   125,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,
     134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,
      -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,   162,    -1,
      -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,
     174,   175,   176,   177,    -1,    -1,    -1,    -1,   182,   183,
      -1,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,    -1,    -1,   201,    -1,   203,
     204,    -1,   206,   207,    -1,   209,   210,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,
      86,    87,    88,    89,    -1,    91,    -1,    93,    -1,    95,
      -1,    -1,    98,    -1,    -1,    -1,   102,   103,   104,   105,
      -1,   107,   108,    -1,   110,    -1,   112,   113,   114,   115,
     116,   117,   118,    -1,   120,   121,   122,    -1,   124,   125,
      -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,
     136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,
      -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,   162,    -1,    -1,   165,
      -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,
     176,   177,    -1,    -1,    -1,    -1,   182,   183,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,    -1,    -1,   201,    -1,   203,   204,   205,
     206,   207,    -1,   209,   210,     3,     4,     5,     6,     7,
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
     168,    -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,
      -1,    -1,    -1,    -1,   182,   183,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,    -1,    -1,   201,    -1,   203,   204,   205,   206,   207,
      -1,   209,   210,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,    89,
      -1,    91,    -1,    93,    94,    95,    -1,    -1,    98,    -1,
      -1,    -1,   102,   103,   104,   105,    -1,   107,   108,    -1,
     110,    -1,   112,   113,   114,   115,   116,   117,   118,    -1,
     120,   121,   122,    -1,   124,   125,    -1,    -1,    -1,    -1,
      -1,   131,   132,    -1,   134,   135,   136,   137,   138,    -1,
      -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,
     150,   151,   152,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,   162,    -1,    -1,   165,    -1,    -1,   168,    -1,
      -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,    -1,
      -1,    -1,   182,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,    -1,
      -1,   201,    -1,   203,   204,    -1,   206,   207,    -1,   209,
     210,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,
     102,   103,   104,   105,    -1,   107,   108,    -1,   110,    -1,
     112,   113,   114,   115,   116,   117,   118,    -1,   120,   121,
     122,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,
     132,    -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,
      -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
     162,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,
      -1,    -1,   174,   175,   176,   177,    -1,    -1,    -1,    -1,
     182,   183,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,    -1,    -1,   201,
      -1,   203,   204,   205,   206,   207,    -1,   209,   210,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    82,    -1,
      -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,    93,
      -1,    95,    -1,    -1,    98,    -1,    -1,    -1,   102,   103,
     104,   105,    -1,   107,   108,    -1,   110,    -1,   112,   113,
     114,   115,   116,   117,   118,    -1,   120,   121,   122,    -1,
     124,   125,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,
     134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,
      -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,   162,    -1,
      -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,
     174,   175,   176,   177,    -1,    -1,    -1,    -1,   182,   183,
      -1,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,    -1,    -1,   201,    -1,   203,
     204,   205,   206,   207,    -1,   209,   210,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,
      86,    87,    88,    89,    -1,    91,    92,    93,    -1,    95,
      -1,    -1,    98,    -1,    -1,    -1,   102,   103,   104,   105,
      -1,   107,   108,    -1,   110,    -1,   112,   113,   114,   115,
     116,   117,   118,    -1,   120,   121,   122,    -1,   124,   125,
      -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,
     136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,
      -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,   162,    -1,    -1,   165,
      -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,
     176,   177,    -1,    -1,    -1,    -1,   182,   183,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,    -1,    -1,   201,    -1,   203,   204,    -1,
     206,   207,    -1,   209,   210,     3,     4,     5,     6,     7,
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
     168,    -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,
      -1,    -1,    -1,    -1,   182,   183,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,    -1,    -1,   201,    -1,   203,   204,   205,   206,   207,
      -1,   209,   210,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,    -1,
      -1,    -1,   102,   103,   104,   105,    -1,   107,   108,    -1,
     110,    -1,   112,   113,   114,   115,   116,   117,   118,    -1,
     120,   121,   122,    -1,   124,   125,    -1,    -1,    -1,    -1,
      -1,   131,   132,    -1,   134,   135,   136,   137,   138,    -1,
      -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,
     150,   151,   152,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,   162,    -1,    -1,   165,    -1,    -1,   168,    -1,
      -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,    -1,
      -1,    -1,   182,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,    -1,
      -1,   201,    -1,   203,   204,   205,   206,   207,    -1,   209,
     210,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,
     102,   103,   104,   105,    -1,   107,   108,    -1,   110,    -1,
     112,   113,   114,   115,   116,   117,   118,    -1,   120,   121,
     122,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,
     132,    -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,
      -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
     162,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,
      -1,    -1,   174,   175,   176,   177,    -1,    -1,    -1,    -1,
     182,   183,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,    -1,    -1,   201,
      -1,   203,   204,   205,   206,   207,    -1,   209,   210,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    82,    -1,
      -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,    93,
      -1,    95,    -1,    -1,    98,    -1,    -1,    -1,   102,   103,
     104,   105,    -1,   107,   108,    -1,   110,    -1,   112,   113,
     114,   115,   116,   117,   118,    -1,   120,   121,   122,    -1,
     124,   125,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,
     134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,
      -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,   162,    -1,
      -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,
     174,   175,   176,   177,    -1,    -1,    -1,    -1,   182,   183,
      -1,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,    -1,    -1,   201,    -1,   203,
     204,    -1,   206,   207,    -1,   209,   210,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    30,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,
      86,    87,    88,    89,    -1,    91,    -1,    93,    -1,    95,
      -1,    -1,    98,    -1,    -1,    -1,   102,   103,   104,   105,
      -1,   107,   108,    -1,   110,    -1,   112,   113,   114,   115,
     116,   117,   118,    -1,   120,   121,   122,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,
     136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,
      -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,
      -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,
     176,   177,    -1,    -1,    -1,    -1,   182,   183,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,    -1,    -1,   201,    -1,   203,   204,    -1,
     206,   207,    -1,   209,   210,     3,     4,     5,     6,     7,
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
     168,    -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,
      -1,    -1,    -1,    -1,   182,   183,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,    -1,    -1,   201,    -1,   203,   204,    -1,   206,   207,
      -1,   209,   210,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,    -1,
      -1,    -1,   102,   103,   104,   105,    -1,   107,   108,    -1,
     110,    -1,   112,   113,   114,   115,   116,   117,   118,    -1,
     120,   121,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   131,   132,    -1,   134,   135,   136,   137,   138,    -1,
      -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,
     150,   151,   152,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,    -1,    -1,    -1,   165,    -1,    -1,   168,    -1,
      -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,    -1,
      -1,    -1,   182,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,    -1,
      -1,   201,    -1,   203,   204,    -1,   206,   207,    -1,   209,
     210,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    30,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,
     102,   103,   104,   105,    -1,   107,   108,    -1,   110,    -1,
     112,   113,   114,   115,   116,   117,   118,    -1,   120,   121,
     122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,
     132,    -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,
      -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,
      -1,    -1,   174,   175,   176,   177,    -1,    -1,    -1,    -1,
     182,   183,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,    -1,    -1,   201,
      -1,   203,   204,    -1,   206,   207,    -1,   209,   210,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    30,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    82,    -1,
      -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,    93,
      -1,    95,    -1,    -1,    98,    -1,    -1,    -1,   102,   103,
     104,   105,    -1,   107,   108,    -1,   110,    -1,   112,   113,
     114,   115,   116,   117,   118,    -1,   120,   121,   122,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,
     134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,
      -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,
      -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,
     174,   175,   176,   177,    -1,    -1,    -1,    -1,   182,   183,
      -1,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,    -1,    -1,   201,    -1,   203,
     204,    -1,   206,   207,    -1,   209,   210,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,
      86,    87,    88,    89,    -1,    91,    -1,    93,    -1,    95,
      -1,    -1,    98,    -1,    -1,    -1,   102,   103,   104,   105,
      -1,   107,   108,    -1,   110,    -1,   112,   113,   114,   115,
     116,   117,   118,    -1,   120,   121,   122,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,
     136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,
      -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,
      -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,
     176,   177,    -1,    -1,    -1,    -1,   182,   183,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,    -1,    -1,   201,    -1,   203,   204,    -1,
     206,   207,    -1,   209,   210,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,    -1,
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
     168,    -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,
      -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,    -1,    -1,   201,    -1,    -1,    -1,    -1,   206,   207,
      -1,   209,   210,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   113,   114,   115,   116,   117,   118,    -1,
      -1,   121,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   131,   132,    -1,   134,   135,   136,   137,   138,    -1,
      -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,
     150,   151,   152,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,    -1,    -1,    -1,   165,    -1,    -1,   168,    -1,
      -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,    -1,
      -1,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,    -1,
      -1,   201,    -1,   203,    -1,    -1,   206,   207,    -1,   209,
     210,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    35,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    65,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   113,   114,   115,   116,   117,   118,    -1,    -1,   121,
     122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,
     132,    -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,
      -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
      -1,   163,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,
      -1,    -1,   174,   175,   176,   177,    -1,    -1,    -1,    -1,
     182,   183,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,    -1,    -1,   201,
      -1,    -1,    -1,    -1,   206,   207,    -1,   209,   210,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,
     114,   115,   116,   117,   118,    -1,    -1,   121,   122,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,
     134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,
      -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,
      -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,
     174,   175,   176,   177,    -1,    -1,    -1,    -1,   182,   183,
      -1,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,    -1,    -1,   201,    11,    12,
     204,    -1,   206,   207,    -1,   209,   210,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    29,    13,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    35,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    65,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,   114,   115,
     116,   117,   118,    -1,    -1,   121,   122,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,
     136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,
      -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,    -1,   163,    -1,   165,
      -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,
     176,   177,    -1,    -1,    -1,    -1,   182,   183,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,    -1,    -1,   201,    -1,    -1,    -1,    -1,
     206,   207,    -1,   209,   210,     3,     4,     5,     6,     7,
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
     168,     3,     4,     5,     6,     7,   174,   175,   176,   177,
      -1,    13,    -1,    -1,   182,   183,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,    -1,    -1,   201,    -1,    -1,    -1,    -1,   206,   207,
      -1,   209,   210,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     102,    -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   113,   114,   115,   116,   117,   118,    -1,    77,   121,
     122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,
     132,    -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,
      -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,
      -1,    -1,   174,   175,   176,   177,    -1,    -1,    -1,    -1,
     182,   183,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   156,   157,   201,
     159,   160,   161,    -1,   206,   207,    -1,   209,   210,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,    35,   201,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,
     114,   115,   116,   117,   118,    -1,    -1,   121,   122,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,
     134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,
      -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,
      -1,   165,    -1,    -1,   168,     3,     4,     5,     6,     7,
     174,   175,   176,   177,    -1,    13,    -1,    -1,   182,   183,
      -1,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,    -1,    -1,   201,    -1,    -1,
      -1,    -1,   206,   207,    -1,   209,   210,    -1,    46,    47,
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
     168,     3,     4,     5,     6,     7,   174,   175,   176,   177,
      -1,    13,    -1,    -1,   182,   183,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,    -1,    -1,   201,    -1,   203,    -1,    -1,   206,   207,
      -1,   209,   210,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   113,   114,   115,   116,   117,   118,    -1,    -1,   121,
     122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,
     132,    -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,
      -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    -1,   165,    -1,    -1,   168,     3,     4,     5,
       6,     7,   174,   175,   176,   177,    -1,    13,    -1,    -1,
     182,   183,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,    -1,    -1,   201,
      -1,   203,    -1,    -1,   206,   207,    -1,   209,   210,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,   114,   115,
     116,   117,   118,    -1,    -1,   121,   122,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,
     136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,
      -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,
      -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,
     176,   177,    -1,    -1,    -1,    -1,   182,   183,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,    -1,    -1,   201,   202,    -1,    -1,    -1,
     206,   207,    -1,   209,   210,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    30,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,   117,
     118,    -1,    77,   121,   122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,
      -1,    -1,    -1,    -1,   182,   183,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   156,   157,   201,   159,   160,   161,    -1,   206,   207,
      -1,   209,   210,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,    35,   201,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   113,   114,   115,   116,   117,   118,    -1,
      77,   121,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   131,   132,    -1,   134,   135,   136,   137,   138,    -1,
      -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,
     150,   151,   152,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,    -1,    -1,    -1,   165,    -1,   124,   168,    -1,
      -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,    -1,
      -1,    -1,   182,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   156,
     157,   201,   159,   160,   161,    -1,   206,   207,    -1,   209,
     210,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   113,   114,   115,   116,   117,   118,    -1,    77,   121,
     122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,
     132,    -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,
      -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    -1,   165,    -1,   124,   168,    -1,    -1,    -1,
      -1,    -1,   174,   175,   176,   177,    -1,    -1,    -1,    -1,
     182,   183,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   156,   157,   201,
     159,   160,   161,    -1,   206,   207,    -1,   209,   210,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,
     114,   115,   116,   117,   118,    -1,    77,   121,   122,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,
     134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,
      -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,
      -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,
     174,   175,   176,   177,    -1,    -1,    -1,    -1,   182,   183,
      -1,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   156,   157,   201,   159,   160,
     161,    -1,   206,   207,    -1,   209,   210,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,   114,   115,
     116,   117,   118,    -1,    -1,   121,   122,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,
     136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,
      -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,
      -1,    -1,   168,     3,     4,     5,     6,     7,   174,   175,
     176,   177,    -1,    13,    -1,    -1,   182,   183,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,    -1,    -1,   201,    -1,    -1,    -1,    -1,
     206,   207,    -1,   209,   210,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   113,   114,   115,   116,   117,   118,    -1,
      -1,   121,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   131,   132,    -1,   134,   135,   136,   137,   138,    -1,
      -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,
     150,   151,   152,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,    -1,    -1,    -1,   165,    -1,    -1,   168,     3,
       4,     5,     6,     7,   174,   175,   176,   177,    -1,    13,
      -1,    -1,   182,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,    -1,
      -1,   201,    -1,    -1,    -1,    -1,   206,   207,    -1,   209,
     210,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,
     114,   115,   116,   117,   118,    -1,    -1,   121,   122,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,
     134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,
      -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,
      -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,
     174,   175,   176,   177,    -1,    -1,    -1,    -1,   182,   183,
      -1,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,    -1,    -1,   201,    -1,    -1,
      -1,    -1,   206,   207,    -1,   209,   210,     3,     4,     5,
       6,     7,    -1,    -1,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    67,    68,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,    -1,    -1,    -1,   131,   132,    -1,   134,   135,
     136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   150,   151,   152,    -1,    -1,    -1,
     156,   157,    -1,   159,   160,   161,   162,    -1,   164,   165,
      -1,   167,    -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,
      -1,    -1,   178,    -1,   180,    -1,   182,   183,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,   203,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,   203,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,   203,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,   203,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,   203,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,   202,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,   202,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    77,    65,    79,    -1,    -1,
      -1,    -1,   202,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     197,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    29,    53,   156,   157,    -1,   159,   160,   161,
     193,   194,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    -1,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    -1,    -1,    -1,    -1,
      77,   190,   204,    -1,   206,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,    -1,    -1,    55,   189,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   132,   133,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    -1,    -1,   150,    -1,    -1,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,   188,    -1,
      -1,    -1,    -1,    -1,   105,    -1,    -1,   174,    -1,    29,
      -1,    -1,    -1,    -1,    -1,    -1,   183,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   132,   133,    -1,   201,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   150,
      -1,    -1,   153,   154,    -1,   156,   157,    77,   159,   160,
     161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   174,    -1,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,    -1,
     201,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,
     150,    -1,    -1,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,    -1,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   174,    -1,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   132,   133,
      -1,   201,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   150,    -1,    -1,   153,
     154,    -1,   156,   157,    77,   159,   160,   161,    -1,   163,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      -1,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    46,    47,   201,    -1,    -1,
      -1,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,   132,
     133,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,   150,    -1,    -1,
     153,   154,    -1,   156,   157,    86,   159,   160,   161,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,    -1,   201,    67,
      68,   132,    -1,   134,   135,   136,   137,   138,    -1,    77,
      -1,    79,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,
     151,   152,   153,   154,    -1,   156,   157,    -1,   159,   160,
     161,    -1,    77,    -1,   165,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   174,   175,   176,   177,    -1,    -1,    -1,
     118,   182,    -1,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    46,    47,    -1,
     201,    -1,    -1,    52,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,   150,    -1,    -1,   153,   154,    66,   156,   157,
      -1,   159,   160,   161,    -1,    74,    75,    76,    77,    -1,
     168,    -1,    -1,    -1,    -1,   150,    -1,    86,   153,    -1,
      -1,   156,   157,    -1,   159,   160,   161,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,   206,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   132,    -1,   134,   135,   136,   137,   138,
     205,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,
      -1,   150,   151,   152,   153,   154,    -1,   156,   157,    -1,
     159,   160,   161,    -1,    -1,    -1,   165,    -1,    -1,    67,
      68,    -1,    -1,    -1,    -1,   174,   175,   176,   177,    77,
      -1,    79,    -1,   182,    -1,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,    -1,   201,    -1,    -1,    -1,    -1,   105,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,   117,
     118,    -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    79,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,   150,    -1,    -1,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    65,    -1,    -1,    -1,    -1,
     168,    -1,    -1,    -1,   118,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   183,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,    -1,    77,   201,    -1,    -1,   150,    -1,   206,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,    77,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     105,   106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,    -1,   201,    -1,   118,
      -1,    -1,   206,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   153,    -1,
      -1,   156,   157,    77,   159,   160,   161,    -1,    -1,    -1,
      -1,   150,    -1,    -1,   153,   154,    -1,   156,   157,    -1,
     159,   160,   161,    77,    -1,    79,    -1,    -1,    -1,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,    -1,   201,    -1,   118,   204,    -1,   206,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,    -1,   153,
      -1,    -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   150,    -1,    -1,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,    77,    -1,
      79,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,    -1,    -1,    -1,    -1,
     204,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,    -1,   201,    -1,   118,
      -1,    -1,   206,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   131,    77,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   150,    -1,    -1,   153,   154,    -1,   156,   157,    -1,
     159,   160,   161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   118,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    77,
      -1,    79,   201,    -1,    -1,    -1,   150,   206,    -1,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,
      -1,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     118,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,    -1,   201,    -1,    -1,
      -1,    -1,   206,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   150,    -1,    -1,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    77,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,
     156,   157,    -1,   159,   160,   161,    77,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,   206,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    -1,    -1,    -1,    -1,    -1,   204,    -1,
     206,   122,    74,    75,    76,    77,    -1,    -1,    -1,    -1,
      -1,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   156,   157,    -1,   159,   160,
     161,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,    -1,    -1,    -1,
      -1,   204,    -1,   206,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,    -1,
     201,    -1,    -1,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,   130,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,   130,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,   130,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,   130,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,   130,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,   130,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,   130,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,   130,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,   130,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,   130,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    65,    10,    11,    12,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    29,   130,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,   119,    -1,    -1,    -1,    -1,    -1,    77,
      -1,   130,    -1,    -1,    -1,   150,   132,   133,   153,    -1,
      -1,   156,   157,    77,   159,   160,   161,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   150,    -1,    -1,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,    -1,    -1,   130,    77,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   150,    77,    -1,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,    -1,
     154,    -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      77,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   153,    -1,    -1,   156,   157,    -1,
     159,   160,   161,    -1,    -1,    -1,    -1,    -1,    -1,   153,
      -1,    -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,   153,    -1,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    28,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      97,    53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    12,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   212,   213,     0,   214,     3,     4,     5,     6,     7,
      13,    27,    28,    45,    46,    47,    52,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    66,    67,
      68,    69,    70,    74,    75,    76,    77,    78,    79,    81,
      82,    86,    87,    88,    89,    91,    93,    95,    98,   102,
     103,   104,   105,   106,   107,   108,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   120,   121,   122,   123,   124,
     125,   131,   132,   134,   135,   136,   137,   138,   145,   150,
     151,   152,   153,   154,   156,   157,   159,   160,   161,   162,
     165,   168,   174,   175,   176,   177,   178,   180,   182,   183,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   201,   203,   204,   206,   207,   209,
     210,   215,   218,   225,   226,   227,   228,   229,   230,   233,
     249,   250,   254,   257,   262,   268,   326,   327,   332,   336,
     337,   338,   339,   340,   341,   342,   343,   345,   348,   360,
     361,   362,   363,   364,   368,   369,   371,   390,   400,   401,
     402,   407,   410,   428,   436,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   449,   470,   472,   474,   116,
     117,   118,   131,   150,   160,   218,   249,   326,   342,   438,
     342,   201,   342,   342,   342,   102,   342,   342,   426,   427,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,    79,   118,   201,   226,   401,   402,   438,   441,
     438,    35,   342,   453,   454,   342,   118,   201,   226,   401,
     402,   403,   437,   445,   450,   451,   201,   333,   404,   201,
     333,   349,   334,   342,   235,   333,   201,   201,   201,   333,
     203,   342,   218,   203,   342,    29,    55,   132,   133,   154,
     174,   201,   218,   229,   475,   487,   488,   184,   203,   339,
     342,   370,   372,   204,   242,   342,   105,   106,   153,   219,
     222,   225,    79,   206,   294,   295,   117,   124,   116,   124,
      79,   296,   201,   201,   201,   201,   218,   266,   476,   201,
     201,    79,    85,   146,   147,   148,   467,   468,   153,   204,
     225,   225,   218,   267,   476,   154,   201,   201,   201,   201,
     476,   476,    79,   198,   204,   351,   332,   342,   343,   438,
     442,   231,   204,    85,   405,   467,    85,   467,   467,    30,
     153,   170,   477,   201,     9,   203,    35,   248,   154,   265,
     476,   118,   183,   249,   327,   203,   203,   203,   203,   203,
     203,    10,    11,    12,    29,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    53,    65,   203,    66,
      66,   203,   204,   149,   125,   160,   162,   268,   325,   326,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    63,    64,   128,   129,   430,    66,   204,
     435,   201,   201,    66,   204,   206,   446,   201,   248,   249,
      14,   342,   203,   130,    44,   218,   425,   201,   332,   438,
     442,   149,   438,   130,   208,     9,   412,   332,   438,   477,
     149,   201,   406,   430,   435,   202,   342,    30,   233,     8,
     354,     9,   203,   233,   234,   334,   335,   342,   218,   280,
     237,   203,   203,   203,   488,   488,   170,   201,   105,   488,
      14,   218,    79,   203,   203,   203,   184,   185,   186,   191,
     192,   195,   196,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   385,   386,   387,   243,   109,   167,   203,   153,
     220,   223,   225,   153,   221,   224,   225,   225,     9,   203,
      97,   204,   438,     9,   203,   124,   124,    14,     9,   203,
     438,   471,   471,   332,   343,   438,   441,   442,   202,   170,
     260,   131,   438,   452,   453,    66,   430,   146,   468,    78,
     342,   438,    85,   146,   468,   225,   217,   203,   204,   255,
     263,   391,   393,    86,   206,   355,   356,   358,   402,   446,
     472,   342,   460,   462,   342,   459,   461,   459,    14,    97,
     473,   350,   352,   353,   290,   291,   428,   429,   202,   202,
     202,   202,   205,   232,   233,   250,   257,   262,   428,   342,
     207,   209,   210,   218,   478,   479,   488,    35,   163,   292,
     293,   342,   475,   201,   476,   258,   248,   342,   342,   342,
      30,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   403,   342,   342,   448,   448,   342,   455,
     456,   124,   204,   218,   445,   446,   266,   218,   267,   265,
     249,    27,    35,   336,   339,   342,   370,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   154,
     204,   218,   431,   432,   433,   434,   445,   448,   342,   292,
     292,   448,   342,   452,   248,   202,   342,   201,   424,     9,
     412,   332,   202,   218,    35,   342,    35,   342,   202,   202,
     445,   292,   204,   218,   431,   432,   445,   202,   231,   284,
     204,   339,   342,   342,    89,    30,   233,   278,   203,    28,
      97,    14,     9,   202,    30,   204,   281,   488,    86,   229,
     484,   485,   486,   201,     9,    46,    47,    52,    54,    66,
      86,   132,   145,   154,   174,   175,   176,   177,   201,   226,
     227,   229,   365,   366,   367,   401,   407,   408,   409,   187,
      79,   342,    79,    79,   342,   382,   383,   342,   342,   375,
     385,   190,   388,   231,   201,   241,   225,   203,     9,    97,
     225,   203,     9,    97,    97,   222,   218,   342,   295,   408,
      79,     9,   202,   202,   202,   202,   202,   202,   202,   203,
      46,    47,   482,   483,   126,   271,   201,     9,   202,   202,
      79,    80,   218,   469,   218,    66,   205,   205,   214,   216,
      30,   127,   270,   169,    50,   154,   169,   395,   130,     9,
     412,   202,   149,   202,     9,   412,   130,   202,     9,   412,
     202,   488,   488,    14,   354,   290,   231,   199,     9,   413,
     488,   489,   430,   435,   205,     9,   412,   171,   438,   342,
     202,     9,   413,    14,   346,   251,   126,   269,   201,   476,
     342,    30,   208,   208,   130,   205,     9,   412,   342,   477,
     201,   261,   256,   264,   259,   248,    68,   438,   342,   477,
     208,   205,   202,   202,   208,   205,   202,    46,    47,    66,
      74,    75,    76,    86,   132,   145,   174,   218,   415,   417,
     420,   423,   218,   438,   438,   130,   430,   435,   202,   342,
     285,    71,    72,   286,   231,   333,   231,   335,    97,    35,
     131,   275,   438,   408,   218,    30,   233,   279,   203,   282,
     203,   282,     9,   171,   130,   149,     9,   412,   202,   163,
     478,   479,   480,   478,   408,   408,   408,   408,   408,   411,
     414,   201,    85,   149,   201,   201,   201,   201,   408,   149,
     204,    10,    11,    12,    29,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    65,   342,   187,   187,
      14,   193,   194,   384,     9,   197,   388,    79,   205,   401,
     204,   245,    97,   223,   218,    97,   224,   218,   218,   205,
      14,   438,   203,     9,   171,   218,   272,   401,   204,   452,
     131,   438,    14,   208,   342,   205,   214,   488,   272,   204,
     394,    14,   342,   355,   218,   342,   342,   342,   203,   488,
     199,   205,    30,   481,   429,    35,    79,   163,   431,   432,
     434,   488,    35,   163,   342,   408,   290,   201,   401,   270,
     347,   252,   342,   342,   342,   205,   201,   292,   271,    30,
     270,   269,   476,   403,   205,   201,    14,    74,    75,    76,
     218,   416,   416,   417,   418,   419,   201,    85,   146,   201,
       9,   412,   202,   424,    35,   342,   431,   432,   205,    71,
      72,   287,   333,   233,   205,   203,    90,   203,   275,   438,
     201,   130,   274,    14,   231,   282,    99,   100,   101,   282,
     205,   488,   488,   218,   484,     9,   202,   412,   130,   208,
       9,   412,   411,   218,   355,   357,   359,   408,   464,   466,
     408,   463,   465,   463,   202,   124,   218,   408,   457,   458,
     408,   408,   408,    30,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   342,   342,   342,
     383,   342,   373,    79,   246,   218,   218,   408,   483,    97,
       9,   300,   202,   201,   336,   339,   342,   208,   205,   473,
     300,   155,   168,   204,   390,   397,   155,   204,   396,   130,
     130,   203,   481,   488,   354,   489,    79,   163,    14,    79,
     477,   438,   342,   202,   290,   204,   290,   201,   130,   201,
     292,   202,   204,   488,   204,   270,   253,   406,   292,   130,
     208,     9,   412,   418,   146,   355,   421,   422,   417,   438,
     333,    30,    73,   233,   203,   335,   274,   452,   275,   202,
     408,    96,    99,   203,   342,    30,   203,   283,   205,   171,
     130,   163,    30,   202,   408,   408,   202,   130,     9,   412,
     202,   202,     9,   412,   130,   202,     9,   412,   202,   130,
     205,     9,   412,   408,    30,   188,   202,   231,   218,   488,
     401,     4,   106,   111,   119,   156,   157,   159,   205,   301,
     324,   325,   326,   331,   428,   452,   205,   204,   205,    50,
     342,   342,   342,   342,   354,    35,    79,   163,    14,    79,
     342,   201,   481,   202,   300,   202,   290,   342,   292,   202,
     300,   473,   300,   204,   201,   202,   417,   417,   202,   130,
     202,     9,   412,    30,   231,   203,   202,   202,   202,   238,
     203,   203,   283,   231,   488,   488,   130,   408,   355,   408,
     408,   408,   408,   408,   408,   342,   204,   205,    97,   126,
     127,   475,   273,   401,   119,   132,   133,   154,   160,   310,
     311,   312,   401,   158,   316,   317,   122,   201,   218,   318,
     319,   302,   249,   488,     9,   203,   325,   202,   297,   154,
     392,   205,   205,    79,   163,    14,    79,   342,   292,   111,
     344,   481,   205,   481,   202,   202,   205,   204,   205,   300,
     290,   130,   417,   355,   231,   236,   239,    30,   233,   277,
     231,   202,   408,   130,   130,   130,   189,   231,   488,   401,
     401,    14,     9,   203,   204,   170,   204,     9,   203,     3,
       4,     5,     6,     7,    10,    11,    12,    13,    27,    28,
      53,    67,    68,    69,    70,    71,    72,    73,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   131,
     132,   134,   135,   136,   137,   138,   150,   151,   152,   162,
     164,   165,   167,   174,   178,   180,   182,   183,   218,   398,
     399,     9,   203,   154,   158,   218,   319,   320,   321,   203,
      79,   330,   248,   303,   475,   249,   205,   298,   299,   475,
      14,    79,   342,   202,   201,   204,   203,   204,   322,   344,
     481,   297,   205,   202,   417,   130,    30,   233,   276,   277,
     231,   408,   408,   408,   342,   205,   203,   203,   408,   401,
     306,   488,   313,   407,   311,    14,    30,    47,   314,   317,
       9,    33,   202,    29,    46,    49,    14,     9,   203,   476,
     330,    14,   248,   203,    14,   342,    35,    79,   389,   231,
     231,   204,   322,   205,   481,   417,   231,    94,   190,   244,
     205,   218,   229,   307,   308,   309,     9,   171,     9,   205,
     408,   399,   399,    55,   315,   320,   320,    29,    46,    49,
     408,    79,   201,   203,   408,   476,   408,    79,     9,   413,
     205,   205,   231,   322,    92,   203,    79,   109,   240,   149,
      97,   488,   407,   161,    14,   304,   201,    35,    79,   202,
     205,   203,   201,   167,   247,   218,   325,   326,   171,   408,
     288,   289,   429,   305,    79,   401,   245,   164,   218,   203,
     202,     9,   413,   113,   114,   115,   328,   329,   288,    79,
     273,   203,   481,   429,   489,   202,   202,   203,   203,   204,
     323,   328,    35,    79,   163,   481,   204,   231,   489,    79,
     163,    14,    79,   323,   231,   205,    35,    79,   163,    14,
      79,   342,   205,    79,   163,    14,    79,   342,    14,    79,
     342,   342
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
#line 737 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 740 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 747 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 748 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 751 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 752 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 753 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 754 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 755 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 756 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 757 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 760 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 762 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 763 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 764 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 765 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 766 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 768 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 770 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 771 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 778 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 780 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 781 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 782 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 783 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 794 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 799 "hphp.y"
    { ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 800 "hphp.y"
    { ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 805 "hphp.y"
    { ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 806 "hphp.y"
    { ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 811 "hphp.y"
    { ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 812 "hphp.y"
    { ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 816 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 817 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 818 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 820 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 824 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 825 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 826 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 828 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 832 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 833 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 834 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 836 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 842 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 845 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 847 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 848 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 851 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 858 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 865 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 873 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 876 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 882 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 883 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 886 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 887 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 888 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 889 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 892 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 896 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 901 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 902 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 904 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 908 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 911 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 915 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 917 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 920 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 922 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 925 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 926 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 928 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 929 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 930 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 931 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 933 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 934 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 935 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 936 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 937 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 938 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 942 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 944 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 949 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 951 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 955 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 962 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 963 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 966 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 967 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 968 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 969 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval));;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 973 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 974 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 976 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 977 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 978 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 979 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 980 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 981 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 982 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 983 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 991 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 992 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1001 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { (yyval).reset();;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1006 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1008 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1014 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1015 "hphp.y"
    { (yyval).reset();;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1019 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1020 "hphp.y"
    { (yyval).reset();;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1024 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1029 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1035 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1041 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1047 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1053 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1059 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1067 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1071 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1075 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1079 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1085 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1088 "hphp.y"
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
#line 1103 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1106 "hphp.y"
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
#line 1120 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1123 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1128 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1131 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1138 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1141 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1149 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1152 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1160 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1161 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1165 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1168 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1171 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1172 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1173 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1176 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1177 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1181 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1182 "hphp.y"
    { (yyval).reset();;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1185 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1186 "hphp.y"
    { (yyval).reset();;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1189 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1190 "hphp.y"
    { (yyval).reset();;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1193 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1195 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1198 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1200 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1204 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1205 "hphp.y"
    { (yyval).reset();;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1208 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1209 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1210 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1214 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1216 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1219 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1221 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1224 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1226 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1229 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1231 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1241 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1242 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1243 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1244 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1249 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1251 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1252 "hphp.y"
    { (yyval).reset();;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1255 "hphp.y"
    { (yyval).reset();;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1256 "hphp.y"
    { (yyval).reset();;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1261 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1262 "hphp.y"
    { (yyval).reset();;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1267 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1268 "hphp.y"
    { (yyval).reset();;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1271 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1272 "hphp.y"
    { (yyval).reset();;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1275 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1276 "hphp.y"
    { (yyval).reset();;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1284 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1290 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1296 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1304 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1309 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1314 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1317 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1323 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1327 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1332 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1337 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1342 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1347 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1353 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1359 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1367 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1372 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1377 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1381 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1384 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1388 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1392 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1395 "hphp.y"
    { (yyval).reset();;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1400 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1403 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1407 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1411 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1415 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1419 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1424 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1429 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1435 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1436 "hphp.y"
    { (yyval).reset();;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1439 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1440 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1441 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1443 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1445 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1447 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1451 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1452 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1455 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1456 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1457 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1461 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1463 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1464 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1465 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1470 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1471 "hphp.y"
    { (yyval).reset();;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1474 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1479 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1485 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1486 "hphp.y"
    { (yyval).reset();;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1489 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1490 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1493 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1494 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1500 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1506 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1513 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1519 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1524 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1526 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1528 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1530 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1533 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1539 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1540 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1547 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1567 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1585 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1587 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1588 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1594 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1596 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1597 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1601 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1603 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1607 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1608 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1617 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1620 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1625 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1630 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1631 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1633 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1637 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1638 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1639 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1640 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1644 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1645 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1646 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1647 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1650 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1652 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1659 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1660 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1664 "hphp.y"
    { (yyval).reset();;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1665 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1669 "hphp.y"
    { (yyval).reset();;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1670 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1673 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { (yyval).reset();;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1677 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1678 "hphp.y"
    { (yyval).reset();;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1681 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1683 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1686 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1687 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1688 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1689 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1690 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1696 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1697 "hphp.y"
    { (yyval).reset();;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1700 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1701 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1702 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1708 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1709 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1710 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1714 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1716 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1722 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1723 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1724 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1725 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1728 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1732 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { (yyval).reset();;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1744 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1748 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1761 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1852 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { (yyval).reset();;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1867 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1873 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(7) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(8) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
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

  case 454:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
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

  case 458:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1969 "hphp.y"
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

  case 460:

/* Line 1455 of yacc.c  */
#line 1983 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1984 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1986 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1990 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1992 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 2002 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 2009 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 2012 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 2017 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 2018 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 2023 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 2024 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 2028 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 2032 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 2033 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 2038 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 2044 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 2045 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 2049 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_VARRAY);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 2054 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2056 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2061 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_VARRAY);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2066 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2073 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2075 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2079 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2080 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2081 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2082 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2084 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2088 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2092 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2096 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2101 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2103 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2105 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2107 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2111 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2117 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2118 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2120 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2121 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2126 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2139 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2148 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2153 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2162 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2163 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2167 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2168 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2176 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2184 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2185 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2186 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2187 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2194 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
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

  case 529:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
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

  case 530:

/* Line 1455 of yacc.c  */
#line 2219 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { (yyval).reset();;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { (yyval).reset();;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2332 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2337 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2338 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2355 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { (yyval).reset();;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { (yyval).reset();;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { (yyval).reset();;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { (yyval).reset();;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2479 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2481 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2485 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { (yyval).reset();;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { (yyval).reset();;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2496 "hphp.y"
    { (yyval).reset();;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { (yyval).reset();;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2506 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2510 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2511 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2517 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2518 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2522 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2524 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2528 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2529 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2534 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2535 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2538 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2539 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2541 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2547 "hphp.y"
    { (yyval).reset();;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2552 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2557 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2562 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2567 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2576 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2581 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2582 "hphp.y"
    { (yyval).reset();;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2593 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2598 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2603 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { (yyval).reset();;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2616 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2617 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2618 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2627 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2628 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2632 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2633 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2638 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2644 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2649 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2655 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2656 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2657 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2659 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2661 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2664 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2667 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2669 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2671 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2672 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2676 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2677 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2678 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2679 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2682 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2686 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2688 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2689 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2693 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2694 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2695 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2699 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2703 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2704 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2710 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2721 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2725 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2729 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2733 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2735 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2740 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2741 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2742 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2745 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2746 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2749 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2750 "hphp.y"
    { (yyval).reset();;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2755 "hphp.y"
    { (yyval)++;;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2759 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2760 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2763 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2766 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2769 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2770 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2777 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2781 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2782 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2786 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2787 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2789 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2790 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2791 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2792 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2798 "hphp.y"
    { (yyval).reset();;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2802 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2803 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2804 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2805 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2808 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2810 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2811 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2812 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2817 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2818 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2822 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2823 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2824 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2825 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2830 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2831 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2836 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2840 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2841 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2846 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2847 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2851 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2852 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2856 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2857 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2860 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2861 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2866 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2867 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2871 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2872 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2877 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2879 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2883 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2884 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2888 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2890 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2891 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2893 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2898 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2900 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2902 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]).num(), (yyvsp[(3) - (3)]));;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2904 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2906 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2907 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2910 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2911 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2912 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2916 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2917 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2918 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2919 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2920 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2922 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2924 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2926 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2930 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2931 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2936 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2938 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2952 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2956 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 2962 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 2963 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 2969 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 2973 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 2979 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 2980 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 2987 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 2993 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 2998 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 2999 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3000 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3001 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3005 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3006 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3011 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3012 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3015 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (6)]).text()); ;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3017 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (4)]).text()); ;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3021 "hphp.y"
    {;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3022 "hphp.y"
    {;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3023 "hphp.y"
    {;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3029 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3034 "hphp.y"
    { ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3048 "hphp.y"
    {;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3052 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3059 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3062 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3065 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3066 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3069 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3072 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3074 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3077 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3080 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3086 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3092 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3100 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3101 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13834 "hphp.tab.cpp"
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
#line 3104 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

