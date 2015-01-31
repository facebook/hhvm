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
#define YYLAST   17138

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  211
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  285
/* YYNRULES -- Number of rules.  */
#define YYNRULES  958
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1802

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
    1605,  1608,  1611,  1614,  1616,  1618,  1620,  1622,  1624,  1628,
    1631,  1633,  1639,  1640,  1641,  1653,  1654,  1667,  1668,  1672,
    1673,  1678,  1679,  1686,  1687,  1695,  1696,  1702,  1705,  1708,
    1713,  1715,  1717,  1723,  1727,  1733,  1737,  1740,  1741,  1744,
    1745,  1750,  1755,  1759,  1764,  1769,  1774,  1779,  1784,  1789,
    1794,  1799,  1804,  1809,  1811,  1813,  1815,  1817,  1821,  1824,
    1828,  1833,  1836,  1840,  1842,  1845,  1847,  1850,  1852,  1854,
    1856,  1858,  1860,  1862,  1867,  1872,  1875,  1884,  1895,  1898,
    1900,  1904,  1906,  1909,  1911,  1913,  1915,  1917,  1920,  1925,
    1929,  1933,  1938,  1940,  1943,  1948,  1951,  1958,  1959,  1961,
    1966,  1967,  1970,  1971,  1973,  1975,  1979,  1981,  1985,  1987,
    1989,  1993,  1997,  1999,  2001,  2003,  2005,  2007,  2009,  2011,
    2013,  2015,  2017,  2019,  2021,  2023,  2025,  2027,  2029,  2031,
    2033,  2035,  2037,  2039,  2041,  2043,  2045,  2047,  2049,  2051,
    2053,  2055,  2057,  2059,  2061,  2063,  2065,  2067,  2069,  2071,
    2073,  2075,  2077,  2079,  2081,  2083,  2085,  2087,  2089,  2091,
    2093,  2095,  2097,  2099,  2101,  2103,  2105,  2107,  2109,  2111,
    2113,  2115,  2117,  2119,  2121,  2123,  2125,  2127,  2129,  2131,
    2133,  2135,  2137,  2139,  2141,  2143,  2145,  2147,  2149,  2151,
    2153,  2155,  2157,  2162,  2164,  2166,  2168,  2170,  2172,  2174,
    2176,  2178,  2181,  2183,  2184,  2185,  2187,  2189,  2193,  2194,
    2196,  2198,  2200,  2202,  2204,  2206,  2208,  2210,  2212,  2214,
    2216,  2218,  2220,  2224,  2227,  2229,  2231,  2236,  2240,  2245,
    2247,  2249,  2251,  2253,  2257,  2261,  2265,  2269,  2273,  2277,
    2281,  2285,  2289,  2293,  2297,  2301,  2305,  2309,  2313,  2317,
    2321,  2325,  2328,  2331,  2334,  2337,  2341,  2345,  2349,  2353,
    2357,  2361,  2365,  2369,  2375,  2380,  2384,  2388,  2392,  2394,
    2396,  2398,  2400,  2404,  2408,  2412,  2415,  2416,  2418,  2419,
    2421,  2422,  2428,  2432,  2436,  2438,  2440,  2442,  2444,  2448,
    2451,  2453,  2455,  2457,  2459,  2461,  2465,  2467,  2469,  2471,
    2474,  2477,  2482,  2486,  2491,  2494,  2495,  2501,  2505,  2509,
    2511,  2515,  2517,  2520,  2521,  2527,  2531,  2534,  2535,  2539,
    2540,  2545,  2548,  2549,  2553,  2557,  2559,  2560,  2562,  2564,
    2566,  2568,  2572,  2574,  2576,  2578,  2582,  2584,  2586,  2590,
    2594,  2597,  2602,  2605,  2610,  2612,  2614,  2616,  2618,  2620,
    2624,  2630,  2634,  2639,  2644,  2648,  2650,  2652,  2654,  2656,
    2660,  2666,  2671,  2675,  2677,  2679,  2683,  2687,  2689,  2691,
    2699,  2709,  2717,  2724,  2733,  2735,  2738,  2743,  2748,  2750,
    2752,  2757,  2759,  2760,  2762,  2765,  2767,  2769,  2773,  2779,
    2783,  2787,  2788,  2790,  2794,  2800,  2804,  2807,  2811,  2818,
    2819,  2821,  2826,  2829,  2830,  2836,  2840,  2844,  2846,  2853,
    2858,  2863,  2866,  2869,  2870,  2876,  2880,  2884,  2886,  2889,
    2890,  2896,  2900,  2904,  2906,  2909,  2910,  2913,  2914,  2920,
    2924,  2928,  2930,  2933,  2934,  2937,  2938,  2944,  2948,  2952,
    2954,  2957,  2960,  2962,  2965,  2967,  2972,  2976,  2980,  2987,
    2991,  2993,  2995,  2997,  3002,  3007,  3012,  3017,  3022,  3027,
    3030,  3033,  3038,  3041,  3044,  3046,  3050,  3054,  3058,  3059,
    3062,  3068,  3075,  3077,  3080,  3082,  3087,  3091,  3092,  3094,
    3098,  3101,  3105,  3107,  3109,  3110,  3111,  3114,  3119,  3122,
    3129,  3134,  3136,  3138,  3139,  3143,  3149,  3153,  3155,  3158,
    3159,  3164,  3166,  3168,  3172,  3175,  3178,  3181,  3183,  3185,
    3187,  3189,  3193,  3198,  3205,  3207,  3216,  3223,  3225
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     212,     0,    -1,    -1,   213,   214,    -1,   214,   215,    -1,
      -1,   233,    -1,   250,    -1,   257,    -1,   254,    -1,   262,
      -1,   478,    -1,   123,   201,   202,   203,    -1,   150,   225,
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
     481,    -1,   226,   481,    -1,   230,     9,   479,    14,   411,
      -1,   106,   479,    14,   411,    -1,   231,   232,    -1,    -1,
     233,    -1,   250,    -1,   257,    -1,   262,    -1,   204,   231,
     205,    -1,    70,   336,   233,   284,   286,    -1,    70,   336,
      30,   231,   285,   287,    73,   203,    -1,    -1,    89,   336,
     234,   278,    -1,    -1,    88,   235,   233,    89,   336,   203,
      -1,    -1,    91,   201,   338,   203,   338,   203,   338,   202,
     236,   276,    -1,    -1,    98,   336,   237,   281,    -1,   102,
     203,    -1,   102,   345,   203,    -1,   104,   203,    -1,   104,
     345,   203,    -1,   107,   203,    -1,   107,   345,   203,    -1,
      27,   102,   203,    -1,   112,   294,   203,    -1,   118,   296,
     203,    -1,    87,   337,   203,    -1,   120,   201,   475,   202,
     203,    -1,   203,    -1,    81,    -1,    82,    -1,    -1,    93,
     201,   345,    97,   275,   274,   202,   238,   277,    -1,    -1,
      93,   201,   345,    28,    97,   275,   274,   202,   239,   277,
      -1,    95,   201,   280,   202,   279,    -1,    -1,   108,   242,
     109,   201,   404,    79,   202,   204,   231,   205,   244,   240,
     247,    -1,    -1,   108,   242,   167,   241,   245,    -1,   110,
     345,   203,    -1,   103,   218,   203,    -1,   345,   203,    -1,
     339,   203,    -1,   340,   203,    -1,   341,   203,    -1,   342,
     203,    -1,   343,   203,    -1,   107,   342,   203,    -1,   344,
     203,    -1,   374,   203,    -1,   107,   373,   203,    -1,   218,
      30,    -1,    -1,   204,   243,   231,   205,    -1,   244,   109,
     201,   404,    79,   202,   204,   231,   205,    -1,    -1,    -1,
     204,   246,   231,   205,    -1,   167,   245,    -1,    -1,    35,
      -1,    -1,   105,    -1,    -1,   249,   248,   480,   251,   201,
     290,   202,   485,   322,    -1,    -1,   326,   249,   248,   480,
     252,   201,   290,   202,   485,   322,    -1,    -1,   432,   325,
     249,   248,   480,   253,   201,   290,   202,   485,   322,    -1,
      -1,   160,   218,   255,    30,   494,   477,   204,   297,   205,
      -1,    -1,   432,   160,   218,   256,    30,   494,   477,   204,
     297,   205,    -1,    -1,   268,   265,   258,   269,   270,   204,
     300,   205,    -1,    -1,   432,   268,   265,   259,   269,   270,
     204,   300,   205,    -1,    -1,   125,   266,   260,   271,   204,
     300,   205,    -1,    -1,   432,   125,   266,   261,   271,   204,
     300,   205,    -1,    -1,   162,   267,   263,   270,   204,   300,
     205,    -1,    -1,   432,   162,   267,   264,   270,   204,   300,
     205,    -1,   480,    -1,   154,    -1,   480,    -1,   480,    -1,
     124,    -1,   117,   124,    -1,   117,   116,   124,    -1,   116,
     117,   124,    -1,   116,   124,    -1,   126,   404,    -1,    -1,
     127,   272,    -1,    -1,   126,   272,    -1,    -1,   404,    -1,
     272,     9,   404,    -1,   404,    -1,   273,     9,   404,    -1,
     130,   275,    -1,    -1,   442,    -1,    35,   442,    -1,   131,
     201,   456,   202,    -1,   233,    -1,    30,   231,    92,   203,
      -1,   233,    -1,    30,   231,    94,   203,    -1,   233,    -1,
      30,   231,    90,   203,    -1,   233,    -1,    30,   231,    96,
     203,    -1,   218,    14,   411,    -1,   280,     9,   218,    14,
     411,    -1,   204,   282,   205,    -1,   204,   203,   282,   205,
      -1,    30,   282,    99,   203,    -1,    30,   203,   282,    99,
     203,    -1,   282,   100,   345,   283,   231,    -1,   282,   101,
     283,   231,    -1,    -1,    30,    -1,   203,    -1,   284,    71,
     336,   233,    -1,    -1,   285,    71,   336,    30,   231,    -1,
      -1,    72,   233,    -1,    -1,    72,    30,   231,    -1,    -1,
     289,     9,   433,   328,   495,   163,    79,    -1,   289,     9,
     433,   328,   495,    35,   163,    79,    -1,   289,     9,   433,
     328,   495,   163,    -1,   289,   416,    -1,   433,   328,   495,
     163,    79,    -1,   433,   328,   495,    35,   163,    79,    -1,
     433,   328,   495,   163,    -1,    -1,   433,   328,   495,    79,
      -1,   433,   328,   495,    35,    79,    -1,   433,   328,   495,
      35,    79,    14,   345,    -1,   433,   328,   495,    79,    14,
     345,    -1,   289,     9,   433,   328,   495,    79,    -1,   289,
       9,   433,   328,   495,    35,    79,    -1,   289,     9,   433,
     328,   495,    35,    79,    14,   345,    -1,   289,     9,   433,
     328,   495,    79,    14,   345,    -1,   291,     9,   433,   495,
     163,    79,    -1,   291,     9,   433,   495,    35,   163,    79,
      -1,   291,     9,   433,   495,   163,    -1,   291,   416,    -1,
     433,   495,   163,    79,    -1,   433,   495,    35,   163,    79,
      -1,   433,   495,   163,    -1,    -1,   433,   495,    79,    -1,
     433,   495,    35,    79,    -1,   433,   495,    35,    79,    14,
     345,    -1,   433,   495,    79,    14,   345,    -1,   291,     9,
     433,   495,    79,    -1,   291,     9,   433,   495,    35,    79,
      -1,   291,     9,   433,   495,    35,    79,    14,   345,    -1,
     291,     9,   433,   495,    79,    14,   345,    -1,   293,   416,
      -1,    -1,   345,    -1,    35,   442,    -1,   163,   345,    -1,
     293,     9,   345,    -1,   293,     9,   163,   345,    -1,   293,
       9,    35,   442,    -1,   294,     9,   295,    -1,   295,    -1,
      79,    -1,   206,   442,    -1,   206,   204,   345,   205,    -1,
     296,     9,    79,    -1,   296,     9,    79,    14,   411,    -1,
      79,    -1,    79,    14,   411,    -1,   297,   298,    -1,    -1,
     299,   203,    -1,   479,    14,   411,    -1,   300,   301,    -1,
      -1,    -1,   324,   302,   330,   203,    -1,    -1,   326,   494,
     303,   330,   203,    -1,   331,   203,    -1,   332,   203,    -1,
     333,   203,    -1,    -1,   325,   249,   248,   480,   201,   304,
     288,   202,   485,   323,    -1,    -1,   432,   325,   249,   248,
     480,   201,   305,   288,   202,   485,   323,    -1,   156,   310,
     203,    -1,   157,   316,   203,    -1,   159,   318,   203,    -1,
       4,   126,   404,   203,    -1,     4,   127,   404,   203,    -1,
     111,   273,   203,    -1,   111,   273,   204,   306,   205,    -1,
     306,   307,    -1,   306,   308,    -1,    -1,   229,   149,   218,
     164,   273,   203,    -1,   309,    97,   325,   218,   203,    -1,
     309,    97,   326,   203,    -1,   229,   149,   218,    -1,   218,
      -1,   311,    -1,   310,     9,   311,    -1,   312,   401,   314,
     315,    -1,   154,    -1,   132,    -1,   132,   170,   494,   171,
      -1,   132,   170,   494,     9,   494,   171,    -1,   404,    -1,
     119,    -1,   160,   204,   313,   205,    -1,   133,    -1,   410,
      -1,   313,     9,   410,    -1,    14,   411,    -1,    -1,    55,
     161,    -1,    -1,   317,    -1,   316,     9,   317,    -1,   158,
      -1,   319,    -1,   218,    -1,   122,    -1,   201,   320,   202,
      -1,   201,   320,   202,    49,    -1,   201,   320,   202,    29,
      -1,   201,   320,   202,    46,    -1,   319,    -1,   321,    -1,
     321,    49,    -1,   321,    29,    -1,   321,    46,    -1,   320,
       9,   320,    -1,   320,    33,   320,    -1,   218,    -1,   154,
      -1,   158,    -1,   203,    -1,   204,   231,   205,    -1,   203,
      -1,   204,   231,   205,    -1,   326,    -1,   119,    -1,   326,
      -1,    -1,   327,    -1,   326,   327,    -1,   113,    -1,   114,
      -1,   115,    -1,   118,    -1,   117,    -1,   116,    -1,   183,
      -1,   329,    -1,    -1,   113,    -1,   114,    -1,   115,    -1,
     330,     9,    79,    -1,   330,     9,    79,    14,   411,    -1,
      79,    -1,    79,    14,   411,    -1,   331,     9,   479,    14,
     411,    -1,   106,   479,    14,   411,    -1,   332,     9,   479,
      -1,   117,   106,   479,    -1,   117,   334,   477,    -1,   334,
     477,    14,   494,    -1,   106,   178,   480,    -1,   201,   335,
     202,    -1,    68,   406,   409,    -1,    67,   345,    -1,   393,
      -1,   365,    -1,   201,   345,   202,    -1,   337,     9,   345,
      -1,   345,    -1,   337,    -1,    -1,    27,    -1,    27,   345,
      -1,    27,   345,   130,   345,    -1,   442,    14,   339,    -1,
     131,   201,   456,   202,    14,   339,    -1,    28,   345,    -1,
     442,    14,   342,    -1,   131,   201,   456,   202,    14,   342,
      -1,   346,    -1,   442,    -1,   335,    -1,   446,    -1,   445,
      -1,   131,   201,   456,   202,    14,   345,    -1,   442,    14,
     345,    -1,   442,    14,    35,   442,    -1,   442,    14,    35,
      68,   406,   409,    -1,   442,    26,   345,    -1,   442,    25,
     345,    -1,   442,    24,   345,    -1,   442,    23,   345,    -1,
     442,    22,   345,    -1,   442,    21,   345,    -1,   442,    20,
     345,    -1,   442,    19,   345,    -1,   442,    18,   345,    -1,
     442,    17,   345,    -1,   442,    16,   345,    -1,   442,    15,
     345,    -1,   442,    64,    -1,    64,   442,    -1,   442,    63,
      -1,    63,   442,    -1,   345,    31,   345,    -1,   345,    32,
     345,    -1,   345,    10,   345,    -1,   345,    12,   345,    -1,
     345,    11,   345,    -1,   345,    33,   345,    -1,   345,    35,
     345,    -1,   345,    34,   345,    -1,   345,    48,   345,    -1,
     345,    46,   345,    -1,   345,    47,   345,    -1,   345,    49,
     345,    -1,   345,    50,   345,    -1,   345,    65,   345,    -1,
     345,    51,   345,    -1,   345,    45,   345,    -1,   345,    44,
     345,    -1,    46,   345,    -1,    47,   345,    -1,    52,   345,
      -1,    54,   345,    -1,   345,    37,   345,    -1,   345,    36,
     345,    -1,   345,    39,   345,    -1,   345,    38,   345,    -1,
     345,    40,   345,    -1,   345,    43,   345,    -1,   345,    41,
     345,    -1,   345,    42,   345,    -1,   345,    53,   406,    -1,
     201,   346,   202,    -1,   345,    29,   345,    30,   345,    -1,
     345,    29,    30,   345,    -1,   474,    -1,    62,   345,    -1,
      61,   345,    -1,    60,   345,    -1,    59,   345,    -1,    58,
     345,    -1,    57,   345,    -1,    56,   345,    -1,    69,   407,
      -1,    55,   345,    -1,   413,    -1,   364,    -1,   363,    -1,
     366,    -1,   367,    -1,   207,   408,   207,    -1,    13,   345,
      -1,   371,    -1,   111,   201,   392,   416,   202,    -1,    -1,
      -1,   249,   248,   201,   349,   290,   202,   485,   347,   204,
     231,   205,    -1,    -1,   326,   249,   248,   201,   350,   290,
     202,   485,   347,   204,   231,   205,    -1,    -1,    79,   352,
     357,    -1,    -1,   183,    79,   353,   357,    -1,    -1,   198,
     354,   290,   199,   485,   357,    -1,    -1,   183,   198,   355,
     290,   199,   485,   357,    -1,    -1,   183,   204,   356,   231,
     205,    -1,     8,   345,    -1,     8,   342,    -1,     8,   204,
     231,   205,    -1,    86,    -1,   476,    -1,   359,     9,   358,
     130,   345,    -1,   358,   130,   345,    -1,   360,     9,   358,
     130,   411,    -1,   358,   130,   411,    -1,   359,   415,    -1,
      -1,   360,   415,    -1,    -1,   174,   201,   361,   202,    -1,
     132,   201,   457,   202,    -1,    66,   457,   208,    -1,   404,
     204,   459,   205,    -1,   176,   201,   463,   202,    -1,   177,
     201,   463,   202,    -1,   175,   201,   464,   202,    -1,   176,
     201,   467,   202,    -1,   177,   201,   467,   202,    -1,   175,
     201,   468,   202,    -1,   404,   204,   461,   205,    -1,   371,
      66,   452,   208,    -1,   372,    66,   452,   208,    -1,   364,
      -1,   476,    -1,   445,    -1,    86,    -1,   201,   346,   202,
      -1,   375,   376,    -1,   442,    14,   373,    -1,   184,    79,
     187,   345,    -1,   377,   388,    -1,   377,   388,   391,    -1,
     388,    -1,   388,   391,    -1,   378,    -1,   377,   378,    -1,
     379,    -1,   380,    -1,   381,    -1,   382,    -1,   383,    -1,
     384,    -1,   184,    79,   187,   345,    -1,   191,    79,    14,
     345,    -1,   185,   345,    -1,   186,    79,   187,   345,   188,
     345,   189,   345,    -1,   186,    79,   187,   345,   188,   345,
     189,   345,   190,    79,    -1,   192,   385,    -1,   386,    -1,
     385,     9,   386,    -1,   345,    -1,   345,   387,    -1,   193,
      -1,   194,    -1,   389,    -1,   390,    -1,   195,   345,    -1,
     196,   345,   197,   345,    -1,   190,    79,   376,    -1,   392,
       9,    79,    -1,   392,     9,    35,    79,    -1,    79,    -1,
      35,    79,    -1,   168,   154,   394,   169,    -1,   396,    50,
      -1,   396,   169,   397,   168,    50,   395,    -1,    -1,   154,
      -1,   396,   398,    14,   399,    -1,    -1,   397,   400,    -1,
      -1,   154,    -1,   155,    -1,   204,   345,   205,    -1,   155,
      -1,   204,   345,   205,    -1,   393,    -1,   402,    -1,   401,
      30,   402,    -1,   401,    47,   402,    -1,   218,    -1,    69,
      -1,   105,    -1,   106,    -1,   107,    -1,    27,    -1,    28,
      -1,   108,    -1,   109,    -1,   167,    -1,   110,    -1,    70,
      -1,    71,    -1,    73,    -1,    72,    -1,    89,    -1,    90,
      -1,    88,    -1,    91,    -1,    92,    -1,    93,    -1,    94,
      -1,    95,    -1,    96,    -1,    53,    -1,    97,    -1,    98,
      -1,    99,    -1,   100,    -1,   101,    -1,   102,    -1,   104,
      -1,   103,    -1,    87,    -1,    13,    -1,   124,    -1,   125,
      -1,   126,    -1,   127,    -1,    68,    -1,    67,    -1,   119,
      -1,     5,    -1,     7,    -1,     6,    -1,     4,    -1,     3,
      -1,   150,    -1,   111,    -1,   112,    -1,   121,    -1,   122,
      -1,   123,    -1,   118,    -1,   117,    -1,   116,    -1,   115,
      -1,   114,    -1,   113,    -1,   183,    -1,   120,    -1,   131,
      -1,   132,    -1,    10,    -1,    12,    -1,    11,    -1,   134,
      -1,   136,    -1,   135,    -1,   137,    -1,   138,    -1,   152,
      -1,   151,    -1,   182,    -1,   162,    -1,   165,    -1,   164,
      -1,   178,    -1,   180,    -1,   174,    -1,   228,   201,   292,
     202,    -1,   229,    -1,   154,    -1,   404,    -1,   118,    -1,
     450,    -1,   404,    -1,   118,    -1,   454,    -1,   201,   202,
      -1,   336,    -1,    -1,    -1,    85,    -1,   471,    -1,   201,
     292,   202,    -1,    -1,    74,    -1,    75,    -1,    76,    -1,
      86,    -1,   137,    -1,   138,    -1,   152,    -1,   134,    -1,
     165,    -1,   135,    -1,   136,    -1,   151,    -1,   182,    -1,
     145,    85,   146,    -1,   145,   146,    -1,   410,    -1,   227,
      -1,   132,   201,   414,   202,    -1,    66,   414,   208,    -1,
     174,   201,   362,   202,    -1,   412,    -1,   370,    -1,   368,
      -1,   369,    -1,   201,   411,   202,    -1,   411,    31,   411,
      -1,   411,    32,   411,    -1,   411,    10,   411,    -1,   411,
      12,   411,    -1,   411,    11,   411,    -1,   411,    33,   411,
      -1,   411,    35,   411,    -1,   411,    34,   411,    -1,   411,
      48,   411,    -1,   411,    46,   411,    -1,   411,    47,   411,
      -1,   411,    49,   411,    -1,   411,    50,   411,    -1,   411,
      51,   411,    -1,   411,    45,   411,    -1,   411,    44,   411,
      -1,   411,    65,   411,    -1,    52,   411,    -1,    54,   411,
      -1,    46,   411,    -1,    47,   411,    -1,   411,    37,   411,
      -1,   411,    36,   411,    -1,   411,    39,   411,    -1,   411,
      38,   411,    -1,   411,    40,   411,    -1,   411,    43,   411,
      -1,   411,    41,   411,    -1,   411,    42,   411,    -1,   411,
      29,   411,    30,   411,    -1,   411,    29,    30,   411,    -1,
     229,   149,   218,    -1,   154,   149,   218,    -1,   229,   149,
     124,    -1,   227,    -1,    78,    -1,   476,    -1,   410,    -1,
     209,   471,   209,    -1,   210,   471,   210,    -1,   145,   471,
     146,    -1,   417,   415,    -1,    -1,     9,    -1,    -1,     9,
      -1,    -1,   417,     9,   411,   130,   411,    -1,   417,     9,
     411,    -1,   411,   130,   411,    -1,   411,    -1,    74,    -1,
      75,    -1,    76,    -1,   145,    85,   146,    -1,   145,   146,
      -1,    74,    -1,    75,    -1,    76,    -1,   218,    -1,    86,
      -1,    86,    48,   420,    -1,   418,    -1,   420,    -1,   218,
      -1,    46,   419,    -1,    47,   419,    -1,   132,   201,   422,
     202,    -1,    66,   422,   208,    -1,   174,   201,   425,   202,
      -1,   423,   415,    -1,    -1,   423,     9,   421,   130,   421,
      -1,   423,     9,   421,    -1,   421,   130,   421,    -1,   421,
      -1,   424,     9,   421,    -1,   421,    -1,   426,   415,    -1,
      -1,   426,     9,   358,   130,   421,    -1,   358,   130,   421,
      -1,   424,   415,    -1,    -1,   201,   427,   202,    -1,    -1,
     429,     9,   218,   428,    -1,   218,   428,    -1,    -1,   431,
     429,   415,    -1,    45,   430,    44,    -1,   432,    -1,    -1,
     128,    -1,   129,    -1,   218,    -1,   154,    -1,   204,   345,
     205,    -1,   435,    -1,   449,    -1,   218,    -1,   204,   345,
     205,    -1,   437,    -1,   449,    -1,    66,   452,   208,    -1,
     204,   345,   205,    -1,   443,   439,    -1,   201,   335,   202,
     439,    -1,   455,   439,    -1,   201,   335,   202,   439,    -1,
     449,    -1,   403,    -1,   447,    -1,   448,    -1,   440,    -1,
     442,   434,   436,    -1,   201,   335,   202,   434,   436,    -1,
     405,   149,   449,    -1,   444,   201,   292,   202,    -1,   445,
     201,   292,   202,    -1,   201,   442,   202,    -1,   403,    -1,
     447,    -1,   448,    -1,   440,    -1,   442,   434,   435,    -1,
     201,   335,   202,   434,   435,    -1,   444,   201,   292,   202,
      -1,   201,   442,   202,    -1,   449,    -1,   440,    -1,   201,
     442,   202,    -1,   201,   446,   202,    -1,   348,    -1,   351,
      -1,   442,   434,   438,   481,   201,   292,   202,    -1,   201,
     335,   202,   434,   438,   481,   201,   292,   202,    -1,   405,
     149,   218,   481,   201,   292,   202,    -1,   405,   149,   449,
     201,   292,   202,    -1,   405,   149,   204,   345,   205,   201,
     292,   202,    -1,   450,    -1,   453,   450,    -1,   450,    66,
     452,   208,    -1,   450,   204,   345,   205,    -1,   451,    -1,
      79,    -1,   206,   204,   345,   205,    -1,   345,    -1,    -1,
     206,    -1,   453,   206,    -1,   449,    -1,   441,    -1,   454,
     434,   436,    -1,   201,   335,   202,   434,   436,    -1,   405,
     149,   449,    -1,   201,   442,   202,    -1,    -1,   441,    -1,
     454,   434,   435,    -1,   201,   335,   202,   434,   435,    -1,
     201,   442,   202,    -1,   456,     9,    -1,   456,     9,   442,
      -1,   456,     9,   131,   201,   456,   202,    -1,    -1,   442,
      -1,   131,   201,   456,   202,    -1,   458,   415,    -1,    -1,
     458,     9,   345,   130,   345,    -1,   458,     9,   345,    -1,
     345,   130,   345,    -1,   345,    -1,   458,     9,   345,   130,
      35,   442,    -1,   458,     9,    35,   442,    -1,   345,   130,
      35,   442,    -1,    35,   442,    -1,   460,   415,    -1,    -1,
     460,     9,   345,   130,   345,    -1,   460,     9,   345,    -1,
     345,   130,   345,    -1,   345,    -1,   462,   415,    -1,    -1,
     462,     9,   411,   130,   411,    -1,   462,     9,   411,    -1,
     411,   130,   411,    -1,   411,    -1,   465,   415,    -1,    -1,
     466,   415,    -1,    -1,   465,     9,   345,   130,   345,    -1,
     345,   130,   345,    -1,   466,     9,   345,    -1,   345,    -1,
     469,   415,    -1,    -1,   470,   415,    -1,    -1,   469,     9,
     411,   130,   411,    -1,   411,   130,   411,    -1,   470,     9,
     411,    -1,   411,    -1,   471,   472,    -1,   471,    85,    -1,
     472,    -1,    85,   472,    -1,    79,    -1,    79,    66,   473,
     208,    -1,    79,   434,   218,    -1,   147,   345,   205,    -1,
     147,    78,    66,   345,   208,   205,    -1,   148,   442,   205,
      -1,   218,    -1,    80,    -1,    79,    -1,   121,   201,   475,
     202,    -1,   122,   201,   442,   202,    -1,   122,   201,   346,
     202,    -1,   122,   201,   446,   202,    -1,   122,   201,   445,
     202,    -1,   122,   201,   335,   202,    -1,     7,   345,    -1,
       6,   345,    -1,     5,   201,   345,   202,    -1,     4,   345,
      -1,     3,   345,    -1,   442,    -1,   475,     9,   442,    -1,
     405,   149,   218,    -1,   405,   149,   124,    -1,    -1,    97,
     494,    -1,   178,   480,    14,   494,   203,    -1,   180,   480,
     477,    14,   494,   203,    -1,   218,    -1,   494,   218,    -1,
     218,    -1,   218,   170,   486,   171,    -1,   170,   483,   171,
      -1,    -1,   494,    -1,   482,     9,   494,    -1,   482,   415,
      -1,   482,     9,   163,    -1,   483,    -1,   163,    -1,    -1,
      -1,    30,   494,    -1,   486,     9,   487,   218,    -1,   487,
     218,    -1,   486,     9,   487,   218,    97,   494,    -1,   487,
     218,    97,   494,    -1,    46,    -1,    47,    -1,    -1,    86,
     130,   494,    -1,   229,   149,   218,   130,   494,    -1,   489,
       9,   488,    -1,   488,    -1,   489,   415,    -1,    -1,   174,
     201,   490,   202,    -1,   404,    -1,   118,    -1,   218,   149,
     493,    -1,   218,   481,    -1,    29,   494,    -1,    55,   494,
      -1,   229,    -1,   132,    -1,   133,    -1,   491,    -1,   492,
     149,   493,    -1,   132,   170,   494,   171,    -1,   132,   170,
     494,     9,   494,   171,    -1,   154,    -1,   201,   105,   201,
     484,   202,    30,   494,   202,    -1,   201,   494,     9,   482,
     415,   202,    -1,   494,    -1,    -1
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
    1486,  1489,  1489,  1493,  1492,  1496,  1498,  1501,  1504,  1502,
    1517,  1514,  1527,  1529,  1531,  1533,  1535,  1537,  1539,  1543,
    1544,  1545,  1548,  1554,  1557,  1563,  1566,  1571,  1573,  1578,
    1583,  1587,  1588,  1590,  1592,  1598,  1599,  1601,  1605,  1606,
    1611,  1612,  1616,  1617,  1621,  1623,  1629,  1634,  1635,  1637,
    1641,  1642,  1643,  1644,  1648,  1649,  1650,  1651,  1652,  1653,
    1655,  1660,  1663,  1664,  1668,  1669,  1673,  1674,  1677,  1678,
    1681,  1682,  1685,  1686,  1690,  1691,  1692,  1693,  1694,  1695,
    1696,  1700,  1701,  1704,  1705,  1706,  1709,  1711,  1713,  1714,
    1717,  1719,  1723,  1725,  1729,  1733,  1737,  1741,  1742,  1744,
    1745,  1746,  1749,  1753,  1754,  1758,  1759,  1763,  1764,  1765,
    1769,  1773,  1778,  1782,  1786,  1791,  1792,  1793,  1794,  1795,
    1799,  1801,  1802,  1803,  1806,  1807,  1808,  1809,  1810,  1811,
    1812,  1813,  1814,  1815,  1816,  1817,  1818,  1819,  1820,  1821,
    1822,  1823,  1824,  1825,  1826,  1827,  1828,  1829,  1830,  1831,
    1832,  1833,  1834,  1835,  1836,  1837,  1838,  1839,  1840,  1841,
    1842,  1843,  1844,  1845,  1846,  1847,  1848,  1850,  1851,  1853,
    1855,  1856,  1857,  1858,  1859,  1860,  1861,  1862,  1863,  1864,
    1865,  1866,  1867,  1868,  1869,  1870,  1871,  1872,  1873,  1874,
    1875,  1879,  1883,  1888,  1887,  1902,  1900,  1917,  1917,  1933,
    1932,  1950,  1950,  1966,  1965,  1984,  1983,  2004,  2005,  2006,
    2011,  2013,  2017,  2021,  2027,  2031,  2037,  2039,  2043,  2045,
    2049,  2053,  2054,  2058,  2065,  2066,  2070,  2074,  2076,  2081,
    2086,  2093,  2095,  2100,  2101,  2102,  2103,  2105,  2109,  2113,
    2117,  2121,  2123,  2125,  2127,  2132,  2133,  2138,  2139,  2140,
    2141,  2142,  2143,  2147,  2151,  2155,  2159,  2164,  2169,  2173,
    2174,  2178,  2179,  2183,  2184,  2188,  2189,  2193,  2197,  2201,
    2205,  2206,  2207,  2208,  2212,  2218,  2227,  2240,  2241,  2244,
    2247,  2250,  2251,  2254,  2258,  2261,  2264,  2271,  2272,  2276,
    2277,  2279,  2283,  2284,  2285,  2286,  2287,  2288,  2289,  2290,
    2291,  2292,  2293,  2294,  2295,  2296,  2297,  2298,  2299,  2300,
    2301,  2302,  2303,  2304,  2305,  2306,  2307,  2308,  2309,  2310,
    2311,  2312,  2313,  2314,  2315,  2316,  2317,  2318,  2319,  2320,
    2321,  2322,  2323,  2324,  2325,  2326,  2327,  2328,  2329,  2330,
    2331,  2332,  2333,  2334,  2335,  2336,  2337,  2338,  2339,  2340,
    2341,  2342,  2343,  2344,  2345,  2346,  2347,  2348,  2349,  2350,
    2351,  2352,  2353,  2354,  2355,  2356,  2357,  2358,  2359,  2360,
    2361,  2362,  2366,  2371,  2372,  2375,  2376,  2377,  2381,  2382,
    2383,  2387,  2388,  2389,  2393,  2394,  2395,  2398,  2400,  2404,
    2405,  2406,  2407,  2409,  2410,  2411,  2412,  2413,  2414,  2415,
    2416,  2417,  2418,  2421,  2426,  2427,  2428,  2430,  2431,  2433,
    2434,  2435,  2436,  2437,  2438,  2440,  2442,  2444,  2446,  2448,
    2449,  2450,  2451,  2452,  2453,  2454,  2455,  2456,  2457,  2458,
    2459,  2460,  2461,  2462,  2463,  2464,  2466,  2468,  2470,  2472,
    2473,  2476,  2477,  2481,  2483,  2487,  2490,  2493,  2499,  2500,
    2501,  2502,  2503,  2504,  2505,  2510,  2512,  2516,  2517,  2520,
    2521,  2525,  2528,  2530,  2532,  2536,  2537,  2538,  2539,  2542,
    2546,  2547,  2548,  2549,  2553,  2555,  2562,  2563,  2564,  2565,
    2566,  2567,  2569,  2570,  2575,  2577,  2580,  2583,  2585,  2587,
    2590,  2592,  2596,  2598,  2601,  2604,  2610,  2612,  2615,  2616,
    2621,  2624,  2628,  2628,  2633,  2636,  2637,  2641,  2642,  2646,
    2647,  2648,  2652,  2653,  2657,  2658,  2662,  2663,  2667,  2668,
    2672,  2673,  2678,  2680,  2685,  2686,  2687,  2688,  2689,  2690,
    2692,  2695,  2698,  2700,  2702,  2706,  2707,  2708,  2709,  2710,
    2713,  2717,  2719,  2723,  2724,  2725,  2729,  2733,  2734,  2738,
    2741,  2748,  2752,  2756,  2763,  2764,  2769,  2771,  2772,  2775,
    2776,  2779,  2780,  2784,  2785,  2789,  2790,  2791,  2794,  2797,
    2800,  2803,  2804,  2805,  2808,  2812,  2816,  2817,  2818,  2820,
    2821,  2822,  2826,  2828,  2831,  2833,  2834,  2835,  2836,  2839,
    2841,  2842,  2846,  2848,  2851,  2853,  2854,  2855,  2859,  2861,
    2864,  2867,  2869,  2871,  2875,  2877,  2880,  2882,  2885,  2887,
    2890,  2891,  2895,  2897,  2900,  2902,  2905,  2908,  2912,  2914,
    2918,  2919,  2921,  2922,  2928,  2929,  2931,  2933,  2935,  2937,
    2940,  2941,  2942,  2946,  2947,  2948,  2949,  2950,  2951,  2952,
    2953,  2954,  2955,  2956,  2960,  2961,  2965,  2967,  2975,  2977,
    2981,  2985,  2992,  2993,  2999,  3000,  3007,  3010,  3014,  3017,
    3022,  3027,  3029,  3030,  3031,  3035,  3036,  3040,  3042,  3043,
    3046,  3051,  3052,  3053,  3057,  3060,  3069,  3071,  3075,  3078,
    3081,  3086,  3087,  3090,  3093,  3100,  3103,  3106,  3107,  3110,
    3113,  3114,  3121,  3124,  3128,  3132,  3138,  3148,  3149
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
  "class_abstract_constant_declaration", "class_type_constant_declaration",
  "class_type_constant", "expr_with_parens", "parenthesis_expr",
  "expr_list", "for_expr", "yield_expr", "yield_assign_expr",
  "yield_list_assign_expr", "await_expr", "await_assign_expr",
  "await_list_assign_expr", "expr", "expr_no_variable", "lambda_use_vars",
  "closure_expression", "$@29", "$@30", "lambda_expression", "$@31",
  "$@32", "$@33", "$@34", "$@35", "lambda_body", "shape_keyname",
  "non_empty_shape_pair_list", "non_empty_static_shape_pair_list",
  "shape_pair_list", "static_shape_pair_list", "shape_literal",
  "array_literal", "collection_literal", "map_array_literal",
  "varray_literal", "static_map_array_literal", "static_varray_literal",
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
  "hh_shape_type", "hh_access_type_start", "hh_access_type", "hh_type",
  "hh_type_opt", 0
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
     300,   302,   301,   303,   301,   301,   301,   301,   304,   301,
     305,   301,   301,   301,   301,   301,   301,   301,   301,   306,
     306,   306,   307,   308,   308,   309,   309,   310,   310,   311,
     311,   312,   312,   312,   312,   312,   312,   312,   313,   313,
     314,   314,   315,   315,   316,   316,   317,   318,   318,   318,
     319,   319,   319,   319,   320,   320,   320,   320,   320,   320,
     320,   321,   321,   321,   322,   322,   323,   323,   324,   324,
     325,   325,   326,   326,   327,   327,   327,   327,   327,   327,
     327,   328,   328,   329,   329,   329,   330,   330,   330,   330,
     331,   331,   332,   332,   333,   333,   334,   335,   335,   335,
     335,   335,   336,   337,   337,   338,   338,   339,   339,   339,
     340,   341,   342,   343,   344,   345,   345,   345,   345,   345,
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
     363,   364,   364,   365,   366,   366,   367,   368,   368,   369,
     370,   371,   371,   372,   372,   372,   372,   372,   373,   374,
     375,   376,   376,   376,   376,   377,   377,   378,   378,   378,
     378,   378,   378,   379,   380,   381,   382,   383,   384,   385,
     385,   386,   386,   387,   387,   388,   388,   389,   390,   391,
     392,   392,   392,   392,   393,   394,   394,   395,   395,   396,
     396,   397,   397,   398,   399,   399,   400,   400,   400,   401,
     401,   401,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   403,   404,   404,   405,   405,   405,   406,   406,
     406,   407,   407,   407,   408,   408,   408,   409,   409,   410,
     410,   410,   410,   410,   410,   410,   410,   410,   410,   410,
     410,   410,   410,   410,   411,   411,   411,   411,   411,   411,
     411,   411,   411,   411,   411,   411,   411,   411,   411,   411,
     411,   411,   411,   411,   411,   411,   411,   411,   411,   411,
     411,   411,   411,   411,   411,   411,   411,   411,   411,   411,
     411,   411,   411,   411,   411,   412,   412,   412,   413,   413,
     413,   413,   413,   413,   413,   414,   414,   415,   415,   416,
     416,   417,   417,   417,   417,   418,   418,   418,   418,   418,
     419,   419,   419,   419,   420,   420,   421,   421,   421,   421,
     421,   421,   421,   421,   422,   422,   423,   423,   423,   423,
     424,   424,   425,   425,   426,   426,   427,   427,   428,   428,
     429,   429,   431,   430,   432,   433,   433,   434,   434,   435,
     435,   435,   436,   436,   437,   437,   438,   438,   439,   439,
     440,   440,   441,   441,   442,   442,   442,   442,   442,   442,
     442,   442,   442,   442,   442,   443,   443,   443,   443,   443,
     443,   443,   443,   444,   444,   444,   445,   446,   446,   447,
     447,   448,   448,   448,   449,   449,   450,   450,   450,   451,
     451,   452,   452,   453,   453,   454,   454,   454,   454,   454,
     454,   455,   455,   455,   455,   455,   456,   456,   456,   456,
     456,   456,   457,   457,   458,   458,   458,   458,   458,   458,
     458,   458,   459,   459,   460,   460,   460,   460,   461,   461,
     462,   462,   462,   462,   463,   463,   464,   464,   465,   465,
     466,   466,   467,   467,   468,   468,   469,   469,   470,   470,
     471,   471,   471,   471,   472,   472,   472,   472,   472,   472,
     473,   473,   473,   474,   474,   474,   474,   474,   474,   474,
     474,   474,   474,   474,   475,   475,   476,   476,   477,   477,
     478,   478,   479,   479,   480,   480,   481,   481,   482,   482,
     483,   484,   484,   484,   484,   485,   485,   486,   486,   486,
     486,   487,   487,   487,   488,   488,   489,   489,   490,   490,
     491,   492,   492,   493,   493,   494,   494,   494,   494,   494,
     494,   494,   494,   494,   494,   494,   494,   495,   495
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
       2,     2,     2,     1,     1,     1,     1,     1,     3,     2,
       1,     5,     0,     0,    11,     0,    12,     0,     3,     0,
       4,     0,     6,     0,     7,     0,     5,     2,     2,     4,
       1,     1,     5,     3,     5,     3,     2,     0,     2,     0,
       4,     4,     3,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     1,     1,     1,     1,     3,     2,     3,
       4,     2,     3,     1,     2,     1,     2,     1,     1,     1,
       1,     1,     1,     4,     4,     2,     8,    10,     2,     1,
       3,     1,     2,     1,     1,     1,     1,     2,     4,     3,
       3,     4,     1,     2,     4,     2,     6,     0,     1,     4,
       0,     2,     0,     1,     1,     3,     1,     3,     1,     1,
       3,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     4,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     1,     0,     0,     1,     1,     3,     0,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     2,     1,     1,     4,     3,     4,     1,
       1,     1,     1,     3,     3,     3,     3,     3,     3,     3,
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
       5,     3,     3,     1,     2,     0,     2,     0,     5,     3,
       3,     1,     2,     0,     2,     0,     5,     3,     3,     1,
       2,     2,     1,     2,     1,     4,     3,     3,     6,     3,
       1,     1,     1,     4,     4,     4,     4,     4,     4,     2,
       2,     4,     2,     2,     1,     3,     3,     3,     0,     2,
       5,     6,     1,     2,     1,     4,     3,     0,     1,     3,
       2,     3,     1,     1,     0,     0,     2,     4,     2,     6,
       4,     1,     1,     0,     3,     5,     3,     1,     2,     0,
       4,     1,     1,     3,     2,     2,     2,     1,     1,     1,
       1,     3,     4,     6,     1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   367,     0,   762,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   843,     0,
     831,   643,     0,   649,   650,   651,    22,   709,   819,    98,
      99,   652,     0,    80,     0,     0,     0,     0,     0,     0,
       0,     0,   132,     0,     0,     0,     0,     0,     0,   334,
     335,   336,   339,   338,   337,     0,     0,     0,     0,   159,
       0,     0,     0,   656,   658,   659,   653,   654,     0,     0,
     660,   655,     0,   634,    23,    24,    25,    27,    26,     0,
     657,     0,     0,     0,     0,     0,     0,     0,   661,   340,
      28,    29,    31,    30,    32,    33,    34,    35,    36,    37,
      38,    39,    40,   461,     0,    97,    70,   823,   644,     0,
       0,     4,    59,    61,    64,   708,     0,   633,     0,     6,
     131,     7,     9,     8,    10,     0,     0,   332,   377,     0,
       0,     0,     0,     0,     0,     0,   375,   807,   808,   445,
     444,   361,   446,   447,   450,     0,     0,   360,   785,   635,
       0,   711,   443,   331,   788,   376,     0,     0,   379,   378,
     786,   787,   784,   814,   818,     0,   433,   710,    11,   339,
     338,   337,     0,     0,    27,    59,   131,     0,   903,   376,
     902,     0,   900,   899,   449,     0,   368,   372,     0,     0,
     417,   418,   419,   420,   442,   440,   439,   438,   437,   436,
     435,   434,   819,   636,     0,   917,   635,     0,   399,     0,
     397,     0,   847,     0,   718,   359,   639,     0,   917,   638,
       0,   648,   826,   825,   640,     0,     0,   642,   441,     0,
       0,     0,     0,   364,     0,    78,   366,     0,     0,    84,
      86,     0,     0,    88,     0,     0,     0,   942,   948,   949,
     954,     0,     0,    59,   947,   941,     0,   950,     0,     0,
       0,    90,     0,     0,     0,     0,   122,     0,     0,     0,
       0,     0,     0,    42,    47,   248,     0,     0,   247,     0,
     163,     0,   160,   253,     0,     0,     0,     0,     0,   914,
     147,   157,   839,   843,   884,     0,   663,     0,     0,     0,
     882,     0,    16,     0,    63,   139,   151,   158,   540,   477,
     867,   865,   865,     0,   908,   459,   463,   465,   766,   377,
       0,   375,   376,   378,     0,     0,   645,     0,   646,     0,
       0,     0,   121,     0,     0,    66,   239,     0,    21,   130,
       0,   156,   143,   155,   337,   340,   131,   333,   112,   113,
     114,   115,   116,   118,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   831,
       0,   111,   822,   822,   119,   853,     0,     0,     0,     0,
       0,     0,   330,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   398,   396,   767,   768,
       0,   822,     0,   780,   239,   239,   822,     0,   824,   815,
     839,     0,   131,     0,     0,    92,     0,   764,   759,   718,
       0,     0,     0,     0,     0,   851,     0,   482,   717,   842,
       0,     0,    66,     0,   239,   358,     0,   782,   641,     0,
      70,   199,     0,   458,     0,    95,     0,     0,   365,     0,
       0,     0,     0,     0,    87,   110,    89,   945,   946,     0,
     939,     0,     0,     0,     0,   913,     0,   117,    91,   120,
       0,     0,     0,     0,     0,     0,     0,   498,     0,   505,
     507,   508,   509,   510,   511,   512,   503,   525,   526,    70,
       0,   107,   109,     0,     0,    44,    51,     0,     0,    46,
      55,    48,     0,    18,     0,     0,   249,     0,    93,   162,
     161,     0,     0,    94,   904,     0,     0,   377,   375,   376,
     379,   378,     0,   933,   169,     0,   840,     0,     0,     0,
       0,   662,   883,   709,     0,     0,   881,   714,   880,    62,
       5,    13,    14,     0,   167,     0,     0,   470,     0,     0,
     718,     0,     0,   637,   471,   871,     0,   718,     0,     0,
     718,     0,     0,     0,     0,     0,   766,    70,     0,   720,
     765,   958,   357,   430,   794,   806,    75,    69,    71,    72,
      73,    74,   331,     0,   448,   712,   713,    60,   718,     0,
     918,     0,     0,     0,   720,   240,     0,   453,   133,   165,
       0,   402,   404,   403,     0,     0,   400,   401,   405,   407,
     406,   422,   421,   424,   423,   425,   427,   428,   426,   416,
     415,   409,   410,   408,   411,   412,   414,   429,   413,   821,
       0,     0,   857,     0,   718,   907,     0,   906,   791,   814,
     149,   141,   153,   145,   131,   367,     0,   370,   373,   381,
     499,   395,   394,   393,   392,   391,   390,   389,   388,   387,
     386,   385,   384,   770,     0,   769,   772,   789,   776,   917,
     773,     0,     0,     0,     0,     0,     0,     0,     0,   901,
     369,   757,   761,   717,   763,     0,     0,   917,     0,   846,
       0,   845,     0,   830,   829,     0,     0,   769,   772,   827,
     773,   362,   201,   203,    70,   468,   467,   363,     0,    70,
     183,    79,   366,     0,     0,     0,     0,     0,   195,   195,
      85,     0,     0,     0,   937,   718,     0,   924,     0,     0,
       0,     0,     0,   716,   652,     0,     0,   634,     0,     0,
       0,     0,     0,    64,   665,   633,   671,   672,   670,     0,
     664,    68,   669,   917,   951,     0,     0,   515,     0,     0,
     521,   518,   519,   527,     0,   506,   501,     0,   504,     0,
       0,     0,    52,    19,     0,     0,    56,    20,     0,     0,
       0,    41,    49,     0,   246,   254,   251,     0,     0,   893,
     898,   895,   894,   897,   896,    12,   931,   932,     0,     0,
       0,     0,   839,   836,     0,   481,   892,   891,   890,     0,
     886,     0,   887,   889,     0,     5,     0,     0,     0,   534,
     535,   543,   542,     0,     0,   717,   476,   480,     0,   486,
     717,   866,     0,   484,   717,   864,   485,     0,   909,     0,
     460,     0,     0,   925,   766,   225,   957,     0,     0,   781,
     820,   717,   920,   916,   241,   242,   632,   719,   238,     0,
     766,     0,     0,   167,   455,   135,   432,     0,   491,   492,
       0,   483,   717,   852,     0,     0,   239,   169,     0,   167,
     165,     0,   831,   382,     0,     0,   778,   779,   792,   793,
     816,   817,     0,     0,     0,   745,   725,   726,   727,   734,
       0,     0,     0,   738,   736,   737,   751,   718,     0,   759,
     850,   849,     0,     0,   783,   647,     0,   205,     0,     0,
      76,     0,     0,     0,     0,     0,     0,     0,   175,   176,
     187,     0,    70,   185,   104,   195,     0,   195,     0,     0,
     952,     0,     0,   717,   938,   940,   923,   718,   922,     0,
     718,   693,   694,   691,   692,   724,     0,   718,   716,     0,
       0,   479,   875,   873,   873,     0,     0,   859,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   944,   500,     0,     0,     0,
     523,   524,   522,     0,     0,   502,     0,   123,     0,   126,
     108,     0,    43,    53,     0,    45,    57,    50,   250,     0,
     905,    96,   933,   915,   928,   168,   170,   260,     0,     0,
     837,     0,   885,     0,    17,     0,   908,   166,   260,     0,
       0,   473,     0,   906,   870,   869,     0,   910,     0,   925,
     466,     0,     0,   958,     0,   230,   228,   772,   790,   917,
     919,     0,     0,   243,    67,     0,   766,   164,     0,   766,
       0,   431,   856,   855,     0,   239,     0,     0,     0,     0,
     167,   137,   648,   771,   239,     0,   730,   731,   732,   733,
     739,   740,   749,     0,   718,     0,   745,     0,   729,   753,
     717,   756,   758,   760,     0,   844,   772,   828,   771,     0,
       0,     0,     0,   202,   469,    81,     0,   366,   175,   177,
     839,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     189,     0,   934,     0,   936,   717,     0,     0,     0,   667,
     717,   715,     0,   706,     0,   718,     0,   879,     0,   718,
       0,     0,   718,     0,   673,   707,   705,   863,     0,   718,
     676,   678,   677,     0,     0,   674,   675,   679,   681,   680,
     696,   695,   698,   697,   699,   701,   702,   700,   689,   688,
     683,   684,   682,   685,   686,   687,   690,   943,   513,     0,
     514,   520,   528,   529,     0,    70,    54,    58,   252,     0,
       0,     0,   331,   841,   839,   371,   374,   380,     0,    15,
       0,   331,   546,     0,     0,   548,   541,   544,     0,   539,
       0,     0,   911,     0,   926,   462,     0,   231,     0,     0,
     226,     0,   245,   244,   925,     0,   260,     0,   766,     0,
     239,     0,   812,   260,   908,   260,     0,     0,   383,     0,
       0,   742,   717,   744,   735,     0,   728,     0,     0,   718,
     750,   848,     0,    70,     0,   198,   184,     0,     0,     0,
     174,   100,   188,     0,     0,   191,     0,   196,   197,    70,
     190,   953,     0,   921,     0,   956,   723,   722,   666,     0,
     717,   478,   668,   489,   717,   874,     0,   487,   717,   872,
     488,     0,   490,   717,   858,   704,     0,     0,     0,     0,
     927,   930,   171,     0,     0,     0,   338,   329,     0,     0,
       0,   148,   259,   261,     0,   328,     0,     0,     0,   908,
     331,     0,   888,   256,   152,   537,     0,     0,   472,   868,
     464,     0,   234,   224,     0,   227,   233,   239,   452,   925,
     331,   925,     0,   854,     0,   811,   331,     0,   331,   260,
     766,   809,   748,   747,   741,     0,   743,   717,   752,    70,
     204,    77,    82,   102,   178,     0,   186,   192,    70,   194,
     935,     0,     0,   475,     0,   878,   877,     0,   862,   861,
     703,     0,    70,   127,     0,     0,     0,     0,     0,     0,
     172,     0,   908,   295,   291,   297,   634,    27,     0,   287,
       0,   294,   306,     0,   304,   309,     0,   308,     0,   307,
       0,   131,   337,   263,     0,   265,     0,   266,   267,     0,
       0,   838,     0,   538,   536,   547,   545,   235,     0,     0,
     222,   232,     0,     0,     0,     0,   144,   452,   925,   813,
     150,   256,   154,   331,     0,     0,   755,     0,   200,     0,
       0,    70,   181,   101,   193,   955,   721,     0,     0,     0,
       0,     0,   929,     0,     0,   356,     0,     0,   277,   281,
     353,   354,     0,     0,     0,   272,   598,   597,   594,   596,
     595,   615,   617,   616,   586,   557,   558,   576,   592,   591,
     553,   563,   564,   566,   565,   585,   569,   567,   568,   570,
     571,   572,   573,   574,   575,   577,   578,   579,   580,   581,
     582,   584,   583,   554,   555,   556,   559,   560,   562,   600,
     601,   610,   609,   608,   607,   606,   605,   593,   612,   602,
     603,   604,   587,   588,   589,   590,   613,   614,   618,   620,
     619,   621,   622,   599,   624,   623,   626,   628,   627,   561,
     631,   629,   630,   625,   611,   552,   301,   549,     0,   273,
     322,   323,   321,   314,     0,   315,   274,   348,     0,     0,
       0,     0,   352,     0,   131,   140,   255,     0,     0,     0,
     223,   237,   810,     0,    70,   324,    70,   134,     0,     0,
       0,   146,   925,   746,     0,    70,   179,    83,   103,     0,
     474,   876,   860,   516,   125,   275,   276,   351,   173,     0,
       0,     0,   298,   288,     0,     0,     0,   303,   305,     0,
       0,   310,   317,   318,   316,     0,     0,   262,     0,     0,
       0,   355,     0,   257,     0,   236,     0,   532,   720,     0,
       0,    70,   136,   142,     0,   754,     0,     0,     0,   105,
     278,    59,     0,   279,   280,     0,     0,   292,     0,   296,
     300,   550,   551,     0,   289,   319,   320,   312,   313,   311,
     349,   346,   268,   264,   350,     0,   258,   533,   719,     0,
     454,   325,     0,   138,     0,   182,   517,     0,   129,     0,
     331,     0,   299,   302,     0,   766,   270,     0,   530,   451,
     456,   180,     0,     0,   106,   285,     0,   330,   293,   347,
       0,   720,   342,   766,   531,     0,   128,     0,     0,   284,
     925,   766,   209,   343,   344,   345,   958,   341,     0,     0,
       0,   283,     0,   342,     0,   925,     0,   282,   326,    70,
     269,   958,     0,   214,   212,     0,    70,     0,     0,   215,
       0,     0,   210,   271,     0,   327,     0,   218,   208,     0,
     211,   217,   124,   219,     0,     0,   206,   216,     0,   207,
     221,   220
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   121,   835,   560,   185,   282,   514,
     518,   283,   515,   519,   123,   124,   125,   126,   127,   128,
     334,   597,   598,   467,   244,  1479,   473,  1395,  1480,  1718,
     791,   277,   509,  1679,  1030,  1215,  1734,   350,   186,   599,
     881,  1090,  1267,   132,   563,   898,   600,   619,   900,   544,
     897,   601,   564,   899,   352,   300,   316,   135,   883,   838,
     821,  1045,  1419,  1142,   948,  1627,  1483,   731,   954,   472,
     740,   956,  1299,   723,   937,   940,  1131,  1740,  1741,   588,
     589,   613,   614,   287,   288,   294,  1452,  1606,  1607,  1222,
    1342,  1440,  1600,  1725,  1743,  1639,  1683,  1684,  1685,  1428,
    1429,  1430,  1641,  1647,  1694,  1433,  1434,  1438,  1593,  1594,
    1595,  1617,  1770,  1343,  1344,   187,   137,  1756,  1757,  1598,
    1346,  1347,  1348,  1349,   138,   237,   468,   469,   139,   140,
     141,   142,   143,   144,   145,   146,  1464,   147,   880,  1089,
     148,   241,   585,   328,   586,   587,   463,   569,   570,  1165,
     571,  1166,   149,   150,   151,   152,   153,   766,   767,   768,
     154,   155,   274,   156,   275,   497,   498,   499,   500,   501,
     502,   503,   504,   505,   781,   782,  1022,   506,   507,   508,
     788,  1668,   157,   565,  1454,   566,  1059,   843,  1239,  1236,
    1586,  1587,   158,   159,   160,   231,   238,   337,   455,   161,
     975,   772,   162,   976,   872,   865,   977,   924,  1110,   925,
    1112,  1113,  1114,   927,  1278,  1279,   928,   702,   439,   198,
     199,   602,   591,   420,   686,   687,   688,   689,   869,   164,
     232,   189,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   650,   175,   234,   235,   547,   223,   224,   653,   654,
    1178,  1179,   579,   576,   580,   577,  1171,  1168,  1172,  1169,
     309,   310,   829,   176,   535,   177,   584,   178,  1608,   301,
     345,   608,   609,   969,  1072,   818,   819,   744,   745,   746,
     267,   268,   774,   269,   867
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1443
static const yytype_int16 yypact[] =
{
   -1443,   126, -1443, -1443,  5954, 13856, 13856,   -73, 13856, 13856,
   13856, 11944, 13856, -1443, 13856, 13856, 13856, 13856, 13856, 13856,
   13856, 13856, 13856, 13856, 13856, 13856, 15717, 15717, 12152, 13856,
   15783,   -69,   -53, -1443, -1443, -1443, -1443, -1443,   154, -1443,
   -1443,    98, 13856, -1443,   -53,   -34,   -23,   -21,   -53, 12318,
   13109, 12484, -1443, 14955, 10946,   -13, 13856,  3333,   132, -1443,
   -1443, -1443,    34,   330,    80,   123,   141,   157,   212, -1443,
   13109,   247,   251, -1443, -1443, -1443, -1443, -1443,   416,  3117,
   -1443, -1443, 13109, -1443, -1443, -1443, -1443, 13109, -1443, 13109,
   -1443,   192,   263,   273,   279,   293, 13109, 13109, -1443,   345,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, 13856, -1443, -1443,    22,   482,   520,
     520, -1443,   407,   368,   468, -1443,   311, -1443,    76, -1443,
     519, -1443, -1443, -1443, -1443, 15472,    39, -1443, -1443,   349,
     355,   366,   395,   403,   425,  4578, -1443, -1443, -1443, -1443,
     514, -1443, -1443, -1443,   522,   566,   431, -1443,    59,   446,
     492, -1443, -1443,   588,    56,  2312,   115,   453,    32, -1443,
     117,   124,   462,   128, -1443,   252, -1443,   599, -1443, -1443,
   -1443,   541,   470,   543, -1443, -1443,   519,    39, 16689,  2783,
   16689, 13856, 16689, 16689,  4504,   491, 14713,  4504,   673, 13109,
     658,   658,   120,   658,   658,   658,   658,   658,   658,   658,
     658,   658, -1443, -1443,  3607,   569, -1443,   595,   418,   548,
     418, 15717, 15459,   546,   746, -1443,   541, 15170,   569,   608,
     609,   562,   131, -1443,   418,   115, 12650, -1443, -1443, 13856,
    9490,   756,    79, 16689, 10530, -1443, 13856, 13856, 13109, -1443,
   -1443,  4763,   565, -1443,  4996, 14955, 14955, -1443,   603, -1443,
     630,   600, 14712,    75,   655, -1443,   791, -1443,   666, 13109,
     737, -1443,   621,  5133,   622,   740, -1443,    15,  5210, 15557,
   16369, 13109,    82, -1443,    48, -1443, 15492,    83, -1443,   703,
   -1443,   704, -1443,   820,    85, 15717, 15717, 13856,   634,   667,
   -1443, -1443, 15577, 12152,   374,   359, -1443, 14022, 15717,   437,
   -1443, 13109, -1443,   436,   368, -1443, -1443, -1443, -1443, 15806,
   13856, 13856, 13856,   826,   751, -1443, -1443, -1443,    37,   641,
   16689,   644,  1443,   652,  6162, 13856,   386,   643,   535,   386,
     351,    28, -1443, 13109, 14955,   656, 11154, 14955, -1443, -1443,
    3867, -1443, -1443, -1443, -1443, -1443,   519, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, 13856, 13856, 13856, 12858, 13856, 13856,
   13856, 13856, 13856, 13856, 13856, 13856, 13856, 13856, 13856, 13856,
   13856, 13856, 13856, 13856, 13856, 13856, 13856, 13856, 13856, 15783,
   13856, -1443, 13856, 13856, -1443, 13856,   841, 13109, 13109, 13109,
   15472,   748,   495, 10738, 13856, 13856, 13856, 13856, 13856, 13856,
   13856, 13856, 13856, 13856, 13856, 13856, -1443, -1443, -1443, -1443,
    2119, 13856, 13856, -1443, 11154, 11154, 13856, 13856,    22,   133,
   15577,   664,   519, 13066,  5445, -1443, 13856, -1443,   670,   847,
    3607,   671,   286,   652,  3058,   418, 13274, -1443, 13482, -1443,
     674,   306, -1443,   255, 11154, -1443,  2907, -1443, -1443, 14418,
   -1443, -1443, 11362, -1443, 13856, -1443,   779,  9698,   868,   677,
   16600,   865,    78,    47, -1443, -1443, -1443, -1443, -1443, 14955,
    4574,   680,   875, 15259, 13109, -1443,   706, -1443, -1443, -1443,
     811, 13856,   821,   823, 13856, 13856, 13856, -1443,   740, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443,   715, -1443, -1443, -1443,
     709, -1443, -1443, 13109,   714,   898,    77, 13109,   716,   900,
     236,   326, 16430, -1443, 13109, 13856,   418,   132, -1443, -1443,
   -1443, 15259,   832, -1443,   418,    95,   106,   719,   721,  2085,
     283,   726,   730,   579,   803,   736,   418,   107,   738,  5343,
   13109, -1443, -1443,   872,  2142,    18, -1443, -1443, -1443,   368,
   -1443, -1443, -1443,   909,   815,   780,    81, -1443,    22,   818,
     942,   750,   804,   133, -1443, 16689,   752,   950, 15994,   759,
     954,   764, 14955, 14955,   953,   756,    37, -1443,   770,   961,
   -1443, 14955,    99,   908,   121, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443,   531,  2219, -1443, -1443, -1443, -1443,   967,   806,
   -1443, 15717, 13856,   776,   971, 16689,   969, -1443, -1443,   858,
    4894, 11553, 13673,  4504, 13856, 16645,  4120, 16917,  3694, 17013,
   12836, 14760, 14760, 14760, 14760,  1533,  1533,  1533,  1533,   848,
     848,   675,   675,   675,   120,   120,   120, -1443,   658, 16689,
     777,   778, 16038,   782,   979, -1443, 13856,   -29,   789,   133,
   -1443, -1443, -1443, -1443,   519, 13856, 15407, -1443, -1443,  4504,
   -1443,  4504,  4504,  4504,  4504,  4504,  4504,  4504,  4504,  4504,
    4504,  4504,  4504, -1443, 13856,   365,   143, -1443, -1443,   569,
     385,   783,  2368,   790,   792,   788,  2830,   108,   798, -1443,
   16689, 15391, -1443, 13109, -1443,   641,    99,   569, 15717, 16689,
   15717, 16094,    99,   144, -1443,   801, 13856, -1443,   148, -1443,
   -1443, -1443,  9282,   587, -1443, -1443, 16689, 16689,   -53, -1443,
   -1443, -1443, 13856,   910,  5189, 15259, 13109,  9906,   802,   805,
   -1443,    69,   880,   862, -1443,  1006,   814, 14805, 14955, 15259,
   15259, 15259, 15259, 15259, -1443,   816,    23,   871,   822,   839,
     842,   843, 15259,   473, -1443,   873, -1443, -1443, -1443,   850,
   -1443, 16775, -1443,   381, -1443, 13856,   837, 16689,   874,  1044,
   14506,  1051, -1443, 16689, 14462, -1443,   715,   984, -1443,  6370,
   15193,   860,   371, -1443, 15557, 13109,   426, -1443, 16369, 13109,
   13109, -1443, -1443,  2956, -1443, 16775,  1052, 15717,   870, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,    88, 13109,
   15193,   861, 15577, 15662,  1054, -1443, -1443, -1443, -1443,   863,
   -1443, 13856, -1443, -1443,  5538, -1443, 14955, 15193,   877, -1443,
   -1443, -1443, -1443,  1060, 13856, 15806, -1443, -1443, 11987, -1443,
   13856, -1443, 13856, -1443, 13856, -1443, -1443,   876, -1443, 14955,
   -1443,   884,  6578,  1045,    45, -1443, -1443,    61,  2119, -1443,
   -1443, 14955, -1443, -1443,   418, 16689, -1443, 11570, -1443, 15259,
      35,   885, 15193,   815, -1443, -1443, 14228, 13856, -1443, -1443,
   13856, -1443, 13856, -1443,  3133,   886, 11154,   803,  1059,   815,
     858, 13109, 15783,   418,  3503,   890, -1443, -1443,   149, -1443,
   -1443, -1443,  1078,  5136,  5136, 15391, -1443, -1443, -1443,  1046,
     892,   299,   894, -1443, -1443, -1443, -1443,  1087,   899,   670,
     418,   418, 13690,  2907, -1443, -1443,  3844,   627,   -53, 10530,
   -1443,  6786,   901,  6994,   902,  5189, 15717,   905,   972,   418,
   16775,  1094, -1443, -1443, -1443, -1443,   676, -1443,    38, 14955,
   -1443, 14955, 13109,  4574, -1443, -1443, -1443,  1100, -1443,   911,
     967,   702,   702,  1047,  1047, 15185,   903,  1105, 15259,   973,
   13109, 15806, 15259, 15259, 15259,  4466, 12901, 15259, 15259, 15259,
   15259, 15107, 15259, 15259, 15259, 15259, 15259, 15259, 15259, 15259,
   15259, 15259, 15259, 15259, 15259, 15259, 15259, 15259, 15259, 15259,
   15259, 15259, 15259, 15259, 13109, -1443, 16689, 13856, 13856, 13856,
   -1443, -1443, -1443, 13856, 13856, -1443,   740, -1443,  1038, -1443,
   -1443, 13109, -1443, -1443, 13109, -1443, -1443, -1443, -1443, 15259,
     418, -1443,   579, -1443,  1021,  1111, -1443, -1443,   109,   920,
     418, 11778, -1443,   749, -1443,  5746,   751,  1111, -1443,   469,
     321, 16689,   992, -1443, 16689, 16689, 16138, -1443,   921,  1045,
   -1443, 14955,   756, 14955,    50,  1112,  1048,   150, -1443,   569,
   -1443, 15717, 13856, 16689, 16775,   926,    35, -1443,   927,    35,
     929, 14228, 16689, 16194,   932, 11154,   933,   930, 14955,   935,
     815, -1443,   562,   432, 11154, 13856, -1443, -1443, -1443, -1443,
   -1443, -1443,  1007,   928,  1131,  1055, 15391,   997, -1443, 15806,
   15391, -1443, -1443, -1443, 15717, 16689,   153, -1443, -1443,   -53,
    1115,  1073, 10530, -1443, -1443, -1443,   945, 13856,   972,   418,
   15577,  5189,   947, 15259,  7202,   708,   948, 13856,    49,   332,
   -1443,   981, -1443,  1020, -1443, 14855,  1123,   955, 15259, -1443,
   15259, -1443,   958, -1443,  1025,  1153,   962, 16775,   964,  1154,
   16238,   965,  1159,   970, -1443, -1443, -1443, 16294,   966,  1164,
   13465, 16815, 11137, 15259, 16733, 16884, 16950, 16982,  4064, 17043,
   17073, 17073, 17073, 17073,  2308,  2308,  2308,  2308,  1002,  1002,
     702,   702,   702,  1047,  1047,  1047,  1047, -1443, 16689, 14639,
   16689, -1443, 16689, -1443,   974, -1443, -1443, -1443, 16775, 13109,
   14955, 15193,    89, -1443, 15577, -1443, -1443,  4504,   976, -1443,
     975,   459, -1443,    71, 13856, -1443, -1443, -1443, 13856, -1443,
   13856, 13856, -1443,   756, -1443, -1443,   177,  1160,  1098, 13856,
   -1443,   977,   418, 16689,  1045,   980, -1443,   988,    35, 13856,
   11154,   990, -1443, -1443,   751, -1443,   989,   982, -1443,   996,
   15391, -1443, 15391, -1443, -1443,   999, -1443,  1064,  1000,  1194,
   -1443,   418,  1174, -1443,  1003, -1443, -1443,  1005,  1008,   110,
   -1443, -1443, 16775,  1009,  1010, -1443,  2655, -1443, -1443, -1443,
   -1443, -1443, 14955, -1443, 14955, -1443, 16775, 16338, -1443, 15259,
   15806, -1443, -1443, -1443, 15259, -1443, 15259, -1443, 15259, -1443,
   -1443, 15259, -1443, 15259, -1443, 16850, 15259, 13856,  1001,  7410,
    1117, -1443, -1443,   581, 14905, 15193,  1102, -1443, 16385,  1057,
    4407, -1443, -1443, -1443,   748,  2415,    90,    92,  1013,   751,
     495,   111, -1443, -1443, -1443,  1066,  3946,  4379, 16689, 16689,
   -1443,    54,  1195,  1139, 13856, -1443, 16689, 11154,  1114,  1045,
    1515,  1045,  1022, 16689,  1024, -1443,  1810,  1019,  1845, -1443,
      35, -1443, -1443,  1097, -1443, 15391, -1443, 15806, -1443, -1443,
    9282, -1443, -1443, -1443, -1443, 10114, -1443, -1443, -1443,  9282,
   -1443,  1026, 15259, 16775,  1107, 16775, 16775, 16394, 16775, 16438,
   16850, 14595, -1443, -1443, 14955, 15193, 15193, 13109,  1218,    72,
   -1443, 14905,   751, -1443,  1068, -1443,    93,  1036,    96, -1443,
   14230, -1443, -1443,    97, -1443, -1443,  3502, -1443,  1039, -1443,
    1166,   519,  1104, -1443, 14955, -1443, 14955, -1443, -1443,  1232,
     748, -1443,  4066, -1443, -1443, -1443, -1443,  1246,  1182, 13856,
   -1443, 16689,  1061,  1065,  1058,   517, -1443,  1114,  1045, -1443,
   -1443, -1443, -1443,  1964,  1067, 15391, -1443,  1137,  9282, 10322,
   10114, -1443, -1443, -1443,  9282, -1443, 16775, 15259, 15259, 15259,
   13856,  7618, -1443,  1069,  1070, -1443, 15259, 15193, -1443, -1443,
   -1443, -1443, 14955,  1113, 16385, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443,   113, -1443,  1057, -1443,
   -1443, -1443, -1443, -1443,    62,   499, -1443,  1254,   101, 13109,
    1166,  1257, -1443, 14955,   519, -1443, -1443,  1071,  1261, 13856,
   -1443, 16689, -1443,   404, -1443, -1443, -1443, -1443,  1072,   517,
   14662, -1443,  1045, -1443, 15391, -1443, -1443, -1443, -1443,  7826,
   16775, 16775, 16775, 14550, -1443, -1443, -1443, 16775, -1443,  2576,
      91,    60, -1443, -1443, 15259, 14230, 14230,  1224, -1443,  3502,
    3502,   651, -1443, -1443, -1443, 15259,  1201, -1443,  1080,   102,
   15259, -1443, 13109, -1443, 15259, 16689,  1203, -1443,  1274,  8034,
    8242, -1443, -1443, -1443,   517, -1443,  8450,  1082,  1207,  1178,
   -1443,  1191,  1140, -1443, -1443,  1196, 14955, -1443,  1113, -1443,
   16775, -1443, -1443,  1130, -1443,  1259, -1443, -1443, -1443, -1443,
   16775,  1280, -1443, -1443, 16775,  1095, 16775, -1443,   480,  1096,
   -1443, -1443,  8658, -1443,  1099, -1443, -1443,  1103,  1132, 13109,
     495,  1126, -1443, -1443, 15259,   105, -1443,  1221, -1443, -1443,
   -1443, -1443, 15193,   860, -1443,  1141, 13109,   478, -1443, 16775,
    1106,  1297,   698,   105, -1443,  1230, -1443, 15193,  1108, -1443,
    1045,   116, -1443, -1443, -1443, -1443, 14955, -1443,  1110,  1116,
     103, -1443,   526,   698,   376,  1045,  1109, -1443, -1443, -1443,
   -1443, 14955,    57,  1296,  1236,   526, -1443,  8866,   377,  1302,
    1238, 13856, -1443, -1443,  9074, -1443,    58,  1306,  1242, 13856,
   -1443, 16689, -1443,  1310,  1247, 13856, -1443, 16689, 13856, -1443,
   16689, 16689
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1443, -1443, -1443,  -496, -1443, -1443, -1443,   422, -1443, -1443,
   -1443,   808,   533,   527,   114,  1636,  3914, -1443,  3336, -1443,
    -242, -1443,    29, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443,  -405, -1443, -1443,  -164,    52,    25,
   -1443, -1443, -1443, -1443, -1443, -1443,    27, -1443, -1443, -1443,
   -1443,    30, -1443, -1443,   931,   936,   939,  -105,   434,  -816,
     435,   502,  -407,   203,  -871, -1443,  -138, -1443, -1443, -1443,
   -1443,  -666,    51, -1443, -1443, -1443, -1443,  -400, -1443,  -536,
   -1443,  -386, -1443, -1443,   819, -1443,  -126, -1443, -1443,  -982,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
    -156, -1443, -1443, -1443, -1443, -1443,  -239, -1443,    11,  -917,
   -1443, -1442,  -423, -1443,  -155,    21,  -115,  -410, -1443,  -246,
   -1443, -1443, -1443,    24,   -48,     9,  1315,  -671,  -375, -1443,
   -1443,   -12, -1443, -1443,    -5,   -39,  -103, -1443, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443,  -558,  -783, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443, -1443,   963, -1443, -1443,   343, -1443,   879, -1443, -1443,
   -1443, -1443, -1443, -1443, -1443,   350, -1443,   881, -1443, -1443,
     589, -1443,   319, -1443, -1443, -1443, -1443, -1443, -1443, -1443,
   -1443,  -910, -1443,  2629,  2525,  -334, -1443, -1443,   278,  3700,
    4122, -1443, -1443,   408,  -110,  -612, -1443, -1443,   474,   272,
    -669,   274, -1443, -1443, -1443, -1443, -1443,   460, -1443, -1443,
   -1443,   145,  -821,  -174,  -397,  -391, -1443,   524,  -112, -1443,
   -1443,   648, -1443, -1443,   889,   -44, -1443, -1443,    73,  -130,
   -1443,  -258, -1443, -1443, -1443,  -395,  1090, -1443, -1443, -1443,
   -1443, -1443,  1076, -1443, -1443, -1443,   411, -1443, -1443, -1443,
     713,   348, -1443, -1443,  1118,  -289,  -993, -1443,   -27,   -49,
    -176,    -6,   653, -1443,  -927, -1443,   362,   438, -1443, -1443,
   -1443, -1443,   391,    -1, -1037
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -943
static const yytype_int16 yytable[] =
{
     188,   190,   878,   192,   193,   194,   196,   197,   401,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   357,   431,   222,   225,   136,   266,   860,   667,   131,
     574,   133,   926,   129,   134,   697,  1246,   243,   693,   694,
     317,   240,   272,  1073,   251,   429,   254,   323,   324,   273,
     861,   278,   452,   245,   423,   647,   130,   249,   400,   718,
     456,   944,  1062,  1230,   834,   719,   329,  1088,   715,  1688,
     333,  1649,   357,   958,  1138,   331,  1231,   738,   959,  1297,
      13,  1497,    13,  1099,  -912,   347,   353,   736,   464,  -912,
      13,   522,   527,  1333,   532,  1650,  1074,  1042,  -495,  1444,
    1686,  1446,  -290,   233,   807,  1504,  1588,   304,   979,   330,
    1656,  1656,  1497,   556,   449,   807,   823,   823,   823,   823,
     823,  1355,  -798,   457,   510,  -795,     3,  1644,   191,  1247,
     550,   840,   236,  1457,    13,   651,  1779,  1793,  1147,  1148,
    1075,   344,  1243,  1645,    52,   524,   418,   419,   239,   163,
      13,   289,    59,    60,    61,   179,   180,   354,   290,   293,
    1646,    13,  -457,   691,  -496,   421,   441,   246,   695,   306,
     443,   284,  -917,   389,   795,   307,   308,  1672,   247,   450,
     248,   421,   511,  -796,   402,   390,   434,  -802,   356,   573,
    -797,   276,   620,   313,   426,  1334,   314,  -832,  1164,   426,
    1335,   343,    59,    60,    61,   179,  1336,   354,  1337,  -799,
    -835,   285,  1361,  1248,  -833,  -801,  -800,  1458,   722,  -834,
    1780,  1794,   355,   833,  1076,   318,   335,   418,   419,  -636,
     343,   459,  1713,   425,   459,   841,  -229,  -229,   606,   432,
     960,   243,   470,  1150,  -719,  1338,  1339,  -719,  1340,   537,
     842,   739,  1298,   541,   477,   478,  1362,  -804,   538,  1043,
    -798,   482,  1687,  -795,  1651,  1689,   659,   789,   698,   461,
    1290,  1377,   355,   466,  1370,  1498,  1499,  -637,  -912,   348,
     737,  1376,   465,  1378,  1266,   523,   528,   357,   533,  1145,
     659,  1149,   330,  1445,  1341,  1447,  -290,   808,   222,  1505,
    1589,   618,   554,   422,  1657,  1703,  1767,  -213,   809,   824,
     912,  1223,  1394,  1451,   659,   575,   578,   578,  -719,   422,
     616,  -796,  -805,   659,   295,  -802,   659,  1368,  -797,   704,
     603,   212,   427,   799,   212,  -832,  1277,   427,   286,  1055,
    1363,   615,   296,   610,  1085,   862,   318,  -799,  -835,  -495,
     317,   353,  -833,  -801,  -800,   136,  1449,  -834,   297,   621,
     622,   623,   625,   626,   627,   628,   629,   630,   631,   632,
     633,   634,   635,   636,   637,   638,   639,   640,   641,   642,
     643,   644,   645,   646,  1117,   648,   130,   649,   649,   343,
     652,   668,   705,   516,   520,   521,   443,  1473,   669,   671,
     672,   673,   674,   675,   676,   677,   678,   679,   680,   681,
     682,  1772,  1786,   298,   418,   419,   649,   692,   868,   615,
     615,   649,   696,   800,   325,   559,   122,  1048,   669,  1501,
     304,   700,  1147,  1148,   418,   419,   556,   342,   304,  1666,
     549,   709,  1465,   711,  1467,  1118,   291,   401,   302,   615,
     725,  1280,   303,   664,   292,  1773,  1787,   726,   428,   727,
     846,   117,   233,  1333,   319,   304,  1287,   851,  1031,   658,
     855,  1077,   252,   590,   320,   263,  1237,  1078,   741,   343,
     321,   895,   941,  1667,   425,   813,   777,   943,   594,   780,
     783,   784,   299,   690,   322,   304,   730,   400,   307,   308,
     901,   305,   418,   419,    13,   551,   307,   308,   713,   315,
    1096,   299,   346,   905,  1245,  1727,   304,   658,   299,   299,
     803,   343,   556,  1034,   343,  1238,   714,  1404,  1652,   720,
    1014,   895,   868,   307,   308,  -774,  1126,  1300,   933,  1774,
    1788,  1619,  1127,   326,   893,  1653,   418,   419,  1654,   327,
    1255,   344,   358,  1257,   349,  -777,   574,   299,   359,  1728,
     605,   304,   306,   307,   308,  1334,  -774,   336,  1102,   360,
    1335,   885,    59,    60,    61,   179,  1336,   354,  1337,   343,
    -493,   857,   858,   557,   307,   308,  -777,   452,   392,   343,
     866,    59,    60,    61,   179,   180,   354,  1015,   361,   304,
     934,  1382,  -775,  1383,  1477,   339,   362,   875,    59,    60,
      61,   179,   180,   354,   304,  1338,  1339,  -917,  1340,   886,
     556,   438,  -917,   402,  1232,   816,   817,   792,   363,   307,
     308,   796,   393,  -775,   394,   964,   284,  1233,   344,   561,
     562,   396,   355,   344,    59,    60,    61,    62,    63,   354,
     395,   894,   165,   552,   424,    69,   397,   558,   938,   939,
     196,   355,   122,  -803,  1354,  -494,   122,   307,   308,  -917,
     471,   430,  -917,  1234,   218,   220,  1225,  -917,   355,   904,
    1697,  1749,   307,   308,   552,  1360,   558,   552,   558,   558,
    -636,   485,   574,   399,   435,  1674,   311,  1698,  1129,  1130,
    1699,    59,    60,    61,    62,    63,   354,  1415,  1416,  1261,
    1144,   936,    69,   397,   355,   573,  1476,   437,  1269,  1764,
    1615,  1616,  1372,   390,   386,   387,   388,   243,   389,  1768,
    1769,   590,  1695,  1696,  1778,  1691,  1692,   942,   659,   344,
     390,   967,   970,   136,   444,  1289,   610,   610,   398,   425,
     399,  1010,  1011,  1012,   447,   448,   122,  -635,   453,   364,
     365,   366,   332,   454,   462,   607,   953,  1013,   475,   263,
    1016,   355,   299,   479,   130,  1146,  1147,  1148,   367,  -634,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,   386,   387,
     388,   480,   389,   659,  -633,   483,  1623,  1294,  1147,  1148,
     136,  1753,  1754,  1755,   390,   484,   486,  1121,   657,   299,
     661,   299,   299,  1762,   487,   489,  1053,   529,   530,  1351,
     574,   338,   340,   341,   531,  1056,   542,   543,  1775,  1061,
     582,   130,   685,   592,  1474,  1064,   593,  1065,   583,  1066,
     604,   573,  1101,    52,   595,   136,   703,   -65,  1068,   131,
    1157,   133,   442,   129,   134,   617,   707,  1161,   728,   445,
    1080,   701,  1083,   706,  1374,   451,   712,   464,   717,   735,
     732,   747,  1091,   136,   748,  1092,   130,  1093,   165,   122,
     776,   615,   165,   775,   383,   384,   385,   386,   387,   388,
     778,   389,   779,  1251,  1742,   787,   773,   794,   516,   798,
     790,   806,   520,   390,   130,   219,   219,   793,    36,   797,
     212,   810,  1742,   811,   490,   491,   492,  1125,   814,   820,
    1763,   493,   494,   815,   526,   495,   496,   822,   831,   836,
     825,   690,   837,   534,   534,   539,   802,  1132,   844,   839,
     546,   845,   847,   848,   849,  1675,   555,  1228,  1151,   850,
    1152,   853,   136,   854,   136,   655,   856,   859,  1133,   863,
     864,   828,   830,  1329,  -497,   233,   871,   873,   876,   163,
     877,  1462,   165,   879,   882,   888,   889,   891,   892,   573,
     896,   906,   908,   130,   909,   130,   910,    84,    85,   884,
      86,   184,    88,   935,  1273,   955,   720,   945,   957,   590,
     961,   962,  1208,  1209,  1210,   963,   965,   978,   780,  1212,
     980,   574,   986,   981,  1017,   590,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,  1226,
     982,  1390,   299,   983,   984,   656,  1227,   117,  1007,  1008,
    1009,  1010,  1011,  1012,   987,  1311,  1709,  1399,  1019,  1315,
    1023,  1018,  1319,  1026,  1029,  1047,  1039,  1013,  1051,  1324,
    1244,  1052,   866,  1041,  1060,  1071,   136,  1253,   546,  1067,
     131,  1058,   133,  1069,   129,   134,  1086,  1095,   442,  1098,
     615,  1104,  1105,  1116,  1115,  1119,  1120,  1264,   574,   615,
    1227,  1122,  1141,   219,  1135,  1137,  1140,   130,  1143,  1155,
     219,  1159,  1013,  1156,  1160,   165,   219,  1214,  1220,   551,
    1221,  1224,  1240,   923,  1242,   929,  1249,  1250,  1254,  1752,
    1258,  1256,   243,  1260,  1263,  1262,  1271,  1270,  1282,  1265,
    1272,   919,  1296,  1276,   122,  1283,  1284,  1478,  1286,  1291,
    1302,  1295,  1301,  1304,  1080,  1309,  1484,  1305,   951,   122,
    1308,  1285,  1310,  1314,  1312,   136,  1313,  1317,  1318,  1388,
    1491,  1322,  1320,  1323,  1364,   219,  1328,  1365,  1367,  1353,
     573,  1352,  1369,  1380,   219,   219,   540,    33,    34,    35,
    1371,   219,  1375,  1379,  1385,  1450,   130,   219,  1381,   754,
     163,  1384,  1386,  1387,  1389,  1412,  1391,  1392,  1421,  1459,
    1393,   122,  1396,  1397,  1414,  1432,  1448,  1033,  1460,  1331,
    1453,  1036,  1037,  1471,  1468,  1463,  1469,  1475,  1485,  1356,
     357,   590,  1496,  1357,   590,  1358,  1359,  1487,  1502,  1629,
    1503,  1044,  1596,  1345,  1366,  1597,  1603,    73,    74,    75,
      76,    77,  1345,  -942,  1373,   615,   122,   573,   756,   874,
    1609,  1610,  1614,  1612,    80,    81,  1613,  1624,  1655,  1622,
    1063,  1660,  1635,  1636,  1663,  1664,  1671,  1599,    90,  1693,
    1701,  1702,  1707,  1708,   122,  1715,  1716,  1717,  -286,  1719,
     685,  1723,  1650,  1720,  1724,    98,  1726,  1738,  1729,  1733,
    1744,  1400,  1731,  1401,  1732,  1747,  1751,  1418,  1750,  1759,
    1781,  1761,  1765,  1776,   903,  1782,  1789,  1790,  1766,   219,
    1795,  1796,  1411,   299,  1798,  1035,  1799,  1032,  1746,   219,
     801,   663,  1097,   660,  1100,  1109,  1109,   923,   662,  1057,
    1760,  1288,  1628,  1758,  1443,  1620,   804,  1398,  1643,  1648,
     136,  1439,  1783,  1771,  1659,   717,   930,   242,   931,  1461,
    1422,   122,   615,   122,  1618,   122,   670,  1350,  1495,  1213,
     165,   402,  1669,  1211,  1670,  1025,  1350,   785,  1235,   786,
    1268,   130,   949,  1676,  1153,   165,  1162,  1274,  1111,  1123,
    1275,  1345,  1079,   548,  1500,  1173,  1441,  1345,   581,  1345,
     968,  1154,  1163,   590,  1219,  1207,     0,     0,  1176,     0,
       0,   136,     0,  1492,   536,     0,     0,  1601,     0,  1602,
     136,     0,     0,     0,  1482,     0,     0,     0,     0,  1712,
       0,     0,     0,     0,     0,     0,   773,   165,     0,     0,
    1662,     0,   130,     0,     0,     0,     0,     0,     0,     0,
       0,   130,     0,  1216,  1611,  1040,  1217,   433,   404,   405,
     406,   407,   408,   409,   410,   411,   412,   413,   414,   415,
     546,  1050,     0,     0,     0,     0,     0,   122,     0,     0,
       0,     0,   165,     0,     0,  1633,     0,     0,     0,     0,
       0,     0,     0,     0,  1345,     0,     0,     0,     0,   136,
     219,  1640,  1604,     0,     0,   136,   416,   417,  1626,  1482,
     165,     0,   136,     0,     0,  1350,     0,     0,     0,  1333,
       0,  1350,     0,  1350,     0,   590,     0,  1777,     0,     0,
     130,     0,     0,     0,  1784,     0,   130,     0,   923,     0,
       0,     0,   923,   130,     0,     0,     0,     0,     0,     0,
    1658,     0,     0,     0,   122,   219,     0,     0,     0,     0,
      13,     0,     0,     0,     0,  1736,   122,     0,     0,     0,
       0,   418,   419,  -943,  -943,  -943,  -943,   381,   382,   383,
     384,   385,   386,   387,   388,     0,   389,   165,     0,   165,
       0,   165,     0,   949,  1139,     0,     0,   219,   390,   219,
       0,     0,  1661,     0,  1665,     0,     0,     0,     0,     0,
       0,     0,     0,  1705,     0,     0,     0,     0,  1350,     0,
       0,  1334,   357,   219,     0,     0,  1335,     0,    59,    60,
      61,   179,  1336,   354,  1337,     0,     0,     0,     0,     0,
       0,  1330,     0,     0,     0,   594,     0,     0,     0,     0,
     136,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   215,   215,     0,     0,   228,     0,     0,     0,
       0,  1338,  1339,     0,  1340,     0,     0,     0,     0,     0,
       0,   130,     0,     0,     0,  1721,     0,     0,     0,   228,
     136,   136,   923,     0,   923,     0,   219,   136,   355,     0,
       0,     0,     0,   165,     0,     0,     0,     0,     0,     0,
       0,   219,   219,     0,     0,     0,     0,     0,     0,     0,
    1466,   130,   130,     0,     0,     0,     0,     0,   130,  1252,
       0,     0,     0,   136,     0,     0,     0,     0,     0,     0,
       0,  1737,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   122,     0,     0,     0,   866,   263,     0,     0,     0,
       0,     0,  1437,     0,   130,     0,     0,     0,     0,     0,
     866,     0,  1281,     0,     0,     0,  1791,     0,     0,     0,
     165,     0,     0,     0,  1797,     0,     0,     0,   546,   949,
    1800,     0,   165,  1801,     0,     0,     0,     0,   136,     0,
       0,     0,     0,     0,     0,   136,     0,   923,     0,     0,
       0,     0,   122,     0,  1333,     0,     0,   122,     0,     0,
       0,   122,     0,     0,     0,     0,     0,     0,     0,   130,
       0,     0,     0,     0,   219,   219,   130,     0,     0,   299,
       0,     0,     0,   263,     0,     0,     0,     0,     0,  1333,
     215,     0,  1585,     0,     0,    13,     0,   215,  1592,     0,
       0,     0,     0,   215,     0,     0,   263,     0,   263,     0,
     590,     0,   546,     0,   263,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   590,     0,
      13,   228,   228,     0,     0,     0,   590,   923,   228,     0,
     122,   122,   122,     0,     0,     0,   122,     0,     0,     0,
       0,     0,     0,   122,     0,     0,  1334,     0,     0,     0,
       0,  1335,   215,    59,    60,    61,   179,  1336,   354,  1337,
       0,   215,   215,     0,     0,     0,     0,     0,   215,     0,
       0,     0,     0,     0,   215,     0,     0,     0,     0,     0,
       0,  1334,     0,     0,     0,   228,  1335,     0,    59,    60,
      61,   179,  1336,   354,  1337,     0,  1338,  1339,  1333,  1340,
     219,     0,     0,     0,     0,     0,     0,   165,     0,     0,
     228,     0,     0,   228,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   355,     0,     0,     0,     0,     0,     0,
       0,  1338,  1339,     0,  1340,     0,     0,     0,     0,    13,
       0,     0,     0,   219,     0,  1470,     0,     0,     0,     0,
       0,   299,     0,     0,     0,   228,     0,     0,   355,   219,
     219,     0,     0,     0,     0,     0,     0,     0,   165,     0,
       0,     0,   263,   165,     0,     0,   923,   165,     0,     0,
    1472,   122,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1681,     0,     0,     0,     0,   215,  1585,  1585,     0,
    1334,  1592,  1592,     0,     0,  1335,   215,    59,    60,    61,
     179,  1336,   354,  1337,   299,     0,     0,     0,     0,     0,
       0,   122,   122,     0,     0,     0,     0,     0,   122,   433,
     404,   405,   406,   407,   408,   409,   410,   411,   412,   413,
     414,   415,     0,   219,     0,   228,   228,     0,     0,   763,
    1338,  1339,     0,  1340,     0,     0,   165,   165,   165,     0,
       0,     0,   165,     0,   122,     0,     0,     0,     0,   165,
       0,  1735,     0,     0,     0,     0,     0,   355,   416,   417,
       0,     0,   364,   365,   366,     0,     0,     0,  1748,     0,
       0,     0,     0,     0,     0,     0,     0,   763,     0,  1621,
       0,   367,     0,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,     0,   389,    36,     0,   212,   122,
       0,     0,     0,     0,     0,     0,   122,   390,     0,     0,
       0,     0,     0,   418,   419,     0,     0,     0,   228,   228,
       0,     0,     0,     0,     0,     0,     0,   228,     0,   364,
     365,   366,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   215,   367,     0,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,   386,   387,
     388,     0,   389,   683,     0,    84,    85,   165,    86,   184,
      88,     0,     0,     0,   390,     0,     0,   812,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   215,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   165,   165,     0,
       0,     0,     0,   684,   165,   117,   403,   404,   405,   406,
     407,   408,   409,   410,   411,   412,   413,   414,   415,     0,
       0,     0,     0,     0,   215,     0,   215,   832,  -943,  -943,
    -943,  -943,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,
     165,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     215,   763,     0,  1013,     0,   416,   417,     0,   364,   365,
     366,     0,     0,   228,   228,   763,   763,   763,   763,   763,
       0,     0,     0,     0,     0,     0,     0,   367,   763,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
       0,   389,     0,     0,   870,   165,   228,     0,     0,     0,
       0,     0,   165,   390,     0,     0,     0,     0,     0,     0,
     418,   419,     0,   215,   255,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   228,     0,   215,   215,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     256,     0,   228,   228,     0,     0,     0,     0,     0,     0,
       0,   228,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,     0,   228,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   228,     0,     0,
       0,     0,     0,     0,     0,   763,     0,     0,   228,     0,
    -330,     0,     0,     0,     0,     0,     0,     0,    59,    60,
      61,   179,   180,  1442,     0,     0,     0,     0,   228,     0,
       0,     0,     0,     0,     0,     0,     0,   258,   259,     0,
       0,   217,   217,     0,     0,   230,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   183,     0,     0,    82,   260,
       0,    84,    85,   907,    86,   184,    88,     0,     0,     0,
       0,   215,   215,     0,     0,     0,     0,     0,     0,   261,
       0,     0,     0,     0,     0,   228,     0,   228,   355,   228,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,   763,     0,   262,   228,   763,   763,
     763,     0,     0,   763,   763,   763,   763,   763,   763,   763,
     763,   763,   763,   763,   763,   763,   763,   763,   763,   763,
     763,   763,   763,   763,   763,   763,   763,   763,   763,   763,
       0,     0,     0,    36,     0,   216,   216,     0,     0,   229,
       0,     0,     0,     0,     0,   364,   365,   366,     0,     0,
       0,     0,     0,     0,     0,   763,     0,     0,     0,     0,
       0,     0,   265,     0,   367,  1297,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,   386,   387,   388,   228,   389,   228,
       0,     0,     0,     0,     0,     0,     0,   215,     0,     0,
     390,     0,     0,     0,     0,     0,   183,     0,     0,    82,
       0,     0,    84,    85,   228,    86,   184,    88,     0,   217,
       0,     0,     0,     0,     0,     0,   217,     0,     0,     0,
       0,     0,   217,     0,     0,   228,     0,     0,     0,     0,
     215,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   215,   215,     0,   763,
       0,  1680,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   228,     0,     0,   763,     0,   763,   433,   404,   405,
     406,   407,   408,   409,   410,   411,   412,   413,   414,   415,
       0,   217,     0,     0,     0,     0,     0,     0,     0,   763,
     217,   217,     0,     0,     0,     0,     0,   217,     0,     0,
       0,     0,     0,   217,     0,     0,     0,     0,     0,     0,
     364,   365,   366,     0,   572,     0,   416,   417,     0,     0,
     216,     0,     0,     0,     0,     0,   228,   228,  1298,   367,
     215,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,   386,
     387,   388,     0,   389,   265,   265,     0,     0,     0,     0,
       0,   265,     0,     0,     0,   390,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   418,   419,     0,   230,   216,     0,     0,     0,     0,
       0,     0,     0,     0,   216,   216,     0,     0,     0,     0,
       0,   216,     0,     0,     0,     0,     0,   216,   228,     0,
     228,     0,     0,     0,     0,   763,   228,     0,   216,     0,
     763,     0,   763,     0,   763,   217,     0,   763,     0,   763,
       0,     0,   763,     0,     0,   217,   364,   365,   366,     0,
     228,   228,     0,   265,   228,     0,   265,     0,     0,     0,
       0,   228,     0,     0,    36,   367,   212,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,     0,   389,
       0,     0,     0,     0,     0,     0,     0,     0,   229,     0,
       0,   390,     0,   228,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   911,     0,     0,   763,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     228,   228,   228,     0,     0,     0,     0,   228,     0,   216,
       0,   683,     0,    84,    85,     0,    86,   184,    88,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     228,     0,   228,     0,     0,     0,     0,     0,   228,     0,
       0,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,     0,   265,     0,
       0,   716,   769,   117,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   763,   763,   763,     0,     0,     0,     0,
       0,     0,   763,   228,     0,    36,   217,   212,   228,     0,
     228,     0,     0,   364,   365,   366,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     769,  1038,   367,     0,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,     0,   389,     0,     0,     0,
       0,   217,     0,     0,    36,     0,     0,     0,   390,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   265,   265,     0,    84,    85,     0,    86,   184,    88,
     265,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   217,     0,   217,     0,     0,     0,   228,
     216,     0,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   228,     0,     0,   217,
       0,     0,   656,     0,   117,     0,     0,     0,     0,     0,
     311,     0,     0,    84,    85,   228,    86,   184,    88,     0,
     763,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   763,     0,     0,     0,   216,   763,     0,     0,     0,
     763,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,     0,     0,     0,
       0,   312,   228,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   217,     0,     0,     0,     0,   216,  1094,   216,
       0,     0,     0,     0,     0,     0,     0,   217,   217,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     763,     0,     0,   216,   769,     0,     0,     0,   228,     0,
     572,     0,     0,     0,     0,     0,   265,   265,   769,   769,
     769,   769,   769,   228,     0,     0,     0,     0,     0,   264,
       0,   769,   228,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   228,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,  1028,
       0,     0,     0,     0,     0,     0,     0,   230,     0,     0,
       0,     0,     0,     0,     0,     0,   216,     0,   279,   280,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1046,
       0,   216,   216,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   265,  1046,     0,     0,     0,
     217,   217,     0,     0,   216,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   281,     0,   265,    84,
      85,     0,    86,   184,    88,     0,     0,     0,     0,     0,
     265,     0,     0,     0,     0,     0,   572,     0,   769,     0,
       0,  1087,     0,   364,   365,   366,     0,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   229,   367,     0,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,     0,   389,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   390,     0,
       0,     0,     0,     0,   216,   216,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,     0,   265,     0,
     265,   264,   264,     0,     0,     0,     0,     0,   264,     0,
       0,     0,     0,     0,     0,     0,   217,   769,     0,     0,
     216,   769,   769,   769,     0,     0,   769,   769,   769,   769,
     769,   769,   769,   769,   769,   769,   769,   769,   769,   769,
     769,   769,   769,   769,   769,   769,   769,   769,   769,   769,
     769,   769,   769,     0,   572,     0,     0,     0,     0,   217,
       0,     0,     0,     0,     0,     0,  1590,     0,    84,    85,
    1591,    86,   184,    88,     0,   217,   217,     0,   769,     0,
       0,     0,     0,     0,    29,    30,     0,     0,     0,     0,
     264,     0,     0,   264,    36,     0,    38,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     265,     0,   265,  1436,     0,     0,     0,     0,  1103,     0,
     216,     0,    52,     0,     0,     0,     0,     0,     0,     0,
      59,    60,    61,   179,   180,   181,     0,   265,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,     0,   389,   216,   217,
       0,     0,     0,   216,     0,     0,     0,   183,     0,   390,
      82,    83,     0,    84,    85,     0,    86,   184,    88,   216,
     216,     0,   769,     0,     0,    91,     0,     0,     0,     0,
       0,     0,     0,     0,   265,     0,     0,   769,     0,   769,
      99,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,     0,     0,   440,     0,
       0,     0,   769,   117,     0,   264,   743,     0,     0,   765,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   572,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   265,
    1332,     0,     0,   216,   364,   365,   366,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   765,     0,     0,
       0,     0,     0,   367,     0,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,     0,   389,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   390,
       0,     0,   572,     0,     0,     0,     0,     0,   264,   264,
       0,     0,     0,     0,     0,     0,     0,   264,     0,     0,
       0,   265,     0,   265,     0,     0,     0,     0,   769,   216,
       0,     0,     0,   769,    36,   769,     0,   769,     0,     0,
     769,     0,   769,     0,     0,   769,   364,   365,   366,     0,
       0,     0,     0,   265,  1420,     0,     0,  1431,     0,     0,
       0,     0,     0,     0,   265,   367,     0,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,     0,   389,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   390,     0,     0,     0,     0,   216,     0,     0,     0,
       0,     0,     0,    84,    85,     0,    86,   184,    88,     0,
       0,   769,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   265,  1493,  1494,     0,     0,     0,  1128,
     265,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,     0,   617,     0,
       0,   765,     0,   265,     0,   265,     0,     0,     0,     0,
       0,   265,     0,   264,   264,   765,   765,   765,   765,   765,
       0,     0,     0,     0,     0,   255,     0,     0,   765,   996,
     997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,   769,   769,   769,     0,
       0,   256,     0,     0,     0,   769,  1638,     0,     0,  1013,
       0,   265,     0,  1431,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,  1455,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,   386,
     387,   388,   264,   389,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   770,   257,   390,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   264,     0,     0,   258,   259,
       0,     0,     0,     0,     0,     0,     0,   264,     0,     0,
       0,     0,     0,     0,     0,   765,   183,     0,     0,    82,
     260,     0,    84,    85,     0,    86,   184,    88,     0,     0,
       0,   770,   265,     0,     0,     0,     0,     0,     0,     0,
     261,     0,     0,     0,     0,     0,     0,     0,     0,   265,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,     0,   262,     0,     0,
       0,  1605,     0,   769,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   769,     0,     0,     0,     0,   769,
       0,     0,     0,   769,     0,   264,     0,   264,     0,   743,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   765,   265,     0,     0,   765,   765,
     765,     0,     0,   765,   765,   765,   765,   765,   765,   765,
     765,   765,   765,   765,   765,   765,   765,   765,   765,   765,
     765,   765,   765,   765,   765,   765,   765,   765,   765,   765,
       0,     0,     0,   769,     0,     0,     0,     0,     0,     0,
       0,  1745,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   765,  1420,     0,     0,     0,
       0,     0,     0,     0,     0,   265,     0,     0,     0,   364,
     365,   366,     0,     0,     0,     0,     0,   764,     0,     0,
     265,     0,     0,     0,     0,     0,     0,   264,   367,   264,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,   386,   387,
     388,     0,   389,     0,   264,   770,     0,     0,     0,     0,
       0,     0,     0,     0,   390,   764,     0,     0,     0,   770,
     770,   770,   770,   770,     0,     0,     0,     0,     0,     0,
       0,     0,   770,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   988,   989,   990,   765,
       0,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,   264,     0,     0,   765,   991,   765,   992,   993,   994,
     995,   996,   997,   998,   999,  1000,  1001,  1002,  1003,  1004,
    1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,     0,   765,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1435,
       0,  1013,     0,   367,     0,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,   264,   389,     0,     0,
       0,     0,     0,    84,    85,     0,    86,   184,    88,   390,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   770,
       0,     0,     0,     0,  1456,     0,     0,     0,   364,   365,
     366,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   771,     0,   367,  1436,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
       0,   389,     0,     0,     0,     0,     0,     0,   264,     0,
     264,     0,     0,   390,     0,   765,     0,     0,     0,   764,
     765,    36,   765,   805,   765,     0,     0,   765,     0,   765,
     742,     0,   765,   764,   764,   764,   764,   764,  1174,     0,
     264,     0,     0,     0,     0,     0,   764,     0,   770,     0,
       0,   264,   770,   770,   770,     0,     0,   770,   770,   770,
     770,   770,   770,   770,   770,   770,   770,   770,   770,   770,
     770,   770,   770,   770,   770,   770,   770,   770,   770,   770,
     770,   770,   770,   770,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   183,     0,     0,    82,     0,     0,
      84,    85,     0,    86,   184,    88,     0,     0,   765,   770,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     264,     0,     0,     0,     0,     0,     0,   264,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,   364,   365,   366,     0,     0,     0,     0,
     264,   391,   264,     0,     0,     0,     0,     0,   264,     0,
       0,     0,   367,   764,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,     0,   389,     0,     0,     0,
       0,     0,     0,   765,   765,   765,     0,     0,   390,     0,
       0,     0,   765,     0,     0,     0,     0,     0,   264,     0,
       0,     0,     0,   770,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   950,   770,     0,
     770,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   971,   972,   973,   974,     0,     0,     0,     0,     0,
       0,     0,     0,   770,   985,     0,     0,     0,     0,     0,
       0,     0,   764,     0,     0,     0,   764,   764,   764,     0,
       0,   764,   764,   764,   764,   764,   764,   764,   764,   764,
     764,   764,   764,   764,   764,   764,   764,   764,   764,   764,
     764,   764,   764,   764,   764,   764,   764,   764,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   264,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   764,     0,     0,   264,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   474,     0,     0,     0,
       0,    36,     0,     0,     0,  1682,     0,     0,     0,     0,
     765,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   765,     0,     0,     0,     0,   765,     0,     0,     0,
     765,  1084,     0,     0,     0,     0,   364,   365,   366,   770,
       0,     0,     0,     0,   770,     0,   770,     0,   770,     0,
       0,   770,   264,   770,     0,   367,   770,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,     0,   389,
      84,    85,     0,    86,   184,    88,     0,   764,     0,     0,
     765,   390,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   764,     0,   764,     0,     0,     0,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   264,     0,     0,   884,     0,   764,     0,     0,
       0,     0,   770,     0,  1167,  1170,  1170,   264,     0,  1177,
    1180,  1181,  1182,  1184,  1185,  1186,  1187,  1188,  1189,  1190,
    1191,  1192,  1193,  1194,  1195,  1196,  1197,  1198,  1199,  1200,
    1201,  1202,  1203,  1204,  1205,  1206,     0,     0,     0,     0,
       0,     0,     0,   364,   365,   366,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1218,   367,     0,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,     0,   389,   770,   770,   770,
       0,     0,     0,     0,     0,     0,   770,     0,   390,   476,
       0,     0,     0,  1642,     0,     0,     0,     0,     0,     0,
    1106,  1107,  1108,    36,     0,     0,     0,     0,     0,     0,
     364,   365,   366,   764,   946,     0,     0,     0,   764,     0,
     764,     0,   764,     0,     0,   764,     0,   764,     0,   367,
     764,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,   386,
     387,   388,     0,   389,     0,  1292,    36,     0,   212,     0,
       0,     0,     0,     0,     0,   390,     0,     0,     0,     0,
    1306,     0,  1307,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    84,    85,     0,    86,   184,    88,     0,     0,
       0,     0,     0,     0,     0,  1325,     0,   213,     0,     0,
       0,     0,     0,     0,     0,     0,   764,     0,     0,     0,
     947,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   488,     0,     0,   183,
       0,     0,    82,    83,   770,    84,    85,     0,    86,   184,
      88,     0,     0,     0,     0,   770,     0,     0,     0,     0,
     770,     0,     0,     0,   770,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,  1722,     0,
     214,     0,     0,     0,     0,   117,     0,     0,     0,     0,
       0,   764,   764,   764,     0,     0,     0,     0,     0,     0,
     764,     0,     0,   512,     0,     0,     0,     0,     0,     0,
      36,     0,   826,   827,   770,     0,     0,     0,     0,     0,
       0,  1403,     0,     0,     0,     0,  1405,     0,  1406,     0,
    1407,     0,     0,  1408,     0,  1409,     0,     0,  1410,     0,
       0,     0,     0,     0,     0,   364,   365,   366,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   367,     0,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,   386,   387,   388,     0,   389,    84,
      85,     0,    86,   184,    88,     0,     0,     0,     0,     0,
     390,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1486,     0,     0,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,   764,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,   764,
       0,     0,     0,     0,   764,     0,     0,     0,   764,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,  1630,
    1631,  1632,    33,    34,    35,    36,    37,    38,  1637,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,     0,   764,     0,
      49,    50,    51,    52,    53,    54,    55,   699,    56,    57,
      58,    59,    60,    61,    62,    63,    64,     0,    65,    66,
      67,    68,    69,    70,     0,     0,     0,     0,     0,    71,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,    79,    80,
      81,    82,    83,     0,    84,    85,     0,    86,    87,    88,
      89,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,    93,    94,    95,    96,     0,    97,     0,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,     0,     0,   114,
       0,   115,   116,  1054,   117,   118,     0,   119,   120,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,  1690,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,  1700,     0,     0,
       0,     0,  1704,     0,     0,     0,  1706,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,     0,  1739,     0,    49,    50,
      51,    52,    53,    54,    55,     0,    56,    57,    58,    59,
      60,    61,    62,    63,    64,     0,    65,    66,    67,    68,
      69,    70,     0,     0,     0,     0,     0,    71,    72,     0,
      73,    74,    75,    76,    77,     0,     0,     0,     0,     0,
       0,    78,     0,     0,     0,     0,    79,    80,    81,    82,
      83,     0,    84,    85,     0,    86,    87,    88,    89,     0,
       0,    90,     0,     0,    91,     0,     0,     0,     0,     0,
      92,    93,    94,    95,    96,     0,    97,     0,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,     0,     0,   114,     0,   115,
     116,  1229,   117,   118,     0,   119,   120,     5,     6,     7,
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
      53,    54,    55,     0,    56,    57,    58,    59,    60,    61,
      62,    63,    64,     0,    65,    66,    67,    68,    69,    70,
       0,     0,     0,     0,     0,    71,    72,     0,    73,    74,
      75,    76,    77,     0,     0,     0,     0,     0,     0,    78,
       0,     0,     0,     0,    79,    80,    81,    82,    83,     0,
      84,    85,     0,    86,    87,    88,    89,     0,     0,    90,
       0,     0,    91,     0,     0,     0,     0,     0,    92,    93,
      94,    95,    96,     0,    97,     0,    98,    99,     0,   100,
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
     113,     0,     0,   114,     0,   115,   116,   596,   117,   118,
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
       0,   114,     0,   115,   116,  1027,   117,   118,     0,   119,
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
       0,   115,   116,  1070,   117,   118,     0,   119,   120,     5,
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
     116,  1134,   117,   118,     0,   119,   120,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,  1136,    45,     0,    46,     0,    47,
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
      43,    44,     0,    45,     0,    46,     0,    47,  1293,     0,
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
     113,     0,     0,   114,     0,   115,   116,     0,   117,   118,
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
       0,   114,     0,   115,   116,  1413,   117,   118,     0,   119,
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
       0,   115,   116,  1634,   117,   118,     0,   119,   120,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
    1677,    47,     0,     0,    48,     0,     0,     0,    49,    50,
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
     111,   112,   113,     0,     0,   114,     0,   115,   116,  1710,
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
     113,     0,     0,   114,     0,   115,   116,  1711,   117,   118,
       0,   119,   120,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,  1714,    46,     0,    47,     0,     0,    48,     0,
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
       0,   115,   116,  1730,   117,   118,     0,   119,   120,     5,
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
     116,  1785,   117,   118,     0,   119,   120,     5,     6,     7,
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
     111,   112,   113,     0,     0,   114,     0,   115,   116,  1792,
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
     113,     0,     0,   114,     0,   115,   116,     0,   117,   118,
       0,   119,   120,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
     460,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    11,    12,     0,   729,     0,
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
       0,     0,     0,    11,    12,     0,   952,     0,     0,     0,
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
       0,    11,    12,     0,  1481,     0,     0,     0,     0,     0,
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
      12,     0,  1625,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,   665,    12,     0,     0,     0,
       0,     0,     0,   666,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
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
       0,     0,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,    93,    94,    95,     0,     0,     0,     0,
      98,    99,   270,   100,   101,   102,   103,   104,   105,   106,
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
     270,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,     0,     0,   114,     0,   271,
       0,     0,   117,   118,     0,   119,   120,     5,     6,     7,
       8,     9,     0,     0,     0,     0,   991,    10,   992,   993,
     994,   995,   996,   997,   998,   999,  1000,  1001,  1002,  1003,
    1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,   611,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,  1013,     0,     0,     0,    16,     0,    17,    18,
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
      84,    85,     0,    86,   184,    88,     0,   612,     0,    90,
       0,     0,    91,     0,     0,     0,     0,     0,    92,    93,
      94,    95,     0,     0,     0,     0,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,     0,     0,   114,     0,     0,     0,     0,
     117,   118,     0,   119,   120,     5,     6,     7,     8,     9,
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
       0,     0,     0,     0,     0,    59,    60,    61,   179,   180,
     181,     0,     0,    66,    67,     0,     0,     0,     0,     0,
       0,     0,     0,   182,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   183,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   184,    88,     0,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,    93,    94,    95,
       0,     0,     0,     0,    98,    99,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,     0,     0,   114,   365,   366,   724,     0,   117,   118,
       0,   119,   120,     5,     6,     7,     8,     9,     0,     0,
       0,     0,   367,    10,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,  1081,   389,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,   390,     0,
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
     184,    88,     0,  1082,     0,    90,     0,     0,    91,     0,
       0,     0,     0,     0,    92,    93,    94,    95,     0,     0,
       0,     0,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,     0,
       0,   114,     0,     0,     0,     0,   117,   118,     0,   119,
     120,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   665,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
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
       0,     0,     0,     0,   117,   118,     0,   119,   120,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   195,     0,     0,    52,
       0,     0,     0,     0,     0,     0,     0,    59,    60,    61,
     179,   180,   181,     0,    36,    66,    67,     0,     0,     0,
       0,     0,     0,     0,     0,   182,    72,     0,    73,    74,
      75,    76,    77,     0,     0,     0,     0,     0,     0,    78,
       0,     0,     0,     0,   183,    80,    81,    82,    83,     0,
      84,    85,     0,    86,   184,    88,     0,     0,     0,    90,
       0,   655,    91,     0,     0,     0,     0,     0,    92,    93,
      94,    95,     0,     0,     0,     0,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,    84,    85,   114,    86,   184,    88,     0,
     117,   118,     0,   119,   120,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   221,     0,     0,
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
       0,   250,     0,     0,   117,   118,     0,   119,   120,     0,
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
     111,   112,   113,     0,     0,   114,     0,   253,     0,     0,
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
     184,    88,     0,     0,     0,    90,     0,     0,    91,     0,
       0,     0,     0,     0,    92,    93,    94,    95,     0,     0,
       0,     0,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,     0,
       0,   114,   458,     0,     0,     0,   117,   118,     0,   119,
     120,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   624,   389,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   390,     0,     0,    14,    15,     0,     0,     0,     0,
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
       0,     0,     0,    90,     0,  1175,    91,     0,     0,     0,
       0,     0,    92,    93,    94,    95,     0,     0,     0,     0,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,    84,    85,   114,
      86,   184,    88,     0,   117,   118,     0,   119,   120,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   666,     0,     0,     0,     0,     0,     0,     0,     0,
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
     106,   107,   108,   109,   110,   111,   112,     0,     0,   708,
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
       0,     0,    91,     0,     0,     0,     0,     0,    92,    93,
      94,    95,     0,     0,     0,     0,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,     0,     0,   114,   989,   990,     0,     0,
     117,   118,     0,   119,   120,     5,     6,     7,     8,     9,
       0,     0,     0,     0,   991,    10,   992,   993,   994,   995,
     996,   997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,
    1006,  1007,  1008,  1009,  1010,  1011,  1012,   710,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
    1013,     0,     0,     0,    16,     0,    17,    18,    19,    20,
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
       0,     0,     0,     0,    98,    99,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,     0,     0,   114,     0,   366,     0,     0,   117,   118,
       0,   119,   120,     5,     6,     7,     8,     9,     0,     0,
       0,     0,   367,    10,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,  1124,   389,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,   390,     0,
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
     553,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    52,     0,     0,
       0,     0,     0,     0,     0,    59,    60,    61,   179,   180,
     181,     0,     0,    66,    67,     0,     0,     0,     0,     0,
       0,     0,     0,   182,    72,     0,    73,    74,    75,    76,
      77,     0,     0,     0,     0,     0,     0,    78,     0,     0,
       0,     0,   183,    80,    81,    82,    83,     0,    84,    85,
       0,    86,   184,    88,     0,     0,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,    93,    94,    95,
       0,     0,     0,     0,    98,    99,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,     0,     0,   114,     0,     0,     0,     0,   117,   118,
       0,   119,   120,  1506,  1507,  1508,  1509,  1510,     0,     0,
    1511,  1512,  1513,  1514,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1515,  1516,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
       0,   389,     0,  1517,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   390,     0,     0,     0,  1518,  1519,  1520,
    1521,  1522,  1523,  1524,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1525,  1526,  1527,
    1528,  1529,  1530,  1531,  1532,  1533,  1534,  1535,  1536,  1537,
    1538,  1539,  1540,  1541,  1542,  1543,  1544,  1545,  1546,  1547,
    1548,  1549,  1550,  1551,  1552,  1553,  1554,  1555,  1556,  1557,
    1558,  1559,  1560,  1561,  1562,  1563,  1564,  1565,     0,     0,
       0,  1566,  1567,     0,  1568,  1569,  1570,  1571,  1572,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1573,  1574,  1575,     0,     0,     0,    84,    85,     0,    86,
     184,    88,  1576,     0,  1577,  1578,     0,  1579,     0,     0,
       0,     0,     0,     0,  1580,     0,     0,     0,  1581,     0,
    1582,     0,  1583,  1584,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   364,   365,
     366,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   367,     0,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
       0,   389,   364,   365,   366,     0,     0,     0,     0,     0,
       0,     0,     0,   390,     0,     0,     0,     0,     0,     0,
       0,   367,     0,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,     0,   389,   364,   365,   366,     0,
       0,     0,     0,     0,     0,     0,     0,   390,     0,     0,
       0,     0,     0,     0,     0,   367,     0,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,     0,   389,
     364,   365,   366,     0,     0,     0,     0,     0,     0,     0,
       0,   390,     0,     0,     0,     0,     0,     0,     0,   367,
       0,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,   386,
     387,   388,     0,   389,     0,   364,   365,   366,     0,     0,
       0,     0,     0,     0,     0,   390,     0,     0,     0,     0,
     721,     0,     0,     0,   367,     0,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,   386,   387,   388,     0,   389,   364,
     365,   366,     0,     0,     0,     0,     0,     0,     0,  1024,
     390,     0,     0,     0,     0,     0,     0,     0,   367,     0,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,   386,   387,
     388,   255,   389,     0,     0,     0,     0,     0,     0,  1020,
    1021,     0,     0,     0,   390,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   256,     0,     0,
       0,     0,     0,   364,   365,   366,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    36,
    1678,   255,   367,     0,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,     0,   389,   256,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   390,     0,
     257,     0,     0,     0,  1490,     0,     0,     0,     0,    36,
       0,     0,     0,     0,   258,   259,  -943,  -943,  -943,  -943,
     377,   378,   379,   380,   381,   382,   383,   384,   385,   386,
     387,   388,   183,   389,     0,    82,   260,   481,    84,    85,
       0,    86,   184,    88,     0,   390,     0,  1327,     0,     0,
     257,     0,     0,     0,   255,     0,   261,     0,     0,     0,
       0,     0,     0,   436,   258,   259,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     256,     0,   183,   262,     0,    82,   260,  1673,    84,    85,
       0,    86,   184,    88,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,   255,     0,   261,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     256,     0,     0,   262,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   257,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,   255,     0,     0,   258,   259,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   183,     0,     0,    82,   260,
     256,    84,    85,     0,    86,   184,    88,     0,   966,     0,
       0,     0,     0,   257,     0,     0,     0,     0,     0,   261,
       0,     0,    36,     0,   255,     0,     0,   258,   259,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   183,   262,     0,    82,   260,
     256,    84,    85,     0,    86,   184,    88,     0,  1303,     0,
       0,     0,     0,   257,     0,     0,     0,     0,     0,   261,
       0,     0,    36,     0,     0,     0,     0,   258,   259,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   183,   262,     0,    82,   260,
       0,    84,    85,     0,    86,   184,    88,     0,     0,     0,
       0,     0,     0,   257,     0,     0,     0,     0,     0,   261,
       0,     0,     0,  1417,     0,     0,     0,   258,   259,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   183,   262,     0,    82,   260,
       0,    84,    85,     0,    86,   184,    88,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   261,
       0,     0,     0,     0,     0,     0,     0,  1183,     0,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   749,   750,     0,   262,     0,     0,   751,
       0,   752,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   753,     0,     0,     0,     0,     0,     0,
       0,    33,    34,    35,    36,     0,     0,     0,     0,     0,
       0,     0,     0,   754,     0,   988,   989,   990,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   991,     0,   992,   993,   994,   995,
     996,   997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,
    1006,  1007,  1008,  1009,  1010,  1011,  1012,    29,    30,   755,
       0,    73,    74,    75,    76,    77,     0,    36,     0,   212,
    1013,     0,   756,     0,     0,     0,     0,   183,    80,    81,
      82,   757,     0,    84,    85,     0,    86,   184,    88,     0,
      36,     0,    90,     0,     0,     0,     0,     0,     0,     0,
       0,   758,   759,   760,   761,     0,     0,     0,   213,    98,
       0,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   749,   750,     0,   762,     0,
       0,   751,     0,   752,     0,  1158,     0,     0,     0,     0,
     183,     0,     0,    82,    83,   753,    84,    85,     0,    86,
     184,    88,     0,    33,    34,    35,    36,     0,    91,     0,
       0,     0,     0,   183,     0,   754,    82,    83,     0,    84,
      85,     0,    86,   184,    88,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
       0,   440,     0,     0,     0,     0,   117,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   755,     0,    73,    74,    75,    76,    77,     0,     0,
       0,     0,     0,     0,   756,     0,     0,     0,     0,   183,
      80,    81,    82,   757,     0,    84,    85,     0,    86,   184,
      88,     0,     0,     0,    90,     0,     0,     0,     0,     0,
       0,     0,     0,   758,   759,   760,   761,   913,   914,     0,
       0,    98,     0,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   915,     0,     0,
     762,     0,     0,     0,     0,   916,   917,   918,    36,   364,
     365,   366,     0,     0,     0,   902,     0,   919,     0,     0,
       0,     0,     0,     0,    36,     0,   212,     0,   367,     0,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,   386,   387,
     388,     0,   389,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   920,   390,   213,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   921,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    84,    85,    36,
      86,   184,    88,     0,     0,     0,     0,   183,     0,     0,
      82,    83,     0,    84,    85,   922,    86,   184,    88,    36,
       0,   212,     0,     0,     0,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   446,
       0,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,     0,   214,     0,
     213,     0,     0,   117,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   351,     0,    84,    85,
       0,    86,   184,    88,    36,     0,     0,     0,     0,     0,
       0,     0,   183,     0,     0,    82,    83,     0,    84,    85,
       0,    86,   184,    88,    36,     0,   212,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,     0,     0,     0,     0,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,     0,   214,     0,   213,   525,     0,   117,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   545,     0,
     513,     0,     0,    84,    85,     0,    86,   184,    88,     0,
       0,     0,     0,     0,     0,     0,     0,   183,     0,     0,
      82,    83,     0,    84,    85,     0,    86,   184,    88,    36,
       0,   212,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,     0,     0,     0,
       0,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,     0,   214,     0,
     213,     0,     0,   117,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1049,    36,     0,   212,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   183,     0,     0,    82,    83,     0,    84,    85,
       0,    86,   184,    88,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   213,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
      36,     0,   212,   214,     0,     0,     0,   183,   117,     0,
      82,    83,     0,    84,    85,     0,    86,   184,    88,     0,
       0,     0,     0,    36,     0,   212,     0,     0,     0,     0,
       0,     0,   567,     0,     0,     0,     0,     0,     0,     0,
       0,   226,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,     0,   214,     0,
       0,     0,     0,   117,   213,     0,     0,     0,     0,     0,
       0,     0,     0,   183,     0,     0,    82,    83,     0,    84,
      85,     0,    86,   184,    88,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   183,     0,     0,    82,
      83,     0,    84,    85,     0,    86,   184,    88,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,     0,   227,     0,     0,     0,     0,   117,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   364,   365,   366,     0,     0,     0,
       0,     0,   568,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   367,     0,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,     0,   389,   364,   365,
     366,     0,     0,     0,     0,     0,     0,     0,     0,   390,
       0,     0,     0,     0,     0,     0,     0,   367,     0,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
       0,   389,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   390,   364,   365,   366,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   367,   852,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,     0,   389,   364,   365,
     366,     0,     0,     0,     0,     0,     0,     0,     0,   390,
       0,     0,     0,     0,     0,     0,     0,   367,   890,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
       0,   389,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   390,   364,   365,   366,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   367,   932,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,     0,   389,   988,   989,
     990,     0,     0,     0,     0,     0,     0,     0,     0,   390,
       0,     0,     0,     0,     0,     0,     0,   991,  1241,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,  1002,
    1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1013,   988,   989,   990,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   991,  1259,   992,   993,   994,   995,   996,
     997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,     0,     0,   988,   989,
     990,     0,     0,     0,     0,     0,     0,     0,     0,  1013,
       0,     0,     0,     0,     0,     0,     0,   991,  1316,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,  1002,
    1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1013,   988,   989,   990,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   991,  1321,   992,   993,   994,   995,   996,
     997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,    36,     0,   988,   989,
     990,     0,     0,     0,     0,     0,     0,     0,     0,  1013,
       0,     0,    36,     0,     0,     0,     0,   991,  1402,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,  1002,
    1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1013,  1423,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1424,  1425,     0,
       0,     0,   517,     0,  1488,    84,    85,     0,    86,   184,
      88,     0,     0,     0,     0,   183,     0,     0,    82,  1426,
       0,    84,    85,     0,    86,  1427,    88,     0,     0,     0,
       0,     0,     0,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,  1489,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   281,     0,     0,    84,    85,     0,    86,
     184,    88,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     364,   365,   366,     0,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   733,   367,
       0,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,   386,
     387,   388,     0,   389,     0,   364,   365,   366,     0,     0,
       0,     0,     0,     0,     0,   390,     0,     0,     0,     0,
       0,     0,     0,     0,   367,   887,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,   386,   387,   388,   734,   389,   364,
     365,   366,     0,     0,     0,     0,     0,     0,     0,     0,
     390,     0,     0,     0,     0,     0,     0,     0,   367,     0,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,   386,   387,
     388,     0,   389,   988,   989,   990,     0,     0,     0,     0,
       0,     0,     0,     0,   390,     0,     0,     0,     0,     0,
       0,     0,   991,  1326,   992,   993,   994,   995,   996,   997,
     998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,   988,   989,   990,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1013,     0,
       0,     0,     0,     0,   991,     0,   992,   993,   994,   995,
     996,   997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,
    1006,  1007,  1008,  1009,  1010,  1011,  1012,   990,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1013,     0,     0,     0,   991,     0,   992,   993,   994,   995,
     996,   997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,
    1006,  1007,  1008,  1009,  1010,  1011,  1012,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1013,   992,   993,   994,   995,   996,   997,   998,   999,  1000,
    1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1013,   993,   994,   995,   996,
     997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,
    1007,  1008,  1009,  1010,  1011,  1012,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1013,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,   386,   387,   388,     0,
     389,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   390,   994,   995,   996,   997,   998,   999,  1000,
    1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1013,   995,   996,   997,   998,
     999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,
    1009,  1010,  1011,  1012,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1013,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,     0,   389,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   390,   997,
     998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1013,  -943,
    -943,  -943,  -943,  1001,  1002,  1003,  1004,  1005,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1013
};

static const yytype_int16 yycheck[] =
{
       5,     6,   614,     8,     9,    10,    11,    12,   163,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,   136,   186,    28,    29,     4,    53,   585,   403,     4,
     319,     4,   701,     4,     4,   430,  1073,    42,   424,   425,
      89,    32,    54,   864,    49,   175,    51,    96,    97,    54,
     586,    56,   228,    44,   166,   389,     4,    48,   163,   456,
     234,   732,   845,  1056,   560,   456,   114,   883,   454,     9,
     114,     9,   187,   739,   945,   114,  1058,    30,     9,    30,
      45,     9,    45,   899,     9,     9,   135,     9,     9,    14,
      45,     9,     9,     4,     9,    33,    35,     9,    66,     9,
       9,     9,     9,    30,     9,     9,     9,    79,    85,   114,
       9,     9,     9,    85,   224,     9,     9,     9,     9,     9,
       9,    50,    66,   235,   109,    66,     0,    14,   201,    79,
     304,    50,   201,    79,    45,   393,    79,    79,   100,   101,
      79,   170,  1069,    30,   105,    97,   128,   129,   201,     4,
      45,   117,   113,   114,   115,   116,   117,   118,   124,    79,
      47,    45,     8,   421,    66,    66,   214,   201,   426,   146,
     214,    57,   201,    53,    97,   147,   148,  1619,   201,   227,
     201,    66,   167,    66,   163,    65,   191,    66,   136,   319,
      66,   204,   356,    79,    66,   106,    82,    66,   981,    66,
     111,   153,   113,   114,   115,   116,   117,   118,   119,    66,
      66,    79,    35,   163,    66,    66,    66,   163,   460,    66,
     163,   163,   183,   205,   163,   154,   204,   128,   129,   149,
     153,   236,  1674,   201,   239,   154,   199,   202,   210,   187,
     171,   246,   247,   205,   199,   156,   157,   202,   159,   297,
     169,   204,   203,   297,   255,   256,    79,   201,   297,   171,
     204,   262,   171,   204,   202,   205,   396,   509,   432,   240,
    1141,  1264,   183,   244,  1256,   203,   204,   149,   203,   203,
     202,  1263,   203,  1265,  1100,   203,   203,   402,   203,   955,
     420,   957,   297,   203,   205,   203,   203,   202,   303,   203,
     203,   350,   307,   204,   203,   203,   203,   202,   202,   202,
     202,   202,   202,   202,   444,   320,   321,   322,   202,   204,
     347,   204,   201,   453,   201,   204,   456,  1254,   204,   439,
     335,    79,   204,    97,    79,   204,  1119,   204,   206,   835,
     163,   346,   201,   344,   880,   587,   154,   204,   204,    66,
     399,   400,   204,   204,   204,   334,  1349,   204,   201,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,    85,   390,   334,   392,   393,   153,
     395,   403,   440,   279,   280,   281,   440,  1379,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,    35,    35,   201,   128,   129,   421,   422,   592,   424,
     425,   426,   427,    97,    79,   311,     4,   822,   433,  1422,
      79,   436,   100,   101,   128,   129,    85,    30,    79,    35,
      66,   446,  1369,   448,  1371,   146,   116,   602,   201,   454,
     462,  1120,   201,   401,   124,    79,    79,   462,   206,   464,
     570,   206,   389,     4,   201,    79,  1137,   577,    97,   396,
     580,   868,    50,   328,   201,    53,   155,   868,   479,   153,
     201,   657,   724,    79,   201,   202,   491,   729,   202,   494,
     495,   496,    70,   420,   201,    79,   467,   602,   147,   148,
     664,    85,   128,   129,    45,   146,   147,   148,   202,    87,
     896,    89,   201,   689,  1072,    35,    79,   444,    96,    97,
     525,   153,    85,    97,   153,   204,   453,  1310,    29,   456,
     149,   707,   706,   147,   148,   170,   933,   205,   712,   163,
     163,  1468,   933,   198,   654,    46,   128,   129,    49,   204,
    1086,   170,   203,  1089,    35,   170,   845,   135,   203,    79,
     209,    79,   146,   147,   148,   106,   201,    85,   902,   203,
     111,   620,   113,   114,   115,   116,   117,   118,   119,   153,
      66,   582,   583,   146,   147,   148,   201,   763,    66,   153,
     591,   113,   114,   115,   116,   117,   118,   773,   203,    79,
     712,  1270,   170,  1272,  1387,    85,   203,   612,   113,   114,
     115,   116,   117,   118,    79,   156,   157,   149,   159,   624,
      85,   199,   149,   602,   155,    46,    47,   513,   203,   147,
     148,   517,    66,   201,   203,   745,   522,   168,   170,   203,
     204,   149,   183,   170,   113,   114,   115,   116,   117,   118,
     204,   656,     4,   305,   201,   124,   125,   309,    71,    72,
     665,   183,   240,   201,   205,    66,   244,   147,   148,   201,
     248,   201,   204,   204,    26,    27,  1051,   204,   183,   684,
      29,   203,   147,   148,   336,  1243,   338,   339,   340,   341,
     149,   269,   981,   162,   203,  1622,   153,    46,    71,    72,
      49,   113,   114,   115,   116,   117,   118,   126,   127,  1095,
     952,   716,   124,   125,   183,   845,  1385,    44,  1104,  1756,
     203,   204,  1258,    65,    49,    50,    51,   732,    53,   203,
     204,   586,  1649,  1650,  1771,  1645,  1646,   728,   868,   170,
      65,   747,   748,   722,   149,  1140,   747,   748,   160,   201,
     162,    49,    50,    51,   208,     9,   334,   149,   149,    10,
      11,    12,   114,   201,     8,   343,   737,    65,   203,   347,
     775,   183,   350,   170,   722,    99,   100,   101,    29,   149,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,   201,    53,   933,   149,    14,  1475,    99,   100,   101,
     789,   113,   114,   115,    65,   149,    79,   927,   396,   397,
     398,   399,   400,  1750,   203,   203,   831,   124,   124,  1224,
    1119,   118,   119,   120,    14,   836,   202,   170,  1765,   844,
      14,   789,   420,   202,  1380,   850,   202,   852,    97,   854,
     207,   981,   901,   105,   202,   834,     9,   201,   859,   834,
     970,   834,   214,   834,   834,   201,   444,   977,    89,   221,
     871,   201,   877,   202,  1260,   227,   202,     9,   456,    14,
     203,   201,   887,   862,     9,   890,   834,   892,   240,   467,
      79,   896,   244,   187,    46,    47,    48,    49,    50,    51,
      79,    53,    79,  1079,  1725,   190,   484,     9,   794,     9,
     201,    79,   798,    65,   862,    26,    27,   203,    77,   203,
      79,   202,  1743,   202,   184,   185,   186,   932,   202,   126,
    1751,   191,   192,   203,   286,   195,   196,   201,    66,    30,
     202,   868,   127,   295,   296,   297,   524,   938,   130,   169,
     302,     9,   202,   149,   202,  1624,   308,   208,   959,     9,
     961,   202,   941,     9,   943,   124,   202,    14,   939,   199,
       9,   549,   550,  1215,    66,   902,     9,   171,   202,   834,
       9,  1367,   334,    14,   126,   208,   208,   205,     9,  1119,
     201,   208,   202,   941,   202,   943,   208,   156,   157,   201,
     159,   160,   161,   202,  1114,   203,   933,    97,   203,   864,
     130,   149,  1017,  1018,  1019,     9,   202,   201,  1023,  1024,
     149,  1310,   149,   201,   187,   880,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,  1051,
     201,  1283,   620,   201,   201,   204,  1051,   206,    46,    47,
      48,    49,    50,    51,   204,  1165,  1668,  1299,    14,  1169,
       9,   187,  1172,    79,   204,   204,    14,    65,    14,  1179,
    1071,   208,  1073,   203,    14,    30,  1055,  1082,   430,   203,
    1055,   204,  1055,   199,  1055,  1055,   201,   201,   440,    30,
    1095,   201,    14,   201,    48,   201,     9,  1098,  1387,  1104,
    1105,   202,   130,   214,   203,   203,   201,  1055,    14,     9,
     221,   208,    65,   202,     9,   467,   227,    79,    97,   146,
       9,   201,   130,   701,   203,   703,    14,    79,   202,  1741,
     201,   204,  1137,   201,   204,   202,   208,   130,  1129,   204,
       9,    86,  1147,   146,   722,    30,    73,  1389,   203,   202,
     130,   203,   171,    30,  1155,   130,  1398,   202,   736,   737,
     202,  1132,     9,     9,   202,  1144,   202,   202,     9,  1279,
    1412,   205,   202,     9,    14,   286,   202,    79,   201,   204,
    1310,   205,   202,   201,   295,   296,   297,    74,    75,    76,
     202,   302,   202,   204,   130,  1350,  1144,   308,   202,    86,
    1055,   202,   202,     9,    30,   204,   203,   202,   106,    14,
     202,   789,   203,   203,    97,   158,   203,   795,    79,  1220,
     154,   799,   800,   204,   202,   111,   202,   130,   202,  1234,
    1345,  1086,    14,  1238,  1089,  1240,  1241,   130,   170,  1481,
     204,   819,   203,  1222,  1249,    79,    14,   134,   135,   136,
     137,   138,  1231,   149,  1259,  1260,   834,  1387,   145,   611,
      14,    79,   204,   202,   151,   152,   201,   130,    14,   202,
     848,    14,   203,   203,   203,    14,   204,  1441,   165,    55,
      79,   201,    79,     9,   862,   203,    79,   109,    97,   149,
     868,   161,    33,    97,    14,   182,   201,   171,   202,   167,
      79,  1302,   203,  1304,   201,   164,     9,  1334,   202,    79,
      14,   203,   202,   204,   666,    79,    14,    79,   202,   430,
      14,    79,  1327,   901,    14,   798,    79,   794,  1733,   440,
     522,   400,   897,   397,   900,   913,   914,   915,   399,   837,
    1747,  1138,  1480,  1743,  1345,  1471,   527,  1296,  1504,  1588,
    1329,  1340,  1775,  1763,  1600,   933,   708,    42,   710,  1364,
    1336,   939,  1367,   941,  1467,   943,   403,  1222,  1417,  1026,
     722,  1350,  1614,  1023,  1616,   786,  1231,   498,  1059,   498,
    1102,  1329,   734,  1625,   962,   737,   978,  1115,   914,   929,
    1116,  1370,   868,   303,  1421,   984,  1344,  1376,   322,  1378,
     747,   963,   980,  1258,  1042,  1014,    -1,    -1,   986,    -1,
      -1,  1390,    -1,  1414,   296,    -1,    -1,  1444,    -1,  1446,
    1399,    -1,    -1,    -1,  1395,    -1,    -1,    -1,    -1,  1671,
      -1,    -1,    -1,    -1,    -1,    -1,  1014,   789,    -1,    -1,
    1604,    -1,  1390,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1399,    -1,  1031,  1459,   807,  1034,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     822,   823,    -1,    -1,    -1,    -1,    -1,  1055,    -1,    -1,
      -1,    -1,   834,    -1,    -1,  1490,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1473,    -1,    -1,    -1,    -1,  1478,
     611,  1502,  1450,    -1,    -1,  1484,    63,    64,  1479,  1480,
     862,    -1,  1491,    -1,    -1,  1370,    -1,    -1,    -1,     4,
      -1,  1376,    -1,  1378,    -1,  1380,    -1,  1769,    -1,    -1,
    1478,    -1,    -1,    -1,  1776,    -1,  1484,    -1,  1116,    -1,
      -1,    -1,  1120,  1491,    -1,    -1,    -1,    -1,    -1,    -1,
    1599,    -1,    -1,    -1,  1132,   666,    -1,    -1,    -1,    -1,
      45,    -1,    -1,    -1,    -1,  1720,  1144,    -1,    -1,    -1,
      -1,   128,   129,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,   939,    -1,   941,
      -1,   943,    -1,   945,   946,    -1,    -1,   708,    65,   710,
      -1,    -1,  1603,    -1,  1609,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1662,    -1,    -1,    -1,    -1,  1473,    -1,
      -1,   106,  1737,   734,    -1,    -1,   111,    -1,   113,   114,
     115,   116,   117,   118,   119,    -1,    -1,    -1,    -1,    -1,
      -1,  1219,    -1,    -1,    -1,   202,    -1,    -1,    -1,    -1,
    1629,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,    27,    -1,    -1,    30,    -1,    -1,    -1,
      -1,   156,   157,    -1,   159,    -1,    -1,    -1,    -1,    -1,
      -1,  1629,    -1,    -1,    -1,  1686,    -1,    -1,    -1,    53,
    1669,  1670,  1270,    -1,  1272,    -1,   807,  1676,   183,    -1,
      -1,    -1,    -1,  1055,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   822,   823,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     205,  1669,  1670,    -1,    -1,    -1,    -1,    -1,  1676,  1081,
      -1,    -1,    -1,  1712,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1720,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1329,    -1,    -1,    -1,  1756,  1334,    -1,    -1,    -1,
      -1,    -1,  1340,    -1,  1712,    -1,    -1,    -1,    -1,    -1,
    1771,    -1,  1124,    -1,    -1,    -1,  1781,    -1,    -1,    -1,
    1132,    -1,    -1,    -1,  1789,    -1,    -1,    -1,  1140,  1141,
    1795,    -1,  1144,  1798,    -1,    -1,    -1,    -1,  1777,    -1,
      -1,    -1,    -1,    -1,    -1,  1784,    -1,  1385,    -1,    -1,
      -1,    -1,  1390,    -1,     4,    -1,    -1,  1395,    -1,    -1,
      -1,  1399,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1777,
      -1,    -1,    -1,    -1,   945,   946,  1784,    -1,    -1,  1417,
      -1,    -1,    -1,  1421,    -1,    -1,    -1,    -1,    -1,     4,
     214,    -1,  1430,    -1,    -1,    45,    -1,   221,  1436,    -1,
      -1,    -1,    -1,   227,    -1,    -1,  1444,    -1,  1446,    -1,
    1725,    -1,  1224,    -1,  1452,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1743,    -1,
      45,   255,   256,    -1,    -1,    -1,  1751,  1475,   262,    -1,
    1478,  1479,  1480,    -1,    -1,    -1,  1484,    -1,    -1,    -1,
      -1,    -1,    -1,  1491,    -1,    -1,   106,    -1,    -1,    -1,
      -1,   111,   286,   113,   114,   115,   116,   117,   118,   119,
      -1,   295,   296,    -1,    -1,    -1,    -1,    -1,   302,    -1,
      -1,    -1,    -1,    -1,   308,    -1,    -1,    -1,    -1,    -1,
      -1,   106,    -1,    -1,    -1,   319,   111,    -1,   113,   114,
     115,   116,   117,   118,   119,    -1,   156,   157,     4,   159,
    1081,    -1,    -1,    -1,    -1,    -1,    -1,  1329,    -1,    -1,
     344,    -1,    -1,   347,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   183,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   156,   157,    -1,   159,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    -1,  1124,    -1,   205,    -1,    -1,    -1,    -1,
      -1,  1599,    -1,    -1,    -1,   389,    -1,    -1,   183,  1140,
    1141,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1390,    -1,
      -1,    -1,  1620,  1395,    -1,    -1,  1624,  1399,    -1,    -1,
     205,  1629,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1639,    -1,    -1,    -1,    -1,   430,  1645,  1646,    -1,
     106,  1649,  1650,    -1,    -1,   111,   440,   113,   114,   115,
     116,   117,   118,   119,  1662,    -1,    -1,    -1,    -1,    -1,
      -1,  1669,  1670,    -1,    -1,    -1,    -1,    -1,  1676,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,  1224,    -1,   479,   480,    -1,    -1,   483,
     156,   157,    -1,   159,    -1,    -1,  1478,  1479,  1480,    -1,
      -1,    -1,  1484,    -1,  1712,    -1,    -1,    -1,    -1,  1491,
      -1,  1719,    -1,    -1,    -1,    -1,    -1,   183,    63,    64,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,  1736,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   531,    -1,   205,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    77,    -1,    79,  1777,
      -1,    -1,    -1,    -1,    -1,    -1,  1784,    65,    -1,    -1,
      -1,    -1,    -1,   128,   129,    -1,    -1,    -1,   582,   583,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   591,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   611,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,   154,    -1,   156,   157,  1629,   159,   160,
     161,    -1,    -1,    -1,    65,    -1,    -1,   202,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   666,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,  1669,  1670,    -1,
      -1,    -1,    -1,   204,  1676,   206,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,    -1,    -1,   708,    -1,   710,   205,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
    1712,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     734,   735,    -1,    65,    -1,    63,    64,    -1,    10,    11,
      12,    -1,    -1,   747,   748,   749,   750,   751,   752,   753,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   762,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,   205,  1777,   790,    -1,    -1,    -1,
      -1,    -1,  1784,    65,    -1,    -1,    -1,    -1,    -1,    -1,
     128,   129,    -1,   807,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   820,    -1,   822,   823,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      55,    -1,   836,   837,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   845,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    -1,   859,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   871,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   879,    -1,    -1,   882,    -1,
     105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,   114,
     115,   116,   117,   118,    -1,    -1,    -1,    -1,   902,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,
      -1,    26,    27,    -1,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   150,    -1,    -1,   153,   154,
      -1,   156,   157,   205,   159,   160,   161,    -1,    -1,    -1,
      -1,   945,   946,    -1,    -1,    -1,    -1,    -1,    -1,   174,
      -1,    -1,    -1,    -1,    -1,   959,    -1,   961,   183,   963,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,   978,    -1,   201,   981,   982,   983,
     984,    -1,    -1,   987,   988,   989,   990,   991,   992,   993,
     994,   995,   996,   997,   998,   999,  1000,  1001,  1002,  1003,
    1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,
      -1,    -1,    -1,    77,    -1,    26,    27,    -1,    -1,    30,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1039,    -1,    -1,    -1,    -1,
      -1,    -1,    53,    -1,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,  1071,    53,  1073,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1081,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,   150,    -1,    -1,   153,
      -1,    -1,   156,   157,  1098,   159,   160,   161,    -1,   214,
      -1,    -1,    -1,    -1,    -1,    -1,   221,    -1,    -1,    -1,
      -1,    -1,   227,    -1,    -1,  1119,    -1,    -1,    -1,    -1,
    1124,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,  1140,  1141,    -1,  1143,
      -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1155,    -1,    -1,  1158,    -1,  1160,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,   286,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1183,
     295,   296,    -1,    -1,    -1,    -1,    -1,   302,    -1,    -1,
      -1,    -1,    -1,   308,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,   319,    -1,    63,    64,    -1,    -1,
     221,    -1,    -1,    -1,    -1,    -1,  1220,  1221,   203,    29,
    1224,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,   255,   256,    -1,    -1,    -1,    -1,
      -1,   262,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   128,   129,    -1,   389,   286,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   295,   296,    -1,    -1,    -1,    -1,
      -1,   302,    -1,    -1,    -1,    -1,    -1,   308,  1302,    -1,
    1304,    -1,    -1,    -1,    -1,  1309,  1310,    -1,   319,    -1,
    1314,    -1,  1316,    -1,  1318,   430,    -1,  1321,    -1,  1323,
      -1,    -1,  1326,    -1,    -1,   440,    10,    11,    12,    -1,
    1334,  1335,    -1,   344,  1338,    -1,   347,    -1,    -1,    -1,
      -1,  1345,    -1,    -1,    77,    29,    79,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   389,    -1,
      -1,    65,    -1,  1387,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   205,    -1,    -1,  1402,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1414,  1415,  1416,    -1,    -1,    -1,    -1,  1421,    -1,   430,
      -1,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1444,    -1,  1446,    -1,    -1,    -1,    -1,    -1,  1452,    -1,
      -1,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,    -1,   479,    -1,
      -1,   204,   483,   206,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1487,  1488,  1489,    -1,    -1,    -1,    -1,
      -1,    -1,  1496,  1497,    -1,    77,   611,    79,  1502,    -1,
    1504,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     531,   205,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,   666,    -1,    -1,    77,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   582,   583,    -1,   156,   157,    -1,   159,   160,   161,
     591,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   708,    -1,   710,    -1,    -1,    -1,  1603,
     611,    -1,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,  1620,    -1,    -1,   734,
      -1,    -1,   204,    -1,   206,    -1,    -1,    -1,    -1,    -1,
     153,    -1,    -1,   156,   157,  1639,   159,   160,   161,    -1,
    1644,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1655,    -1,    -1,    -1,   666,  1660,    -1,    -1,    -1,
    1664,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,    -1,    -1,    -1,
      -1,   204,  1686,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   807,    -1,    -1,    -1,    -1,   708,   205,   710,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   822,   823,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1724,    -1,    -1,   734,   735,    -1,    -1,    -1,  1732,    -1,
     845,    -1,    -1,    -1,    -1,    -1,   747,   748,   749,   750,
     751,   752,   753,  1747,    -1,    -1,    -1,    -1,    -1,    53,
      -1,   762,  1756,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1771,    -1,    -1,
      77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   790,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   902,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   807,    -1,   105,   106,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   820,
      -1,   822,   823,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   836,   837,    -1,    -1,    -1,
     945,   946,    -1,    -1,   845,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   153,    -1,   859,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,
     871,    -1,    -1,    -1,    -1,    -1,   981,    -1,   879,    -1,
      -1,   882,    -1,    10,    11,    12,    -1,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   902,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,   945,   946,    -1,    -1,    -1,    77,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   959,    -1,
     961,   255,   256,    -1,    -1,    -1,    -1,    -1,   262,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1081,   978,    -1,    -1,
     981,   982,   983,   984,    -1,    -1,   987,   988,   989,   990,
     991,   992,   993,   994,   995,   996,   997,   998,   999,  1000,
    1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,  1010,
    1011,  1012,  1013,    -1,  1119,    -1,    -1,    -1,    -1,  1124,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,   156,   157,
     158,   159,   160,   161,    -1,  1140,  1141,    -1,  1039,    -1,
      -1,    -1,    -1,    -1,    67,    68,    -1,    -1,    -1,    -1,
     344,    -1,    -1,   347,    77,    -1,    79,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
    1071,    -1,  1073,   201,    -1,    -1,    -1,    -1,   205,    -1,
    1081,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     113,   114,   115,   116,   117,   118,    -1,  1098,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,  1119,  1224,
      -1,    -1,    -1,  1124,    -1,    -1,    -1,   150,    -1,    65,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,  1140,
    1141,    -1,  1143,    -1,    -1,   168,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1155,    -1,    -1,  1158,    -1,  1160,
     183,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,    -1,    -1,   201,    -1,
      -1,    -1,  1183,   206,    -1,   479,   480,    -1,    -1,   483,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1310,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1220,
    1221,    -1,    -1,  1224,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   531,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,  1387,    -1,    -1,    -1,    -1,    -1,   582,   583,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   591,    -1,    -1,
      -1,  1302,    -1,  1304,    -1,    -1,    -1,    -1,  1309,  1310,
      -1,    -1,    -1,  1314,    77,  1316,    -1,  1318,    -1,    -1,
    1321,    -1,  1323,    -1,    -1,  1326,    10,    11,    12,    -1,
      -1,    -1,    -1,  1334,  1335,    -1,    -1,  1338,    -1,    -1,
      -1,    -1,    -1,    -1,  1345,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,  1387,    -1,    -1,    -1,
      -1,    -1,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,  1402,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1414,  1415,  1416,    -1,    -1,    -1,   205,
    1421,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,    -1,   201,    -1,
      -1,   735,    -1,  1444,    -1,  1446,    -1,    -1,    -1,    -1,
      -1,  1452,    -1,   747,   748,   749,   750,   751,   752,   753,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    -1,   762,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,  1487,  1488,  1489,    -1,
      -1,    55,    -1,    -1,    -1,  1496,  1497,    -1,    -1,    65,
      -1,  1502,    -1,  1504,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   205,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,   836,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   483,   118,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   859,    -1,    -1,   132,   133,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   871,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   879,   150,    -1,    -1,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,
      -1,   531,  1603,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1620,
      -1,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,    -1,   201,    -1,    -1,
      -1,   205,    -1,  1644,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1655,    -1,    -1,    -1,    -1,  1660,
      -1,    -1,    -1,  1664,    -1,   959,    -1,   961,    -1,   963,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   978,  1686,    -1,    -1,   982,   983,
     984,    -1,    -1,   987,   988,   989,   990,   991,   992,   993,
     994,   995,   996,   997,   998,   999,  1000,  1001,  1002,  1003,
    1004,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,
      -1,    -1,    -1,  1724,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1732,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1039,  1747,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1756,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,   483,    -1,    -1,
    1771,    -1,    -1,    -1,    -1,    -1,    -1,  1071,    29,  1073,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,  1098,   735,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,   531,    -1,    -1,    -1,   749,
     750,   751,   752,   753,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   762,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,  1143,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,
      -1,  1155,    -1,    -1,  1158,    29,  1160,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,  1183,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   122,
      -1,    65,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,  1220,    53,    -1,    -1,
      -1,    -1,    -1,   156,   157,    -1,   159,   160,   161,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   879,
      -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,    10,    11,
      12,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   483,    -1,    29,   201,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,  1302,    -1,
    1304,    -1,    -1,    65,    -1,  1309,    -1,    -1,    -1,   735,
    1314,    77,  1316,   531,  1318,    -1,    -1,  1321,    -1,  1323,
      86,    -1,  1326,   749,   750,   751,   752,   753,   202,    -1,
    1334,    -1,    -1,    -1,    -1,    -1,   762,    -1,   978,    -1,
      -1,  1345,   982,   983,   984,    -1,    -1,   987,   988,   989,
     990,   991,   992,   993,   994,   995,   996,   997,   998,   999,
    1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,  1008,  1009,
    1010,  1011,  1012,  1013,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   150,    -1,    -1,   153,    -1,    -1,
     156,   157,    -1,   159,   160,   161,    -1,    -1,  1402,  1039,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1414,    -1,    -1,    -1,    -1,    -1,    -1,  1421,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
    1444,   203,  1446,    -1,    -1,    -1,    -1,    -1,  1452,    -1,
      -1,    -1,    29,   879,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,  1487,  1488,  1489,    -1,    -1,    65,    -1,
      -1,    -1,  1496,    -1,    -1,    -1,    -1,    -1,  1502,    -1,
      -1,    -1,    -1,  1143,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   735,  1158,    -1,
    1160,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   749,   750,   751,   752,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1183,   762,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   978,    -1,    -1,    -1,   982,   983,   984,    -1,
      -1,   987,   988,   989,   990,   991,   992,   993,   994,   995,
     996,   997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,
    1006,  1007,  1008,  1009,  1010,  1011,  1012,  1013,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1603,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1039,    -1,    -1,  1620,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   203,    -1,    -1,    -1,
      -1,    77,    -1,    -1,    -1,  1639,    -1,    -1,    -1,    -1,
    1644,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1655,    -1,    -1,    -1,    -1,  1660,    -1,    -1,    -1,
    1664,   879,    -1,    -1,    -1,    -1,    10,    11,    12,  1309,
      -1,    -1,    -1,    -1,  1314,    -1,  1316,    -1,  1318,    -1,
      -1,  1321,  1686,  1323,    -1,    29,  1326,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
     156,   157,    -1,   159,   160,   161,    -1,  1143,    -1,    -1,
    1724,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1158,    -1,  1160,    -1,    -1,    -1,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,  1756,    -1,    -1,   201,    -1,  1183,    -1,    -1,
      -1,    -1,  1402,    -1,   982,   983,   984,  1771,    -1,   987,
     988,   989,   990,   991,   992,   993,   994,   995,   996,   997,
     998,   999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,  1007,
    1008,  1009,  1010,  1011,  1012,  1013,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1039,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,  1487,  1488,  1489,
      -1,    -1,    -1,    -1,    -1,    -1,  1496,    -1,    65,   203,
      -1,    -1,    -1,  1503,    -1,    -1,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,  1309,    35,    -1,    -1,    -1,  1314,    -1,
    1316,    -1,  1318,    -1,    -1,  1321,    -1,  1323,    -1,    29,
    1326,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,  1143,    77,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
    1158,    -1,  1160,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1183,    -1,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1402,    -1,    -1,    -1,
     131,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,   203,    -1,    -1,   150,
      -1,    -1,   153,   154,  1644,   156,   157,    -1,   159,   160,
     161,    -1,    -1,    -1,    -1,  1655,    -1,    -1,    -1,    -1,
    1660,    -1,    -1,    -1,  1664,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,  1688,    -1,
     201,    -1,    -1,    -1,    -1,   206,    -1,    -1,    -1,    -1,
      -1,  1487,  1488,  1489,    -1,    -1,    -1,    -1,    -1,    -1,
    1496,    -1,    -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,
      77,    -1,    79,    80,  1724,    -1,    -1,    -1,    -1,    -1,
      -1,  1309,    -1,    -1,    -1,    -1,  1314,    -1,  1316,    -1,
    1318,    -1,    -1,  1321,    -1,  1323,    -1,    -1,  1326,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1402,    -1,    -1,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,  1644,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,  1655,
      -1,    -1,    -1,    -1,  1660,    -1,    -1,    -1,  1664,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,  1487,
    1488,  1489,    74,    75,    76,    77,    78,    79,  1496,    81,
      82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    95,    -1,    -1,    98,    -1,  1724,    -1,
     102,   103,   104,   105,   106,   107,   108,   202,   110,   111,
     112,   113,   114,   115,   116,   117,   118,    -1,   120,   121,
     122,   123,   124,   125,    -1,    -1,    -1,    -1,    -1,   131,
     132,    -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,
      -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
     162,    -1,    -1,   165,    -1,    -1,   168,    -1,    -1,    -1,
      -1,    -1,   174,   175,   176,   177,   178,    -1,   180,    -1,
     182,   183,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,    -1,    -1,   201,
      -1,   203,   204,   205,   206,   207,    -1,   209,   210,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,  1644,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,  1655,    -1,    -1,
      -1,    -1,  1660,    -1,    -1,    -1,  1664,    -1,    -1,    -1,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    82,    -1,
      -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,    93,
      -1,    95,    -1,    -1,    98,    -1,  1724,    -1,   102,   103,
     104,   105,   106,   107,   108,    -1,   110,   111,   112,   113,
     114,   115,   116,   117,   118,    -1,   120,   121,   122,   123,
     124,   125,    -1,    -1,    -1,    -1,    -1,   131,   132,    -1,
     134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,
      -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,   162,    -1,
      -1,   165,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,
     174,   175,   176,   177,   178,    -1,   180,    -1,   182,   183,
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
     106,   107,   108,    -1,   110,   111,   112,   113,   114,   115,
     116,   117,   118,    -1,   120,   121,   122,   123,   124,   125,
      -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,
     136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,
      -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,   162,    -1,    -1,   165,
      -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,
     176,   177,   178,    -1,   180,    -1,   182,   183,    -1,   185,
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
     204,   205,   206,   207,    -1,   209,   210,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,
      86,    87,    88,    89,    90,    91,    -1,    93,    -1,    95,
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
      88,    89,    -1,    91,    -1,    93,    -1,    95,    96,    -1,
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
     198,    -1,    -1,   201,    -1,   203,   204,    -1,   206,   207,
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
      94,    95,    -1,    -1,    98,    -1,    -1,    -1,   102,   103,
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
      -1,    91,    92,    93,    -1,    95,    -1,    -1,    98,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   174,   175,   176,   177,    -1,    -1,    -1,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
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
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,    -1,    -1,   201,    -1,   203,
      -1,    -1,   206,   207,    -1,   209,   210,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    29,    13,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     168,    -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,
      -1,    -1,    -1,    -1,   182,   183,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,    -1,    -1,   201,    11,    12,   204,    -1,   206,   207,
      -1,   209,   210,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    29,    13,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    35,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    65,    -1,
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
     160,   161,    -1,   163,    -1,   165,    -1,    -1,   168,    -1,
      -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,    -1,
      -1,    -1,   182,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,    -1,
      -1,   201,    -1,    -1,    -1,    -1,   206,   207,    -1,   209,
     210,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   165,    -1,    -1,   168,     3,     4,     5,
       6,     7,   174,   175,   176,   177,    -1,    13,    -1,    -1,
     182,   183,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,    -1,    -1,   201,
      -1,    -1,    -1,    -1,   206,   207,    -1,   209,   210,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   102,    -1,    -1,   105,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,   114,   115,
     116,   117,   118,    -1,    77,   121,   122,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,
     136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,
      -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,
      -1,   124,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,
     176,   177,    -1,    -1,    -1,    -1,   182,   183,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   156,   157,   201,   159,   160,   161,    -1,
     206,   207,    -1,   209,   210,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,    35,    -1,    -1,
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
      -1,    -1,   168,     3,     4,     5,     6,     7,   174,   175,
     176,   177,    -1,    13,    -1,    -1,   182,   183,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,    -1,    -1,   201,    -1,   203,    -1,    -1,
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
     160,   161,    -1,    -1,    -1,   165,    -1,    -1,   168,    -1,
      -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,    -1,
      -1,    -1,   182,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,    -1,
      -1,   201,   202,    -1,    -1,    -1,   206,   207,    -1,   209,
     210,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
      -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,
     176,   177,    -1,    -1,    -1,    -1,   182,   183,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,    -1,    -1,   201,    11,    12,    -1,    -1,
     206,   207,    -1,   209,   210,     3,     4,     5,     6,     7,
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
     168,    -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,
      -1,    -1,    -1,    -1,   182,   183,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,    -1,    -1,   201,    -1,    12,    -1,    -1,   206,   207,
      -1,   209,   210,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    29,    13,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    35,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    65,    -1,
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
     168,    -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,
      -1,    -1,    -1,    -1,   182,   183,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,    -1,    -1,   201,    -1,    -1,    -1,    -1,   206,   207,
      -1,   209,   210,     3,     4,     5,     6,     7,    -1,    -1,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    67,    68,    69,
      70,    71,    72,    73,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,    -1,    -1,
      -1,   131,   132,    -1,   134,   135,   136,   137,   138,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     150,   151,   152,    -1,    -1,    -1,   156,   157,    -1,   159,
     160,   161,   162,    -1,   164,   165,    -1,   167,    -1,    -1,
      -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,   178,    -1,
     180,    -1,   182,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    10,    11,
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
      50,    51,    -1,    53,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
     202,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   197,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    29,    53,    -1,    -1,    -1,    -1,    -1,    -1,   193,
     194,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,
     190,    29,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
     118,    -1,    -1,    -1,   189,    -1,    -1,    -1,    -1,    77,
      -1,    -1,    -1,    -1,   132,   133,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,   150,    53,    -1,   153,   154,   105,   156,   157,
      -1,   159,   160,   161,    -1,    65,    -1,   188,    -1,    -1,
     118,    -1,    -1,    -1,    29,    -1,   174,    -1,    -1,    -1,
      -1,    -1,    -1,   130,   132,   133,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      55,    -1,   150,   201,    -1,   153,   154,   205,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    29,    -1,   174,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      55,    -1,    -1,   201,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   118,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    29,    -1,    -1,   132,   133,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   150,    -1,    -1,   153,   154,
      55,   156,   157,    -1,   159,   160,   161,    -1,   163,    -1,
      -1,    -1,    -1,   118,    -1,    -1,    -1,    -1,    -1,   174,
      -1,    -1,    77,    -1,    29,    -1,    -1,   132,   133,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,   150,   201,    -1,   153,   154,
      55,   156,   157,    -1,   159,   160,   161,    -1,   163,    -1,
      -1,    -1,    -1,   118,    -1,    -1,    -1,    -1,    -1,   174,
      -1,    -1,    77,    -1,    -1,    -1,    -1,   132,   133,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,   150,   201,    -1,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,
      -1,    -1,    -1,   118,    -1,    -1,    -1,    -1,    -1,   174,
      -1,    -1,    -1,   178,    -1,    -1,    -1,   132,   133,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,   150,   201,    -1,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    46,    47,    -1,   201,    -1,    -1,    52,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    67,    68,   132,
      -1,   134,   135,   136,   137,   138,    -1,    77,    -1,    79,
      65,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,   152,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      77,    -1,   165,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   174,   175,   176,   177,    -1,    -1,    -1,   118,   182,
      -1,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    46,    47,    -1,   201,    -1,
      -1,    52,    -1,    54,    -1,   130,    -1,    -1,    -1,    -1,
     150,    -1,    -1,   153,   154,    66,   156,   157,    -1,   159,
     160,   161,    -1,    74,    75,    76,    77,    -1,   168,    -1,
      -1,    -1,    -1,   150,    -1,    86,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
      -1,   201,    -1,    -1,    -1,    -1,   206,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   132,    -1,   134,   135,   136,   137,   138,    -1,    -1,
      -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,
     151,   152,   153,   154,    -1,   156,   157,    -1,   159,   160,
     161,    -1,    -1,    -1,   165,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   174,   175,   176,   177,    46,    47,    -1,
      -1,   182,    -1,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    66,    -1,    -1,
     201,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    10,
      11,    12,    -1,    -1,    -1,    68,    -1,    86,    -1,    -1,
      -1,    -1,    -1,    -1,    77,    -1,    79,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   132,    65,   118,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   156,   157,    77,
     159,   160,   161,    -1,    -1,    -1,    -1,   150,    -1,    -1,
     153,   154,    -1,   156,   157,   174,   159,   160,   161,    77,
      -1,    79,    -1,    -1,    -1,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   130,
      -1,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,    -1,   201,    -1,
     118,    -1,    -1,   206,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,   156,   157,
      -1,   159,   160,   161,    77,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   150,    -1,    -1,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    77,    -1,    79,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,    -1,   201,    -1,   118,   204,    -1,   206,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,    -1,
     153,    -1,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   150,    -1,    -1,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    77,
      -1,    79,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,    -1,   201,    -1,
     118,    -1,    -1,   206,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   131,    77,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   150,    -1,    -1,   153,   154,    -1,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   118,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      77,    -1,    79,   201,    -1,    -1,    -1,   150,   206,    -1,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,    -1,    77,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   118,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    -1,    -1,    -1,   201,    -1,
      -1,    -1,    -1,   206,   118,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   150,    -1,    -1,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   150,    -1,    -1,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,   206,
      -1,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,   206,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    65,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   130,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    77,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    29,   130,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,   119,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,
      -1,    -1,   153,    -1,   130,   156,   157,    -1,   159,   160,
     161,    -1,    -1,    -1,    -1,   150,    -1,    -1,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,   130,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   153,    -1,    -1,   156,   157,    -1,   159,
     160,   161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    28,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    97,    53,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65
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
     249,   250,   254,   257,   262,   268,   326,   327,   335,   339,
     340,   341,   342,   343,   344,   345,   346,   348,   351,   363,
     364,   365,   366,   367,   371,   372,   374,   393,   403,   404,
     405,   410,   413,   432,   440,   442,   443,   444,   445,   446,
     447,   448,   449,   450,   451,   453,   474,   476,   478,   116,
     117,   118,   131,   150,   160,   218,   249,   326,   345,   442,
     345,   201,   345,   345,   345,   102,   345,   345,   430,   431,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,    79,   118,   201,   226,   404,   405,   442,   445,
     442,    35,   345,   457,   458,   345,   118,   201,   226,   404,
     405,   406,   441,   449,   454,   455,   201,   336,   407,   201,
     336,   352,   337,   345,   235,   336,   201,   201,   201,   336,
     203,   345,   218,   203,   345,    29,    55,   118,   132,   133,
     154,   174,   201,   218,   229,   404,   479,   491,   492,   494,
     184,   203,   342,   345,   373,   375,   204,   242,   345,   105,
     106,   153,   219,   222,   225,    79,   206,   294,   295,   117,
     124,   116,   124,    79,   296,   201,   201,   201,   201,   218,
     266,   480,   201,   201,    79,    85,   146,   147,   148,   471,
     472,   153,   204,   225,   225,   218,   267,   480,   154,   201,
     201,   201,   201,   480,   480,    79,   198,   204,   354,   335,
     345,   346,   442,   446,   231,   204,    85,   408,   471,    85,
     471,   471,    30,   153,   170,   481,   201,     9,   203,    35,
     248,   154,   265,   480,   118,   183,   249,   327,   203,   203,
     203,   203,   203,   203,    10,    11,    12,    29,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    53,
      65,   203,    66,    66,   203,   204,   149,   125,   160,   162,
     268,   325,   326,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    63,    64,   128,   129,
     434,    66,   204,   439,   201,   201,    66,   204,   206,   450,
     201,   248,   249,    14,   345,   203,   130,    44,   218,   429,
     201,   335,   442,   446,   149,   442,   130,   208,     9,   415,
     335,   442,   481,   149,   201,   409,   434,   439,   202,   345,
      30,   233,     8,   357,     9,   203,   233,   234,   337,   338,
     345,   218,   280,   237,   203,   203,   203,   494,   494,   170,
     201,   105,   494,    14,   149,   218,    79,   203,   203,   203,
     184,   185,   186,   191,   192,   195,   196,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   388,   389,   390,   243,
     109,   167,   203,   153,   220,   223,   225,   153,   221,   224,
     225,   225,     9,   203,    97,   204,   442,     9,   203,   124,
     124,    14,     9,   203,   442,   475,   475,   335,   346,   442,
     445,   446,   202,   170,   260,   131,   442,   456,   457,    66,
     434,   146,   472,    78,   345,   442,    85,   146,   472,   225,
     217,   203,   204,   255,   263,   394,   396,    86,   206,   358,
     359,   361,   405,   450,   476,   345,   464,   466,   345,   463,
     465,   463,    14,    97,   477,   353,   355,   356,   290,   291,
     432,   433,   202,   202,   202,   202,   205,   232,   233,   250,
     257,   262,   432,   345,   207,   209,   210,   218,   482,   483,
     494,    35,   163,   292,   293,   345,   479,   201,   480,   258,
     248,   345,   345,   345,    30,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   406,   345,   345,
     452,   452,   345,   459,   460,   124,   204,   218,   449,   450,
     266,   218,   267,   265,   249,    27,    35,   339,   342,   345,
     373,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   154,   204,   218,   435,   436,   437,   438,
     449,   452,   345,   292,   292,   452,   345,   456,   248,   202,
     345,   201,   428,     9,   415,   335,   202,   218,    35,   345,
      35,   345,   202,   202,   449,   292,   204,   218,   435,   436,
     449,   202,   231,   284,   204,   342,   345,   345,    89,    30,
     233,   278,   203,    28,    97,    14,     9,   202,    30,   204,
     281,   494,    86,   229,   488,   489,   490,   201,     9,    46,
      47,    52,    54,    66,    86,   132,   145,   154,   174,   175,
     176,   177,   201,   226,   227,   229,   368,   369,   370,   404,
     410,   411,   412,   218,   493,   187,    79,   345,    79,    79,
     345,   385,   386,   345,   345,   378,   388,   190,   391,   231,
     201,   241,   225,   203,     9,    97,   225,   203,     9,    97,
      97,   222,   218,   345,   295,   411,    79,     9,   202,   202,
     202,   202,   202,   202,   202,   203,    46,    47,   486,   487,
     126,   271,   201,     9,   202,   202,    79,    80,   218,   473,
     218,    66,   205,   205,   214,   216,    30,   127,   270,   169,
      50,   154,   169,   398,   130,     9,   415,   202,   149,   202,
       9,   415,   130,   202,     9,   415,   202,   494,   494,    14,
     357,   290,   231,   199,     9,   416,   494,   495,   434,   439,
     205,     9,   415,   171,   442,   345,   202,     9,   416,    14,
     349,   251,   126,   269,   201,   480,   345,    30,   208,   208,
     130,   205,     9,   415,   345,   481,   201,   261,   256,   264,
     259,   248,    68,   442,   345,   481,   208,   205,   202,   202,
     208,   205,   202,    46,    47,    66,    74,    75,    76,    86,
     132,   145,   174,   218,   418,   420,   421,   424,   427,   218,
     442,   442,   130,   434,   439,   202,   345,   285,    71,    72,
     286,   231,   336,   231,   338,    97,    35,   131,   275,   442,
     411,   218,    30,   233,   279,   203,   282,   203,   282,     9,
     171,   130,   149,     9,   415,   202,   163,   482,   483,   484,
     482,   411,   411,   411,   411,   411,   414,   417,   201,    85,
     149,   201,   201,   201,   201,   411,   149,   204,    10,    11,
      12,    29,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    65,   149,   481,   345,   187,   187,    14,
     193,   194,   387,     9,   197,   391,    79,   205,   404,   204,
     245,    97,   223,   218,    97,   224,   218,   218,   205,    14,
     442,   203,     9,   171,   218,   272,   404,   204,   456,   131,
     442,    14,   208,   345,   205,   214,   494,   272,   204,   397,
      14,   345,   358,   218,   345,   345,   345,   203,   494,   199,
     205,    30,   485,   433,    35,    79,   163,   435,   436,   438,
     494,    35,   163,   345,   411,   290,   201,   404,   270,   350,
     252,   345,   345,   345,   205,   201,   292,   271,    30,   270,
     269,   480,   406,   205,   201,    14,    74,    75,    76,   218,
     419,   419,   421,   422,   423,    48,   201,    85,   146,   201,
       9,   415,   202,   428,    35,   345,   435,   436,   205,    71,
      72,   287,   336,   233,   205,   203,    90,   203,   275,   442,
     201,   130,   274,    14,   231,   282,    99,   100,   101,   282,
     205,   494,   494,   218,   488,     9,   202,   415,   130,   208,
       9,   415,   414,   218,   358,   360,   362,   411,   468,   470,
     411,   467,   469,   467,   202,   124,   218,   411,   461,   462,
     411,   411,   411,    30,   411,   411,   411,   411,   411,   411,
     411,   411,   411,   411,   411,   411,   411,   411,   411,   411,
     411,   411,   411,   411,   411,   411,   411,   493,   345,   345,
     345,   386,   345,   376,    79,   246,   218,   218,   411,   487,
      97,     9,   300,   202,   201,   339,   342,   345,   208,   205,
     477,   300,   155,   168,   204,   393,   400,   155,   204,   399,
     130,   130,   203,   485,   494,   357,   495,    79,   163,    14,
      79,   481,   442,   345,   202,   290,   204,   290,   201,   130,
     201,   292,   202,   204,   494,   204,   270,   253,   409,   292,
     130,   208,     9,   415,   420,   422,   146,   358,   425,   426,
     421,   442,   336,    30,    73,   233,   203,   338,   274,   456,
     275,   202,   411,    96,    99,   203,   345,    30,   203,   283,
     205,   171,   130,   163,    30,   202,   411,   411,   202,   130,
       9,   415,   202,   202,     9,   415,   130,   202,     9,   415,
     202,   130,   205,     9,   415,   411,    30,   188,   202,   231,
     218,   494,   404,     4,   106,   111,   117,   119,   156,   157,
     159,   205,   301,   324,   325,   326,   331,   332,   333,   334,
     432,   456,   205,   204,   205,    50,   345,   345,   345,   345,
     357,    35,    79,   163,    14,    79,   345,   201,   485,   202,
     300,   202,   290,   345,   292,   202,   300,   477,   300,   204,
     201,   202,   421,   421,   202,   130,   202,     9,   415,    30,
     231,   203,   202,   202,   202,   238,   203,   203,   283,   231,
     494,   494,   130,   411,   358,   411,   411,   411,   411,   411,
     411,   345,   204,   205,    97,   126,   127,   178,   479,   273,
     404,   106,   334,   119,   132,   133,   154,   160,   310,   311,
     312,   404,   158,   316,   317,   122,   201,   218,   318,   319,
     302,   249,   118,   494,     9,   203,     9,   203,   203,   477,
     325,   202,   297,   154,   395,   205,   205,    79,   163,    14,
      79,   345,   292,   111,   347,   485,   205,   485,   202,   202,
     205,   204,   205,   300,   290,   130,   421,   358,   231,   236,
     239,    30,   233,   277,   231,   202,   411,   130,   130,   130,
     189,   231,   494,   404,   404,   480,    14,     9,   203,   204,
     479,   477,   170,   204,     9,   203,     3,     4,     5,     6,
       7,    10,    11,    12,    13,    27,    28,    53,    67,    68,
      69,    70,    71,    72,    73,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   131,   132,   134,   135,
     136,   137,   138,   150,   151,   152,   162,   164,   165,   167,
     174,   178,   180,   182,   183,   218,   401,   402,     9,   203,
     154,   158,   218,   319,   320,   321,   203,    79,   330,   248,
     303,   479,   479,    14,   249,   205,   298,   299,   479,    14,
      79,   345,   202,   201,   204,   203,   204,   322,   347,   485,
     297,   205,   202,   421,   130,    30,   233,   276,   277,   231,
     411,   411,   411,   345,   205,   203,   203,   411,   404,   306,
     494,   313,   410,   311,    14,    30,    47,   314,   317,     9,
      33,   202,    29,    46,    49,    14,     9,   203,   480,   330,
      14,   494,   248,   203,    14,   345,    35,    79,   392,   231,
     231,   204,   322,   205,   485,   421,   231,    94,   190,   244,
     205,   218,   229,   307,   308,   309,     9,   171,     9,   205,
     411,   402,   402,    55,   315,   320,   320,    29,    46,    49,
     411,    79,   201,   203,   411,   480,   411,    79,     9,   416,
     205,   205,   231,   322,    92,   203,    79,   109,   240,   149,
      97,   494,   410,   161,    14,   304,   201,    35,    79,   202,
     205,   203,   201,   167,   247,   218,   325,   326,   171,   411,
     288,   289,   433,   305,    79,   404,   245,   164,   218,   203,
     202,     9,   416,   113,   114,   115,   328,   329,   288,    79,
     273,   203,   485,   433,   495,   202,   202,   203,   203,   204,
     323,   328,    35,    79,   163,   485,   204,   231,   495,    79,
     163,    14,    79,   323,   231,   205,    35,    79,   163,    14,
      79,   345,   205,    79,   163,    14,    79,   345,    14,    79,
     345,   345
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
#line 1499 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1501 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1504 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1510 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1517 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1523 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1528 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1530 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1534 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1537 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1540 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1543 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1545 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1555 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1558 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1566 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1581 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1587 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1589 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1591 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1592 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1598 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1600 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1601 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1605 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1607 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1617 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1621 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1624 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1629 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1634 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1635 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1637 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1641 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1642 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1643 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1644 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1649 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1650 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1651 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1652 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1654 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1660 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1663 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1664 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1668 "hphp.y"
    { (yyval).reset();;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1669 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1673 "hphp.y"
    { (yyval).reset();;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
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
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1682 "hphp.y"
    { (yyval).reset();;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1685 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1687 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1690 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1693 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1694 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1695 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1696 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1700 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1701 "hphp.y"
    { (yyval).reset();;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1704 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1710 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1713 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1714 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1718 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1724 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1726 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1730 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1744 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1745 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1749 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { (yyval).reset();;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
#line 1800 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1858 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1864 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1867 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1868 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1869 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
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
#line 2065 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2066 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2070 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_VARRAY);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2075 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2077 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2082 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_VARRAY);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2087 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2094 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2096 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2100 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2101 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2102 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2103 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2105 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2109 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2117 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2124 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2126 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2128 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2132 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2139 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2140 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2141 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2142 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2147 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2151 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2155 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2160 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2165 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2169 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2173 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2174 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2178 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2179 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2183 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2184 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2193 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2201 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2205 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2207 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2215 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
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

  case 536:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
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

  case 537:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { (yyval).reset();;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { (yyval).reset();;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2332 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2337 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2338 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2343 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2344 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2345 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2348 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2349 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2352 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2353 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2355 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { (yyval).reset();;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { (yyval).reset();;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { (yyval).reset();;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { (yyval).reset();;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2452 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2459 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2465 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2467 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2471 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2476 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2479 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2489 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2503 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2506 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2511 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2512 "hphp.y"
    { (yyval).reset();;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { (yyval).reset();;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2517 "hphp.y"
    { (yyval).reset();;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2520 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2521 "hphp.y"
    { (yyval).reset();;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2529 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2538 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2541 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2547 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2549 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2553 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2564 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2565 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2569 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2576 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2577 "hphp.y"
    { (yyval).reset();;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2582 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2584 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2591 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2597 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2598 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2603 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { (yyval).reset();;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2615 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2616 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2628 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2630 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2633 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { (yyval).reset();;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2646 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2647 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2648 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2652 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2653 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2657 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
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
#line 2667 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2672 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2674 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2679 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2681 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2685 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2686 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2687 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2688 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2689 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2691 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2694 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2697 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2699 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2701 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2702 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2706 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2707 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2708 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2709 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2712 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2716 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2718 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2723 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2724 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2725 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2729 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2733 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2740 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2744 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2751 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2755 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2759 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2763 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2765 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2770 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2771 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2775 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2776 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2780 "hphp.y"
    { (yyval).reset();;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2784 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2785 "hphp.y"
    { (yyval)++;;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2793 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2796 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2800 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2804 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2807 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2811 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2812 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2816 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2817 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2819 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2820 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2821 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2822 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2827 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2828 "hphp.y"
    { (yyval).reset();;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2832 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2833 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2834 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2835 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2840 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2841 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2842 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2847 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2848 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2852 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2853 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2854 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2855 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2860 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2861 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2866 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2868 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2870 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2871 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2876 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2877 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2881 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2882 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2886 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2887 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2890 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2891 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2896 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2897 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2901 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2902 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2907 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2909 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2913 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2914 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2918 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2920 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2928 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2930 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2932 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]).num(), (yyvsp[(3) - (3)]));;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2934 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2936 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2937 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2940 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2941 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2942 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2946 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2947 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2948 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2949 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2950 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2951 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2952 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2953 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2954 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2955 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 2956 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 2961 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 2966 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 2968 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 2982 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 2986 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 2992 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 2993 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 2999 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3003 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3009 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3010 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3014 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3017 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3023 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3028 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3029 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3030 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3031 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3035 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3036 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3041 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3042 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3045 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (6)]).text()); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3047 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (4)]).text()); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3051 "hphp.y"
    {;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3052 "hphp.y"
    {;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3053 "hphp.y"
    {;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3059 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3064 "hphp.y"
    { ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3076 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3078 "hphp.y"
    {;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3082 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3086 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3087 "hphp.y"
    { (yyvsp[(1) - (1)]).setText("static"); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3090 "hphp.y"
    { Token t; t.reset();
                                          _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                          _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3093 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3100 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3103 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3106 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 948:

/* Line 1455 of yacc.c  */
#line 3107 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 949:

/* Line 1455 of yacc.c  */
#line 3110 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 950:

/* Line 1455 of yacc.c  */
#line 3113 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 951:

/* Line 1455 of yacc.c  */
#line 3116 "hphp.y"
    { only_in_hh_syntax(_p);
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                         _p->onTypeList((yyval), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 952:

/* Line 1455 of yacc.c  */
#line 3122 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 953:

/* Line 1455 of yacc.c  */
#line 3125 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 954:

/* Line 1455 of yacc.c  */
#line 3128 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 955:

/* Line 1455 of yacc.c  */
#line 3134 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 956:

/* Line 1455 of yacc.c  */
#line 3140 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 957:

/* Line 1455 of yacc.c  */
#line 3148 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 958:

/* Line 1455 of yacc.c  */
#line 3149 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13979 "hphp.tab.cpp"
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
#line 3152 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

