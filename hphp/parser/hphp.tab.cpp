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
#line 647 "hphp.tab.cpp"

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
     T_BOOLEAN_OR = 280,
     T_BOOLEAN_AND = 281,
     T_IS_NOT_IDENTICAL = 282,
     T_IS_IDENTICAL = 283,
     T_IS_NOT_EQUAL = 284,
     T_IS_EQUAL = 285,
     T_IS_GREATER_OR_EQUAL = 286,
     T_IS_SMALLER_OR_EQUAL = 287,
     T_SR = 288,
     T_SL = 289,
     T_INSTANCEOF = 290,
     T_UNSET_CAST = 291,
     T_BOOL_CAST = 292,
     T_OBJECT_CAST = 293,
     T_ARRAY_CAST = 294,
     T_STRING_CAST = 295,
     T_DOUBLE_CAST = 296,
     T_INT_CAST = 297,
     T_DEC = 298,
     T_INC = 299,
     T_POW = 300,
     T_CLONE = 301,
     T_NEW = 302,
     T_EXIT = 303,
     T_IF = 304,
     T_ELSEIF = 305,
     T_ELSE = 306,
     T_ENDIF = 307,
     T_LNUMBER = 308,
     T_DNUMBER = 309,
     T_ONUMBER = 310,
     T_STRING = 311,
     T_STRING_VARNAME = 312,
     T_VARIABLE = 313,
     T_NUM_STRING = 314,
     T_INLINE_HTML = 315,
     T_CHARACTER = 316,
     T_BAD_CHARACTER = 317,
     T_ENCAPSED_AND_WHITESPACE = 318,
     T_CONSTANT_ENCAPSED_STRING = 319,
     T_ECHO = 320,
     T_DO = 321,
     T_WHILE = 322,
     T_ENDWHILE = 323,
     T_FOR = 324,
     T_ENDFOR = 325,
     T_FOREACH = 326,
     T_ENDFOREACH = 327,
     T_DECLARE = 328,
     T_ENDDECLARE = 329,
     T_AS = 330,
     T_SWITCH = 331,
     T_ENDSWITCH = 332,
     T_CASE = 333,
     T_DEFAULT = 334,
     T_BREAK = 335,
     T_GOTO = 336,
     T_CONTINUE = 337,
     T_FUNCTION = 338,
     T_CONST = 339,
     T_RETURN = 340,
     T_TRY = 341,
     T_CATCH = 342,
     T_THROW = 343,
     T_USE = 344,
     T_GLOBAL = 345,
     T_PUBLIC = 346,
     T_PROTECTED = 347,
     T_PRIVATE = 348,
     T_FINAL = 349,
     T_ABSTRACT = 350,
     T_STATIC = 351,
     T_VAR = 352,
     T_UNSET = 353,
     T_ISSET = 354,
     T_EMPTY = 355,
     T_HALT_COMPILER = 356,
     T_CLASS = 357,
     T_INTERFACE = 358,
     T_EXTENDS = 359,
     T_IMPLEMENTS = 360,
     T_OBJECT_OPERATOR = 361,
     T_DOUBLE_ARROW = 362,
     T_LIST = 363,
     T_ARRAY = 364,
     T_CALLABLE = 365,
     T_CLASS_C = 366,
     T_METHOD_C = 367,
     T_FUNC_C = 368,
     T_LINE = 369,
     T_FILE = 370,
     T_COMMENT = 371,
     T_DOC_COMMENT = 372,
     T_OPEN_TAG = 373,
     T_OPEN_TAG_WITH_ECHO = 374,
     T_CLOSE_TAG = 375,
     T_WHITESPACE = 376,
     T_START_HEREDOC = 377,
     T_END_HEREDOC = 378,
     T_DOLLAR_OPEN_CURLY_BRACES = 379,
     T_CURLY_OPEN = 380,
     T_DOUBLE_COLON = 381,
     T_NAMESPACE = 382,
     T_NS_C = 383,
     T_DIR = 384,
     T_NS_SEPARATOR = 385,
     T_YIELD = 386,
     T_XHP_LABEL = 387,
     T_XHP_TEXT = 388,
     T_XHP_ATTRIBUTE = 389,
     T_XHP_CATEGORY = 390,
     T_XHP_CATEGORY_LABEL = 391,
     T_XHP_CHILDREN = 392,
     T_XHP_ENUM = 393,
     T_XHP_REQUIRED = 394,
     T_TRAIT = 395,
     T_ELLIPSIS = 396,
     T_INSTEADOF = 397,
     T_TRAIT_C = 398,
     T_HH_ERROR = 399,
     T_FINALLY = 400,
     T_XHP_TAG_LT = 401,
     T_XHP_TAG_GT = 402,
     T_TYPELIST_LT = 403,
     T_TYPELIST_GT = 404,
     T_UNRESOLVED_LT = 405,
     T_COLLECTION = 406,
     T_SHAPE = 407,
     T_TYPE = 408,
     T_UNRESOLVED_TYPE = 409,
     T_NEWTYPE = 410,
     T_UNRESOLVED_NEWTYPE = 411,
     T_COMPILER_HALT_OFFSET = 412,
     T_AWAIT = 413,
     T_ASYNC = 414,
     T_FROM = 415,
     T_WHERE = 416,
     T_JOIN = 417,
     T_IN = 418,
     T_ON = 419,
     T_EQUALS = 420,
     T_INTO = 421,
     T_LET = 422,
     T_ORDERBY = 423,
     T_ASCENDING = 424,
     T_DESCENDING = 425,
     T_SELECT = 426,
     T_GROUP = 427,
     T_BY = 428,
     T_LAMBDA_OP = 429,
     T_LAMBDA_CP = 430,
     T_UNRESOLVED_OP = 431
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
#line 878 "hphp.tab.cpp"

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
#define YYLAST   15624

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  206
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  251
/* YYNRULES -- Number of rules.  */
#define YYNRULES  872
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1622

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   431

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    50,   204,     2,   201,    49,    33,   205,
     196,   197,    47,    44,     9,    45,    46,    48,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    28,   198,
      38,    14,    39,    27,    53,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    64,     2,   203,    32,     2,   202,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   199,    31,   200,    52,     2,     2,     2,
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
      29,    30,    34,    35,    36,    37,    40,    41,    42,    43,
      51,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    65,    66,    67,    68,    69,    70,    71,    72,    73,
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
     194,   195
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    11,    13,    15,    17,
      19,    21,    26,    30,    31,    38,    39,    45,    49,    54,
      59,    62,    64,    66,    68,    70,    72,    74,    76,    78,
      80,    82,    84,    86,    88,    90,    92,    94,    96,    98,
     100,   104,   106,   110,   112,   116,   118,   120,   123,   127,
     132,   134,   137,   141,   146,   148,   151,   155,   160,   162,
     166,   168,   172,   175,   177,   180,   183,   189,   194,   197,
     198,   200,   202,   204,   206,   210,   216,   225,   226,   231,
     232,   239,   240,   251,   252,   257,   260,   264,   267,   271,
     274,   278,   282,   286,   290,   294,   300,   302,   304,   305,
     315,   321,   322,   336,   337,   343,   347,   351,   354,   357,
     360,   363,   366,   369,   373,   376,   379,   383,   386,   387,
     392,   402,   403,   404,   409,   412,   413,   415,   416,   418,
     419,   429,   430,   441,   442,   454,   455,   464,   465,   475,
     476,   484,   485,   494,   495,   503,   504,   513,   515,   517,
     519,   521,   523,   526,   529,   532,   533,   536,   537,   540,
     541,   543,   547,   549,   553,   556,   557,   559,   562,   567,
     569,   574,   576,   581,   583,   588,   590,   595,   599,   605,
     609,   614,   619,   625,   631,   636,   637,   639,   641,   646,
     647,   653,   654,   657,   658,   662,   663,   671,   678,   681,
     687,   692,   693,   698,   704,   712,   719,   726,   734,   744,
     753,   760,   766,   769,   774,   778,   779,   783,   788,   795,
     801,   807,   814,   823,   831,   834,   835,   837,   840,   844,
     849,   853,   855,   857,   860,   865,   869,   875,   877,   881,
     884,   885,   886,   891,   892,   898,   901,   902,   913,   914,
     926,   930,   934,   938,   943,   948,   952,   958,   961,   964,
     965,   972,   978,   983,   987,   989,   991,   995,  1000,  1002,
    1004,  1006,  1008,  1013,  1015,  1017,  1021,  1024,  1025,  1028,
    1029,  1031,  1035,  1037,  1039,  1041,  1043,  1047,  1052,  1057,
    1062,  1064,  1066,  1069,  1072,  1075,  1079,  1083,  1085,  1087,
    1089,  1091,  1095,  1097,  1101,  1103,  1105,  1107,  1108,  1110,
    1113,  1115,  1117,  1119,  1121,  1123,  1125,  1127,  1129,  1130,
    1132,  1134,  1136,  1140,  1146,  1148,  1152,  1158,  1163,  1167,
    1171,  1174,  1176,  1178,  1182,  1186,  1188,  1190,  1191,  1194,
    1199,  1203,  1210,  1213,  1217,  1224,  1226,  1228,  1230,  1237,
    1241,  1246,  1253,  1257,  1261,  1265,  1269,  1273,  1277,  1281,
    1285,  1289,  1293,  1297,  1301,  1304,  1307,  1310,  1313,  1317,
    1321,  1325,  1329,  1333,  1337,  1341,  1345,  1349,  1353,  1357,
    1361,  1365,  1369,  1373,  1377,  1381,  1384,  1387,  1390,  1393,
    1397,  1401,  1405,  1409,  1413,  1417,  1421,  1425,  1429,  1433,
    1439,  1444,  1446,  1449,  1452,  1455,  1458,  1461,  1464,  1467,
    1470,  1473,  1475,  1477,  1479,  1483,  1486,  1488,  1490,  1492,
    1498,  1499,  1500,  1512,  1513,  1526,  1527,  1531,  1532,  1539,
    1542,  1547,  1549,  1555,  1559,  1565,  1569,  1572,  1573,  1576,
    1577,  1582,  1587,  1591,  1596,  1601,  1606,  1611,  1613,  1615,
    1617,  1621,  1624,  1628,  1633,  1636,  1640,  1642,  1645,  1647,
    1650,  1652,  1654,  1656,  1658,  1660,  1662,  1667,  1672,  1675,
    1684,  1695,  1698,  1700,  1704,  1706,  1709,  1711,  1713,  1715,
    1717,  1720,  1725,  1729,  1733,  1738,  1740,  1743,  1748,  1751,
    1758,  1759,  1761,  1766,  1767,  1770,  1771,  1773,  1775,  1779,
    1781,  1785,  1787,  1789,  1793,  1797,  1799,  1801,  1803,  1805,
    1807,  1809,  1811,  1813,  1815,  1817,  1819,  1821,  1823,  1825,
    1827,  1829,  1831,  1833,  1835,  1837,  1839,  1841,  1843,  1845,
    1847,  1849,  1851,  1853,  1855,  1857,  1859,  1861,  1863,  1865,
    1867,  1869,  1871,  1873,  1875,  1877,  1879,  1881,  1883,  1885,
    1887,  1889,  1891,  1893,  1895,  1897,  1899,  1901,  1903,  1905,
    1907,  1909,  1911,  1913,  1915,  1917,  1919,  1921,  1923,  1925,
    1927,  1929,  1931,  1933,  1935,  1937,  1939,  1941,  1943,  1945,
    1947,  1949,  1951,  1953,  1955,  1957,  1962,  1964,  1966,  1968,
    1970,  1972,  1974,  1976,  1978,  1981,  1983,  1984,  1985,  1987,
    1989,  1993,  1994,  1996,  1998,  2000,  2002,  2004,  2006,  2008,
    2010,  2012,  2014,  2016,  2018,  2020,  2024,  2027,  2029,  2031,
    2034,  2037,  2042,  2046,  2051,  2053,  2055,  2057,  2061,  2065,
    2069,  2073,  2077,  2081,  2085,  2089,  2093,  2097,  2101,  2105,
    2109,  2113,  2117,  2121,  2125,  2128,  2131,  2135,  2139,  2143,
    2147,  2151,  2155,  2159,  2163,  2169,  2174,  2178,  2182,  2186,
    2188,  2190,  2192,  2194,  2198,  2202,  2206,  2209,  2210,  2212,
    2213,  2215,  2216,  2222,  2226,  2230,  2232,  2234,  2236,  2238,
    2240,  2244,  2247,  2249,  2251,  2253,  2255,  2257,  2259,  2262,
    2265,  2270,  2274,  2279,  2282,  2283,  2289,  2293,  2297,  2299,
    2303,  2305,  2308,  2309,  2315,  2319,  2322,  2323,  2327,  2328,
    2333,  2336,  2337,  2341,  2345,  2347,  2348,  2350,  2353,  2356,
    2361,  2365,  2369,  2372,  2377,  2380,  2385,  2387,  2389,  2391,
    2393,  2395,  2398,  2403,  2407,  2412,  2416,  2418,  2420,  2422,
    2424,  2427,  2432,  2437,  2441,  2443,  2445,  2449,  2457,  2464,
    2473,  2483,  2492,  2503,  2511,  2518,  2527,  2529,  2532,  2537,
    2542,  2544,  2546,  2551,  2553,  2554,  2556,  2559,  2561,  2563,
    2566,  2571,  2575,  2579,  2580,  2582,  2585,  2590,  2594,  2597,
    2601,  2608,  2609,  2611,  2616,  2619,  2620,  2626,  2630,  2634,
    2636,  2643,  2648,  2653,  2656,  2659,  2660,  2666,  2670,  2674,
    2676,  2679,  2680,  2686,  2690,  2694,  2696,  2699,  2702,  2704,
    2707,  2709,  2714,  2718,  2722,  2729,  2733,  2735,  2737,  2739,
    2744,  2749,  2754,  2759,  2762,  2765,  2770,  2773,  2776,  2778,
    2782,  2786,  2790,  2791,  2794,  2800,  2807,  2809,  2812,  2814,
    2819,  2823,  2824,  2826,  2830,  2834,  2836,  2838,  2839,  2840,
    2843,  2847,  2849,  2855,  2859,  2863,  2867,  2869,  2872,  2873,
    2878,  2881,  2884,  2886,  2888,  2890,  2892,  2897,  2904,  2906,
    2915,  2921,  2923
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     207,     0,    -1,    -1,   208,   209,    -1,   209,   210,    -1,
      -1,   228,    -1,   244,    -1,   248,    -1,   253,    -1,   443,
      -1,   120,   196,   197,   198,    -1,   146,   220,   198,    -1,
      -1,   146,   220,   199,   211,   209,   200,    -1,    -1,   146,
     199,   212,   209,   200,    -1,   108,   214,   198,    -1,   108,
     102,   215,   198,    -1,   108,   103,   216,   198,    -1,   225,
     198,    -1,    75,    -1,   153,    -1,   154,    -1,   156,    -1,
     158,    -1,   157,    -1,   180,    -1,   181,    -1,   183,    -1,
     182,    -1,   184,    -1,   185,    -1,   186,    -1,   187,    -1,
     188,    -1,   189,    -1,   190,    -1,   191,    -1,   192,    -1,
     214,     9,   217,    -1,   217,    -1,   218,     9,   218,    -1,
     218,    -1,   219,     9,   219,    -1,   219,    -1,   220,    -1,
     149,   220,    -1,   220,    94,   213,    -1,   149,   220,    94,
     213,    -1,   220,    -1,   149,   220,    -1,   220,    94,   213,
      -1,   149,   220,    94,   213,    -1,   220,    -1,   149,   220,
      -1,   220,    94,   213,    -1,   149,   220,    94,   213,    -1,
     213,    -1,   220,   149,   213,    -1,   220,    -1,   146,   149,
     220,    -1,   149,   220,    -1,   221,    -1,   221,   446,    -1,
     221,   446,    -1,   225,     9,   444,    14,   390,    -1,   103,
     444,    14,   390,    -1,   226,   227,    -1,    -1,   228,    -1,
     244,    -1,   248,    -1,   253,    -1,   199,   226,   200,    -1,
      68,   321,   228,   275,   277,    -1,    68,   321,    28,   226,
     276,   278,    71,   198,    -1,    -1,    86,   321,   229,   269,
      -1,    -1,    85,   230,   228,    86,   321,   198,    -1,    -1,
      88,   196,   323,   198,   323,   198,   323,   197,   231,   267,
      -1,    -1,    95,   321,   232,   272,    -1,    99,   198,    -1,
      99,   330,   198,    -1,   101,   198,    -1,   101,   330,   198,
      -1,   104,   198,    -1,   104,   330,   198,    -1,   150,    99,
     198,    -1,   109,   285,   198,    -1,   115,   287,   198,    -1,
      84,   322,   198,    -1,   117,   196,   440,   197,   198,    -1,
     198,    -1,    79,    -1,    -1,    90,   196,   330,    94,   266,
     265,   197,   233,   268,    -1,    92,   196,   271,   197,   270,
      -1,    -1,   105,   236,   106,   196,   382,    77,   197,   199,
     226,   200,   238,   234,   241,    -1,    -1,   105,   236,   164,
     235,   239,    -1,   107,   330,   198,    -1,   100,   213,   198,
      -1,   330,   198,    -1,   324,   198,    -1,   325,   198,    -1,
     326,   198,    -1,   327,   198,    -1,   328,   198,    -1,   104,
     327,   198,    -1,   329,   198,    -1,   352,   198,    -1,   104,
     351,   198,    -1,   213,    28,    -1,    -1,   199,   237,   226,
     200,    -1,   238,   106,   196,   382,    77,   197,   199,   226,
     200,    -1,    -1,    -1,   199,   240,   226,   200,    -1,   164,
     239,    -1,    -1,    33,    -1,    -1,   102,    -1,    -1,   243,
     242,   445,   245,   196,   281,   197,   449,   310,    -1,    -1,
     314,   243,   242,   445,   246,   196,   281,   197,   449,   310,
      -1,    -1,   410,   313,   243,   242,   445,   247,   196,   281,
     197,   449,   310,    -1,    -1,   259,   256,   249,   260,   261,
     199,   288,   200,    -1,    -1,   410,   259,   256,   250,   260,
     261,   199,   288,   200,    -1,    -1,   122,   257,   251,   262,
     199,   288,   200,    -1,    -1,   410,   122,   257,   252,   262,
     199,   288,   200,    -1,    -1,   159,   258,   254,   261,   199,
     288,   200,    -1,    -1,   410,   159,   258,   255,   261,   199,
     288,   200,    -1,   445,    -1,   151,    -1,   445,    -1,   445,
      -1,   121,    -1,   114,   121,    -1,   113,   121,    -1,   123,
     382,    -1,    -1,   124,   263,    -1,    -1,   123,   263,    -1,
      -1,   382,    -1,   263,     9,   382,    -1,   382,    -1,   264,
       9,   382,    -1,   126,   266,    -1,    -1,   417,    -1,    33,
     417,    -1,   127,   196,   429,   197,    -1,   228,    -1,    28,
     226,    89,   198,    -1,   228,    -1,    28,   226,    91,   198,
      -1,   228,    -1,    28,   226,    87,   198,    -1,   228,    -1,
      28,   226,    93,   198,    -1,   213,    14,   390,    -1,   271,
       9,   213,    14,   390,    -1,   199,   273,   200,    -1,   199,
     198,   273,   200,    -1,    28,   273,    96,   198,    -1,    28,
     198,   273,    96,   198,    -1,   273,    97,   330,   274,   226,
      -1,   273,    98,   274,   226,    -1,    -1,    28,    -1,   198,
      -1,   275,    69,   321,   228,    -1,    -1,   276,    69,   321,
      28,   226,    -1,    -1,    70,   228,    -1,    -1,    70,    28,
     226,    -1,    -1,   280,     9,   411,   316,   456,   160,    77,
      -1,   280,     9,   411,   316,   456,   160,    -1,   280,   395,
      -1,   411,   316,   456,   160,    77,    -1,   411,   316,   456,
     160,    -1,    -1,   411,   316,   456,    77,    -1,   411,   316,
     456,    33,    77,    -1,   411,   316,   456,    33,    77,    14,
     390,    -1,   411,   316,   456,    77,    14,   390,    -1,   280,
       9,   411,   316,   456,    77,    -1,   280,     9,   411,   316,
     456,    33,    77,    -1,   280,     9,   411,   316,   456,    33,
      77,    14,   390,    -1,   280,     9,   411,   316,   456,    77,
      14,   390,    -1,   282,     9,   411,   456,   160,    77,    -1,
     282,     9,   411,   456,   160,    -1,   282,   395,    -1,   411,
     456,   160,    77,    -1,   411,   456,   160,    -1,    -1,   411,
     456,    77,    -1,   411,   456,    33,    77,    -1,   411,   456,
      33,    77,    14,   390,    -1,   411,   456,    77,    14,   390,
      -1,   282,     9,   411,   456,    77,    -1,   282,     9,   411,
     456,    33,    77,    -1,   282,     9,   411,   456,    33,    77,
      14,   390,    -1,   282,     9,   411,   456,    77,    14,   390,
      -1,   284,   395,    -1,    -1,   330,    -1,    33,   417,    -1,
     284,     9,   330,    -1,   284,     9,    33,   417,    -1,   285,
       9,   286,    -1,   286,    -1,    77,    -1,   201,   417,    -1,
     201,   199,   330,   200,    -1,   287,     9,    77,    -1,   287,
       9,    77,    14,   390,    -1,    77,    -1,    77,    14,   390,
      -1,   288,   289,    -1,    -1,    -1,   312,   290,   318,   198,
      -1,    -1,   314,   455,   291,   318,   198,    -1,   319,   198,
      -1,    -1,   313,   243,   242,   445,   196,   292,   279,   197,
     449,   311,    -1,    -1,   410,   313,   243,   242,   445,   196,
     293,   279,   197,   449,   311,    -1,   153,   298,   198,    -1,
     154,   304,   198,    -1,   156,   306,   198,    -1,     4,   123,
     382,   198,    -1,     4,   124,   382,   198,    -1,   108,   264,
     198,    -1,   108,   264,   199,   294,   200,    -1,   294,   295,
      -1,   294,   296,    -1,    -1,   224,   145,   213,   161,   264,
     198,    -1,   297,    94,   313,   213,   198,    -1,   297,    94,
     314,   198,    -1,   224,   145,   213,    -1,   213,    -1,   299,
      -1,   298,     9,   299,    -1,   300,   379,   302,   303,    -1,
     151,    -1,   128,    -1,   382,    -1,   116,    -1,   157,   199,
     301,   200,    -1,   129,    -1,   388,    -1,   301,     9,   388,
      -1,    14,   390,    -1,    -1,    53,   158,    -1,    -1,   305,
      -1,   304,     9,   305,    -1,   155,    -1,   307,    -1,   213,
      -1,   119,    -1,   196,   308,   197,    -1,   196,   308,   197,
      47,    -1,   196,   308,   197,    27,    -1,   196,   308,   197,
      44,    -1,   307,    -1,   309,    -1,   309,    47,    -1,   309,
      27,    -1,   309,    44,    -1,   308,     9,   308,    -1,   308,
      31,   308,    -1,   213,    -1,   151,    -1,   155,    -1,   198,
      -1,   199,   226,   200,    -1,   198,    -1,   199,   226,   200,
      -1,   314,    -1,   116,    -1,   314,    -1,    -1,   315,    -1,
     314,   315,    -1,   110,    -1,   111,    -1,   112,    -1,   115,
      -1,   114,    -1,   113,    -1,   178,    -1,   317,    -1,    -1,
     110,    -1,   111,    -1,   112,    -1,   318,     9,    77,    -1,
     318,     9,    77,    14,   390,    -1,    77,    -1,    77,    14,
     390,    -1,   319,     9,   444,    14,   390,    -1,   103,   444,
      14,   390,    -1,   196,   320,   197,    -1,    66,   384,   387,
      -1,    65,   330,    -1,   371,    -1,   347,    -1,   196,   330,
     197,    -1,   322,     9,   330,    -1,   330,    -1,   322,    -1,
      -1,   150,   330,    -1,   150,   330,   126,   330,    -1,   417,
      14,   324,    -1,   127,   196,   429,   197,    14,   324,    -1,
     177,   330,    -1,   417,    14,   327,    -1,   127,   196,   429,
     197,    14,   327,    -1,   331,    -1,   417,    -1,   320,    -1,
     127,   196,   429,   197,    14,   330,    -1,   417,    14,   330,
      -1,   417,    14,    33,   417,    -1,   417,    14,    33,    66,
     384,   387,    -1,   417,    26,   330,    -1,   417,    25,   330,
      -1,   417,    24,   330,    -1,   417,    23,   330,    -1,   417,
      22,   330,    -1,   417,    21,   330,    -1,   417,    20,   330,
      -1,   417,    19,   330,    -1,   417,    18,   330,    -1,   417,
      17,   330,    -1,   417,    16,   330,    -1,   417,    15,   330,
      -1,   417,    62,    -1,    62,   417,    -1,   417,    61,    -1,
      61,   417,    -1,   330,    29,   330,    -1,   330,    30,   330,
      -1,   330,    10,   330,    -1,   330,    12,   330,    -1,   330,
      11,   330,    -1,   330,    31,   330,    -1,   330,    33,   330,
      -1,   330,    32,   330,    -1,   330,    46,   330,    -1,   330,
      44,   330,    -1,   330,    45,   330,    -1,   330,    47,   330,
      -1,   330,    48,   330,    -1,   330,    63,   330,    -1,   330,
      49,   330,    -1,   330,    43,   330,    -1,   330,    42,   330,
      -1,    44,   330,    -1,    45,   330,    -1,    50,   330,    -1,
      52,   330,    -1,   330,    35,   330,    -1,   330,    34,   330,
      -1,   330,    37,   330,    -1,   330,    36,   330,    -1,   330,
      38,   330,    -1,   330,    41,   330,    -1,   330,    39,   330,
      -1,   330,    40,   330,    -1,   330,    51,   384,    -1,   196,
     331,   197,    -1,   330,    27,   330,    28,   330,    -1,   330,
      27,    28,   330,    -1,   439,    -1,    60,   330,    -1,    59,
     330,    -1,    58,   330,    -1,    57,   330,    -1,    56,   330,
      -1,    55,   330,    -1,    54,   330,    -1,    67,   385,    -1,
      53,   330,    -1,   392,    -1,   346,    -1,   345,    -1,   202,
     386,   202,    -1,    13,   330,    -1,   333,    -1,   336,    -1,
     349,    -1,   108,   196,   370,   395,   197,    -1,    -1,    -1,
     243,   242,   196,   334,   281,   197,   449,   332,   199,   226,
     200,    -1,    -1,   314,   243,   242,   196,   335,   281,   197,
     449,   332,   199,   226,   200,    -1,    -1,    77,   337,   339,
      -1,    -1,   193,   338,   281,   194,   449,   339,    -1,     8,
     330,    -1,     8,   199,   226,   200,    -1,    83,    -1,   341,
       9,   340,   126,   330,    -1,   340,   126,   330,    -1,   342,
       9,   340,   126,   390,    -1,   340,   126,   390,    -1,   341,
     394,    -1,    -1,   342,   394,    -1,    -1,   171,   196,   343,
     197,    -1,   128,   196,   430,   197,    -1,    64,   430,   203,
      -1,   382,   199,   432,   200,    -1,   382,   199,   434,   200,
      -1,   349,    64,   425,   203,    -1,   350,    64,   425,   203,
      -1,   346,    -1,   441,    -1,    83,    -1,   196,   331,   197,
      -1,   353,   354,    -1,   417,    14,   351,    -1,   179,    77,
     182,   330,    -1,   355,   366,    -1,   355,   366,   369,    -1,
     366,    -1,   366,   369,    -1,   356,    -1,   355,   356,    -1,
     357,    -1,   358,    -1,   359,    -1,   360,    -1,   361,    -1,
     362,    -1,   179,    77,   182,   330,    -1,   186,    77,    14,
     330,    -1,   180,   330,    -1,   181,    77,   182,   330,   183,
     330,   184,   330,    -1,   181,    77,   182,   330,   183,   330,
     184,   330,   185,    77,    -1,   187,   363,    -1,   364,    -1,
     363,     9,   364,    -1,   330,    -1,   330,   365,    -1,   188,
      -1,   189,    -1,   367,    -1,   368,    -1,   190,   330,    -1,
     191,   330,   192,   330,    -1,   185,    77,   354,    -1,   370,
       9,    77,    -1,   370,     9,    33,    77,    -1,    77,    -1,
      33,    77,    -1,   165,   151,   372,   166,    -1,   374,    48,
      -1,   374,   166,   375,   165,    48,   373,    -1,    -1,   151,
      -1,   374,   376,    14,   377,    -1,    -1,   375,   378,    -1,
      -1,   151,    -1,   152,    -1,   199,   330,   200,    -1,   152,
      -1,   199,   330,   200,    -1,   371,    -1,   380,    -1,   379,
      28,   380,    -1,   379,    45,   380,    -1,   213,    -1,    67,
      -1,   102,    -1,   103,    -1,   104,    -1,   150,    -1,   177,
      -1,   105,    -1,   106,    -1,   164,    -1,   107,    -1,    68,
      -1,    69,    -1,    71,    -1,    70,    -1,    86,    -1,    87,
      -1,    85,    -1,    88,    -1,    89,    -1,    90,    -1,    91,
      -1,    92,    -1,    93,    -1,    51,    -1,    94,    -1,    95,
      -1,    96,    -1,    97,    -1,    98,    -1,    99,    -1,   101,
      -1,   100,    -1,    84,    -1,    13,    -1,   121,    -1,   122,
      -1,   123,    -1,   124,    -1,    66,    -1,    65,    -1,   116,
      -1,     5,    -1,     7,    -1,     6,    -1,     4,    -1,     3,
      -1,   146,    -1,   108,    -1,   109,    -1,   118,    -1,   119,
      -1,   120,    -1,   115,    -1,   114,    -1,   113,    -1,   112,
      -1,   111,    -1,   110,    -1,   178,    -1,   117,    -1,   127,
      -1,   128,    -1,    10,    -1,    12,    -1,    11,    -1,   130,
      -1,   132,    -1,   131,    -1,   133,    -1,   134,    -1,   148,
      -1,   147,    -1,   176,    -1,   159,    -1,   162,    -1,   161,
      -1,   172,    -1,   174,    -1,   171,    -1,   223,   196,   283,
     197,    -1,   224,    -1,   151,    -1,   382,    -1,   115,    -1,
     423,    -1,   382,    -1,   115,    -1,   427,    -1,   196,   197,
      -1,   321,    -1,    -1,    -1,    82,    -1,   436,    -1,   196,
     283,   197,    -1,    -1,    72,    -1,    73,    -1,    74,    -1,
      83,    -1,   133,    -1,   134,    -1,   148,    -1,   130,    -1,
     162,    -1,   131,    -1,   132,    -1,   147,    -1,   176,    -1,
     141,    82,   142,    -1,   141,   142,    -1,   388,    -1,   222,
      -1,    44,   389,    -1,    45,   389,    -1,   128,   196,   393,
     197,    -1,    64,   393,   203,    -1,   171,   196,   344,   197,
      -1,   391,    -1,   348,    -1,   389,    -1,   196,   390,   197,
      -1,   390,    29,   390,    -1,   390,    30,   390,    -1,   390,
      10,   390,    -1,   390,    12,   390,    -1,   390,    11,   390,
      -1,   390,    31,   390,    -1,   390,    33,   390,    -1,   390,
      32,   390,    -1,   390,    46,   390,    -1,   390,    44,   390,
      -1,   390,    45,   390,    -1,   390,    47,   390,    -1,   390,
      48,   390,    -1,   390,    49,   390,    -1,   390,    43,   390,
      -1,   390,    42,   390,    -1,    50,   390,    -1,    52,   390,
      -1,   390,    35,   390,    -1,   390,    34,   390,    -1,   390,
      37,   390,    -1,   390,    36,   390,    -1,   390,    38,   389,
      -1,   390,    41,   390,    -1,   390,    39,   389,    -1,   390,
      40,   390,    -1,   390,    27,   390,    28,   390,    -1,   390,
      27,    28,   390,    -1,   224,   145,   213,    -1,   151,   145,
     213,    -1,   224,   145,   121,    -1,   222,    -1,    76,    -1,
     441,    -1,   388,    -1,   204,   436,   204,    -1,   205,   436,
     205,    -1,   141,   436,   142,    -1,   396,   394,    -1,    -1,
       9,    -1,    -1,     9,    -1,    -1,   396,     9,   390,   126,
     390,    -1,   396,     9,   390,    -1,   390,   126,   390,    -1,
     390,    -1,    72,    -1,    73,    -1,    74,    -1,    83,    -1,
     141,    82,   142,    -1,   141,   142,    -1,    72,    -1,    73,
      -1,    74,    -1,   213,    -1,   397,    -1,   213,    -1,    44,
     398,    -1,    45,   398,    -1,   128,   196,   400,   197,    -1,
      64,   400,   203,    -1,   171,   196,   403,   197,    -1,   401,
     394,    -1,    -1,   401,     9,   399,   126,   399,    -1,   401,
       9,   399,    -1,   399,   126,   399,    -1,   399,    -1,   402,
       9,   399,    -1,   399,    -1,   404,   394,    -1,    -1,   404,
       9,   340,   126,   399,    -1,   340,   126,   399,    -1,   402,
     394,    -1,    -1,   196,   405,   197,    -1,    -1,   407,     9,
     213,   406,    -1,   213,   406,    -1,    -1,   409,   407,   394,
      -1,    43,   408,    42,    -1,   410,    -1,    -1,   413,    -1,
     125,   422,    -1,   125,   213,    -1,   125,   199,   330,   200,
      -1,    64,   425,   203,    -1,   199,   330,   200,    -1,   418,
     414,    -1,   196,   320,   197,   414,    -1,   428,   414,    -1,
     196,   320,   197,   414,    -1,   422,    -1,   381,    -1,   420,
      -1,   421,    -1,   415,    -1,   417,   412,    -1,   196,   320,
     197,   412,    -1,   383,   145,   422,    -1,   419,   196,   283,
     197,    -1,   196,   417,   197,    -1,   381,    -1,   420,    -1,
     421,    -1,   415,    -1,   417,   413,    -1,   196,   320,   197,
     413,    -1,   419,   196,   283,   197,    -1,   196,   417,   197,
      -1,   422,    -1,   415,    -1,   196,   417,   197,    -1,   417,
     125,   213,   446,   196,   283,   197,    -1,   417,   125,   422,
     196,   283,   197,    -1,   417,   125,   199,   330,   200,   196,
     283,   197,    -1,   196,   320,   197,   125,   213,   446,   196,
     283,   197,    -1,   196,   320,   197,   125,   422,   196,   283,
     197,    -1,   196,   320,   197,   125,   199,   330,   200,   196,
     283,   197,    -1,   383,   145,   213,   446,   196,   283,   197,
      -1,   383,   145,   422,   196,   283,   197,    -1,   383,   145,
     199,   330,   200,   196,   283,   197,    -1,   423,    -1,   426,
     423,    -1,   423,    64,   425,   203,    -1,   423,   199,   330,
     200,    -1,   424,    -1,    77,    -1,   201,   199,   330,   200,
      -1,   330,    -1,    -1,   201,    -1,   426,   201,    -1,   422,
      -1,   416,    -1,   427,   412,    -1,   196,   320,   197,   412,
      -1,   383,   145,   422,    -1,   196,   417,   197,    -1,    -1,
     416,    -1,   427,   413,    -1,   196,   320,   197,   413,    -1,
     196,   417,   197,    -1,   429,     9,    -1,   429,     9,   417,
      -1,   429,     9,   127,   196,   429,   197,    -1,    -1,   417,
      -1,   127,   196,   429,   197,    -1,   431,   394,    -1,    -1,
     431,     9,   330,   126,   330,    -1,   431,     9,   330,    -1,
     330,   126,   330,    -1,   330,    -1,   431,     9,   330,   126,
      33,   417,    -1,   431,     9,    33,   417,    -1,   330,   126,
      33,   417,    -1,    33,   417,    -1,   433,   394,    -1,    -1,
     433,     9,   330,   126,   330,    -1,   433,     9,   330,    -1,
     330,   126,   330,    -1,   330,    -1,   435,   394,    -1,    -1,
     435,     9,   390,   126,   390,    -1,   435,     9,   390,    -1,
     390,   126,   390,    -1,   390,    -1,   436,   437,    -1,   436,
      82,    -1,   437,    -1,    82,   437,    -1,    77,    -1,    77,
      64,   438,   203,    -1,    77,   125,   213,    -1,   143,   330,
     200,    -1,   143,    76,    64,   330,   203,   200,    -1,   144,
     417,   200,    -1,   213,    -1,    78,    -1,    77,    -1,   118,
     196,   440,   197,    -1,   119,   196,   417,   197,    -1,   119,
     196,   331,   197,    -1,   119,   196,   320,   197,    -1,     7,
     330,    -1,     6,   330,    -1,     5,   196,   330,   197,    -1,
       4,   330,    -1,     3,   330,    -1,   417,    -1,   440,     9,
     417,    -1,   383,   145,   213,    -1,   383,   145,   121,    -1,
      -1,    94,   455,    -1,   172,   445,    14,   455,   198,    -1,
     174,   445,   442,    14,   455,   198,    -1,   213,    -1,   455,
     213,    -1,   213,    -1,   213,   167,   450,   168,    -1,   167,
     447,   168,    -1,    -1,   455,    -1,   447,     9,   455,    -1,
     447,     9,   160,    -1,   447,    -1,   160,    -1,    -1,    -1,
      28,   455,    -1,   450,     9,   213,    -1,   213,    -1,   450,
       9,   213,    94,   455,    -1,   213,    94,   455,    -1,    83,
     126,   455,    -1,   452,     9,   451,    -1,   451,    -1,   452,
     394,    -1,    -1,   171,   196,   453,   197,    -1,    27,   455,
      -1,    53,   455,    -1,   224,    -1,   128,    -1,   129,    -1,
     454,    -1,   128,   167,   455,   168,    -1,   128,   167,   455,
       9,   455,   168,    -1,   151,    -1,   196,   102,   196,   448,
     197,    28,   455,   197,    -1,   196,   447,     9,   455,   197,
      -1,   455,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   735,   735,   735,   744,   746,   749,   750,   751,   752,
     753,   754,   757,   759,   759,   761,   761,   763,   764,   766,
     768,   773,   774,   775,   776,   777,   778,   779,   780,   781,
     782,   783,   784,   785,   786,   787,   788,   789,   790,   791,
     795,   797,   801,   803,   807,   809,   813,   814,   815,   816,
     821,   822,   823,   824,   829,   830,   831,   832,   837,   838,
     842,   843,   845,   848,   854,   861,   868,   872,   878,   880,
     883,   884,   885,   886,   889,   890,   894,   899,   899,   905,
     905,   912,   911,   917,   917,   922,   923,   924,   925,   926,
     927,   928,   929,   930,   931,   932,   933,   934,   937,   935,
     942,   950,   944,   954,   952,   956,   957,   961,   962,   963,
     964,   965,   966,   967,   968,   969,   970,   971,   979,   979,
     984,   990,   994,   994,  1002,  1003,  1007,  1008,  1012,  1017,
    1016,  1029,  1027,  1041,  1039,  1055,  1054,  1073,  1071,  1090,
    1089,  1098,  1096,  1108,  1107,  1119,  1117,  1130,  1131,  1135,
    1138,  1141,  1142,  1143,  1146,  1148,  1151,  1152,  1155,  1156,
    1159,  1160,  1164,  1165,  1170,  1171,  1174,  1175,  1176,  1180,
    1181,  1185,  1186,  1190,  1191,  1195,  1196,  1201,  1202,  1207,
    1208,  1209,  1210,  1213,  1216,  1218,  1221,  1222,  1226,  1228,
    1231,  1234,  1237,  1238,  1241,  1242,  1246,  1252,  1259,  1261,
    1266,  1272,  1276,  1280,  1284,  1289,  1294,  1299,  1304,  1310,
    1319,  1324,  1330,  1332,  1336,  1341,  1345,  1348,  1351,  1355,
    1359,  1363,  1367,  1372,  1380,  1382,  1385,  1386,  1387,  1389,
    1394,  1395,  1398,  1399,  1400,  1404,  1405,  1407,  1408,  1412,
    1414,  1417,  1417,  1421,  1420,  1424,  1428,  1426,  1441,  1438,
    1451,  1453,  1455,  1457,  1459,  1461,  1463,  1467,  1468,  1469,
    1472,  1478,  1481,  1487,  1490,  1495,  1497,  1502,  1507,  1511,
    1512,  1518,  1519,  1521,  1525,  1526,  1531,  1532,  1536,  1537,
    1541,  1543,  1549,  1554,  1555,  1557,  1561,  1562,  1563,  1564,
    1568,  1569,  1570,  1571,  1572,  1573,  1575,  1580,  1583,  1584,
    1588,  1589,  1593,  1594,  1597,  1598,  1601,  1602,  1605,  1606,
    1610,  1611,  1612,  1613,  1614,  1615,  1616,  1620,  1621,  1624,
    1625,  1626,  1629,  1631,  1633,  1634,  1637,  1639,  1644,  1645,
    1647,  1648,  1649,  1652,  1656,  1657,  1661,  1662,  1666,  1667,
    1671,  1675,  1680,  1684,  1688,  1693,  1694,  1695,  1698,  1700,
    1701,  1702,  1705,  1706,  1707,  1708,  1709,  1710,  1711,  1712,
    1713,  1714,  1715,  1716,  1717,  1718,  1719,  1720,  1721,  1722,
    1723,  1724,  1725,  1726,  1727,  1728,  1729,  1730,  1731,  1732,
    1733,  1734,  1735,  1736,  1737,  1738,  1739,  1740,  1741,  1742,
    1743,  1744,  1745,  1746,  1747,  1749,  1750,  1752,  1754,  1755,
    1756,  1757,  1758,  1759,  1760,  1761,  1762,  1763,  1764,  1765,
    1766,  1767,  1768,  1769,  1770,  1771,  1772,  1773,  1774,  1778,
    1782,  1787,  1786,  1801,  1799,  1816,  1816,  1831,  1831,  1849,
    1850,  1855,  1860,  1864,  1870,  1874,  1880,  1882,  1886,  1888,
    1892,  1896,  1897,  1901,  1908,  1915,  1917,  1922,  1923,  1924,
    1926,  1930,  1934,  1938,  1942,  1944,  1946,  1948,  1953,  1954,
    1959,  1960,  1961,  1962,  1963,  1964,  1968,  1972,  1976,  1980,
    1985,  1990,  1994,  1995,  1999,  2000,  2004,  2005,  2009,  2010,
    2014,  2018,  2022,  2026,  2027,  2028,  2029,  2033,  2039,  2048,
    2061,  2062,  2065,  2068,  2071,  2072,  2075,  2079,  2082,  2085,
    2092,  2093,  2097,  2098,  2100,  2104,  2105,  2106,  2107,  2108,
    2109,  2110,  2111,  2112,  2113,  2114,  2115,  2116,  2117,  2118,
    2119,  2120,  2121,  2122,  2123,  2124,  2125,  2126,  2127,  2128,
    2129,  2130,  2131,  2132,  2133,  2134,  2135,  2136,  2137,  2138,
    2139,  2140,  2141,  2142,  2143,  2144,  2145,  2146,  2147,  2148,
    2149,  2150,  2151,  2152,  2153,  2154,  2155,  2156,  2157,  2158,
    2159,  2160,  2161,  2162,  2163,  2164,  2165,  2166,  2167,  2168,
    2169,  2170,  2171,  2172,  2173,  2174,  2175,  2176,  2177,  2178,
    2179,  2180,  2181,  2182,  2183,  2187,  2192,  2193,  2196,  2197,
    2198,  2202,  2203,  2204,  2208,  2209,  2210,  2214,  2215,  2216,
    2219,  2221,  2225,  2226,  2227,  2228,  2230,  2231,  2232,  2233,
    2234,  2235,  2236,  2237,  2238,  2239,  2242,  2247,  2248,  2249,
    2250,  2251,  2253,  2254,  2256,  2257,  2261,  2262,  2263,  2265,
    2267,  2269,  2271,  2273,  2274,  2275,  2276,  2277,  2278,  2279,
    2280,  2281,  2282,  2283,  2284,  2285,  2286,  2288,  2290,  2292,
    2294,  2295,  2298,  2299,  2303,  2305,  2309,  2312,  2315,  2321,
    2322,  2323,  2324,  2325,  2326,  2327,  2332,  2334,  2338,  2339,
    2342,  2343,  2347,  2350,  2352,  2354,  2358,  2359,  2360,  2361,
    2363,  2366,  2370,  2371,  2372,  2373,  2376,  2377,  2378,  2379,
    2380,  2382,  2383,  2388,  2390,  2393,  2396,  2398,  2400,  2403,
    2405,  2409,  2411,  2414,  2417,  2423,  2425,  2428,  2429,  2434,
    2437,  2441,  2441,  2446,  2449,  2450,  2454,  2455,  2460,  2461,
    2465,  2466,  2470,  2471,  2476,  2478,  2483,  2484,  2485,  2486,
    2487,  2488,  2489,  2491,  2494,  2496,  2500,  2501,  2502,  2503,
    2504,  2506,  2508,  2510,  2514,  2515,  2516,  2520,  2523,  2526,
    2529,  2533,  2537,  2544,  2548,  2552,  2559,  2560,  2565,  2567,
    2568,  2571,  2572,  2575,  2576,  2580,  2581,  2585,  2586,  2587,
    2588,  2590,  2593,  2596,  2597,  2598,  2600,  2602,  2606,  2607,
    2608,  2610,  2611,  2612,  2616,  2618,  2621,  2623,  2624,  2625,
    2626,  2629,  2631,  2632,  2636,  2638,  2641,  2643,  2644,  2645,
    2649,  2651,  2654,  2657,  2659,  2661,  2665,  2666,  2668,  2669,
    2675,  2676,  2678,  2680,  2682,  2684,  2687,  2688,  2689,  2693,
    2694,  2695,  2696,  2697,  2698,  2699,  2700,  2701,  2705,  2706,
    2710,  2712,  2720,  2722,  2726,  2730,  2737,  2738,  2744,  2745,
    2752,  2755,  2759,  2762,  2767,  2768,  2769,  2770,  2774,  2775,
    2779,  2781,  2782,  2784,  2788,  2794,  2796,  2800,  2803,  2806,
    2814,  2817,  2820,  2821,  2824,  2827,  2828,  2831,  2835,  2839,
    2845,  2853,  2854
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
  "T_MINUS_EQUAL", "T_PLUS_EQUAL", "'?'", "':'", "T_BOOLEAN_OR",
  "T_BOOLEAN_AND", "'|'", "'^'", "'&'", "T_IS_NOT_IDENTICAL",
  "T_IS_IDENTICAL", "T_IS_NOT_EQUAL", "T_IS_EQUAL", "'<'", "'>'",
  "T_IS_GREATER_OR_EQUAL", "T_IS_SMALLER_OR_EQUAL", "T_SR", "T_SL", "'+'",
  "'-'", "'.'", "'*'", "'/'", "'%'", "'!'", "T_INSTANCEOF", "'~'", "'@'",
  "T_UNSET_CAST", "T_BOOL_CAST", "T_OBJECT_CAST", "T_ARRAY_CAST",
  "T_STRING_CAST", "T_DOUBLE_CAST", "T_INT_CAST", "T_DEC", "T_INC",
  "T_POW", "'['", "T_CLONE", "T_NEW", "T_EXIT", "T_IF", "T_ELSEIF",
  "T_ELSE", "T_ENDIF", "T_LNUMBER", "T_DNUMBER", "T_ONUMBER", "T_STRING",
  "T_STRING_VARNAME", "T_VARIABLE", "T_NUM_STRING", "T_INLINE_HTML",
  "T_CHARACTER", "T_BAD_CHARACTER", "T_ENCAPSED_AND_WHITESPACE",
  "T_CONSTANT_ENCAPSED_STRING", "T_ECHO", "T_DO", "T_WHILE", "T_ENDWHILE",
  "T_FOR", "T_ENDFOR", "T_FOREACH", "T_ENDFOREACH", "T_DECLARE",
  "T_ENDDECLARE", "T_AS", "T_SWITCH", "T_ENDSWITCH", "T_CASE", "T_DEFAULT",
  "T_BREAK", "T_GOTO", "T_CONTINUE", "T_FUNCTION", "T_CONST", "T_RETURN",
  "T_TRY", "T_CATCH", "T_THROW", "T_USE", "T_GLOBAL", "T_PUBLIC",
  "T_PROTECTED", "T_PRIVATE", "T_FINAL", "T_ABSTRACT", "T_STATIC", "T_VAR",
  "T_UNSET", "T_ISSET", "T_EMPTY", "T_HALT_COMPILER", "T_CLASS",
  "T_INTERFACE", "T_EXTENDS", "T_IMPLEMENTS", "T_OBJECT_OPERATOR",
  "T_DOUBLE_ARROW", "T_LIST", "T_ARRAY", "T_CALLABLE", "T_CLASS_C",
  "T_METHOD_C", "T_FUNC_C", "T_LINE", "T_FILE", "T_COMMENT",
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
  "use_declarations", "use_fn_declarations", "use_const_declarations",
  "use_declaration", "use_fn_declaration", "use_const_declaration",
  "namespace_name", "namespace_string_base", "namespace_string",
  "namespace_string_typeargs", "class_namespace_string_typeargs",
  "constant_declaration", "inner_statement_list", "inner_statement",
  "statement", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9", "$@10",
  "try_statement_list", "$@11", "additional_catches",
  "finally_statement_list", "$@12", "optional_finally", "is_reference",
  "function_loc", "function_declaration_statement", "$@13", "$@14", "$@15",
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
  "static_expr", "static_class_constant", "scalar",
  "static_array_pair_list", "possible_comma", "hh_possible_comma",
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
     273,   274,   275,   276,   277,   278,   279,    63,    58,   280,
     281,   124,    94,    38,   282,   283,   284,   285,    60,    62,
     286,   287,   288,   289,    43,    45,    46,    42,    47,    37,
      33,   290,   126,    64,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,    91,   301,   302,   303,   304,   305,
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
     426,   427,   428,   429,   430,   431,    40,    41,    59,   123,
     125,    36,    96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   206,   208,   207,   209,   209,   210,   210,   210,   210,
     210,   210,   210,   211,   210,   212,   210,   210,   210,   210,
     210,   213,   213,   213,   213,   213,   213,   213,   213,   213,
     213,   213,   213,   213,   213,   213,   213,   213,   213,   213,
     214,   214,   215,   215,   216,   216,   217,   217,   217,   217,
     218,   218,   218,   218,   219,   219,   219,   219,   220,   220,
     221,   221,   221,   222,   223,   224,   225,   225,   226,   226,
     227,   227,   227,   227,   228,   228,   228,   229,   228,   230,
     228,   231,   228,   232,   228,   228,   228,   228,   228,   228,
     228,   228,   228,   228,   228,   228,   228,   228,   233,   228,
     228,   234,   228,   235,   228,   228,   228,   228,   228,   228,
     228,   228,   228,   228,   228,   228,   228,   228,   237,   236,
     238,   238,   240,   239,   241,   241,   242,   242,   243,   245,
     244,   246,   244,   247,   244,   249,   248,   250,   248,   251,
     248,   252,   248,   254,   253,   255,   253,   256,   256,   257,
     258,   259,   259,   259,   260,   260,   261,   261,   262,   262,
     263,   263,   264,   264,   265,   265,   266,   266,   266,   267,
     267,   268,   268,   269,   269,   270,   270,   271,   271,   272,
     272,   272,   272,   273,   273,   273,   274,   274,   275,   275,
     276,   276,   277,   277,   278,   278,   279,   279,   279,   279,
     279,   279,   280,   280,   280,   280,   280,   280,   280,   280,
     281,   281,   281,   281,   281,   281,   282,   282,   282,   282,
     282,   282,   282,   282,   283,   283,   284,   284,   284,   284,
     285,   285,   286,   286,   286,   287,   287,   287,   287,   288,
     288,   290,   289,   291,   289,   289,   292,   289,   293,   289,
     289,   289,   289,   289,   289,   289,   289,   294,   294,   294,
     295,   296,   296,   297,   297,   298,   298,   299,   299,   300,
     300,   300,   300,   300,   301,   301,   302,   302,   303,   303,
     304,   304,   305,   306,   306,   306,   307,   307,   307,   307,
     308,   308,   308,   308,   308,   308,   308,   309,   309,   309,
     310,   310,   311,   311,   312,   312,   313,   313,   314,   314,
     315,   315,   315,   315,   315,   315,   315,   316,   316,   317,
     317,   317,   318,   318,   318,   318,   319,   319,   320,   320,
     320,   320,   320,   321,   322,   322,   323,   323,   324,   324,
     325,   326,   327,   328,   329,   330,   330,   330,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   332,
     332,   334,   333,   335,   333,   337,   336,   338,   336,   339,
     339,   340,   341,   341,   342,   342,   343,   343,   344,   344,
     345,   346,   346,   347,   348,   349,   349,   350,   350,   350,
     350,   351,   352,   353,   354,   354,   354,   354,   355,   355,
     356,   356,   356,   356,   356,   356,   357,   358,   359,   360,
     361,   362,   363,   363,   364,   364,   365,   365,   366,   366,
     367,   368,   369,   370,   370,   370,   370,   371,   372,   372,
     373,   373,   374,   374,   375,   375,   376,   377,   377,   378,
     378,   378,   379,   379,   379,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   381,   382,   382,   383,   383,
     383,   384,   384,   384,   385,   385,   385,   386,   386,   386,
     387,   387,   388,   388,   388,   388,   388,   388,   388,   388,
     388,   388,   388,   388,   388,   388,   388,   389,   389,   389,
     389,   389,   389,   389,   389,   389,   390,   390,   390,   390,
     390,   390,   390,   390,   390,   390,   390,   390,   390,   390,
     390,   390,   390,   390,   390,   390,   390,   390,   390,   390,
     390,   390,   390,   390,   390,   390,   391,   391,   391,   392,
     392,   392,   392,   392,   392,   392,   393,   393,   394,   394,
     395,   395,   396,   396,   396,   396,   397,   397,   397,   397,
     397,   397,   398,   398,   398,   398,   399,   399,   399,   399,
     399,   399,   399,   400,   400,   401,   401,   401,   401,   402,
     402,   403,   403,   404,   404,   405,   405,   406,   406,   407,
     407,   409,   408,   410,   411,   411,   412,   412,   413,   413,
     414,   414,   415,   415,   416,   416,   417,   417,   417,   417,
     417,   417,   417,   417,   417,   417,   418,   418,   418,   418,
     418,   418,   418,   418,   419,   419,   419,   420,   420,   420,
     420,   420,   420,   421,   421,   421,   422,   422,   423,   423,
     423,   424,   424,   425,   425,   426,   426,   427,   427,   427,
     427,   427,   427,   428,   428,   428,   428,   428,   429,   429,
     429,   429,   429,   429,   430,   430,   431,   431,   431,   431,
     431,   431,   431,   431,   432,   432,   433,   433,   433,   433,
     434,   434,   435,   435,   435,   435,   436,   436,   436,   436,
     437,   437,   437,   437,   437,   437,   438,   438,   438,   439,
     439,   439,   439,   439,   439,   439,   439,   439,   440,   440,
     441,   441,   442,   442,   443,   443,   444,   444,   445,   445,
     446,   446,   447,   447,   448,   448,   448,   448,   449,   449,
     450,   450,   450,   450,   451,   452,   452,   453,   453,   454,
     455,   455,   455,   455,   455,   455,   455,   455,   455,   455,
     455,   456,   456
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     4,     3,     0,     6,     0,     5,     3,     4,     4,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     3,     1,     3,     1,     1,     2,     3,     4,
       1,     2,     3,     4,     1,     2,     3,     4,     1,     3,
       1,     3,     2,     1,     2,     2,     5,     4,     2,     0,
       1,     1,     1,     1,     3,     5,     8,     0,     4,     0,
       6,     0,    10,     0,     4,     2,     3,     2,     3,     2,
       3,     3,     3,     3,     3,     5,     1,     1,     0,     9,
       5,     0,    13,     0,     5,     3,     3,     2,     2,     2,
       2,     2,     2,     3,     2,     2,     3,     2,     0,     4,
       9,     0,     0,     4,     2,     0,     1,     0,     1,     0,
       9,     0,    10,     0,    11,     0,     8,     0,     9,     0,
       7,     0,     8,     0,     7,     0,     8,     1,     1,     1,
       1,     1,     2,     2,     2,     0,     2,     0,     2,     0,
       1,     3,     1,     3,     2,     0,     1,     2,     4,     1,
       4,     1,     4,     1,     4,     1,     4,     3,     5,     3,
       4,     4,     5,     5,     4,     0,     1,     1,     4,     0,
       5,     0,     2,     0,     3,     0,     7,     6,     2,     5,
       4,     0,     4,     5,     7,     6,     6,     7,     9,     8,
       6,     5,     2,     4,     3,     0,     3,     4,     6,     5,
       5,     6,     8,     7,     2,     0,     1,     2,     3,     4,
       3,     1,     1,     2,     4,     3,     5,     1,     3,     2,
       0,     0,     4,     0,     5,     2,     0,    10,     0,    11,
       3,     3,     3,     4,     4,     3,     5,     2,     2,     0,
       6,     5,     4,     3,     1,     1,     3,     4,     1,     1,
       1,     1,     4,     1,     1,     3,     2,     0,     2,     0,
       1,     3,     1,     1,     1,     1,     3,     4,     4,     4,
       1,     1,     2,     2,     2,     3,     3,     1,     1,     1,
       1,     3,     1,     3,     1,     1,     1,     0,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     1,
       1,     1,     3,     5,     1,     3,     5,     4,     3,     3,
       2,     1,     1,     3,     3,     1,     1,     0,     2,     4,
       3,     6,     2,     3,     6,     1,     1,     1,     6,     3,
       4,     6,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     5,
       4,     1,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     1,     1,     1,     3,     2,     1,     1,     1,     5,
       0,     0,    11,     0,    12,     0,     3,     0,     6,     2,
       4,     1,     5,     3,     5,     3,     2,     0,     2,     0,
       4,     4,     3,     4,     4,     4,     4,     1,     1,     1,
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
       1,     1,     1,     1,     1,     3,     2,     1,     1,     2,
       2,     4,     3,     4,     1,     1,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     5,     4,     3,     3,     3,     1,
       1,     1,     1,     3,     3,     3,     2,     0,     1,     0,
       1,     0,     5,     3,     3,     1,     1,     1,     1,     1,
       3,     2,     1,     1,     1,     1,     1,     1,     2,     2,
       4,     3,     4,     2,     0,     5,     3,     3,     1,     3,
       1,     2,     0,     5,     3,     2,     0,     3,     0,     4,
       2,     0,     3,     3,     1,     0,     1,     2,     2,     4,
       3,     3,     2,     4,     2,     4,     1,     1,     1,     1,
       1,     2,     4,     3,     4,     3,     1,     1,     1,     1,
       2,     4,     4,     3,     1,     1,     3,     7,     6,     8,
       9,     8,    10,     7,     6,     8,     1,     2,     4,     4,
       1,     1,     4,     1,     0,     1,     2,     1,     1,     2,
       4,     3,     3,     0,     1,     2,     4,     3,     2,     3,
       6,     0,     1,     4,     2,     0,     5,     3,     3,     1,
       6,     4,     4,     2,     2,     0,     5,     3,     3,     1,
       2,     0,     5,     3,     3,     1,     2,     2,     1,     2,
       1,     4,     3,     3,     6,     3,     1,     1,     1,     4,
       4,     4,     4,     2,     2,     4,     2,     2,     1,     3,
       3,     3,     0,     2,     5,     6,     1,     2,     1,     4,
       3,     0,     1,     3,     3,     1,     1,     0,     0,     2,
       3,     1,     5,     3,     3,     3,     1,     2,     0,     4,
       2,     2,     1,     1,     1,     1,     4,     6,     1,     8,
       5,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   711,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   785,     0,   773,   596,
       0,   602,   603,   604,    21,   660,   761,    97,   605,     0,
      79,     0,     0,     0,     0,     0,     0,     0,     0,   128,
       0,     0,     0,     0,     0,     0,   310,   311,   312,   315,
     314,   313,     0,     0,     0,     0,   151,     0,     0,     0,
     609,   611,   612,   606,   607,     0,     0,   613,   608,     0,
       0,   587,    22,    23,    24,    26,    25,     0,   610,     0,
       0,     0,     0,   614,     0,   316,    27,    28,    30,    29,
      31,    32,    33,    34,    35,    36,    37,    38,    39,   427,
       0,    96,    69,   765,   597,     0,     0,     4,    58,    60,
      63,   659,     0,   586,     0,     6,   127,     7,     8,     9,
       0,     0,   308,   347,     0,     0,     0,     0,     0,     0,
       0,   345,   416,   417,   413,   412,   332,   418,     0,     0,
     331,   727,   588,     0,   662,   411,   307,   730,   346,     0,
       0,   728,   729,   726,   756,   760,     0,   401,   661,    10,
     315,   314,   313,     0,     0,    58,   127,     0,   827,   346,
     826,     0,   824,   823,   415,     0,     0,   385,   386,   387,
     388,   410,   408,   407,   406,   405,   404,   403,   402,   761,
     589,     0,   841,   588,     0,   367,   365,     0,   789,     0,
     669,   330,   592,     0,   841,   591,     0,   601,   768,   767,
     593,     0,     0,   595,   409,     0,     0,     0,     0,   335,
       0,    77,   337,     0,     0,    83,    85,     0,     0,    87,
       0,     0,     0,   863,   864,   868,     0,     0,    58,   862,
       0,   865,     0,     0,    89,     0,     0,     0,     0,   118,
       0,     0,     0,     0,     0,     0,    41,    46,   232,     0,
       0,   231,   153,   152,   237,     0,     0,     0,     0,     0,
     838,   139,   149,   781,   785,   810,     0,   616,     0,     0,
       0,   808,     0,    15,     0,    62,     0,   338,   143,   150,
     493,   437,     0,   832,   342,   715,   347,     0,   345,   346,
       0,     0,   598,     0,   599,     0,     0,     0,   117,     0,
       0,    65,   225,     0,    20,   126,     0,   148,   135,   147,
     313,   127,   309,   108,   109,   110,   111,   112,   114,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   773,     0,   107,   764,   764,   115,
     795,     0,     0,     0,     0,     0,   306,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     366,   364,     0,   731,   716,   764,     0,   722,   225,   764,
       0,   766,   757,   781,     0,   127,     0,     0,   713,   708,
     669,     0,     0,     0,     0,   793,     0,   442,   668,   784,
       0,     0,    65,     0,   225,   329,     0,   769,   716,   724,
     594,     0,    69,   189,     0,   426,     0,    94,     0,     0,
     336,     0,     0,     0,     0,     0,    86,   106,    88,   860,
     861,     0,   858,     0,     0,   842,     0,   837,     0,   113,
      90,   116,     0,     0,     0,     0,     0,     0,     0,   451,
       0,   458,   460,   461,   462,   463,   464,   465,   456,   478,
     479,    69,     0,   103,   105,     0,     0,    43,    50,     0,
       0,    45,    54,    47,     0,    17,     0,     0,   233,     0,
      92,     0,     0,    93,   828,     0,     0,   347,   345,   346,
       0,     0,   159,     0,   782,     0,     0,     0,     0,   615,
     809,   660,     0,     0,   807,   665,   806,    61,     5,    12,
      13,    91,     0,   157,     0,     0,   431,     0,   669,     0,
       0,     0,     0,     0,   671,   714,   872,   328,   398,   735,
      74,    68,    70,    71,    72,    73,     0,   414,   663,   664,
      59,     0,     0,     0,   671,   226,     0,   421,   129,   155,
       0,   370,   372,   371,     0,     0,   368,   369,   373,   375,
     374,   390,   389,   392,   391,   393,   395,   396,   394,   384,
     383,   377,   378,   376,   379,   380,   382,   397,   381,   763,
       0,     0,   799,     0,   669,   831,     0,   830,   733,   756,
     141,   145,   137,   127,     0,     0,   340,   343,   349,   452,
     363,   362,   361,   360,   359,   358,   357,   356,   355,   354,
     353,   352,     0,   718,   717,     0,     0,     0,     0,     0,
       0,     0,   825,   706,   710,   668,   712,     0,     0,   841,
       0,   788,     0,   787,     0,   772,   771,     0,     0,   718,
     717,   333,   191,   193,    69,   429,   334,     0,    69,   173,
      78,   337,     0,     0,     0,     0,   185,   185,    84,     0,
       0,   856,   669,     0,   847,     0,     0,     0,     0,     0,
     667,   605,     0,     0,   587,     0,     0,    63,   618,   586,
     625,     0,   617,   626,    67,   624,     0,     0,   468,     0,
       0,   474,   471,   472,   480,     0,   459,   454,     0,   457,
       0,     0,     0,    51,    18,     0,     0,    55,    19,     0,
       0,     0,    40,    48,     0,   230,   238,   235,     0,     0,
     819,   822,   821,   820,    11,   851,     0,     0,     0,   781,
     778,     0,   441,   818,   817,   816,     0,   812,     0,   813,
     815,     0,     5,   339,     0,     0,   487,   488,   496,   495,
       0,     0,   668,   436,   440,     0,   833,     0,   848,   715,
     212,   871,     0,     0,   732,   716,   723,   762,     0,   840,
     227,   585,   670,   224,     0,   715,     0,     0,   157,   423,
     131,   400,     0,   445,   446,     0,   443,   668,   794,     0,
       0,   225,   159,   157,   155,     0,   773,   350,     0,     0,
     225,   720,   721,   734,   758,   759,     0,     0,     0,   694,
     676,   677,   678,   679,     0,     0,     0,   687,   686,   700,
     669,     0,   708,   792,   791,     0,   770,   716,   725,   600,
       0,   195,     0,     0,    75,     0,     0,     0,     0,     0,
       0,   165,   166,   177,     0,    69,   175,   100,   185,     0,
     185,     0,     0,   866,     0,   668,   857,   859,   846,   845,
       0,   843,   619,   620,   644,   645,   675,     0,   669,   667,
       0,     0,   439,     0,     0,   801,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   453,     0,     0,     0,   476,   477,   475,     0,     0,
     455,     0,   119,     0,   122,   104,     0,    42,    52,     0,
      44,    56,    49,   234,     0,   829,    95,     0,     0,   839,
     158,   160,   240,     0,     0,   779,     0,   811,     0,    16,
       0,   156,   240,     0,     0,   433,     0,   834,     0,     0,
       0,   872,     0,   216,   214,     0,   718,   717,   843,     0,
     228,    66,     0,   715,   154,     0,   715,     0,   399,   798,
     797,     0,   225,     0,     0,     0,   157,   133,   601,   719,
     225,     0,     0,   682,   683,   684,   685,   688,   689,   698,
       0,   669,   694,     0,   681,   702,   668,   705,   707,   709,
       0,   786,   719,     0,     0,     0,     0,   192,   430,    80,
       0,   337,   167,   781,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   179,     0,   854,   855,     0,     0,   870,
       0,   622,   668,   666,     0,   657,     0,   669,     0,   627,
     658,   656,   805,     0,   669,   630,   632,   631,     0,     0,
     628,   629,   633,   635,   634,   647,   646,   649,   648,   650,
     652,   653,   651,   643,   642,   637,   638,   636,   639,   640,
     641,   466,     0,   467,   473,   481,   482,     0,    69,    53,
      57,   236,   853,   850,     0,   307,   783,   781,   341,   344,
     348,     0,    14,   307,   499,     0,     0,   501,   494,   497,
       0,   492,     0,   835,   849,   428,     0,   217,     0,   213,
       0,     0,   225,   229,   848,     0,   240,     0,   715,     0,
     225,     0,   754,   240,   240,     0,     0,   351,   225,     0,
     748,     0,   691,   668,   693,     0,   680,     0,     0,   669,
     699,   790,     0,    69,     0,   188,   174,     0,     0,   164,
      98,   178,     0,     0,   181,     0,   186,   187,    69,   180,
     867,   844,     0,   674,   673,   621,     0,   668,   438,   623,
       0,   444,   668,   800,   655,     0,     0,     0,     0,     0,
     161,     0,     0,     0,   305,     0,     0,     0,   140,   239,
     241,     0,   304,     0,   307,     0,   814,   144,   490,     0,
       0,   432,     0,   220,   211,     0,   219,   719,   225,     0,
     420,   848,   307,   848,     0,   796,     0,   753,   307,   307,
     240,   715,     0,   747,   697,   696,   690,     0,   692,   668,
     701,    69,   194,    76,    81,   168,     0,   176,   182,    69,
     184,     0,     0,   435,     0,   804,   803,   654,     0,    69,
     123,   852,     0,     0,     0,     0,   162,   271,   269,   273,
     587,    26,     0,   265,     0,   270,   282,     0,   280,   285,
       0,   284,     0,   283,     0,   127,   243,     0,   245,     0,
     780,   491,   489,   500,   498,   221,     0,   210,   218,   225,
       0,   751,     0,     0,     0,   136,   420,   848,   755,   142,
     146,   307,     0,   749,     0,   704,     0,   190,     0,    69,
     171,    99,   183,   869,   672,     0,     0,     0,     0,     0,
       0,     0,     0,   255,   259,     0,     0,   250,   551,   550,
     547,   549,   548,   568,   570,   569,   539,   529,   545,   544,
     506,   516,   517,   519,   518,   538,   522,   520,   521,   523,
     524,   525,   526,   527,   528,   530,   531,   532,   533,   534,
     535,   537,   536,   507,   508,   509,   512,   513,   515,   553,
     554,   563,   562,   561,   560,   559,   558,   546,   565,   555,
     556,   557,   540,   541,   542,   543,   566,   567,   571,   573,
     572,   574,   575,   552,   577,   576,   510,   579,   581,   580,
     514,   584,   582,   583,   578,   511,   564,   505,   277,   502,
       0,   251,   298,   299,   297,   290,     0,   291,   252,   324,
       0,     0,     0,     0,   127,     0,   223,     0,   750,     0,
      69,   300,    69,   130,     0,     0,   138,   848,   695,     0,
      69,   169,    82,     0,   434,   802,   469,   121,   253,   254,
     327,   163,     0,     0,   274,   266,     0,     0,     0,   279,
     281,     0,     0,   286,   293,   294,   292,     0,     0,   242,
       0,     0,     0,     0,   222,   752,     0,   485,   671,     0,
       0,    69,   132,     0,   703,     0,     0,     0,   101,   256,
      58,     0,   257,   258,     0,     0,   272,   276,   503,   504,
       0,   267,   295,   296,   288,   289,   287,   325,   322,   246,
     244,   326,     0,   486,   670,     0,   422,   301,     0,   134,
       0,   172,   470,     0,   125,     0,   307,   275,   278,     0,
     715,   248,     0,   483,   419,   424,   170,     0,     0,   102,
     263,     0,   306,   323,     0,   671,   318,   715,   484,     0,
     124,     0,     0,   262,   848,   715,   198,   319,   320,   321,
     872,   317,     0,     0,     0,   261,     0,   318,     0,   848,
       0,   260,   302,    69,   247,   872,     0,   202,   200,     0,
      69,     0,     0,   203,     0,   199,   249,     0,   303,     0,
     206,   197,     0,   205,   120,   207,     0,   196,   204,     0,
     209,   208
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   117,   762,   528,   175,   265,   486,
     490,   266,   487,   491,   119,   120,   121,   122,   123,   124,
     310,   551,   552,   439,   230,  1328,   445,  1256,  1544,   722,
     260,   481,  1508,   935,  1098,  1559,   326,   176,   553,   796,
     987,  1146,   554,   569,   814,   512,   812,   555,   533,   813,
     328,   281,   298,   130,   798,   765,   748,   950,  1275,  1035,
     861,  1462,  1331,   670,   867,   444,   678,   869,  1178,   663,
     851,   854,  1025,  1564,  1565,   543,   544,   563,   564,   270,
     271,   275,  1105,  1209,  1294,  1442,  1550,  1567,  1472,  1512,
    1513,  1514,  1282,  1283,  1284,  1473,  1479,  1521,  1287,  1288,
    1292,  1435,  1436,  1437,  1453,  1594,  1210,  1211,   177,   132,
    1580,  1581,  1440,  1213,   133,   223,   440,   441,   134,   135,
     136,   137,   138,   139,   140,   141,  1313,   142,   795,   986,
     143,   227,   305,   435,   537,   538,  1057,   539,  1058,   144,
     145,   146,   700,   147,   148,   257,   149,   258,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   712,   713,   927,
     478,   479,   480,   719,  1498,   150,   534,  1302,   535,   963,
     770,  1121,  1118,  1428,  1429,   151,   152,   153,   217,   224,
     313,   425,   154,   703,   886,   705,   155,   887,   419,   780,
     888,   838,  1007,  1009,  1010,  1011,   840,  1158,  1159,   841,
     644,   410,   185,   186,   156,   546,   393,   394,   786,   157,
     218,   179,   159,   160,   161,   162,   163,   164,   165,   600,
     166,   220,   221,   515,   209,   210,   603,   604,  1063,  1064,
     290,   291,   756,   167,   505,   168,   542,   169,   250,   282,
     321,   454,   880,   970,   746,   681,   682,   683,   251,   252,
     782
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1352
static const yytype_int16 yypact[] =
{
   -1352,   111, -1352, -1352,  5441, 12749, 12749,   -78, 12749, 12749,
   12749, -1352, 12749, 12749, 12749, 12749, 12749, 12749, 12749, 12749,
   12749, 12749, 12749, 12749, 14344, 14344, 10110, 12749, 14393,   -67,
     -58, -1352, -1352, -1352, -1352, -1352,   141, -1352,   107, 12749,
   -1352,   -58,   -55,   -35,   -27,   -58, 10313, 11369, 10516, -1352,
   13771,  4718,   -26, 12749, 14575,   133, -1352, -1352, -1352,    76,
     228,    13,     4,   155,   181,   187, -1352, 11369,   189,   191,
   -1352, -1352, -1352, -1352, -1352,    23,  2504, -1352, -1352, 11369,
   10719, -1352, -1352, -1352, -1352, -1352, -1352, 11369, -1352,   238,
     195, 11369, 11369, -1352, 12749, -1352, -1352, -1352, -1352, -1352,
   -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352,
   12749, -1352, -1352,   194,    68,   350,   350, -1352,   370,   256,
     441, -1352,   211, -1352,    25, -1352,   388, -1352, -1352, -1352,
   14675,   409, -1352, -1352,   232,   236,   253,   279,   282,   287,
    4178, -1352, -1352, -1352, -1352,   401, -1352,   405,   431,   302,
   -1352,    78,   313,   358, -1352, -1352,   691,    20,   553,   108,
     332,   110,   114,   335,   121, -1352,   267, -1352,   480, -1352,
   -1352, -1352,   389,   356,   449, -1352,   388,   409, 15493,  2040,
   15493, 12749, 15493, 15493,  4951,   548, 11369,   541,   541,    62,
     541,   541,   541,   541,   541,   541,   541,   541,   541, -1352,
   -1352,  4253,   442, -1352,   457,   481,   481, 14344, 15003,   416,
     603, -1352,   389,  4253,   442,   483,   493,   450,   115, -1352,
     522,   108, 10922, -1352, -1352, 12749,  8689,   641,    53, 15493,
    9704, -1352, 12749, 12749, 11369, -1352, -1352,  4588,   462, -1352,
    4916, 13771, 13771,   505, -1352, -1352,   485, 13552,   663, -1352,
     665, -1352, 11369,   605, -1352,   487, 13338,   488,   475, -1352,
      -7, 13380, 14719, 14763, 11369,    57, -1352,    41, -1352, 14244,
      59, -1352, -1352, -1352,   674,    60, 14344, 14344, 12749,   495,
     528, -1352, -1352,  4338, 10110,    30,   347, -1352, 12952, 14344,
     453, -1352, 11369, -1352,   298,   256,   498, 15045, -1352, -1352,
   -1352,   614,   684,   616, 15493,   105,   511, 15493,   515,   539,
    5644, 12749,    40,   512,   372,    40,   270,   268, -1352, 11369,
   13771,   517, 11125, 13771, -1352, -1352,  5074, -1352, -1352, -1352,
   -1352,   388, -1352, -1352, -1352, -1352, -1352, -1352, -1352, 12749,
   12749, 12749, 11328, 12749, 12749, 12749, 12749, 12749, 12749, 12749,
   12749, 12749, 12749, 12749, 12749, 12749, 12749, 12749, 12749, 12749,
   12749, 12749, 12749, 12749, 14393, 12749, -1352, 12749, 12749, -1352,
   12749,  1697, 11369, 11369, 14675,   613,   470,  3133, 12749, 12749,
   12749, 12749, 12749, 12749, 12749, 12749, 12749, 12749, 12749, 12749,
   -1352, -1352,  1578, -1352,   117, 12749, 12749, -1352, 11125, 12749,
   12749,   194,   122,  4338,   521,   388, 11531,  4264, -1352,   523,
     709,  4253,   526,   -46,  1895,   481, 11734, -1352, 11937, -1352,
     531,   -33, -1352,   269, 11125, -1352, 13927, -1352,   124, -1352,
   -1352, 13422, -1352, -1352, 12140, -1352, 12749, -1352,   634,  8892,
     716,   532, 12937,   717,    56,    29, -1352, -1352, -1352, -1352,
   -1352, 13771,   649,   538,   726, -1352, 14015, -1352,   555, -1352,
   -1352, -1352,   661, 12749,   662,   666, 12749, 12749, 12749, -1352,
     475, -1352, -1352, -1352, -1352, -1352, -1352, -1352,   559, -1352,
   -1352, -1352,   544, -1352, -1352, 11369,   550,   733,   241, 11369,
     551,   737,   259,   266, 14807, -1352, 11369, 12749,   481,   133,
   -1352, 14015,   675, -1352,   481,    77,    87,   556,   558,  1590,
     562, 11369,   631,   565,   481,    89,   566, 14662, 11369, -1352,
   -1352,   692,  1794,   -54, -1352, -1352, -1352,   256, -1352, -1352,
   -1352, -1352, 12749,   638,   599,   291, -1352,   650,   769,   583,
   13771, 13771,   767,   590,   781, -1352, 13771,   106,   727,   104,
   -1352, -1352, -1352, -1352, -1352, -1352,  1914, -1352, -1352, -1352,
   -1352,    45, 14344,   596,   785, 15493,   782, -1352, -1352,   672,
    9948, 10501,  3915,  4951, 12749, 15439,  5628,  3622,  2804,  2319,
    2081,  4618,  4618,  4618,  4618,  2254,  2254,  2254,  2254,   812,
     812,   573,   573,   573,    62,    62,    62, -1352,   541, 15493,
     594,   611, 15101,   607,   806, -1352, 12749,   -57,   620,   122,
   -1352, -1352, -1352,   388, 13369, 12749, -1352, -1352,  4951, -1352,
    4951,  4951,  4951,  4951,  4951,  4951,  4951,  4951,  4951,  4951,
    4951,  4951, 12749,   -57,   623,   617,  2169,   624,   619,  2228,
      92,   630, -1352, 14197, -1352, 11369, -1352,   511,   106,   442,
   14344, 15493, 14344, 15143,   237,   125, -1352,   633, 12749, -1352,
   -1352, -1352,  8486,   439, -1352, 15493, 15493,   -58, -1352, -1352,
   -1352, 12749,  2754, 14015, 11369,  9095,   629,   636, -1352,    47,
     702, -1352,   822,   635, 13629, 13771, 14106, 14106, 14015, 14015,
   14015, -1352,   640,    34,   693,   643, 14015,   -24, -1352,   695,
   -1352,   642, -1352, -1352, 15575, -1352, 12749,   655, 15493,   670,
     829, 10907,   839, -1352, 15493, 10298, -1352,   559,   777, -1352,
    5847, 14622,   667,   288, -1352, 14719, 11369,   292, -1352, 14763,
   11369, 11369, -1352, -1352,  2469, -1352, 15575,   853, 14344,   673,
   -1352, -1352, -1352, -1352, -1352,   774,    61, 14622,   671,  4338,
   14295,   858, -1352, -1352, -1352, -1352,   676, -1352, 12749, -1352,
   -1352,  5020, -1352, 15493, 14622,   678, -1352, -1352, -1352, -1352,
     859, 12749,   614, -1352, -1352,   682, -1352, 13771,   846,    24,
   -1352, -1352,   271, 14434, -1352,   129, -1352, -1352, 13771, -1352,
     481, -1352, 12343, -1352, 14015,    37,   685, 14622,   638, -1352,
   -1352,  5426, 12749, -1352, -1352, 12749, -1352, 12749, -1352,  2518,
     686, 11125,   631,   638,   672, 11369, 14393,   481,  2771,   687,
   11125, -1352, -1352,   131, -1352, -1352,   870, 14483, 14483, 14197,
   -1352, -1352, -1352, -1352,   689,    54,   696, -1352, -1352, -1352,
     882,   697,   523,   481,   481, 12546, -1352,   134, -1352, -1352,
    2975,   456,   -58,  9704, -1352,  6050,   698,  6253,   699, 14344,
     707,   772,   481, 15575,   879, -1352, -1352, -1352, -1352,   321,
   -1352,    17, 13771, -1352, 13771,   649, -1352, -1352, -1352,   886,
     708,   710, -1352, -1352, -1352, -1352, 15241,   706,   903, 14015,
     771, 11369,   614, 13464, 14820, 14015, 14015, 14015, 14015, 13866,
   14015, 14015, 14015, 14015, 14015, 14015, 14015, 14015, 14015, 14106,
   14106, 14015, 14015, 14015, 14015, 14015, 14015, 14015, 14015, 14015,
   14015, 15493, 12749, 12749, 12749, -1352, -1352, -1352, 12749, 12749,
   -1352,   475, -1352,   837, -1352, -1352, 11369, -1352, -1352, 11369,
   -1352, -1352, -1352, -1352, 14015,   481, -1352, 13771, 11369, -1352,
     906, -1352, -1352,    97,   720,   481,  9907, -1352,  1663, -1352,
    5238,   906, -1352,   -12,   -19, 15493,   791, -1352,   721, 13771,
     641, 13771,   841,   907,   843, 12749,   -57,   728, -1352, 14344,
   15493, 15575,   729,    37, -1352,   724,    37,   732,  5426, 15493,
   15199,   734, 11125,   736,   730,   735,   638, -1352,   450,   742,
   11125,   743, 12749, -1352, -1352, -1352, -1352, -1352, -1352,   805,
     738,   930, 14197,   804, -1352,   614, 14197, -1352, -1352, -1352,
   14344, 15493, -1352,   -58,   919,   877,  9704, -1352, -1352, -1352,
     751, 12749,   481,  4338,  2754,   753, 14015,  6456,   443,   754,
   12749,    94,   258, -1352,   783, -1352, -1352, 13694,   925, -1352,
   14015, -1352, 14015, -1352,   758, -1352,   836,   954,   768, -1352,
   -1352, -1352, 15297,   764,   957, 10704,  3485,  5832, 14015, 15535,
    6237,  6439,  3316,  6640,  4712,  6842,  6842,  6842,  6842, -1352,
   -1352,  1169,  1169,   658,   658,   604,   604,   604, -1352, -1352,
   -1352, 15493, 12734, 15493, -1352, 15493, -1352,   775, -1352, -1352,
   -1352, 15575, -1352,   883, 14622,    91, -1352,  4338, -1352, -1352,
    4951,   773, -1352,   435, -1352,   328, 12749, -1352, -1352, -1352,
   12749, -1352, 12749, -1352, -1352, -1352,   284,   962, 14015, -1352,
    3447,   784, 11125,   481,   846,   786, -1352,   788,    37, 12749,
   11125,   789, -1352, -1352, -1352,   779,   792, -1352, 11125,   790,
   -1352, 14197, -1352, 14197, -1352,   793, -1352,   856,   796,   970,
   -1352,   481,   966, -1352,   797, -1352, -1352,   799,    98, -1352,
   -1352, 15575,   800,   803, -1352,  4006, -1352, -1352, -1352, -1352,
   -1352, -1352, 13771, 15575, 15339, -1352, 14015,   614, -1352, -1352,
   14015, -1352, 14015, -1352,  6035, 14015, 12749,   798,  6659, 13771,
   -1352,   469, 13771, 14622, -1352, 14530,   847,  2972, -1352, -1352,
   -1352,   613, 13487,    63,   470,   100, -1352, -1352,   854,  3525,
    3877, 15493,   929,   993,   934, 14015, 15575,   817, 11125,   818,
     909,   846,   988,   846,   821, 15493,   823, -1352,  1006,  1067,
   -1352,    37,   824, -1352, -1352,   888, -1352, 14197, -1352,   614,
   -1352, -1352,  8486, -1352, -1352, -1352,  9298, -1352, -1352, -1352,
    8486,   826, 14015, 15575,   893, 15575, 15395,  6035, 12125, -1352,
   -1352, -1352, 14622, 14622,  1010,    43, -1352, -1352, -1352, -1352,
      67,   828,    69, -1352, 13155, -1352, -1352,    72, -1352, -1352,
    2898, -1352,   832, -1352,   949,   388, -1352, 13771, -1352,   613,
   -1352, -1352, -1352, -1352, -1352,  1018, 14015, -1352, 15575, 11125,
     840, -1352,   838,   842,   418, -1352,   909,   846, -1352, -1352,
   -1352,  1308,   845, -1352, 14197, -1352,   910,  8486,  9501, -1352,
   -1352, -1352,  8486, -1352, 15575, 14015, 14015, 12749,  6862,   849,
     852, 14015, 14622, -1352, -1352,   827, 14530, -1352, -1352, -1352,
   -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352,
   -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352,
   -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352,
   -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352,
   -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352,
   -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352,
   -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352,
   -1352, -1352, -1352, -1352, -1352, -1352, -1352, -1352,   381, -1352,
     847, -1352, -1352, -1352, -1352, -1352,    66,   396, -1352,  1024,
      80, 11369,   949,  1025,   388, 14015, 15575,   848, -1352,    70,
   -1352, -1352, -1352, -1352,   844,   418, -1352,   846, -1352, 14197,
   -1352, -1352, -1352,  7065, 15575, 15575,  5005, -1352, -1352, -1352,
   15575, -1352, 13027,    46, -1352, -1352, 14015, 13155, 13155,   987,
   -1352,  2898,  2898,   406, -1352, -1352, -1352, 14015,   967, -1352,
     855,    82, 14015, 11369, 15575, -1352,   975, -1352,  1044,  7268,
    7471, -1352, -1352,   418, -1352,  7674,   857,   977,   951, -1352,
     965,   916, -1352, -1352,   968,   827, -1352, 15575, -1352, -1352,
     905, -1352,  1033, -1352, -1352, -1352, -1352, 15575,  1051, -1352,
   -1352, 15575,   871, -1352,    79,   869, -1352, -1352,  7877, -1352,
     872, -1352, -1352,   876,   912, 11369,   470, -1352, -1352, 14015,
      39, -1352,   996, -1352, -1352, -1352, -1352, 14622,   667, -1352,
     917, 11369,   289, 15575,   880,  1070,   557,    39, -1352,  1003,
   -1352, 14622,   889, -1352,   846,    88, -1352, -1352, -1352, -1352,
   13771, -1352,   884,   887,    84, -1352,   427,   557,   303,   846,
     890, -1352, -1352, -1352, -1352, 13771,  1011,  1076,  1015,   427,
   -1352,  8080,   304,  1079, 14015, -1352, -1352,  8283, -1352,  1017,
    1081,  1028, 14015, 15575, -1352,  1092, 14015, -1352, 15575, 14015,
   15575, 15575
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1352, -1352, -1352,  -489, -1352, -1352, -1352,    -4, -1352, -1352,
   -1352,   618,   382,   379,    48,  1126,  3196, -1352,  2203, -1352,
    -404, -1352,     9, -1352, -1352, -1352, -1352, -1352, -1352, -1352,
   -1352, -1352, -1352,  -447, -1352, -1352,  -168,    31,     7, -1352,
   -1352, -1352,    10, -1352, -1352, -1352, -1352,    11, -1352, -1352,
     739,   752,   750,   969,   316,  -753,   319,   368,  -438, -1352,
     101, -1352, -1352, -1352, -1352, -1352, -1352,  -616,   -39, -1352,
   -1352, -1352, -1352,  -430, -1352,  -759, -1352,  -382, -1352, -1352,
     639, -1352,  -911, -1352, -1352, -1352, -1352, -1352, -1352, -1352,
   -1352, -1352, -1352,  -207, -1352, -1352, -1352, -1352, -1352,  -290,
   -1352,   -64, -1290, -1352, -1351,  -454, -1352,  -154,    21,  -127,
    -441, -1352,  -294, -1352,   -69,   -23,  1113,  -634,  -357, -1352,
   -1352,   -34, -1352, -1352,  3361,    -2,  -163, -1352, -1352, -1352,
   -1352, -1352, -1352,   185,  -732, -1352, -1352, -1352, -1352, -1352,
   -1352, -1352, -1352, -1352, -1352,   780, -1352, -1352,   227, -1352,
     694, -1352, -1352, -1352, -1352, -1352, -1352, -1352,   233, -1352,
     701, -1352, -1352,   446, -1352,   202, -1352, -1352, -1352, -1352,
   -1352, -1352, -1352, -1352,  -835, -1352,  1728,    95,  -340, -1352,
   -1352,   169,  2936,  -638,  3668, -1352, -1352,   280,  -384,  -561,
   -1352, -1352,   340,  -624,   160, -1352, -1352, -1352, -1352, -1352,
     331, -1352, -1352, -1352,  -296,  -767,  -182,  -173,  -136, -1352,
   -1352,    49, -1352, -1352, -1352, -1352,     2,  -133, -1352,   137,
   -1352, -1352, -1352,  -374,   901, -1352, -1352, -1352, -1352, -1352,
     560,   317, -1352, -1352,   897, -1352, -1352, -1352,  -313,   -86,
    -187,  -288, -1352, -1103, -1352,   311, -1352, -1352, -1352,  -183,
    -950
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -842
static const yytype_int16 yytable[] =
{
     118,   299,   375,   793,   332,   302,   303,   226,   404,   545,
     566,   127,   971,   125,   128,   129,   637,   255,   231,   839,
     616,  1126,   235,   397,   597,   131,   646,   422,   662,   640,
     219,  1230,   561,   402,   323,   126,   982,   858,   427,   761,
     966,   306,   657,   238,   329,   985,   248,   428,   882,   883,
     332,  1113,  1342,   158,   788,  1515,   872,   676,   449,   450,
     995,   871,   436,   280,   455,   674,   494,    11,   499,   502,
     948,   392,  1297,   205,   206,  1481,  -268,   720,  1346,   392,
      11,  1430,    11,   280,  -739,   429,   738,   280,   280,  1488,
     274,  1488,   392,  1342,   517,  1201,   738,  1482,   750,   482,
     285,   750,   267,  1496,  1502,   286,   750,   750,   308,   750,
     320,     3,  1552,   364,  1040,  1041,   890,   285,   181,   204,
     204,  -841,  1176,   216,   294,   365,   280,   295,  1314,   222,
    1316,    11,   412,  1119,    11,   496,  1013,   455,   225,  -841,
    1114,   232,  -736,   320,   420,   285,   760,  1497,    11,  -425,
     312,   549,  1539,  1115,   773,   518,  1553,   483,  -589,   309,
    1056,   233,   331,   570,   655,   287,   288,   289,  -743,   234,
     395,  -449,   395,   259,  -737,  -841,   287,   376,  -738,  -774,
    1120,  -740,   409,   288,   289,   399,   399,  1116,  -775,  -777,
     319,  1522,  1523,  -741,  1202,  -742,  1014,   272,  -776,  1203,
     276,    56,    57,    58,   170,   171,   330,  1204,   405,   507,
     268,   288,   289,   789,  1455,   873,  -745,  1043,  -670,  -739,
     808,  -670,   118,   324,  1135,  1232,   118,  1137,   677,   949,
     443,   783,  1238,  1239,  -215,   433,  -201,   641,   609,   438,
     568,  1343,  1344,  1145,  1205,  1206,  1516,  1207,   457,   332,
     413,   437,  1038,   675,  1042,   495,   415,   500,   503,   609,
     855,  1298,   421,  1483,   857,  -268,  -590,  1347,   679,    95,
    1431,  1079,  1080,   960,   739,   158,   508,  -736,  1489,   158,
    1530,   609,  1591,  1157,   740,  -670,   751,   299,   329,   826,
     609,  1208,  1177,   609,  1106,  1255,   204,  1300,   876,  -215,
    -746,   395,   204,  -743,   972,   396,   118,   396,   204,  -737,
     488,   492,   493,  -738,  -774,   560,  -740,  1222,   498,   248,
     400,   400,   280,  -775,  -777,   504,   504,   509,  -741,  1321,
    -742,   131,   514,  -776,   269,   726,  1596,  1609,   523,   767,
     527,   126,   647,   617,   199,   285,   199,   285,   973,   273,
     524,   277,   524,   730,  1503,  1040,  1041,   775,   776,   158,
     731,  1223,   426,   781,   204,   784,   219,   607,   280,   280,
     280,   204,   204,   608,   785,   953,  1218,   278,   204,  1234,
    1597,  1610,   936,   279,   204,   283,   939,   284,   633,   300,
     319,   301,  1160,   311,   634,  1476,   879,  1167,   318,    56,
      57,    58,   170,   171,   330,   319,   613,   322,   319,  1477,
     649,   288,   289,   288,   289,   319,   608,  1039,  1040,  1041,
     810,   325,   659,  1484,   285,   656,  1478,   285,   660,   993,
     333,   974,   315,  1524,   334,   118,   396,   319,  1001,  1201,
    1485,   319,   768,  1486,  1224,   815,   819,   319,   669,   285,
    1525,   335,   514,  1526,   524,  1264,  1017,   769,  1179,   216,
     413,  1037,   810,  1598,  1611,  -447,   784,    95,   401,   367,
     113,  1586,   846,   559,   558,   785,   998,   336,    11,   300,
     337,   847,  1322,   545,   800,   338,  1599,  1573,   158,   519,
     288,   289,   733,   288,   289,   368,   529,   530,   204,   545,
     369,   455,   881,   371,  1053,   601,   204,   745,   852,   853,
     422,    49,   370,   755,   757,   288,   289,  1326,   848,    56,
      57,    58,   170,   171,   330,  1023,  1024,  1244,   398,  1245,
     285,  -744,   635,   723,  -589,   524,   638,   727,  1202,  1173,
    1040,  1041,   267,  1203,  -448,    56,    57,    58,   170,   171,
     330,  1204,   403,   406,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,   389,   280,   377,   378,   379,
     380,   381,   382,   383,   384,   385,   386,   387,   388,   389,
      56,    57,    58,   170,   171,   330,  -841,    95,  1205,  1206,
     408,  1207,  1272,  1273,   968,   525,   288,   289,   292,  1108,
     390,   391,   414,   520,   365,   978,   392,   526,   320,   320,
    1141,   790,   418,    95,   390,   391,  1451,  1452,  1149,   417,
     361,   362,   363,  1325,   364,  1592,  1593,  1154,  -588,   520,
    1588,   526,   520,   526,   526,  1217,   365,  -841,   423,   837,
    -841,   842,  1518,  1519,   856,  1602,   424,   426,    95,   434,
     609,   918,   919,   920,   462,   463,   464,   204,   118,  1168,
     447,   465,   466,   817,   392,   467,   468,  1577,  1578,  1579,
     864,   118,   451,  1188,   314,   316,   317,  -836,   392,   456,
    1193,   452,   458,   131,   866,   459,   461,   545,   501,  1044,
     545,  1045,   510,   126,  1198,   511,   531,   536,   540,   843,
    1458,   844,   915,   916,   917,   918,   919,   920,   547,   204,
     541,   158,   548,   -64,   557,    49,   118,   567,   645,   643,
     667,   862,   938,   648,   158,   436,   941,   942,   654,   997,
     671,   673,   680,  1215,   684,   685,   549,   706,   707,   709,
     721,   131,   725,   710,   718,   204,   729,   204,   724,   728,
    1229,   126,   737,   741,   747,   742,   758,   118,  1236,  1252,
     744,   749,   764,   752,  1102,   766,  1242,   204,   127,   158,
     125,   128,   129,   488,  1260,  1250,   771,   492,   772,   976,
     774,   777,   131,  1566,   778,   977,  1124,   945,   781,  1131,
     779,  -450,   126,   791,   792,   797,   794,   803,   514,   955,
    1566,    56,    57,    58,    59,    60,   330,   806,  1587,  1214,
     158,   280,    66,   372,   804,   807,   811,  1214,   219,   820,
     821,   823,   824,  1006,  1006,   837,   799,   868,   874,  1026,
     849,   875,   877,   204,   870,  1504,   889,   922,   891,   892,
     894,   895,   545,   924,   204,   204,  1310,  1327,   928,   118,
     373,   118,   923,   118,   931,  1332,   358,   359,   360,   361,
     362,   363,  1027,   364,   978,  1338,   934,   944,   947,    95,
     952,   946,   956,   964,   969,   365,   131,   962,   131,   957,
     967,   983,   992,  1000,  1002,  1012,   126,  1055,   126,  1274,
    1061,  1016,  1015,  1036,  1018,  1047,  1029,  1031,  1034,    31,
      32,    33,   158,  1033,   158,  1048,   158,  1049,  1032,  1051,
     691,   216,  1052,   519,  1097,  1104,  1107,  1122,  1127,  1123,
    1129,  1128,  1109,  1136,  1132,  1463,  1134,  1447,  1138,  1143,
    1140,  1151,  1099,  1142,  1144,  1100,  1214,  1535,  1148,  1153,
    1150,  1152,  1214,  1214,  1103,   545,  1156,  1163,  1164,  1166,
    1170,  1180,  1174,  1182,   204,  1185,   118,    70,    71,    72,
      73,    74,  1186,  1187,  1191,  1189,  1192,   127,   693,   125,
     128,   129,  1197,  1216,    77,    78,  1225,  1199,  1240,  1249,
    1228,   131,  1247,  1231,  1443,  1233,  1237,  1243,  1241,    88,
    1246,   126,  1201,  1248,  1251,  1253,  1254,  1269,  1257,  1261,
    1162,  1258,  1286,    93,  1576,  1301,  1305,  1306,   837,   158,
    1201,  1307,   837,  1309,  1324,  1311,  1271,  1312,  1317,  1335,
    1318,  1323,   118,  1333,  1341,  1214,  1439,  1345,  1133,  1296,
    1438,    11,  1445,   118,  1449,  1165,  1459,  1448,  1487,  1492,
    1520,  1450,  1457,  1501,  1528,  1495,  1499,  1468,  1500,    11,
    1469,  1529,  1533,  1534,  1542,  1541,  1505,  1543,   131,  -264,
    1299,  1545,  1546,  1548,  1482,  1549,  1554,  1551,   126,  1161,
    1556,  1201,  1557,  1568,   204,   158,  1558,  1574,  1571,  1575,
    1583,  1589,   514,   862,  1590,   332,   158,  1585,  1603,  1600,
    1604,  1202,  1605,  1612,  1615,  1616,  1203,  1538,    56,    57,
      58,   170,   171,   330,  1204,  1617,  1619,   937,   940,  1202,
      11,  1570,   732,   612,  1203,   204,    56,    57,    58,   170,
     171,   330,  1204,   611,   610,   374,  1212,  1441,   204,   204,
     996,   994,   961,  1584,  1212,  1169,  1259,  1582,   735,  1475,
    1480,  1205,  1206,  1293,  1207,  1606,  1595,   837,  1491,   837,
     202,   202,   228,  1454,   214,  1125,   514,   619,  1096,  1205,
    1206,  1094,  1207,   930,   716,  1117,    95,  1147,  1008,  1054,
    1202,   717,  1155,  1019,   506,  1203,   214,    56,    57,    58,
     170,   171,   330,  1204,    95,   516,  1046,     0,  1315,  1601,
       0,     0,     0,     0,   118,     0,  1607,     0,   248,     0,
       0,     0,   204,  1291,     0,     0,  1319,  -842,  -842,  -842,
    -842,   913,   914,   915,   916,   917,   918,   919,   920,   131,
    1205,  1206,     0,  1207,     0,     0,     0,     0,     0,   126,
       0,     0,     0,     0,     0,   376,     0,     0,     0,     0,
       0,     0,  1295,   837,     0,    95,     0,   158,   118,     0,
       0,     0,   118,  1212,   545,     0,   118,     0,     0,  1212,
    1212,     0,     0,     0,     0,  1330,     0,  1320,     0,     0,
       0,   545,     0,   131,     0,     0,  1493,     0,     0,   545,
    1427,   131,     0,   126,     0,     0,  1434,     0,     0,     0,
       0,   126,     0,   248,     0,     0,     0,     0,     0,     0,
       0,   158,     0,     0,     0,   158,     0,     0,     0,   158,
       0,     0,  1201,     0,     0,     0,     0,     0,     0,     0,
     837,     0,     0,   118,   118,     0,     0,   202,   118,     0,
    1444,     0,     0,   202,   118,     0,     0,  1461,     0,   202,
       0,     0,  1212,     0,     0,     0,     0,     0,   131,     0,
       0,    11,     0,   131,     0,  1490,     0,     0,   126,   131,
       0,     0,     0,   126,     0,     0,     0,   214,   214,   126,
       0,     0,     0,   214,     0,     0,   158,   158,     0,     0,
       0,   158,     0,     0,     0,     0,     0,   158,     0,     0,
       0,     0,  1561,     0,     0,   202,     0,   781,     0,     0,
       0,     0,   202,   202,     0,     0,     0,  1532,     0,   202,
       0,  1202,   781,     0,     0,   202,  1203,     0,    56,    57,
      58,   170,   171,   330,  1204,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   332,     0,   280,     0,     0,
       0,     0,     0,     0,     0,     0,   214,     0,     0,   214,
       0,     0,     0,     0,     0,   837,     0,     0,     0,   118,
       0,  1205,  1206,     0,  1207,     0,     0,     0,  1510,     0,
       0,     0,     0,  1427,  1427,     0,     0,  1434,  1434,     0,
       0,     0,     0,     0,   131,     0,    95,     0,     0,   280,
     214,     0,     0,     0,   126,   118,   118,     0,     0,     0,
       0,   118,     0,     0,     0,     0,     0,     0,  1456,     0,
       0,     0,   158,     0,     0,     0,     0,     0,     0,     0,
     131,   131,     0,     0,     0,     0,   131,     0,     0,   202,
     126,   126,     0,     0,   118,     0,   126,   202,     0,     0,
       0,  1560,     0,     0,     0,     0,     0,     0,   158,   158,
       0,     0,     0,     0,   158,     0,     0,  1572,     0,   131,
       0,     0,     0,     0,     0,     0,     0,  1562,     0,   126,
       0,     0,     0,     0,     0,     0,     0,   214,     0,     0,
       0,     0,   697,     0,     0,     0,     0,   158,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   118,     0,     0,
       0,     0,     0,   118,   406,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,   389,     0,     0,     0,
       0,     0,   131,     0,     0,     0,     0,   697,   131,     0,
       0,     0,   126,     0,     0,     0,     0,     0,   126,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     158,   390,   391,    34,     0,   199,   158,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   214,   214,     0,     0,
       0,     0,   214,   339,   340,   341,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   202,     0,
     342,     0,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,     0,   364,   392,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   365,     0,     0,     0,
       0,    82,    83,     0,    84,    85,    86,     0,     0,     0,
     202,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   203,   203,     0,     0,   215,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,     0,    34,     0,   199,     0,   202,   632,   202,   113,
       0,     0,     0,     0,     0,     0,     0,   743,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   202,   697,
       0,     0,     0,     0,   339,   340,   341,     0,     0,     0,
     214,   214,   697,   697,   697,   697,   697,     0,   605,     0,
       0,   342,   697,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,     0,   364,     0,   214,     0,     0,
      82,    83,     0,    84,    85,    86,     0,   365,     0,     0,
       0,     0,     0,     0,   202,     0,  1111,     0,     0,     0,
       0,     0,     0,   214,     0,   202,   202,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     214,     0,     0,     0,     0,     0,   606,     0,   113,     0,
       0,     0,     0,   214,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   214,     0,     0,     0,     0,     0,
     697,     0,     0,   214,   339,   340,   341,     0,     0,     0,
       0,     0,     0,     0,     0,   203,     0,     0,     0,     0,
       0,   342,   214,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,     0,   364,     0,     0,     0,     0,
      34,     0,   199,     0,     0,     0,     0,   365,     0,     0,
       0,     0,     0,     0,     0,   202,     0,     0,     0,     0,
       0,     0,     0,     0,   759,     0,     0,   203,   214,     0,
     214,     0,     0,     0,   203,   203,     0,     0,     0,     0,
       0,   203,     0,     0,     0,   697,     0,   203,     0,     0,
       0,   697,   697,   697,   697,   697,   697,   697,   697,   697,
     697,   697,   697,   697,   697,   697,   697,   697,   697,   697,
     697,   697,   697,   697,   697,   697,   697,     0,    82,    83,
       0,    84,    85,    86,   406,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,   389,     0,     0,     0,
     697,     0,     0,   214,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,     0,     0,
       0,     0,   215,     0,   606,   214,   113,   214,     0,     0,
       0,   390,   391,     0,     0,   202,     0,     0,     0,     0,
       0,     0,     0,     0,   787,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   203,   364,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   365,     0,   202,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   202,
     202,     0,   697,     0,     0,   392,     0,     0,     0,     0,
       0,     0,     0,   214,     0,     0,   697,     0,   697,   339,
     340,   341,     0,     0,   701,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   697,     0,   342,     0,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,     0,
     364,     0,     0,     0,     0,     0,     0,     0,     0,   701,
     214,     0,   365,   202,     0,     0,     0,     0,   339,   340,
     341,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   249,   697,   342,     0,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,     0,   364,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     203,   365,  -842,  -842,  -842,  -842,   356,   357,   358,   359,
     360,   361,   362,   363,     0,   364,     0,     0,   214,     0,
       0,     0,   697,     0,     0,     0,   697,   365,   697,     0,
       0,   697,     0,     0,     0,   214,     0,     0,   214,   214,
       0,   214,     0,     0,     0,     0,     0,     0,   214,     0,
       0,     0,   203,     0,     0,     0,     0,     0,     0,     0,
       0,   697,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   822,
     364,     0,     0,     0,     0,     0,     0,     0,   203,     0,
     203,     0,   365,     0,     0,     0,     0,     0,   697,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   214,   214,
     203,   701,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   701,   701,   701,   701,   701,     0,
       0,     0,     0,   214,   701,     0,     0,     0,   825,     0,
       0,     0,   697,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   249,   249,     0,     0,     0,   933,
     249,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   697,   697,     0,     0,     0,   203,   697,   214,     0,
       0,     0,   214,     0,     0,   951,     0,   203,   203,   339,
     340,   341,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   951,     0,     0,     0,   342,     0,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,     0,
     364,     0,   701,   249,     0,   984,   249,     0,   339,   340,
     341,     0,   365,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   215,   342,     0,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,     0,   364,
       0,   697,     0,     0,     0,     0,     0,     0,     0,    34,
       0,   365,     0,     0,     0,     0,     0,   203,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   214,     0,
       0,     0,   697,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   697,     0,     0,     0,   701,   697,     0,
       0,     0,     0,   701,   701,   701,   701,   701,   701,   701,
     701,   701,   701,   701,   701,   701,   701,   701,   701,   701,
     701,   701,   701,   701,   701,   701,   701,   701,   701,     0,
       0,     0,     0,   292,   249,     0,     0,    82,    83,   699,
      84,    85,    86,     0,     0,     0,     0,     0,     0,   943,
       0,     0,   701,     0,     0,   697,     0,     0,     0,     0,
       0,     0,     0,   214,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   214,     0,     0,
       0,     0,     0,   293,   699,     0,   214,   203,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   991,     0,
       0,   214,     0,     0,     0,     0,     0,     0,     0,     0,
     697,     0,     0,     0,     0,     0,     0,     0,   697,     0,
       0,     0,   697,   249,   249,   697,     0,     0,   203,   249,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   203,   203,     0,   701,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   701,     0,
     701,   339,   340,   341,     0,     0,     0,   859,     0,     0,
       0,     0,     0,     0,     0,     0,   701,     0,   342,     0,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,     0,   364,     0,     0,     0,     0,     0,     0,    34,
       0,   199,  1200,     0,   365,   203,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,     0,   364,   701,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   365,     0,   200,
       0,     0,     0,     0,     0,     0,   699,     0,     0,     0,
       0,   860,     0,     0,     0,     0,     0,   249,   249,   699,
     699,   699,   699,   699,     0,     0,     0,     0,     0,   699,
     174,     0,     0,    79,     0,    81,     0,    82,    83,     0,
      84,    85,    86,     0,   701,     0,     0,     0,   701,     0,
     701,     0,     0,   701,     0,     0,     0,     0,     0,     0,
       0,  1276,     0,  1285,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,     0,     0,     0,
     201,     0,     0,   701,     0,   113,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   999,     0,    34,     0,     0,     0,     0,     0,     0,
     249,     0,     0,     0,     0,   339,   340,   341,     0,     0,
     701,   249,     0,     0,     0,     0,     0,   699,     0,     0,
    1339,  1340,   342,     0,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,     0,   364,     0,     0,     0,
       0,     0,     0,     0,   701,     0,     0,     0,   365,     0,
       0,     0,     0,     0,     0,     0,     0,    34,     0,  1432,
       0,    82,    83,  1433,    84,    85,    86,     0,     0,     0,
       0,     0,     0,   701,   701,     0,     0,     0,     0,   701,
    1471,     0,     0,     0,  1285,   249,     0,   249,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,  1289,   699,     0,  1290,     0,     0,     0,   699,   699,
     699,   699,   699,   699,   699,   699,   699,   699,   699,   699,
     699,   699,   699,   699,   699,   699,   699,   699,   699,   699,
     699,   699,   699,   699,     0,    82,    83,     0,    84,    85,
      86,     0,     0,     0,     0,     0,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   699,     0,     0,
     249,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,     0,   614,     0,  1290,     0,
       0,     0,   249,   701,   249,  1022,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,     0,    26,    27,    28,
      29,     0,     0,     0,   701,    31,    32,    33,    34,    35,
      36,     0,     0,     0,     0,   701,    38,     0,     0,     0,
     701,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,   699,
       0,     0,     0,    56,    57,    58,   170,   171,   172,     0,
     249,    63,    64,   699,     0,   699,     0,     0,     0,     0,
     173,    69,     0,    70,    71,    72,    73,    74,     0,     0,
       0,   699,     0,     0,    75,     0,     0,   701,     0,   174,
      77,    78,    79,   615,    81,  1569,    82,    83,     0,    84,
      85,    86,     0,     0,     0,    88,     0,     0,    89,  1276,
       0,     0,     0,     0,    90,     0,     0,     0,     0,    93,
      94,    95,   253,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,   110,
       0,   699,   701,     0,   113,   114,     0,   115,   116,     0,
     701,     0,     0,     0,   701,     0,     0,   701,   903,   904,
     905,   906,   907,   908,   909,   910,   911,   912,   913,   914,
     915,   916,   917,   918,   919,   920,   178,   180,     0,   182,
     183,   184,     0,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   249,     0,   208,   211,   699,
       0,     0,   702,   699,     0,   699,     0,     0,   699,     0,
     229,     0,   249,     0,     0,   249,     0,   237,     0,   240,
       0,     0,   256,     0,   261,   249,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   699,     0,
       0,     0,     0,     0,     0,     0,     0,   702,     0,     0,
       0,   297,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   304,     0,   339,   340,   341,
       0,     0,     0,     0,     0,   699,     0,     0,     0,     0,
       0,   307,     0,     0,   342,     0,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   898,   364,     0,
     249,     0,     0,     0,     0,     0,     0,     0,     0,   699,
     365,     0,   899,     0,   900,   901,   902,   903,   904,   905,
     906,   907,   908,   909,   910,   911,   912,   913,   914,   915,
     916,   917,   918,   919,   920,   339,   340,   341,   699,   699,
       0,     0,   407,     0,   699,     0,     0,     0,     0,     0,
       0,     0,   342,     0,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,     0,   364,     0,     0,     0,
       0,     0,     0,   431,     0,     0,   431,     0,   365,     0,
       0,     0,     0,   229,   442,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   702,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   702,   702,   702,   702,   702,     0,     0,     0,
       0,     0,   702,     0,     0,     0,     0,     0,     0,   307,
       0,     0,     0,     0,     0,   208,     0,  1227,   699,   522,
       0,     0,   698,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   556,   364,     0,  1511,     0,     0,     0,   699,
       0,     0,     0,   565,     0,   365,     0,     0,     0,     0,
     699,     0,     0,     0,     0,   699,     0,   698,     0,     0,
     571,   572,   573,   575,   576,   577,   578,   579,   580,   581,
     582,   583,   584,   585,   586,   587,   588,   589,   590,   591,
     592,   593,   594,   595,   596,  1303,   598,     0,   599,   599,
     702,   602,     0,     0,     0,     0,     0,     0,   618,   620,
     621,   622,   623,   624,   625,   626,   627,   628,   629,   630,
     631,     0,   699,     0,     0,     0,   599,   636,     0,   565,
     599,   639,     0,     0,     0,     0,     0,   618,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   651,     0,   653,
       0,     0,     0,   249,     0,   565,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   665,     0,   666,   249,     0,
       0,     0,     0,     0,     0,     0,     0,   699,     0,     0,
       0,     0,     0,     0,     0,   699,     0,     0,     0,   699,
       0,     0,   699,     0,   708,   702,     0,   711,   714,   715,
       0,   702,   702,   702,   702,   702,   702,   702,   702,   702,
     702,   702,   702,   702,   702,   702,   702,   702,   702,   702,
     702,   702,   702,   702,   702,   702,   702,     0,   734,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   698,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     702,     0,   698,   698,   698,   698,   698,   339,   340,   341,
       0,     0,   698,   763,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   342,     0,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   341,   364,     0,
       0,     0,     0,     0,     0,   801,     0,     0,     0,     0,
     365,     0,   342,     0,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,     0,   364,   809,     0,     0,
       0,     0,   702,     0,     0,     0,   297,     0,   365,     0,
       0,     0,     0,     0,     0,     0,   702,     0,   702,     0,
     698,     0,     0,   818,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   702,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   339,   340,   341,   850,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   229,   342,  1176,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,     0,   364,     0,     0,
       0,     0,     0,     0,   702,     0,     0,   921,     0,   365,
       0,     0,     0,     0,     0,     0,     0,  1304,     0,     0,
       0,     0,     0,     0,     0,   698,     0,     0,     0,     0,
       0,   698,   698,   698,   698,   698,   698,   698,   698,   698,
     698,   698,   698,   698,   698,   698,   698,   698,   698,   698,
     698,   698,   698,   698,   698,   698,   698,     0,     0,   958,
       0,     0,   702,     0,   704,     0,   702,     0,   702,     0,
       0,   702,   965,     0,     0,     0,     0,     0,     0,     0,
     698,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   980,     0,     0,     0,     0,     0,     0,
       0,   702,     0,   988,     0,     0,   989,     0,   990,   736,
       0,     0,   565,     0,     0,     0,     0,     0,     0,     0,
       0,   565,     0,     0,     0,     0,     0,     0,   339,   340,
     341,     0,     0,     0,     0,     0,     0,     0,   702,     0,
       0,     0,     0,     0,  1177,   342,  1021,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,     0,   364,
       0,     0,   698,     0,     0,     0,     0,     0,     0,     0,
       0,   365,   702,     0,     0,     0,   698,     0,   698,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   698,     0,     0,     0,     0,     0,
       0,   702,   702,     0,   339,   340,   341,   702,     0,     0,
       0,  1474,     0,  1091,  1092,  1093,     0,     0,     0,   711,
    1095,   342,     0,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,     0,   364,     0,  1110,    27,    28,
       0,     0,     0,     0,   698,     0,     0,   365,    34,     0,
     199,     0,     0,     0,     0,     0,  1130,     0,     0,     0,
       0,   863,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   565,     0,     0,   884,   885,     0,     0,
       0,   565,     0,  1110,   893,     0,     0,     0,   200,     0,
       0,     0,     0,     0,     0,     0,   366,     0,     0,     0,
       0,   702,   698,     0,     0,     0,   698,     0,   698,     0,
       0,   698,   229,     0,     0,     0,     0,     0,     0,   174,
       0,  1175,    79,     0,    81,     0,    82,    83,     0,    84,
      85,    86,   702,    34,     0,   199,     0,     0,    89,     0,
       0,   698,     0,   702,     0,     0,     0,     0,   702,     0,
       0,     0,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,     0,     0,     0,   411,
       0,  1547,     0,   200,   113,     0,     0,     0,   698,     0,
       0,   642,   981,     0,     0,   513,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1219,     0,     0,
       0,  1220,     0,  1221,   174,   702,     0,    79,     0,    81,
       0,    82,    83,   565,    84,    85,    86,     0,     0,     0,
    1235,   565,   698,     0,     0,     0,     0,     0,     0,   565,
       0,     0,     0,     0,     0,     0,     0,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   698,   698,     0,   201,     0,     0,   698,     0,   113,
     702,     0,     0,     0,     0,     0,     0,     0,   702,     0,
       0,     0,   702,     0,     0,   702,     0,  1268,     0,     0,
       0,     0,     0,  1062,  1065,  1066,  1067,  1069,  1070,  1071,
    1072,  1073,  1074,  1075,  1076,  1077,  1078,     0,     0,  1081,
    1082,  1083,  1084,  1085,  1086,  1087,  1088,  1089,  1090,   565,
       0,     0,     0,     0,     0,     0,     0,     0,   339,   340,
     341,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1101,     0,     0,   342,     0,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,     0,   364,
       0,   698,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   365,  -842,  -842,  -842,  -842,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,     0,   364,
     565,     0,   698,     0,     0,     0,     0,     0,     0,     0,
       0,   365,     0,   698,     0,     0,     0,     0,   698,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1466,     0,
       0,     0,     0,     0,  1171,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1183,     0,
    1184,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,  1194,     0,     0,     0,
       0,     0,     0,     0,     0,   698,   905,   906,   907,   908,
     909,   910,   911,   912,   913,   914,   915,   916,   917,   918,
     919,   920,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,     0,    26,    27,    28,    29,   446,     0,     0,     0,
      31,    32,    33,    34,    35,    36,  1226,     0,     0,     0,
     698,    38,     0,     0,     0,     0,     0,     0,   698,     0,
       0,     0,   698,     0,     0,   698,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,    56,    57,
      58,   170,   171,   172,     0,     0,    63,    64,     0,     0,
       0,     0,     0,     0,     0,   173,    69,     0,    70,    71,
      72,    73,    74,     0,  1263,     0,     0,     0,  1265,    75,
    1266,     0,     0,  1267,   174,    77,    78,    79,     0,    81,
       0,    82,    83,     0,    84,    85,    86,     0,     0,     0,
      88,     0,     0,    89,     0,     0,     0,     0,     0,    90,
       0,     0,     0,  1308,    93,    94,    95,   253,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,   110,     0,   254,     0,     0,   113,
     114,     0,   115,   116,     0,     0,   339,   340,   341,     0,
    1334,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   342,     0,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,     0,   364,     0,     0,
       0,     0,     0,     0,  1446,     0,     0,     0,   342,   365,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,     0,   364,  1464,  1465,     0,     0,     0,     0,  1470,
       0,     0,     0,     0,   365,   339,   340,   341,     0,     0,
       0,     0,     0,     5,     6,     7,     8,     9,     0,     0,
       0,     0,   342,    10,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,     0,   364,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,   365,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,    36,     0,    37,
       0,     0,     0,    38,    39,    40,    41,     0,    42,     0,
      43,     0,    44,  1494,   448,    45,     0,     0,     0,    46,
      47,    48,    49,    50,    51,    52,     0,    53,    54,    55,
      56,    57,    58,    59,    60,    61,     0,    62,    63,    64,
      65,    66,    67,     0,  1517,     0,     0,    68,    69,    34,
      70,    71,    72,    73,    74,  1527,     0,     0,     0,     0,
    1531,    75,     0,     0,     0,     0,    76,    77,    78,    79,
      80,    81,     0,    82,    83,     0,    84,    85,    86,    87,
       0,     0,    88,     0,     0,    89,     0,     0,     0,     0,
    1507,    90,    91,     0,    92,     0,    93,    94,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,   110,  1563,   111,   112,
     959,   113,   114,     0,   115,   116,     0,    82,    83,     0,
      84,    85,    86,     0,     0,     0,     0,     0,     0,     0,
       0,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,     0,     0,     0,
     567,     0,  1613,     0,     0,     0,     0,     0,     0,     0,
    1618,    11,    12,    13,  1620,     0,     0,  1621,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,     0,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,    36,     0,    37,     0,     0,
       0,    38,    39,    40,    41,     0,    42,     0,    43,     0,
      44,     0,     0,    45,     0,     0,     0,    46,    47,    48,
      49,    50,    51,    52,     0,    53,    54,    55,    56,    57,
      58,    59,    60,    61,     0,    62,    63,    64,    65,    66,
      67,     0,     0,     0,     0,    68,    69,     0,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,    75,
       0,     0,     0,     0,    76,    77,    78,    79,    80,    81,
       0,    82,    83,     0,    84,    85,    86,    87,     0,     0,
      88,     0,     0,    89,     0,     0,     0,     0,     0,    90,
      91,     0,    92,     0,    93,    94,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,   110,     0,   111,   112,  1112,   113,
     114,     0,   115,   116,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,     0,   364,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,   365,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,     0,    26,    27,    28,    29,    30,
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
       8,     9,     0,     0,     0,     0,     0,    10,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,     0,   364,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,   365,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,     0,    26,    27,
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
     110,     0,   111,   112,   550,   113,   114,     0,   115,   116,
       5,     6,     7,     8,     9,     0,     0,     0,     0,   899,
      10,   900,   901,   902,   903,   904,   905,   906,   907,   908,
     909,   910,   911,   912,   913,   914,   915,   916,   917,   918,
     919,   920,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
       0,    26,    27,    28,    29,    30,     0,     0,     0,    31,
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
     109,     0,     0,   110,     0,   111,   112,   932,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   900,   901,   902,   903,   904,   905,
     906,   907,   908,   909,   910,   911,   912,   913,   914,   915,
     916,   917,   918,   919,   920,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,    29,    30,     0,
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
    1028,   113,   114,     0,   115,   116,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   901,   902,   903,
     904,   905,   906,   907,   908,   909,   910,   911,   912,   913,
     914,   915,   916,   917,   918,   919,   920,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,     0,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,    37,     0,     0,     0,    38,    39,    40,    41,
    1030,    42,     0,    43,     0,    44,     0,     0,    45,     0,
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
       0,   111,   112,     0,   113,   114,     0,   115,   116,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     902,   903,   904,   905,   906,   907,   908,   909,   910,   911,
     912,   913,   914,   915,   916,   917,   918,   919,   920,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,     0,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,    36,     0,    37,     0,     0,     0,    38,
      39,    40,    41,     0,    42,     0,    43,     0,    44,  1172,
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
       0,     0,    10,   904,   905,   906,   907,   908,   909,   910,
     911,   912,   913,   914,   915,   916,   917,   918,   919,   920,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,     0,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,    37,     0,
       0,     0,    38,    39,    40,    41,     0,    42,     0,    43,
       0,    44,     0,     0,    45,     0,     0,     0,    46,    47,
      48,    49,     0,    51,    52,     0,    53,     0,    55,    56,
      57,    58,    59,    60,    61,     0,    62,    63,    64,     0,
      66,    67,     0,     0,     0,     0,    68,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,   174,    77,    78,    79,    80,
      81,     0,    82,    83,     0,    84,    85,    86,    87,     0,
       0,    88,     0,     0,    89,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,    93,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,   110,     0,   111,   112,  1270,
     113,   114,     0,   115,   116,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,  -842,  -842,  -842,  -842,
     909,   910,   911,   912,   913,   914,   915,   916,   917,   918,
     919,   920,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,     0,    26,    27,    28,    29,
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
     111,   112,  1467,   113,   114,     0,   115,   116,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,     0,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,    36,     0,    37,     0,     0,     0,    38,    39,
      40,    41,     0,    42,     0,    43,  1506,    44,     0,     0,
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
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,     0,    26,    27,    28,    29,    30,     0,     0,     0,
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
     108,   109,     0,     0,   110,     0,   111,   112,  1536,   113,
     114,     0,   115,   116,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,     0,    26,    27,    28,    29,    30,
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
     112,  1537,   113,   114,     0,   115,   116,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,     0,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,    36,     0,    37,     0,     0,     0,    38,    39,    40,
      41,     0,    42,  1540,    43,     0,    44,     0,     0,    45,
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
     110,     0,   111,   112,     0,   113,   114,     0,   115,   116,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
       0,    26,    27,    28,    29,    30,     0,     0,     0,    31,
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
     109,     0,     0,   110,     0,   111,   112,  1555,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,    29,    30,     0,
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
    1608,   113,   114,     0,   115,   116,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,     0,    26,    27,    28,
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
       0,   111,   112,  1614,   113,   114,     0,   115,   116,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,     0,
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
       0,     0,     0,     0,     0,     0,     0,   432,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,     0,    26,    27,    28,    29,    30,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     668,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,     0,    26,    27,    28,    29,
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
       0,     0,     0,   865,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,     0,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,    36,     0,    37,     0,     0,     0,    38,    39,
      40,    41,     0,    42,     0,    43,     0,    44,     0,     0,
      45,     0,     0,     0,    46,    47,    48,    49,     0,    51,
      52,     0,    53,     0,    55,    56,    57,    58,   170,   171,
      61,     0,    62,    63,    64,     0,     0,     0,     0,     0,
       0,     0,    68,    69,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
       0,   174,    77,    78,    79,    80,    81,     0,    82,    83,
       0,    84,    85,    86,     0,     0,     0,    88,     0,     0,
      89,     0,     0,     0,     0,     0,    90,     0,     0,     0,
       0,    93,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,   110,     0,   111,   112,     0,   113,   114,     0,   115,
     116,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1329,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,     0,    26,    27,    28,    29,    30,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1460,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,     0,    26,    27,    28,    29,    30,
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
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,     0,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,    36,     0,    37,     0,     0,     0,    38,    39,    40,
      41,     0,    42,     0,    43,     0,    44,     0,     0,    45,
       0,     0,     0,    46,    47,    48,    49,     0,    51,    52,
       0,    53,     0,    55,    56,    57,    58,   170,   171,    61,
       0,    62,    63,    64,     0,     0,     0,     0,     0,     0,
       0,    68,    69,     0,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,    75,     0,     0,     0,     0,
     174,    77,    78,    79,    80,    81,     0,    82,    83,     0,
      84,    85,    86,     0,     0,     0,    88,     0,     0,    89,
       0,     0,     0,     0,     0,    90,     0,     0,     0,     0,
      93,    94,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
     110,     0,   111,   112,     0,   113,   114,     0,   115,   116,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
       0,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,    35,    36,     0,     0,     0,     0,     0,
      38,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    56,    57,    58,
     170,   171,   172,    34,     0,    63,    64,     0,     0,     0,
       0,     0,     0,     0,   173,    69,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,   174,    77,    78,    79,   615,    81,     0,
      82,    83,     0,    84,    85,    86,     0,     0,     0,    88,
       0,     0,    89,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,    82,    83,   110,    84,    85,    86,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,     0,     0,   207,   799,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,    29,     0,     0,
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
     106,   107,   108,   109,     0,     0,   110,     0,   339,   340,
     341,   113,   114,     0,   115,   116,     5,     6,     7,     8,
       9,     0,     0,     0,     0,   342,    10,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,     0,   364,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,   365,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,     0,    26,    27,    28,
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
     929,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,   110,
       0,   236,   340,   341,   113,   114,     0,   115,   116,     5,
       6,     7,     8,     9,     0,     0,     0,     0,   342,    10,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,     0,   364,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,   365,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,     0,
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
       0,     0,   110,     0,   239,   897,   898,   113,   114,     0,
     115,   116,     5,     6,     7,     8,     9,     0,     0,     0,
       0,   899,    10,   900,   901,   902,   903,   904,   905,   906,
     907,   908,   909,   910,   911,   912,   913,   914,   915,   916,
     917,   918,   919,   920,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,     0,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,     0,     0,
       0,     0,    38,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   296,     0,
       0,    49,     0,     0,     0,     0,     0,     0,     0,    56,
      57,    58,   170,   171,   172,     0,     0,    63,    64,     0,
       0,     0,     0,     0,     0,     0,   173,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,   174,    77,    78,    79,     0,
      81,     0,    82,    83,     0,    84,    85,    86,     0,     0,
       0,    88,     0,     0,    89,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,    93,     0,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,   110,     0,   339,   340,   341,
     113,   114,     0,   115,   116,     5,     6,     7,     8,     9,
       0,     0,     0,     0,   342,    10,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,     0,   364,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
     365,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,     0,    26,    27,    28,    29,
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
       0,     0,     0,    90,     0,   925,   926,     0,    93,     0,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,   110,   430,
       0,     0,     0,   113,   114,     0,   115,   116,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   562,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,     0,    26,
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
       0,   110,     0,     0,     0,     0,   113,   114,     0,   115,
     116,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   574,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,     0,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,    36,     0,     0,     0,     0,
       0,    38,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
     107,   108,     0,     0,   614,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,     0,    26,    27,    28,    29,     0,
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
       0,     0,     0,     0,     0,     0,     0,   650,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,     0,    26,    27,
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
     652,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
       0,    26,    27,    28,    29,     0,     0,     0,     0,    31,
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
     109,     0,     0,   110,     0,   339,   340,   341,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,   342,    10,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,     0,   364,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,   365,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,    34,    35,    36,     0,     0,
       0,     0,     0,    38,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    49,     0,     0,     0,     0,     0,     0,     0,
      56,    57,    58,   170,   171,   172,     0,     0,    63,    64,
       0,     0,     0,     0,     0,     0,     0,   173,    69,     0,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,    75,     0,     0,     0,     0,   174,    77,    78,    79,
       0,    81,     0,    82,    83,     0,    84,    85,    86,     0,
       0,     0,    88,     0,     0,    89,     0,     0,     0,  1337,
       0,    90,     0,     0,     0,     0,    93,     0,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,   110,     0,     0,   664,
       0,   113,   114,     0,   115,   116,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   979,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,     0,    26,    27,    28,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1020,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,     0,
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
       0,     0,   110,     0,   339,   340,   341,   113,   114,     0,
     115,   116,     5,     6,     7,     8,     9,     0,     0,     0,
       0,   342,    10,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,     0,   364,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,   365,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,     0,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,     0,     0,
       0,     0,    38,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    49,     0,     0,     0,     0,     0,     0,     0,    56,
      57,    58,   170,   171,   172,     0,     0,    63,    64,     0,
       0,     0,     0,     0,     0,     0,   173,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,   174,    77,    78,    79,     0,
      81,     0,    82,    83,     0,    84,    85,    86,     0,     0,
       0,    88,     0,     0,    89,     0,     0,  1196,     0,     0,
      90,     0,     0,     0,     0,    93,     0,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,   110,     0,   339,   340,   341,
     113,   114,     0,   115,   116,     5,     6,     7,     8,     9,
       0,     0,     0,     0,   342,    10,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,     0,   364,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
     365,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,     0,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,   521,    36,
       0,   672,     0,     0,     0,    38,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,    56,    57,    58,   170,   171,   172,     0,     0,
      63,    64,     0,     0,     0,     0,     0,     0,     0,   173,
      69,     0,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,    75,     0,     0,     0,     0,   174,    77,
      78,    79,    34,    81,     0,    82,    83,     0,    84,    85,
      86,     0,     0,     0,    88,     0,     0,    89,     0,     0,
       0,     0,     0,    90,     0,     0,     0,     0,    93,     0,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,   110,     0,
       0,     0,     0,   113,   114,     0,   115,   116,  1348,  1349,
    1350,  1351,  1352,     0,     0,  1353,  1354,  1355,  1356,     0,
       0,     0,     0,   174,     0,     0,    79,     0,     0,     0,
      82,    83,     0,    84,    85,    86,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1357,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
    1358,  1359,  1360,  1361,  1362,  1363,  1364,  1509,     0,     0,
      34,     0,     0,     0,     0,     0,     0,     0,     0,  1365,
    1366,  1367,  1368,  1369,  1370,  1371,  1372,  1373,  1374,  1375,
    1376,  1377,  1378,  1379,  1380,  1381,  1382,  1383,  1384,  1385,
    1386,  1387,  1388,  1389,  1390,  1391,  1392,  1393,  1394,  1395,
    1396,  1397,  1398,  1399,  1400,  1401,  1402,  1403,  1404,  1405,
       0,     0,  1406,  1407,     0,  1408,  1409,  1410,  1411,  1412,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1413,  1414,  1415,     0,  1416,     0,     0,    82,    83,
       0,    84,    85,    86,  1417,     0,  1418,  1419,     0,  1420,
       0,     0,     0,     0,     0,     0,  1421,  1422,     0,  1423,
       0,  1424,  1425,  1426,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   339,   340,
     341,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   342,     0,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,     0,   364,
     339,   340,   341,     0,     0,     0,     0,     0,     0,     0,
       0,   365,     0,     0,     0,     0,     0,   342,     0,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
       0,   364,   339,   340,   341,   816,     0,     0,     0,     0,
       0,     0,     0,   365,    34,     0,   199,     0,     0,   342,
       0,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,     0,   364,   896,   897,   898,     0,     0,     0,
       0,     0,     0,     0,   200,   365,     0,     0,     0,     0,
       0,   899,     0,   900,   901,   902,   903,   904,   905,   906,
     907,   908,   909,   910,   911,   912,   913,   914,   915,   916,
     917,   918,   919,   920,   241,   174,     0,     0,    79,     0,
      81,     0,    82,    83,     0,    84,    85,    86,     0,     0,
       0,     0,     0,     0,     0,     0,   460,     0,     0,     0,
     242,     0,     0,     0,     0,     0,     0,     0,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,    34,     0,     0,   201,     0,     0,     0,     0,
     113,     0,     0,     0,     0,     0,     0,     0,   484,   241,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  -306,
       0,     0,     0,     0,     0,     0,     0,    56,    57,    58,
     170,   171,   330,     0,     0,   242,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   243,   244,     0,     0,   661,
       0,     0,     0,     0,     0,     0,     0,    34,     0,     0,
       0,     0,     0,   174,     0,     0,    79,     0,   245,     0,
      82,    83,     0,    84,    85,    86,     0,     0,     0,     0,
       0,     0,     0,     0,   453,     0,   241,     0,   246,     0,
       0,  1059,     0,     0,     0,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     243,   244,   242,   247,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   174,     0,
       0,    79,     0,   245,    34,    82,    83,     0,    84,    85,
      86,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   241,     0,   246,     0,     0,     0,     0,     0,     0,
       0,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,     0,     0,   242,   247,     0,
       0,     0,     0,     0,     0,     0,     0,   243,   244,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    34,
       0,     0,     0,     0,     0,   174,     0,     0,    79,     0,
     245,     0,    82,    83,     0,    84,    85,    86,     0,   878,
       0,     0,     0,     0,     0,     0,     0,     0,   241,     0,
     246,     0,     0,     0,     0,     0,     0,     0,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   243,   244,   242,   247,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     174,     0,     0,    79,     0,   245,    34,    82,    83,     0,
      84,    85,    86,     0,  1181,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   246,     0,     0,     0,     0,
       0,     0,     0,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,     0,     0,     0,
     247,     0,     0,     0,  1068,     0,     0,     0,     0,   243,
     244,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     686,   687,     0,     0,     0,     0,   688,   174,   689,     0,
      79,     0,   245,     0,    82,    83,     0,    84,    85,    86,
     690,     0,     0,     0,     0,     0,     0,     0,    31,    32,
      33,    34,   246,     0,     0,     0,     0,     0,     0,   691,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,     0,     0,     0,   247,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   692,     0,    70,    71,    72,    73,
      74,     0,    34,     0,   199,     0,     0,   693,     0,     0,
       0,     0,   174,    77,    78,    79,     0,   694,     0,    82,
      83,     0,    84,    85,    86,     0,     0,     0,    88,     0,
       0,     0,     0,     0,     0,     0,     0,   695,     0,     0,
       0,     0,    93,     0,     0,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   686,
     687,     0,   696,     0,     0,   688,     0,   689,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   690,
      82,    83,     0,    84,    85,    86,     0,    31,    32,    33,
      34,     0,     0,     0,     0,     0,     0,     0,   691,     0,
       0,     0,     0,     0,     0,     0,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
       0,     0,     0,     0,     0,     0,   658,     0,   113,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   692,     0,    70,    71,    72,    73,    74,
     686,   687,     0,     0,     0,     0,   693,     0,     0,     0,
       0,   174,    77,    78,    79,     0,   694,     0,    82,    83,
     690,    84,    85,    86,     0,     0,     0,    88,    31,    32,
      33,    34,     0,     0,     0,     0,   695,     0,     0,   691,
       0,    93,     0,     0,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,     0,     0,
       0,   696,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   692,     0,    70,    71,    72,    73,
      74,   827,   828,     0,     0,     0,     0,   693,     0,     0,
       0,     0,   174,    77,    78,    79,     0,   694,     0,    82,
      83,   829,    84,    85,    86,     0,     0,     0,    88,   830,
     831,   832,    34,     0,     0,     0,     0,   695,     0,     0,
     833,     0,    93,     0,     0,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    34,
       0,   199,     0,     0,     0,   834,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   835,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      82,    83,     0,    84,    85,    86,     0,     0,     0,   200,
       0,     0,     0,     0,     0,     0,     0,     0,   836,     0,
      34,     0,   199,     0,     0,     0,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     174,     0,     0,    79,     0,    81,     0,    82,    83,     0,
      84,    85,    86,     0,     0,     0,     0,     0,     0,     0,
     200,     0,     0,     0,     0,     0,     0,     0,     0,    34,
       0,   199,   954,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,     0,     0,     0,
     201,   174,     0,   497,    79,   113,    81,     0,    82,    83,
       0,    84,    85,    86,     0,     0,     0,     0,     0,   200,
       0,     0,     0,     0,     0,     0,     0,     0,    34,     0,
     199,     0,     0,     0,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,     0,     0,
     174,   201,     0,    79,     0,    81,   113,    82,    83,     0,
      84,    85,    86,     0,     0,     0,     0,     0,   212,    34,
       0,   199,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,     0,     0,   174,
     201,     0,    79,     0,    81,   113,    82,    83,     0,    84,
      85,    86,     0,     0,     0,  1003,  1004,  1005,    34,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,     0,    82,    83,   213,
      84,    85,    86,     0,   113,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    34,     0,     0,     0,     0,
       0,     0,     0,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,     0,     0,     0,
       0,     0,     0,   975,     0,   113,    82,    83,     0,    84,
      85,    86,     0,     0,     0,     0,  1277,     0,     0,     0,
      34,     0,     0,     0,     0,     0,     0,     0,  1278,  1279,
       0,     0,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   174,   262,   263,    79,
       0,  1280,     0,    82,    83,     0,    84,  1281,    86,     0,
       0,     0,     0,     0,     0,     0,     0,    34,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,     0,   264,     0,     0,     0,    82,    83,
       0,    84,    85,    86,     0,     0,     0,    34,     0,   753,
     754,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      34,     0,     0,     0,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   174,     0,
       0,    79,     0,    81,     0,    82,    83,     0,    84,    85,
      86,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    34,     0,     0,     0,     0,     0,
       0,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,    82,    83,     0,    84,    85,
      86,     0,     0,     0,     0,     0,   327,     0,    82,    83,
       0,    84,    85,    86,     0,     0,     0,     0,    34,     0,
       0,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   485,     0,
       0,     0,    82,    83,     0,    84,    85,    86,     0,     0,
       0,     0,    34,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    34,     0,     0,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   489,     0,     0,     0,    82,    83,     0,    84,
      85,    86,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1060,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   264,     0,     0,     0,
      82,    83,     0,    84,    85,    86,     0,     0,     0,     0,
       0,     0,     0,    82,    83,     0,    84,    85,    86,     0,
       0,     0,     0,     0,     0,     0,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   339,   340,   341,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     342,     0,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,     0,   364,   339,   340,   341,     0,     0,
       0,     0,     0,     0,     0,     0,   365,     0,     0,     0,
       0,     0,   342,     0,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,     0,   364,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   365,     0,
       0,   339,   340,   341,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   342,   416,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,     0,   364,   339,   340,   341,     0,     0,     0,     0,
       0,     0,     0,     0,   365,     0,     0,     0,     0,     0,
     342,   532,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,     0,   364,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   365,     0,     0,   339,
     340,   341,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   342,   805,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,     0,
     364,   896,   897,   898,     0,     0,     0,     0,     0,     0,
       0,     0,   365,     0,     0,     0,     0,     0,   899,   845,
     900,   901,   902,   903,   904,   905,   906,   907,   908,   909,
     910,   911,   912,   913,   914,   915,   916,   917,   918,   919,
     920,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   896,   897,   898,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   899,  1139,   900,   901,   902,   903,
     904,   905,   906,   907,   908,   909,   910,   911,   912,   913,
     914,   915,   916,   917,   918,   919,   920,     0,     0,   896,
     897,   898,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   899,  1050,   900,   901,
     902,   903,   904,   905,   906,   907,   908,   909,   910,   911,
     912,   913,   914,   915,   916,   917,   918,   919,   920,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   896,   897,   898,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   899,  1190,   900,   901,   902,   903,   904,   905,
     906,   907,   908,   909,   910,   911,   912,   913,   914,   915,
     916,   917,   918,   919,   920,     0,     0,     0,     0,   339,
     340,   341,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1262,   342,   802,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,     0,
     364,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   365,   339,   340,   341,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     342,  1336,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,     0,   364,   896,   897,   898,     0,     0,
       0,     0,     0,     0,     0,     0,   365,     0,     0,     0,
       0,     0,   899,  1195,   900,   901,   902,   903,   904,   905,
     906,   907,   908,   909,   910,   911,   912,   913,   914,   915,
     916,   917,   918,   919,   920,   896,   897,   898,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   899,     0,   900,   901,   902,   903,   904,   905,
     906,   907,   908,   909,   910,   911,   912,   913,   914,   915,
     916,   917,   918,   919,   920
};

static const yytype_int16 yycheck[] =
{
       4,    87,   156,   564,   131,    91,    92,    30,   176,   305,
     323,     4,   779,     4,     4,     4,   398,    51,    41,   643,
     377,   971,    45,   159,   364,     4,   410,   214,   432,   403,
      28,  1134,   320,   166,     9,     4,   795,   671,   220,   528,
     772,   110,   424,    47,   130,   798,    50,   220,   686,   687,
     177,   962,     9,     4,     9,     9,     9,    28,   241,   242,
     813,   677,     9,    67,   247,     9,     9,    43,     9,     9,
       9,   125,     9,    24,    25,     9,     9,   481,     9,   125,
      43,     9,    43,    87,    64,   221,     9,    91,    92,     9,
      77,     9,   125,     9,    64,     4,     9,    31,     9,   106,
      77,     9,    54,    33,  1455,    82,     9,     9,   110,     9,
     167,     0,    33,    51,    97,    98,    82,    77,   196,    24,
      25,   145,    28,    28,    76,    63,   130,    79,  1231,   196,
    1233,    43,   201,   152,    43,    94,    82,   320,   196,   196,
     152,   196,    64,   167,   213,    77,   200,    77,    43,     8,
      82,   197,  1503,   165,   538,   125,    77,   164,   145,   110,
     892,   196,   131,   331,   197,   142,   143,   144,    64,   196,
      64,    64,    64,   199,    64,   199,   142,   156,    64,    64,
     199,    64,   186,   143,   144,    64,    64,   199,    64,    64,
     149,  1481,  1482,    64,   103,    64,   142,   121,    64,   108,
     196,   110,   111,   112,   113,   114,   115,   116,   177,   278,
      77,   143,   144,   168,  1317,   168,   196,   200,   194,   199,
     604,   197,   226,   198,   983,  1136,   230,   986,   199,   168,
     234,   125,  1143,  1144,   197,   226,   197,   405,   371,   230,
     326,   198,   199,   996,   153,   154,   200,   156,   252,   376,
     201,   198,   868,   197,   870,   198,   207,   198,   198,   392,
     664,   198,   213,   197,   668,   198,   145,   198,   451,   178,
     198,   909,   910,   762,   197,   226,   278,   199,   198,   230,
     198,   414,   198,  1015,   197,   197,   197,   373,   374,   197,
     423,   200,   198,   426,   197,   197,   201,   197,   682,   194,
     196,    64,   207,   199,    33,   199,   310,   199,   213,   199,
     262,   263,   264,   199,   199,   319,   199,    33,   269,   323,
     199,   199,   326,   199,   199,   276,   277,   278,   199,  1240,
     199,   310,   283,   199,   201,    94,    33,    33,   289,    48,
     292,   310,   411,   377,    77,    77,    77,    77,    77,   121,
      82,   196,    82,    94,  1457,    97,    98,   540,   541,   310,
      94,    77,   125,   546,   269,   547,   364,   371,   372,   373,
     374,   276,   277,   371,   547,   749,    48,   196,   283,  1138,
      77,    77,    94,   196,   289,   196,    94,   196,   392,   151,
     149,   196,  1016,   199,   392,    14,   684,  1031,    28,   110,
     111,   112,   113,   114,   115,   149,   375,   196,   149,    28,
     414,   143,   144,   143,   144,   149,   414,    96,    97,    98,
     607,    33,   426,    27,    77,   423,    45,    77,   426,   811,
     198,   160,    82,    27,   198,   439,   199,   149,   820,     4,
      44,   149,   151,    47,   160,   613,   633,   149,   439,    77,
      44,   198,   403,    47,    82,  1187,   840,   166,   200,   364,
     411,   865,   649,   160,   160,    64,   648,   178,   201,    64,
     201,  1574,   654,   205,   204,   648,   816,   198,    43,   151,
     198,   654,  1241,   779,   570,   198,  1589,   198,   439,   142,
     143,   144,   496,   143,   144,    64,   198,   199,   403,   795,
     198,   684,   685,   145,   888,   368,   411,   511,    69,    70,
     697,   102,   199,   517,   518,   143,   144,  1249,   654,   110,
     111,   112,   113,   114,   115,    69,    70,  1151,   196,  1153,
      77,   196,   395,   485,   145,    82,   399,   489,   103,    96,
      97,    98,   494,   108,    64,   110,   111,   112,   113,   114,
     115,   116,   196,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,   570,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     110,   111,   112,   113,   114,   115,   145,   178,   153,   154,
      42,   156,   123,   124,   777,   142,   143,   144,   149,   956,
      61,    62,   145,   286,    63,   788,   125,   290,   167,   167,
     992,   562,     9,   178,    61,    62,   198,   199,  1000,   203,
      47,    48,    49,  1247,    51,   198,   199,  1011,   145,   312,
    1580,   314,   315,   316,   317,   200,    63,   196,   145,   643,
     199,   645,  1477,  1478,   667,  1595,   196,   125,   178,     8,
     783,    47,    48,    49,   179,   180,   181,   562,   662,  1033,
     198,   186,   187,   614,   125,   190,   191,   110,   111,   112,
     674,   675,   167,  1057,   114,   115,   116,    14,   125,    14,
    1064,   196,    77,   662,   675,   198,   198,   983,    14,   872,
     986,   874,   197,   662,  1098,   167,   198,    83,    14,   650,
    1324,   652,    44,    45,    46,    47,    48,    49,   197,   614,
      94,   662,   197,   196,   202,   102,   720,   196,     9,   196,
      86,   672,   726,   197,   675,     9,   730,   731,   197,   815,
     198,    14,    83,  1107,   196,     9,   197,   182,    77,    77,
     196,   720,     9,    77,   185,   650,     9,   652,   198,   198,
    1132,   720,    77,   197,   123,   197,    64,   761,  1140,  1163,
     198,   196,   124,   197,   947,   166,  1148,   672,   761,   720,
     761,   761,   761,   725,  1178,  1159,   126,   729,     9,   783,
     197,    14,   761,  1550,   194,   783,   969,   738,   971,   976,
       9,    64,   761,   197,     9,   123,    14,   203,   749,   750,
    1567,   110,   111,   112,   113,   114,   115,   200,  1575,  1105,
     761,   815,   121,   122,   203,     9,   196,  1113,   816,   196,
     203,   197,   203,   827,   828,   829,   196,   198,   126,   852,
     197,     9,   197,   738,   198,  1459,   196,   182,   145,   196,
     145,   199,  1138,    14,   749,   750,  1228,  1251,     9,   853,
     159,   855,   182,   857,    77,  1259,    44,    45,    46,    47,
      48,    49,   853,    51,  1047,  1269,   199,    14,    94,   178,
     199,   198,    14,    14,    28,    63,   855,   199,   857,   203,
     198,   196,   196,   196,    14,   196,   855,   891,   857,  1202,
     894,     9,   196,    14,   197,     9,   198,   198,   126,    72,
      73,    74,   853,   196,   855,   197,   857,   197,   859,   203,
      83,   816,     9,   142,    77,     9,   196,   126,    77,   198,
      77,    14,   956,   199,   196,  1329,   197,  1309,   196,   199,
     196,   126,   936,   197,   199,   939,  1232,  1498,   196,     9,
     197,   203,  1238,  1239,   948,  1241,   142,    28,    71,   198,
     197,   168,   198,    28,   859,   197,   960,   130,   131,   132,
     133,   134,   126,     9,   200,   197,     9,   960,   141,   960,
     960,   960,   197,   200,   147,   148,    14,    94,   199,     9,
     196,   960,   126,   197,  1297,   197,   197,   197,   196,   162,
     197,   960,     4,   197,    28,   198,   197,   199,   198,  1182,
    1023,   198,   155,   176,  1565,   151,    77,    14,  1012,   960,
       4,    77,  1016,   196,   126,   197,  1199,   108,   197,   126,
     197,   197,  1026,   197,    14,  1321,    77,   199,   979,  1212,
     198,    43,    14,  1037,   196,  1026,   126,   197,    14,    14,
      53,   199,   197,   199,    77,   197,  1450,   198,  1452,    43,
     198,   196,    77,     9,    77,   198,  1460,   106,  1037,    94,
    1214,   145,    94,   158,    31,    14,   197,   196,  1037,  1020,
     198,     4,   196,    77,   979,  1026,   164,   197,   161,     9,
      77,   197,  1033,  1034,   197,  1212,  1037,   198,    77,   199,
      14,   103,    77,    14,    77,    14,   108,  1501,   110,   111,
     112,   113,   114,   115,   116,    77,    14,   725,   729,   103,
      43,  1558,   494,   374,   108,  1020,   110,   111,   112,   113,
     114,   115,   116,   373,   372,   156,  1105,  1295,  1033,  1034,
     814,   812,   764,  1571,  1113,  1034,  1175,  1567,   499,  1346,
    1430,   153,   154,  1207,   156,  1599,  1587,  1151,  1442,  1153,
      24,    25,    39,  1316,    28,   970,  1107,   377,   931,   153,
     154,   928,   156,   717,   470,   963,   178,   998,   828,   889,
     103,   470,  1012,   842,   277,   108,    50,   110,   111,   112,
     113,   114,   115,   116,   178,   284,   875,    -1,   200,  1593,
      -1,    -1,    -1,    -1,  1198,    -1,  1600,    -1,  1202,    -1,
      -1,    -1,  1107,  1207,    -1,    -1,   200,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,  1198,
     153,   154,    -1,   156,    -1,    -1,    -1,    -1,    -1,  1198,
      -1,    -1,    -1,    -1,    -1,  1214,    -1,    -1,    -1,    -1,
      -1,    -1,  1211,  1247,    -1,   178,    -1,  1198,  1252,    -1,
      -1,    -1,  1256,  1232,  1550,    -1,  1260,    -1,    -1,  1238,
    1239,    -1,    -1,    -1,    -1,  1256,    -1,   200,    -1,    -1,
      -1,  1567,    -1,  1252,    -1,    -1,  1444,    -1,    -1,  1575,
    1284,  1260,    -1,  1252,    -1,    -1,  1290,    -1,    -1,    -1,
      -1,  1260,    -1,  1297,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1252,    -1,    -1,    -1,  1256,    -1,    -1,    -1,  1260,
      -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1324,    -1,    -1,  1327,  1328,    -1,    -1,   201,  1332,    -1,
    1299,    -1,    -1,   207,  1338,    -1,    -1,  1328,    -1,   213,
      -1,    -1,  1321,    -1,    -1,    -1,    -1,    -1,  1327,    -1,
      -1,    43,    -1,  1332,    -1,  1441,    -1,    -1,  1327,  1338,
      -1,    -1,    -1,  1332,    -1,    -1,    -1,   241,   242,  1338,
      -1,    -1,    -1,   247,    -1,    -1,  1327,  1328,    -1,    -1,
      -1,  1332,    -1,    -1,    -1,    -1,    -1,  1338,    -1,    -1,
      -1,    -1,  1546,    -1,    -1,   269,    -1,  1580,    -1,    -1,
      -1,    -1,   276,   277,    -1,    -1,    -1,  1493,    -1,   283,
      -1,   103,  1595,    -1,    -1,   289,   108,    -1,   110,   111,
     112,   113,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1562,    -1,  1441,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   320,    -1,    -1,   323,
      -1,    -1,    -1,    -1,    -1,  1459,    -1,    -1,    -1,  1463,
      -1,   153,   154,    -1,   156,    -1,    -1,    -1,  1472,    -1,
      -1,    -1,    -1,  1477,  1478,    -1,    -1,  1481,  1482,    -1,
      -1,    -1,    -1,    -1,  1463,    -1,   178,    -1,    -1,  1493,
     364,    -1,    -1,    -1,  1463,  1499,  1500,    -1,    -1,    -1,
      -1,  1505,    -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,
      -1,    -1,  1463,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1499,  1500,    -1,    -1,    -1,    -1,  1505,    -1,    -1,   403,
    1499,  1500,    -1,    -1,  1538,    -1,  1505,   411,    -1,    -1,
      -1,  1545,    -1,    -1,    -1,    -1,    -1,    -1,  1499,  1500,
      -1,    -1,    -1,    -1,  1505,    -1,    -1,  1561,    -1,  1538,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1546,    -1,  1538,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   451,    -1,    -1,
      -1,    -1,   456,    -1,    -1,    -1,    -1,  1538,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1601,    -1,    -1,
      -1,    -1,    -1,  1607,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
      -1,    -1,  1601,    -1,    -1,    -1,    -1,   501,  1607,    -1,
      -1,    -1,  1601,    -1,    -1,    -1,    -1,    -1,  1607,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1601,    61,    62,    75,    -1,    77,  1607,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   540,   541,    -1,    -1,
      -1,    -1,   546,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   562,    -1,
      27,    -1,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    -1,    51,   125,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,
      -1,   153,   154,    -1,   156,   157,   158,    -1,    -1,    -1,
     614,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    24,    25,    -1,    -1,    28,    -1,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,    -1,    75,    -1,    77,    -1,   650,   199,   652,   201,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   197,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   672,   673,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
     684,   685,   686,   687,   688,   689,   690,    -1,   121,    -1,
      -1,    27,   696,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    -1,    51,    -1,   721,    -1,    -1,
     153,   154,    -1,   156,   157,   158,    -1,    63,    -1,    -1,
      -1,    -1,    -1,    -1,   738,    -1,   203,    -1,    -1,    -1,
      -1,    -1,    -1,   747,    -1,   749,   750,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     764,    -1,    -1,    -1,    -1,    -1,   199,    -1,   201,    -1,
      -1,    -1,    -1,   777,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   788,    -1,    -1,    -1,    -1,    -1,
     794,    -1,    -1,   797,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   207,    -1,    -1,    -1,    -1,
      -1,    27,   816,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    -1,    51,    -1,    -1,    -1,    -1,
      75,    -1,    77,    -1,    -1,    -1,    -1,    63,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   859,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   200,    -1,    -1,   269,   872,    -1,
     874,    -1,    -1,    -1,   276,   277,    -1,    -1,    -1,    -1,
      -1,   283,    -1,    -1,    -1,   889,    -1,   289,    -1,    -1,
      -1,   895,   896,   897,   898,   899,   900,   901,   902,   903,
     904,   905,   906,   907,   908,   909,   910,   911,   912,   913,
     914,   915,   916,   917,   918,   919,   920,    -1,   153,   154,
      -1,   156,   157,   158,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    -1,    -1,
     944,    -1,    -1,   947,    -1,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    -1,    -1,
      -1,    -1,   364,    -1,   199,   969,   201,   971,    -1,    -1,
      -1,    61,    62,    -1,    -1,   979,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   200,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,   403,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    63,    -1,  1020,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1033,
    1034,    -1,  1036,    -1,    -1,   125,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1047,    -1,    -1,  1050,    -1,  1052,    10,
      11,    12,    -1,    -1,   456,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1068,    -1,    27,    -1,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    -1,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   501,
    1104,    -1,    63,  1107,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,  1128,    27,    -1,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     562,    63,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    -1,    51,    -1,    -1,  1182,    -1,
      -1,    -1,  1186,    -1,    -1,    -1,  1190,    63,  1192,    -1,
      -1,  1195,    -1,    -1,    -1,  1199,    -1,    -1,  1202,  1203,
      -1,  1205,    -1,    -1,    -1,    -1,    -1,    -1,  1212,    -1,
      -1,    -1,   614,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1225,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,   200,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   650,    -1,
     652,    -1,    63,    -1,    -1,    -1,    -1,    -1,  1262,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1272,  1273,
     672,   673,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   686,   687,   688,   689,   690,    -1,
      -1,    -1,    -1,  1297,   696,    -1,    -1,    -1,   200,    -1,
      -1,    -1,  1306,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   241,   242,    -1,    -1,    -1,   721,
     247,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1335,  1336,    -1,    -1,    -1,   738,  1341,  1342,    -1,
      -1,    -1,  1346,    -1,    -1,   747,    -1,   749,   750,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   764,    -1,    -1,    -1,    27,    -1,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    -1,
      51,    -1,   794,   320,    -1,   797,   323,    -1,    10,    11,
      12,    -1,    63,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   816,    27,    -1,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    51,
      -1,  1445,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,
      -1,    63,    -1,    -1,    -1,    -1,    -1,   859,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1472,    -1,
      -1,    -1,  1476,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1487,    -1,    -1,    -1,   889,  1492,    -1,
      -1,    -1,    -1,   895,   896,   897,   898,   899,   900,   901,
     902,   903,   904,   905,   906,   907,   908,   909,   910,   911,
     912,   913,   914,   915,   916,   917,   918,   919,   920,    -1,
      -1,    -1,    -1,   149,   451,    -1,    -1,   153,   154,   456,
     156,   157,   158,    -1,    -1,    -1,    -1,    -1,    -1,   200,
      -1,    -1,   944,    -1,    -1,  1549,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1557,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,  1571,    -1,    -1,
      -1,    -1,    -1,   199,   501,    -1,  1580,   979,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,
      -1,  1595,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1604,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1612,    -1,
      -1,    -1,  1616,   540,   541,  1619,    -1,    -1,  1020,   546,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1033,  1034,    -1,  1036,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1050,    -1,
    1052,    10,    11,    12,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1068,    -1,    27,    -1,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,    75,
      -1,    77,  1104,    -1,    63,  1107,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    -1,    51,  1128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,   115,
      -1,    -1,    -1,    -1,    -1,    -1,   673,    -1,    -1,    -1,
      -1,   127,    -1,    -1,    -1,    -1,    -1,   684,   685,   686,
     687,   688,   689,   690,    -1,    -1,    -1,    -1,    -1,   696,
     146,    -1,    -1,   149,    -1,   151,    -1,   153,   154,    -1,
     156,   157,   158,    -1,  1186,    -1,    -1,    -1,  1190,    -1,
    1192,    -1,    -1,  1195,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1203,    -1,  1205,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,    -1,    -1,    -1,
     196,    -1,    -1,  1225,    -1,   201,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   200,    -1,    75,    -1,    -1,    -1,    -1,    -1,    -1,
     777,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
    1262,   788,    -1,    -1,    -1,    -1,    -1,   794,    -1,    -1,
    1272,  1273,    27,    -1,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    -1,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1306,    -1,    -1,    -1,    63,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    -1,   151,
      -1,   153,   154,   155,   156,   157,   158,    -1,    -1,    -1,
      -1,    -1,    -1,  1335,  1336,    -1,    -1,    -1,    -1,  1341,
    1342,    -1,    -1,    -1,  1346,   872,    -1,   874,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   119,   889,    -1,   196,    -1,    -1,    -1,   895,   896,
     897,   898,   899,   900,   901,   902,   903,   904,   905,   906,
     907,   908,   909,   910,   911,   912,   913,   914,   915,   916,
     917,   918,   919,   920,    -1,   153,   154,    -1,   156,   157,
     158,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,   944,    -1,    -1,
     947,    -1,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,    -1,    33,    -1,   196,    -1,
      -1,    -1,   969,  1445,   971,   200,    -1,    44,    45,    -1,
      -1,    -1,    -1,    50,    -1,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    -1,    64,    65,    66,
      67,    -1,    -1,    -1,  1476,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    -1,  1487,    83,    -1,    -1,    -1,
    1492,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   102,    -1,    -1,    -1,  1036,
      -1,    -1,    -1,   110,   111,   112,   113,   114,   115,    -1,
    1047,   118,   119,  1050,    -1,  1052,    -1,    -1,    -1,    -1,
     127,   128,    -1,   130,   131,   132,   133,   134,    -1,    -1,
      -1,  1068,    -1,    -1,   141,    -1,    -1,  1549,    -1,   146,
     147,   148,   149,   150,   151,  1557,   153,   154,    -1,   156,
     157,   158,    -1,    -1,    -1,   162,    -1,    -1,   165,  1571,
      -1,    -1,    -1,    -1,   171,    -1,    -1,    -1,    -1,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,    -1,    -1,   196,
      -1,  1128,  1604,    -1,   201,   202,    -1,   204,   205,    -1,
    1612,    -1,    -1,    -1,  1616,    -1,    -1,  1619,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,     5,     6,    -1,     8,
       9,    10,    -1,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,  1182,    -1,    26,    27,  1186,
      -1,    -1,   456,  1190,    -1,  1192,    -1,    -1,  1195,    -1,
      39,    -1,  1199,    -1,    -1,  1202,    -1,    46,    -1,    48,
      -1,    -1,    51,    -1,    53,  1212,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1225,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   501,    -1,    -1,
      -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    94,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,  1262,    -1,    -1,    -1,    -1,
      -1,   110,    -1,    -1,    27,    -1,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    12,    51,    -1,
    1297,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1306,
      63,    -1,    27,    -1,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    10,    11,    12,  1335,  1336,
      -1,    -1,   181,    -1,  1341,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    -1,    51,    -1,    -1,    -1,
      -1,    -1,    -1,   222,    -1,    -1,   225,    -1,    63,    -1,
      -1,    -1,    -1,   232,   233,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   673,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   686,   687,   688,   689,   690,    -1,    -1,    -1,
      -1,    -1,   696,    -1,    -1,    -1,    -1,    -1,    -1,   278,
      -1,    -1,    -1,    -1,    -1,   284,    -1,   200,  1445,   288,
      -1,    -1,   456,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,   311,    51,    -1,  1472,    -1,    -1,    -1,  1476,
      -1,    -1,    -1,   322,    -1,    63,    -1,    -1,    -1,    -1,
    1487,    -1,    -1,    -1,    -1,  1492,    -1,   501,    -1,    -1,
     339,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   200,   365,    -1,   367,   368,
     794,   370,    -1,    -1,    -1,    -1,    -1,    -1,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
     389,    -1,  1549,    -1,    -1,    -1,   395,   396,    -1,   398,
     399,   400,    -1,    -1,    -1,    -1,    -1,   406,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   416,    -1,   418,
      -1,    -1,    -1,  1580,    -1,   424,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   434,    -1,   436,  1595,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1604,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1612,    -1,    -1,    -1,  1616,
      -1,    -1,  1619,    -1,   463,   889,    -1,   466,   467,   468,
      -1,   895,   896,   897,   898,   899,   900,   901,   902,   903,
     904,   905,   906,   907,   908,   909,   910,   911,   912,   913,
     914,   915,   916,   917,   918,   919,   920,    -1,   497,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   673,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     944,    -1,   686,   687,   688,   689,   690,    10,    11,    12,
      -1,    -1,   696,   532,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    -1,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    12,    51,    -1,
      -1,    -1,    -1,    -1,    -1,   574,    -1,    -1,    -1,    -1,
      63,    -1,    27,    -1,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    -1,    51,   606,    -1,    -1,
      -1,    -1,  1036,    -1,    -1,    -1,   615,    -1,    63,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1050,    -1,  1052,    -1,
     794,    -1,    -1,   632,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1068,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,   658,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   671,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    -1,    51,    -1,    -1,
      -1,    -1,    -1,    -1,  1128,    -1,    -1,   706,    -1,    63,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   889,    -1,    -1,    -1,    -1,
      -1,   895,   896,   897,   898,   899,   900,   901,   902,   903,
     904,   905,   906,   907,   908,   909,   910,   911,   912,   913,
     914,   915,   916,   917,   918,   919,   920,    -1,    -1,   758,
      -1,    -1,  1186,    -1,   456,    -1,  1190,    -1,  1192,    -1,
      -1,  1195,   771,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     944,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   792,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1225,    -1,   802,    -1,    -1,   805,    -1,   807,   501,
      -1,    -1,   811,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   820,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1262,    -1,
      -1,    -1,    -1,    -1,   198,    27,   845,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    51,
      -1,    -1,  1036,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    63,  1306,    -1,    -1,    -1,  1050,    -1,  1052,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1068,    -1,    -1,    -1,    -1,    -1,
      -1,  1335,  1336,    -1,    10,    11,    12,  1341,    -1,    -1,
      -1,  1345,    -1,   922,   923,   924,    -1,    -1,    -1,   928,
     929,    27,    -1,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    -1,    51,    -1,   956,    65,    66,
      -1,    -1,    -1,    -1,  1128,    -1,    -1,    63,    75,    -1,
      77,    -1,    -1,    -1,    -1,    -1,   975,    -1,    -1,    -1,
      -1,   673,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   992,    -1,    -1,   688,   689,    -1,    -1,
      -1,  1000,    -1,  1002,   696,    -1,    -1,    -1,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,
      -1,  1445,  1186,    -1,    -1,    -1,  1190,    -1,  1192,    -1,
      -1,  1195,  1031,    -1,    -1,    -1,    -1,    -1,    -1,   146,
      -1,  1040,   149,    -1,   151,    -1,   153,   154,    -1,   156,
     157,   158,  1476,    75,    -1,    77,    -1,    -1,   165,    -1,
      -1,  1225,    -1,  1487,    -1,    -1,    -1,    -1,  1492,    -1,
      -1,    -1,    -1,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,    -1,    -1,    -1,   196,
      -1,  1515,    -1,   115,   201,    -1,    -1,    -1,  1262,    -1,
      -1,   197,   794,    -1,    -1,   127,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1116,    -1,    -1,
      -1,  1120,    -1,  1122,   146,  1549,    -1,   149,    -1,   151,
      -1,   153,   154,  1132,   156,   157,   158,    -1,    -1,    -1,
    1139,  1140,  1306,    -1,    -1,    -1,    -1,    -1,    -1,  1148,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,  1335,  1336,    -1,   196,    -1,    -1,  1341,    -1,   201,
    1604,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1612,    -1,
      -1,    -1,  1616,    -1,    -1,  1619,    -1,  1196,    -1,    -1,
      -1,    -1,    -1,   895,   896,   897,   898,   899,   900,   901,
     902,   903,   904,   905,   906,   907,   908,    -1,    -1,   911,
     912,   913,   914,   915,   916,   917,   918,   919,   920,  1228,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   944,    -1,    -1,    27,    -1,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    51,
      -1,  1445,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    63,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    51,
    1309,    -1,  1476,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    63,    -1,  1487,    -1,    -1,    -1,    -1,  1492,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1337,    -1,
      -1,    -1,    -1,    -1,  1036,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1050,    -1,
    1052,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,  1068,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1549,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    44,    45,    -1,    -1,    -1,    -1,    50,    -1,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    -1,    64,    65,    66,    67,   198,    -1,    -1,    -1,
      72,    73,    74,    75,    76,    77,  1128,    -1,    -1,    -1,
    1604,    83,    -1,    -1,    -1,    -1,    -1,    -1,  1612,    -1,
      -1,    -1,  1616,    -1,    -1,  1619,    -1,    -1,    -1,    -1,
     102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   110,   111,
     112,   113,   114,   115,    -1,    -1,   118,   119,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   127,   128,    -1,   130,   131,
     132,   133,   134,    -1,  1186,    -1,    -1,    -1,  1190,   141,
    1192,    -1,    -1,  1195,   146,   147,   148,   149,    -1,   151,
      -1,   153,   154,    -1,   156,   157,   158,    -1,    -1,    -1,
     162,    -1,    -1,   165,    -1,    -1,    -1,    -1,    -1,   171,
      -1,    -1,    -1,  1225,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,    -1,    -1,   196,    -1,   198,    -1,    -1,   201,
     202,    -1,   204,   205,    -1,    -1,    10,    11,    12,    -1,
    1262,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    -1,    51,    -1,    -1,
      -1,    -1,    -1,    -1,  1306,    -1,    -1,    -1,    27,    63,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    -1,    51,  1335,  1336,    -1,    -1,    -1,    -1,  1341,
      -1,    -1,    -1,    -1,    63,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    27,    13,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    -1,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    45,    -1,    -1,    63,    -1,
      50,    -1,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    -1,    64,    65,    66,    67,    68,    -1,
      -1,    -1,    72,    73,    74,    75,    76,    77,    -1,    79,
      -1,    -1,    -1,    83,    84,    85,    86,    -1,    88,    -1,
      90,    -1,    92,  1445,   198,    95,    -1,    -1,    -1,    99,
     100,   101,   102,   103,   104,   105,    -1,   107,   108,   109,
     110,   111,   112,   113,   114,   115,    -1,   117,   118,   119,
     120,   121,   122,    -1,  1476,    -1,    -1,   127,   128,    75,
     130,   131,   132,   133,   134,  1487,    -1,    -1,    -1,    -1,
    1492,   141,    -1,    -1,    -1,    -1,   146,   147,   148,   149,
     150,   151,    -1,   153,   154,    -1,   156,   157,   158,   159,
      -1,    -1,   162,    -1,    -1,   165,    -1,    -1,    -1,    -1,
     185,   171,   172,    -1,   174,    -1,   176,   177,   178,    -1,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,    -1,    -1,   196,  1549,   198,   199,
     200,   201,   202,    -1,   204,   205,    -1,   153,   154,    -1,
     156,   157,   158,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,    -1,    -1,    -1,
     196,    -1,  1604,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1612,    43,    44,    45,  1616,    -1,    -1,  1619,    50,    -1,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    -1,    64,    65,    66,    67,    68,    -1,    -1,    -1,
      72,    73,    74,    75,    76,    77,    -1,    79,    -1,    -1,
      -1,    83,    84,    85,    86,    -1,    88,    -1,    90,    -1,
      92,    -1,    -1,    95,    -1,    -1,    -1,    99,   100,   101,
     102,   103,   104,   105,    -1,   107,   108,   109,   110,   111,
     112,   113,   114,   115,    -1,   117,   118,   119,   120,   121,
     122,    -1,    -1,    -1,    -1,   127,   128,    -1,   130,   131,
     132,   133,   134,    -1,    -1,    -1,    -1,    -1,    -1,   141,
      -1,    -1,    -1,    -1,   146,   147,   148,   149,   150,   151,
      -1,   153,   154,    -1,   156,   157,   158,   159,    -1,    -1,
     162,    -1,    -1,   165,    -1,    -1,    -1,    -1,    -1,   171,
     172,    -1,   174,    -1,   176,   177,   178,    -1,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,    -1,    -1,   196,    -1,   198,   199,   200,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    -1,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    44,    45,    -1,    -1,    63,
      -1,    50,    -1,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    -1,    64,    65,    66,    67,    68,
      -1,    -1,    -1,    72,    73,    74,    75,    76,    77,    -1,
      79,    -1,    -1,    -1,    83,    84,    85,    86,    -1,    88,
      -1,    90,    -1,    92,    -1,    -1,    95,    -1,    -1,    -1,
      99,   100,   101,   102,   103,   104,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,   114,   115,    -1,   117,   118,
     119,   120,   121,   122,    -1,    -1,    -1,    -1,   127,   128,
      -1,   130,   131,   132,   133,   134,    -1,    -1,    -1,    -1,
      -1,    -1,   141,    -1,    -1,    -1,    -1,   146,   147,   148,
     149,   150,   151,    -1,   153,   154,    -1,   156,   157,   158,
     159,    -1,    -1,   162,    -1,    -1,   165,    -1,    -1,    -1,
      -1,    -1,   171,   172,    -1,   174,    -1,   176,   177,   178,
      -1,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,    -1,    -1,   196,    -1,   198,
     199,    -1,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    45,
      -1,    63,    -1,    -1,    50,    -1,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    -1,    64,    65,
      66,    67,    68,    -1,    -1,    -1,    72,    73,    74,    75,
      76,    77,    -1,    79,    -1,    -1,    -1,    83,    84,    85,
      86,    -1,    88,    -1,    90,    -1,    92,    -1,    -1,    95,
      -1,    -1,    -1,    99,   100,   101,   102,    -1,   104,   105,
      -1,   107,    -1,   109,   110,   111,   112,   113,   114,   115,
      -1,   117,   118,   119,    -1,   121,   122,    -1,    -1,    -1,
      -1,   127,   128,    -1,   130,   131,   132,   133,   134,    -1,
      -1,    -1,    -1,    -1,    -1,   141,    -1,    -1,    -1,    -1,
     146,   147,   148,   149,   150,   151,    -1,   153,   154,    -1,
     156,   157,   158,   159,    -1,    -1,   162,    -1,    -1,   165,
      -1,    -1,    -1,    -1,    -1,   171,    -1,    -1,    -1,    -1,
     176,   177,   178,    -1,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,    -1,    -1,
     196,    -1,   198,   199,   200,   201,   202,    -1,   204,   205,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    27,
      13,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    45,    -1,    -1,    -1,    -1,    50,    -1,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      -1,    64,    65,    66,    67,    68,    -1,    -1,    -1,    72,
      73,    74,    75,    76,    77,    -1,    79,    -1,    -1,    -1,
      83,    84,    85,    86,    -1,    88,    -1,    90,    -1,    92,
      -1,    -1,    95,    -1,    -1,    -1,    99,   100,   101,   102,
      -1,   104,   105,    -1,   107,    -1,   109,   110,   111,   112,
     113,   114,   115,    -1,   117,   118,   119,    -1,   121,   122,
      -1,    -1,    -1,    -1,   127,   128,    -1,   130,   131,   132,
     133,   134,    -1,    -1,    -1,    -1,    -1,    -1,   141,    -1,
      -1,    -1,    -1,   146,   147,   148,   149,   150,   151,    -1,
     153,   154,    -1,   156,   157,   158,   159,    -1,    -1,   162,
      -1,    -1,   165,    -1,    -1,    -1,    -1,    -1,   171,    -1,
      -1,    -1,    -1,   176,   177,   178,    -1,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,    -1,    -1,   196,    -1,   198,   199,   200,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    45,    -1,    -1,    -1,    -1,
      50,    -1,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    -1,    64,    65,    66,    67,    68,    -1,
      -1,    -1,    72,    73,    74,    75,    76,    77,    -1,    79,
      -1,    -1,    -1,    83,    84,    85,    86,    -1,    88,    -1,
      90,    -1,    92,    -1,    -1,    95,    -1,    -1,    -1,    99,
     100,   101,   102,    -1,   104,   105,    -1,   107,    -1,   109,
     110,   111,   112,   113,   114,   115,    -1,   117,   118,   119,
      -1,   121,   122,    -1,    -1,    -1,    -1,   127,   128,    -1,
     130,   131,   132,   133,   134,    -1,    -1,    -1,    -1,    -1,
      -1,   141,    -1,    -1,    -1,    -1,   146,   147,   148,   149,
     150,   151,    -1,   153,   154,    -1,   156,   157,   158,   159,
      -1,    -1,   162,    -1,    -1,   165,    -1,    -1,    -1,    -1,
      -1,   171,    -1,    -1,    -1,    -1,   176,   177,   178,    -1,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,    -1,    -1,   196,    -1,   198,   199,
     200,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    45,    -1,
      -1,    -1,    -1,    50,    -1,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    -1,    64,    65,    66,
      67,    68,    -1,    -1,    -1,    72,    73,    74,    75,    76,
      77,    -1,    79,    -1,    -1,    -1,    83,    84,    85,    86,
      87,    88,    -1,    90,    -1,    92,    -1,    -1,    95,    -1,
      -1,    -1,    99,   100,   101,   102,    -1,   104,   105,    -1,
     107,    -1,   109,   110,   111,   112,   113,   114,   115,    -1,
     117,   118,   119,    -1,   121,   122,    -1,    -1,    -1,    -1,
     127,   128,    -1,   130,   131,   132,   133,   134,    -1,    -1,
      -1,    -1,    -1,    -1,   141,    -1,    -1,    -1,    -1,   146,
     147,   148,   149,   150,   151,    -1,   153,   154,    -1,   156,
     157,   158,   159,    -1,    -1,   162,    -1,    -1,   165,    -1,
      -1,    -1,    -1,    -1,   171,    -1,    -1,    -1,    -1,   176,
     177,   178,    -1,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,    -1,    -1,   196,
      -1,   198,   199,    -1,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      44,    45,    -1,    -1,    -1,    -1,    50,    -1,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    -1,
      64,    65,    66,    67,    68,    -1,    -1,    -1,    72,    73,
      74,    75,    76,    77,    -1,    79,    -1,    -1,    -1,    83,
      84,    85,    86,    -1,    88,    -1,    90,    -1,    92,    93,
      -1,    95,    -1,    -1,    -1,    99,   100,   101,   102,    -1,
     104,   105,    -1,   107,    -1,   109,   110,   111,   112,   113,
     114,   115,    -1,   117,   118,   119,    -1,   121,   122,    -1,
      -1,    -1,    -1,   127,   128,    -1,   130,   131,   132,   133,
     134,    -1,    -1,    -1,    -1,    -1,    -1,   141,    -1,    -1,
      -1,    -1,   146,   147,   148,   149,   150,   151,    -1,   153,
     154,    -1,   156,   157,   158,   159,    -1,    -1,   162,    -1,
      -1,   165,    -1,    -1,    -1,    -1,    -1,   171,    -1,    -1,
      -1,    -1,   176,   177,   178,    -1,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
      -1,    -1,   196,    -1,   198,   199,    -1,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    45,    -1,    -1,    -1,    -1,    50,
      -1,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    -1,    64,    65,    66,    67,    68,    -1,    -1,
      -1,    72,    73,    74,    75,    76,    77,    -1,    79,    -1,
      -1,    -1,    83,    84,    85,    86,    -1,    88,    -1,    90,
      -1,    92,    -1,    -1,    95,    -1,    -1,    -1,    99,   100,
     101,   102,    -1,   104,   105,    -1,   107,    -1,   109,   110,
     111,   112,   113,   114,   115,    -1,   117,   118,   119,    -1,
     121,   122,    -1,    -1,    -1,    -1,   127,   128,    -1,   130,
     131,   132,   133,   134,    -1,    -1,    -1,    -1,    -1,    -1,
     141,    -1,    -1,    -1,    -1,   146,   147,   148,   149,   150,
     151,    -1,   153,   154,    -1,   156,   157,   158,   159,    -1,
      -1,   162,    -1,    -1,   165,    -1,    -1,    -1,    -1,    -1,
     171,    -1,    -1,    -1,    -1,   176,   177,   178,    -1,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,    -1,    -1,   196,    -1,   198,   199,   200,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    45,    -1,    -1,
      -1,    -1,    50,    -1,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    -1,    64,    65,    66,    67,
      68,    -1,    -1,    -1,    72,    73,    74,    75,    76,    77,
      -1,    79,    -1,    -1,    -1,    83,    84,    85,    86,    -1,
      88,    -1,    90,    -1,    92,    -1,    -1,    95,    -1,    -1,
      -1,    99,   100,   101,   102,    -1,   104,   105,    -1,   107,
      -1,   109,   110,   111,   112,   113,   114,   115,    -1,   117,
     118,   119,    -1,   121,   122,    -1,    -1,    -1,    -1,   127,
     128,    -1,   130,   131,   132,   133,   134,    -1,    -1,    -1,
      -1,    -1,    -1,   141,    -1,    -1,    -1,    -1,   146,   147,
     148,   149,   150,   151,    -1,   153,   154,    -1,   156,   157,
     158,   159,    -1,    -1,   162,    -1,    -1,   165,    -1,    -1,
      -1,    -1,    -1,   171,    -1,    -1,    -1,    -1,   176,   177,
     178,    -1,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,    -1,    -1,   196,    -1,
     198,   199,   200,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      45,    -1,    -1,    -1,    -1,    50,    -1,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    -1,    64,
      65,    66,    67,    68,    -1,    -1,    -1,    72,    73,    74,
      75,    76,    77,    -1,    79,    -1,    -1,    -1,    83,    84,
      85,    86,    -1,    88,    -1,    90,    91,    92,    -1,    -1,
      95,    -1,    -1,    -1,    99,   100,   101,   102,    -1,   104,
     105,    -1,   107,    -1,   109,   110,   111,   112,   113,   114,
     115,    -1,   117,   118,   119,    -1,   121,   122,    -1,    -1,
      -1,    -1,   127,   128,    -1,   130,   131,   132,   133,   134,
      -1,    -1,    -1,    -1,    -1,    -1,   141,    -1,    -1,    -1,
      -1,   146,   147,   148,   149,   150,   151,    -1,   153,   154,
      -1,   156,   157,   158,   159,    -1,    -1,   162,    -1,    -1,
     165,    -1,    -1,    -1,    -1,    -1,   171,    -1,    -1,    -1,
      -1,   176,   177,   178,    -1,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,    -1,
      -1,   196,    -1,   198,   199,    -1,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    45,    -1,    -1,    -1,    -1,    50,    -1,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    -1,    64,    65,    66,    67,    68,    -1,    -1,    -1,
      72,    73,    74,    75,    76,    77,    -1,    79,    -1,    -1,
      -1,    83,    84,    85,    86,    -1,    88,    -1,    90,    -1,
      92,    -1,    -1,    95,    -1,    -1,    -1,    99,   100,   101,
     102,    -1,   104,   105,    -1,   107,    -1,   109,   110,   111,
     112,   113,   114,   115,    -1,   117,   118,   119,    -1,   121,
     122,    -1,    -1,    -1,    -1,   127,   128,    -1,   130,   131,
     132,   133,   134,    -1,    -1,    -1,    -1,    -1,    -1,   141,
      -1,    -1,    -1,    -1,   146,   147,   148,   149,   150,   151,
      -1,   153,   154,    -1,   156,   157,   158,   159,    -1,    -1,
     162,    -1,    -1,   165,    -1,    -1,    -1,    -1,    -1,   171,
      -1,    -1,    -1,    -1,   176,   177,   178,    -1,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,    -1,    -1,   196,    -1,   198,   199,   200,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    44,    45,    -1,    -1,    -1,
      -1,    50,    -1,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    -1,    64,    65,    66,    67,    68,
      -1,    -1,    -1,    72,    73,    74,    75,    76,    77,    -1,
      79,    -1,    -1,    -1,    83,    84,    85,    86,    -1,    88,
      -1,    90,    -1,    92,    -1,    -1,    95,    -1,    -1,    -1,
      99,   100,   101,   102,    -1,   104,   105,    -1,   107,    -1,
     109,   110,   111,   112,   113,   114,   115,    -1,   117,   118,
     119,    -1,   121,   122,    -1,    -1,    -1,    -1,   127,   128,
      -1,   130,   131,   132,   133,   134,    -1,    -1,    -1,    -1,
      -1,    -1,   141,    -1,    -1,    -1,    -1,   146,   147,   148,
     149,   150,   151,    -1,   153,   154,    -1,   156,   157,   158,
     159,    -1,    -1,   162,    -1,    -1,   165,    -1,    -1,    -1,
      -1,    -1,   171,    -1,    -1,    -1,    -1,   176,   177,   178,
      -1,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,    -1,    -1,   196,    -1,   198,
     199,   200,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    45,
      -1,    -1,    -1,    -1,    50,    -1,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    -1,    64,    65,
      66,    67,    68,    -1,    -1,    -1,    72,    73,    74,    75,
      76,    77,    -1,    79,    -1,    -1,    -1,    83,    84,    85,
      86,    -1,    88,    89,    90,    -1,    92,    -1,    -1,    95,
      -1,    -1,    -1,    99,   100,   101,   102,    -1,   104,   105,
      -1,   107,    -1,   109,   110,   111,   112,   113,   114,   115,
      -1,   117,   118,   119,    -1,   121,   122,    -1,    -1,    -1,
      -1,   127,   128,    -1,   130,   131,   132,   133,   134,    -1,
      -1,    -1,    -1,    -1,    -1,   141,    -1,    -1,    -1,    -1,
     146,   147,   148,   149,   150,   151,    -1,   153,   154,    -1,
     156,   157,   158,   159,    -1,    -1,   162,    -1,    -1,   165,
      -1,    -1,    -1,    -1,    -1,   171,    -1,    -1,    -1,    -1,
     176,   177,   178,    -1,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,    -1,    -1,
     196,    -1,   198,   199,    -1,   201,   202,    -1,   204,   205,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    45,    -1,    -1,    -1,    -1,    50,    -1,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      -1,    64,    65,    66,    67,    68,    -1,    -1,    -1,    72,
      73,    74,    75,    76,    77,    -1,    79,    -1,    -1,    -1,
      83,    84,    85,    86,    -1,    88,    -1,    90,    -1,    92,
      -1,    -1,    95,    -1,    -1,    -1,    99,   100,   101,   102,
      -1,   104,   105,    -1,   107,    -1,   109,   110,   111,   112,
     113,   114,   115,    -1,   117,   118,   119,    -1,   121,   122,
      -1,    -1,    -1,    -1,   127,   128,    -1,   130,   131,   132,
     133,   134,    -1,    -1,    -1,    -1,    -1,    -1,   141,    -1,
      -1,    -1,    -1,   146,   147,   148,   149,   150,   151,    -1,
     153,   154,    -1,   156,   157,   158,   159,    -1,    -1,   162,
      -1,    -1,   165,    -1,    -1,    -1,    -1,    -1,   171,    -1,
      -1,    -1,    -1,   176,   177,   178,    -1,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,    -1,    -1,   196,    -1,   198,   199,   200,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    45,    -1,    -1,    -1,    -1,
      50,    -1,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    -1,    64,    65,    66,    67,    68,    -1,
      -1,    -1,    72,    73,    74,    75,    76,    77,    -1,    79,
      -1,    -1,    -1,    83,    84,    85,    86,    -1,    88,    -1,
      90,    -1,    92,    -1,    -1,    95,    -1,    -1,    -1,    99,
     100,   101,   102,    -1,   104,   105,    -1,   107,    -1,   109,
     110,   111,   112,   113,   114,   115,    -1,   117,   118,   119,
      -1,   121,   122,    -1,    -1,    -1,    -1,   127,   128,    -1,
     130,   131,   132,   133,   134,    -1,    -1,    -1,    -1,    -1,
      -1,   141,    -1,    -1,    -1,    -1,   146,   147,   148,   149,
     150,   151,    -1,   153,   154,    -1,   156,   157,   158,   159,
      -1,    -1,   162,    -1,    -1,   165,    -1,    -1,    -1,    -1,
      -1,   171,    -1,    -1,    -1,    -1,   176,   177,   178,    -1,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,    -1,    -1,   196,    -1,   198,   199,
     200,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    45,    -1,
      -1,    -1,    -1,    50,    -1,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    -1,    64,    65,    66,
      67,    68,    -1,    -1,    -1,    72,    73,    74,    75,    76,
      77,    -1,    79,    -1,    -1,    -1,    83,    84,    85,    86,
      -1,    88,    -1,    90,    -1,    92,    -1,    -1,    95,    -1,
      -1,    -1,    99,   100,   101,   102,    -1,   104,   105,    -1,
     107,    -1,   109,   110,   111,   112,   113,   114,   115,    -1,
     117,   118,   119,    -1,   121,   122,    -1,    -1,    -1,    -1,
     127,   128,    -1,   130,   131,   132,   133,   134,    -1,    -1,
      -1,    -1,    -1,    -1,   141,    -1,    -1,    -1,    -1,   146,
     147,   148,   149,   150,   151,    -1,   153,   154,    -1,   156,
     157,   158,   159,    -1,    -1,   162,    -1,    -1,   165,    -1,
      -1,    -1,    -1,    -1,   171,    -1,    -1,    -1,    -1,   176,
     177,   178,    -1,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,    -1,    -1,   196,
      -1,   198,   199,   200,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      44,    45,    -1,    -1,    -1,    -1,    50,    -1,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    -1,
      64,    65,    66,    67,    68,    -1,    -1,    -1,    72,    73,
      74,    75,    76,    77,    -1,    79,    -1,    -1,    -1,    83,
      84,    85,    86,    -1,    88,    -1,    90,    -1,    92,    -1,
      -1,    95,    -1,    -1,    -1,    99,   100,   101,   102,    -1,
     104,   105,    -1,   107,    -1,   109,   110,   111,   112,   113,
     114,   115,    -1,   117,   118,   119,    -1,   121,   122,    -1,
      -1,    -1,    -1,   127,   128,    -1,   130,   131,   132,   133,
     134,    -1,    -1,    -1,    -1,    -1,    -1,   141,    -1,    -1,
      -1,    -1,   146,   147,   148,   149,   150,   151,    -1,   153,
     154,    -1,   156,   157,   158,   159,    -1,    -1,   162,    -1,
      -1,   165,    -1,    -1,    -1,    -1,    -1,   171,    -1,    -1,
      -1,    -1,   176,   177,   178,    -1,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
      -1,    -1,   196,    -1,   198,   199,    -1,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    50,
      -1,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    -1,    64,    65,    66,    67,    68,    -1,    -1,
      -1,    72,    73,    74,    75,    76,    77,    -1,    79,    -1,
      -1,    -1,    83,    84,    85,    86,    -1,    88,    -1,    90,
      -1,    92,    -1,    -1,    95,    -1,    -1,    -1,    99,   100,
     101,   102,    -1,   104,   105,    -1,   107,    -1,   109,   110,
     111,   112,   113,   114,   115,    -1,   117,   118,   119,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   127,   128,    -1,   130,
     131,   132,   133,   134,    -1,    -1,    -1,    -1,    -1,    -1,
     141,    -1,    -1,    -1,    -1,   146,   147,   148,   149,   150,
     151,    -1,   153,   154,    -1,   156,   157,   158,    -1,    -1,
      -1,   162,    -1,    -1,   165,    -1,    -1,    -1,    -1,    -1,
     171,    -1,    -1,    -1,    -1,   176,   177,   178,    -1,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,    -1,    -1,   196,    -1,   198,   199,    -1,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,
      -1,    -1,    50,    -1,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    -1,    64,    65,    66,    67,
      68,    -1,    -1,    -1,    72,    73,    74,    75,    76,    77,
      -1,    79,    -1,    -1,    -1,    83,    84,    85,    86,    -1,
      88,    -1,    90,    -1,    92,    -1,    -1,    95,    -1,    -1,
      -1,    99,   100,   101,   102,    -1,   104,   105,    -1,   107,
      -1,   109,   110,   111,   112,   113,   114,   115,    -1,   117,
     118,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,
     128,    -1,   130,   131,   132,   133,   134,    -1,    -1,    -1,
      -1,    -1,    -1,   141,    -1,    -1,    -1,    -1,   146,   147,
     148,   149,   150,   151,    -1,   153,   154,    -1,   156,   157,
     158,    -1,    -1,    -1,   162,    -1,    -1,   165,    -1,    -1,
      -1,    -1,    -1,   171,    -1,    -1,    -1,    -1,   176,   177,
     178,    -1,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,    -1,    -1,   196,    -1,
     198,   199,    -1,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      45,    -1,    -1,    -1,    -1,    50,    -1,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    -1,    64,
      65,    66,    67,    68,    -1,    -1,    -1,    72,    73,    74,
      75,    76,    77,    -1,    79,    -1,    -1,    -1,    83,    84,
      85,    86,    -1,    88,    -1,    90,    -1,    92,    -1,    -1,
      95,    -1,    -1,    -1,    99,   100,   101,   102,    -1,   104,
     105,    -1,   107,    -1,   109,   110,   111,   112,   113,   114,
     115,    -1,   117,   118,   119,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   127,   128,    -1,   130,   131,   132,   133,   134,
      -1,    -1,    -1,    -1,    -1,    -1,   141,    -1,    -1,    -1,
      -1,   146,   147,   148,   149,   150,   151,    -1,   153,   154,
      -1,   156,   157,   158,    -1,    -1,    -1,   162,    -1,    -1,
     165,    -1,    -1,    -1,    -1,    -1,   171,    -1,    -1,    -1,
      -1,   176,   177,   178,    -1,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,    -1,
      -1,   196,    -1,   198,   199,    -1,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    50,    -1,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    -1,    64,    65,    66,    67,    68,    -1,    -1,    -1,
      72,    73,    74,    75,    76,    77,    -1,    79,    -1,    -1,
      -1,    83,    84,    85,    86,    -1,    88,    -1,    90,    -1,
      92,    -1,    -1,    95,    -1,    -1,    -1,    99,   100,   101,
     102,    -1,   104,   105,    -1,   107,    -1,   109,   110,   111,
     112,   113,   114,   115,    -1,   117,   118,   119,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   127,   128,    -1,   130,   131,
     132,   133,   134,    -1,    -1,    -1,    -1,    -1,    -1,   141,
      -1,    -1,    -1,    -1,   146,   147,   148,   149,   150,   151,
      -1,   153,   154,    -1,   156,   157,   158,    -1,    -1,    -1,
     162,    -1,    -1,   165,    -1,    -1,    -1,    -1,    -1,   171,
      -1,    -1,    -1,    -1,   176,   177,   178,    -1,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,    -1,    -1,   196,    -1,   198,   199,    -1,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,
      -1,    50,    -1,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    -1,    64,    65,    66,    67,    68,
      -1,    -1,    -1,    72,    73,    74,    75,    76,    77,    -1,
      79,    -1,    -1,    -1,    83,    84,    85,    86,    -1,    88,
      -1,    90,    -1,    92,    -1,    -1,    95,    -1,    -1,    -1,
      99,   100,   101,   102,    -1,   104,   105,    -1,   107,    -1,
     109,   110,   111,   112,   113,   114,   115,    -1,   117,   118,
     119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,   128,
      -1,   130,   131,   132,   133,   134,    -1,    -1,    -1,    -1,
      -1,    -1,   141,    -1,    -1,    -1,    -1,   146,   147,   148,
     149,   150,   151,    -1,   153,   154,    -1,   156,   157,   158,
      -1,    -1,    -1,   162,    -1,    -1,   165,    -1,    -1,    -1,
      -1,    -1,   171,    -1,    -1,    -1,    -1,   176,   177,   178,
      -1,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,    -1,    -1,   196,    -1,   198,
     199,    -1,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    45,
      -1,    -1,    -1,    -1,    50,    -1,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    -1,    64,    65,
      66,    67,    68,    -1,    -1,    -1,    72,    73,    74,    75,
      76,    77,    -1,    79,    -1,    -1,    -1,    83,    84,    85,
      86,    -1,    88,    -1,    90,    -1,    92,    -1,    -1,    95,
      -1,    -1,    -1,    99,   100,   101,   102,    -1,   104,   105,
      -1,   107,    -1,   109,   110,   111,   112,   113,   114,   115,
      -1,   117,   118,   119,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   127,   128,    -1,   130,   131,   132,   133,   134,    -1,
      -1,    -1,    -1,    -1,    -1,   141,    -1,    -1,    -1,    -1,
     146,   147,   148,   149,   150,   151,    -1,   153,   154,    -1,
     156,   157,   158,    -1,    -1,    -1,   162,    -1,    -1,   165,
      -1,    -1,    -1,    -1,    -1,   171,    -1,    -1,    -1,    -1,
     176,   177,   178,    -1,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,    -1,    -1,
     196,    -1,   198,   199,    -1,   201,   202,    -1,   204,   205,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    44,    45,    -1,    -1,    -1,    -1,    50,    -1,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      -1,    64,    65,    66,    67,    -1,    -1,    -1,    -1,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,
      83,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   102,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   110,   111,   112,
     113,   114,   115,    75,    -1,   118,   119,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   127,   128,    -1,   130,   131,   132,
     133,   134,    -1,    -1,    -1,    -1,    -1,    -1,   141,    -1,
      -1,    -1,    -1,   146,   147,   148,   149,   150,   151,    -1,
     153,   154,    -1,   156,   157,   158,    -1,    -1,    -1,   162,
      -1,    -1,   165,    -1,    -1,    -1,    -1,    -1,   171,    -1,
      -1,    -1,    -1,   176,   177,   178,    -1,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   153,   154,   196,   156,   157,   158,    -1,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,    -1,    -1,    33,   196,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,
      50,    -1,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    -1,    64,    65,    66,    67,    -1,    -1,
      -1,    -1,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    -1,    -1,    83,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     110,   111,   112,   113,   114,   115,    -1,    -1,   118,   119,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,   128,    -1,
     130,   131,   132,   133,   134,    -1,    -1,    -1,    -1,    -1,
      -1,   141,    -1,    -1,    -1,    -1,   146,   147,   148,   149,
      -1,   151,    -1,   153,   154,    -1,   156,   157,   158,    -1,
      -1,    -1,   162,    -1,    -1,   165,    -1,    -1,    -1,    -1,
      -1,   171,    -1,    -1,    -1,    -1,   176,    -1,   178,    -1,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,    -1,    -1,   196,    -1,    10,    11,
      12,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    27,    13,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,
      -1,    63,    -1,    50,    -1,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    -1,    64,    65,    66,
      67,    -1,    -1,    -1,    -1,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    -1,    -1,    83,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   102,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   110,   111,   112,   113,   114,   115,    -1,
      -1,   118,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     127,   128,    -1,   130,   131,   132,   133,   134,    -1,    -1,
      -1,    -1,    -1,    -1,   141,    -1,    -1,    -1,    -1,   146,
     147,   148,   149,    -1,   151,    -1,   153,   154,    -1,   156,
     157,   158,    -1,    -1,    -1,   162,    -1,    -1,   165,    -1,
      -1,    -1,    -1,    -1,   171,    -1,    -1,    -1,    -1,   176,
     192,   178,    -1,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,    -1,    -1,   196,
      -1,   198,    11,    12,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    27,    13,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      44,    45,    -1,    -1,    63,    -1,    50,    -1,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    -1,
      64,    65,    66,    67,    -1,    -1,    -1,    -1,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,    83,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   102,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   110,   111,   112,   113,
     114,   115,    -1,    -1,   118,   119,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   127,   128,    -1,   130,   131,   132,   133,
     134,    -1,    -1,    -1,    -1,    -1,    -1,   141,    -1,    -1,
      -1,    -1,   146,   147,   148,   149,    -1,   151,    -1,   153,
     154,    -1,   156,   157,   158,    -1,    -1,    -1,   162,    -1,
      -1,   165,    -1,    -1,    -1,    -1,    -1,   171,    -1,    -1,
      -1,    -1,   176,    -1,   178,    -1,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
      -1,    -1,   196,    -1,   198,    11,    12,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    27,    13,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    50,
      -1,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    -1,    64,    65,    66,    67,    -1,    -1,    -1,
      -1,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      -1,    -1,    83,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,
      -1,   102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   110,
     111,   112,   113,   114,   115,    -1,    -1,   118,   119,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   127,   128,    -1,   130,
     131,   132,   133,   134,    -1,    -1,    -1,    -1,    -1,    -1,
     141,    -1,    -1,    -1,    -1,   146,   147,   148,   149,    -1,
     151,    -1,   153,   154,    -1,   156,   157,   158,    -1,    -1,
      -1,   162,    -1,    -1,   165,    -1,    -1,    -1,    -1,    -1,
     171,    -1,    -1,    -1,    -1,   176,    -1,   178,    -1,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,    -1,    -1,   196,    -1,    10,    11,    12,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    27,    13,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    -1,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,
      63,    -1,    50,    -1,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    -1,    64,    65,    66,    67,
      -1,    -1,    -1,    -1,    72,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    -1,    -1,    83,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   102,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   110,   111,   112,   113,   114,   115,    -1,    -1,
     118,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,
     128,    -1,   130,   131,   132,   133,   134,    -1,    -1,    -1,
      -1,    -1,    -1,   141,    -1,    -1,    -1,    -1,   146,   147,
     148,   149,    -1,   151,    -1,   153,   154,    -1,   156,   157,
     158,    -1,    -1,    -1,   162,    -1,    -1,   165,    -1,    -1,
      -1,    -1,    -1,   171,    -1,   188,   189,    -1,   176,    -1,
     178,    -1,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,    -1,    -1,   196,   197,
      -1,    -1,    -1,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      45,    -1,    -1,    -1,    -1,    50,    -1,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    -1,    64,
      65,    66,    67,    -1,    -1,    -1,    -1,    72,    73,    74,
      75,    76,    77,    -1,    -1,    -1,    -1,    -1,    83,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   102,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   110,   111,   112,   113,   114,
     115,    -1,    -1,   118,   119,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   127,   128,    -1,   130,   131,   132,   133,   134,
      -1,    -1,    -1,    -1,    -1,    -1,   141,    -1,    -1,    -1,
      -1,   146,   147,   148,   149,    -1,   151,    -1,   153,   154,
      -1,   156,   157,   158,    -1,    -1,    -1,   162,    -1,    -1,
     165,    -1,    -1,    -1,    -1,    -1,   171,    -1,    -1,    -1,
      -1,   176,    -1,   178,    -1,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,    -1,
      -1,   196,    -1,    -1,    -1,    -1,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    50,    -1,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    -1,    64,    65,    66,    67,    -1,    -1,    -1,    -1,
      72,    73,    74,    75,    76,    77,    -1,    -1,    -1,    -1,
      -1,    83,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   110,   111,
     112,   113,   114,   115,    75,    -1,   118,   119,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   127,   128,    -1,   130,   131,
     132,   133,   134,    -1,    -1,    -1,    -1,    -1,    -1,   141,
      -1,    -1,    -1,    -1,   146,   147,   148,   149,    -1,   151,
      -1,   153,   154,    -1,   156,   157,   158,    -1,    -1,    -1,
     162,    -1,    -1,   165,    -1,    -1,    -1,    -1,    -1,   171,
      -1,    -1,    -1,    -1,   176,    -1,   178,    -1,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   153,   154,   196,   156,   157,   158,    -1,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,
      -1,    50,    -1,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    -1,    64,    65,    66,    67,    -1,
      -1,    -1,    -1,    72,    73,    74,    75,    76,    77,    -1,
      -1,    -1,    -1,    -1,    83,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   102,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   110,   111,   112,   113,   114,   115,    -1,    -1,   118,
     119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,   128,
      -1,   130,   131,   132,   133,   134,    -1,    -1,    -1,    -1,
      -1,    -1,   141,    -1,    -1,    -1,    -1,   146,   147,   148,
     149,    -1,   151,    -1,   153,   154,    -1,   156,   157,   158,
      -1,    -1,    -1,   162,    -1,    -1,   165,    -1,    -1,    -1,
      -1,    -1,   171,    -1,    -1,    -1,    -1,   176,    -1,   178,
      -1,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,    -1,    -1,   196,    -1,    -1,
      -1,    -1,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    33,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    45,
      -1,    -1,    -1,    -1,    50,    -1,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    -1,    64,    65,
      66,    67,    -1,    -1,    -1,    -1,    72,    73,    74,    75,
      76,    77,    -1,    -1,    -1,    -1,    -1,    83,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   102,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   110,   111,   112,   113,   114,   115,
      -1,    -1,   118,   119,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   127,   128,    -1,   130,   131,   132,   133,   134,    -1,
      -1,    -1,    -1,    -1,    -1,   141,    -1,    -1,    -1,    -1,
     146,   147,   148,   149,    -1,   151,    -1,   153,   154,    -1,
     156,   157,   158,    -1,    -1,    -1,   162,    -1,    -1,   165,
      -1,    -1,    -1,    -1,    -1,   171,    -1,    -1,    -1,    -1,
     176,    -1,   178,    -1,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,    -1,    -1,
     196,    -1,    -1,    -1,    -1,   201,   202,    -1,   204,   205,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    44,    45,    -1,    -1,    -1,    -1,    50,    -1,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      -1,    64,    65,    66,    67,    -1,    -1,    -1,    -1,    72,
      73,    74,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,
      83,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   102,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   110,   111,   112,
     113,   114,   115,    -1,    -1,   118,   119,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   127,   128,    -1,   130,   131,   132,
     133,   134,    -1,    -1,    -1,    -1,    -1,    -1,   141,    -1,
      -1,    -1,    -1,   146,   147,   148,   149,    -1,   151,    -1,
     153,   154,    -1,   156,   157,   158,    -1,    -1,    -1,   162,
      -1,    -1,   165,    -1,    -1,    -1,    -1,    -1,   171,    -1,
      -1,    -1,    -1,   176,    -1,   178,    -1,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,    -1,    -1,   196,    -1,    10,    11,    12,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    27,    13,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    -1,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    63,    -1,
      50,    -1,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    -1,    64,    65,    66,    67,    -1,    -1,
      -1,    -1,    72,    73,    74,    75,    76,    77,    -1,    -1,
      -1,    -1,    -1,    83,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     110,   111,   112,   113,   114,   115,    -1,    -1,   118,   119,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,   128,    -1,
     130,   131,   132,   133,   134,    -1,    -1,    -1,    -1,    -1,
      -1,   141,    -1,    -1,    -1,    -1,   146,   147,   148,   149,
      -1,   151,    -1,   153,   154,    -1,   156,   157,   158,    -1,
      -1,    -1,   162,    -1,    -1,   165,    -1,    -1,    -1,   184,
      -1,   171,    -1,    -1,    -1,    -1,   176,    -1,   178,    -1,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,    -1,    -1,   196,    -1,    -1,   199,
      -1,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,
      -1,    -1,    -1,    50,    -1,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    -1,    64,    65,    66,
      67,    -1,    -1,    -1,    -1,    72,    73,    74,    75,    76,
      77,    -1,    -1,    -1,    -1,    -1,    83,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   102,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   110,   111,   112,   113,   114,   115,    -1,
      -1,   118,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     127,   128,    -1,   130,   131,   132,   133,   134,    -1,    -1,
      -1,    -1,    -1,    -1,   141,    -1,    -1,    -1,    -1,   146,
     147,   148,   149,    -1,   151,    -1,   153,   154,    -1,   156,
     157,   158,    -1,    -1,    -1,   162,    -1,    -1,   165,    -1,
      -1,    -1,    -1,    -1,   171,    -1,    -1,    -1,    -1,   176,
      -1,   178,    -1,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,    -1,    -1,   196,
      -1,    -1,    -1,    -1,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    33,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      44,    45,    -1,    -1,    -1,    -1,    50,    -1,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    -1,
      64,    65,    66,    67,    -1,    -1,    -1,    -1,    72,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,    83,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   102,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   110,   111,   112,   113,
     114,   115,    -1,    -1,   118,   119,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   127,   128,    -1,   130,   131,   132,   133,
     134,    -1,    -1,    -1,    -1,    -1,    -1,   141,    -1,    -1,
      -1,    -1,   146,   147,   148,   149,    -1,   151,    -1,   153,
     154,    -1,   156,   157,   158,    -1,    -1,    -1,   162,    -1,
      -1,   165,    -1,    -1,    -1,    -1,    -1,   171,    -1,    -1,
      -1,    -1,   176,    -1,   178,    -1,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
      -1,    -1,   196,    -1,    10,    11,    12,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    27,    13,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    -1,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    44,    45,    -1,    -1,    63,    -1,    50,
      -1,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    -1,    64,    65,    66,    67,    -1,    -1,    -1,
      -1,    72,    73,    74,    75,    76,    77,    -1,    -1,    -1,
      -1,    -1,    83,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   110,
     111,   112,   113,   114,   115,    -1,    -1,   118,   119,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   127,   128,    -1,   130,
     131,   132,   133,   134,    -1,    -1,    -1,    -1,    -1,    -1,
     141,    -1,    -1,    -1,    -1,   146,   147,   148,   149,    -1,
     151,    -1,   153,   154,    -1,   156,   157,   158,    -1,    -1,
      -1,   162,    -1,    -1,   165,    -1,    -1,   183,    -1,    -1,
     171,    -1,    -1,    -1,    -1,   176,    -1,   178,    -1,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,    -1,    -1,   196,    -1,    10,    11,    12,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    27,    13,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    -1,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,
      63,    -1,    50,    -1,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    -1,    64,    65,    66,    67,
      -1,    -1,    -1,    -1,    72,    73,    74,    75,    76,    77,
      -1,    94,    -1,    -1,    -1,    83,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   102,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   110,   111,   112,   113,   114,   115,    -1,    -1,
     118,   119,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,
     128,    -1,   130,   131,   132,   133,   134,    -1,    -1,    -1,
      -1,    -1,    -1,   141,    -1,    -1,    -1,    -1,   146,   147,
     148,   149,    75,   151,    -1,   153,   154,    -1,   156,   157,
     158,    -1,    -1,    -1,   162,    -1,    -1,   165,    -1,    -1,
      -1,    -1,    -1,   171,    -1,    -1,    -1,    -1,   176,    -1,
     178,    -1,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,    -1,    -1,   196,    -1,
      -1,    -1,    -1,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,   146,    -1,    -1,   149,    -1,    -1,    -1,
     153,   154,    -1,   156,   157,   158,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    51,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
      65,    66,    67,    68,    69,    70,    71,   200,    -1,    -1,
      75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
      -1,    -1,   127,   128,    -1,   130,   131,   132,   133,   134,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   146,   147,   148,    -1,   150,    -1,    -1,   153,   154,
      -1,   156,   157,   158,   159,    -1,   161,   162,    -1,   164,
      -1,    -1,    -1,    -1,    -1,    -1,   171,   172,    -1,   174,
      -1,   176,   177,   178,    -1,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    -1,    51,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    63,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      -1,    51,    10,    11,    12,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    63,    75,    -1,    77,    -1,    -1,    27,
      -1,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    -1,    51,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   115,    63,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    27,   146,    -1,    -1,   149,    -1,
     151,    -1,   153,   154,    -1,   156,   157,   158,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,    75,    -1,    -1,   196,    -1,    -1,    -1,    -1,
     201,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   102,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   110,   111,   112,
     113,   114,   115,    -1,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   128,   129,    -1,    -1,   197,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    -1,    -1,
      -1,    -1,    -1,   146,    -1,    -1,   149,    -1,   151,    -1,
     153,   154,    -1,   156,   157,   158,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   102,    -1,    27,    -1,   171,    -1,
      -1,   197,    -1,    -1,    -1,   178,    -1,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     128,   129,    53,   196,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,
      -1,   149,    -1,   151,    75,   153,   154,    -1,   156,   157,
     158,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,   171,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,    -1,    -1,    53,   196,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,
      -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,   149,    -1,
     151,    -1,   153,   154,    -1,   156,   157,   158,    -1,   160,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,
     171,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   128,   129,    53,   196,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     146,    -1,    -1,   149,    -1,   151,    75,   153,   154,    -1,
     156,   157,   158,    -1,   160,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   171,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,    -1,    -1,    -1,
     196,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      44,    45,    -1,    -1,    -1,    -1,    50,   146,    52,    -1,
     149,    -1,   151,    -1,   153,   154,    -1,   156,   157,   158,
      64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    72,    73,
      74,    75,   171,    -1,    -1,    -1,    -1,    -1,    -1,    83,
      -1,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,    -1,    -1,    -1,   196,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   128,    -1,   130,   131,   132,   133,
     134,    -1,    75,    -1,    77,    -1,    -1,   141,    -1,    -1,
      -1,    -1,   146,   147,   148,   149,    -1,   151,    -1,   153,
     154,    -1,   156,   157,   158,    -1,    -1,    -1,   162,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   171,    -1,    -1,
      -1,    -1,   176,    -1,    -1,    -1,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,    44,
      45,    -1,   196,    -1,    -1,    50,    -1,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    64,
     153,   154,    -1,   156,   157,   158,    -1,    72,    73,    74,
      75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    83,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
      -1,    -1,    -1,    -1,    -1,    -1,   199,    -1,   201,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   128,    -1,   130,   131,   132,   133,   134,
      44,    45,    -1,    -1,    -1,    -1,   141,    -1,    -1,    -1,
      -1,   146,   147,   148,   149,    -1,   151,    -1,   153,   154,
      64,   156,   157,   158,    -1,    -1,    -1,   162,    72,    73,
      74,    75,    -1,    -1,    -1,    -1,   171,    -1,    -1,    83,
      -1,   176,    -1,    -1,    -1,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    -1,    -1,
      -1,   196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   128,    -1,   130,   131,   132,   133,
     134,    44,    45,    -1,    -1,    -1,    -1,   141,    -1,    -1,
      -1,    -1,   146,   147,   148,   149,    -1,   151,    -1,   153,
     154,    64,   156,   157,   158,    -1,    -1,    -1,   162,    72,
      73,    74,    75,    -1,    -1,    -1,    -1,   171,    -1,    -1,
      83,    -1,   176,    -1,    -1,    -1,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,
      -1,    77,    -1,    -1,    -1,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     153,   154,    -1,   156,   157,   158,    -1,    -1,    -1,   115,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   171,    -1,
      75,    -1,    77,    -1,    -1,    -1,    -1,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     146,    -1,    -1,   149,    -1,   151,    -1,   153,   154,    -1,
     156,   157,   158,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,
      -1,    77,   127,    -1,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,    -1,    -1,    -1,
     196,   146,    -1,   199,   149,   201,   151,    -1,   153,   154,
      -1,   156,   157,   158,    -1,    -1,    -1,    -1,    -1,   115,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    -1,
      77,    -1,    -1,    -1,    -1,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    -1,    -1,
     146,   196,    -1,   149,    -1,   151,   201,   153,   154,    -1,
     156,   157,   158,    -1,    -1,    -1,    -1,    -1,   115,    75,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,    -1,    -1,   146,
     196,    -1,   149,    -1,   151,   201,   153,   154,    -1,   156,
     157,   158,    -1,    -1,    -1,    72,    73,    74,    75,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,    -1,   153,   154,   196,
     156,   157,   158,    -1,   201,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,    -1,    -1,    -1,
      -1,    -1,    -1,   199,    -1,   201,   153,   154,    -1,   156,
     157,   158,    -1,    -1,    -1,    -1,   116,    -1,    -1,    -1,
      75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   128,   129,
      -1,    -1,    -1,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   146,   102,   103,   149,
      -1,   151,    -1,   153,   154,    -1,   156,   157,   158,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,    -1,   149,    -1,    -1,    -1,   153,   154,
      -1,   156,   157,   158,    -1,    -1,    -1,    75,    -1,    77,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      75,    -1,    -1,    -1,    -1,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   146,    -1,
      -1,   149,    -1,   151,    -1,   153,   154,    -1,   156,   157,
     158,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   153,   154,    -1,   156,   157,
     158,    -1,    -1,    -1,    -1,    -1,   151,    -1,   153,   154,
      -1,   156,   157,   158,    -1,    -1,    -1,    -1,    75,    -1,
      -1,    -1,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   149,    -1,
      -1,    -1,   153,   154,    -1,   156,   157,   158,    -1,    -1,
      -1,    -1,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   149,    -1,    -1,    -1,   153,   154,    -1,   156,
     157,   158,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   121,    -1,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   149,    -1,    -1,    -1,
     153,   154,    -1,   156,   157,   158,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   153,   154,    -1,   156,   157,   158,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    -1,    51,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    -1,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,   126,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    -1,    51,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,    -1,    -1,
      27,   126,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    -1,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,   126,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    -1,
      51,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    63,    -1,    -1,    -1,    -1,    -1,    27,   126,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,   126,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,   126,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,   126,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   126,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    -1,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    63,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,   126,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    -1,    51,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,
      -1,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   207,   208,     0,   209,     3,     4,     5,     6,     7,
      13,    43,    44,    45,    50,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    64,    65,    66,    67,
      68,    72,    73,    74,    75,    76,    77,    79,    83,    84,
      85,    86,    88,    90,    92,    95,    99,   100,   101,   102,
     103,   104,   105,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   117,   118,   119,   120,   121,   122,   127,   128,
     130,   131,   132,   133,   134,   141,   146,   147,   148,   149,
     150,   151,   153,   154,   156,   157,   158,   159,   162,   165,
     171,   172,   174,   176,   177,   178,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     196,   198,   199,   201,   202,   204,   205,   210,   213,   220,
     221,   222,   223,   224,   225,   228,   243,   244,   248,   253,
     259,   314,   315,   320,   324,   325,   326,   327,   328,   329,
     330,   331,   333,   336,   345,   346,   347,   349,   350,   352,
     371,   381,   382,   383,   388,   392,   410,   415,   417,   418,
     419,   420,   421,   422,   423,   424,   426,   439,   441,   443,
     113,   114,   115,   127,   146,   213,   243,   314,   330,   417,
     330,   196,   330,   330,   330,   408,   409,   330,   330,   330,
     330,   330,   330,   330,   330,   330,   330,   330,   330,    77,
     115,   196,   221,   382,   383,   417,   417,    33,   330,   430,
     431,   330,   115,   196,   221,   382,   383,   384,   416,   422,
     427,   428,   196,   321,   385,   196,   321,   337,   322,   330,
     230,   321,   196,   196,   196,   321,   198,   330,   213,   198,
     330,    27,    53,   128,   129,   151,   171,   196,   213,   224,
     444,   454,   455,   179,   198,   327,   330,   351,   353,   199,
     236,   330,   102,   103,   149,   214,   217,   220,    77,   201,
     285,   286,   121,   121,    77,   287,   196,   196,   196,   196,
     213,   257,   445,   196,   196,    77,    82,   142,   143,   144,
     436,   437,   149,   199,   220,   220,    99,   330,   258,   445,
     151,   196,   445,   445,   330,   338,   320,   330,   331,   417,
     226,   199,    82,   386,   436,    82,   436,   436,    28,   149,
     167,   446,   196,     9,   198,    33,   242,   151,   256,   445,
     115,   243,   315,   198,   198,   198,   198,   198,   198,    10,
      11,    12,    27,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    51,    63,   198,    64,    64,   198,
     199,   145,   122,   159,   259,   313,   314,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      61,    62,   125,   412,   413,    64,   199,   414,   196,    64,
     199,   201,   423,   196,   242,   243,    14,   330,    42,   213,
     407,   196,   320,   417,   145,   417,   126,   203,     9,   394,
     320,   417,   446,   145,   196,   387,   125,   412,   413,   414,
     197,   330,    28,   228,     8,   339,     9,   198,   228,   229,
     322,   323,   330,   213,   271,   232,   198,   198,   198,   455,
     455,   167,   196,   102,   447,   455,    14,   213,    77,   198,
     198,   198,   179,   180,   181,   186,   187,   190,   191,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   366,   367,
     368,   237,   106,   164,   198,   149,   215,   218,   220,   149,
     216,   219,   220,   220,     9,   198,    94,   199,   417,     9,
     198,    14,     9,   198,   417,   440,   440,   320,   331,   417,
     197,   167,   251,   127,   417,   429,   430,    64,   125,   142,
     437,    76,   330,   417,    82,   142,   437,   220,   212,   198,
     199,   198,   126,   254,   372,   374,    83,   340,   341,   343,
      14,    94,   442,   281,   282,   410,   411,   197,   197,   197,
     200,   227,   228,   244,   248,   253,   330,   202,   204,   205,
     213,   447,    33,   283,   284,   330,   444,   196,   445,   249,
     242,   330,   330,   330,    28,   330,   330,   330,   330,   330,
     330,   330,   330,   330,   330,   330,   330,   330,   330,   330,
     330,   330,   330,   330,   330,   330,   330,   384,   330,   330,
     425,   425,   330,   432,   433,   121,   199,   213,   422,   423,
     257,   258,   256,   243,    33,   150,   324,   327,   330,   351,
     330,   330,   330,   330,   330,   330,   330,   330,   330,   330,
     330,   330,   199,   213,   422,   425,   330,   283,   425,   330,
     429,   242,   197,   196,   406,     9,   394,   320,   197,   213,
      33,   330,    33,   330,   197,   197,   422,   283,   199,   213,
     422,   197,   226,   275,   199,   330,   330,    86,    28,   228,
     269,   198,    94,    14,     9,   197,    28,   199,   272,   455,
      83,   451,   452,   453,   196,     9,    44,    45,    50,    52,
      64,    83,   128,   141,   151,   171,   196,   221,   222,   224,
     348,   382,   388,   389,   390,   391,   182,    77,   330,    77,
      77,   330,   363,   364,   330,   330,   356,   366,   185,   369,
     226,   196,   235,   220,   198,     9,    94,   220,   198,     9,
      94,    94,   217,   213,   330,   286,   390,    77,     9,   197,
     197,   197,   197,   197,   198,   213,   450,   123,   262,   196,
       9,   197,   197,    77,    78,   213,   438,   213,    64,   200,
     200,   209,   211,   330,   124,   261,   166,    48,   151,   166,
     376,   126,     9,   394,   197,   455,   455,    14,   194,     9,
     395,   455,   456,   125,   412,   413,   414,   200,     9,   168,
     417,   197,     9,   395,    14,   334,   245,   123,   260,   196,
     445,   330,    28,   203,   203,   126,   200,     9,   394,   330,
     446,   196,   252,   255,   250,   242,    66,   417,   330,   446,
     196,   203,   200,   197,   203,   200,   197,    44,    45,    64,
      72,    73,    74,    83,   128,   141,   171,   213,   397,   399,
     402,   405,   213,   417,   417,   126,   412,   413,   414,   197,
     330,   276,    69,    70,   277,   226,   321,   226,   323,    33,
     127,   266,   417,   390,   213,    28,   228,   270,   198,   273,
     198,   273,     9,   168,   126,     9,   394,   197,   160,   447,
     448,   455,   389,   389,   390,   390,   390,   393,   396,   196,
      82,   145,   196,   390,   145,   199,    10,    11,    12,    27,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,   330,   182,   182,    14,   188,   189,   365,     9,   192,
     369,    77,   200,   382,   199,   239,    94,   218,   213,    94,
     219,   213,   213,   200,    14,   417,   198,    94,     9,   168,
     263,   382,   199,   429,   127,   417,    14,   203,   330,   200,
     209,   263,   199,   375,    14,   330,   340,   198,   455,    28,
     449,   411,    33,    77,   160,   199,   213,   422,   455,    33,
     330,   390,   281,   196,   382,   261,   335,   246,   330,   330,
     330,   200,   196,   283,   262,   261,   260,   445,   384,   200,
     196,   283,    14,    72,    73,    74,   213,   398,   398,   399,
     400,   401,   196,    82,   142,   196,     9,   394,   197,   406,
      33,   330,   200,    69,    70,   278,   321,   228,   200,   198,
      87,   198,   417,   196,   126,   265,    14,   226,   273,    96,
      97,    98,   273,   200,   455,   455,   451,     9,   197,   197,
     126,   203,     9,   394,   393,   213,   340,   342,   344,   197,
     121,   213,   390,   434,   435,   390,   390,   390,    28,   390,
     390,   390,   390,   390,   390,   390,   390,   390,   390,   389,
     389,   390,   390,   390,   390,   390,   390,   390,   390,   390,
     390,   330,   330,   330,   364,   330,   354,    77,   240,   213,
     213,   390,   455,   213,     9,   288,   197,   196,   324,   327,
     330,   203,   200,   288,   152,   165,   199,   371,   378,   152,
     199,   377,   126,   198,   455,   339,   456,    77,    14,    77,
     330,   446,   196,   417,   197,   281,   199,   281,   196,   126,
     196,   283,   197,   199,   199,   261,   247,   387,   196,   283,
     197,   126,   203,     9,   394,   400,   142,   340,   403,   404,
     399,   417,   321,    28,    71,   228,   198,   323,   429,   266,
     197,   390,    93,    96,   198,   330,    28,   198,   274,   200,
     168,   160,    28,   390,   390,   197,   126,     9,   394,   197,
     126,   200,     9,   394,   390,    28,   183,   197,   226,    94,
     382,     4,   103,   108,   116,   153,   154,   156,   200,   289,
     312,   313,   314,   319,   410,   429,   200,   200,    48,   330,
     330,   330,    33,    77,   160,    14,   390,   200,   196,   283,
     449,   197,   288,   197,   281,   330,   283,   197,   288,   288,
     199,   196,   283,   197,   399,   399,   197,   126,   197,     9,
     394,    28,   226,   198,   197,   197,   233,   198,   198,   274,
     226,   455,   126,   390,   340,   390,   390,   390,   330,   199,
     200,   455,   123,   124,   444,   264,   382,   116,   128,   129,
     151,   157,   298,   299,   300,   382,   155,   304,   305,   119,
     196,   213,   306,   307,   290,   243,   455,     9,   198,   313,
     197,   151,   373,   200,   200,    77,    14,    77,   390,   196,
     283,   197,   108,   332,   449,   200,   449,   197,   197,   200,
     200,   288,   281,   197,   126,   399,   340,   226,   231,    28,
     228,   268,   226,   197,   390,   126,   126,   184,   226,   382,
     382,    14,     9,   198,   199,   199,     9,   198,     3,     4,
       5,     6,     7,    10,    11,    12,    13,    51,    65,    66,
      67,    68,    69,    70,    71,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   127,   128,   130,   131,
     132,   133,   134,   146,   147,   148,   150,   159,   161,   162,
     164,   171,   172,   174,   176,   177,   178,   213,   379,   380,
       9,   198,   151,   155,   213,   307,   308,   309,   198,    77,
     318,   242,   291,   444,   243,    14,   390,   283,   197,   196,
     199,   198,   199,   310,   332,   449,   200,   197,   399,   126,
      28,   228,   267,   226,   390,   390,   330,   200,   198,   198,
     390,   382,   294,   301,   388,   299,    14,    28,    45,   302,
     305,     9,    31,   197,    27,    44,    47,    14,     9,   198,
     445,   318,    14,   242,   390,   197,    33,    77,   370,   226,
     226,   199,   310,   449,   399,   226,    91,   185,   238,   200,
     213,   224,   295,   296,   297,     9,   200,   390,   380,   380,
      53,   303,   308,   308,    27,    44,    47,   390,    77,   196,
     198,   390,   445,    77,     9,   395,   200,   200,   226,   310,
      89,   198,    77,   106,   234,   145,    94,   388,   158,    14,
     292,   196,    33,    77,   197,   200,   198,   196,   164,   241,
     213,   313,   314,   390,   279,   280,   411,   293,    77,   382,
     239,   161,   213,   198,   197,     9,   395,   110,   111,   112,
     316,   317,   279,    77,   264,   198,   449,   411,   456,   197,
     197,   198,   198,   199,   311,   316,    33,    77,   160,   449,
     199,   226,   456,    77,    14,    77,   311,   226,   200,    33,
      77,   160,    14,   390,   200,    77,    14,    77,   390,    14,
     390,   390
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
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 754 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 757 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 759 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 760 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 761 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 762 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 763 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 765 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 767 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 768 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
#line 796 "hphp.y"
    { ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 797 "hphp.y"
    { ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 802 "hphp.y"
    { ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 803 "hphp.y"
    { ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 808 "hphp.y"
    { ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 809 "hphp.y"
    { ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 813 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 814 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 815 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 817 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 821 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 822 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 823 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 825 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 829 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 830 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 831 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 833 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 842 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 844 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 845 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 848 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 855 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 862 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 870 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 873 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 879 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 880 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 883 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 884 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
#line 889 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 893 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 898 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 899 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 901 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 905 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 908 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 912 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 914 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 917 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 919 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 922 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 923 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 924 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 925 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 926 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 928 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 929 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 930 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 931 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 933 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 934 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 937 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 939 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 943 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 950 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 951 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 954 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 955 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 956 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 957 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval));;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 961 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 962 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 963 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 964 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 965 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 966 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 967 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 968 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 969 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 970 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 971 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 979 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 980 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 989 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 990 "hphp.y"
    { (yyval).reset();;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 994 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 996 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1003 "hphp.y"
    { (yyval).reset();;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1008 "hphp.y"
    { (yyval).reset();;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1017 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1023 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1029 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1035 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1041 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1047 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1055 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1058 "hphp.y"
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

  case 137:

/* Line 1455 of yacc.c  */
#line 1073 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1076 "hphp.y"
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

  case 139:

/* Line 1455 of yacc.c  */
#line 1090 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1093 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1098 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1101 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1108 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1111 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1119 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1122 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1130 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1131 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1135 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1138 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1141 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1142 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1143 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1147 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1148 "hphp.y"
    { (yyval).reset();;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1151 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1152 "hphp.y"
    { (yyval).reset();;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1155 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1156 "hphp.y"
    { (yyval).reset();;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1159 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1161 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1164 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1166 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1170 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1171 "hphp.y"
    { (yyval).reset();;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1174 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1175 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1176 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1180 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1182 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1185 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1187 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1190 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1192 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1195 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1197 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1207 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1208 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1209 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1210 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1215 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1217 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1218 "hphp.y"
    { (yyval).reset();;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1221 "hphp.y"
    { (yyval).reset();;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1222 "hphp.y"
    { (yyval).reset();;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1227 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1228 "hphp.y"
    { (yyval).reset();;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1233 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1234 "hphp.y"
    { (yyval).reset();;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1237 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1238 "hphp.y"
    { (yyval).reset();;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1241 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1242 "hphp.y"
    { (yyval).reset();;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1250 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1256 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1264 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1269 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1272 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1278 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1282 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1287 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1292 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1297 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1302 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1308 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1314 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1322 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1327 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1331 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1334 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1338 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1341 "hphp.y"
    { (yyval).reset();;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1346 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1349 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1353 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1357 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1361 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1365 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1370 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1375 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1381 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1382 "hphp.y"
    { (yyval).reset();;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1385 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),0);;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1386 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),1);;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1388 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1390 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1394 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1395 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1398 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1399 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1400 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1404 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1406 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1407 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1408 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1413 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1414 "hphp.y"
    { (yyval).reset();;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1417 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1418 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1421 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1422 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1424 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1428 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1434 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1441 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1447 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1452 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1454 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1456 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1458 "hphp.y"
    { _p->onTraitRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1460 "hphp.y"
    { _p->onTraitRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1461 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1464 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1467 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1468 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1469 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1475 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1479 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1482 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1489 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1490 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1495 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1498 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1505 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1507 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1511 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1512 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1518 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1520 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1521 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1525 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1527 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1531 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1537 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1555 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1557 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1563 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1568 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1580 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1584 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1588 "hphp.y"
    { (yyval).reset();;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1589 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1593 "hphp.y"
    { (yyval).reset();;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1594 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1597 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1598 "hphp.y"
    { (yyval).reset();;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1601 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { (yyval).reset();;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1605 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1607 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1610 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1615 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1620 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1621 "hphp.y"
    { (yyval).reset();;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1624 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1625 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1626 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1630 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1632 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1633 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1634 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1638 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1640 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1644 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1646 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1647 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1649 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1652 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1657 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1661 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1662 "hphp.y"
    { (yyval).reset();;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1666 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1667 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1671 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1676 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1680 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1684 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1689 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1693 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1694 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1695 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1699 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1700 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1701 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1704 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1708 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1709 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1710 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1713 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1714 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1716 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1717 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1718 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1719 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1721 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1722 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1723 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1724 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1725 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1726 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1727 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1728 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1729 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1730 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1731 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1732 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1735 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1736 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1739 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1744 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1745 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1749 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1761 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { (yyval).reset();;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { Token v; Token w;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1873 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1876 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1888 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1892 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1897 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1968 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1981 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1986 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1990 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1994 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1995 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 2000 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 2004 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 2005 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 2009 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 2010 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 2014 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2018 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2022 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2026 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2027 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2028 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2029 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2036 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2039 "hphp.y"
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

  case 489:

/* Line 1455 of yacc.c  */
#line 2050 "hphp.y"
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

  case 490:

/* Line 1455 of yacc.c  */
#line 2061 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2062 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2067 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2068 "hphp.y"
    { (yyval).reset();;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2071 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2072 "hphp.y"
    { (yyval).reset();;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2075 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2079 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2082 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2085 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2092 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2093 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2097 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2099 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2101 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2104 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2105 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2106 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2107 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2108 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2109 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2110 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2111 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2112 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2114 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2115 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2116 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2117 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2118 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2120 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2121 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2123 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2124 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2125 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2126 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2127 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2128 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2129 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2132 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2133 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2135 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2136 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2137 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2139 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2140 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2141 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2142 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2146 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2147 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2148 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2149 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2150 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2151 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2153 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2154 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2155 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2156 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2159 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2160 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2161 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2162 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2163 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2164 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2165 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2166 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2167 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2168 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2169 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2170 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2171 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2173 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2174 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2175 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2176 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2177 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2178 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2179 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2181 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2182 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2183 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2193 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2204 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { (yyval).reset();;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2210 "hphp.y"
    { (yyval).reset();;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2214 "hphp.y"
    { (yyval).reset();;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2215 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2216 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2221 "hphp.y"
    { (yyval).reset();;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2227 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2236 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { (yyval).reset();;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2338 "hphp.y"
    { (yyval).reset();;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { (yyval).reset();;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2343 "hphp.y"
    { (yyval).reset();;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2349 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2353 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { (yyval).reset();;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { (yyval).reset();;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { (yyval).reset();;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2465 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2466 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2479 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2485 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2486 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2487 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2488 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2493 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2496 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2503 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2505 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2510 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2522 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2528 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2540 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2547 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2551 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2559 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2567 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2572 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2576 "hphp.y"
    { (yyval).reset();;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2580 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2581 "hphp.y"
    { (yyval)++;;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2589 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2593 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2597 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2599 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2601 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2610 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2617 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2618 "hphp.y"
    { (yyval).reset();;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2624 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2628 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2630 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2631 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2632 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2638 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2643 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2644 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2645 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2650 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2656 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2660 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2661 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2665 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2667 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2675 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2677 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2679 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2681 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2683 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2684 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2687 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2688 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2689 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2693 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2694 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2695 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2696 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2697 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2698 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2699 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2700 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2701 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2705 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2706 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2711 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2713 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2727 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2731 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2737 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2738 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2744 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2748 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2755 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2759 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2762 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2769 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2770 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2774 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2775 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2780 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); ;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2781 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); ;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2783 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); ;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2784 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2790 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2803 "hphp.y"
    {;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2807 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2814 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2817 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2820 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2821 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2824 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2827 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2829 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2832 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2835 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2841 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2845 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (5)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2853 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2854 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 12876 "hphp.tab.cpp"
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
#line 2857 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

