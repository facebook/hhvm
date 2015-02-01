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
#define YYLAST   16592

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  208
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  273
/* YYNRULES -- Number of rules.  */
#define YYNRULES  932
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1746

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
    3063,  3067,  3069,  3072,  3073,  3078,  3080,  3082,  3086,  3089,
    3092,  3095,  3097,  3099,  3101,  3103,  3107,  3112,  3119,  3121,
    3130,  3137,  3139
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
     408,    -1,    -1,   174,   198,   475,   199,    -1,   397,    -1,
     118,    -1,   215,   149,   478,    -1,   215,   466,    -1,    29,
     479,    -1,    55,   479,    -1,   226,    -1,   132,    -1,   133,
      -1,   476,    -1,   477,   149,   478,    -1,   132,   170,   479,
     171,    -1,   132,   170,   479,     9,   479,   171,    -1,   154,
      -1,   198,   105,   198,   469,   199,    30,   479,   199,    -1,
     198,   479,     9,   467,   408,   199,    -1,   479,    -1,    -1
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
    2998,  3000,  3004,  3007,  3010,  3015,  3016,  3019,  3022,  3029,
    3032,  3035,  3036,  3039,  3042,  3043,  3050,  3053,  3057,  3061,
    3067,  3077,  3078
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
     474,   474,   475,   475,   476,   477,   477,   478,   478,   479,
     479,   479,   479,   479,   479,   479,   479,   479,   479,   479,
     479,   480,   480
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
       3,     1,     2,     0,     4,     1,     1,     3,     2,     2,
       2,     1,     1,     1,     1,     3,     4,     6,     1,     8,
       6,     1,     0
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
       0,     0,   916,   922,   923,   928,     0,     0,    59,   921,
     915,     0,   924,     0,     0,     0,    90,     0,     0,     0,
       0,   122,     0,     0,     0,     0,     0,     0,    42,    47,
     248,     0,     0,   247,     0,   163,     0,   160,   253,     0,
       0,     0,     0,     0,   888,   147,   157,   829,   833,   858,
       0,   655,     0,     0,     0,   856,     0,    16,     0,    63,
     139,   151,   158,   532,   475,     0,   882,   457,   461,   463,
     756,   377,     0,   375,   376,   378,     0,     0,   637,     0,
     638,     0,     0,     0,   121,     0,     0,    66,   239,     0,
      21,   130,     0,   156,   143,   155,   337,   340,   131,   333,
     112,   113,   114,   115,   116,   118,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   821,     0,   111,   812,   812,   119,   843,     0,     0,
       0,     0,     0,     0,   330,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   398,   396,
     757,   758,     0,   812,     0,   770,   239,   239,   812,     0,
     814,   805,   829,     0,   131,     0,     0,    92,     0,   754,
     749,   708,     0,     0,     0,     0,     0,   841,     0,   480,
     707,   832,     0,     0,    66,     0,   239,   358,     0,   772,
     633,     0,    70,   199,     0,   456,     0,    95,     0,     0,
     365,     0,     0,     0,     0,     0,    87,   110,    89,   919,
     920,     0,   913,     0,     0,     0,     0,   887,     0,   117,
      91,   120,     0,     0,     0,     0,     0,     0,     0,   490,
       0,   497,   499,   500,   501,   502,   503,   504,   495,   517,
     518,    70,     0,   107,   109,     0,     0,    44,    51,     0,
       0,    46,    55,    48,     0,    18,     0,     0,   249,     0,
      93,   162,   161,     0,     0,    94,   878,     0,     0,   377,
     375,   376,   379,   378,     0,   907,   169,     0,   830,     0,
       0,     0,     0,   654,   857,   699,     0,     0,   855,   704,
     854,    62,     5,    13,    14,     0,   167,     0,     0,   468,
       0,     0,   708,     0,     0,   629,   469,     0,     0,     0,
       0,   756,    70,     0,   710,   755,   932,   357,   430,   784,
     796,    75,    69,    71,    72,    73,    74,   331,     0,   446,
     702,   703,    60,   708,     0,   892,     0,     0,     0,   710,
     240,     0,   451,   133,   165,     0,   402,   404,   403,     0,
       0,   400,   401,   405,   407,   406,   422,   421,   424,   423,
     425,   427,   428,   426,   416,   415,   409,   410,   408,   411,
     412,   414,   429,   413,   811,     0,     0,   847,     0,   708,
     881,     0,   880,   781,   804,   149,   141,   153,   145,   131,
     367,     0,   370,   373,   381,   491,   395,   394,   393,   392,
     391,   390,   389,   388,   387,   386,   385,   384,   760,     0,
     759,   762,   779,   766,   891,   763,     0,     0,     0,     0,
       0,     0,     0,     0,   875,   369,   747,   751,   707,   753,
       0,     0,   891,     0,   836,     0,   835,     0,   820,   819,
       0,     0,   759,   762,   817,   763,   362,   201,   203,    70,
     466,   465,   363,     0,    70,   183,    79,   366,     0,     0,
       0,     0,     0,   195,   195,    85,     0,     0,     0,   911,
     708,     0,   898,     0,     0,     0,     0,     0,   706,   644,
       0,     0,   626,     0,     0,    64,   657,   625,   662,     0,
     656,    68,   661,   891,   925,     0,     0,   507,     0,     0,
     513,   510,   511,   519,     0,   498,   493,     0,   496,     0,
       0,     0,    52,    19,     0,     0,    56,    20,     0,     0,
       0,    41,    49,     0,   246,   254,   251,     0,     0,   867,
     872,   869,   868,   871,   870,    12,   905,   906,     0,     0,
       0,     0,   829,   826,     0,   479,   866,   865,   864,     0,
     860,     0,   861,   863,     0,     5,     0,     0,     0,   526,
     527,   535,   534,     0,     0,   707,   474,   478,     0,     0,
     883,     0,   458,     0,     0,   899,   756,   225,   931,     0,
       0,   771,   810,   707,   894,   890,   241,   242,   624,   709,
     238,     0,   756,     0,     0,   167,   453,   135,   432,     0,
     483,   484,     0,   481,   707,   842,     0,     0,   239,   169,
       0,   167,   165,     0,   821,   382,     0,     0,   768,   769,
     782,   783,   806,   807,     0,     0,     0,   735,   715,   716,
     717,   724,     0,     0,     0,   728,   726,   727,   741,   708,
       0,   749,   840,   839,     0,     0,   773,   639,     0,   205,
       0,     0,    76,     0,     0,     0,     0,     0,     0,     0,
     175,   176,   187,     0,    70,   185,   104,   195,     0,   195,
       0,     0,   926,     0,     0,   707,   912,   914,   897,   708,
     896,     0,   708,   683,   684,   681,   682,   714,     0,   708,
     706,     0,     0,   477,     0,     0,   849,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   918,   492,     0,     0,     0,   515,
     516,   514,     0,     0,   494,     0,   123,     0,   126,   108,
       0,    43,    53,     0,    45,    57,    50,   250,     0,   879,
      96,   907,   889,   902,   168,   170,   260,     0,     0,   827,
       0,   859,     0,    17,     0,   882,   166,   260,     0,     0,
     471,     0,   880,   884,     0,   899,   464,     0,     0,   932,
       0,   230,   228,   762,   780,   891,   893,     0,     0,   243,
      67,     0,   756,   164,     0,   756,     0,   431,   846,   845,
       0,   239,     0,     0,     0,     0,   167,   137,   640,   761,
     239,     0,   720,   721,   722,   723,   729,   730,   739,     0,
     708,     0,   735,     0,   719,   743,   707,   746,   748,   750,
       0,   834,   762,   818,   761,     0,     0,     0,     0,   202,
     467,    81,     0,   366,   175,   177,   829,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   189,     0,   908,     0,
     910,   707,     0,     0,     0,   659,   707,   705,     0,   696,
       0,   708,     0,   663,   697,   695,   853,     0,   708,   666,
     668,   667,     0,     0,   664,   665,   669,   671,   670,   686,
     685,   688,   687,   689,   691,   692,   690,   679,   678,   673,
     674,   672,   675,   676,   677,   680,   917,   505,     0,   506,
     512,   520,   521,     0,    70,    54,    58,   252,     0,     0,
       0,   331,   831,   829,   371,   374,   380,     0,    15,     0,
     331,   538,     0,     0,   540,   533,   536,     0,   531,     0,
     885,     0,   900,   460,     0,   231,     0,     0,   226,     0,
     245,   244,   899,     0,   260,     0,   756,     0,   239,     0,
     802,   260,   882,   260,     0,     0,   383,     0,     0,   732,
     707,   734,   725,     0,   718,     0,     0,   708,   740,   838,
       0,    70,     0,   198,   184,     0,     0,     0,   174,   100,
     188,     0,     0,   191,     0,   196,   197,    70,   190,   927,
       0,   895,     0,   930,   713,   712,   658,     0,   707,   476,
     660,     0,   482,   707,   848,   694,     0,     0,     0,     0,
     901,   904,   171,     0,     0,     0,   338,   329,     0,     0,
       0,   148,   259,   261,     0,   328,     0,     0,     0,   882,
     331,     0,   862,   256,   152,   529,     0,     0,   470,   462,
       0,   234,   224,     0,   227,   233,   239,   450,   899,   331,
     899,     0,   844,     0,   801,   331,     0,   331,   260,   756,
     799,   738,   737,   731,     0,   733,   707,   742,    70,   204,
      77,    82,   102,   178,     0,   186,   192,    70,   194,   909,
       0,     0,   473,     0,   852,   851,   693,     0,    70,   127,
       0,     0,     0,     0,     0,     0,   172,     0,   882,   295,
     291,   297,   626,    27,     0,   287,     0,   294,   306,     0,
     304,   309,     0,   308,     0,   307,     0,   131,   337,   263,
       0,   265,     0,   266,   267,     0,     0,   828,     0,   530,
     528,   539,   537,   235,     0,     0,   222,   232,     0,     0,
       0,     0,   144,   450,   899,   803,   150,   256,   154,   331,
       0,     0,   745,     0,   200,     0,     0,    70,   181,   101,
     193,   929,   711,     0,     0,     0,     0,   903,     0,     0,
     356,     0,     0,   277,   281,   353,   354,     0,     0,     0,
     272,   590,   589,   586,   588,   587,   607,   609,   608,   578,
     549,   550,   568,   584,   583,   545,   555,   556,   558,   557,
     577,   561,   559,   560,   562,   563,   564,   565,   566,   567,
     569,   570,   571,   572,   573,   574,   576,   575,   546,   547,
     548,   551,   552,   554,   592,   593,   602,   601,   600,   599,
     598,   597,   585,   604,   594,   595,   596,   579,   580,   581,
     582,   605,   606,   610,   612,   611,   613,   614,   591,   616,
     615,   618,   620,   619,   553,   623,   621,   622,   617,   603,
     544,   301,   541,     0,   273,   322,   323,   321,   314,     0,
     315,   274,   348,     0,     0,     0,     0,   352,     0,   131,
     140,   255,     0,     0,     0,   223,   237,   800,     0,    70,
     324,    70,   134,     0,     0,     0,   146,   899,   736,     0,
      70,   179,    83,   103,     0,   472,   850,   508,   125,   275,
     276,   351,   173,     0,     0,     0,   298,   288,     0,     0,
       0,   303,   305,     0,     0,   310,   317,   318,   316,     0,
       0,   262,     0,     0,     0,   355,     0,   257,     0,   236,
       0,   524,   710,     0,     0,    70,   136,   142,     0,   744,
       0,     0,     0,   105,   278,    59,     0,   279,   280,     0,
       0,   292,     0,   296,   300,   542,   543,     0,   289,   319,
     320,   312,   313,   311,   349,   346,   268,   264,   350,     0,
     258,   525,   709,     0,   452,   325,     0,   138,     0,   182,
     509,     0,   129,     0,   331,     0,   299,   302,     0,   756,
     270,     0,   522,   449,   454,   180,     0,     0,   106,   285,
       0,   330,   293,   347,     0,   710,   342,   756,   523,     0,
     128,     0,     0,   284,   899,   756,   209,   343,   344,   345,
     932,   341,     0,     0,     0,   283,     0,   342,     0,   899,
       0,   282,   326,    70,   269,   932,     0,   214,   212,     0,
      70,     0,     0,   215,     0,     0,   210,   271,     0,   327,
       0,   218,   208,     0,   211,   217,   124,   219,     0,     0,
     206,   216,     0,   207,   221,   220
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   118,   815,   552,   180,   277,   506,
     510,   278,   507,   511,   120,   121,   122,   123,   124,   125,
     326,   582,   583,   459,   239,  1425,   465,  1344,  1426,  1662,
     771,   272,   501,  1623,   999,  1174,  1678,   342,   181,   584,
     853,  1056,  1225,   129,   555,   870,   585,   604,   872,   536,
     869,   586,   556,   871,   344,   295,   311,   132,   855,   818,
     801,  1014,  1365,  1108,   920,  1572,  1429,   716,   926,   464,
     725,   928,  1257,   708,   909,   912,  1097,  1684,  1685,   573,
     574,   598,   599,   282,   283,   289,  1398,  1551,  1552,  1181,
    1292,  1386,  1545,  1669,  1687,  1583,  1627,  1628,  1629,  1374,
    1375,  1376,  1585,  1591,  1638,  1379,  1380,  1384,  1538,  1539,
    1540,  1562,  1714,  1293,  1294,   182,   134,  1700,  1701,  1543,
    1296,  1297,  1298,  1299,   135,   232,   460,   461,   136,   137,
     138,   139,   140,   141,   142,   143,  1410,   144,   852,  1055,
     145,   236,   570,   320,   571,   572,   455,   561,   562,  1131,
     563,  1132,   146,   147,   148,   748,   149,   150,   269,   151,
     270,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     761,   762,   991,   498,   499,   500,   768,  1612,   152,   557,
    1400,   558,  1028,   823,  1198,  1195,  1531,  1532,   153,   154,
     155,   226,   233,   329,   447,   156,   947,   752,   157,   948,
     844,   837,   949,   896,  1076,   897,  1078,  1079,  1080,   899,
    1236,  1237,   900,   687,   431,   193,   194,   587,   576,   412,
     671,   672,   673,   674,   841,   159,   227,   184,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   635,   170,   229,
     230,   539,   218,   219,   638,   639,  1137,  1138,   304,   305,
     809,   171,   527,   172,   569,   173,  1553,   296,   337,   593,
     594,   941,  1038,   798,   799,   729,   730,   731,   262,   263,
     754,   264,   839
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1394
static const yytype_int16 yypact[] =
{
   -1394,   172, -1394, -1394,  5228, 13223, 13223,   -16, 13223, 13223,
   13223, 11173, 13223, -1394, 13223, 13223, 13223, 13223, 13223, 13223,
   13223, 13223, 13223, 13223, 13223, 13223, 15278, 15278, 11378, 13223,
   15329,   -13,   -11, -1394, -1394, -1394, -1394, -1394,   188, -1394,
   -1394,   164, 13223, -1394,   -11,    23,    41,    70,   -11, 11583,
   14414, 11788, -1394, 14342, 10148,   110, 13223, 14399,    16, -1394,
   -1394, -1394,   518,   363,   358,   145,   250,   270,   313, -1394,
   14414,   323,   342, -1394, -1394, -1394, -1394, -1394,   453,  2562,
   -1394, -1394, 14414, -1394, -1394, -1394, -1394, 14414, -1394, 14414,
   -1394,   104,   357, 14414, 14414, -1394,   262, -1394, -1394, -1394,
   -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394,
   -1394, 13223, -1394, -1394,   241,   449,   535,   535, -1394,   252,
     319,   474, -1394,   395, -1394,    63, -1394,   524, -1394, -1394,
   -1394, -1394, 14611,   791, -1394, -1394,   398,   417,   458,   462,
     480,   485,  4486, -1394, -1394, -1394, -1394,   408, -1394,   625,
     628,   505, -1394,    49,   523,   577, -1394, -1394,   667,    37,
    4038,   134,   541,    47, -1394,   136,   138,   553,    52, -1394,
     143, -1394,   699, -1394, -1394, -1394,   621,   598,   633, -1394,
   -1394,   524,   791, 16254,  4334, 16254, 13223, 16254, 16254,  3304,
     605, 15514,  3304,   769, 14414,   751,   751,   588,   751,   751,
     751,   751,   751,   751,   751,   751,   751, -1394, -1394, 14865,
     656, -1394,   682,   548,   636,   548, 15278, 15558,   634,   824,
   -1394,   621, 14917,   656,   691,   693,   650,   146, -1394,   548,
     134, 11993, -1394, -1394, 13223,  8713,   841,    65, 16254,  9738,
   -1394, 13223, 13223, 14414, -1394, -1394,  4573,   653, -1394, 13818,
   14342, 14342, -1394,   680, -1394,   702,   657, 14109,    61,   707,
   -1394,   844, -1394,   711, 14414,   782, -1394,   662, 13862,   669,
     816, -1394,    64, 13906, 15906, 15922, 14414,    74, -1394,   361,
   -1394, 15055,    83, -1394,   746, -1394,   747, -1394,   858,    85,
   15278, 15278, 13223,   675,   705, -1394, -1394, 15141, 11378,   439,
      43, -1394, 13428, 15278,   479, -1394, 14414, -1394,    17,   319,
   -1394, -1394, -1394, -1394, 14624,   863,   781, -1394, -1394, -1394,
      60,   684, 16254,   685,  2374,   686,  5433, 13223,   349,   678,
     545,   349,   409,   345, -1394, 14414, 14342,   681, 10353, 14342,
   -1394, -1394,  4088, -1394, -1394, -1394, -1394, -1394,   524, -1394,
   -1394, -1394, -1394, -1394, -1394, -1394, 13223, 13223, 13223, 12198,
   13223, 13223, 13223, 13223, 13223, 13223, 13223, 13223, 13223, 13223,
   13223, 13223, 13223, 13223, 13223, 13223, 13223, 13223, 13223, 13223,
   13223, 15329, 13223, -1394, 13223, 13223, -1394, 13223,  1759, 14414,
   14414, 14414, 14611,   784,   533,  9943, 13223, 13223, 13223, 13223,
   13223, 13223, 13223, 13223, 13223, 13223, 13223, 13223, -1394, -1394,
   -1394, -1394,  3717, 13223, 13223, -1394, 10353, 10353, 13223, 13223,
     241,   148, 15141,   688,   524, 12403, 13950, -1394, 13223, -1394,
     689,   883, 14865,   698,    -4,   686,  2848,   548, 12608, -1394,
   12813, -1394,   701,   290, -1394,   181, 10353, -1394,  3955, -1394,
   -1394, 13994, -1394, -1394, 10558, -1394, 13223, -1394,   809,  8918,
     892,   710, 16209,   897,   114,    46, -1394, -1394, -1394, -1394,
   -1394, 14342, 15127,   717,   914, 14687, 14414, -1394,   738, -1394,
   -1394, -1394,   846, 13223,   847,   849, 13223, 13223, 13223, -1394,
     816, -1394, -1394, -1394, -1394, -1394, -1394, -1394,   745, -1394,
   -1394, -1394,   736, -1394, -1394, 14414,   739,   931,   374, 14414,
     742,   935,   454,   457, 15964, -1394, 14414, 13223,   548,    16,
   -1394, -1394, -1394, 14687,   867, -1394,   548,   118,   119,   749,
     752,  2431,    35,   754,   750,   556,   828,   757,   548,   122,
     760, 14957, 14414, -1394, -1394,   890,  1953,    40, -1394, -1394,
   -1394,   319, -1394, -1394, -1394,   932,   834,   794,   379, -1394,
     241,   836,   955,   774,   826,   148, -1394, 14342, 14342,   962,
     841,    60, -1394,   783,   968, -1394, 14342,   276,   918,   147,
   -1394, -1394, -1394, -1394, -1394, -1394, -1394,   920,  2133, -1394,
   -1394, -1394, -1394,   971,   814, -1394, 15278, 13223,   790,   981,
   16254,   977, -1394, -1394,   868, 13491, 10746, 11773,  3304, 13223,
   14876, 12796, 16373, 15325, 16436, 12176, 16497, 16497, 16497, 16497,
    1643,  1643,  1643,  1643,   706,   706,   772,   772,   772,   588,
     588,   588, -1394,   751, 16254,   788,   796, 15614,   793,   993,
   -1394, 13223,   286,   812,   148, -1394, -1394, -1394, -1394,   524,
   13223, 15004, -1394, -1394,  3304, -1394,  3304,  3304,  3304,  3304,
    3304,  3304,  3304,  3304,  3304,  3304,  3304,  3304, -1394, 13223,
     338,   215, -1394, -1394,   656,   424,   802,  2222,   813,   815,
     806,  2488,   124,   822, -1394, 16254, 14816, -1394, 14414, -1394,
     684,   276,   656, 15278, 16254, 15278, 15658,   276,   233, -1394,
     823, 13223, -1394,   244, -1394, -1394, -1394,  8508,   616, -1394,
   -1394, 16254, 16254,   -11, -1394, -1394, -1394, 13223,   916, 14537,
   14687, 14414,  9123,   821,   825, -1394,    73,   893,   891, -1394,
    1032,   848, 14156, 14342, 14687, 14687, 14687, 14687, 14687, -1394,
     851,    81,   901,   854, 14687,   352, -1394,   908, -1394,   845,
   -1394, 16340, -1394,   467, -1394, 13223,   875, 16254,   876,  1047,
   11158,  1053, -1394, 16254, 14038, -1394,   745,   984, -1394,  5638,
   15844,   866,   465, -1394, 15906, 14414,   487, -1394, 15922, 14414,
   14414, -1394, -1394,  2663, -1394, 16340,  1050, 15278,   871, -1394,
   -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394,    84, 14414,
   15844,   873, 15141, 15227,  1054, -1394, -1394, -1394, -1394,   870,
   -1394, 13223, -1394, -1394,  4805, -1394, 14342, 15844,   877, -1394,
   -1394, -1394, -1394,  1062, 13223, 14624, -1394, -1394, 16003,   879,
   -1394, 14342, -1394,   881,  5843,  1055,    44, -1394, -1394,   153,
    3717, -1394, -1394, 14342, -1394, -1394,   548, 16254, -1394, 10763,
   -1394, 14687,   120,   886, 15844,   834, -1394, -1394, 12386, 13223,
   -1394, -1394, 13223, -1394, 13223, -1394,  2927,   889, 10353,   828,
    1058,   834,   868, 14414, 15329,   548,  3530,   894, -1394, -1394,
     272, -1394, -1394, -1394,  1075,  4384,  4384, 14816, -1394, -1394,
   -1394,  1042,   898,    82,   899, -1394, -1394, -1394, -1394,  1086,
     902,   689,   548,   548, 13018,  3955, -1394, -1394,  3723,   670,
     -11,  9738, -1394,  6048,   903,  6253,   904, 14537, 15278,   907,
     976,   548, 16340,  1093, -1394, -1394, -1394, -1394,   555, -1394,
     359, 14342, -1394, 14342, 14414, 15127, -1394, -1394, -1394,  1100,
   -1394,   911,   971,   697,   697,  1046,  1046, 15758,   909,  1104,
   14687,   972, 14414, 14624,  2845, 16042, 14687, 14687, 14687, 14687,
   14491, 14687, 14687, 14687, 14687, 14687, 14687, 14687, 14687, 14687,
   14687, 14687, 14687, 14687, 14687, 14687, 14687, 14687, 14687, 14687,
   14687, 14687, 14687, 14414, -1394, 16254, 13223, 13223, 13223, -1394,
   -1394, -1394, 13223, 13223, -1394,   816, -1394,  1038, -1394, -1394,
   14414, -1394, -1394, 14414, -1394, -1394, -1394, -1394, 14687,   548,
   -1394,   556, -1394,  1023,  1114, -1394, -1394,   126,   923,   548,
   10968, -1394,   109, -1394,  5023,   781,  1114, -1394,   464,   -21,
   16254,   994, -1394, -1394,   926,  1055, -1394, 14342,   841, 14342,
      53,  1113,  1051,   274, -1394,   656, -1394, 15278, 13223, 16254,
   16340,   930,   120, -1394,   933,   120,   937, 12386, 16254, 15714,
     938, 10353,   934,   936, 14342,   939,   834, -1394,   650,   436,
   10353, 13223, -1394, -1394, -1394, -1394, -1394, -1394,  1001,   941,
    1129,  1056, 14816,   997, -1394, 14624, 14816, -1394, -1394, -1394,
   15278, 16254,   344, -1394, -1394,   -11,  1117,  1078,  9738, -1394,
   -1394, -1394,   952, 13223,   976,   548, 15141, 14537,   954, 14687,
    6458,   635,   958, 13223,    77,   369, -1394,   989, -1394,  1033,
   -1394, 14249,  1135,   967, 14687, -1394, 14687, -1394,   969, -1394,
    1041,  1163,   974, -1394, -1394, -1394, 15814,   973,  1167, 11361,
   11978, 10336, 14687, 16298, 13001,  4406, 16405, 16467, 15185, 16527,
   16527, 16527, 16527,  2165,  2165,  2165,  2165,   651,   651,   697,
     697,   697,  1046,  1046,  1046,  1046, -1394, 16254, 13413, 16254,
   -1394, 16254, -1394,   978, -1394, -1394, -1394, 16340, 14414, 14342,
   15844,    92, -1394, 15141, -1394, -1394,  3304,   979, -1394,   983,
     472, -1394,    66, 13223, -1394, -1394, -1394, 13223, -1394, 13223,
   -1394,   841, -1394, -1394,   353,  1164,  1101, 13223, -1394,   987,
     548, 16254,  1055,   980, -1394,   988,   120, 13223, 10353,   990,
   -1394, -1394,   781, -1394,   985,   992, -1394,   996, 14816, -1394,
   14816, -1394, -1394,  1002, -1394,  1052,  1004,  1187, -1394,   548,
    1176, -1394,  1007, -1394, -1394,  1010,  1011,   177, -1394, -1394,
   16340,  1014,  1015, -1394,  4279, -1394, -1394, -1394, -1394, -1394,
   14342, -1394, 14342, -1394, 16340, 15858, -1394, 14687, 14624, -1394,
   -1394, 14687, -1394, 14687, -1394, 12591, 14687, 13223,  1018,  6663,
    1119, -1394, -1394,   582, 14296, 15844,  1105, -1394, 15863,  1063,
   14202, -1394, -1394, -1394,   784, 14063,    88,    89,  1022,   781,
     533,   184, -1394, -1394, -1394,  1070,  3962,  4173, 16254, -1394,
     327,  1218,  1155, 13223, -1394, 16254, 10353,  1124,  1055,   913,
    1055,  1037, 16254,  1039, -1394,  1112,  1036,  1261, -1394,   120,
   -1394, -1394,  1110, -1394, 14816, -1394, 14624, -1394, -1394,  8508,
   -1394, -1394, -1394, -1394,  9328, -1394, -1394, -1394,  8508, -1394,
    1043, 14687, 16340,  1111, 16340, 15914, 12591, 13208, -1394, -1394,
   14342, 15844, 15844, 14414,  1229,    79, -1394, 14296,   781, -1394,
    1074, -1394,    90,  1045,    91, -1394, 13633, -1394, -1394,    95,
   -1394, -1394, 13939, -1394,  1049, -1394,  1168,   524,  1109, -1394,
   14342, -1394, 14342, -1394, -1394,  1236,   784, -1394,  3330, -1394,
   -1394, -1394, -1394,  1239,  1175, 13223, -1394, 16254,  1060,  1064,
    1059,   560, -1394,  1124,  1055, -1394, -1394, -1394, -1394,  1542,
    1065, 14816, -1394,  1131,  8508,  9533,  9328, -1394, -1394, -1394,
    8508, -1394, 16340, 14687, 14687, 13223,  6868, -1394,  1066,  1073,
   -1394, 14687, 15844, -1394, -1394, -1394, -1394, 14342,   593, 15863,
   -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394,
   -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394,
   -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394,
   -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394,
   -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394,
   -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394,
   -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394,
   -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394,
   -1394,   643, -1394,  1063, -1394, -1394, -1394, -1394, -1394,    93,
     415, -1394,  1249,   101, 14414,  1168,  1256, -1394, 14342,   524,
   -1394, -1394,  1076,  1260, 13223, -1394, 16254, -1394,   144, -1394,
   -1394, -1394, -1394,  1077,   560,  4603, -1394,  1055, -1394, 14816,
   -1394, -1394, -1394, -1394,  7073, 16340, 16340, 11568, -1394, -1394,
   -1394, 16340, -1394,  4483,   155,    57, -1394, -1394, 14687, 13633,
   13633,  1222, -1394, 13939, 13939,   614, -1394, -1394, -1394, 14687,
    1200, -1394,  1082,   102, 14687, -1394, 14414, -1394, 14687, 16254,
    1202, -1394,  1273,  7278,  7483, -1394, -1394, -1394,   560, -1394,
    7688,  1084,  1208,  1179, -1394,  1192,  1142, -1394, -1394,  1196,
   14342, -1394,   593, -1394, 16340, -1394, -1394,  1133, -1394,  1262,
   -1394, -1394, -1394, -1394, 16340,  1284, -1394, -1394, 16340,  1107,
   16340, -1394,   427,  1108, -1394, -1394,  7893, -1394,  1103, -1394,
   -1394,  1115,  1148, 14414,   533,  1146, -1394, -1394, 14687,   130,
   -1394,  1240, -1394, -1394, -1394, -1394, 15844,   866, -1394,  1156,
   14414,   459, -1394, 16340,  1122,  1313,   661,   130, -1394,  1244,
   -1394, 15844,  1125, -1394,  1055,   132, -1394, -1394, -1394, -1394,
   14342, -1394,  1128,  1130,   108, -1394,   567,   661,   386,  1055,
    1123, -1394, -1394, -1394, -1394, 14342,   336,  1314,  1251,   567,
   -1394,  8098,   387,  1317,  1254, 13223, -1394, -1394,  8303, -1394,
     346,  1320,  1257, 13223, -1394, 16254, -1394,  1325,  1263, 13223,
   -1394, 16254, 13223, -1394, 16254, 16254
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1394, -1394, -1394,  -491, -1394, -1394, -1394,   476, -1394, -1394,
   -1394,   827,   566,   568,    30,  1508,  3456, -1394,  3116, -1394,
    -411, -1394,    27, -1394, -1394, -1394, -1394, -1394, -1394, -1394,
   -1394, -1394, -1394, -1394,  -333, -1394, -1394,  -154,    59,    24,
   -1394, -1394, -1394, -1394, -1394, -1394,    29, -1394, -1394, -1394,
   -1394,    31, -1394, -1394,   960,   961,   963,  -102,   483,  -791,
     490,   539,  -326,   260,  -718, -1394,   -58, -1394, -1394, -1394,
   -1394,  -643,   115, -1394, -1394, -1394, -1394,  -317, -1394,  -532,
   -1394,  -338, -1394, -1394,   865, -1394,   -44, -1394, -1394,  -950,
   -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394,
     -67, -1394, -1394, -1394, -1394, -1394,  -150, -1394,    98,  -816,
   -1394, -1393,  -334, -1394,  -156,    18,  -125,  -314, -1394,  -155,
   -1394, -1394, -1394,   111,   -25,     4,  1357,  -650,  -342, -1394,
   -1394,   -14, -1394, -1394,    -5,   -38,   -12, -1394, -1394, -1394,
   -1394, -1394, -1394, -1394, -1394, -1394,  -520,  -770, -1394, -1394,
   -1394, -1394, -1394, -1394, -1394, -1394, -1394, -1394,  1008, -1394,
   -1394,   407, -1394,   915, -1394, -1394, -1394, -1394, -1394, -1394,
   -1394,   412, -1394,   922, -1394, -1394,   640, -1394,   380, -1394,
   -1394, -1394, -1394, -1394, -1394, -1394, -1394,  -796, -1394,  2350,
    1321,  -327, -1394, -1394,   351,  2282,  2695, -1394, -1394,   471,
    -135,  -573, -1394, -1394,   528,   341,  -648,   343, -1394, -1394,
   -1394, -1394, -1394,   522, -1394, -1394, -1394,    67,  -793,  -160,
    -390,  -386, -1394,   584,  -116, -1394, -1394,   498, -1394, -1394,
     864,   -46, -1394, -1394,    55,  -136, -1394,   248, -1394, -1394,
   -1394,  -363,  1134, -1394, -1394, -1394, -1394, -1394,   683,   637,
   -1394, -1394,  1138,  -282,  -965, -1394,   -32,   -64,  -132,   103,
     694, -1394,  -988, -1394,   422,   499, -1394, -1394, -1394, -1394,
     452,  -121,  -997
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -917
static const yytype_int16 yytable[] =
{
     183,   185,   393,   187,   188,   189,   191,   192,   349,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   261,   133,   217,   220,   312,   850,   423,   128,   315,
     316,   126,   566,   130,   421,   131,   235,   238,   898,   833,
     267,   707,  1204,  1039,   246,   415,   249,  1201,   240,   268,
     832,   273,   244,   652,   632,  1031,   392,   349,   703,   682,
    1189,   814,   704,   127,  1054,   325,  1632,   916,   345,   448,
    -886,   158,   339,   323,   456,  -886,   723,  1190,   678,   679,
    1065,   930,   931,   514,   441,   228,   321,   279,  1442,    13,
     769,   444,   519,  1011,   524,   280,  1283,  1390,  1392,  -290,
    1449,  -487,  1593,  -788,  1533,    13,   322,  1255,   700,   308,
    1600,  1600,   309,  -487,   449,  -785,  1305,  1442,   418,   356,
     357,   358,   299,   721,   410,   411,  1594,   787,   787,   469,
     470,   803,  1205,   803,  1196,   803,   474,    13,   359,   542,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   834,   381,   435,  1630,    13,   951,  1083,   410,   411,
     335,  1616,     3,   502,   382,    13,   394,    13,   565,  1610,
    1197,   426,   186,  1130,   433,   231,   803,   234,  1040,   543,
     302,   303,   348,   803,   605,   579,  -455,   442,  1284,  1104,
     413,  -629,  -786,  1285,  -787,    59,    60,    61,   174,  1286,
     346,  1287,  -822,  -792,   418,   595,  1206,   553,   554,   281,
     313,   241,   207,  1611,  1317,  1657,   451,   301,  1084,   451,
    -488,   503,  1041,   417,   793,  -794,   238,   462,  -788,   242,
    -709,   424,   813,  -709,   932,   417,   533,   724,  1288,  1289,
    -785,  1290,   644,   419,   530,  1012,  -229,  1326,   313,  1633,
     207,  -886,   453,   340,  1319,   457,   458,   529,   243,   349,
     683,  1325,   347,  1327,   515,  1224,   644,  1256,   603,  1443,
    1444,  -789,   334,   520,  1111,   525,  1115,   322,  1391,  1393,
    -290,  1450,  1595,   217,  1291,  1534,   689,   546,   913,  -825,
     644,  1601,  1647,   915,   508,   512,   513,   601,  1711,   644,
    -823,   271,   644,   722,  1187,  1235,  1042,   788,   789,  -229,
    1051,   804,   588,   884,  1024,  1182,  1631,   312,   345,  -213,
    1411,  -709,  1413,   600,  1395,   414,   551,  -786,  -791,  -787,
    -790,   317,   413,   290,   133,  -795,   420,  -822,  -792,   419,
     726,   606,   607,   608,   610,   611,   612,   613,   614,   615,
     616,   617,   618,   619,   620,   621,   622,   623,   624,   625,
     626,   627,   628,   629,   630,   631,  1343,   633,  1419,   634,
     634,   653,   637,  1397,   114,   127,   435,   575,  1310,  1248,
     654,   656,   657,   658,   659,   660,   661,   662,   663,   664,
     665,   666,   667,  1446,   410,   411,  1403,   690,   634,   677,
    -824,   600,   600,   634,   681,  1723,  -789,   840,   410,   411,
     654,  1716,  1730,   685,   299,  1737,  1564,   826,   299,   820,
     548,   393,  1311,   694,  -825,   696,   228,   288,  1238,  1017,
     710,   600,   327,   643,  1596,  -823,   829,   830,   291,   711,
    1043,   712,   649,  1245,  1044,   838,   336,   318,   516,  1113,
    1114,  1597,  1671,   319,  1598,  1717,  1731,   675,   292,  1113,
    1114,   775,   335,  -791,  -485,  -790,  1283,   414,   757,   286,
     119,   760,   763,   764,  -891,   392,   715,   287,   299,   698,
    1404,   643,   302,   303,   548,   873,   302,   303,  1353,  1724,
     699,  -891,   160,   705,   865,   541,  1672,  -628,  -764,  1738,
     867,   293,   783,  1110,   335,  1092,  1312,    13,  1203,  1093,
    1213,   297,   336,  1215,   213,   215,   247,   335,   299,   258,
    1062,   840,   299,   821,   328,   772,  -764,   905,   300,   776,
     298,   857,   877,   566,   279,  -824,   294,  1068,   822,  1718,
    1732,   779,   591,  -891,   780,   314,   302,   303,   299,   341,
     867,  1116,  1000,   310,   548,   294,  1423,   410,   411,   294,
     294,  1258,    59,    60,    61,   174,   175,   346,  1284,  1618,
    1331,   906,  1332,  1285,  1003,    59,    60,    61,   174,  1286,
     346,  1287,   847,   338,  -767,   936,   302,   303,   350,   301,
     302,   303,   796,   797,   858,   394,  -765,   335,   294,   324,
     335,   595,   595,   444,   299,   590,   983,   351,   335,  1191,
     331,   984,  -767,  -891,   299,   549,   302,   303,  1288,  1289,
     548,  1290,  1192,   636,  -765,   284,   866,   336,   575,   347,
     335,   381,   285,  1641,   336,   191,    59,    60,    61,   174,
     175,   346,   347,   382,  1112,  1113,  1114,  1588,   352,  1693,
    1642,   676,   353,  1643,   876,  1193,   680,    33,    34,    35,
     430,   566,  -891,  1589,  1304,  -891,   410,   411,  1184,   739,
     354,  1309,   302,   303,  1321,   355,  1422,   910,   911,   565,
    1590,   384,   302,   303,   385,  1025,   908,   976,   977,   978,
     979,   980,   981,  1708,   644,   386,  1706,   434,  1361,  1362,
    1034,   119,   238,   347,   437,   119,   982,   914,  1722,   463,
     443,  1719,  1046,  1219,   387,   133,   388,    73,    74,    75,
      76,    77,  1227,   160,  1252,  1113,  1114,   160,   741,   416,
     477,  1095,  1096,  1247,    80,    81,   979,   980,   981,   925,
     985,  -793,   375,   376,   377,   378,   379,   380,    90,   381,
    1560,  1561,   982,  1279,  1087,  -486,   127,  1712,  1713,   644,
    -628,   382,    95,  1568,  1697,  1698,  1699,  1639,  1640,   518,
      59,    60,    61,    62,    63,   346,   306,   133,   526,   526,
     531,    69,   389,  1635,  1636,   538,   422,  1420,   330,   332,
     333,   547,   119,   566,   508,   427,  1022,  1123,   512,  1067,
    1117,   592,  1118,   429,  1127,   258,   382,   565,   294,  1030,
    1301,   378,   379,   380,   160,   381,   336,   390,   127,   391,
    1339,   436,   133,   440,   417,   939,   942,   382,   128,   439,
    -627,   126,   445,   130,  1049,   131,  1348,   347,   446,   454,
     471,  -626,   133,   467,  1057,   472,  -625,  1058,   475,  1059,
     476,   478,   479,   600,   642,   294,   646,   294,   294,   481,
     521,   522,   523,   127,   534,   535,  1686,   567,   568,   -65,
    1323,   158,   589,   577,   578,   580,   602,   686,   670,    52,
     214,   214,   688,   127,  1686,   675,    52,   691,   713,  1091,
     697,   456,  1707,   575,    59,    60,    61,   174,   175,   346,
     717,   720,   692,  1209,  1098,   732,  1202,  1283,   838,   575,
     538,  1619,   755,   733,   702,   756,   758,  1424,   759,   228,
     434,   133,   767,   133,   770,   119,  1430,   544,  1099,   773,
     774,   550,   777,  1222,   778,  1231,   786,  1436,   790,   565,
     795,   791,   753,   794,   800,   802,   811,   160,    13,   805,
     705,   817,   816,   819,   825,   544,   824,   550,   544,   550,
     550,   347,   127,   827,   127,   828,   831,   836,  1408,   835,
     843,  1167,  1168,  1169,  -489,   845,   566,   760,  1171,   848,
     849,   851,   782,   860,   854,   863,  1269,   482,   483,   484,
    1046,   861,   864,  1274,   485,   486,  1185,   878,   487,   488,
     868,   882,   880,   917,   881,  1186,  1574,   808,   810,  1284,
     856,   927,   907,   933,  1285,   929,    59,    60,    61,   174,
    1286,   346,  1287,    59,    60,    61,    62,    63,   346,  1653,
     934,   935,   133,  1211,    69,   389,   956,   937,   128,   950,
     952,   126,   953,   130,   566,   131,   600,   955,  1281,   986,
     987,   988,   992,   995,  1008,   600,  1186,   998,  1020,  1288,
    1289,  1010,  1290,   214,  1016,  1021,  1029,  1035,  1027,  1033,
     214,   294,   391,   127,  1052,  1037,   214,  1061,  1064,  1071,
    1081,   158,  1070,   347,   846,  1086,  1082,  1085,   238,  1240,
     347,  1088,  1337,  1101,  1103,  1106,  1107,  1109,  1254,  1121,
    1122,   982,  1696,  1126,  1125,  1412,  1283,  1173,   543,   575,
    1179,  1183,   575,  1180,  1199,  1243,  1200,  1207,   133,  1212,
    1208,  1228,   565,  1220,  1214,  1216,  1218,  1221,  1230,  1349,
    1223,  1350,   891,  1234,  1396,   214,  1229,  1241,  1613,   875,
    1614,  1242,  1244,  1249,   214,   214,   532,    13,  1253,  1620,
    1259,   214,   895,  1260,   901,  1262,  1263,   214,  1266,   127,
     349,  1267,  1268,  1270,  1389,  1272,  1273,  1278,  1313,  1318,
    1314,  1302,  1334,   119,  1303,  1316,  1328,  1320,  1306,  1324,
    1329,   902,  1307,   903,  1308,  1330,  1336,   923,   119,  1295,
     565,  1333,  1315,  1335,  1656,   160,  1338,  1340,  1295,  1341,
    1342,  1367,  1322,   600,  1345,  1346,  1360,   921,  1284,  1358,
     160,  1378,  1394,  1285,  1399,    59,    60,    61,   174,  1286,
     346,  1287,  1405,  1544,  1406,  1409,  1414,  1417,  1415,  1437,
    1421,  1433,  1431,  1441,  1447,   119,  1448,  1542,  1300,  1541,
    1548,  1002,  1364,  1554,  1555,  1005,  1006,  1300,  -916,  1557,
    1559,  1569,  1558,  1599,  1567,  1283,  1579,   160,  1288,  1289,
    1604,  1290,  1357,  1580,  1608,  1013,  1607,  1637,  1615,  1645,
    1646,  1651,  1652,   575,  1659,  1009,   214,  1660,  1661,  -286,
     119,  1663,   347,  1664,  1667,  1594,   214,   133,  1668,  1440,
     538,  1019,  1721,  1675,  1032,  1670,    13,  1673,  1407,  1728,
     119,   600,   160,  1676,  1416,  1677,   670,  1682,   394,  1688,
    1691,  1694,  1695,  1703,  1720,  1705,  1584,  1709,  1725,  1710,
    1726,  1733,   160,  1734,  1739,  1445,  1740,  1295,   127,  1742,
    1001,   781,  1743,  1295,  1690,  1295,  1004,   212,   212,   294,
     645,   225,   648,  1387,   647,  1066,  1026,   133,  1546,  1063,
    1547,  1075,  1075,   895,  1246,  1704,   133,  1284,  1573,  1347,
    1702,  1428,  1285,  1565,    59,    60,    61,   174,  1286,   346,
    1287,   702,  1587,  1592,   784,  1727,  1300,   119,  1385,   119,
    1603,   119,  1300,  1715,  1300,  1606,   575,  1368,   127,   237,
    1556,  1563,  1172,   655,  1170,   765,   994,   127,  1194,   160,
    1119,   160,   766,   160,  1077,   921,  1105,  1288,  1289,  1226,
    1290,  1128,  1232,  1089,  1045,  1233,   940,  1605,  1129,   528,
    1577,  1135,   540,  1178,  1120,  1166,     0,  1295,     0,     0,
       0,   347,   133,     0,     0,     0,     0,     0,   133,     0,
       0,     0,  1571,  1428,   133,  1549,     0,     0,     0,   753,
     214,     0,     0,  1418,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1175,     0,     0,  1176,
    1602,     0,     0,   127,     0,     0,  1300,     0,     0,   127,
       0,     0,     0,     0,     0,   127,     0,     0,     0,     0,
     119,     0,     0,     0,     0,     0,     0,     0,  1680,  1665,
       0,     0,     0,     0,     0,   214,     0,     0,     0,     0,
       0,     0,   160,     0,     0,     0,     0,     0,     0,     0,
     212,     0,     0,     0,   210,   210,     0,   212,   223,     0,
       0,     0,  1649,   212,     0,  1210,  1283,     0,     0,  1609,
       0,     0,     0,     0,     0,     0,   349,   214,   895,   214,
       0,   223,   895,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   119,     0,     0,     0,     0,   838,
       0,     0,     0,   214,     0,     0,   119,    13,  1239,     0,
       0,     0,   133,     0,   838,     0,   160,     0,     0,     0,
       0,     0,   212,     0,   538,   921,     0,     0,   160,     0,
       0,   212,   212,     0,     0,     0,     0,     0,   212,     0,
       0,     0,     0,     0,   212,     0,     0,     0,     0,     0,
       0,   133,   133,   127,     0,   564,     0,     0,   133,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1284,     0,
       0,   214,     0,  1285,  1280,    59,    60,    61,   174,  1286,
     346,  1287,     0,     0,     0,     0,   214,   214,     0,     0,
       0,     0,   127,   127,   133,     0,     0,     0,     0,   127,
       0,   538,  1681,  -917,  -917,  -917,  -917,   373,   374,   375,
     376,   377,   378,   379,   380,     0,   381,     0,  1288,  1289,
       0,  1290,   225,     0,   895,     0,   895,     0,   382,     0,
       0,     0,     0,     0,     0,   127,     0,   210,     0,     0,
    1735,     0,   347,     0,   210,     0,     0,     0,  1741,     0,
     210,     0,     0,     0,  1744,     0,   575,  1745,     0,   133,
       0,     0,     0,   212,  1566,     0,   133,     0,     0,     0,
       0,     0,     0,   212,   575,   119,     0,     0,   223,   223,
     258,     0,   575,     0,     0,   223,  1383,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   160,     0,     0,
     127,   214,   214,     0,     0,     0,     0,   127,     0,   210,
       0,     0,     0,     0,     0,     0,     0,     0,   210,   210,
       0,     0,     0,     0,     0,   210,     0,     0,     0,     0,
     895,   210,     0,     0,     0,   119,     0,     0,     0,     0,
     119,     0,   223,     0,   119,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,   160,   207,   294,
       0,     0,   160,   258,   223,     0,   160,   223,     0,     0,
       0,     0,  1530,     0,     0,     0,     0,     0,  1537,     0,
       0,     0,     0,     0,     0,     0,   258,     0,   258,     0,
       0,     0,     0,     0,   258,     0,     0,     0,     0,     0,
       0,     0,     0,   640,     0,     0,     0,     0,     0,   223,
       0,     0,     0,     0,     0,     0,     0,   895,     0,     0,
     119,   119,   119,     0,     0,     0,   119,     0,     0,     0,
       0,   214,   119,     0,     0,    84,    85,   212,    86,   179,
      88,     0,   160,   160,   160,     0,     0,     0,   160,     0,
     210,     0,     0,     0,   160,     0,     0,     0,     0,     0,
     210,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   214,     0,     0,     0,     0,     0,
     641,     0,   114,   356,   357,   358,     0,     0,     0,     0,
     214,   214,   212,     0,     0,     0,     0,     0,     0,   223,
     223,     0,   359,   745,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,     0,   381,     0,     0,     0,
       0,     0,     0,     0,   212,     0,   212,     0,   382,     0,
     294,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   745,     0,     0,     0,     0,     0,     0,     0,     0,
     212,   258,     0,     0,     0,   895,     0,   214,     0,     0,
     119,     0,     0,     0,     0,     0,     0,     0,     0,  1625,
       0,     0,     0,     0,     0,  1530,  1530,     0,     0,  1537,
    1537,     0,   160,     0,     0,   223,   223,     0,     0,     0,
       0,     0,   294,     0,   223,     0,     0,     0,     0,   119,
     119,     0,     0,     0,     0,     0,   119,     0,     0,     0,
       0,     0,     0,     0,   210,     0,     0,     0,   212,     0,
       0,   160,   160,     0,     0,     0,     0,     0,   160,     0,
       0,     0,     0,   212,   212,     0,     0,     0,     0,     0,
       0,     0,   119,     0,     0,     0,     0,     0,     0,  1679,
       0,     0,     0,   356,   357,   358,   564,     0,     0,     0,
       0,     0,     0,     0,   160,   812,  1692,     0,     0,   210,
       0,     0,   359,     0,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,     0,   381,     0,     0,     0,
       0,     0,     0,     0,     0,   225,     0,   119,   382,     0,
       0,   210,     0,   210,   119,  -917,  -917,  -917,  -917,   974,
     975,   976,   977,   978,   979,   980,   981,     0,     0,   160,
       0,     0,     0,     0,     0,     0,   160,   210,   745,     0,
     982,     0,   356,   357,   358,     0,     0,     0,   212,   212,
     223,   223,   745,   745,   745,   745,   745,     0,     0,     0,
       0,   359,   745,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   564,   381,     0,     0,   223,     0,
       0,     0,     0,     0,     0,     0,     0,   382,     0,     0,
       0,     0,     0,     0,     0,   210,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   223,     0,
     210,   210,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   223,   223,     0,     0,     0,     0,
       0,     0,     0,   223,     0,   842,     0,     0,     0,   223,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   223,     0,     0,     0,     0,     0,     0,     0,   745,
       0,     0,   223,     0,     0,     0,     0,     0,   212,     0,
       0,     0,     0,     0,     0,     0,   211,   211,     0,     0,
     224,     0,   223,     0,     0,     0,     0,     0,   425,   396,
     397,   398,   399,   400,   401,   402,   403,   404,   405,   406,
     407,     0,     0,   260,     0,     0,   564,     0,     0,     0,
       0,   212,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   879,   210,   210,   212,   212,     0,
       0,     0,     0,     0,     0,     0,     0,   408,   409,   223,
       0,   223,     0,   223,     0,   425,   396,   397,   398,   399,
     400,   401,   402,   403,   404,   405,   406,   407,   745,     0,
       0,   223,     0,     0,   745,   745,   745,   745,   745,   745,
     745,   745,   745,   745,   745,   745,   745,   745,   745,   745,
     745,   745,   745,   745,   745,   745,   745,   745,   745,   745,
     745,     0,     0,     0,   408,   409,     0,     0,   356,   357,
     358,     0,   410,   411,   212,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   745,   359,     0,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
       0,   381,     0,     0,     0,   223,     0,   223,     0,     0,
       0,     0,     0,   382,     0,   210,     0,     0,     0,   410,
     411,     0,     0,     0,     0,     0,   211,     0,     0,     0,
       0,     0,   223,   579,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   564,
       0,     0,     0,   223,     0,     0,     0,     0,   210,     0,
     260,   260,     0,     0,     0,     0,     0,   260,     0,     0,
       0,     0,     0,     0,   210,   210,     0,   745,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   223,
     792,   211,   745,     0,   745,     0,     0,     0,     0,    36,
     211,   211,     0,     0,     0,     0,     0,   211,     0,     0,
     745,     0,     0,   211,     0,     0,     0,   564,     0,     0,
       0,     0,     0,     0,   211,     0,     0,     0,     0,     0,
       0,     0,     0,   356,   357,   358,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   260,   223,   223,   260,
     883,   210,   359,     0,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   306,   381,     0,    84,    85,
       0,    86,   179,    88,     0,     0,     0,     0,   382,     0,
       0,   224,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   750,     0,     0,
       0,     0,     0,   307,     0,     0,     0,     0,   223,     0,
     223,     0,   211,     0,     0,   745,   223,     0,     0,   745,
       0,   745,     0,     0,   745,     0,     0,     0,     0,     0,
       0,     0,   223,   223,     0,     0,   223,     0,     0,     0,
       0,     0,     0,   223,     0,   750,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   260,     0,     0,     0,   749,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   223,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   957,   958,   959,     0,   745,
       0,     0,     0,     0,     0,  1007,     0,     0,   223,   223,
     223,     0,     0,   749,   960,   223,   961,   962,   963,   964,
     965,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   978,   979,   980,   981,     0,   223,     0,
     223,     0,     0,     0,     0,     0,   223,     0,     0,     0,
     982,     0,     0,     0,     0,     0,     0,   260,   260,     0,
       0,     0,     0,     0,     0,    36,   260,   207,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   356,   357,   358,
       0,   745,   745,     0,     0,     0,   211,     0,     0,   745,
     223,     0,     0,     0,     0,   223,   359,   223,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,     0,
     381,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   382,     0,     0,     0,     0,     0,     0,     0,
       0,   211,   750,     0,    84,    85,     0,    86,   179,    88,
       0,     0,     0,     0,     0,     0,   750,   750,   750,   750,
     750,     0,     0,     0,     0,     0,   750,     0,     0,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   211,  1133,   211,     0,     0,     0,   641,
       0,   114,     0,     0,     0,     0,   223,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   211,
     749,     0,     0,   223,     0,     0,     0,     0,     0,     0,
       0,     0,   260,   260,   749,   749,   749,   749,   749,     0,
       0,   223,     0,     0,   749,     0,   745,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   745,     0,     0,
       0,     0,   745,     0,     0,     0,   745,     0,     0,     0,
     997,     0,     0,     0,     0,     0,     0,     0,     0,  1060,
       0,     0,     0,   750,     0,     0,     0,   211,   223,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1015,     0,   211,   211,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   260,  1015,     0,   259,
     751,     0,     0,     0,     0,   211,   745,     0,     0,     0,
       0,   260,     0,     0,   223,     0,     0,     0,     0,     0,
       0,     0,     0,   260,     0,     0,     0,     0,     0,   223,
       0,   749,     0,     0,  1053,     0,     0,     0,   223,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   785,     0,
       0,     0,     0,   223,   224,     0,     0,     0,     0,     0,
       0,     0,   750,     0,     0,     0,     0,     0,   750,   750,
     750,   750,   750,   750,   750,   750,   750,   750,   750,   750,
     750,   750,   750,   750,   750,   750,   750,   750,   750,   750,
     750,   750,   750,   750,   750,     0,     0,   211,   211,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   260,     0,   260,     0,     0,     0,     0,     0,     0,
     750,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     749,     0,     0,   211,     0,     0,   749,   749,   749,   749,
     749,   749,   749,   749,   749,   749,   749,   749,   749,   749,
     749,   749,   749,   749,   749,   749,   749,   749,   749,   749,
     749,   749,   749,   359,     0,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,     0,   381,   749,   250,
       0,     0,     0,     0,     0,     0,   259,   259,     0,   382,
       0,     0,     0,   259,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   251,     0,   260,     0,   260,
       0,   750,     0,     0,     0,     0,     0,   211,     0,     0,
       0,     0,     0,     0,     0,     0,   750,    36,   750,     0,
       0,     0,     0,     0,   260,   922,     0,     0,     0,     0,
       0,     0,     0,     0,   750,     0,     0,     0,     0,   943,
     944,   945,   946,     0,     0,   211,     0,     0,     0,   954,
     211,     0,     0,     0,     0,     0,     0,     0,   252,     0,
       0,     0,   259,     0,     0,   259,   211,   211,     0,   749,
       0,     0,   253,   254,     0,     0,     0,     0,     0,     0,
       0,   260,     0,     0,   749,     0,   749,     0,     0,     0,
     178,     0,     0,    82,   255,     0,    84,    85,     0,    86,
     179,    88,   749,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   256,     0,     0,     0,     0,     0,
       0,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,     0,   257,   260,
    1282,     0,  1550,   211,     0,     0,     0,     0,     0,     0,
     356,   357,   358,     0,     0,     0,  1050,     0,     0,   750,
       0,     0,     0,   750,     0,   750,     0,     0,   750,   359,
       0,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,     0,   381,     0,     0,     0,   259,   728,     0,
       0,   747,     0,     0,     0,   382,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     260,     0,   260,     0,     0,     0,     0,   749,   211,     0,
       0,   749,     0,   749,     0,     0,   749,     0,     0,     0,
       0,     0,     0,   750,   260,  1366,     0,     0,  1377,   747,
       0,     0,     0,     0,     0,   260,     0,     0,     0,     0,
       0,  1136,  1139,  1140,  1141,  1143,  1144,  1145,  1146,  1147,
    1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,
    1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,     0,     0,
       0,     0,     0,   259,   259,     0,   211,     0,     0,     0,
       0,     0,   259,     0,     0,     0,     0,     0,     0,     0,
       0,   749,     0,  1177,     0,     0,     0,     0,     0,     0,
     260,  1438,  1439,     0,     0,   750,   750,   260,     0,     0,
       0,     0,     0,   750,     0,     0,     0,     0,     0,     0,
    1586,     0,  1069,   356,   357,   358,     0,     0,     0,     0,
     260,     0,   260,     0,     0,     0,     0,     0,   260,     0,
       0,     0,   359,     0,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,     0,   381,     0,     0,     0,
       0,     0,     0,   749,   749,     0,     0,     0,   382,     0,
       0,   749,  1582,     0,    36,     0,   207,   260,     0,  1377,
       0,     0,     0,     0,  1250,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1264,
       0,  1265,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   747,  1275,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   259,   259,
     747,   747,   747,   747,   747,     0,     0,     0,     0,     0,
     747,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     750,   668,     0,    84,    85,     0,    86,   179,    88,     0,
       0,   750,     0,     0,     0,     0,   750,     0,     0,     0,
     750,     0,     0,     0,     0,     0,     0,     0,   260,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,  1666,   260,     0,     0,   669,     0,
     114,     0,     0,     0,     0,  1094,     0,     0,     0,     0,
       0,   746,   259,     0,     0,     0,     0,     0,   749,     0,
       0,     0,     0,     0,     0,     0,     0,   259,     0,   749,
     750,     0,     0,     0,   749,     0,     0,     0,   749,   259,
       0,     0,  1352,     0,     0,     0,  1354,   747,  1355,     0,
       0,  1356,   356,   357,   358,     0,     0,     0,     0,   746,
     260,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   359,     0,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,     0,   381,     0,     0,   749,     0,
       0,     0,     0,     0,     0,     0,  1689,   382,     0,     0,
       0,     0,    36,     0,   207,     0,     0,     0,     0,     0,
       0,  1366,     0,     0,     0,     0,  1432,   259,     0,   259,
     260,   728,   395,   396,   397,   398,   399,   400,   401,   402,
     403,   404,   405,   406,   407,   260,   747,     0,     0,     0,
       0,     0,   747,   747,   747,   747,   747,   747,   747,   747,
     747,   747,   747,   747,   747,   747,   747,   747,   747,   747,
     747,   747,   747,   747,   747,   747,   747,   747,   747,     0,
       0,   408,   409,     0,     0,     0,     0,     0,     0,   668,
       0,    84,    85,     0,    86,   179,    88,     0,     0,     0,
       0,     0,     0,     0,   747,     0,     0,     0,  1575,  1576,
       0,     0,     0,     0,     0,     0,  1581,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,     0,   259,     0,   259,   701,     0,   114,     0,
       0,     0,     0,     0,  1401,    36,   410,   411,     0,     0,
       0,     0,     0,     0,     0,     0,   746,     0,     0,     0,
     259,     0,     0,   356,   357,   358,     0,     0,     0,     0,
     746,   746,   746,   746,   746,     0,     0,     0,     0,     0,
     746,     0,   359,     0,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   747,   381,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   259,   382,     0,
     747,     0,   747,     0,    84,    85,     0,    86,   179,    88,
       0,     0,     0,     0,     0,     0,     0,     0,   747,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,  1634,     0,     0,   602,     0,     0,   356,
     357,   358,     0,     0,  1644,   259,     0,     0,     0,  1648,
       0,     0,     0,  1650,     0,     0,     0,   746,   359,  1255,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,     0,   381,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   382,     0,     0,     0,   425,   396,
     397,   398,   399,   400,   401,   402,   403,   404,   405,   406,
     407,     0,     0,  1683,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1402,   259,     0,   259,     0,
       0,     0,     0,   747,     0,     0,     0,   747,     0,   747,
       0,     0,   747,     0,     0,     0,     0,   408,   409,     0,
     259,     0,     0,     0,     0,     0,   746,     0,     0,     0,
       0,   259,   746,   746,   746,   746,   746,   746,   746,   746,
     746,   746,   746,   746,   746,   746,   746,   746,   746,   746,
     746,   746,   746,   746,   746,   746,   746,   746,   746,   963,
     964,   965,   966,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,   978,   979,   980,   981,  1072,  1073,
    1074,    36,   410,   411,   746,     0,     0,   747,     0,     0,
       0,   982,     0,     0,     0,     0,   259,     0,     0,  1256,
       0,     0,     0,   259,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   356,   357,   358,     0,
       0,     0,     0,     0,     0,     0,   259,     0,   259,     0,
       0,     0,     0,     0,   259,   359,     0,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,     0,   381,
      84,    85,     0,    86,   179,    88,     0,     0,     0,   747,
     747,   382,     0,     0,     0,     0,     0,   747,     0,     0,
      36,     0,     0,   259,     0,   746,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
     746,     0,   746,   356,   357,   358,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   746,     0,
       0,     0,   359,     0,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,     0,   381,     0,     0,     0,
       0,     0,   250,   178,     0,     0,    82,     0,   382,    84,
      85,     0,    86,   179,    88,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   251,     0,
       0,     0,     0,     0,   259,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
      36,   259,     0,     0,     0,  1624,   383,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1626,
       0,     0,     0,     0,   747,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   747,     0,     0,     0,     0,
     747,   252,     0,   746,   747,     0,     0,   746,     0,   746,
       0,     0,   746,     0,     0,   253,   254,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   259,     0,     0,     0,
       0,     0,     0,   178,     0,     0,    82,   255,     0,    84,
      85,     0,    86,   179,    88,     0,     0,     0,     0,     0,
       0,     0,     0,   466,     0,     0,     0,   256,     0,     0,
       0,     0,     0,     0,   747,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
       0,   257,     0,     0,     0,  1617,     0,   746,     5,     6,
       7,     8,     9,     0,     0,     0,   259,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   259,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,    40,     0,   746,
     746,    41,    42,    43,    44,     0,    45,   746,    46,     0,
      47,     0,     0,    48,     0,     0,     0,    49,    50,    51,
      52,    53,    54,    55,     0,    56,    57,    58,    59,    60,
      61,    62,    63,    64,     0,    65,    66,    67,    68,    69,
      70,     0,     0,     0,     0,     0,    71,    72,     0,    73,
      74,    75,    76,    77,     0,     0,     0,     0,     0,     0,
      78,     0,     0,     0,     0,    79,    80,    81,    82,    83,
       0,    84,    85,     0,    86,    87,    88,    89,     0,     0,
      90,     0,     0,    91,     0,     0,     0,     0,     0,    92,
      93,     0,    94,     0,    95,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,     0,   111,     0,   112,   113,  1023,   114,   115,
       0,   116,   117,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,   746,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,   746,     0,     0,     0,     0,
     746,     0,     0,     0,   746,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,     0,     0,   746,    49,    50,    51,    52,    53,
      54,    55,     0,    56,    57,    58,    59,    60,    61,    62,
      63,    64,     0,    65,    66,    67,    68,    69,    70,     0,
       0,     0,     0,     0,    71,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,    79,    80,    81,    82,    83,     0,    84,
      85,     0,    86,    87,    88,    89,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,    93,     0,
      94,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,   112,   113,  1188,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
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
       0,     0,    92,    93,     0,    94,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,     0,     0,     0,    49,    50,    51,    52,     0,
      54,    55,     0,    56,     0,    58,    59,    60,    61,    62,
      63,    64,     0,    65,    66,    67,     0,    69,    70,     0,
       0,     0,     0,     0,    71,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   178,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   179,    88,    89,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,     0,     0,
       0,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,   112,   113,   581,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
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
       0,     0,     0,    78,     0,     0,     0,     0,   178,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   179,    88,
      89,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,     0,     0,     0,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
     996,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,     0,     0,     0,    49,    50,    51,    52,     0,
      54,    55,     0,    56,     0,    58,    59,    60,    61,    62,
      63,    64,     0,    65,    66,    67,     0,    69,    70,     0,
       0,     0,     0,     0,    71,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   178,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   179,    88,    89,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,     0,     0,
       0,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,   112,   113,  1036,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
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
       0,     0,     0,    78,     0,     0,     0,     0,   178,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   179,    88,
      89,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,     0,     0,     0,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
    1100,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,  1102,    45,     0,    46,     0,    47,     0,
       0,    48,     0,     0,     0,    49,    50,    51,    52,     0,
      54,    55,     0,    56,     0,    58,    59,    60,    61,    62,
      63,    64,     0,    65,    66,    67,     0,    69,    70,     0,
       0,     0,     0,     0,    71,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   178,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   179,    88,    89,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,     0,     0,
       0,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,   112,   113,     0,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,  1251,     0,    48,     0,     0,     0,
      49,    50,    51,    52,     0,    54,    55,     0,    56,     0,
      58,    59,    60,    61,    62,    63,    64,     0,    65,    66,
      67,     0,    69,    70,     0,     0,     0,     0,     0,    71,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   178,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   179,    88,
      89,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,     0,     0,     0,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,     0,     0,     0,    49,    50,    51,    52,     0,
      54,    55,     0,    56,     0,    58,    59,    60,    61,    62,
      63,    64,     0,    65,    66,    67,     0,    69,    70,     0,
       0,     0,     0,     0,    71,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   178,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   179,    88,    89,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,     0,     0,
       0,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,   112,   113,  1359,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
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
       0,     0,     0,    78,     0,     0,     0,     0,   178,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   179,    88,
      89,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,     0,     0,     0,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
    1578,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,  1621,    47,     0,
       0,    48,     0,     0,     0,    49,    50,    51,    52,     0,
      54,    55,     0,    56,     0,    58,    59,    60,    61,    62,
      63,    64,     0,    65,    66,    67,     0,    69,    70,     0,
       0,     0,     0,     0,    71,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   178,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   179,    88,    89,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,     0,     0,
       0,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,   112,   113,     0,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
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
       0,     0,     0,    78,     0,     0,     0,     0,   178,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   179,    88,
      89,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,     0,     0,     0,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
    1654,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,     0,     0,     0,    49,    50,    51,    52,     0,
      54,    55,     0,    56,     0,    58,    59,    60,    61,    62,
      63,    64,     0,    65,    66,    67,     0,    69,    70,     0,
       0,     0,     0,     0,    71,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   178,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   179,    88,    89,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,     0,     0,
       0,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,   112,   113,  1655,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
    1658,    46,     0,    47,     0,     0,    48,     0,     0,     0,
      49,    50,    51,    52,     0,    54,    55,     0,    56,     0,
      58,    59,    60,    61,    62,    63,    64,     0,    65,    66,
      67,     0,    69,    70,     0,     0,     0,     0,     0,    71,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   178,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   179,    88,
      89,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,     0,     0,     0,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,     0,     0,     0,    49,    50,    51,    52,     0,
      54,    55,     0,    56,     0,    58,    59,    60,    61,    62,
      63,    64,     0,    65,    66,    67,     0,    69,    70,     0,
       0,     0,     0,     0,    71,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   178,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   179,    88,    89,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,     0,     0,
       0,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,   112,   113,  1674,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
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
       0,     0,     0,    78,     0,     0,     0,     0,   178,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   179,    88,
      89,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,     0,     0,     0,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
    1729,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,     0,     0,     0,    49,    50,    51,    52,     0,
      54,    55,     0,    56,     0,    58,    59,    60,    61,    62,
      63,    64,     0,    65,    66,    67,     0,    69,    70,     0,
       0,     0,     0,     0,    71,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   178,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   179,    88,    89,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,     0,     0,
       0,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,   112,   113,  1736,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
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
       0,     0,     0,    78,     0,     0,     0,     0,   178,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   179,    88,
      89,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,     0,     0,     0,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,   452,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,     0,     0,     0,    49,    50,    51,    52,     0,
      54,    55,     0,    56,     0,    58,    59,    60,    61,   174,
     175,    64,     0,    65,    66,    67,     0,     0,     0,     0,
       0,     0,     0,     0,    71,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   178,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   179,    88,     0,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,     0,     0,
       0,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,   112,   113,     0,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,   714,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,     0,     0,     0,
      49,    50,    51,    52,     0,    54,    55,     0,    56,     0,
      58,    59,    60,    61,   174,   175,    64,     0,    65,    66,
      67,     0,     0,     0,     0,     0,     0,     0,     0,    71,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   178,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   179,    88,
       0,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,     0,     0,     0,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,   924,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,     0,     0,     0,    49,    50,    51,    52,     0,
      54,    55,     0,    56,     0,    58,    59,    60,    61,   174,
     175,    64,     0,    65,    66,    67,     0,     0,     0,     0,
       0,     0,     0,     0,    71,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   178,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   179,    88,     0,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,     0,     0,
       0,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,   112,   113,     0,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,  1427,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,     0,     0,     0,
      49,    50,    51,    52,     0,    54,    55,     0,    56,     0,
      58,    59,    60,    61,   174,   175,    64,     0,    65,    66,
      67,     0,     0,     0,     0,     0,     0,     0,     0,    71,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   178,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   179,    88,
       0,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,     0,     0,     0,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,  1570,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,    40,     0,     0,     0,    41,
      42,    43,    44,     0,    45,     0,    46,     0,    47,     0,
       0,    48,     0,     0,     0,    49,    50,    51,    52,     0,
      54,    55,     0,    56,     0,    58,    59,    60,    61,   174,
     175,    64,     0,    65,    66,    67,     0,     0,     0,     0,
       0,     0,     0,     0,    71,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   178,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   179,    88,     0,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,     0,     0,
       0,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,   112,   113,     0,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,     0,     0,     0,
      49,    50,    51,    52,     0,    54,    55,     0,    56,     0,
      58,    59,    60,    61,   174,   175,    64,     0,    65,    66,
      67,     0,     0,     0,     0,     0,     0,     0,     0,    71,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   178,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   179,    88,
       0,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,     0,     0,     0,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     650,    12,     0,     0,     0,     0,     0,     0,   651,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    52,     0,
       0,     0,     0,     0,     0,     0,    59,    60,    61,   174,
     175,   176,     0,     0,    66,    67,     0,     0,     0,     0,
       0,     0,     0,     0,   177,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   178,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   179,    88,     0,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,     0,     0,
       0,     0,    95,    96,   265,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,     0,     0,     0,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    52,     0,     0,     0,     0,     0,     0,
       0,    59,    60,    61,   174,   175,   176,     0,     0,    66,
      67,     0,     0,     0,     0,     0,     0,     0,     0,   177,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   178,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   179,    88,
       0,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,     0,     0,     0,     0,    95,    96,   265,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   266,     0,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,   960,    10,   961,   962,   963,
     964,   965,   966,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,   978,   979,   980,   981,   596,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,   982,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    52,     0,
       0,     0,     0,     0,     0,     0,    59,    60,    61,   174,
     175,   176,     0,     0,    66,    67,     0,     0,     0,     0,
       0,     0,     0,     0,   177,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   178,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   179,    88,     0,   597,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,     0,     0,
       0,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,     0,     0,     0,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    52,     0,     0,     0,     0,     0,     0,
       0,    59,    60,    61,   174,   175,   176,     0,     0,    66,
      67,     0,     0,     0,     0,     0,     0,     0,     0,   177,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   178,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   179,    88,
       0,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,     0,     0,     0,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,   357,   358,   709,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,   359,    10,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,  1047,   381,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,   382,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    52,     0,
       0,     0,     0,     0,     0,     0,    59,    60,    61,   174,
     175,   176,     0,     0,    66,    67,     0,     0,     0,     0,
       0,     0,     0,     0,   177,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   178,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   179,    88,     0,  1048,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,     0,     0,
       0,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,     0,     0,     0,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   650,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    52,     0,     0,     0,     0,     0,     0,
       0,    59,    60,    61,   174,   175,   176,     0,     0,    66,
      67,     0,     0,     0,     0,     0,     0,     0,     0,   177,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   178,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   179,    88,
       0,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,     0,     0,     0,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   356,   357,
     358,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   359,     0,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
       0,   381,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,   382,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   190,     0,     0,    52,     0,
       0,     0,     0,     0,     0,     0,    59,    60,    61,   174,
     175,   176,     0,     0,    66,    67,     0,     0,     0,     0,
       0,     0,     0,     0,   177,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   178,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   179,    88,     0,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,   989,   990,
       0,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,   958,   959,     0,     0,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
     960,    10,   961,   962,   963,   964,   965,   966,   967,   968,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   980,   981,   216,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,   982,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    52,     0,     0,     0,     0,     0,     0,
       0,    59,    60,    61,   174,   175,   176,     0,     0,    66,
      67,     0,     0,     0,     0,     0,     0,     0,     0,   177,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   178,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   179,    88,
       0,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,     0,     0,     0,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   356,   357,
     358,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   359,     0,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
       0,   381,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,   382,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    52,     0,
       0,     0,     0,     0,     0,     0,    59,    60,    61,   174,
     175,   176,     0,     0,    66,    67,     0,     0,     0,     0,
       0,     0,     0,     0,   177,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   178,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   179,    88,     0,     0,     0,    90,     0,
       0,    91,     0,     0,     0,  1622,     0,    92,     0,     0,
       0,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,   245,     0,   358,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   359,     0,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,     0,   381,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,   382,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    52,     0,     0,     0,     0,     0,     0,
       0,    59,    60,    61,   174,   175,   176,     0,     0,    66,
      67,     0,     0,     0,     0,     0,     0,     0,     0,   177,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   178,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   179,    88,
       0,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,     0,     0,     0,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   248,     0,
     959,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   960,     0,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,   980,   981,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,   982,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    52,     0,
       0,     0,     0,     0,     0,     0,    59,    60,    61,   174,
     175,   176,     0,     0,    66,    67,     0,     0,     0,     0,
       0,     0,     0,     0,   177,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   178,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   179,    88,     0,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,     0,     0,
       0,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,   450,     0,     0,     0,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   609,   381,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   382,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    52,     0,     0,     0,     0,     0,     0,
       0,    59,    60,    61,   174,   175,   176,     0,     0,    66,
      67,     0,     0,     0,     0,     0,     0,     0,     0,   177,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   178,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   179,    88,
       0,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,     0,     0,     0,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,     0,     0,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   651,   381,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,   382,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    52,     0,
       0,     0,     0,     0,     0,     0,    59,    60,    61,   174,
     175,   176,     0,     0,    66,    67,     0,     0,     0,     0,
       0,     0,     0,     0,   177,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   178,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   179,    88,     0,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,     0,     0,
       0,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,     0,     0,     0,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   961,   962,   963,   964,   965,   966,   967,   968,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   980,   981,   693,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,   982,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    52,     0,     0,     0,     0,     0,     0,
       0,    59,    60,    61,   174,   175,   176,     0,     0,    66,
      67,     0,     0,     0,     0,     0,     0,     0,     0,   177,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   178,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   179,    88,
       0,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,     0,     0,     0,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,     0,     0,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   695,   381,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,   382,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    52,     0,
       0,     0,     0,     0,     0,     0,    59,    60,    61,   174,
     175,   176,     0,     0,    66,    67,     0,     0,     0,     0,
       0,     0,     0,     0,   177,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   178,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   179,    88,     0,     0,     0,    90,     0,
       0,    91,     0,     0,     0,     0,     0,    92,     0,     0,
       0,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,     0,     0,     0,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,   962,   963,   964,   965,   966,   967,   968,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   980,   981,  1090,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,   982,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    52,     0,     0,     0,     0,     0,     0,
       0,    59,    60,    61,   174,   175,   176,     0,     0,    66,
      67,     0,     0,     0,     0,     0,     0,     0,     0,   177,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   178,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   179,    88,
       0,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,     0,     0,     0,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   356,   357,
     358,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   359,     0,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
       0,   381,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,   382,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    52,     0,
       0,     0,     0,     0,     0,     0,    59,    60,    61,   174,
     175,   176,     0,     0,    66,    67,     0,     0,     0,     0,
       0,     0,     0,     0,   177,    72,     0,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,   178,    80,    81,    82,    83,     0,    84,
      85,     0,    86,   179,    88,     0,     0,     0,    90,     0,
       0,    91,     0,     0,  1435,     0,     0,    92,     0,     0,
       0,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,   356,   357,   358,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   359,     0,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,     0,   381,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,   382,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,   545,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    52,     0,     0,     0,     0,     0,     0,
       0,    59,    60,    61,   174,   175,   176,     0,     0,    66,
      67,     0,     0,     0,     0,     0,     0,     0,     0,   177,
      72,     0,    73,    74,    75,    76,    77,     0,    36,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   178,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   179,    88,
       0,     0,     0,    90,     0,     0,    91,     0,  1277,     0,
       0,     0,    92,     0,     0,     0,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,     0,     0,
       0,   114,   115,     0,   116,   117,  1451,  1452,  1453,  1454,
    1455,     0,     0,  1456,  1457,  1458,  1459,    84,    85,     0,
      86,   179,    88,     0,     0,     0,     0,     0,     0,     0,
    1460,  1461,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,  1462,     0,     0,   856,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1463,  1464,  1465,  1466,  1467,  1468,  1469,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1470,  1471,  1472,  1473,  1474,  1475,  1476,  1477,  1478,  1479,
    1480,  1481,  1482,  1483,  1484,  1485,  1486,  1487,  1488,  1489,
    1490,  1491,  1492,  1493,  1494,  1495,  1496,  1497,  1498,  1499,
    1500,  1501,  1502,  1503,  1504,  1505,  1506,  1507,  1508,  1509,
    1510,     0,     0,     0,  1511,  1512,     0,  1513,  1514,  1515,
    1516,  1517,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1518,  1519,  1520,     0,     0,     0,    84,
      85,     0,    86,   179,    88,  1521,     0,  1522,  1523,     0,
    1524,     0,     0,     0,     0,     0,     0,  1525,  1526,     0,
    1527,     0,  1528,  1529,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   356,   357,
     358,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   359,     0,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
       0,   381,   356,   357,   358,     0,     0,     0,     0,     0,
       0,     0,     0,   382,     0,     0,     0,     0,     0,     0,
       0,   359,     0,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,     0,   381,   356,   357,   358,     0,
       0,     0,     0,     0,     0,     0,     0,   382,     0,     0,
       0,     0,     0,     0,     0,   359,     0,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,     0,   381,
     356,   357,   358,     0,     0,     0,     0,     0,     0,     0,
       0,   382,     0,     0,     0,     0,     0,     0,     0,   359,
       0,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,     0,   381,   356,   357,   358,     0,     0,     0,
       0,     0,     0,     0,     0,   382,    36,     0,   468,     0,
       0,     0,     0,   359,     0,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,     0,   381,   356,   357,
     358,     0,     0,     0,     0,     0,     0,     0,     0,   382,
       0,     0,   480,     0,     0,     0,     0,   359,     0,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
       0,   381,   250,  1535,     0,    84,    85,  1536,    86,   179,
      88,     0,     0,   382,     0,     0,   504,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   251,     0,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,     0,  1382,   250,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,   684,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   251,     0,     0,     0,  -330,     0,
       0,     0,     0,     0,     0,     0,    59,    60,    61,   174,
     175,  1388,     0,     0,     0,   250,    36,     0,     0,     0,
       0,     0,     0,   706,     0,   253,   254,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   251,     0,   178,   473,     0,    82,   255,     0,    84,
      85,     0,    86,   179,    88,     0,     0,   252,     0,     0,
       0,     0,   993,    36,     0,     0,     0,   256,     0,     0,
       0,   253,   254,   347,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,   178,
       0,   257,    82,   255,     0,    84,    85,     0,    86,   179,
      88,     0,     0,     0,   252,     0,     0,     0,   250,    36,
       0,     0,     0,   256,     0,     0,     0,     0,   253,   254,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   251,     0,   178,   257,     0,    82,
     255,     0,    84,    85,     0,    86,   179,    88,     0,   938,
       0,     0,     0,     0,  1381,   250,    36,     0,     0,     0,
     256,     0,     0,     0,     0,     0,     0,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   251,     0,     0,   257,     0,     0,     0,    84,    85,
       0,    86,   179,    88,     0,     0,     0,   252,     0,     0,
       0,   250,     0,    36,     0,     0,     0,     0,     0,     0,
       0,   253,   254,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   251,     0,   178,
    1382,     0,    82,   255,     0,    84,    85,     0,    86,   179,
      88,     0,  1261,     0,   252,     0,     0,     0,     0,    36,
       0,     0,     0,   256,     0,     0,     0,     0,   253,   254,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,   178,   257,     0,    82,
     255,     0,    84,    85,     0,    86,   179,    88,     0,     0,
     252,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     256,  1363,     0,     0,   253,   254,    36,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,    36,   178,     0,   257,    82,   255,     0,    84,    85,
       0,    86,   179,    88,   274,   275,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   256,     0,     0,     0,
       0,  1142,     0,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   734,   735,     0,
     257,     0,     0,   736,     0,   737,     0,     0,     0,     0,
       0,     0,   276,     0,     0,    84,    85,   738,    86,   179,
      88,     0,     0,     0,     0,    33,    34,    35,    36,     0,
      84,    85,   918,    86,   179,    88,     0,   739,     0,     0,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,     0,     0,     0,    36,     0,   207,     0,     0,     0,
       0,     0,     0,   740,     0,    73,    74,    75,    76,    77,
       0,     0,     0,     0,     0,     0,   741,     0,     0,     0,
       0,   178,    80,    81,    82,   742,     0,    84,    85,     0,
      86,   179,    88,     0,     0,   208,    90,     0,     0,     0,
       0,     0,     0,     0,     0,   743,     0,     0,   919,     0,
      95,     0,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,   178,    36,   744,
      82,    83,     0,    84,    85,     0,    86,   179,    88,     0,
       0,    36,     0,   207,     0,     0,     0,     0,     0,     0,
     559,     0,     0,     0,     0,     0,     0,     0,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,   734,   735,   209,     0,     0,     0,   736,
     114,   737,   208,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   738,     0,     0,     0,     0,     0,     0,
       0,    33,    34,    35,    36,   343,     0,    84,    85,     0,
      86,   179,    88,   739,   178,     0,     0,    82,    83,     0,
      84,    85,     0,    86,   179,    88,     0,     0,     0,     0,
       0,     0,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   740,
       0,    73,    74,    75,    76,    77,     0,   560,     0,     0,
       0,     0,   741,     0,     0,     0,     0,   178,    80,    81,
      82,   742,     0,    84,    85,     0,    86,   179,    88,     0,
       0,     0,    90,     0,     0,     0,     0,     0,     0,     0,
       0,   743,   885,   886,     0,     0,    95,     0,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   887,     0,     0,   744,   356,   357,   358,     0,
     888,   889,   890,    36,     0,     0,     0,     0,     0,     0,
       0,     0,   891,     0,     0,   359,   859,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,     0,   381,
       0,     0,    29,    30,     0,     0,     0,     0,     0,     0,
       0,   382,    36,     0,    38,     0,     0,     0,   892,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   893,     0,     0,     0,     0,     0,     0,     0,     0,
      52,     0,    84,    85,     0,    86,   179,    88,    59,    60,
      61,   174,   175,   176,    29,    30,     0,     0,     0,     0,
     894,     0,     0,     0,    36,     0,   207,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,     0,     0,   178,     0,     0,    82,    83,
       0,    84,    85,     0,    86,   179,    88,     0,     0,     0,
       0,     0,     0,    91,    36,   208,   806,   807,     0,     0,
       0,     0,     0,     0,     0,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,     0,   432,     0,     0,     0,   178,   114,     0,
      82,    83,   874,    84,    85,     0,    86,   179,    88,     0,
       0,    36,     0,   207,     0,    91,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,    84,    85,   432,    86,   179,    88,     0,
     114,     0,   208,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,   207,     0,     0,     0,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,   178,     0,     0,    82,    83,     0,
      84,    85,     0,    86,   179,    88,     0,     0,     0,     0,
       0,     0,     0,   208,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,     0,   209,     0,    36,   178,     0,   114,    82,    83,
       0,    84,    85,   727,    86,   179,    88,     0,    36,     0,
     207,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   978,   979,   980,   981,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     982,     0,     0,   209,     0,     0,   517,     0,   114,   208,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   537,     0,     0,     0,     0,   178,     0,     0,
      82,     0,     0,    84,    85,     0,    86,   179,    88,     0,
       0,   178,     0,     0,    82,    83,     0,    84,    85,     0,
      86,   179,    88,     0,    36,     0,   207,     0,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,     0,   209,
       0,     0,     0,     0,   114,   208,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    36,     0,   207,  1018,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   178,   381,     0,
      82,    83,     0,    84,    85,     0,    86,   179,    88,     0,
     382,     0,     0,     0,     0,     0,   208,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,   207,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,     0,   209,     0,     0,   178,     0,
     114,    82,    83,     0,    84,    85,     0,    86,   179,    88,
       0,     0,     0,     0,     0,     0,     0,   221,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,     0,   209,     0,     0,   178,
       0,   114,    82,    83,     0,    84,    85,     0,    86,   179,
      88,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   356,   357,   358,   222,     0,     0,
       0,     0,   114,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   359,     0,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,     0,   381,   356,   357,
     358,     0,     0,     0,     0,     0,     0,     0,     0,   382,
       0,     0,     0,     0,     0,     0,     0,   359,     0,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
       0,   381,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   382,   356,   357,   358,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   359,   428,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,     0,   381,   356,   357,
     358,     0,     0,     0,     0,     0,     0,     0,     0,   382,
       0,     0,     0,     0,     0,     0,     0,   359,   438,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
       0,   381,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   382,   356,   357,   358,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   359,   862,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,     0,   381,   957,   958,
     959,     0,     0,     0,     0,     0,     0,     0,     0,   382,
       0,     0,     0,     0,     0,     0,     0,   960,   904,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,   980,   981,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   982,   957,   958,   959,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   960,  1217,   961,   962,   963,   964,   965,
     966,   967,   968,   969,   970,   971,   972,   973,   974,   975,
     976,   977,   978,   979,   980,   981,     0,     0,   957,   958,
     959,     0,     0,     0,     0,     0,     0,     0,     0,   982,
       0,     0,     0,     0,     0,     0,     0,   960,  1124,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,   980,   981,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    36,     0,   982,   957,   958,   959,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      36,     0,     0,   960,  1271,   961,   962,   963,   964,   965,
     966,   967,   968,   969,   970,   971,   972,   973,   974,   975,
     976,   977,   978,   979,   980,   981,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   982,
       0,     0,  1369,    36,     0,     0,     0,     0,  1351,     0,
       0,     0,     0,     0,   178,  1370,  1371,    82,    83,    36,
      84,    85,     0,    86,   179,    88,     0,     0,     0,     0,
       0,     0,     0,   178,     0,     0,    82,  1372,     0,    84,
      85,     0,    86,  1373,    88,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,    36,     0,     0,  1434,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,   505,
       0,     0,    84,    85,     0,    86,   179,    88,     0,     0,
       0,     0,     0,     0,     0,   509,     0,     0,    84,    85,
      36,    86,   179,    88,     0,     0,     0,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   276,     0,    36,
      84,    85,     0,    86,   179,    88,     0,   640,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,    84,
      85,     0,    86,   179,    88,     0,  1134,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,    84,    85,
       0,    86,   179,    88,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   356,
     357,   358,     0,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   718,   359,     0,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,     0,   381,     0,   356,   357,   358,     0,     0,     0,
       0,     0,     0,     0,   382,     0,     0,     0,     0,     0,
       0,     0,     0,   359,     0,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   719,   381,   957,   958,
     959,     0,     0,     0,     0,     0,     0,     0,     0,   382,
       0,     0,     0,     0,     0,     0,     0,   960,  1276,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,   980,   981,
     957,   958,   959,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   982,     0,     0,     0,     0,     0,   960,
       0,   961,   962,   963,   964,   965,   966,   967,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   977,   978,   979,
     980,   981,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   982,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,     0,   381,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   382,   964,
     965,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   978,   979,   980,   981,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     982,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,     0,   381,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   382,   965,   966,   967,   968,   969,   970,   971,   972,
     973,   974,   975,   976,   977,   978,   979,   980,   981,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   982,  -917,  -917,  -917,  -917,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,     0,
     381,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   382,  -917,  -917,  -917,  -917,   970,   971,   972,
     973,   974,   975,   976,   977,   978,   979,   980,   981,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   982
};

static const yytype_int16 yycheck[] =
{
       5,     6,   158,     8,     9,    10,    11,    12,   133,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    53,     4,    28,    29,    89,   599,   181,     4,    93,
      94,     4,   314,     4,   170,     4,    32,    42,   686,   571,
      54,   452,  1039,   836,    49,   161,    51,  1035,    44,    54,
     570,    56,    48,   395,   381,   825,   158,   182,   448,   422,
    1025,   552,   448,     4,   855,   111,     9,   717,   132,   229,
       9,     4,     9,   111,     9,    14,    30,  1027,   416,   417,
     871,   724,     9,     9,   219,    30,   111,    57,     9,    45,
     501,   223,     9,     9,     9,    79,     4,     9,     9,     9,
       9,    66,     9,    66,     9,    45,   111,    30,   446,    79,
       9,     9,    82,    66,   230,    66,    50,     9,    66,    10,
      11,    12,    79,     9,   128,   129,    33,     9,     9,   250,
     251,     9,    79,     9,   155,     9,   257,    45,    29,   299,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,   572,    53,   209,     9,    45,    85,    85,   128,   129,
     153,  1564,     0,   109,    65,    45,   158,    45,   314,    35,
     201,   186,   198,   953,   209,   198,     9,   198,    35,   146,
     147,   148,   133,     9,   348,   199,     8,   222,   106,   917,
      66,   149,    66,   111,    66,   113,   114,   115,   116,   117,
     118,   119,    66,    66,    66,   336,   163,   200,   201,   203,
     154,   198,    79,    79,  1212,  1618,   231,   146,   146,   234,
      66,   167,    79,   198,   199,   198,   241,   242,   201,   198,
     196,   182,   202,   199,   171,   198,   292,   201,   156,   157,
     201,   159,   388,   201,   292,   171,   196,  1222,   154,   202,
      79,   200,   235,   200,  1214,   200,   239,   292,   198,   394,
     424,  1221,   180,  1223,   200,  1066,   412,   200,   342,   200,
     201,    66,    30,   200,   927,   200,   929,   292,   200,   200,
     200,   200,   199,   298,   202,   200,   431,   302,   709,    66,
     436,   200,   200,   714,   274,   275,   276,   339,   200,   445,
      66,   201,   448,   199,   205,  1085,   163,   199,   199,   199,
     852,   199,   327,   199,   815,   199,   171,   391,   392,   199,
    1318,   199,  1320,   338,  1299,   201,   306,   201,    66,   201,
      66,    79,    66,   198,   326,   198,   203,   201,   201,   201,
     471,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   199,   382,  1328,   384,
     385,   395,   387,   199,   203,   326,   432,   320,    35,  1107,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,  1368,   128,   129,    79,   432,   413,   414,
      66,   416,   417,   418,   419,    79,   201,   577,   128,   129,
     425,    35,    35,   428,    79,    79,  1414,   562,    79,    50,
      85,   587,    79,   438,   201,   440,   381,    79,  1086,   802,
     454,   446,   201,   388,    29,   201,   567,   568,   198,   454,
     840,   456,   393,  1103,   840,   576,   170,   195,    97,   100,
     101,    46,    35,   201,    49,    79,    79,   412,   198,   100,
     101,    97,   153,   201,    66,   201,     4,   201,   483,   116,
       4,   486,   487,   488,   198,   587,   459,   124,    79,   199,
     163,   436,   147,   148,    85,   649,   147,   148,  1268,   163,
     445,   149,     4,   448,   639,    66,    79,   149,   170,   163,
     642,   198,   517,   924,   153,   905,   163,    45,  1038,   905,
    1052,   198,   170,  1055,    26,    27,    50,   153,    79,    53,
     868,   691,    79,   154,    85,   505,   198,   697,    85,   509,
     198,   605,   674,   825,   514,   201,    70,   874,   169,   163,
     163,    97,   207,   201,    97,   198,   147,   148,    79,    35,
     692,   202,    97,    87,    85,    89,  1336,   128,   129,    93,
      94,   202,   113,   114,   115,   116,   117,   118,   106,  1567,
    1228,   697,  1230,   111,    97,   113,   114,   115,   116,   117,
     118,   119,   597,   198,   170,   730,   147,   148,   200,   146,
     147,   148,    46,    47,   609,   587,   170,   153,   132,   111,
     153,   732,   733,   745,    79,   206,   149,   200,   153,   155,
      85,   753,   198,   149,    79,   146,   147,   148,   156,   157,
      85,   159,   168,   385,   198,   117,   641,   170,   571,   180,
     153,    53,   124,    29,   170,   650,   113,   114,   115,   116,
     117,   118,   180,    65,    99,   100,   101,    14,   200,   200,
      46,   413,   200,    49,   669,   201,   418,    74,    75,    76,
     194,   953,   198,    30,   202,   201,   128,   129,  1020,    86,
     200,  1201,   147,   148,  1216,   200,  1334,    71,    72,   825,
      47,    66,   147,   148,    66,   816,   701,    46,    47,    48,
      49,    50,    51,  1700,   840,   200,  1694,   209,   126,   127,
     831,   235,   717,   180,   216,   239,    65,   713,  1715,   243,
     222,  1709,   843,  1061,   201,   707,   149,   134,   135,   136,
     137,   138,  1070,   235,    99,   100,   101,   239,   145,   198,
     264,    71,    72,  1106,   151,   152,    49,    50,    51,   722,
     755,   198,    46,    47,    48,    49,    50,    51,   165,    53,
     200,   201,    65,  1174,   899,    66,   707,   200,   201,   905,
     149,    65,   179,  1421,   113,   114,   115,  1593,  1594,   281,
     113,   114,   115,   116,   117,   118,   153,   769,   290,   291,
     292,   124,   125,  1589,  1590,   297,   198,  1329,   115,   116,
     117,   303,   326,  1085,   774,   200,   811,   942,   778,   873,
     931,   335,   933,    44,   949,   339,    65,   953,   342,   824,
    1183,    49,    50,    51,   326,    53,   170,   160,   769,   162,
    1241,   149,   814,     9,   198,   732,   733,    65,   814,   205,
     149,   814,   149,   814,   849,   814,  1257,   180,   198,     8,
     170,   149,   834,   200,   859,   198,   149,   862,    14,   864,
     149,    79,   200,   868,   388,   389,   390,   391,   392,   200,
     124,   124,    14,   814,   199,   170,  1669,    14,    97,   198,
    1218,   814,   204,   199,   199,   199,   198,   198,   412,   105,
      26,    27,     9,   834,  1687,   840,   105,   199,    89,   904,
     199,     9,  1695,   836,   113,   114,   115,   116,   117,   118,
     200,    14,   436,  1045,   910,   198,  1037,     4,  1039,   852,
     422,  1569,   184,     9,   448,    79,    79,  1338,    79,   874,
     432,   913,   187,   915,   198,   459,  1347,   300,   911,   200,
       9,   304,   200,  1064,     9,  1080,    79,  1358,   199,  1085,
     200,   199,   476,   199,   126,   198,    66,   459,    45,   199,
     905,   127,    30,   169,     9,   328,   130,   330,   331,   332,
     333,   180,   913,   199,   915,   149,    14,     9,  1316,   196,
       9,   986,   987,   988,    66,   171,  1268,   992,   993,   199,
       9,    14,   516,   205,   126,   202,  1131,   181,   182,   183,
    1121,   205,     9,  1138,   188,   189,  1020,   205,   192,   193,
     198,   205,   199,    97,   199,  1020,  1427,   541,   542,   106,
     198,   200,   199,   130,   111,   200,   113,   114,   115,   116,
     117,   118,   119,   113,   114,   115,   116,   117,   118,  1612,
     149,     9,  1024,  1048,   124,   125,   201,   199,  1024,   198,
     149,  1024,   198,  1024,  1336,  1024,  1061,   149,  1179,   184,
     184,    14,     9,    79,    14,  1070,  1071,   201,    14,   156,
     157,   200,   159,   209,   201,   205,    14,   196,   201,   200,
     216,   605,   162,  1024,   198,    30,   222,   198,    30,    14,
      48,  1024,   198,   180,   596,     9,   198,   198,  1103,  1095,
     180,   199,  1237,   200,   200,   198,   130,    14,  1113,     9,
     199,    65,  1685,     9,   205,   202,     4,    79,   146,  1052,
      97,   198,  1055,     9,   130,  1098,   200,    14,  1110,   199,
      79,   130,  1268,   199,   201,   198,   198,   201,     9,  1260,
     201,  1262,    86,   146,  1300,   281,   205,    30,  1559,   651,
    1561,    73,   200,   199,   290,   291,   292,    45,   200,  1570,
     171,   297,   686,   130,   688,    30,   199,   303,   199,  1110,
    1295,   130,     9,   199,  1295,   202,     9,   199,    14,   199,
      79,   202,   130,   707,   201,   198,   201,   199,  1193,   199,
     198,   693,  1197,   695,  1199,   199,     9,   721,   722,  1181,
    1336,   199,  1207,   199,  1615,   707,    30,   200,  1190,   199,
     199,   106,  1217,  1218,   200,   200,    97,   719,   106,   201,
     722,   158,   200,   111,   154,   113,   114,   115,   116,   117,
     118,   119,    14,  1387,    79,   111,   199,   201,   199,  1360,
     130,   130,   199,    14,   170,   769,   201,    79,  1181,   200,
      14,   775,  1284,    14,    79,   779,   780,  1190,   149,   199,
     201,   130,   198,    14,   199,     4,   200,   769,   156,   157,
      14,   159,  1277,   200,    14,   799,   200,    55,   201,    79,
     198,    79,     9,  1216,   200,   787,   422,    79,   109,    97,
     814,   149,   180,    97,   161,    33,   432,  1279,    14,  1363,
     802,   803,  1713,   200,   828,   198,    45,   199,  1313,  1720,
     834,  1316,   814,   198,   202,   167,   840,   171,  1300,    79,
     164,   199,     9,    79,   201,   200,  1447,   199,    14,   199,
      79,    14,   834,    79,    14,  1367,    79,  1319,  1279,    14,
     774,   514,    79,  1325,  1677,  1327,   778,    26,    27,   873,
     389,    30,   392,  1294,   391,   872,   817,  1339,  1390,   869,
    1392,   885,   886,   887,  1104,  1691,  1348,   106,  1426,  1254,
    1687,  1344,   111,  1417,   113,   114,   115,   116,   117,   118,
     119,   905,  1449,  1533,   519,  1719,  1319,   911,  1290,   913,
    1545,   915,  1325,  1707,  1327,  1549,  1329,  1286,  1339,    42,
    1405,  1413,   995,   395,   992,   490,   766,  1348,  1028,   911,
     934,   913,   490,   915,   886,   917,   918,   156,   157,  1068,
     159,   950,  1081,   901,   840,  1082,   732,  1548,   952,   291,
    1435,   955,   298,  1011,   935,   983,    -1,  1419,    -1,    -1,
      -1,   180,  1424,    -1,    -1,    -1,    -1,    -1,  1430,    -1,
      -1,    -1,  1425,  1426,  1436,  1396,    -1,    -1,    -1,   983,
     596,    -1,    -1,   202,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1000,    -1,    -1,  1003,
    1544,    -1,    -1,  1424,    -1,    -1,  1419,    -1,    -1,  1430,
      -1,    -1,    -1,    -1,    -1,  1436,    -1,    -1,    -1,    -1,
    1024,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1664,  1630,
      -1,    -1,    -1,    -1,    -1,   651,    -1,    -1,    -1,    -1,
      -1,    -1,  1024,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     209,    -1,    -1,    -1,    26,    27,    -1,   216,    30,    -1,
      -1,    -1,  1606,   222,    -1,  1047,     4,    -1,    -1,  1554,
      -1,    -1,    -1,    -1,    -1,    -1,  1681,   693,  1082,   695,
      -1,    53,  1086,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1098,    -1,    -1,    -1,    -1,  1700,
      -1,    -1,    -1,   719,    -1,    -1,  1110,    45,  1090,    -1,
      -1,    -1,  1574,    -1,  1715,    -1,  1098,    -1,    -1,    -1,
      -1,    -1,   281,    -1,  1106,  1107,    -1,    -1,  1110,    -1,
      -1,   290,   291,    -1,    -1,    -1,    -1,    -1,   297,    -1,
      -1,    -1,    -1,    -1,   303,    -1,    -1,    -1,    -1,    -1,
      -1,  1613,  1614,  1574,    -1,   314,    -1,    -1,  1620,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,
      -1,   787,    -1,   111,  1178,   113,   114,   115,   116,   117,
     118,   119,    -1,    -1,    -1,    -1,   802,   803,    -1,    -1,
      -1,    -1,  1613,  1614,  1656,    -1,    -1,    -1,    -1,  1620,
      -1,  1183,  1664,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,   156,   157,
      -1,   159,   381,    -1,  1228,    -1,  1230,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,  1656,    -1,   209,    -1,    -1,
    1725,    -1,   180,    -1,   216,    -1,    -1,    -1,  1733,    -1,
     222,    -1,    -1,    -1,  1739,    -1,  1669,  1742,    -1,  1721,
      -1,    -1,    -1,   422,   202,    -1,  1728,    -1,    -1,    -1,
      -1,    -1,    -1,   432,  1687,  1279,    -1,    -1,   250,   251,
    1284,    -1,  1695,    -1,    -1,   257,  1290,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1279,    -1,    -1,
    1721,   917,   918,    -1,    -1,    -1,    -1,  1728,    -1,   281,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   290,   291,
      -1,    -1,    -1,    -1,    -1,   297,    -1,    -1,    -1,    -1,
    1334,   303,    -1,    -1,    -1,  1339,    -1,    -1,    -1,    -1,
    1344,    -1,   314,    -1,  1348,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,  1339,    79,  1363,
      -1,    -1,  1344,  1367,   336,    -1,  1348,   339,    -1,    -1,
      -1,    -1,  1376,    -1,    -1,    -1,    -1,    -1,  1382,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1390,    -1,  1392,    -1,
      -1,    -1,    -1,    -1,  1398,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,   381,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1421,    -1,    -1,
    1424,  1425,  1426,    -1,    -1,    -1,  1430,    -1,    -1,    -1,
      -1,  1047,  1436,    -1,    -1,   156,   157,   596,   159,   160,
     161,    -1,  1424,  1425,  1426,    -1,    -1,    -1,  1430,    -1,
     422,    -1,    -1,    -1,  1436,    -1,    -1,    -1,    -1,    -1,
     432,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,  1090,    -1,    -1,    -1,    -1,    -1,
     201,    -1,   203,    10,    11,    12,    -1,    -1,    -1,    -1,
    1106,  1107,   651,    -1,    -1,    -1,    -1,    -1,    -1,   471,
     472,    -1,    29,   475,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   693,    -1,   695,    -1,    65,    -1,
    1544,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   523,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     719,  1565,    -1,    -1,    -1,  1569,    -1,  1183,    -1,    -1,
    1574,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1583,
      -1,    -1,    -1,    -1,    -1,  1589,  1590,    -1,    -1,  1593,
    1594,    -1,  1574,    -1,    -1,   567,   568,    -1,    -1,    -1,
      -1,    -1,  1606,    -1,   576,    -1,    -1,    -1,    -1,  1613,
    1614,    -1,    -1,    -1,    -1,    -1,  1620,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   596,    -1,    -1,    -1,   787,    -1,
      -1,  1613,  1614,    -1,    -1,    -1,    -1,    -1,  1620,    -1,
      -1,    -1,    -1,   802,   803,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1656,    -1,    -1,    -1,    -1,    -1,    -1,  1663,
      -1,    -1,    -1,    10,    11,    12,   825,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1656,   202,  1680,    -1,    -1,   651,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   874,    -1,  1721,    65,    -1,
      -1,   693,    -1,   695,  1728,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,  1721,
      -1,    -1,    -1,    -1,    -1,    -1,  1728,   719,   720,    -1,
      65,    -1,    10,    11,    12,    -1,    -1,    -1,   917,   918,
     732,   733,   734,   735,   736,   737,   738,    -1,    -1,    -1,
      -1,    29,   744,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,   953,    53,    -1,    -1,   770,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   787,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   800,    -1,
     802,   803,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   816,   817,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   825,    -1,   202,    -1,    -1,    -1,   831,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   843,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   851,
      -1,    -1,   854,    -1,    -1,    -1,    -1,    -1,  1047,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    26,    27,    -1,    -1,
      30,    -1,   874,    -1,    -1,    -1,    -1,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,    53,    -1,    -1,  1085,    -1,    -1,    -1,
      -1,  1090,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   202,   917,   918,  1106,  1107,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    64,   931,
      -1,   933,    -1,   935,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,   950,    -1,
      -1,   953,    -1,    -1,   956,   957,   958,   959,   960,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,   980,   981,
     982,    -1,    -1,    -1,    63,    64,    -1,    -1,    10,    11,
      12,    -1,   128,   129,  1183,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1008,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,  1037,    -1,  1039,    -1,    -1,
      -1,    -1,    -1,    65,    -1,  1047,    -1,    -1,    -1,   128,
     129,    -1,    -1,    -1,    -1,    -1,   216,    -1,    -1,    -1,
      -1,    -1,  1064,   199,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1268,
      -1,    -1,    -1,  1085,    -1,    -1,    -1,    -1,  1090,    -1,
     250,   251,    -1,    -1,    -1,    -1,    -1,   257,    -1,    -1,
      -1,    -1,    -1,    -1,  1106,  1107,    -1,  1109,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1121,
     199,   281,  1124,    -1,  1126,    -1,    -1,    -1,    -1,    77,
     290,   291,    -1,    -1,    -1,    -1,    -1,   297,    -1,    -1,
    1142,    -1,    -1,   303,    -1,    -1,    -1,  1336,    -1,    -1,
      -1,    -1,    -1,    -1,   314,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   336,  1179,  1180,   339,
     202,  1183,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,   153,    53,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,    -1,    65,    -1,
      -1,   381,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   475,    -1,    -1,
      -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,  1260,    -1,
    1262,    -1,   422,    -1,    -1,  1267,  1268,    -1,    -1,  1271,
      -1,  1273,    -1,    -1,  1276,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1284,  1285,    -1,    -1,  1288,    -1,    -1,    -1,
      -1,    -1,    -1,  1295,    -1,   523,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   471,    -1,    -1,    -1,   475,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1336,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,  1351,
      -1,    -1,    -1,    -1,    -1,   202,    -1,    -1,  1360,  1361,
    1362,    -1,    -1,   523,    29,  1367,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,  1390,    -1,
    1392,    -1,    -1,    -1,    -1,    -1,  1398,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,   567,   568,    -1,
      -1,    -1,    -1,    -1,    -1,    77,   576,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,  1433,  1434,    -1,    -1,    -1,   596,    -1,    -1,  1441,
    1442,    -1,    -1,    -1,    -1,  1447,    29,  1449,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   651,   720,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    -1,    -1,    -1,    -1,   734,   735,   736,   737,
     738,    -1,    -1,    -1,    -1,    -1,   744,    -1,    -1,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   693,   199,   695,    -1,    -1,    -1,   201,
      -1,   203,    -1,    -1,    -1,    -1,  1548,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   719,
     720,    -1,    -1,  1565,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   732,   733,   734,   735,   736,   737,   738,    -1,
      -1,  1583,    -1,    -1,   744,    -1,  1588,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1599,    -1,    -1,
      -1,    -1,  1604,    -1,    -1,    -1,  1608,    -1,    -1,    -1,
     770,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   202,
      -1,    -1,    -1,   851,    -1,    -1,    -1,   787,  1630,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     800,    -1,   802,   803,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   816,   817,    -1,    53,
     475,    -1,    -1,    -1,    -1,   825,  1668,    -1,    -1,    -1,
      -1,   831,    -1,    -1,  1676,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   843,    -1,    -1,    -1,    -1,    -1,  1691,
      -1,   851,    -1,    -1,   854,    -1,    -1,    -1,  1700,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   523,    -1,
      -1,    -1,    -1,  1715,   874,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   950,    -1,    -1,    -1,    -1,    -1,   956,   957,
     958,   959,   960,   961,   962,   963,   964,   965,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     978,   979,   980,   981,   982,    -1,    -1,   917,   918,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   931,    -1,   933,    -1,    -1,    -1,    -1,    -1,    -1,
    1008,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     950,    -1,    -1,   953,    -1,    -1,   956,   957,   958,   959,
     960,   961,   962,   963,   964,   965,   966,   967,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   977,   978,   979,
     980,   981,   982,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,  1008,    29,
      -1,    -1,    -1,    -1,    -1,    -1,   250,   251,    -1,    65,
      -1,    -1,    -1,   257,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    55,    -1,  1037,    -1,  1039,
      -1,  1109,    -1,    -1,    -1,    -1,    -1,  1047,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1124,    77,  1126,    -1,
      -1,    -1,    -1,    -1,  1064,   720,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1142,    -1,    -1,    -1,    -1,   734,
     735,   736,   737,    -1,    -1,  1085,    -1,    -1,    -1,   744,
    1090,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,    -1,
      -1,    -1,   336,    -1,    -1,   339,  1106,  1107,    -1,  1109,
      -1,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1121,    -1,    -1,  1124,    -1,  1126,    -1,    -1,    -1,
     150,    -1,    -1,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,  1142,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    -1,    -1,    -1,   198,  1179,
    1180,    -1,   202,  1183,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,   851,    -1,    -1,  1267,
      -1,    -1,    -1,  1271,    -1,  1273,    -1,    -1,  1276,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,   471,   472,    -1,
      -1,   475,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1260,    -1,  1262,    -1,    -1,    -1,    -1,  1267,  1268,    -1,
      -1,  1271,    -1,  1273,    -1,    -1,  1276,    -1,    -1,    -1,
      -1,    -1,    -1,  1351,  1284,  1285,    -1,    -1,  1288,   523,
      -1,    -1,    -1,    -1,    -1,  1295,    -1,    -1,    -1,    -1,
      -1,   956,   957,   958,   959,   960,   961,   962,   963,   964,
     965,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   978,   979,   980,   981,   982,    -1,    -1,
      -1,    -1,    -1,   567,   568,    -1,  1336,    -1,    -1,    -1,
      -1,    -1,   576,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1351,    -1,  1008,    -1,    -1,    -1,    -1,    -1,    -1,
    1360,  1361,  1362,    -1,    -1,  1433,  1434,  1367,    -1,    -1,
      -1,    -1,    -1,  1441,    -1,    -1,    -1,    -1,    -1,    -1,
    1448,    -1,   202,    10,    11,    12,    -1,    -1,    -1,    -1,
    1390,    -1,  1392,    -1,    -1,    -1,    -1,    -1,  1398,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,  1433,  1434,    -1,    -1,    -1,    65,    -1,
      -1,  1441,  1442,    -1,    77,    -1,    79,  1447,    -1,  1449,
      -1,    -1,    -1,    -1,  1109,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1124,
      -1,  1126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   720,  1142,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   732,   733,
     734,   735,   736,   737,   738,    -1,    -1,    -1,    -1,    -1,
     744,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1588,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,  1599,    -1,    -1,    -1,    -1,  1604,    -1,    -1,    -1,
    1608,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1548,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,    -1,  1632,  1565,    -1,    -1,   201,    -1,
     203,    -1,    -1,    -1,    -1,   202,    -1,    -1,    -1,    -1,
      -1,   475,   816,    -1,    -1,    -1,    -1,    -1,  1588,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   831,    -1,  1599,
    1668,    -1,    -1,    -1,  1604,    -1,    -1,    -1,  1608,   843,
      -1,    -1,  1267,    -1,    -1,    -1,  1271,   851,  1273,    -1,
      -1,  1276,    10,    11,    12,    -1,    -1,    -1,    -1,   523,
    1630,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,  1668,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1676,    65,    -1,    -1,
      -1,    -1,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,  1691,    -1,    -1,    -1,    -1,  1351,   931,    -1,   933,
    1700,   935,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,  1715,   950,    -1,    -1,    -1,
      -1,    -1,   956,   957,   958,   959,   960,   961,   962,   963,
     964,   965,   966,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,   978,   979,   980,   981,   982,    -1,
      -1,    63,    64,    -1,    -1,    -1,    -1,    -1,    -1,   154,
      -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1008,    -1,    -1,    -1,  1433,  1434,
      -1,    -1,    -1,    -1,    -1,    -1,  1441,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      -1,    -1,    -1,  1037,    -1,  1039,   201,    -1,   203,    -1,
      -1,    -1,    -1,    -1,   202,    77,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   720,    -1,    -1,    -1,
    1064,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
     734,   735,   736,   737,   738,    -1,    -1,    -1,    -1,    -1,
     744,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,  1109,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1121,    65,    -1,
    1124,    -1,  1126,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1142,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,  1588,    -1,    -1,   198,    -1,    -1,    10,
      11,    12,    -1,    -1,  1599,  1179,    -1,    -1,    -1,  1604,
      -1,    -1,    -1,  1608,    -1,    -1,    -1,   851,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,  1668,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   202,  1260,    -1,  1262,    -1,
      -1,    -1,    -1,  1267,    -1,    -1,    -1,  1271,    -1,  1273,
      -1,    -1,  1276,    -1,    -1,    -1,    -1,    63,    64,    -1,
    1284,    -1,    -1,    -1,    -1,    -1,   950,    -1,    -1,    -1,
      -1,  1295,   956,   957,   958,   959,   960,   961,   962,   963,
     964,   965,   966,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,   978,   979,   980,   981,   982,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    74,    75,
      76,    77,   128,   129,  1008,    -1,    -1,  1351,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,  1360,    -1,    -1,   200,
      -1,    -1,    -1,  1367,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1390,    -1,  1392,    -1,
      -1,    -1,    -1,    -1,  1398,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
     156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,  1433,
    1434,    65,    -1,    -1,    -1,    -1,    -1,  1441,    -1,    -1,
      77,    -1,    -1,  1447,    -1,  1109,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,    -1,
    1124,    -1,  1126,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1142,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    29,   150,    -1,    -1,   153,    -1,    65,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    -1,    -1,    -1,  1548,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,    -1,
      77,  1565,    -1,    -1,    -1,   202,   200,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1583,
      -1,    -1,    -1,    -1,  1588,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1599,    -1,    -1,    -1,    -1,
    1604,   118,    -1,  1267,  1608,    -1,    -1,  1271,    -1,  1273,
      -1,    -1,  1276,    -1,    -1,   132,   133,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1630,    -1,    -1,    -1,
      -1,    -1,    -1,   150,    -1,    -1,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   200,    -1,    -1,    -1,   174,    -1,    -1,
      -1,    -1,    -1,    -1,  1668,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,    -1,
      -1,   198,    -1,    -1,    -1,   202,    -1,  1351,     3,     4,
       5,     6,     7,    -1,    -1,    -1,  1700,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1715,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    82,    -1,  1433,
    1434,    86,    87,    88,    89,    -1,    91,  1441,    93,    -1,
      95,    -1,    -1,    98,    -1,    -1,    -1,   102,   103,   104,
     105,   106,   107,   108,    -1,   110,   111,   112,   113,   114,
     115,   116,   117,   118,    -1,   120,   121,   122,   123,   124,
     125,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,
     135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,
     145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,   162,    -1,    -1,
     165,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,   174,
     175,    -1,   177,    -1,   179,   180,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,   198,    -1,   200,   201,   202,   203,   204,
      -1,   206,   207,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1588,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,  1599,    -1,    -1,    -1,    -1,
    1604,    -1,    -1,    -1,  1608,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    -1,    -1,  1668,   102,   103,   104,   105,   106,
     107,   108,    -1,   110,   111,   112,   113,   114,   115,   116,
     117,   118,    -1,   120,   121,   122,   123,   124,   125,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,    -1,
     177,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    -1,   200,   201,   202,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,   174,   175,    -1,   177,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,   200,   201,
      -1,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,
     107,   108,    -1,   110,    -1,   112,   113,   114,   115,   116,
     117,   118,    -1,   120,   121,   122,    -1,   124,   125,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,
      -1,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    -1,   200,   201,   202,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,   200,   201,
     202,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,
     107,   108,    -1,   110,    -1,   112,   113,   114,   115,   116,
     117,   118,    -1,   120,   121,   122,    -1,   124,   125,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,
      -1,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    -1,   200,   201,   202,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,   200,   201,
     202,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    90,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,
     107,   108,    -1,   110,    -1,   112,   113,   114,   115,   116,
     117,   118,    -1,   120,   121,   122,    -1,   124,   125,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,
      -1,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    -1,   200,   201,    -1,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    95,    96,    -1,    98,    -1,    -1,    -1,
     102,   103,   104,   105,    -1,   107,   108,    -1,   110,    -1,
     112,   113,   114,   115,   116,   117,   118,    -1,   120,   121,
     122,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,
     132,    -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,
      -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
     162,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,
      -1,    -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,   200,   201,
      -1,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,
     107,   108,    -1,   110,    -1,   112,   113,   114,   115,   116,
     117,   118,    -1,   120,   121,   122,    -1,   124,   125,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,
      -1,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    -1,   200,   201,   202,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,   200,   201,
     202,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    -1,    91,    -1,    93,    94,    95,    -1,
      -1,    98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,
     107,   108,    -1,   110,    -1,   112,   113,   114,   115,   116,
     117,   118,    -1,   120,   121,   122,    -1,   124,   125,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,
      -1,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    -1,   200,   201,    -1,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,   200,   201,
     202,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,
     107,   108,    -1,   110,    -1,   112,   113,   114,   115,   116,
     117,   118,    -1,   120,   121,   122,    -1,   124,   125,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,
      -1,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    -1,   200,   201,   202,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,
      92,    93,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,
     102,   103,   104,   105,    -1,   107,   108,    -1,   110,    -1,
     112,   113,   114,   115,   116,   117,   118,    -1,   120,   121,
     122,    -1,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,
     132,    -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,
      -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
     162,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,
      -1,    -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,   200,   201,
      -1,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,
     107,   108,    -1,   110,    -1,   112,   113,   114,   115,   116,
     117,   118,    -1,   120,   121,   122,    -1,   124,   125,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,
      -1,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    -1,   200,   201,   202,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,   200,   201,
     202,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,
     107,   108,    -1,   110,    -1,   112,   113,   114,   115,   116,
     117,   118,    -1,   120,   121,   122,    -1,   124,   125,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,   162,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,
      -1,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    -1,   200,   201,   202,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,   200,   201,
      -1,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,
     107,   108,    -1,   110,    -1,   112,   113,   114,   115,   116,
     117,   118,    -1,   120,   121,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,
      -1,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    -1,   200,   201,    -1,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,   200,   201,
      -1,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,
     107,   108,    -1,   110,    -1,   112,   113,   114,   115,   116,
     117,   118,    -1,   120,   121,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,
      -1,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    -1,   200,   201,    -1,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,   200,   201,
      -1,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,
      -1,    98,    -1,    -1,    -1,   102,   103,   104,   105,    -1,
     107,   108,    -1,   110,    -1,   112,   113,   114,   115,   116,
     117,   118,    -1,   120,   121,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,
      -1,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    -1,   200,   201,    -1,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
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
      -1,    -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,   200,   201,
      -1,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,    -1,    -1,   121,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,
      -1,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    -1,    -1,    -1,    -1,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,
      -1,    -1,   174,    -1,    -1,    -1,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,   200,    -1,
      -1,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    29,    13,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    65,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,    -1,    -1,   121,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,   163,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,
      -1,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    -1,    -1,    -1,    -1,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,
      -1,    -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    11,    12,   201,
      -1,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    29,    13,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    35,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    65,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,    -1,    -1,   121,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,   163,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,
      -1,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    -1,    -1,    -1,    -1,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,
      -1,    -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,    10,    11,
      12,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    65,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   102,    -1,    -1,   105,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,    -1,    -1,   121,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   190,   191,
      -1,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    11,    12,    -1,    -1,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      29,    13,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    35,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,
      -1,    -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,    10,    11,
      12,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    65,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,    -1,    -1,   121,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,   187,    -1,   174,    -1,    -1,
      -1,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    -1,   200,    -1,    12,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    65,    -1,
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
      -1,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,
      -1,    -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,   200,    -1,
      12,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    65,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,    -1,    -1,   121,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,
      -1,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,   199,    -1,    -1,    -1,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    30,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,
      -1,    -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,    -1,    -1,
      -1,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    35,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    65,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,    -1,    -1,   121,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,
      -1,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    -1,    -1,    -1,    -1,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    35,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,
      -1,    -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,    -1,    -1,
      -1,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    35,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    65,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,    -1,    -1,   121,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,
      -1,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    -1,    -1,    -1,    -1,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    35,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,
      -1,    -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,    10,    11,
      12,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    65,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,    -1,    -1,   121,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,
      -1,   168,    -1,    -1,   186,    -1,    -1,   174,    -1,    -1,
      -1,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    -1,    10,    11,    12,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    65,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   113,   114,   115,   116,   117,   118,    -1,    -1,   121,
     122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,
     132,    -1,   134,   135,   136,   137,   138,    -1,    77,    -1,
      -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    -1,   165,    -1,    -1,   168,    -1,   185,    -1,
      -1,    -1,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,    -1,    -1,
      -1,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    10,    11,    12,    13,   156,   157,    -1,
     159,   160,   161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,    53,    -1,    -1,   198,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    68,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,    -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   150,   151,   152,    -1,    -1,    -1,   156,
     157,    -1,   159,   160,   161,   162,    -1,   164,   165,    -1,
     167,    -1,    -1,    -1,    -1,    -1,    -1,   174,   175,    -1,
     177,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    65,    77,    -1,   200,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,   200,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    29,   154,    -1,   156,   157,   158,   159,   160,
     161,    -1,    -1,    65,    -1,    -1,   200,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    -1,    -1,    -1,   198,    29,    -1,
      77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   199,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,   105,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,    -1,    -1,    -1,    29,    77,    -1,    -1,    -1,
      -1,    -1,    -1,   199,    -1,   132,   133,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    55,    -1,   150,   105,    -1,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,   118,    -1,    -1,
      -1,    -1,   194,    77,    -1,    -1,    -1,   174,    -1,    -1,
      -1,   132,   133,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,   150,
      -1,   198,   153,   154,    -1,   156,   157,    -1,   159,   160,
     161,    -1,    -1,    -1,   118,    -1,    -1,    -1,    29,    77,
      -1,    -1,    -1,   174,    -1,    -1,    -1,    -1,   132,   133,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    55,    -1,   150,   198,    -1,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,    -1,   163,
      -1,    -1,    -1,    -1,   122,    29,    77,    -1,    -1,    -1,
     174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,    55,    -1,    -1,   198,    -1,    -1,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,   118,    -1,    -1,
      -1,    29,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   132,   133,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,    55,    -1,   150,
     198,    -1,   153,   154,    -1,   156,   157,    -1,   159,   160,
     161,    -1,   163,    -1,   118,    -1,    -1,    -1,    -1,    77,
      -1,    -1,    -1,   174,    -1,    -1,    -1,    -1,   132,   133,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    -1,    -1,   150,   198,    -1,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,
     118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     174,   175,    -1,    -1,   132,   133,    77,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,    77,   150,    -1,   198,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,   105,   106,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,    30,    -1,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,    46,    47,    -1,
     198,    -1,    -1,    52,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,   153,    -1,    -1,   156,   157,    66,   159,   160,
     161,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    -1,
     156,   157,    35,   159,   160,   161,    -1,    86,    -1,    -1,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    -1,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,    -1,
      -1,    -1,    -1,    -1,    77,    -1,    79,    -1,    -1,    -1,
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
      79,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      65,    -1,    -1,   198,    -1,    -1,   201,    -1,   203,   118,
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
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    79,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,    -1,    -1,   198,    -1,    -1,   150,    -1,
     203,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    -1,    -1,    -1,   198,    -1,    -1,   150,
      -1,   203,   153,   154,    -1,   156,   157,    -1,   159,   160,
     161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    10,    11,    12,   198,    -1,    -1,
      -1,    -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   130,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   130,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   130,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   130,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   130,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   130,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    77,    -1,    65,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      77,    -1,    -1,    29,   130,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,   119,    77,    -1,    -1,    -1,    -1,   130,    -1,
      -1,    -1,    -1,    -1,   150,   132,   133,   153,   154,    77,
     156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   150,    -1,    -1,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,    -1,
      -1,    77,    -1,    -1,   130,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,   153,
      -1,    -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,   156,   157,
      77,   159,   160,   161,    -1,    -1,    -1,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,    -1,    -1,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   153,    -1,    77,
     156,   157,    -1,   159,   160,   161,    -1,   124,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   156,
     157,    -1,   159,   160,   161,    -1,   124,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,    28,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    97,    53,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65
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
      29,    55,   118,   132,   133,   154,   174,   198,   215,   226,
     397,   464,   476,   477,   479,   181,   200,   339,   342,   366,
     368,   201,   239,   342,   105,   106,   153,   216,   219,   222,
      79,   203,   291,   292,   117,   124,   116,   124,    79,   293,
     198,   198,   198,   198,   215,   263,   465,   198,   198,    79,
      85,   146,   147,   148,   456,   457,   153,   201,   222,   222,
     215,   264,   465,   154,   198,   465,   465,    79,   195,   201,
     351,   332,   342,   343,   435,   439,   228,   201,    85,   401,
     456,    85,   456,   456,    30,   153,   170,   466,   198,     9,
     200,    35,   245,   154,   262,   465,   118,   180,   246,   324,
     200,   200,   200,   200,   200,   200,    10,    11,    12,    29,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    53,    65,   200,    66,    66,   200,   201,   149,   125,
     160,   162,   265,   322,   323,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    63,    64,
     128,   129,   427,    66,   201,   432,   198,   198,    66,   201,
     203,   443,   198,   245,   246,    14,   342,   200,   130,    44,
     215,   422,   198,   332,   435,   439,   149,   435,   130,   205,
       9,   408,   332,   435,   466,   149,   198,   402,   427,   432,
     199,   342,    30,   230,     8,   354,     9,   200,   230,   231,
     334,   335,   342,   215,   277,   234,   200,   200,   200,   479,
     479,   170,   198,   105,   479,    14,   149,   215,    79,   200,
     200,   200,   181,   182,   183,   188,   189,   192,   193,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   381,   382,
     383,   240,   109,   167,   200,   153,   217,   220,   222,   153,
     218,   221,   222,   222,     9,   200,    97,   201,   435,     9,
     200,   124,   124,    14,     9,   200,   435,   460,   460,   332,
     343,   435,   438,   439,   199,   170,   257,   131,   435,   449,
     450,    66,   427,   146,   457,    78,   342,   435,    85,   146,
     457,   222,   214,   200,   201,   252,   260,   387,   389,    86,
     203,   355,   356,   358,   398,   443,   461,    14,    97,   462,
     350,   352,   353,   287,   288,   425,   426,   199,   199,   199,
     199,   202,   229,   230,   247,   254,   259,   425,   342,   204,
     206,   207,   215,   467,   468,   479,    35,   163,   289,   290,
     342,   464,   198,   465,   255,   245,   342,   342,   342,    30,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   342,   342,
     342,   342,   399,   342,   342,   445,   445,   342,   452,   453,
     124,   201,   215,   442,   443,   263,   215,   264,   262,   246,
      27,    35,   336,   339,   342,   366,   342,   342,   342,   342,
     342,   342,   342,   342,   342,   342,   342,   342,   154,   201,
     215,   428,   429,   430,   431,   442,   445,   342,   289,   289,
     445,   342,   449,   245,   199,   342,   198,   421,     9,   408,
     332,   199,   215,    35,   342,    35,   342,   199,   199,   442,
     289,   201,   215,   428,   429,   442,   199,   228,   281,   201,
     339,   342,   342,    89,    30,   230,   275,   200,    28,    97,
      14,     9,   199,    30,   201,   278,   479,    86,   226,   473,
     474,   475,   198,     9,    46,    47,    52,    54,    66,    86,
     132,   145,   154,   174,   198,   223,   224,   226,   363,   397,
     403,   404,   405,   215,   478,   184,    79,   342,    79,    79,
     342,   378,   379,   342,   342,   371,   381,   187,   384,   228,
     198,   238,   222,   200,     9,    97,   222,   200,     9,    97,
      97,   219,   215,   342,   292,   404,    79,     9,   199,   199,
     199,   199,   199,   199,   199,   200,    46,    47,   471,   472,
     126,   268,   198,     9,   199,   199,    79,    80,   215,   458,
     215,    66,   202,   202,   211,   213,    30,   127,   267,   169,
      50,   154,   169,   391,   130,     9,   408,   199,   149,   479,
     479,    14,   354,   287,   228,   196,     9,   409,   479,   480,
     427,   432,   202,     9,   408,   171,   435,   342,   199,     9,
     409,    14,   346,   248,   126,   266,   198,   465,   342,    30,
     205,   205,   130,   202,     9,   408,   342,   466,   198,   258,
     253,   261,   256,   245,    68,   435,   342,   466,   205,   202,
     199,   199,   205,   202,   199,    46,    47,    66,    74,    75,
      76,    86,   132,   145,   174,   215,   411,   413,   414,   417,
     420,   215,   435,   435,   130,   427,   432,   199,   342,   282,
      71,    72,   283,   228,   333,   228,   335,    97,    35,   131,
     272,   435,   404,   215,    30,   230,   276,   200,   279,   200,
     279,     9,   171,   130,   149,     9,   408,   199,   163,   467,
     468,   469,   467,   404,   404,   404,   404,   404,   407,   410,
     198,    85,   149,   198,   404,   149,   201,    10,    11,    12,
      29,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    65,   149,   466,   342,   184,   184,    14,   190,
     191,   380,     9,   194,   384,    79,   202,   397,   201,   242,
      97,   220,   215,    97,   221,   215,   215,   202,    14,   435,
     200,     9,   171,   215,   269,   397,   201,   449,   131,   435,
      14,   205,   342,   202,   211,   479,   269,   201,   390,    14,
     342,   355,   215,   200,   479,   196,   202,    30,   470,   426,
      35,    79,   163,   428,   429,   431,   479,    35,   163,   342,
     404,   287,   198,   397,   267,   347,   249,   342,   342,   342,
     202,   198,   289,   268,    30,   267,   266,   465,   399,   202,
     198,    14,    74,    75,    76,   215,   412,   412,   414,   415,
     416,    48,   198,    85,   146,   198,     9,   408,   199,   421,
      35,   342,   428,   429,   202,    71,    72,   284,   333,   230,
     202,   200,    90,   200,   272,   435,   198,   130,   271,    14,
     228,   279,    99,   100,   101,   279,   202,   479,   479,   215,
     473,     9,   199,   408,   130,   205,     9,   408,   407,   215,
     355,   357,   359,   199,   124,   215,   404,   454,   455,   404,
     404,   404,    30,   404,   404,   404,   404,   404,   404,   404,
     404,   404,   404,   404,   404,   404,   404,   404,   404,   404,
     404,   404,   404,   404,   404,   404,   478,   342,   342,   342,
     379,   342,   369,    79,   243,   215,   215,   404,   472,    97,
       9,   297,   199,   198,   336,   339,   342,   205,   202,   462,
     297,   155,   168,   201,   386,   393,   155,   201,   392,   130,
     200,   470,   479,   354,   480,    79,   163,    14,    79,   466,
     435,   342,   199,   287,   201,   287,   198,   130,   198,   289,
     199,   201,   479,   201,   267,   250,   402,   289,   130,   205,
       9,   408,   413,   415,   146,   355,   418,   419,   414,   435,
     333,    30,    73,   230,   200,   335,   271,   449,   272,   199,
     404,    96,    99,   200,   342,    30,   200,   280,   202,   171,
     130,   163,    30,   199,   404,   404,   199,   130,     9,   408,
     199,   130,   202,     9,   408,   404,    30,   185,   199,   228,
     215,   479,   397,     4,   106,   111,   117,   119,   156,   157,
     159,   202,   298,   321,   322,   323,   328,   329,   330,   331,
     425,   449,   202,   201,   202,    50,   342,   342,   342,   354,
      35,    79,   163,    14,    79,   342,   198,   470,   199,   297,
     199,   287,   342,   289,   199,   297,   462,   297,   201,   198,
     199,   414,   414,   199,   130,   199,     9,   408,    30,   228,
     200,   199,   199,   199,   235,   200,   200,   280,   228,   479,
     479,   130,   404,   355,   404,   404,   404,   342,   201,   202,
      97,   126,   127,   175,   464,   270,   397,   106,   331,   119,
     132,   133,   154,   160,   307,   308,   309,   397,   158,   313,
     314,   122,   198,   215,   315,   316,   299,   246,   118,   479,
       9,   200,     9,   200,   200,   462,   322,   199,   294,   154,
     388,   202,   202,    79,   163,    14,    79,   342,   289,   111,
     344,   470,   202,   470,   199,   199,   202,   201,   202,   297,
     287,   130,   414,   355,   228,   233,   236,    30,   230,   274,
     228,   199,   404,   130,   130,   186,   228,   479,   397,   397,
     465,    14,     9,   200,   201,   464,   462,   170,   201,     9,
     200,     3,     4,     5,     6,     7,    10,    11,    12,    13,
      27,    28,    53,    67,    68,    69,    70,    71,    72,    73,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   131,   132,   134,   135,   136,   137,   138,   150,   151,
     152,   162,   164,   165,   167,   174,   175,   177,   179,   180,
     215,   394,   395,     9,   200,   154,   158,   215,   316,   317,
     318,   200,    79,   327,   245,   300,   464,   464,    14,   246,
     202,   295,   296,   464,    14,    79,   342,   199,   198,   201,
     200,   201,   319,   344,   470,   294,   202,   199,   414,   130,
      30,   230,   273,   274,   228,   404,   404,   342,   202,   200,
     200,   404,   397,   303,   479,   310,   403,   308,    14,    30,
      47,   311,   314,     9,    33,   199,    29,    46,    49,    14,
       9,   200,   465,   327,    14,   479,   245,   200,    14,   342,
      35,    79,   385,   228,   228,   201,   319,   202,   470,   414,
     228,    94,   187,   241,   202,   215,   226,   304,   305,   306,
       9,   171,     9,   202,   404,   395,   395,    55,   312,   317,
     317,    29,    46,    49,   404,    79,   198,   200,   404,   465,
     404,    79,     9,   409,   202,   202,   228,   319,    92,   200,
      79,   109,   237,   149,    97,   479,   403,   161,    14,   301,
     198,    35,    79,   199,   202,   200,   198,   167,   244,   215,
     322,   323,   171,   404,   285,   286,   426,   302,    79,   397,
     242,   164,   215,   200,   199,     9,   409,   113,   114,   115,
     325,   326,   285,    79,   270,   200,   470,   426,   480,   199,
     199,   200,   200,   201,   320,   325,    35,    79,   163,   470,
     201,   228,   480,    79,   163,    14,    79,   320,   228,   202,
      35,    79,   163,    14,    79,   342,   202,    79,   163,    14,
      79,   342,    14,    79,   342,   342
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
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
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
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
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
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
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
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
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
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
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
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
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
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
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
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
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
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2915 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
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
#line 3016 "hphp.y"
    { (yyvsp[(1) - (1)]).setText("static"); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3019 "hphp.y"
    { Token t; t.reset();
                                          _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                          _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3022 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3029 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3032 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3035 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3036 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3039 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3042 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3045 "hphp.y"
    { only_in_hh_syntax(_p);
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                         _p->onTypeList((yyval), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3051 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3054 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3057 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3063 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3069 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3077 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3078 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13641 "hphp.tab.cpp"
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
#line 3081 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

