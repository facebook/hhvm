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
#define YYLAST   16885

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  211
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  280
/* YYNRULES -- Number of rules.  */
#define YYNRULES  947
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1779

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
     980,   981,   982,   987,   988,   994,   997,  1000,  1001,  1012,
    1013,  1025,  1029,  1033,  1037,  1042,  1047,  1051,  1057,  1060,
    1063,  1064,  1071,  1077,  1082,  1086,  1088,  1090,  1094,  1099,
    1101,  1103,  1108,  1115,  1117,  1119,  1124,  1126,  1128,  1132,
    1135,  1136,  1139,  1140,  1142,  1146,  1148,  1150,  1152,  1154,
    1158,  1163,  1168,  1173,  1175,  1177,  1180,  1183,  1186,  1190,
    1194,  1196,  1198,  1200,  1202,  1206,  1208,  1212,  1214,  1216,
    1218,  1219,  1221,  1224,  1226,  1228,  1230,  1232,  1234,  1236,
    1238,  1240,  1241,  1243,  1245,  1247,  1251,  1257,  1259,  1263,
    1269,  1274,  1278,  1282,  1286,  1290,  1293,  1295,  1297,  1301,
    1305,  1307,  1309,  1310,  1312,  1315,  1320,  1324,  1331,  1334,
    1338,  1345,  1347,  1349,  1351,  1353,  1355,  1362,  1366,  1371,
    1378,  1382,  1386,  1390,  1394,  1398,  1402,  1406,  1410,  1414,
    1418,  1422,  1426,  1429,  1432,  1435,  1438,  1442,  1446,  1450,
    1454,  1458,  1462,  1466,  1470,  1474,  1478,  1482,  1486,  1490,
    1494,  1498,  1502,  1506,  1509,  1512,  1515,  1518,  1522,  1526,
    1530,  1534,  1538,  1542,  1546,  1550,  1554,  1558,  1564,  1569,
    1571,  1574,  1577,  1580,  1583,  1586,  1589,  1592,  1595,  1598,
    1600,  1602,  1604,  1606,  1608,  1612,  1615,  1617,  1623,  1624,
    1625,  1637,  1638,  1651,  1652,  1656,  1657,  1662,  1663,  1670,
    1671,  1679,  1680,  1686,  1689,  1692,  1697,  1699,  1701,  1707,
    1711,  1717,  1721,  1724,  1725,  1728,  1729,  1734,  1739,  1743,
    1748,  1753,  1758,  1763,  1768,  1773,  1778,  1783,  1788,  1793,
    1795,  1797,  1799,  1801,  1805,  1808,  1812,  1817,  1820,  1824,
    1826,  1829,  1831,  1834,  1836,  1838,  1840,  1842,  1844,  1846,
    1851,  1856,  1859,  1868,  1879,  1882,  1884,  1888,  1890,  1893,
    1895,  1897,  1899,  1901,  1904,  1909,  1913,  1917,  1922,  1924,
    1927,  1932,  1935,  1942,  1943,  1945,  1950,  1951,  1954,  1955,
    1957,  1959,  1963,  1965,  1969,  1971,  1973,  1977,  1981,  1983,
    1985,  1987,  1989,  1991,  1993,  1995,  1997,  1999,  2001,  2003,
    2005,  2007,  2009,  2011,  2013,  2015,  2017,  2019,  2021,  2023,
    2025,  2027,  2029,  2031,  2033,  2035,  2037,  2039,  2041,  2043,
    2045,  2047,  2049,  2051,  2053,  2055,  2057,  2059,  2061,  2063,
    2065,  2067,  2069,  2071,  2073,  2075,  2077,  2079,  2081,  2083,
    2085,  2087,  2089,  2091,  2093,  2095,  2097,  2099,  2101,  2103,
    2105,  2107,  2109,  2111,  2113,  2115,  2117,  2119,  2121,  2123,
    2125,  2127,  2129,  2131,  2133,  2135,  2137,  2139,  2141,  2146,
    2148,  2150,  2152,  2154,  2156,  2158,  2160,  2162,  2165,  2167,
    2168,  2169,  2171,  2173,  2177,  2178,  2180,  2182,  2184,  2186,
    2188,  2190,  2192,  2194,  2196,  2198,  2200,  2202,  2204,  2208,
    2211,  2213,  2215,  2220,  2224,  2229,  2231,  2233,  2235,  2237,
    2241,  2245,  2249,  2253,  2257,  2261,  2265,  2269,  2273,  2277,
    2281,  2285,  2289,  2293,  2297,  2301,  2305,  2309,  2312,  2315,
    2318,  2321,  2325,  2329,  2333,  2337,  2341,  2345,  2349,  2353,
    2359,  2364,  2368,  2372,  2376,  2378,  2380,  2382,  2384,  2388,
    2392,  2396,  2399,  2400,  2402,  2403,  2405,  2406,  2412,  2416,
    2420,  2422,  2424,  2426,  2428,  2430,  2434,  2437,  2439,  2441,
    2443,  2445,  2447,  2449,  2452,  2455,  2460,  2464,  2469,  2472,
    2473,  2479,  2483,  2487,  2489,  2493,  2495,  2498,  2499,  2505,
    2509,  2512,  2513,  2517,  2518,  2523,  2526,  2527,  2531,  2535,
    2537,  2538,  2540,  2542,  2544,  2546,  2550,  2552,  2554,  2556,
    2560,  2562,  2564,  2568,  2572,  2575,  2580,  2583,  2588,  2590,
    2592,  2594,  2596,  2598,  2602,  2608,  2612,  2617,  2622,  2626,
    2628,  2630,  2632,  2634,  2638,  2644,  2649,  2653,  2655,  2657,
    2661,  2665,  2667,  2669,  2677,  2687,  2695,  2702,  2711,  2713,
    2716,  2721,  2726,  2728,  2730,  2735,  2737,  2738,  2740,  2743,
    2745,  2747,  2751,  2757,  2761,  2765,  2766,  2768,  2772,  2778,
    2782,  2785,  2789,  2796,  2797,  2799,  2804,  2807,  2808,  2814,
    2818,  2822,  2824,  2831,  2836,  2841,  2844,  2847,  2848,  2854,
    2858,  2862,  2864,  2867,  2868,  2874,  2878,  2882,  2884,  2887,
    2888,  2891,  2892,  2898,  2902,  2906,  2908,  2911,  2912,  2915,
    2916,  2922,  2926,  2930,  2932,  2935,  2938,  2940,  2943,  2945,
    2950,  2954,  2958,  2965,  2969,  2971,  2973,  2975,  2980,  2985,
    2990,  2995,  3000,  3005,  3008,  3011,  3016,  3019,  3022,  3024,
    3028,  3032,  3036,  3037,  3040,  3046,  3053,  3055,  3058,  3060,
    3065,  3069,  3070,  3072,  3076,  3079,  3083,  3085,  3087,  3088,
    3089,  3092,  3097,  3100,  3107,  3112,  3114,  3116,  3117,  3121,
    3127,  3131,  3133,  3136,  3137,  3142,  3145,  3148,  3150,  3152,
    3154,  3156,  3161,  3168,  3170,  3179,  3186,  3188
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     212,     0,    -1,    -1,   213,   214,    -1,   214,   215,    -1,
      -1,   233,    -1,   250,    -1,   257,    -1,   254,    -1,   262,
      -1,   475,    -1,   123,   201,   202,   203,    -1,   150,   225,
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
     478,    -1,   226,   478,    -1,   230,     9,   476,    14,   409,
      -1,   106,   476,    14,   409,    -1,   231,   232,    -1,    -1,
     233,    -1,   250,    -1,   257,    -1,   262,    -1,   204,   231,
     205,    -1,    70,   334,   233,   284,   286,    -1,    70,   334,
      30,   231,   285,   287,    73,   203,    -1,    -1,    89,   334,
     234,   278,    -1,    -1,    88,   235,   233,    89,   334,   203,
      -1,    -1,    91,   201,   336,   203,   336,   203,   336,   202,
     236,   276,    -1,    -1,    98,   334,   237,   281,    -1,   102,
     203,    -1,   102,   343,   203,    -1,   104,   203,    -1,   104,
     343,   203,    -1,   107,   203,    -1,   107,   343,   203,    -1,
      27,   102,   203,    -1,   112,   294,   203,    -1,   118,   296,
     203,    -1,    87,   335,   203,    -1,   120,   201,   472,   202,
     203,    -1,   203,    -1,    81,    -1,    82,    -1,    -1,    93,
     201,   343,    97,   275,   274,   202,   238,   277,    -1,    -1,
      93,   201,   343,    28,    97,   275,   274,   202,   239,   277,
      -1,    95,   201,   280,   202,   279,    -1,    -1,   108,   242,
     109,   201,   402,    79,   202,   204,   231,   205,   244,   240,
     247,    -1,    -1,   108,   242,   167,   241,   245,    -1,   110,
     343,   203,    -1,   103,   218,   203,    -1,   343,   203,    -1,
     337,   203,    -1,   338,   203,    -1,   339,   203,    -1,   340,
     203,    -1,   341,   203,    -1,   107,   340,   203,    -1,   342,
     203,    -1,   372,   203,    -1,   107,   371,   203,    -1,   218,
      30,    -1,    -1,   204,   243,   231,   205,    -1,   244,   109,
     201,   402,    79,   202,   204,   231,   205,    -1,    -1,    -1,
     204,   246,   231,   205,    -1,   167,   245,    -1,    -1,    35,
      -1,    -1,   105,    -1,    -1,   249,   248,   477,   251,   201,
     290,   202,   482,   322,    -1,    -1,   326,   249,   248,   477,
     252,   201,   290,   202,   482,   322,    -1,    -1,   429,   325,
     249,   248,   477,   253,   201,   290,   202,   482,   322,    -1,
      -1,   160,   218,   255,    30,   489,   474,   204,   297,   205,
      -1,    -1,   429,   160,   218,   256,    30,   489,   474,   204,
     297,   205,    -1,    -1,   268,   265,   258,   269,   270,   204,
     300,   205,    -1,    -1,   429,   268,   265,   259,   269,   270,
     204,   300,   205,    -1,    -1,   125,   266,   260,   271,   204,
     300,   205,    -1,    -1,   429,   125,   266,   261,   271,   204,
     300,   205,    -1,    -1,   162,   267,   263,   270,   204,   300,
     205,    -1,    -1,   429,   162,   267,   264,   270,   204,   300,
     205,    -1,   477,    -1,   154,    -1,   477,    -1,   477,    -1,
     124,    -1,   117,   124,    -1,   117,   116,   124,    -1,   116,
     117,   124,    -1,   116,   124,    -1,   126,   402,    -1,    -1,
     127,   272,    -1,    -1,   126,   272,    -1,    -1,   402,    -1,
     272,     9,   402,    -1,   402,    -1,   273,     9,   402,    -1,
     130,   275,    -1,    -1,   439,    -1,    35,   439,    -1,   131,
     201,   453,   202,    -1,   233,    -1,    30,   231,    92,   203,
      -1,   233,    -1,    30,   231,    94,   203,    -1,   233,    -1,
      30,   231,    90,   203,    -1,   233,    -1,    30,   231,    96,
     203,    -1,   218,    14,   409,    -1,   280,     9,   218,    14,
     409,    -1,   204,   282,   205,    -1,   204,   203,   282,   205,
      -1,    30,   282,    99,   203,    -1,    30,   203,   282,    99,
     203,    -1,   282,   100,   343,   283,   231,    -1,   282,   101,
     283,   231,    -1,    -1,    30,    -1,   203,    -1,   284,    71,
     334,   233,    -1,    -1,   285,    71,   334,    30,   231,    -1,
      -1,    72,   233,    -1,    -1,    72,    30,   231,    -1,    -1,
     289,     9,   430,   328,   490,   163,    79,    -1,   289,     9,
     430,   328,   490,    35,   163,    79,    -1,   289,     9,   430,
     328,   490,   163,    -1,   289,   414,    -1,   430,   328,   490,
     163,    79,    -1,   430,   328,   490,    35,   163,    79,    -1,
     430,   328,   490,   163,    -1,    -1,   430,   328,   490,    79,
      -1,   430,   328,   490,    35,    79,    -1,   430,   328,   490,
      35,    79,    14,   343,    -1,   430,   328,   490,    79,    14,
     343,    -1,   289,     9,   430,   328,   490,    79,    -1,   289,
       9,   430,   328,   490,    35,    79,    -1,   289,     9,   430,
     328,   490,    35,    79,    14,   343,    -1,   289,     9,   430,
     328,   490,    79,    14,   343,    -1,   291,     9,   430,   490,
     163,    79,    -1,   291,     9,   430,   490,    35,   163,    79,
      -1,   291,     9,   430,   490,   163,    -1,   291,   414,    -1,
     430,   490,   163,    79,    -1,   430,   490,    35,   163,    79,
      -1,   430,   490,   163,    -1,    -1,   430,   490,    79,    -1,
     430,   490,    35,    79,    -1,   430,   490,    35,    79,    14,
     343,    -1,   430,   490,    79,    14,   343,    -1,   291,     9,
     430,   490,    79,    -1,   291,     9,   430,   490,    35,    79,
      -1,   291,     9,   430,   490,    35,    79,    14,   343,    -1,
     291,     9,   430,   490,    79,    14,   343,    -1,   293,   414,
      -1,    -1,   343,    -1,    35,   439,    -1,   163,   343,    -1,
     293,     9,   343,    -1,   293,     9,   163,   343,    -1,   293,
       9,    35,   439,    -1,   294,     9,   295,    -1,   295,    -1,
      79,    -1,   206,   439,    -1,   206,   204,   343,   205,    -1,
     296,     9,    79,    -1,   296,     9,    79,    14,   409,    -1,
      79,    -1,    79,    14,   409,    -1,   297,   298,    -1,    -1,
     299,   203,    -1,   476,    14,   409,    -1,   300,   301,    -1,
      -1,    -1,   324,   302,   330,   203,    -1,    -1,   326,   489,
     303,   330,   203,    -1,   331,   203,    -1,   332,   203,    -1,
      -1,   325,   249,   248,   477,   201,   304,   288,   202,   482,
     323,    -1,    -1,   429,   325,   249,   248,   477,   201,   305,
     288,   202,   482,   323,    -1,   156,   310,   203,    -1,   157,
     316,   203,    -1,   159,   318,   203,    -1,     4,   126,   402,
     203,    -1,     4,   127,   402,   203,    -1,   111,   273,   203,
      -1,   111,   273,   204,   306,   205,    -1,   306,   307,    -1,
     306,   308,    -1,    -1,   229,   149,   218,   164,   273,   203,
      -1,   309,    97,   325,   218,   203,    -1,   309,    97,   326,
     203,    -1,   229,   149,   218,    -1,   218,    -1,   311,    -1,
     310,     9,   311,    -1,   312,   399,   314,   315,    -1,   154,
      -1,   132,    -1,   132,   170,   489,   171,    -1,   132,   170,
     489,     9,   489,   171,    -1,   402,    -1,   119,    -1,   160,
     204,   313,   205,    -1,   133,    -1,   408,    -1,   313,     9,
     408,    -1,    14,   409,    -1,    -1,    55,   161,    -1,    -1,
     317,    -1,   316,     9,   317,    -1,   158,    -1,   319,    -1,
     218,    -1,   122,    -1,   201,   320,   202,    -1,   201,   320,
     202,    49,    -1,   201,   320,   202,    29,    -1,   201,   320,
     202,    46,    -1,   319,    -1,   321,    -1,   321,    49,    -1,
     321,    29,    -1,   321,    46,    -1,   320,     9,   320,    -1,
     320,    33,   320,    -1,   218,    -1,   154,    -1,   158,    -1,
     203,    -1,   204,   231,   205,    -1,   203,    -1,   204,   231,
     205,    -1,   326,    -1,   119,    -1,   326,    -1,    -1,   327,
      -1,   326,   327,    -1,   113,    -1,   114,    -1,   115,    -1,
     118,    -1,   117,    -1,   116,    -1,   183,    -1,   329,    -1,
      -1,   113,    -1,   114,    -1,   115,    -1,   330,     9,    79,
      -1,   330,     9,    79,    14,   409,    -1,    79,    -1,    79,
      14,   409,    -1,   331,     9,   476,    14,   409,    -1,   106,
     476,    14,   409,    -1,   332,     9,   476,    -1,   117,   106,
     476,    -1,   201,   333,   202,    -1,    68,   404,   407,    -1,
      67,   343,    -1,   391,    -1,   363,    -1,   201,   343,   202,
      -1,   335,     9,   343,    -1,   343,    -1,   335,    -1,    -1,
      27,    -1,    27,   343,    -1,    27,   343,   130,   343,    -1,
     439,    14,   337,    -1,   131,   201,   453,   202,    14,   337,
      -1,    28,   343,    -1,   439,    14,   340,    -1,   131,   201,
     453,   202,    14,   340,    -1,   344,    -1,   439,    -1,   333,
      -1,   443,    -1,   442,    -1,   131,   201,   453,   202,    14,
     343,    -1,   439,    14,   343,    -1,   439,    14,    35,   439,
      -1,   439,    14,    35,    68,   404,   407,    -1,   439,    26,
     343,    -1,   439,    25,   343,    -1,   439,    24,   343,    -1,
     439,    23,   343,    -1,   439,    22,   343,    -1,   439,    21,
     343,    -1,   439,    20,   343,    -1,   439,    19,   343,    -1,
     439,    18,   343,    -1,   439,    17,   343,    -1,   439,    16,
     343,    -1,   439,    15,   343,    -1,   439,    64,    -1,    64,
     439,    -1,   439,    63,    -1,    63,   439,    -1,   343,    31,
     343,    -1,   343,    32,   343,    -1,   343,    10,   343,    -1,
     343,    12,   343,    -1,   343,    11,   343,    -1,   343,    33,
     343,    -1,   343,    35,   343,    -1,   343,    34,   343,    -1,
     343,    48,   343,    -1,   343,    46,   343,    -1,   343,    47,
     343,    -1,   343,    49,   343,    -1,   343,    50,   343,    -1,
     343,    65,   343,    -1,   343,    51,   343,    -1,   343,    45,
     343,    -1,   343,    44,   343,    -1,    46,   343,    -1,    47,
     343,    -1,    52,   343,    -1,    54,   343,    -1,   343,    37,
     343,    -1,   343,    36,   343,    -1,   343,    39,   343,    -1,
     343,    38,   343,    -1,   343,    40,   343,    -1,   343,    43,
     343,    -1,   343,    41,   343,    -1,   343,    42,   343,    -1,
     343,    53,   404,    -1,   201,   344,   202,    -1,   343,    29,
     343,    30,   343,    -1,   343,    29,    30,   343,    -1,   471,
      -1,    62,   343,    -1,    61,   343,    -1,    60,   343,    -1,
      59,   343,    -1,    58,   343,    -1,    57,   343,    -1,    56,
     343,    -1,    69,   405,    -1,    55,   343,    -1,   411,    -1,
     362,    -1,   361,    -1,   364,    -1,   365,    -1,   207,   406,
     207,    -1,    13,   343,    -1,   369,    -1,   111,   201,   390,
     414,   202,    -1,    -1,    -1,   249,   248,   201,   347,   290,
     202,   482,   345,   204,   231,   205,    -1,    -1,   326,   249,
     248,   201,   348,   290,   202,   482,   345,   204,   231,   205,
      -1,    -1,    79,   350,   355,    -1,    -1,   183,    79,   351,
     355,    -1,    -1,   198,   352,   290,   199,   482,   355,    -1,
      -1,   183,   198,   353,   290,   199,   482,   355,    -1,    -1,
     183,   204,   354,   231,   205,    -1,     8,   343,    -1,     8,
     340,    -1,     8,   204,   231,   205,    -1,    86,    -1,   473,
      -1,   357,     9,   356,   130,   343,    -1,   356,   130,   343,
      -1,   358,     9,   356,   130,   409,    -1,   356,   130,   409,
      -1,   357,   413,    -1,    -1,   358,   413,    -1,    -1,   174,
     201,   359,   202,    -1,   132,   201,   454,   202,    -1,    66,
     454,   208,    -1,   402,   204,   456,   205,    -1,   176,   201,
     460,   202,    -1,   177,   201,   460,   202,    -1,   175,   201,
     461,   202,    -1,   176,   201,   464,   202,    -1,   177,   201,
     464,   202,    -1,   175,   201,   465,   202,    -1,   402,   204,
     458,   205,    -1,   369,    66,   449,   208,    -1,   370,    66,
     449,   208,    -1,   362,    -1,   473,    -1,   442,    -1,    86,
      -1,   201,   344,   202,    -1,   373,   374,    -1,   439,    14,
     371,    -1,   184,    79,   187,   343,    -1,   375,   386,    -1,
     375,   386,   389,    -1,   386,    -1,   386,   389,    -1,   376,
      -1,   375,   376,    -1,   377,    -1,   378,    -1,   379,    -1,
     380,    -1,   381,    -1,   382,    -1,   184,    79,   187,   343,
      -1,   191,    79,    14,   343,    -1,   185,   343,    -1,   186,
      79,   187,   343,   188,   343,   189,   343,    -1,   186,    79,
     187,   343,   188,   343,   189,   343,   190,    79,    -1,   192,
     383,    -1,   384,    -1,   383,     9,   384,    -1,   343,    -1,
     343,   385,    -1,   193,    -1,   194,    -1,   387,    -1,   388,
      -1,   195,   343,    -1,   196,   343,   197,   343,    -1,   190,
      79,   374,    -1,   390,     9,    79,    -1,   390,     9,    35,
      79,    -1,    79,    -1,    35,    79,    -1,   168,   154,   392,
     169,    -1,   394,    50,    -1,   394,   169,   395,   168,    50,
     393,    -1,    -1,   154,    -1,   394,   396,    14,   397,    -1,
      -1,   395,   398,    -1,    -1,   154,    -1,   155,    -1,   204,
     343,   205,    -1,   155,    -1,   204,   343,   205,    -1,   391,
      -1,   400,    -1,   399,    30,   400,    -1,   399,    47,   400,
      -1,   218,    -1,    69,    -1,   105,    -1,   106,    -1,   107,
      -1,    27,    -1,    28,    -1,   108,    -1,   109,    -1,   167,
      -1,   110,    -1,    70,    -1,    71,    -1,    73,    -1,    72,
      -1,    89,    -1,    90,    -1,    88,    -1,    91,    -1,    92,
      -1,    93,    -1,    94,    -1,    95,    -1,    96,    -1,    53,
      -1,    97,    -1,    98,    -1,    99,    -1,   100,    -1,   101,
      -1,   102,    -1,   104,    -1,   103,    -1,    87,    -1,    13,
      -1,   124,    -1,   125,    -1,   126,    -1,   127,    -1,    68,
      -1,    67,    -1,   119,    -1,     5,    -1,     7,    -1,     6,
      -1,     4,    -1,     3,    -1,   150,    -1,   111,    -1,   112,
      -1,   121,    -1,   122,    -1,   123,    -1,   118,    -1,   117,
      -1,   116,    -1,   115,    -1,   114,    -1,   113,    -1,   183,
      -1,   120,    -1,   131,    -1,   132,    -1,    10,    -1,    12,
      -1,    11,    -1,   134,    -1,   136,    -1,   135,    -1,   137,
      -1,   138,    -1,   152,    -1,   151,    -1,   182,    -1,   162,
      -1,   165,    -1,   164,    -1,   178,    -1,   180,    -1,   174,
      -1,   228,   201,   292,   202,    -1,   229,    -1,   154,    -1,
     402,    -1,   118,    -1,   447,    -1,   402,    -1,   118,    -1,
     451,    -1,   201,   202,    -1,   334,    -1,    -1,    -1,    85,
      -1,   468,    -1,   201,   292,   202,    -1,    -1,    74,    -1,
      75,    -1,    76,    -1,    86,    -1,   137,    -1,   138,    -1,
     152,    -1,   134,    -1,   165,    -1,   135,    -1,   136,    -1,
     151,    -1,   182,    -1,   145,    85,   146,    -1,   145,   146,
      -1,   408,    -1,   227,    -1,   132,   201,   412,   202,    -1,
      66,   412,   208,    -1,   174,   201,   360,   202,    -1,   410,
      -1,   368,    -1,   366,    -1,   367,    -1,   201,   409,   202,
      -1,   409,    31,   409,    -1,   409,    32,   409,    -1,   409,
      10,   409,    -1,   409,    12,   409,    -1,   409,    11,   409,
      -1,   409,    33,   409,    -1,   409,    35,   409,    -1,   409,
      34,   409,    -1,   409,    48,   409,    -1,   409,    46,   409,
      -1,   409,    47,   409,    -1,   409,    49,   409,    -1,   409,
      50,   409,    -1,   409,    51,   409,    -1,   409,    45,   409,
      -1,   409,    44,   409,    -1,   409,    65,   409,    -1,    52,
     409,    -1,    54,   409,    -1,    46,   409,    -1,    47,   409,
      -1,   409,    37,   409,    -1,   409,    36,   409,    -1,   409,
      39,   409,    -1,   409,    38,   409,    -1,   409,    40,   409,
      -1,   409,    43,   409,    -1,   409,    41,   409,    -1,   409,
      42,   409,    -1,   409,    29,   409,    30,   409,    -1,   409,
      29,    30,   409,    -1,   229,   149,   218,    -1,   154,   149,
     218,    -1,   229,   149,   124,    -1,   227,    -1,    78,    -1,
     473,    -1,   408,    -1,   209,   468,   209,    -1,   210,   468,
     210,    -1,   145,   468,   146,    -1,   415,   413,    -1,    -1,
       9,    -1,    -1,     9,    -1,    -1,   415,     9,   409,   130,
     409,    -1,   415,     9,   409,    -1,   409,   130,   409,    -1,
     409,    -1,    74,    -1,    75,    -1,    76,    -1,    86,    -1,
     145,    85,   146,    -1,   145,   146,    -1,    74,    -1,    75,
      -1,    76,    -1,   218,    -1,   416,    -1,   218,    -1,    46,
     417,    -1,    47,   417,    -1,   132,   201,   419,   202,    -1,
      66,   419,   208,    -1,   174,   201,   422,   202,    -1,   420,
     413,    -1,    -1,   420,     9,   418,   130,   418,    -1,   420,
       9,   418,    -1,   418,   130,   418,    -1,   418,    -1,   421,
       9,   418,    -1,   418,    -1,   423,   413,    -1,    -1,   423,
       9,   356,   130,   418,    -1,   356,   130,   418,    -1,   421,
     413,    -1,    -1,   201,   424,   202,    -1,    -1,   426,     9,
     218,   425,    -1,   218,   425,    -1,    -1,   428,   426,   413,
      -1,    45,   427,    44,    -1,   429,    -1,    -1,   128,    -1,
     129,    -1,   218,    -1,   154,    -1,   204,   343,   205,    -1,
     432,    -1,   446,    -1,   218,    -1,   204,   343,   205,    -1,
     434,    -1,   446,    -1,    66,   449,   208,    -1,   204,   343,
     205,    -1,   440,   436,    -1,   201,   333,   202,   436,    -1,
     452,   436,    -1,   201,   333,   202,   436,    -1,   446,    -1,
     401,    -1,   444,    -1,   445,    -1,   437,    -1,   439,   431,
     433,    -1,   201,   333,   202,   431,   433,    -1,   403,   149,
     446,    -1,   441,   201,   292,   202,    -1,   442,   201,   292,
     202,    -1,   201,   439,   202,    -1,   401,    -1,   444,    -1,
     445,    -1,   437,    -1,   439,   431,   432,    -1,   201,   333,
     202,   431,   432,    -1,   441,   201,   292,   202,    -1,   201,
     439,   202,    -1,   446,    -1,   437,    -1,   201,   439,   202,
      -1,   201,   443,   202,    -1,   346,    -1,   349,    -1,   439,
     431,   435,   478,   201,   292,   202,    -1,   201,   333,   202,
     431,   435,   478,   201,   292,   202,    -1,   403,   149,   218,
     478,   201,   292,   202,    -1,   403,   149,   446,   201,   292,
     202,    -1,   403,   149,   204,   343,   205,   201,   292,   202,
      -1,   447,    -1,   450,   447,    -1,   447,    66,   449,   208,
      -1,   447,   204,   343,   205,    -1,   448,    -1,    79,    -1,
     206,   204,   343,   205,    -1,   343,    -1,    -1,   206,    -1,
     450,   206,    -1,   446,    -1,   438,    -1,   451,   431,   433,
      -1,   201,   333,   202,   431,   433,    -1,   403,   149,   446,
      -1,   201,   439,   202,    -1,    -1,   438,    -1,   451,   431,
     432,    -1,   201,   333,   202,   431,   432,    -1,   201,   439,
     202,    -1,   453,     9,    -1,   453,     9,   439,    -1,   453,
       9,   131,   201,   453,   202,    -1,    -1,   439,    -1,   131,
     201,   453,   202,    -1,   455,   413,    -1,    -1,   455,     9,
     343,   130,   343,    -1,   455,     9,   343,    -1,   343,   130,
     343,    -1,   343,    -1,   455,     9,   343,   130,    35,   439,
      -1,   455,     9,    35,   439,    -1,   343,   130,    35,   439,
      -1,    35,   439,    -1,   457,   413,    -1,    -1,   457,     9,
     343,   130,   343,    -1,   457,     9,   343,    -1,   343,   130,
     343,    -1,   343,    -1,   459,   413,    -1,    -1,   459,     9,
     409,   130,   409,    -1,   459,     9,   409,    -1,   409,   130,
     409,    -1,   409,    -1,   462,   413,    -1,    -1,   463,   413,
      -1,    -1,   462,     9,   343,   130,   343,    -1,   343,   130,
     343,    -1,   463,     9,   343,    -1,   343,    -1,   466,   413,
      -1,    -1,   467,   413,    -1,    -1,   466,     9,   409,   130,
     409,    -1,   409,   130,   409,    -1,   467,     9,   409,    -1,
     409,    -1,   468,   469,    -1,   468,    85,    -1,   469,    -1,
      85,   469,    -1,    79,    -1,    79,    66,   470,   208,    -1,
      79,   431,   218,    -1,   147,   343,   205,    -1,   147,    78,
      66,   343,   208,   205,    -1,   148,   439,   205,    -1,   218,
      -1,    80,    -1,    79,    -1,   121,   201,   472,   202,    -1,
     122,   201,   439,   202,    -1,   122,   201,   344,   202,    -1,
     122,   201,   443,   202,    -1,   122,   201,   442,   202,    -1,
     122,   201,   333,   202,    -1,     7,   343,    -1,     6,   343,
      -1,     5,   201,   343,   202,    -1,     4,   343,    -1,     3,
     343,    -1,   439,    -1,   472,     9,   439,    -1,   403,   149,
     218,    -1,   403,   149,   124,    -1,    -1,    97,   489,    -1,
     178,   477,    14,   489,   203,    -1,   180,   477,   474,    14,
     489,   203,    -1,   218,    -1,   489,   218,    -1,   218,    -1,
     218,   170,   483,   171,    -1,   170,   480,   171,    -1,    -1,
     489,    -1,   479,     9,   489,    -1,   479,   413,    -1,   479,
       9,   163,    -1,   480,    -1,   163,    -1,    -1,    -1,    30,
     489,    -1,   483,     9,   484,   218,    -1,   484,   218,    -1,
     483,     9,   484,   218,    97,   489,    -1,   484,   218,    97,
     489,    -1,    46,    -1,    47,    -1,    -1,    86,   130,   489,
      -1,   229,   149,   218,   130,   489,    -1,   486,     9,   485,
      -1,   485,    -1,   486,   413,    -1,    -1,   174,   201,   487,
     202,    -1,    29,   489,    -1,    55,   489,    -1,   229,    -1,
     132,    -1,   133,    -1,   488,    -1,   132,   170,   489,   171,
      -1,   132,   170,   489,     9,   489,   171,    -1,   154,    -1,
     201,   105,   201,   481,   202,    30,   489,   202,    -1,   201,
     489,     9,   479,   413,   202,    -1,   489,    -1,    -1
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
    1486,  1489,  1489,  1493,  1492,  1496,  1498,  1503,  1501,  1516,
    1513,  1526,  1528,  1530,  1532,  1534,  1536,  1538,  1542,  1543,
    1544,  1547,  1553,  1556,  1562,  1565,  1570,  1572,  1577,  1582,
    1586,  1587,  1589,  1591,  1597,  1598,  1600,  1604,  1605,  1610,
    1611,  1615,  1616,  1620,  1622,  1628,  1633,  1634,  1636,  1640,
    1641,  1642,  1643,  1647,  1648,  1649,  1650,  1651,  1652,  1654,
    1659,  1662,  1663,  1667,  1668,  1672,  1673,  1676,  1677,  1680,
    1681,  1684,  1685,  1689,  1690,  1691,  1692,  1693,  1694,  1695,
    1699,  1700,  1703,  1704,  1705,  1708,  1710,  1712,  1713,  1716,
    1718,  1722,  1724,  1729,  1730,  1732,  1733,  1734,  1737,  1741,
    1742,  1746,  1747,  1751,  1752,  1753,  1757,  1761,  1766,  1770,
    1774,  1779,  1780,  1781,  1782,  1783,  1787,  1789,  1790,  1791,
    1794,  1795,  1796,  1797,  1798,  1799,  1800,  1801,  1802,  1803,
    1804,  1805,  1806,  1807,  1808,  1809,  1810,  1811,  1812,  1813,
    1814,  1815,  1816,  1817,  1818,  1819,  1820,  1821,  1822,  1823,
    1824,  1825,  1826,  1827,  1828,  1829,  1830,  1831,  1832,  1833,
    1834,  1835,  1836,  1838,  1839,  1841,  1843,  1844,  1845,  1846,
    1847,  1848,  1849,  1850,  1851,  1852,  1853,  1854,  1855,  1856,
    1857,  1858,  1859,  1860,  1861,  1862,  1863,  1867,  1871,  1876,
    1875,  1890,  1888,  1905,  1905,  1921,  1920,  1938,  1938,  1954,
    1953,  1972,  1971,  1992,  1993,  1994,  1999,  2001,  2005,  2009,
    2015,  2019,  2025,  2027,  2031,  2033,  2037,  2041,  2042,  2046,
    2053,  2054,  2058,  2062,  2064,  2069,  2074,  2081,  2083,  2088,
    2089,  2090,  2091,  2093,  2097,  2101,  2105,  2109,  2111,  2113,
    2115,  2120,  2121,  2126,  2127,  2128,  2129,  2130,  2131,  2135,
    2139,  2143,  2147,  2152,  2157,  2161,  2162,  2166,  2167,  2171,
    2172,  2176,  2177,  2181,  2185,  2189,  2193,  2194,  2195,  2196,
    2200,  2206,  2215,  2228,  2229,  2232,  2235,  2238,  2239,  2242,
    2246,  2249,  2252,  2259,  2260,  2264,  2265,  2267,  2271,  2272,
    2273,  2274,  2275,  2276,  2277,  2278,  2279,  2280,  2281,  2282,
    2283,  2284,  2285,  2286,  2287,  2288,  2289,  2290,  2291,  2292,
    2293,  2294,  2295,  2296,  2297,  2298,  2299,  2300,  2301,  2302,
    2303,  2304,  2305,  2306,  2307,  2308,  2309,  2310,  2311,  2312,
    2313,  2314,  2315,  2316,  2317,  2318,  2319,  2320,  2321,  2322,
    2323,  2324,  2325,  2326,  2327,  2328,  2329,  2330,  2331,  2332,
    2333,  2334,  2335,  2336,  2337,  2338,  2339,  2340,  2341,  2342,
    2343,  2344,  2345,  2346,  2347,  2348,  2349,  2350,  2354,  2359,
    2360,  2363,  2364,  2365,  2369,  2370,  2371,  2375,  2376,  2377,
    2381,  2382,  2383,  2386,  2388,  2392,  2393,  2394,  2395,  2397,
    2398,  2399,  2400,  2401,  2402,  2403,  2404,  2405,  2406,  2409,
    2414,  2415,  2416,  2418,  2419,  2421,  2422,  2423,  2424,  2425,
    2426,  2428,  2430,  2432,  2434,  2436,  2437,  2438,  2439,  2440,
    2441,  2442,  2443,  2444,  2445,  2446,  2447,  2448,  2449,  2450,
    2451,  2452,  2454,  2456,  2458,  2460,  2461,  2464,  2465,  2469,
    2471,  2475,  2478,  2481,  2487,  2488,  2489,  2490,  2491,  2492,
    2493,  2498,  2500,  2504,  2505,  2508,  2509,  2513,  2516,  2518,
    2520,  2524,  2525,  2526,  2527,  2529,  2532,  2536,  2537,  2538,
    2539,  2542,  2543,  2544,  2545,  2546,  2548,  2549,  2554,  2556,
    2559,  2562,  2564,  2566,  2569,  2571,  2575,  2577,  2580,  2583,
    2589,  2591,  2594,  2595,  2600,  2603,  2607,  2607,  2612,  2615,
    2616,  2620,  2621,  2625,  2626,  2627,  2631,  2632,  2636,  2637,
    2641,  2642,  2646,  2647,  2651,  2652,  2657,  2659,  2664,  2665,
    2666,  2667,  2668,  2669,  2671,  2674,  2677,  2679,  2681,  2685,
    2686,  2687,  2688,  2689,  2692,  2696,  2698,  2702,  2703,  2704,
    2708,  2712,  2713,  2717,  2720,  2727,  2731,  2735,  2742,  2743,
    2748,  2750,  2751,  2754,  2755,  2758,  2759,  2763,  2764,  2768,
    2769,  2770,  2773,  2776,  2779,  2782,  2783,  2784,  2787,  2791,
    2795,  2796,  2797,  2799,  2800,  2801,  2805,  2807,  2810,  2812,
    2813,  2814,  2815,  2818,  2820,  2821,  2825,  2827,  2830,  2832,
    2833,  2834,  2838,  2840,  2843,  2846,  2848,  2850,  2854,  2856,
    2859,  2861,  2864,  2866,  2869,  2870,  2874,  2876,  2879,  2881,
    2884,  2887,  2891,  2893,  2897,  2898,  2900,  2901,  2907,  2908,
    2910,  2912,  2914,  2916,  2919,  2920,  2921,  2925,  2926,  2927,
    2928,  2929,  2930,  2931,  2932,  2933,  2934,  2935,  2939,  2940,
    2944,  2946,  2954,  2956,  2960,  2964,  2971,  2972,  2978,  2979,
    2986,  2989,  2993,  2996,  3001,  3006,  3008,  3009,  3010,  3014,
    3015,  3019,  3021,  3022,  3025,  3030,  3031,  3032,  3036,  3039,
    3048,  3050,  3054,  3057,  3060,  3068,  3071,  3074,  3075,  3078,
    3081,  3082,  3085,  3089,  3093,  3099,  3109,  3110
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
  "class_abstract_constant_declaration", "expr_with_parens",
  "parenthesis_expr", "expr_list", "for_expr", "yield_expr",
  "yield_assign_expr", "yield_list_assign_expr", "await_expr",
  "await_assign_expr", "await_list_assign_expr", "expr",
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
     300,   302,   301,   303,   301,   301,   301,   304,   301,   305,
     301,   301,   301,   301,   301,   301,   301,   301,   306,   306,
     306,   307,   308,   308,   309,   309,   310,   310,   311,   311,
     312,   312,   312,   312,   312,   312,   312,   313,   313,   314,
     314,   315,   315,   316,   316,   317,   318,   318,   318,   319,
     319,   319,   319,   320,   320,   320,   320,   320,   320,   320,
     321,   321,   321,   322,   322,   323,   323,   324,   324,   325,
     325,   326,   326,   327,   327,   327,   327,   327,   327,   327,
     328,   328,   329,   329,   329,   330,   330,   330,   330,   331,
     331,   332,   332,   333,   333,   333,   333,   333,   334,   335,
     335,   336,   336,   337,   337,   337,   338,   339,   340,   341,
     342,   343,   343,   343,   343,   343,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
     344,   344,   344,   344,   344,   344,   344,   345,   345,   347,
     346,   348,   346,   350,   349,   351,   349,   352,   349,   353,
     349,   354,   349,   355,   355,   355,   356,   356,   357,   357,
     358,   358,   359,   359,   360,   360,   361,   362,   362,   363,
     364,   364,   365,   366,   366,   367,   368,   369,   369,   370,
     370,   370,   370,   370,   371,   372,   373,   374,   374,   374,
     374,   375,   375,   376,   376,   376,   376,   376,   376,   377,
     378,   379,   380,   381,   382,   383,   383,   384,   384,   385,
     385,   386,   386,   387,   388,   389,   390,   390,   390,   390,
     391,   392,   392,   393,   393,   394,   394,   395,   395,   396,
     397,   397,   398,   398,   398,   399,   399,   399,   400,   400,
     400,   400,   400,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   400,   400,   400,   401,   402,
     402,   403,   403,   403,   404,   404,   404,   405,   405,   405,
     406,   406,   406,   407,   407,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   408,   408,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   410,   410,   410,   411,   411,   411,   411,   411,   411,
     411,   412,   412,   413,   413,   414,   414,   415,   415,   415,
     415,   416,   416,   416,   416,   416,   416,   417,   417,   417,
     417,   418,   418,   418,   418,   418,   418,   418,   419,   419,
     420,   420,   420,   420,   421,   421,   422,   422,   423,   423,
     424,   424,   425,   425,   426,   426,   428,   427,   429,   430,
     430,   431,   431,   432,   432,   432,   433,   433,   434,   434,
     435,   435,   436,   436,   437,   437,   438,   438,   439,   439,
     439,   439,   439,   439,   439,   439,   439,   439,   439,   440,
     440,   440,   440,   440,   440,   440,   440,   441,   441,   441,
     442,   443,   443,   444,   444,   445,   445,   445,   446,   446,
     447,   447,   447,   448,   448,   449,   449,   450,   450,   451,
     451,   451,   451,   451,   451,   452,   452,   452,   452,   452,
     453,   453,   453,   453,   453,   453,   454,   454,   455,   455,
     455,   455,   455,   455,   455,   455,   456,   456,   457,   457,
     457,   457,   458,   458,   459,   459,   459,   459,   460,   460,
     461,   461,   462,   462,   463,   463,   464,   464,   465,   465,
     466,   466,   467,   467,   468,   468,   468,   468,   469,   469,
     469,   469,   469,   469,   470,   470,   470,   471,   471,   471,
     471,   471,   471,   471,   471,   471,   471,   471,   472,   472,
     473,   473,   474,   474,   475,   475,   476,   476,   477,   477,
     478,   478,   479,   479,   480,   481,   481,   481,   481,   482,
     482,   483,   483,   483,   483,   484,   484,   484,   485,   485,
     486,   486,   487,   487,   488,   489,   489,   489,   489,   489,
     489,   489,   489,   489,   489,   489,   490,   490
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
       0,     0,     4,     0,     5,     2,     2,     0,    10,     0,
      11,     3,     3,     3,     4,     4,     3,     5,     2,     2,
       0,     6,     5,     4,     3,     1,     1,     3,     4,     1,
       1,     4,     6,     1,     1,     4,     1,     1,     3,     2,
       0,     2,     0,     1,     3,     1,     1,     1,     1,     3,
       4,     4,     4,     1,     1,     2,     2,     2,     3,     3,
       1,     1,     1,     1,     3,     1,     3,     1,     1,     1,
       0,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     1,     1,     1,     3,     5,     1,     3,     5,
       4,     3,     3,     3,     3,     2,     1,     1,     3,     3,
       1,     1,     0,     1,     2,     4,     3,     6,     2,     3,
       6,     1,     1,     1,     1,     1,     6,     3,     4,     6,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     2,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     5,     4,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     1,
       1,     1,     1,     1,     3,     2,     1,     5,     0,     0,
      11,     0,    12,     0,     3,     0,     4,     0,     6,     0,
       7,     0,     5,     2,     2,     4,     1,     1,     5,     3,
       5,     3,     2,     0,     2,     0,     4,     4,     3,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     1,
       1,     1,     1,     3,     2,     3,     4,     2,     3,     1,
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
       1,     1,     4,     3,     4,     1,     1,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     5,
       4,     3,     3,     3,     1,     1,     1,     1,     3,     3,
       3,     2,     0,     1,     0,     1,     0,     5,     3,     3,
       1,     1,     1,     1,     1,     3,     2,     1,     1,     1,
       1,     1,     1,     2,     2,     4,     3,     4,     2,     0,
       5,     3,     3,     1,     3,     1,     2,     0,     5,     3,
       2,     0,     3,     0,     4,     2,     0,     3,     3,     1,
       0,     1,     1,     1,     1,     3,     1,     1,     1,     3,
       1,     1,     3,     3,     2,     4,     2,     4,     1,     1,
       1,     1,     1,     3,     5,     3,     4,     4,     3,     1,
       1,     1,     1,     3,     5,     4,     3,     1,     1,     3,
       3,     1,     1,     7,     9,     7,     6,     8,     1,     2,
       4,     4,     1,     1,     4,     1,     0,     1,     2,     1,
       1,     3,     5,     3,     3,     0,     1,     3,     5,     3,
       2,     3,     6,     0,     1,     4,     2,     0,     5,     3,
       3,     1,     6,     4,     4,     2,     2,     0,     5,     3,
       3,     1,     2,     0,     5,     3,     3,     1,     2,     0,
       2,     0,     5,     3,     3,     1,     2,     0,     2,     0,
       5,     3,     3,     1,     2,     2,     1,     2,     1,     4,
       3,     3,     6,     3,     1,     1,     1,     4,     4,     4,
       4,     4,     4,     2,     2,     4,     2,     2,     1,     3,
       3,     3,     0,     2,     5,     6,     1,     2,     1,     4,
       3,     0,     1,     3,     2,     3,     1,     1,     0,     0,
       2,     4,     2,     6,     4,     1,     1,     0,     3,     5,
       3,     1,     2,     0,     4,     2,     2,     1,     1,     1,
       1,     4,     6,     1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   363,     0,   756,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   837,     0,
     825,   639,     0,   645,   646,   647,    22,   705,   813,    98,
      99,   648,     0,    80,     0,     0,     0,     0,     0,     0,
       0,     0,   132,     0,     0,     0,     0,     0,     0,   333,
     334,   335,   338,   337,   336,     0,     0,     0,     0,   159,
       0,     0,     0,   652,   654,   655,   649,   650,     0,     0,
     656,   651,     0,   630,    23,    24,    25,    27,    26,     0,
     653,     0,     0,     0,     0,     0,     0,     0,   657,   339,
      28,    29,    31,    30,    32,    33,    34,    35,    36,    37,
      38,    39,    40,   457,     0,    97,    70,   817,   640,     0,
       0,     4,    59,    61,    64,   704,     0,   629,     0,     6,
     131,     7,     9,     8,    10,     0,     0,   331,   373,     0,
       0,     0,     0,     0,     0,     0,   371,   801,   802,   441,
     440,   357,   442,   443,   446,     0,     0,   356,   779,   631,
       0,   707,   439,   330,   782,   372,     0,     0,   375,   374,
     780,   781,   778,   808,   812,     0,   429,   706,    11,   338,
     337,   336,     0,     0,    27,    59,   131,     0,   897,   372,
     896,     0,   894,   893,   445,     0,   364,   368,     0,     0,
     413,   414,   415,   416,   438,   436,   435,   434,   433,   432,
     431,   430,   813,   632,     0,   911,   631,     0,   395,     0,
     393,     0,   841,     0,   714,   355,   635,     0,   911,   634,
       0,   644,   820,   819,   636,     0,     0,   638,   437,     0,
       0,     0,     0,   360,     0,    78,   362,     0,     0,    84,
      86,     0,     0,    88,     0,     0,     0,   938,   939,   943,
       0,     0,    59,   937,     0,   940,     0,     0,    90,     0,
       0,     0,     0,   122,     0,     0,     0,     0,     0,     0,
      42,    47,   248,     0,     0,   247,     0,   163,     0,   160,
     253,     0,     0,     0,     0,     0,   908,   147,   157,   833,
     837,   878,     0,   659,     0,     0,     0,   876,     0,    16,
       0,    63,   139,   151,   158,   536,   473,   861,   859,   859,
       0,   902,   455,   459,   461,   760,   373,     0,   371,   372,
     374,     0,     0,   641,     0,   642,     0,     0,     0,   121,
       0,     0,    66,   239,     0,    21,   130,     0,   156,   143,
     155,   336,   339,   131,   332,   112,   113,   114,   115,   116,
     118,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   825,     0,   111,   816,
     816,   119,   847,     0,     0,     0,     0,     0,     0,   329,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   394,   392,   761,   762,     0,   816,     0,
     774,   239,   239,   816,     0,   818,   809,   833,     0,   131,
       0,     0,    92,     0,   758,   753,   714,     0,     0,     0,
       0,     0,   845,     0,   478,   713,   836,     0,     0,    66,
       0,   239,   354,     0,   776,   637,     0,    70,   199,     0,
     454,     0,    95,     0,     0,   361,     0,     0,     0,     0,
       0,    87,   110,    89,   935,   936,     0,   933,     0,     0,
       0,   907,     0,   117,    91,   120,     0,     0,     0,     0,
       0,     0,     0,   494,     0,   501,   503,   504,   505,   506,
     507,   508,   499,   521,   522,    70,     0,   107,   109,     0,
       0,    44,    51,     0,     0,    46,    55,    48,     0,    18,
       0,     0,   249,     0,    93,   162,   161,     0,     0,    94,
     898,     0,     0,   373,   371,   372,   375,   374,     0,   927,
     169,     0,   834,     0,     0,     0,     0,   658,   877,   705,
       0,     0,   875,   710,   874,    62,     5,    13,    14,     0,
     167,     0,     0,   466,     0,     0,   714,     0,     0,   633,
     467,   865,     0,   714,     0,     0,   714,     0,     0,     0,
       0,     0,   760,    70,     0,   716,   759,   947,   353,   426,
     788,   800,    75,    69,    71,    72,    73,    74,   330,     0,
     444,   708,   709,    60,   714,     0,   912,     0,     0,     0,
     716,   240,     0,   449,   133,   165,     0,   398,   400,   399,
       0,     0,   396,   397,   401,   403,   402,   418,   417,   420,
     419,   421,   423,   424,   422,   412,   411,   405,   406,   404,
     407,   408,   410,   425,   409,   815,     0,     0,   851,     0,
     714,   901,     0,   900,   785,   808,   149,   141,   153,   145,
     131,   363,     0,   366,   369,   377,   495,   391,   390,   389,
     388,   387,   386,   385,   384,   383,   382,   381,   380,   764,
       0,   763,   766,   783,   770,   911,   767,     0,     0,     0,
       0,     0,     0,     0,     0,   895,   365,   751,   755,   713,
     757,     0,     0,   911,     0,   840,     0,   839,     0,   824,
     823,     0,     0,   763,   766,   821,   767,   358,   201,   203,
      70,   464,   463,   359,     0,    70,   183,    79,   362,     0,
       0,     0,     0,     0,   195,   195,    85,     0,     0,     0,
     931,   714,     0,   918,     0,     0,     0,     0,     0,   712,
     648,     0,     0,   630,     0,     0,     0,     0,     0,    64,
     661,   629,   667,   668,   666,     0,   660,    68,   665,     0,
       0,   511,     0,     0,   517,   514,   515,   523,     0,   502,
     497,     0,   500,     0,     0,     0,    52,    19,     0,     0,
      56,    20,     0,     0,     0,    41,    49,     0,   246,   254,
     251,     0,     0,   887,   892,   889,   888,   891,   890,    12,
     925,   926,     0,     0,     0,     0,   833,   830,     0,   477,
     886,   885,   884,     0,   880,     0,   881,   883,     0,     5,
       0,     0,     0,   530,   531,   539,   538,     0,     0,   713,
     472,   476,     0,   482,   713,   860,     0,   480,   713,   858,
     481,     0,   903,     0,   456,     0,     0,   919,   760,   225,
     946,     0,     0,   775,   814,   713,   914,   910,   241,   242,
     628,   715,   238,     0,   760,     0,     0,   167,   451,   135,
     428,     0,   487,   488,     0,   479,   713,   846,     0,     0,
     239,   169,     0,   167,   165,     0,   825,   378,     0,     0,
     772,   773,   786,   787,   810,   811,     0,     0,     0,   739,
     721,   722,   723,   724,     0,     0,     0,   732,   731,   745,
     714,     0,   753,   844,   843,     0,     0,   777,   643,     0,
     205,     0,     0,    76,     0,     0,     0,     0,     0,     0,
       0,   175,   176,   187,     0,    70,   185,   104,   195,     0,
     195,     0,     0,   941,     0,     0,   713,   932,   934,   917,
     714,   916,     0,   714,   689,   690,   687,   688,   720,     0,
     714,   712,     0,     0,   475,   869,   867,   867,     0,     0,
     853,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   496,     0,     0,
       0,   519,   520,   518,     0,     0,   498,     0,   123,     0,
     126,   108,     0,    43,    53,     0,    45,    57,    50,   250,
       0,   899,    96,   927,   909,   922,   168,   170,   260,     0,
       0,   831,     0,   879,     0,    17,     0,   902,   166,   260,
       0,     0,   469,     0,   900,   864,   863,     0,   904,     0,
     919,   462,     0,     0,   947,     0,   230,   228,   766,   784,
     911,   913,     0,     0,   243,    67,     0,   760,   164,     0,
     760,     0,   427,   850,   849,     0,   239,     0,     0,     0,
       0,   167,   137,   644,   765,   239,     0,   727,   728,   729,
     730,   733,   734,   743,     0,   714,   739,     0,   726,   747,
     713,   750,   752,   754,     0,   838,   766,   822,   765,     0,
       0,     0,     0,   202,   465,    81,     0,   362,   175,   177,
     833,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     189,     0,   928,     0,   930,   713,     0,     0,     0,   663,
     713,   711,     0,   702,     0,   714,     0,   873,     0,   714,
       0,     0,   714,     0,   669,   703,   701,   857,     0,   714,
     672,   674,   673,     0,     0,   670,   671,   675,   677,   676,
     692,   691,   694,   693,   695,   697,   698,   696,   685,   684,
     679,   680,   678,   681,   682,   683,   686,   509,     0,   510,
     516,   524,   525,     0,    70,    54,    58,   252,     0,     0,
       0,   330,   835,   833,   367,   370,   376,     0,    15,     0,
     330,   542,     0,     0,   544,   537,   540,     0,   535,     0,
       0,   905,     0,   920,   458,     0,   231,     0,     0,   226,
       0,   245,   244,   919,     0,   260,     0,   760,     0,   239,
       0,   806,   260,   902,   260,     0,     0,   379,     0,     0,
     736,   713,   738,     0,   725,     0,     0,   714,   744,   842,
       0,    70,     0,   198,   184,     0,     0,     0,   174,   100,
     188,     0,     0,   191,     0,   196,   197,    70,   190,   942,
       0,   915,     0,   945,   719,   718,   662,     0,   713,   474,
     664,   485,   713,   868,     0,   483,   713,   866,   484,     0,
     486,   713,   852,   700,     0,     0,     0,     0,   921,   924,
     171,     0,     0,     0,   337,   328,     0,     0,     0,   148,
     259,   261,     0,   327,     0,     0,   330,     0,   882,   256,
     152,   533,     0,     0,   468,   862,   460,     0,   234,   224,
       0,   227,   233,   239,   448,   919,   330,   919,     0,   848,
       0,   805,   330,     0,   330,   260,   760,   803,   742,   741,
     735,     0,   737,   713,   746,    70,   204,    77,    82,   102,
     178,     0,   186,   192,    70,   194,   929,     0,     0,   471,
       0,   872,   871,     0,   856,   855,   699,     0,    70,   127,
       0,     0,     0,     0,     0,   172,     0,   294,   290,   296,
     630,    27,     0,   286,     0,   293,   305,     0,   303,   308,
       0,   307,     0,   306,     0,   131,   263,     0,   265,     0,
     266,     0,   832,     0,   534,   532,   543,   541,   235,     0,
       0,   222,   232,     0,     0,     0,     0,   144,   448,   919,
     807,   150,   256,   154,   330,     0,     0,   749,     0,   200,
       0,     0,    70,   181,   101,   193,   944,   717,     0,     0,
       0,     0,     0,   923,     0,     0,     0,     0,   276,   280,
     352,     0,     0,     0,   271,   594,   593,   590,   592,   591,
     611,   613,   612,   582,   553,   554,   572,   588,   587,   549,
     559,   560,   562,   561,   581,   565,   563,   564,   566,   567,
     568,   569,   570,   571,   573,   574,   575,   576,   577,   578,
     580,   579,   550,   551,   552,   555,   556,   558,   596,   597,
     606,   605,   604,   603,   602,   601,   589,   608,   598,   599,
     600,   583,   584,   585,   586,   609,   610,   614,   616,   615,
     617,   618,   595,   620,   619,   622,   624,   623,   557,   627,
     625,   626,   621,   607,   548,   300,   545,     0,   272,   321,
     322,   320,   313,     0,   314,   273,   347,     0,     0,     0,
       0,   351,   131,   140,   255,     0,     0,     0,   223,   237,
     804,     0,    70,   323,    70,   134,     0,     0,     0,   146,
     919,   740,     0,    70,   179,    83,   103,     0,   470,   870,
     854,   512,   125,   274,   275,   350,   173,     0,     0,     0,
     297,   287,     0,     0,     0,   302,   304,     0,     0,   309,
     316,   317,   315,     0,     0,   262,     0,     0,     0,     0,
     257,     0,   236,     0,   528,   716,     0,     0,    70,   136,
     142,     0,   748,     0,     0,     0,   105,   277,    59,     0,
     278,   279,     0,     0,   291,     0,   295,   299,   546,   547,
       0,   288,   318,   319,   311,   312,   310,   348,   345,   267,
     264,   349,     0,   258,   529,   715,     0,   450,   324,     0,
     138,     0,   182,   513,     0,   129,     0,   330,     0,   298,
     301,     0,   760,   269,     0,   526,   447,   452,   180,     0,
       0,   106,   284,     0,   329,   292,   346,     0,   716,   341,
     760,   527,     0,   128,     0,     0,   283,   919,   760,   209,
     342,   343,   344,   947,   340,     0,     0,     0,   282,     0,
     341,     0,   919,     0,   281,   325,    70,   268,   947,     0,
     214,   212,     0,    70,     0,     0,   215,     0,     0,   210,
     270,     0,   326,     0,   218,   208,     0,   211,   217,   124,
     219,     0,     0,   206,   216,     0,   207,   221,   220
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   121,   829,   556,   185,   279,   510,
     514,   280,   511,   515,   123,   124,   125,   126,   127,   128,
     331,   593,   594,   464,   244,  1460,   470,  1381,  1461,  1695,
     785,   274,   505,  1656,  1021,  1204,  1711,   347,   186,   595,
     875,  1081,  1256,   132,   559,   892,   596,   615,   894,   540,
     891,   597,   560,   893,   349,   297,   313,   135,   877,   832,
     815,  1036,  1404,  1132,   941,  1605,  1464,   727,   947,   469,
     736,   949,  1287,   719,   930,   933,  1121,  1717,  1718,   584,
     585,   609,   610,   284,   285,   291,  1433,  1584,  1585,  1211,
    1330,  1424,  1579,  1702,  1720,  1617,  1660,  1661,  1662,  1412,
    1413,  1414,  1619,  1625,  1671,  1417,  1418,  1422,  1572,  1573,
    1574,  1595,  1747,  1331,  1332,   187,   137,  1733,  1734,  1577,
    1334,  1335,   138,   237,   465,   466,   139,   140,   141,   142,
     143,   144,   145,   146,  1445,   147,   874,  1080,   148,   241,
     581,   325,   582,   583,   460,   565,   566,  1155,   567,  1156,
     149,   150,   151,   152,   153,   762,   763,   764,   154,   155,
     271,   156,   272,   493,   494,   495,   496,   497,   498,   499,
     500,   501,   775,   776,  1013,   502,   503,   504,   782,  1645,
     157,   561,  1435,   562,  1050,   837,  1228,  1225,  1565,  1566,
     158,   159,   160,   231,   238,   334,   452,   161,   968,   768,
     162,   969,   866,   859,   970,   918,  1101,  1103,  1104,  1105,
     920,  1266,  1267,   921,   698,   436,   198,   199,   598,   587,
     417,   682,   683,   684,   685,   863,   164,   232,   189,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   646,   175,
     234,   235,   543,   223,   224,   649,   650,  1168,  1169,   575,
     572,   576,   573,  1161,  1158,  1162,  1159,   306,   307,   823,
     176,   531,   177,   580,   178,  1586,   298,   342,   604,   605,
     962,  1063,   812,   813,   740,   741,   742,   265,   266,   861
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1459
static const yytype_int16 yypact[] =
{
   -1459,   137, -1459, -1459,  5480, 13008, 13008,   -28, 13008, 13008,
   13008,  4804, 13008, -1459, 13008, 13008, 13008, 13008, 13008, 13008,
   13008, 13008, 13008, 13008, 13008, 13008, 14879, 14879, 11304, 13008,
   14945,   -26,   -17, -1459, -1459, -1459, -1459, -1459,   186, -1459,
   -1459,   133, 13008, -1459,   -17,    -4,     8,    11,   -17, 11470,
   12677, 11636, -1459, 14367, 10264,    23, 13008, 15864,    26, -1459,
   -1459, -1459,    19,   323,   351,    22,    43,   134,   152, -1459,
   12677,   231,   241, -1459, -1459, -1459, -1459, -1459,   434,  2826,
   -1459, -1459, 12677, -1459, -1459, -1459, -1459, 12677, -1459, 12677,
   -1459,   294,   251,   258,   264,   279, 12677, 12677, -1459,   211,
   -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459,
   -1459, -1459, -1459, -1459, 13008, -1459, -1459,   310,   476,   543,
     543, -1459,   200,   331,    17, -1459,   368, -1459,    68, -1459,
     542, -1459, -1459, -1459, -1459, 15923,   503, -1459, -1459,   395,
     398,   404,   426,   452,   455,  4207, -1459, -1459, -1459, -1459,
     507, -1459, -1459, -1459,   513,   530,   471, -1459,    58,   484,
     544, -1459, -1459,   517,   144,  3505,   114,   494,    60, -1459,
     130,   135,   500,    49, -1459,    40, -1459,   656, -1459, -1459,
   -1459,   577,   533,   583, -1459, -1459,   542,   503, 16319,  3794,
   16319, 13008, 16319, 16319, 10455,   545, 15327, 10455,   702, 12677,
     682,   682,   117,   682,   682,   682,   682,   682,   682,   682,
     682,   682, -1459, -1459,  2305,   579, -1459,   603,   418,   556,
     418, 14879, 15371,   552,   749, -1459,   577, 14502,   579,   614,
     616,   568,   136, -1459,   418,   114, 11802, -1459, -1459, 13008,
    4638,   760,    79, 16319,  9848, -1459, 13008, 13008, 12677, -1459,
   -1459,  4351,   580, -1459,  4436, 14367, 14367,   612, -1459, -1459,
     584, 14139,    78, -1459,   770, -1459, 12677,   707, -1459,   586,
   13570,   588,   676, -1459,    14, 13614, 15998, 16011, 12677,    82,
   -1459,   338, -1459,  1871,    84, -1459,   663, -1459,   668, -1459,
     782,    85, 14879, 14879, 13008,   605,   638, -1459, -1459, 13813,
   11304,    61,   424, -1459, 13174, 14879,   466, -1459, 12677, -1459,
     390,   331, -1459, -1459, -1459, -1459, 14968, 13008, 13008, 13008,
     800,   718, -1459, -1459, -1459,   101,   617, 16319,   620,  1952,
     621,  5688, 13008,   417,   618,   567,   417,   393,   376, -1459,
   12677, 14367,   623, 10472, 14367, -1459, -1459, 11139, -1459, -1459,
   -1459, -1459, -1459,   542, -1459, -1459, -1459, -1459, -1459, -1459,
   -1459, 13008, 13008, 13008, 12010, 13008, 13008, 13008, 13008, 13008,
   13008, 13008, 13008, 13008, 13008, 13008, 13008, 13008, 13008, 13008,
   13008, 13008, 13008, 13008, 13008, 13008, 14945, 13008, -1459, 13008,
   13008, -1459, 13008, 14634, 12677, 12677, 12677, 15923,   721,    45,
   10056, 13008, 13008, 13008, 13008, 13008, 13008, 13008, 13008, 13008,
   13008, 13008, 13008, -1459, -1459, -1459, -1459,  4467, 13008, 13008,
   -1459, 10472, 10472, 13008, 13008,   310,   145, 13813,   629,   542,
   12218, 13658, -1459, 13008, -1459,   631,   825,  2305,   633,    20,
     621,  3751,   418, 12426, -1459, 12634, -1459,   634,   329, -1459,
      50, 10472, -1459, 15012, -1459, -1459, 13702, -1459, -1459, 10680,
   -1459, 13008, -1459,   748,  9016,   829,   639, 16230,   826,    64,
      46, -1459, -1459, -1459, -1459, -1459, 14367, 15802,   643,   836,
   14591, -1459,   660, -1459, -1459, -1459,   769, 13008,   771,   772,
   13008, 13008, 13008, -1459,   676, -1459, -1459, -1459, -1459, -1459,
   -1459, -1459,   659, -1459, -1459, -1459,   652, -1459, -1459, 12677,
     651,   846,   367, 12677,   661,   861,   386,   397, 16060, -1459,
   12677, 13008,   418,    26, -1459, -1459, -1459, 14591,   796, -1459,
     418,   102,   103,   675,   678,  2020,   105,   680,   681,   158,
     752,   685,   418,   107,   688, 15939, 12677, -1459, -1459,   831,
    2209,     3, -1459, -1459, -1459,   331, -1459, -1459, -1459,   862,
     768,   729,   373, -1459,   310,   773,   891,   700,   756,   145,
   -1459, 16319,   704,   898, 15427,   709,   903,   712, 14367, 14367,
     901,   760,   101, -1459,   717,   908, -1459, 14367,   370,   852,
     150, -1459, -1459, -1459, -1459, -1459, -1459, -1459,   626,  2302,
   -1459, -1459, -1459, -1459,   914,   762, -1459, 14879, 13008,   732,
     926, 16319,   923, -1459, -1459,   813, 12053, 10871, 16486, 10455,
   13008, 16275,  2164, 16663, 16728,  2246,  4015, 11988, 11988, 11988,
   11988,  1431,  1431,  1431,  1431,   725,   725,   613,   613,   613,
     117,   117,   117, -1459,   682, 16319,   733,   735, 15471,   739,
     936, -1459, 13008,     2,   750,   145, -1459, -1459, -1459, -1459,
     542, 13008, 14739, -1459, -1459, 10455, -1459, 10455, 10455, 10455,
   10455, 10455, 10455, 10455, 10455, 10455, 10455, 10455, 10455, -1459,
   13008,   252,   151, -1459, -1459,   579,   261,   741,  2794,   753,
     754,   742,  3094,   108,   757, -1459, 16319, 14723, -1459, 12677,
   -1459,   617,   370,   579, 14879, 16319, 14879, 15527,   370,   177,
   -1459,   755, 13008, -1459,   188, -1459, -1459, -1459,  8808,   637,
   -1459, -1459, 16319, 16319,   -17, -1459, -1459, -1459, 13008,   857,
    2575, 14591, 12677,  9224,   759,   761, -1459,    76,   833,   810,
   -1459,   956,   765, 14220, 14367, 14591, 14591, 14591, 14591, 14591,
   -1459,   767,    67,   820,   775,   777,   778,   779, 14591,    15,
   -1459,   821, -1459, -1459, -1459,   780, -1459, 16405, -1459, 13008,
     784, 16319,   785,   959,  4997,   965, -1459, 16319,  4134, -1459,
     659,   902, -1459,  5896, 15877,   781,   415, -1459, 15998, 12677,
     457, -1459, 16011, 12677, 12677, -1459, -1459,  3375, -1459, 16405,
     968, 14879,   783, -1459, -1459, -1459, -1459, -1459, -1459, -1459,
   -1459, -1459,    77, 12677, 15877,   789, 13813, 14824,   969, -1459,
   -1459, -1459, -1459,   787, -1459, 13008, -1459, -1459,  5064, -1459,
   14367, 15877,   792, -1459, -1459, -1459, -1459,   976, 13008, 14968,
   -1459, -1459, 12261, -1459, 13008, -1459, 13008, -1459, 13008, -1459,
   -1459,   795, -1459, 14367, -1459,   803,  6104,   977,    34, -1459,
   -1459,   100,  4467, -1459, -1459, 14367, -1459, -1459,   418, 16319,
   -1459, 10888, -1459, 14591,    38,   807, 15877,   768, -1459, -1459,
   16561, 13008, -1459, -1459, 13008, -1459, 13008, -1459,  3419,   812,
   10472,   752,   981,   768,   813, 12677, 14945,   418,  3553,   816,
   -1459, -1459,   189, -1459, -1459, -1459,  1005, 15158, 15158, 14723,
   -1459, -1459, -1459, -1459,   819,    83,   822, -1459, -1459, -1459,
    1012,   823,   631,   418,   418, 12842, 15012, -1459, -1459,  3673,
     645,   -17,  9848, -1459,  6312,   824,  6520,   827,  2575, 14879,
     828,   892,   418, 16405,  1010, -1459, -1459, -1459, -1459,   505,
   -1459,    44, 14367, -1459, 14367, 12677, 15802, -1459, -1459, -1459,
    1017, -1459,   830,   914,   594,   594,   966,   966, 15261,   832,
    1024, 14591,   888, 12677, 14968, 14591, 14591, 14591, 13746, 12469,
   14591, 14591, 14591, 14591, 14439, 14591, 14591, 14591, 14591, 14591,
   14591, 14591, 14591, 14591, 14591, 14591, 14591, 14591, 14591, 14591,
   14591, 14591, 14591, 14591, 14591, 14591, 14591, 16319, 13008, 13008,
   13008, -1459, -1459, -1459, 13008, 13008, -1459,   676, -1459,   957,
   -1459, -1459, 12677, -1459, -1459, 12677, -1459, -1459, -1459, -1459,
   14591,   418, -1459,   158, -1459,   941,  1030, -1459, -1459,   119,
     842,   418, 11096, -1459,  2092, -1459,  5272,   718,  1030, -1459,
     423,   -21, 16319,   915, -1459, 16319, 16319, 15571, -1459,   841,
     977, -1459, 14367,   760, 14367,    72,  1032,   970,   237, -1459,
     579, -1459, 14879, 13008, 16319, 16405,   848,    38, -1459,   844,
      38,   850, 16561, 16319, 15627,   851, 10472,   853,   854, 14367,
     858,   768, -1459,   568,   280, 10472, 13008, -1459, -1459, -1459,
   -1459, -1459, -1459,   924,   849,  1047, 14723,   917, -1459, 14968,
   14723, -1459, -1459, -1459, 14879, 16319,   245, -1459, -1459,   -17,
    1031,   991,  9848, -1459, -1459, -1459,   863, 13008,   892,   418,
   13813,  2575,   865, 14591,  6728,   550,   868, 13008,    48,    55,
   -1459,   894, -1459,   943, -1459, 14286,  1048,   878, 14591, -1459,
   14591, -1459,   880, -1459,   953,  1075,   883, 16405,   884,  1078,
   15671,   886,  1080,   890, -1459, -1459, -1459, 15727,   889,  1087,
   16446, 16526, 13383, 14591, 16363, 16630, 16696, 16760,  4567, 16790,
   16820, 16820, 16820, 16820,  3701,  3701,  3701,  3701,   881,   881,
     594,   594,   594,   966,   966,   966,   966, 16319, 13877, 16319,
   -1459, 16319, -1459,   895, -1459, -1459, -1459, 16405, 12677, 14367,
   15877,   554, -1459, 13813, -1459, -1459, 10455,   893, -1459,   896,
    1178, -1459,    70, 13008, -1459, -1459, -1459, 13008, -1459, 13008,
   13008, -1459,   760, -1459, -1459,   160,  1085,  1022, 13008, -1459,
     905,   418, 16319,   977,   906, -1459,   909,    38, 13008, 10472,
     912, -1459, -1459,   718, -1459,   899,   916, -1459,   913, 14723,
   -1459, 14723, -1459,   918, -1459,   974,   919,  1098, -1459,   418,
    1086, -1459,   907, -1459, -1459,   921,   927,   129, -1459, -1459,
   16405,   922,   925, -1459,  2582, -1459, -1459, -1459, -1459, -1459,
   14367, -1459, 14367, -1459, 16405, 15771, -1459, 14591, 14968, -1459,
   -1459, -1459, 14591, -1459, 14591, -1459, 14591, -1459, -1459, 14591,
   -1459, 14591, -1459, 16596, 14591, 13008,   930,  6936,  1033, -1459,
   -1459,   512, 14367, 15877,  1013, -1459, 15818,   973, 15090, -1459,
   -1459, -1459,   721, 14073,    88,    89,    45,   131, -1459, -1459,
   -1459,   982,  3981,  4089, 16319, 16319, -1459,    74,  1121,  1058,
   13008, -1459, 16319, 10472,  1027,   977,  1515,   977,   937, 16319,
     938, -1459,  1575,   940,  1697, -1459,    38, -1459, -1459,  1011,
   -1459, 14723, -1459, 14968, -1459, -1459,  8808, -1459, -1459, -1459,
   -1459,  9432, -1459, -1459, -1459,  8808, -1459,   945, 14591, 16405,
    1018, 16405, 16405, 15827, 16405, 15871, 16596, 13833, -1459, -1459,
   14367, 15877, 15877,  1128,    73, -1459, 14367, -1459,   980, -1459,
      90,   949,    91, -1459, 13382, -1459, -1459,    94, -1459, -1459,
    3099, -1459,   951, -1459,  1076,   542, -1459, 14367, -1459, 14367,
   -1459,   721, -1459, 13900, -1459, -1459, -1459, -1459,  1142,  1079,
   13008, -1459, 16319,   955,   960,   958,   385, -1459,  1027,   977,
   -1459, -1459, -1459, -1459,  1824,   961, 14723, -1459,  1034,  8808,
    9640,  9432, -1459, -1459, -1459,  8808, -1459, 16405, 14591, 14591,
   14591, 13008,  7144, -1459,   962,   964, 14591, 15877, -1459, -1459,
   -1459, 14367,  1195, 15818, -1459, -1459, -1459, -1459, -1459, -1459,
   -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459,
   -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459,
   -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459,
   -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459,
   -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459,
   -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459,
   -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459,
   -1459, -1459, -1459, -1459, -1459,   127, -1459,   973, -1459, -1459,
   -1459, -1459, -1459,    81,   538, -1459,  1146,    95, 12677,  1076,
    1155, -1459,   542, -1459, -1459,   967,  1157, 13008, -1459, 16319,
   -1459,   518, -1459, -1459, -1459, -1459,   971,   385, 13966, -1459,
     977, -1459, 14723, -1459, -1459, -1459, -1459,  7352, 16405, 16405,
   16405, 13789, -1459, -1459, -1459, 16405, -1459, 15034,    86,    62,
   -1459, -1459, 14591, 13382, 13382,  1117, -1459,  3099,  3099,   608,
   -1459, -1459, -1459, 14591,  1095, -1459,   975,    98, 14591, 12677,
   -1459, 14591, 16319,  1099, -1459,  1168,  7560,  7768, -1459, -1459,
   -1459,   385, -1459,  7976,   984,  1100,  1074, -1459,  1088,  1039,
   -1459, -1459,  1094, 14367, -1459,  1195, -1459, 16405, -1459, -1459,
    1035, -1459,  1160, -1459, -1459, -1459, -1459, 16405,  1180, -1459,
   -1459, 16405,   997, 16405, -1459,   557,   998, -1459, -1459,  8184,
   -1459,  1000, -1459, -1459,  1003,  1038, 12677,    45,  1036, -1459,
   -1459, 14591,    56, -1459,  1122, -1459, -1459, -1459, -1459, 15877,
     781, -1459,  1042, 12677,   392, -1459, 16405,  1006,  1201,   590,
      56, -1459,  1133, -1459, 15877,  1016, -1459,   977,    57, -1459,
   -1459, -1459, -1459, 14367, -1459,  1014,  1019,    99, -1459,   481,
     590,   354,   977,  1009, -1459, -1459, -1459, -1459, 14367,    75,
    1212,  1148,   481, -1459,  8392,   355,  1214,  1150, 13008, -1459,
   -1459,  8600, -1459,   304,  1216,  1152, 13008, -1459, 16319, -1459,
    1218,  1156, 13008, -1459, 16319, 13008, -1459, 16319, 16319
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1459, -1459, -1459,  -493, -1459, -1459, -1459,    80, -1459, -1459,
   -1459,   716,   448,   445,    39,  1574,  3589, -1459,  2900, -1459,
    -383, -1459,    29, -1459, -1459, -1459, -1459, -1459, -1459, -1459,
   -1459, -1459, -1459, -1459,  -472, -1459, -1459,  -165,   138,    24,
   -1459, -1459, -1459, -1459, -1459, -1459,    30, -1459, -1459, -1459,
   -1459,    31, -1459, -1459,   843,   845,   855,  -108,   348,  -813,
     356,   419,  -478,   120,  -863, -1459,  -212, -1459, -1459, -1459,
   -1459,  -678,   -32, -1459, -1459, -1459, -1459,  -467, -1459,  -555,
   -1459,  -362, -1459, -1459,   731, -1459,  -197, -1459, -1459,  -902,
   -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459,
    -227, -1459, -1459, -1459, -1459, -1459,  -309, -1459,   -69,  -930,
   -1459, -1458,  -491, -1459,  -155,    63,  -134,  -474, -1459,  -312,
   -1459, -1459,   -49,    -6,  1226,  -676,  -355, -1459, -1459,   -14,
   -1459, -1459,    -5,   -42,  -175, -1459, -1459, -1459, -1459, -1459,
   -1459, -1459, -1459, -1459,  -533,  -783, -1459, -1459, -1459, -1459,
   -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459,
     875, -1459, -1459,   259, -1459,   788, -1459, -1459, -1459, -1459,
   -1459, -1459, -1459,   266, -1459,   791, -1459, -1459,   519, -1459,
     248, -1459, -1459, -1459, -1459, -1459, -1459, -1459, -1459,  -904,
   -1459,  2366,  1879,  -336, -1459, -1459,   194,  2801,  1929, -1459,
   -1459,   330,   -10,  -585, -1459, -1459,   394,  -650,   197, -1459,
   -1459, -1459, -1459, -1459,   382, -1459, -1459, -1459,    66,  -819,
    -176,  -392,  -391, -1459,   443,  -125, -1459, -1459,   462, -1459,
   -1459,   511,    -8, -1459, -1459,    51,  -109, -1459,  -198, -1459,
   -1459, -1459,  -395,  1007, -1459, -1459, -1459, -1459, -1459,   987,
   -1459, -1459, -1459,   332, -1459, -1459, -1459,   562,   464, -1459,
   -1459,  1015,  -280,  -978, -1459,   -22,   -67,  -174,   -15,   571,
   -1459,  -947, -1459,   278,   359, -1459, -1459, -1459,   786, -1021
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -912
static const yytype_int16 yytable[] =
{
     188,   190,   354,   192,   193,   194,   196,   197,   398,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   428,   314,   222,   225,   872,   240,   855,   131,   320,
     321,   264,   693,   129,   133,   134,   570,   243,   245,  1064,
     269,   420,   249,  1235,   251,   663,   254,   919,   854,   270,
     643,   275,   937,   354,   449,   397,  1053,   951,   453,   689,
     690,   714,   715,   828,  1079,   326,   426,   136,   350,  1219,
     163,  1665,   328,   732,   718,  1128,   734,   344,  1285,    13,
    1090,   233,  1477,    13,   122,   952,  1033,  -906,   461,   711,
    1627,   518,  -906,   523,   528,  1663,   281,  1427,  1429,  -289,
    1483,    13,    13,  1567,  1634,   282,   330,  1634,  1477,   327,
     454,   801,   801,  1232,  1628,   423,   817,   817,   310,   212,
    1341,   311,   783,   506,  -789,   546,  -491,   545,   817,   212,
     252,   415,   416,   262,  1226,  1065,   286,     3,   817,  1649,
     817,  1622,   130,   287,  1137,  1138,    13,  1220,   415,   416,
     296,  1236,   972,  1438,  1756,  1137,  1138,  1623,    59,    60,
      61,   179,   180,   351,  -911,   438,  -911,   312,  1107,   296,
     386,  -491,   341,   191,  1624,   236,   296,   296,   447,  1066,
     418,   507,   387,  1227,   239,   341,   431,   341,   616,   415,
     416,  1154,   647,  1690,  -453,  1347,  -790,   246,  -633,  -492,
     856,  -791,  -826,  -911,   810,   811,   440,   569,   827,   247,
    -792,   423,   248,   303,   446,   296,  -796,  -793,  -911,  -911,
     687,  -911,   590,   292,   315,   691,   399,   273,   352,  1108,
     339,   456,   283,  -715,   456,  1237,  -715,  1439,  1757,  1348,
    -229,   243,   467,  -829,   293,   533,   425,   953,  1034,  1140,
     735,  1286,   534,   424,  -827,  -795,   117,  1664,  -213,  -715,
    1288,   422,  -789,  1067,   694,   354,   733,  1666,  1278,   458,
    1135,   345,  1139,   463,   353,  1363,  1478,  1479,  1255,   435,
     614,  -906,   462,  1629,   655,   519,   537,   524,   529,   327,
     322,  1428,  1430,  -289,  1484,   222,  1354,  1568,  1635,   550,
    -229,  1680,  1744,  -794,   802,   803,   422,   807,   655,   818,
     906,  -828,   571,   574,   574,   512,   516,   517,   419,  1076,
     122,  1212,   612,  1349,   122,   429,  1265,   599,   468,   314,
     350,  1380,   655,  1432,  -790,   294,  1046,   934,   611,  -791,
    -826,   655,   936,  1356,   655,  -798,   481,   555,  -792,   424,
    1362,  -799,  1364,   295,  -796,  -793,   617,   618,   619,   621,
     622,   623,   624,   625,   626,   627,   628,   629,   630,   631,
     632,   633,   634,   635,   636,   637,   638,   639,   640,   641,
     642,  -829,   644,  1770,   645,   645,   664,   648,   701,  1749,
    1763,   586,  -827,  -795,   136,   665,   667,   668,   669,   670,
     671,   672,   673,   674,   675,   676,   677,   678,  1446,   323,
    1448,   122,   862,   645,   688,   324,   611,   611,   645,   692,
     603,  1039,  -768,   834,   262,   665,   700,   296,   696,   440,
     290,  -771,   299,  1750,  1764,   520,   418,   233,   705,   288,
     707,  -794,   300,   398,   654,   721,   611,   289,   315,  -828,
    -769,  1275,   316,  -768,   722,   301,   723,   415,   416,   317,
    1268,   552,  -771,  1454,   789,   318,   165,  1771,   686,   130,
    1068,  1069,   301,   653,   296,   657,   296,   296,   552,   889,
     319,  -769,   771,   793,   340,   774,   777,   778,   218,   220,
     397,   340,   654,   726,   794,   895,   301,   681,   415,   416,
    -632,   710,  1597,   301,   716,    59,    60,    61,   179,   180,
     351,   899,  1022,   301,   332,  1390,   797,  1751,  1765,   302,
     340,   703,  1244,   304,   305,  1246,   862,   835,  1087,   889,
    1234,   709,   926,   713,  1116,  1117,   660,   219,   219,   340,
     304,   305,   836,   340,   122,   301,   415,   416,   786,   879,
     340,   552,   790,  1643,  1025,   301,   840,   281,  1321,   570,
    1093,   333,  1134,   845,   304,   305,   849,  1630,   340,   343,
     547,   304,   305,  -489,   419,   352,   329,   346,  1221,   389,
     303,   304,   305,   927,  1631,   449,   602,  1632,  1593,  1594,
    1458,  1222,  1704,   557,   558,  1726,   390,  1644,   355,    13,
     796,   356,   601,   869,  1136,  1137,  1138,   357,    52,  1368,
     340,  1369,   553,   304,   305,   880,    59,    60,    61,   179,
     180,   351,   301,   304,   305,   822,   824,  1223,   336,   358,
      59,    60,    61,    62,    63,   351,  1705,  1674,  1401,  1402,
     887,    69,   394,  1003,  1004,  1005,   301,   888,   586,  1282,
    1137,  1138,   552,  1651,  1675,   359,   196,  1676,   360,  1006,
    1322,   399,   383,   384,   385,  1323,   386,    59,    60,    61,
     179,  1324,   351,  1325,   391,   898,   439,   395,   387,   396,
     335,   337,   338,   442,  1745,  1746,   352,  1214,   392,   448,
     304,   305,  1358,   393,   570,   421,   296,  1672,  1673,  1346,
     352,  -797,   165,  1730,  1731,  1732,   165,   929,   931,   932,
    1326,  1327,  1741,  1328,   304,   305,  1119,  1120,   935,  1668,
    1669,  1457,  -490,   243,  1250,   219,  -632,  1755,   960,   963,
     569,   957,   219,  1258,   427,  1277,   308,   352,   219,    59,
      60,    61,    62,    63,   351,   522,   434,   387,   432,   341,
      69,   394,   441,   655,   530,   530,   535,   422,   445,  1329,
     444,   542,   946,  -631,  1007,   450,   548,   551,   459,   451,
     554,   380,   381,   382,   383,   384,   385,   917,   386,   922,
    1739,   136,   476,   472,   480,   477,   482,   525,   396,   483,
     387,   485,   526,   165,   219,  1752,   527,   548,   122,   554,
     548,   554,   554,   219,   219,   536,  1601,   538,   539,   352,
     219,  1455,   944,   122,   578,   579,   219,   655,  1337,   588,
    1044,  1317,   589,   591,   -65,   600,    52,   512,  1092,   570,
     613,   516,   697,  1052,   699,   702,   708,   724,   461,  1055,
     731,  1056,   728,  1057,   743,   744,   136,   769,   770,   781,
     772,   773,   131,   784,   787,   788,   130,   129,   133,   134,
     486,   487,   488,   122,   791,   569,  1074,   489,   490,  1024,
     792,   491,   492,  1027,  1028,   800,  1082,   804,   814,  1083,
     805,  1084,   808,  1719,   809,   611,   816,  1360,  1376,   542,
     819,   136,   830,  1035,   163,   831,  1240,   825,   833,   439,
     839,  1719,   841,   838,  1385,   842,   843,   844,   122,  1740,
    1111,   847,   848,   686,   850,   853,   857,   858,  -493,   136,
    1115,   130,  1054,   865,   586,  1122,   165,  1000,  1001,  1002,
    1003,  1004,  1005,   867,   870,   871,   122,   873,   219,   876,
     586,   882,   681,   883,   885,   886,  1006,   233,   219,   900,
     904,   890,  1652,  1147,   938,   902,   903,   928,   878,   955,
    1151,  1123,   948,   954,   950,   956,   130,   958,   971,   973,
     979,  1008,  1009,  1010,  1014,   296,   974,   716,   975,   976,
     977,  1017,  1030,  1042,   980,  1020,  1032,  1100,  1100,   917,
    1051,  1443,  1459,  1038,   130,  1043,  1049,   136,  1058,   136,
     569,  1465,  1060,  1197,  1198,  1199,   713,  1062,  1077,   774,
    1201,  1089,   122,  1086,   122,  1472,   122,  1095,   570,  1096,
    1106,  1110,  1131,  1109,  1133,  1112,  1145,  1125,  1215,  1130,
    1127,  1006,  1146,  1150,   547,  1143,  1203,  1216,  1209,  1210,
    1149,   474,   475,  1213,  1231,  1229,  1238,   479,  1245,  1239,
    1243,  1247,  1249,  1153,  1259,  1251,  1261,  1260,  1252,  1166,
    1686,  1271,  1254,  1264,  1272,  1289,  1274,  1279,  1242,   868,
     131,  1283,   130,  1290,   130,   129,   133,   134,  1292,  1607,
    1293,   611,  1296,  1297,  1298,  1300,  1301,  1302,  1305,  1306,
     611,  1216,  1308,   570,  1310,  1262,  1311,  1316,  1338,  1350,
    1339,  1351,  1205,  1365,  1371,  1206,  1353,  1373,  1355,   136,
    1377,  1357,   163,  1270,  1361,  1367,  1375,  1366,   219,  1406,
    1370,  1372,   243,  1378,   897,  1382,   122,   606,  1383,  1379,
    1400,  1416,  1284,  1729,  1398,  1440,  1434,  1441,  1444,  1449,
    1450,  1456,  1476,   586,  1452,  1299,   586,  1466,  1468,  1303,
    1481,  1273,  1307,  1482,  1575,  1576,  1587,  1590,  1588,  1312,
    1633,  1591,  1592,  1600,  1602,  1613,   923,  1614,   924,  1638,
    1640,  1641,  1670,   219,  1678,  1648,  1679,  1685,  1684,  1693,
     165,  1431,  1321,  1694,   130,  -285,   917,  1692,  1696,   569,
     917,  1697,   942,  1628,  1701,   165,  1700,   136,  1703,   354,
    1706,  1721,   122,  1708,  1709,  1710,  1724,  1715,  1727,  1646,
    1728,  1647,  1736,  1753,   122,   219,  1742,   219,  1342,  1738,
    1653,  1743,  1343,    13,  1344,  1345,  1758,  1759,  1766,  1767,
    1772,  1773,  1775,  1352,   795,  1776,  1023,  1026,  1723,   656,
     659,   219,  1091,  1359,   611,   165,  1737,  1088,  1276,  1606,
    1048,   658,  1384,  1735,   798,  1598,  1621,  1374,  1626,  1423,
    1578,  1760,   737,  1031,   569,  1689,  1748,  1637,   242,    33,
      34,    35,   130,  1596,  1333,   666,  1202,  1336,   542,  1041,
    1200,   750,   779,  1333,  1322,   780,  1336,  1257,  1318,  1323,
     165,    59,    60,    61,   179,  1324,   351,  1325,  1224,  1016,
    1403,  1152,  1102,  1263,  1113,  1070,   577,   544,   532,  1163,
    1397,  1208,   219,   586,   961,  1144,     0,     0,   165,     0,
       0,     0,     0,     0,     0,     0,     0,   219,   219,    73,
      74,    75,    76,    77,  1326,  1327,     0,  1328,     0,   917,
     752,   917,     0,     0,     0,  1442,    80,    81,   611,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      90,   352,     0,  1754,   851,   852,     0,     0,     0,     0,
    1761,     0,     0,   860,     0,     0,     0,    98,     0,     0,
     136,     0,     0,  1340,  1480,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   165,     0,   165,   122,   165,   399,
     942,  1129,   262,     0,     0,  1580,     0,  1581,  1421,     0,
    1463,     0,     0,     0,     0,     0,     0,  1639,     0,  1333,
       0,     0,  1336,     0,     0,  1333,     0,  1333,  1336,     0,
    1336,     0,   586,     0,     0,  1589,     0,     0,     0,   136,
       0,     0,     0,     0,     0,     0,     0,     0,   136,   219,
     219,   917,     0,     0,     0,   130,   122,     0,     0,     0,
       0,   122,     0,     0,     0,   122,  1611,     0,     0,     0,
    1425,  -912,  -912,  -912,  -912,   378,   379,   380,   381,   382,
     383,   384,   385,     0,   386,     0,   262,     0,     0,  1604,
    1463,     0,     0,     0,  1564,     0,   387,     0,     0,     0,
    1571,     0,     0,     0,     0,     0,     0,   262,   165,   262,
       0,  1636,     0,   262,   130,     0,     0,  1333,     0,  1321,
    1336,     0,   136,   130,     0,     0,     0,     0,   136,   606,
     606,     0,     0,     0,  1241,   136,   917,     0,     0,   122,
     122,   122,  1713,     0,     0,   122,     0,     0,     0,     0,
       0,     0,   122,     0,     0,     0,     0,     0,     0,     0,
      13,     0,     0,     0,     0,     0,     0,     0,     0,  1582,
       0,     0,  1682,     0,     0,     0,  1269,     0,     0,  1321,
     354,     0,  1642,   219,   165,     0,     0,     0,     0,     0,
       0,     0,   542,   942,     0,     0,   165,   130,     0,     0,
     215,   215,     0,   130,   228,     0,     0,     0,     0,     0,
     130,     0,     0,     0,     0,     0,  1047,     0,     0,     0,
      13,  1322,     0,     0,     0,   219,  1323,   228,    59,    60,
      61,   179,  1324,   351,  1325,     0,     0,     0,     0,  1059,
       0,   219,   219,     0,     0,     0,     0,     0,     0,     0,
       0,  1071,     0,     0,     0,     0,     0,     0,   296,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     136,  1326,  1327,     0,  1328,   542,     0,     0,   262,     0,
       0,  1322,   917,     0,     0,     0,  1323,   122,    59,    60,
      61,   179,  1324,   351,  1325,     0,     0,  1658,   352,     0,
       0,  1321,     0,  1564,  1564,     0,     0,  1571,  1571,   136,
     136,     0,     0,     0,     0,     0,   136,     0,     0,   296,
    1447,     0,     0,     0,   219,     0,   122,   122,     0,     0,
       0,  1326,  1327,   122,  1328,     0,     0,     0,  1141,     0,
    1142,     0,    13,     0,     0,   130,     0,     0,     0,     0,
       0,     0,   136,  1768,     0,     0,     0,     0,   352,     0,
    1714,  1774,     0,     0,     0,     0,     0,  1777,   586,   122,
    1778,     0,     0,     0,     0,     0,  1712,     0,     0,   165,
    1451,     0,     0,     0,   130,   130,   586,     0,   215,     0,
       0,   130,     0,  1725,   586,   215,     0,     0,     0,     0,
       0,   215,     0,  1322,     0,     0,     0,     0,  1323,     0,
      59,    60,    61,   179,  1324,   351,  1325,   136,     0,     0,
       0,     0,     0,     0,   136,     0,     0,   130,  1321,   228,
     228,     0,     0,     0,   122,   228,     0,     0,   165,     0,
       0,   122,     0,   165,     0,     0,     0,   165,  1233,     0,
     860,     0,     0,  1326,  1327,     0,  1328,   215,     0,     0,
       0,     0,     0,     0,     0,     0,   215,   215,     0,    13,
       0,     0,     0,   215,     0,  1253,     0,     0,     0,   215,
     352,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     228,     0,   130,     0,     0,     0,     0,     0,     0,   130,
       0,     0,  1453,     0,     0,   217,   217,     0,     0,   230,
       0,     0,     0,     0,     0,   228,     0,     0,   228,     0,
       0,   165,   165,   165,     0,     0,     0,   165,     0,     0,
    1322,  1071,     0,     0,   165,  1323,     0,    59,    60,    61,
     179,  1324,   351,  1325,     0,     0,     0,     0,    36,     0,
     212,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     228,     0,     0,     0,     0,     0,   430,   401,   402,   403,
     404,   405,   406,   407,   408,   409,   410,   411,   412,     0,
    1326,  1327,     0,  1328,     0,     0,     0,     0,     0,   213,
       0,     0,     0,     0,     0,  1319,     0,     0,     0,     0,
       0,   215,     0,     0,     0,     0,     0,   352,     0,     0,
       0,   215,     0,     0,     0,   413,   414,     0,     0,     0,
       0,   183,     0,     0,    82,    83,     0,    84,    85,  1599,
      86,   184,    88,     0,   430,   401,   402,   403,   404,   405,
     406,   407,   408,   409,   410,   411,   412,     0,     0,     0,
     228,   228,     0,     0,   759,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   165,
       0,     0,   214,     0,     0,   521,  1386,   117,  1387,     0,
     415,   416,     0,   413,   414,     0,     0,     0,     0,     0,
       0,     0,     0,   217,     0,     0,     0,     0,     0,     0,
     217,   759,   361,   362,   363,     0,   217,     0,   165,   165,
       0,     0,     0,     0,     0,   165,     0,     0,     0,  1426,
       0,   364,     0,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,     0,   386,     0,     0,   415,   416,
       0,   165,   228,   228,   590,     0,     0,   387,     0,     0,
       0,   228,   217,     0,     0,     0,     0,     0,     0,     0,
       0,   217,   217,     0,     0,     0,     0,     0,   217,     0,
       0,   215,     0,     0,   217,     0,  1473,     0,     0,     0,
       0,     0,     0,     0,     0,   568,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,   165,   386,     0,   361,
     362,   363,   806,   165,     0,     0,     0,     0,     0,   387,
       0,     0,     0,     0,     0,     0,   215,     0,   364,     0,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,     0,   386,     0,     0,   230,     0,  1618,     0,     0,
       0,     0,     0,     0,   387,     0,     0,     0,   215,     0,
     215,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,     0,   386,
    1217,     0,     0,     0,   215,   759,   217,     0,     0,     0,
       0,   387,   361,   362,   363,     0,   217,   228,   228,   759,
     759,   759,   759,   759,     0,     0,     0,     0,     0,     0,
       0,   364,   759,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,     0,   386,     0,     0,   228,     0,
       0,     0,     0,     0,     0,     0,     0,   387,     0,     0,
       0,     0,    29,    30,     0,   215,     0,     0,     0,     0,
       0,     0,    36,     0,    38,     0,     0,     0,   228,     0,
     215,   215,   216,   216,     0,     0,   229,     0,     0,     0,
       0,     0,     0,     0,   228,   228,     0,     0,     0,   767,
      52,     0,     0,   228,   826,     0,     0,     0,    59,    60,
      61,   179,   180,   181,     0,     0,     0,   228,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   228,
       0,     0,     0,     0,     0,     0,     0,   759,     0,  1698,
     228,     0,     0,     0,     0,   183,   799,     0,    82,    83,
       0,    84,    85,     0,    86,   184,    88,     0,     0,     0,
     228,     0,     0,    91,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   217,     0,    99,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,     0,     0,   437,   864,     0,     0,
       0,   117,   215,   215,     0,     0,     0,     0,     0,   860,
       0,     0,     0,     0,     0,     0,   228,     0,   228,     0,
     228,     0,     0,     0,   860,     0,     0,     0,     0,     0,
       0,   217,     0,     0,     0,   759,     0,     0,   228,   759,
     759,   759,     0,     0,   759,   759,   759,   759,   759,   759,
     759,   759,   759,   759,   759,   759,   759,   759,   759,   759,
     759,   759,   759,   759,   759,   759,   759,   759,   759,   759,
     759,     0,     0,   217,     0,   217,     0,   216,     0,     0,
       0,     0,   361,   362,   363,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   759,     0,     0,     0,     0,   217,
     939,   364,  1285,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,     0,   386,   228,     0,   228,     0,
       0,     0,     0,     0,     0,     0,   215,   387,     0,   216,
       0,     0,    36,     0,   212,     0,     0,     0,   216,   216,
     943,     0,     0,   228,     0,   216,     0,     0,     0,     0,
       0,   216,     0,     0,   964,   965,   966,   967,     0,     0,
     217,     0,   216,   228,     0,     0,     0,   978,   215,     0,
       0,     0,     0,   213,     0,   217,   217,     0,     0,     0,
       0,     0,     0,     0,   215,   215,   940,   759,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   568,   228,
       0,     0,   759,     0,   759,   183,     0,     0,    82,    83,
       0,    84,    85,     0,    86,   184,    88,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   759,     0,     0,
       0,     0,   229,     0,     0,     0,     0,     0,     0,     0,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   230,   214,     0,     0,     0,
       0,   117,     0,   228,   228,  1286,     0,   215,     0,     0,
       0,     0,     0,   216,     0,     0,     0,     0,     0,     0,
       0,     0,  1075,     0,   361,   362,   363,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   217,   217,     0,
       0,     0,     0,   364,     0,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,   765,   386,     0,     0,
       0,     0,     0,   568,     0,     0,     0,     0,     0,   387,
       0,     0,     0,     0,   228,     0,   228,     0,     0,     0,
       0,   759,   228,     0,     0,     0,   759,     0,   759,     0,
     759,     0,     0,   759,     0,   759,     0,     0,   759,     0,
       0,     0,     0,   765,     0,     0,   228,   228,     0,     0,
     228,     0,     0,    36,  1157,  1160,  1160,   228,     0,  1167,
    1170,  1171,  1172,  1174,  1175,  1176,  1177,  1178,  1179,  1180,
    1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,  1189,  1190,
    1191,  1192,  1193,  1194,  1195,  1196,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   228,     0,     0,
       0,   217,     0,   263,     0,     0,     0,     0,     0,  1207,
       0,     0,   759,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   216,   228,   228,   228,     0,     0,   308,
     228,     0,    84,    85,     0,    86,   184,    88,   568,     0,
       0,     0,     0,   217,     0,     0,     0,     0,     0,   901,
       0,   228,     0,   228,     0,     0,     0,   228,     0,   217,
     217,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,     0,     0,   216,     0,
     309,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   759,   759,   759,     0,     0,     0,     0,     0,
     759,   228,     0,     0,     0,   228,     0,   228,     0,     0,
       0,     0,  1280,     0,     0,     0,     0,     0,     0,     0,
     216,     0,   216,     0,     0,     0,     0,  1294,     0,  1295,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   217,     0,     0,     0,   216,   765,     0,     0,
       0,     0,  1313,     0,   361,   362,   363,     0,     0,     0,
       0,   765,   765,   765,   765,   765,     0,     0,     0,     0,
       0,     0,     0,   364,   765,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,     0,   386,     0,     0,
    1019,     0,     0,     0,     0,   263,   263,     0,     0,   387,
       0,   263,     0,     0,     0,     0,     0,   216,     0,     0,
       0,     0,   228,     0,     0,     0,    36,   568,     0,     0,
    1037,     0,   216,   216,     0,     0,     0,     0,     0,     0,
       0,   228,     0,     0,     0,     0,   759,  1037,     0,     0,
       0,     0,     0,     0,     0,   216,     0,   759,     0,     0,
       0,     0,   759,     0,     0,   759,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1389,     0,     0,     0,
       0,  1391,     0,  1392,     0,  1393,     0,   228,  1394,   765,
    1395,   263,  1078,  1396,   263,     0,     0,     0,     0,     0,
       0,     0,   568,  1569,     0,    84,    85,  1570,    86,   184,
      88,     0,   229,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   759,     0,     0,     0,     0,
       0,   766,     0,   228,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,   228,   905,
    1420,     0,     0,     0,   216,   216,     0,   228,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1467,     0,     0,
       0,     0,   228,     0,     0,     0,     0,     0,   766,     0,
       0,     0,     0,     0,     0,     0,     0,   765,     0,     0,
     216,   765,   765,   765,     0,     0,   765,   765,   765,   765,
     765,   765,   765,   765,   765,   765,   765,   765,   765,   765,
     765,   765,   765,   765,   765,   765,   765,   765,   765,   765,
     765,   765,   765,     0,     0,     0,   263,   739,     0,     0,
     761,     0,     0,     0,     0,   361,   362,   363,     0,     0,
       0,     0,     0,     0,     0,     0,   765,  1608,  1609,  1610,
       0,     0,     0,     0,   364,  1615,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   761,   386,   361,
     362,   363,     0,     0,     0,     0,     0,     0,   216,     0,
     387,     0,     0,     0,     0,     0,     0,     0,   364,     0,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,     0,   386,     0,     0,   216,     0,     0,   263,   263,
     216,     0,     0,     0,   387,     0,     0,   263,     0,     0,
       0,     0,     0,     0,     0,     0,   216,   216,     0,   765,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   765,     0,   765,     0,     0,   400,
     401,   402,   403,   404,   405,   406,   407,   408,   409,   410,
     411,   412,   766,     0,     0,     0,     0,     0,     0,   765,
       0,     0,     0,     0,     0,     0,   766,   766,   766,   766,
     766,  1667,     0,     0,     0,     0,     0,     0,     0,   766,
       0,     0,  1677,   361,   362,   363,     0,  1681,   413,   414,
    1683,     0,     0,     0,     0,     0,  1320,     0,     0,   216,
    1029,     0,   364,     0,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,     0,   386,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   387,     0,
       0,     0,     0,     0,  1085,     0,     0,     0,     0,     0,
    1716,   761,     0,   415,   416,     0,     0,     0,     0,     0,
       0,     0,     0,   263,   263,   761,   761,   761,   761,   761,
       0,     0,     0,     0,     0,     0,     0,     0,   761,     0,
       0,     0,     0,   765,   216,     0,     0,     0,   765,     0,
     765,     0,   765,     0,   766,   765,     0,   765,     0,     0,
     765,     0,     0,   361,   362,   363,     0,     0,     0,  1405,
       0,     0,  1415,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   364,     0,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,     0,   386,     0,     0,     0,
     263,     0,     0,     0,     0,     0,     0,     0,   387,   216,
       0,  -912,  -912,  -912,  -912,   998,   999,  1000,  1001,  1002,
    1003,  1004,  1005,   263,   765,     0,     0,     0,  1094,     0,
       0,     0,     0,     0,     0,   263,  1006,  1474,  1475,     0,
       0,     0,   766,   761,     0,     0,   766,   766,   766,     0,
       0,   766,   766,   766,   766,   766,   766,   766,   766,   766,
     766,   766,   766,   766,   766,   766,   766,   766,   766,   766,
     766,   766,   766,   766,   766,   766,   766,   766,   430,   401,
     402,   403,   404,   405,   406,   407,   408,   409,   410,   411,
     412,     0,     0,     0,     0,     0,     0,     0,    36,     0,
     212,   766,     0,     0,   765,   765,   765,     0,     0,     0,
       0,     0,   765,  1616,     0,     0,     0,     0,     0,  1415,
       0,     0,   263,     0,   263,     0,   739,   413,   414,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   761,     0,     0,     0,   761,   761,   761,  1118,     0,
     761,   761,   761,   761,   761,   761,   761,   761,   761,   761,
     761,   761,   761,   761,   761,   761,   761,   761,   761,   761,
     761,   761,   761,   761,   761,   761,   761,    84,    85,     0,
      86,   184,    88,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   415,   416,     0,     0,     0,     0,     0,     0,
     761,     0,     0,     0,   766,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   766,
       0,   766,     0,     0,     0,   652,     0,   117,     0,     0,
       0,     0,   263,     0,   263,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   766,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   765,   263,
       0,   361,   362,   363,     0,     0,     0,     0,     0,   765,
       0,     0,     0,     0,   765,     0,     0,   765,     0,     0,
     364,     0,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   761,   386,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   263,   387,     0,   761,     0,
     761,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   765,   386,   760,
       0,     0,     0,   761,     0,  1722,     0,     0,     0,     0,
     387,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1405,     0,     0,     0,     0,     0,     0,     0,   766,   361,
     362,   363,     0,   766,     0,   766,     0,   766,     0,   263,
     766,     0,   766,     0,     0,   766,   760,     0,   364,     0,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,     0,   386,     0,   361,   362,   363,     0,     0,     0,
       0,     0,     0,     0,   387,     0,     0,     0,     0,     0,
       0,     0,     0,   364,     0,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,  1436,   386,     0,   766,
     263,     0,   263,     0,     0,     0,     0,   761,     0,   387,
       0,     0,   761,     0,   761,     0,   761,     0,     0,   761,
       0,   761,     0,     0,   761,     0,     0,   361,   362,   363,
       0,     0,   263,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   263,     0,     0,   364,     0,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,     0,
     386,     0,     0,     0,     0,     0,     0,     0,     0,   766,
     766,   766,   387,     0,     0,     0,     0,   766,     0,     0,
       0,     0,     0,  1620,     0,     0,     0,     0,   761,     0,
       0,     0,     0,     0,  1437,     0,     0,     0,     0,     0,
     263,     0,     0,     0,     0,     0,   263,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     760,     0,     0,     0,     0,     0,     0,   263,     0,   263,
       0,  1015,     0,   263,   760,   760,   760,   760,   760,     0,
       0,     0,     0,     0,     0,     0,     0,   760,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   361,   362,   363,     0,     0,     0,     0,   761,   761,
     761,     0,     0,     0,     0,     0,   761,     0,     0,     0,
     364,   263,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,     0,   386,     0,     0,     0,     0,     0,
     388,     0,     0,     0,     0,     0,   387,     0,     0,     0,
       0,     0,     0,   766,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   766,     0,     0,     0,     0,   766,
       0,     0,   766,     0,     0,     0,   361,   362,   363,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   760,     0,     0,   364,  1699,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,     0,   386,
       0,     0,     0,     0,     0,     0,     0,     0,   263,     0,
       0,   387,   766,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1659,     0,     0,
       0,     0,   761,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   761,     0,     0,     0,     0,   761,     0,
       0,   761,     0,     0,    36,     0,   212,     0,     0,     0,
       0,     0,     0,     0,   471,     0,     0,     0,     0,     0,
     760,     0,     0,   263,   760,   760,   760,     0,     0,   760,
     760,   760,   760,   760,   760,   760,   760,   760,   760,   760,
     760,   760,   760,   760,   760,   760,   760,   760,   760,   760,
     760,   760,   760,   760,   760,   760,     0,     0,     0,     0,
       0,   761,   989,   990,   991,   992,   993,   994,   995,   996,
     997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,   760,
       0,   679,     0,    84,    85,     0,    86,   184,    88,     0,
       0,     0,  1006,   263,     0,     0,     0,     0,     0,   473,
       0,     5,     6,     7,     8,     9,     0,     0,   263,     0,
       0,    10,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,    11,    12,     0,   457,     0,
       0,   680,     0,   117,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
      40,     0,   760,     0,    41,    42,    43,    44,     0,    45,
       0,    46,     0,    47,     0,     0,    48,   760,     0,   760,
      49,    50,    51,    52,     0,    54,    55,     0,    56,     0,
      58,    59,    60,    61,   179,   180,    64,     0,    65,    66,
      67,     0,   760,     0,     0,     0,     0,     0,     0,    71,
      72,     0,    73,    74,    75,    76,    77,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,     0,   183,    80,
      81,    82,    83,     0,    84,    85,     0,    86,   184,    88,
       0,     0,     0,    90,     0,     0,    91,     5,     6,     7,
       8,     9,    92,    93,    94,    95,     0,    10,     0,     0,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,     0,     0,   114,
       0,   115,   116,     0,   117,   118,     0,   119,   120,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,   760,     0,     0,     0,
      41,   760,     0,   760,     0,   760,     0,     0,   760,     0,
     760,     0,     0,   760,     0,     0,   195,     0,     0,    52,
       0,     0,     0,     0,     0,     0,     0,    59,    60,    61,
     179,   180,   181,     0,     0,    66,    67,     0,     0,     0,
       0,     0,     0,     0,     0,   182,    72,     0,    73,    74,
      75,    76,    77,     0,     0,     0,     0,     0,     0,    78,
       0,     0,     0,     0,   183,    80,    81,    82,    83,     0,
      84,    85,     0,    86,   184,    88,     0,     0,     0,    90,
       0,     0,    91,     0,     0,     0,     0,   760,    92,    93,
      94,    95,     0,     0,     0,     0,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,     0,     0,   114,     0,   361,   362,   363,
     117,   118,     0,   119,   120,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   364,     0,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,     0,
     386,     0,     0,     0,     0,     0,     0,   760,   760,   760,
       0,     0,   387,     0,     0,   760,     0,     5,     6,     7,
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
    1011,  1012,     0,     0,     0,    71,    72,     0,    73,    74,
      75,    76,    77,     0,     0,     0,     0,     0,     0,    78,
       0,   760,     0,     0,    79,    80,    81,    82,    83,     0,
      84,    85,   760,    86,    87,    88,    89,   760,     0,    90,
     760,     0,    91,     0,     0,     0,     0,     0,    92,    93,
      94,    95,    96,     0,    97,     0,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,     0,     0,   114,     0,   115,   116,  1045,
     117,   118,     0,   119,   120,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
     760,     0,     0,     0,     0,     0,     0,     0,     0,    11,
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
      91,     0,     0,     0,     0,     0,    92,    93,    94,    95,
      96,     0,    97,     0,    98,    99,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,     0,     0,   114,     0,   115,   116,  1218,   117,   118,
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
       0,   115,   116,   592,   117,   118,     0,   119,   120,     5,
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
     116,  1018,   117,   118,     0,   119,   120,     5,     6,     7,
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
     111,   112,   113,     0,     0,   114,     0,   115,   116,  1061,
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
     113,     0,     0,   114,     0,   115,   116,  1124,   117,   118,
       0,   119,   120,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,    40,     0,     0,     0,    41,    42,    43,    44,
    1126,    45,     0,    46,     0,    47,     0,     0,    48,     0,
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
       0,    46,     0,    47,  1281,     0,    48,     0,     0,     0,
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
     116,  1399,   117,   118,     0,   119,   120,     5,     6,     7,
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
     111,   112,   113,     0,     0,   114,     0,   115,   116,  1612,
     117,   118,     0,   119,   120,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,    40,     0,     0,     0,    41,    42,
      43,    44,     0,    45,     0,    46,  1654,    47,     0,     0,
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
       0,   114,     0,   115,   116,  1687,   117,   118,     0,   119,
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
       0,   115,   116,  1688,   117,   118,     0,   119,   120,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,    40,     0,
       0,     0,    41,    42,    43,    44,     0,    45,  1691,    46,
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
     111,   112,   113,     0,     0,   114,     0,   115,   116,  1707,
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
     113,     0,     0,   114,     0,   115,   116,  1762,   117,   118,
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
       0,   114,     0,   115,   116,  1769,   117,   118,     0,   119,
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
       0,   115,   116,     0,   117,   118,     0,   119,   120,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,   725,     0,     0,     0,
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
       0,    11,    12,     0,   945,     0,     0,     0,     0,     0,
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
      12,     0,  1462,     0,     0,     0,     0,     0,     0,     0,
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
    1603,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
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
       0,     0,     0,   661,    12,     0,     0,     0,     0,     0,
       0,   662,     0,     0,     0,     0,     0,     0,     0,     0,
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
     267,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,     0,     0,   114,     0,     0,
       0,     0,   117,   118,     0,   119,   120,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,     0,     0,     0,     0,     0,     0,     0,
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
      94,    95,     0,     0,     0,     0,    98,    99,   267,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,     0,     0,   114,     0,   268,     0,     0,
     117,   118,     0,   119,   120,     5,     6,     7,     8,     9,
       0,     0,     0,     0,   364,    10,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   607,   386,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
     387,     0,     0,     0,    16,     0,    17,    18,    19,    20,
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
       0,    86,   184,    88,     0,   608,     0,    90,     0,     0,
      91,     0,     0,     0,     0,     0,    92,    93,    94,    95,
       0,     0,     0,     0,    98,    99,     0,   100,   101,   102,
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
       0,     0,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,     0,
       0,   114,   362,   363,   720,     0,   117,   118,     0,   119,
     120,     5,     6,     7,     8,     9,     0,     0,     0,     0,
     364,    10,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,  1072,   386,     0,     0,     0,     0,     0,
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
       0,  1073,     0,    90,     0,     0,    91,     0,     0,     0,
       0,     0,    92,    93,    94,    95,     0,     0,     0,     0,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,     0,     0,   114,
       0,     0,     0,     0,   117,   118,     0,   119,   120,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   661,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
     106,   107,   108,   109,   110,   111,   112,     0,     0,   221,
     613,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,   114,     0,   250,     0,     0,   117,   118,     0,   119,
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
     109,   110,   111,   112,   113,     0,     0,   114,     0,   253,
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
      91,     0,     0,     0,     0,     0,    92,    93,    94,    95,
       0,     0,     0,     0,    98,    99,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,     0,     0,   114,   455,     0,     0,     0,   117,   118,
       0,   119,   120,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,  -912,  -912,  -912,  -912,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     620,   386,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   387,     0,     0,    14,    15,     0,     0,
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
     184,    88,     0,     0,     0,    90,     0,     0,    91,     0,
       0,     0,     0,     0,    92,    93,    94,    95,     0,     0,
       0,     0,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,    84,
      85,   114,    86,   184,    88,     0,   117,   118,     0,   119,
     120,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   662,   878,     0,     0,     0,     0,     0,
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
       0,     0,     0,    90,     0,   651,    91,     0,     0,     0,
       0,     0,    92,    93,    94,    95,     0,     0,     0,     0,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,    84,    85,   114,
      86,   184,    88,     0,   117,   118,     0,   119,   120,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   704,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,    90,     0,  1165,    91,     0,     0,     0,     0,     0,
      92,    93,    94,    95,     0,     0,     0,     0,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,    84,    85,   114,    86,   184,
      88,     0,   117,   118,     0,   119,   120,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   706,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,     0,
      41,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    52,
       0,     0,     0,     0,     0,     0,     0,    59,    60,    61,
     179,   180,   181,     0,    36,    66,    67,     0,     0,     0,
       0,     0,     0,     0,     0,   182,    72,     0,    73,    74,
      75,    76,    77,     0,     0,     0,     0,     0,     0,    78,
       0,     0,     0,     0,   183,    80,    81,    82,    83,     0,
      84,    85,     0,    86,   184,    88,     0,     0,     0,    90,
       0,     0,    91,     0,     0,     0,     0,     0,    92,    93,
      94,    95,     0,     0,     0,     0,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,    84,    85,   114,    86,   184,    88,     0,
     117,   118,     0,   119,   120,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,  1114,     0,     0,
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
       0,     0,     0,     0,   117,   118,     0,   119,   120,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,   549,    38,     0,     0,     0,     0,     0,     0,
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
     111,   112,   113,     0,     0,   114,     0,     0,     0,     0,
     117,   118,     0,   119,   120,  1485,  1486,  1487,  1488,  1489,
       0,     0,  1490,  1491,  1492,  1493,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1494,
    1495,     0,   984,     0,   985,   986,   987,   988,   989,   990,
     991,   992,   993,   994,   995,   996,   997,   998,   999,  1000,
    1001,  1002,  1003,  1004,  1005,  1496,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1006,  1497,
    1498,  1499,  1500,  1501,  1502,  1503,     0,     0,     0,    36,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1504,
    1505,  1506,  1507,  1508,  1509,  1510,  1511,  1512,  1513,  1514,
    1515,  1516,  1517,  1518,  1519,  1520,  1521,  1522,  1523,  1524,
    1525,  1526,  1527,  1528,  1529,  1530,  1531,  1532,  1533,  1534,
    1535,  1536,  1537,  1538,  1539,  1540,  1541,  1542,  1543,  1544,
       0,     0,     0,  1545,  1546,     0,  1547,  1548,  1549,  1550,
    1551,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1552,  1553,  1554,     0,     0,     0,    84,    85,
       0,    86,   184,    88,  1555,     0,  1556,  1557,     0,  1558,
       0,     0,     0,     0,     0,     0,  1559,     0,     0,     0,
    1560,     0,  1561,     0,  1562,  1563,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     361,   362,   363,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   364,
       0,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,     0,   386,   361,   362,   363,     0,     0,     0,
       0,     0,     0,     0,     0,   387,     0,     0,     0,     0,
       0,     0,     0,   364,     0,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,     0,   386,   361,   362,
     363,     0,     0,     0,     0,     0,     0,     0,     0,   387,
       0,     0,     0,     0,     0,     0,     0,   364,     0,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
       0,   386,   361,   362,   363,     0,     0,     0,     0,     0,
       0,     0,     0,   387,     0,     0,     0,     0,     0,     0,
       0,   364,     0,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,     0,   386,   981,   982,   983,     0,
       0,     0,     0,     0,     0,     0,     0,   387,     0,     0,
       0,     0,     0,   484,     0,   984,     0,   985,   986,   987,
     988,   989,   990,   991,   992,   993,   994,   995,   996,   997,
     998,   999,  1000,  1001,  1002,  1003,  1004,  1005,     0,   361,
     362,   363,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1006,     0,     0,     0,     0,     0,   508,   364,     0,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,     0,   386,   361,   362,   363,     0,     0,     0,     0,
       0,     0,     0,     0,   387,     0,     0,     0,     0,     0,
     695,     0,   364,     0,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,     0,   386,   361,   362,   363,
      36,     0,   212,     0,     0,     0,     0,     0,   387,     0,
       0,     0,     0,     0,   717,     0,   364,     0,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,   255,
     386,   213,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   387,     0,   541,     0,     0,     0,  1164,     0,
       0,     0,     0,     0,     0,   256,     0,     0,     0,     0,
       0,     0,     0,   183,     0,     0,    82,    83,     0,    84,
      85,     0,    86,   184,    88,     0,     0,    36,     0,  1655,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   255,     0,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,     0,   214,     0,     0,     0,     0,   117,
       0,   256,  1471,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   257,   258,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
     183,     0,     0,    82,   259,     0,    84,    85,     0,    86,
     184,    88,     0,     0,     0,  1315,     0,     0,     0,     0,
       0,     0,     0,     0,   260,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   257,   258,
       0,   261,   255,     0,     0,  1583,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   183,     0,     0,    82,
     259,     0,    84,    85,     0,    86,   184,    88,   256,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     260,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      36,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,     0,   261,   255,     0,
       0,  1650,     0,     0,     0,     0,     0,     0,  -329,     0,
       0,     0,     0,     0,     0,     0,    59,    60,    61,   179,
     180,   351,     0,     0,   256,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   257,   258,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,   183,     0,     0,    82,   259,     0,    84,
      85,     0,    86,   184,    88,     0,     0,     0,     0,     0,
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
      88,     0,     0,     0,    90,     0,     0,     0,     0,     0,
       0,     0,     0,   754,   755,   756,   757,     0,     0,     0,
     213,    98,     0,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   745,   746,     0,
     758,     0,     0,   747,     0,   748,     0,     0,     0,     0,
       0,     0,   183,     0,     0,    82,    83,   749,    84,    85,
       0,    86,   184,    88,     0,    33,    34,    35,    36,     0,
      91,     0,     0,     0,     0,     0,     0,   750,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,     0,   437,     0,     0,     0,     0,   117,     0,
       0,    36,     0,   212,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   751,     0,    73,    74,    75,    76,    77,
       0,     0,     0,     0,     0,     0,   752,     0,     0,     0,
       0,   183,    80,    81,    82,   753,     0,    84,    85,     0,
      86,   184,    88,     0,     0,     0,    90,     0,   651,     0,
       0,     0,     0,     0,     0,   754,   755,   756,   757,   907,
     908,     0,     0,    98,     0,     0,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   909,
      84,    85,   758,    86,   184,    88,     0,   910,   911,   912,
      36,     0,     0,     0,     0,     0,     0,   896,     0,   913,
       0,     0,     0,     0,     0,     0,    36,     0,   212,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,     0,     0,     0,     0,   652,     0,
     117,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   914,     0,   213,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   915,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    84,
      85,     0,    86,   184,    88,     0,     0,     0,     0,   183,
       0,     0,    82,    83,     0,    84,    85,   916,    86,   184,
      88,    36,     0,   212,     0,     0,     0,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,     0,
     214,     0,   213,     0,     0,   117,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1040,    36,     0,   212,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   183,     0,     0,    82,    83,     0,
      84,    85,     0,    86,   184,    88,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   213,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,    36,     0,   212,   214,     0,     0,     0,   183,
     117,     0,    82,    83,     0,    84,    85,     0,    86,   184,
      88,     0,     0,     0,     0,    36,     0,   212,     0,     0,
       0,     0,     0,     0,   563,     0,     0,     0,     0,     0,
       0,     0,     0,   226,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,     0,
     214,     0,     0,     0,     0,   117,   213,     0,     0,    36,
       0,   212,     0,     0,     0,   183,     0,     0,    82,    83,
       0,    84,    85,     0,    86,   184,    88,     0,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,   183,     0,
       0,    82,    83,     0,    84,    85,     0,    86,   184,    88,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,     0,   227,     0,     0,     0,
       0,   117,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   679,    36,    84,    85,
       0,    86,   184,    88,   564,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   183,     0,     0,    82,     0,     0,
      84,    85,     0,    86,   184,    88,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,  1419,     0,     0,     0,   712,     0,   117,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,  1097,  1098,  1099,    36,     0,     0,     0,  1657,
       0,     0,     0,     0,     0,     0,    84,    85,     0,    86,
     184,    88,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   981,   982,   983,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     984,  1420,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,  1002,
    1003,  1004,  1005,     0,    84,    85,     0,    86,   184,    88,
       0,     0,     0,     0,     0,     0,  1006,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   361,   362,   363,
       0,     0,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   364,     0,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,     0,
     386,   361,   362,   363,     0,     0,     0,     0,     0,     0,
       0,  1148,   387,     0,     0,     0,     0,     0,     0,     0,
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
     984,  1304,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,  1002,
    1003,  1004,  1005,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1006,   981,   982,   983,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   984,  1309,   985,   986,
     987,   988,   989,   990,   991,   992,   993,   994,   995,   996,
     997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,    36,
       0,   981,   982,   983,     0,     0,     0,     0,   738,     0,
       0,     0,  1006,     0,     0,    36,     0,     0,     0,     0,
     984,  1388,   985,   986,   987,   988,   989,   990,   991,   992,
     993,   994,   995,   996,   997,   998,   999,  1000,  1001,  1002,
    1003,  1004,  1005,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1006,  1407,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
    1408,  1409,   183,     0,    36,    82,     0,  1469,    84,    85,
       0,    86,   184,    88,     0,     0,     0,     0,   183,   276,
     277,    82,  1410,     0,    84,    85,     0,    86,  1411,    88,
       0,     0,     0,     0,     0,     0,     0,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
      36,  1470,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,    36,   278,   820,   821,
      84,    85,     0,    86,   184,    88,     0,   183,     0,     0,
      82,    83,     0,    84,    85,     0,    86,   184,    88,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,    36,     0,   348,     0,    84,
      85,     0,    86,   184,    88,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,    84,    85,     0,    86,   184,
      88,     0,     0,     0,     0,     0,     0,     0,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,     0,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   509,     0,     0,    84,    85,     0,    86,   184,    88,
       0,     0,     0,     0,   513,     0,     0,    84,    85,     0,
      86,   184,    88,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,     0,     0,   278,     0,     0,    84,    85,     0,    86,
     184,    88,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     361,   362,   363,     0,     0,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   729,   364,
       0,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,     0,   386,     0,   361,   362,   363,     0,     0,
       0,     0,     0,     0,     0,   387,     0,     0,     0,     0,
       0,     0,     0,     0,   364,   881,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   730,   386,   361,
     362,   363,     0,     0,     0,     0,     0,     0,     0,     0,
     387,     0,     0,     0,     0,     0,     0,     0,   364,     0,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,     0,   386,   981,   982,   983,     0,     0,     0,     0,
       0,     0,     0,     0,   387,     0,     0,     0,     0,     0,
       0,     0,   984,  1314,   985,   986,   987,   988,   989,   990,
     991,   992,   993,   994,   995,   996,   997,   998,   999,  1000,
    1001,  1002,  1003,  1004,  1005,   981,   982,   983,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1006,     0,
       0,     0,     0,     0,   984,     0,   985,   986,   987,   988,
     989,   990,   991,   992,   993,   994,   995,   996,   997,   998,
     999,  1000,  1001,  1002,  1003,  1004,  1005,   982,   983,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1006,     0,     0,     0,     0,   984,     0,   985,   986,   987,
     988,   989,   990,   991,   992,   993,   994,   995,   996,   997,
     998,   999,  1000,  1001,  1002,  1003,  1004,  1005,   363,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1006,     0,     0,     0,   364,     0,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,   983,   386,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   387,     0,     0,     0,   984,     0,   985,   986,   987,
     988,   989,   990,   991,   992,   993,   994,   995,   996,   997,
     998,   999,  1000,  1001,  1002,  1003,  1004,  1005,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1006,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,     0,   386,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   387,   985,   986,   987,
     988,   989,   990,   991,   992,   993,   994,   995,   996,   997,
     998,   999,  1000,  1001,  1002,  1003,  1004,  1005,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1006,   986,   987,   988,   989,   990,   991,   992,   993,
     994,   995,   996,   997,   998,   999,  1000,  1001,  1002,  1003,
    1004,  1005,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1006,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,     0,   386,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   387,   987,
     988,   989,   990,   991,   992,   993,   994,   995,   996,   997,
     998,   999,  1000,  1001,  1002,  1003,  1004,  1005,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1006,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
       0,   386,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   387,   988,   989,   990,   991,   992,   993,
     994,   995,   996,   997,   998,   999,  1000,  1001,  1002,  1003,
    1004,  1005,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1006,   990,   991,   992,   993,
     994,   995,   996,   997,   998,   999,  1000,  1001,  1002,  1003,
    1004,  1005,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1006,  -912,  -912,  -912,  -912,
     994,   995,   996,   997,   998,   999,  1000,  1001,  1002,  1003,
    1004,  1005,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1006
};

static const yytype_int16 yycheck[] =
{
       5,     6,   136,     8,     9,    10,    11,    12,   163,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,   186,    89,    28,    29,   610,    32,   582,     4,    96,
      97,    53,   427,     4,     4,     4,   316,    42,    44,   858,
      54,   166,    48,  1064,    49,   400,    51,   697,   581,    54,
     386,    56,   728,   187,   228,   163,   839,   735,   234,   421,
     422,   453,   453,   556,   877,   114,   175,     4,   135,  1047,
       4,     9,   114,     9,   457,   938,    30,     9,    30,    45,
     893,    30,     9,    45,     4,     9,     9,     9,     9,   451,
       9,     9,    14,     9,     9,     9,    57,     9,     9,     9,
       9,    45,    45,     9,     9,    79,   114,     9,     9,   114,
     235,     9,     9,  1060,    33,    66,     9,     9,    79,    79,
      50,    82,   505,   109,    66,   301,    66,    66,     9,    79,
      50,   128,   129,    53,   155,    35,   117,     0,     9,  1597,
       9,    14,     4,   124,   100,   101,    45,  1049,   128,   129,
      70,    79,    85,    79,    79,   100,   101,    30,   113,   114,
     115,   116,   117,   118,   149,   214,   149,    87,    85,    89,
      53,    66,   170,   201,    47,   201,    96,    97,   227,    79,
      66,   167,    65,   204,   201,   170,   191,   170,   353,   128,
     129,   974,   390,  1651,     8,    35,    66,   201,   149,    66,
     583,    66,    66,   201,    46,    47,   214,   316,   205,   201,
      66,    66,   201,   146,   224,   135,    66,    66,   201,   204,
     418,   204,   202,   201,   154,   423,   163,   204,   183,   146,
      30,   236,   206,   199,   239,   163,   202,   163,   163,    79,
     202,   246,   247,    66,   201,   294,   206,   171,   171,   205,
     204,   203,   294,   204,    66,    66,   206,   171,   202,   202,
     205,   201,   204,   163,   429,   399,   202,   205,  1131,   240,
     948,   203,   950,   244,   136,  1253,   203,   204,  1091,   199,
     347,   203,   203,   202,   393,   203,   294,   203,   203,   294,
      79,   203,   203,   203,   203,   300,  1243,   203,   203,   304,
     199,   203,   203,    66,   202,   202,   201,   202,   417,   202,
     202,    66,   317,   318,   319,   276,   277,   278,   204,   874,
     240,   202,   344,   163,   244,   187,  1109,   332,   248,   396,
     397,   202,   441,   202,   204,   201,   829,   720,   343,   204,
     204,   450,   725,  1245,   453,   201,   266,   308,   204,   204,
    1252,   201,  1254,   201,   204,   204,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   204,   387,    79,   389,   390,   400,   392,   437,    35,
      35,   325,   204,   204,   331,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,  1355,   198,
    1357,   331,   588,   418,   419,   204,   421,   422,   423,   424,
     340,   816,   170,    50,   344,   430,   436,   347,   433,   437,
      79,   170,   201,    79,    79,    97,    66,   386,   443,   116,
     445,   204,   201,   598,   393,   459,   451,   124,   154,   204,
     170,  1127,   201,   201,   459,    79,   461,   128,   129,   201,
    1110,    85,   201,  1365,    97,   201,     4,   163,   417,   331,
     862,   862,    79,   393,   394,   395,   396,   397,    85,   653,
     201,   201,   487,    97,   153,   490,   491,   492,    26,    27,
     598,   153,   441,   464,    97,   660,    79,   417,   128,   129,
     149,   450,  1449,    79,   453,   113,   114,   115,   116,   117,
     118,   685,    97,    79,   204,  1298,   521,   163,   163,    85,
     153,   441,  1077,   147,   148,  1080,   702,   154,   890,   703,
    1063,   202,   708,   453,   926,   926,   398,    26,    27,   153,
     147,   148,   169,   153,   464,    79,   128,   129,   509,   616,
     153,    85,   513,    35,    97,    79,   566,   518,     4,   839,
     896,    85,   945,   573,   147,   148,   576,    29,   153,   201,
     146,   147,   148,    66,   204,   183,   114,    35,   155,    66,
     146,   147,   148,   708,    46,   759,   210,    49,   203,   204,
    1373,   168,    35,   203,   204,   203,    66,    79,   203,    45,
     520,   203,   209,   608,    99,   100,   101,   203,   105,  1259,
     153,  1261,   146,   147,   148,   620,   113,   114,   115,   116,
     117,   118,    79,   147,   148,   545,   546,   204,    85,   203,
     113,   114,   115,   116,   117,   118,    79,    29,   126,   127,
     650,   124,   125,    49,    50,    51,    79,   652,   582,    99,
     100,   101,    85,  1600,    46,   203,   661,    49,   203,    65,
     106,   598,    49,    50,    51,   111,    53,   113,   114,   115,
     116,   117,   118,   119,   203,   680,   214,   160,    65,   162,
     118,   119,   120,   221,   203,   204,   183,  1042,   204,   227,
     147,   148,  1247,   149,   974,   201,   616,  1627,  1628,  1232,
     183,   201,   240,   113,   114,   115,   244,   712,    71,    72,
     156,   157,  1733,   159,   147,   148,    71,    72,   724,  1623,
    1624,  1371,    66,   728,  1086,   214,   149,  1748,   743,   744,
     839,   741,   221,  1095,   201,  1130,   153,   183,   227,   113,
     114,   115,   116,   117,   118,   283,    44,    65,   203,   170,
     124,   125,   149,   862,   292,   293,   294,   201,     9,   205,
     208,   299,   733,   149,   769,   149,   302,   305,     8,   201,
     306,    46,    47,    48,    49,    50,    51,   697,    53,   699,
    1727,   718,   170,   203,    14,   201,    79,   124,   162,   203,
      65,   203,   124,   331,   283,  1742,    14,   333,   718,   335,
     336,   337,   338,   292,   293,   294,  1456,   202,   170,   183,
     299,  1366,   732,   733,    14,    97,   305,   926,  1213,   202,
     825,  1204,   202,   202,   201,   207,   105,   788,   895,  1109,
     201,   792,   201,   838,     9,   202,   202,    89,     9,   844,
      14,   846,   203,   848,   201,     9,   783,   187,    79,   190,
      79,    79,   828,   201,   203,     9,   718,   828,   828,   828,
     184,   185,   186,   783,   203,   974,   871,   191,   192,   789,
       9,   195,   196,   793,   794,    79,   881,   202,   126,   884,
     202,   886,   202,  1702,   203,   890,   201,  1249,  1271,   427,
     202,   828,    30,   813,   828,   127,  1070,    66,   169,   437,
       9,  1720,   202,   130,  1287,   149,   202,     9,   828,  1728,
     920,   202,     9,   862,   202,    14,   199,     9,    66,   856,
     925,   783,   842,     9,   858,   931,   464,    46,    47,    48,
      49,    50,    51,   171,   202,     9,   856,    14,   427,   126,
     874,   208,   862,   208,   205,     9,    65,   896,   437,   208,
     208,   201,  1602,   963,    97,   202,   202,   202,   201,   149,
     970,   932,   203,   130,   203,     9,   828,   202,   201,   149,
     149,   187,   187,    14,     9,   895,   201,   926,   201,   201,
     201,    79,    14,    14,   204,   204,   203,   907,   908,   909,
      14,  1353,  1375,   204,   856,   208,   204,   934,   203,   936,
    1109,  1384,   199,  1008,  1009,  1010,   926,    30,   201,  1014,
    1015,    30,   932,   201,   934,  1398,   936,   201,  1298,    14,
     201,     9,   130,   201,    14,   202,     9,   203,  1042,   201,
     203,    65,   202,     9,   146,   955,    79,  1042,    97,     9,
     208,   255,   256,   201,   203,   130,    14,   261,   204,    79,
     202,   201,   201,   973,   130,   202,     9,   208,   204,   979,
    1645,    30,   204,   146,    73,   171,   203,   202,  1073,   607,
    1046,   203,   934,   130,   936,  1046,  1046,  1046,    30,  1462,
     202,  1086,   202,   130,     9,   202,   202,     9,   202,     9,
    1095,  1096,   202,  1373,   205,  1105,     9,   202,   205,    14,
     204,    79,  1022,   204,   130,  1025,   201,     9,   202,  1046,
     203,   202,  1046,  1119,   202,   202,    30,   201,   607,   106,
     202,   202,  1127,   202,   662,   203,  1046,   341,   203,   202,
      97,   158,  1137,  1718,   204,    14,   154,    79,   111,   202,
     202,   130,    14,  1077,   204,  1155,  1080,   202,   130,  1159,
     170,  1122,  1162,   204,   203,    79,    14,   202,    79,  1169,
      14,   201,   204,   202,   130,   203,   704,   203,   706,    14,
     203,    14,    55,   662,    79,   204,   201,     9,    79,    79,
     718,  1336,     4,   109,  1046,    97,  1106,   203,   149,  1298,
    1110,    97,   730,    33,    14,   733,   161,  1134,   201,  1333,
     202,    79,  1122,   203,   201,   167,   164,   171,   202,  1592,
       9,  1594,    79,   204,  1134,   704,   202,   706,  1223,   203,
    1603,   202,  1227,    45,  1229,  1230,    14,    79,    14,    79,
      14,    79,    14,  1238,   518,    79,   788,   792,  1710,   394,
     397,   730,   894,  1248,  1249,   783,  1724,   891,  1128,  1461,
     831,   396,  1284,  1720,   523,  1452,  1483,  1267,  1567,  1328,
    1425,  1752,   476,   801,  1373,  1648,  1740,  1579,    42,    74,
      75,    76,  1134,  1448,  1211,   400,  1017,  1211,   816,   817,
    1014,    86,   494,  1220,   106,   494,  1220,  1093,  1208,   111,
     828,   113,   114,   115,   116,   117,   118,   119,  1050,   780,
    1322,   971,   908,  1106,   922,   862,   319,   300,   293,   977,
    1315,  1033,   801,  1247,   743,   956,    -1,    -1,   856,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   816,   817,   134,
     135,   136,   137,   138,   156,   157,    -1,   159,    -1,  1259,
     145,  1261,    -1,    -1,    -1,  1350,   151,   152,  1353,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     165,   183,    -1,  1746,   578,   579,    -1,    -1,    -1,    -1,
    1753,    -1,    -1,   587,    -1,    -1,    -1,   182,    -1,    -1,
    1317,    -1,    -1,   205,  1406,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   932,    -1,   934,  1317,   936,  1336,
     938,   939,  1322,    -1,    -1,  1427,    -1,  1429,  1328,    -1,
    1381,    -1,    -1,    -1,    -1,    -1,    -1,  1582,    -1,  1356,
      -1,    -1,  1356,    -1,    -1,  1362,    -1,  1364,  1362,    -1,
    1364,    -1,  1366,    -1,    -1,  1440,    -1,    -1,    -1,  1376,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1385,   938,
     939,  1371,    -1,    -1,    -1,  1317,  1376,    -1,    -1,    -1,
      -1,  1381,    -1,    -1,    -1,  1385,  1471,    -1,    -1,    -1,
    1332,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,  1406,    -1,    -1,  1460,
    1461,    -1,    -1,    -1,  1414,    -1,    65,    -1,    -1,    -1,
    1420,    -1,    -1,    -1,    -1,    -1,    -1,  1427,  1046,  1429,
      -1,  1578,    -1,  1433,  1376,    -1,    -1,  1454,    -1,     4,
    1454,    -1,  1459,  1385,    -1,    -1,    -1,    -1,  1465,   743,
     744,    -1,    -1,    -1,  1072,  1472,  1456,    -1,    -1,  1459,
    1460,  1461,  1697,    -1,    -1,  1465,    -1,    -1,    -1,    -1,
      -1,    -1,  1472,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1431,
      -1,    -1,  1639,    -1,    -1,    -1,  1114,    -1,    -1,     4,
    1714,    -1,  1587,  1072,  1122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1130,  1131,    -1,    -1,  1134,  1459,    -1,    -1,
      26,    27,    -1,  1465,    30,    -1,    -1,    -1,    -1,    -1,
    1472,    -1,    -1,    -1,    -1,    -1,   830,    -1,    -1,    -1,
      45,   106,    -1,    -1,    -1,  1114,   111,    53,   113,   114,
     115,   116,   117,   118,   119,    -1,    -1,    -1,    -1,   853,
      -1,  1130,  1131,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   865,    -1,    -1,    -1,    -1,    -1,    -1,  1578,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1607,   156,   157,    -1,   159,  1213,    -1,    -1,  1598,    -1,
      -1,   106,  1602,    -1,    -1,    -1,   111,  1607,   113,   114,
     115,   116,   117,   118,   119,    -1,    -1,  1617,   183,    -1,
      -1,     4,    -1,  1623,  1624,    -1,    -1,  1627,  1628,  1646,
    1647,    -1,    -1,    -1,    -1,    -1,  1653,    -1,    -1,  1639,
     205,    -1,    -1,    -1,  1213,    -1,  1646,  1647,    -1,    -1,
      -1,   156,   157,  1653,   159,    -1,    -1,    -1,   952,    -1,
     954,    -1,    45,    -1,    -1,  1607,    -1,    -1,    -1,    -1,
      -1,    -1,  1689,  1758,    -1,    -1,    -1,    -1,   183,    -1,
    1697,  1766,    -1,    -1,    -1,    -1,    -1,  1772,  1702,  1689,
    1775,    -1,    -1,    -1,    -1,    -1,  1696,    -1,    -1,  1317,
     205,    -1,    -1,    -1,  1646,  1647,  1720,    -1,   214,    -1,
      -1,  1653,    -1,  1713,  1728,   221,    -1,    -1,    -1,    -1,
      -1,   227,    -1,   106,    -1,    -1,    -1,    -1,   111,    -1,
     113,   114,   115,   116,   117,   118,   119,  1754,    -1,    -1,
      -1,    -1,    -1,    -1,  1761,    -1,    -1,  1689,     4,   255,
     256,    -1,    -1,    -1,  1754,   261,    -1,    -1,  1376,    -1,
      -1,  1761,    -1,  1381,    -1,    -1,    -1,  1385,  1062,    -1,
    1064,    -1,    -1,   156,   157,    -1,   159,   283,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   292,   293,    -1,    45,
      -1,    -1,    -1,   299,    -1,  1089,    -1,    -1,    -1,   305,
     183,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     316,    -1,  1754,    -1,    -1,    -1,    -1,    -1,    -1,  1761,
      -1,    -1,   205,    -1,    -1,    26,    27,    -1,    -1,    30,
      -1,    -1,    -1,    -1,    -1,   341,    -1,    -1,   344,    -1,
      -1,  1459,  1460,  1461,    -1,    -1,    -1,  1465,    -1,    -1,
     106,  1145,    -1,    -1,  1472,   111,    -1,   113,   114,   115,
     116,   117,   118,   119,    -1,    -1,    -1,    -1,    77,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     386,    -1,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
     156,   157,    -1,   159,    -1,    -1,    -1,    -1,    -1,   118,
      -1,    -1,    -1,    -1,    -1,  1209,    -1,    -1,    -1,    -1,
      -1,   427,    -1,    -1,    -1,    -1,    -1,   183,    -1,    -1,
      -1,   437,    -1,    -1,    -1,    63,    64,    -1,    -1,    -1,
      -1,   150,    -1,    -1,   153,   154,    -1,   156,   157,   205,
     159,   160,   161,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
     476,   477,    -1,    -1,   480,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,  1607,
      -1,    -1,   201,    -1,    -1,   204,  1290,   206,  1292,    -1,
     128,   129,    -1,    63,    64,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   214,    -1,    -1,    -1,    -1,    -1,    -1,
     221,   527,    10,    11,    12,    -1,   227,    -1,  1646,  1647,
      -1,    -1,    -1,    -1,    -1,  1653,    -1,    -1,    -1,  1333,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,   128,   129,
      -1,  1689,   578,   579,   202,    -1,    -1,    65,    -1,    -1,
      -1,   587,   283,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   292,   293,    -1,    -1,    -1,    -1,    -1,   299,    -1,
      -1,   607,    -1,    -1,   305,    -1,  1400,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   316,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,  1754,    53,    -1,    10,
      11,    12,   202,  1761,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,   662,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,   386,    -1,  1481,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,   704,    -1,
     706,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
     208,    -1,    -1,    -1,   730,   731,   427,    -1,    -1,    -1,
      -1,    65,    10,    11,    12,    -1,   437,   743,   744,   745,
     746,   747,   748,   749,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,   758,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,   784,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    67,    68,    -1,   801,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    79,    -1,    -1,    -1,   814,    -1,
     816,   817,    26,    27,    -1,    -1,    30,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   830,   831,    -1,    -1,    -1,   480,
     105,    -1,    -1,   839,   205,    -1,    -1,    -1,   113,   114,
     115,   116,   117,   118,    -1,    -1,    -1,   853,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   865,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   873,    -1,  1663,
     876,    -1,    -1,    -1,    -1,   150,   527,    -1,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,
     896,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   607,    -1,   183,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,    -1,    -1,   201,   205,    -1,    -1,
      -1,   206,   938,   939,    -1,    -1,    -1,    -1,    -1,  1733,
      -1,    -1,    -1,    -1,    -1,    -1,   952,    -1,   954,    -1,
     956,    -1,    -1,    -1,  1748,    -1,    -1,    -1,    -1,    -1,
      -1,   662,    -1,    -1,    -1,   971,    -1,    -1,   974,   975,
     976,   977,    -1,    -1,   980,   981,   982,   983,   984,   985,
     986,   987,   988,   989,   990,   991,   992,   993,   994,   995,
     996,   997,   998,   999,  1000,  1001,  1002,  1003,  1004,  1005,
    1006,    -1,    -1,   704,    -1,   706,    -1,   221,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1030,    -1,    -1,    -1,    -1,   730,
      35,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,  1062,    -1,  1064,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1072,    65,    -1,   283,
      -1,    -1,    77,    -1,    79,    -1,    -1,    -1,   292,   293,
     731,    -1,    -1,  1089,    -1,   299,    -1,    -1,    -1,    -1,
      -1,   305,    -1,    -1,   745,   746,   747,   748,    -1,    -1,
     801,    -1,   316,  1109,    -1,    -1,    -1,   758,  1114,    -1,
      -1,    -1,    -1,   118,    -1,   816,   817,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1130,  1131,   131,  1133,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   839,  1145,
      -1,    -1,  1148,    -1,  1150,   150,    -1,    -1,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1173,    -1,    -1,
      -1,    -1,   386,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,   896,   201,    -1,    -1,    -1,
      -1,   206,    -1,  1209,  1210,   203,    -1,  1213,    -1,    -1,
      -1,    -1,    -1,   427,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   873,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   938,   939,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,   480,    53,    -1,    -1,
      -1,    -1,    -1,   974,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,  1290,    -1,  1292,    -1,    -1,    -1,
      -1,  1297,  1298,    -1,    -1,    -1,  1302,    -1,  1304,    -1,
    1306,    -1,    -1,  1309,    -1,  1311,    -1,    -1,  1314,    -1,
      -1,    -1,    -1,   527,    -1,    -1,  1322,  1323,    -1,    -1,
    1326,    -1,    -1,    77,   975,   976,   977,  1333,    -1,   980,
     981,   982,   983,   984,   985,   986,   987,   988,   989,   990,
     991,   992,   993,   994,   995,   996,   997,   998,   999,  1000,
    1001,  1002,  1003,  1004,  1005,  1006,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1373,    -1,    -1,
      -1,  1072,    -1,    53,    -1,    -1,    -1,    -1,    -1,  1030,
      -1,    -1,  1388,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   607,  1400,  1401,  1402,    -1,    -1,   153,
    1406,    -1,   156,   157,    -1,   159,   160,   161,  1109,    -1,
      -1,    -1,    -1,  1114,    -1,    -1,    -1,    -1,    -1,   205,
      -1,  1427,    -1,  1429,    -1,    -1,    -1,  1433,    -1,  1130,
    1131,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,    -1,    -1,   662,    -1,
     204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1468,  1469,  1470,    -1,    -1,    -1,    -1,    -1,
    1476,  1477,    -1,    -1,    -1,  1481,    -1,  1483,    -1,    -1,
      -1,    -1,  1133,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     704,    -1,   706,    -1,    -1,    -1,    -1,  1148,    -1,  1150,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1213,    -1,    -1,    -1,   730,   731,    -1,    -1,
      -1,    -1,  1173,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,   745,   746,   747,   748,   749,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   758,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
     784,    -1,    -1,    -1,    -1,   255,   256,    -1,    -1,    65,
      -1,   261,    -1,    -1,    -1,    -1,    -1,   801,    -1,    -1,
      -1,    -1,  1598,    -1,    -1,    -1,    77,  1298,    -1,    -1,
     814,    -1,   816,   817,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1617,    -1,    -1,    -1,    -1,  1622,   831,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   839,    -1,  1633,    -1,    -1,
      -1,    -1,  1638,    -1,    -1,  1641,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1297,    -1,    -1,    -1,
      -1,  1302,    -1,  1304,    -1,  1306,    -1,  1663,  1309,   873,
    1311,   341,   876,  1314,   344,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1373,   154,    -1,   156,   157,   158,   159,   160,
     161,    -1,   896,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1701,    -1,    -1,    -1,    -1,
      -1,   480,    -1,  1709,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,  1724,   205,
     201,    -1,    -1,    -1,   938,   939,    -1,  1733,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1388,    -1,    -1,
      -1,    -1,  1748,    -1,    -1,    -1,    -1,    -1,   527,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   971,    -1,    -1,
     974,   975,   976,   977,    -1,    -1,   980,   981,   982,   983,
     984,   985,   986,   987,   988,   989,   990,   991,   992,   993,
     994,   995,   996,   997,   998,   999,  1000,  1001,  1002,  1003,
    1004,  1005,  1006,    -1,    -1,    -1,   476,   477,    -1,    -1,
     480,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1030,  1468,  1469,  1470,
      -1,    -1,    -1,    -1,    29,  1476,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,   527,    53,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,  1072,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,  1109,    -1,    -1,   578,   579,
    1114,    -1,    -1,    -1,    65,    -1,    -1,   587,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1130,  1131,    -1,  1133,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1148,    -1,  1150,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,   731,    -1,    -1,    -1,    -1,    -1,    -1,  1173,
      -1,    -1,    -1,    -1,    -1,    -1,   745,   746,   747,   748,
     749,  1622,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   758,
      -1,    -1,  1633,    10,    11,    12,    -1,  1638,    63,    64,
    1641,    -1,    -1,    -1,    -1,    -1,  1210,    -1,    -1,  1213,
     205,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,
    1701,   731,    -1,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   743,   744,   745,   746,   747,   748,   749,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   758,    -1,
      -1,    -1,    -1,  1297,  1298,    -1,    -1,    -1,  1302,    -1,
    1304,    -1,  1306,    -1,   873,  1309,    -1,  1311,    -1,    -1,
    1314,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,  1323,
      -1,    -1,  1326,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
     830,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,  1373,
      -1,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,   853,  1388,    -1,    -1,    -1,   205,    -1,
      -1,    -1,    -1,    -1,    -1,   865,    65,  1401,  1402,    -1,
      -1,    -1,   971,   873,    -1,    -1,   975,   976,   977,    -1,
      -1,   980,   981,   982,   983,   984,   985,   986,   987,   988,
     989,   990,   991,   992,   993,   994,   995,   996,   997,   998,
     999,  1000,  1001,  1002,  1003,  1004,  1005,  1006,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,
      79,  1030,    -1,    -1,  1468,  1469,  1470,    -1,    -1,    -1,
      -1,    -1,  1476,  1477,    -1,    -1,    -1,    -1,    -1,  1483,
      -1,    -1,   952,    -1,   954,    -1,   956,    63,    64,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   971,    -1,    -1,    -1,   975,   976,   977,   205,    -1,
     980,   981,   982,   983,   984,   985,   986,   987,   988,   989,
     990,   991,   992,   993,   994,   995,   996,   997,   998,   999,
    1000,  1001,  1002,  1003,  1004,  1005,  1006,   156,   157,    -1,
     159,   160,   161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
    1030,    -1,    -1,    -1,  1133,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,  1148,
      -1,  1150,    -1,    -1,    -1,   204,    -1,   206,    -1,    -1,
      -1,    -1,  1062,    -1,  1064,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1173,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1622,  1089,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,  1633,
      -1,    -1,    -1,    -1,  1638,    -1,    -1,  1641,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,  1133,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1145,    65,    -1,  1148,    -1,
    1150,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,  1701,    53,   480,
      -1,    -1,    -1,  1173,    -1,  1709,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1724,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1297,    10,
      11,    12,    -1,  1302,    -1,  1304,    -1,  1306,    -1,  1209,
    1309,    -1,  1311,    -1,    -1,  1314,   527,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,   205,    53,    -1,  1388,
    1290,    -1,  1292,    -1,    -1,    -1,    -1,  1297,    -1,    65,
      -1,    -1,  1302,    -1,  1304,    -1,  1306,    -1,    -1,  1309,
      -1,  1311,    -1,    -1,  1314,    -1,    -1,    10,    11,    12,
      -1,    -1,  1322,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1333,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1468,
    1469,  1470,    65,    -1,    -1,    -1,    -1,  1476,    -1,    -1,
      -1,    -1,    -1,  1482,    -1,    -1,    -1,    -1,  1388,    -1,
      -1,    -1,    -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,
    1400,    -1,    -1,    -1,    -1,    -1,  1406,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     731,    -1,    -1,    -1,    -1,    -1,    -1,  1427,    -1,  1429,
      -1,   197,    -1,  1433,   745,   746,   747,   748,   749,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   758,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,  1468,  1469,
    1470,    -1,    -1,    -1,    -1,    -1,  1476,    -1,    -1,    -1,
      29,  1481,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
     203,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,  1622,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1633,    -1,    -1,    -1,    -1,  1638,
      -1,    -1,  1641,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   873,    -1,    -1,    29,  1665,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1598,    -1,
      -1,    65,  1701,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1617,    -1,    -1,
      -1,    -1,  1622,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1633,    -1,    -1,    -1,    -1,  1638,    -1,
      -1,  1641,    -1,    -1,    77,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   203,    -1,    -1,    -1,    -1,    -1,
     971,    -1,    -1,  1663,   975,   976,   977,    -1,    -1,   980,
     981,   982,   983,   984,   985,   986,   987,   988,   989,   990,
     991,   992,   993,   994,   995,   996,   997,   998,   999,  1000,
    1001,  1002,  1003,  1004,  1005,  1006,    -1,    -1,    -1,    -1,
      -1,  1701,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,  1030,
      -1,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,    65,  1733,    -1,    -1,    -1,    -1,    -1,   203,
      -1,     3,     4,     5,     6,     7,    -1,    -1,  1748,    -1,
      -1,    13,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    27,    28,    -1,    30,    -1,
      -1,   204,    -1,   206,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      82,    -1,  1133,    -1,    86,    87,    88,    89,    -1,    91,
      -1,    93,    -1,    95,    -1,    -1,    98,  1148,    -1,  1150,
     102,   103,   104,   105,    -1,   107,   108,    -1,   110,    -1,
     112,   113,   114,   115,   116,   117,   118,    -1,   120,   121,
     122,    -1,  1173,    -1,    -1,    -1,    -1,    -1,    -1,   131,
     132,    -1,   134,   135,   136,   137,   138,    -1,    -1,    -1,
      -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,   150,   151,
     152,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    -1,   165,    -1,    -1,   168,     3,     4,     5,
       6,     7,   174,   175,   176,   177,    -1,    13,    -1,    -1,
     182,   183,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,    -1,    -1,   201,
      -1,   203,   204,    -1,   206,   207,    -1,   209,   210,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    -1,  1297,    -1,    -1,    -1,
      86,  1302,    -1,  1304,    -1,  1306,    -1,    -1,  1309,    -1,
    1311,    -1,    -1,  1314,    -1,    -1,   102,    -1,    -1,   105,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,   114,   115,
     116,   117,   118,    -1,    -1,   121,   122,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,
     136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,
      -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,
      -1,    -1,   168,    -1,    -1,    -1,    -1,  1388,   174,   175,
     176,   177,    -1,    -1,    -1,    -1,   182,   183,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,    -1,    -1,   201,    -1,    10,    11,    12,
     206,   207,    -1,   209,   210,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,  1468,  1469,  1470,
      -1,    -1,    65,    -1,    -1,  1476,    -1,     3,     4,     5,
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
     193,   194,    -1,    -1,    -1,   131,   132,    -1,   134,   135,
     136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,
      -1,  1622,    -1,    -1,   150,   151,   152,   153,   154,    -1,
     156,   157,  1633,   159,   160,   161,   162,  1638,    -1,   165,
    1641,    -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,
     176,   177,   178,    -1,   180,    -1,   182,   183,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,    -1,    -1,   201,    -1,   203,   204,   205,
     206,   207,    -1,   209,   210,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
    1701,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
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
      90,    91,    -1,    93,    -1,    95,    -1,    -1,    98,    -1,
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
      -1,    93,    -1,    95,    96,    -1,    98,    -1,    -1,    -1,
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
      88,    89,    -1,    91,    -1,    93,    94,    95,    -1,    -1,
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
      -1,    -1,    86,    87,    88,    89,    -1,    91,    92,    93,
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
      -1,    -1,   174,   175,   176,   177,    -1,    -1,    -1,    -1,
     182,   183,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,    -1,    -1,   201,
      -1,   203,   204,    -1,   206,   207,    -1,   209,   210,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     194,   195,   196,   197,   198,    -1,    -1,   201,    -1,    -1,
      -1,    -1,   206,   207,    -1,   209,   210,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     176,   177,    -1,    -1,    -1,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,    -1,    -1,   201,    -1,   203,    -1,    -1,
     206,   207,    -1,   209,   210,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    29,    13,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    35,    53,    -1,
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
      -1,   159,   160,   161,    -1,   163,    -1,   165,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,
      -1,    -1,    -1,    -1,   182,   183,    -1,   185,   186,   187,
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
      -1,    -1,   182,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,    -1,
      -1,   201,    11,    12,   204,    -1,   206,   207,    -1,   209,
     210,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      29,    13,    31,    32,    33,    34,    35,    36,    37,    38,
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
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     201,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   201,    -1,   203,    -1,    -1,   206,   207,    -1,   209,
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
     194,   195,   196,   197,   198,    -1,    -1,   201,    -1,   203,
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
     198,    -1,    -1,   201,   202,    -1,    -1,    -1,   206,   207,
      -1,   209,   210,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      30,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    46,    47,    -1,    -1,
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
     160,   161,    -1,    -1,    -1,   165,    -1,    -1,   168,    -1,
      -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,    -1,
      -1,    -1,   182,   183,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   156,
     157,   201,   159,   160,   161,    -1,   206,   207,    -1,   209,
     210,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,    35,   201,    -1,    -1,    -1,    -1,    -1,
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
      -1,   165,    -1,   124,   168,    -1,    -1,    -1,    -1,    -1,
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
     116,   117,   118,    -1,    77,   121,   122,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   131,   132,    -1,   134,   135,
     136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,   145,
      -1,    -1,    -1,    -1,   150,   151,   152,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,   165,
      -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,   174,   175,
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
      -1,    -1,    -1,    -1,   206,   207,    -1,   209,   210,    -1,
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
     196,   197,   198,    -1,    -1,   201,    -1,    -1,    -1,    -1,
     206,   207,    -1,   209,   210,     3,     4,     5,     6,     7,
      -1,    -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    67,
      68,    69,    70,    71,    72,    73,    -1,    -1,    -1,    77,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
      -1,    -1,    -1,   131,   132,    -1,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   150,   151,   152,    -1,    -1,    -1,   156,   157,
      -1,   159,   160,   161,   162,    -1,   164,   165,    -1,   167,
      -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,
     178,    -1,   180,    -1,   182,   183,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   203,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,   203,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
     202,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    10,    11,    12,
      77,    -1,    79,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,   202,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    29,
      53,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,   131,    -1,    -1,    -1,   202,    -1,
      -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   150,    -1,    -1,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    77,    -1,   190,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,   206,
      -1,    55,   189,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   132,   133,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,
     150,    -1,    -1,   153,   154,    -1,   156,   157,    -1,   159,
     160,   161,    -1,    -1,    -1,   188,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   132,   133,
      -1,   201,    29,    -1,    -1,   205,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   150,    -1,    -1,   153,
     154,    -1,   156,   157,    -1,   159,   160,   161,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     174,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      77,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,    -1,    -1,    -1,   201,    29,    -1,
      -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,   105,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   113,   114,   115,   116,
     117,   118,    -1,    -1,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   132,   133,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    -1,    -1,   150,    -1,    -1,   153,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,    -1,
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
     161,    -1,    -1,    -1,   165,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   174,   175,   176,   177,    -1,    -1,    -1,
     118,   182,    -1,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    46,    47,    -1,
     201,    -1,    -1,    52,    -1,    54,    -1,    -1,    -1,    -1,
      -1,    -1,   150,    -1,    -1,   153,   154,    66,   156,   157,
      -1,   159,   160,   161,    -1,    74,    75,    76,    77,    -1,
     168,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,   206,    -1,
      -1,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   132,    -1,   134,   135,   136,   137,   138,
      -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    -1,    -1,
      -1,   150,   151,   152,   153,   154,    -1,   156,   157,    -1,
     159,   160,   161,    -1,    -1,    -1,   165,    -1,   124,    -1,
      -1,    -1,    -1,    -1,    -1,   174,   175,   176,   177,    46,
      47,    -1,    -1,   182,    -1,    -1,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    66,
     156,   157,   201,   159,   160,   161,    -1,    74,    75,    76,
      77,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    86,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    79,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    -1,    -1,    -1,    -1,    -1,    -1,   204,    -1,
     206,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   132,    -1,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,   150,
      -1,    -1,   153,   154,    -1,   156,   157,   174,   159,   160,
     161,    77,    -1,    79,    -1,    -1,    -1,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,    -1,
     201,    -1,   118,    -1,    -1,   206,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   131,    77,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   150,    -1,    -1,   153,   154,    -1,
     156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    77,    -1,    79,   201,    -1,    -1,    -1,   150,
     206,    -1,   153,   154,    -1,   156,   157,    -1,   159,   160,
     161,    -1,    -1,    -1,    -1,    77,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   118,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    -1,    -1,    -1,
     201,    -1,    -1,    -1,    -1,   206,   118,    -1,    -1,    77,
      -1,    79,    -1,    -1,    -1,   150,    -1,    -1,   153,   154,
      -1,   156,   157,    -1,   159,   160,   161,    -1,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,   150,    -1,
      -1,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,    -1,    -1,    -1,   201,    -1,    -1,    -1,
      -1,   206,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   154,    77,   156,   157,
      -1,   159,   160,   161,   206,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   150,    -1,    -1,   153,    -1,    -1,
     156,   157,    -1,   159,   160,   161,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      -1,    -1,   122,    -1,    -1,    -1,   204,    -1,   206,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,    74,    75,    76,    77,    -1,    -1,    -1,   205,
      -1,    -1,    -1,    -1,    -1,    -1,   156,   157,    -1,   159,
     160,   161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,    -1,    -1,
      29,   201,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   130,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      43,    44,    45,    46,    47,    48,    49,    50,    51,    77,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    -1,    65,    -1,    -1,    77,    -1,    -1,    -1,    -1,
      29,   130,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,   119,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     132,   133,   150,    -1,    77,   153,    -1,   130,   156,   157,
      -1,   159,   160,   161,    -1,    -1,    -1,    -1,   150,   105,
     106,   153,   154,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
      77,   130,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,    77,   153,    79,    80,
     156,   157,    -1,   159,   160,   161,    -1,   150,    -1,    -1,
     153,   154,    -1,   156,   157,    -1,   159,   160,   161,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,    77,    -1,   154,    -1,   156,
     157,    -1,   159,   160,   161,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    -1,    -1,   156,   157,    -1,   159,   160,
     161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,    -1,    -1,    -1,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   153,    -1,    -1,   156,   157,    -1,   159,   160,   161,
      -1,    -1,    -1,    -1,   153,    -1,    -1,   156,   157,    -1,
     159,   160,   161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,    -1,
      -1,    -1,    -1,   153,    -1,    -1,   156,   157,    -1,   159,
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
      45,    46,    47,    48,    49,    50,    51,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    12,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65
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
     249,   250,   254,   257,   262,   268,   326,   327,   333,   337,
     338,   339,   340,   341,   342,   343,   344,   346,   349,   361,
     362,   363,   364,   365,   369,   370,   372,   391,   401,   402,
     403,   408,   411,   429,   437,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   450,   471,   473,   475,   116,
     117,   118,   131,   150,   160,   218,   249,   326,   343,   439,
     343,   201,   343,   343,   343,   102,   343,   343,   427,   428,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,    79,   118,   201,   226,   402,   403,   439,   442,
     439,    35,   343,   454,   455,   343,   118,   201,   226,   402,
     403,   404,   438,   446,   451,   452,   201,   334,   405,   201,
     334,   350,   335,   343,   235,   334,   201,   201,   201,   334,
     203,   343,   218,   203,   343,    29,    55,   132,   133,   154,
     174,   201,   218,   229,   476,   488,   489,   184,   203,   340,
     343,   371,   373,   204,   242,   343,   105,   106,   153,   219,
     222,   225,    79,   206,   294,   295,   117,   124,   116,   124,
      79,   296,   201,   201,   201,   201,   218,   266,   477,   201,
     201,    79,    85,   146,   147,   148,   468,   469,   153,   204,
     225,   225,   218,   267,   477,   154,   201,   201,   201,   201,
     477,   477,    79,   198,   204,   352,   333,   343,   344,   439,
     443,   231,   204,    85,   406,   468,    85,   468,   468,    30,
     153,   170,   478,   201,     9,   203,    35,   248,   154,   265,
     477,   118,   183,   249,   327,   203,   203,   203,   203,   203,
     203,    10,    11,    12,    29,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    53,    65,   203,    66,
      66,   203,   204,   149,   125,   160,   162,   268,   325,   326,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    63,    64,   128,   129,   431,    66,   204,
     436,   201,   201,    66,   204,   206,   447,   201,   248,   249,
      14,   343,   203,   130,    44,   218,   426,   201,   333,   439,
     443,   149,   439,   130,   208,     9,   413,   333,   439,   478,
     149,   201,   407,   431,   436,   202,   343,    30,   233,     8,
     355,     9,   203,   233,   234,   335,   336,   343,   218,   280,
     237,   203,   203,   203,   489,   489,   170,   201,   105,   489,
      14,   218,    79,   203,   203,   203,   184,   185,   186,   191,
     192,   195,   196,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   386,   387,   388,   243,   109,   167,   203,   153,
     220,   223,   225,   153,   221,   224,   225,   225,     9,   203,
      97,   204,   439,     9,   203,   124,   124,    14,     9,   203,
     439,   472,   472,   333,   344,   439,   442,   443,   202,   170,
     260,   131,   439,   453,   454,    66,   431,   146,   469,    78,
     343,   439,    85,   146,   469,   225,   217,   203,   204,   255,
     263,   392,   394,    86,   206,   356,   357,   359,   403,   447,
     473,   343,   461,   463,   343,   460,   462,   460,    14,    97,
     474,   351,   353,   354,   290,   291,   429,   430,   202,   202,
     202,   202,   205,   232,   233,   250,   257,   262,   429,   343,
     207,   209,   210,   218,   479,   480,   489,    35,   163,   292,
     293,   343,   476,   201,   477,   258,   248,   343,   343,   343,
      30,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   343,
     343,   343,   343,   404,   343,   343,   449,   449,   343,   456,
     457,   124,   204,   218,   446,   447,   266,   218,   267,   265,
     249,    27,    35,   337,   340,   343,   371,   343,   343,   343,
     343,   343,   343,   343,   343,   343,   343,   343,   343,   154,
     204,   218,   432,   433,   434,   435,   446,   449,   343,   292,
     292,   449,   343,   453,   248,   202,   343,   201,   425,     9,
     413,   333,   202,   218,    35,   343,    35,   343,   202,   202,
     446,   292,   204,   218,   432,   433,   446,   202,   231,   284,
     204,   340,   343,   343,    89,    30,   233,   278,   203,    28,
      97,    14,     9,   202,    30,   204,   281,   489,    86,   229,
     485,   486,   487,   201,     9,    46,    47,    52,    54,    66,
      86,   132,   145,   154,   174,   175,   176,   177,   201,   226,
     227,   229,   366,   367,   368,   402,   408,   409,   410,   187,
      79,   343,    79,    79,   343,   383,   384,   343,   343,   376,
     386,   190,   389,   231,   201,   241,   225,   203,     9,    97,
     225,   203,     9,    97,    97,   222,   218,   343,   295,   409,
      79,     9,   202,   202,   202,   202,   202,   202,   202,   203,
      46,    47,   483,   484,   126,   271,   201,     9,   202,   202,
      79,    80,   218,   470,   218,    66,   205,   205,   214,   216,
      30,   127,   270,   169,    50,   154,   169,   396,   130,     9,
     413,   202,   149,   202,     9,   413,   130,   202,     9,   413,
     202,   489,   489,    14,   355,   290,   231,   199,     9,   414,
     489,   490,   431,   436,   205,     9,   413,   171,   439,   343,
     202,     9,   414,    14,   347,   251,   126,   269,   201,   477,
     343,    30,   208,   208,   130,   205,     9,   413,   343,   478,
     201,   261,   256,   264,   259,   248,    68,   439,   343,   478,
     208,   205,   202,   202,   208,   205,   202,    46,    47,    66,
      74,    75,    76,    86,   132,   145,   174,   218,   416,   418,
     421,   424,   218,   439,   439,   130,   431,   436,   202,   343,
     285,    71,    72,   286,   231,   334,   231,   336,    97,    35,
     131,   275,   439,   409,   218,    30,   233,   279,   203,   282,
     203,   282,     9,   171,   130,   149,     9,   413,   202,   163,
     479,   480,   481,   479,   409,   409,   409,   409,   409,   412,
     415,   201,    85,   149,   201,   201,   201,   201,   409,   149,
     204,    10,    11,    12,    29,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    65,   343,   187,   187,
      14,   193,   194,   385,     9,   197,   389,    79,   205,   402,
     204,   245,    97,   223,   218,    97,   224,   218,   218,   205,
      14,   439,   203,     9,   171,   218,   272,   402,   204,   453,
     131,   439,    14,   208,   343,   205,   214,   489,   272,   204,
     395,    14,   343,   356,   218,   343,   343,   343,   203,   489,
     199,   205,    30,   482,   430,    35,    79,   163,   432,   433,
     435,   489,    35,   163,   343,   409,   290,   201,   402,   270,
     348,   252,   343,   343,   343,   205,   201,   292,   271,    30,
     270,   269,   477,   404,   205,   201,    14,    74,    75,    76,
     218,   417,   417,   418,   419,   420,   201,    85,   146,   201,
       9,   413,   202,   425,    35,   343,   432,   433,   205,    71,
      72,   287,   334,   233,   205,   203,    90,   203,   275,   439,
     201,   130,   274,    14,   231,   282,    99,   100,   101,   282,
     205,   489,   489,   218,   485,     9,   202,   413,   130,   208,
       9,   413,   412,   218,   356,   358,   360,   409,   465,   467,
     409,   464,   466,   464,   202,   124,   218,   409,   458,   459,
     409,   409,   409,    30,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   409,   409,   409,
     409,   409,   409,   409,   409,   409,   409,   343,   343,   343,
     384,   343,   374,    79,   246,   218,   218,   409,   484,    97,
       9,   300,   202,   201,   337,   340,   343,   208,   205,   474,
     300,   155,   168,   204,   391,   398,   155,   204,   397,   130,
     130,   203,   482,   489,   355,   490,    79,   163,    14,    79,
     478,   439,   343,   202,   290,   204,   290,   201,   130,   201,
     292,   202,   204,   489,   204,   270,   253,   407,   292,   130,
     208,     9,   413,   419,   146,   356,   422,   423,   418,   439,
     334,    30,    73,   233,   203,   336,   274,   453,   275,   202,
     409,    96,    99,   203,   343,    30,   203,   283,   205,   171,
     130,   163,    30,   202,   409,   409,   202,   130,     9,   413,
     202,   202,     9,   413,   130,   202,     9,   413,   202,   130,
     205,     9,   413,   409,    30,   188,   202,   231,   218,   489,
     402,     4,   106,   111,   117,   119,   156,   157,   159,   205,
     301,   324,   325,   326,   331,   332,   429,   453,   205,   204,
     205,    50,   343,   343,   343,   343,   355,    35,    79,   163,
      14,    79,   343,   201,   482,   202,   300,   202,   290,   343,
     292,   202,   300,   474,   300,   204,   201,   202,   418,   418,
     202,   130,   202,     9,   413,    30,   231,   203,   202,   202,
     202,   238,   203,   203,   283,   231,   489,   489,   130,   409,
     356,   409,   409,   409,   409,   409,   409,   343,   204,   205,
      97,   126,   127,   476,   273,   402,   106,   119,   132,   133,
     154,   160,   310,   311,   312,   402,   158,   316,   317,   122,
     201,   218,   318,   319,   302,   249,   489,     9,   203,     9,
     203,   325,   202,   297,   154,   393,   205,   205,    79,   163,
      14,    79,   343,   292,   111,   345,   482,   205,   482,   202,
     202,   205,   204,   205,   300,   290,   130,   418,   356,   231,
     236,   239,    30,   233,   277,   231,   202,   409,   130,   130,
     130,   189,   231,   489,   402,   402,    14,     9,   203,   204,
     476,   170,   204,     9,   203,     3,     4,     5,     6,     7,
      10,    11,    12,    13,    27,    28,    53,    67,    68,    69,
      70,    71,    72,    73,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   131,   132,   134,   135,   136,
     137,   138,   150,   151,   152,   162,   164,   165,   167,   174,
     178,   180,   182,   183,   218,   399,   400,     9,   203,   154,
     158,   218,   319,   320,   321,   203,    79,   330,   248,   303,
     476,   476,   249,   205,   298,   299,   476,    14,    79,   343,
     202,   201,   204,   203,   204,   322,   345,   482,   297,   205,
     202,   418,   130,    30,   233,   276,   277,   231,   409,   409,
     409,   343,   205,   203,   203,   409,   402,   306,   489,   313,
     408,   311,    14,    30,    47,   314,   317,     9,    33,   202,
      29,    46,    49,    14,     9,   203,   477,   330,    14,   248,
     203,    14,   343,    35,    79,   390,   231,   231,   204,   322,
     205,   482,   418,   231,    94,   190,   244,   205,   218,   229,
     307,   308,   309,     9,   171,     9,   205,   409,   400,   400,
      55,   315,   320,   320,    29,    46,    49,   409,    79,   201,
     203,   409,   477,   409,    79,     9,   414,   205,   205,   231,
     322,    92,   203,    79,   109,   240,   149,    97,   489,   408,
     161,    14,   304,   201,    35,    79,   202,   205,   203,   201,
     167,   247,   218,   325,   326,   171,   409,   288,   289,   430,
     305,    79,   402,   245,   164,   218,   203,   202,     9,   414,
     113,   114,   115,   328,   329,   288,    79,   273,   203,   482,
     430,   490,   202,   202,   203,   203,   204,   323,   328,    35,
      79,   163,   482,   204,   231,   490,    79,   163,    14,    79,
     323,   231,   205,    35,    79,   163,    14,    79,   343,   205,
      79,   163,    14,    79,   343,    14,    79,   343,   343
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
#line 1503 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1516 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1522 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1527 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1529 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1531 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1533 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1535 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1539 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1542 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1543 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1550 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1557 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1573 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1580 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1582 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1586 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1588 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1590 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1591 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1597 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1600 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1604 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1606 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1610 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1615 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1620 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1623 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1628 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1633 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1634 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1636 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1640 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1641 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1642 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1643 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1647 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1649 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1650 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1651 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1655 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1659 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1662 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1663 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1667 "hphp.y"
    { (yyval).reset();;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1668 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1672 "hphp.y"
    { (yyval).reset();;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1673 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1676 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1677 "hphp.y"
    { (yyval).reset();;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1680 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1681 "hphp.y"
    { (yyval).reset();;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1684 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1686 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1689 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1690 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1693 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1694 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1695 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1699 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1700 "hphp.y"
    { (yyval).reset();;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1703 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1704 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1709 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1713 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1717 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1719 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1723 "hphp.y"
    { _p->onClassAbstractConstant((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1725 "hphp.y"
    { _p->onClassAbstractConstant((yyval),NULL,(yyvsp[(3) - (3)]));;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1729 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1731 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1732 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { (yyval).reset();;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1751 "hphp.y"
    { _p->onYield((yyval), NULL);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1752 "hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (2)]));;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { _p->onYieldPair((yyval), &(yyvsp[(2) - (4)]), &(yyvsp[(4) - (4)]));;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1852 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1858 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1871 "hphp.y"
    { (yyval).reset();;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1876 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(7) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1890 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),(yyvsp[(8) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { Token v; Token w; Token x;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]),x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
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

  case 457:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
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

  case 461:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1978 "hphp.y"
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

  case 463:

/* Line 1455 of yacc.c  */
#line 1992 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1993 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1995 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 2001 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 2008 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 2011 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 2018 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 2021 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 2026 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 2027 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 2032 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 2033 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 2037 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 2041 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 2042 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 2047 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 2053 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2054 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2058 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_VARRAY);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2063 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2065 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2070 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_VARRAY);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2075 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2082 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2084 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2088 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2089 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2090 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2091 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2093 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2097 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2101 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2105 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2110 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2112 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2114 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2116 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2120 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2126 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2127 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2128 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2129 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2135 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2139 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2148 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2153 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2161 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2162 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2166 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2167 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2171 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2176 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2177 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2181 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2185 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2193 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2194 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2195 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
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

  case 532:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
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

  case 533:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { (yyval).reset();;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { (yyval).reset();;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2332 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2337 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2338 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2343 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2344 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2345 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2348 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2349 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2355 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { (yyval).reset();;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { (yyval).reset();;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { (yyval).reset();;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { (yyval).reset();;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2421 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2459 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2467 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2471 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2479 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2487 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2488 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2489 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2492 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2494 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { (yyval).reset();;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { (yyval).reset();;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2505 "hphp.y"
    { (yyval).reset();;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { (yyval).reset();;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2517 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2519 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2520 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2524 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2526 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2538 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2539 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2544 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2545 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2547 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2550 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { (yyval).reset();;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2565 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2570 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2576 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2577 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2582 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2590 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2591 "hphp.y"
    { (yyval).reset();;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2594 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2615 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2616 "hphp.y"
    { (yyval).reset();;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2621 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2626 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropXhpAttr;;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2627 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2631 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2632 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = HPHP::ObjPropNormal;;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2646 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2647 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2653 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2660 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
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
#line 2667 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2673 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2676 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2678 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2680 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2681 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2685 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2686 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2687 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2688 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2691 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2695 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2697 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2698 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2702 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2703 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2704 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2708 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2712 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2713 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(2) - (7)]).num(),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2723 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]).num(),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2730 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2738 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2742 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2744 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2749 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2750 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2751 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2755 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2758 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2759 "hphp.y"
    { (yyval).reset();;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2763 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2764 "hphp.y"
    { (yyval)++;;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2769 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2772 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2775 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2778 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2779 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2783 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2786 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2790 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]).num(),(yyvsp[(5) - (5)]));;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2791 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2795 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2796 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2798 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2800 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2806 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2807 "hphp.y"
    { (yyval).reset();;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2811 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2812 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2813 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2814 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2817 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2819 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2820 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2821 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2826 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2827 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2831 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2832 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2833 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2834 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2840 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2845 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2847 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2849 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2850 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2856 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2860 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2861 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2865 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2866 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2869 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2870 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2875 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2876 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2880 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2881 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2886 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2888 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2892 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2893 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2897 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2899 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2900 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2902 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2907 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2909 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2911 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]).num(), (yyvsp[(3) - (3)]));;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2913 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2915 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2916 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2919 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2920 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2926 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2927 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2928 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2929 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2930 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2931 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2932 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2933 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2934 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2935 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2939 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2940 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2945 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2947 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 2961 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 2965 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 2971 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 2972 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 2978 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 2982 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 2989 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 2993 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 2996 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 3002 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 3007 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3009 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3010 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3014 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3015 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3020 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3021 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 923:

/* Line 1455 of yacc.c  */
#line 3024 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (6)]).text()); ;}
    break;

  case 924:

/* Line 1455 of yacc.c  */
#line 3026 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (4)]).text()); ;}
    break;

  case 925:

/* Line 1455 of yacc.c  */
#line 3030 "hphp.y"
    {;}
    break;

  case 926:

/* Line 1455 of yacc.c  */
#line 3031 "hphp.y"
    {;}
    break;

  case 927:

/* Line 1455 of yacc.c  */
#line 3032 "hphp.y"
    {;}
    break;

  case 928:

/* Line 1455 of yacc.c  */
#line 3038 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 929:

/* Line 1455 of yacc.c  */
#line 3043 "hphp.y"
    { ;}
    break;

  case 932:

/* Line 1455 of yacc.c  */
#line 3055 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 933:

/* Line 1455 of yacc.c  */
#line 3057 "hphp.y"
    {;}
    break;

  case 934:

/* Line 1455 of yacc.c  */
#line 3061 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 935:

/* Line 1455 of yacc.c  */
#line 3068 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 936:

/* Line 1455 of yacc.c  */
#line 3071 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 937:

/* Line 1455 of yacc.c  */
#line 3074 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 938:

/* Line 1455 of yacc.c  */
#line 3075 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 939:

/* Line 1455 of yacc.c  */
#line 3078 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 940:

/* Line 1455 of yacc.c  */
#line 3081 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 941:

/* Line 1455 of yacc.c  */
#line 3083 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 942:

/* Line 1455 of yacc.c  */
#line 3086 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 943:

/* Line 1455 of yacc.c  */
#line 3089 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 944:

/* Line 1455 of yacc.c  */
#line 3095 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 945:

/* Line 1455 of yacc.c  */
#line 3101 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 946:

/* Line 1455 of yacc.c  */
#line 3109 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 947:

/* Line 1455 of yacc.c  */
#line 3110 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13828 "hphp.tab.cpp"
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
#line 3113 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

