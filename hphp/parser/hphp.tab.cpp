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
     T_SUPER = 334,
     T_SWITCH = 335,
     T_ENDSWITCH = 336,
     T_CASE = 337,
     T_DEFAULT = 338,
     T_BREAK = 339,
     T_GOTO = 340,
     T_CONTINUE = 341,
     T_FUNCTION = 342,
     T_CONST = 343,
     T_RETURN = 344,
     T_TRY = 345,
     T_CATCH = 346,
     T_THROW = 347,
     T_USE = 348,
     T_GLOBAL = 349,
     T_PUBLIC = 350,
     T_PROTECTED = 351,
     T_PRIVATE = 352,
     T_FINAL = 353,
     T_ABSTRACT = 354,
     T_STATIC = 355,
     T_VAR = 356,
     T_UNSET = 357,
     T_ISSET = 358,
     T_EMPTY = 359,
     T_HALT_COMPILER = 360,
     T_CLASS = 361,
     T_INTERFACE = 362,
     T_EXTENDS = 363,
     T_IMPLEMENTS = 364,
     T_OBJECT_OPERATOR = 365,
     T_NULLSAFE_OBJECT_OPERATOR = 366,
     T_DOUBLE_ARROW = 367,
     T_LIST = 368,
     T_ARRAY = 369,
     T_CALLABLE = 370,
     T_CLASS_C = 371,
     T_METHOD_C = 372,
     T_FUNC_C = 373,
     T_LINE = 374,
     T_FILE = 375,
     T_COMMENT = 376,
     T_DOC_COMMENT = 377,
     T_OPEN_TAG = 378,
     T_OPEN_TAG_WITH_ECHO = 379,
     T_CLOSE_TAG = 380,
     T_WHITESPACE = 381,
     T_START_HEREDOC = 382,
     T_END_HEREDOC = 383,
     T_DOLLAR_OPEN_CURLY_BRACES = 384,
     T_CURLY_OPEN = 385,
     T_DOUBLE_COLON = 386,
     T_NAMESPACE = 387,
     T_NS_C = 388,
     T_DIR = 389,
     T_NS_SEPARATOR = 390,
     T_XHP_LABEL = 391,
     T_XHP_TEXT = 392,
     T_XHP_ATTRIBUTE = 393,
     T_XHP_CATEGORY = 394,
     T_XHP_CATEGORY_LABEL = 395,
     T_XHP_CHILDREN = 396,
     T_ENUM = 397,
     T_XHP_REQUIRED = 398,
     T_TRAIT = 399,
     T_ELLIPSIS = 400,
     T_INSTEADOF = 401,
     T_TRAIT_C = 402,
     T_HH_ERROR = 403,
     T_FINALLY = 404,
     T_XHP_TAG_LT = 405,
     T_XHP_TAG_GT = 406,
     T_TYPELIST_LT = 407,
     T_TYPELIST_GT = 408,
     T_UNRESOLVED_LT = 409,
     T_COLLECTION = 410,
     T_SHAPE = 411,
     T_TYPE = 412,
     T_UNRESOLVED_TYPE = 413,
     T_NEWTYPE = 414,
     T_UNRESOLVED_NEWTYPE = 415,
     T_COMPILER_HALT_OFFSET = 416,
     T_ASYNC = 417,
     T_FROM = 418,
     T_WHERE = 419,
     T_JOIN = 420,
     T_IN = 421,
     T_ON = 422,
     T_EQUALS = 423,
     T_INTO = 424,
     T_LET = 425,
     T_ORDERBY = 426,
     T_ASCENDING = 427,
     T_DESCENDING = 428,
     T_SELECT = 429,
     T_GROUP = 430,
     T_BY = 431,
     T_LAMBDA_OP = 432,
     T_LAMBDA_CP = 433,
     T_UNRESOLVED_OP = 434
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
#line 880 "hphp.tab.cpp"

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
#define YYLAST   17110

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  209
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  275
/* YYNRULES -- Number of rules.  */
#define YYNRULES  938
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1760

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   434

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    52,   207,     2,   204,    51,    35,   208,
     199,   200,    49,    46,     9,    47,    48,    50,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    30,   201,
      40,    14,    41,    29,    55,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    66,     2,   206,    34,     2,   205,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   202,    33,   203,    54,     2,     2,     2,
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
     194,   195,   196,   197,   198
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
     100,   102,   104,   108,   110,   114,   116,   120,   122,   124,
     127,   131,   136,   138,   141,   145,   150,   152,   155,   159,
     164,   166,   170,   172,   176,   179,   181,   184,   187,   193,
     198,   201,   202,   204,   206,   208,   210,   214,   220,   229,
     230,   235,   236,   243,   244,   255,   256,   261,   264,   268,
     271,   275,   278,   282,   286,   290,   294,   298,   304,   306,
     308,   310,   311,   321,   322,   333,   339,   340,   354,   355,
     361,   365,   369,   372,   375,   378,   381,   384,   387,   391,
     394,   397,   401,   404,   405,   410,   420,   421,   422,   427,
     430,   431,   433,   434,   436,   437,   447,   448,   459,   460,
     472,   473,   483,   484,   495,   496,   505,   506,   516,   517,
     525,   526,   535,   536,   544,   545,   554,   556,   558,   560,
     562,   564,   567,   571,   575,   578,   581,   582,   585,   586,
     589,   590,   592,   596,   598,   602,   605,   606,   608,   611,
     616,   618,   623,   625,   630,   632,   637,   639,   644,   648,
     654,   658,   663,   668,   674,   680,   685,   686,   688,   690,
     695,   696,   702,   703,   706,   707,   711,   712,   720,   729,
     736,   739,   745,   752,   757,   758,   763,   769,   777,   784,
     791,   799,   809,   818,   825,   833,   839,   842,   847,   853,
     857,   858,   862,   867,   874,   880,   886,   893,   902,   910,
     913,   914,   916,   919,   922,   926,   931,   936,   940,   942,
     944,   947,   952,   956,   962,   964,   968,   971,   972,   975,
     979,   982,   983,   984,   989,   990,   996,   999,  1002,  1005,
    1006,  1017,  1018,  1030,  1034,  1038,  1042,  1047,  1052,  1056,
    1062,  1065,  1068,  1069,  1076,  1082,  1087,  1091,  1093,  1095,
    1099,  1104,  1106,  1109,  1111,  1113,  1118,  1125,  1127,  1129,
    1134,  1136,  1138,  1142,  1145,  1146,  1149,  1150,  1152,  1156,
    1158,  1160,  1162,  1164,  1168,  1173,  1178,  1183,  1185,  1187,
    1190,  1193,  1196,  1200,  1204,  1206,  1208,  1210,  1212,  1216,
    1218,  1222,  1224,  1226,  1228,  1229,  1231,  1234,  1236,  1238,
    1240,  1242,  1244,  1246,  1248,  1250,  1251,  1253,  1255,  1257,
    1261,  1267,  1269,  1273,  1279,  1284,  1288,  1292,  1296,  1301,
    1305,  1309,  1313,  1316,  1318,  1320,  1324,  1328,  1330,  1332,
    1333,  1335,  1338,  1343,  1347,  1354,  1357,  1361,  1368,  1370,
    1372,  1374,  1376,  1378,  1385,  1389,  1394,  1401,  1405,  1409,
    1413,  1417,  1421,  1425,  1429,  1433,  1437,  1441,  1445,  1449,
    1452,  1455,  1458,  1461,  1465,  1469,  1473,  1477,  1481,  1485,
    1489,  1493,  1497,  1501,  1505,  1509,  1513,  1517,  1521,  1525,
    1529,  1532,  1535,  1538,  1541,  1545,  1549,  1553,  1557,  1561,
    1565,  1569,  1573,  1577,  1581,  1587,  1592,  1594,  1597,  1600,
    1603,  1606,  1609,  1612,  1615,  1618,  1621,  1623,  1625,  1627,
    1631,  1634,  1636,  1642,  1643,  1644,  1656,  1657,  1670,  1671,
    1675,  1676,  1681,  1682,  1689,  1690,  1698,  1699,  1705,  1708,
    1711,  1716,  1718,  1720,  1726,  1730,  1736,  1740,  1743,  1744,
    1747,  1748,  1753,  1758,  1762,  1767,  1772,  1777,  1782,  1784,
    1786,  1788,  1790,  1794,  1797,  1801,  1806,  1809,  1813,  1815,
    1818,  1820,  1823,  1825,  1827,  1829,  1831,  1833,  1835,  1840,
    1845,  1848,  1857,  1868,  1871,  1873,  1877,  1879,  1882,  1884,
    1886,  1888,  1890,  1893,  1898,  1902,  1906,  1911,  1913,  1916,
    1921,  1924,  1931,  1932,  1934,  1939,  1940,  1943,  1944,  1946,
    1948,  1952,  1954,  1958,  1960,  1962,  1966,  1970,  1972,  1974,
    1976,  1978,  1980,  1982,  1984,  1986,  1988,  1990,  1992,  1994,
    1996,  1998,  2000,  2002,  2004,  2006,  2008,  2010,  2012,  2014,
    2016,  2018,  2020,  2022,  2024,  2026,  2028,  2030,  2032,  2034,
    2036,  2038,  2040,  2042,  2044,  2046,  2048,  2050,  2052,  2054,
    2056,  2058,  2060,  2062,  2064,  2066,  2068,  2070,  2072,  2074,
    2076,  2078,  2080,  2082,  2084,  2086,  2088,  2090,  2092,  2094,
    2096,  2098,  2100,  2102,  2104,  2106,  2108,  2110,  2112,  2114,
    2116,  2118,  2120,  2122,  2124,  2126,  2128,  2130,  2135,  2137,
    2139,  2141,  2143,  2145,  2147,  2149,  2151,  2154,  2156,  2157,
    2158,  2160,  2162,  2166,  2167,  2169,  2171,  2173,  2175,  2177,
    2179,  2181,  2183,  2185,  2187,  2189,  2191,  2193,  2197,  2200,
    2202,  2204,  2209,  2213,  2218,  2220,  2222,  2226,  2230,  2234,
    2238,  2242,  2246,  2250,  2254,  2258,  2262,  2266,  2270,  2274,
    2278,  2282,  2286,  2290,  2294,  2297,  2300,  2303,  2306,  2310,
    2314,  2318,  2322,  2326,  2330,  2334,  2338,  2344,  2349,  2353,
    2357,  2361,  2363,  2365,  2367,  2369,  2373,  2377,  2381,  2384,
    2385,  2387,  2388,  2390,  2391,  2397,  2401,  2405,  2407,  2409,
    2411,  2413,  2417,  2420,  2422,  2424,  2426,  2428,  2430,  2434,
    2436,  2438,  2440,  2443,  2446,  2451,  2455,  2460,  2463,  2464,
    2470,  2474,  2478,  2480,  2484,  2486,  2489,  2490,  2496,  2500,
    2503,  2504,  2508,  2509,  2514,  2517,  2518,  2522,  2526,  2528,
    2529,  2531,  2533,  2535,  2537,  2541,  2543,  2545,  2547,  2551,
    2553,  2555,  2559,  2563,  2566,  2571,  2574,  2579,  2581,  2583,
    2585,  2587,  2589,  2593,  2599,  2603,  2608,  2613,  2617,  2619,
    2621,  2623,  2625,  2629,  2635,  2640,  2644,  2646,  2648,  2652,
    2656,  2658,  2660,  2668,  2678,  2686,  2693,  2702,  2704,  2707,
    2712,  2717,  2719,  2721,  2726,  2728,  2729,  2731,  2734,  2736,
    2738,  2742,  2748,  2752,  2756,  2757,  2759,  2763,  2769,  2773,
    2776,  2780,  2787,  2788,  2790,  2795,  2798,  2799,  2805,  2809,
    2813,  2815,  2822,  2827,  2832,  2835,  2838,  2839,  2845,  2849,
    2853,  2855,  2858,  2859,  2865,  2869,  2873,  2875,  2878,  2881,
    2883,  2886,  2888,  2893,  2897,  2901,  2908,  2912,  2914,  2916,
    2918,  2923,  2928,  2933,  2938,  2943,  2948,  2951,  2954,  2959,
    2962,  2965,  2967,  2971,  2975,  2979,  2980,  2983,  2989,  2996,
    3003,  3011,  3013,  3016,  3018,  3023,  3027,  3028,  3030,  3034,
    3037,  3041,  3043,  3045,  3046,  3047,  3050,  3053,  3056,  3061,
    3064,  3070,  3074,  3076,  3078,  3079,  3083,  3089,  3093,  3095,
    3098,  3099,  3104,  3106,  3110,  3113,  3116,  3119,  3121,  3123,
    3125,  3127,  3131,  3136,  3143,  3145,  3154,  3161,  3163
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     210,     0,    -1,    -1,   211,   212,    -1,   212,   213,    -1,
      -1,   231,    -1,   248,    -1,   255,    -1,   252,    -1,   260,
      -1,   465,    -1,   124,   199,   200,   201,    -1,   151,   223,
     201,    -1,    -1,   151,   223,   202,   214,   212,   203,    -1,
      -1,   151,   202,   215,   212,   203,    -1,   112,   217,   201,
      -1,   112,   106,   218,   201,    -1,   112,   107,   219,   201,
      -1,   228,   201,    -1,    77,    -1,    98,    -1,   157,    -1,
     158,    -1,   160,    -1,   162,    -1,   161,    -1,   183,    -1,
     184,    -1,   186,    -1,   185,    -1,   187,    -1,   188,    -1,
     189,    -1,   190,    -1,   191,    -1,   192,    -1,   193,    -1,
     194,    -1,   195,    -1,   217,     9,   220,    -1,   220,    -1,
     221,     9,   221,    -1,   221,    -1,   222,     9,   222,    -1,
     222,    -1,   223,    -1,   154,   223,    -1,   223,    97,   216,
      -1,   154,   223,    97,   216,    -1,   223,    -1,   154,   223,
      -1,   223,    97,   216,    -1,   154,   223,    97,   216,    -1,
     223,    -1,   154,   223,    -1,   223,    97,   216,    -1,   154,
     223,    97,   216,    -1,   216,    -1,   223,   154,   216,    -1,
     223,    -1,   151,   154,   223,    -1,   154,   223,    -1,   224,
      -1,   224,   468,    -1,   224,   468,    -1,   228,     9,   466,
      14,   406,    -1,   107,   466,    14,   406,    -1,   229,   230,
      -1,    -1,   231,    -1,   248,    -1,   255,    -1,   260,    -1,
     202,   229,   203,    -1,    70,   335,   231,   282,   284,    -1,
      70,   335,    30,   229,   283,   285,    73,   201,    -1,    -1,
      89,   335,   232,   276,    -1,    -1,    88,   233,   231,    89,
     335,   201,    -1,    -1,    91,   199,   337,   201,   337,   201,
     337,   200,   234,   274,    -1,    -1,    99,   335,   235,   279,
      -1,   103,   201,    -1,   103,   344,   201,    -1,   105,   201,
      -1,   105,   344,   201,    -1,   108,   201,    -1,   108,   344,
     201,    -1,    27,   103,   201,    -1,   113,   292,   201,    -1,
     119,   294,   201,    -1,    87,   336,   201,    -1,   121,   199,
     462,   200,   201,    -1,   201,    -1,    81,    -1,    82,    -1,
      -1,    93,   199,   344,    97,   273,   272,   200,   236,   275,
      -1,    -1,    93,   199,   344,    28,    97,   273,   272,   200,
     237,   275,    -1,    95,   199,   278,   200,   277,    -1,    -1,
     109,   240,   110,   199,   399,    79,   200,   202,   229,   203,
     242,   238,   245,    -1,    -1,   109,   240,   168,   239,   243,
      -1,   111,   344,   201,    -1,   104,   216,   201,    -1,   344,
     201,    -1,   338,   201,    -1,   339,   201,    -1,   340,   201,
      -1,   341,   201,    -1,   342,   201,    -1,   108,   341,   201,
      -1,   343,   201,    -1,   369,   201,    -1,   108,   368,   201,
      -1,   216,    30,    -1,    -1,   202,   241,   229,   203,    -1,
     242,   110,   199,   399,    79,   200,   202,   229,   203,    -1,
      -1,    -1,   202,   244,   229,   203,    -1,   168,   243,    -1,
      -1,    35,    -1,    -1,   106,    -1,    -1,   247,   246,   467,
     249,   199,   288,   200,   472,   321,    -1,    -1,   325,   247,
     246,   467,   250,   199,   288,   200,   472,   321,    -1,    -1,
     427,   324,   247,   246,   467,   251,   199,   288,   200,   472,
     321,    -1,    -1,   161,   216,   253,    30,   482,   464,   202,
     295,   203,    -1,    -1,   427,   161,   216,   254,    30,   482,
     464,   202,   295,   203,    -1,    -1,   266,   263,   256,   267,
     268,   202,   298,   203,    -1,    -1,   427,   266,   263,   257,
     267,   268,   202,   298,   203,    -1,    -1,   126,   264,   258,
     269,   202,   298,   203,    -1,    -1,   427,   126,   264,   259,
     269,   202,   298,   203,    -1,    -1,   163,   265,   261,   268,
     202,   298,   203,    -1,    -1,   427,   163,   265,   262,   268,
     202,   298,   203,    -1,   467,    -1,   155,    -1,   467,    -1,
     467,    -1,   125,    -1,   118,   125,    -1,   118,   117,   125,
      -1,   117,   118,   125,    -1,   117,   125,    -1,   127,   399,
      -1,    -1,   128,   270,    -1,    -1,   127,   270,    -1,    -1,
     399,    -1,   270,     9,   399,    -1,   399,    -1,   271,     9,
     399,    -1,   131,   273,    -1,    -1,   437,    -1,    35,   437,
      -1,   132,   199,   451,   200,    -1,   231,    -1,    30,   229,
      92,   201,    -1,   231,    -1,    30,   229,    94,   201,    -1,
     231,    -1,    30,   229,    90,   201,    -1,   231,    -1,    30,
     229,    96,   201,    -1,   216,    14,   406,    -1,   278,     9,
     216,    14,   406,    -1,   202,   280,   203,    -1,   202,   201,
     280,   203,    -1,    30,   280,   100,   201,    -1,    30,   201,
     280,   100,   201,    -1,   280,   101,   344,   281,   229,    -1,
     280,   102,   281,   229,    -1,    -1,    30,    -1,   201,    -1,
     282,    71,   335,   231,    -1,    -1,   283,    71,   335,    30,
     229,    -1,    -1,    72,   231,    -1,    -1,    72,    30,   229,
      -1,    -1,   287,     9,   428,   327,   483,   164,    79,    -1,
     287,     9,   428,   327,   483,    35,   164,    79,    -1,   287,
       9,   428,   327,   483,   164,    -1,   287,   411,    -1,   428,
     327,   483,   164,    79,    -1,   428,   327,   483,    35,   164,
      79,    -1,   428,   327,   483,   164,    -1,    -1,   428,   327,
     483,    79,    -1,   428,   327,   483,    35,    79,    -1,   428,
     327,   483,    35,    79,    14,   344,    -1,   428,   327,   483,
      79,    14,   344,    -1,   287,     9,   428,   327,   483,    79,
      -1,   287,     9,   428,   327,   483,    35,    79,    -1,   287,
       9,   428,   327,   483,    35,    79,    14,   344,    -1,   287,
       9,   428,   327,   483,    79,    14,   344,    -1,   289,     9,
     428,   483,   164,    79,    -1,   289,     9,   428,   483,    35,
     164,    79,    -1,   289,     9,   428,   483,   164,    -1,   289,
     411,    -1,   428,   483,   164,    79,    -1,   428,   483,    35,
     164,    79,    -1,   428,   483,   164,    -1,    -1,   428,   483,
      79,    -1,   428,   483,    35,    79,    -1,   428,   483,    35,
      79,    14,   344,    -1,   428,   483,    79,    14,   344,    -1,
     289,     9,   428,   483,    79,    -1,   289,     9,   428,   483,
      35,    79,    -1,   289,     9,   428,   483,    35,    79,    14,
     344,    -1,   289,     9,   428,   483,    79,    14,   344,    -1,
     291,   411,    -1,    -1,   344,    -1,    35,   437,    -1,   164,
     344,    -1,   291,     9,   344,    -1,   291,     9,   164,   344,
      -1,   291,     9,    35,   437,    -1,   292,     9,   293,    -1,
     293,    -1,    79,    -1,   204,   437,    -1,   204,   202,   344,
     203,    -1,   294,     9,    79,    -1,   294,     9,    79,    14,
     406,    -1,    79,    -1,    79,    14,   406,    -1,   295,   296,
      -1,    -1,   297,   201,    -1,   466,    14,   406,    -1,   298,
     299,    -1,    -1,    -1,   323,   300,   329,   201,    -1,    -1,
     325,   482,   301,   329,   201,    -1,   330,   201,    -1,   331,
     201,    -1,   332,   201,    -1,    -1,   324,   247,   246,   467,
     199,   302,   286,   200,   472,   322,    -1,    -1,   427,   324,
     247,   246,   467,   199,   303,   286,   200,   472,   322,    -1,
     157,   308,   201,    -1,   158,   315,   201,    -1,   160,   317,
     201,    -1,     4,   127,   399,   201,    -1,     4,   128,   399,
     201,    -1,   112,   271,   201,    -1,   112,   271,   202,   304,
     203,    -1,   304,   305,    -1,   304,   306,    -1,    -1,   227,
     150,   216,   165,   271,   201,    -1,   307,    97,   324,   216,
     201,    -1,   307,    97,   325,   201,    -1,   227,   150,   216,
      -1,   216,    -1,   309,    -1,   308,     9,   309,    -1,   310,
     396,   313,   314,    -1,   155,    -1,    29,   311,    -1,   311,
      -1,   133,    -1,   133,   171,   482,   172,    -1,   133,   171,
     482,     9,   482,   172,    -1,   399,    -1,   120,    -1,   161,
     202,   312,   203,    -1,   134,    -1,   405,    -1,   312,     9,
     405,    -1,    14,   406,    -1,    -1,    55,   162,    -1,    -1,
     316,    -1,   315,     9,   316,    -1,   159,    -1,   318,    -1,
     216,    -1,   123,    -1,   199,   319,   200,    -1,   199,   319,
     200,    49,    -1,   199,   319,   200,    29,    -1,   199,   319,
     200,    46,    -1,   318,    -1,   320,    -1,   320,    49,    -1,
     320,    29,    -1,   320,    46,    -1,   319,     9,   319,    -1,
     319,    33,   319,    -1,   216,    -1,   155,    -1,   159,    -1,
     201,    -1,   202,   229,   203,    -1,   201,    -1,   202,   229,
     203,    -1,   325,    -1,   120,    -1,   325,    -1,    -1,   326,
      -1,   325,   326,    -1,   114,    -1,   115,    -1,   116,    -1,
     119,    -1,   118,    -1,   117,    -1,   181,    -1,   328,    -1,
      -1,   114,    -1,   115,    -1,   116,    -1,   329,     9,    79,
      -1,   329,     9,    79,    14,   406,    -1,    79,    -1,    79,
      14,   406,    -1,   330,     9,   466,    14,   406,    -1,   107,
     466,    14,   406,    -1,   331,     9,   466,    -1,   118,   107,
     466,    -1,   118,   333,   464,    -1,   333,   464,    14,   482,
      -1,   107,   176,   467,    -1,   199,   334,   200,    -1,    68,
     401,   404,    -1,    67,   344,    -1,   388,    -1,   364,    -1,
     199,   344,   200,    -1,   336,     9,   344,    -1,   344,    -1,
     336,    -1,    -1,    27,    -1,    27,   344,    -1,    27,   344,
     131,   344,    -1,   437,    14,   338,    -1,   132,   199,   451,
     200,    14,   338,    -1,    28,   344,    -1,   437,    14,   341,
      -1,   132,   199,   451,   200,    14,   341,    -1,   345,    -1,
     437,    -1,   334,    -1,   441,    -1,   440,    -1,   132,   199,
     451,   200,    14,   344,    -1,   437,    14,   344,    -1,   437,
      14,    35,   437,    -1,   437,    14,    35,    68,   401,   404,
      -1,   437,    26,   344,    -1,   437,    25,   344,    -1,   437,
      24,   344,    -1,   437,    23,   344,    -1,   437,    22,   344,
      -1,   437,    21,   344,    -1,   437,    20,   344,    -1,   437,
      19,   344,    -1,   437,    18,   344,    -1,   437,    17,   344,
      -1,   437,    16,   344,    -1,   437,    15,   344,    -1,   437,
      64,    -1,    64,   437,    -1,   437,    63,    -1,    63,   437,
      -1,   344,    31,   344,    -1,   344,    32,   344,    -1,   344,
      10,   344,    -1,   344,    12,   344,    -1,   344,    11,   344,
      -1,   344,    33,   344,    -1,   344,    35,   344,    -1,   344,
      34,   344,    -1,   344,    48,   344,    -1,   344,    46,   344,
      -1,   344,    47,   344,    -1,   344,    49,   344,    -1,   344,
      50,   344,    -1,   344,    65,   344,    -1,   344,    51,   344,
      -1,   344,    45,   344,    -1,   344,    44,   344,    -1,    46,
     344,    -1,    47,   344,    -1,    52,   344,    -1,    54,   344,
      -1,   344,    37,   344,    -1,   344,    36,   344,    -1,   344,
      39,   344,    -1,   344,    38,   344,    -1,   344,    40,   344,
      -1,   344,    43,   344,    -1,   344,    41,   344,    -1,   344,
      42,   344,    -1,   344,    53,   401,    -1,   199,   345,   200,
      -1,   344,    29,   344,    30,   344,    -1,   344,    29,    30,
     344,    -1,   461,    -1,    62,   344,    -1,    61,   344,    -1,
      60,   344,    -1,    59,   344,    -1,    58,   344,    -1,    57,
     344,    -1,    56,   344,    -1,    69,   402,    -1,    55,   344,
      -1,   408,    -1,   363,    -1,   362,    -1,   205,   403,   205,
      -1,    13,   344,    -1,   366,    -1,   112,   199,   387,   411,
     200,    -1,    -1,    -1,   247,   246,   199,   348,   288,   200,
     472,   346,   202,   229,   203,    -1,    -1,   325,   247,   246,
     199,   349,   288,   200,   472,   346,   202,   229,   203,    -1,
      -1,    79,   351,   356,    -1,    -1,   181,    79,   352,   356,
      -1,    -1,   196,   353,   288,   197,   472,   356,    -1,    -1,
     181,   196,   354,   288,   197,   472,   356,    -1,    -1,   181,
     202,   355,   229,   203,    -1,     8,   344,    -1,     8,   341,
      -1,     8,   202,   229,   203,    -1,    86,    -1,   463,    -1,
     358,     9,   357,   131,   344,    -1,   357,   131,   344,    -1,
     359,     9,   357,   131,   406,    -1,   357,   131,   406,    -1,
     358,   410,    -1,    -1,   359,   410,    -1,    -1,   175,   199,
     360,   200,    -1,   133,   199,   452,   200,    -1,    66,   452,
     206,    -1,   399,   202,   454,   203,    -1,   399,   202,   456,
     203,    -1,   366,    66,   447,   206,    -1,   367,    66,   447,
     206,    -1,   363,    -1,   463,    -1,   440,    -1,    86,    -1,
     199,   345,   200,    -1,   370,   371,    -1,   437,    14,   368,
      -1,   182,    79,   185,   344,    -1,   372,   383,    -1,   372,
     383,   386,    -1,   383,    -1,   383,   386,    -1,   373,    -1,
     372,   373,    -1,   374,    -1,   375,    -1,   376,    -1,   377,
      -1,   378,    -1,   379,    -1,   182,    79,   185,   344,    -1,
     189,    79,    14,   344,    -1,   183,   344,    -1,   184,    79,
     185,   344,   186,   344,   187,   344,    -1,   184,    79,   185,
     344,   186,   344,   187,   344,   188,    79,    -1,   190,   380,
      -1,   381,    -1,   380,     9,   381,    -1,   344,    -1,   344,
     382,    -1,   191,    -1,   192,    -1,   384,    -1,   385,    -1,
     193,   344,    -1,   194,   344,   195,   344,    -1,   188,    79,
     371,    -1,   387,     9,    79,    -1,   387,     9,    35,    79,
      -1,    79,    -1,    35,    79,    -1,   169,   155,   389,   170,
      -1,   391,    50,    -1,   391,   170,   392,   169,    50,   390,
      -1,    -1,   155,    -1,   391,   393,    14,   394,    -1,    -1,
     392,   395,    -1,    -1,   155,    -1,   156,    -1,   202,   344,
     203,    -1,   156,    -1,   202,   344,   203,    -1,   388,    -1,
     397,    -1,   396,    30,   397,    -1,   396,    47,   397,    -1,
     216,    -1,    69,    -1,   106,    -1,   107,    -1,   108,    -1,
      27,    -1,    28,    -1,   109,    -1,   110,    -1,   168,    -1,
     111,    -1,    70,    -1,    71,    -1,    73,    -1,    72,    -1,
      89,    -1,    90,    -1,    88,    -1,    91,    -1,    92,    -1,
      93,    -1,    94,    -1,    95,    -1,    96,    -1,    53,    -1,
      97,    -1,    99,    -1,   100,    -1,   101,    -1,   102,    -1,
     103,    -1,   105,    -1,   104,    -1,    87,    -1,    13,    -1,
     125,    -1,   126,    -1,   127,    -1,   128,    -1,    68,    -1,
      67,    -1,   120,    -1,     5,    -1,     7,    -1,     6,    -1,
       4,    -1,     3,    -1,   151,    -1,   112,    -1,   113,    -1,
     122,    -1,   123,    -1,   124,    -1,   119,    -1,   118,    -1,
     117,    -1,   116,    -1,   115,    -1,   114,    -1,   181,    -1,
     121,    -1,   132,    -1,   133,    -1,    10,    -1,    12,    -1,
      11,    -1,   135,    -1,   137,    -1,   136,    -1,   138,    -1,
     139,    -1,   153,    -1,   152,    -1,   180,    -1,   163,    -1,
     166,    -1,   165,    -1,   176,    -1,   178,    -1,   175,    -1,
     226,   199,   290,   200,    -1,   227,    -1,   155,    -1,   399,
      -1,   119,    -1,   445,    -1,   399,    -1,   119,    -1,   449,
      -1,   199,   200,    -1,   335,    -1,    -1,    -1,    85,    -1,
     458,    -1,   199,   290,   200,    -1,    -1,    74,    -1,    75,
      -1,    76,    -1,    86,    -1,   138,    -1,   139,    -1,   153,
      -1,   135,    -1,   166,    -1,   136,    -1,   137,    -1,   152,
      -1,   180,    -1,   146,    85,   147,    -1,   146,   147,    -1,
     405,    -1,   225,    -1,   133,   199,   409,   200,    -1,    66,
     409,   206,    -1,   175,   199,   361,   200,    -1,   407,    -1,
     365,    -1,   199,   406,   200,    -1,   406,    31,   406,    -1,
     406,    32,   406,    -1,   406,    10,   406,    -1,   406,    12,
     406,    -1,   406,    11,   406,    -1,   406,    33,   406,    -1,
     406,    35,   406,    -1,   406,    34,   406,    -1,   406,    48,
     406,    -1,   406,    46,   406,    -1,   406,    47,   406,    -1,
     406,    49,   406,    -1,   406,    50,   406,    -1,   406,    51,
     406,    -1,   406,    45,   406,    -1,   406,    44,   406,    -1,
     406,    65,   406,    -1,    52,   406,    -1,    54,   406,    -1,
      46,   406,    -1,    47,   406,    -1,   406,    37,   406,    -1,
     406,    36,   406,    -1,   406,    39,   406,    -1,   406,    38,
     406,    -1,   406,    40,   406,    -1,   406,    43,   406,    -1,
     406,    41,   406,    -1,   406,    42,   406,    -1,   406,    29,
     406,    30,   406,    -1,   406,    29,    30,   406,    -1,   227,
     150,   216,    -1,   155,   150,   216,    -1,   227,   150,   125,
      -1,   225,    -1,    78,    -1,   463,    -1,   405,    -1,   207,
     458,   207,    -1,   208,   458,   208,    -1,   146,   458,   147,
      -1,   412,   410,    -1,    -1,     9,    -1,    -1,     9,    -1,
      -1,   412,     9,   406,   131,   406,    -1,   412,     9,   406,
      -1,   406,   131,   406,    -1,   406,    -1,    74,    -1,    75,
      -1,    76,    -1,   146,    85,   147,    -1,   146,   147,    -1,
      74,    -1,    75,    -1,    76,    -1,   216,    -1,    86,    -1,
      86,    48,   415,    -1,   413,    -1,   415,    -1,   216,    -1,
      46,   414,    -1,    47,   414,    -1,   133,   199,   417,   200,
      -1,    66,   417,   206,    -1,   175,   199,   420,   200,    -1,
     418,   410,    -1,    -1,   418,     9,   416,   131,   416,    -1,
     418,     9,   416,    -1,   416,   131,   416,    -1,   416,    -1,
     419,     9,   416,    -1,   416,    -1,   421,   410,    -1,    -1,
     421,     9,   357,   131,   416,    -1,   357,   131,   416,    -1,
     419,   410,    -1,    -1,   199,   422,   200,    -1,    -1,   424,
       9,   216,   423,    -1,   216,   423,    -1,    -1,   426,   424,
     410,    -1,    45,   425,    44,    -1,   427,    -1,    -1,   129,
      -1,   130,    -1,   216,    -1,   155,    -1,   202,   344,   203,
      -1,   430,    -1,   444,    -1,   216,    -1,   202,   344,   203,
      -1,   432,    -1,   444,    -1,    66,   447,   206,    -1,   202,
     344,   203,    -1,   438,   434,    -1,   199,   334,   200,   434,
      -1,   450,   434,    -1,   199,   334,   200,   434,    -1,   444,
      -1,   398,    -1,   442,    -1,   443,    -1,   435,    -1,   437,
     429,   431,    -1,   199,   334,   200,   429,   431,    -1,   400,
     150,   444,    -1,   439,   199,   290,   200,    -1,   440,   199,
     290,   200,    -1,   199,   437,   200,    -1,   398,    -1,   442,
      -1,   443,    -1,   435,    -1,   437,   429,   430,    -1,   199,
     334,   200,   429,   430,    -1,   439,   199,   290,   200,    -1,
     199,   437,   200,    -1,   444,    -1,   435,    -1,   199,   437,
     200,    -1,   199,   441,   200,    -1,   347,    -1,   350,    -1,
     437,   429,   433,   468,   199,   290,   200,    -1,   199,   334,
     200,   429,   433,   468,   199,   290,   200,    -1,   400,   150,
     216,   468,   199,   290,   200,    -1,   400,   150,   444,   199,
     290,   200,    -1,   400,   150,   202,   344,   203,   199,   290,
     200,    -1,   445,    -1,   448,   445,    -1,   445,    66,   447,
     206,    -1,   445,   202,   344,   203,    -1,   446,    -1,    79,
      -1,   204,   202,   344,   203,    -1,   344,    -1,    -1,   204,
      -1,   448,   204,    -1,   444,    -1,   436,    -1,   449,   429,
     431,    -1,   199,   334,   200,   429,   431,    -1,   400,   150,
     444,    -1,   199,   437,   200,    -1,    -1,   436,    -1,   449,
     429,   430,    -1,   199,   334,   200,   429,   430,    -1,   199,
     437,   200,    -1,   451,     9,    -1,   451,     9,   437,    -1,
     451,     9,   132,   199,   451,   200,    -1,    -1,   437,    -1,
     132,   199,   451,   200,    -1,   453,   410,    -1,    -1,   453,
       9,   344,   131,   344,    -1,   453,     9,   344,    -1,   344,
     131,   344,    -1,   344,    -1,   453,     9,   344,   131,    35,
     437,    -1,   453,     9,    35,   437,    -1,   344,   131,    35,
     437,    -1,    35,   437,    -1,   455,   410,    -1,    -1,   455,
       9,   344,   131,   344,    -1,   455,     9,   344,    -1,   344,
     131,   344,    -1,   344,    -1,   457,   410,    -1,    -1,   457,
       9,   406,   131,   406,    -1,   457,     9,   406,    -1,   406,
     131,   406,    -1,   406,    -1,   458,   459,    -1,   458,    85,
      -1,   459,    -1,    85,   459,    -1,    79,    -1,    79,    66,
     460,   206,    -1,    79,   429,   216,    -1,   148,   344,   203,
      -1,   148,    78,    66,   344,   206,   203,    -1,   149,   437,
     203,    -1,   216,    -1,    80,    -1,    79,    -1,   122,   199,
     462,   200,    -1,   123,   199,   437,   200,    -1,   123,   199,
     345,   200,    -1,   123,   199,   441,   200,    -1,   123,   199,
     440,   200,    -1,   123,   199,   334,   200,    -1,     7,   344,
      -1,     6,   344,    -1,     5,   199,   344,   200,    -1,     4,
     344,    -1,     3,   344,    -1,   437,    -1,   462,     9,   437,
      -1,   400,   150,   216,    -1,   400,   150,   125,    -1,    -1,
      97,   482,    -1,   176,   467,    14,   482,   201,    -1,   427,
     176,   467,    14,   482,   201,    -1,   178,   467,   464,    14,
     482,   201,    -1,   427,   178,   467,   464,    14,   482,   201,
      -1,   216,    -1,   482,   216,    -1,   216,    -1,   216,   171,
     474,   172,    -1,   171,   470,   172,    -1,    -1,   482,    -1,
     469,     9,   482,    -1,   469,   410,    -1,   469,     9,   164,
      -1,   470,    -1,   164,    -1,    -1,    -1,    30,   482,    -1,
      97,   482,    -1,    98,   482,    -1,   474,     9,   475,   216,
      -1,   475,   216,    -1,   474,     9,   475,   216,   473,    -1,
     475,   216,   473,    -1,    46,    -1,    47,    -1,    -1,    86,
     131,   482,    -1,   227,   150,   216,   131,   482,    -1,   477,
       9,   476,    -1,   476,    -1,   477,   410,    -1,    -1,   175,
     199,   478,   200,    -1,   227,    -1,   216,   150,   481,    -1,
     216,   468,    -1,    29,   482,    -1,    55,   482,    -1,   227,
      -1,   133,    -1,   134,    -1,   479,    -1,   480,   150,   481,
      -1,   133,   171,   482,   172,    -1,   133,   171,   482,     9,
     482,   172,    -1,   155,    -1,   199,   106,   199,   471,   200,
      30,   482,   200,    -1,   199,   482,     9,   469,   410,   200,
      -1,   482,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   735,   735,   735,   744,   746,   749,   750,   751,   752,
     753,   754,   755,   758,   760,   760,   762,   762,   764,   765,
     767,   769,   774,   775,   776,   777,   778,   779,   780,   781,
     782,   783,   784,   785,   786,   787,   788,   789,   790,   791,
     792,   793,   797,   799,   803,   805,   809,   811,   815,   816,
     817,   818,   823,   824,   825,   826,   831,   832,   833,   834,
     839,   840,   844,   845,   847,   850,   856,   863,   870,   874,
     880,   882,   885,   886,   887,   888,   891,   892,   896,   901,
     901,   907,   907,   914,   913,   919,   919,   924,   925,   926,
     927,   928,   929,   930,   931,   932,   933,   934,   935,   936,
     937,   941,   939,   948,   946,   953,   961,   955,   965,   963,
     967,   968,   972,   973,   974,   975,   976,   977,   978,   979,
     980,   981,   982,   990,   990,   995,  1001,  1005,  1005,  1013,
    1014,  1018,  1019,  1023,  1028,  1027,  1040,  1038,  1052,  1050,
    1066,  1065,  1074,  1072,  1084,  1083,  1102,  1100,  1119,  1118,
    1127,  1125,  1137,  1136,  1148,  1146,  1159,  1160,  1164,  1167,
    1170,  1171,  1172,  1175,  1176,  1179,  1181,  1184,  1185,  1188,
    1189,  1192,  1193,  1197,  1198,  1203,  1204,  1207,  1208,  1209,
    1213,  1214,  1218,  1219,  1223,  1224,  1228,  1229,  1234,  1235,
    1240,  1241,  1242,  1243,  1246,  1249,  1251,  1254,  1255,  1259,
    1261,  1264,  1267,  1270,  1271,  1274,  1275,  1279,  1285,  1291,
    1298,  1300,  1305,  1310,  1316,  1320,  1324,  1328,  1333,  1338,
    1343,  1348,  1354,  1363,  1368,  1373,  1379,  1381,  1385,  1389,
    1394,  1398,  1401,  1404,  1408,  1412,  1416,  1420,  1425,  1433,
    1435,  1438,  1439,  1440,  1441,  1443,  1445,  1450,  1451,  1454,
    1455,  1456,  1460,  1461,  1463,  1464,  1468,  1470,  1473,  1477,
    1483,  1485,  1488,  1488,  1492,  1491,  1495,  1497,  1500,  1503,
    1501,  1516,  1513,  1526,  1528,  1530,  1532,  1534,  1536,  1538,
    1542,  1543,  1544,  1547,  1553,  1556,  1562,  1565,  1570,  1572,
    1577,  1582,  1586,  1587,  1591,  1592,  1594,  1596,  1602,  1603,
    1605,  1609,  1610,  1615,  1616,  1620,  1621,  1625,  1627,  1633,
    1638,  1639,  1641,  1645,  1646,  1647,  1648,  1652,  1653,  1654,
    1655,  1656,  1657,  1659,  1664,  1667,  1668,  1672,  1673,  1677,
    1678,  1681,  1682,  1685,  1686,  1689,  1690,  1694,  1695,  1696,
    1697,  1698,  1699,  1700,  1704,  1705,  1708,  1709,  1710,  1713,
    1715,  1717,  1718,  1721,  1723,  1727,  1729,  1733,  1737,  1741,
    1745,  1746,  1748,  1749,  1750,  1753,  1757,  1758,  1762,  1763,
    1767,  1768,  1769,  1773,  1777,  1782,  1786,  1790,  1795,  1796,
    1797,  1798,  1799,  1803,  1805,  1806,  1807,  1810,  1811,  1812,
    1813,  1814,  1815,  1816,  1817,  1818,  1819,  1820,  1821,  1822,
    1823,  1824,  1825,  1826,  1827,  1828,  1829,  1830,  1831,  1832,
    1833,  1834,  1835,  1836,  1837,  1838,  1839,  1840,  1841,  1842,
    1843,  1844,  1845,  1846,  1847,  1848,  1849,  1850,  1851,  1852,
    1854,  1855,  1857,  1859,  1860,  1861,  1862,  1863,  1864,  1865,
    1866,  1867,  1868,  1869,  1870,  1871,  1872,  1873,  1874,  1875,
    1876,  1877,  1881,  1885,  1890,  1889,  1904,  1902,  1919,  1919,
    1935,  1934,  1952,  1952,  1968,  1967,  1986,  1985,  2006,  2007,
    2008,  2013,  2015,  2019,  2023,  2029,  2033,  2039,  2041,  2045,
    2047,  2051,  2055,  2056,  2060,  2067,  2074,  2076,  2081,  2082,
    2083,  2084,  2086,  2090,  2094,  2098,  2102,  2104,  2106,  2108,
    2113,  2114,  2119,  2120,  2121,  2122,  2123,  2124,  2128,  2132,
    2136,  2140,  2145,  2150,  2154,  2155,  2159,  2160,  2164,  2165,
    2169,  2170,  2174,  2178,  2182,  2186,  2187,  2188,  2189,  2193,
    2199,  2208,  2221,  2222,  2225,  2228,  2231,  2232,  2235,  2239,
    2242,  2245,  2252,  2253,  2257,  2258,  2260,  2264,  2265,  2266,
    2267,  2268,  2269,  2270,  2271,  2272,  2273,  2274,  2275,  2276,
    2277,  2278,  2279,  2280,  2281,  2282,  2283,  2284,  2285,  2286,
    2287,  2288,  2289,  2290,  2291,  2292,  2293,  2294,  2295,  2296,
    2297,  2298,  2299,  2300,  2301,  2302,  2303,  2304,  2305,  2306,
    2307,  2308,  2309,  2310,  2311,  2312,  2313,  2314,  2315,  2316,
    2317,  2318,  2319,  2320,  2321,  2322,  2323,  2324,  2325,  2326,
    2327,  2328,  2329,  2330,  2331,  2332,  2333,  2334,  2335,  2336,
    2337,  2338,  2339,  2340,  2341,  2342,  2343,  2347,  2352,  2353,
    2356,  2357,  2358,  2362,  2363,  2364,  2368,  2369,  2370,  2374,
    2375,  2376,  2379,  2381,  2385,  2386,  2387,  2388,  2390,  2391,
    2392,  2393,  2394,  2395,  2396,  2397,  2398,  2399,  2402,  2407,
    2408,  2409,  2411,  2412,  2414,  2415,  2416,  2417,  2419,  2421,
    2423,  2425,  2427,  2428,  2429,  2430,  2431,  2432,  2433,  2434,
    2435,  2436,  2437,  2438,  2439,  2440,  2441,  2442,  2443,  2445,
    2447,  2449,  2451,  2452,  2455,  2456,  2460,  2462,  2466,  2469,
    2472,  2478,  2479,  2480,  2481,  2482,  2483,  2484,  2489,  2491,
    2495,  2496,  2499,  2500,  2504,  2507,  2509,  2511,  2515,  2516,
    2517,  2518,  2521,  2525,  2526,  2527,  2528,  2532,  2534,  2541,
    2542,  2543,  2544,  2545,  2546,  2548,  2549,  2554,  2556,  2559,
    2562,  2564,  2566,  2569,  2571,  2575,  2577,  2580,  2583,  2589,
    2591,  2594,  2595,  2600,  2603,  2607,  2607,  2612,  2615,  2616,
    2620,  2621,  2625,  2626,  2627,  2631,  2632,  2636,  2637,  2641,
    2642,  2646,  2647,  2651,  2652,  2657,  2659,  2664,  2665,  2666,
    2667,  2668,  2669,  2679,  2690,  2693,  2695,  2697,  2701,  2702,
    2703,  2704,  2705,  2716,  2728,  2730,  2734,  2735,  2736,  2740,
    2744,  2745,  2749,  2752,  2759,  2763,  2767,  2774,  2775,  2780,
    2782,  2783,  2786,  2787,  2790,  2791,  2795,  2796,  2800,  2801,
    2802,  2813,  2824,  2827,  2830,  2831,  2832,  2843,  2855,  2859,
    2860,  2861,  2863,  2864,  2865,  2869,  2871,  2874,  2876,  2877,
    2878,  2879,  2882,  2884,  2885,  2889,  2891,  2894,  2896,  2897,
    2898,  2902,  2904,  2907,  2910,  2912,  2914,  2918,  2919,  2921,
    2922,  2928,  2929,  2931,  2941,  2943,  2945,  2948,  2949,  2950,
    2954,  2955,  2956,  2957,  2958,  2959,  2960,  2961,  2962,  2963,
    2964,  2968,  2969,  2973,  2975,  2983,  2985,  2989,  2993,  2998,
    3002,  3010,  3011,  3017,  3018,  3025,  3028,  3032,  3035,  3040,
    3045,  3047,  3048,  3049,  3053,  3054,  3058,  3059,  3062,  3064,
    3065,  3068,  3073,  3074,  3075,  3079,  3082,  3091,  3093,  3097,
    3100,  3103,  3108,  3111,  3114,  3121,  3124,  3127,  3128,  3131,
    3134,  3135,  3140,  3143,  3147,  3151,  3157,  3167,  3168
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
  "T_ENDFOREACH", "T_DECLARE", "T_ENDDECLARE", "T_AS", "T_SUPER",
  "T_SWITCH", "T_ENDSWITCH", "T_CASE", "T_DEFAULT", "T_BREAK", "T_GOTO",
  "T_CONTINUE", "T_FUNCTION", "T_CONST", "T_RETURN", "T_TRY", "T_CATCH",
  "T_THROW", "T_USE", "T_GLOBAL", "T_PUBLIC", "T_PROTECTED", "T_PRIVATE",
  "T_FINAL", "T_ABSTRACT", "T_STATIC", "T_VAR", "T_UNSET", "T_ISSET",
  "T_EMPTY", "T_HALT_COMPILER", "T_CLASS", "T_INTERFACE", "T_EXTENDS",
  "T_IMPLEMENTS", "T_OBJECT_OPERATOR", "T_NULLSAFE_OBJECT_OPERATOR",
  "T_DOUBLE_ARROW", "T_LIST", "T_ARRAY", "T_CALLABLE", "T_CLASS_C",
  "T_METHOD_C", "T_FUNC_C", "T_LINE", "T_FILE", "T_COMMENT",
  "T_DOC_COMMENT", "T_OPEN_TAG", "T_OPEN_TAG_WITH_ECHO", "T_CLOSE_TAG",
  "T_WHITESPACE", "T_START_HEREDOC", "T_END_HEREDOC",
  "T_DOLLAR_OPEN_CURLY_BRACES", "T_CURLY_OPEN", "T_DOUBLE_COLON",
  "T_NAMESPACE", "T_NS_C", "T_DIR", "T_NS_SEPARATOR", "T_XHP_LABEL",
  "T_XHP_TEXT", "T_XHP_ATTRIBUTE", "T_XHP_CATEGORY",
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
  "hh_opt_return_type", "hh_constraint", "hh_typevar_list",
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
     426,   427,   428,   429,   430,   431,   432,   433,   434,    40,
      41,    59,   123,   125,    36,    96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   209,   211,   210,   212,   212,   213,   213,   213,   213,
     213,   213,   213,   213,   214,   213,   215,   213,   213,   213,
     213,   213,   216,   216,   216,   216,   216,   216,   216,   216,
     216,   216,   216,   216,   216,   216,   216,   216,   216,   216,
     216,   216,   217,   217,   218,   218,   219,   219,   220,   220,
     220,   220,   221,   221,   221,   221,   222,   222,   222,   222,
     223,   223,   224,   224,   224,   225,   226,   227,   228,   228,
     229,   229,   230,   230,   230,   230,   231,   231,   231,   232,
     231,   233,   231,   234,   231,   235,   231,   231,   231,   231,
     231,   231,   231,   231,   231,   231,   231,   231,   231,   231,
     231,   236,   231,   237,   231,   231,   238,   231,   239,   231,
     231,   231,   231,   231,   231,   231,   231,   231,   231,   231,
     231,   231,   231,   241,   240,   242,   242,   244,   243,   245,
     245,   246,   246,   247,   249,   248,   250,   248,   251,   248,
     253,   252,   254,   252,   256,   255,   257,   255,   258,   255,
     259,   255,   261,   260,   262,   260,   263,   263,   264,   265,
     266,   266,   266,   266,   266,   267,   267,   268,   268,   269,
     269,   270,   270,   271,   271,   272,   272,   273,   273,   273,
     274,   274,   275,   275,   276,   276,   277,   277,   278,   278,
     279,   279,   279,   279,   280,   280,   280,   281,   281,   282,
     282,   283,   283,   284,   284,   285,   285,   286,   286,   286,
     286,   286,   286,   286,   286,   287,   287,   287,   287,   287,
     287,   287,   287,   288,   288,   288,   288,   288,   288,   288,
     288,   289,   289,   289,   289,   289,   289,   289,   289,   290,
     290,   291,   291,   291,   291,   291,   291,   292,   292,   293,
     293,   293,   294,   294,   294,   294,   295,   295,   296,   297,
     298,   298,   300,   299,   301,   299,   299,   299,   299,   302,
     299,   303,   299,   299,   299,   299,   299,   299,   299,   299,
     304,   304,   304,   305,   306,   306,   307,   307,   308,   308,
     309,   309,   310,   310,   311,   311,   311,   311,   311,   311,
     311,   312,   312,   313,   313,   314,   314,   315,   315,   316,
     317,   317,   317,   318,   318,   318,   318,   319,   319,   319,
     319,   319,   319,   319,   320,   320,   320,   321,   321,   322,
     322,   323,   323,   324,   324,   325,   325,   326,   326,   326,
     326,   326,   326,   326,   327,   327,   328,   328,   328,   329,
     329,   329,   329,   330,   330,   331,   331,   332,   332,   333,
     334,   334,   334,   334,   334,   335,   336,   336,   337,   337,
     338,   338,   338,   339,   340,   341,   342,   343,   344,   344,
     344,   344,   344,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   345,   345,   345,   345,   345,   345,   345,   345,
     345,   345,   346,   346,   348,   347,   349,   347,   351,   350,
     352,   350,   353,   350,   354,   350,   355,   350,   356,   356,
     356,   357,   357,   358,   358,   359,   359,   360,   360,   361,
     361,   362,   363,   363,   364,   365,   366,   366,   367,   367,
     367,   367,   367,   368,   369,   370,   371,   371,   371,   371,
     372,   372,   373,   373,   373,   373,   373,   373,   374,   375,
     376,   377,   378,   379,   380,   380,   381,   381,   382,   382,
     383,   383,   384,   385,   386,   387,   387,   387,   387,   388,
     389,   389,   390,   390,   391,   391,   392,   392,   393,   394,
     394,   395,   395,   395,   396,   396,   396,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   398,   399,   399,
     400,   400,   400,   401,   401,   401,   402,   402,   402,   403,
     403,   403,   404,   404,   405,   405,   405,   405,   405,   405,
     405,   405,   405,   405,   405,   405,   405,   405,   405,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   407,   407,
     407,   408,   408,   408,   408,   408,   408,   408,   409,   409,
     410,   410,   411,   411,   412,   412,   412,   412,   413,   413,
     413,   413,   413,   414,   414,   414,   414,   415,   415,   416,
     416,   416,   416,   416,   416,   416,   416,   417,   417,   418,
     418,   418,   418,   419,   419,   420,   420,   421,   421,   422,
     422,   423,   423,   424,   424,   426,   425,   427,   428,   428,
     429,   429,   430,   430,   430,   431,   431,   432,   432,   433,
     433,   434,   434,   435,   435,   436,   436,   437,   437,   437,
     437,   437,   437,   437,   437,   437,   437,   437,   438,   438,
     438,   438,   438,   438,   438,   438,   439,   439,   439,   440,
     441,   441,   442,   442,   443,   443,   443,   444,   444,   445,
     445,   445,   446,   446,   447,   447,   448,   448,   449,   449,
     449,   449,   449,   449,   450,   450,   450,   450,   450,   451,
     451,   451,   451,   451,   451,   452,   452,   453,   453,   453,
     453,   453,   453,   453,   453,   454,   454,   455,   455,   455,
     455,   456,   456,   457,   457,   457,   457,   458,   458,   458,
     458,   459,   459,   459,   459,   459,   459,   460,   460,   460,
     461,   461,   461,   461,   461,   461,   461,   461,   461,   461,
     461,   462,   462,   463,   463,   464,   464,   465,   465,   465,
     465,   466,   466,   467,   467,   468,   468,   469,   469,   470,
     471,   471,   471,   471,   472,   472,   473,   473,   474,   474,
     474,   474,   475,   475,   475,   476,   476,   477,   477,   478,
     478,   479,   480,   481,   481,   482,   482,   482,   482,   482,
     482,   482,   482,   482,   482,   482,   482,   483,   483
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     1,     4,     3,     0,     6,     0,     5,     3,     4,
       4,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     1,     3,     1,     3,     1,     1,     2,
       3,     4,     1,     2,     3,     4,     1,     2,     3,     4,
       1,     3,     1,     3,     2,     1,     2,     2,     5,     4,
       2,     0,     1,     1,     1,     1,     3,     5,     8,     0,
       4,     0,     6,     0,    10,     0,     4,     2,     3,     2,
       3,     2,     3,     3,     3,     3,     3,     5,     1,     1,
       1,     0,     9,     0,    10,     5,     0,    13,     0,     5,
       3,     3,     2,     2,     2,     2,     2,     2,     3,     2,
       2,     3,     2,     0,     4,     9,     0,     0,     4,     2,
       0,     1,     0,     1,     0,     9,     0,    10,     0,    11,
       0,     9,     0,    10,     0,     8,     0,     9,     0,     7,
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
      10,     0,    11,     3,     3,     3,     4,     4,     3,     5,
       2,     2,     0,     6,     5,     4,     3,     1,     1,     3,
       4,     1,     2,     1,     1,     4,     6,     1,     1,     4,
       1,     1,     3,     2,     0,     2,     0,     1,     3,     1,
       1,     1,     1,     3,     4,     4,     4,     1,     1,     2,
       2,     2,     3,     3,     1,     1,     1,     1,     3,     1,
       3,     1,     1,     1,     0,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     1,     1,     1,     3,
       5,     1,     3,     5,     4,     3,     3,     3,     4,     3,
       3,     3,     2,     1,     1,     3,     3,     1,     1,     0,
       1,     2,     4,     3,     6,     2,     3,     6,     1,     1,
       1,     1,     1,     6,     3,     4,     6,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     5,     4,     1,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     1,     1,     3,
       2,     1,     5,     0,     0,    11,     0,    12,     0,     3,
       0,     4,     0,     6,     0,     7,     0,     5,     2,     2,
       4,     1,     1,     5,     3,     5,     3,     2,     0,     2,
       0,     4,     4,     3,     4,     4,     4,     4,     1,     1,
       1,     1,     3,     2,     3,     4,     2,     3,     1,     2,
       1,     2,     1,     1,     1,     1,     1,     1,     4,     4,
       2,     8,    10,     2,     1,     3,     1,     2,     1,     1,
       1,     1,     2,     4,     3,     3,     4,     1,     2,     4,
       2,     6,     0,     1,     4,     0,     2,     0,     1,     1,
       3,     1,     3,     1,     1,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     4,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     1,     0,     0,
       1,     1,     3,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     2,     1,
       1,     4,     3,     4,     1,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     5,     4,     3,     3,
       3,     1,     1,     1,     1,     3,     3,     3,     2,     0,
       1,     0,     1,     0,     5,     3,     3,     1,     1,     1,
       1,     3,     2,     1,     1,     1,     1,     1,     3,     1,
       1,     1,     2,     2,     4,     3,     4,     2,     0,     5,
       3,     3,     1,     3,     1,     2,     0,     5,     3,     2,
       0,     3,     0,     4,     2,     0,     3,     3,     1,     0,
       1,     1,     1,     1,     3,     1,     1,     1,     3,     1,
       1,     3,     3,     2,     4,     2,     4,     1,     1,     1,
       1,     1,     3,     5,     3,     4,     4,     3,     1,     1,
       1,     1,     3,     5,     4,     3,     1,     1,     3,     3,
       1,     1,     7,     9,     7,     6,     8,     1,     2,     4,
       4,     1,     1,     4,     1,     0,     1,     2,     1,     1,
       3,     5,     3,     3,     0,     1,     3,     5,     3,     2,
       3,     6,     0,     1,     4,     2,     0,     5,     3,     3,
       1,     6,     4,     4,     2,     2,     0,     5,     3,     3,
       1,     2,     0,     5,     3,     3,     1,     2,     2,     1,
       2,     1,     4,     3,     3,     6,     3,     1,     1,     1,
       4,     4,     4,     4,     4,     4,     2,     2,     4,     2,
       2,     1,     3,     3,     3,     0,     2,     5,     6,     6,
       7,     1,     2,     1,     4,     3,     0,     1,     3,     2,
       3,     1,     1,     0,     0,     2,     2,     2,     4,     2,
       5,     3,     1,     1,     0,     3,     5,     3,     1,     2,
       0,     4,     1,     3,     2,     2,     2,     1,     1,     1,
       1,     3,     4,     6,     1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   370,     0,   755,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   836,     0,
     824,   638,     0,   644,   645,   646,    22,   702,   812,    99,
     100,   647,     0,    81,     0,     0,     0,     0,    23,     0,
       0,     0,     0,   133,     0,     0,     0,     0,     0,     0,
     337,   338,   339,   342,   341,   340,     0,     0,     0,     0,
     160,     0,     0,     0,   651,   653,   654,   648,   649,     0,
       0,   655,   650,     0,   629,    24,    25,    26,    28,    27,
       0,   652,     0,     0,     0,     0,   656,   343,    29,    30,
      32,    31,    33,    34,    35,    36,    37,    38,    39,    40,
      41,   462,     0,    98,    71,   816,   639,     0,     0,     4,
      60,    62,    65,   701,     0,   628,     0,     6,   132,     7,
       9,     8,    10,     0,     0,   335,   380,     0,     0,     0,
       0,     0,     0,     0,   378,   800,   801,   448,   447,   364,
     451,     0,     0,   363,   778,   630,     0,   704,   446,   334,
     781,   379,     0,     0,   382,   381,   779,   780,   777,   807,
     811,     0,   436,   703,    11,   342,   341,   340,     0,     0,
      28,    60,   132,     0,   880,   379,   879,     0,   877,   876,
     450,     0,   371,   375,     0,     0,   420,   421,   422,   423,
     445,   443,   442,   441,   440,   439,   438,   437,   812,   631,
       0,   896,   630,     0,   402,     0,   400,     0,   840,     0,
     711,   362,   634,     0,   896,   633,     0,   643,   819,   818,
     635,     0,     0,   637,   444,     0,     0,     0,     0,   367,
       0,    79,   369,     0,     0,    85,    87,     0,     0,    89,
       0,     0,     0,   928,   929,   934,     0,     0,    60,   927,
       0,   930,     0,     0,     0,    91,     0,     0,     0,     0,
     123,     0,     0,     0,     0,     0,     0,    43,    48,   249,
       0,     0,   248,     0,   164,     0,   161,   254,     0,     0,
       0,     0,     0,   893,   148,   158,   832,   836,   861,     0,
     658,     0,     0,     0,   859,     0,    16,     0,    64,   140,
     152,   159,   535,   478,     0,   885,   460,   464,   466,   759,
     380,     0,   378,   379,   381,     0,     0,   640,     0,   641,
       0,     0,     0,   122,     0,     0,    67,   240,     0,    21,
     131,     0,   157,   144,   156,   340,   343,   132,   336,   113,
     114,   115,   116,   117,   119,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     824,     0,   112,   815,   815,   120,   846,     0,     0,     0,
       0,     0,     0,     0,     0,   333,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   401,
     399,   760,   761,     0,   815,     0,   773,   240,   240,   815,
       0,   817,   808,   832,     0,   132,     0,     0,    93,     0,
     757,   752,   711,     0,     0,     0,     0,     0,   844,     0,
     483,   710,   835,     0,     0,    67,     0,   240,   361,     0,
     775,   636,     0,    71,   200,     0,   459,     0,    96,     0,
       0,   368,     0,     0,     0,     0,     0,    88,   111,    90,
     925,   926,     0,   920,     0,     0,     0,     0,   892,     0,
     118,    92,   121,     0,     0,     0,     0,     0,     0,     0,
     493,     0,   500,   502,   503,   504,   505,   506,   507,   498,
     520,   521,    71,     0,   108,   110,     0,     0,    45,    52,
       0,     0,    47,    56,    49,     0,    18,     0,     0,   250,
       0,    94,   163,   162,     0,     0,    95,   881,     0,     0,
     380,   378,   379,   382,   381,     0,   914,   170,     0,   833,
       0,     0,     0,     0,   657,   860,   702,     0,     0,   858,
     707,   857,    63,     5,    13,    14,     0,   168,     0,     0,
     471,     0,     0,   711,     0,     0,   632,   472,     0,     0,
       0,     0,   759,    71,     0,   713,   758,   938,   360,   433,
     787,   799,    76,    70,    72,    73,    74,    75,   334,     0,
     449,   705,   706,    61,   711,     0,   897,     0,     0,     0,
     713,   241,     0,   454,   134,   166,     0,   405,   407,   406,
       0,     0,   403,   404,   408,   410,   409,   425,   424,   427,
     426,   428,   430,   431,   429,   419,   418,   412,   413,   411,
     414,   415,   417,   432,   416,   814,     0,     0,   850,     0,
     711,   884,     0,   883,   784,   807,   150,   142,   154,     0,
     885,   146,   132,   370,     0,   373,   376,   384,   494,   398,
     397,   396,   395,   394,   393,   392,   391,   390,   389,   388,
     387,   763,     0,   762,   765,   782,   769,   896,   766,     0,
       0,     0,     0,     0,     0,     0,     0,   878,   372,   750,
     754,   710,   756,     0,     0,   896,     0,   839,     0,   838,
       0,   823,   822,     0,     0,   762,   765,   820,   766,   365,
     202,   204,    71,   469,   468,   366,     0,    71,   184,    80,
     369,     0,     0,     0,     0,     0,   196,   196,    86,     0,
       0,     0,   918,   711,     0,   903,     0,     0,     0,     0,
       0,   709,   647,     0,     0,   629,     0,     0,    65,   660,
     628,   665,     0,   659,    69,   664,   896,   931,     0,     0,
     510,     0,     0,   516,   513,   514,   522,     0,   501,   496,
       0,   499,     0,     0,     0,    53,    19,     0,     0,    57,
      20,     0,     0,     0,    42,    50,     0,   247,   255,   252,
       0,     0,   870,   875,   872,   871,   874,   873,    12,   912,
     913,     0,     0,     0,     0,   832,   829,     0,   482,   869,
     868,   867,     0,   863,     0,   864,   866,     0,     5,     0,
       0,     0,   529,   530,   538,   537,     0,     0,   710,   477,
     481,     0,     0,   886,     0,   461,     0,     0,   904,   759,
     226,   937,     0,     0,   774,   813,   710,   899,   895,   242,
     243,   627,   712,   239,     0,   759,     0,     0,   168,   456,
     136,   435,     0,   486,   487,     0,   484,   710,   845,     0,
       0,   240,   170,     0,   168,     0,     0,   166,     0,   824,
     385,     0,     0,   771,   772,   785,   786,   809,   810,     0,
       0,     0,   738,   718,   719,   720,   727,     0,     0,     0,
     731,   729,   730,   744,   711,     0,   752,   843,   842,     0,
       0,   776,   642,     0,   206,     0,     0,    77,     0,     0,
       0,     0,     0,     0,     0,   176,   177,   188,     0,    71,
     186,   105,   196,     0,   196,     0,     0,   932,     0,     0,
     710,   919,   921,   902,   711,   901,     0,   711,   686,   687,
     684,   685,   717,     0,   711,   709,     0,     0,   480,     0,
       0,   852,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   924,
     495,     0,     0,     0,   518,   519,   517,     0,     0,   497,
       0,   124,     0,   127,   109,     0,    44,    54,     0,    46,
      58,    51,   251,     0,   882,    97,   914,   894,   909,   169,
     171,   261,     0,     0,   830,     0,   862,     0,    17,     0,
     885,   167,   261,     0,     0,   474,     0,   883,   887,     0,
     904,   467,     0,     0,   938,     0,   231,   229,   765,   783,
     896,   898,     0,     0,   244,    68,     0,   759,   165,     0,
     759,     0,   434,   849,   848,     0,   240,     0,     0,     0,
       0,     0,     0,   168,   138,   643,   764,   240,     0,   723,
     724,   725,   726,   732,   733,   742,     0,   711,     0,   738,
       0,   722,   746,   710,   749,   751,   753,     0,   837,   765,
     821,   764,     0,     0,     0,     0,   203,   470,    82,     0,
     369,   176,   178,   832,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   190,     0,   915,     0,   917,   710,     0,
       0,     0,   662,   710,   708,     0,   699,     0,   711,     0,
     666,   700,   698,   856,     0,   711,   669,   671,   670,     0,
       0,   667,   668,   672,   674,   673,   689,   688,   691,   690,
     692,   694,   695,   693,   682,   681,   676,   677,   675,   678,
     679,   680,   683,   923,   508,     0,   509,   515,   523,   524,
       0,    71,    55,    59,   253,     0,     0,     0,   911,     0,
     334,   834,   832,   374,   377,   383,     0,    15,     0,   334,
     541,     0,     0,   543,   536,   539,     0,   534,     0,   889,
       0,   905,   463,     0,   232,     0,     0,   227,     0,   246,
     245,   904,     0,   261,     0,   759,     0,   240,     0,   805,
     261,   885,   261,   888,     0,     0,     0,   386,     0,     0,
     735,   710,   737,   728,     0,   721,     0,     0,   711,   743,
     841,     0,    71,     0,   199,   185,     0,     0,     0,   175,
     101,   189,     0,     0,   192,     0,   197,   198,    71,   191,
     933,     0,   900,     0,   936,   716,   715,   661,     0,   710,
     479,   663,     0,   485,   710,   851,   697,     0,     0,     0,
       0,   908,   906,   907,   172,     0,     0,     0,   341,   332,
       0,     0,     0,   149,   260,   262,     0,   331,     0,     0,
       0,   885,   334,     0,   865,   257,   153,   532,     0,     0,
     473,   465,     0,   235,   225,     0,   228,   234,   240,   453,
     904,   334,   904,     0,   847,     0,   804,   334,     0,   334,
     890,   261,   759,   802,   741,   740,   734,     0,   736,   710,
     745,    71,   205,    78,    83,   103,   179,     0,   187,   193,
      71,   195,   916,     0,     0,   476,     0,   855,   854,   696,
       0,    71,   128,   910,     0,     0,     0,     0,     0,   173,
       0,   885,     0,   298,   294,   300,   629,    28,     0,   288,
       0,   293,   297,   309,     0,   307,   312,     0,   311,     0,
     310,     0,   132,   264,     0,   266,     0,   267,   268,     0,
       0,   831,     0,   533,   531,   542,   540,   236,     0,     0,
     223,   233,     0,     0,     0,     0,   145,   453,   904,   806,
     151,   257,   155,   334,     0,     0,   748,     0,   201,     0,
       0,    71,   182,   102,   194,   935,   714,     0,     0,     0,
       0,     0,     0,   359,     0,     0,   278,   282,   356,   357,
     292,     0,     0,     0,   273,   593,   592,   589,   591,   590,
     610,   612,   611,   581,   552,   553,   571,   587,   586,   548,
     558,   559,   561,   560,   580,   564,   562,   563,   565,   566,
     567,   568,   569,   570,   572,   573,   574,   575,   576,   577,
     579,   578,   549,   550,   551,   554,   555,   557,   595,   596,
     605,   604,   603,   602,   601,   600,   588,   607,   597,   598,
     599,   582,   583,   584,   585,   608,   609,   613,   615,   614,
     616,   617,   594,   619,   618,   621,   623,   622,   556,   626,
     624,   625,   620,   606,   547,   304,   544,     0,   274,   325,
     326,   324,   317,     0,   318,   275,   351,     0,     0,     0,
       0,   355,     0,   132,   141,   256,     0,     0,     0,   224,
     238,   803,     0,    71,   327,    71,   135,     0,     0,     0,
     147,   904,   739,     0,    71,   180,    84,   104,     0,   475,
     853,   511,   126,   276,   277,   354,   174,     0,     0,     0,
     301,   289,     0,     0,     0,   306,   308,     0,     0,   313,
     320,   321,   319,     0,     0,   263,     0,     0,     0,   358,
       0,   258,     0,   237,     0,   527,   713,     0,     0,    71,
     137,   143,     0,   747,     0,     0,     0,   106,   279,    60,
       0,   280,   281,     0,     0,   295,     0,   299,   303,   545,
     546,     0,   290,   322,   323,   315,   316,   314,   352,   349,
     269,   265,   353,     0,   259,   528,   712,     0,   455,   328,
       0,   139,     0,   183,   512,     0,   130,     0,   334,     0,
     302,   305,     0,   759,   271,     0,   525,   452,   457,   181,
       0,     0,   107,   286,     0,   333,   296,   350,     0,   713,
     345,   759,   526,     0,   129,     0,     0,   285,   904,   759,
     210,   346,   347,   348,   938,   344,     0,     0,     0,   284,
       0,   345,     0,   904,     0,   283,   329,    71,   270,   938,
       0,   215,   213,     0,    71,     0,     0,   216,     0,     0,
     211,   272,     0,   330,     0,   219,   209,     0,   212,   218,
     125,   220,     0,     0,   207,   217,     0,   208,   222,   221
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   119,   818,   553,   181,   276,   507,
     511,   277,   508,   512,   121,   122,   123,   124,   125,   126,
     325,   583,   584,   460,   240,  1439,   466,  1357,  1440,  1676,
     774,   271,   502,  1637,  1004,  1181,  1692,   341,   182,   585,
     856,  1061,  1236,   130,   556,   873,   586,   605,   877,   537,
     872,   587,   557,   874,   343,   294,   310,   133,   858,   821,
     804,  1019,  1378,  1115,   925,  1586,  1443,   719,   931,   465,
     728,   933,  1268,   711,   914,   917,  1104,  1698,  1699,   574,
     575,   599,   600,   281,   282,   288,  1412,  1565,  1566,  1190,
    1304,  1401,  1559,  1683,  1701,  1597,  1641,  1642,  1643,  1388,
    1389,  1390,  1391,  1599,  1605,  1652,  1394,  1395,  1399,  1552,
    1553,  1554,  1576,  1728,  1305,  1306,   183,   135,  1714,  1715,
    1557,  1308,  1309,  1310,  1311,   136,   233,   461,   462,   137,
     138,   139,   140,   141,   142,   143,   144,  1424,   145,   855,
    1060,   146,   237,   571,   319,   572,   573,   456,   562,   563,
    1138,   564,  1139,   147,   148,   149,   751,   150,   151,   268,
     152,   269,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   764,   765,   996,   499,   500,   501,   771,  1626,   153,
     558,  1414,   559,  1033,   826,  1207,  1204,  1545,  1546,   154,
     155,   156,   227,   234,   328,   448,   157,   952,   755,   158,
     953,   847,   840,   954,   901,  1083,   902,  1085,  1086,  1087,
     904,  1247,  1248,   905,   690,   432,   194,   195,   588,   577,
     413,   674,   675,   676,   677,   844,   160,   228,   185,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   636,   171,
     230,   231,   540,   219,   220,   639,   640,  1144,  1145,   303,
     304,   812,   172,   528,   173,   570,   174,  1567,   295,   336,
     594,   595,   946,  1043,  1188,   801,   802,   732,   733,   734,
     261,   262,   757,   263,   842
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1427
static const yytype_int16 yypact[] =
{
   -1427,   161, -1427, -1427,  4864, 12898, 12898,   -34, 12898, 12898,
   12898, 10838, 12898, -1427, 12898, 12898, 12898, 12898, 12898, 12898,
   12898, 12898, 12898, 12898, 12898, 12898, 15465, 15465, 11044, 12898,
   15516,   -27,   -25, -1427, -1427, -1427, -1427, -1427,   173, -1427,
   -1427,   149, 12898, -1427,   -25,   176,   188,   221, -1427,   -25,
   11250, 16654, 11456, -1427, 14540,  9808,    15, 12898, 16297,    36,
   -1427, -1427, -1427,   443,    25,    33,   226,   229,   262,   264,
   -1427, 16654,   268,   302, -1427, -1427, -1427, -1427, -1427,   464,
   15168, -1427, -1427, 16654, -1427, -1427, -1427, -1427, 16654, -1427,
   16654, -1427,   363,   348, 16654, 16654, -1427,   344, -1427, -1427,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427, 12898, -1427, -1427,   360,   536,   539,   539, -1427,
     535,   413,   258, -1427,   370, -1427,    64, -1427,   542, -1427,
   -1427, -1427, -1427, 16401,   439, -1427, -1427,   388,   396,   406,
     408,   418,   432, 13861, -1427, -1427, -1427, -1427,   505, -1427,
     583,   602,   468, -1427,    50,   469,   523, -1427, -1427,   960,
      62,  2038,   120,   475,    37, -1427,   124,   125,   477,   269,
   -1427,    52, -1427,   612, -1427, -1427, -1427,   531,   483,   532,
   -1427, -1427,   542,   439, 16929,  2203, 16929, 12898, 16929, 16929,
    9997,   488, 15903,  9997,   648, 16654,   630,   630,    94,   630,
     630,   630,   630,   630,   630,   630,   630,   630, -1427, -1427,
   15066,   527, -1427,   551,    28,   508,    28, 15465, 15947,   504,
     700, -1427,   531, 15120,   527,   569,   591,   543,   132, -1427,
      28,   120, 11662, -1427, -1427, 12898,  8366,   738,    76, 16929,
    9396, -1427, 12898, 12898, 16654, -1427, -1427, 13905,   553, -1427,
   13949, 14540, 14540,   584, -1427, -1427,   558, 14280,    61,   608,
     749, -1427,   622, 16654,   695, -1427,   574, 13993,   576,   554,
   -1427,    44, 14037, 16459, 16474, 16654,    77, -1427,    71, -1427,
   15218,    81, -1427,   654, -1427,   655, -1427,   764,    83, 15465,
   15465, 12898,   582,   613, -1427, -1427, 15316, 11044,    66,    40,
   -1427, 13104, 15465,   503, -1427, 16654, -1427,   337,   413, -1427,
   -1427, -1427, -1427, 15635,   772,   690, -1427, -1427, -1427,    42,
     593, 16929,   594,  1311,   597,  5070, 12898,   366,   598,   568,
     366,   415,   362, -1427, 16654, 14540,   599, 10014, 14540, -1427,
   -1427, 15615, -1427, -1427, -1427, -1427, -1427,   542, -1427, -1427,
   -1427, -1427, -1427, -1427, -1427, 12898, 12898, 12898, 11868, 12898,
   12898, 12898, 12898, 12898, 12898, 12898, 12898, 12898, 12898, 12898,
   12898, 12898, 12898, 12898, 12898, 12898, 12898, 12898, 12898, 12898,
   15516, 12898, -1427, 12898, 12898, -1427, 12898,  4121, 16654, 16654,
   16654, 16654, 16654, 16401,   696,   762,  9602, 12898, 12898, 12898,
   12898, 12898, 12898, 12898, 12898, 12898, 12898, 12898, 12898, -1427,
   -1427, -1427, -1427,  2279, 12898, 12898, -1427, 10014, 10014, 12898,
   12898,   360,   142, 15316,   605,   542, 12074, 14081, -1427, 12898,
   -1427,   618,   798, 15066,   621,    16,   597, 14809,    28, 12280,
   -1427, 12486, -1427,   623,    19, -1427,    59, 10014, -1427, 15676,
   -1427, -1427, 14125, -1427, -1427, 10220, -1427, 12898, -1427,   719,
    8572,   803,   617, 16840,   811,    70,    53, -1427, -1427, -1427,
   -1427, -1427, 14540, 16236,   627,   818, 14887, 16654, -1427,   643,
   -1427, -1427, -1427,   750, 12898,   753,   755, 12898, 12898, 12898,
   -1427,   554, -1427, -1427, -1427, -1427, -1427, -1427, -1427,   649,
   -1427, -1427, -1427,   637, -1427, -1427, 16654,   644,   829,    74,
   16654,   647,   840,    78,    80, 16520, -1427, 16654, 12898,    28,
      36, -1427, -1427, -1427, 14887,   771, -1427,    28,    73,    87,
     656,   661,  1462,   254,   664,   652,   495,   727,   666,    28,
      89,   667, 16416, 16654, -1427, -1427,   789,  3578,    11, -1427,
   -1427, -1427,   413, -1427, -1427, -1427,   838,   741,   701,    63,
   -1427,   360,   742,   863,   675,   734,   142, -1427, 14540, 14540,
     871,   738,    42, -1427,   689,   878, -1427, 14540,   276,   823,
     147, -1427, -1427, -1427, -1427, -1427, -1427, -1427,   881,  4224,
   -1427, -1427, -1427, -1427,   882,   722, -1427, 15465, 12898,   705,
     886, 16929,   876, -1427, -1427,   779, 15734, 10409, 11441,  9997,
   12898, 16885, 12675, 14257,  3073,  2988,  4070, 11846, 11846, 11846,
   11846,  1742,  1742,  1742,  1742,   718,   718,   607,   607,   607,
      94,    94,    94, -1427,   630, 16929,   706,   707, 16004,   704,
     902, -1427, 12898,   -15,   715,   142, -1427, -1427, -1427,   901,
     690, -1427,   542, 12898,  3237, -1427, -1427,  9997, -1427,  9997,
    9997,  9997,  9997,  9997,  9997,  9997,  9997,  9997,  9997,  9997,
    9997, -1427, 12898,   256,   143, -1427, -1427,   527,   299,   713,
    4335,   721,   723,   716, 13294,   111,   725, -1427, 16929, 15017,
   -1427, 16654, -1427,   593,   276,   527, 15465, 16929, 15465, 16048,
     276,   175, -1427,   726, 12898, -1427,   267, -1427, -1427, -1427,
    8160,   528, -1427, -1427, 16929, 16929,   -25, -1427, -1427, -1427,
   12898,   828, 14736, 14887, 16654,  8778,   729,   730, -1427,    79,
     801,   783, -1427,   925,   735, 14353, 14540, 14887, 14887, 14887,
   14887, 14887, -1427,   740,    84,   787,   746, 14887,     5, -1427,
     790, -1427,   744, -1427, 17015, -1427,   325, -1427, 12898,   765,
   16929,   767,   940, 10823,   946, -1427, 16929, 14211, -1427,   649,
     877, -1427,  5276, 16355,   758,   283, -1427, 16459, 16654,   285,
   -1427, 16474, 16654, 16654, -1427, -1427, 13338, -1427, 17015,   947,
   15465,   766, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427,    82, 16654, 16355,   763, 15316, 15414,   954, -1427, -1427,
   -1427, -1427,   769, -1427, 12898, -1427, -1427,  4442, -1427, 14540,
   16355,   768, -1427, -1427, -1427, -1427,   955, 12898, 15635, -1427,
   -1427, 16593,   770, -1427, 14540, -1427,   775,  5482,   943,   117,
   -1427, -1427,    65,  2279, -1427, -1427, 14540, -1427, -1427,    28,
   16929, -1427, 10426, -1427, 14887,    49,   777, 16355,   741, -1427,
   -1427, 12263, 12898, -1427, -1427, 12898, -1427, 12898, -1427, 13382,
     780, 10014,   727,   950,   741, 14540,   967,   779, 16654, 15516,
      28, 13426,   784, -1427, -1427,   270, -1427, -1427, -1427,   968,
    2433,  2433, 15017, -1427, -1427, -1427,   936,   791,   138,   792,
   -1427, -1427, -1427, -1427,   980,   794,   618,    28,    28, 12692,
   15676, -1427, -1427, 13470,   545,   -25,  9396, -1427,  5688,   802,
    5894,   804, 14736, 15465,   805,   883,    28, 17015,   988, -1427,
   -1427, -1427, -1427,   561, -1427,    32, 14540, -1427, 14540, 16654,
   16236, -1427, -1427, -1427,  1003, -1427,   816,   882,   530,   530,
     952,   952, 16149,   812,  1012, 14887,   879, 16654, 15635, 14169,
   16639, 14887, 14887, 14887, 14887, 14690, 14887, 14887, 14887, 14887,
   14887, 14887, 14887, 14887, 14887, 14887, 14887, 14887, 14887, 14887,
   14887, 14887, 14887, 14887, 14887, 14887, 14887, 14887, 16654, -1427,
   16929, 12898, 12898, 12898, -1427, -1427, -1427, 12898, 12898, -1427,
     554, -1427,   944, -1427, -1427, 16654, -1427, -1427, 16654, -1427,
   -1427, -1427, -1427, 14887,    28, -1427,   495, -1427,   476,  1015,
   -1427, -1427,   112,   830,    28, 10632, -1427,  3246, -1427,  4658,
     690,  1015, -1427,    24,     8, 16929,   896, -1427, -1427,   831,
     943, -1427, 14540,   738, 14540,    47,  1023,   959,   271, -1427,
     527, -1427, 15465, 12898, 16929, 17015,   839,    49, -1427,   841,
      49,   842, 12263, 16929, 16105,   843, 10014,   845,   844, 14540,
     847,   849, 14540,   741, -1427,   543,   361, 10014, 12898, -1427,
   -1427, -1427, -1427, -1427, -1427,   909,   848,  1044,   970, 15017,
     911, -1427, 15635, 15017, -1427, -1427, -1427, 15465, 16929,   274,
   -1427, -1427,   -25,  1029,   987,  9396, -1427, -1427, -1427,   864,
   12898,   883,    28, 15316, 14736,   866, 14887,  6100,   604,   867,
   12898,    99,   331, -1427,   895, -1427,   938, -1427, 14421,  1050,
     887, 14887, -1427, 14887, -1427,   888, -1427,   951,  1072,   889,
   -1427, -1427, -1427, 16206,   880,  1082, 11027, 11647, 12057, 14887,
   16973, 14757, 15701, 15807,  3678, 15081, 17045, 17045, 17045, 17045,
    2078,  2078,  2078,  2078,   851,   851,   530,   530,   530,   952,
     952,   952,   952, -1427, 16929, 13089, 16929, -1427, 16929, -1427,
     892, -1427, -1427, -1427, 17015, 16654, 14540, 14540, -1427, 16355,
      85, -1427, 15316, -1427, -1427,  9997,   890, -1427,   893,  1464,
   -1427,    67, 12898, -1427, -1427, -1427, 12898, -1427, 12898, -1427,
     738, -1427, -1427,   372,  1080,  1018, 12898, -1427,   899,    28,
   16929,   943,   900, -1427,   903,    49, 12898, 10014,   906, -1427,
   -1427,   690, -1427, -1427,   898,   905,   910, -1427,   908, 15017,
   -1427, 15017, -1427, -1427,   912, -1427,   979,   913,  1092, -1427,
      28,  1081, -1427,   915, -1427, -1427,   919,   922,   113, -1427,
   -1427, 17015,   926,   927, -1427, 13817, -1427, -1427, -1427, -1427,
   -1427, 14540, -1427, 14540, -1427, 17015, 16250, -1427, 14887, 15635,
   -1427, -1427, 14887, -1427, 14887, -1427, 12469, 14887, 12898,   924,
    6306,   476, -1427, -1427, -1427,   457, 14494, 16355,  1022, -1427,
    2453,   971, 15365, -1427, -1427, -1427,   696, 14234,    90,    93,
     931,   690,   762,   116, -1427, -1427, -1427,   978, 13514, 13558,
   16929, -1427,    56,  1120,  1056, 12898, -1427, 16929, 10014,  1025,
     943,  1595,   943,   942, 16929,   949, -1427,  1820,   945,  1834,
   -1427, -1427,    49, -1427, -1427,  1019, -1427, 15017, -1427, 15635,
   -1427, -1427,  8160, -1427, -1427, -1427, -1427,  8984, -1427, -1427,
   -1427,  8160, -1427,   957, 14887, 17015,  1021, 17015, 16307, 12469,
   12883, -1427, -1427, -1427, 16355, 16355, 16654,  1132,    58, -1427,
   14494,   690, 16282, -1427,   983, -1427,    96,   958,    97, -1427,
   13631, -1427, -1427, -1427,   100, -1427, -1427, 15266, -1427,   963,
   -1427,  1086,   542, -1427, 14540, -1427, 14540, -1427, -1427,  1145,
     696, -1427,  3029, -1427, -1427, -1427, -1427,  1147,  1087, 12898,
   -1427, 16929,   972,   969,   973,   434, -1427,  1025,   943, -1427,
   -1427, -1427, -1427,  1910,   974, 15017, -1427,  1046,  8160,  9190,
    8984, -1427, -1427, -1427,  8160, -1427, 17015, 14887, 14887, 12898,
    6512,   977,   982, -1427, 14887, 16355, -1427, -1427, -1427, -1427,
   -1427, 14540,   653,  2453, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427, -1427, -1427, -1427,   123, -1427,   971, -1427, -1427,
   -1427, -1427, -1427,    68,   502, -1427,  1166,   101, 16654,  1086,
    1170, -1427, 14540,   542, -1427, -1427,   985,  1173, 12898, -1427,
   16929, -1427,   424, -1427, -1427, -1427, -1427,   986,   434,  3823,
   -1427,   943, -1427, 15017, -1427, -1427, -1427, -1427,  6718, 17015,
   17015, 11235, -1427, -1427, -1427, 17015, -1427,  2611,    95,    55,
   -1427, -1427, 14887, 13631, 13631,  1134, -1427, 15266, 15266,   537,
   -1427, -1427, -1427, 14887,  1111, -1427,   992,   102, 14887, -1427,
   16654, -1427, 14887, 16929,  1114, -1427,  1185,  6924,  7130, -1427,
   -1427, -1427,   434, -1427,  7336,   994,  1117,  1088, -1427,  1102,
    1052, -1427, -1427,  1103, 14540, -1427,   653, -1427, 17015, -1427,
   -1427,  1042, -1427,  1172, -1427, -1427, -1427, -1427, 17015,  1192,
   -1427, -1427, 17015,  1010, 17015, -1427,   430,  1014, -1427, -1427,
    7542, -1427,  1011, -1427, -1427,  1016,  1048, 16654,   762,  1045,
   -1427, -1427, 14887,   115, -1427,  1139, -1427, -1427, -1427, -1427,
   16355,   758, -1427,  1054, 16654,   513, -1427, 17015,  1020,  1214,
     609,   115, -1427,  1146, -1427, 16355,  1028, -1427,   943,   118,
   -1427, -1427, -1427, -1427, 14540, -1427,  1026,  1030,   109, -1427,
     437,   609,   395,   943,  1031, -1427, -1427, -1427, -1427, 14540,
      57,  1217,  1157,   437, -1427,  7748,   411,  1223,  1159, 12898,
   -1427, -1427,  7954, -1427,    60,  1225,  1161, 12898, -1427, 16929,
   -1427,  1228,  1164, 12898, -1427, 16929, 12898, -1427, 16929, 16929
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1427, -1427, -1427,  -493, -1427, -1427, -1427,   253, -1427, -1427,
   -1427,   731,   467,   466,   447,  1443,  2793, -1427,  2525, -1427,
    -422, -1427,    35, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427, -1427, -1427,  -446, -1427, -1427,  -153,   110,    18,
   -1427, -1427, -1427, -1427, -1427, -1427,    23, -1427, -1427, -1427,
   -1427,    34, -1427, -1427,   855,   865,   861,   -96,   375,  -793,
     382,   435,  -449,   146,  -838, -1427,  -181, -1427, -1427, -1427,
   -1427,  -651,    -3, -1427, -1427, -1427, -1427,  -441, -1427,  -532,
   -1427,  -369, -1427, -1427,   747, -1427,  -167, -1427, -1427,  -924,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
    -195, -1427,  -113, -1427, -1427, -1427, -1427,  -277, -1427,   -31,
    -982, -1427, -1426,  -461, -1427,  -157,    91,  -126,  -448, -1427,
    -285, -1427, -1427, -1427,   -23,   -44,     2,  1234,  -661,  -366,
   -1427, -1427,   -12, -1427, -1427,    -5,   -38,  -149, -1427, -1427,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427,  -535,  -773, -1427,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,   884,
   -1427, -1427,   279, -1427,   793, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427,   288, -1427,   795, -1427, -1427,   518, -1427,   255,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,  -949, -1427,
    1955,  1862,  -327, -1427, -1427,   214,  2717,  3078, -1427, -1427,
     336,  -123,  -575, -1427, -1427,   401,   205,  -635,   206, -1427,
   -1427, -1427, -1427, -1427,   390, -1427, -1427, -1427,   119,  -813,
    -174,  -387,  -377, -1427,   451,  -104, -1427, -1427,   460, -1427,
   -1427,   312,   -43, -1427, -1427,    31,    75, -1427,  -241, -1427,
   -1427, -1427,  -379,  1001, -1427, -1427, -1427, -1427, -1427,   615,
     512, -1427, -1427,  1009,  -278,  -608, -1427,   -33,   -62,  -158,
     -71,   565, -1427,  -947,    10, -1427,   286,   364, -1427, -1427,
   -1427, -1427,   315,  1271, -1003
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -923
static const yytype_int16 yytable[] =
{
     184,   186,   394,   188,   189,   190,   192,   193,   348,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   260,   129,   218,   221,   853,  1044,   131,   311,   424,
     655,   710,   314,   315,   236,   567,   835,   239,   132,   127,
     836,  1213,   876,   266,   685,   247,   241,   250,   681,   682,
     267,   245,   272,   633,   903,  1036,   449,   348,   416,   921,
     817,   229,   706,   393,  1646,  1059,   445,  1455,   320,   324,
    -891,   344,   707,   338,   322,  -891,   935,  1607,   703,   724,
     772,  1070,   790,   726,  1111,   457,   515,    13,   936,  1295,
     520,  1016,   525,  1210,    13,   134,   790,   442,   806,  1404,
    1045,  1608,  1406,  -490,  1644,  -291,  1463,   321,  1199,  1547,
    1614,  1614,   287,   823,   128,   279,  -788,  1317,  1455,   298,
     806,   806,   806,   159,   543,   806,  1214,   450,  -791,  1266,
      13,   208,   542,  1120,  1121,  1417,  1737,  1602,   208,  1751,
     411,   412,   285,   637,  1046,   411,   412,   380,   411,   412,
     286,   837,  1630,  1603,   503,  -896,   335,   411,   412,   381,
      13,     3,    13,    13,  1205,   187,   434,   436,   517,   956,
    1604,   778,   232,   679,   235,   782,   335,   783,   683,   443,
    1200,  -458,   427,  -631,  -896,  1137,   414,   544,   301,   302,
    -789,  -790,  1296,  1201,   606,   411,   412,  1297,  -825,    60,
      61,    62,   175,  1298,   345,  1299,  1671,  -896,   419,  -792,
    1206,  1215,   504,  -795,   816,  -491,   580,   270,   824,   701,
    1418,  1738,   312,  1090,  1752,   334,  1202,   452,   334,  1047,
     452,   300,   334,   825,   334,  1123,   418,   239,   463,  -230,
     280,  -828,  1300,  1301,   347,  1302,   422,   530,   534,  -230,
     395,   937,  -788,   531,  1017,   727,   421,   120,  1647,  1456,
    1457,  -797,  -891,   115,  -791,   339,   346,  1645,  1609,   348,
     725,   454,   686,   791,  1329,   459,  1259,   458,   516,   604,
    1235,  1118,   521,  1122,   526,  1091,   321,   792,  1303,   807,
     918,  1405,   218,   425,  1407,   920,   547,  -291,  1464,  1331,
    1267,  1548,  1615,  1661,   248,   602,  1337,   258,  1339,   692,
    1725,   889,  1191,  1356,  -712,  -214,  1411,  -712,  -712,  1246,
    -490,   589,   415,  1056,   293,  1029,  -789,  -790,   311,   649,
     650,   344,   601,  -826,  -825,   419,  -794,  -793,   215,   215,
    -827,   309,   414,   293,   420,  -792,  -798,   293,   293,  -795,
     607,   608,   609,   611,   612,   613,   614,   615,   616,   617,
     618,   619,   620,   621,   622,   623,   624,   625,   626,   627,
     628,   629,   630,   631,   632,   242,   634,  -828,   635,   635,
    1005,   638,  1008,  1425,   656,  1427,   293,   243,   566,   693,
     436,   657,   659,   660,   661,   662,   663,   664,   665,   666,
     667,   668,   669,   670,   843,   411,   412,  1322,  -896,   635,
     680,   229,   601,   601,   635,   684,   134,  1433,   644,  -632,
     244,   657,  1198,   316,   688,   289,  1022,  -767,   290,   335,
    1730,   394,  1120,  1121,   697,   128,   699,   334,   576,   334,
     829,   298,   601,   713,   678,   298,  1744,   549,   431,  1256,
     714,  1323,   715,   418,   796,  -767,  1048,  -896,  1249,  1624,
    -896,   291,   645,   292,   161,  1685,  1049,   296,   644,  -826,
    -770,   420,  -794,  -793,  1731,   988,  -827,   702,   415,   760,
     708,  1578,   763,   766,   767,   870,   214,   216,   645,   120,
    1745,   334,   393,   120,   298,   718,   335,   464,  -770,   878,
     549,   297,  1067,  1625,   652,   278,  1366,  1117,  1212,  1686,
     301,   302,   645,   786,   301,   302,   478,   868,   312,   882,
     843,   645,   215,  1099,   645,  1222,   910,   307,  1224,   215,
     308,  1610,  -768,  1100,  1269,   215,  1324,   870,   554,   555,
     317,   799,   800,   298,   860,    53,   318,   313,  1611,   299,
     567,  1612,  1075,    60,    61,    62,   175,   176,   345,  1732,
    -768,   283,   326,   301,   302,   333,  1655,   334,   284,   337,
     592,  -488,   323,  1186,  1187,  1746,  1437,   340,   120,   984,
     985,   986,   298,  1656,  1374,  1375,  1657,   593,   549,   349,
     445,   258,   215,   850,   293,   987,   911,   350,   989,   915,
     916,   215,   215,   533,  1344,   861,  1345,   351,   215,   352,
     941,   300,   301,   302,   215,   298,  1102,  1103,   298,   353,
     346,   327,   591,  1338,   330,  1653,  1654,    60,    61,    62,
     175,   176,   345,   354,  1632,  1574,  1575,   869,  1726,  1727,
     643,   293,   647,   293,   293,   293,   293,   298,   192,   383,
     550,   301,   302,   549,  1649,  1650,   377,   378,   379,  1193,
     380,  1119,  1120,  1121,   944,   947,   673,   881,   384,   385,
     435,   386,   381,   387,   417,  1321,  -796,   438,  -489,   395,
     567,  -631,   423,   444,   301,   302,   305,   301,   302,   428,
     695,   576,   430,  1333,   346,   381,   161,  1228,   335,   913,
     161,   437,   705,  1409,  1263,  1120,  1121,   418,  1238,   441,
     440,  1722,  1436,   120,  1707,   239,   301,   302,   919,  -630,
     509,   513,   514,  1711,  1712,  1713,  1736,    33,    34,    35,
     756,   329,   331,   332,  1258,   215,   483,   484,   485,   742,
     519,   446,   447,   486,   487,   215,   455,   488,   489,   527,
     527,   532,   552,   990,   468,   472,   539,   473,  -922,  1290,
     930,  1720,   548,   476,   374,   375,   376,   377,   378,   379,
     785,   380,   477,  1459,   479,   480,  1733,   482,   524,   522,
     523,  1094,   535,   381,   536,   161,   568,   569,    74,    75,
      76,    77,    78,   578,   579,   811,   813,   581,   -66,   744,
    1582,   134,    53,   590,   603,    81,    82,   691,   716,  1027,
    1434,   545,   457,  1313,   567,   551,  1074,   689,   720,    91,
     128,   694,  1035,   700,  1130,   723,   735,   736,   758,   759,
    1352,  1134,   761,    96,   762,   129,   773,   770,   777,   545,
     131,   551,   545,   551,   551,   776,  1361,  1054,   780,   781,
     789,   132,   127,   798,   803,   814,   793,  1062,  1335,   293,
    1063,   794,  1064,   134,   797,   805,   601,   808,   819,   820,
    1700,   822,   828,   827,   678,   830,    60,    61,    62,   175,
     176,   345,   128,   539,   831,   834,   838,   839,  1700,  -492,
     854,   846,  1218,   435,   848,   852,  1721,   981,   982,   983,
     984,   985,   986,   566,  1098,   851,   857,   866,   134,   215,
     229,   867,   863,   864,   871,   875,   987,  1105,   645,   883,
     161,   885,   887,   886,   859,   922,   912,   128,   134,  1438,
     932,   934,   938,   939,   940,   942,   159,   957,  1444,   955,
     960,   708,   900,   346,   906,   958,   961,   128,  1633,  1450,
     991,  1106,   992,   775,   993,   997,  1000,   779,   576,  1422,
    1003,  1013,   278,   120,  1242,  1021,   215,  1015,  1025,  1034,
    1032,  1038,  1040,  1042,   576,  1026,  1057,   928,   120,  1066,
    1069,  1072,  1078,  1077,  1088,   645,  1174,  1175,  1176,  1093,
    1089,  1092,   763,  1178,  1095,    60,    61,    62,    63,    64,
     345,   567,  1116,  1108,  1113,  1110,    70,   388,   215,   134,
     215,   134,  1128,  1194,  1114,  1280,  1129,   987,  1132,  1588,
    1195,  1133,  1285,  1180,  1189,   120,   544,  1208,   128,  1192,
     128,  1007,  1209,   566,   215,  1010,  1011,  1216,  1217,  1221,
    1239,  1225,  1227,  1223,   390,  1229,  1230,   129,  1220,  1232,
    1233,  1667,   131,  1241,  1240,  1018,   896,   849,  1245,  1252,
    1253,   601,   346,   132,   127,  1255,  1260,  1270,  1264,  1271,
     120,   567,   601,  1195,    60,    61,    62,    63,    64,   345,
    1273,  1279,  1278,  1283,  1037,    70,   388,  1274,  1277,  1281,
     120,  1284,  1289,  1314,  1325,  1315,   673,  1326,  1328,  1340,
    1330,  1349,   215,  1332,  1251,   239,  1336,  1341,  1343,  1342,
    1347,  1351,  1346,  1348,   880,  1265,  1353,   215,   215,  1354,
     134,   389,  1355,   390,  1710,  1350,  1371,  1358,  1359,  1380,
    1393,   293,  1408,  1413,  1419,  1420,   391,  1423,   392,   128,
    1254,   346,  1428,  1082,  1082,   900,  1454,  1431,   159,  1429,
    1435,  1627,  1447,  1628,  1461,  1410,   907,  1445,   908,  1562,
    1462,  1568,  1634,   705,  1555,  1556,  1569,   566,  1572,   120,
     161,   120,  1571,   120,  1581,  1573,   576,  1583,  1593,   576,
    1613,   348,   926,  1594,  1618,   161,  1621,  1622,  1629,  1651,
    1659,  1660,  1126,  1665,  1666,  1673,  1674,  1318,  1675,  -287,
    1678,  1319,  1677,  1320,  1681,  1608,  1682,  1670,   134,  1684,
    1136,  1327,  1689,  1142,  1687,  1690,  1691,  1696,  1702,  1705,
    1708,  1334,   601,  1709,   509,  1717,  1723,   128,   513,  1719,
    1724,  1739,   161,  1734,   215,   215,  1740,  1747,  1748,  1753,
    1754,   756,  1756,  1757,  1006,  1704,   784,  1009,   651,  1558,
    1014,   648,  1073,   646,  1068,  1031,  1718,  1257,  1182,  1587,
    1716,  1183,  1360,  1377,  1579,   539,  1024,   787,  1601,  1460,
    1606,  1400,  1741,  1729,  1617,  1381,   238,   161,  1577,  1179,
     658,  1307,   120,  1370,   768,  1177,   769,   999,  1203,  1237,
    1307,  1135,  1084,  1243,  1050,  1244,  1096,   161,   541,   529,
     945,  1373,  1185,  1173,  1127,  1735,     0,     0,     0,  1312,
       0,     0,  1742,     0,  1453,     0,     0,     0,  1312,     0,
    1421,     0,     0,   601,     0,   426,   397,   398,   399,   400,
     401,   402,   403,   404,   405,   406,   407,   408,     0,     0,
       0,     0,   900,     0,   576,     0,   900,  1458,     0,     0,
       0,     0,     0,     0,   566,     0,     0,     0,   120,     0,
       0,     0,     0,     0,   215,     0,     0,     0,     0,     0,
     120,  1560,     0,  1561,   409,   410,   161,     0,   161,     0,
     161,   134,   926,  1112,     0,     0,     0,     0,     0,     0,
       0,     0,  1442,     0,     0,     0,     0,     0,     0,     0,
     128,     0,     0,   395,     0,     0,     0,     0,     0,   215,
    1620,     0,     0,     0,  1570,     0,  1402,     0,     0,     0,
       0,     0,  1307,     0,   566,   215,   215,     0,  1307,     0,
    1307,     0,     0,     0,     0,     0,     0,     0,  1291,     0,
     411,   412,     0,   134,  1591,     0,     0,     0,     0,     0,
    1312,     0,   134,     0,     0,     0,  1312,     0,  1312,     0,
       0,   576,   128,     0,     0,     0,     0,     0,  1295,   211,
     211,   128,     0,   224,  1585,  1442,   426,   397,   398,   399,
     400,   401,   402,   403,   404,   405,   406,   407,   408,   161,
       0,     0,   900,     0,   900,     0,  1616,   224,     0,     0,
       0,     0,     0,     0,   215,     0,     0,     0,     0,    13,
       0,   580,  1219,     0,     0,     0,     0,     0,     0,     0,
    1563,  1694,   470,   471,  1307,   409,   410,     0,   475,   134,
       0,     0,     0,     0,     0,   134,     0,     0,     0,     0,
       0,   134,     0,   120,     0,     0,     0,     0,   128,   258,
       0,     0,  1312,     0,   128,  1398,     0,  1250,  1663,     0,
     128,     0,     0,  1623,     0,   161,     0,     0,     0,   348,
       0,  1296,     0,   539,   926,     0,  1297,   161,    60,    61,
      62,   175,  1298,   345,  1299,     0,     0,     0,     0,     0,
       0,   411,   412,     0,     0,     0,     0,     0,     0,  1295,
     900,     0,     0,     0,     0,   120,   596,     0,     0,     0,
     120,     0,     0,     0,   120,     0,     0,     0,     0,     0,
       0,  1300,  1301,     0,  1302,     0,     0,     0,     0,   293,
       0,     0,     0,   258,     0,     0,     0,     0,     0,     0,
      13,     0,     0,  1544,     0,   346,     0,     0,     0,     0,
    1551,     0,   539,   211,     0,     0,     0,   258,     0,   258,
     211,     0,   795,     0,     0,   258,   211,  1316,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   134,
       0,     0,     0,     0,     0,     0,     0,     0,   900,     0,
       0,   120,   120,   120,   224,   224,     0,   120,   128,     0,
     224,     0,  1296,   120,     0,     0,     0,  1297,     0,    60,
      61,    62,   175,  1298,   345,  1299,     0,     0,   134,   134,
       0,     0,     0,   211,     0,   134,     0,     0,     0,     0,
       0,     0,   211,   211,  1749,     0,     0,   128,   128,   211,
       0,     0,  1755,   729,   128,   211,     0,     0,  1758,     0,
     161,  1759,  1300,  1301,     0,  1302,   224,     0,     0,     0,
       0,   134,     0,     0,     0,     0,     0,     0,     0,  1695,
       0,     0,     0,     0,     0,     0,   346,     0,   224,     0,
     128,   224,  -923,  -923,  -923,  -923,   372,   373,   374,   375,
     376,   377,   378,   379,     0,   380,     0,     0,  1426,     0,
       0,     0,   576,     0,     0,     0,     0,   381,     0,     0,
       0,   293,   161,     0,     0,     0,     0,   161,     0,     0,
     576,   161,     0,   224,  1295,     0,   134,     0,   576,     0,
       0,     0,   258,   134,     0,     0,   900,     0,  1295,   832,
     833,   120,     0,     0,     0,   128,     0,     0,   841,     0,
    1639,     0,   128,     0,     0,     0,  1544,  1544,     0,     0,
    1551,  1551,     0,     0,     0,    13,   211,     0,     0,     0,
       0,     0,     0,   293,     0,     0,   211,     0,     0,    13,
     120,   120,     0,     0,     0,     0,     0,   120,   213,   213,
       0,     0,   226,     0,     0,     0,     0,     0,   161,   161,
     161,     0,     0,     0,   161,     0,     0,     0,     0,     0,
     161,     0,     0,     0,  1295,   224,   224,     0,     0,   748,
       0,     0,     0,   120,     0,     0,     0,  1296,     0,     0,
    1693,     0,  1297,     0,    60,    61,    62,   175,  1298,   345,
    1299,  1296,     0,     0,     0,     0,  1297,  1706,    60,    61,
      62,   175,  1298,   345,  1299,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   748,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1300,  1301,     0,
    1302,   212,   212,     0,     0,   225,     0,     0,   120,     0,
       0,  1300,  1301,     0,  1302,   120,     0,     0,     0,     0,
       0,   346,     0,     0,     0,     0,   596,   596,     0,     0,
       0,   224,   224,     0,     0,   346,     0,  1296,     0,     0,
     224,     0,  1297,  1430,    60,    61,    62,   175,  1298,   345,
    1299,     0,     0,     0,     0,     0,     0,  1432,     0,     0,
     211,     0,     0,     0,     0,     0,     0,     0,   161,     0,
       0,     0,   396,   397,   398,   399,   400,   401,   402,   403,
     404,   405,   406,   407,   408,     0,     0,  1300,  1301,     0,
    1302,     0,   213,     0,     0,     0,     0,     0,     0,   213,
       0,     0,     0,     0,     0,   213,     0,   161,   161,     0,
    1030,   346,     0,     0,   161,     0,     0,   211,     0,     0,
       0,   409,   410,     0,     0,  1039,     0,     0,     0,     0,
       0,     0,     0,  1580,     0,     0,     0,  1051,  -923,  -923,
    -923,  -923,   979,   980,   981,   982,   983,   984,   985,   986,
     161,     0,     0,     0,     0,     0,     0,     0,     0,   211,
       0,   211,   213,   987,     0,     0,  1071,     0,     0,     0,
       0,   213,   213,     0,     0,     0,     0,     0,   213,     0,
       0,     0,     0,     0,   213,   211,   748,   411,   412,     0,
       0,     0,   212,     0,     0,   565,     0,     0,   224,   224,
     748,   748,   748,   748,   748,     0,     0,     0,     0,     0,
     748,     0,     0,     0,     0,   161,     0,     0,     0,     0,
       0,     0,   161,     0,     0,     0,     0,  1124,     0,  1125,
       0,     0,     0,     0,     0,     0,   224,   426,   397,   398,
     399,   400,   401,   402,   403,   404,   405,   406,   407,   408,
       0,     0,     0,   211,     0,   212,     0,     0,     0,     0,
       0,     0,   226,     0,   212,   212,   224,     0,   211,   211,
       0,   212,     0,     0,     0,     0,     0,   212,     0,     0,
       0,     0,   224,   224,     0,     0,   409,   410,   212,     0,
       0,   224,     0,     0,     0,     0,     0,   224,     0,     0,
       0,     0,     0,     0,     0,   213,     0,     0,     0,   224,
       0,     0,     0,     0,     0,   213,     0,   748,     0,     0,
     224,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1211,     0,   841,     0,     0,   224,     0,
       0,     0,   224,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   411,   412,     0,   225,     0,     0,     0,     0,
    1231,     0,     0,  1234,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,   208,     0,
       0,     0,     0,     0,     0,   211,   211,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,   212,   224,
       0,   224,     0,   224,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   748,  1051,
       0,   224,     0,     0,   748,   748,   748,   748,   748,   748,
     748,   748,   748,   748,   748,   748,   748,   748,   748,   748,
     748,   748,   748,   748,   748,   748,   748,   748,   748,   748,
     748,   752,     0,     0,   671,     0,    85,    86,     0,    87,
     180,    89,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   748,  1292,  1293,   213,
       0,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,     0,     0,   752,
       0,   672,  1382,   115,     0,   224,     0,   224,     0,     0,
       0,     0,     0,     0,     0,   211,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1079,  1080,  1081,
      36,     0,   224,     0,     0,   224,   213,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      36,    48,     0,     0,     0,   224,     0,     0,     0,     0,
     211,     0,  1362,     0,  1363,     0,     0,     0,     0,     0,
       0,    48,   212,     0,     0,     0,   211,   211,   213,   748,
     213,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   224,     0,  1383,   748,     0,   748,     0,  1403,   259,
       0,     0,     0,     0,   213,     0,  1384,  1385,     0,     0,
      85,    86,   748,    87,   180,    89,     0,     0,     0,     0,
       0,     0,     0,     0,   179,     0,     0,    83,  1386,   212,
      85,    86,     0,    87,  1387,    89,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   224,
     224,     0,   224,     0,     0,   211,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   212,   213,   212,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   213,   213,     0,
       0,     0,     0,     0,     0,     0,     0,   212,   752,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,     0,
     565,     0,   752,   752,   752,   752,   752,     0,     0,     0,
       0,     0,   752,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,   224,     0,   224,     0,     0,     0,
       0,   748,   224,     0,     0,   748,     0,   748,  1002,     0,
     748,     0,  1598,     0,     0,     0,     0,     0,     0,   224,
     224,   226,     0,   224,     0,   212,     0,     0,     0,     0,
     224,     0,     0,     0,     0,     0,     0,     0,  1020,     0,
     212,   212,   179,     0,     0,    83,     0,     0,    85,    86,
       0,    87,   180,    89,     0,  1020,   259,   259,     0,     0,
       0,     0,   259,   212,   213,   213,     0,     0,     0,     0,
       0,     0,   224,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   748,     0,   752,
       0,     0,  1058,     0,  1638,     0,     0,   224,   224,     0,
     565,     0,     0,   224,     0,   224,     0,     0,     0,     0,
       0,     0,     0,  1619,   225,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   224,     0,   224,
       0,     0,     0,     0,     0,   224,     0,     0,     0,     0,
     259,     0,     0,   259,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   212,   212,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     748,   748,     0,     0,     0,     0,     0,   748,   224,     0,
       0,     0,     0,     0,   224,     0,   224,     0,     0,     0,
     752,     0,     0,   212,   213,  1679,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   565,     0,     0,     0,     0,   213,
       0,     0,     0,     0,     0,     0,     0,     0,   752,     0,
       0,     0,     0,     0,     0,   213,   213,     0,     0,     0,
       0,     0,     0,     0,     0,   841,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   259,   731,     0,
     841,   750,     0,     0,     0,   224,     0,   212,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   224,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     224,   380,     0,     0,     0,   748,     0,   212,     0,   750,
       0,     0,   212,   381,   213,     0,   748,     0,   251,     0,
       0,   748,     0,     0,     0,   748,     0,     0,   212,   212,
       0,   752,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   252,     0,   752,   224,   752,     0,
       0,     0,     0,   259,   259,     0,     0,     0,     0,     0,
       0,     0,   259,     0,   752,     0,    36,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   748,   380,    48,     0,     0,
       0,     0,     0,   224,     0,     0,     0,     0,   381,     0,
       0,   565,     0,     0,  1294,     0,     0,   212,   224,     0,
       0,     0,     0,     0,     0,     0,     0,   224,     0,     0,
       0,     0,   253,   254,     0,     0,     0,     0,     0,     0,
       0,     0,   224,     0,     0,     0,     0,     0,     0,     0,
     179,     0,     0,    83,   255,     0,    85,    86,     0,    87,
     180,    89,     0,   753,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   256,     0,     0,     0,     0,     0,
       0,   565,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,     0,   257,     0,
       0,     0,  1564,   752,   212,     0,     0,   752,     0,   752,
       0,   753,   752,     0,     0,     0,     0,     0,   750,     0,
       0,     0,  1379,     0,     0,  1392,   355,   356,   357,     0,
     259,   259,   750,   750,   750,   750,   750,     0,     0,   749,
       0,     0,   750,     0,     0,   358,     0,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,     0,   380,
       0,     0,     0,     0,   212,   879,     0,     0,     0,     0,
       0,   381,     0,     0,    36,     0,   208,   749,     0,   752,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1451,
    1452,     0,     0,     0,     0,    48,     0,  1392,     0,     0,
       0,     0,     0,     0,   259,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   209,     0,     0,   259,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   259,     0,     0,     0,     0,     0,     0,     0,   750,
       0,     0,     0,     0,     0,     0,     0,     0,   179,     0,
       0,    83,    84,     0,    85,    86,     0,    87,   180,    89,
     259,     0,   752,   752,     0,     0,     0,     0,     0,   752,
    1596,     0,     0,     0,     0,     0,     0,     0,  1392,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,     0,   210,     0,     0,     0,
     753,   115,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1196,     0,   753,   753,   753,   753,   753,     0,
       0,   259,     0,   259,   753,   731,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     750,     0,     0,     0,     0,     0,   750,   750,   750,   750,
     750,   750,   750,   750,   750,   750,   750,   750,   750,   750,
     750,   750,   750,   750,   750,   750,   750,   750,   750,   750,
     750,   750,   750,     0,     0,     0,   749,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     749,   749,   749,   749,   749,     0,     0,     0,   750,     0,
     749,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   754,     0,     0,   752,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   259,   752,   259,
       0,   753,     0,   752,     0,     0,     0,   752,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   355,   356,
     357,     0,     0,     0,   259,     0,     0,   259,     0,     0,
       0,     0,   788,     0,     0,     0,     0,   358,     0,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
       0,   380,     0,     0,     0,     0,     0,   752,     0,     0,
       0,   750,     0,   381,     0,  1703,     0,   749,     0,     0,
       0,     0,     0,   259,     0,     0,   750,     0,   750,     0,
    1379,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   753,     0,   750,     0,     0,     0,   753,   753,
     753,   753,   753,   753,   753,   753,   753,   753,   753,   753,
     753,   753,   753,   753,   753,   753,   753,   753,   753,   753,
     753,   753,   753,   753,   753,     0,     0,     0,     0,     0,
       0,   259,   259,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,   982,   983,   984,   985,   986,
     753,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   987,     0,     0,     0,     0,   749,     0,
       0,     0,     0,     0,   749,   749,   749,   749,   749,   749,
     749,   749,   749,   749,   749,   749,   749,   749,   749,   749,
     749,   749,   749,   749,   749,   749,   749,   749,   749,   749,
     749,   815,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   259,     0,   259,     0,
       0,   927,     0,   750,     0,     0,   749,   750,     0,   750,
       0,     0,   750,     0,     0,   948,   949,   950,   951,     0,
       0,   259,     0,     0,     0,   959,     0,     0,     0,     0,
       0,     0,   259,   753,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   753,     0,
     753,     0,   251,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   753,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   252,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   750,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      36,     0,     0,     0,     0,   259,     0,     0,     0,   749,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,   749,     0,   749,     0,     0,   259,
       0,   259,  1055,     0,     0,     0,     0,   259,     0,     0,
       0,     0,   749,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   253,   254,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   750,   750,   179,     0,     0,    83,   255,   750,
      85,    86,     0,    87,   180,    89,   259,     0,     0,     0,
       0,     0,     0,     0,     0,   753,     0,     0,   256,   753,
       0,   753,     0,     0,   753,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,     0,   257,     0,     0,     0,  1631,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1143,
    1146,  1147,  1148,  1150,  1151,  1152,  1153,  1154,  1155,  1156,
    1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
    1167,  1168,  1169,  1170,  1171,  1172,     0,     0,     0,     0,
       0,   749,     0,     0,     0,   749,     0,   749,     0,     0,
     749,   753,     0,     0,     0,     0,     0,   259,     0,     0,
       0,  1184,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   259,     0,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,  1640,   380,     0,     0,     0,   750,     0,     0,
       0,     0,     0,     0,     0,   381,     0,     0,   750,     0,
       0,     0,     0,   750,     0,     0,     0,   750,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   749,     0,     0,
       0,     0,     0,     0,   753,   753,     0,     0,     0,   259,
       0,   753,     0,     0,     0,     0,     0,     0,     0,  1600,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1261,     0,     0,     0,    36,     0,
     208,     0,     0,     0,     0,     0,     0,   750,     0,  1275,
       0,  1276,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,  1286,     0,     0,
       0,     0,     0,     0,   355,   356,   357,     0,     0,   259,
     749,   749,     0,     0,     0,     0,   641,   749,     0,     0,
       0,     0,     0,   358,   259,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,     0,   380,    85,    86,
       0,    87,   180,    89,     0,     0,     0,     0,     0,   381,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,     0,     0,   753,
       0,     0,     0,   642,     0,   115,     0,     0,     0,     0,
     753,     0,     0,     0,     0,   753,     0,     0,     0,   753,
       0,     0,     0,     0,     0,   355,   356,   357,     0,     0,
       0,     0,     0,     0,     0,     0,  1365,     0,     0,     0,
    1367,     0,  1368,  1680,   358,  1369,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,     0,   380,     0,
       0,     0,     0,     0,     0,   749,     0,     0,     0,   753,
     381,     0,     0,     0,     0,     0,   749,     0,     0,     0,
       0,   749,     0,     0,     0,   749,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   845,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1446,     0,     0,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,   749,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,  1589,  1590,     0,    41,    42,
      43,    44,  1595,    45,     0,    46,     0,    47,   884,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,    54,
      55,    56,     0,    57,    58,    59,    60,    61,    62,    63,
      64,    65,     0,    66,    67,    68,    69,    70,    71,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    79,     0,
       0,     0,     0,    80,    81,    82,    83,    84,     0,    85,
      86,     0,    87,    88,    89,    90,     0,     0,    91,     0,
       0,    92,     0,     0,     0,     0,     0,    93,    94,     0,
      95,     0,    96,    97,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,   112,     0,   113,   114,  1028,   115,   116,     0,   117,
     118,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
    1648,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,  1658,     0,     0,     0,     0,  1662,     0,     0,     0,
    1664,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
    1697,    50,    51,    52,    53,    54,    55,    56,     0,    57,
      58,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,    69,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    79,     0,     0,     0,     0,    80,
      81,    82,    83,    84,     0,    85,    86,     0,    87,    88,
      89,    90,     0,     0,    91,     0,     0,    92,     0,     0,
       0,     0,     0,    93,    94,     0,    95,     0,    96,    97,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,   112,     0,   113,
     114,  1197,   115,   116,     0,   117,   118,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,     0,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,    54,    55,    56,     0,    57,    58,    59,    60,    61,
      62,    63,    64,    65,     0,    66,    67,    68,    69,    70,
      71,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      79,     0,     0,     0,     0,    80,    81,    82,    83,    84,
       0,    85,    86,     0,    87,    88,    89,    90,     0,     0,
      91,     0,     0,    92,     0,     0,     0,     0,     0,    93,
      94,     0,    95,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,   113,   114,     0,   115,   116,
       0,   117,   118,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,    63,    64,    65,
       0,    66,    67,    68,     0,    70,    71,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    79,     0,     0,     0,
       0,   179,    81,    82,    83,    84,     0,    85,    86,     0,
      87,   180,    89,    90,     0,     0,    91,     0,     0,    92,
       0,     0,     0,     0,     0,    93,     0,     0,     0,     0,
      96,    97,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,   112,
       0,   113,   114,   582,   115,   116,     0,   117,   118,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,    63,    64,    65,     0,    66,    67,    68,
       0,    70,    71,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    79,     0,     0,     0,     0,   179,    81,    82,
      83,    84,     0,    85,    86,     0,    87,   180,    89,    90,
       0,     0,    91,     0,     0,    92,     0,     0,     0,     0,
       0,    93,     0,     0,     0,     0,    96,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,     0,   113,   114,  1001,
     115,   116,     0,   117,   118,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,     0,
      55,    56,     0,    57,     0,    59,    60,    61,    62,    63,
      64,    65,     0,    66,    67,    68,     0,    70,    71,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    79,     0,
       0,     0,     0,   179,    81,    82,    83,    84,     0,    85,
      86,     0,    87,   180,    89,    90,     0,     0,    91,     0,
       0,    92,     0,     0,     0,     0,     0,    93,     0,     0,
       0,     0,    96,    97,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,   112,     0,   113,   114,  1041,   115,   116,     0,   117,
     118,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,     0,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    79,     0,     0,     0,     0,   179,
      81,    82,    83,    84,     0,    85,    86,     0,    87,   180,
      89,    90,     0,     0,    91,     0,     0,    92,     0,     0,
       0,     0,     0,    93,     0,     0,     0,     0,    96,    97,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,   112,     0,   113,
     114,  1107,   115,   116,     0,   117,   118,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,  1109,    45,     0,    46,     0,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,     0,    55,    56,     0,    57,     0,    59,    60,    61,
      62,    63,    64,    65,     0,    66,    67,    68,     0,    70,
      71,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      79,     0,     0,     0,     0,   179,    81,    82,    83,    84,
       0,    85,    86,     0,    87,   180,    89,    90,     0,     0,
      91,     0,     0,    92,     0,     0,     0,     0,     0,    93,
       0,     0,     0,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,   113,   114,     0,   115,   116,
       0,   117,   118,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,  1262,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,    63,    64,    65,
       0,    66,    67,    68,     0,    70,    71,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    79,     0,     0,     0,
       0,   179,    81,    82,    83,    84,     0,    85,    86,     0,
      87,   180,    89,    90,     0,     0,    91,     0,     0,    92,
       0,     0,     0,     0,     0,    93,     0,     0,     0,     0,
      96,    97,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,   112,
       0,   113,   114,     0,   115,   116,     0,   117,   118,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,    63,    64,    65,     0,    66,    67,    68,
       0,    70,    71,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    79,     0,     0,     0,     0,   179,    81,    82,
      83,    84,     0,    85,    86,     0,    87,   180,    89,    90,
       0,     0,    91,     0,     0,    92,     0,     0,     0,     0,
       0,    93,     0,     0,     0,     0,    96,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,     0,   113,   114,  1372,
     115,   116,     0,   117,   118,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,     0,
      55,    56,     0,    57,     0,    59,    60,    61,    62,    63,
      64,    65,     0,    66,    67,    68,     0,    70,    71,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    79,     0,
       0,     0,     0,   179,    81,    82,    83,    84,     0,    85,
      86,     0,    87,   180,    89,    90,     0,     0,    91,     0,
       0,    92,     0,     0,     0,     0,     0,    93,     0,     0,
       0,     0,    96,    97,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,   112,     0,   113,   114,  1592,   115,   116,     0,   117,
     118,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,  1635,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,     0,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    79,     0,     0,     0,     0,   179,
      81,    82,    83,    84,     0,    85,    86,     0,    87,   180,
      89,    90,     0,     0,    91,     0,     0,    92,     0,     0,
       0,     0,     0,    93,     0,     0,     0,     0,    96,    97,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,   112,     0,   113,
     114,     0,   115,   116,     0,   117,   118,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,     0,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,     0,    55,    56,     0,    57,     0,    59,    60,    61,
      62,    63,    64,    65,     0,    66,    67,    68,     0,    70,
      71,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      79,     0,     0,     0,     0,   179,    81,    82,    83,    84,
       0,    85,    86,     0,    87,   180,    89,    90,     0,     0,
      91,     0,     0,    92,     0,     0,     0,     0,     0,    93,
       0,     0,     0,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,   113,   114,  1668,   115,   116,
       0,   117,   118,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,    63,    64,    65,
       0,    66,    67,    68,     0,    70,    71,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    79,     0,     0,     0,
       0,   179,    81,    82,    83,    84,     0,    85,    86,     0,
      87,   180,    89,    90,     0,     0,    91,     0,     0,    92,
       0,     0,     0,     0,     0,    93,     0,     0,     0,     0,
      96,    97,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,   112,
       0,   113,   114,  1669,   115,   116,     0,   117,   118,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,  1672,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,    63,    64,    65,     0,    66,    67,    68,
       0,    70,    71,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    79,     0,     0,     0,     0,   179,    81,    82,
      83,    84,     0,    85,    86,     0,    87,   180,    89,    90,
       0,     0,    91,     0,     0,    92,     0,     0,     0,     0,
       0,    93,     0,     0,     0,     0,    96,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,     0,   113,   114,     0,
     115,   116,     0,   117,   118,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,     0,
      55,    56,     0,    57,     0,    59,    60,    61,    62,    63,
      64,    65,     0,    66,    67,    68,     0,    70,    71,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    79,     0,
       0,     0,     0,   179,    81,    82,    83,    84,     0,    85,
      86,     0,    87,   180,    89,    90,     0,     0,    91,     0,
       0,    92,     0,     0,     0,     0,     0,    93,     0,     0,
       0,     0,    96,    97,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,   112,     0,   113,   114,  1688,   115,   116,     0,   117,
     118,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,    63,    64,    65,     0,    66,
      67,    68,     0,    70,    71,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    79,     0,     0,     0,     0,   179,
      81,    82,    83,    84,     0,    85,    86,     0,    87,   180,
      89,    90,     0,     0,    91,     0,     0,    92,     0,     0,
       0,     0,     0,    93,     0,     0,     0,     0,    96,    97,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,   112,     0,   113,
     114,  1743,   115,   116,     0,   117,   118,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,     0,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,     0,    55,    56,     0,    57,     0,    59,    60,    61,
      62,    63,    64,    65,     0,    66,    67,    68,     0,    70,
      71,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      79,     0,     0,     0,     0,   179,    81,    82,    83,    84,
       0,    85,    86,     0,    87,   180,    89,    90,     0,     0,
      91,     0,     0,    92,     0,     0,     0,     0,     0,    93,
       0,     0,     0,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,   113,   114,  1750,   115,   116,
       0,   117,   118,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,    63,    64,    65,
       0,    66,    67,    68,     0,    70,    71,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    79,     0,     0,     0,
       0,   179,    81,    82,    83,    84,     0,    85,    86,     0,
      87,   180,    89,    90,     0,     0,    91,     0,     0,    92,
       0,     0,     0,     0,     0,    93,     0,     0,     0,     0,
      96,    97,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,   112,
       0,   113,   114,     0,   115,   116,     0,   117,   118,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,   453,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,   175,   176,    65,     0,    66,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    79,     0,     0,     0,     0,   179,    81,    82,
      83,    84,     0,    85,    86,     0,    87,   180,    89,     0,
       0,     0,    91,     0,     0,    92,     0,     0,     0,     0,
       0,    93,     0,     0,     0,     0,    96,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,     0,   113,   114,     0,
     115,   116,     0,   117,   118,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,   717,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,     0,    47,     0,     0,
      48,    49,     0,     0,     0,    50,    51,    52,    53,     0,
      55,    56,     0,    57,     0,    59,    60,    61,    62,   175,
     176,    65,     0,    66,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,    72,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    79,     0,
       0,     0,     0,   179,    81,    82,    83,    84,     0,    85,
      86,     0,    87,   180,    89,     0,     0,     0,    91,     0,
       0,    92,     0,     0,     0,     0,     0,    93,     0,     0,
       0,     0,    96,    97,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,   112,     0,   113,   114,     0,   115,   116,     0,   117,
     118,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,   929,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,     0,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,    49,     0,     0,
       0,    50,    51,    52,    53,     0,    55,    56,     0,    57,
       0,    59,    60,    61,    62,   175,   176,    65,     0,    66,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
      72,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    79,     0,     0,     0,     0,   179,
      81,    82,    83,    84,     0,    85,    86,     0,    87,   180,
      89,     0,     0,     0,    91,     0,     0,    92,     0,     0,
       0,     0,     0,    93,     0,     0,     0,     0,    96,    97,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,   112,     0,   113,
     114,     0,   115,   116,     0,   117,   118,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,  1441,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,    40,     0,     0,     0,
      41,    42,    43,    44,     0,    45,     0,    46,     0,    47,
       0,     0,    48,    49,     0,     0,     0,    50,    51,    52,
      53,     0,    55,    56,     0,    57,     0,    59,    60,    61,
      62,   175,   176,    65,     0,    66,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,    72,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      79,     0,     0,     0,     0,   179,    81,    82,    83,    84,
       0,    85,    86,     0,    87,   180,    89,     0,     0,     0,
      91,     0,     0,    92,     0,     0,     0,     0,     0,    93,
       0,     0,     0,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,   113,   114,     0,   115,   116,
       0,   117,   118,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
    1584,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
       0,    45,     0,    46,     0,    47,     0,     0,    48,    49,
       0,     0,     0,    50,    51,    52,    53,     0,    55,    56,
       0,    57,     0,    59,    60,    61,    62,   175,   176,    65,
       0,    66,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,    72,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    79,     0,     0,     0,
       0,   179,    81,    82,    83,    84,     0,    85,    86,     0,
      87,   180,    89,     0,     0,     0,    91,     0,     0,    92,
       0,     0,     0,     0,     0,    93,     0,     0,     0,     0,
      96,    97,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,   112,
       0,   113,   114,     0,   115,   116,     0,   117,   118,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,     0,    46,
       0,    47,     0,     0,    48,    49,     0,     0,     0,    50,
      51,    52,    53,     0,    55,    56,     0,    57,     0,    59,
      60,    61,    62,   175,   176,    65,     0,    66,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,    72,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    79,     0,     0,     0,     0,   179,    81,    82,
      83,    84,     0,    85,    86,     0,    87,   180,    89,     0,
       0,     0,    91,     0,     0,    92,     0,     0,     0,     0,
       0,    93,     0,     0,     0,     0,    96,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,     0,   113,   114,     0,
     115,   116,     0,   117,   118,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   653,
      12,     0,     0,     0,     0,     0,     0,   654,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   175,
     176,   177,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   178,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    79,     0,
       0,     0,     0,   179,    81,    82,    83,    84,     0,    85,
      86,     0,    87,   180,    89,     0,     0,     0,    91,     0,
       0,    92,     0,     0,     0,     0,     0,    93,     0,     0,
       0,     0,    96,    97,   264,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,   112,     0,     0,     0,     0,   115,   116,     0,   117,
     118,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   175,   176,   177,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     178,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    79,     0,     0,     0,     0,   179,
      81,    82,    83,    84,     0,    85,    86,     0,    87,   180,
      89,     0,     0,     0,    91,     0,     0,    92,     0,     0,
       0,     0,     0,    93,     0,     0,     0,     0,    96,    97,
     264,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,   112,     0,   265,
       0,     0,   115,   116,     0,   117,   118,     5,     6,     7,
       8,     9,     0,     0,     0,     0,   358,    10,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   597,
     380,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,   381,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   175,   176,   177,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   178,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      79,     0,     0,     0,     0,   179,    81,    82,    83,    84,
       0,    85,    86,     0,    87,   180,    89,     0,   598,     0,
      91,     0,     0,    92,     0,     0,     0,     0,     0,    93,
       0,     0,     0,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,     0,     0,     0,   115,   116,
       0,   117,   118,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   175,   176,   177,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   178,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    79,     0,     0,     0,
       0,   179,    81,    82,    83,    84,     0,    85,    86,     0,
      87,   180,    89,     0,     0,     0,    91,     0,     0,    92,
       0,     0,     0,     0,     0,    93,     0,     0,     0,     0,
      96,    97,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,   112,
     356,   357,   712,     0,   115,   116,     0,   117,   118,     5,
       6,     7,     8,     9,     0,     0,     0,     0,   358,    10,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,  1052,   380,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,   381,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   175,   176,   177,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   178,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    79,     0,     0,     0,     0,   179,    81,    82,
      83,    84,     0,    85,    86,     0,    87,   180,    89,     0,
    1053,     0,    91,     0,     0,    92,     0,     0,     0,     0,
       0,    93,     0,     0,     0,     0,    96,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,     0,     0,     0,     0,
     115,   116,     0,   117,   118,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   653,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   175,
     176,   177,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   178,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    79,     0,
       0,     0,     0,   179,    81,    82,    83,    84,     0,    85,
      86,     0,    87,   180,    89,     0,     0,     0,    91,     0,
       0,    92,     0,     0,     0,     0,     0,    93,     0,     0,
       0,     0,    96,    97,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,   112,     0,   355,   356,   357,   115,   116,     0,   117,
     118,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   358,     0,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,     0,   380,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,   381,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,   191,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   175,   176,   177,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     178,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    79,     0,     0,     0,     0,   179,
      81,    82,    83,    84,     0,    85,    86,     0,    87,   180,
      89,     0,     0,     0,    91,     0,     0,    92,     0,     0,
       0,     0,     0,    93,   994,   995,     0,     0,    96,    97,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,   112,   963,   964,
       0,     0,   115,   116,     0,   117,   118,     5,     6,     7,
       8,     9,     0,     0,     0,     0,   965,    10,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     978,   979,   980,   981,   982,   983,   984,   985,   986,   217,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,   987,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   175,   176,   177,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   178,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      79,     0,     0,     0,     0,   179,    81,    82,    83,    84,
       0,    85,    86,     0,    87,   180,    89,     0,     0,     0,
      91,     0,     0,    92,     0,     0,     0,     0,     0,    93,
       0,     0,     0,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,   355,   356,   357,   115,   116,
       0,   117,   118,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   358,     0,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,     0,   380,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
     381,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   175,   176,   177,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   178,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    79,     0,     0,     0,
       0,   179,    81,    82,    83,    84,     0,    85,    86,     0,
      87,   180,    89,     0,     0,     0,    91,     0,     0,    92,
       0,     0,     0,  1636,     0,    93,     0,     0,     0,     0,
      96,    97,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,   112,
       0,   246,     0,   357,   115,   116,     0,   117,   118,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     358,     0,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,     0,   380,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,   381,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   175,   176,   177,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   178,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    79,     0,     0,     0,     0,   179,    81,    82,
      83,    84,     0,    85,    86,     0,    87,   180,    89,     0,
       0,     0,    91,     0,     0,    92,     0,     0,     0,     0,
       0,    93,     0,     0,     0,     0,    96,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,     0,   249,     0,   964,
     115,   116,     0,   117,   118,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   965,     0,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     978,   979,   980,   981,   982,   983,   984,   985,   986,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,   987,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   175,
     176,   177,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   178,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    79,     0,
       0,     0,     0,   179,    81,    82,    83,    84,     0,    85,
      86,     0,    87,   180,    89,     0,     0,     0,    91,     0,
       0,    92,     0,     0,     0,     0,     0,    93,     0,     0,
       0,     0,    96,    97,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,   112,   451,     0,     0,     0,   115,   116,     0,   117,
     118,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,  -923,  -923,  -923,  -923,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   610,   380,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   381,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   175,   176,   177,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     178,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    79,     0,     0,     0,     0,   179,
      81,    82,    83,    84,     0,    85,    86,     0,    87,   180,
      89,     0,     0,     0,    91,     0,     0,    92,     0,     0,
       0,     0,     0,    93,     0,     0,     0,     0,    96,    97,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,   112,     0,     0,
       0,     0,   115,   116,     0,   117,   118,     5,     6,     7,
       8,     9,     0,     0,     0,     0,   965,    10,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     978,   979,   980,   981,   982,   983,   984,   985,   986,   654,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,   987,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   175,   176,   177,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   178,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      79,     0,     0,     0,     0,   179,    81,    82,    83,    84,
       0,    85,    86,     0,    87,   180,    89,     0,     0,     0,
      91,     0,     0,    92,     0,     0,     0,     0,     0,    93,
       0,     0,     0,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,     0,     0,     0,   115,   116,
       0,   117,   118,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   696,   380,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,   381,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,     0,    41,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    53,     0,     0,     0,
       0,     0,     0,     0,    60,    61,    62,   175,   176,   177,
       0,     0,    67,    68,     0,     0,     0,     0,     0,     0,
       0,     0,   178,    73,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,     0,     0,    79,     0,     0,     0,
       0,   179,    81,    82,    83,    84,     0,    85,    86,     0,
      87,   180,    89,     0,     0,     0,    91,     0,     0,    92,
       0,     0,     0,     0,     0,    93,     0,     0,     0,     0,
      96,    97,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,   112,
       0,     0,     0,     0,   115,   116,     0,   117,   118,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     966,   967,   968,   969,   970,   971,   972,   973,   974,   975,
     976,   977,   978,   979,   980,   981,   982,   983,   984,   985,
     986,   698,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,   987,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
      60,    61,    62,   175,   176,   177,     0,     0,    67,    68,
       0,     0,     0,     0,     0,     0,     0,     0,   178,    73,
       0,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,     0,    79,     0,     0,     0,     0,   179,    81,    82,
      83,    84,     0,    85,    86,     0,    87,   180,    89,     0,
       0,     0,    91,     0,     0,    92,     0,     0,     0,     0,
       0,    93,     0,     0,     0,     0,    96,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,     0,     0,     0,     0,
     115,   116,     0,   117,   118,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,  1097,   380,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
     381,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,     0,    41,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    53,     0,
       0,     0,     0,     0,     0,     0,    60,    61,    62,   175,
     176,   177,     0,     0,    67,    68,     0,     0,     0,     0,
       0,     0,     0,     0,   178,    73,     0,    74,    75,    76,
      77,    78,     0,     0,     0,     0,     0,     0,    79,     0,
       0,     0,     0,   179,    81,    82,    83,    84,     0,    85,
      86,     0,    87,   180,    89,     0,     0,     0,    91,     0,
       0,    92,     0,     0,     0,     0,     0,    93,     0,     0,
       0,     0,    96,    97,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,   112,     0,   355,   356,   357,   115,   116,     0,   117,
     118,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   358,     0,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,     0,   380,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,   381,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    53,     0,     0,     0,     0,     0,
       0,     0,    60,    61,    62,   175,   176,   177,     0,     0,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
     178,    73,     0,    74,    75,    76,    77,    78,     0,     0,
       0,     0,     0,     0,    79,     0,     0,     0,     0,   179,
      81,    82,    83,    84,     0,    85,    86,     0,    87,   180,
      89,     0,     0,     0,    91,     0,     0,    92,     0,     0,
    1449,     0,     0,    93,     0,     0,     0,     0,    96,    97,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,   112,     0,   355,
     356,   357,   115,   116,     0,   117,   118,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   358,     0,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,     0,   380,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,   381,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,   546,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      53,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   175,   176,   177,     0,     0,    67,    68,     0,     0,
       0,     0,     0,     0,     0,     0,   178,    73,     0,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,     0,
      79,     0,     0,     0,     0,   179,    81,    82,    83,    84,
       0,    85,    86,     0,    87,   180,    89,     0,     0,     0,
      91,     0,     0,    92,     0,  1288,     0,     0,     0,    93,
       0,     0,     0,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,   355,   356,   357,     0,   115,   116,
       0,   117,   118,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   358,     0,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,     0,   380,   355,   356,
     357,     0,     0,     0,     0,     0,     0,     0,     0,   381,
       0,     0,     0,     0,     0,     0,     0,   358,     0,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
       0,   380,   355,   356,   357,     0,     0,     0,     0,     0,
       0,     0,     0,   381,     0,     0,     0,     0,     0,     0,
       0,   358,     0,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,     0,   380,   355,   356,   357,     0,
       0,     0,     0,     0,     0,     0,     0,   381,     0,     0,
       0,     0,     0,     0,     0,   358,     0,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,     0,   380,
     355,   356,   357,     0,     0,     0,     0,     0,     0,     0,
       0,   381,     0,     0,     0,     0,     0,   888,     0,   358,
       0,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,     0,   380,   355,   356,   357,     0,     0,     0,
       0,     0,     0,     0,     0,   381,     0,     0,     0,     0,
       0,  1012,     0,   358,     0,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,     0,   380,   355,   356,
     357,     0,     0,     0,     0,     0,     0,     0,     0,   381,
       0,     0,     0,     0,     0,  1065,     0,   358,     0,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
       0,   380,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   381,     0,     0,     0,     0,     0,  1076,
       0,     0,     0,     0,  1465,  1466,  1467,  1468,  1469,     0,
       0,  1470,  1471,  1472,  1473,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1474,  1475,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1101,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1476,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1477,  1478,
    1479,  1480,  1481,  1482,  1483,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,  1415,  1484,  1485,
    1486,  1487,  1488,  1489,  1490,  1491,  1492,  1493,  1494,    48,
    1495,  1496,  1497,  1498,  1499,  1500,  1501,  1502,  1503,  1504,
    1505,  1506,  1507,  1508,  1509,  1510,  1511,  1512,  1513,  1514,
    1515,  1516,  1517,  1518,  1519,  1520,  1521,  1522,  1523,  1524,
       0,  1416,     0,  1525,  1526,     0,  1527,  1528,  1529,  1530,
    1531,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1532,  1533,  1534,     0,     0,     0,    85,    86,
       0,    87,   180,    89,  1535,     0,  1536,  1537,     0,  1538,
       0,     0,     0,     0,     0,     0,  1539,  1540,     0,  1541,
       0,  1542,  1543,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   355,   356,   357,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   358,  1266,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,     0,
     380,   355,   356,   357,     0,     0,     0,     0,     0,     0,
       0,     0,   381,     0,     0,     0,     0,     0,     0,     0,
     358,     0,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,     0,   380,   355,   356,   357,     0,     0,
       0,     0,     0,     0,     0,     0,   381,     0,     0,     0,
       0,     0,     0,     0,   358,     0,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,     0,   380,   355,
     356,   357,     0,     0,     0,     0,     0,     0,     0,     0,
     381,     0,     0,     0,     0,     0,     0,     0,   358,     0,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,     0,   380,   355,   356,   357,     0,     0,     0,     0,
       0,     0,     0,     0,   381,     0,     0,     0,  1267,     0,
       0,     0,   358,     0,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,     0,   380,   355,   356,   357,
       0,     0,     0,     0,     0,     0,     0,     0,   381,     0,
       0,     0,   382,     0,     0,     0,   358,     0,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,     0,
     380,   355,   356,   357,     0,     0,     0,     0,     0,     0,
       0,     0,   381,     0,     0,     0,   467,     0,     0,     0,
     358,     0,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,     0,   380,   355,   356,   357,     0,     0,
       0,     0,     0,     0,     0,     0,   381,     0,     0,     0,
     469,     0,     0,     0,   358,     0,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,     0,   380,   962,
     963,   964,     0,     0,     0,     0,     0,     0,     0,     0,
     381,     0,     0,     0,   481,     0,     0,     0,   965,     0,
     966,   967,   968,   969,   970,   971,   972,   973,   974,   975,
     976,   977,   978,   979,   980,   981,   982,   983,   984,   985,
     986,   355,   356,   357,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   987,     0,     0,     0,   505,     0,
     358,     0,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   251,   380,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   381,     0,     0,     0,
       0,   687,     0,     0,     0,     0,     0,     0,     0,   252,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   251,
     380,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   381,     0,     0,   709,     0,     0,     0,     0,
       0,     0,    48,     0,     0,   252,     0,     0,     0,     0,
    -333,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,   175,   176,   345,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   253,   254,  1140,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,   251,     0,     0,   179,   474,     0,    83,   255,
       0,    85,    86,     0,    87,   180,    89,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   998,     0,   252,   256,
       0,     0,     0,   253,   254,   346,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
      36,   179,     0,   257,    83,   255,     0,    85,    86,     0,
      87,   180,    89,     0,     0,     0,     0,     0,     0,     0,
     251,    48,     0,     0,     0,   256,     0,     0,     0,     0,
       0,     0,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   252,     0,     0,   257,
       0,     0,     0,     0,     0,     0,   253,   254,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,   179,     0,     0,    83,   255,     0,
      85,    86,     0,    87,   180,    89,     0,   943,     0,    48,
       0,     0,     0,   251,     0,     0,     0,     0,   256,     0,
       0,     0,     0,     0,     0,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   252,
       0,     0,   257,     0,   253,   254,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   251,
       0,    36,   179,     0,     0,    83,   255,     0,    85,    86,
       0,    87,   180,    89,     0,  1272,     0,     0,     0,     0,
       0,     0,    48,     0,     0,   252,   256,     0,     0,     0,
       0,     0,     0,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,    36,     0,     0,
     257,     0,     0,     0,     0,     0,     0,   253,   254,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,   179,     0,     0,    83,   255,
       0,    85,    86,     0,    87,   180,    89,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   256,
    1376,     0,     0,   253,   254,     0,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,   179,     0,   257,    83,   255,     0,    85,    86,     0,
      87,   180,    89,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   256,     0,     0,     0,     0,
    1149,     0,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   737,   738,     0,   257,
       0,     0,   739,     0,   740,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   741,     0,     0,     0,
       0,     0,     0,     0,    33,    34,    35,    36,     0,     0,
       0,   923,     0,     0,     0,     0,   742,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     978,   979,   980,   981,   982,   983,   984,   985,   986,     0,
       0,     0,     0,    36,     0,   208,     0,     0,     0,     0,
       0,     0,   987,   743,     0,    74,    75,    76,    77,    78,
       0,     0,     0,     0,    48,     0,   744,     0,     0,     0,
       0,   179,    81,    82,    83,   745,     0,    85,    86,     0,
      87,   180,    89,     0,     0,   209,    91,     0,     0,     0,
       0,     0,     0,     0,     0,   746,     0,     0,   924,     0,
      96,     0,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,    36,   179,   208,   747,
      83,    84,     0,    85,    86,     0,    87,   180,    89,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,   737,   738,   210,     0,     0,     0,   739,
     115,   740,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   741,     0,     0,     0,     0,     0,     0,
       0,    33,    34,    35,    36,     0,    85,    86,     0,    87,
     180,    89,     0,   742,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,     0,     0,     0,
       0,   642,     0,   115,     0,     0,     0,     0,     0,     0,
     743,     0,    74,    75,    76,    77,    78,     0,     0,     0,
       0,     0,     0,   744,     0,     0,     0,     0,   179,    81,
      82,    83,   745,     0,    85,    86,     0,    87,   180,    89,
       0,     0,     0,    91,     0,     0,     0,     0,     0,     0,
       0,     0,   746,   890,   891,     0,     0,    96,     0,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   892,     0,     0,   747,     0,     0,     0,
       0,   893,   894,   895,    36,     0,     0,     0,     0,     0,
       0,     0,     0,   896,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,   971,   972,   973,
     974,   975,   976,   977,   978,   979,   980,   981,   982,   983,
     984,   985,   986,    29,    30,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,    38,   987,     0,     0,     0,
     897,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   898,    48,     0,     0,     0,     0,     0,
       0,     0,    53,     0,    85,    86,     0,    87,   180,    89,
      60,    61,    62,   175,   176,   177,     0,    29,    30,     0,
       0,     0,   899,     0,     0,     0,     0,    36,     0,   208,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,     0,     0,   179,    48,     0,
      83,    84,     0,    85,    86,     0,    87,   180,    89,     0,
       0,     0,     0,     0,     0,    92,     0,     0,     0,   209,
       0,     0,     0,     0,     0,    36,     0,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   433,    48,     0,     0,     0,
     115,   179,     0,     0,    83,    84,     0,    85,    86,     0,
      87,   180,    89,     0,     0,     0,     0,     0,     0,    92,
       0,     0,     0,     0,     0,    36,     0,   208,     0,     0,
       0,     0,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,    48,     0,     0,   433,
       0,     0,   305,     0,   115,    85,    86,     0,    87,   180,
      89,     0,     0,     0,     0,     0,     0,   209,     0,     0,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,    48,     0,     0,     0,     0,   179,
     306,     0,    83,    84,     0,    85,    86,     0,    87,   180,
      89,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,   208,     0,     0,     0,     0,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,    48,     0,     0,   210,     0,     0,
     518,  1549,   115,    85,    86,  1550,    87,   180,    89,     0,
       0,     0,     0,     0,     0,   209,     0,     0,     0,     0,
       0,     0,    36,     0,     0,     0,     0,     0,   538,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,    48,     0,  1397,     0,   179,     0,     0,
      83,    84,     0,    85,    86,     0,    87,   180,    89,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1396,     0,
       0,    36,     0,   208,     0,     0,     0,     0,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,    48,     0,     0,   210,     0,     0,     0,     0,
     115,     0,    85,    86,     0,    87,   180,    89,     0,     0,
       0,     0,     0,   209,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,   208,     0,  1023,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,     0,    48,  1397,   179,     0,     0,    83,    84,
       0,    85,    86,     0,    87,   180,    89,     0,     0,     0,
       0,     0,     0,     0,   209,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,   208,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,     0,   210,    48,     0,   179,     0,   115,    83,
      84,     0,    85,    86,     0,    87,   180,    89,     0,     0,
       0,     0,     0,     0,     0,   222,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,     0,     0,   210,     0,     0,   179,     0,   115,
      83,    84,     0,    85,    86,     0,    87,   180,    89,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,    36,    48,   208,   223,     0,     0,     0,     0,
     115,   560,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,   978,   979,   980,   981,   982,   983,
     984,   985,   986,    36,   209,   208,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   987,     0,     0,     0,
       0,     0,    85,    86,    48,    87,   180,    89,     0,     0,
       0,     0,     0,     0,     0,     0,   179,     0,     0,    83,
      84,     0,    85,    86,     0,    87,   180,    89,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,    36,     0,     0,   603,     0,     0,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   671,    48,    85,    86,     0,    87,   180,    89,   561,
       0,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     978,   979,   980,   981,   982,   983,   984,   985,   986,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   987,     0,     0,     0,     0,     0,   704,     0,
     115,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    85,    86,     0,    87,   180,    89,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   355,   356,   357,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,   358,   859,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,     0,   380,   355,   356,   357,
       0,     0,     0,     0,     0,     0,     0,     0,   381,     0,
       0,     0,     0,     0,     0,     0,   358,     0,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,     0,
     380,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   381,     0,   355,   356,   357,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   358,   429,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,     0,   380,   355,   356,
     357,     0,     0,     0,     0,     0,     0,     0,     0,   381,
       0,     0,     0,     0,     0,     0,     0,   358,   439,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
       0,   380,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   381,     0,   355,   356,   357,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   358,   865,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,     0,   380,   962,
     963,   964,     0,     0,     0,     0,     0,     0,     0,     0,
     381,     0,     0,     0,     0,     0,     0,     0,   965,   909,
     966,   967,   968,   969,   970,   971,   972,   973,   974,   975,
     976,   977,   978,   979,   980,   981,   982,   983,   984,   985,
     986,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   987,     0,   962,   963,   964,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   965,  1226,   966,   967,   968,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   980,   981,   982,   983,   984,   985,   986,     0,     0,
     962,   963,   964,     0,     0,     0,     0,     0,     0,     0,
       0,   987,     0,     0,     0,     0,     0,     0,     0,   965,
    1131,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   978,   979,   980,   981,   982,   983,   984,
     985,   986,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,   987,     0,   962,   963,   964,
       0,     0,   730,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,   965,  1282,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     978,   979,   980,   981,   982,   983,   984,   985,   986,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   987,     0,    36,     0,     0,     0,     0,     0,
      48,  1364,     0,     0,     0,     0,     0,   179,     0,     0,
      83,     0,     0,    85,    86,    48,    87,   180,    89,     0,
       0,     0,  1383,   273,   274,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1384,  1385,     0,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,    36,   179,     0,     0,    83,    84,  1448,    85,
      86,     0,    87,  1387,    89,     0,     0,     0,     0,     0,
       0,   275,     0,    48,    85,    86,     0,    87,   180,    89,
       0,     0,     0,     0,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,    36,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,    36,     0,   809,   810,     0,     0,    48,
       0,     0,     0,     0,     0,     0,   179,     0,     0,    83,
      84,     0,    85,    86,    48,    87,   180,    89,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,    36,     0,     0,     0,     0,   342,    48,    85,    86,
       0,    87,   180,    89,     0,     0,     0,     0,     0,     0,
       0,     0,    48,    85,    86,     0,    87,   180,    89,     0,
       0,     0,     0,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,    36,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,   506,     0,     0,    85,    86,    48,    87,
     180,    89,     0,     0,     0,     0,     0,     0,   510,     0,
       0,    85,    86,     0,    87,   180,    89,     0,     0,     0,
       0,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
      36,     0,     0,     0,   275,     0,     0,    85,    86,     0,
      87,   180,    89,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,    36,     0,   641,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    36,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      85,    86,    48,    87,   180,    89,     0,     0,     0,     0,
       0,     0,     0,     0,  1141,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,     0,     0,     0,     0,     0,    85,    86,     0,    87,
     180,    89,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    85,    86,     0,    87,   180,    89,     0,     0,     0,
       0,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     355,   356,   357,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   721,   358,
       0,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,     0,   380,     0,   355,   356,   357,     0,     0,
       0,     0,     0,     0,     0,   381,     0,     0,     0,     0,
       0,     0,     0,     0,   358,   862,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   722,   380,   355,
     356,   357,     0,     0,     0,     0,     0,     0,     0,     0,
     381,     0,     0,     0,     0,     0,     0,     0,   358,     0,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,     0,   380,   962,   963,   964,     0,     0,     0,     0,
       0,     0,     0,     0,   381,     0,     0,     0,     0,     0,
       0,     0,   965,  1287,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,   980,   981,
     982,   983,   984,   985,   986,   962,   963,   964,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   987,     0,
       0,     0,     0,     0,   965,     0,   966,   967,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   977,   978,   979,
     980,   981,   982,   983,   984,   985,   986,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     987,  -923,  -923,  -923,  -923,   975,   976,   977,   978,   979,
     980,   981,   982,   983,   984,   985,   986,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     987
};

static const yytype_int16 yycheck[] =
{
       5,     6,   159,     8,     9,    10,    11,    12,   134,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    54,     4,    28,    29,   600,   839,     4,    90,   182,
     396,   453,    94,    95,    32,   313,   571,    42,     4,     4,
     572,  1044,   650,    55,   423,    50,    44,    52,   417,   418,
      55,    49,    57,   380,   689,   828,   230,   183,   162,   720,
     553,    30,   449,   159,     9,   858,   224,     9,   112,   112,
       9,   133,   449,     9,   112,    14,   727,     9,   447,     9,
     502,   874,     9,    30,   922,     9,     9,    45,     9,     4,
       9,     9,     9,  1040,    45,     4,     9,   220,     9,     9,
      35,    33,     9,    66,     9,     9,     9,   112,  1032,     9,
       9,     9,    79,    50,     4,    79,    66,    50,     9,    79,
       9,     9,     9,     4,   298,     9,    79,   231,    66,    30,
      45,    79,    66,   101,   102,    79,    79,    14,    79,    79,
     129,   130,   117,   384,    79,   129,   130,    53,   129,   130,
     125,   573,  1578,    30,   110,   150,   171,   129,   130,    65,
      45,     0,    45,    45,   156,   199,   210,   210,    97,    85,
      47,    97,   199,   414,   199,    97,   171,    97,   419,   223,
     156,     8,   187,   150,   199,   958,    66,   147,   148,   149,
      66,    66,   107,   169,   347,   129,   130,   112,    66,   114,
     115,   116,   117,   118,   119,   120,  1632,   202,    66,    66,
     202,   164,   168,    66,   203,    66,   200,   202,   155,   200,
     164,   164,   155,    85,   164,   154,   202,   232,   154,   164,
     235,   147,   154,   170,   154,   203,   199,   242,   243,   197,
     204,    66,   157,   158,   134,   160,   171,   291,   291,   200,
     159,   172,   202,   291,   172,   202,   204,     4,   203,   201,
     202,   199,   201,   204,   202,   201,   181,   172,   200,   395,
     200,   236,   425,   200,  1221,   240,  1114,   201,   201,   341,
    1073,   932,   201,   934,   201,   147,   291,   200,   203,   200,
     712,   201,   297,   183,   201,   717,   301,   201,   201,  1223,
     201,   201,   201,   201,    51,   338,  1230,    54,  1232,   432,
     201,   200,   200,   200,   197,   200,   200,   200,   200,  1092,
      66,   326,   202,   855,    71,   818,   202,   202,   390,   391,
     392,   393,   337,    66,   202,    66,    66,    66,    26,    27,
      66,    88,    66,    90,   202,   202,   199,    94,    95,   202,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   199,   381,   202,   383,   384,
      97,   386,    97,  1330,   396,  1332,   133,   199,   313,   433,
     433,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   578,   129,   130,    35,   150,   414,
     415,   380,   417,   418,   419,   420,   325,  1341,   387,   150,
     199,   426,  1030,    79,   429,   199,   805,   171,   199,   171,
      35,   588,   101,   102,   439,   325,   441,   154,   319,   154,
     563,    79,   447,   455,   413,    79,    35,    85,   195,  1110,
     455,    79,   457,   199,   200,   199,   843,   199,  1093,    35,
     202,   199,   387,   199,     4,    35,   843,   199,   437,   202,
     171,   202,   202,   202,    79,   150,   202,   446,   202,   484,
     449,  1428,   487,   488,   489,   643,    26,    27,   413,   236,
      79,   154,   588,   240,    79,   460,   171,   244,   199,   652,
      85,   199,   871,    79,   394,    58,  1279,   929,  1043,    79,
     148,   149,   437,   518,   148,   149,   263,   640,   155,   677,
     694,   446,   210,   910,   449,  1057,   700,    80,  1060,   217,
      83,    29,   171,   910,   203,   223,   164,   695,   201,   202,
     196,    46,    47,    79,   606,   106,   202,   199,    46,    85,
     828,    49,   879,   114,   115,   116,   117,   118,   119,   164,
     199,   118,   202,   148,   149,    30,    29,   154,   125,   199,
     208,    66,   112,    97,    98,   164,  1349,    35,   325,    49,
      50,    51,    79,    46,   127,   128,    49,   334,    85,   201,
     748,   338,   280,   598,   341,    65,   700,   201,   756,    71,
      72,   289,   290,   291,  1239,   610,  1241,   201,   296,   201,
     733,   147,   148,   149,   302,    79,    71,    72,    79,   201,
     181,    85,   207,  1231,    85,  1607,  1608,   114,   115,   116,
     117,   118,   119,   201,  1581,   201,   202,   642,   201,   202,
     387,   388,   389,   390,   391,   392,   393,    79,   653,    66,
     147,   148,   149,    85,  1603,  1604,    49,    50,    51,  1025,
      53,   100,   101,   102,   735,   736,   413,   672,    66,   201,
     210,   202,    65,   150,   199,  1210,   199,   217,    66,   588,
     958,   150,   199,   223,   148,   149,   154,   148,   149,   201,
     437,   572,    44,  1225,   181,    65,   236,  1066,   171,   704,
     240,   150,   449,  1311,   100,   101,   102,   199,  1077,     9,
     206,  1714,  1347,   460,   201,   720,   148,   149,   716,   150,
     273,   274,   275,   114,   115,   116,  1729,    74,    75,    76,
     477,   116,   117,   118,  1113,   423,   182,   183,   184,    86,
     280,   150,   199,   189,   190,   433,     8,   193,   194,   289,
     290,   291,   305,   758,   201,   171,   296,   199,   150,  1181,
     725,  1708,   302,    14,    46,    47,    48,    49,    50,    51,
     517,    53,   150,  1381,    79,   201,  1723,   201,    14,   125,
     125,   904,   200,    65,   171,   325,    14,    97,   135,   136,
     137,   138,   139,   200,   200,   542,   543,   200,   199,   146,
    1435,   710,   106,   205,   199,   152,   153,     9,    89,   814,
    1342,   299,     9,  1192,  1092,   303,   878,   199,   201,   166,
     710,   200,   827,   200,   947,    14,   199,     9,   185,    79,
    1252,   954,    79,   180,    79,   817,   199,   188,     9,   327,
     817,   329,   330,   331,   332,   201,  1268,   852,   201,     9,
      79,   817,   817,   201,   127,    66,   200,   862,  1227,   606,
     865,   200,   867,   772,   200,   199,   871,   200,    30,   128,
    1683,   170,     9,   131,   843,   200,   114,   115,   116,   117,
     118,   119,   772,   423,   150,    14,   197,     9,  1701,    66,
      14,     9,  1050,   433,   172,     9,  1709,    46,    47,    48,
      49,    50,    51,   828,   909,   200,   127,   203,   817,   597,
     879,     9,   206,   206,   199,    14,    65,   915,   843,   206,
     460,   200,   206,   200,   199,    97,   200,   817,   837,  1351,
     201,   201,   131,   150,     9,   200,   817,   150,  1360,   199,
     150,   910,   689,   181,   691,   199,   202,   837,  1583,  1371,
     185,   916,   185,   506,    14,     9,    79,   510,   839,  1328,
     202,    14,   515,   710,  1087,   202,   654,   201,    14,    14,
     202,   201,   197,    30,   855,   206,   199,   724,   725,   199,
      30,    14,    14,   199,    48,   910,   991,   992,   993,     9,
     199,   199,   997,   998,   200,   114,   115,   116,   117,   118,
     119,  1279,    14,   201,   199,   201,   125,   126,   696,   918,
     698,   920,     9,  1025,   131,  1138,   200,    65,   206,  1441,
    1025,     9,  1145,    79,     9,   772,   147,   131,   918,   199,
     920,   778,   201,   958,   722,   782,   783,    14,    79,   200,
     131,   199,   199,   202,   163,   200,   202,  1029,  1053,   202,
     201,  1626,  1029,     9,   206,   802,    86,   597,   147,    30,
      73,  1066,   181,  1029,  1029,   201,   200,   172,   201,   131,
     817,  1349,  1077,  1078,   114,   115,   116,   117,   118,   119,
      30,     9,   131,   203,   831,   125,   126,   200,   200,   200,
     837,     9,   200,   203,    14,   202,   843,    79,   199,   201,
     200,     9,   790,   200,  1102,  1110,   200,   202,   200,   199,
     131,    30,   200,   200,   654,  1120,   201,   805,   806,   200,
    1029,   161,   200,   163,  1699,  1248,   202,   201,   201,   107,
     159,   878,   201,   155,    14,    79,   176,   112,   178,  1029,
    1105,   181,   200,   890,   891,   892,    14,   202,  1029,   200,
     131,  1573,   131,  1575,   171,  1312,   696,   200,   698,    14,
     202,    14,  1584,   910,   201,    79,    79,  1092,   199,   916,
     710,   918,   200,   920,   200,   202,  1057,   131,   201,  1060,
      14,  1307,   722,   201,    14,   725,   201,    14,   202,    55,
      79,   199,   939,    79,     9,   201,    79,  1202,   110,    97,
      97,  1206,   150,  1208,   162,    33,    14,  1629,  1117,   199,
     957,  1216,   201,   960,   200,   199,   168,   172,    79,   165,
     200,  1226,  1227,     9,   777,    79,   200,  1117,   781,   201,
     200,    14,   772,   202,   922,   923,    79,    14,    79,    14,
      79,   988,    14,    79,   777,  1691,   515,   781,   393,  1402,
     790,   390,   877,   388,   872,   820,  1705,  1111,  1005,  1440,
    1701,  1008,  1265,  1296,  1431,   805,   806,   520,  1463,  1382,
    1547,  1302,  1733,  1721,  1559,  1298,    42,   817,  1427,  1000,
     396,  1190,  1029,  1288,   491,   997,   491,   769,  1033,  1075,
    1199,   955,   891,  1088,   843,  1089,   906,   837,   297,   290,
     735,  1291,  1016,   988,   940,  1727,    -1,    -1,    -1,  1190,
      -1,    -1,  1734,    -1,  1376,    -1,    -1,    -1,  1199,    -1,
    1325,    -1,    -1,  1328,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
      -1,    -1,  1089,    -1,  1225,    -1,  1093,  1380,    -1,    -1,
      -1,    -1,    -1,    -1,  1279,    -1,    -1,    -1,  1105,    -1,
      -1,    -1,    -1,    -1,  1052,    -1,    -1,    -1,    -1,    -1,
    1117,  1404,    -1,  1406,    63,    64,   916,    -1,   918,    -1,
     920,  1290,   922,   923,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1357,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1290,    -1,    -1,  1312,    -1,    -1,    -1,    -1,    -1,  1097,
    1563,    -1,    -1,    -1,  1419,    -1,  1306,    -1,    -1,    -1,
      -1,    -1,  1331,    -1,  1349,  1113,  1114,    -1,  1337,    -1,
    1339,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1185,    -1,
     129,   130,    -1,  1352,  1449,    -1,    -1,    -1,    -1,    -1,
    1331,    -1,  1361,    -1,    -1,    -1,  1337,    -1,  1339,    -1,
      -1,  1342,  1352,    -1,    -1,    -1,    -1,    -1,     4,    26,
      27,  1361,    -1,    30,  1439,  1440,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,  1029,
      -1,    -1,  1239,    -1,  1241,    -1,  1558,    54,    -1,    -1,
      -1,    -1,    -1,    -1,  1192,    -1,    -1,    -1,    -1,    45,
      -1,   200,  1052,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1410,  1678,   251,   252,  1433,    63,    64,    -1,   257,  1438,
      -1,    -1,    -1,    -1,    -1,  1444,    -1,    -1,    -1,    -1,
      -1,  1450,    -1,  1290,    -1,    -1,    -1,    -1,  1438,  1296,
      -1,    -1,  1433,    -1,  1444,  1302,    -1,  1097,  1620,    -1,
    1450,    -1,    -1,  1568,    -1,  1105,    -1,    -1,    -1,  1695,
      -1,   107,    -1,  1113,  1114,    -1,   112,  1117,   114,   115,
     116,   117,   118,   119,   120,    -1,    -1,    -1,    -1,    -1,
      -1,   129,   130,    -1,    -1,    -1,    -1,    -1,    -1,     4,
    1347,    -1,    -1,    -1,    -1,  1352,   335,    -1,    -1,    -1,
    1357,    -1,    -1,    -1,  1361,    -1,    -1,    -1,    -1,    -1,
      -1,   157,   158,    -1,   160,    -1,    -1,    -1,    -1,  1376,
      -1,    -1,    -1,  1380,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    -1,    -1,  1390,    -1,   181,    -1,    -1,    -1,    -1,
    1397,    -1,  1192,   210,    -1,    -1,    -1,  1404,    -1,  1406,
     217,    -1,   200,    -1,    -1,  1412,   223,   203,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1588,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1435,    -1,
      -1,  1438,  1439,  1440,   251,   252,    -1,  1444,  1588,    -1,
     257,    -1,   107,  1450,    -1,    -1,    -1,   112,    -1,   114,
     115,   116,   117,   118,   119,   120,    -1,    -1,  1627,  1628,
      -1,    -1,    -1,   280,    -1,  1634,    -1,    -1,    -1,    -1,
      -1,    -1,   289,   290,  1739,    -1,    -1,  1627,  1628,   296,
      -1,    -1,  1747,   472,  1634,   302,    -1,    -1,  1753,    -1,
    1290,  1756,   157,   158,    -1,   160,   313,    -1,    -1,    -1,
      -1,  1670,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1678,
      -1,    -1,    -1,    -1,    -1,    -1,   181,    -1,   335,    -1,
    1670,   338,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,   203,    -1,
      -1,    -1,  1683,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,  1558,  1352,    -1,    -1,    -1,    -1,  1357,    -1,    -1,
    1701,  1361,    -1,   380,     4,    -1,  1735,    -1,  1709,    -1,
      -1,    -1,  1579,  1742,    -1,    -1,  1583,    -1,     4,   568,
     569,  1588,    -1,    -1,    -1,  1735,    -1,    -1,   577,    -1,
    1597,    -1,  1742,    -1,    -1,    -1,  1603,  1604,    -1,    -1,
    1607,  1608,    -1,    -1,    -1,    45,   423,    -1,    -1,    -1,
      -1,    -1,    -1,  1620,    -1,    -1,   433,    -1,    -1,    45,
    1627,  1628,    -1,    -1,    -1,    -1,    -1,  1634,    26,    27,
      -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,  1438,  1439,
    1440,    -1,    -1,    -1,  1444,    -1,    -1,    -1,    -1,    -1,
    1450,    -1,    -1,    -1,     4,   472,   473,    -1,    -1,   476,
      -1,    -1,    -1,  1670,    -1,    -1,    -1,   107,    -1,    -1,
    1677,    -1,   112,    -1,   114,   115,   116,   117,   118,   119,
     120,   107,    -1,    -1,    -1,    -1,   112,  1694,   114,   115,
     116,   117,   118,   119,   120,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   524,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   157,   158,    -1,
     160,    26,    27,    -1,    -1,    30,    -1,    -1,  1735,    -1,
      -1,   157,   158,    -1,   160,  1742,    -1,    -1,    -1,    -1,
      -1,   181,    -1,    -1,    -1,    -1,   735,   736,    -1,    -1,
      -1,   568,   569,    -1,    -1,   181,    -1,   107,    -1,    -1,
     577,    -1,   112,   203,   114,   115,   116,   117,   118,   119,
     120,    -1,    -1,    -1,    -1,    -1,    -1,   203,    -1,    -1,
     597,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1588,    -1,
      -1,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,   157,   158,    -1,
     160,    -1,   210,    -1,    -1,    -1,    -1,    -1,    -1,   217,
      -1,    -1,    -1,    -1,    -1,   223,    -1,  1627,  1628,    -1,
     819,   181,    -1,    -1,  1634,    -1,    -1,   654,    -1,    -1,
      -1,    63,    64,    -1,    -1,   834,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   203,    -1,    -1,    -1,   846,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
    1670,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   696,
      -1,   698,   280,    65,    -1,    -1,   875,    -1,    -1,    -1,
      -1,   289,   290,    -1,    -1,    -1,    -1,    -1,   296,    -1,
      -1,    -1,    -1,    -1,   302,   722,   723,   129,   130,    -1,
      -1,    -1,   217,    -1,    -1,   313,    -1,    -1,   735,   736,
     737,   738,   739,   740,   741,    -1,    -1,    -1,    -1,    -1,
     747,    -1,    -1,    -1,    -1,  1735,    -1,    -1,    -1,    -1,
      -1,    -1,  1742,    -1,    -1,    -1,    -1,   936,    -1,   938,
      -1,    -1,    -1,    -1,    -1,    -1,   773,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,   790,    -1,   280,    -1,    -1,    -1,    -1,
      -1,    -1,   380,    -1,   289,   290,   803,    -1,   805,   806,
      -1,   296,    -1,    -1,    -1,    -1,    -1,   302,    -1,    -1,
      -1,    -1,   819,   820,    -1,    -1,    63,    64,   313,    -1,
      -1,   828,    -1,    -1,    -1,    -1,    -1,   834,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   423,    -1,    -1,    -1,   846,
      -1,    -1,    -1,    -1,    -1,   433,    -1,   854,    -1,    -1,
     857,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1042,    -1,  1044,    -1,    -1,   875,    -1,
      -1,    -1,   879,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,   130,    -1,   380,    -1,    -1,    -1,    -1,
    1069,    -1,    -1,  1072,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,   922,   923,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,   423,   936,
      -1,   938,    -1,   940,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   955,  1128,
      -1,   958,    -1,    -1,   961,   962,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,   982,   983,   984,   985,   986,
     987,   476,    -1,    -1,   155,    -1,   157,   158,    -1,   160,
     161,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1013,  1186,  1187,   597,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,    -1,    -1,    -1,   524,
      -1,   202,    29,   204,    -1,  1042,    -1,  1044,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1052,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    -1,  1069,    -1,    -1,  1072,   654,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      77,    98,    -1,    -1,    -1,  1092,    -1,    -1,    -1,    -1,
    1097,    -1,  1271,    -1,  1273,    -1,    -1,    -1,    -1,    -1,
      -1,    98,   597,    -1,    -1,    -1,  1113,  1114,   696,  1116,
     698,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1128,    -1,   120,  1131,    -1,  1133,    -1,  1307,    54,
      -1,    -1,    -1,    -1,   722,    -1,   133,   134,    -1,    -1,
     157,   158,  1149,   160,   161,   162,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   151,    -1,    -1,   154,   155,   654,
     157,   158,    -1,   160,   161,   162,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,  1186,
    1187,    -1,  1189,    -1,    -1,  1192,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   696,   790,   698,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   805,   806,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   722,   723,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,
     828,    -1,   737,   738,   739,   740,   741,    -1,    -1,    -1,
      -1,    -1,   747,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,    -1,    -1,    -1,  1271,    -1,  1273,    -1,    -1,    -1,
      -1,  1278,  1279,    -1,    -1,  1282,    -1,  1284,   773,    -1,
    1287,    -1,  1461,    -1,    -1,    -1,    -1,    -1,    -1,  1296,
    1297,   879,    -1,  1300,    -1,   790,    -1,    -1,    -1,    -1,
    1307,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   803,    -1,
     805,   806,   151,    -1,    -1,   154,    -1,    -1,   157,   158,
      -1,   160,   161,   162,    -1,   820,   251,   252,    -1,    -1,
      -1,    -1,   257,   828,   922,   923,    -1,    -1,    -1,    -1,
      -1,    -1,  1349,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,  1364,    -1,   854,
      -1,    -1,   857,    -1,   203,    -1,    -1,  1374,  1375,    -1,
     958,    -1,    -1,  1380,    -1,  1382,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1562,   879,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1404,    -1,  1406,
      -1,    -1,    -1,    -1,    -1,  1412,    -1,    -1,    -1,    -1,
     335,    -1,    -1,   338,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   922,   923,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1447,  1448,    -1,    -1,    -1,    -1,    -1,  1454,  1455,    -1,
      -1,    -1,    -1,    -1,  1461,    -1,  1463,    -1,    -1,    -1,
     955,    -1,    -1,   958,  1052,  1644,   961,   962,   963,   964,
     965,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   978,   979,   980,   981,   982,   983,   984,
     985,   986,   987,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1092,    -1,    -1,    -1,    -1,  1097,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1013,    -1,
      -1,    -1,    -1,    -1,    -1,  1113,  1114,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1714,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   472,   473,    -1,
    1729,   476,    -1,    -1,    -1,  1562,    -1,  1052,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1579,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
    1597,    53,    -1,    -1,    -1,  1602,    -1,  1092,    -1,   524,
      -1,    -1,  1097,    65,  1192,    -1,  1613,    -1,    29,    -1,
      -1,  1618,    -1,    -1,    -1,  1622,    -1,    -1,  1113,  1114,
      -1,  1116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    55,    -1,  1131,  1644,  1133,    -1,
      -1,    -1,    -1,   568,   569,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   577,    -1,  1149,    -1,    77,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,  1682,    53,    98,    -1,    -1,
      -1,    -1,    -1,  1690,    -1,    -1,    -1,    -1,    65,    -1,
      -1,  1279,    -1,    -1,  1189,    -1,    -1,  1192,  1705,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1714,    -1,    -1,
      -1,    -1,   133,   134,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1729,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     151,    -1,    -1,   154,   155,    -1,   157,   158,    -1,   160,
     161,   162,    -1,   476,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,
      -1,  1349,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,    -1,    -1,   199,    -1,
      -1,    -1,   203,  1278,  1279,    -1,    -1,  1282,    -1,  1284,
      -1,   524,  1287,    -1,    -1,    -1,    -1,    -1,   723,    -1,
      -1,    -1,  1297,    -1,    -1,  1300,    10,    11,    12,    -1,
     735,   736,   737,   738,   739,   740,   741,    -1,    -1,   476,
      -1,    -1,   747,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,  1349,    68,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    77,    -1,    79,   524,    -1,  1364,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1374,
    1375,    -1,    -1,    -1,    -1,    98,    -1,  1382,    -1,    -1,
      -1,    -1,    -1,    -1,   819,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,   834,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   846,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   854,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,
      -1,   154,   155,    -1,   157,   158,    -1,   160,   161,   162,
     875,    -1,  1447,  1448,    -1,    -1,    -1,    -1,    -1,  1454,
    1455,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1463,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,    -1,   199,    -1,    -1,    -1,
     723,   204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   206,    -1,   737,   738,   739,   740,   741,    -1,
      -1,   936,    -1,   938,   747,   940,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     955,    -1,    -1,    -1,    -1,    -1,   961,   962,   963,   964,
     965,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   978,   979,   980,   981,   982,   983,   984,
     985,   986,   987,    -1,    -1,    -1,   723,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     737,   738,   739,   740,   741,    -1,    -1,    -1,  1013,    -1,
     747,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   476,    -1,    -1,  1602,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1042,  1613,  1044,
      -1,   854,    -1,  1618,    -1,    -1,    -1,  1622,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,  1069,    -1,    -1,  1072,    -1,    -1,
      -1,    -1,   524,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,  1682,    -1,    -1,
      -1,  1116,    -1,    65,    -1,  1690,    -1,   854,    -1,    -1,
      -1,    -1,    -1,  1128,    -1,    -1,  1131,    -1,  1133,    -1,
    1705,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   955,    -1,  1149,    -1,    -1,    -1,   961,   962,
     963,   964,   965,   966,   967,   968,   969,   970,   971,   972,
     973,   974,   975,   976,   977,   978,   979,   980,   981,   982,
     983,   984,   985,   986,   987,    -1,    -1,    -1,    -1,    -1,
      -1,  1186,  1187,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
    1013,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,   955,    -1,
      -1,    -1,    -1,    -1,   961,   962,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,   982,   983,   984,   985,   986,
     987,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1271,    -1,  1273,    -1,
      -1,   723,    -1,  1278,    -1,    -1,  1013,  1282,    -1,  1284,
      -1,    -1,  1287,    -1,    -1,   737,   738,   739,   740,    -1,
      -1,  1296,    -1,    -1,    -1,   747,    -1,    -1,    -1,    -1,
      -1,    -1,  1307,  1116,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1131,    -1,
    1133,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1149,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1364,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    -1,  1380,    -1,    -1,    -1,  1116,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,  1131,    -1,  1133,    -1,    -1,  1404,
      -1,  1406,   854,    -1,    -1,    -1,    -1,  1412,    -1,    -1,
      -1,    -1,  1149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1447,  1448,   151,    -1,    -1,   154,   155,  1454,
     157,   158,    -1,   160,   161,   162,  1461,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1278,    -1,    -1,   175,  1282,
      -1,  1284,    -1,    -1,  1287,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,    -1,   199,    -1,    -1,    -1,   203,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,   980,   981,
     982,   983,   984,   985,   986,   987,    -1,    -1,    -1,    -1,
      -1,  1278,    -1,    -1,    -1,  1282,    -1,  1284,    -1,    -1,
    1287,  1364,    -1,    -1,    -1,    -1,    -1,  1562,    -1,    -1,
      -1,  1013,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1579,    -1,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,  1597,    53,    -1,    -1,    -1,  1602,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,  1613,    -1,
      -1,    -1,    -1,  1618,    -1,    -1,    -1,  1622,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1364,    -1,    -1,
      -1,    -1,    -1,    -1,  1447,  1448,    -1,    -1,    -1,  1644,
      -1,  1454,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1462,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1116,    -1,    -1,    -1,    77,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,  1682,    -1,  1131,
      -1,  1133,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1149,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,  1714,
    1447,  1448,    -1,    -1,    -1,    -1,   125,  1454,    -1,    -1,
      -1,    -1,    -1,    29,  1729,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,   157,   158,
      -1,   160,   161,   162,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,  1602,
      -1,    -1,    -1,   202,    -1,   204,    -1,    -1,    -1,    -1,
    1613,    -1,    -1,    -1,    -1,  1618,    -1,    -1,    -1,  1622,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1278,    -1,    -1,    -1,
    1282,    -1,  1284,  1646,    29,  1287,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    -1,  1602,    -1,    -1,    -1,  1682,
      65,    -1,    -1,    -1,    -1,    -1,  1613,    -1,    -1,    -1,
      -1,  1618,    -1,    -1,    -1,  1622,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   203,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1364,    -1,    -1,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,  1682,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,  1447,  1448,    -1,    86,    87,
      88,    89,  1454,    91,    -1,    93,    -1,    95,   203,    -1,
      98,    99,    -1,    -1,    -1,   103,   104,   105,   106,   107,
     108,   109,    -1,   111,   112,   113,   114,   115,   116,   117,
     118,   119,    -1,   121,   122,   123,   124,   125,   126,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,
      -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,   163,    -1,    -1,   166,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,   175,   176,    -1,
     178,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,   201,   202,   203,   204,   205,    -1,   207,
     208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1602,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,  1613,    -1,    -1,    -1,    -1,  1618,    -1,    -1,    -1,
    1622,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    95,    -1,    -1,    98,    99,    -1,    -1,
    1682,   103,   104,   105,   106,   107,   108,   109,    -1,   111,
     112,   113,   114,   115,   116,   117,   118,   119,    -1,   121,
     122,   123,   124,   125,   126,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,    -1,    -1,
      -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,
     152,   153,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,   163,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,
      -1,    -1,    -1,   175,   176,    -1,   178,    -1,   180,   181,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    -1,   201,
     202,   203,   204,   205,    -1,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,
      86,    87,    88,    89,    -1,    91,    -1,    93,    -1,    95,
      -1,    -1,    98,    99,    -1,    -1,    -1,   103,   104,   105,
     106,   107,   108,   109,    -1,   111,   112,   113,   114,   115,
     116,   117,   118,   119,    -1,   121,   122,   123,   124,   125,
     126,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,
     146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,   163,    -1,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,
     176,    -1,   178,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,   201,   202,    -1,   204,   205,
      -1,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,    99,
      -1,    -1,    -1,   103,   104,   105,   106,    -1,   108,   109,
      -1,   111,    -1,   113,   114,   115,   116,   117,   118,   119,
      -1,   121,   122,   123,    -1,   125,   126,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
      -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,
      -1,   151,   152,   153,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,   163,    -1,    -1,   166,    -1,    -1,   169,
      -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   199,
      -1,   201,   202,   203,   204,   205,    -1,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    82,    -1,
      -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,    93,
      -1,    95,    -1,    -1,    98,    99,    -1,    -1,    -1,   103,
     104,   105,   106,    -1,   108,   109,    -1,   111,    -1,   113,
     114,   115,   116,   117,   118,   119,    -1,   121,   122,   123,
      -1,   125,   126,    -1,    -1,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,
      -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,   163,
      -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,   201,   202,   203,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    99,    -1,    -1,    -1,   103,   104,   105,   106,    -1,
     108,   109,    -1,   111,    -1,   113,   114,   115,   116,   117,
     118,   119,    -1,   121,   122,   123,    -1,   125,   126,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,
      -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,   163,    -1,    -1,   166,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,   201,   202,   203,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    95,    -1,    -1,    98,    99,    -1,    -1,
      -1,   103,   104,   105,   106,    -1,   108,   109,    -1,   111,
      -1,   113,   114,   115,   116,   117,   118,   119,    -1,   121,
     122,   123,    -1,   125,   126,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,    -1,    -1,
      -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,
     152,   153,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,   163,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,
      -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    -1,   201,
     202,   203,   204,   205,    -1,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,
      86,    87,    88,    89,    90,    91,    -1,    93,    -1,    95,
      -1,    -1,    98,    99,    -1,    -1,    -1,   103,   104,   105,
     106,    -1,   108,   109,    -1,   111,    -1,   113,   114,   115,
     116,   117,   118,   119,    -1,   121,   122,   123,    -1,   125,
     126,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,
     146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,   163,    -1,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,   201,   202,    -1,   204,   205,
      -1,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    95,    96,    -1,    98,    99,
      -1,    -1,    -1,   103,   104,   105,   106,    -1,   108,   109,
      -1,   111,    -1,   113,   114,   115,   116,   117,   118,   119,
      -1,   121,   122,   123,    -1,   125,   126,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
      -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,
      -1,   151,   152,   153,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,   163,    -1,    -1,   166,    -1,    -1,   169,
      -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   199,
      -1,   201,   202,    -1,   204,   205,    -1,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    82,    -1,
      -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,    93,
      -1,    95,    -1,    -1,    98,    99,    -1,    -1,    -1,   103,
     104,   105,   106,    -1,   108,   109,    -1,   111,    -1,   113,
     114,   115,   116,   117,   118,   119,    -1,   121,   122,   123,
      -1,   125,   126,    -1,    -1,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,
      -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,   163,
      -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,   201,   202,   203,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    99,    -1,    -1,    -1,   103,   104,   105,   106,    -1,
     108,   109,    -1,   111,    -1,   113,   114,   115,   116,   117,
     118,   119,    -1,   121,   122,   123,    -1,   125,   126,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,
      -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,   163,    -1,    -1,   166,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,   201,   202,   203,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,
      -1,    93,    94,    95,    -1,    -1,    98,    99,    -1,    -1,
      -1,   103,   104,   105,   106,    -1,   108,   109,    -1,   111,
      -1,   113,   114,   115,   116,   117,   118,   119,    -1,   121,
     122,   123,    -1,   125,   126,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,    -1,    -1,
      -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,
     152,   153,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,   163,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,
      -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    -1,   201,
     202,    -1,   204,   205,    -1,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,
      86,    87,    88,    89,    -1,    91,    -1,    93,    -1,    95,
      -1,    -1,    98,    99,    -1,    -1,    -1,   103,   104,   105,
     106,    -1,   108,   109,    -1,   111,    -1,   113,   114,   115,
     116,   117,   118,   119,    -1,   121,   122,   123,    -1,   125,
     126,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,
     146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,   163,    -1,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,   201,   202,   203,   204,   205,
      -1,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,    99,
      -1,    -1,    -1,   103,   104,   105,   106,    -1,   108,   109,
      -1,   111,    -1,   113,   114,   115,   116,   117,   118,   119,
      -1,   121,   122,   123,    -1,   125,   126,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
      -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,
      -1,   151,   152,   153,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,   163,    -1,    -1,   166,    -1,    -1,   169,
      -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   199,
      -1,   201,   202,   203,   204,   205,    -1,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    82,    -1,
      -1,    -1,    86,    87,    88,    89,    -1,    91,    92,    93,
      -1,    95,    -1,    -1,    98,    99,    -1,    -1,    -1,   103,
     104,   105,   106,    -1,   108,   109,    -1,   111,    -1,   113,
     114,   115,   116,   117,   118,   119,    -1,   121,   122,   123,
      -1,   125,   126,    -1,    -1,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,
      -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,   163,
      -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,   201,   202,    -1,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    99,    -1,    -1,    -1,   103,   104,   105,   106,    -1,
     108,   109,    -1,   111,    -1,   113,   114,   115,   116,   117,
     118,   119,    -1,   121,   122,   123,    -1,   125,   126,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,
      -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,   163,    -1,    -1,   166,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,   201,   202,   203,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    95,    -1,    -1,    98,    99,    -1,    -1,
      -1,   103,   104,   105,   106,    -1,   108,   109,    -1,   111,
      -1,   113,   114,   115,   116,   117,   118,   119,    -1,   121,
     122,   123,    -1,   125,   126,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,    -1,    -1,
      -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,
     152,   153,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,   163,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,
      -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    -1,   201,
     202,   203,   204,   205,    -1,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,
      86,    87,    88,    89,    -1,    91,    -1,    93,    -1,    95,
      -1,    -1,    98,    99,    -1,    -1,    -1,   103,   104,   105,
     106,    -1,   108,   109,    -1,   111,    -1,   113,   114,   115,
     116,   117,   118,   119,    -1,   121,   122,   123,    -1,   125,
     126,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,
     146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,   163,    -1,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,   201,   202,   203,   204,   205,
      -1,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,    99,
      -1,    -1,    -1,   103,   104,   105,   106,    -1,   108,   109,
      -1,   111,    -1,   113,   114,   115,   116,   117,   118,   119,
      -1,   121,   122,   123,    -1,   125,   126,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
      -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,
      -1,   151,   152,   153,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,   163,    -1,    -1,   166,    -1,    -1,   169,
      -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   199,
      -1,   201,   202,    -1,   204,   205,    -1,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    30,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    82,    -1,
      -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,    93,
      -1,    95,    -1,    -1,    98,    99,    -1,    -1,    -1,   103,
     104,   105,   106,    -1,   108,   109,    -1,   111,    -1,   113,
     114,   115,   116,   117,   118,   119,    -1,   121,   122,   123,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,
      -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,   201,   202,    -1,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    82,    -1,    -1,    -1,    86,    87,
      88,    89,    -1,    91,    -1,    93,    -1,    95,    -1,    -1,
      98,    99,    -1,    -1,    -1,   103,   104,   105,   106,    -1,
     108,   109,    -1,   111,    -1,   113,   114,   115,   116,   117,
     118,   119,    -1,   121,   122,   123,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,
      -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    -1,   166,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,   201,   202,    -1,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    30,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      82,    -1,    -1,    -1,    86,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    95,    -1,    -1,    98,    99,    -1,    -1,
      -1,   103,   104,   105,   106,    -1,   108,   109,    -1,   111,
      -1,   113,   114,   115,   116,   117,   118,   119,    -1,   121,
     122,   123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,    -1,    -1,
      -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,
     152,   153,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,    -1,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,
      -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    -1,   201,
     202,    -1,   204,   205,    -1,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    30,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    82,    -1,    -1,    -1,
      86,    87,    88,    89,    -1,    91,    -1,    93,    -1,    95,
      -1,    -1,    98,    99,    -1,    -1,    -1,   103,   104,   105,
     106,    -1,   108,   109,    -1,   111,    -1,   113,   114,   115,
     116,   117,   118,   119,    -1,   121,   122,   123,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,
     146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,   201,   202,    -1,   204,   205,
      -1,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    82,    -1,    -1,    -1,    86,    87,    88,    89,
      -1,    91,    -1,    93,    -1,    95,    -1,    -1,    98,    99,
      -1,    -1,    -1,   103,   104,   105,   106,    -1,   108,   109,
      -1,   111,    -1,   113,   114,   115,   116,   117,   118,   119,
      -1,   121,   122,   123,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
      -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,
      -1,   151,   152,   153,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    -1,   166,    -1,    -1,   169,
      -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   199,
      -1,   201,   202,    -1,   204,   205,    -1,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    82,    -1,
      -1,    -1,    86,    87,    88,    89,    -1,    91,    -1,    93,
      -1,    95,    -1,    -1,    98,    99,    -1,    -1,    -1,   103,
     104,   105,   106,    -1,   108,   109,    -1,   111,    -1,   113,
     114,   115,   116,   117,   118,   119,    -1,   121,   122,   123,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,
      -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,   201,   202,    -1,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   114,   115,   116,   117,
     118,   119,    -1,    -1,   122,   123,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,
      -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    -1,   166,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,
      -1,    -1,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,    -1,    -1,    -1,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   114,   115,   116,   117,   118,   119,    -1,    -1,
     122,   123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,    -1,    -1,
      -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,
     152,   153,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,    -1,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,
      -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    -1,   201,
      -1,    -1,   204,   205,    -1,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    29,    13,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    35,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    65,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,   115,
     116,   117,   118,   119,    -1,    -1,   122,   123,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,
     146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,    -1,   164,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,    -1,    -1,    -1,   204,   205,
      -1,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   114,   115,   116,   117,   118,   119,
      -1,    -1,   122,   123,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
      -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,
      -1,   151,   152,   153,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    -1,   166,    -1,    -1,   169,
      -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   199,
      11,    12,   202,    -1,   204,   205,    -1,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    29,    13,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    35,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    65,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     114,   115,   116,   117,   118,   119,    -1,    -1,   122,   123,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,
      -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,    -1,
     164,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,    -1,    -1,    -1,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   114,   115,   116,   117,
     118,   119,    -1,    -1,   122,   123,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,
      -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    -1,   166,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,    10,    11,    12,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    65,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,   103,    -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   114,   115,   116,   117,   118,   119,    -1,    -1,
     122,   123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,    -1,    -1,
      -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,
     152,   153,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,    -1,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,
      -1,    -1,    -1,   175,   191,   192,    -1,    -1,   180,   181,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    11,    12,
      -1,    -1,   204,   205,    -1,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    29,    13,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    65,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,   115,
     116,   117,   118,   119,    -1,    -1,   122,   123,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,
     146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,    10,    11,    12,   204,   205,
      -1,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,
      65,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   114,   115,   116,   117,   118,   119,
      -1,    -1,   122,   123,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
      -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,
      -1,   151,   152,   153,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    -1,   166,    -1,    -1,   169,
      -1,    -1,    -1,   188,    -1,   175,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   199,
      -1,   201,    -1,    12,   204,   205,    -1,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    65,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     114,   115,   116,   117,   118,   119,    -1,    -1,   122,   123,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,
      -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,   201,    -1,    12,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    65,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   114,   115,   116,   117,
     118,   119,    -1,    -1,   122,   123,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,
      -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    -1,   166,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,   200,    -1,    -1,    -1,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    30,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   114,   115,   116,   117,   118,   119,    -1,    -1,
     122,   123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,    -1,    -1,
      -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,
     152,   153,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,    -1,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,
      -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    -1,    -1,
      -1,    -1,   204,   205,    -1,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    29,    13,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    65,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,   115,
     116,   117,   118,   119,    -1,    -1,   122,   123,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,
     146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
     166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,    -1,    -1,    -1,   204,   205,
      -1,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    35,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    65,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   114,   115,   116,   117,   118,   119,
      -1,    -1,   122,   123,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,   135,   136,   137,   138,   139,
      -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,
      -1,   151,   152,   153,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    -1,   166,    -1,    -1,   169,
      -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   199,
      -1,    -1,    -1,    -1,   204,   205,    -1,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    65,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     114,   115,   116,   117,   118,   119,    -1,    -1,   122,   123,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,
      -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,    -1,
      -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,   153,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      -1,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,    -1,    -1,    -1,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    35,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      65,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   114,   115,   116,   117,
     118,   119,    -1,    -1,   122,   123,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   132,   133,    -1,   135,   136,   137,
     138,   139,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,
      -1,    -1,    -1,   151,   152,   153,   154,   155,    -1,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    -1,   166,    -1,
      -1,   169,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,    10,    11,    12,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    65,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   114,   115,   116,   117,   118,   119,    -1,    -1,
     122,   123,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     132,   133,    -1,   135,   136,   137,   138,   139,    -1,    -1,
      -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,
     152,   153,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,    -1,    -1,    -1,   166,    -1,    -1,   169,    -1,    -1,
     187,    -1,    -1,   175,    -1,    -1,    -1,    -1,   180,   181,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    -1,    10,
      11,    12,   204,   205,    -1,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    65,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,   115,
     116,   117,   118,   119,    -1,    -1,   122,   123,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   132,   133,    -1,   135,
     136,   137,   138,   139,    -1,    -1,    -1,    -1,    -1,    -1,
     146,    -1,    -1,    -1,    -1,   151,   152,   153,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
     166,    -1,    -1,   169,    -1,   186,    -1,    -1,    -1,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    10,    11,    12,    -1,   204,   205,
      -1,   207,   208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
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
      -1,    65,    -1,    -1,    -1,    -1,    -1,   203,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,   203,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,   203,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,   203,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,
      -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    67,    68,
      69,    70,    71,    72,    73,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   203,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
      -1,   203,    -1,   132,   133,    -1,   135,   136,   137,   138,
     139,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   151,   152,   153,    -1,    -1,    -1,   157,   158,
      -1,   160,   161,   162,   163,    -1,   165,   166,    -1,   168,
      -1,    -1,    -1,    -1,    -1,    -1,   175,   176,    -1,   178,
      -1,   180,   181,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,   201,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,   201,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,   201,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
     201,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,   201,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,   201,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    29,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,   200,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    29,
      53,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,   200,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,    -1,    55,    -1,    -1,    -1,    -1,
     106,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,   115,
     116,   117,   118,   119,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,   134,   200,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    -1,    29,    -1,    -1,   151,   106,    -1,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,    55,   175,
      -1,    -1,    -1,   133,   134,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      77,   151,    -1,   199,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    98,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    55,    -1,    -1,   199,
      -1,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    -1,   151,    -1,    -1,   154,   155,    -1,
     157,   158,    -1,   160,   161,   162,    -1,   164,    -1,    98,
      -1,    -1,    -1,    29,    -1,    -1,    -1,    -1,   175,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    55,
      -1,    -1,   199,    -1,   133,   134,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    77,   151,    -1,    -1,   154,   155,    -1,   157,   158,
      -1,   160,   161,   162,    -1,   164,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,    -1,    55,   175,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    77,    -1,    -1,
     199,    -1,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   175,
     176,    -1,    -1,   133,   134,    -1,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      -1,   151,    -1,   199,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,
      30,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    46,    47,    -1,   199,
      -1,    -1,    52,    -1,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    77,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    65,   133,    -1,   135,   136,   137,   138,   139,
      -1,    -1,    -1,    -1,    98,    -1,   146,    -1,    -1,    -1,
      -1,   151,   152,   153,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,    -1,    -1,   119,   166,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,   132,    -1,
     180,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    77,   151,    79,   199,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    46,    47,   199,    -1,    -1,    -1,    52,
     204,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    -1,   157,   158,    -1,   160,
     161,   162,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,    -1,    -1,    -1,    -1,
      -1,   202,    -1,   204,    -1,    -1,    -1,    -1,    -1,    -1,
     133,    -1,   135,   136,   137,   138,   139,    -1,    -1,    -1,
      -1,    -1,    -1,   146,    -1,    -1,    -1,    -1,   151,   152,
     153,   154,   155,    -1,   157,   158,    -1,   160,   161,   162,
      -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   175,    46,    47,    -1,    -1,   180,    -1,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    66,    -1,    -1,   199,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    67,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    79,    65,    -1,    -1,    -1,
     133,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   146,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   106,    -1,   157,   158,    -1,   160,   161,   162,
     114,   115,   116,   117,   118,   119,    -1,    67,    68,    -1,
      -1,    -1,   175,    -1,    -1,    -1,    -1,    77,    -1,    79,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,    -1,    -1,   151,    98,    -1,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,   119,
      -1,    -1,    -1,    -1,    -1,    77,    -1,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    98,    -1,    -1,    -1,
     204,   151,    -1,    -1,   154,   155,    -1,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    -1,    -1,    -1,    -1,   169,
      -1,    -1,    -1,    -1,    -1,    77,    -1,    79,    -1,    -1,
      -1,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    98,    -1,    -1,   199,
      -1,    -1,   154,    -1,   204,   157,   158,    -1,   160,   161,
     162,    -1,    -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    98,    -1,    -1,    -1,    -1,   151,
     202,    -1,   154,   155,    -1,   157,   158,    -1,   160,   161,
     162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    79,    -1,    -1,    -1,    -1,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    98,    -1,    -1,   199,    -1,    -1,
     202,   155,   204,   157,   158,   159,   160,   161,   162,    -1,
      -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,   132,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    98,    -1,   199,    -1,   151,    -1,    -1,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,    -1,
      -1,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    98,    -1,    -1,   199,    -1,    -1,    -1,    -1,
     204,    -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,
      -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    79,    -1,   132,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,    98,   199,   151,    -1,    -1,   154,   155,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    79,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      -1,    -1,    -1,   199,    98,    -1,   151,    -1,   204,   154,
     155,    -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,    -1,   199,    -1,    -1,   151,    -1,   204,
     154,   155,    -1,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    77,    98,    79,   199,    -1,    -1,    -1,    -1,
     204,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    77,   119,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,   157,   158,    98,   160,   161,   162,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,   154,
     155,    -1,   157,   158,    -1,   160,   161,   162,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    77,    -1,    -1,   199,    -1,    -1,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   155,    98,   157,   158,    -1,   160,   161,   162,   204,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    65,    -1,    -1,    -1,    -1,    -1,   202,    -1,
     204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      -1,    -1,    29,   199,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   131,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   131,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,   131,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   131,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,   131,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
     131,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    65,    -1,    10,    11,    12,
      -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    29,   131,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    77,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    77,    -1,    -1,    -1,    -1,    -1,
      98,   131,    -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,
     154,    -1,    -1,   157,   158,    98,   160,   161,   162,    -1,
      -1,    -1,   120,   106,   107,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   133,   134,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    77,   151,    -1,    -1,   154,   155,   131,   157,
     158,    -1,   160,   161,   162,    -1,    -1,    -1,    -1,    -1,
      -1,   154,    -1,    98,   157,   158,    -1,   160,   161,   162,
      -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    77,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    77,    -1,    79,    80,    -1,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,   154,
     155,    -1,   157,   158,    98,   160,   161,   162,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    77,    -1,    -1,    -1,    -1,   155,    98,   157,   158,
      -1,   160,   161,   162,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,   157,   158,    -1,   160,   161,   162,    -1,
      -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    77,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,   154,    -1,    -1,   157,   158,    98,   160,
     161,   162,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      77,    -1,    -1,    -1,   154,    -1,    -1,   157,   158,    -1,
     160,   161,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    77,    -1,   125,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     157,   158,    98,   160,   161,   162,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   125,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   157,   158,    -1,   160,
     161,   162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   157,   158,    -1,   160,   161,   162,    -1,    -1,    -1,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    29,
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
       0,   210,   211,     0,   212,     3,     4,     5,     6,     7,
      13,    27,    28,    45,    46,    47,    52,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    66,    67,
      68,    69,    70,    74,    75,    76,    77,    78,    79,    81,
      82,    86,    87,    88,    89,    91,    93,    95,    98,    99,
     103,   104,   105,   106,   107,   108,   109,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   121,   122,   123,   124,
     125,   126,   132,   133,   135,   136,   137,   138,   139,   146,
     151,   152,   153,   154,   155,   157,   158,   160,   161,   162,
     163,   166,   169,   175,   176,   178,   180,   181,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   199,   201,   202,   204,   205,   207,   208,   213,
     216,   223,   224,   225,   226,   227,   228,   231,   247,   248,
     252,   255,   260,   266,   325,   326,   334,   338,   339,   340,
     341,   342,   343,   344,   345,   347,   350,   362,   363,   364,
     366,   367,   369,   388,   398,   399,   400,   405,   408,   427,
     435,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   448,   461,   463,   465,   117,   118,   119,   132,   151,
     161,   216,   247,   325,   344,   437,   344,   199,   344,   344,
     344,   103,   344,   344,   425,   426,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,    79,   119,
     199,   224,   399,   400,   437,   440,   437,    35,   344,   452,
     453,   344,   119,   199,   224,   399,   400,   401,   436,   444,
     449,   450,   199,   335,   402,   199,   335,   351,   336,   344,
     233,   335,   199,   199,   199,   335,   201,   344,   216,   201,
     344,    29,    55,   133,   134,   155,   175,   199,   216,   227,
     466,   479,   480,   482,   182,   201,   341,   344,   368,   370,
     202,   240,   344,   106,   107,   154,   217,   220,   223,    79,
     204,   292,   293,   118,   125,   117,   125,    79,   294,   199,
     199,   199,   199,   216,   264,   467,   199,   199,    79,    85,
     147,   148,   149,   458,   459,   154,   202,   223,   223,   216,
     265,   467,   155,   199,   467,   467,    79,   196,   202,   353,
     334,   344,   345,   437,   441,   229,   202,    85,   403,   458,
      85,   458,   458,    30,   154,   171,   468,   199,     9,   201,
      35,   246,   155,   263,   467,   119,   181,   247,   326,   201,
     201,   201,   201,   201,   201,    10,    11,    12,    29,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      53,    65,   201,    66,    66,   201,   202,   150,   126,   161,
     163,   176,   178,   266,   324,   325,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    63,
      64,   129,   130,   429,    66,   202,   434,   199,   199,    66,
     202,   204,   445,   199,   246,   247,    14,   344,   201,   131,
      44,   216,   424,   199,   334,   437,   441,   150,   437,   131,
     206,     9,   410,   334,   437,   468,   150,   199,   404,   429,
     434,   200,   344,    30,   231,     8,   356,     9,   201,   231,
     232,   336,   337,   344,   216,   278,   235,   201,   201,   201,
     482,   482,   171,   199,   106,   482,    14,   150,   216,    79,
     201,   201,   201,   182,   183,   184,   189,   190,   193,   194,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   383,
     384,   385,   241,   110,   168,   201,   154,   218,   221,   223,
     154,   219,   222,   223,   223,     9,   201,    97,   202,   437,
       9,   201,   125,   125,    14,     9,   201,   437,   462,   462,
     334,   345,   437,   440,   441,   200,   171,   258,   132,   437,
     451,   452,    66,   429,   147,   459,    78,   344,   437,    85,
     147,   459,   223,   215,   201,   202,   253,   261,   389,   391,
      86,   204,   357,   358,   360,   400,   445,   463,    14,    97,
     464,   352,   354,   355,   288,   289,   427,   428,   200,   200,
     200,   200,   203,   230,   231,   248,   255,   260,   427,   344,
     205,   207,   208,   216,   469,   470,   482,    35,   164,   290,
     291,   344,   466,   199,   467,   256,   246,   344,   344,   344,
      30,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   401,   344,   344,   447,   447,   344,   454,
     455,   125,   202,   216,   444,   445,   264,   216,   265,   467,
     467,   263,   247,    27,    35,   338,   341,   344,   368,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   155,   202,   216,   430,   431,   432,   433,   444,   447,
     344,   290,   290,   447,   344,   451,   246,   200,   344,   199,
     423,     9,   410,   334,   200,   216,    35,   344,    35,   344,
     200,   200,   444,   290,   202,   216,   430,   431,   444,   200,
     229,   282,   202,   341,   344,   344,    89,    30,   231,   276,
     201,    28,    97,    14,     9,   200,    30,   202,   279,   482,
      86,   227,   476,   477,   478,   199,     9,    46,    47,    52,
      54,    66,    86,   133,   146,   155,   175,   199,   224,   225,
     227,   365,   399,   405,   406,   407,   216,   481,   185,    79,
     344,    79,    79,   344,   380,   381,   344,   344,   373,   383,
     188,   386,   229,   199,   239,   223,   201,     9,    97,   223,
     201,     9,    97,    97,   220,   216,   344,   293,   406,    79,
       9,   200,   200,   200,   200,   200,   200,   200,   201,    46,
      47,   474,   475,   127,   269,   199,     9,   200,   200,    79,
      80,   216,   460,   216,    66,   203,   203,   212,   214,    30,
     128,   268,   170,    50,   155,   170,   393,   131,     9,   410,
     200,   150,   482,   482,    14,   356,   288,   229,   197,     9,
     411,   482,   483,   429,   434,   203,     9,   410,   172,   437,
     344,   200,     9,   411,    14,   348,   249,   127,   267,   199,
     467,   344,    30,   206,   206,   131,   203,     9,   410,   344,
     468,   199,   259,   254,   262,    14,   464,   257,   246,    68,
     437,   344,   468,   206,   203,   200,   200,   206,   203,   200,
      46,    47,    66,    74,    75,    76,    86,   133,   146,   175,
     216,   413,   415,   416,   419,   422,   216,   437,   437,   131,
     429,   434,   200,   344,   283,    71,    72,   284,   229,   335,
     229,   337,    97,    35,   132,   273,   437,   406,   216,    30,
     231,   277,   201,   280,   201,   280,     9,   172,   131,   150,
       9,   410,   200,   164,   469,   470,   471,   469,   406,   406,
     406,   406,   406,   409,   412,   199,    85,   150,   199,   406,
     150,   202,    10,    11,    12,    29,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    65,   150,   468,
     344,   185,   185,    14,   191,   192,   382,     9,   195,   386,
      79,   203,   399,   202,   243,    97,   221,   216,    97,   222,
     216,   216,   203,    14,   437,   201,     9,   172,   216,   270,
     399,   202,   451,   132,   437,    14,   206,   344,   203,   212,
     482,   270,   202,   392,    14,   344,   357,   216,   201,   482,
     197,   203,    30,   472,   428,    35,    79,   164,   430,   431,
     433,   482,    35,   164,   344,   406,   288,   199,   399,   268,
     349,   250,   344,   344,   344,   203,   199,   290,   269,    30,
     268,   482,    14,   267,   467,   401,   203,   199,    14,    74,
      75,    76,   216,   414,   414,   416,   417,   418,    48,   199,
      85,   147,   199,     9,   410,   200,   423,    35,   344,   430,
     431,   203,    71,    72,   285,   335,   231,   203,   201,    90,
     201,   273,   437,   199,   131,   272,    14,   229,   280,   100,
     101,   102,   280,   203,   482,   482,   216,   476,     9,   200,
     410,   131,   206,     9,   410,   409,   216,   357,   359,   361,
     200,   125,   216,   406,   456,   457,   406,   406,   406,    30,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   406,   406,   406,   406,   406,   406,   406,
     406,   406,   406,   481,   344,   344,   344,   381,   344,   371,
      79,   244,   216,   216,   406,   475,    97,    98,   473,     9,
     298,   200,   199,   338,   341,   344,   206,   203,   464,   298,
     156,   169,   202,   388,   395,   156,   202,   394,   131,   201,
     472,   482,   356,   483,    79,   164,    14,    79,   468,   437,
     344,   200,   288,   202,   288,   199,   131,   199,   290,   200,
     202,   482,   202,   201,   482,   268,   251,   404,   290,   131,
     206,     9,   410,   415,   417,   147,   357,   420,   421,   416,
     437,   335,    30,    73,   231,   201,   337,   272,   451,   273,
     200,   406,    96,   100,   201,   344,    30,   201,   281,   203,
     172,   131,   164,    30,   200,   406,   406,   200,   131,     9,
     410,   200,   131,   203,     9,   410,   406,    30,   186,   200,
     229,   216,   482,   482,   399,     4,   107,   112,   118,   120,
     157,   158,   160,   203,   299,   323,   324,   325,   330,   331,
     332,   333,   427,   451,   203,   202,   203,    50,   344,   344,
     344,   356,    35,    79,   164,    14,    79,   344,   199,   472,
     200,   298,   200,   288,   344,   290,   200,   298,   464,   298,
     201,   202,   199,   200,   416,   416,   200,   131,   200,     9,
     410,    30,   229,   201,   200,   200,   200,   236,   201,   201,
     281,   229,   482,   482,   131,   406,   357,   406,   406,   406,
     344,   202,   203,   473,   127,   128,   176,   466,   271,   399,
     107,   333,    29,   120,   133,   134,   155,   161,   308,   309,
     310,   311,   399,   159,   315,   316,   123,   199,   216,   317,
     318,   300,   247,   482,     9,   201,     9,   201,   201,   464,
     324,   200,   295,   155,   390,   203,   203,    79,   164,    14,
      79,   344,   290,   112,   346,   472,   203,   472,   200,   200,
     203,   202,   203,   298,   288,   131,   416,   357,   229,   234,
     237,    30,   231,   275,   229,   200,   406,   131,   131,   187,
     229,   399,   399,   467,    14,     9,   201,   202,   466,   464,
     311,   171,   202,     9,   201,     3,     4,     5,     6,     7,
      10,    11,    12,    13,    27,    28,    53,    67,    68,    69,
      70,    71,    72,    73,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   132,   133,   135,   136,   137,
     138,   139,   151,   152,   153,   163,   165,   166,   168,   175,
     176,   178,   180,   181,   216,   396,   397,     9,   201,   155,
     159,   216,   318,   319,   320,   201,    79,   329,   246,   301,
     466,   466,    14,   247,   203,   296,   297,   466,    14,    79,
     344,   200,   199,   202,   201,   202,   321,   346,   472,   295,
     203,   200,   416,   131,    30,   231,   274,   275,   229,   406,
     406,   344,   203,   201,   201,   406,   399,   304,   482,   312,
     405,   309,    14,    30,    47,   313,   316,     9,    33,   200,
      29,    46,    49,    14,     9,   201,   467,   329,    14,   482,
     246,   201,    14,   344,    35,    79,   387,   229,   229,   202,
     321,   203,   472,   416,   229,    94,   188,   242,   203,   216,
     227,   305,   306,   307,     9,   172,     9,   203,   406,   397,
     397,    55,   314,   319,   319,    29,    46,    49,   406,    79,
     199,   201,   406,   467,   406,    79,     9,   411,   203,   203,
     229,   321,    92,   201,    79,   110,   238,   150,    97,   482,
     405,   162,    14,   302,   199,    35,    79,   200,   203,   201,
     199,   168,   245,   216,   324,   325,   172,   406,   286,   287,
     428,   303,    79,   399,   243,   165,   216,   201,   200,     9,
     411,   114,   115,   116,   327,   328,   286,    79,   271,   201,
     472,   428,   483,   200,   200,   201,   201,   202,   322,   327,
      35,    79,   164,   472,   202,   229,   483,    79,   164,    14,
      79,   322,   229,   203,    35,    79,   164,    14,    79,   344,
     203,    79,   164,    14,    79,   344,    14,    79,   344,   344
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
#line 735 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 738 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 745 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 746 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 749 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 750 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 751 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 752 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 753 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 754 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 755 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 758 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 760 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 761 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 762 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 763 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 764 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 766 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 768 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 769 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 778 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 780 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 781 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 782 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 783 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 798 "hphp.y"
    { ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 799 "hphp.y"
    { ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 804 "hphp.y"
    { ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 805 "hphp.y"
    { ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 810 "hphp.y"
    { ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 811 "hphp.y"
    { ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 815 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 816 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 817 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 819 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 823 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 824 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 825 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 827 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 831 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 832 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 833 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 835 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 846 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 847 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 850 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 857 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 864 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 872 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 875 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 881 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 882 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 885 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 886 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 887 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 888 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 891 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 895 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 900 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 901 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 903 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 907 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 910 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 914 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 916 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 919 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 921 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 924 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 925 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 926 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 928 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 929 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 930 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 931 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 933 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 934 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 935 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 936 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 937 "hphp.y"
    { _p->onHashBang((yyval), (yyvsp[(1) - (1)]));
                                         (yyval) = T_HASHBANG;;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 941 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 943 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 948 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 950 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 954 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 961 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 962 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 965 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 966 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 967 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 968 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval));;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 972 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 973 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 974 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 976 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 977 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 978 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 979 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 980 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 981 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 982 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 990 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 991 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1000 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1001 "hphp.y"
    { (yyval).reset();;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1005 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1013 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1014 "hphp.y"
    { (yyval).reset();;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1018 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1019 "hphp.y"
    { (yyval).reset();;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1023 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1028 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1034 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1040 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1046 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1052 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1058 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1066 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1070 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1074 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1078 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1084 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1087 "hphp.y"
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

  case 146:

/* Line 1455 of yacc.c  */
#line 1102 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1105 "hphp.y"
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

  case 148:

/* Line 1455 of yacc.c  */
#line 1119 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1122 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1127 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1130 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1137 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsClassDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1140 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1148 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsClassDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1151 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1159 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1160 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1164 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1167 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1170 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1171 "hphp.y"
    { (yyval) = T_ABSTRACT; ;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1172 "hphp.y"
    { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      (yyval) = T_STATIC; ;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1175 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = T_STATIC; ;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1176 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1180 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1181 "hphp.y"
    { (yyval).reset();;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1184 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1185 "hphp.y"
    { (yyval).reset();;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1188 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1189 "hphp.y"
    { (yyval).reset();;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1192 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1194 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1197 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1199 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1203 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1204 "hphp.y"
    { (yyval).reset();;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1207 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1208 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1209 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1213 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1215 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1218 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1220 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1223 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1225 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1228 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1230 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1240 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1241 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1242 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1243 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1248 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1250 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1251 "hphp.y"
    { (yyval).reset();;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1254 "hphp.y"
    { (yyval).reset();;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1255 "hphp.y"
    { (yyval).reset();;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1260 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1261 "hphp.y"
    { (yyval).reset();;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1266 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1267 "hphp.y"
    { (yyval).reset();;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1270 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1271 "hphp.y"
    { (yyval).reset();;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1274 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1275 "hphp.y"
    { (yyval).reset();;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1283 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1289 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(8) - (8)]),true,
                                                            &(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)])); ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1295 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1303 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1308 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),true,
                                                            &(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)])); ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1313 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1316 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1322 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1326 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1331 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1336 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1341 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1346 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1352 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1358 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1366 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1371 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]),
                                        true,&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1376 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1380 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1383 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1387 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),
                                                            true,&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1391 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1394 "hphp.y"
    { (yyval).reset();;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1399 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1402 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1406 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1410 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1414 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1418 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1423 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1428 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1434 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1435 "hphp.y"
    { (yyval).reset();;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1438 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1439 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1440 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1442 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1444 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1446 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1450 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1451 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1454 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1455 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1456 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1460 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1462 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1463 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1464 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1469 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1470 "hphp.y"
    { (yyval).reset();;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1473 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1478 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1484 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1485 "hphp.y"
    { (yyval).reset();;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1488 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1489 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1492 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1493 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1495 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1498 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL, true);;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1500 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1503 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1516 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1522 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1527 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1529 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1531 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1533 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1535 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1539 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1542 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1543 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1550 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1557 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1573 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1580 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1582 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1586 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
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
#line 1595 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1596 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1604 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1605 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1615 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1620 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1621 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1625 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1628 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1633 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1638 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1639 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1641 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1645 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1646 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1647 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1652 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1654 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1655 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1658 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1660 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1664 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1667 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1668 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1672 "hphp.y"
    { (yyval).reset();;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1673 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1677 "hphp.y"
    { (yyval).reset();;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1678 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1681 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1682 "hphp.y"
    { (yyval).reset();;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1685 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1686 "hphp.y"
    { (yyval).reset();;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1689 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1694 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1695 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1696 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1697 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1698 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1699 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1700 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1704 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { (yyval).reset();;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1708 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1709 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1710 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1714 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1716 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1717 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1718 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1722 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1724 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1728 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1730 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { Token t;
                                          _p->onClassTypeConstant((yyval), (yyvsp[(2) - (3)]), t);
                                          _p->popTypeScope(); ;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { _p->onClassTypeConstant((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]));
                                          _p->popTypeScope(); ;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]); ;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1745 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1748 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1749 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { (yyval).reset();;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1852 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1858 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1864 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1867 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1868 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1869 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1871 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1872 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1873 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1874 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1875 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1876 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1877 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1884 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { (yyval).reset();;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1890 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(7) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(8) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
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

  case 462:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1968 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
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

  case 466:

/* Line 1455 of yacc.c  */
#line 1986 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1992 "hphp.y"
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

  case 468:

/* Line 1455 of yacc.c  */
#line 2006 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 2007 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 2009 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 2013 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 2015 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 2022 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 2025 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 2032 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 2035 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 2040 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 2041 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 2046 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 2047 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2051 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2055 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2056 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2061 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2068 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2075 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2077 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2081 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2082 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2083 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2084 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2086 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2090 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2094 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2098 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2103 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2105 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2107 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2109 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2115 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
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
#line 2123 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2124 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2128 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2132 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2136 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2141 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2146 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2150 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2154 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2155 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2159 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2160 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2164 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2165 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2169 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2170 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2174 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2178 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2182 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2186 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2187 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2199 "hphp.y"
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

  case 531:

/* Line 1455 of yacc.c  */
#line 2210 "hphp.y"
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

  case 532:

/* Line 1455 of yacc.c  */
#line 2221 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2222 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2227 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { (yyval).reset();;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { (yyval).reset();;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
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
#line 2342 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2343 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2348 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2352 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2353 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { (yyval).reset();;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { (yyval).reset();;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { (yyval).reset();;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { (yyval).reset();;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
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
#line 2520 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2522 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2526 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2528 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,(yyvsp[(1) - (1)]));;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2535 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,
                                         (yyvsp[(1) - (3)]) + (yyvsp[(3) - (3)]));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2544 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2545 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2547 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2550 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { (yyval).reset();;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2565 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2570 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2576 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2577 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2582 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2590 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2591 "hphp.y"
    { (yyval).reset();;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2594 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2615 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2616 "hphp.y"
    { (yyval).reset();;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2621 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2626 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2627 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2631 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2632 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2646 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2647 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2653 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2660 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2664 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2665 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2666 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2667 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
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

  case 783:

/* Line 1455 of yacc.c  */
#line 2681 "hphp.y"
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

  case 784:

/* Line 1455 of yacc.c  */
#line 2692 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2694 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2696 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2697 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2701 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2702 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2703 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2704 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2707 "hphp.y"
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

  case 793:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
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

  case 794:

/* Line 1455 of yacc.c  */
#line 2729 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2730 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2735 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2740 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2744 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2745 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2751 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2755 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2762 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2766 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2770 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2776 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2781 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2782 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2783 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2786 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2787 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2791 "hphp.y"
    { (yyval).reset();;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2795 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2796 "hphp.y"
    { (yyval)++;;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2800 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2804 "hphp.y"
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

  case 821:

/* Line 1455 of yacc.c  */
#line 2815 "hphp.y"
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

  case 822:

/* Line 1455 of yacc.c  */
#line 2826 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2827 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2831 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2834 "hphp.y"
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

  case 827:

/* Line 1455 of yacc.c  */
#line 2846 "hphp.y"
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

  case 828:

/* Line 1455 of yacc.c  */
#line 2855 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2859 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2860 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2862 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2863 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2864 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2865 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2870 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2871 "hphp.y"
    { (yyval).reset();;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2875 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2876 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2877 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2878 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2881 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2883 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2884 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2885 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2890 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2891 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2895 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2896 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2897 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2898 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2903 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2904 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2909 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2911 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2913 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2914 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2918 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2920 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2928 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2930 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2932 "hphp.y"
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

  case 864:

/* Line 1455 of yacc.c  */
#line 2942 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2944 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2945 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2948 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2949 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2950 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2954 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2955 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2956 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2957 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2958 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2959 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2961 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2962 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2963 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2964 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2968 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2969 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2974 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2976 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2990 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsClassDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2995 "hphp.y"
    { (yyvsp[(3) - (6)]).setText(_p->nsClassDecl((yyvsp[(3) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2999 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsClassDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 3004 "hphp.y"
    { (yyvsp[(3) - (7)]).setText(_p->nsClassDecl((yyvsp[(3) - (7)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(3) - (7)]), (yyvsp[(6) - (7)]));
                                         _p->popTypeScope(); ;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 3010 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 3011 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 3017 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 3021 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 3027 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 3028 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 3032 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 3035 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 3041 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 3046 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 3047 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 3048 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 3049 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 3053 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 3054 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 3063 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 3064 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 3067 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (5)]).text()); ;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 3069 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (3)]).text()); ;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 3073 "hphp.y"
    {;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 3074 "hphp.y"
    {;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3075 "hphp.y"
    {;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3081 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3086 "hphp.y"
    { ;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3098 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3100 "hphp.y"
    {;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3104 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3108 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3111 "hphp.y"
    { Token t; t.reset();
                                          _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), t);
                                          _p->onTypeList((yyval), (yyvsp[(3) - (3)])); ;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3114 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3121 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3124 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3127 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3128 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3131 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 930:

/* Line 1455 of yacc.c  */
#line 3134 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 931:

/* Line 1455 of yacc.c  */
#line 3137 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         _p->onTypeSpecialization((yyval), 'a'); ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3141 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3144 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3147 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3153 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3159 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3167 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3168 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13850 "hphp.tab.cpp"
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
#line 3171 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

