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
#define YYLAST   16680

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  208
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  274
/* YYNRULES -- Number of rules.  */
#define YYNRULES  933
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
    1102,  1104,  1107,  1109,  1111,  1116,  1123,  1125,  1127,  1132,
    1134,  1136,  1140,  1143,  1144,  1147,  1148,  1150,  1154,  1156,
    1158,  1160,  1162,  1166,  1171,  1176,  1181,  1183,  1185,  1188,
    1191,  1194,  1198,  1202,  1204,  1206,  1208,  1210,  1214,  1216,
    1220,  1222,  1224,  1226,  1227,  1229,  1232,  1234,  1236,  1238,
    1240,  1242,  1244,  1246,  1248,  1249,  1251,  1253,  1255,  1259,
    1265,  1267,  1271,  1277,  1282,  1286,  1290,  1294,  1299,  1303,
    1307,  1311,  1314,  1316,  1318,  1322,  1326,  1328,  1330,  1331,
    1333,  1336,  1341,  1345,  1352,  1355,  1359,  1366,  1368,  1370,
    1372,  1374,  1376,  1383,  1387,  1392,  1399,  1403,  1407,  1411,
    1415,  1419,  1423,  1427,  1431,  1435,  1439,  1443,  1447,  1450,
    1453,  1456,  1459,  1463,  1467,  1471,  1475,  1479,  1483,  1487,
    1491,  1495,  1499,  1503,  1507,  1511,  1515,  1519,  1523,  1527,
    1530,  1533,  1536,  1539,  1543,  1547,  1551,  1555,  1559,  1563,
    1567,  1571,  1575,  1579,  1585,  1590,  1592,  1595,  1598,  1601,
    1604,  1607,  1610,  1613,  1616,  1619,  1621,  1623,  1625,  1629,
    1632,  1634,  1640,  1641,  1642,  1654,  1655,  1668,  1669,  1673,
    1674,  1679,  1680,  1687,  1688,  1696,  1697,  1703,  1706,  1709,
    1714,  1716,  1718,  1724,  1728,  1734,  1738,  1741,  1742,  1745,
    1746,  1751,  1756,  1760,  1765,  1770,  1775,  1780,  1782,  1784,
    1786,  1788,  1792,  1795,  1799,  1804,  1807,  1811,  1813,  1816,
    1818,  1821,  1823,  1825,  1827,  1829,  1831,  1833,  1838,  1843,
    1846,  1855,  1866,  1869,  1871,  1875,  1877,  1880,  1882,  1884,
    1886,  1888,  1891,  1896,  1900,  1904,  1909,  1911,  1914,  1919,
    1922,  1929,  1930,  1932,  1937,  1938,  1941,  1942,  1944,  1946,
    1950,  1952,  1956,  1958,  1960,  1964,  1968,  1970,  1972,  1974,
    1976,  1978,  1980,  1982,  1984,  1986,  1988,  1990,  1992,  1994,
    1996,  1998,  2000,  2002,  2004,  2006,  2008,  2010,  2012,  2014,
    2016,  2018,  2020,  2022,  2024,  2026,  2028,  2030,  2032,  2034,
    2036,  2038,  2040,  2042,  2044,  2046,  2048,  2050,  2052,  2054,
    2056,  2058,  2060,  2062,  2064,  2066,  2068,  2070,  2072,  2074,
    2076,  2078,  2080,  2082,  2084,  2086,  2088,  2090,  2092,  2094,
    2096,  2098,  2100,  2102,  2104,  2106,  2108,  2110,  2112,  2114,
    2116,  2118,  2120,  2122,  2124,  2126,  2128,  2133,  2135,  2137,
    2139,  2141,  2143,  2145,  2147,  2149,  2152,  2154,  2155,  2156,
    2158,  2160,  2164,  2165,  2167,  2169,  2171,  2173,  2175,  2177,
    2179,  2181,  2183,  2185,  2187,  2189,  2191,  2195,  2198,  2200,
    2202,  2207,  2211,  2216,  2218,  2220,  2224,  2228,  2232,  2236,
    2240,  2244,  2248,  2252,  2256,  2260,  2264,  2268,  2272,  2276,
    2280,  2284,  2288,  2292,  2295,  2298,  2301,  2304,  2308,  2312,
    2316,  2320,  2324,  2328,  2332,  2336,  2342,  2347,  2351,  2355,
    2359,  2361,  2363,  2365,  2367,  2371,  2375,  2379,  2382,  2383,
    2385,  2386,  2388,  2389,  2395,  2399,  2403,  2405,  2407,  2409,
    2411,  2415,  2418,  2420,  2422,  2424,  2426,  2428,  2432,  2434,
    2436,  2438,  2441,  2444,  2449,  2453,  2458,  2461,  2462,  2468,
    2472,  2476,  2478,  2482,  2484,  2487,  2488,  2494,  2498,  2501,
    2502,  2506,  2507,  2512,  2515,  2516,  2520,  2524,  2526,  2527,
    2529,  2531,  2533,  2535,  2539,  2541,  2543,  2545,  2549,  2551,
    2553,  2557,  2561,  2564,  2569,  2572,  2577,  2579,  2581,  2583,
    2585,  2587,  2591,  2597,  2601,  2606,  2611,  2615,  2617,  2619,
    2621,  2623,  2627,  2633,  2638,  2642,  2644,  2646,  2650,  2654,
    2656,  2658,  2666,  2676,  2684,  2691,  2700,  2702,  2705,  2710,
    2715,  2717,  2719,  2724,  2726,  2727,  2729,  2732,  2734,  2736,
    2740,  2746,  2750,  2754,  2755,  2757,  2761,  2767,  2771,  2774,
    2778,  2785,  2786,  2788,  2793,  2796,  2797,  2803,  2807,  2811,
    2813,  2820,  2825,  2830,  2833,  2836,  2837,  2843,  2847,  2851,
    2853,  2856,  2857,  2863,  2867,  2871,  2873,  2876,  2879,  2881,
    2884,  2886,  2891,  2895,  2899,  2906,  2910,  2912,  2914,  2916,
    2921,  2926,  2931,  2936,  2941,  2946,  2949,  2952,  2957,  2960,
    2963,  2965,  2969,  2973,  2977,  2978,  2981,  2987,  2994,  2996,
    2999,  3001,  3006,  3010,  3011,  3013,  3017,  3020,  3024,  3026,
    3028,  3029,  3030,  3033,  3038,  3041,  3048,  3053,  3055,  3057,
    3058,  3062,  3068,  3072,  3074,  3077,  3078,  3083,  3085,  3089,
    3092,  3095,  3098,  3100,  3102,  3104,  3106,  3110,  3115,  3122,
    3124,  3133,  3140,  3142
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     209,     0,    -1,    -1,   210,   211,    -1,   211,   212,    -1,
      -1,   230,    -1,   247,    -1,   254,    -1,   251,    -1,   259,
      -1,   464,    -1,   123,   198,   199,   200,    -1,   150,   222,
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
     467,    -1,   223,   467,    -1,   227,     9,   465,    14,   405,
      -1,   106,   465,    14,   405,    -1,   228,   229,    -1,    -1,
     230,    -1,   247,    -1,   254,    -1,   259,    -1,   201,   228,
     202,    -1,    70,   334,   230,   281,   283,    -1,    70,   334,
      30,   228,   282,   284,    73,   200,    -1,    -1,    89,   334,
     231,   275,    -1,    -1,    88,   232,   230,    89,   334,   200,
      -1,    -1,    91,   198,   336,   200,   336,   200,   336,   199,
     233,   273,    -1,    -1,    98,   334,   234,   278,    -1,   102,
     200,    -1,   102,   343,   200,    -1,   104,   200,    -1,   104,
     343,   200,    -1,   107,   200,    -1,   107,   343,   200,    -1,
      27,   102,   200,    -1,   112,   291,   200,    -1,   118,   293,
     200,    -1,    87,   335,   200,    -1,   120,   198,   461,   199,
     200,    -1,   200,    -1,    81,    -1,    82,    -1,    -1,    93,
     198,   343,    97,   272,   271,   199,   235,   274,    -1,    -1,
      93,   198,   343,    28,    97,   272,   271,   199,   236,   274,
      -1,    95,   198,   277,   199,   276,    -1,    -1,   108,   239,
     109,   198,   398,    79,   199,   201,   228,   202,   241,   237,
     244,    -1,    -1,   108,   239,   167,   238,   242,    -1,   110,
     343,   200,    -1,   103,   215,   200,    -1,   343,   200,    -1,
     337,   200,    -1,   338,   200,    -1,   339,   200,    -1,   340,
     200,    -1,   341,   200,    -1,   107,   340,   200,    -1,   342,
     200,    -1,   368,   200,    -1,   107,   367,   200,    -1,   215,
      30,    -1,    -1,   201,   240,   228,   202,    -1,   241,   109,
     198,   398,    79,   199,   201,   228,   202,    -1,    -1,    -1,
     201,   243,   228,   202,    -1,   167,   242,    -1,    -1,    35,
      -1,    -1,   105,    -1,    -1,   246,   245,   466,   248,   198,
     287,   199,   471,   320,    -1,    -1,   324,   246,   245,   466,
     249,   198,   287,   199,   471,   320,    -1,    -1,   426,   323,
     246,   245,   466,   250,   198,   287,   199,   471,   320,    -1,
      -1,   160,   215,   252,    30,   480,   463,   201,   294,   202,
      -1,    -1,   426,   160,   215,   253,    30,   480,   463,   201,
     294,   202,    -1,    -1,   265,   262,   255,   266,   267,   201,
     297,   202,    -1,    -1,   426,   265,   262,   256,   266,   267,
     201,   297,   202,    -1,    -1,   125,   263,   257,   268,   201,
     297,   202,    -1,    -1,   426,   125,   263,   258,   268,   201,
     297,   202,    -1,    -1,   162,   264,   260,   267,   201,   297,
     202,    -1,    -1,   426,   162,   264,   261,   267,   201,   297,
     202,    -1,   466,    -1,   154,    -1,   466,    -1,   466,    -1,
     124,    -1,   117,   124,    -1,   117,   116,   124,    -1,   116,
     117,   124,    -1,   116,   124,    -1,   126,   398,    -1,    -1,
     127,   269,    -1,    -1,   126,   269,    -1,    -1,   398,    -1,
     269,     9,   398,    -1,   398,    -1,   270,     9,   398,    -1,
     130,   272,    -1,    -1,   436,    -1,    35,   436,    -1,   131,
     198,   450,   199,    -1,   230,    -1,    30,   228,    92,   200,
      -1,   230,    -1,    30,   228,    94,   200,    -1,   230,    -1,
      30,   228,    90,   200,    -1,   230,    -1,    30,   228,    96,
     200,    -1,   215,    14,   405,    -1,   277,     9,   215,    14,
     405,    -1,   201,   279,   202,    -1,   201,   200,   279,   202,
      -1,    30,   279,    99,   200,    -1,    30,   200,   279,    99,
     200,    -1,   279,   100,   343,   280,   228,    -1,   279,   101,
     280,   228,    -1,    -1,    30,    -1,   200,    -1,   281,    71,
     334,   230,    -1,    -1,   282,    71,   334,    30,   228,    -1,
      -1,    72,   230,    -1,    -1,    72,    30,   228,    -1,    -1,
     286,     9,   427,   326,   481,   163,    79,    -1,   286,     9,
     427,   326,   481,    35,   163,    79,    -1,   286,     9,   427,
     326,   481,   163,    -1,   286,   410,    -1,   427,   326,   481,
     163,    79,    -1,   427,   326,   481,    35,   163,    79,    -1,
     427,   326,   481,   163,    -1,    -1,   427,   326,   481,    79,
      -1,   427,   326,   481,    35,    79,    -1,   427,   326,   481,
      35,    79,    14,   343,    -1,   427,   326,   481,    79,    14,
     343,    -1,   286,     9,   427,   326,   481,    79,    -1,   286,
       9,   427,   326,   481,    35,    79,    -1,   286,     9,   427,
     326,   481,    35,    79,    14,   343,    -1,   286,     9,   427,
     326,   481,    79,    14,   343,    -1,   288,     9,   427,   481,
     163,    79,    -1,   288,     9,   427,   481,    35,   163,    79,
      -1,   288,     9,   427,   481,   163,    -1,   288,   410,    -1,
     427,   481,   163,    79,    -1,   427,   481,    35,   163,    79,
      -1,   427,   481,   163,    -1,    -1,   427,   481,    79,    -1,
     427,   481,    35,    79,    -1,   427,   481,    35,    79,    14,
     343,    -1,   427,   481,    79,    14,   343,    -1,   288,     9,
     427,   481,    79,    -1,   288,     9,   427,   481,    35,    79,
      -1,   288,     9,   427,   481,    35,    79,    14,   343,    -1,
     288,     9,   427,   481,    79,    14,   343,    -1,   290,   410,
      -1,    -1,   343,    -1,    35,   436,    -1,   163,   343,    -1,
     290,     9,   343,    -1,   290,     9,   163,   343,    -1,   290,
       9,    35,   436,    -1,   291,     9,   292,    -1,   292,    -1,
      79,    -1,   203,   436,    -1,   203,   201,   343,   202,    -1,
     293,     9,    79,    -1,   293,     9,    79,    14,   405,    -1,
      79,    -1,    79,    14,   405,    -1,   294,   295,    -1,    -1,
     296,   200,    -1,   465,    14,   405,    -1,   297,   298,    -1,
      -1,    -1,   322,   299,   328,   200,    -1,    -1,   324,   480,
     300,   328,   200,    -1,   329,   200,    -1,   330,   200,    -1,
     331,   200,    -1,    -1,   323,   246,   245,   466,   198,   301,
     285,   199,   471,   321,    -1,    -1,   426,   323,   246,   245,
     466,   198,   302,   285,   199,   471,   321,    -1,   156,   307,
     200,    -1,   157,   314,   200,    -1,   159,   316,   200,    -1,
       4,   126,   398,   200,    -1,     4,   127,   398,   200,    -1,
     111,   270,   200,    -1,   111,   270,   201,   303,   202,    -1,
     303,   304,    -1,   303,   305,    -1,    -1,   226,   149,   215,
     164,   270,   200,    -1,   306,    97,   323,   215,   200,    -1,
     306,    97,   324,   200,    -1,   226,   149,   215,    -1,   215,
      -1,   308,    -1,   307,     9,   308,    -1,   309,   395,   312,
     313,    -1,   154,    -1,    29,   310,    -1,   310,    -1,   132,
      -1,   132,   170,   480,   171,    -1,   132,   170,   480,     9,
     480,   171,    -1,   398,    -1,   119,    -1,   160,   201,   311,
     202,    -1,   133,    -1,   404,    -1,   311,     9,   404,    -1,
      14,   405,    -1,    -1,    55,   161,    -1,    -1,   315,    -1,
     314,     9,   315,    -1,   158,    -1,   317,    -1,   215,    -1,
     122,    -1,   198,   318,   199,    -1,   198,   318,   199,    49,
      -1,   198,   318,   199,    29,    -1,   198,   318,   199,    46,
      -1,   317,    -1,   319,    -1,   319,    49,    -1,   319,    29,
      -1,   319,    46,    -1,   318,     9,   318,    -1,   318,    33,
     318,    -1,   215,    -1,   154,    -1,   158,    -1,   200,    -1,
     201,   228,   202,    -1,   200,    -1,   201,   228,   202,    -1,
     324,    -1,   119,    -1,   324,    -1,    -1,   325,    -1,   324,
     325,    -1,   113,    -1,   114,    -1,   115,    -1,   118,    -1,
     117,    -1,   116,    -1,   180,    -1,   327,    -1,    -1,   113,
      -1,   114,    -1,   115,    -1,   328,     9,    79,    -1,   328,
       9,    79,    14,   405,    -1,    79,    -1,    79,    14,   405,
      -1,   329,     9,   465,    14,   405,    -1,   106,   465,    14,
     405,    -1,   330,     9,   465,    -1,   117,   106,   465,    -1,
     117,   332,   463,    -1,   332,   463,    14,   480,    -1,   106,
     175,   466,    -1,   198,   333,   199,    -1,    68,   400,   403,
      -1,    67,   343,    -1,   387,    -1,   363,    -1,   198,   343,
     199,    -1,   335,     9,   343,    -1,   343,    -1,   335,    -1,
      -1,    27,    -1,    27,   343,    -1,    27,   343,   130,   343,
      -1,   436,    14,   337,    -1,   131,   198,   450,   199,    14,
     337,    -1,    28,   343,    -1,   436,    14,   340,    -1,   131,
     198,   450,   199,    14,   340,    -1,   344,    -1,   436,    -1,
     333,    -1,   440,    -1,   439,    -1,   131,   198,   450,   199,
      14,   343,    -1,   436,    14,   343,    -1,   436,    14,    35,
     436,    -1,   436,    14,    35,    68,   400,   403,    -1,   436,
      26,   343,    -1,   436,    25,   343,    -1,   436,    24,   343,
      -1,   436,    23,   343,    -1,   436,    22,   343,    -1,   436,
      21,   343,    -1,   436,    20,   343,    -1,   436,    19,   343,
      -1,   436,    18,   343,    -1,   436,    17,   343,    -1,   436,
      16,   343,    -1,   436,    15,   343,    -1,   436,    64,    -1,
      64,   436,    -1,   436,    63,    -1,    63,   436,    -1,   343,
      31,   343,    -1,   343,    32,   343,    -1,   343,    10,   343,
      -1,   343,    12,   343,    -1,   343,    11,   343,    -1,   343,
      33,   343,    -1,   343,    35,   343,    -1,   343,    34,   343,
      -1,   343,    48,   343,    -1,   343,    46,   343,    -1,   343,
      47,   343,    -1,   343,    49,   343,    -1,   343,    50,   343,
      -1,   343,    65,   343,    -1,   343,    51,   343,    -1,   343,
      45,   343,    -1,   343,    44,   343,    -1,    46,   343,    -1,
      47,   343,    -1,    52,   343,    -1,    54,   343,    -1,   343,
      37,   343,    -1,   343,    36,   343,    -1,   343,    39,   343,
      -1,   343,    38,   343,    -1,   343,    40,   343,    -1,   343,
      43,   343,    -1,   343,    41,   343,    -1,   343,    42,   343,
      -1,   343,    53,   400,    -1,   198,   344,   199,    -1,   343,
      29,   343,    30,   343,    -1,   343,    29,    30,   343,    -1,
     460,    -1,    62,   343,    -1,    61,   343,    -1,    60,   343,
      -1,    59,   343,    -1,    58,   343,    -1,    57,   343,    -1,
      56,   343,    -1,    69,   401,    -1,    55,   343,    -1,   407,
      -1,   362,    -1,   361,    -1,   204,   402,   204,    -1,    13,
     343,    -1,   365,    -1,   111,   198,   386,   410,   199,    -1,
      -1,    -1,   246,   245,   198,   347,   287,   199,   471,   345,
     201,   228,   202,    -1,    -1,   324,   246,   245,   198,   348,
     287,   199,   471,   345,   201,   228,   202,    -1,    -1,    79,
     350,   355,    -1,    -1,   180,    79,   351,   355,    -1,    -1,
     195,   352,   287,   196,   471,   355,    -1,    -1,   180,   195,
     353,   287,   196,   471,   355,    -1,    -1,   180,   201,   354,
     228,   202,    -1,     8,   343,    -1,     8,   340,    -1,     8,
     201,   228,   202,    -1,    86,    -1,   462,    -1,   357,     9,
     356,   130,   343,    -1,   356,   130,   343,    -1,   358,     9,
     356,   130,   405,    -1,   356,   130,   405,    -1,   357,   409,
      -1,    -1,   358,   409,    -1,    -1,   174,   198,   359,   199,
      -1,   132,   198,   451,   199,    -1,    66,   451,   205,    -1,
     398,   201,   453,   202,    -1,   398,   201,   455,   202,    -1,
     365,    66,   446,   205,    -1,   366,    66,   446,   205,    -1,
     362,    -1,   462,    -1,   439,    -1,    86,    -1,   198,   344,
     199,    -1,   369,   370,    -1,   436,    14,   367,    -1,   181,
      79,   184,   343,    -1,   371,   382,    -1,   371,   382,   385,
      -1,   382,    -1,   382,   385,    -1,   372,    -1,   371,   372,
      -1,   373,    -1,   374,    -1,   375,    -1,   376,    -1,   377,
      -1,   378,    -1,   181,    79,   184,   343,    -1,   188,    79,
      14,   343,    -1,   182,   343,    -1,   183,    79,   184,   343,
     185,   343,   186,   343,    -1,   183,    79,   184,   343,   185,
     343,   186,   343,   187,    79,    -1,   189,   379,    -1,   380,
      -1,   379,     9,   380,    -1,   343,    -1,   343,   381,    -1,
     190,    -1,   191,    -1,   383,    -1,   384,    -1,   192,   343,
      -1,   193,   343,   194,   343,    -1,   187,    79,   370,    -1,
     386,     9,    79,    -1,   386,     9,    35,    79,    -1,    79,
      -1,    35,    79,    -1,   168,   154,   388,   169,    -1,   390,
      50,    -1,   390,   169,   391,   168,    50,   389,    -1,    -1,
     154,    -1,   390,   392,    14,   393,    -1,    -1,   391,   394,
      -1,    -1,   154,    -1,   155,    -1,   201,   343,   202,    -1,
     155,    -1,   201,   343,   202,    -1,   387,    -1,   396,    -1,
     395,    30,   396,    -1,   395,    47,   396,    -1,   215,    -1,
      69,    -1,   105,    -1,   106,    -1,   107,    -1,    27,    -1,
      28,    -1,   108,    -1,   109,    -1,   167,    -1,   110,    -1,
      70,    -1,    71,    -1,    73,    -1,    72,    -1,    89,    -1,
      90,    -1,    88,    -1,    91,    -1,    92,    -1,    93,    -1,
      94,    -1,    95,    -1,    96,    -1,    53,    -1,    97,    -1,
      98,    -1,    99,    -1,   100,    -1,   101,    -1,   102,    -1,
     104,    -1,   103,    -1,    87,    -1,    13,    -1,   124,    -1,
     125,    -1,   126,    -1,   127,    -1,    68,    -1,    67,    -1,
     119,    -1,     5,    -1,     7,    -1,     6,    -1,     4,    -1,
       3,    -1,   150,    -1,   111,    -1,   112,    -1,   121,    -1,
     122,    -1,   123,    -1,   118,    -1,   117,    -1,   116,    -1,
     115,    -1,   114,    -1,   113,    -1,   180,    -1,   120,    -1,
     131,    -1,   132,    -1,    10,    -1,    12,    -1,    11,    -1,
     134,    -1,   136,    -1,   135,    -1,   137,    -1,   138,    -1,
     152,    -1,   151,    -1,   179,    -1,   162,    -1,   165,    -1,
     164,    -1,   175,    -1,   177,    -1,   174,    -1,   225,   198,
     289,   199,    -1,   226,    -1,   154,    -1,   398,    -1,   118,
      -1,   444,    -1,   398,    -1,   118,    -1,   448,    -1,   198,
     199,    -1,   334,    -1,    -1,    -1,    85,    -1,   457,    -1,
     198,   289,   199,    -1,    -1,    74,    -1,    75,    -1,    76,
      -1,    86,    -1,   137,    -1,   138,    -1,   152,    -1,   134,
      -1,   165,    -1,   135,    -1,   136,    -1,   151,    -1,   179,
      -1,   145,    85,   146,    -1,   145,   146,    -1,   404,    -1,
     224,    -1,   132,   198,   408,   199,    -1,    66,   408,   205,
      -1,   174,   198,   360,   199,    -1,   406,    -1,   364,    -1,
     198,   405,   199,    -1,   405,    31,   405,    -1,   405,    32,
     405,    -1,   405,    10,   405,    -1,   405,    12,   405,    -1,
     405,    11,   405,    -1,   405,    33,   405,    -1,   405,    35,
     405,    -1,   405,    34,   405,    -1,   405,    48,   405,    -1,
     405,    46,   405,    -1,   405,    47,   405,    -1,   405,    49,
     405,    -1,   405,    50,   405,    -1,   405,    51,   405,    -1,
     405,    45,   405,    -1,   405,    44,   405,    -1,   405,    65,
     405,    -1,    52,   405,    -1,    54,   405,    -1,    46,   405,
      -1,    47,   405,    -1,   405,    37,   405,    -1,   405,    36,
     405,    -1,   405,    39,   405,    -1,   405,    38,   405,    -1,
     405,    40,   405,    -1,   405,    43,   405,    -1,   405,    41,
     405,    -1,   405,    42,   405,    -1,   405,    29,   405,    30,
     405,    -1,   405,    29,    30,   405,    -1,   226,   149,   215,
      -1,   154,   149,   215,    -1,   226,   149,   124,    -1,   224,
      -1,    78,    -1,   462,    -1,   404,    -1,   206,   457,   206,
      -1,   207,   457,   207,    -1,   145,   457,   146,    -1,   411,
     409,    -1,    -1,     9,    -1,    -1,     9,    -1,    -1,   411,
       9,   405,   130,   405,    -1,   411,     9,   405,    -1,   405,
     130,   405,    -1,   405,    -1,    74,    -1,    75,    -1,    76,
      -1,   145,    85,   146,    -1,   145,   146,    -1,    74,    -1,
      75,    -1,    76,    -1,   215,    -1,    86,    -1,    86,    48,
     414,    -1,   412,    -1,   414,    -1,   215,    -1,    46,   413,
      -1,    47,   413,    -1,   132,   198,   416,   199,    -1,    66,
     416,   205,    -1,   174,   198,   419,   199,    -1,   417,   409,
      -1,    -1,   417,     9,   415,   130,   415,    -1,   417,     9,
     415,    -1,   415,   130,   415,    -1,   415,    -1,   418,     9,
     415,    -1,   415,    -1,   420,   409,    -1,    -1,   420,     9,
     356,   130,   415,    -1,   356,   130,   415,    -1,   418,   409,
      -1,    -1,   198,   421,   199,    -1,    -1,   423,     9,   215,
     422,    -1,   215,   422,    -1,    -1,   425,   423,   409,    -1,
      45,   424,    44,    -1,   426,    -1,    -1,   128,    -1,   129,
      -1,   215,    -1,   154,    -1,   201,   343,   202,    -1,   429,
      -1,   443,    -1,   215,    -1,   201,   343,   202,    -1,   431,
      -1,   443,    -1,    66,   446,   205,    -1,   201,   343,   202,
      -1,   437,   433,    -1,   198,   333,   199,   433,    -1,   449,
     433,    -1,   198,   333,   199,   433,    -1,   443,    -1,   397,
      -1,   441,    -1,   442,    -1,   434,    -1,   436,   428,   430,
      -1,   198,   333,   199,   428,   430,    -1,   399,   149,   443,
      -1,   438,   198,   289,   199,    -1,   439,   198,   289,   199,
      -1,   198,   436,   199,    -1,   397,    -1,   441,    -1,   442,
      -1,   434,    -1,   436,   428,   429,    -1,   198,   333,   199,
     428,   429,    -1,   438,   198,   289,   199,    -1,   198,   436,
     199,    -1,   443,    -1,   434,    -1,   198,   436,   199,    -1,
     198,   440,   199,    -1,   346,    -1,   349,    -1,   436,   428,
     432,   467,   198,   289,   199,    -1,   198,   333,   199,   428,
     432,   467,   198,   289,   199,    -1,   399,   149,   215,   467,
     198,   289,   199,    -1,   399,   149,   443,   198,   289,   199,
      -1,   399,   149,   201,   343,   202,   198,   289,   199,    -1,
     444,    -1,   447,   444,    -1,   444,    66,   446,   205,    -1,
     444,   201,   343,   202,    -1,   445,    -1,    79,    -1,   203,
     201,   343,   202,    -1,   343,    -1,    -1,   203,    -1,   447,
     203,    -1,   443,    -1,   435,    -1,   448,   428,   430,    -1,
     198,   333,   199,   428,   430,    -1,   399,   149,   443,    -1,
     198,   436,   199,    -1,    -1,   435,    -1,   448,   428,   429,
      -1,   198,   333,   199,   428,   429,    -1,   198,   436,   199,
      -1,   450,     9,    -1,   450,     9,   436,    -1,   450,     9,
     131,   198,   450,   199,    -1,    -1,   436,    -1,   131,   198,
     450,   199,    -1,   452,   409,    -1,    -1,   452,     9,   343,
     130,   343,    -1,   452,     9,   343,    -1,   343,   130,   343,
      -1,   343,    -1,   452,     9,   343,   130,    35,   436,    -1,
     452,     9,    35,   436,    -1,   343,   130,    35,   436,    -1,
      35,   436,    -1,   454,   409,    -1,    -1,   454,     9,   343,
     130,   343,    -1,   454,     9,   343,    -1,   343,   130,   343,
      -1,   343,    -1,   456,   409,    -1,    -1,   456,     9,   405,
     130,   405,    -1,   456,     9,   405,    -1,   405,   130,   405,
      -1,   405,    -1,   457,   458,    -1,   457,    85,    -1,   458,
      -1,    85,   458,    -1,    79,    -1,    79,    66,   459,   205,
      -1,    79,   428,   215,    -1,   147,   343,   202,    -1,   147,
      78,    66,   343,   205,   202,    -1,   148,   436,   202,    -1,
     215,    -1,    80,    -1,    79,    -1,   121,   198,   461,   199,
      -1,   122,   198,   436,   199,    -1,   122,   198,   344,   199,
      -1,   122,   198,   440,   199,    -1,   122,   198,   439,   199,
      -1,   122,   198,   333,   199,    -1,     7,   343,    -1,     6,
     343,    -1,     5,   198,   343,   199,    -1,     4,   343,    -1,
       3,   343,    -1,   436,    -1,   461,     9,   436,    -1,   399,
     149,   215,    -1,   399,   149,   124,    -1,    -1,    97,   480,
      -1,   175,   466,    14,   480,   200,    -1,   177,   466,   463,
      14,   480,   200,    -1,   215,    -1,   480,   215,    -1,   215,
      -1,   215,   170,   472,   171,    -1,   170,   469,   171,    -1,
      -1,   480,    -1,   468,     9,   480,    -1,   468,   409,    -1,
     468,     9,   163,    -1,   469,    -1,   163,    -1,    -1,    -1,
      30,   480,    -1,   472,     9,   473,   215,    -1,   473,   215,
      -1,   472,     9,   473,   215,    97,   480,    -1,   473,   215,
      97,   480,    -1,    46,    -1,    47,    -1,    -1,    86,   130,
     480,    -1,   226,   149,   215,   130,   480,    -1,   475,     9,
     474,    -1,   474,    -1,   475,   409,    -1,    -1,   174,   198,
     476,   199,    -1,   226,    -1,   215,   149,   479,    -1,   215,
     467,    -1,    29,   480,    -1,    55,   480,    -1,   226,    -1,
     132,    -1,   133,    -1,   477,    -1,   478,   149,   479,    -1,
     132,   170,   480,   171,    -1,   132,   170,   480,     9,   480,
     171,    -1,   154,    -1,   198,   105,   198,   470,   199,    30,
     480,   199,    -1,   198,   480,     9,   468,   409,   199,    -1,
     480,    -1,    -1
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
    1580,  1584,  1585,  1589,  1590,  1592,  1594,  1600,  1601,  1603,
    1607,  1608,  1613,  1614,  1618,  1619,  1623,  1625,  1631,  1636,
    1637,  1639,  1643,  1644,  1645,  1646,  1650,  1651,  1652,  1653,
    1654,  1655,  1657,  1662,  1665,  1666,  1670,  1671,  1675,  1676,
    1679,  1680,  1683,  1684,  1687,  1688,  1692,  1693,  1694,  1695,
    1696,  1697,  1698,  1702,  1703,  1706,  1707,  1708,  1711,  1713,
    1715,  1716,  1719,  1721,  1725,  1727,  1731,  1735,  1739,  1743,
    1744,  1746,  1747,  1748,  1751,  1755,  1756,  1760,  1761,  1765,
    1766,  1767,  1771,  1775,  1780,  1784,  1788,  1793,  1794,  1795,
    1796,  1797,  1801,  1803,  1804,  1805,  1808,  1809,  1810,  1811,
    1812,  1813,  1814,  1815,  1816,  1817,  1818,  1819,  1820,  1821,
    1822,  1823,  1824,  1825,  1826,  1827,  1828,  1829,  1830,  1831,
    1832,  1833,  1834,  1835,  1836,  1837,  1838,  1839,  1840,  1841,
    1842,  1843,  1844,  1845,  1846,  1847,  1848,  1849,  1850,  1852,
    1853,  1855,  1857,  1858,  1859,  1860,  1861,  1862,  1863,  1864,
    1865,  1866,  1867,  1868,  1869,  1870,  1871,  1872,  1873,  1874,
    1875,  1879,  1883,  1888,  1887,  1902,  1900,  1917,  1917,  1933,
    1932,  1950,  1950,  1966,  1965,  1984,  1983,  2004,  2005,  2006,
    2011,  2013,  2017,  2021,  2027,  2031,  2037,  2039,  2043,  2045,
    2049,  2053,  2054,  2058,  2065,  2072,  2074,  2079,  2080,  2081,
    2082,  2084,  2088,  2092,  2096,  2100,  2102,  2104,  2106,  2111,
    2112,  2117,  2118,  2119,  2120,  2121,  2122,  2126,  2130,  2134,
    2138,  2143,  2148,  2152,  2153,  2157,  2158,  2162,  2163,  2167,
    2168,  2172,  2176,  2180,  2184,  2185,  2186,  2187,  2191,  2197,
    2206,  2219,  2220,  2223,  2226,  2229,  2230,  2233,  2237,  2240,
    2243,  2250,  2251,  2255,  2256,  2258,  2262,  2263,  2264,  2265,
    2266,  2267,  2268,  2269,  2270,  2271,  2272,  2273,  2274,  2275,
    2276,  2277,  2278,  2279,  2280,  2281,  2282,  2283,  2284,  2285,
    2286,  2287,  2288,  2289,  2290,  2291,  2292,  2293,  2294,  2295,
    2296,  2297,  2298,  2299,  2300,  2301,  2302,  2303,  2304,  2305,
    2306,  2307,  2308,  2309,  2310,  2311,  2312,  2313,  2314,  2315,
    2316,  2317,  2318,  2319,  2320,  2321,  2322,  2323,  2324,  2325,
    2326,  2327,  2328,  2329,  2330,  2331,  2332,  2333,  2334,  2335,
    2336,  2337,  2338,  2339,  2340,  2341,  2345,  2350,  2351,  2354,
    2355,  2356,  2360,  2361,  2362,  2366,  2367,  2368,  2372,  2373,
    2374,  2377,  2379,  2383,  2384,  2385,  2386,  2388,  2389,  2390,
    2391,  2392,  2393,  2394,  2395,  2396,  2397,  2400,  2405,  2406,
    2407,  2409,  2410,  2412,  2413,  2414,  2415,  2417,  2419,  2421,
    2423,  2425,  2426,  2427,  2428,  2429,  2430,  2431,  2432,  2433,
    2434,  2435,  2436,  2437,  2438,  2439,  2440,  2441,  2443,  2445,
    2447,  2449,  2450,  2453,  2454,  2458,  2460,  2464,  2467,  2470,
    2476,  2477,  2478,  2479,  2480,  2481,  2482,  2487,  2489,  2493,
    2494,  2497,  2498,  2502,  2505,  2507,  2509,  2513,  2514,  2515,
    2516,  2519,  2523,  2524,  2525,  2526,  2530,  2532,  2539,  2540,
    2541,  2542,  2543,  2544,  2546,  2547,  2552,  2554,  2557,  2560,
    2562,  2564,  2567,  2569,  2573,  2575,  2578,  2581,  2587,  2589,
    2592,  2593,  2598,  2601,  2605,  2605,  2610,  2613,  2614,  2618,
    2619,  2623,  2624,  2625,  2629,  2630,  2634,  2635,  2639,  2640,
    2644,  2645,  2649,  2650,  2655,  2657,  2662,  2663,  2664,  2665,
    2666,  2667,  2677,  2688,  2691,  2693,  2695,  2699,  2700,  2701,
    2702,  2703,  2714,  2726,  2728,  2732,  2733,  2734,  2738,  2742,
    2743,  2747,  2750,  2757,  2761,  2765,  2772,  2773,  2778,  2780,
    2781,  2784,  2785,  2788,  2789,  2793,  2794,  2798,  2799,  2800,
    2811,  2822,  2825,  2828,  2829,  2830,  2841,  2853,  2857,  2858,
    2859,  2861,  2862,  2863,  2867,  2869,  2872,  2874,  2875,  2876,
    2877,  2880,  2882,  2883,  2887,  2889,  2892,  2894,  2895,  2896,
    2900,  2902,  2905,  2908,  2910,  2912,  2916,  2917,  2919,  2920,
    2926,  2927,  2929,  2939,  2941,  2943,  2946,  2947,  2948,  2952,
    2953,  2954,  2955,  2956,  2957,  2958,  2959,  2960,  2961,  2962,
    2966,  2967,  2971,  2973,  2981,  2983,  2987,  2991,  2998,  2999,
    3005,  3006,  3013,  3016,  3020,  3023,  3028,  3033,  3035,  3036,
    3037,  3041,  3042,  3046,  3048,  3049,  3052,  3057,  3058,  3059,
    3063,  3066,  3075,  3077,  3081,  3084,  3087,  3092,  3095,  3098,
    3105,  3108,  3111,  3112,  3115,  3118,  3119,  3124,  3127,  3131,
    3135,  3141,  3151,  3152
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
  "xhp_nullable_attribute_decl_type", "xhp_attribute_decl_type",
  "xhp_attribute_enum", "xhp_attribute_default",
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
     308,   309,   309,   310,   310,   310,   310,   310,   310,   310,
     311,   311,   312,   312,   313,   313,   314,   314,   315,   316,
     316,   316,   317,   317,   317,   317,   318,   318,   318,   318,
     318,   318,   318,   319,   319,   319,   320,   320,   321,   321,
     322,   322,   323,   323,   324,   324,   325,   325,   325,   325,
     325,   325,   325,   326,   326,   327,   327,   327,   328,   328,
     328,   328,   329,   329,   330,   330,   331,   331,   332,   333,
     333,   333,   333,   333,   334,   335,   335,   336,   336,   337,
     337,   337,   338,   339,   340,   341,   342,   343,   343,   343,
     343,   343,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   345,   345,   347,   346,   348,   346,   350,   349,   351,
     349,   352,   349,   353,   349,   354,   349,   355,   355,   355,
     356,   356,   357,   357,   358,   358,   359,   359,   360,   360,
     361,   362,   362,   363,   364,   365,   365,   366,   366,   366,
     366,   366,   367,   368,   369,   370,   370,   370,   370,   371,
     371,   372,   372,   372,   372,   372,   372,   373,   374,   375,
     376,   377,   378,   379,   379,   380,   380,   381,   381,   382,
     382,   383,   384,   385,   386,   386,   386,   386,   387,   388,
     388,   389,   389,   390,   390,   391,   391,   392,   393,   393,
     394,   394,   394,   395,   395,   395,   396,   396,   396,   396,
     396,   396,   396,   396,   396,   396,   396,   396,   396,   396,
     396,   396,   396,   396,   396,   396,   396,   396,   396,   396,
     396,   396,   396,   396,   396,   396,   396,   396,   396,   396,
     396,   396,   396,   396,   396,   396,   396,   396,   396,   396,
     396,   396,   396,   396,   396,   396,   396,   396,   396,   396,
     396,   396,   396,   396,   396,   396,   396,   396,   396,   396,
     396,   396,   396,   396,   396,   396,   396,   396,   396,   396,
     396,   396,   396,   396,   396,   396,   397,   398,   398,   399,
     399,   399,   400,   400,   400,   401,   401,   401,   402,   402,
     402,   403,   403,   404,   404,   404,   404,   404,   404,   404,
     404,   404,   404,   404,   404,   404,   404,   404,   405,   405,
     405,   405,   405,   405,   405,   405,   405,   405,   405,   405,
     405,   405,   405,   405,   405,   405,   405,   405,   405,   405,
     405,   405,   405,   405,   405,   405,   405,   405,   405,   405,
     405,   405,   405,   405,   405,   405,   405,   406,   406,   406,
     407,   407,   407,   407,   407,   407,   407,   408,   408,   409,
     409,   410,   410,   411,   411,   411,   411,   412,   412,   412,
     412,   412,   413,   413,   413,   413,   414,   414,   415,   415,
     415,   415,   415,   415,   415,   415,   416,   416,   417,   417,
     417,   417,   418,   418,   419,   419,   420,   420,   421,   421,
     422,   422,   423,   423,   425,   424,   426,   427,   427,   428,
     428,   429,   429,   429,   430,   430,   431,   431,   432,   432,
     433,   433,   434,   434,   435,   435,   436,   436,   436,   436,
     436,   436,   436,   436,   436,   436,   436,   437,   437,   437,
     437,   437,   437,   437,   437,   438,   438,   438,   439,   440,
     440,   441,   441,   442,   442,   442,   443,   443,   444,   444,
     444,   445,   445,   446,   446,   447,   447,   448,   448,   448,
     448,   448,   448,   449,   449,   449,   449,   449,   450,   450,
     450,   450,   450,   450,   451,   451,   452,   452,   452,   452,
     452,   452,   452,   452,   453,   453,   454,   454,   454,   454,
     455,   455,   456,   456,   456,   456,   457,   457,   457,   457,
     458,   458,   458,   458,   458,   458,   459,   459,   459,   460,
     460,   460,   460,   460,   460,   460,   460,   460,   460,   460,
     461,   461,   462,   462,   463,   463,   464,   464,   465,   465,
     466,   466,   467,   467,   468,   468,   469,   470,   470,   470,
     470,   471,   471,   472,   472,   472,   472,   473,   473,   473,
     474,   474,   475,   475,   476,   476,   477,   478,   479,   479,
     480,   480,   480,   480,   480,   480,   480,   480,   480,   480,
     480,   480,   481,   481
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
       1,     2,     1,     1,     4,     6,     1,     1,     4,     1,
       1,     3,     2,     0,     2,     0,     1,     3,     1,     1,
       1,     1,     3,     4,     4,     4,     1,     1,     2,     2,
       2,     3,     3,     1,     1,     1,     1,     3,     1,     3,
       1,     1,     1,     0,     1,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     0,     1,     1,     1,     3,     5,
       1,     3,     5,     4,     3,     3,     3,     4,     3,     3,
       3,     2,     1,     1,     3,     3,     1,     1,     0,     1,
       2,     4,     3,     6,     2,     3,     6,     1,     1,     1,
       1,     1,     6,     3,     4,     6,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     5,     4,     1,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     1,     1,     1,     3,     2,
       1,     5,     0,     0,    11,     0,    12,     0,     3,     0,
       4,     0,     6,     0,     7,     0,     5,     2,     2,     4,
       1,     1,     5,     3,     5,     3,     2,     0,     2,     0,
       4,     4,     3,     4,     4,     4,     4,     1,     1,     1,
       1,     3,     2,     3,     4,     2,     3,     1,     2,     1,
       2,     1,     1,     1,     1,     1,     1,     4,     4,     2,
       8,    10,     2,     1,     3,     1,     2,     1,     1,     1,
       1,     2,     4,     3,     3,     4,     1,     2,     4,     2,
       6,     0,     1,     4,     0,     2,     0,     1,     1,     3,
       1,     3,     1,     1,     3,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     4,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     1,     0,     0,     1,
       1,     3,     0,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     2,     1,     1,
       4,     3,     4,     1,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     2,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     5,     4,     3,     3,     3,
       1,     1,     1,     1,     3,     3,     3,     2,     0,     1,
       0,     1,     0,     5,     3,     3,     1,     1,     1,     1,
       3,     2,     1,     1,     1,     1,     1,     3,     1,     1,
       1,     2,     2,     4,     3,     4,     2,     0,     5,     3,
       3,     1,     3,     1,     2,     0,     5,     3,     2,     0,
       3,     0,     4,     2,     0,     3,     3,     1,     0,     1,
       1,     1,     1,     3,     1,     1,     1,     3,     1,     1,
       3,     3,     2,     4,     2,     4,     1,     1,     1,     1,
       1,     3,     5,     3,     4,     4,     3,     1,     1,     1,
       1,     3,     5,     4,     3,     1,     1,     3,     3,     1,
       1,     7,     9,     7,     6,     8,     1,     2,     4,     4,
       1,     1,     4,     1,     0,     1,     2,     1,     1,     3,
       5,     3,     3,     0,     1,     3,     5,     3,     2,     3,
       6,     0,     1,     4,     2,     0,     5,     3,     3,     1,
       6,     4,     4,     2,     2,     0,     5,     3,     3,     1,
       2,     0,     5,     3,     3,     1,     2,     2,     1,     2,
       1,     4,     3,     3,     6,     3,     1,     1,     1,     4,
       4,     4,     4,     4,     4,     2,     2,     4,     2,     2,
       1,     3,     3,     3,     0,     2,     5,     6,     1,     2,
       1,     4,     3,     0,     1,     3,     2,     3,     1,     1,
       0,     0,     2,     4,     2,     6,     4,     1,     1,     0,
       3,     5,     3,     1,     2,     0,     4,     1,     3,     2,
       2,     2,     1,     1,     1,     1,     3,     4,     6,     1,
       8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   369,     0,   754,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   835,     0,
     823,   637,     0,   643,   644,   645,    22,   701,   811,    98,
      99,   646,     0,    80,     0,     0,     0,     0,     0,     0,
       0,     0,   132,     0,     0,     0,     0,     0,     0,   336,
     337,   338,   341,   340,   339,     0,     0,     0,     0,   159,
       0,     0,     0,   650,   652,   653,   647,   648,     0,     0,
     654,   649,     0,   628,    23,    24,    25,    27,    26,     0,
     651,     0,     0,     0,     0,   655,   342,    28,    29,    31,
      30,    32,    33,    34,    35,    36,    37,    38,    39,    40,
     461,     0,    97,    70,   815,   638,     0,     0,     4,    59,
      61,    64,   700,     0,   627,     0,     6,   131,     7,     9,
       8,    10,     0,     0,   334,   379,     0,     0,     0,     0,
       0,     0,     0,   377,   799,   800,   447,   446,   363,   450,
       0,     0,   362,   777,   629,     0,   703,   445,   333,   780,
     378,     0,     0,   381,   380,   778,   779,   776,   806,   810,
       0,   435,   702,    11,   341,   340,   339,     0,     0,    27,
      59,   131,     0,   879,   378,   878,     0,   876,   875,   449,
       0,   370,   374,     0,     0,   419,   420,   421,   422,   444,
     442,   441,   440,   439,   438,   437,   436,   811,   630,     0,
     893,   629,     0,   401,     0,   399,     0,   839,     0,   710,
     361,   633,     0,   893,   632,     0,   642,   818,   817,   634,
       0,     0,   636,   443,     0,     0,     0,     0,   366,     0,
      78,   368,     0,     0,    84,    86,     0,     0,    88,     0,
       0,     0,   923,   924,   929,     0,     0,    59,   922,     0,
     925,     0,     0,     0,    90,     0,     0,     0,     0,   122,
       0,     0,     0,     0,     0,     0,    42,    47,   248,     0,
       0,   247,     0,   163,     0,   160,   253,     0,     0,     0,
       0,     0,   890,   147,   157,   831,   835,   860,     0,   657,
       0,     0,     0,   858,     0,    16,     0,    63,   139,   151,
     158,   534,   477,     0,   884,   459,   463,   465,   758,   379,
       0,   377,   378,   380,     0,     0,   639,     0,   640,     0,
       0,     0,   121,     0,     0,    66,   239,     0,    21,   130,
       0,   156,   143,   155,   339,   342,   131,   335,   112,   113,
     114,   115,   116,   118,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   823,
       0,   111,   814,   814,   119,   845,     0,     0,     0,     0,
       0,     0,   332,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   400,   398,   759,   760,
       0,   814,     0,   772,   239,   239,   814,     0,   816,   807,
     831,     0,   131,     0,     0,    92,     0,   756,   751,   710,
       0,     0,     0,     0,     0,   843,     0,   482,   709,   834,
       0,     0,    66,     0,   239,   360,     0,   774,   635,     0,
      70,   199,     0,   458,     0,    95,     0,     0,   367,     0,
       0,     0,     0,     0,    87,   110,    89,   920,   921,     0,
     915,     0,     0,     0,     0,   889,     0,   117,    91,   120,
       0,     0,     0,     0,     0,     0,     0,   492,     0,   499,
     501,   502,   503,   504,   505,   506,   497,   519,   520,    70,
       0,   107,   109,     0,     0,    44,    51,     0,     0,    46,
      55,    48,     0,    18,     0,     0,   249,     0,    93,   162,
     161,     0,     0,    94,   880,     0,     0,   379,   377,   378,
     381,   380,     0,   909,   169,     0,   832,     0,     0,     0,
       0,   656,   859,   701,     0,     0,   857,   706,   856,    62,
       5,    13,    14,     0,   167,     0,     0,   470,     0,     0,
     710,     0,     0,   631,   471,     0,     0,     0,     0,   758,
      70,     0,   712,   757,   933,   359,   432,   786,   798,    75,
      69,    71,    72,    73,    74,   333,     0,   448,   704,   705,
      60,   710,     0,   894,     0,     0,     0,   712,   240,     0,
     453,   133,   165,     0,   404,   406,   405,     0,     0,   402,
     403,   407,   409,   408,   424,   423,   426,   425,   427,   429,
     430,   428,   418,   417,   411,   412,   410,   413,   414,   416,
     431,   415,   813,     0,     0,   849,     0,   710,   883,     0,
     882,   783,   806,   149,   141,   153,   145,   131,   369,     0,
     372,   375,   383,   493,   397,   396,   395,   394,   393,   392,
     391,   390,   389,   388,   387,   386,   762,     0,   761,   764,
     781,   768,   893,   765,     0,     0,     0,     0,     0,     0,
       0,     0,   877,   371,   749,   753,   709,   755,     0,     0,
     893,     0,   838,     0,   837,     0,   822,   821,     0,     0,
     761,   764,   819,   765,   364,   201,   203,    70,   468,   467,
     365,     0,    70,   183,    79,   368,     0,     0,     0,     0,
       0,   195,   195,    85,     0,     0,     0,   913,   710,     0,
     900,     0,     0,     0,     0,     0,   708,   646,     0,     0,
     628,     0,     0,    64,   659,   627,   664,     0,   658,    68,
     663,   893,   926,     0,     0,   509,     0,     0,   515,   512,
     513,   521,     0,   500,   495,     0,   498,     0,     0,     0,
      52,    19,     0,     0,    56,    20,     0,     0,     0,    41,
      49,     0,   246,   254,   251,     0,     0,   869,   874,   871,
     870,   873,   872,    12,   907,   908,     0,     0,     0,     0,
     831,   828,     0,   481,   868,   867,   866,     0,   862,     0,
     863,   865,     0,     5,     0,     0,     0,   528,   529,   537,
     536,     0,     0,   709,   476,   480,     0,     0,   885,     0,
     460,     0,     0,   901,   758,   225,   932,     0,     0,   773,
     812,   709,   896,   892,   241,   242,   626,   711,   238,     0,
     758,     0,     0,   167,   455,   135,   434,     0,   485,   486,
       0,   483,   709,   844,     0,     0,   239,   169,     0,   167,
     165,     0,   823,   384,     0,     0,   770,   771,   784,   785,
     808,   809,     0,     0,     0,   737,   717,   718,   719,   726,
       0,     0,     0,   730,   728,   729,   743,   710,     0,   751,
     842,   841,     0,     0,   775,   641,     0,   205,     0,     0,
      76,     0,     0,     0,     0,     0,     0,     0,   175,   176,
     187,     0,    70,   185,   104,   195,     0,   195,     0,     0,
     927,     0,     0,   709,   914,   916,   899,   710,   898,     0,
     710,   685,   686,   683,   684,   716,     0,   710,   708,     0,
       0,   479,     0,     0,   851,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   919,   494,     0,     0,     0,   517,   518,   516,
       0,     0,   496,     0,   123,     0,   126,   108,     0,    43,
      53,     0,    45,    57,    50,   250,     0,   881,    96,   909,
     891,   904,   168,   170,   260,     0,     0,   829,     0,   861,
       0,    17,     0,   884,   166,   260,     0,     0,   473,     0,
     882,   886,     0,   901,   466,     0,     0,   933,     0,   230,
     228,   764,   782,   893,   895,     0,     0,   243,    67,     0,
     758,   164,     0,   758,     0,   433,   848,   847,     0,   239,
       0,     0,     0,     0,   167,   137,   642,   763,   239,     0,
     722,   723,   724,   725,   731,   732,   741,     0,   710,     0,
     737,     0,   721,   745,   709,   748,   750,   752,     0,   836,
     764,   820,   763,     0,     0,     0,     0,   202,   469,    81,
       0,   368,   175,   177,   831,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   189,     0,   910,     0,   912,   709,
       0,     0,     0,   661,   709,   707,     0,   698,     0,   710,
       0,   665,   699,   697,   855,     0,   710,   668,   670,   669,
       0,     0,   666,   667,   671,   673,   672,   688,   687,   690,
     689,   691,   693,   694,   692,   681,   680,   675,   676,   674,
     677,   678,   679,   682,   918,   507,     0,   508,   514,   522,
     523,     0,    70,    54,    58,   252,     0,     0,     0,   333,
     833,   831,   373,   376,   382,     0,    15,     0,   333,   540,
       0,     0,   542,   535,   538,     0,   533,     0,   887,     0,
     902,   462,     0,   231,     0,     0,   226,     0,   245,   244,
     901,     0,   260,     0,   758,     0,   239,     0,   804,   260,
     884,   260,     0,     0,   385,     0,     0,   734,   709,   736,
     727,     0,   720,     0,     0,   710,   742,   840,     0,    70,
       0,   198,   184,     0,     0,     0,   174,   100,   188,     0,
       0,   191,     0,   196,   197,    70,   190,   928,     0,   897,
       0,   931,   715,   714,   660,     0,   709,   478,   662,     0,
     484,   709,   850,   696,     0,     0,     0,     0,   903,   906,
     171,     0,     0,     0,   340,   331,     0,     0,     0,   148,
     259,   261,     0,   330,     0,     0,     0,   884,   333,     0,
     864,   256,   152,   531,     0,     0,   472,   464,     0,   234,
     224,     0,   227,   233,   239,   452,   901,   333,   901,     0,
     846,     0,   803,   333,     0,   333,   260,   758,   801,   740,
     739,   733,     0,   735,   709,   744,    70,   204,    77,    82,
     102,   178,     0,   186,   192,    70,   194,   911,     0,     0,
     475,     0,   854,   853,   695,     0,    70,   127,     0,     0,
       0,     0,     0,     0,   172,     0,   884,     0,   297,   293,
     299,   628,    27,     0,   287,     0,   292,   296,   308,     0,
     306,   311,     0,   310,     0,   309,     0,   131,   263,     0,
     265,     0,   266,   267,     0,     0,   830,     0,   532,   530,
     541,   539,   235,     0,     0,   222,   232,     0,     0,     0,
       0,   144,   452,   901,   805,   150,   256,   154,   333,     0,
       0,   747,     0,   200,     0,     0,    70,   181,   101,   193,
     930,   713,     0,     0,     0,     0,   905,     0,     0,   358,
       0,     0,   277,   281,   355,   356,   291,     0,     0,     0,
     272,   592,   591,   588,   590,   589,   609,   611,   610,   580,
     551,   552,   570,   586,   585,   547,   557,   558,   560,   559,
     579,   563,   561,   562,   564,   565,   566,   567,   568,   569,
     571,   572,   573,   574,   575,   576,   578,   577,   548,   549,
     550,   553,   554,   556,   594,   595,   604,   603,   602,   601,
     600,   599,   587,   606,   596,   597,   598,   581,   582,   583,
     584,   607,   608,   612,   614,   613,   615,   616,   593,   618,
     617,   620,   622,   621,   555,   625,   623,   624,   619,   605,
     546,   303,   543,     0,   273,   324,   325,   323,   316,     0,
     317,   274,   350,     0,     0,     0,     0,   354,     0,   131,
     140,   255,     0,     0,     0,   223,   237,   802,     0,    70,
     326,    70,   134,     0,     0,     0,   146,   901,   738,     0,
      70,   179,    83,   103,     0,   474,   852,   510,   125,   275,
     276,   353,   173,     0,     0,     0,   300,   288,     0,     0,
       0,   305,   307,     0,     0,   312,   319,   320,   318,     0,
       0,   262,     0,     0,     0,   357,     0,   257,     0,   236,
       0,   526,   712,     0,     0,    70,   136,   142,     0,   746,
       0,     0,     0,   105,   278,    59,     0,   279,   280,     0,
       0,   294,     0,   298,   302,   544,   545,     0,   289,   321,
     322,   314,   315,   313,   351,   348,   268,   264,   352,     0,
     258,   527,   711,     0,   454,   327,     0,   138,     0,   182,
     511,     0,   129,     0,   333,     0,   301,   304,     0,   758,
     270,     0,   524,   451,   456,   180,     0,     0,   106,   285,
       0,   332,   295,   349,     0,   712,   344,   758,   525,     0,
     128,     0,     0,   284,   901,   758,   209,   345,   346,   347,
     933,   343,     0,     0,     0,   283,     0,   344,     0,   901,
       0,   282,   328,    70,   269,   933,     0,   214,   212,     0,
      70,     0,     0,   215,     0,     0,   210,   271,     0,   329,
       0,   218,   208,     0,   211,   217,   124,   219,     0,     0,
     206,   216,     0,   207,   221,   220
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   118,   813,   550,   180,   275,   504,
     508,   276,   505,   509,   120,   121,   122,   123,   124,   125,
     324,   580,   581,   457,   239,  1424,   463,  1342,  1425,  1662,
     769,   270,   499,  1623,   997,  1172,  1678,   340,   181,   582,
     851,  1054,  1223,   129,   553,   868,   583,   602,   870,   534,
     867,   584,   554,   869,   342,   293,   309,   132,   853,   816,
     799,  1012,  1363,  1106,   918,  1572,  1428,   714,   924,   462,
     723,   926,  1255,   706,   907,   910,  1095,  1684,  1685,   571,
     572,   596,   597,   280,   281,   287,  1397,  1551,  1552,  1179,
    1290,  1386,  1545,  1669,  1687,  1583,  1627,  1628,  1629,  1373,
    1374,  1375,  1376,  1585,  1591,  1638,  1379,  1380,  1384,  1538,
    1539,  1540,  1562,  1714,  1291,  1292,   182,   134,  1700,  1701,
    1543,  1294,  1295,  1296,  1297,   135,   232,   458,   459,   136,
     137,   138,   139,   140,   141,   142,   143,  1409,   144,   850,
    1053,   145,   236,   568,   318,   569,   570,   453,   559,   560,
    1129,   561,  1130,   146,   147,   148,   746,   149,   150,   267,
     151,   268,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   759,   760,   989,   496,   497,   498,   766,  1612,   152,
     555,  1399,   556,  1026,   821,  1196,  1193,  1531,  1532,   153,
     154,   155,   226,   233,   327,   445,   156,   945,   750,   157,
     946,   842,   835,   947,   894,  1074,   895,  1076,  1077,  1078,
     897,  1234,  1235,   898,   685,   429,   193,   194,   585,   574,
     410,   669,   670,   671,   672,   839,   159,   227,   184,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   633,   170,
     229,   230,   537,   218,   219,   636,   637,  1135,  1136,   302,
     303,   807,   171,   525,   172,   567,   173,  1553,   294,   335,
     591,   592,   939,  1036,   796,   797,   727,   728,   729,   260,
     261,   752,   262,   837
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1395
static const yytype_int16 yypact[] =
{
   -1395,   190, -1395, -1395,  5192, 13187, 13187,   -94, 13187, 13187,
   13187, 11137, 13187, -1395, 13187, 13187, 13187, 13187, 13187, 13187,
   13187, 13187, 13187, 13187, 13187, 13187, 15160, 15160, 11342, 13187,
   15211,   -90,   -13, -1395, -1395, -1395, -1395, -1395,   186, -1395,
   -1395,   150, 13187, -1395,   -13,    27,   106,   129,   -13, 11547,
   16256, 11752, -1395, 14363, 10112,   142, 13187, 15008,    16, -1395,
   -1395, -1395,   213,   -22,    83,   250,   254,   274,   277, -1395,
   16256,   304,   319, -1395, -1395, -1395, -1395, -1395,   465, 15305,
   -1395, -1395, 16256, -1395, -1395, -1395, -1395, 16256, -1395, 16256,
   -1395,   230,   321, 16256, 16256, -1395,   138, -1395, -1395, -1395,
   -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395,
   -1395, 13187, -1395, -1395,   258,   461,   500,   500, -1395,   267,
     314,   455, -1395,   339, -1395,    84, -1395,   522, -1395, -1395,
   -1395, -1395, 16077,    58, -1395, -1395,   360,   381,   386,   389,
     401,   419,  3957, -1395, -1395, -1395, -1395,   576, -1395,   578,
     593,   440, -1395,    55,   365,   497, -1395, -1395,  1139,    17,
     718,    65,   470,    56, -1395,    99,   100,   488,    57, -1395,
      19, -1395,   600, -1395, -1395, -1395,   523,   495,   535, -1395,
   -1395,   522,    58, 13929,  3477, 13929, 13187, 13929, 13929,  3743,
     505, 15576,  3743,   659, 16256,   644,   644,   490,   644,   644,
     644,   644,   644,   644,   644,   644,   644, -1395, -1395, 14886,
     550, -1395,   582,   468,   525,   468, 15160, 15620,   555,   778,
   -1395,   523,  3734,   550,   639,   648,   592,   131, -1395,   468,
      65, 11957, -1395, -1395, 13187,  8677,   790,    92, 13929,  9702,
   -1395, 13187, 13187, 16256, -1395, -1395,  4389,   599, -1395,  4474,
   14363, 14363,   640, -1395, -1395,   608, 14111,    62,   662,   799,
   -1395,   665, 16256,   741, -1395,   622,  4590,   623,   713, -1395,
     296,  4665, 16094, 16138, 16256,    94, -1395,    70, -1395, 14420,
      96, -1395,   700, -1395,   701, -1395,   821,    98, 15160, 15160,
   13187,   637,   669, -1395, -1395, 15023, 11342,   498,   469, -1395,
   13392, 15160,   474, -1395, 16256, -1395,   438,   314, -1395, -1395,
   -1395, -1395,  4575,   830,   751, -1395, -1395, -1395,    43,   651,
   13929,   652,  2953,   657,  5397, 13187,   358,   649,   503,   358,
     451,   356, -1395, 16256, 14363,   660, 10317, 14363, -1395, -1395,
   15391, -1395, -1395, -1395, -1395, -1395,   522, -1395, -1395, -1395,
   -1395, -1395, -1395, -1395, 13187, 13187, 13187, 12162, 13187, 13187,
   13187, 13187, 13187, 13187, 13187, 13187, 13187, 13187, 13187, 13187,
   13187, 13187, 13187, 13187, 13187, 13187, 13187, 13187, 13187, 15211,
   13187, -1395, 13187, 13187, -1395, 13187,  4372, 16256, 16256, 16256,
   16077,   755,   638,  9907, 13187, 13187, 13187, 13187, 13187, 13187,
   13187, 13187, 13187, 13187, 13187, 13187, -1395, -1395, -1395, -1395,
    4081, 13187, 13187, -1395, 10317, 10317, 13187, 13187,   258,   141,
   15023,   667,   522, 12367, 13782, -1395, 13187, -1395,   668,   854,
   14886,   672,    32,   657, 13455,   468, 12572, -1395, 12777, -1395,
     673,    60, -1395,   132, 10317, -1395, 15256, -1395, -1395, 13826,
   -1395, -1395, 10522, -1395, 13187, -1395,   785,  8882,   858,   676,
   16423,   863,    76,    42, -1395, -1395, -1395, -1395, -1395, 14363,
   15959,   680,   870, 14708, 16256, -1395,   696, -1395, -1395, -1395,
     803, 13187,   804,   805, 13187, 13187, 13187, -1395,   713, -1395,
   -1395, -1395, -1395, -1395, -1395, -1395,   698, -1395, -1395, -1395,
     688, -1395, -1395, 16256,   690,   883,   359, 16256,   703,   889,
     450,   452, 16152, -1395, 16256, 13187,   468,    16, -1395, -1395,
   -1395, 14708,   829, -1395,   468,    91,   108,   710,   711,  2984,
     217,   712,   716,   174,   774,   714,   468,   147,   719, 16034,
   16256, -1395, -1395,   856,  2170,   372, -1395, -1395, -1395,   314,
   -1395, -1395, -1395,   890,   796,   756,   373, -1395,   258,   794,
     917,   729,   781,   141, -1395, 14363, 14363,   915,   790,    43,
   -1395,   736,   924, -1395, 14363,   366,   887,    54, -1395, -1395,
   -1395, -1395, -1395, -1395, -1395,  1349,  2216, -1395, -1395, -1395,
   -1395,   927,   784, -1395, 15160, 13187,   757,   948, 13929,   944,
   -1395, -1395,   833, 15408, 10710, 11737,  3743, 13187, 16468, 12760,
    4086, 15207, 16585, 12140, 16615, 16615, 16615, 16615,   899,   899,
     899,   899,   780,   780,   726,   726,   726,   490,   490,   490,
   -1395,   644, 13929,   758,   760, 15676,   766,   951, -1395, 13187,
     287,   771,   141, -1395, -1395, -1395, -1395,   522, 13187, 14937,
   -1395, -1395,  3743, -1395,  3743,  3743,  3743,  3743,  3743,  3743,
    3743,  3743,  3743,  3743,  3743,  3743, -1395, 13187,   402,   143,
   -1395, -1395,   550,   462,   765,  2309,   772,   776,   782,  2441,
     149,   779, -1395, 13929, 14837, -1395, 16256, -1395,   651,   366,
     550, 15160, 13929, 15160, 15720,   366,   146, -1395,   777, 13187,
   -1395,   180, -1395, -1395, -1395,  8472,   564, -1395, -1395, 13929,
   13929,   -13, -1395, -1395, -1395, 13187,   886, 14558, 14708, 16256,
    9087,   788,   791, -1395,    69,   860,   843, -1395,   985,   797,
   14174, 14363, 14708, 14708, 14708, 14708, 14708, -1395,   800,    33,
     846,   801, 14708,   482, -1395,   848, -1395,   802, -1395, 16554,
   -1395,   268, -1395, 13187,   817, 13929,   818,   990, 11122,   998,
   -1395, 13929, 13870, -1395,   698,   930, -1395,  5602, 16020,   810,
     471, -1395, 16094, 16256,   476, -1395, 16138, 16256, 16256, -1395,
   -1395,  2575, -1395, 16554,  1000, 15160,   816, -1395, -1395, -1395,
   -1395, -1395, -1395, -1395, -1395, -1395,    77, 16256, 16020,   820,
   15023, 15109,  1003, -1395, -1395, -1395, -1395,   813, -1395, 13187,
   -1395, -1395,  4782, -1395, 14363, 16020,   837, -1395, -1395, -1395,
   -1395,  1005, 13187,  4575, -1395, -1395, 16195,   839, -1395, 14363,
   -1395,   844,  5807,  1012,    34, -1395, -1395,   391,  4081, -1395,
   -1395, 14363, -1395, -1395,   468, 13929, -1395, 10727, -1395, 14708,
      35,   845, 16020,   796, -1395, -1395, 12350, 13187, -1395, -1395,
   13187, -1395, 13187, -1395,  2715,   850, 10317,   774,  1025,   796,
     833, 16256, 15211,   468,  2759,   859, -1395, -1395,   185, -1395,
   -1395, -1395,  1044, 15898, 15898, 14837, -1395, -1395, -1395,  1014,
     862,   256,   872, -1395, -1395, -1395, -1395,  1063,   877,   668,
     468,   468, 12982, 15256, -1395, -1395,  3397,   583,   -13,  9702,
   -1395,  6012,   878,  6217,   879, 14558, 15160,   884,   947,   468,
   16554,  1070, -1395, -1395, -1395, -1395,   597, -1395,   350, 14363,
   -1395, 14363, 16256, 15959, -1395, -1395, -1395,  1077, -1395,   888,
     927,   630,   630,  1023,  1023, 15820,   885,  1082, 14708,   946,
   16256,  4575,  2864, 16237, 14708, 14708, 14708, 14708, 14512, 14708,
   14708, 14708, 14708, 14708, 14708, 14708, 14708, 14708, 14708, 14708,
   14708, 14708, 14708, 14708, 14708, 14708, 14708, 14708, 14708, 14708,
   14708, 16256, -1395, 13929, 13187, 13187, 13187, -1395, -1395, -1395,
   13187, 13187, -1395,   713, -1395,  1018, -1395, -1395, 16256, -1395,
   -1395, 16256, -1395, -1395, -1395, -1395, 14708,   468, -1395,   174,
   -1395,   996,  1089, -1395, -1395,   177,   901,   468, 10932, -1395,
     104, -1395,  4987,   751,  1089, -1395,   328,    13, 13929,   970,
   -1395, -1395,   902,  1012, -1395, 14363,   790, 14363,    50,  1091,
    1024,   239, -1395,   550, -1395, 15160, 13187, 13929, 16554,   908,
      35, -1395,   907,    35,   911, 12350, 13929, 15776,   912, 10317,
     913,   914, 14363,   921,   796, -1395,   592,   463, 10317, 13187,
   -1395, -1395, -1395, -1395, -1395, -1395,   983,   918,  1105,  1032,
   14837,   978, -1395,  4575, 14837, -1395, -1395, -1395, 15160, 13929,
     270, -1395, -1395,   -13,  1095,  1053,  9702, -1395, -1395, -1395,
     928, 13187,   947,   468, 15023, 14558,   931, 14708,  6422,   658,
     929, 13187,    47,   390, -1395,   960, -1395,  1002, -1395, 14237,
    1108,   940, 14708, -1395, 14708, -1395,   941, -1395,  1011,  1134,
     955, -1395, -1395, -1395, 15876,   943,  1137, 11325, 11942, 10300,
   14708, 16512, 12965, 14688, 15481,  3029,  3798, 16028, 16028, 16028,
   16028,  2474,  2474,  2474,  2474,   984,   984,   630,   630,   630,
    1023,  1023,  1023,  1023, -1395, 13929, 13377, 13929, -1395, 13929,
   -1395,   956, -1395, -1395, -1395, 16554, 16256, 14363, 16020,    85,
   -1395, 15023, -1395, -1395,  3743,   945, -1395,   957,   909, -1395,
      24, 13187, -1395, -1395, -1395, 13187, -1395, 13187, -1395,   790,
   -1395, -1395,   395,  1142,  1078, 13187, -1395,   963,   468, 13929,
    1012,   964, -1395,   966,    35, 13187, 10317,   967, -1395, -1395,
     751, -1395,   961,   971, -1395,   976, 14837, -1395, 14837, -1395,
   -1395,   977, -1395,  1040,   980,  1172, -1395,   468,  1153, -1395,
     988, -1395, -1395,   986,   992,   183, -1395, -1395, 16554,   989,
     993, -1395,  4162, -1395, -1395, -1395, -1395, -1395, 14363, -1395,
   14363, -1395, 16554, 15920, -1395, 14708,  4575, -1395, -1395, 14708,
   -1395, 14708, -1395, 12555, 14708, 13187,   995,  6627,  1087, -1395,
   -1395,   574, 14300, 16020,  1088, -1395,  3060,  1039, 14748, -1395,
   -1395, -1395,   755, 13993,   102,   110,  1004,   751,   638,   184,
   -1395, -1395, -1395,  1045,  3539,  3631, 13929, -1395,   259,  1184,
    1123, 13187, -1395, 13929, 10317,  1098,  1012,  1714,  1012,  1007,
   13929,  1013, -1395,  1748,  1015,  1926, -1395,    35, -1395, -1395,
    1084, -1395, 14837, -1395,  4575, -1395, -1395,  8472, -1395, -1395,
   -1395, -1395,  9292, -1395, -1395, -1395,  8472, -1395,  1016, 14708,
   16554,  1090, 16554, 15976, 12555, 13172, -1395, -1395, 14363, 16020,
   16020, 16256,  1204,    88, -1395, 14300,   751, 15975, -1395,  1049,
   -1395,   115,  1020,   116, -1395, 13597, -1395, -1395, -1395,   118,
   -1395, -1395, 15351, -1395,  1022, -1395,  1144,   522, -1395, 14363,
   -1395, 14363, -1395, -1395,  1210,   755, -1395, 13867, -1395, -1395,
   -1395, -1395,  1211,  1148, 13187, -1395, 13929,  1031,  1034,  1033,
     513, -1395,  1098,  1012, -1395, -1395, -1395, -1395,  1973,  1036,
   14837, -1395,  1103,  8472,  9497,  9292, -1395, -1395, -1395,  8472,
   -1395, 16554, 14708, 14708, 13187,  6832, -1395,  1037,  1038, -1395,
   14708, 16020, -1395, -1395, -1395, -1395, -1395, 14363,   999,  3060,
   -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395,
   -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395,
   -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395,
   -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395,
   -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395,
   -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395,
   -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395,
   -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395,
   -1395,   547, -1395,  1039, -1395, -1395, -1395, -1395, -1395,   175,
     636, -1395,  1222,   119, 16256,  1144,  1225, -1395, 14363,   522,
   -1395, -1395,  1041,  1226, 13187, -1395, 13929, -1395,   148, -1395,
   -1395, -1395, -1395,  1043,   513, 13930, -1395,  1012, -1395, 14837,
   -1395, -1395, -1395, -1395,  7037, 16554, 16554, 11532, -1395, -1395,
   -1395, 16554, -1395, 14616,    82,    66, -1395, -1395, 14708, 13597,
   13597,  1187, -1395, 15351, 15351,   641, -1395, -1395, -1395, 14708,
    1166, -1395,  1050,   123, 14708, -1395, 16256, -1395, 14708, 13929,
    1168, -1395,  1240,  7242,  7447, -1395, -1395, -1395,   513, -1395,
    7652,  1051,  1171,  1152, -1395,  1165,  1116, -1395, -1395,  1169,
   14363, -1395,   999, -1395, 16554, -1395, -1395,  1106, -1395,  1235,
   -1395, -1395, -1395, -1395, 16554,  1255, -1395, -1395, 16554,  1074,
   16554, -1395,   399,  1075, -1395, -1395,  7857, -1395,  1076, -1395,
   -1395,  1083,  1113, 16256,   638,  1104, -1395, -1395, 14708,    36,
   -1395,  1203, -1395, -1395, -1395, -1395, 16020,   810, -1395,  1119,
   16256,   612, -1395, 16554,  1093,  1278,   653,    36, -1395,  1215,
   -1395, 16020,  1096, -1395,  1012,    51, -1395, -1395, -1395, -1395,
   14363, -1395,  1099,  1109,   125, -1395,   546,   653,   408,  1012,
    1094, -1395, -1395, -1395, -1395, 14363,   261,  1276,  1223,   546,
   -1395,  8062,   420,  1289,  1232, 13187, -1395, -1395,  8267, -1395,
     266,  1291,  1233, 13187, -1395, 13929, -1395,  1299,  1236, 13187,
   -1395, 13929, 13187, -1395, 13929, 13929
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1395, -1395, -1395,  -489, -1395, -1395, -1395,   375, -1395, -1395,
   -1395,   806,   544,   541,     8,  1597,  3138, -1395,  2789, -1395,
      71, -1395,    25, -1395, -1395, -1395, -1395, -1395, -1395, -1395,
   -1395, -1395, -1395, -1395,  -357, -1395, -1395,  -159,    95,    28,
   -1395, -1395, -1395, -1395, -1395, -1395,    29, -1395, -1395, -1395,
   -1395,    31, -1395, -1395,   932,   934,   935,  -108,   453,  -785,
     464,   514,  -361,   231,  -802, -1395,   -89, -1395, -1395, -1395,
   -1395,  -649,    87, -1395, -1395, -1395, -1395,  -350, -1395,  -522,
   -1395,  -352, -1395, -1395,   824, -1395,   -81, -1395, -1395,  -913,
   -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395,
    -107, -1395,   -23, -1395, -1395, -1395, -1395,  -188, -1395,    64,
    -844, -1395, -1394,  -373, -1395,  -131,   269,  -125,  -356, -1395,
    -195, -1395, -1395, -1395,    73,   -45,    11,  1311,  -659,  -351,
   -1395, -1395,    -6, -1395, -1395,    -5,   -41,   -58, -1395, -1395,
   -1395, -1395, -1395, -1395, -1395, -1395, -1395,  -528,  -769, -1395,
   -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395,   962,
   -1395, -1395,   367, -1395,   871, -1395, -1395, -1395, -1395, -1395,
   -1395, -1395,   368, -1395,   874, -1395, -1395,   602, -1395,   337,
   -1395, -1395, -1395, -1395, -1395, -1395, -1395, -1395,  -816, -1395,
    2366,   244,  -334, -1395, -1395,   298,  2421,  3341, -1395, -1395,
     422,    -9,  -566, -1395, -1395,   484,   292,  -648,   294, -1395,
   -1395, -1395, -1395, -1395,   479, -1395, -1395, -1395,   155,  -796,
    -171,  -394,  -377, -1395,   537,  -120, -1395, -1395,   580, -1395,
   -1395,  1359,   -29, -1395, -1395,    79,  -117, -1395,   247, -1395,
   -1395, -1395,  -386,  1086, -1395, -1395, -1395, -1395, -1395,   654,
     347, -1395, -1395,  1100,  -282,  -963, -1395,   -51,   -68,  -156,
      63,   661, -1395, -1005, -1395,   370,   460, -1395, -1395, -1395,
   -1395,   403,   242,  -998
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -918
static const yytype_int16 yytable[] =
{
     183,   185,   259,   187,   188,   189,   191,   192,   347,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   310,   421,   217,   220,   313,   314,   391,  1199,   126,
     564,   848,   128,   130,   680,   131,   896,   238,  1037,  1202,
     830,   413,   650,   235,   246,   630,   249,   831,   265,   266,
     390,   271,   701,   419,  1029,   240,   914,   347,   446,   244,
    1187,   812,   676,   677,   343,   277,   319,   442,  1052,   702,
     321,  -888,   721,   928,  1303,  1632,  -888,  1253,   929,    13,
      13,    13,   323,  -790,  1063,   719,  1009,   306,    13,  1281,
     307,  1630,   698,   337,   284,   278,    13,  1441,   207,   127,
     785,   454,   285,   512,   186,   517,   320,   522,   231,   228,
     447,  1389,  1188,  1102,   354,   355,   356,   785,   949,  1391,
    -794,  -787,  -489,   416,  -290,  1449,   540,  1533,  1600,  1203,
      13,   411,  1600,   357,  1441,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   801,   379,   801,   158,
     408,   409,   286,    52,   431,  -788,  -789,   514,  1194,   380,
    1616,    59,    60,    61,   174,   175,   344,   440,   311,   299,
     433,   424,  1128,  1610,  1593,   234,   801,   603,   408,   409,
       3,  1282,   801,   801,  -457,   563,  1283,  -824,    59,    60,
      61,   174,  1284,   344,  1285,  1315,  -631,   416,  1594,  -791,
     439,   207,  -827,  1204,  1195,  -796,  -490,   315,  -790,   279,
     794,   795,   418,   333,  1657,   241,   449,  1611,   346,   449,
    -711,   577,  -630,  -711,  -229,  -213,   238,   460,   345,  -229,
     930,  1286,  1287,   722,  1288,   527,  -825,  1254,  1010,   528,
    -711,  -793,  -797,  1631,   415,  -794,  -787,  1324,   417,   696,
     451,   531,  -888,   681,   456,   345,   412,   347,  1633,   642,
     212,   212,   601,   133,   225,   720,  1109,   422,  1113,  1222,
     506,   510,   511,  -489,   338,   320,   599,  1289,  1442,  1443,
     786,   217,   455,   642,   513,   544,   518,   332,   523,  1317,
    -788,  -789,  1390,  1246,   242,  -792,  1323,   787,  1325,  1185,
    1392,  1410,   549,  1412,  1233,  -290,  1450,   642,  1534,  1601,
     586,   310,   343,  1647,  1022,  1711,   642,   243,  1049,   642,
     282,   598,  -824,   316,  1394,   114,  -826,   283,  1402,   317,
    1723,  1081,   417,   269,  -791,  1737,   802,  -827,   882,   604,
     605,   606,   608,   609,   610,   611,   612,   613,   614,   615,
     616,   617,   618,   619,   620,   621,   622,   623,   624,   625,
     626,   627,   628,   629,  1595,   631,  1180,   632,   632,   119,
     635,  -825,  1341,  1396,   311,   688,  -793,   651,   652,   654,
     655,   656,   657,   658,   659,   660,   661,   662,   663,   664,
     665,   433,  1082,  1445,   838,   500,   632,   675,  1564,   598,
     598,   632,   679,  1418,  1015,   415,   791,   981,   652,   127,
     687,   683,  1403,   818,  1724,   247,  1038,   392,   257,  1738,
    1308,   692,   411,   694,  1671,   297,  1236,   297,   334,   598,
    -792,   546,  1243,  1716,  1041,   292,   708,   709,   288,   710,
    1111,  1112,   289,   212,   391,  1730,   773,   334,   228,   325,
     212,  1042,   308,   501,   292,   641,   212,   333,   292,   292,
    1039,  -826,   290,   573,  1309,   291,   755,   390,  1672,   758,
     761,   762,   713,  1189,   865,  -893,   647,  1717,   871,   673,
    1111,  1112,   467,   468,   408,   409,  1190,  1351,   472,  1731,
     408,   409,   295,   300,   301,   300,   301,   292,  1201,  1090,
     781,   770,   333,   641,  1060,   774,   875,   296,   838,   312,
     277,   705,   697,   212,   903,   703,  1091,   819,  1211,  1191,
     297,  1213,   212,   212,   865,   855,   546,   336,  1066,   212,
     297,   564,   820,   379,   297,   212,   326,   777,   297,   778,
     298,   824,  1114,   297,  1040,   380,   562,   339,  1310,   546,
     348,  1588,  1618,   589,   539,  1422,   385,   412,   998,   428,
     767,  1718,  -766,  1001,   811,   904,   593,  1589,  1329,   297,
    1330,   349,   297,  1732,   160,   329,   350,   442,   546,   351,
     845,   333,  1256,   133,  1590,   982,   408,   409,   300,   301,
    -766,   352,   856,   333,  -893,   333,   213,   215,   300,   301,
     119,   299,   300,   301,   119,   541,   300,   301,   461,   353,
     547,   300,   301,   225,   333,   334,   408,   409,   863,   333,
     634,  -893,  -769,  -767,   864,   908,   909,   475,   551,   552,
     384,   832,  -487,   191,   382,   542,   386,   300,   301,   548,
     300,   301,   334,  -893,  1093,  1094,  -893,   588,   674,   383,
    -769,  -767,   874,   678,   212,  1596,  -488,  1182,   414,   564,
    1641,  1307,  -630,   542,   212,   548,   542,   548,   548,   977,
     978,   979,  1597,  -893,  1421,  1598,  -795,  1642,   304,  1706,
    1643,   322,  1319,   420,   906,   980,  1110,  1111,  1112,   119,
    1359,  1360,  1708,   427,  1719,   425,   563,  1217,   590,   380,
     238,   724,   257,  1560,  1561,   292,  1225,  1722,  1245,   934,
     334,   642,   912,   415,   573,    59,    60,    61,   174,   175,
     344,   434,   393,   394,   395,   396,   397,   398,   399,   400,
     401,   402,   403,   404,   405,   923,  1712,  1713,   983,  1639,
    1640,    59,    60,    61,   174,   175,   344,  1250,  1111,  1112,
     437,   640,   292,   644,   292,   292,  1697,  1698,  1699,   328,
     330,   331,  1568,  1635,  1636,   376,   377,   378,   911,   379,
     506,   406,   407,   913,   510,   668,   642,   438,  -629,   432,
     444,   380,   345,   937,   940,  1299,   435,   443,   452,   465,
     127,   564,   441,  1065,  1020,  1419,   470,   827,   828,   690,
     469,  -917,  1693,   473,   474,   160,   836,  1028,   345,   160,
     476,   700,   477,   479,   519,   520,   373,   374,   375,   376,
     377,   378,   119,   379,   563,   521,   532,   126,   212,   533,
     128,   130,  1047,   131,   565,   380,   408,   409,   566,   751,
     575,   576,  1055,   587,   392,  1056,   578,  1057,   -65,   516,
      52,   598,   127,   686,  1321,   600,   684,   454,   524,   524,
     529,   689,   695,  1686,   711,   536,   715,   718,   730,   731,
     753,   545,   754,   756,   757,   765,   768,  1207,  1085,   780,
     771,  1686,   772,   212,   480,   481,   482,  1089,   776,  1707,
     798,   483,   484,   775,   160,   485,   486,   127,   784,   788,
     789,   792,   800,  1281,   806,   808,   793,   673,   803,  1096,
     814,  1619,   809,   815,   822,   817,   823,   127,   825,   829,
     826,  1121,   833,   834,  1097,   212,   841,   212,  1125,  -918,
    -918,  -918,  -918,   371,   372,   373,   374,   375,   376,   377,
     378,   228,   379,  -491,    13,   843,   846,   847,   849,   852,
     862,   212,  1407,   858,   380,   859,   563,   158,   861,   866,
     876,   878,   593,   593,   133,   879,   905,   854,   292,  1165,
    1166,  1167,   703,   915,   564,   758,  1169,   880,   925,   573,
     931,   927,   932,  1108,   933,   950,   935,   953,   948,   951,
     536,   984,   985,   954,   986,   573,   127,   990,   127,   993,
     432,   996,  1183,  1184,  1006,  1282,  1008,  1018,  1019,  1027,
    1283,  1014,    59,    60,    61,   174,  1284,   344,  1285,   212,
     974,   975,   976,   977,   978,   979,   133,   160,  1025,  1031,
    1033,  1209,  1035,  1050,   212,   212,  1653,   126,  1059,   980,
     128,   130,   564,   131,   598,  1062,  1023,  1068,  1069,   893,
    1080,   899,  1079,   598,  1184,  1286,  1287,   562,  1288,  1229,
    1083,  1032,  1084,    33,    34,    35,  1086,  1105,  1099,  1101,
     119,   133,  1104,  1044,  1107,   737,  1119,  1120,   980,   345,
    1123,  1124,   541,  1177,   921,   119,   238,  1171,  1178,  1181,
    1197,   133,  1198,  1206,  1238,  1205,  1252,  1210,  1212,  1214,
    1216,  1302,  1218,  1226,  1228,  1219,   225,   127,   889,  1696,
    1267,  1241,  1221,  1227,  1232,  1239,  1240,  1272,  1242,  1251,
    1247,  1257,  1258,    73,    74,    75,    76,    77,  1260,  1261,
    1264,  1265,   119,  1266,   739,  1270,  1271,  1300,  1000,   563,
      80,    81,  1003,  1004,  1268,  1276,  1311,  1312,  1301,   212,
     212,  1314,  1326,  1316,    90,  1318,  1322,  1395,   347,  1327,
    1332,  1115,  1011,  1116,   844,  1328,  1331,   158,    95,  1333,
     133,  1334,   133,  1336,  1358,  1339,  1304,   119,  1338,  1343,
    1305,  1340,  1306,  1344,  1365,   562,  1356,  1378,  1404,  1398,
    1313,  1030,  1405,   127,  1393,   573,  1413,   119,   573,  1408,
    1320,   598,  1414,   668,  1420,  1430,  1416,   563,  1440,  1447,
    1432,  1448,  1541,  1542,  1548,  1554,  1335,  1555,  1544,   873,
    1557,  1362,  1558,  1569,  1559,  1567,  1599,  1579,  1580,  1604,
    1608,  1607,  1637,  1277,  1615,  1645,   292,  1651,  1646,  1652,
    1660,  1659,    59,    60,    61,    62,    63,   344,  1073,  1073,
     893,  1661,  -286,    69,   387,  1663,  1664,  1667,  1594,  1668,
    1355,   900,  1670,   901,  1673,  1682,  1675,  1200,   700,   836,
    1677,  1676,  1688,  1691,   119,   160,   119,  1695,   119,   212,
    1725,   133,  1694,  1439,  1703,  1720,  1705,   919,  1709,   388,
     160,   389,  1726,  1733,  1220,  1739,  1406,  1117,  1710,   598,
    1337,  1734,  1740,  1742,  1444,  1743,   999,  1002,   779,   345,
    1690,   643,   646,  1064,   645,  1127,  1346,   562,  1133,  1024,
    1704,  1061,   212,  1244,  1298,  1565,  1573,  1702,  1546,  1345,
    1547,   782,  1587,  1298,  1446,  1592,  1727,   160,   212,   212,
    1603,  1715,  1385,   237,  1563,   653,   751,  1366,  1168,   763,
    1170,  1044,   764,  1192,  1224,  1007,   992,  1427,  1075,   573,
    1126,  1230,   127,  1173,  1231,  1043,  1174,   133,  1087,  1176,
     536,  1017,   538,     0,  1164,   214,   214,  1387,     0,   526,
    1606,   938,   160,  1118,     0,     0,     0,   119,     0,  1556,
       0,     0,     0,     0,     0,     0,     0,  1423,     0,     0,
       0,     0,   160,     0,     0,     0,  1429,     0,     0,  1279,
       0,     0,     0,     0,     0,   212,     0,  1435,     0,  1577,
       0,     0,   127,     0,     0,     0,     0,     0,     0,     0,
       0,   127,     0,     0,     0,     0,     0,     0,  1293,  1571,
    1427,     0,     0,     0,     0,   893,     0,  1293,     0,   893,
       0,     0,    59,    60,    61,    62,    63,   344,     0,     0,
       0,   119,  1298,    69,   387,     0,  1602,     0,  1298,     0,
    1298,     0,   573,   119,     0,     0,     0,     0,     0,   160,
    1549,   160,     0,   160,     0,   919,  1103,  1574,     0,     0,
    1347,     0,  1348,     0,     0,     0,     0,     0,     0,     0,
     562,   389,     0,     0,     0,     0,     0,     0,   127,     0,
       0,     0,     0,     0,   127,     0,     0,     0,     0,   345,
     127,     0,     0,  1680,     0,  1388,     0,     0,  1649,     0,
       0,     0,     0,     0,     0,     0,   133,     0,     0,  1609,
       0,  1278,     0,     0,     0,     0,   347,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   392,   214,     0,
       0,     0,     0,  1298,     0,   214,     0,     0,   562,     0,
       0,   214,     0,     0,     0,     0,  1293,     0,     0,     0,
       0,     0,  1293,     0,  1293,     0,     0,     0,     0,     0,
    1436,   893,   160,   893,     0,     0,   133,     0,     0,     0,
       0,     0,     0,     0,     0,   133,     0,     0,     0,     0,
       0,     0,     0,   210,   210,  1208,     0,   223,     0,     0,
    1613,     0,  1614,     0,     0,     0,     0,     0,   214,     0,
       0,  1620,     0,     0,     0,     0,     0,   214,   214,   530,
     223,     0,   119,     0,   214,     0,     0,   257,     0,     0,
     214,     0,     0,  1383,     0,     0,     0,     0,  1237,   127,
       0,     0,     0,     0,     0,     0,   160,     0,     0,     0,
       0,     0,     0,     0,   536,   919,  1656,  1293,   160,  1584,
       0,     0,   133,     0,     0,     0,     0,     0,   133,     0,
       0,     0,     0,     0,   133,     0,     0,   893,   127,   127,
       0,     0,   119,     0,     0,   127,     0,   119,  1281,     0,
    1735,   119,     0,     0,     0,     0,     0,     0,  1741,     0,
       0,     0,     0,     0,  1744,     0,   292,  1745,     0,     0,
     257,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1530,   127,  1281,     0,     0,     0,     0,  1537,     0,    13,
       0,   536,     0,     0,   257,     0,   257,     0,     0,     0,
       0,     0,   257,     0,     0,     0,     0,     0,     0,   214,
       0,     0,     0,     0,  1721,     0,     0,     0,     0,   214,
    1605,  1728,     0,    13,     0,   893,     0,     0,   119,   119,
     119,     0,     0,     0,   119,     0,   210,     0,     0,     0,
     119,     0,     0,   210,     0,     0,   127,     0,     0,   210,
    1282,     0,     0,   127,   573,  1283,     0,    59,    60,    61,
     174,  1284,   344,  1285,     0,     0,     0,     0,     0,     0,
       0,     0,   573,   133,     0,     0,     0,   223,   223,     0,
     573,     0,     0,   223,  1282,     0,     0,   160,     0,  1283,
       0,    59,    60,    61,   174,  1284,   344,  1285,     0,     0,
    1286,  1287,  1665,  1288,     0,     0,   210,     0,     0,     0,
       0,     0,   133,   133,     0,   210,   210,     0,     0,   133,
       0,     0,   210,     0,   345,     0,     0,     0,   210,     0,
       0,     0,     0,     0,  1286,  1287,     0,  1288,     0,   223,
       0,     0,     0,     0,     0,     0,  1411,   160,     0,   292,
       0,     0,   160,     0,     0,   133,   160,     0,   345,     0,
    1281,   223,     0,  1681,   223,     0,     0,     0,     0,     0,
     257,     0,   836,     0,   893,     0,     0,     0,     0,   119,
    1415,     0,     0,   214,     0,     0,     0,   836,  1625,     0,
       0,     0,     0,     0,  1530,  1530,     0,     0,  1537,  1537,
       0,    13,     0,     0,     0,     0,   223,  1281,     0,     0,
       0,   292,     0,     0,     0,     0,     0,     0,   119,   119,
     133,     0,     0,     0,     0,   119,     0,   133,     0,     0,
       0,     0,     0,   160,   160,   160,     0,     0,   214,   160,
       0,     0,     0,     0,     0,   160,     0,   210,    13,     0,
       0,     0,     0,     0,     0,     0,     0,   210,     0,     0,
       0,   119,  1282,     0,     0,     0,     0,  1283,  1679,    59,
      60,    61,   174,  1284,   344,  1285,     0,     0,     0,     0,
     214,     0,   214,     0,     0,  1692,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   223,   223,     0,     0,
     743,     0,     0,     0,     0,     0,   214,     0,     0,  1282,
       0,     0,  1286,  1287,  1283,  1288,    59,    60,    61,   174,
    1284,   344,  1285,     0,     0,     0,   119,     0,     0,     0,
       0,     0,     0,   119,     0,     0,   345,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   743,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1417,  1286,
    1287,     0,  1288,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   214,     0,     0,     0,     0,     0,
       0,     0,     0,   345,   160,     0,     0,     0,     0,   214,
     214,     0,   223,   223,     0,     0,     0,     0,     0,     0,
       0,   223,     0,     0,     0,  1566,     0,     0,     0,     0,
     354,   355,   356,     0,     0,     0,     0,     0,     0,     0,
       0,   210,     0,   160,   160,     0,     0,     0,     0,   357,
     160,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,     0,   379,     0,     0,   354,   355,   356,     0,
       0,     0,     0,     0,     0,   380,   160,     0,     0,     0,
       0,     0,     0,     0,     0,   357,   210,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,     0,   379,
       0,     0,     0,     0,   214,   214,     0,     0,     0,     0,
       0,   380,     0,     0,     0,     0,     0,     0,   210,     0,
     210,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   160,     0,     0,     0,     0,     0,     0,   160,     0,
       0,     0,     0,     0,   210,   743,     0,     0,     0,   354,
     355,   356,     0,     0,     0,     0,     0,   223,   223,   743,
     743,   743,   743,   743,     0,     0,     0,     0,   357,   743,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,     0,   379,     0,     0,   223,     0,     0,     0,     0,
       0,     0,   810,     0,   380,     0,     0,     0,     0,     0,
       0,     0,   210,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   211,   211,     0,   223,   224,   210,   210,     0,
       0,     0,     0,     0,   214,     0,     0,     0,     0,     0,
       0,   223,   223,     0,     0,     0,     0,     0,   840,     0,
     223,     0,     0,     0,     0,     0,   223,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   223,     0,
       0,     0,     0,     0,     0,     0,   743,   214,     0,   223,
       0,   354,   355,   356,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   214,   214,     0,     0,     0,     0,   223,
     357,     0,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,     0,   379,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   380,     0,     0,     0,
       0,   877,   210,   210,  -918,  -918,  -918,  -918,   972,   973,
     974,   975,   976,   977,   978,   979,   223,     0,   223,     0,
     223,     0,     0,     0,     0,     0,     0,     0,     0,   980,
     214,     0,     0,     0,     0,   743,     0,     0,   223,     0,
       0,   743,   743,   743,   743,   743,   743,   743,   743,   743,
     743,   743,   743,   743,   743,   743,   743,   743,   743,   743,
     743,   743,   743,   743,   743,   743,   743,   743,     0,     0,
       0,     0,   211,     0,     0,   354,   355,   356,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   743,   357,     0,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,     0,   379,     0,
       0,     0,   223,     0,   223,     0,     0,     0,     0,     0,
     380,     0,   210,   881,     0,   211,     0,     0,     0,     0,
       0,     0,     0,     0,   211,   211,     0,     0,     0,   223,
       0,   211,     0,     0,     0,     0,     0,   211,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   211,     0,
     223,     0,     0,     0,     0,   210,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   210,   210,     0,   743,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   223,     0,     0,   743,
       0,   743,     0,     0,     0,   354,   355,   356,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   743,     0,     0,
       0,     0,     0,     0,   357,   224,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,     0,   379,   354,
     355,   356,     0,     0,   223,   223,     0,  1005,   210,     0,
     380,     0,     0,     0,     0,     0,   211,     0,   357,     0,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,     0,   379,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   380,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   747,
       0,     0,   258,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   223,     0,   223,     0,     0,
       0,     0,   743,   223,     0,     0,   743,     0,   743,     0,
       0,   743,     0,     0,   955,   956,   957,     0,     0,   223,
     223,     0,     0,   223,     0,     0,     0,   747,     0,     0,
     223,     0,     0,   958,   748,   959,   960,   961,   962,   963,
     964,   965,   966,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,   978,   979,     0,  1058,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   980,
       0,   223,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   748,     0,     0,     0,   743,     0,     0,     0,
       0,     0,     0,     0,     0,   223,   223,   223,     0,     0,
     211,  1067,   223,     0,   223,     0,     0,   423,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,   405,
       0,     0,     0,     0,     0,     0,   223,     0,   223,     0,
       0,     0,     0,     0,   223,     0,     0,     0,   423,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,     0,     0,     0,     0,   211,   406,   407,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   743,
     743,     0,     0,     0,     0,     0,     0,   743,   223,   258,
     258,     0,     0,     0,   223,   258,   223,   406,   407,     0,
       0,     0,     0,     0,     0,     0,     0,   211,     0,   211,
       0,     0,     0,  1131,   963,   964,   965,   966,   967,   968,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   408,   409,   211,   747,     0,     0,     0,     0,  1367,
       0,     0,     0,     0,   980,     0,     0,     0,   747,   747,
     747,   747,   747,     0,     0,     0,     0,     0,   747,     0,
       0,     0,   408,   409,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   258,     0,     0,   258,     0,     0,     0,
       0,     0,     0,     0,   995,     0,     0,    36,     0,   748,
       0,     0,     0,     0,     0,   223,     0,     0,     0,     0,
       0,   211,   577,   748,   748,   748,   748,   748,     0,     0,
       0,     0,   223,   748,  1013,     0,   211,   211,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1368,
     223,  1013,     0,   790,     0,   743,     0,     0,     0,   211,
       0,     0,  1369,  1370,     0,     0,   743,     0,     0,     0,
       0,   743,     0,     0,     0,   743,     0,     0,     0,     0,
     178,     0,     0,    82,  1371,   747,    84,    85,  1051,    86,
    1372,    88,     0,     0,     0,     0,     0,   223,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   224,     0,
       0,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,     0,   258,   726,
       0,     0,   745,     0,     0,   743,     0,     0,     0,     0,
     748,     0,     0,   223,     0,     0,     0,     0,     0,     0,
       0,   211,   211,     0,     0,     0,     0,     0,   223,     0,
       0,     0,     0,     0,     0,     0,     0,   223,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     745,     0,   223,     0,   747,     0,     0,   211,     0,     0,
     747,   747,   747,   747,   747,   747,   747,   747,   747,   747,
     747,   747,   747,   747,   747,   747,   747,   747,   747,   747,
     747,   747,   747,   747,   747,   747,   747,     0,     0,     0,
       0,     0,     0,     0,   258,   258,     0,     0,     0,     0,
       0,     0,     0,   258,     0,     0,     0,     0,     0,   748,
       0,     0,   747,     0,     0,   748,   748,   748,   748,   748,
     748,   748,   748,   748,   748,   748,   748,   748,   748,   748,
     748,   748,   748,   748,   748,   748,   748,   748,   748,   748,
     748,   748,     0,     0,     0,     0,     0,   354,   355,   356,
       0,   211,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   357,   748,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   211,
     379,     0,     0,     0,   211,     0,     0,     0,     0,     0,
       0,     0,   380,     0,     0,     0,     0,     0,     0,     0,
     211,   211,     0,   747,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   747,     0,
     747,   423,   394,   395,   396,   397,   398,   399,   400,   401,
     402,   403,   404,   405,     0,     0,   747,   745,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   258,
     258,   745,   745,   745,   745,   745,     0,     0,   748,     0,
       0,   745,     0,     0,     0,     0,     0,     0,     0,     0,
     406,   407,     0,   748,  1280,   748,     0,   211,     0,   354,
     355,   356,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   748,     0,     0,     0,     0,     0,     0,   357,     0,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,     0,   379,     0,     0,     0,     0,     0,     0,  1092,
       0,     0,     0,   258,   380,   408,   409,     0,     0,     0,
       0,   744,     0,     0,     0,     0,     0,     0,   258,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     258,   747,   211,     0,     0,   747,     0,   747,   745,     0,
     747,   354,   355,   356,     0,     0,     0,     0,     0,  1364,
       0,     0,  1377,     0,     0,     0,     0,     0,     0,   744,
     357,     0,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,     0,   379,     0,   748,     0,     0,     0,
     748,     0,   748,     0,     0,   748,   380,     0,     0,     0,
     211,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   747,     0,     0,   258,     0,
     258,     0,   726,     0,     0,  1437,  1438,     0,     0,     0,
       0,     0,     0,  1377,     0,     0,     0,   745,     0,     0,
       0,  1400,     0,   745,   745,   745,   745,   745,   745,   745,
     745,   745,   745,   745,   745,   745,   745,   745,   745,   745,
     745,   745,   745,   745,   745,   745,   745,   745,   745,   745,
     748,     0,   357,     0,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   745,   379,     0,   747,   747,
       0,    29,    30,     0,     0,     0,   747,  1582,   380,     0,
       0,    36,     0,   207,   749,  1377,     0,     0,     0,     0,
       0,     0,     0,     0,   258,     0,   258,     0,     0,     0,
       0,     0,     0,  1401,   964,   965,   966,   967,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   977,   978,   979,
       0,   258,   208,   748,   748,     0,   744,     0,     0,     0,
       0,   748,   783,   980,     0,     0,     0,     0,     0,  1586,
     744,   744,   744,   744,   744,     0,     0,     0,     0,     0,
     744,     0,     0,     0,   178,     0,     0,    82,    83,     0,
      84,    85,     0,    86,   179,    88,   745,     0,     0,     0,
       0,     0,    91,     0,     0,     0,     0,     0,   258,     0,
       0,   745,     0,   745,     0,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   745,
       0,     0,   430,     0,     0,     0,     0,   114,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   747,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   747,   258,   354,   355,   356,
     747,     0,     0,     0,   747,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   357,   744,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   748,
     379,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     748,     0,   380,     0,     0,   748,     0,     0,     0,   748,
       0,     0,     0,     0,   747,     0,     0,     0,     0,     0,
       0,     0,  1689,     0,     0,     0,     0,   258,     0,   258,
       0,     0,     0,  1666,   745,     0,     0,  1364,   745,   920,
     745,     0,     0,   745,     0,     0,     0,     0,     0,     0,
       0,   258,     0,   941,   942,   943,   944,     0,     0,     0,
       0,     0,   258,   952,     0,     0,   744,     0,     0,   748,
       0,     0,   744,   744,   744,   744,   744,   744,   744,   744,
     744,   744,   744,   744,   744,   744,   744,   744,   744,   744,
     744,   744,   744,   744,   744,   744,   744,   744,   744,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   745,   379,
       0,     0,     0,     0,   744,     0,     0,   258,     0,     0,
       0,   380,     0,     0,   258,     0,     0,   381,    36,     0,
     207,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   354,   355,   356,     0,     0,     0,   258,     0,
     258,     0,     0,     0,     0,     0,   258,     0,     0,     0,
    1048,   357,  1253,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,     0,   379,     0,     0,     0,     0,
       0,   745,   745,     0,     0,     0,     0,   380,     0,   745,
       0,     0,     0,     0,     0,   666,   258,    84,    85,     0,
      86,   179,    88,     0,     0,   744,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     744,     0,   744,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,   744,     0,
       0,     0,   667,     0,   114,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1134,  1137,  1138,  1139,  1141,
    1142,  1143,  1144,  1145,  1146,  1147,  1148,  1149,  1150,  1151,
    1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,
    1162,  1163,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   258,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1175,     0,     0,
       0,     0,     0,     0,   258,     0,     0,     0,     0,     0,
       0,     0,  1254,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1626,     0,     0,     0,     0,   745,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   745,     0,
       0,     0,     0,   745,     0,     0,     0,   745,     0,   354,
     355,   356,     0,   744,     0,     0,     0,   744,     0,   744,
       0,     0,   744,     0,     0,     0,     0,     0,   357,   258,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,     0,   379,     0,     0,     0,     0,     0,  1248,    36,
       0,   207,     0,     0,   380,     0,     0,   745,     0,     0,
       0,     0,     0,  1262,     0,  1263,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1273,     0,     0,   354,   355,   356,   744,     0,   258,
       0,     0,     0,     0,     0,     0,   638,     0,     0,     0,
       0,     0,     0,   357,   258,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,     0,   379,    84,    85,
       0,    86,   179,    88,     0,     0,     0,     0,     0,   380,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,     0,
     744,   744,     0,   639,     0,   114,     0,     0,   744,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   464,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     354,   355,   356,     0,     0,     0,  1350,     0,     0,     0,
    1352,     0,  1353,     0,     0,  1354,     0,     0,     0,   357,
       0,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,     0,   379,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,   207,   380,     0,     0,     0,     0,
       0,   557,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   466,   354,   355,   356,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1431,     0,     0,   208,   357,     0,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,     0,   379,     0,
       0,     0,     0,     0,     0,   178,   744,     0,    82,    83,
     380,    84,    85,     0,    86,   179,    88,   744,     0,     0,
       0,     0,   744,     0,     0,     0,   744,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,     0,  1575,  1576,     0,     0,     0,   558,     0,
       0,  1581,     0,     0,     0,     5,     6,     7,     8,     9,
     478,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   744,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,   502,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,     0,     0,     0,    49,    50,    51,    52,    53,    54,
      55,     0,    56,    57,    58,    59,    60,    61,    62,    63,
      64,     0,    65,    66,    67,    68,    69,    70,     0,     0,
       0,     0,     0,    71,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,  1634,
       0,     0,    79,    80,    81,    82,    83,     0,    84,    85,
    1644,    86,    87,    88,    89,  1648,     0,    90,     0,  1650,
      91,     0,     0,     0,     0,     0,    92,    93,     0,    94,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,  1021,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,  1683,
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
     108,   109,   110,     0,     0,   111,     0,   112,   113,  1186,
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
     108,   109,   110,     0,     0,   111,     0,   112,   113,   579,
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
     111,     0,   112,   113,  1578,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,    40,
       0,     0,     0,    41,    42,    43,    44,     0,    45,     0,
      46,  1621,    47,     0,     0,    48,     0,     0,     0,    49,
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
     111,     0,   112,   113,  1654,   114,   115,     0,   116,   117,
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
     108,   109,   110,     0,     0,   111,     0,   112,   113,  1655,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,  1658,    46,     0,    47,     0,     0,
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
     108,   109,   110,     0,     0,   111,     0,   112,   113,  1674,
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
     111,     0,   112,   113,  1729,   114,   115,     0,   116,   117,
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
     108,   109,   110,     0,     0,   111,     0,   112,   113,  1736,
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
      12,     0,  1426,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,    11,    12,     0,  1570,     0,     0,
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
       5,     6,     7,     8,     9,     0,     0,     0,     0,   958,
      10,   959,   960,   961,   962,   963,   964,   965,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     978,   979,   594,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,    90,     0,     0,    91,     0,     0,     0,  1622,
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
       0,     0,     0,     0,     0,    10,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   607,   379,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   380,     0,     0,    14,    15,
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
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   649,   379,     0,     0,     0,     0,     0,     0,
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
       0,     0,    90,     0,     0,    91,     0,     0,  1434,     0,
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
      77,     0,    36,     0,   207,     0,     0,    78,     0,     0,
       0,     0,   178,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   179,    88,     0,     0,     0,    90,     0,     0,
      91,     0,  1275,     0,     0,     0,    92,     0,     0,     0,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,     0,     0,     0,   114,   115,     0,   116,   117,
    1451,  1452,  1453,  1454,  1455,     0,     0,  1456,  1457,  1458,
    1459,    84,    85,     0,    86,   179,    88,     0,     0,     0,
       0,     0,     0,     0,  1460,  1461,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
    1462,     0,     0,     0,     0,     0,   639,     0,   114,     0,
       0,     0,     0,     0,  1463,  1464,  1465,  1466,  1467,  1468,
    1469,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1470,  1471,  1472,  1473,  1474,  1475,
    1476,  1477,  1478,  1479,  1480,  1481,  1482,  1483,  1484,  1485,
    1486,  1487,  1488,  1489,  1490,  1491,  1492,  1493,  1494,  1495,
    1496,  1497,  1498,  1499,  1500,  1501,  1502,  1503,  1504,  1505,
    1506,  1507,  1508,  1509,  1510,     0,     0,     0,  1511,  1512,
       0,  1513,  1514,  1515,  1516,  1517,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1518,  1519,  1520,
       0,     0,     0,    84,    85,     0,    86,   179,    88,  1521,
       0,  1522,  1523,     0,  1524,     0,     0,     0,     0,     0,
       0,  1525,  1526,     0,  1527,     0,  1528,  1529,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   354,   355,   356,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   357,     0,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,     0,   379,   354,   355,   356,     0,
       0,     0,     0,     0,     0,     0,     0,   380,     0,     0,
       0,     0,     0,     0,     0,   357,     0,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,     0,   379,
     354,   355,   356,     0,     0,     0,     0,     0,     0,     0,
       0,   380,     0,     0,     0,     0,   250,     0,     0,   357,
       0,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   251,   379,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   380,     0,     0,     0,   354,
     355,   356,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   357,   250,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   682,   379,     0,     0,   251,     0,     0,     0,     0,
       0,     0,     0,     0,   380,     0,     0,     0,     0,   252,
     253,     0,     0,     0,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   178,     0,     0,
      82,   254,   250,    84,    85,   704,    86,   179,    88,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   255,     0,     0,     0,     0,     0,     0,   251,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   252,   253,   991,   256,     0,     0,     0,  1550,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     178,     0,     0,    82,   254,     0,    84,    85,     0,    86,
     179,    88,     0,     0,     0,     0,     0,     0,  -332,     0,
       0,     0,     0,     0,   255,     0,    59,    60,    61,   174,
     175,   344,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   252,   253,     0,   256,     0,
       0,     0,  1617,     0,     0,     0,     0,     0,     0,     0,
     250,     0,     0,   178,     0,     0,    82,   254,     0,    84,
      85,     0,    86,   179,    88,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   251,   255,     0,     0,
       0,     0,     0,   345,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,    36,     0,
       0,   256,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   250,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   471,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   251,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   252,   253,     0,     0,     0,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   178,     0,     0,    82,   254,   250,    84,    85,     0,
      86,   179,    88,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   255,     0,     0,     0,     0,
       0,     0,   251,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   252,   253,     0,   256,
       0,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   178,     0,     0,    82,   254,   250,
      84,    85,     0,    86,   179,    88,     0,   936,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   255,     0,
       0,     0,     0,     0,     0,   251,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   252,
     253,     0,   256,     0,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   178,     0,     0,
      82,   254,   250,    84,    85,     0,    86,   179,    88,     0,
    1259,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   255,     0,     0,     0,     0,     0,     0,   251,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   252,   253,     0,   256,     0,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     178,     0,     0,    82,   254,     0,    84,    85,     0,    86,
     179,    88,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   255,  1361,     0,     0,     0,     0,
       0,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   252,   253,    36,   256,   207,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   178,     0,     0,    82,   254,     0,    84,
      85,     0,    86,   179,    88,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   255,   208,     0,
       0,     0,  1140,     0,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   732,   733,
       0,   256,     0,     0,   734,     0,   735,     0,     0,     0,
     178,     0,     0,    82,    83,     0,    84,    85,   736,    86,
     179,    88,     0,     0,     0,     0,    33,    34,    35,    36,
       0,     0,     0,   916,     0,     0,     0,     0,   737,     0,
       0,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,     0,   209,     0,
       0,   515,     0,   114,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    36,     0,   207,     0,     0,
       0,     0,     0,     0,   738,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,   739,     0,     0,
       0,     0,   178,    80,    81,    82,   740,     0,    84,    85,
       0,    86,   179,    88,     0,     0,   208,    90,     0,     0,
       0,     0,     0,     0,     0,     0,   741,     0,     0,   917,
       0,    95,     0,    36,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,   178,     0,
     742,    82,    83,     0,    84,    85,     0,    86,   179,    88,
       0,   961,   962,   963,   964,   965,   966,   967,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   977,   978,   979,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   980,   732,   733,   209,     0,     0,     0,
     734,   114,   735,     0,     0,     0,   178,     0,     0,    82,
       0,     0,    84,    85,   736,    86,   179,    88,     0,     0,
       0,     0,    33,    34,    35,    36,     0,     0,     0,     0,
       0,     0,     0,     0,   737,     0,     0,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,     0,     0,     0,     0,     0,  1624,     0,
       0,     0,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     738,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,   739,     0,     0,     0,     0,   178,    80,
      81,    82,   740,     0,    84,    85,     0,    86,   179,    88,
    1381,     0,     0,    90,     0,     0,     0,     0,     0,     0,
       0,     0,   741,   883,   884,     0,     0,    95,     0,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   885,    84,    85,   742,    86,   179,    88,
       0,   886,   887,   888,    36,     0,     0,     0,     0,     0,
       0,     0,     0,   889,     0,     0,     0,     0,     0,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,     0,  1382,     0,     0,     0,
       0,     0,     0,    29,    30,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,    38,     0,     0,     0,   890,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   891,     0,     0,     0,     0,     0,     0,     0,
       0,    52,     0,    84,    85,     0,    86,   179,    88,    59,
      60,    61,   174,   175,   176,   872,     0,     0,     0,     0,
       0,   892,     0,     0,    36,     0,   207,     0,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,     0,     0,   178,     0,     0,    82,
      83,     0,    84,    85,     0,    86,   179,    88,     0,     0,
       0,     0,     0,     0,    91,   208,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,   430,    36,     0,   178,     0,   114,
      82,    83,     0,    84,    85,     0,    86,   179,    88,     0,
      36,     0,   207,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   272,   273,     0,     0,     0,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,     0,   209,     0,     0,     0,     0,
     114,   208,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   535,     0,     0,     0,     0,     0,
       0,   274,     0,     0,    84,    85,     0,    86,   179,    88,
       0,     0,     0,   178,     0,     0,    82,    83,     0,    84,
      85,     0,    86,   179,    88,     0,    36,     0,   207,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
       0,   209,     0,     0,     0,     0,   114,   208,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    36,     0,   207,
    1016,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   178,
     379,     0,    82,    83,     0,    84,    85,     0,    86,   179,
      88,     0,   380,     0,     0,     0,     0,     0,   208,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,     0,
     207,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,     0,   209,     0,     0,
     178,     0,   114,    82,    83,     0,    84,    85,     0,    86,
     179,    88,     0,     0,     0,     0,     0,     0,     0,   221,
       0,     0,     0,    36,     0,   207,     0,     0,     0,     0,
       0,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,     0,   209,     0,
       0,   178,     0,   114,    82,    83,     0,    84,    85,     0,
      86,   179,    88,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,     0,   222,
     666,     0,    84,    85,   114,    86,   179,    88,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,     0,     0,     0,     0,   699,   304,   114,
       0,    84,    85,     0,    86,   179,    88,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    36,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,     0,     0,     0,  1535,   305,    84,    85,  1536,
      86,   179,    88,     0,     0,   962,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   980,    84,    85,  1382,
      86,   179,    88,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    84,    85,     0,    86,   179,    88,
       0,     0,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   354,   355,   356,   600,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,   357,   854,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,     0,   379,
     354,   355,   356,     0,     0,     0,     0,     0,     0,     0,
       0,   380,     0,     0,     0,     0,     0,     0,     0,   357,
       0,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,     0,   379,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   380,   354,   355,   356,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   357,   426,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,     0,   379,
     354,   355,   356,     0,     0,     0,     0,     0,     0,     0,
       0,   380,     0,     0,     0,     0,     0,     0,     0,   357,
     436,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,     0,   379,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   380,   354,   355,   356,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   357,   860,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,     0,   379,
     955,   956,   957,     0,     0,     0,     0,     0,     0,     0,
       0,   380,     0,     0,     0,     0,     0,     0,     0,   958,
     902,   959,   960,   961,   962,   963,   964,   965,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     978,   979,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   980,   955,   956,   957,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   958,  1215,   959,   960,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,     0,     0,
     955,   956,   957,     0,     0,     0,     0,     0,     0,     0,
       0,   980,     0,     0,     0,     0,     0,     0,     0,   958,
    1122,   959,   960,   961,   962,   963,   964,   965,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     978,   979,  1070,  1071,  1072,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   980,   955,   956,   957,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   958,  1269,   959,   960,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,     0,     0,
       0,   980,     0,     0,     0,   725,     0,     0,     0,     0,
    1349,     0,    36,     0,    84,    85,     0,    86,   179,    88,
       0,     0,     0,     0,  -918,  -918,  -918,  -918,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   977,   978,   979,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   980,  1368,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,  1433,  1369,  1370,   178,
       0,    36,    82,   804,   805,    84,    85,     0,    86,   179,
      88,     0,     0,     0,     0,   178,     0,     0,    82,    83,
       0,    84,    85,     0,    86,  1372,    88,     0,     0,     0,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,    36,     0,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     178,    36,     0,    82,    83,     0,    84,    85,     0,    86,
     179,    88,     0,     0,     0,     0,     0,     0,     0,     0,
      84,    85,     0,    86,   179,    88,     0,     0,     0,     0,
       0,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,    36,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,    36,
       0,   341,     0,    84,    85,     0,    86,   179,    88,     0,
       0,     0,     0,     0,     0,     0,     0,   503,     0,     0,
      84,    85,     0,    86,   179,    88,     0,     0,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,    36,     0,     0,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,   507,     0,     0,    84,    85,     0,    86,   179,    88,
       0,     0,     0,     0,     0,   274,     0,     0,    84,    85,
       0,    86,   179,    88,    36,     0,     0,     0,     0,   638,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,    36,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,     0,
       0,    84,    85,     0,    86,   179,    88,     0,     0,     0,
       0,  1132,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,     0,    84,    85,     0,    86,   179,    88,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    84,    85,     0,    86,   179,    88,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,   354,   355,   356,     0,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   716,   357,     0,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,     0,   379,     0,   354,   355,
     356,     0,     0,     0,     0,     0,     0,     0,   380,     0,
       0,     0,     0,     0,     0,     0,     0,   357,   857,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     717,   379,   955,   956,   957,     0,     0,     0,     0,     0,
       0,     0,     0,   380,     0,     0,     0,     0,     0,     0,
       0,   958,  1274,   959,   960,   961,   962,   963,   964,   965,
     966,   967,   968,   969,   970,   971,   972,   973,   974,   975,
     976,   977,   978,   979,   955,   956,   957,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   980,     0,     0,
       0,     0,     0,   958,     0,   959,   960,   961,   962,   963,
     964,   965,   966,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,   978,   979,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   980,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,     0,   379,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     380,  -918,  -918,  -918,  -918,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,     0,   379,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     380
};

static const yytype_int16 yycheck[] =
{
       5,     6,    53,     8,     9,    10,    11,    12,   133,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    89,   181,    28,    29,    93,    94,   158,  1033,     4,
     312,   597,     4,     4,   420,     4,   684,    42,   834,  1037,
     568,   161,   393,    32,    49,   379,    51,   569,    54,    54,
     158,    56,   446,   170,   823,    44,   715,   182,   229,    48,
    1023,   550,   414,   415,   132,    57,   111,   223,   853,   446,
     111,     9,    30,   722,    50,     9,    14,    30,     9,    45,
      45,    45,   111,    66,   869,     9,     9,    79,    45,     4,
      82,     9,   444,     9,   116,    79,    45,     9,    79,     4,
       9,     9,   124,     9,   198,     9,   111,     9,   198,    30,
     230,     9,  1025,   915,    10,    11,    12,     9,    85,     9,
      66,    66,    66,    66,     9,     9,   297,     9,     9,    79,
      45,    66,     9,    29,     9,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,     9,    53,     9,     4,
     128,   129,    79,   105,   209,    66,    66,    97,   155,    65,
    1564,   113,   114,   115,   116,   117,   118,   222,   154,   146,
     209,   186,   951,    35,     9,   198,     9,   346,   128,   129,
       0,   106,     9,     9,     8,   312,   111,    66,   113,   114,
     115,   116,   117,   118,   119,  1210,   149,    66,    33,    66,
     219,    79,    66,   163,   201,   198,    66,    79,   201,   203,
      46,    47,   203,   153,  1618,   198,   231,    79,   133,   234,
     196,   199,   149,   199,   199,   199,   241,   242,   180,   196,
     171,   156,   157,   201,   159,   290,    66,   200,   171,   290,
     199,    66,   198,   171,   198,   201,   201,  1220,   201,   199,
     235,   290,   200,   422,   239,   180,   201,   392,   202,   386,
      26,    27,   340,     4,    30,   199,   925,   182,   927,  1064,
     272,   273,   274,    66,   200,   290,   337,   202,   200,   201,
     199,   296,   200,   410,   200,   300,   200,    30,   200,  1212,
     201,   201,   200,  1105,   198,    66,  1219,   199,  1221,   205,
     200,  1316,   304,  1318,  1083,   200,   200,   434,   200,   200,
     325,   389,   390,   200,   813,   200,   443,   198,   850,   446,
     117,   336,   201,   195,  1297,   203,    66,   124,    79,   201,
      79,    85,   201,   201,   201,    79,   199,   201,   199,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   199,   380,   199,   382,   383,     4,
     385,   201,   199,   199,   154,   430,   201,   393,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   430,   146,  1366,   575,   109,   411,   412,  1413,   414,
     415,   416,   417,  1326,   800,   198,   199,   149,   423,   324,
     429,   426,   163,    50,   163,    50,    35,   158,    53,   163,
      35,   436,    66,   438,    35,    79,  1084,    79,   170,   444,
     201,    85,  1101,    35,   838,    70,   452,   452,   198,   454,
     100,   101,   198,   209,   585,    35,    97,   170,   379,   201,
     216,   838,    87,   167,    89,   386,   222,   153,    93,    94,
      79,   201,   198,   318,    79,   198,   481,   585,    79,   484,
     485,   486,   457,   155,   640,   198,   391,    79,   647,   410,
     100,   101,   250,   251,   128,   129,   168,  1266,   256,    79,
     128,   129,   198,   147,   148,   147,   148,   132,  1036,   903,
     515,   503,   153,   434,   866,   507,   672,   198,   689,   198,
     512,   450,   443,   279,   695,   446,   903,   154,  1050,   201,
      79,  1053,   288,   289,   690,   603,    85,   198,   872,   295,
      79,   823,   169,    53,    79,   301,    85,    97,    79,    97,
      85,   560,   202,    79,   163,    65,   312,    35,   163,    85,
     200,    14,  1567,   207,    66,  1334,   201,   201,    97,   194,
     499,   163,   170,    97,   202,   695,   334,    30,  1226,    79,
    1228,   200,    79,   163,     4,    85,   200,   743,    85,   200,
     595,   153,   202,   324,    47,   751,   128,   129,   147,   148,
     198,   200,   607,   153,   149,   153,    26,    27,   147,   148,
     235,   146,   147,   148,   239,   146,   147,   148,   243,   200,
     146,   147,   148,   379,   153,   170,   128,   129,   637,   153,
     383,   149,   170,   170,   639,    71,    72,   262,   200,   201,
     200,   570,    66,   648,    66,   298,   149,   147,   148,   302,
     147,   148,   170,   198,    71,    72,   201,   206,   411,    66,
     198,   198,   667,   416,   420,    29,    66,  1018,   198,   951,
      29,  1199,   149,   326,   430,   328,   329,   330,   331,    49,
      50,    51,    46,   201,  1332,    49,   198,    46,   153,  1694,
      49,   111,  1214,   198,   699,    65,    99,   100,   101,   324,
     126,   127,  1700,    44,  1709,   200,   823,  1059,   333,    65,
     715,   469,   337,   200,   201,   340,  1068,  1715,  1104,   728,
     170,   838,   711,   198,   569,   113,   114,   115,   116,   117,
     118,   149,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,   720,   200,   201,   753,  1593,
    1594,   113,   114,   115,   116,   117,   118,    99,   100,   101,
     205,   386,   387,   388,   389,   390,   113,   114,   115,   115,
     116,   117,  1420,  1589,  1590,    49,    50,    51,   707,    53,
     772,    63,    64,   712,   776,   410,   903,     9,   149,   209,
     198,    65,   180,   730,   731,  1181,   216,   149,     8,   200,
     705,  1083,   222,   871,   809,  1327,   198,   565,   566,   434,
     170,   149,   200,    14,   149,   235,   574,   822,   180,   239,
      79,   446,   200,   200,   124,   124,    46,    47,    48,    49,
      50,    51,   457,    53,   951,    14,   199,   812,   594,   170,
     812,   812,   847,   812,    14,    65,   128,   129,    97,   474,
     199,   199,   857,   204,   585,   860,   199,   862,   198,   279,
     105,   866,   767,     9,  1216,   198,   198,     9,   288,   289,
     290,   199,   199,  1669,    89,   295,   200,    14,   198,     9,
     184,   301,    79,    79,    79,   187,   198,  1043,   897,   514,
     200,  1687,     9,   649,   181,   182,   183,   902,     9,  1695,
     126,   188,   189,   200,   324,   192,   193,   812,    79,   199,
     199,   199,   198,     4,   539,   540,   200,   838,   199,   908,
      30,  1569,    66,   127,   130,   169,     9,   832,   199,    14,
     149,   940,   196,     9,   909,   691,     9,   693,   947,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,   872,    53,    66,    45,   171,   199,     9,    14,   126,
       9,   717,  1314,   205,    65,   205,  1083,   812,   202,   198,
     205,   199,   730,   731,   705,   199,   199,   198,   603,   984,
     985,   986,   903,    97,  1266,   990,   991,   205,   200,   834,
     130,   200,   149,   922,     9,   149,   199,   149,   198,   198,
     420,   184,   184,   201,    14,   850,   911,     9,   913,    79,
     430,   201,  1018,  1018,    14,   106,   200,    14,   205,    14,
     111,   201,   113,   114,   115,   116,   117,   118,   119,   785,
      46,    47,    48,    49,    50,    51,   767,   457,   201,   200,
     196,  1046,    30,   198,   800,   801,  1612,  1022,   198,    65,
    1022,  1022,  1334,  1022,  1059,    30,   814,   198,    14,   684,
     198,   686,    48,  1068,  1069,   156,   157,   823,   159,  1078,
     198,   829,     9,    74,    75,    76,   199,   130,   200,   200,
     705,   812,   198,   841,    14,    86,     9,   199,    65,   180,
     205,     9,   146,    97,   719,   720,  1101,    79,     9,   198,
     130,   832,   200,    79,  1093,    14,  1111,   199,   201,   198,
     198,   202,   199,   130,     9,   201,   872,  1022,    86,  1685,
    1129,  1096,   201,   205,   146,    30,    73,  1136,   200,   200,
     199,   171,   130,   134,   135,   136,   137,   138,    30,   199,
     199,   130,   767,     9,   145,   202,     9,   202,   773,  1266,
     151,   152,   777,   778,   199,   199,    14,    79,   201,   915,
     916,   198,   201,   199,   165,   199,   199,  1298,  1293,   198,
     130,   929,   797,   931,   594,   199,   199,  1022,   179,   199,
     911,     9,   913,    30,    97,   199,  1191,   812,   200,   200,
    1195,   199,  1197,   200,   106,   951,   201,   158,    14,   154,
    1205,   826,    79,  1108,   200,  1050,   199,   832,  1053,   111,
    1215,  1216,   199,   838,   130,   199,   201,  1334,    14,   170,
     130,   201,   200,    79,    14,    14,  1235,    79,  1387,   649,
     199,  1282,   198,   130,   201,   199,    14,   200,   200,    14,
      14,   200,    55,  1172,   201,    79,   871,    79,   198,     9,
      79,   200,   113,   114,   115,   116,   117,   118,   883,   884,
     885,   109,    97,   124,   125,   149,    97,   161,    33,    14,
    1275,   691,   198,   693,   199,   171,   200,  1035,   903,  1037,
     167,   198,    79,   164,   909,   705,   911,     9,   913,  1045,
      14,  1022,   199,  1361,    79,   201,   200,   717,   199,   160,
     720,   162,    79,    14,  1062,    14,  1311,   932,   199,  1314,
    1239,    79,    79,    14,  1365,    79,   772,   776,   512,   180,
    1677,   387,   390,   870,   389,   950,  1255,  1083,   953,   815,
    1691,   867,  1088,  1102,  1179,  1416,  1425,  1687,  1389,  1252,
    1391,   517,  1449,  1188,  1367,  1533,  1719,   767,  1104,  1105,
    1545,  1707,  1288,    42,  1412,   393,   981,  1284,   990,   488,
     993,  1119,   488,  1026,  1066,   785,   764,  1342,   884,  1214,
     948,  1079,  1277,   998,  1080,   838,  1001,  1108,   899,  1009,
     800,   801,   296,    -1,   981,    26,    27,  1292,    -1,   289,
    1549,   730,   812,   933,    -1,    -1,    -1,  1022,    -1,  1404,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1336,    -1,    -1,
      -1,    -1,   832,    -1,    -1,    -1,  1345,    -1,    -1,  1177,
      -1,    -1,    -1,    -1,    -1,  1181,    -1,  1356,    -1,  1434,
      -1,    -1,  1337,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1346,    -1,    -1,    -1,    -1,    -1,    -1,  1179,  1424,
    1425,    -1,    -1,    -1,    -1,  1080,    -1,  1188,    -1,  1084,
      -1,    -1,   113,   114,   115,   116,   117,   118,    -1,    -1,
      -1,  1096,  1317,   124,   125,    -1,  1544,    -1,  1323,    -1,
    1325,    -1,  1327,  1108,    -1,    -1,    -1,    -1,    -1,   909,
    1395,   911,    -1,   913,    -1,   915,   916,  1426,    -1,    -1,
    1258,    -1,  1260,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1266,   162,    -1,    -1,    -1,    -1,    -1,    -1,  1423,    -1,
      -1,    -1,    -1,    -1,  1429,    -1,    -1,    -1,    -1,   180,
    1435,    -1,    -1,  1664,    -1,  1293,    -1,    -1,  1606,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1277,    -1,    -1,  1554,
      -1,  1176,    -1,    -1,    -1,    -1,  1681,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1298,   209,    -1,
      -1,    -1,    -1,  1418,    -1,   216,    -1,    -1,  1334,    -1,
      -1,   222,    -1,    -1,    -1,    -1,  1317,    -1,    -1,    -1,
      -1,    -1,  1323,    -1,  1325,    -1,    -1,    -1,    -1,    -1,
    1358,  1226,  1022,  1228,    -1,    -1,  1337,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1346,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,    27,  1045,    -1,    30,    -1,    -1,
    1559,    -1,  1561,    -1,    -1,    -1,    -1,    -1,   279,    -1,
      -1,  1570,    -1,    -1,    -1,    -1,    -1,   288,   289,   290,
      53,    -1,  1277,    -1,   295,    -1,    -1,  1282,    -1,    -1,
     301,    -1,    -1,  1288,    -1,    -1,    -1,    -1,  1088,  1574,
      -1,    -1,    -1,    -1,    -1,    -1,  1096,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1104,  1105,  1615,  1418,  1108,  1447,
      -1,    -1,  1423,    -1,    -1,    -1,    -1,    -1,  1429,    -1,
      -1,    -1,    -1,    -1,  1435,    -1,    -1,  1332,  1613,  1614,
      -1,    -1,  1337,    -1,    -1,  1620,    -1,  1342,     4,    -1,
    1725,  1346,    -1,    -1,    -1,    -1,    -1,    -1,  1733,    -1,
      -1,    -1,    -1,    -1,  1739,    -1,  1361,  1742,    -1,    -1,
    1365,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1375,  1656,     4,    -1,    -1,    -1,    -1,  1382,    -1,    45,
      -1,  1181,    -1,    -1,  1389,    -1,  1391,    -1,    -1,    -1,
      -1,    -1,  1397,    -1,    -1,    -1,    -1,    -1,    -1,   420,
      -1,    -1,    -1,    -1,  1713,    -1,    -1,    -1,    -1,   430,
    1548,  1720,    -1,    45,    -1,  1420,    -1,    -1,  1423,  1424,
    1425,    -1,    -1,    -1,  1429,    -1,   209,    -1,    -1,    -1,
    1435,    -1,    -1,   216,    -1,    -1,  1721,    -1,    -1,   222,
     106,    -1,    -1,  1728,  1669,   111,    -1,   113,   114,   115,
     116,   117,   118,   119,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1687,  1574,    -1,    -1,    -1,   250,   251,    -1,
    1695,    -1,    -1,   256,   106,    -1,    -1,  1277,    -1,   111,
      -1,   113,   114,   115,   116,   117,   118,   119,    -1,    -1,
     156,   157,  1630,   159,    -1,    -1,   279,    -1,    -1,    -1,
      -1,    -1,  1613,  1614,    -1,   288,   289,    -1,    -1,  1620,
      -1,    -1,   295,    -1,   180,    -1,    -1,    -1,   301,    -1,
      -1,    -1,    -1,    -1,   156,   157,    -1,   159,    -1,   312,
      -1,    -1,    -1,    -1,    -1,    -1,   202,  1337,    -1,  1544,
      -1,    -1,  1342,    -1,    -1,  1656,  1346,    -1,   180,    -1,
       4,   334,    -1,  1664,   337,    -1,    -1,    -1,    -1,    -1,
    1565,    -1,  1700,    -1,  1569,    -1,    -1,    -1,    -1,  1574,
     202,    -1,    -1,   594,    -1,    -1,    -1,  1715,  1583,    -1,
      -1,    -1,    -1,    -1,  1589,  1590,    -1,    -1,  1593,  1594,
      -1,    45,    -1,    -1,    -1,    -1,   379,     4,    -1,    -1,
      -1,  1606,    -1,    -1,    -1,    -1,    -1,    -1,  1613,  1614,
    1721,    -1,    -1,    -1,    -1,  1620,    -1,  1728,    -1,    -1,
      -1,    -1,    -1,  1423,  1424,  1425,    -1,    -1,   649,  1429,
      -1,    -1,    -1,    -1,    -1,  1435,    -1,   420,    45,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   430,    -1,    -1,
      -1,  1656,   106,    -1,    -1,    -1,    -1,   111,  1663,   113,
     114,   115,   116,   117,   118,   119,    -1,    -1,    -1,    -1,
     691,    -1,   693,    -1,    -1,  1680,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   469,   470,    -1,    -1,
     473,    -1,    -1,    -1,    -1,    -1,   717,    -1,    -1,   106,
      -1,    -1,   156,   157,   111,   159,   113,   114,   115,   116,
     117,   118,   119,    -1,    -1,    -1,  1721,    -1,    -1,    -1,
      -1,    -1,    -1,  1728,    -1,    -1,   180,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   521,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   202,   156,
     157,    -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   785,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   180,  1574,    -1,    -1,    -1,    -1,   800,
     801,    -1,   565,   566,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   574,    -1,    -1,    -1,   202,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   594,    -1,  1613,  1614,    -1,    -1,    -1,    -1,    29,
    1620,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    65,  1656,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,   649,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,   915,   916,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,   691,    -1,
     693,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1721,    -1,    -1,    -1,    -1,    -1,    -1,  1728,    -1,
      -1,    -1,    -1,    -1,   717,   718,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,   730,   731,   732,
     733,   734,   735,   736,    -1,    -1,    -1,    -1,    29,   742,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,   768,    -1,    -1,    -1,    -1,
      -1,    -1,   202,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   785,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,    27,    -1,   798,    30,   800,   801,    -1,
      -1,    -1,    -1,    -1,  1045,    -1,    -1,    -1,    -1,    -1,
      -1,   814,   815,    -1,    -1,    -1,    -1,    -1,   202,    -1,
     823,    -1,    -1,    -1,    -1,    -1,   829,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   841,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   849,  1088,    -1,   852,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1104,  1105,    -1,    -1,    -1,    -1,   872,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,   202,   915,   916,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,   929,    -1,   931,    -1,
     933,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
    1181,    -1,    -1,    -1,    -1,   948,    -1,    -1,   951,    -1,
      -1,   954,   955,   956,   957,   958,   959,   960,   961,   962,
     963,   964,   965,   966,   967,   968,   969,   970,   971,   972,
     973,   974,   975,   976,   977,   978,   979,   980,    -1,    -1,
      -1,    -1,   216,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1006,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,  1035,    -1,  1037,    -1,    -1,    -1,    -1,    -1,
      65,    -1,  1045,   202,    -1,   279,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   288,   289,    -1,    -1,    -1,  1062,
      -1,   295,    -1,    -1,    -1,    -1,    -1,   301,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   312,    -1,
    1083,    -1,    -1,    -1,    -1,  1088,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1104,  1105,    -1,  1107,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1119,    -1,    -1,  1122,
      -1,  1124,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1140,    -1,    -1,
      -1,    -1,    -1,    -1,    29,   379,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    10,
      11,    12,    -1,    -1,  1177,  1178,    -1,   202,  1181,    -1,
      65,    -1,    -1,    -1,    -1,    -1,   420,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   473,
      -1,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1258,    -1,  1260,    -1,    -1,
      -1,    -1,  1265,  1266,    -1,    -1,  1269,    -1,  1271,    -1,
      -1,  1274,    -1,    -1,    10,    11,    12,    -1,    -1,  1282,
    1283,    -1,    -1,  1286,    -1,    -1,    -1,   521,    -1,    -1,
    1293,    -1,    -1,    29,   473,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,   202,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,  1334,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   521,    -1,    -1,    -1,  1349,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1358,  1359,  1360,    -1,    -1,
     594,   202,  1365,    -1,  1367,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,    -1,    -1,    -1,  1389,    -1,  1391,    -1,
      -1,    -1,    -1,    -1,  1397,    -1,    -1,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,    -1,    -1,   649,    63,    64,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1432,
    1433,    -1,    -1,    -1,    -1,    -1,    -1,  1440,  1441,   250,
     251,    -1,    -1,    -1,  1447,   256,  1449,    63,    64,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   691,    -1,   693,
      -1,    -1,    -1,   199,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,   128,   129,   717,   718,    -1,    -1,    -1,    -1,    29,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,   732,   733,
     734,   735,   736,    -1,    -1,    -1,    -1,    -1,   742,    -1,
      -1,    -1,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   334,    -1,    -1,   337,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   768,    -1,    -1,    77,    -1,   718,
      -1,    -1,    -1,    -1,    -1,  1548,    -1,    -1,    -1,    -1,
      -1,   785,   199,   732,   733,   734,   735,   736,    -1,    -1,
      -1,    -1,  1565,   742,   798,    -1,   800,   801,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   119,
    1583,   815,    -1,   199,    -1,  1588,    -1,    -1,    -1,   823,
      -1,    -1,   132,   133,    -1,    -1,  1599,    -1,    -1,    -1,
      -1,  1604,    -1,    -1,    -1,  1608,    -1,    -1,    -1,    -1,
     150,    -1,    -1,   153,   154,   849,   156,   157,   852,   159,
     160,   161,    -1,    -1,    -1,    -1,    -1,  1630,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   872,    -1,
      -1,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    -1,    -1,    -1,   469,   470,
      -1,    -1,   473,    -1,    -1,  1668,    -1,    -1,    -1,    -1,
     849,    -1,    -1,  1676,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   915,   916,    -1,    -1,    -1,    -1,    -1,  1691,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1700,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     521,    -1,  1715,    -1,   948,    -1,    -1,   951,    -1,    -1,
     954,   955,   956,   957,   958,   959,   960,   961,   962,   963,
     964,   965,   966,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,   978,   979,   980,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   565,   566,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   574,    -1,    -1,    -1,    -1,    -1,   948,
      -1,    -1,  1006,    -1,    -1,   954,   955,   956,   957,   958,
     959,   960,   961,   962,   963,   964,   965,   966,   967,   968,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   980,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,  1045,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,  1006,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,  1083,
      53,    -1,    -1,    -1,  1088,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1104,  1105,    -1,  1107,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1122,    -1,
    1124,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,  1140,   718,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   730,
     731,   732,   733,   734,   735,   736,    -1,    -1,  1107,    -1,
      -1,   742,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      63,    64,    -1,  1122,  1178,  1124,    -1,  1181,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1140,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,   202,
      -1,    -1,    -1,   814,    65,   128,   129,    -1,    -1,    -1,
      -1,   473,    -1,    -1,    -1,    -1,    -1,    -1,   829,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     841,  1265,  1266,    -1,    -1,  1269,    -1,  1271,   849,    -1,
    1274,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,  1283,
      -1,    -1,  1286,    -1,    -1,    -1,    -1,    -1,    -1,   521,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,  1265,    -1,    -1,    -1,
    1269,    -1,  1271,    -1,    -1,  1274,    65,    -1,    -1,    -1,
    1334,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1349,    -1,    -1,   929,    -1,
     931,    -1,   933,    -1,    -1,  1359,  1360,    -1,    -1,    -1,
      -1,    -1,    -1,  1367,    -1,    -1,    -1,   948,    -1,    -1,
      -1,   202,    -1,   954,   955,   956,   957,   958,   959,   960,
     961,   962,   963,   964,   965,   966,   967,   968,   969,   970,
     971,   972,   973,   974,   975,   976,   977,   978,   979,   980,
    1349,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,  1006,    53,    -1,  1432,  1433,
      -1,    67,    68,    -1,    -1,    -1,  1440,  1441,    65,    -1,
      -1,    77,    -1,    79,   473,  1449,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1035,    -1,  1037,    -1,    -1,    -1,
      -1,    -1,    -1,   202,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,  1062,   118,  1432,  1433,    -1,   718,    -1,    -1,    -1,
      -1,  1440,   521,    65,    -1,    -1,    -1,    -1,    -1,  1448,
     732,   733,   734,   735,   736,    -1,    -1,    -1,    -1,    -1,
     742,    -1,    -1,    -1,   150,    -1,    -1,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,  1107,    -1,    -1,    -1,
      -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,  1119,    -1,
      -1,  1122,    -1,  1124,    -1,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,  1140,
      -1,    -1,   198,    -1,    -1,    -1,    -1,   203,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1588,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1599,  1177,    10,    11,    12,
    1604,    -1,    -1,    -1,  1608,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,   849,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,  1588,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1599,    -1,    65,    -1,    -1,  1604,    -1,    -1,    -1,  1608,
      -1,    -1,    -1,    -1,  1668,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1676,    -1,    -1,    -1,    -1,  1258,    -1,  1260,
      -1,    -1,    -1,  1632,  1265,    -1,    -1,  1691,  1269,   718,
    1271,    -1,    -1,  1274,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1282,    -1,   732,   733,   734,   735,    -1,    -1,    -1,
      -1,    -1,  1293,   742,    -1,    -1,   948,    -1,    -1,  1668,
      -1,    -1,   954,   955,   956,   957,   958,   959,   960,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,   980,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,  1349,    53,
      -1,    -1,    -1,    -1,  1006,    -1,    -1,  1358,    -1,    -1,
      -1,    65,    -1,    -1,  1365,    -1,    -1,   200,    77,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,  1389,    -1,
    1391,    -1,    -1,    -1,    -1,    -1,  1397,    -1,    -1,    -1,
     849,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,  1432,  1433,    -1,    -1,    -1,    -1,    65,    -1,  1440,
      -1,    -1,    -1,    -1,    -1,   154,  1447,   156,   157,    -1,
     159,   160,   161,    -1,    -1,  1107,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1122,    -1,  1124,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,    -1,    -1,  1140,    -1,
      -1,    -1,   201,    -1,   203,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   954,   955,   956,   957,   958,
     959,   960,   961,   962,   963,   964,   965,   966,   967,   968,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   980,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1548,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1006,    -1,    -1,
      -1,    -1,    -1,    -1,  1565,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   200,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1583,    -1,    -1,    -1,    -1,  1588,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1599,    -1,
      -1,    -1,    -1,  1604,    -1,    -1,    -1,  1608,    -1,    10,
      11,    12,    -1,  1265,    -1,    -1,    -1,  1269,    -1,  1271,
      -1,    -1,  1274,    -1,    -1,    -1,    -1,    -1,    29,  1630,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,  1107,    77,
      -1,    79,    -1,    -1,    65,    -1,    -1,  1668,    -1,    -1,
      -1,    -1,    -1,  1122,    -1,  1124,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1140,    -1,    -1,    10,    11,    12,  1349,    -1,  1700,
      -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,
      -1,    -1,    -1,    29,  1715,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,    -1,    -1,    -1,
    1432,  1433,    -1,   201,    -1,   203,    -1,    -1,  1440,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   200,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,  1265,    -1,    -1,    -1,
    1269,    -1,  1271,    -1,    -1,  1274,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    79,    65,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   200,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1349,    -1,    -1,   118,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    -1,   150,  1588,    -1,   153,   154,
      65,   156,   157,    -1,   159,   160,   161,  1599,    -1,    -1,
      -1,    -1,  1604,    -1,    -1,    -1,  1608,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      -1,    -1,    -1,  1432,  1433,    -1,    -1,    -1,   203,    -1,
      -1,  1440,    -1,    -1,    -1,     3,     4,     5,     6,     7,
     200,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1668,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,   200,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    -1,    -1,    -1,   102,   103,   104,   105,   106,   107,
     108,    -1,   110,   111,   112,   113,   114,   115,   116,   117,
     118,    -1,   120,   121,   122,   123,   124,   125,    -1,    -1,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,  1588,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
    1599,   159,   160,   161,   162,  1604,    -1,   165,    -1,  1608,
     168,    -1,    -1,    -1,    -1,    -1,   174,   175,    -1,   177,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,   200,   201,   202,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1668,
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
     138,    -1,    77,    -1,    79,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,   165,    -1,    -1,
     168,    -1,   185,    -1,    -1,    -1,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,    -1,    -1,    -1,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    10,    11,    12,
      13,   156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      53,    -1,    -1,    -1,    -1,    -1,   201,    -1,   203,    -1,
      -1,    -1,    -1,    -1,    67,    68,    69,    70,    71,    72,
      73,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,    -1,    -1,    -1,   131,   132,
      -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   150,   151,   152,
      -1,    -1,    -1,   156,   157,    -1,   159,   160,   161,   162,
      -1,   164,   165,    -1,   167,    -1,    -1,    -1,    -1,    -1,
      -1,   174,   175,    -1,   177,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    29,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    55,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    29,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,   199,    53,    -1,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,   132,
     133,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   150,    -1,    -1,
     153,   154,    29,   156,   157,   199,   159,   160,   161,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,    -1,    -1,    -1,    -1,    55,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   132,   133,   194,   198,    -1,    -1,    -1,   202,
      77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     150,    -1,    -1,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,
      -1,    -1,    -1,    -1,   174,    -1,   113,   114,   115,   116,
     117,   118,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   132,   133,    -1,   198,    -1,
      -1,    -1,   202,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    -1,   150,    -1,    -1,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    55,   174,    -1,    -1,
      -1,    -1,    -1,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    77,    -1,
      -1,   198,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   150,    -1,    -1,   153,   154,    29,   156,   157,    -1,
     159,   160,   161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,    -1,
      -1,    -1,    55,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   132,   133,    -1,   198,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   150,    -1,    -1,   153,   154,    29,
     156,   157,    -1,   159,   160,   161,    -1,   163,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,
      -1,    -1,    -1,    -1,    -1,    55,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   132,
     133,    -1,   198,    -1,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   150,    -1,    -1,
     153,   154,    29,   156,   157,    -1,   159,   160,   161,    -1,
     163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,    -1,    -1,    -1,    -1,    55,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   132,   133,    -1,   198,    -1,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     150,    -1,    -1,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   174,   175,    -1,    -1,    -1,    -1,
      -1,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   132,   133,    77,   198,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   150,    -1,    -1,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,   118,    -1,
      -1,    -1,    30,    -1,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    46,    47,
      -1,   198,    -1,    -1,    52,    -1,    54,    -1,    -1,    -1,
     150,    -1,    -1,   153,   154,    -1,   156,   157,    66,   159,
     160,   161,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    -1,    -1,    -1,   198,    -1,
      -1,   201,    -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    77,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,
      -1,    -1,   150,   151,   152,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,   118,   165,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,   131,
      -1,   179,    -1,    77,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,    -1,   150,    -1,
     198,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    65,    46,    47,   198,    -1,    -1,    -1,
      52,   203,    54,    -1,    -1,    -1,   150,    -1,    -1,   153,
      -1,    -1,   156,   157,    66,   159,   160,   161,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   202,    -1,
      -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     132,    -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,
      -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
     122,    -1,    -1,   165,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   174,    46,    47,    -1,    -1,   179,    -1,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    66,   156,   157,   198,   159,   160,   161,
      -1,    74,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    -1,    -1,    -1,   198,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    79,    -1,    -1,    -1,   132,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   105,    -1,   156,   157,    -1,   159,   160,   161,   113,
     114,   115,   116,   117,   118,    68,    -1,    -1,    -1,    -1,
      -1,   174,    -1,    -1,    77,    -1,    79,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,    -1,    -1,    -1,   150,    -1,    -1,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,
      -1,    -1,    -1,    -1,   168,   118,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   180,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,   198,    77,    -1,   150,    -1,   203,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      77,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   105,   106,    -1,    -1,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,    -1,    -1,   198,    -1,    -1,    -1,    -1,
     203,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   131,    -1,    -1,    -1,    -1,    -1,
      -1,   153,    -1,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    -1,   150,    -1,    -1,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    77,    -1,    79,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    -1,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,    -1,
      -1,   198,    -1,    -1,    -1,    -1,   203,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    79,
     131,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,   150,
      53,    -1,   153,   154,    -1,   156,   157,    -1,   159,   160,
     161,    -1,    65,    -1,    -1,    -1,    -1,    -1,   118,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,
      79,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    -1,    -1,    -1,   198,    -1,    -1,
     150,    -1,   203,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,
      -1,    -1,    -1,    77,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    -1,    -1,    -1,   198,    -1,
      -1,   150,    -1,   203,   153,   154,    -1,   156,   157,    -1,
     159,   160,   161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,    -1,    -1,    -1,   198,
     154,    -1,   156,   157,   203,   159,   160,   161,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,    -1,    -1,    -1,    -1,    -1,    -1,   201,   153,   203,
      -1,   156,   157,    -1,   159,   160,   161,    -1,    77,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    77,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      -1,    -1,    -1,    -1,    -1,   154,   201,   156,   157,   158,
     159,   160,   161,    -1,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,    65,   156,   157,   198,
     159,   160,   161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,    10,    11,    12,   198,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    -1,    -1,    29,   198,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,   130,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
     130,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,   130,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
     130,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,   130,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
     130,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    74,    75,    76,    77,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,   130,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,
     130,    -1,    77,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    -1,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    65,   119,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   130,   132,   133,   150,
      -1,    77,   153,    79,    80,   156,   157,    -1,   159,   160,
     161,    -1,    -1,    -1,    -1,   150,    -1,    -1,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    77,    -1,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     150,    77,    -1,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,
      -1,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    77,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,    77,
      -1,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,
     156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    77,    -1,    -1,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,    -1,
      -1,   153,    -1,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    -1,    -1,    -1,   153,    -1,    -1,   156,   157,
      -1,   159,   160,   161,    77,    -1,    -1,    -1,    -1,   124,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    77,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,    -1,    -1,    -1,
      -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,
      -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      -1,    -1,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   156,   157,    -1,   159,   160,   161,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,    10,    11,    12,    -1,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,    28,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      97,    53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65
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
     254,   259,   265,   324,   325,   333,   337,   338,   339,   340,
     341,   342,   343,   344,   346,   349,   361,   362,   363,   365,
     366,   368,   387,   397,   398,   399,   404,   407,   426,   434,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     447,   460,   462,   464,   116,   117,   118,   131,   150,   160,
     215,   246,   324,   343,   436,   343,   198,   343,   343,   343,
     102,   343,   343,   424,   425,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,    79,   118,   198,
     223,   398,   399,   436,   439,   436,    35,   343,   451,   452,
     343,   118,   198,   223,   398,   399,   400,   435,   443,   448,
     449,   198,   334,   401,   198,   334,   350,   335,   343,   232,
     334,   198,   198,   198,   334,   200,   343,   215,   200,   343,
      29,    55,   132,   133,   154,   174,   198,   215,   226,   465,
     477,   478,   480,   181,   200,   340,   343,   367,   369,   201,
     239,   343,   105,   106,   153,   216,   219,   222,    79,   203,
     291,   292,   117,   124,   116,   124,    79,   293,   198,   198,
     198,   198,   215,   263,   466,   198,   198,    79,    85,   146,
     147,   148,   457,   458,   153,   201,   222,   222,   215,   264,
     466,   154,   198,   466,   466,    79,   195,   201,   352,   333,
     343,   344,   436,   440,   228,   201,    85,   402,   457,    85,
     457,   457,    30,   153,   170,   467,   198,     9,   200,    35,
     245,   154,   262,   466,   118,   180,   246,   325,   200,   200,
     200,   200,   200,   200,    10,    11,    12,    29,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    53,
      65,   200,    66,    66,   200,   201,   149,   125,   160,   162,
     265,   323,   324,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    63,    64,   128,   129,
     428,    66,   201,   433,   198,   198,    66,   201,   203,   444,
     198,   245,   246,    14,   343,   200,   130,    44,   215,   423,
     198,   333,   436,   440,   149,   436,   130,   205,     9,   409,
     333,   436,   467,   149,   198,   403,   428,   433,   199,   343,
      30,   230,     8,   355,     9,   200,   230,   231,   335,   336,
     343,   215,   277,   234,   200,   200,   200,   480,   480,   170,
     198,   105,   480,    14,   149,   215,    79,   200,   200,   200,
     181,   182,   183,   188,   189,   192,   193,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   382,   383,   384,   240,
     109,   167,   200,   153,   217,   220,   222,   153,   218,   221,
     222,   222,     9,   200,    97,   201,   436,     9,   200,   124,
     124,    14,     9,   200,   436,   461,   461,   333,   344,   436,
     439,   440,   199,   170,   257,   131,   436,   450,   451,    66,
     428,   146,   458,    78,   343,   436,    85,   146,   458,   222,
     214,   200,   201,   252,   260,   388,   390,    86,   203,   356,
     357,   359,   399,   444,   462,    14,    97,   463,   351,   353,
     354,   287,   288,   426,   427,   199,   199,   199,   199,   202,
     229,   230,   247,   254,   259,   426,   343,   204,   206,   207,
     215,   468,   469,   480,    35,   163,   289,   290,   343,   465,
     198,   466,   255,   245,   343,   343,   343,    30,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     400,   343,   343,   446,   446,   343,   453,   454,   124,   201,
     215,   443,   444,   263,   215,   264,   262,   246,    27,    35,
     337,   340,   343,   367,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   154,   201,   215,   429,
     430,   431,   432,   443,   446,   343,   289,   289,   446,   343,
     450,   245,   199,   343,   198,   422,     9,   409,   333,   199,
     215,    35,   343,    35,   343,   199,   199,   443,   289,   201,
     215,   429,   430,   443,   199,   228,   281,   201,   340,   343,
     343,    89,    30,   230,   275,   200,    28,    97,    14,     9,
     199,    30,   201,   278,   480,    86,   226,   474,   475,   476,
     198,     9,    46,    47,    52,    54,    66,    86,   132,   145,
     154,   174,   198,   223,   224,   226,   364,   398,   404,   405,
     406,   215,   479,   184,    79,   343,    79,    79,   343,   379,
     380,   343,   343,   372,   382,   187,   385,   228,   198,   238,
     222,   200,     9,    97,   222,   200,     9,    97,    97,   219,
     215,   343,   292,   405,    79,     9,   199,   199,   199,   199,
     199,   199,   199,   200,    46,    47,   472,   473,   126,   268,
     198,     9,   199,   199,    79,    80,   215,   459,   215,    66,
     202,   202,   211,   213,    30,   127,   267,   169,    50,   154,
     169,   392,   130,     9,   409,   199,   149,   480,   480,    14,
     355,   287,   228,   196,     9,   410,   480,   481,   428,   433,
     202,     9,   409,   171,   436,   343,   199,     9,   410,    14,
     347,   248,   126,   266,   198,   466,   343,    30,   205,   205,
     130,   202,     9,   409,   343,   467,   198,   258,   253,   261,
     256,   245,    68,   436,   343,   467,   205,   202,   199,   199,
     205,   202,   199,    46,    47,    66,    74,    75,    76,    86,
     132,   145,   174,   215,   412,   414,   415,   418,   421,   215,
     436,   436,   130,   428,   433,   199,   343,   282,    71,    72,
     283,   228,   334,   228,   336,    97,    35,   131,   272,   436,
     405,   215,    30,   230,   276,   200,   279,   200,   279,     9,
     171,   130,   149,     9,   409,   199,   163,   468,   469,   470,
     468,   405,   405,   405,   405,   405,   408,   411,   198,    85,
     149,   198,   405,   149,   201,    10,    11,    12,    29,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      65,   149,   467,   343,   184,   184,    14,   190,   191,   381,
       9,   194,   385,    79,   202,   398,   201,   242,    97,   220,
     215,    97,   221,   215,   215,   202,    14,   436,   200,     9,
     171,   215,   269,   398,   201,   450,   131,   436,    14,   205,
     343,   202,   211,   480,   269,   201,   391,    14,   343,   356,
     215,   200,   480,   196,   202,    30,   471,   427,    35,    79,
     163,   429,   430,   432,   480,    35,   163,   343,   405,   287,
     198,   398,   267,   348,   249,   343,   343,   343,   202,   198,
     289,   268,    30,   267,   266,   466,   400,   202,   198,    14,
      74,    75,    76,   215,   413,   413,   415,   416,   417,    48,
     198,    85,   146,   198,     9,   409,   199,   422,    35,   343,
     429,   430,   202,    71,    72,   284,   334,   230,   202,   200,
      90,   200,   272,   436,   198,   130,   271,    14,   228,   279,
      99,   100,   101,   279,   202,   480,   480,   215,   474,     9,
     199,   409,   130,   205,     9,   409,   408,   215,   356,   358,
     360,   199,   124,   215,   405,   455,   456,   405,   405,   405,
      30,   405,   405,   405,   405,   405,   405,   405,   405,   405,
     405,   405,   405,   405,   405,   405,   405,   405,   405,   405,
     405,   405,   405,   405,   479,   343,   343,   343,   380,   343,
     370,    79,   243,   215,   215,   405,   473,    97,     9,   297,
     199,   198,   337,   340,   343,   205,   202,   463,   297,   155,
     168,   201,   387,   394,   155,   201,   393,   130,   200,   471,
     480,   355,   481,    79,   163,    14,    79,   467,   436,   343,
     199,   287,   201,   287,   198,   130,   198,   289,   199,   201,
     480,   201,   267,   250,   403,   289,   130,   205,     9,   409,
     414,   416,   146,   356,   419,   420,   415,   436,   334,    30,
      73,   230,   200,   336,   271,   450,   272,   199,   405,    96,
      99,   200,   343,    30,   200,   280,   202,   171,   130,   163,
      30,   199,   405,   405,   199,   130,     9,   409,   199,   130,
     202,     9,   409,   405,    30,   185,   199,   228,   215,   480,
     398,     4,   106,   111,   117,   119,   156,   157,   159,   202,
     298,   322,   323,   324,   329,   330,   331,   332,   426,   450,
     202,   201,   202,    50,   343,   343,   343,   355,    35,    79,
     163,    14,    79,   343,   198,   471,   199,   297,   199,   287,
     343,   289,   199,   297,   463,   297,   201,   198,   199,   415,
     415,   199,   130,   199,     9,   409,    30,   228,   200,   199,
     199,   199,   235,   200,   200,   280,   228,   480,   480,   130,
     405,   356,   405,   405,   405,   343,   201,   202,    97,   126,
     127,   175,   465,   270,   398,   106,   332,    29,   119,   132,
     133,   154,   160,   307,   308,   309,   310,   398,   158,   314,
     315,   122,   198,   215,   316,   317,   299,   246,   480,     9,
     200,     9,   200,   200,   463,   323,   199,   294,   154,   389,
     202,   202,    79,   163,    14,    79,   343,   289,   111,   345,
     471,   202,   471,   199,   199,   202,   201,   202,   297,   287,
     130,   415,   356,   228,   233,   236,    30,   230,   274,   228,
     199,   405,   130,   130,   186,   228,   480,   398,   398,   466,
      14,     9,   200,   201,   465,   463,   310,   170,   201,     9,
     200,     3,     4,     5,     6,     7,    10,    11,    12,    13,
      27,    28,    53,    67,    68,    69,    70,    71,    72,    73,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   131,   132,   134,   135,   136,   137,   138,   150,   151,
     152,   162,   164,   165,   167,   174,   175,   177,   179,   180,
     215,   395,   396,     9,   200,   154,   158,   215,   317,   318,
     319,   200,    79,   328,   245,   300,   465,   465,    14,   246,
     202,   295,   296,   465,    14,    79,   343,   199,   198,   201,
     200,   201,   320,   345,   471,   294,   202,   199,   415,   130,
      30,   230,   273,   274,   228,   405,   405,   343,   202,   200,
     200,   405,   398,   303,   480,   311,   404,   308,    14,    30,
      47,   312,   315,     9,    33,   199,    29,    46,    49,    14,
       9,   200,   466,   328,    14,   480,   245,   200,    14,   343,
      35,    79,   386,   228,   228,   201,   320,   202,   471,   415,
     228,    94,   187,   241,   202,   215,   226,   304,   305,   306,
       9,   171,     9,   202,   405,   396,   396,    55,   313,   318,
     318,    29,    46,    49,   405,    79,   198,   200,   405,   466,
     405,    79,     9,   410,   202,   202,   228,   320,    92,   200,
      79,   109,   237,   149,    97,   480,   404,   161,    14,   301,
     198,    35,    79,   199,   202,   200,   198,   167,   244,   215,
     323,   324,   171,   405,   285,   286,   427,   302,    79,   398,
     242,   164,   215,   200,   199,     9,   410,   113,   114,   115,
     326,   327,   285,    79,   270,   200,   471,   427,   481,   199,
     199,   200,   200,   201,   321,   326,    35,    79,   163,   471,
     201,   228,   481,    79,   163,    14,    79,   321,   228,   202,
      35,    79,   163,    14,    79,   343,   202,    79,   163,    14,
      79,   343,    14,    79,   343,   343
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
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1589 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1591 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1593 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1594 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1600 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1603 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1607 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1618 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1619 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1623 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1626 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1631 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1636 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1637 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1639 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1643 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1644 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1645 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1646 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1650 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1651 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1652 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1654 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1658 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1662 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1665 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1666 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
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
#line 1675 "hphp.y"
    { (yyval).reset();;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1676 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1680 "hphp.y"
    { (yyval).reset();;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1683 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1684 "hphp.y"
    { (yyval).reset();;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1687 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1689 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1693 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1694 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1695 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1696 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1697 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1698 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1702 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1703 "hphp.y"
    { (yyval).reset();;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1708 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1714 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1716 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1722 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1726 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1728 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1732 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1736 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1739 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1745 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1748 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1751 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1761 "hphp.y"
    { (yyval).reset();;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1852 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1858 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1864 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1867 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1868 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1869 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1871 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1872 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1873 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1874 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1875 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { (yyval).reset();;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1888 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(7) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(8) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1941 "hphp.y"
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

  case 461:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1966 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1974 "hphp.y"
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

  case 465:

/* Line 1455 of yacc.c  */
#line 1984 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1990 "hphp.y"
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

  case 467:

/* Line 1455 of yacc.c  */
#line 2004 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 2005 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 2007 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 2011 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 2013 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 2020 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 2023 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 2030 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 2033 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 2038 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 2039 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 2044 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 2045 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 2049 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2053 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2054 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2059 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2066 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2073 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2075 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2079 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2080 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2081 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2082 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2084 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2088 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2092 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2096 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2101 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2103 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2105 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2107 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2111 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2117 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2118 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2120 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2121 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2126 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2139 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2148 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2153 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
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
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2168 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2176 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2184 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2185 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2186 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2187 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2194 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 529:

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

  case 530:

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

  case 531:

/* Line 1455 of yacc.c  */
#line 2219 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { (yyval).reset();;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { (yyval).reset();;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 540:

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

  case 541:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2332 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2337 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2338 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2355 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { (yyval).reset();;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { (yyval).reset();;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { (yyval).reset();;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { (yyval).reset();;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2459 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2466 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2476 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2479 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2481 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2488 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2489 "hphp.y"
    { (yyval).reset();;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2493 "hphp.y"
    { (yyval).reset();;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2494 "hphp.y"
    { (yyval).reset();;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2497 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2498 "hphp.y"
    { (yyval).reset();;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2506 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2513 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2518 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2520 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2523 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2524 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2526 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2540 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2541 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2545 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2553 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { (yyval).reset();;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2559 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2564 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2569 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2574 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2580 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2583 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2588 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2589 "hphp.y"
    { (yyval).reset();;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2593 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2610 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2613 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { (yyval).reset();;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2618 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2619 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2624 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2629 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2630 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2634 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2635 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2639 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2640 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2644 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2645 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2649 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2656 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2662 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2663 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2664 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2665 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2666 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
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

  case 782:

/* Line 1455 of yacc.c  */
#line 2679 "hphp.y"
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

  case 783:

/* Line 1455 of yacc.c  */
#line 2690 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2692 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2694 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2695 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2699 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2700 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2701 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2702 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2705 "hphp.y"
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

  case 792:

/* Line 1455 of yacc.c  */
#line 2717 "hphp.y"
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

  case 793:

/* Line 1455 of yacc.c  */
#line 2727 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2728 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2732 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2733 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2738 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2742 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2743 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2749 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2753 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2760 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2764 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2774 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2779 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2780 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2781 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2784 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2785 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2789 "hphp.y"
    { (yyval).reset();;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2793 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2794 "hphp.y"
    { (yyval)++;;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2802 "hphp.y"
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

  case 820:

/* Line 1455 of yacc.c  */
#line 2813 "hphp.y"
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

  case 821:

/* Line 1455 of yacc.c  */
#line 2824 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2825 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2829 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2832 "hphp.y"
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

  case 826:

/* Line 1455 of yacc.c  */
#line 2844 "hphp.y"
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

  case 827:

/* Line 1455 of yacc.c  */
#line 2853 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2857 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2858 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2860 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2861 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2862 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2863 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2868 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2869 "hphp.y"
    { (yyval).reset();;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2873 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2874 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2875 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2876 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2879 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2881 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2882 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2883 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2888 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2889 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2893 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2894 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2895 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2896 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2901 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2902 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2907 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2909 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2911 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2912 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2916 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2918 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2919 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2926 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2928 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2930 "hphp.y"
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

  case 863:

/* Line 1455 of yacc.c  */
#line 2940 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2942 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2946 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2947 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2948 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2952 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2953 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2954 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2955 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2956 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2957 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2958 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2959 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2961 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2962 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2966 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2967 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2972 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2974 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2992 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2998 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2999 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 3005 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 3009 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 3015 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 3016 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 3020 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 3023 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 3029 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 3034 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 3035 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 3036 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 3037 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 3041 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3042 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3047 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3048 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3051 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (6)]).text()); ;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 3053 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (4)]).text()); ;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 3057 "hphp.y"
    {;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3058 "hphp.y"
    {;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3059 "hphp.y"
    {;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3065 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3070 "hphp.y"
    { ;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3082 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3084 "hphp.y"
    {;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3088 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3092 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3095 "hphp.y"
    { Token t; t.reset();
                                          _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                          _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3098 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3105 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3108 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3111 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3112 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3115 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3118 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3121 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3125 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3128 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3131 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3143 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3151 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3152 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13731 "hphp.tab.cpp"
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
#line 3155 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

