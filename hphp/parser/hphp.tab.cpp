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
#include <boost/lexical_cast.hpp>
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
     T_CHARACTER = 318,
     T_BAD_CHARACTER = 319,
     T_ENCAPSED_AND_WHITESPACE = 320,
     T_CONSTANT_ENCAPSED_STRING = 321,
     T_ECHO = 322,
     T_DO = 323,
     T_WHILE = 324,
     T_ENDWHILE = 325,
     T_FOR = 326,
     T_ENDFOR = 327,
     T_FOREACH = 328,
     T_ENDFOREACH = 329,
     T_DECLARE = 330,
     T_ENDDECLARE = 331,
     T_AS = 332,
     T_SWITCH = 333,
     T_ENDSWITCH = 334,
     T_CASE = 335,
     T_DEFAULT = 336,
     T_BREAK = 337,
     T_GOTO = 338,
     T_CONTINUE = 339,
     T_FUNCTION = 340,
     T_CONST = 341,
     T_RETURN = 342,
     T_TRY = 343,
     T_CATCH = 344,
     T_THROW = 345,
     T_USE = 346,
     T_GLOBAL = 347,
     T_PUBLIC = 348,
     T_PROTECTED = 349,
     T_PRIVATE = 350,
     T_FINAL = 351,
     T_ABSTRACT = 352,
     T_STATIC = 353,
     T_VAR = 354,
     T_UNSET = 355,
     T_ISSET = 356,
     T_EMPTY = 357,
     T_HALT_COMPILER = 358,
     T_CLASS = 359,
     T_INTERFACE = 360,
     T_EXTENDS = 361,
     T_IMPLEMENTS = 362,
     T_OBJECT_OPERATOR = 363,
     T_DOUBLE_ARROW = 364,
     T_LIST = 365,
     T_ARRAY = 366,
     T_CALLABLE = 367,
     T_CLASS_C = 368,
     T_METHOD_C = 369,
     T_FUNC_C = 370,
     T_LINE = 371,
     T_FILE = 372,
     T_COMMENT = 373,
     T_DOC_COMMENT = 374,
     T_OPEN_TAG = 375,
     T_OPEN_TAG_WITH_ECHO = 376,
     T_CLOSE_TAG = 377,
     T_WHITESPACE = 378,
     T_START_HEREDOC = 379,
     T_END_HEREDOC = 380,
     T_DOLLAR_OPEN_CURLY_BRACES = 381,
     T_CURLY_OPEN = 382,
     T_DOUBLE_COLON = 383,
     T_NAMESPACE = 384,
     T_NS_C = 385,
     T_DIR = 386,
     T_NS_SEPARATOR = 387,
     T_XHP_LABEL = 388,
     T_XHP_TEXT = 389,
     T_XHP_ATTRIBUTE = 390,
     T_XHP_CATEGORY = 391,
     T_XHP_CATEGORY_LABEL = 392,
     T_XHP_CHILDREN = 393,
     T_ENUM = 394,
     T_XHP_REQUIRED = 395,
     T_TRAIT = 396,
     T_ELLIPSIS = 397,
     T_INSTEADOF = 398,
     T_TRAIT_C = 399,
     T_HH_ERROR = 400,
     T_FINALLY = 401,
     T_XHP_TAG_LT = 402,
     T_XHP_TAG_GT = 403,
     T_TYPELIST_LT = 404,
     T_TYPELIST_GT = 405,
     T_UNRESOLVED_LT = 406,
     T_COLLECTION = 407,
     T_SHAPE = 408,
     T_VARRAY = 409,
     T_MIARRAY = 410,
     T_MSARRAY = 411,
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
#define YYLAST   16601

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  209
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  273
/* YYNRULES -- Number of rules.  */
#define YYNRULES  922
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1746

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
     100,   102,   106,   108,   112,   114,   118,   120,   122,   125,
     129,   134,   136,   139,   143,   148,   150,   153,   157,   162,
     164,   168,   170,   174,   177,   179,   182,   185,   191,   196,
     199,   200,   202,   204,   206,   208,   212,   218,   227,   228,
     233,   234,   241,   242,   253,   254,   259,   262,   266,   269,
     273,   276,   280,   284,   288,   292,   296,   302,   304,   306,
     307,   317,   318,   329,   335,   336,   350,   351,   357,   361,
     365,   368,   371,   374,   377,   380,   383,   387,   390,   393,
     397,   400,   401,   406,   416,   417,   418,   423,   426,   427,
     429,   430,   432,   433,   443,   444,   455,   456,   468,   469,
     479,   480,   491,   492,   501,   502,   512,   513,   521,   522,
     531,   532,   540,   541,   550,   552,   554,   556,   558,   560,
     563,   566,   569,   570,   573,   574,   577,   578,   580,   584,
     586,   590,   593,   594,   596,   599,   604,   606,   611,   613,
     618,   620,   625,   627,   632,   636,   642,   646,   651,   656,
     662,   668,   673,   674,   676,   678,   683,   684,   690,   691,
     694,   695,   699,   700,   708,   715,   718,   724,   729,   730,
     735,   741,   749,   756,   763,   771,   781,   790,   797,   803,
     806,   811,   815,   816,   820,   825,   832,   838,   844,   851,
     860,   868,   871,   872,   874,   877,   880,   884,   889,   894,
     898,   900,   902,   905,   910,   914,   920,   922,   926,   929,
     930,   933,   937,   940,   941,   942,   947,   948,   954,   957,
     958,   969,   970,   982,   986,   990,   994,   999,  1004,  1008,
    1014,  1017,  1020,  1021,  1028,  1034,  1039,  1043,  1045,  1047,
    1051,  1056,  1058,  1060,  1062,  1064,  1069,  1071,  1073,  1077,
    1080,  1081,  1084,  1085,  1087,  1091,  1093,  1095,  1097,  1099,
    1103,  1108,  1113,  1118,  1120,  1122,  1125,  1128,  1131,  1135,
    1139,  1141,  1143,  1145,  1147,  1151,  1153,  1157,  1159,  1161,
    1163,  1164,  1166,  1169,  1171,  1173,  1175,  1177,  1179,  1181,
    1183,  1185,  1186,  1188,  1190,  1192,  1196,  1202,  1204,  1208,
    1214,  1219,  1223,  1227,  1230,  1232,  1234,  1238,  1242,  1244,
    1246,  1247,  1250,  1255,  1259,  1266,  1269,  1273,  1280,  1282,
    1284,  1286,  1293,  1297,  1302,  1309,  1313,  1317,  1321,  1325,
    1329,  1333,  1337,  1341,  1345,  1349,  1353,  1357,  1360,  1363,
    1366,  1369,  1373,  1377,  1381,  1385,  1389,  1393,  1397,  1401,
    1405,  1409,  1413,  1417,  1421,  1425,  1429,  1433,  1437,  1440,
    1443,  1446,  1449,  1453,  1457,  1461,  1465,  1469,  1473,  1477,
    1481,  1485,  1489,  1495,  1500,  1502,  1505,  1508,  1511,  1514,
    1517,  1520,  1523,  1526,  1529,  1531,  1533,  1535,  1537,  1539,
    1543,  1546,  1548,  1550,  1552,  1558,  1559,  1560,  1572,  1573,
    1586,  1587,  1591,  1592,  1597,  1598,  1605,  1606,  1614,  1617,
    1620,  1625,  1627,  1629,  1635,  1639,  1645,  1649,  1652,  1653,
    1656,  1657,  1662,  1667,  1671,  1676,  1681,  1686,  1691,  1696,
    1701,  1706,  1711,  1716,  1721,  1723,  1725,  1727,  1731,  1734,
    1738,  1743,  1746,  1750,  1752,  1755,  1757,  1760,  1762,  1764,
    1766,  1768,  1770,  1772,  1777,  1782,  1785,  1794,  1805,  1808,
    1810,  1814,  1816,  1819,  1821,  1823,  1825,  1827,  1830,  1835,
    1839,  1843,  1848,  1850,  1853,  1858,  1861,  1868,  1869,  1871,
    1876,  1877,  1880,  1881,  1883,  1885,  1889,  1891,  1895,  1897,
    1899,  1903,  1907,  1909,  1911,  1913,  1915,  1917,  1919,  1921,
    1923,  1925,  1927,  1929,  1931,  1933,  1935,  1937,  1939,  1941,
    1943,  1945,  1947,  1949,  1951,  1953,  1955,  1957,  1959,  1961,
    1963,  1965,  1967,  1969,  1971,  1973,  1975,  1977,  1979,  1981,
    1983,  1985,  1987,  1989,  1991,  1993,  1995,  1997,  1999,  2001,
    2003,  2005,  2007,  2009,  2011,  2013,  2015,  2017,  2019,  2021,
    2023,  2025,  2027,  2029,  2031,  2033,  2035,  2037,  2039,  2041,
    2043,  2045,  2047,  2049,  2051,  2053,  2055,  2057,  2059,  2061,
    2063,  2065,  2067,  2072,  2074,  2076,  2078,  2080,  2082,  2084,
    2086,  2088,  2091,  2093,  2094,  2095,  2097,  2099,  2103,  2104,
    2106,  2108,  2110,  2112,  2114,  2116,  2118,  2120,  2122,  2124,
    2126,  2128,  2130,  2134,  2137,  2139,  2141,  2146,  2150,  2155,
    2157,  2159,  2161,  2163,  2167,  2171,  2175,  2179,  2183,  2187,
    2191,  2195,  2199,  2203,  2207,  2211,  2215,  2219,  2223,  2227,
    2231,  2235,  2238,  2241,  2244,  2247,  2251,  2255,  2259,  2263,
    2267,  2271,  2275,  2279,  2285,  2290,  2294,  2298,  2302,  2304,
    2306,  2308,  2310,  2314,  2318,  2322,  2325,  2326,  2328,  2329,
    2331,  2332,  2338,  2342,  2346,  2348,  2350,  2352,  2354,  2356,
    2360,  2363,  2365,  2367,  2369,  2371,  2373,  2375,  2378,  2381,
    2386,  2390,  2395,  2398,  2399,  2405,  2409,  2413,  2415,  2419,
    2421,  2424,  2425,  2431,  2435,  2438,  2439,  2443,  2444,  2449,
    2452,  2453,  2457,  2461,  2463,  2464,  2466,  2469,  2472,  2477,
    2481,  2485,  2488,  2493,  2496,  2501,  2503,  2505,  2507,  2509,
    2511,  2514,  2519,  2523,  2528,  2532,  2534,  2536,  2538,  2540,
    2543,  2548,  2553,  2557,  2559,  2561,  2565,  2573,  2580,  2589,
    2599,  2608,  2619,  2627,  2634,  2643,  2645,  2648,  2653,  2658,
    2660,  2662,  2667,  2669,  2670,  2672,  2675,  2677,  2679,  2682,
    2687,  2691,  2695,  2696,  2698,  2701,  2706,  2710,  2713,  2717,
    2724,  2725,  2727,  2732,  2735,  2736,  2742,  2746,  2750,  2752,
    2759,  2764,  2769,  2772,  2775,  2776,  2782,  2786,  2790,  2792,
    2795,  2796,  2802,  2806,  2810,  2812,  2815,  2816,  2819,  2820,
    2826,  2830,  2834,  2836,  2839,  2840,  2843,  2844,  2850,  2854,
    2858,  2860,  2863,  2866,  2868,  2871,  2873,  2878,  2882,  2886,
    2893,  2897,  2899,  2901,  2903,  2908,  2913,  2918,  2923,  2926,
    2929,  2934,  2937,  2940,  2942,  2946,  2950,  2954,  2955,  2958,
    2964,  2971,  2973,  2976,  2978,  2983,  2987,  2988,  2990,  2994,
    2997,  3001,  3003,  3005,  3006,  3007,  3010,  3015,  3018,  3025,
    3030,  3032,  3034,  3035,  3039,  3045,  3049,  3051,  3054,  3055,
    3060,  3063,  3066,  3068,  3070,  3072,  3074,  3079,  3086,  3088,
    3097,  3104,  3106
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     210,     0,    -1,    -1,   211,   212,    -1,   212,   213,    -1,
      -1,   231,    -1,   248,    -1,   255,    -1,   252,    -1,   260,
      -1,   466,    -1,   122,   199,   200,   201,    -1,   148,   223,
     201,    -1,    -1,   148,   223,   202,   214,   212,   203,    -1,
      -1,   148,   202,   215,   212,   203,    -1,   110,   217,   201,
      -1,   110,   104,   218,   201,    -1,   110,   105,   219,   201,
      -1,   228,   201,    -1,    77,    -1,   154,    -1,   155,    -1,
     157,    -1,   159,    -1,   158,    -1,   183,    -1,   184,    -1,
     186,    -1,   185,    -1,   187,    -1,   188,    -1,   189,    -1,
     190,    -1,   191,    -1,   192,    -1,   193,    -1,   194,    -1,
     195,    -1,   217,     9,   220,    -1,   220,    -1,   221,     9,
     221,    -1,   221,    -1,   222,     9,   222,    -1,   222,    -1,
     223,    -1,   151,   223,    -1,   223,    96,   216,    -1,   151,
     223,    96,   216,    -1,   223,    -1,   151,   223,    -1,   223,
      96,   216,    -1,   151,   223,    96,   216,    -1,   223,    -1,
     151,   223,    -1,   223,    96,   216,    -1,   151,   223,    96,
     216,    -1,   216,    -1,   223,   151,   216,    -1,   223,    -1,
     148,   151,   223,    -1,   151,   223,    -1,   224,    -1,   224,
     469,    -1,   224,   469,    -1,   228,     9,   467,    14,   405,
      -1,   105,   467,    14,   405,    -1,   229,   230,    -1,    -1,
     231,    -1,   248,    -1,   255,    -1,   260,    -1,   202,   229,
     203,    -1,    70,   331,   231,   282,   284,    -1,    70,   331,
      30,   229,   283,   285,    73,   201,    -1,    -1,    88,   331,
     232,   276,    -1,    -1,    87,   233,   231,    88,   331,   201,
      -1,    -1,    90,   199,   333,   201,   333,   201,   333,   200,
     234,   274,    -1,    -1,    97,   331,   235,   279,    -1,   101,
     201,    -1,   101,   340,   201,    -1,   103,   201,    -1,   103,
     340,   201,    -1,   106,   201,    -1,   106,   340,   201,    -1,
      27,   101,   201,    -1,   111,   292,   201,    -1,   117,   294,
     201,    -1,    86,   332,   201,    -1,   119,   199,   463,   200,
     201,    -1,   201,    -1,    81,    -1,    -1,    92,   199,   340,
      96,   273,   272,   200,   236,   275,    -1,    -1,    92,   199,
     340,    28,    96,   273,   272,   200,   237,   275,    -1,    94,
     199,   278,   200,   277,    -1,    -1,   107,   240,   108,   199,
     398,    79,   200,   202,   229,   203,   242,   238,   245,    -1,
      -1,   107,   240,   165,   239,   243,    -1,   109,   340,   201,
      -1,   102,   216,   201,    -1,   340,   201,    -1,   334,   201,
      -1,   335,   201,    -1,   336,   201,    -1,   337,   201,    -1,
     338,   201,    -1,   106,   337,   201,    -1,   339,   201,    -1,
     368,   201,    -1,   106,   367,   201,    -1,   216,    30,    -1,
      -1,   202,   241,   229,   203,    -1,   242,   108,   199,   398,
      79,   200,   202,   229,   203,    -1,    -1,    -1,   202,   244,
     229,   203,    -1,   165,   243,    -1,    -1,    35,    -1,    -1,
     104,    -1,    -1,   247,   246,   468,   249,   199,   288,   200,
     473,   320,    -1,    -1,   324,   247,   246,   468,   250,   199,
     288,   200,   473,   320,    -1,    -1,   425,   323,   247,   246,
     468,   251,   199,   288,   200,   473,   320,    -1,    -1,   158,
     216,   253,    30,   480,   465,   202,   295,   203,    -1,    -1,
     425,   158,   216,   254,    30,   480,   465,   202,   295,   203,
      -1,    -1,   266,   263,   256,   267,   268,   202,   298,   203,
      -1,    -1,   425,   266,   263,   257,   267,   268,   202,   298,
     203,    -1,    -1,   124,   264,   258,   269,   202,   298,   203,
      -1,    -1,   425,   124,   264,   259,   269,   202,   298,   203,
      -1,    -1,   160,   265,   261,   268,   202,   298,   203,    -1,
      -1,   425,   160,   265,   262,   268,   202,   298,   203,    -1,
     468,    -1,   152,    -1,   468,    -1,   468,    -1,   123,    -1,
     116,   123,    -1,   115,   123,    -1,   125,   398,    -1,    -1,
     126,   270,    -1,    -1,   125,   270,    -1,    -1,   398,    -1,
     270,     9,   398,    -1,   398,    -1,   271,     9,   398,    -1,
     128,   273,    -1,    -1,   432,    -1,    35,   432,    -1,   129,
     199,   444,   200,    -1,   231,    -1,    30,   229,    91,   201,
      -1,   231,    -1,    30,   229,    93,   201,    -1,   231,    -1,
      30,   229,    89,   201,    -1,   231,    -1,    30,   229,    95,
     201,    -1,   216,    14,   405,    -1,   278,     9,   216,    14,
     405,    -1,   202,   280,   203,    -1,   202,   201,   280,   203,
      -1,    30,   280,    98,   201,    -1,    30,   201,   280,    98,
     201,    -1,   280,    99,   340,   281,   229,    -1,   280,   100,
     281,   229,    -1,    -1,    30,    -1,   201,    -1,   282,    71,
     331,   231,    -1,    -1,   283,    71,   331,    30,   229,    -1,
      -1,    72,   231,    -1,    -1,    72,    30,   229,    -1,    -1,
     287,     9,   426,   326,   481,   161,    79,    -1,   287,     9,
     426,   326,   481,   161,    -1,   287,   410,    -1,   426,   326,
     481,   161,    79,    -1,   426,   326,   481,   161,    -1,    -1,
     426,   326,   481,    79,    -1,   426,   326,   481,    35,    79,
      -1,   426,   326,   481,    35,    79,    14,   405,    -1,   426,
     326,   481,    79,    14,   405,    -1,   287,     9,   426,   326,
     481,    79,    -1,   287,     9,   426,   326,   481,    35,    79,
      -1,   287,     9,   426,   326,   481,    35,    79,    14,   405,
      -1,   287,     9,   426,   326,   481,    79,    14,   405,    -1,
     289,     9,   426,   481,   161,    79,    -1,   289,     9,   426,
     481,   161,    -1,   289,   410,    -1,   426,   481,   161,    79,
      -1,   426,   481,   161,    -1,    -1,   426,   481,    79,    -1,
     426,   481,    35,    79,    -1,   426,   481,    35,    79,    14,
     405,    -1,   426,   481,    79,    14,   405,    -1,   289,     9,
     426,   481,    79,    -1,   289,     9,   426,   481,    35,    79,
      -1,   289,     9,   426,   481,    35,    79,    14,   405,    -1,
     289,     9,   426,   481,    79,    14,   405,    -1,   291,   410,
      -1,    -1,   340,    -1,    35,   432,    -1,   161,   340,    -1,
     291,     9,   340,    -1,   291,     9,   161,   340,    -1,   291,
       9,    35,   432,    -1,   292,     9,   293,    -1,   293,    -1,
      79,    -1,   204,   432,    -1,   204,   202,   340,   203,    -1,
     294,     9,    79,    -1,   294,     9,    79,    14,   405,    -1,
      79,    -1,    79,    14,   405,    -1,   295,   296,    -1,    -1,
     297,   201,    -1,   467,    14,   405,    -1,   298,   299,    -1,
      -1,    -1,   322,   300,   328,   201,    -1,    -1,   324,   480,
     301,   328,   201,    -1,   329,   201,    -1,    -1,   323,   247,
     246,   468,   199,   302,   286,   200,   473,   321,    -1,    -1,
     425,   323,   247,   246,   468,   199,   303,   286,   200,   473,
     321,    -1,   154,   308,   201,    -1,   155,   314,   201,    -1,
     157,   316,   201,    -1,     4,   125,   398,   201,    -1,     4,
     126,   398,   201,    -1,   110,   271,   201,    -1,   110,   271,
     202,   304,   203,    -1,   304,   305,    -1,   304,   306,    -1,
      -1,   227,   147,   216,   162,   271,   201,    -1,   307,    96,
     323,   216,   201,    -1,   307,    96,   324,   201,    -1,   227,
     147,   216,    -1,   216,    -1,   309,    -1,   308,     9,   309,
      -1,   310,   395,   312,   313,    -1,   152,    -1,   130,    -1,
     398,    -1,   118,    -1,   158,   202,   311,   203,    -1,   131,
      -1,   404,    -1,   311,     9,   404,    -1,    14,   405,    -1,
      -1,    55,   159,    -1,    -1,   315,    -1,   314,     9,   315,
      -1,   156,    -1,   317,    -1,   216,    -1,   121,    -1,   199,
     318,   200,    -1,   199,   318,   200,    49,    -1,   199,   318,
     200,    29,    -1,   199,   318,   200,    46,    -1,   317,    -1,
     319,    -1,   319,    49,    -1,   319,    29,    -1,   319,    46,
      -1,   318,     9,   318,    -1,   318,    33,   318,    -1,   216,
      -1,   152,    -1,   156,    -1,   201,    -1,   202,   229,   203,
      -1,   201,    -1,   202,   229,   203,    -1,   324,    -1,   118,
      -1,   324,    -1,    -1,   325,    -1,   324,   325,    -1,   112,
      -1,   113,    -1,   114,    -1,   117,    -1,   116,    -1,   115,
      -1,   181,    -1,   327,    -1,    -1,   112,    -1,   113,    -1,
     114,    -1,   328,     9,    79,    -1,   328,     9,    79,    14,
     405,    -1,    79,    -1,    79,    14,   405,    -1,   329,     9,
     467,    14,   405,    -1,   105,   467,    14,   405,    -1,   199,
     330,   200,    -1,    68,   400,   403,    -1,    67,   340,    -1,
     387,    -1,   359,    -1,   199,   340,   200,    -1,   332,     9,
     340,    -1,   340,    -1,   332,    -1,    -1,    27,   340,    -1,
      27,   340,   128,   340,    -1,   432,    14,   334,    -1,   129,
     199,   444,   200,    14,   334,    -1,    28,   340,    -1,   432,
      14,   337,    -1,   129,   199,   444,   200,    14,   337,    -1,
     341,    -1,   432,    -1,   330,    -1,   129,   199,   444,   200,
      14,   340,    -1,   432,    14,   340,    -1,   432,    14,    35,
     432,    -1,   432,    14,    35,    68,   400,   403,    -1,   432,
      26,   340,    -1,   432,    25,   340,    -1,   432,    24,   340,
      -1,   432,    23,   340,    -1,   432,    22,   340,    -1,   432,
      21,   340,    -1,   432,    20,   340,    -1,   432,    19,   340,
      -1,   432,    18,   340,    -1,   432,    17,   340,    -1,   432,
      16,   340,    -1,   432,    15,   340,    -1,   432,    64,    -1,
      64,   432,    -1,   432,    63,    -1,    63,   432,    -1,   340,
      31,   340,    -1,   340,    32,   340,    -1,   340,    10,   340,
      -1,   340,    12,   340,    -1,   340,    11,   340,    -1,   340,
      33,   340,    -1,   340,    35,   340,    -1,   340,    34,   340,
      -1,   340,    48,   340,    -1,   340,    46,   340,    -1,   340,
      47,   340,    -1,   340,    49,   340,    -1,   340,    50,   340,
      -1,   340,    65,   340,    -1,   340,    51,   340,    -1,   340,
      45,   340,    -1,   340,    44,   340,    -1,    46,   340,    -1,
      47,   340,    -1,    52,   340,    -1,    54,   340,    -1,   340,
      37,   340,    -1,   340,    36,   340,    -1,   340,    39,   340,
      -1,   340,    38,   340,    -1,   340,    40,   340,    -1,   340,
      43,   340,    -1,   340,    41,   340,    -1,   340,    42,   340,
      -1,   340,    53,   400,    -1,   199,   341,   200,    -1,   340,
      29,   340,    30,   340,    -1,   340,    29,    30,   340,    -1,
     462,    -1,    62,   340,    -1,    61,   340,    -1,    60,   340,
      -1,    59,   340,    -1,    58,   340,    -1,    57,   340,    -1,
      56,   340,    -1,    69,   401,    -1,    55,   340,    -1,   407,
      -1,   358,    -1,   357,    -1,   360,    -1,   361,    -1,   205,
     402,   205,    -1,    13,   340,    -1,   343,    -1,   346,    -1,
     365,    -1,   110,   199,   386,   410,   200,    -1,    -1,    -1,
     247,   246,   199,   344,   288,   200,   473,   342,   202,   229,
     203,    -1,    -1,   324,   247,   246,   199,   345,   288,   200,
     473,   342,   202,   229,   203,    -1,    -1,    79,   347,   351,
      -1,    -1,   181,    79,   348,   351,    -1,    -1,   196,   349,
     288,   197,   473,   351,    -1,    -1,   181,   196,   350,   288,
     197,   473,   351,    -1,     8,   340,    -1,     8,   337,    -1,
       8,   202,   229,   203,    -1,    85,    -1,   464,    -1,   353,
       9,   352,   128,   340,    -1,   352,   128,   340,    -1,   354,
       9,   352,   128,   405,    -1,   352,   128,   405,    -1,   353,
     409,    -1,    -1,   354,   409,    -1,    -1,   172,   199,   355,
     200,    -1,   130,   199,   445,   200,    -1,    66,   445,   206,
      -1,   398,   202,   447,   203,    -1,   174,   199,   451,   200,
      -1,   175,   199,   451,   200,    -1,   173,   199,   452,   200,
      -1,   174,   199,   455,   200,    -1,   175,   199,   455,   200,
      -1,   173,   199,   456,   200,    -1,   398,   202,   449,   203,
      -1,   365,    66,   440,   206,    -1,   366,    66,   440,   206,
      -1,   358,    -1,   464,    -1,    85,    -1,   199,   341,   200,
      -1,   369,   370,    -1,   432,    14,   367,    -1,   182,    79,
     185,   340,    -1,   371,   382,    -1,   371,   382,   385,    -1,
     382,    -1,   382,   385,    -1,   372,    -1,   371,   372,    -1,
     373,    -1,   374,    -1,   375,    -1,   376,    -1,   377,    -1,
     378,    -1,   182,    79,   185,   340,    -1,   189,    79,    14,
     340,    -1,   183,   340,    -1,   184,    79,   185,   340,   186,
     340,   187,   340,    -1,   184,    79,   185,   340,   186,   340,
     187,   340,   188,    79,    -1,   190,   379,    -1,   380,    -1,
     379,     9,   380,    -1,   340,    -1,   340,   381,    -1,   191,
      -1,   192,    -1,   383,    -1,   384,    -1,   193,   340,    -1,
     194,   340,   195,   340,    -1,   188,    79,   370,    -1,   386,
       9,    79,    -1,   386,     9,    35,    79,    -1,    79,    -1,
      35,    79,    -1,   166,   152,   388,   167,    -1,   390,    50,
      -1,   390,   167,   391,   166,    50,   389,    -1,    -1,   152,
      -1,   390,   392,    14,   393,    -1,    -1,   391,   394,    -1,
      -1,   152,    -1,   153,    -1,   202,   340,   203,    -1,   153,
      -1,   202,   340,   203,    -1,   387,    -1,   396,    -1,   395,
      30,   396,    -1,   395,    47,   396,    -1,   216,    -1,    69,
      -1,   104,    -1,   105,    -1,   106,    -1,    27,    -1,    28,
      -1,   107,    -1,   108,    -1,   165,    -1,   109,    -1,    70,
      -1,    71,    -1,    73,    -1,    72,    -1,    88,    -1,    89,
      -1,    87,    -1,    90,    -1,    91,    -1,    92,    -1,    93,
      -1,    94,    -1,    95,    -1,    53,    -1,    96,    -1,    97,
      -1,    98,    -1,    99,    -1,   100,    -1,   101,    -1,   103,
      -1,   102,    -1,    86,    -1,    13,    -1,   123,    -1,   124,
      -1,   125,    -1,   126,    -1,    68,    -1,    67,    -1,   118,
      -1,     5,    -1,     7,    -1,     6,    -1,     4,    -1,     3,
      -1,   148,    -1,   110,    -1,   111,    -1,   120,    -1,   121,
      -1,   122,    -1,   117,    -1,   116,    -1,   115,    -1,   114,
      -1,   113,    -1,   112,    -1,   181,    -1,   119,    -1,   129,
      -1,   130,    -1,    10,    -1,    12,    -1,    11,    -1,   132,
      -1,   134,    -1,   133,    -1,   135,    -1,   136,    -1,   150,
      -1,   149,    -1,   180,    -1,   160,    -1,   163,    -1,   162,
      -1,   176,    -1,   178,    -1,   172,    -1,   226,   199,   290,
     200,    -1,   227,    -1,   152,    -1,   398,    -1,   117,    -1,
     438,    -1,   398,    -1,   117,    -1,   442,    -1,   199,   200,
      -1,   331,    -1,    -1,    -1,    84,    -1,   459,    -1,   199,
     290,   200,    -1,    -1,    74,    -1,    75,    -1,    76,    -1,
      85,    -1,   135,    -1,   136,    -1,   150,    -1,   132,    -1,
     163,    -1,   133,    -1,   134,    -1,   149,    -1,   180,    -1,
     143,    84,   144,    -1,   143,   144,    -1,   404,    -1,   225,
      -1,   130,   199,   408,   200,    -1,    66,   408,   206,    -1,
     172,   199,   356,   200,    -1,   406,    -1,   364,    -1,   362,
      -1,   363,    -1,   199,   405,   200,    -1,   405,    31,   405,
      -1,   405,    32,   405,    -1,   405,    10,   405,    -1,   405,
      12,   405,    -1,   405,    11,   405,    -1,   405,    33,   405,
      -1,   405,    35,   405,    -1,   405,    34,   405,    -1,   405,
      48,   405,    -1,   405,    46,   405,    -1,   405,    47,   405,
      -1,   405,    49,   405,    -1,   405,    50,   405,    -1,   405,
      51,   405,    -1,   405,    45,   405,    -1,   405,    44,   405,
      -1,   405,    65,   405,    -1,    52,   405,    -1,    54,   405,
      -1,    46,   405,    -1,    47,   405,    -1,   405,    37,   405,
      -1,   405,    36,   405,    -1,   405,    39,   405,    -1,   405,
      38,   405,    -1,   405,    40,   405,    -1,   405,    43,   405,
      -1,   405,    41,   405,    -1,   405,    42,   405,    -1,   405,
      29,   405,    30,   405,    -1,   405,    29,    30,   405,    -1,
     227,   147,   216,    -1,   152,   147,   216,    -1,   227,   147,
     123,    -1,   225,    -1,    78,    -1,   464,    -1,   404,    -1,
     207,   459,   207,    -1,   208,   459,   208,    -1,   143,   459,
     144,    -1,   411,   409,    -1,    -1,     9,    -1,    -1,     9,
      -1,    -1,   411,     9,   405,   128,   405,    -1,   411,     9,
     405,    -1,   405,   128,   405,    -1,   405,    -1,    74,    -1,
      75,    -1,    76,    -1,    85,    -1,   143,    84,   144,    -1,
     143,   144,    -1,    74,    -1,    75,    -1,    76,    -1,   216,
      -1,   412,    -1,   216,    -1,    46,   413,    -1,    47,   413,
      -1,   130,   199,   415,   200,    -1,    66,   415,   206,    -1,
     172,   199,   418,   200,    -1,   416,   409,    -1,    -1,   416,
       9,   414,   128,   414,    -1,   416,     9,   414,    -1,   414,
     128,   414,    -1,   414,    -1,   417,     9,   414,    -1,   414,
      -1,   419,   409,    -1,    -1,   419,     9,   352,   128,   414,
      -1,   352,   128,   414,    -1,   417,   409,    -1,    -1,   199,
     420,   200,    -1,    -1,   422,     9,   216,   421,    -1,   216,
     421,    -1,    -1,   424,   422,   409,    -1,    45,   423,    44,
      -1,   425,    -1,    -1,   428,    -1,   127,   437,    -1,   127,
     216,    -1,   127,   202,   340,   203,    -1,    66,   440,   206,
      -1,   202,   340,   203,    -1,   433,   429,    -1,   199,   330,
     200,   429,    -1,   443,   429,    -1,   199,   330,   200,   429,
      -1,   437,    -1,   397,    -1,   435,    -1,   436,    -1,   430,
      -1,   432,   427,    -1,   199,   330,   200,   427,    -1,   399,
     147,   437,    -1,   434,   199,   290,   200,    -1,   199,   432,
     200,    -1,   397,    -1,   435,    -1,   436,    -1,   430,    -1,
     432,   428,    -1,   199,   330,   200,   428,    -1,   434,   199,
     290,   200,    -1,   199,   432,   200,    -1,   437,    -1,   430,
      -1,   199,   432,   200,    -1,   432,   127,   216,   469,   199,
     290,   200,    -1,   432,   127,   437,   199,   290,   200,    -1,
     432,   127,   202,   340,   203,   199,   290,   200,    -1,   199,
     330,   200,   127,   216,   469,   199,   290,   200,    -1,   199,
     330,   200,   127,   437,   199,   290,   200,    -1,   199,   330,
     200,   127,   202,   340,   203,   199,   290,   200,    -1,   399,
     147,   216,   469,   199,   290,   200,    -1,   399,   147,   437,
     199,   290,   200,    -1,   399,   147,   202,   340,   203,   199,
     290,   200,    -1,   438,    -1,   441,   438,    -1,   438,    66,
     440,   206,    -1,   438,   202,   340,   203,    -1,   439,    -1,
      79,    -1,   204,   202,   340,   203,    -1,   340,    -1,    -1,
     204,    -1,   441,   204,    -1,   437,    -1,   431,    -1,   442,
     427,    -1,   199,   330,   200,   427,    -1,   399,   147,   437,
      -1,   199,   432,   200,    -1,    -1,   431,    -1,   442,   428,
      -1,   199,   330,   200,   428,    -1,   199,   432,   200,    -1,
     444,     9,    -1,   444,     9,   432,    -1,   444,     9,   129,
     199,   444,   200,    -1,    -1,   432,    -1,   129,   199,   444,
     200,    -1,   446,   409,    -1,    -1,   446,     9,   340,   128,
     340,    -1,   446,     9,   340,    -1,   340,   128,   340,    -1,
     340,    -1,   446,     9,   340,   128,    35,   432,    -1,   446,
       9,    35,   432,    -1,   340,   128,    35,   432,    -1,    35,
     432,    -1,   448,   409,    -1,    -1,   448,     9,   340,   128,
     340,    -1,   448,     9,   340,    -1,   340,   128,   340,    -1,
     340,    -1,   450,   409,    -1,    -1,   450,     9,   405,   128,
     405,    -1,   450,     9,   405,    -1,   405,   128,   405,    -1,
     405,    -1,   453,   409,    -1,    -1,   454,   409,    -1,    -1,
     453,     9,   340,   128,   340,    -1,   340,   128,   340,    -1,
     454,     9,   340,    -1,   340,    -1,   457,   409,    -1,    -1,
     458,   409,    -1,    -1,   457,     9,   405,   128,   405,    -1,
     405,   128,   405,    -1,   458,     9,   405,    -1,   405,    -1,
     459,   460,    -1,   459,    84,    -1,   460,    -1,    84,   460,
      -1,    79,    -1,    79,    66,   461,   206,    -1,    79,   127,
     216,    -1,   145,   340,   203,    -1,   145,    78,    66,   340,
     206,   203,    -1,   146,   432,   203,    -1,   216,    -1,    80,
      -1,    79,    -1,   120,   199,   463,   200,    -1,   121,   199,
     432,   200,    -1,   121,   199,   341,   200,    -1,   121,   199,
     330,   200,    -1,     7,   340,    -1,     6,   340,    -1,     5,
     199,   340,   200,    -1,     4,   340,    -1,     3,   340,    -1,
     432,    -1,   463,     9,   432,    -1,   399,   147,   216,    -1,
     399,   147,   123,    -1,    -1,    96,   480,    -1,   176,   468,
      14,   480,   201,    -1,   178,   468,   465,    14,   480,   201,
      -1,   216,    -1,   480,   216,    -1,   216,    -1,   216,   168,
     474,   169,    -1,   168,   471,   169,    -1,    -1,   480,    -1,
     470,     9,   480,    -1,   470,   409,    -1,   470,     9,   161,
      -1,   471,    -1,   161,    -1,    -1,    -1,    30,   480,    -1,
     474,     9,   475,   216,    -1,   475,   216,    -1,   474,     9,
     475,   216,    96,   480,    -1,   475,   216,    96,   480,    -1,
      46,    -1,    47,    -1,    -1,    85,   128,   480,    -1,   227,
     147,   216,   128,   480,    -1,   477,     9,   476,    -1,   476,
      -1,   477,   409,    -1,    -1,   172,   199,   478,   200,    -1,
      29,   480,    -1,    55,   480,    -1,   227,    -1,   130,    -1,
     131,    -1,   479,    -1,   130,   168,   480,   169,    -1,   130,
     168,   480,     9,   480,   169,    -1,   152,    -1,   199,   104,
     199,   472,   200,    30,   480,   200,    -1,   199,   480,     9,
     470,   409,   200,    -1,   480,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   735,   735,   735,   744,   746,   749,   750,   751,   752,
     753,   754,   755,   758,   760,   760,   762,   762,   764,   765,
     767,   769,   774,   775,   776,   777,   778,   779,   780,   781,
     782,   783,   784,   785,   786,   787,   788,   789,   790,   791,
     792,   796,   798,   802,   804,   808,   810,   814,   815,   816,
     817,   822,   823,   824,   825,   830,   831,   832,   833,   838,
     839,   843,   844,   846,   849,   855,   862,   869,   873,   879,
     881,   884,   885,   886,   887,   890,   891,   895,   900,   900,
     906,   906,   913,   912,   918,   918,   923,   924,   925,   926,
     927,   928,   929,   930,   931,   932,   933,   934,   935,   938,
     936,   945,   943,   950,   958,   952,   962,   960,   964,   965,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   987,   987,   992,   998,  1002,  1002,  1010,  1011,  1015,
    1016,  1020,  1025,  1024,  1037,  1035,  1049,  1047,  1063,  1062,
    1071,  1069,  1081,  1080,  1099,  1097,  1116,  1115,  1124,  1122,
    1134,  1133,  1145,  1143,  1156,  1157,  1161,  1164,  1167,  1168,
    1169,  1172,  1174,  1177,  1178,  1181,  1182,  1185,  1186,  1190,
    1191,  1196,  1197,  1200,  1201,  1202,  1206,  1207,  1211,  1212,
    1216,  1217,  1221,  1222,  1227,  1228,  1233,  1234,  1235,  1236,
    1239,  1242,  1244,  1247,  1248,  1252,  1254,  1257,  1260,  1263,
    1264,  1267,  1268,  1272,  1278,  1285,  1287,  1292,  1298,  1302,
    1306,  1310,  1315,  1320,  1325,  1330,  1336,  1345,  1350,  1356,
    1358,  1362,  1367,  1371,  1374,  1377,  1381,  1385,  1389,  1393,
    1398,  1406,  1408,  1411,  1412,  1413,  1414,  1416,  1418,  1423,
    1424,  1427,  1428,  1429,  1433,  1434,  1436,  1437,  1441,  1443,
    1446,  1450,  1456,  1458,  1461,  1461,  1465,  1464,  1468,  1472,
    1470,  1485,  1482,  1495,  1497,  1499,  1501,  1503,  1505,  1507,
    1511,  1512,  1513,  1516,  1522,  1525,  1531,  1534,  1539,  1541,
    1546,  1551,  1555,  1556,  1562,  1563,  1565,  1569,  1570,  1575,
    1576,  1580,  1581,  1585,  1587,  1593,  1598,  1599,  1601,  1605,
    1606,  1607,  1608,  1612,  1613,  1614,  1615,  1616,  1617,  1619,
    1624,  1627,  1628,  1632,  1633,  1637,  1638,  1641,  1642,  1645,
    1646,  1649,  1650,  1654,  1655,  1656,  1657,  1658,  1659,  1660,
    1664,  1665,  1668,  1669,  1670,  1673,  1675,  1677,  1678,  1681,
    1683,  1688,  1689,  1691,  1692,  1693,  1696,  1700,  1701,  1705,
    1706,  1710,  1711,  1715,  1719,  1724,  1728,  1732,  1737,  1738,
    1739,  1742,  1744,  1745,  1746,  1749,  1750,  1751,  1752,  1753,
    1754,  1755,  1756,  1757,  1758,  1759,  1760,  1761,  1762,  1763,
    1764,  1765,  1766,  1767,  1768,  1769,  1770,  1771,  1772,  1773,
    1774,  1775,  1776,  1777,  1778,  1779,  1780,  1781,  1782,  1783,
    1784,  1785,  1786,  1787,  1788,  1789,  1790,  1791,  1793,  1794,
    1796,  1798,  1799,  1800,  1801,  1802,  1803,  1804,  1805,  1806,
    1807,  1808,  1809,  1810,  1811,  1812,  1813,  1814,  1815,  1816,
    1817,  1818,  1819,  1820,  1824,  1828,  1833,  1832,  1847,  1845,
    1862,  1862,  1878,  1877,  1895,  1895,  1911,  1910,  1931,  1932,
    1933,  1938,  1940,  1944,  1948,  1954,  1958,  1964,  1966,  1970,
    1972,  1976,  1980,  1981,  1985,  1992,  1993,  1997,  2001,  2003,
    2008,  2013,  2020,  2022,  2027,  2028,  2029,  2031,  2035,  2039,
    2043,  2047,  2049,  2051,  2053,  2058,  2059,  2064,  2065,  2066,
    2067,  2068,  2069,  2073,  2077,  2081,  2085,  2090,  2095,  2099,
    2100,  2104,  2105,  2109,  2110,  2114,  2115,  2119,  2123,  2127,
    2131,  2132,  2133,  2134,  2138,  2144,  2153,  2166,  2167,  2170,
    2173,  2176,  2177,  2180,  2184,  2187,  2190,  2197,  2198,  2202,
    2203,  2205,  2209,  2210,  2211,  2212,  2213,  2214,  2215,  2216,
    2217,  2218,  2219,  2220,  2221,  2222,  2223,  2224,  2225,  2226,
    2227,  2228,  2229,  2230,  2231,  2232,  2233,  2234,  2235,  2236,
    2237,  2238,  2239,  2240,  2241,  2242,  2243,  2244,  2245,  2246,
    2247,  2248,  2249,  2250,  2251,  2252,  2253,  2254,  2255,  2256,
    2257,  2258,  2259,  2260,  2261,  2262,  2263,  2264,  2265,  2266,
    2267,  2268,  2269,  2270,  2271,  2272,  2273,  2274,  2275,  2276,
    2277,  2278,  2279,  2280,  2281,  2282,  2283,  2284,  2285,  2286,
    2287,  2288,  2292,  2297,  2298,  2301,  2302,  2303,  2307,  2308,
    2309,  2313,  2314,  2315,  2319,  2320,  2321,  2324,  2326,  2330,
    2331,  2332,  2333,  2335,  2336,  2337,  2338,  2339,  2340,  2341,
    2342,  2343,  2344,  2347,  2352,  2353,  2354,  2356,  2357,  2359,
    2360,  2361,  2362,  2363,  2364,  2366,  2368,  2370,  2372,  2374,
    2375,  2376,  2377,  2378,  2379,  2380,  2381,  2382,  2383,  2384,
    2385,  2386,  2387,  2388,  2389,  2390,  2392,  2394,  2396,  2398,
    2399,  2402,  2403,  2407,  2409,  2413,  2416,  2419,  2425,  2426,
    2427,  2428,  2429,  2430,  2431,  2436,  2438,  2442,  2443,  2446,
    2447,  2451,  2454,  2456,  2458,  2462,  2463,  2464,  2465,  2467,
    2470,  2474,  2475,  2476,  2477,  2480,  2481,  2482,  2483,  2484,
    2486,  2487,  2492,  2494,  2497,  2500,  2502,  2504,  2507,  2509,
    2513,  2515,  2518,  2521,  2527,  2529,  2532,  2533,  2538,  2541,
    2545,  2545,  2550,  2553,  2554,  2558,  2559,  2564,  2565,  2569,
    2570,  2574,  2575,  2580,  2582,  2587,  2588,  2589,  2590,  2591,
    2592,  2593,  2595,  2598,  2600,  2604,  2605,  2606,  2607,  2608,
    2610,  2612,  2614,  2618,  2619,  2620,  2624,  2627,  2630,  2633,
    2637,  2641,  2648,  2652,  2656,  2663,  2664,  2669,  2671,  2672,
    2675,  2676,  2679,  2680,  2684,  2685,  2689,  2690,  2691,  2692,
    2694,  2697,  2700,  2701,  2702,  2704,  2706,  2710,  2711,  2712,
    2714,  2715,  2716,  2720,  2722,  2725,  2727,  2728,  2729,  2730,
    2733,  2735,  2736,  2740,  2742,  2745,  2747,  2748,  2749,  2753,
    2755,  2758,  2761,  2763,  2765,  2769,  2771,  2774,  2776,  2779,
    2781,  2784,  2785,  2789,  2791,  2794,  2796,  2799,  2802,  2806,
    2808,  2812,  2813,  2815,  2816,  2822,  2823,  2825,  2827,  2829,
    2831,  2834,  2835,  2836,  2840,  2841,  2842,  2843,  2844,  2845,
    2846,  2847,  2848,  2852,  2853,  2857,  2859,  2867,  2869,  2873,
    2877,  2884,  2885,  2891,  2892,  2899,  2902,  2906,  2909,  2914,
    2919,  2921,  2922,  2923,  2927,  2928,  2932,  2934,  2935,  2938,
    2943,  2944,  2945,  2949,  2952,  2961,  2963,  2967,  2970,  2973,
    2981,  2984,  2987,  2988,  2991,  2994,  2995,  2998,  3002,  3006,
    3012,  3022,  3023
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
  "T_INLINE_HTML", "T_CHARACTER", "T_BAD_CHARACTER",
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
  "T_NAMESPACE", "T_NS_C", "T_DIR", "T_NS_SEPARATOR", "T_XHP_LABEL",
  "T_XHP_TEXT", "T_XHP_ATTRIBUTE", "T_XHP_CATEGORY",
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
  "$@30", "lambda_expression", "$@31", "$@32", "$@33", "$@34",
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
  "user_attribute_list", "$@35", "non_empty_user_attributes",
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
     216,   217,   217,   218,   218,   219,   219,   220,   220,   220,
     220,   221,   221,   221,   221,   222,   222,   222,   222,   223,
     223,   224,   224,   224,   225,   226,   227,   228,   228,   229,
     229,   230,   230,   230,   230,   231,   231,   231,   232,   231,
     233,   231,   234,   231,   235,   231,   231,   231,   231,   231,
     231,   231,   231,   231,   231,   231,   231,   231,   231,   236,
     231,   237,   231,   231,   238,   231,   239,   231,   231,   231,
     231,   231,   231,   231,   231,   231,   231,   231,   231,   231,
     231,   241,   240,   242,   242,   244,   243,   245,   245,   246,
     246,   247,   249,   248,   250,   248,   251,   248,   253,   252,
     254,   252,   256,   255,   257,   255,   258,   255,   259,   255,
     261,   260,   262,   260,   263,   263,   264,   265,   266,   266,
     266,   267,   267,   268,   268,   269,   269,   270,   270,   271,
     271,   272,   272,   273,   273,   273,   274,   274,   275,   275,
     276,   276,   277,   277,   278,   278,   279,   279,   279,   279,
     280,   280,   280,   281,   281,   282,   282,   283,   283,   284,
     284,   285,   285,   286,   286,   286,   286,   286,   286,   287,
     287,   287,   287,   287,   287,   287,   287,   288,   288,   288,
     288,   288,   288,   289,   289,   289,   289,   289,   289,   289,
     289,   290,   290,   291,   291,   291,   291,   291,   291,   292,
     292,   293,   293,   293,   294,   294,   294,   294,   295,   295,
     296,   297,   298,   298,   300,   299,   301,   299,   299,   302,
     299,   303,   299,   299,   299,   299,   299,   299,   299,   299,
     304,   304,   304,   305,   306,   306,   307,   307,   308,   308,
     309,   309,   310,   310,   310,   310,   310,   311,   311,   312,
     312,   313,   313,   314,   314,   315,   316,   316,   316,   317,
     317,   317,   317,   318,   318,   318,   318,   318,   318,   318,
     319,   319,   319,   320,   320,   321,   321,   322,   322,   323,
     323,   324,   324,   325,   325,   325,   325,   325,   325,   325,
     326,   326,   327,   327,   327,   328,   328,   328,   328,   329,
     329,   330,   330,   330,   330,   330,   331,   332,   332,   333,
     333,   334,   334,   335,   336,   337,   338,   339,   340,   340,
     340,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   341,   341,   341,   341,   341,   341,
     341,   341,   341,   341,   342,   342,   344,   343,   345,   343,
     347,   346,   348,   346,   349,   346,   350,   346,   351,   351,
     351,   352,   352,   353,   353,   354,   354,   355,   355,   356,
     356,   357,   358,   358,   359,   360,   360,   361,   362,   362,
     363,   364,   365,   365,   366,   366,   366,   366,   367,   368,
     369,   370,   370,   370,   370,   371,   371,   372,   372,   372,
     372,   372,   372,   373,   374,   375,   376,   377,   378,   379,
     379,   380,   380,   381,   381,   382,   382,   383,   384,   385,
     386,   386,   386,   386,   387,   388,   388,   389,   389,   390,
     390,   391,   391,   392,   393,   393,   394,   394,   394,   395,
     395,   395,   396,   396,   396,   396,   396,   396,   396,   396,
     396,   396,   396,   396,   396,   396,   396,   396,   396,   396,
     396,   396,   396,   396,   396,   396,   396,   396,   396,   396,
     396,   396,   396,   396,   396,   396,   396,   396,   396,   396,
     396,   396,   396,   396,   396,   396,   396,   396,   396,   396,
     396,   396,   396,   396,   396,   396,   396,   396,   396,   396,
     396,   396,   396,   396,   396,   396,   396,   396,   396,   396,
     396,   396,   396,   396,   396,   396,   396,   396,   396,   396,
     396,   396,   397,   398,   398,   399,   399,   399,   400,   400,
     400,   401,   401,   401,   402,   402,   402,   403,   403,   404,
     404,   404,   404,   404,   404,   404,   404,   404,   404,   404,
     404,   404,   404,   404,   405,   405,   405,   405,   405,   405,
     405,   405,   405,   405,   405,   405,   405,   405,   405,   405,
     405,   405,   405,   405,   405,   405,   405,   405,   405,   405,
     405,   405,   405,   405,   405,   405,   405,   405,   405,   405,
     405,   405,   405,   405,   405,   406,   406,   406,   407,   407,
     407,   407,   407,   407,   407,   408,   408,   409,   409,   410,
     410,   411,   411,   411,   411,   412,   412,   412,   412,   412,
     412,   413,   413,   413,   413,   414,   414,   414,   414,   414,
     414,   414,   415,   415,   416,   416,   416,   416,   417,   417,
     418,   418,   419,   419,   420,   420,   421,   421,   422,   422,
     424,   423,   425,   426,   426,   427,   427,   428,   428,   429,
     429,   430,   430,   431,   431,   432,   432,   432,   432,   432,
     432,   432,   432,   432,   432,   433,   433,   433,   433,   433,
     433,   433,   433,   434,   434,   434,   435,   435,   435,   435,
     435,   435,   436,   436,   436,   437,   437,   438,   438,   438,
     439,   439,   440,   440,   441,   441,   442,   442,   442,   442,
     442,   442,   443,   443,   443,   443,   443,   444,   444,   444,
     444,   444,   444,   445,   445,   446,   446,   446,   446,   446,
     446,   446,   446,   447,   447,   448,   448,   448,   448,   449,
     449,   450,   450,   450,   450,   451,   451,   452,   452,   453,
     453,   454,   454,   455,   455,   456,   456,   457,   457,   458,
     458,   459,   459,   459,   459,   460,   460,   460,   460,   460,
     460,   461,   461,   461,   462,   462,   462,   462,   462,   462,
     462,   462,   462,   463,   463,   464,   464,   465,   465,   466,
     466,   467,   467,   468,   468,   469,   469,   470,   470,   471,
     472,   472,   472,   472,   473,   473,   474,   474,   474,   474,
     475,   475,   475,   476,   476,   477,   477,   478,   478,   479,
     480,   480,   480,   480,   480,   480,   480,   480,   480,   480,
     480,   481,   481
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
       2,     3,     3,     3,     3,     3,     5,     1,     1,     0,
       9,     0,    10,     5,     0,    13,     0,     5,     3,     3,
       2,     2,     2,     2,     2,     2,     3,     2,     2,     3,
       2,     0,     4,     9,     0,     0,     4,     2,     0,     1,
       0,     1,     0,     9,     0,    10,     0,    11,     0,     9,
       0,    10,     0,     8,     0,     9,     0,     7,     0,     8,
       0,     7,     0,     8,     1,     1,     1,     1,     1,     2,
       2,     2,     0,     2,     0,     2,     0,     1,     3,     1,
       3,     2,     0,     1,     2,     4,     1,     4,     1,     4,
       1,     4,     1,     4,     3,     5,     3,     4,     4,     5,
       5,     4,     0,     1,     1,     4,     0,     5,     0,     2,
       0,     3,     0,     7,     6,     2,     5,     4,     0,     4,
       5,     7,     6,     6,     7,     9,     8,     6,     5,     2,
       4,     3,     0,     3,     4,     6,     5,     5,     6,     8,
       7,     2,     0,     1,     2,     2,     3,     4,     4,     3,
       1,     1,     2,     4,     3,     5,     1,     3,     2,     0,
       2,     3,     2,     0,     0,     4,     0,     5,     2,     0,
      10,     0,    11,     3,     3,     3,     4,     4,     3,     5,
       2,     2,     0,     6,     5,     4,     3,     1,     1,     3,
       4,     1,     1,     1,     1,     4,     1,     1,     3,     2,
       0,     2,     0,     1,     3,     1,     1,     1,     1,     3,
       4,     4,     4,     1,     1,     2,     2,     2,     3,     3,
       1,     1,     1,     1,     3,     1,     3,     1,     1,     1,
       0,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     1,     1,     1,     3,     5,     1,     3,     5,
       4,     3,     3,     2,     1,     1,     3,     3,     1,     1,
       0,     2,     4,     3,     6,     2,     3,     6,     1,     1,
       1,     6,     3,     4,     6,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     5,     4,     1,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     1,     1,     1,     1,     1,     3,
       2,     1,     1,     1,     5,     0,     0,    11,     0,    12,
       0,     3,     0,     4,     0,     6,     0,     7,     2,     2,
       4,     1,     1,     5,     3,     5,     3,     2,     0,     2,
       0,     4,     4,     3,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     1,     1,     1,     3,     2,     3,
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
       0,     5,     3,     3,     1,     2,     0,     2,     0,     5,
       3,     3,     1,     2,     0,     2,     0,     5,     3,     3,
       1,     2,     2,     1,     2,     1,     4,     3,     3,     6,
       3,     1,     1,     1,     4,     4,     4,     4,     2,     2,
       4,     2,     2,     1,     3,     3,     3,     0,     2,     5,
       6,     1,     2,     1,     4,     3,     0,     1,     3,     2,
       3,     1,     1,     0,     0,     2,     4,     2,     6,     4,
       1,     1,     0,     3,     5,     3,     1,     2,     0,     4,
       2,     2,     1,     1,     1,     1,     4,     6,     1,     8,
       6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,     0,     0,   740,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   814,     0,
     802,   623,     0,   629,   630,   631,    22,   689,   790,    98,
     632,     0,    80,     0,     0,     0,     0,     0,     0,     0,
       0,   131,     0,     0,     0,     0,     0,     0,   323,   324,
     325,   328,   327,   326,     0,     0,     0,     0,   158,     0,
       0,     0,   636,   638,   639,   633,   634,     0,     0,   640,
     635,     0,   614,    23,    24,    25,    27,    26,     0,   637,
       0,     0,     0,     0,     0,     0,     0,   641,   329,    28,
      29,    31,    30,    32,    33,    34,    35,    36,    37,    38,
      39,    40,   444,     0,    97,    70,   794,   624,     0,     0,
       4,    59,    61,    64,   688,     0,   613,     0,     6,   130,
       7,     9,     8,    10,     0,     0,   321,   360,     0,     0,
       0,     0,     0,     0,     0,   358,   431,   432,   426,   425,
     345,   427,   428,   433,     0,     0,   344,   756,   615,     0,
     691,   424,   320,   759,   359,     0,     0,   757,   758,   755,
     785,   789,     0,   414,   690,    11,   328,   327,   326,     0,
       0,    27,    59,   130,     0,   872,   359,   871,     0,   869,
     868,   430,     0,   351,   355,     0,     0,   398,   399,   400,
     401,   423,   421,   420,   419,   418,   417,   416,   415,   790,
     616,     0,   886,   615,     0,   380,   378,     0,   818,     0,
     698,   343,   619,     0,   886,   618,     0,   628,   797,   796,
     620,     0,     0,   622,   422,     0,     0,     0,     0,   348,
       0,    78,   350,     0,     0,    84,    86,     0,     0,    88,
       0,     0,     0,   913,   914,   918,     0,     0,    59,   912,
       0,   915,     0,     0,    90,     0,     0,     0,     0,   121,
       0,     0,     0,     0,     0,     0,    42,    47,   241,     0,
       0,   240,   160,   159,   246,     0,     0,     0,     0,     0,
     883,   146,   156,   810,   814,   855,     0,   643,     0,     0,
       0,   853,     0,    16,     0,    63,   138,   150,   157,   520,
     458,   838,   836,   836,     0,   877,   442,   446,   744,   360,
       0,   358,   359,     0,     0,   625,     0,   626,     0,     0,
       0,   120,     0,     0,    66,   232,     0,    21,   129,     0,
     155,   142,   154,   326,   329,   130,   322,   111,   112,   113,
     114,   115,   117,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   802,     0,
     110,   793,   793,   118,   824,     0,     0,     0,     0,     0,
       0,   319,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   379,   377,     0,   760,   745,
     793,     0,   751,   232,   793,     0,   795,   786,   810,     0,
     130,     0,     0,    92,     0,   742,   737,   698,     0,     0,
       0,     0,   822,     0,   463,   697,   813,     0,     0,    66,
       0,   232,   342,     0,   798,   745,   753,   621,     0,    70,
     196,     0,   441,     0,    95,     0,     0,   349,     0,     0,
       0,     0,     0,    87,   109,    89,   910,   911,     0,   908,
       0,     0,     0,   882,     0,   116,    91,   119,     0,     0,
       0,     0,     0,     0,     0,   478,     0,   485,   487,   488,
     489,   490,   491,   492,   483,   505,   506,    70,     0,   106,
     108,     0,     0,    44,    51,     0,     0,    46,    55,    48,
       0,    18,     0,     0,   242,     0,    93,     0,     0,    94,
     873,     0,     0,   360,   358,   359,     0,   902,   166,     0,
     811,     0,     0,     0,     0,   642,   854,   689,     0,     0,
     852,   694,   851,    62,     5,    13,    14,     0,   164,     0,
       0,   451,     0,     0,   698,     0,     0,   617,   452,   842,
       0,   698,     0,     0,   698,     0,     0,     0,     0,     0,
     744,     0,   700,   743,   922,   341,   411,   764,    75,    69,
      71,    72,    73,    74,   320,     0,   429,   692,   693,    60,
     698,     0,   887,     0,     0,     0,   700,   233,     0,   436,
     132,   162,     0,   383,   385,   384,     0,     0,   381,   382,
     386,   388,   387,   403,   402,   405,   404,   406,   408,   409,
     407,   397,   396,   390,   391,   389,   392,   393,   395,   410,
     394,   792,     0,     0,   828,     0,   698,   876,     0,   875,
     762,   785,   148,   140,   152,   144,   130,     0,     0,   353,
     356,   362,   479,   376,   375,   374,   373,   372,   371,   370,
     369,   368,   367,   366,   365,     0,   747,   746,     0,     0,
       0,     0,     0,     0,     0,   870,   352,   735,   739,   697,
     741,     0,     0,   886,     0,   817,     0,   816,     0,   801,
     800,     0,     0,   747,   746,   346,   198,   200,    70,   449,
     448,   347,     0,    70,   180,    79,   350,     0,     0,     0,
       0,     0,   192,   192,    85,     0,     0,     0,   906,   698,
       0,   893,     0,     0,     0,     0,     0,   696,   632,     0,
       0,   614,     0,     0,     0,     0,     0,    64,   645,   613,
     651,   652,   650,     0,   644,    68,   649,     0,     0,   495,
       0,     0,   501,   498,   499,   507,     0,   486,   481,     0,
     484,     0,     0,     0,    52,    19,     0,     0,    56,    20,
       0,     0,     0,    41,    49,     0,   239,   247,   244,     0,
       0,   864,   867,   866,   865,    12,   900,   901,     0,     0,
       0,     0,   810,   807,     0,   462,   863,   862,   861,     0,
     857,     0,   858,   860,     0,     5,     0,     0,     0,   514,
     515,   523,   522,     0,     0,   697,   457,   461,     0,   467,
     697,   837,     0,   465,   697,   835,   466,     0,   878,     0,
     443,     0,   894,   744,   219,   921,     0,     0,   761,   745,
     752,   791,   697,   889,   885,   234,   235,   612,   699,   231,
       0,   744,     0,     0,   164,   438,   134,   413,     0,   472,
     473,     0,   464,   697,   823,     0,     0,   232,   166,     0,
     164,   162,     0,   802,   363,     0,     0,   232,   749,   750,
     763,   787,   788,     0,     0,     0,   723,   705,   706,   707,
     708,     0,     0,     0,   716,   715,   729,   698,     0,   737,
     821,   820,     0,   799,   745,   754,   627,     0,   202,     0,
       0,    76,     0,     0,     0,     0,     0,     0,     0,   172,
     173,   184,     0,    70,   182,   103,   192,     0,   192,     0,
       0,   916,     0,     0,   697,   907,   909,   892,   698,   891,
       0,   698,   673,   674,   671,   672,   704,     0,   698,   696,
       0,     0,   460,   846,   844,   844,     0,     0,   830,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   480,     0,     0,     0,   503,
     504,   502,     0,     0,   482,     0,   122,     0,   125,   107,
       0,    43,    53,     0,    45,    57,    50,   243,     0,   874,
      96,   902,   884,   897,   165,   167,   253,     0,     0,   808,
       0,   856,     0,    17,     0,   877,   163,   253,     0,     0,
     454,     0,   875,   841,   840,     0,   879,     0,   894,     0,
       0,   922,     0,   223,   221,     0,   747,   746,   888,     0,
       0,   236,    67,     0,   744,   161,     0,   744,     0,   412,
     827,   826,     0,   232,     0,     0,     0,     0,   164,   136,
     628,   748,   232,     0,     0,   711,   712,   713,   714,   717,
     718,   727,     0,   698,   723,     0,   710,   731,   697,   734,
     736,   738,     0,   815,   748,     0,     0,     0,     0,   199,
     450,    81,     0,   350,   172,   174,   810,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   186,     0,   903,     0,
     905,   697,     0,     0,     0,   647,   697,   695,     0,   686,
       0,   698,     0,   850,     0,   698,     0,     0,   698,     0,
     653,   687,   685,   834,     0,   698,   656,   658,   657,     0,
       0,   654,   655,   659,   661,   660,   676,   675,   678,   677,
     679,   681,   682,   680,   669,   668,   663,   664,   662,   665,
     666,   667,   670,   493,     0,   494,   500,   508,   509,     0,
      70,    54,    58,   245,     0,     0,     0,   320,   812,   810,
     354,   357,   361,     0,    15,     0,   320,   526,     0,     0,
     528,   521,   524,     0,   519,     0,     0,   880,     0,   895,
     445,     0,   224,     0,   220,     0,     0,   232,   238,   237,
     894,     0,   253,     0,   744,     0,   232,     0,   783,   253,
     877,   253,     0,     0,   364,   232,     0,   777,     0,   720,
     697,   722,     0,   709,     0,     0,   698,   728,   819,     0,
      70,     0,   195,   181,     0,     0,     0,   171,    99,   185,
       0,     0,   188,     0,   193,   194,    70,   187,   917,     0,
     890,     0,   920,   703,   702,   646,     0,   697,   459,   648,
     470,   697,   845,     0,   468,   697,   843,   469,     0,   471,
     697,   829,   684,     0,     0,     0,     0,   896,   899,   168,
       0,     0,     0,   318,     0,     0,     0,   147,   252,   254,
       0,   317,     0,   320,     0,   859,   249,   151,   517,     0,
       0,   453,   839,   447,     0,   227,   218,     0,   226,   748,
     232,     0,   435,   894,   320,   894,     0,   825,     0,   782,
     320,     0,   320,   253,   744,     0,   776,   726,   725,   719,
       0,   721,   697,   730,    70,   201,    77,    82,   101,   175,
       0,   183,   189,    70,   191,   904,     0,     0,   456,     0,
     849,   848,     0,   833,   832,   683,     0,    70,   126,     0,
       0,     0,     0,     0,   169,   284,   282,   286,   614,    27,
       0,   278,     0,   283,   295,     0,   293,   298,     0,   297,
       0,   296,     0,   130,   256,     0,   258,     0,   809,     0,
     518,   516,   527,   525,   228,     0,   217,   225,   232,     0,
     780,     0,     0,     0,   143,   435,   894,   784,   149,   249,
     153,   320,     0,   778,     0,   733,     0,   197,     0,     0,
      70,   178,   100,   190,   919,   701,     0,     0,     0,     0,
       0,   898,     0,     0,     0,     0,   268,   272,     0,     0,
     263,   578,   577,   574,   576,   575,   595,   597,   596,   566,
     537,   538,   556,   572,   571,   533,   543,   544,   546,   545,
     565,   549,   547,   548,   550,   551,   552,   553,   554,   555,
     557,   558,   559,   560,   561,   562,   564,   563,   534,   535,
     536,   539,   540,   542,   580,   581,   590,   589,   588,   587,
     586,   585,   573,   592,   582,   583,   584,   567,   568,   569,
     570,   593,   594,   598,   600,   599,   601,   602,   579,   604,
     603,   606,   608,   607,   541,   611,   609,   610,   605,   591,
     532,   290,   529,     0,   264,   311,   312,   310,   303,     0,
     304,   265,   337,     0,     0,     0,     0,   130,   139,   248,
       0,     0,     0,   230,     0,   779,     0,    70,   313,    70,
     133,     0,     0,     0,   145,   894,   724,     0,    70,   176,
      83,   102,     0,   455,   847,   831,   496,   124,   266,   267,
     340,   170,     0,     0,   287,   279,     0,     0,     0,   292,
     294,     0,     0,   299,   306,   307,   305,     0,     0,   255,
       0,     0,     0,     0,   250,     0,   229,   781,     0,   512,
     700,     0,     0,    70,   135,   141,     0,   732,     0,     0,
       0,   104,   269,    59,     0,   270,   271,     0,     0,   285,
     289,   530,   531,     0,   280,   308,   309,   301,   302,   300,
     338,   335,   259,   257,   339,     0,   251,   513,   699,     0,
     437,   314,     0,   137,     0,   179,   497,     0,   128,     0,
     320,   288,   291,     0,   744,   261,     0,   510,   434,   439,
     177,     0,     0,   105,   276,     0,   319,   336,     0,   700,
     331,   744,   511,     0,   127,     0,     0,   275,   894,   744,
     205,   332,   333,   334,   922,   330,     0,     0,     0,   274,
       0,   331,     0,   894,     0,   273,   315,    70,   260,   922,
       0,   209,   207,     0,    70,     0,     0,   210,     0,   206,
     262,     0,   316,     0,   213,   204,     0,   212,   123,   214,
       0,   203,   211,     0,   216,   215
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   120,   805,   544,   182,   275,   502,
     506,   276,   503,   507,   122,   123,   124,   125,   126,   127,
     323,   579,   580,   456,   240,  1438,   462,  1360,  1439,  1668,
     763,   270,   497,  1631,   999,  1180,  1683,   339,   183,   581,
     852,  1058,  1233,   131,   547,   869,   582,   601,   871,   528,
     868,   583,   548,   870,   341,   291,   307,   134,   854,   808,
     791,  1014,  1383,  1108,   919,  1580,  1442,   705,   925,   461,
     714,   927,  1266,   697,   908,   911,  1097,  1688,  1689,   571,
     572,   595,   596,   280,   281,   285,  1409,  1559,  1560,  1187,
    1308,  1402,  1555,  1674,  1691,  1592,  1635,  1636,  1637,  1390,
    1391,  1392,  1593,  1599,  1644,  1395,  1396,  1400,  1548,  1549,
    1550,  1570,  1718,  1309,  1310,   184,   136,  1704,  1705,  1553,
    1312,   137,   233,   457,   458,   138,   139,   140,   141,   142,
     143,   144,   145,  1422,   146,   851,  1057,   147,   237,   569,
     318,   570,   452,   553,   554,  1131,   555,  1132,   148,   149,
     150,   151,   152,   740,   741,   742,   153,   154,   267,   155,
     268,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     753,   754,   991,   494,   495,   496,   760,  1620,   156,   549,
    1411,   550,  1028,   813,  1204,  1201,  1541,  1542,   157,   158,
     159,   227,   234,   326,   442,   160,   946,   746,   161,   947,
     843,   834,   948,   895,  1079,  1081,  1082,  1083,   897,  1245,
    1246,   898,   678,   427,   195,   196,   584,   574,   408,   409,
     840,   163,   228,   186,   165,   166,   167,   168,   169,   170,
     171,   632,   172,   230,   231,   531,   219,   220,   635,   636,
    1144,  1145,   563,   560,   564,   561,  1137,  1134,  1138,  1135,
     300,   301,   799,   173,   521,   174,   568,   175,  1561,   292,
     334,   590,   591,   940,  1040,   788,   789,   718,   719,   720,
     261,   262,   836
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1427
static const yytype_int16 yypact[] =
{
   -1427,   154, -1427, -1427,  6198, 13610, 13610,     4, 13610, 13610,
   13610, 11718, 13610, -1427, 13610, 13610, 13610, 13610, 13610, 13610,
   13610, 13610, 13610, 13610, 13610, 13610, 14797, 14797, 11924, 13610,
   15166,   101,   137, -1427, -1427, -1427, -1427, -1427,   206, -1427,
     161, 13610, -1427,   137,   149,   158,   171,   137, 12088, 16176,
   12252, -1427, 14587, 10730,   170, 13610, 15188,    22, -1427, -1427,
   -1427,   257,   335,    16,   194,   199,   266,   284, -1427, 16176,
     292,   314, -1427, -1427, -1427, -1427, -1427,   270, 14113, -1427,
   -1427, 16176, -1427, -1427, -1427, -1427, 16176, -1427, 16176, -1427,
     253,   316,   318,   324,   326, 16176, 16176, -1427,    29, -1427,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427, -1427, 13610, -1427, -1427,   331,   375,   382,   382,
   -1427,   507,   388,   211, -1427,   342, -1427,    34, -1427,   516,
   -1427, -1427, -1427, -1427, 16021,   359, -1427, -1427,   341,   352,
     353,   363,   365,   377,  3861, -1427, -1427, -1427, -1427,   514,
   -1427, -1427, -1427,   517,   524,   396, -1427,    37,   402,   464,
   -1427, -1427,   542,    -3,  1769,    69,   413,    82,   108,   417,
       9, -1427,   125, -1427,   555, -1427, -1427, -1427,   475,   425,
     474, -1427, -1427,   516,   359, 16450,  2179, 16450, 13610, 16450,
   16450, 10919,   426, 15435, 10919,   585, 16176,   566,   566,   153,
     566,   566,   566,   566,   566,   566,   566,   566,   566, -1427,
   -1427,  4911,   468, -1427,   490,   512,   512, 14797, 15479,   440,
     638, -1427,   475,  4911,   468,   505,   513,   451,   109, -1427,
     535,    69, 12416, -1427, -1427, 13610,  9494,   655,    58, 16450,
   10524, -1427, 13610, 13610, 16176, -1427, -1427,  3905,   476, -1427,
    4032, 14587, 14587,   501, -1427, -1427,   477, 14467,   666, -1427,
     671, -1427, 16176,   609, -1427,   492,  4242,   493,   424, -1427,
       7,  4301, 14820, 16034, 16176,    62, -1427,   275, -1427,  2316,
      63, -1427, -1427, -1427,   681,    71, 14797, 14797, 13610,   497,
     530, -1427, -1427, 15029, 11924,    43,   343, -1427, 13774, 14797,
     351, -1427, 16176, -1427,   300,   388, -1427, -1427, -1427, -1427,
   15249, 13610, 13610, 13610,   687,   607, -1427, -1427,    86,   508,
   16450,   509,   102,  6404, 13610,   241,   510,   401,   241,   261,
     230, -1427, 16176, 14587,   511, 10936, 14587, -1427, -1427,  5067,
   -1427, -1427, -1427, -1427, -1427,   516, -1427, -1427, -1427, -1427,
   -1427, -1427, -1427, 13610, 13610, 13610, 12622, 13610, 13610, 13610,
   13610, 13610, 13610, 13610, 13610, 13610, 13610, 13610, 13610, 13610,
   13610, 13610, 13610, 13610, 13610, 13610, 13610, 13610, 15166, 13610,
   -1427, 13610, 13610, -1427, 13610,  2026, 16176, 16176, 16176, 16021,
     608,   457,  5556, 13610, 13610, 13610, 13610, 13610, 13610, 13610,
   13610, 13610, 13610, 13610, 13610, -1427, -1427,  3806, -1427,   111,
   13610, 13610, -1427, 10936, 13610, 13610,   331,   119, 15029,   519,
     516, 12828,  4391, -1427, 13610, -1427,   520,   704,  4911,   522,
     -13,  4460,   512, 13034, -1427, 13240, -1427,   525,     5, -1427,
     130, 10936, -1427,  4627, -1427,   120, -1427, -1427,  4532, -1427,
   -1427, 11142, -1427, 13610, -1427,   628,  9700,   715,   537, 16362,
     706,    33,    28, -1427, -1427, -1427, -1427, -1427, 14587, 14670,
     528,   727, 14887, -1427,   554, -1427, -1427, -1427,   661, 13610,
     662,   664, 13610, 13610, 13610, -1427,   424, -1427, -1427, -1427,
   -1427, -1427, -1427, -1427,   556, -1427, -1427, -1427,   546, -1427,
   -1427, 16176,   547,   740,   294, 16176,   549,   742,   296,   304,
   16079, -1427, 16176, 13610,   512,    22, -1427, 14887,   674, -1427,
     512,    52,    60,   558,   560,   968,   564,    87,   630,   557,
     512,    88,   569, 15955, 16176, -1427, -1427,   700,  2023,   -14,
   -1427, -1427, -1427,   388, -1427, -1427, -1427,   744,   646,   610,
      46, -1427,   331,   648,   769,   580,   634,   119, -1427, 16450,
     583,   775, 15535,   586,   778,   590, 14587, 14587,   777,   655,
      86,   595,   784, -1427, 14587,    15,   729,    77, -1427, -1427,
   -1427, -1427, -1427, -1427,   711,  2229, -1427, -1427, -1427, -1427,
     787,   629, -1427, 14797, 13610,   599,   793, 16450,   789, -1427,
   -1427,   680, 14326, 11331,  3038, 10919, 13610, 16406, 13977,  4448,
    5086,  3149, 12600, 14329, 14329, 14329, 14329,  1761,  1761,  1761,
    1761,   854,   854,   621,   621,   621,   153,   153,   153, -1427,
     566, 16450,   601,   603, 15579,   615,   802, -1427, 13610,   205,
     614,   119, -1427, -1427, -1427, -1427,   516, 13610, 14647, -1427,
   -1427, 10919, -1427, 10919, 10919, 10919, 10919, 10919, 10919, 10919,
   10919, 10919, 10919, 10919, 10919, 13610,   205,   622,   623,  2335,
     620,   625,  2511,    89,   637, -1427, 16450, 14981, -1427, 16176,
   -1427,   508,    15,   468, 14797, 16450, 14797, 15635,    20,   122,
   -1427,   639, 13610, -1427, -1427, -1427,  9288,   136, -1427, -1427,
   16450, 16450,   137, -1427, -1427, -1427, 13610,   741,  2631, 14887,
   16176,  9906,   641,   644, -1427,    50,   710,   693, -1427,   837,
     651,  3274, 14587, 14887, 14887, 14887, 14887, 14887, -1427,   649,
      23,   705,   654,   657,   658,   659, 14887,   -11, -1427,   712,
   -1427, -1427, -1427,   660, -1427, 16536, -1427, 13610,   679, 16450,
     682,   851,  5330,   857, -1427, 16450,  5236, -1427,   556,   790,
   -1427,  6610, 15236,   668,   313, -1427, 14820, 16176,   327, -1427,
   16034, 16176, 16176, -1427, -1427,  2857, -1427, 16536,   858, 14797,
     672, -1427, -1427, -1427, -1427, -1427, -1427, -1427,    55, 16176,
   15236,   673, 15029, 15112,   860, -1427, -1427, -1427, -1427,   677,
   -1427, 13610, -1427, -1427,  5786, -1427, 14587, 15236,   685, -1427,
   -1427, -1427, -1427,   865, 13610, 15249, -1427, -1427, 16121, -1427,
   13610, -1427, 13610, -1427, 13610, -1427, -1427,   683, -1427, 14587,
   -1427,   692,   861,   104, -1427, -1427,   280,  5322, -1427,   124,
   -1427, -1427, 14587, -1427, -1427,   512, 16450, -1427, 11348, -1427,
   14887,    65,   694, 15236,   646, -1427, -1427, 13223, 13610, -1427,
   -1427, 13610, -1427, 13610, -1427,  2907,   695, 10936,   630,   867,
     646,   680, 16176, 15166,   512,  2998,   696, 10936, -1427, -1427,
     129, -1427, -1427,   884,  4718,  4718, 14981, -1427, -1427, -1427,
   -1427,   714,    57,   716, -1427, -1427, -1427,   890,   709,   520,
     512,   512, 13446, -1427,   131, -1427, -1427,  3233,   306,   137,
   10524, -1427,  6816,   725,  7022,   726,  2631, 14797,   732,   783,
     512, 16536,   914, -1427, -1427, -1427, -1427,   489, -1427,   243,
   14587, -1427, 14587, 16176, 14670, -1427, -1427, -1427,   924, -1427,
     734,   787,   526,   526,   870,   870, 15779,   730,   928, 14887,
     794, 16176, 15249, 14887, 14887, 14887,  4919, 16163, 14887, 14887,
   14887, 14887, 14737, 14887, 14887, 14887, 14887, 14887, 14887, 14887,
   14887, 14887, 14887, 14887, 14887, 14887, 14887, 14887, 14887, 14887,
   14887, 14887, 14887, 14887, 14887, 16450, 13610, 13610, 13610, -1427,
   -1427, -1427, 13610, 13610, -1427,   424, -1427,   862, -1427, -1427,
   16176, -1427, -1427, 16176, -1427, -1427, -1427, -1427, 14887,   512,
   -1427,    87, -1427,   844,   933, -1427, -1427,    93,   745,   512,
   11554, -1427,  1883, -1427,  5992,   607,   933, -1427,   346,   -41,
   16450,   820, -1427, 16450, 16450, 15679, -1427,   748,   861, 14587,
     655, 14587,   872,   938,   875, 13610,   205,   756, -1427, 14797,
   13610, 16450, 16536,   757,    65, -1427,   754,    65,   759, 13223,
   16450, 15735,   760, 10936,   764,   758, 14587,   767,   646, -1427,
     451,   771, 10936,   766, 13610, -1427, -1427, -1427, -1427, -1427,
   -1427,   843,   768,   963, 14981,   829, -1427, 15249, 14981, -1427,
   -1427, -1427, 14797, 16450, -1427,   137,   945,   903, 10524, -1427,
   -1427, -1427,   776, 13610,   783,   512, 15029,  2631,   779, 14887,
    7228,   543,   796, 13610,    30,   247, -1427,   811, -1427,   873,
   -1427, 14517,   965,   800, 14887, -1427, 14887, -1427,   803, -1427,
     876,   997,   808, 16536,   809,  1001, 15835,   814,  1002,   815,
   -1427, -1427, -1427, 15877,   813,  1008, 11907, 12811, 13017, 14887,
   16494,  3272,  4804,  5505, 14281,  4562, 14539, 14539, 14539, 14539,
    1516,  1516,  1516,  1516,   874,   874,   526,   526,   526,   870,
     870,   870,   870, 16450, 14210, 16450, -1427, 16450, -1427,   818,
   -1427, -1427, -1427, 16536, 16176, 14587, 15236,    66, -1427, 15029,
   -1427, -1427, 10919,   816, -1427,   819,   445, -1427,   160, 13610,
   -1427, -1427, -1427, 13610, -1427, 13610, 13610, -1427,   655, -1427,
   -1427,   281,  1011, 14887, -1427,  3466,   823, 10936,   512, 16450,
     861,   827, -1427,   828,    65, 13610, 10936,   834, -1427, -1427,
     607, -1427,   821,   830, -1427, 10936,   835, -1427, 14981, -1427,
   14981, -1427,   838, -1427,   912,   841,  1034, -1427,   512,  1014,
   -1427,   845, -1427, -1427,   847,   848,    95, -1427, -1427, 16536,
     850,   855, -1427,  3719, -1427, -1427, -1427, -1427, -1427, 14587,
   -1427, 14587, -1427, 16536, 15933, -1427, 14887, 15249, -1427, -1427,
   -1427, 14887, -1427, 14887, -1427, 14887, -1427, -1427, 14887, -1427,
   14887, -1427, 13429, 14887, 13610,   853,  7434,   949, -1427, -1427,
     226, 14587, 15236, -1427, 15977,   893,  2614, -1427, -1427, -1427,
     608, 14375,    74,   457,    96, -1427, -1427, -1427,   900,  3544,
    3600, 16450, 16450, -1427,   978,  1044,   981, 14887, 16536,   863,
   10936,   866,   951,   861,   616,   861,   871, 16450,   877, -1427,
    1009,   868,  1105, -1427,    65,   878, -1427, -1427,   935, -1427,
   14981, -1427, 15249, -1427, -1427,  9288, -1427, -1427, -1427, -1427,
   10112, -1427, -1427, -1427,  9288, -1427,   879, 14887, 16536,   939,
   16536, 16536, 15975, 16536, 16031, 13429, 14166, -1427, -1427, 14587,
   15236, 15236,  1060,    53, -1427, -1427, -1427, -1427,    76,   880,
      79, -1427, 13980, -1427, -1427,    80, -1427, -1427,  5139, -1427,
     882, -1427,  1006,   516, -1427, 14587, -1427,   608, -1427, 14233,
   -1427, -1427, -1427, -1427,  1062, 14887, -1427, 16536, 10936,   881,
   -1427,   887,   885,   -64, -1427,   951,   861, -1427, -1427, -1427,
   -1427,  1365,   888, -1427, 14981, -1427,   961,  9288, 10318, 10112,
   -1427, -1427, -1427,  9288, -1427, 16536, 14887, 14887, 14887, 13610,
    7640, -1427,   891,   895, 14887, 15236, -1427, -1427,   460, 15977,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427,   387, -1427,   893, -1427, -1427, -1427, -1427, -1427,    67,
     503, -1427,  1083,    81, 16176,  1006,  1084,   516, -1427, -1427,
     902,  1087, 14887, 16536,   905, -1427,    94, -1427, -1427, -1427,
   -1427,   909,   -64, 14283, -1427,   861, -1427, 14981, -1427, -1427,
   -1427, -1427,  7846, 16536, 16536, 16536,  5401, -1427, -1427, -1427,
   16536, -1427,  1509,    35, -1427, -1427, 14887, 13980, 13980,  1057,
   -1427,  5139,  5139,   536, -1427, -1427, -1427, 14887,  1036, -1427,
     917,    84, 14887, 16176, -1427, 14887, 16536, -1427,  1039, -1427,
    1108,  8052,  8258, -1427, -1427, -1427,   -64, -1427,  8464,   919,
    1050,  1022, -1427,  1035,   985, -1427, -1427,  1041,   460, -1427,
   16536, -1427, -1427,   975, -1427,  1102, -1427, -1427, -1427, -1427,
   16536,  1124, -1427, -1427, 16536,   940, 16536, -1427,   115,   941,
   -1427, -1427,  8670, -1427,   943, -1427, -1427,   946,   977, 16176,
     457, -1427, -1427, 14887,    99, -1427,  1061, -1427, -1427, -1427,
   -1427, 15236,   668, -1427,   984, 16176,   252, 16536,   947,  1139,
     570,    99, -1427,  1072, -1427, 15236,   952, -1427,   861,   107,
   -1427, -1427, -1427, -1427, 14587, -1427,   955,   956,    85, -1427,
     218,   570,   282,   861,   950, -1427, -1427, -1427, -1427, 14587,
    1080,  1146,  1086,   218, -1427,  8876,   283,  1148, 14887, -1427,
   -1427,  9082, -1427,  1088,  1155,  1091, 14887, 16536, -1427,  1157,
   14887, -1427, 16536, 14887, 16536, 16536
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1427, -1427, -1427,  -507, -1427, -1427, -1427,    -4, -1427, -1427,
   -1427,   663,   406,   404,   -24,  1038,  3791, -1427,  2413, -1427,
    -304, -1427,     6, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427, -1427, -1427,  -506, -1427, -1427,  -176,    18,     2,
   -1427, -1427, -1427, -1427, -1427, -1427,    12, -1427, -1427, -1427,
   -1427,    13, -1427, -1427,   786,   792,   791,  -128,   310,  -802,
     315,   378,  -511,    90,  -810, -1427,  -253, -1427, -1427, -1427,
   -1427,  -675,   -75, -1427, -1427, -1427, -1427,  -502, -1427,  -543,
   -1427,  -385, -1427, -1427,   676, -1427,  -237, -1427, -1427,  -988,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,
    -266, -1427, -1427, -1427, -1427, -1427,  -348, -1427,  -109, -1165,
   -1427, -1426,  -525, -1427,  -159,     0,  -134,  -512, -1427,  -355,
   -1427,   -72,   -23,  1161,  -671,  -371, -1427, -1427,   -39, -1427,
   -1427,  3815,   -60,  -222, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427,  -540,  -784, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1427,   812, -1427,
   -1427,   210, -1427,   720, -1427, -1427, -1427, -1427, -1427, -1427,
   -1427,   215, -1427,   722, -1427, -1427,   453, -1427,   181, -1427,
   -1427, -1427, -1427, -1427, -1427, -1427, -1427, -1092, -1427,  1879,
    1828,  -342, -1427, -1427,   143,  3410,  4211, -1427, -1427,   265,
    -133,  -581, -1427, -1427,   339,  -659,   132, -1427, -1427, -1427,
   -1427, -1427,   329, -1427, -1427, -1427,    45,  -814,  -184,  -164,
    -132, -1427, -1427,    51, -1427, -1427, -1427, -1427,    17,  -170,
   -1427,   112, -1427, -1427, -1427,  -395,   931, -1427, -1427, -1427,
   -1427, -1427,   913, -1427, -1427, -1427,   274, -1427, -1427, -1427,
      41,   305, -1427, -1427,   944,  -299,  -985, -1427,   -44,   -83,
    -199,  -212,   518, -1427, -1008, -1427,   219,   299, -1427, -1427,
   -1427,  -178, -1015
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -887
static const yytype_int16 yytable[] =
{
     121,   346,   417,   390,   135,   308,   130,   419,   260,   236,
     128,   558,   314,   315,   265,   849,   132,   133,   896,  1041,
     241,   649,   129,   673,   245,   439,  1211,   831,   670,   830,
    1208,  1031,   277,   412,   389,   915,   629,   804,   929,  1196,
    1195,   319,   710,   336,  1638,   248,   444,   229,   258,   162,
     346,   342,  1056,   321,   304,   164,   691,   305,   712,   930,
    1264,   779,  1455,  -768,  1011,   290,   445,   453,  1067,   779,
    1300,   510,   515,   466,   467,   414,  1601,   215,   216,   471,
     518,   410,   306,  1405,   290,  -281,   410,   436,  1459,  1543,
    1608,   290,   290,  1608,  1455,   284,   810,   793,   793,   446,
    1602,   278,   793,  -765,   793,   793,  1104,   950,   316,   533,
      13,    13,  1202,   407,   407,   498,   421,   393,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,  1618,
     290,    13,   407,   786,   787,   410,  -886,  1568,  1569,   429,
     557,  1085,   837,  -772,    13,   696,  1624,   443,  -766,    13,
    1676,   437,    13,   345,     3,   592,  -617,   333,   327,   329,
     330,  1203,   391,  -616,   322,   405,   406,   297,  1130,   602,
     534,  1301,   499,  1619,  -767,  -803,  1302,  -769,    58,    59,
      60,   176,   177,   343,  1303,   414,  -804,   577,  -806,   803,
    -770,  -886,   426,   761,  1677,  -771,  -774,  -805,   811,  -768,
    1663,  1086,   420,   188,   209,   689,   378,   909,   910,   209,
    1318,   415,  1332,   812,  -440,   641,   523,   411,   379,   931,
    1304,  1305,   411,  1306,  1012,   317,   279,  -476,   524,   407,
     713,  1265,   121,   711,  1334,   337,   121,   641,  1639,  -765,
     460,  1340,   450,  1342,   674,  1341,   455,   344,   504,   508,
     509,  1111,   780,  1115,  1456,  1457,   600,   346,   473,   454,
     781,   641,   430,   511,   516,  -222,  1232,  1603,   432,  1307,
     641,   411,   519,   641,   438,  1406,  -775,  -281,   543,  -772,
    1460,  1544,  1609,  -222,  -766,  1653,  1715,   164,   794,   883,
     715,   164,   598,  1188,   680,  1359,  1408,  1257,  1024,  -208,
     232,  -699,   577,  1244,  -699,   308,   342,  -699,  1053,   295,
    -767,  -803,   309,  -769,   540,  1042,  1324,  1720,  1733,   121,
     295,   415,  -804,   135,  -806,  1423,  -770,  1425,   589,   416,
     514,  -771,   258,  -805,   116,   290,   235,   520,   520,   525,
     295,   129,  1113,  1114,   530,   540,  1113,  1114,   242,   295,
     539,  1380,  1381,   650,   296,  1431,   681,   243,  -886,  1043,
    1325,  1721,  1734,   573,    58,    59,    60,   176,   177,   343,
     244,   512,   269,   333,   164,   298,   299,  1095,  1096,   333,
     282,   639,   290,   643,   290,   290,   298,   299,   827,   828,
     767,   838,   771,   286,   912,   229,   835,  1017,   287,   914,
     772,  1596,   640,   666,  -886,   309,   298,   299,   646,  1000,
    -886,   839,   699,  -886,   297,   298,   299,  1597,  1572,  1716,
    1717,   816,   295,  1003,   667,   390,   332,   683,   821,  1247,
     295,   825,  1254,   344,  1598,   540,  1645,  1646,   588,   693,
     866,  1044,  1326,  1722,  1735,   332,  1116,   332,   640,  1300,
    1267,   332,   121,  1697,   295,   332,   389,   690,   283,   325,
     694,   295,   704,    51,   332,   288,   328,   876,   587,   530,
     872,    58,    59,    60,   176,   177,   343,   764,   332,   430,
     295,   768,  1064,   289,   866,   540,   277,   535,   298,   299,
      13,   293,  1073,  1369,   633,   541,   298,   299,   838,  1197,
    1210,   545,   546,   864,   903,  1641,  1642,   164,   774,   938,
     941,  1221,  1198,   294,  1223,   310,   558,   311,   839,   856,
     298,   299,   668,   312,   904,   313,   671,   298,   299,   798,
     800,  1070,  1604,   324,    33,    34,    35,   331,   439,   332,
     344,   335,   347,   592,   592,   728,   298,   299,  1199,  1605,
    1301,   338,  1606,   348,   349,  1302,   905,    58,    59,    60,
     176,   177,   343,  1303,   350,  1647,   351,  1626,  1436,    58,
      59,    60,   176,   177,   343,   981,   982,   983,   352,  1347,
    -474,  1348,  1648,   381,   391,  1649,   935,  1112,  1113,  1114,
     382,   984,    72,    73,    74,    75,    76,   383,   290,  1304,
    1305,   536,  1306,   730,   384,   542,   478,   479,   480,    79,
      80,   385,   413,   481,   482,   573,  -773,   483,   484,  1110,
    1300,  -475,  -616,    89,   418,   302,   344,   423,  1025,   425,
     536,   379,   542,   536,   542,   542,   333,   431,   344,   407,
      97,  1261,  1113,  1114,   845,   557,   434,   435,  1317,  1190,
     441,  1037,  -615,   558,    58,    59,    60,    61,    62,   343,
     440,    13,   443,   451,  1048,    68,   386,   641,  1323,   468,
     375,   376,   377,   894,   378,   899,   469,   464,  1227,   913,
    -881,  1336,  1701,  1702,  1703,   472,   379,  1236,   474,  1712,
    1710,  1435,   121,   475,   477,   517,   135,   526,   527,   874,
     387,   566,   388,   567,  1726,  1723,   922,   121,   575,   576,
     -65,  1256,    51,   679,   129,   586,   702,   924,   599,   677,
     709,  1301,   682,   344,   453,   688,  1302,   721,    58,    59,
      60,   176,   177,   343,  1303,   900,   722,   901,   706,   747,
     748,   750,   504,   751,   759,   762,   508,   164,   765,   766,
     769,   770,  1117,   778,  1118,   790,   792,   121,   782,   920,
     783,   135,   164,  1002,  1089,   785,   801,  1005,  1006,   795,
    1304,  1305,   807,  1306,   806,  1576,   814,   809,   815,   129,
     817,   818,   557,   819,   820,  1013,   823,   824,   558,  1069,
     826,   829,   832,   833,  1314,  -477,   842,   344,   844,   847,
     121,  1432,   848,   850,   135,   853,   130,   859,  1123,   860,
     128,   863,   164,   867,  1032,  1127,   132,   133,   862,  1424,
     880,   877,   129,    58,    59,    60,    61,    62,   343,   878,
    1009,   881,  1331,  1046,    68,   386,   855,   916,   932,   906,
     933,  1338,   926,   530,  1019,   928,   934,  1216,   949,   162,
    1345,   936,   951,   952,  1047,   164,   953,   954,   955,   957,
    1690,  1209,   958,   835,   986,   988,   992,   987,   290,   995,
     998,   388,  1008,  1010,  1020,  1016,  1296,  1690,   573,  1029,
    1078,  1078,   894,  1021,  1036,  1711,  1098,  1027,  1230,  1038,
     229,  1039,   344,  1054,  1063,  1072,   573,  1066,  1074,  1088,
     372,   373,   374,   375,   376,   377,   121,   378,   121,  1090,
     121,  1107,   135,  1084,   135,  1087,  1099,   557,  1627,   379,
     978,   979,   980,   981,   982,   983,  1101,  1103,  1109,  1119,
     129,  1106,   129,  1121,  1122,   984,  1125,  1126,   535,   984,
    1185,  1179,  1186,  1048,  1189,  1419,  1355,  1129,  1205,  1207,
    1241,  1212,  1213,  1142,  1214,  1217,  1222,  1220,  1224,  1226,
    1229,   164,  1364,   164,  1228,   164,  1237,   920,  1105,  1231,
    1235,  1238,  1240,  1243,  1239,  1250,  1251,  1253,   558,  1258,
    1268,  1191,   421,   393,   394,   395,   396,   397,   398,   399,
     400,   401,   402,   403,   404,  1271,  1181,  1262,  1278,  1182,
    1272,  1269,  1282,  1275,  1276,  1286,  1277,  1298,  1279,  1280,
    1281,  1285,  1291,  1300,  1284,  1287,  1289,  1290,  1295,  1315,
     121,  1316,  1330,  1343,   135,  1327,   130,  1333,  1335,  1344,
     128,   405,   406,  1564,  1339,  1346,   132,   133,  1349,  1659,
    1350,  1351,   129,  1352,  1354,  1379,  1356,  1357,  1358,  1394,
    1437,  1361,  1410,   558,    13,  1377,  1362,  1414,  1415,  1443,
    1416,  1421,  1418,  1434,   212,   212,  1420,  1446,   224,   162,
    1429,  1426,  1249,  1450,  1454,   164,  1562,  1427,  1433,  1444,
     894,  1565,  1458,  1551,   894,  1552,  1566,  1567,  1575,  1577,
     224,  1365,  1588,  1366,   121,   407,  1589,  1607,  1612,   573,
    1218,  1615,   573,  1614,  1252,  1617,   121,   557,  1700,  1300,
     135,  1623,  1643,  1353,  1301,  1651,  1652,  1658,  1657,  1302,
    1665,    58,    59,    60,   176,   177,   343,  1303,   129,  1666,
    1667,  -277,  1669,  1404,  1672,  1602,  1582,  1670,  1673,  1675,
    1692,  1678,  1682,  1248,  1680,  1681,  1695,  1698,  1699,   164,
      13,  1707,  1724,  1709,  1407,  1713,  1714,   530,   920,  1727,
    1728,   164,  1736,  1304,  1305,  1729,  1306,  1739,   784,  1740,
    1741,  1743,  1001,   773,  1004,   645,  1694,   346,   642,   644,
    1297,  1068,   557,  1065,  1708,  1026,  1581,  1311,  1363,  1706,
     344,   776,  1573,  1595,  1255,  1600,  1311,  1401,  1730,  1719,
    1611,  1451,   238,  1571,   652,  1178,   757,  1176,   758,  1200,
    1301,   994,  1428,  1234,  1128,  1302,  1242,    58,    59,    60,
     176,   177,   343,  1303,  1080,   532,   565,  1554,  1091,  1139,
    1184,   522,  1313,  1120,   894,     0,   894,     0,     0,   939,
     530,  1313,     0,     0,     0,     0,     0,     0,     0,   212,
       0,     0,     0,     0,     0,   212,     0,  1382,     0,  1304,
    1305,   212,  1306,  1621,     0,  1622,     0,     0,     0,   573,
       0,     0,     0,     0,  1628,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   344,     0,     0,   224,
     224,     0,   121,     0,     0,   224,   135,   258,     0,     0,
       0,     0,  1399,     0,     0,     0,     0,     0,  1430,     0,
       0,     0,     0,   391,   129,     0,     0,   212,     0,  1662,
       0,     0,     0,     0,   212,   212,     0,     0,  1403,     0,
       0,   212,     0,     0,  1311,     0,     0,   212,     0,     0,
    1311,     0,  1311,     0,     0,     0,   894,   164,   224,     0,
       0,   121,     0,     0,     0,   135,   121,     0,     0,     0,
     121,  1556,     0,     0,   135,     0,  1441,     0,     0,  1300,
       0,   224,     0,   129,   224,     0,     0,     0,     0,  1313,
       0,  1613,   129,     0,     0,  1313,     0,  1313,  1540,   573,
       0,     0,     0,     0,  1547,     0,     0,     0,     0,     0,
       0,   258,     0,     0,     0,   258,   164,     0,     0,     0,
      13,   164,     0,  1725,     0,   164,   224,     0,     0,     0,
    1731,     0,     0,     0,     0,  1557,     0,     0,     0,     0,
     894,  1311,     0,   121,   121,   121,     0,   135,     0,   121,
       0,     0,     0,   135,  1579,  1441,   121,     0,     0,     0,
     135,     0,     0,     0,     0,   129,   212,     0,     0,     0,
       0,   129,     0,     0,     0,     0,   212,     0,   129,     0,
    1301,  1610,     0,     0,     0,  1302,  1313,    58,    59,    60,
     176,   177,   343,  1303,     0,     0,     0,     0,   164,   164,
     164,     0,     0,     0,   164,     0,     0,     0,     0,     0,
       0,   164,     0,     0,     0,     0,   224,   224,     0,     0,
     737,  1685,     0,     0,     0,     0,     0,     0,     0,  1304,
    1305,     0,  1306,     0,     0,     0,   835,     0,     0,     0,
    1655,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   835,     0,     0,     0,     0,   344,     0,     0,     0,
     290,     0,   346,     0,     0,   737,  -887,  -887,  -887,  -887,
     976,   977,   978,   979,   980,   981,   982,   983,  1574,   258,
       0,     0,     0,   894,     0,     0,     0,     0,   121,     0,
       0,   984,   135,     0,     0,     0,    36,     0,  1633,     0,
       0,     0,     0,  1540,  1540,     0,     0,  1547,  1547,     0,
     129,     0,     0,     0,   224,   224,     0,     0,     0,   290,
       0,     0,   224,     0,     0,     0,     0,   121,   121,     0,
       0,   135,   135,     0,   121,     0,     0,     0,   135,     0,
       0,   212,     0,   164,     0,     0,     0,     0,     0,   129,
     129,     0,     0,     0,     0,     0,   129,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   180,   121,     0,
      81,     0,   135,    83,    84,  1684,    85,   181,    87,     0,
    1686,     0,   164,   164,     0,     0,     0,     0,     0,   164,
     129,  1696,     0,     0,     0,     0,   212,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,     0,     0,     0,
       0,     0,  1632,   164,     0,     0,     0,     0,     0,   573,
       0,   121,   212,     0,   212,   135,     0,   121,     0,     0,
       0,   135,     0,     0,     0,     0,   573,     0,     0,     0,
       0,     0,     0,   129,   573,     0,   212,   737,     0,   129,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   224,
     224,   737,   737,   737,   737,   737,     0,     0,     0,     0,
       0,     0,     0,     0,   737,     0,   164,     0,     0,     0,
       0,     0,   164,   392,   393,   394,   395,   396,   397,   398,
     399,   400,   401,   402,   403,   404,     0,     0,     0,     0,
     224,  -887,  -887,  -887,  -887,   370,   371,   372,   373,   374,
     375,   376,   377,     0,   378,     0,     0,   212,     0,     0,
       0,     0,     0,     0,     0,     0,   379,     0,   224,     0,
     212,   212,   405,   406,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   224,   224,     0,     0,     0,     0,
       0,     0,     0,   224,   214,   214,     0,     0,   226,     0,
       0,     0,     0,     0,     0,     0,     0,   224,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     224,     0,     0,     0,     0,     0,     0,     0,   737,     0,
       0,   224,     0,   353,   354,   355,   407,     0,     0,     0,
       0,     0,     0,     0,     0,   213,   213,     0,     0,   225,
       0,   224,   356,     0,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,     0,   378,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   379,     0,
       0,     0,     0,     0,   212,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   224,     0,
     224,     0,   224,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   737,     0,     0,
     224,   737,   737,   737,     0,     0,   737,   737,   737,   737,
     737,   737,   737,   737,   737,   737,   737,   737,   737,   737,
     737,   737,   737,   737,   737,   737,   737,   737,   737,   737,
     737,   737,   737,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   353,   354,   355,     0,     0,     0,   214,
       0,     0,     0,     0,     0,   214,   737,     0,     0,     0,
       0,   214,   356,     0,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,     0,   378,   224,     0,   224,
       0,     0,     0,     0,     0,     0,     0,   212,   379,  1193,
       0,     0,     0,     0,     0,     0,   213,     0,     0,     0,
       0,     0,     0,    36,   224,   209,     0,   214,     0,     0,
       0,     0,     0,     0,   214,   214,     0,     0,     0,     0,
       0,   214,     0,     0,     0,   224,     0,   214,     0,     0,
     212,     0,     0,     0,     0,     0,     0,     0,   556,     0,
       0,     0,     0,     0,   212,   212,     0,   737,     0,   637,
       0,     0,     0,     0,     0,     0,     0,     0,   213,   224,
       0,     0,   737,     0,   737,   213,   213,     0,     0,     0,
       0,     0,   213,     0,     0,     0,     0,     0,   213,     0,
      83,    84,     0,    85,   181,    87,     0,   737,     0,   213,
       0,     0,     0,   421,   393,   394,   395,   396,   397,   398,
     399,   400,   401,   402,   403,   404,   226,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,   224,   224,     0,   802,   212,   638,     0,
     116,     0,     0,     0,     0,     0,     0,     0,     0,   353,
     354,   355,   405,   406,     0,     0,   214,     0,     0,     0,
       0,   737,     0,     0,     0,     0,   214,   225,   356,     0,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,     0,   378,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   379,     0,     0,   213,     0,     0,
       0,     0,     0,     0,     0,     0,   407,   224,     0,   224,
       0,     0,     0,     0,   737,   224,     0,     0,     0,   737,
       0,   737,     0,   737,     0,     0,   737,     0,   737,     0,
       0,   737,     0,     0,     0,     0,     0,     0,     0,   224,
     224,     0,   224,     0,     0,   353,   354,   355,     0,   224,
       0,   743,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   356,   737,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,     0,   378,     0,
     224,     0,     0,    36,     0,   209,   743,     0,     0,     0,
     379,     0,     0,     0,     0,   737,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   224,   224,   224,
       0,   214,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   841,   210,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   224,     0,     0,     0,   224,     0,     0,
       0,     0,     0,   737,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   180,   259,     0,    81,    82,     0,
      83,    84,   213,    85,   181,    87,   214,     0,     0,     0,
       0,     0,     0,     0,   737,   737,   737,     0,     0,     0,
       0,     0,   737,   224,     0,     0,     0,   224,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   214,     0,   214,   211,     0,     0,   513,     0,
     116,   353,   354,   355,     0,     0,     0,   213,     0,     0,
       0,     0,     0,     0,     0,     0,   214,     0,   879,     0,
     356,     0,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   213,   378,   213,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   379,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   213,   743,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     737,     0,   743,   743,   743,   743,   743,   214,     0,     0,
       0,   224,     0,     0,     0,   743,     0,     0,     0,     0,
     214,   214,     0,     0,     0,     0,     0,     0,     0,     0,
     224,     0,     0,     0,   737,     0,     0,     0,     0,     0,
       0,   997,     0,   556,     0,   737,     0,     0,     0,     0,
     737,     0,     0,   737,     0,     0,     0,     0,   213,     0,
       0,     0,     0,     0,   259,   259,   917,     0,     0,  1015,
     259,   213,   213,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1015,     0,     0,     0,
       0,    36,     0,     0,   213,     0,     0,     0,     0,     0,
       0,   226,     0,     0,     0,     0,     0,     0,    36,     0,
     209,   737,     0,     0,   882,     0,     0,     0,     0,   224,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   743,
       0,     0,  1055,   224,     0,  1397,     0,     0,     0,     0,
       0,     0,   224,     0,   214,   214,   259,     0,   210,   259,
       0,     0,   225,     0,     0,     0,     0,   224,     0,     0,
     918,     0,     0,     0,     0,     0,   737,     0,    83,    84,
       0,    85,   181,    87,   737,     0,     0,     0,   737,   180,
     556,   737,    81,    82,     0,    83,    84,     0,    85,   181,
      87,     0,     0,     0,     0,   213,   213,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,     0,     0,  1398,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,   743,     0,
     211,   213,   743,   743,   743,   116,     0,   743,   743,   743,
     743,   743,   743,   743,   743,   743,   743,   743,   743,   743,
     743,   743,   743,   743,   743,   743,   743,   743,   743,   743,
     743,   743,   743,   743,     0,     0,     0,   353,   354,   355,
       0,     0,     0,     0,     0,     0,     0,   214,     0,     0,
       0,   259,   717,     0,     0,   739,   356,   743,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,     0,
     378,     0,     0,     0,     0,   556,     0,   353,   354,   355,
     214,     0,   379,     0,     0,     0,     0,     0,   213,     0,
     739,     0,     0,     0,   214,   214,   356,     0,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,     0,
     378,     0,     0,     0,     0,     0,   213,     0,     0,     0,
       0,   213,   379,     0,     0,     0,     0,     0,     0,   259,
     259,     0,     0,     0,     0,   213,   213,   259,   743,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   743,     0,   743,     0,     0,   353,   354,
     355,     0,     0,     0,     0,     0,     0,   214,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   356,   743,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     355,   378,     0,     0,     0,     0,     0,     0,     0,     0,
    1007,     0,     0,   379,     0,  1299,     0,   356,   213,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
       0,   378,   743,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   379,     0,   556,     0,     0,     0,     0,
    1062,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   739,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   259,   259,   739,   739,   739,   739,
     739,     0,     0,     0,     0,     0,     0,     0,     0,   739,
       0,     0,     0,     0,     0,   743,   213,     0,     0,     0,
     743,     0,   743,     0,   743,     0,     0,   743,     0,   743,
       0,     0,   743,     0,     0,     0,     0,     0,     0,     0,
     556,  1384,     0,  1393,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,  1071,   378,     0,     0,     0,   743,     0,     0,     0,
       0,     0,     0,     0,   379,     0,     0,     0,     0,   259,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   213,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   259,   353,   354,   355,   743,     0,     0,     0,
       0,     0,     0,     0,     0,   259,     0,     0,     0,  1452,
    1453,     0,   356,   739,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,     0,   378,     0,     0,     0,
       0,     0,     0,     0,   743,     0,     0,     0,   379,     0,
       0,     0,     0,   251,   964,   965,   966,   967,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   977,   978,   979,
     980,   981,   982,   983,     0,   743,   743,   743,     0,   252,
       0,     0,     0,   743,  1591,     0,     0,   984,  1393,     0,
       0,     0,     0,   259,     0,   259,     0,   717,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   739,     0,     0,     0,   739,   739,   739,     0,
       0,   739,   739,   739,   739,   739,   739,   739,   739,   739,
     739,   739,   739,   739,   739,   739,   739,   739,   739,   739,
     739,   739,   739,   739,   739,   739,   739,   739,     0,     0,
       0,     0,     0,     0,   253,   254,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   739,   180,     0,     0,    81,   255,     0,    83,    84,
       0,    85,   181,    87,     0,   937,  1094,     0,     0,     0,
       0,   743,     0,     0,     0,     0,   256,     0,     0,     0,
       0,     0,   259,     0,   259,     0,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,     0,     0,   257,     0,   743,   353,   354,   355,   259,
       0,     0,     0,     0,     0,     0,   743,     0,     0,     0,
       0,   743,     0,     0,   743,   356,     0,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,     0,   378,
       0,     0,   739,     0,     0,     0,     0,     0,     0,     0,
       0,   379,     0,     0,   259,     0,     0,   739,     0,   739,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   743,     0,   353,   354,   355,     0,     0,     0,
    1693,     0,   739,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   356,  1384,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,     0,   378,   259,     0,
       0,     0,     0,     0,     0,     0,     0,   743,     0,   379,
     353,   354,   355,     0,     0,   743,     0,     0,     0,   743,
       0,     0,   743,     0,     0,     0,   739,     0,     0,   356,
       0,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,     0,   378,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   379,     0,     0,     0,  1329,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   259,     0,   259,     0,     0,     0,     0,   739,
       0,     0,     0,     0,   739,     0,   739,     0,   739,     0,
       0,   739,     0,   739,     0,     0,   739,     0,     0,     0,
       0,     0,     0,     0,   259,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   259,     0,     0,     0,     0,   353,
     354,   355,     0,     0,     0,     0,     0,     0,     0,     0,
     739,     0,     0,     0,     0,     0,     0,  1412,   356,  1264,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,     0,   378,     0,     0,     0,     0,     0,     0,     0,
     739,     0,     0,     0,   379,     0,     0,     0,     0,     0,
       0,     0,   259,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1413,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   259,     0,
     185,   187,   259,   189,   190,   191,   193,   194,   739,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,     0,     0,   218,   221,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   239,     0,     0,   739,
     739,   739,     0,   247,     0,   250,     0,   739,   266,     0,
     271,   353,   354,   355,     0,     0,     0,     0,     0,     0,
       0,     0,   744,    36,     0,   209,     0,     0,     0,     0,
     356,     0,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,     0,   378,   353,   354,   355,     0,     0,
    1265,     0,     0,     0,     0,     0,   379,   744,   320,     0,
       0,     0,     0,     0,   356,     0,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,     0,   378,     0,
      83,    84,     0,    85,   181,    87,     0,     0,     0,     0,
     379,     0,     0,     0,     0,   739,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   259,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,   422,     0,  1634,     0,     0,   665,   739,
     116,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     739,     0,     0,     0,     0,   739,     0,     0,   739,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   353,   354,   355,     0,     0,   448,     0,     0,
     448,     0,     0,     0,     0,     0,     0,   239,   459,     0,
       0,   356,   380,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,     0,   378,   739,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   379,     0,     0,
       0,     0,     0,   320,     0,     0,   463,     0,     0,   218,
       0,     0,     0,   538,     0,     0,     0,   259,     0,   744,
       0,     0,     0,     0,     0,     0,   559,   562,   562,     0,
       0,     0,   259,   744,   744,   744,   744,   744,     0,   585,
       0,   739,     0,     0,     0,     0,   744,     0,     0,   739,
     597,     0,     0,   739,     0,     0,   739,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   603,   604,
     605,   607,   608,   609,   610,   611,   612,   613,   614,   615,
     616,   617,   618,   619,   620,   621,   622,   623,   624,   625,
     626,   627,   628,     0,   630,     0,   631,   631,     0,   634,
       0,     0,     0,     0,     0,     0,     0,   651,   653,   654,
     655,   656,   657,   658,   659,   660,   661,   662,   663,   664,
       0,     0,     0,     0,     0,   631,   669,     0,   597,   631,
     672,     0,     0,   465,     0,     0,   651,     0,     0,   676,
       0,     0,     0,     0,     0,     0,     0,     0,   685,     0,
     687,     0,   353,   354,   355,     0,   597,     0,     0,     0,
     744,     0,     0,   738,     0,     0,   700,     0,   701,     0,
       0,   356,     0,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   749,   378,     0,   752,   755,   756,
       0,     0,     0,     0,     0,     0,     0,   379,   738,     0,
       0,   353,   354,   355,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   775,     0,
     356,     0,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,     0,   378,     0,     0,     0,     0,   744,
       0,     0,     0,   744,   744,   744,   379,     0,   744,   744,
     744,   744,   744,   744,   744,   744,   744,   744,   744,   744,
     744,   744,   744,   744,   744,   744,   744,   744,   744,   744,
     744,   744,   744,   744,   744,     0,     0,     0,     0,     0,
       0,   353,   354,   355,     0,     0,     0,     0,     0,   846,
       0,     0,     0,     0,     0,     0,     0,     0,   744,     0,
     356,   857,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   476,   378,     0,     0,     0,     0,     0,
       0,     0,     0,   865,     0,     0,   379,     0,     0,     0,
       0,     0,   193,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     875,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     738,   378,   500,     0,     0,     0,     0,   907,     0,     0,
       0,     0,     0,   379,   738,   738,   738,   738,   738,   744,
       0,   239,     0,     0,     0,     0,     0,   738,     0,     0,
       0,     0,     0,     0,   744,     0,   744,    36,     0,   209,
       0,     0,   353,   354,   355,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   744,
       0,   356,   985,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,     0,   378,     0,     0,     0,     0,
       0,   675,     0,     0,     0,     0,     0,   379,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   977,   978,   979,
     980,   981,   982,   983,    83,    84,  1022,    85,   181,    87,
       0,     0,     0,   744,     0,     0,     0,   984,     0,  1030,
       0,     0,     0,     0,     0,  1033,     0,  1034,     0,  1035,
       0,   738,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,     0,     0,
       0,     0,   638,  1051,   116,     0,     0,     0,     0,     0,
       0,     0,     0,  1059,     0,     0,  1060,     0,  1061,     0,
       0,     0,   597,   745,     0,     0,   744,     0,     0,     0,
       0,   744,   597,   744,     0,   744,     0,     0,   744,     0,
     744,     0,     0,   744,    36,     0,   209,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1093,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   777,     0,
       0,     0,   695,     0,     0,     0,     0,   744,     0,     0,
     738,     0,     0,     0,   738,   738,   738,     0,     0,   738,
     738,   738,   738,   738,   738,   738,   738,   738,   738,   738,
     738,   738,   738,   738,   738,   738,   738,   738,   738,   738,
     738,   738,   738,   738,   738,   738,     0,   744,     0,     0,
       0,    83,    84,     0,    85,   181,    87,     0,     0,     0,
       0,     0,  1075,  1076,  1077,    36,     0,     0,     0,   738,
       0,  1173,  1174,  1175,     0,     0,     0,   752,  1177,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   744,     0,     0,     0,   692,
       0,   116,     0,     0,     0,  1192,     0,   965,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     978,   979,   980,   981,   982,   983,   744,   744,   744,     0,
    1215,     0,     0,     0,   744,  1219,     0,     0,  1594,   984,
       0,     0,    83,    84,     0,    85,   181,    87,   597,     0,
       0,     0,     0,     0,     0,     0,     0,   597,     0,  1192,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     738,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,   738,     0,   738,   239,     0,
     921,     0,     0,     0,     0,     0,     0,     0,  1263,   959,
     960,   961,     0,     0,   942,   943,   944,   945,     0,     0,
     738,     0,     0,     0,     0,     0,     0,   956,   962,     0,
     963,   964,   965,   966,   967,   968,   969,   970,   971,   972,
     973,   974,   975,   976,   977,   978,   979,   980,   981,   982,
     983,     0,   744,     0,     0,     0,     0,     0,    29,    30,
       0,     0,     0,     0,   984,     0,     0,     0,    36,     0,
     209,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   738,     0,   744,     0,     0,     0,
       0,     0,     0,     0,  1319,     0,     0,   744,  1320,     0,
    1321,  1322,   744,     0,     0,   744,     0,     0,   210,     0,
       0,     0,   597,     0,     0,     0,     0,     0,     0,     0,
    1337,   597,     0,     0,     0,     0,     0,     0,  1671,     0,
     597,     0,     0,     0,     0,     0,     0,     0,     0,   180,
       0,  1052,    81,    82,     0,    83,    84,   738,    85,   181,
      87,     0,   738,     0,   738,     0,   738,    90,     0,   738,
       0,   738,     0,   744,   738,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,  1376,
     428,     0,     0,     0,     0,   116,     0,     0,   738,  1140,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   744,   378,
       0,     0,     0,     0,    36,   597,   744,     0,     0,     0,
     744,   379,     0,   744,     0,     0,     0,     0,   738,     0,
       0,     0,     0,     0,  1133,  1136,  1136,     0,     0,  1143,
    1146,  1147,  1148,  1150,  1151,  1152,  1153,  1154,  1155,  1156,
    1157,  1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
    1167,  1168,  1169,  1170,  1171,  1172,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   738,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,     0,  1183,
       0,    83,    84,     0,    85,   181,    87,     0,     0,     0,
       0,     0,     0,   597,     0,     0,     0,   738,   738,   738,
       0,     0,     0,     0,     0,   738,   353,   354,   355,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,  1586,   356,   599,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,     0,   378,
       0,  1545,     0,    83,    84,  1546,    85,   181,    87,     0,
       0,   379,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1259,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,  1273,     0,  1274,  1398,     0,
     353,   354,   355,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   738,     0,     0,     0,     0,     0,   356,
    1292,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,     0,   378,     0,     0,     0,   738,     0,     0,
       0,     0,     0,     0,     0,   379,     0,     0,   738,    36,
       0,   209,     0,   738,     0,     0,   738,     0,     0,     0,
       0,   353,   354,   355,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1328,     0,     0,     0,     0,     0,
     356,   993,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,     0,   378,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   738,     0,   379,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    83,    84,     0,    85,
     181,    87,     0,     0,     0,     0,     0,  1368,     0,     0,
       0,     0,  1370,     0,  1371,     0,  1372,     0,     0,  1373,
       0,  1374,     0,     0,  1375,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,   738,
       0,   989,   990,     0,  1045,     0,   116,   738,     0,     0,
       0,   738,     0,     0,   738,     0,     0,     0,  1417,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,   982,   983,     0,     0,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     984,     0,     0,     0,     0,     0,     0,     0,  1445,     0,
       0,     0,     0,   647,    12,     0,     0,     0,     0,  1630,
       0,   648,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,  1563,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,    40,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1583,  1584,  1585,
      51,     0,     0,     0,     0,  1590,     0,     0,    58,    59,
      60,   176,   177,   178,     0,     0,    65,    66,     0,     0,
       0,     0,     0,     0,     0,   179,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   180,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   181,    87,     0,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,    94,     0,     0,     0,     0,    97,    98,   263,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,     0,     0,     0,
     116,   117,     0,   118,   119,     0,     0,     0,     0,     0,
       0,     0,     0,  1616,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,  1640,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,  1650,     0,
       0,     0,     0,  1654,     0,     0,  1656,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,     0,     0,
       0,    40,    41,    42,    43,     0,    44,     0,    45,     0,
      46,     0,     0,    47,  1687,     0,     0,    48,    49,    50,
      51,    52,    53,    54,     0,    55,    56,    57,    58,    59,
      60,    61,    62,    63,     0,    64,    65,    66,    67,    68,
      69,     0,     0,     0,     0,    70,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,    78,    79,    80,    81,    82,  1737,
      83,    84,     0,    85,    86,    87,    88,  1742,     0,    89,
       0,  1744,    90,     0,  1745,     0,     0,     0,    91,    92,
      93,    94,    95,     0,    96,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,  1023,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,     0,     0,     0,    40,    41,    42,
      43,     0,    44,     0,    45,     0,    46,     0,     0,    47,
       0,     0,     0,    48,    49,    50,    51,    52,    53,    54,
       0,    55,    56,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,    70,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
      78,    79,    80,    81,    82,     0,    83,    84,     0,    85,
      86,    87,    88,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,    94,    95,     0,
      96,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,  1194,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
       0,     0,     0,    40,    41,    42,    43,     0,    44,     0,
      45,     0,    46,     0,     0,    47,     0,     0,     0,    48,
      49,    50,    51,    52,    53,    54,     0,    55,    56,    57,
      58,    59,    60,    61,    62,    63,     0,    64,    65,    66,
      67,    68,    69,     0,     0,     0,     0,    70,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,    78,    79,    80,    81,
      82,     0,    83,    84,     0,    85,    86,    87,    88,     0,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,    92,    93,    94,    95,     0,    96,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,     0,     0,     0,    40,
      41,    42,    43,     0,    44,     0,    45,     0,    46,     0,
       0,    47,     0,     0,     0,    48,    49,    50,    51,     0,
      53,    54,     0,    55,     0,    57,    58,    59,    60,    61,
      62,    63,     0,    64,    65,    66,     0,    68,    69,     0,
       0,     0,     0,    70,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   180,    79,    80,    81,    82,     0,    83,    84,
       0,    85,   181,    87,    88,     0,     0,    89,     0,     0,
      90,     0,     0,     0,     0,     0,    91,    92,    93,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,   578,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,     0,     0,     0,    40,    41,    42,    43,     0,
      44,     0,    45,     0,    46,     0,     0,    47,     0,     0,
       0,    48,    49,    50,    51,     0,    53,    54,     0,    55,
       0,    57,    58,    59,    60,    61,    62,    63,     0,    64,
      65,    66,     0,    68,    69,     0,     0,     0,     0,    70,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   180,    79,
      80,    81,    82,     0,    83,    84,     0,    85,   181,    87,
      88,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,   996,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,     0,     0,
       0,    40,    41,    42,    43,     0,    44,     0,    45,     0,
      46,     0,     0,    47,     0,     0,     0,    48,    49,    50,
      51,     0,    53,    54,     0,    55,     0,    57,    58,    59,
      60,    61,    62,    63,     0,    64,    65,    66,     0,    68,
      69,     0,     0,     0,     0,    70,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   180,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   181,    87,    88,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,  1100,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,     0,     0,     0,    40,    41,    42,
      43,  1102,    44,     0,    45,     0,    46,     0,     0,    47,
       0,     0,     0,    48,    49,    50,    51,     0,    53,    54,
       0,    55,     0,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,     0,    68,    69,     0,     0,     0,
       0,    70,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     180,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     181,    87,    88,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,     0,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
       0,     0,     0,    40,    41,    42,    43,     0,    44,     0,
      45,     0,    46,  1260,     0,    47,     0,     0,     0,    48,
      49,    50,    51,     0,    53,    54,     0,    55,     0,    57,
      58,    59,    60,    61,    62,    63,     0,    64,    65,    66,
       0,    68,    69,     0,     0,     0,     0,    70,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   180,    79,    80,    81,
      82,     0,    83,    84,     0,    85,   181,    87,    88,     0,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,    92,    93,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,     0,     0,     0,    40,
      41,    42,    43,     0,    44,     0,    45,     0,    46,     0,
       0,    47,     0,     0,     0,    48,    49,    50,    51,     0,
      53,    54,     0,    55,     0,    57,    58,    59,    60,    61,
      62,    63,     0,    64,    65,    66,     0,    68,    69,     0,
       0,     0,     0,    70,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   180,    79,    80,    81,    82,     0,    83,    84,
       0,    85,   181,    87,    88,     0,     0,    89,     0,     0,
      90,     0,     0,     0,     0,     0,    91,    92,    93,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,  1378,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,     0,     0,     0,    40,    41,    42,    43,     0,
      44,     0,    45,     0,    46,     0,     0,    47,     0,     0,
       0,    48,    49,    50,    51,     0,    53,    54,     0,    55,
       0,    57,    58,    59,    60,    61,    62,    63,     0,    64,
      65,    66,     0,    68,    69,     0,     0,     0,     0,    70,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   180,    79,
      80,    81,    82,     0,    83,    84,     0,    85,   181,    87,
      88,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,  1587,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,     0,     0,
       0,    40,    41,    42,    43,     0,    44,     0,    45,  1629,
      46,     0,     0,    47,     0,     0,     0,    48,    49,    50,
      51,     0,    53,    54,     0,    55,     0,    57,    58,    59,
      60,    61,    62,    63,     0,    64,    65,    66,     0,    68,
      69,     0,     0,     0,     0,    70,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   180,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   181,    87,    88,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,     0,     0,     0,    40,    41,    42,
      43,     0,    44,     0,    45,     0,    46,     0,     0,    47,
       0,     0,     0,    48,    49,    50,    51,     0,    53,    54,
       0,    55,     0,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,     0,    68,    69,     0,     0,     0,
       0,    70,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     180,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     181,    87,    88,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,  1660,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
       0,     0,     0,    40,    41,    42,    43,     0,    44,     0,
      45,     0,    46,     0,     0,    47,     0,     0,     0,    48,
      49,    50,    51,     0,    53,    54,     0,    55,     0,    57,
      58,    59,    60,    61,    62,    63,     0,    64,    65,    66,
       0,    68,    69,     0,     0,     0,     0,    70,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   180,    79,    80,    81,
      82,     0,    83,    84,     0,    85,   181,    87,    88,     0,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,    92,    93,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,  1661,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,     0,     0,     0,    40,
      41,    42,    43,     0,    44,  1664,    45,     0,    46,     0,
       0,    47,     0,     0,     0,    48,    49,    50,    51,     0,
      53,    54,     0,    55,     0,    57,    58,    59,    60,    61,
      62,    63,     0,    64,    65,    66,     0,    68,    69,     0,
       0,     0,     0,    70,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   180,    79,    80,    81,    82,     0,    83,    84,
       0,    85,   181,    87,    88,     0,     0,    89,     0,     0,
      90,     0,     0,     0,     0,     0,    91,    92,    93,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,     0,     0,     0,    40,    41,    42,    43,     0,
      44,     0,    45,     0,    46,     0,     0,    47,     0,     0,
       0,    48,    49,    50,    51,     0,    53,    54,     0,    55,
       0,    57,    58,    59,    60,    61,    62,    63,     0,    64,
      65,    66,     0,    68,    69,     0,     0,     0,     0,    70,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   180,    79,
      80,    81,    82,     0,    83,    84,     0,    85,   181,    87,
      88,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,  1679,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,     0,     0,
       0,    40,    41,    42,    43,     0,    44,     0,    45,     0,
      46,     0,     0,    47,     0,     0,     0,    48,    49,    50,
      51,     0,    53,    54,     0,    55,     0,    57,    58,    59,
      60,    61,    62,    63,     0,    64,    65,    66,     0,    68,
      69,     0,     0,     0,     0,    70,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   180,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   181,    87,    88,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,  1732,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,     0,     0,     0,    40,    41,    42,
      43,     0,    44,     0,    45,     0,    46,     0,     0,    47,
       0,     0,     0,    48,    49,    50,    51,     0,    53,    54,
       0,    55,     0,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,     0,    68,    69,     0,     0,     0,
       0,    70,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     180,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     181,    87,    88,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,  1738,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
       0,     0,     0,    40,    41,    42,    43,     0,    44,     0,
      45,     0,    46,     0,     0,    47,     0,     0,     0,    48,
      49,    50,    51,     0,    53,    54,     0,    55,     0,    57,
      58,    59,    60,    61,    62,    63,     0,    64,    65,    66,
       0,    68,    69,     0,     0,     0,     0,    70,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   180,    79,    80,    81,
      82,     0,    83,    84,     0,    85,   181,    87,    88,     0,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,    92,    93,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,   449,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,     0,     0,     0,    40,
      41,    42,    43,     0,    44,     0,    45,     0,    46,     0,
       0,    47,     0,     0,     0,    48,    49,    50,    51,     0,
      53,    54,     0,    55,     0,    57,    58,    59,    60,   176,
     177,    63,     0,    64,    65,    66,     0,     0,     0,     0,
       0,     0,     0,    70,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   180,    79,    80,    81,    82,     0,    83,    84,
       0,    85,   181,    87,     0,     0,     0,    89,     0,     0,
      90,     0,     0,     0,     0,     0,    91,    92,    93,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
     703,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,     0,     0,     0,    40,    41,    42,    43,     0,
      44,     0,    45,     0,    46,     0,     0,    47,     0,     0,
       0,    48,    49,    50,    51,     0,    53,    54,     0,    55,
       0,    57,    58,    59,    60,   176,   177,    63,     0,    64,
      65,    66,     0,     0,     0,     0,     0,     0,     0,    70,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   180,    79,
      80,    81,    82,     0,    83,    84,     0,    85,   181,    87,
       0,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   114,   115,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,   923,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,     0,     0,
       0,    40,    41,    42,    43,     0,    44,     0,    45,     0,
      46,     0,     0,    47,     0,     0,     0,    48,    49,    50,
      51,     0,    53,    54,     0,    55,     0,    57,    58,    59,
      60,   176,   177,    63,     0,    64,    65,    66,     0,     0,
       0,     0,     0,     0,     0,    70,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   180,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   181,    87,     0,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,   114,   115,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,  1440,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,     0,     0,     0,    40,    41,    42,
      43,     0,    44,     0,    45,     0,    46,     0,     0,    47,
       0,     0,     0,    48,    49,    50,    51,     0,    53,    54,
       0,    55,     0,    57,    58,    59,    60,   176,   177,    63,
       0,    64,    65,    66,     0,     0,     0,     0,     0,     0,
       0,    70,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     180,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     181,    87,     0,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   114,   115,     0,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,  1578,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
       0,     0,     0,    40,    41,    42,    43,     0,    44,     0,
      45,     0,    46,     0,     0,    47,     0,     0,     0,    48,
      49,    50,    51,     0,    53,    54,     0,    55,     0,    57,
      58,    59,    60,   176,   177,    63,     0,    64,    65,    66,
       0,     0,     0,     0,     0,     0,     0,    70,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   180,    79,    80,    81,
      82,     0,    83,    84,     0,    85,   181,    87,     0,     0,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,    92,    93,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   114,
     115,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,     0,     0,     0,    40,
      41,    42,    43,     0,    44,     0,    45,     0,    46,     0,
       0,    47,     0,     0,     0,    48,    49,    50,    51,     0,
      53,    54,     0,    55,     0,    57,    58,    59,    60,   176,
     177,    63,     0,    64,    65,    66,     0,     0,     0,     0,
       0,     0,     0,    70,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   180,    79,    80,    81,    82,     0,    83,    84,
       0,    85,   181,    87,     0,     0,     0,    89,     0,     0,
      90,     0,     0,     0,     0,     0,    91,    92,    93,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,   114,   115,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,    40,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    51,     0,     0,     0,     0,     0,
       0,     0,    58,    59,    60,   176,   177,   178,     0,     0,
      65,    66,     0,     0,     0,     0,     0,     0,     0,   179,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   180,    79,
      80,    81,    82,     0,    83,    84,     0,    85,   181,    87,
       0,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,    94,     0,     0,     0,     0,
      97,    98,   263,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,   264,     0,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,   356,    10,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   593,   378,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,   379,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,    40,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      51,     0,     0,     0,     0,     0,     0,     0,    58,    59,
      60,   176,   177,   178,     0,     0,    65,    66,     0,     0,
       0,     0,     0,     0,     0,   179,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   180,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   181,    87,     0,   594,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,     0,     0,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,    40,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    51,     0,     0,     0,
       0,     0,     0,     0,    58,    59,    60,   176,   177,   178,
       0,     0,    65,    66,     0,     0,     0,     0,     0,     0,
       0,   179,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     180,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     181,    87,     0,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,   354,   355,   698,     0,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
     356,    10,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,  1049,   378,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,   379,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,    40,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    51,     0,     0,     0,     0,     0,     0,     0,
      58,    59,    60,   176,   177,   178,     0,     0,    65,    66,
       0,     0,     0,     0,     0,     0,     0,   179,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   180,    79,    80,    81,
      82,     0,    83,    84,     0,    85,   181,    87,     0,  1050,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,    92,    93,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,     0,
       0,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   647,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,    40,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    51,     0,
       0,     0,     0,     0,     0,     0,    58,    59,    60,   176,
     177,   178,     0,     0,    65,    66,     0,     0,     0,     0,
       0,     0,     0,   179,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   180,    79,    80,    81,    82,     0,    83,    84,
       0,    85,   181,    87,     0,     0,     0,    89,     0,     0,
      90,     5,     6,     7,     8,     9,    91,    92,    93,    94,
       0,    10,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,     0,     0,     0,   116,   117,
       0,   118,   119,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,    40,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   192,
       0,     0,    51,     0,     0,     0,     0,     0,     0,     0,
      58,    59,    60,   176,   177,   178,     0,     0,    65,    66,
       0,     0,     0,     0,     0,     0,     0,   179,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   180,    79,    80,    81,
      82,     0,    83,    84,     0,    85,   181,    87,     0,     0,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,    92,    93,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,   960,   961,
       0,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,   962,    10,   963,   964,
     965,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   978,   979,   980,   981,   982,   983,   217,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,   984,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,    40,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    51,     0,
       0,     0,     0,     0,     0,     0,    58,    59,    60,   176,
     177,   178,     0,     0,    65,    66,     0,     0,     0,     0,
       0,     0,     0,   179,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   180,    79,    80,    81,    82,     0,    83,    84,
       0,    85,   181,    87,     0,     0,     0,    89,     0,     0,
      90,     5,     6,     7,     8,     9,    91,    92,    93,    94,
       0,    10,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,     0,     0,     0,   116,   117,
       0,   118,   119,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,    40,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    51,     0,     0,     0,     0,     0,     0,     0,
      58,    59,    60,   176,   177,   178,     0,     0,    65,    66,
       0,     0,     0,     0,     0,     0,     0,   179,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   180,    79,    80,    81,
      82,     0,    83,    84,     0,    85,   181,    87,     0,     0,
       0,    89,     0,     0,    90,     5,     6,     7,     8,     9,
      91,    92,    93,    94,     0,    10,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,   246,
       0,     0,   116,   117,     0,   118,   119,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,    40,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    51,     0,     0,     0,
       0,     0,     0,     0,    58,    59,    60,   176,   177,   178,
       0,     0,    65,    66,     0,     0,     0,     0,     0,     0,
       0,   179,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     180,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     181,    87,     0,     0,     0,    89,     0,     0,    90,     5,
       6,     7,     8,     9,    91,    92,    93,    94,     0,    10,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   249,     0,     0,   116,   117,     0,   118,
     119,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,    40,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      51,     0,     0,     0,     0,     0,     0,     0,    58,    59,
      60,   176,   177,   178,     0,     0,    65,    66,     0,     0,
       0,     0,     0,     0,     0,   179,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   180,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   181,    87,     0,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,    94,     0,     0,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,   447,     0,     0,     0,
     116,   117,     0,   118,   119,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   606,   378,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   379,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,    40,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    51,     0,     0,     0,
       0,     0,     0,     0,    58,    59,    60,   176,   177,   178,
       0,     0,    65,    66,     0,     0,     0,     0,     0,     0,
       0,   179,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     180,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     181,    87,     0,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,    94,     0,     0,
       0,     0,    97,    98,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,   961,     0,     0,   116,   117,     0,   118,
     119,     5,     6,     7,     8,     9,     0,     0,     0,     0,
     962,    10,   963,   964,   965,   966,   967,   968,   969,   970,
     971,   972,   973,   974,   975,   976,   977,   978,   979,   980,
     981,   982,   983,   648,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,   984,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,    40,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    51,     0,     0,     0,     0,     0,     0,     0,
      58,    59,    60,   176,   177,   178,     0,     0,    65,    66,
       0,     0,     0,     0,     0,     0,     0,   179,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   180,    79,    80,    81,
      82,     0,    83,    84,     0,    85,   181,    87,     0,     0,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,    92,    93,    94,     0,     0,     0,     0,    97,    98,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,   113,     0,     0,
       0,     0,   116,   117,     0,   118,   119,     5,     6,     7,
       8,     9,     0,     0,     0,     0,   962,    10,   963,   964,
     965,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   978,   979,   980,   981,   982,   983,   684,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,   984,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,    40,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    51,     0,
       0,     0,     0,     0,     0,     0,    58,    59,    60,   176,
     177,   178,     0,     0,    65,    66,     0,     0,     0,     0,
       0,     0,     0,   179,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   180,    79,    80,    81,    82,     0,    83,    84,
       0,    85,   181,    87,     0,     0,     0,    89,     0,     0,
      90,     0,     0,     0,     0,     0,    91,    92,    93,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,     0,     0,     0,   116,   117,
       0,   118,   119,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   686,   378,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,   379,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,    40,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    51,     0,     0,     0,     0,     0,
       0,     0,    58,    59,    60,   176,   177,   178,     0,     0,
      65,    66,     0,     0,     0,     0,     0,     0,     0,   179,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   180,    79,
      80,    81,    82,     0,    83,    84,     0,    85,   181,    87,
       0,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,    94,     0,     0,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,     0,     0,     0,   116,   117,     0,   118,   119,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
     963,   964,   965,   966,   967,   968,   969,   970,   971,   972,
     973,   974,   975,   976,   977,   978,   979,   980,   981,   982,
     983,  1092,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,   984,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,    40,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      51,     0,     0,     0,     0,     0,     0,     0,    58,    59,
      60,   176,   177,   178,     0,     0,    65,    66,     0,     0,
       0,     0,     0,     0,     0,   179,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   180,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   181,    87,     0,     0,     0,    89,
       0,     0,    90,     5,     6,     7,     8,     9,    91,    92,
      93,    94,     0,    10,     0,     0,    97,    98,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,     0,   113,     0,     0,     0,     0,
     116,   117,     0,   118,   119,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,    40,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    51,     0,     0,     0,     0,     0,
       0,     0,    58,    59,    60,   176,   177,   178,     0,     0,
      65,    66,     0,     0,     0,     0,     0,     0,     0,   179,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   180,    79,
      80,    81,    82,     0,    83,    84,     0,    85,   181,    87,
       0,     0,     0,    89,     0,     0,    90,     5,     6,     7,
       8,     9,    91,    92,    93,    94,     0,    10,     0,     0,
      97,    98,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,     0,     0,   113,
       0,     0,     0,     0,   116,   117,     0,   118,   119,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,   537,    38,     0,     0,     0,     0,     0,    40,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    51,     0,
       0,     0,     0,     0,     0,     0,    58,    59,    60,   176,
     177,   178,     0,     0,    65,    66,     0,     0,     0,     0,
       0,     0,     0,   179,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   180,    79,    80,    81,    82,     0,    83,    84,
       0,    85,   181,    87,     0,     0,     0,    89,     0,     0,
      90,     0,     0,     0,     0,     0,    91,    92,    93,    94,
       0,     0,     0,     0,    97,    98,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,     0,     0,   113,     0,     0,     0,     0,   116,   117,
       0,   118,   119,  1461,  1462,  1463,  1464,  1465,     0,     0,
    1466,  1467,  1468,  1469,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1470,  1471,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,     0,
     378,     0,     0,  1472,     0,     0,     0,     0,     0,     0,
       0,     0,   379,     0,     0,     0,     0,  1473,  1474,  1475,
    1476,  1477,  1478,  1479,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,  1480,  1481,  1482,  1483,
    1484,  1485,  1486,  1487,  1488,  1489,  1490,  1491,  1492,  1493,
    1494,  1495,  1496,  1497,  1498,  1499,  1500,  1501,  1502,  1503,
    1504,  1505,  1506,  1507,  1508,  1509,  1510,  1511,  1512,  1513,
    1514,  1515,  1516,  1517,  1518,  1519,  1520,     0,     0,  1521,
    1522,     0,  1523,  1524,  1525,  1526,  1527,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1528,  1529,
    1530,     0,     0,     0,    83,    84,     0,    85,   181,    87,
    1531,     0,  1532,  1533,     0,  1534,     0,     0,     0,     0,
       0,     0,  1535,     0,     0,     0,  1536,     0,  1537,     0,
    1538,  1539,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   353,   354,   355,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      36,     0,     0,     0,     0,   356,     0,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,     0,   378,
     353,   354,   355,     0,     0,     0,     0,     0,     0,     0,
       0,   379,     0,     0,     0,     0,     0,     0,     0,   356,
       0,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   251,   378,   302,     0,     0,    83,    84,     0,
      85,   181,    87,     0,     0,   379,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   252,     0,
       0,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
      36,     0,   251,     0,     0,   303,   967,   968,   969,   970,
     971,   972,   973,   974,   975,   976,   977,   978,   979,   980,
     981,   982,   983,     0,     0,     0,     0,     0,   252,     0,
       0,     0,     0,     0,     0,     0,   984,     0,     0,     0,
       0,     0,     0,  1449,     0,     0,     0,     0,     0,     0,
      36,     0,     0,   253,   254,  -887,  -887,  -887,  -887,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   180,   378,     0,    81,   255,     0,    83,    84,     0,
      85,   181,    87,     0,   379,     0,  1294,     0,     0,     0,
       0,     0,     0,    36,   251,   256,     0,     0,     0,     0,
       0,     0,     0,   253,   254,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
     252,   180,   257,     0,    81,   255,  1558,    83,    84,     0,
      85,   181,    87,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,     0,   256,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,  -319,
      83,    84,   257,    85,   181,    87,  1625,    58,    59,    60,
     176,   177,   343,     0,     0,     0,   251,     0,     0,     0,
       0,     0,     0,     0,     0,   253,   254,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   252,   180,     0,   855,    81,   255,     0,    83,
      84,     0,    85,   181,    87,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    36,     0,   251,   256,     0,     0,
       0,     0,     0,     0,     0,     0,   344,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   470,   252,     0,   257,  -887,  -887,  -887,  -887,   972,
     973,   974,   975,   976,   977,   978,   979,   980,   981,   982,
     983,     0,     0,     0,    36,     0,     0,   253,   254,     0,
       0,     0,     0,     0,   984,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   180,   251,     0,    81,   255,
       0,    83,    84,     0,    85,   181,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   256,
       0,     0,   252,     0,     0,     0,     0,   253,   254,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,    36,   180,   257,     0,    81,   255,
       0,    83,    84,     0,    85,   181,    87,     0,  1270,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   256,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   873,   257,   253,   254,     0,
       0,     0,     0,     0,    36,     0,   209,     0,     0,     0,
       0,     0,     0,     0,     0,   180,     0,     0,    81,   255,
       0,    83,    84,     0,    85,   181,    87,    36,     0,     0,
       0,     0,     0,     0,     0,   716,     0,     0,     0,   256,
       0,     0,     0,     0,   210,     0,     0,  1149,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   723,   724,     0,   257,     0,     0,   725,
       0,   726,     0,     0,     0,   180,     0,     0,    81,    82,
       0,    83,    84,   727,    85,   181,    87,     0,     0,     0,
       0,    33,    34,    35,    36,     0,     0,     0,   180,     0,
       0,    81,   728,     0,    83,    84,     0,    85,   181,    87,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,     0,   211,     0,     0,     0,
       0,   116,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,   729,     0,    72,
      73,    74,    75,    76,    36,     0,   209,     0,     0,     0,
     730,     0,     0,     0,     0,   180,    79,    80,    81,   731,
       0,    83,    84,     0,    85,   181,    87,    36,     0,     0,
      89,     0,     0,     0,     0,     0,     0,     0,     0,   732,
     733,   734,   735,     0,   210,     0,     0,    97,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   723,   724,     0,   736,     0,     0,   725,
       0,   726,     0,     0,     0,   180,     0,     0,    81,    82,
       0,    83,    84,   727,    85,   181,    87,     0,     0,     0,
       0,    33,    34,    35,    36,     0,     0,     0,     0,     0,
       0,   501,   728,     0,    83,    84,     0,    85,   181,    87,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,     0,   211,     0,     0,     0,
       0,   116,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,   729,     0,    72,
      73,    74,    75,    76,     0,     0,     0,   884,   885,     0,
     730,     0,     0,     0,     0,   180,    79,    80,    81,   731,
       0,    83,    84,     0,    85,   181,    87,   886,     0,     0,
      89,     0,     0,     0,     0,   887,   888,   889,    36,   732,
     733,   734,   735,     0,     0,     0,   890,    97,     0,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,     0,   736,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,   209,     0,
       0,   891,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   892,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    83,    84,     0,    85,   181,
      87,     0,     0,     0,     0,     0,   210,     0,     0,     0,
       0,     0,     0,   893,     0,     0,     0,     0,   529,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   180,     0,     0,
      81,    82,     0,    83,    84,     0,    85,   181,    87,    36,
       0,   209,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,     0,   211,   210,
       0,     0,     0,   116,     0,     0,     0,     0,     0,     0,
       0,  1018,     0,    36,     0,   209,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     180,     0,     0,    81,    82,    36,    83,    84,     0,    85,
     181,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   222,     0,     0,     0,     0,     0,     0,
       0,     0,   272,   273,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
       0,   211,     0,    36,   180,     0,   116,    81,    82,     0,
      83,    84,     0,    85,   181,    87,    36,     0,   209,     0,
       0,     0,     0,     0,   551,     0,     0,     0,     0,   274,
       0,     0,    83,    84,     0,    85,   181,    87,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,   223,   210,     0,     0,     0,
     116,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   180,     0,     0,    81,    82,     0,
      83,    84,     0,    85,   181,    87,     0,   180,     0,     0,
      81,    82,     0,    83,    84,     0,    85,   181,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   353,   354,   355,     0,     0,
       0,     0,     0,   552,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   356,     0,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,     0,   378,   353,
     354,   355,     0,     0,     0,     0,     0,     0,     0,     0,
     379,     0,     0,     0,     0,     0,     0,     0,   356,     0,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,     0,   378,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   379,   353,   354,   355,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   424,   356,     0,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,     0,   378,   353,
     354,   355,     0,     0,     0,     0,     0,     0,     0,     0,
     379,     0,     0,     0,     0,     0,     0,   433,   356,     0,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,     0,   378,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   379,   353,   354,   355,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   822,   356,     0,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,     0,   378,   353,
     354,   355,     0,     0,     0,     0,     0,     0,     0,     0,
     379,     0,     0,     0,     0,     0,     0,   861,   356,     0,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,     0,   378,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   379,   353,   354,   355,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   902,   356,     0,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,     0,   378,   959,
     960,   961,     0,     0,     0,     0,     0,     0,     0,     0,
     379,     0,     0,     0,     0,     0,     0,  1206,   962,     0,
     963,   964,   965,   966,   967,   968,   969,   970,   971,   972,
     973,   974,   975,   976,   977,   978,   979,   980,   981,   982,
     983,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   984,   959,   960,   961,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1225,   962,     0,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,   982,   983,   959,   960,   961,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     984,     0,     0,     0,     0,     0,   962,  1124,   963,   964,
     965,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   978,   979,   980,   981,   982,   983,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   984,   959,   960,   961,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   962,  1283,   963,   964,   965,   966,   967,   968,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   980,   981,   982,   983,   959,   960,   961,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   984,     0,
       0,     0,     0,     0,   962,  1288,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,   982,   983,     0,     0,     0,
       0,     0,    36,     0,   796,   797,     0,     0,     0,     0,
     984,   959,   960,   961,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    36,     0,     0,     0,     0,     0,
     962,  1367,   963,   964,   965,   966,   967,   968,   969,   970,
     971,   972,   973,   974,   975,   976,   977,   978,   979,   980,
     981,   982,   983,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1385,   984,     0,    36,     0,
       0,     0,     0,  1447,     0,     0,     0,  1386,  1387,    83,
      84,    36,    85,   181,    87,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   180,     0,     0,    81,  1388,
       0,    83,    84,     0,    85,  1389,    87,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,     0,     0,     0,    36,     0,     0,  1448,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   340,     0,    83,    84,     0,    85,   181,
      87,     0,     0,     0,     0,   505,     0,     0,    83,    84,
       0,    85,   181,    87,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     274,     0,     0,    83,    84,     0,    85,   181,    87,     0,
      36,     0,     0,     0,   637,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,    83,    84,     0,    85,   181,
      87,     0,     0,     0,     0,     0,  1141,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,    83,    84,     0,
      85,   181,    87,     0,     0,     0,     0,     0,     0,     0,
      83,    84,     0,    85,   181,    87,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   353,   354,   355,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     707,   356,     0,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,     0,   378,   353,   354,   355,     0,
       0,     0,     0,     0,     0,     0,     0,   379,     0,     0,
       0,     0,     0,     0,     0,   356,   858,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   708,   378,
     353,   354,   355,     0,     0,     0,     0,     0,     0,     0,
       0,   379,     0,     0,     0,     0,     0,     0,     0,   356,
       0,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,     0,   378,   959,   960,   961,     0,     0,     0,
       0,     0,     0,     0,     0,   379,     0,     0,     0,     0,
       0,     0,     0,   962,  1293,   963,   964,   965,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     978,   979,   980,   981,   982,   983,   959,   960,   961,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   984,
       0,     0,     0,     0,     0,   962,     0,   963,   964,   965,
     966,   967,   968,   969,   970,   971,   972,   973,   974,   975,
     976,   977,   978,   979,   980,   981,   982,   983,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   984
};

static const yytype_int16 yycheck[] =
{
       4,   135,   172,   162,     4,    88,     4,   183,    52,    32,
       4,   310,    95,    96,    53,   596,     4,     4,   677,   833,
      43,   392,     4,   418,    47,   224,  1041,   570,   413,   569,
    1038,   815,    56,   165,   162,   706,   378,   544,   713,  1027,
    1025,   113,     9,     9,     9,    49,   230,    30,    52,     4,
     184,   134,   854,   113,    78,     4,   441,    81,    30,     9,
      30,     9,     9,    66,     9,    69,   230,     9,   870,     9,
       4,     9,     9,   251,   252,    66,     9,    26,    27,   257,
       9,    66,    86,     9,    88,     9,    66,   220,     9,     9,
       9,    95,    96,     9,     9,    79,    50,     9,     9,   231,
      33,    79,     9,    66,     9,     9,   916,    84,    79,    66,
      45,    45,   153,   127,   127,   108,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    35,
     134,    45,   127,    46,    47,    66,   147,   201,   202,   211,
     310,    84,   127,    66,    45,   449,  1572,   127,    66,    45,
      35,   223,    45,   135,     0,   333,   147,   168,   117,   118,
     119,   202,   162,   147,   113,    63,    64,   144,   952,   345,
     127,   105,   165,    79,    66,    66,   110,    66,   112,   113,
     114,   115,   116,   117,   118,    66,    66,   200,    66,   203,
      66,   202,   196,   497,    79,    66,   199,    66,   152,   202,
    1626,   144,   184,   199,    79,   200,    53,    71,    72,    79,
      50,   202,  1220,   167,     8,   385,   288,   202,    65,   169,
     154,   155,   202,   157,   169,   196,   204,    66,   288,   127,
     202,   201,   236,   200,  1222,   201,   240,   407,   203,   202,
     244,  1229,   236,  1231,   420,  1230,   240,   181,   272,   273,
     274,   926,   200,   928,   201,   202,   339,   391,   262,   201,
     200,   431,   211,   201,   201,   200,  1068,   200,   217,   203,
     440,   202,   201,   443,   223,   201,   199,   201,   302,   202,
     201,   201,   201,   197,   202,   201,   201,   236,   200,   200,
     468,   240,   336,   200,   427,   200,   200,  1107,   805,   200,
     199,   197,   200,  1087,   200,   388,   389,   200,   851,    79,
     202,   202,   152,   202,    84,    35,    35,    35,    35,   323,
      79,   202,   202,   323,   202,  1333,   202,  1335,   332,   204,
     279,   202,   336,   202,   204,   339,   199,   286,   287,   288,
      79,   323,    99,   100,   293,    84,    99,   100,   199,    79,
     299,   125,   126,   392,    84,  1343,   428,   199,   147,    79,
      79,    79,    79,   318,   112,   113,   114,   115,   116,   117,
     199,    96,   202,   168,   323,   145,   146,    71,    72,   168,
     123,   385,   386,   387,   388,   389,   145,   146,   566,   567,
      96,   575,    96,   199,   698,   378,   574,   792,   199,   703,
      96,    14,   385,   407,   199,   152,   145,   146,   390,    96,
     199,   575,   451,   202,   144,   145,   146,    30,  1426,   201,
     202,   554,    79,    96,   407,   584,   151,   431,   561,  1088,
      79,   564,  1103,   181,    47,    84,  1601,  1602,   208,   443,
     639,   161,   161,   161,   161,   151,   203,   151,   431,     4,
     203,   151,   456,   201,    79,   151,   584,   440,   123,    84,
     443,    79,   456,   104,   151,   199,    84,   666,   207,   418,
     646,   112,   113,   114,   115,   116,   117,   501,   151,   428,
      79,   505,   867,   199,   683,    84,   510,   144,   145,   146,
      45,   199,   877,  1277,   382,   144,   145,   146,   682,   153,
    1040,   201,   202,   636,   688,  1597,  1598,   456,   512,   721,
     722,  1054,   166,   199,  1057,   199,   815,   199,   682,   602,
     145,   146,   410,   199,   688,   199,   414,   145,   146,   533,
     534,   873,    29,   202,    74,    75,    76,    30,   737,   151,
     181,   199,   201,   721,   722,    85,   145,   146,   202,    46,
     105,    35,    49,   201,   201,   110,   688,   112,   113,   114,
     115,   116,   117,   118,   201,    29,   201,  1575,  1352,   112,
     113,   114,   115,   116,   117,    49,    50,    51,   201,  1238,
      66,  1240,    46,    66,   584,    49,   719,    98,    99,   100,
      66,    65,   132,   133,   134,   135,   136,   201,   602,   154,
     155,   296,   157,   143,   202,   300,   182,   183,   184,   149,
     150,   147,   199,   189,   190,   570,   199,   193,   194,   923,
       4,    66,   147,   163,   199,   151,   181,   201,   806,    44,
     325,    65,   327,   328,   329,   330,   168,   147,   181,   127,
     180,    98,    99,   100,   593,   815,   206,     9,   203,  1020,
     199,   829,   147,   952,   112,   113,   114,   115,   116,   117,
     147,    45,   127,     8,   842,   123,   124,   837,  1208,   168,
      49,    50,    51,   677,    53,   679,   199,   201,  1063,   702,
      14,  1224,   112,   113,   114,    14,    65,  1072,    79,  1704,
    1698,  1350,   696,   201,   201,    14,   696,   200,   168,   648,
     158,    14,   160,    96,  1719,  1713,   710,   711,   200,   200,
     199,  1106,   104,     9,   696,   205,    88,   711,   199,   199,
      14,   105,   200,   181,     9,   200,   110,   199,   112,   113,
     114,   115,   116,   117,   118,   684,     9,   686,   201,   185,
      79,    79,   766,    79,   188,   199,   770,   696,   201,     9,
     201,     9,   930,    79,   932,   125,   199,   761,   200,   708,
     200,   761,   711,   767,   897,   201,    66,   771,   772,   200,
     154,   155,   126,   157,    30,  1434,   128,   167,     9,   761,
     200,   147,   952,   200,     9,   789,   200,     9,  1087,   872,
     200,    14,   197,     9,  1189,    66,     9,   181,   169,   200,
     804,  1344,     9,    14,   804,   125,   804,   206,   941,   206,
     804,     9,   761,   199,   818,   948,   804,   804,   203,   203,
     200,   199,   804,   112,   113,   114,   115,   116,   117,   206,
     779,   206,  1217,   837,   123,   124,   199,    96,   128,   200,
     147,  1226,   201,   792,   793,   201,     9,  1046,   199,   804,
    1235,   200,   147,   199,   837,   804,   199,   199,   199,   147,
    1674,  1039,   202,  1041,   185,    14,     9,   185,   872,    79,
     202,   160,    14,   201,    14,   202,  1180,  1691,   833,    14,
     884,   885,   886,   206,   201,  1699,   909,   202,  1066,   197,
     873,    30,   181,   199,   199,   199,   851,    30,    14,     9,
      46,    47,    48,    49,    50,    51,   910,    53,   912,   200,
     914,   128,   912,   199,   914,   199,   910,  1087,  1577,    65,
      46,    47,    48,    49,    50,    51,   201,   201,    14,   933,
     912,   199,   914,     9,   200,    65,   206,     9,   144,    65,
      96,    79,     9,  1121,   199,  1330,  1250,   951,   128,   201,
    1083,    79,    14,   957,    79,   199,   202,   200,   199,   199,
     202,   910,  1266,   912,   200,   914,   200,   916,   917,   202,
     199,   128,     9,   144,   206,    30,    73,   201,  1277,   200,
     169,  1020,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    30,  1000,   201,  1131,  1003,
     200,   128,  1135,   200,   128,  1138,     9,  1185,   200,   200,
       9,     9,  1145,     4,   200,   200,   203,     9,   200,   203,
    1024,   202,   199,   202,  1024,    14,  1024,   200,   200,   199,
    1024,    63,    64,  1418,   200,   200,  1024,  1024,   200,  1620,
     128,   200,  1024,     9,    30,    96,   201,   200,   200,   156,
    1354,   201,   152,  1352,    45,   202,   201,    79,    14,  1363,
      79,   110,   199,   128,    26,    27,   200,   128,    30,  1024,
     202,   200,  1095,  1377,    14,  1024,    14,   200,   200,   200,
    1084,   200,   202,   201,  1088,    79,   199,   202,   200,   128,
      52,  1269,   201,  1271,  1098,   127,   201,    14,    14,  1054,
    1049,    14,  1057,   201,  1098,   200,  1110,  1277,  1689,     4,
    1110,   202,    55,  1246,   105,    79,   199,     9,    79,   110,
     201,   112,   113,   114,   115,   116,   117,   118,  1110,    79,
     108,    96,   147,  1311,   159,    33,  1440,    96,    14,   199,
      79,   200,   165,  1092,   201,   199,   162,   200,     9,  1098,
      45,    79,   202,   201,  1313,   200,   200,  1106,  1107,    79,
      14,  1110,    14,   154,   155,    79,   157,    79,   200,    14,
      79,    14,   766,   510,   770,   389,  1682,  1311,   386,   388,
    1184,   871,  1352,   868,  1695,   807,  1439,  1187,  1263,  1691,
     181,   515,  1429,  1459,  1104,  1543,  1196,  1306,  1723,  1711,
    1555,  1379,    41,  1425,   392,   995,   486,   992,   486,  1028,
     105,   758,   203,  1070,   949,   110,  1084,   112,   113,   114,
     115,   116,   117,   118,   885,   294,   313,  1403,   899,   955,
    1011,   287,  1187,   934,  1238,    -1,  1240,    -1,    -1,   721,
    1189,  1196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   211,
      -1,    -1,    -1,    -1,    -1,   217,    -1,  1301,    -1,   154,
     155,   223,   157,  1567,    -1,  1569,    -1,    -1,    -1,  1224,
      -1,    -1,    -1,    -1,  1578,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,   251,
     252,    -1,  1296,    -1,    -1,   257,  1296,  1301,    -1,    -1,
      -1,    -1,  1306,    -1,    -1,    -1,    -1,    -1,   203,    -1,
      -1,    -1,    -1,  1313,  1296,    -1,    -1,   279,    -1,  1623,
      -1,    -1,    -1,    -1,   286,   287,    -1,    -1,  1310,    -1,
      -1,   293,    -1,    -1,  1334,    -1,    -1,   299,    -1,    -1,
    1340,    -1,  1342,    -1,    -1,    -1,  1350,  1296,   310,    -1,
      -1,  1355,    -1,    -1,    -1,  1355,  1360,    -1,    -1,    -1,
    1364,  1405,    -1,    -1,  1364,    -1,  1360,    -1,    -1,     4,
      -1,   333,    -1,  1355,   336,    -1,    -1,    -1,    -1,  1334,
      -1,  1557,  1364,    -1,    -1,  1340,    -1,  1342,  1392,  1344,
      -1,    -1,    -1,    -1,  1398,    -1,    -1,    -1,    -1,    -1,
      -1,  1405,    -1,    -1,    -1,  1409,  1355,    -1,    -1,    -1,
      45,  1360,    -1,  1717,    -1,  1364,   378,    -1,    -1,    -1,
    1724,    -1,    -1,    -1,    -1,  1407,    -1,    -1,    -1,    -1,
    1434,  1431,    -1,  1437,  1438,  1439,    -1,  1437,    -1,  1443,
      -1,    -1,    -1,  1443,  1438,  1439,  1450,    -1,    -1,    -1,
    1450,    -1,    -1,    -1,    -1,  1437,   418,    -1,    -1,    -1,
      -1,  1443,    -1,    -1,    -1,    -1,   428,    -1,  1450,    -1,
     105,  1554,    -1,    -1,    -1,   110,  1431,   112,   113,   114,
     115,   116,   117,   118,    -1,    -1,    -1,    -1,  1437,  1438,
    1439,    -1,    -1,    -1,  1443,    -1,    -1,    -1,    -1,    -1,
      -1,  1450,    -1,    -1,    -1,    -1,   468,   469,    -1,    -1,
     472,  1670,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,
     155,    -1,   157,    -1,    -1,    -1,  1704,    -1,    -1,    -1,
    1613,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1719,    -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,
    1554,    -1,  1686,    -1,    -1,   517,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,   203,  1573,
      -1,    -1,    -1,  1577,    -1,    -1,    -1,    -1,  1582,    -1,
      -1,    65,  1582,    -1,    -1,    -1,    77,    -1,  1592,    -1,
      -1,    -1,    -1,  1597,  1598,    -1,    -1,  1601,  1602,    -1,
    1582,    -1,    -1,    -1,   566,   567,    -1,    -1,    -1,  1613,
      -1,    -1,   574,    -1,    -1,    -1,    -1,  1621,  1622,    -1,
      -1,  1621,  1622,    -1,  1628,    -1,    -1,    -1,  1628,    -1,
      -1,   593,    -1,  1582,    -1,    -1,    -1,    -1,    -1,  1621,
    1622,    -1,    -1,    -1,    -1,    -1,  1628,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   148,  1662,    -1,
     151,    -1,  1662,   154,   155,  1669,   157,   158,   159,    -1,
    1670,    -1,  1621,  1622,    -1,    -1,    -1,    -1,    -1,  1628,
    1662,  1685,    -1,    -1,    -1,    -1,   648,    -1,    -1,    -1,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   203,  1662,    -1,    -1,    -1,    -1,    -1,  1674,
      -1,  1725,   684,    -1,   686,  1725,    -1,  1731,    -1,    -1,
      -1,  1731,    -1,    -1,    -1,    -1,  1691,    -1,    -1,    -1,
      -1,    -1,    -1,  1725,  1699,    -1,   708,   709,    -1,  1731,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   721,
     722,   723,   724,   725,   726,   727,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   736,    -1,  1725,    -1,    -1,    -1,
      -1,    -1,  1731,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,    -1,    -1,    -1,
     762,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,   779,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,   790,    -1,
     792,   793,    63,    64,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   806,   807,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   815,    26,    27,    -1,    -1,    30,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   829,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     842,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   850,    -1,
      -1,   853,    -1,    10,    11,    12,   127,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,    27,    -1,    -1,    30,
      -1,   873,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,   916,   917,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   930,    -1,
     932,    -1,   934,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   949,    -1,    -1,
     952,   953,   954,   955,    -1,    -1,   958,   959,   960,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,   980,   981,
     982,   983,   984,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,   211,
      -1,    -1,    -1,    -1,    -1,   217,  1008,    -1,    -1,    -1,
      -1,   223,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,  1039,    -1,  1041,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1049,    65,   206,
      -1,    -1,    -1,    -1,    -1,    -1,   217,    -1,    -1,    -1,
      -1,    -1,    -1,    77,  1066,    79,    -1,   279,    -1,    -1,
      -1,    -1,    -1,    -1,   286,   287,    -1,    -1,    -1,    -1,
      -1,   293,    -1,    -1,    -1,  1087,    -1,   299,    -1,    -1,
    1092,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   310,    -1,
      -1,    -1,    -1,    -1,  1106,  1107,    -1,  1109,    -1,   123,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   279,  1121,
      -1,    -1,  1124,    -1,  1126,   286,   287,    -1,    -1,    -1,
      -1,    -1,   293,    -1,    -1,    -1,    -1,    -1,   299,    -1,
     154,   155,    -1,   157,   158,   159,    -1,  1149,    -1,   310,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,   378,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,  1185,  1186,    -1,   203,  1189,   202,    -1,
     204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    63,    64,    -1,    -1,   418,    -1,    -1,    -1,
      -1,  1213,    -1,    -1,    -1,    -1,   428,   378,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,   418,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   127,  1269,    -1,  1271,
      -1,    -1,    -1,    -1,  1276,  1277,    -1,    -1,    -1,  1281,
      -1,  1283,    -1,  1285,    -1,    -1,  1288,    -1,  1290,    -1,
      -1,  1293,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1301,
    1302,    -1,  1304,    -1,    -1,    10,    11,    12,    -1,  1311,
      -1,   472,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,  1327,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
    1352,    -1,    -1,    77,    -1,    79,   517,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,  1367,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1379,  1380,  1381,
      -1,   593,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   203,   117,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1405,    -1,    -1,    -1,  1409,    -1,    -1,
      -1,    -1,    -1,  1415,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   148,    52,    -1,   151,   152,    -1,
     154,   155,   593,   157,   158,   159,   648,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1446,  1447,  1448,    -1,    -1,    -1,
      -1,    -1,  1454,  1455,    -1,    -1,    -1,  1459,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   684,    -1,   686,   199,    -1,    -1,   202,    -1,
     204,    10,    11,    12,    -1,    -1,    -1,   648,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   708,    -1,   203,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,   684,    53,   686,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   708,   709,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1562,    -1,   723,   724,   725,   726,   727,   779,    -1,    -1,
      -1,  1573,    -1,    -1,    -1,   736,    -1,    -1,    -1,    -1,
     792,   793,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1592,    -1,    -1,    -1,  1596,    -1,    -1,    -1,    -1,    -1,
      -1,   762,    -1,   815,    -1,  1607,    -1,    -1,    -1,    -1,
    1612,    -1,    -1,  1615,    -1,    -1,    -1,    -1,   779,    -1,
      -1,    -1,    -1,    -1,   251,   252,    35,    -1,    -1,   790,
     257,   792,   793,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   807,    -1,    -1,    -1,
      -1,    77,    -1,    -1,   815,    -1,    -1,    -1,    -1,    -1,
      -1,   873,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,
      79,  1673,    -1,    -1,   203,    -1,    -1,    -1,    -1,  1681,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   850,
      -1,    -1,   853,  1695,    -1,   121,    -1,    -1,    -1,    -1,
      -1,    -1,  1704,    -1,   916,   917,   333,    -1,   117,   336,
      -1,    -1,   873,    -1,    -1,    -1,    -1,  1719,    -1,    -1,
     129,    -1,    -1,    -1,    -1,    -1,  1728,    -1,   154,   155,
      -1,   157,   158,   159,  1736,    -1,    -1,    -1,  1740,   148,
     952,  1743,   151,   152,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    -1,    -1,   916,   917,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      -1,    -1,    -1,   199,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,   949,    -1,
     199,   952,   953,   954,   955,   204,    -1,   958,   959,   960,
     961,   962,   963,   964,   965,   966,   967,   968,   969,   970,
     971,   972,   973,   974,   975,   976,   977,   978,   979,   980,
     981,   982,   983,   984,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1049,    -1,    -1,
      -1,   468,   469,    -1,    -1,   472,    29,  1008,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,  1087,    -1,    10,    11,    12,
    1092,    -1,    65,    -1,    -1,    -1,    -1,    -1,  1049,    -1,
     517,    -1,    -1,    -1,  1106,  1107,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,  1087,    -1,    -1,    -1,
      -1,  1092,    65,    -1,    -1,    -1,    -1,    -1,    -1,   566,
     567,    -1,    -1,    -1,    -1,  1106,  1107,   574,  1109,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1124,    -1,  1126,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,  1189,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,  1149,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      12,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     203,    -1,    -1,    65,    -1,  1186,    -1,    29,  1189,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,  1213,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,  1277,    -1,    -1,    -1,    -1,
     203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   709,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   721,   722,   723,   724,   725,   726,
     727,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   736,
      -1,    -1,    -1,    -1,    -1,  1276,  1277,    -1,    -1,    -1,
    1281,    -1,  1283,    -1,  1285,    -1,    -1,  1288,    -1,  1290,
      -1,    -1,  1293,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1352,  1302,    -1,  1304,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,   203,    53,    -1,    -1,    -1,  1327,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,   806,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1352,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   829,    10,    11,    12,  1367,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   842,    -1,    -1,    -1,  1380,
    1381,    -1,    29,   850,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1415,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    29,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,  1446,  1447,  1448,    -1,    55,
      -1,    -1,    -1,  1454,  1455,    -1,    -1,    65,  1459,    -1,
      -1,    -1,    -1,   930,    -1,   932,    -1,   934,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   949,    -1,    -1,    -1,   953,   954,   955,    -1,
      -1,   958,   959,   960,   961,   962,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,   982,   983,   984,    -1,    -1,
      -1,    -1,    -1,    -1,   130,   131,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1008,   148,    -1,    -1,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,    -1,   161,   203,    -1,    -1,    -1,
      -1,  1562,    -1,    -1,    -1,    -1,   172,    -1,    -1,    -1,
      -1,    -1,  1039,    -1,  1041,    -1,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      -1,    -1,    -1,   199,    -1,  1596,    10,    11,    12,  1066,
      -1,    -1,    -1,    -1,    -1,    -1,  1607,    -1,    -1,    -1,
      -1,  1612,    -1,    -1,  1615,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,  1109,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,  1121,    -1,    -1,  1124,    -1,  1126,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1673,    -1,    10,    11,    12,    -1,    -1,    -1,
    1681,    -1,  1149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,  1695,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,  1185,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1728,    -1,    65,
      10,    11,    12,    -1,    -1,  1736,    -1,    -1,    -1,  1740,
      -1,    -1,  1743,    -1,    -1,    -1,  1213,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,   203,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1269,    -1,  1271,    -1,    -1,    -1,    -1,  1276,
      -1,    -1,    -1,    -1,  1281,    -1,  1283,    -1,  1285,    -1,
      -1,  1288,    -1,  1290,    -1,    -1,  1293,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1301,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1311,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1327,    -1,    -1,    -1,    -1,    -1,    -1,   203,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1367,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1379,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1405,    -1,
       5,     6,  1409,     8,     9,    10,    11,    12,  1415,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    -1,    -1,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,  1446,
    1447,  1448,    -1,    48,    -1,    50,    -1,  1454,    53,    -1,
      55,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   472,    77,    -1,    79,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    10,    11,    12,    -1,    -1,
     201,    -1,    -1,    -1,    -1,    -1,    65,   517,   113,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
     154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,  1562,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1573,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,   188,    -1,  1592,    -1,    -1,   202,  1596,
     204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1607,    -1,    -1,    -1,    -1,  1612,    -1,    -1,  1615,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,   232,    -1,    -1,
     235,    -1,    -1,    -1,    -1,    -1,    -1,   242,   243,    -1,
      -1,    29,   201,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,  1673,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,   288,    -1,    -1,   201,    -1,    -1,   294,
      -1,    -1,    -1,   298,    -1,    -1,    -1,  1704,    -1,   709,
      -1,    -1,    -1,    -1,    -1,    -1,   311,   312,   313,    -1,
      -1,    -1,  1719,   723,   724,   725,   726,   727,    -1,   324,
      -1,  1728,    -1,    -1,    -1,    -1,   736,    -1,    -1,  1736,
     335,    -1,    -1,  1740,    -1,    -1,  1743,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,    -1,   379,    -1,   381,   382,    -1,   384,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
      -1,    -1,    -1,    -1,    -1,   410,   411,    -1,   413,   414,
     415,    -1,    -1,   201,    -1,    -1,   421,    -1,    -1,   424,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   433,    -1,
     435,    -1,    10,    11,    12,    -1,   441,    -1,    -1,    -1,
     850,    -1,    -1,   472,    -1,    -1,   451,    -1,   453,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,   479,    53,    -1,   482,   483,   484,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,   517,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   513,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,   949,
      -1,    -1,    -1,   953,   954,   955,    65,    -1,   958,   959,
     960,   961,   962,   963,   964,   965,   966,   967,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   977,   978,   979,
     980,   981,   982,   983,   984,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,   594,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1008,    -1,
      29,   606,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,   201,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   638,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,   647,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     665,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
     709,    53,   201,    -1,    -1,    -1,    -1,   692,    -1,    -1,
      -1,    -1,    -1,    65,   723,   724,   725,   726,   727,  1109,
      -1,   706,    -1,    -1,    -1,    -1,    -1,   736,    -1,    -1,
      -1,    -1,    -1,    -1,  1124,    -1,  1126,    77,    -1,    79,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1149,
      -1,    29,   747,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,   200,    -1,    -1,    -1,    -1,    -1,    65,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,   154,   155,   801,   157,   158,   159,
      -1,    -1,    -1,  1213,    -1,    -1,    -1,    65,    -1,   814,
      -1,    -1,    -1,    -1,    -1,   820,    -1,   822,    -1,   824,
      -1,   850,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,    -1,    -1,
      -1,    -1,   202,   848,   204,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   858,    -1,    -1,   861,    -1,   863,    -1,
      -1,    -1,   867,   472,    -1,    -1,  1276,    -1,    -1,    -1,
      -1,  1281,   877,  1283,    -1,  1285,    -1,    -1,  1288,    -1,
    1290,    -1,    -1,  1293,    77,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   902,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   517,    -1,
      -1,    -1,   200,    -1,    -1,    -1,    -1,  1327,    -1,    -1,
     949,    -1,    -1,    -1,   953,   954,   955,    -1,    -1,   958,
     959,   960,   961,   962,   963,   964,   965,   966,   967,   968,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   980,   981,   982,   983,   984,    -1,  1367,    -1,    -1,
      -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    -1,    -1,    -1,  1008,
      -1,   986,   987,   988,    -1,    -1,    -1,   992,   993,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,  1415,    -1,    -1,    -1,   202,
      -1,   204,    -1,    -1,    -1,  1020,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,  1446,  1447,  1448,    -1,
    1045,    -1,    -1,    -1,  1454,  1050,    -1,    -1,  1458,    65,
      -1,    -1,   154,   155,    -1,   157,   158,   159,  1063,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1072,    -1,  1074,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1109,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,  1124,    -1,  1126,  1103,    -1,
     709,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1113,    10,
      11,    12,    -1,    -1,   723,   724,   725,   726,    -1,    -1,
    1149,    -1,    -1,    -1,    -1,    -1,    -1,   736,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,  1562,    -1,    -1,    -1,    -1,    -1,    67,    68,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    77,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1213,    -1,  1596,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1199,    -1,    -1,  1607,  1203,    -1,
    1205,  1206,  1612,    -1,    -1,  1615,    -1,    -1,   117,    -1,
      -1,    -1,  1217,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1225,  1226,    -1,    -1,    -1,    -1,    -1,    -1,  1638,    -1,
    1235,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   148,
      -1,   850,   151,   152,    -1,   154,   155,  1276,   157,   158,
     159,    -1,  1281,    -1,  1283,    -1,  1285,   166,    -1,  1288,
      -1,  1290,    -1,  1673,  1293,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,  1294,
     199,    -1,    -1,    -1,    -1,   204,    -1,    -1,  1327,   200,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,  1728,    53,
      -1,    -1,    -1,    -1,    77,  1330,  1736,    -1,    -1,    -1,
    1740,    65,    -1,  1743,    -1,    -1,    -1,    -1,  1367,    -1,
      -1,    -1,    -1,    -1,   953,   954,   955,    -1,    -1,   958,
     959,   960,   961,   962,   963,   964,   965,   966,   967,   968,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   980,   981,   982,   983,   984,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1415,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,  1008,
      -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,
      -1,    -1,    -1,  1418,    -1,    -1,    -1,  1446,  1447,  1448,
      -1,    -1,    -1,    -1,    -1,  1454,    10,    11,    12,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,  1449,    29,   199,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,   152,    -1,   154,   155,   156,   157,   158,   159,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1109,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,  1124,    -1,  1126,   199,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1562,    -1,    -1,    -1,    -1,    -1,    29,
    1149,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,  1596,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,  1607,    77,
      -1,    79,    -1,  1612,    -1,    -1,  1615,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1213,    -1,    -1,    -1,    -1,    -1,
      29,   195,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1673,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    -1,    -1,    -1,  1276,    -1,    -1,
      -1,    -1,  1281,    -1,  1283,    -1,  1285,    -1,    -1,  1288,
      -1,  1290,    -1,    -1,  1293,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,  1728,
      -1,   191,   192,    -1,   202,    -1,   204,  1736,    -1,    -1,
      -1,  1740,    -1,    -1,  1743,    -1,    -1,    -1,  1327,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1367,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,   188,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,  1415,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1446,  1447,  1448,
     104,    -1,    -1,    -1,    -1,  1454,    -1,    -1,   112,   113,
     114,   115,   116,   117,    -1,    -1,   120,   121,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,
     134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,   163,
      -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,
     174,   175,    -1,    -1,    -1,    -1,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,    -1,    -1,    -1,
     204,   205,    -1,   207,   208,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1562,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1596,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,  1607,    -1,
      -1,    -1,    -1,  1612,    -1,    -1,  1615,    -1,    -1,    -1,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    -1,    -1,
      -1,    85,    86,    87,    88,    -1,    90,    -1,    92,    -1,
      94,    -1,    -1,    97,  1673,    -1,    -1,   101,   102,   103,
     104,   105,   106,   107,    -1,   109,   110,   111,   112,   113,
     114,   115,   116,   117,    -1,   119,   120,   121,   122,   123,
     124,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,
     134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,  1728,
     154,   155,    -1,   157,   158,   159,   160,  1736,    -1,   163,
      -1,  1740,   166,    -1,  1743,    -1,    -1,    -1,   172,   173,
     174,   175,   176,    -1,   178,    -1,   180,   181,    -1,   183,
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
      78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,   105,   106,   107,
      -1,   109,   110,   111,   112,   113,   114,   115,   116,   117,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,   160,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,   175,   176,    -1,
     178,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
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
      -1,    -1,    -1,    85,    86,    87,    88,    -1,    90,    -1,
      92,    -1,    94,    -1,    -1,    97,    -1,    -1,    -1,   101,
     102,   103,   104,   105,   106,   107,    -1,   109,   110,   111,
     112,   113,   114,   115,   116,   117,    -1,   119,   120,   121,
     122,   123,   124,    -1,    -1,    -1,    -1,   129,   130,    -1,
     132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,
      -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,   160,    -1,
      -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,
     172,   173,   174,   175,   176,    -1,   178,    -1,   180,   181,
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
      76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,    85,
      86,    87,    88,    -1,    90,    -1,    92,    -1,    94,    -1,
      -1,    97,    -1,    -1,    -1,   101,   102,   103,   104,    -1,
     106,   107,    -1,   109,    -1,   111,   112,   113,   114,   115,
     116,   117,    -1,   119,   120,   121,    -1,   123,   124,    -1,
      -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,   135,
     136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,   160,    -1,    -1,   163,    -1,    -1,
     166,    -1,    -1,    -1,    -1,    -1,   172,   173,   174,   175,
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
      -1,    81,    -1,    -1,    -1,    85,    86,    87,    88,    -1,
      90,    -1,    92,    -1,    94,    -1,    -1,    97,    -1,    -1,
      -1,   101,   102,   103,   104,    -1,   106,   107,    -1,   109,
      -1,   111,   112,   113,   114,   115,   116,   117,    -1,   119,
     120,   121,    -1,   123,   124,    -1,    -1,    -1,    -1,   129,
     130,    -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,
      -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,   152,    -1,   154,   155,    -1,   157,   158,   159,
     160,    -1,    -1,   163,    -1,    -1,   166,    -1,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,    -1,    -1,    -1,
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
      74,    75,    76,    77,    78,    79,    -1,    81,    -1,    -1,
      -1,    85,    86,    87,    88,    -1,    90,    -1,    92,    -1,
      94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,   103,
     104,    -1,   106,   107,    -1,   109,    -1,   111,   112,   113,
     114,   115,   116,   117,    -1,   119,   120,   121,    -1,   123,
     124,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,
     134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,   160,    -1,    -1,   163,
      -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,
     174,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
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
      78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,
      88,    89,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,   107,
      -1,   109,    -1,   111,   112,   113,   114,   115,   116,   117,
      -1,   119,   120,   121,    -1,   123,   124,    -1,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,   160,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,   201,   202,    -1,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      -1,    -1,    -1,    85,    86,    87,    88,    -1,    90,    -1,
      92,    -1,    94,    95,    -1,    97,    -1,    -1,    -1,   101,
     102,   103,   104,    -1,   106,   107,    -1,   109,    -1,   111,
     112,   113,   114,   115,   116,   117,    -1,   119,   120,   121,
      -1,   123,   124,    -1,    -1,    -1,    -1,   129,   130,    -1,
     132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,
      -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,   160,    -1,
      -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,
     172,   173,   174,   175,    -1,    -1,    -1,    -1,   180,   181,
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
      76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,    85,
      86,    87,    88,    -1,    90,    -1,    92,    -1,    94,    -1,
      -1,    97,    -1,    -1,    -1,   101,   102,   103,   104,    -1,
     106,   107,    -1,   109,    -1,   111,   112,   113,   114,   115,
     116,   117,    -1,   119,   120,   121,    -1,   123,   124,    -1,
      -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,   135,
     136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,   160,    -1,    -1,   163,    -1,    -1,
     166,    -1,    -1,    -1,    -1,    -1,   172,   173,   174,   175,
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
      -1,    81,    -1,    -1,    -1,    85,    86,    87,    88,    -1,
      90,    -1,    92,    -1,    94,    -1,    -1,    97,    -1,    -1,
      -1,   101,   102,   103,   104,    -1,   106,   107,    -1,   109,
      -1,   111,   112,   113,   114,   115,   116,   117,    -1,   119,
     120,   121,    -1,   123,   124,    -1,    -1,    -1,    -1,   129,
     130,    -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,
      -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,   152,    -1,   154,   155,    -1,   157,   158,   159,
     160,    -1,    -1,   163,    -1,    -1,   166,    -1,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,    -1,    -1,    -1,
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
      74,    75,    76,    77,    78,    79,    -1,    81,    -1,    -1,
      -1,    85,    86,    87,    88,    -1,    90,    -1,    92,    93,
      94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,   103,
     104,    -1,   106,   107,    -1,   109,    -1,   111,   112,   113,
     114,   115,   116,   117,    -1,   119,   120,   121,    -1,   123,
     124,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,
     134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,   160,    -1,    -1,   163,
      -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,
     174,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
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
      78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,   107,
      -1,   109,    -1,   111,   112,   113,   114,   115,   116,   117,
      -1,   119,   120,   121,    -1,   123,   124,    -1,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,   160,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,   175,    -1,    -1,
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
      -1,    -1,    -1,    85,    86,    87,    88,    -1,    90,    -1,
      92,    -1,    94,    -1,    -1,    97,    -1,    -1,    -1,   101,
     102,   103,   104,    -1,   106,   107,    -1,   109,    -1,   111,
     112,   113,   114,   115,   116,   117,    -1,   119,   120,   121,
      -1,   123,   124,    -1,    -1,    -1,    -1,   129,   130,    -1,
     132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,
      -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,   160,    -1,
      -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,
     172,   173,   174,   175,    -1,    -1,    -1,    -1,   180,   181,
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
      76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,    85,
      86,    87,    88,    -1,    90,    91,    92,    -1,    94,    -1,
      -1,    97,    -1,    -1,    -1,   101,   102,   103,   104,    -1,
     106,   107,    -1,   109,    -1,   111,   112,   113,   114,   115,
     116,   117,    -1,   119,   120,   121,    -1,   123,   124,    -1,
      -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,   135,
     136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,   160,    -1,    -1,   163,    -1,    -1,
     166,    -1,    -1,    -1,    -1,    -1,   172,   173,   174,   175,
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
      -1,    81,    -1,    -1,    -1,    85,    86,    87,    88,    -1,
      90,    -1,    92,    -1,    94,    -1,    -1,    97,    -1,    -1,
      -1,   101,   102,   103,   104,    -1,   106,   107,    -1,   109,
      -1,   111,   112,   113,   114,   115,   116,   117,    -1,   119,
     120,   121,    -1,   123,   124,    -1,    -1,    -1,    -1,   129,
     130,    -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,
      -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,   152,    -1,   154,   155,    -1,   157,   158,   159,
     160,    -1,    -1,   163,    -1,    -1,   166,    -1,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,    -1,    -1,    -1,
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
      74,    75,    76,    77,    78,    79,    -1,    81,    -1,    -1,
      -1,    85,    86,    87,    88,    -1,    90,    -1,    92,    -1,
      94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,   103,
     104,    -1,   106,   107,    -1,   109,    -1,   111,   112,   113,
     114,   115,   116,   117,    -1,   119,   120,   121,    -1,   123,
     124,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,
     134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,   160,    -1,    -1,   163,
      -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,
     174,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
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
      78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,   107,
      -1,   109,    -1,   111,   112,   113,   114,   115,   116,   117,
      -1,   119,   120,   121,    -1,   123,   124,    -1,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,   160,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,   175,    -1,    -1,
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
      -1,    -1,    -1,    85,    86,    87,    88,    -1,    90,    -1,
      92,    -1,    94,    -1,    -1,    97,    -1,    -1,    -1,   101,
     102,   103,   104,    -1,   106,   107,    -1,   109,    -1,   111,
     112,   113,   114,   115,   116,   117,    -1,   119,   120,   121,
      -1,   123,   124,    -1,    -1,    -1,    -1,   129,   130,    -1,
     132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,
      -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,   160,    -1,
      -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,
     172,   173,   174,   175,    -1,    -1,    -1,    -1,   180,   181,
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
      76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,    85,
      86,    87,    88,    -1,    90,    -1,    92,    -1,    94,    -1,
      -1,    97,    -1,    -1,    -1,   101,   102,   103,   104,    -1,
     106,   107,    -1,   109,    -1,   111,   112,   113,   114,   115,
     116,   117,    -1,   119,   120,   121,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,   135,
     136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,    -1,    -1,    -1,   163,    -1,    -1,
     166,    -1,    -1,    -1,    -1,    -1,   172,   173,   174,   175,
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
      -1,    81,    -1,    -1,    -1,    85,    86,    87,    88,    -1,
      90,    -1,    92,    -1,    94,    -1,    -1,    97,    -1,    -1,
      -1,   101,   102,   103,   104,    -1,   106,   107,    -1,   109,
      -1,   111,   112,   113,   114,   115,   116,   117,    -1,   119,
     120,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,
     130,    -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,
      -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,   152,    -1,   154,   155,    -1,   157,   158,   159,
      -1,    -1,    -1,   163,    -1,    -1,   166,    -1,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,    -1,    -1,    -1,
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
      74,    75,    76,    77,    78,    79,    -1,    81,    -1,    -1,
      -1,    85,    86,    87,    88,    -1,    90,    -1,    92,    -1,
      94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,   103,
     104,    -1,   106,   107,    -1,   109,    -1,   111,   112,   113,
     114,   115,   116,   117,    -1,   119,   120,   121,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,
     134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,   163,
      -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,
     174,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
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
      78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,   107,
      -1,   109,    -1,   111,   112,   113,   114,   115,   116,   117,
      -1,   119,   120,   121,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,   175,    -1,    -1,
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
      -1,    -1,    -1,    85,    86,    87,    88,    -1,    90,    -1,
      92,    -1,    94,    -1,    -1,    97,    -1,    -1,    -1,   101,
     102,   103,   104,    -1,   106,   107,    -1,   109,    -1,   111,
     112,   113,   114,   115,   116,   117,    -1,   119,   120,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,    -1,
     132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,
      -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,
     172,   173,   174,   175,    -1,    -1,    -1,    -1,   180,   181,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    -1,   201,
     202,    -1,   204,   205,    -1,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,    85,
      86,    87,    88,    -1,    90,    -1,    92,    -1,    94,    -1,
      -1,    97,    -1,    -1,    -1,   101,   102,   103,   104,    -1,
     106,   107,    -1,   109,    -1,   111,   112,   113,   114,   115,
     116,   117,    -1,   119,   120,   121,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,   135,
     136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,    -1,    -1,    -1,   163,    -1,    -1,
     166,    -1,    -1,    -1,    -1,    -1,   172,   173,   174,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,   201,   202,    -1,   204,   205,
      -1,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   112,   113,   114,   115,   116,   117,    -1,    -1,
     120,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,
     130,    -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,
      -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,   152,    -1,   154,   155,    -1,   157,   158,   159,
      -1,    -1,    -1,   163,    -1,    -1,   166,    -1,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,    -1,    -1,    -1,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   199,
      -1,   201,    -1,    -1,   204,   205,    -1,   207,   208,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    29,    13,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    35,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    65,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,   113,
     114,   115,   116,   117,    -1,    -1,   120,   121,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,
     134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,    -1,   161,    -1,   163,
      -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,
     174,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,    -1,    -1,    -1,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   112,   113,   114,   115,   116,   117,
      -1,    -1,   120,   121,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    11,    12,   202,    -1,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      29,    13,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    35,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    65,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     112,   113,   114,   115,   116,   117,    -1,    -1,   120,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,    -1,
     132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,
      -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,    -1,   161,
      -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,
     172,   173,   174,   175,    -1,    -1,    -1,    -1,   180,   181,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    -1,    -1,
      -1,    -1,   204,   205,    -1,   207,   208,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,   113,   114,   115,
     116,   117,    -1,    -1,   120,   121,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,   135,
     136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,    -1,    -1,    -1,   163,    -1,    -1,
     166,     3,     4,     5,     6,     7,   172,   173,   174,   175,
      -1,    13,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,    -1,    -1,    -1,   204,   205,
      -1,   207,   208,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     112,   113,   114,   115,   116,   117,    -1,    -1,   120,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,    -1,
     132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,
      -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,
     172,   173,   174,   175,    -1,    -1,    -1,    -1,   180,   181,
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
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,   113,   114,   115,
     116,   117,    -1,    -1,   120,   121,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,   135,
     136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,    -1,    -1,    -1,   163,    -1,    -1,
     166,     3,     4,     5,     6,     7,   172,   173,   174,   175,
      -1,    13,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,    -1,    -1,    -1,   204,   205,
      -1,   207,   208,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     112,   113,   114,   115,   116,   117,    -1,    -1,   120,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,    -1,
     132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,
      -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,   163,    -1,    -1,   166,     3,     4,     5,     6,     7,
     172,   173,   174,   175,    -1,    13,    -1,    -1,   180,   181,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,    -1,    -1,   199,    -1,   201,
      -1,    -1,   204,   205,    -1,   207,   208,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   112,   113,   114,   115,   116,   117,
      -1,    -1,   120,   121,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    -1,   163,    -1,    -1,   166,     3,
       4,     5,     6,     7,   172,   173,   174,   175,    -1,    13,
      -1,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,   201,    -1,    -1,   204,   205,    -1,   207,
     208,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,
      -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,   113,
     114,   115,   116,   117,    -1,    -1,   120,   121,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,
     134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,   163,
      -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,
     174,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,   200,    -1,    -1,    -1,
     204,   205,    -1,   207,   208,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    30,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   112,   113,   114,   115,   116,   117,
      -1,    -1,   120,   121,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,    12,    -1,    -1,   204,   205,    -1,   207,
     208,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      29,    13,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    65,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    -1,
      -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     112,   113,   114,   115,   116,   117,    -1,    -1,   120,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,    -1,
     132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,
      -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,
     172,   173,   174,   175,    -1,    -1,    -1,    -1,   180,   181,
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
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,   113,   114,   115,
     116,   117,    -1,    -1,   120,   121,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,   135,
     136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,    -1,    -1,    -1,   163,    -1,    -1,
     166,    -1,    -1,    -1,    -1,    -1,   172,   173,   174,   175,
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
      -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   112,   113,   114,   115,   116,   117,    -1,    -1,
     120,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,
     130,    -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,
      -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,   152,    -1,   154,   155,    -1,   157,   158,   159,
      -1,    -1,    -1,   163,    -1,    -1,   166,    -1,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,    -1,    -1,    -1,
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
      -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,   113,
     114,   115,   116,   117,    -1,    -1,   120,   121,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,
     134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,   163,
      -1,    -1,   166,     3,     4,     5,     6,     7,   172,   173,
     174,   175,    -1,    13,    -1,    -1,   180,   181,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,    -1,    -1,   199,    -1,    -1,    -1,    -1,
     204,   205,    -1,   207,   208,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   112,   113,   114,   115,   116,   117,    -1,    -1,
     120,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,
     130,    -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,
      -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,   152,    -1,   154,   155,    -1,   157,   158,   159,
      -1,    -1,    -1,   163,    -1,    -1,   166,     3,     4,     5,
       6,     7,   172,   173,   174,   175,    -1,    13,    -1,    -1,
     180,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,    -1,    -1,   199,
      -1,    -1,    -1,    -1,   204,   205,    -1,   207,   208,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,   113,   114,   115,
     116,   117,    -1,    -1,   120,   121,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,   135,
     136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,    -1,    -1,    -1,   163,    -1,    -1,
     166,    -1,    -1,    -1,    -1,    -1,   172,   173,   174,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,    -1,    -1,   199,    -1,    -1,    -1,    -1,   204,   205,
      -1,   207,   208,     3,     4,     5,     6,     7,    -1,    -1,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    67,    68,    69,
      70,    71,    72,    73,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,    -1,    -1,   129,
     130,    -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   148,   149,
     150,    -1,    -1,    -1,   154,   155,    -1,   157,   158,   159,
     160,    -1,   162,   163,    -1,   165,    -1,    -1,    -1,    -1,
      -1,    -1,   172,    -1,    -1,    -1,   176,    -1,   178,    -1,
     180,   181,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    29,    53,   151,    -1,    -1,   154,   155,    -1,
     157,   158,   159,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      77,    -1,    29,    -1,    -1,   202,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,   187,    -1,    -1,    -1,    -1,    -1,    -1,
      77,    -1,    -1,   130,   131,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,   148,    53,    -1,   151,   152,    -1,   154,   155,    -1,
     157,   158,   159,    -1,    65,    -1,   186,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    29,   172,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   130,   131,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      55,   148,   199,    -1,   151,   152,   203,   154,   155,    -1,
     157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    -1,   172,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   104,
     154,   155,   199,   157,   158,   159,   203,   112,   113,   114,
     115,   116,   117,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   130,   131,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    55,   148,    -1,   199,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    77,    -1,    29,   172,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   181,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   104,    55,    -1,   199,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    -1,    -1,    77,    -1,    -1,   130,   131,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   148,    29,    -1,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   172,
      -1,    -1,    55,    -1,    -1,    -1,    -1,   130,   131,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    77,   148,   199,    -1,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,    -1,   161,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   172,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,    68,   199,   130,   131,    -1,
      -1,    -1,    -1,    -1,    77,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,   172,
      -1,    -1,    -1,    -1,   117,    -1,    -1,    30,    -1,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    46,    47,    -1,   199,    -1,    -1,    52,
      -1,    54,    -1,    -1,    -1,   148,    -1,    -1,   151,   152,
      -1,   154,   155,    66,   157,   158,   159,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    -1,    -1,    -1,   148,    -1,
      -1,   151,    85,    -1,   154,   155,    -1,   157,   158,   159,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,    -1,   199,    -1,    -1,    -1,
      -1,   204,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,   130,    -1,   132,
     133,   134,   135,   136,    77,    -1,    79,    -1,    -1,    -1,
     143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,    77,    -1,    -1,
     163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   172,
     173,   174,   175,    -1,   117,    -1,    -1,   180,    -1,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    46,    47,    -1,   199,    -1,    -1,    52,
      -1,    54,    -1,    -1,    -1,   148,    -1,    -1,   151,   152,
      -1,   154,   155,    66,   157,   158,   159,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,
      -1,   151,    85,    -1,   154,   155,    -1,   157,   158,   159,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,    -1,   199,    -1,    -1,    -1,
      -1,   204,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,   130,    -1,   132,
     133,   134,   135,   136,    -1,    -1,    -1,    46,    47,    -1,
     143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,    66,    -1,    -1,
     163,    -1,    -1,    -1,    -1,    74,    75,    76,    77,   172,
     173,   174,   175,    -1,    -1,    -1,    85,   180,    -1,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,    -1,   199,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    79,    -1,
      -1,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    -1,    -1,    -1,   117,    -1,    -1,    -1,
      -1,    -1,    -1,   172,    -1,    -1,    -1,    -1,   129,    -1,
      -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   148,    -1,    -1,
     151,   152,    -1,   154,   155,    -1,   157,   158,   159,    77,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,    -1,    -1,   199,   117,
      -1,    -1,    -1,   204,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   129,    -1,    77,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     148,    -1,    -1,   151,   152,    77,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   117,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,   105,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
      -1,   199,    -1,    77,   148,    -1,   204,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,    77,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,   151,
      -1,    -1,   154,   155,    -1,   157,   158,   159,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,    -1,   199,   117,    -1,    -1,    -1,
     204,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   148,    -1,    -1,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,    -1,   148,    -1,    -1,
     151,   152,    -1,   154,   155,    -1,   157,   158,   159,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,   204,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   128,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,   128,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   128,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,   128,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   128,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,   128,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   128,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    29,   128,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,   128,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    29,   128,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    79,    80,    -1,    -1,    -1,    -1,
      65,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,
      29,   128,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   118,    65,    -1,    77,    -1,
      -1,    -1,    -1,   128,    -1,    -1,    -1,   130,   131,   154,
     155,    77,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,   128,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   152,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    -1,    -1,   151,    -1,    -1,   154,   155,
      -1,   157,   158,   159,    -1,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     151,    -1,    -1,   154,   155,    -1,   157,   158,   159,    -1,
      77,    -1,    -1,    -1,   123,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    -1,    -1,    -1,   123,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   154,   155,    -1,
     157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      28,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    96,    53,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   210,   211,     0,   212,     3,     4,     5,     6,     7,
      13,    27,    28,    45,    46,    47,    52,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    66,    67,
      68,    69,    70,    74,    75,    76,    77,    78,    79,    81,
      85,    86,    87,    88,    90,    92,    94,    97,   101,   102,
     103,   104,   105,   106,   107,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   119,   120,   121,   122,   123,   124,
     129,   130,   132,   133,   134,   135,   136,   143,   148,   149,
     150,   151,   152,   154,   155,   157,   158,   159,   160,   163,
     166,   172,   173,   174,   175,   176,   178,   180,   181,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   199,   201,   202,   204,   205,   207,   208,
     213,   216,   223,   224,   225,   226,   227,   228,   231,   247,
     248,   252,   255,   260,   266,   324,   325,   330,   334,   335,
     336,   337,   338,   339,   340,   341,   343,   346,   357,   358,
     359,   360,   361,   365,   366,   368,   387,   397,   398,   399,
     404,   407,   425,   430,   432,   433,   434,   435,   436,   437,
     438,   439,   441,   462,   464,   466,   115,   116,   117,   129,
     148,   158,   216,   247,   324,   340,   432,   340,   199,   340,
     340,   340,   101,   340,   340,   423,   424,   340,   340,   340,
     340,   340,   340,   340,   340,   340,   340,   340,   340,    79,
     117,   199,   224,   398,   399,   432,   432,    35,   340,   445,
     446,   340,   117,   199,   224,   398,   399,   400,   431,   437,
     442,   443,   199,   331,   401,   199,   331,   347,   332,   340,
     233,   331,   199,   199,   199,   331,   201,   340,   216,   201,
     340,    29,    55,   130,   131,   152,   172,   199,   216,   227,
     467,   479,   480,   182,   201,   337,   340,   367,   369,   202,
     240,   340,   104,   105,   151,   217,   220,   223,    79,   204,
     292,   293,   123,   123,    79,   294,   199,   199,   199,   199,
     216,   264,   468,   199,   199,    79,    84,   144,   145,   146,
     459,   460,   151,   202,   223,   223,   216,   265,   468,   152,
     199,   199,   199,   199,   468,   468,    79,   196,   349,   330,
     340,   341,   432,   229,   202,    84,   402,   459,    84,   459,
     459,    30,   151,   168,   469,   199,     9,   201,    35,   246,
     152,   263,   468,   117,   181,   247,   325,   201,   201,   201,
     201,   201,   201,    10,    11,    12,    29,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    53,    65,
     201,    66,    66,   201,   202,   147,   124,   158,   160,   266,
     323,   324,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    63,    64,   127,   427,   428,
      66,   202,   429,   199,    66,   202,   204,   438,   199,   246,
     247,    14,   340,   201,   128,    44,   216,   422,   199,   330,
     432,   147,   432,   128,   206,     9,   409,   330,   432,   469,
     147,   199,   403,   127,   427,   428,   429,   200,   340,    30,
     231,     8,   351,     9,   201,   231,   232,   332,   333,   340,
     216,   278,   235,   201,   201,   201,   480,   480,   168,   199,
     104,   480,    14,   216,    79,   201,   201,   201,   182,   183,
     184,   189,   190,   193,   194,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   382,   383,   384,   241,   108,   165,
     201,   151,   218,   221,   223,   151,   219,   222,   223,   223,
       9,   201,    96,   202,   432,     9,   201,    14,     9,   201,
     432,   463,   463,   330,   341,   432,   200,   168,   258,   129,
     432,   444,   445,    66,   127,   144,   460,    78,   340,   432,
      84,   144,   460,   223,   215,   201,   202,   253,   261,   388,
     390,    85,   204,   352,   353,   355,   399,   438,   464,   340,
     452,   454,   340,   451,   453,   451,    14,    96,   465,   348,
     350,   288,   289,   425,   426,   200,   200,   200,   203,   230,
     231,   248,   255,   260,   425,   340,   205,   207,   208,   216,
     470,   471,   480,    35,   161,   290,   291,   340,   467,   199,
     468,   256,   246,   340,   340,   340,    30,   340,   340,   340,
     340,   340,   340,   340,   340,   340,   340,   340,   340,   340,
     340,   340,   340,   340,   340,   340,   340,   340,   340,   400,
     340,   340,   440,   440,   340,   447,   448,   123,   202,   216,
     437,   438,   264,   216,   265,   263,   247,    27,    35,   334,
     337,   340,   367,   340,   340,   340,   340,   340,   340,   340,
     340,   340,   340,   340,   340,   202,   216,   437,   440,   340,
     290,   440,   340,   444,   246,   200,   340,   199,   421,     9,
     409,   330,   200,   216,    35,   340,    35,   340,   200,   200,
     437,   290,   202,   216,   437,   200,   229,   282,   202,   337,
     340,   340,    88,    30,   231,   276,   201,    28,    96,    14,
       9,   200,    30,   202,   279,   480,    85,   227,   476,   477,
     478,   199,     9,    46,    47,    52,    54,    66,    85,   130,
     143,   152,   172,   173,   174,   175,   199,   224,   225,   227,
     362,   363,   364,   398,   404,   405,   406,   185,    79,   340,
      79,    79,   340,   379,   380,   340,   340,   372,   382,   188,
     385,   229,   199,   239,   223,   201,     9,    96,   223,   201,
       9,    96,    96,   220,   216,   340,   293,   405,    79,     9,
     200,   200,   200,   200,   200,   201,    46,    47,   474,   475,
     125,   269,   199,     9,   200,   200,    79,    80,   216,   461,
     216,    66,   203,   203,   212,   214,    30,   126,   268,   167,
      50,   152,   167,   392,   128,     9,   409,   200,   147,   200,
       9,   409,   128,   200,     9,   409,   200,   480,   480,    14,
     351,   288,   197,     9,   410,   480,   481,   127,   427,   428,
     429,   203,     9,   409,   169,   432,   340,   200,     9,   410,
      14,   344,   249,   125,   267,   199,   468,   340,    30,   206,
     206,   128,   203,     9,   409,   340,   469,   199,   259,   254,
     262,   257,   246,    68,   432,   340,   469,   199,   206,   203,
     200,   206,   203,   200,    46,    47,    66,    74,    75,    76,
      85,   130,   143,   172,   216,   412,   414,   417,   420,   216,
     432,   432,   128,   427,   428,   429,   200,   340,   283,    71,
      72,   284,   229,   331,   229,   333,    96,    35,   129,   273,
     432,   405,   216,    30,   231,   277,   201,   280,   201,   280,
       9,   169,   128,   147,     9,   409,   200,   161,   470,   471,
     472,   470,   405,   405,   405,   405,   405,   408,   411,   199,
      84,   147,   199,   199,   199,   199,   405,   147,   202,    10,
      11,    12,    29,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    65,   340,   185,   185,    14,   191,
     192,   381,     9,   195,   385,    79,   203,   398,   202,   243,
      96,   221,   216,    96,   222,   216,   216,   203,    14,   432,
     201,     9,   169,   216,   270,   398,   202,   444,   129,   432,
      14,   206,   340,   203,   212,   480,   270,   202,   391,    14,
     340,   352,   216,   340,   340,   340,   201,   480,   197,    30,
     473,   426,    35,    79,   161,   202,   216,   437,   480,    35,
     161,   340,   405,   288,   199,   398,   268,   345,   250,   340,
     340,   340,   203,   199,   290,   269,    30,   268,   267,   468,
     400,   203,   199,   290,    14,    74,    75,    76,   216,   413,
     413,   414,   415,   416,   199,    84,   144,   199,     9,   409,
     200,   421,    35,   340,   203,    71,    72,   285,   331,   231,
     203,   201,    89,   201,   273,   432,   199,   128,   272,    14,
     229,   280,    98,    99,   100,   280,   203,   480,   480,   216,
     476,     9,   200,   409,   128,   206,     9,   409,   408,   216,
     352,   354,   356,   405,   456,   458,   405,   455,   457,   455,
     200,   123,   216,   405,   449,   450,   405,   405,   405,    30,
     405,   405,   405,   405,   405,   405,   405,   405,   405,   405,
     405,   405,   405,   405,   405,   405,   405,   405,   405,   405,
     405,   405,   405,   340,   340,   340,   380,   340,   370,    79,
     244,   216,   216,   405,   475,    96,     9,   298,   200,   199,
     334,   337,   340,   206,   203,   465,   298,   153,   166,   202,
     387,   394,   153,   202,   393,   128,   128,   201,   473,   480,
     351,   481,    79,    14,    79,   340,   469,   199,   432,   340,
     200,   288,   202,   288,   199,   128,   199,   290,   200,   202,
     480,   202,   268,   251,   403,   199,   290,   200,   128,   206,
       9,   409,   415,   144,   352,   418,   419,   414,   432,   331,
      30,    73,   231,   201,   333,   272,   444,   273,   200,   405,
      95,    98,   201,   340,    30,   201,   281,   203,   169,   128,
     161,    30,   200,   405,   405,   200,   128,     9,   409,   200,
     200,     9,   409,   128,   200,     9,   409,   200,   128,   203,
       9,   409,   405,    30,   186,   200,   229,   216,   480,   398,
       4,   105,   110,   118,   154,   155,   157,   203,   299,   322,
     323,   324,   329,   425,   444,   203,   202,   203,    50,   340,
     340,   340,   340,   351,    35,    79,   161,    14,   405,   203,
     199,   290,   473,   200,   298,   200,   288,   340,   290,   200,
     298,   465,   298,   202,   199,   290,   200,   414,   414,   200,
     128,   200,     9,   409,    30,   229,   201,   200,   200,   200,
     236,   201,   201,   281,   229,   480,   480,   128,   405,   352,
     405,   405,   405,   405,   405,   405,   340,   202,   203,    96,
     125,   126,   467,   271,   398,   118,   130,   131,   152,   158,
     308,   309,   310,   398,   156,   314,   315,   121,   199,   216,
     316,   317,   300,   247,   480,     9,   201,   323,   200,   295,
     152,   389,   203,   203,    79,    14,    79,   405,   199,   290,
     200,   110,   342,   473,   203,   473,   200,   200,   203,   202,
     203,   298,   288,   200,   128,   414,   352,   229,   234,   237,
      30,   231,   275,   229,   200,   405,   128,   128,   128,   187,
     229,   480,   398,   398,    14,     9,   201,   202,   202,     9,
     201,     3,     4,     5,     6,     7,    10,    11,    12,    13,
      27,    28,    53,    67,    68,    69,    70,    71,    72,    73,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   129,   130,   132,   133,   134,   135,   136,   148,   149,
     150,   160,   162,   163,   165,   172,   176,   178,   180,   181,
     216,   395,   396,     9,   201,   152,   156,   216,   317,   318,
     319,   201,    79,   328,   246,   301,   467,   247,   203,   296,
     297,   467,    14,   405,   290,   200,   199,   202,   201,   202,
     320,   342,   473,   295,   203,   200,   414,   128,    30,   231,
     274,   275,   229,   405,   405,   405,   340,   203,   201,   201,
     405,   398,   304,   311,   404,   309,    14,    30,    47,   312,
     315,     9,    33,   200,    29,    46,    49,    14,     9,   201,
     468,   328,    14,   246,   201,    14,   405,   200,    35,    79,
     386,   229,   229,   202,   320,   203,   473,   414,   229,    93,
     188,   242,   203,   216,   227,   305,   306,   307,     9,   203,
     405,   396,   396,    55,   313,   318,   318,    29,    46,    49,
     405,    79,   199,   201,   405,   468,   405,    79,     9,   410,
     203,   203,   229,   320,    91,   201,    79,   108,   238,   147,
      96,   404,   159,    14,   302,   199,    35,    79,   200,   203,
     201,   199,   165,   245,   216,   323,   324,   405,   286,   287,
     426,   303,    79,   398,   243,   162,   216,   201,   200,     9,
     410,   112,   113,   114,   326,   327,   286,    79,   271,   201,
     473,   426,   481,   200,   200,   201,   201,   202,   321,   326,
      35,    79,   161,   473,   202,   229,   481,    79,    14,    79,
     321,   229,   203,    35,    79,   161,    14,   405,   203,    79,
      14,    79,   405,    14,   405,   405
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
#line 797 "hphp.y"
    { ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 798 "hphp.y"
    { ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 803 "hphp.y"
    { ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 804 "hphp.y"
    { ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 809 "hphp.y"
    { ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 810 "hphp.y"
    { ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 814 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 815 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 816 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 818 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 822 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 823 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 824 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 826 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 830 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 831 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 832 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 834 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 843 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 845 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 846 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 849 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 856 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 863 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 871 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 874 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 880 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 881 "hphp.y"
    { _p->onStatementListStart((yyval));;}
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
#line 887 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 890 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 894 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 899 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 900 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 902 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 906 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 909 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 913 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 915 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 918 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 920 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 923 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 924 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 925 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 926 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 928 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 929 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 930 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 931 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 933 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 934 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 935 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 938 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 940 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 945 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 947 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 951 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 958 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 959 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 962 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 963 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 964 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 965 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval));;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 969 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
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
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 976 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 977 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 978 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 979 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 987 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 988 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 997 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 998 "hphp.y"
    { (yyval).reset();;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1004 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1010 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1011 "hphp.y"
    { (yyval).reset();;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1015 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1016 "hphp.y"
    { (yyval).reset();;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1020 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1025 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1031 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1037 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1043 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1049 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1055 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1063 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1067 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1071 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1075 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1081 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1084 "hphp.y"
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

  case 144:

/* Line 1455 of yacc.c  */
#line 1099 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1102 "hphp.y"
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

  case 146:

/* Line 1455 of yacc.c  */
#line 1116 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1119 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1124 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1127 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1134 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1137 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1145 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1148 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1156 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1157 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1161 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1164 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1167 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1168 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1169 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1173 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1174 "hphp.y"
    { (yyval).reset();;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1177 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1178 "hphp.y"
    { (yyval).reset();;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1181 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1182 "hphp.y"
    { (yyval).reset();;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1185 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1187 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1190 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1192 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1196 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1197 "hphp.y"
    { (yyval).reset();;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1200 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1201 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1202 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1206 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1208 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1211 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1213 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1216 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1218 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1221 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1223 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1233 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1234 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1235 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1236 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1241 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1243 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1244 "hphp.y"
    { (yyval).reset();;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1247 "hphp.y"
    { (yyval).reset();;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1248 "hphp.y"
    { (yyval).reset();;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1253 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1254 "hphp.y"
    { (yyval).reset();;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1259 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1260 "hphp.y"
    { (yyval).reset();;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1263 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1264 "hphp.y"
    { (yyval).reset();;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1267 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1268 "hphp.y"
    { (yyval).reset();;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1276 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1282 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1290 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1295 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1298 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1304 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1308 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1313 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1318 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1323 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1328 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1334 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1340 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1348 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1353 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1357 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1360 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1364 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1367 "hphp.y"
    { (yyval).reset();;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1372 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1375 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1379 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1383 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1387 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1391 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1396 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1401 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1407 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1408 "hphp.y"
    { (yyval).reset();;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1411 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1412 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1413 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1415 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1417 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1419 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1423 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1424 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1427 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1428 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1429 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1433 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1435 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1436 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1437 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1442 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1443 "hphp.y"
    { (yyval).reset();;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1446 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1451 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1457 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1458 "hphp.y"
    { (yyval).reset();;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1461 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1462 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1465 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1466 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1468 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1472 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1478 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1485 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1491 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1498 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1500 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1502 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1504 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1505 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1508 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1511 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1512 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1513 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1519 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1523 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1526 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1533 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1534 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1539 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1542 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1555 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1556 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1575 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1580 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1581 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1585 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1588 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1593 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1598 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1601 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1605 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1606 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1607 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1608 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1615 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1618 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1620 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1624 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1627 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1628 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1632 "hphp.y"
    { (yyval).reset();;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1633 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1637 "hphp.y"
    { (yyval).reset();;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1638 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1641 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1642 "hphp.y"
    { (yyval).reset();;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1645 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1646 "hphp.y"
    { (yyval).reset();;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1649 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1651 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1654 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1655 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1657 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1658 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1659 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1660 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1664 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1665 "hphp.y"
    { (yyval).reset();;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1668 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1669 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1670 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1676 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1677 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1678 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1682 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1684 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1688 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1690 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1693 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1696 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1700 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1701 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { (yyval).reset();;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1710 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1724 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1728 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1739 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1744 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1745 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1748 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1749 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1751 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1752 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1761 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { (yyval).reset();;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { Token v; Token w;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1878 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1886 "hphp.y"
    { Token v; Token w;
                                         (yyvsp[(1) - (4)]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[(1) - (4)]), nullptr, (yyvsp[(1) - (4)]));
                                         _p->finishStatement((yyvsp[(4) - (4)]), (yyvsp[(4) - (4)])); (yyvsp[(4) - (4)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[(1) - (4)]),
                                                            v,(yyvsp[(2) - (4)]),w,(yyvsp[(4) - (4)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1895 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { Token u; Token v;
                                         (yyvsp[(1) - (7)]) = T_ASYNC;
                                         _p->onMemberModifier((yyvsp[(1) - (7)]), nullptr, (yyvsp[(1) - (7)]));
                                         _p->finishStatement((yyvsp[(7) - (7)]), (yyvsp[(7) - (7)])); (yyvsp[(7) - (7)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            &(yyvsp[(1) - (7)]),
                                                            u,(yyvsp[(4) - (7)]),v,(yyvsp[(7) - (7)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1966 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1971 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1980 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1981 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1986 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1992 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1993 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1997 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_VARRAY);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 2002 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 2004 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 2009 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_VARRAY);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 2014 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 2021 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 2023 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 2027 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 2028 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 2029 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 2031 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 2035 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 2039 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 2043 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2048 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2050 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2052 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2054 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2058 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2060 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2064 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2065 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2066 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2067 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2068 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2069 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2073 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2077 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2081 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2086 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2091 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2095 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2099 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2100 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2104 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2105 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2109 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2110 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2114 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2115 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2123 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2127 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2132 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2133 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2141 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
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

  case 516:

/* Line 1455 of yacc.c  */
#line 2155 "hphp.y"
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

  case 517:

/* Line 1455 of yacc.c  */
#line 2166 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2167 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2173 "hphp.y"
    { (yyval).reset();;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2176 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2177 "hphp.y"
    { (yyval).reset();;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2184 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2187 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2190 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2204 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2210 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2211 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2213 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2214 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2215 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2216 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2219 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2221 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2222 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2227 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2236 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval).reset();;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval).reset();;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval).reset();;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { (yyval).reset();;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2332 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2337 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2338 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2343 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2348 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2352 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2353 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2355 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2421 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2438 "hphp.y"
    { (yyval).reset();;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { (yyval).reset();;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { (yyval).reset();;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2447 "hphp.y"
    { (yyval).reset();;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2465 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2471 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2475 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2476 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2481 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2485 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2486 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2488 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2493 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2494 "hphp.y"
    { (yyval).reset();;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2503 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2520 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2523 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2528 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2529 "hphp.y"
    { (yyval).reset();;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2540 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2545 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2547 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2550 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2553 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { (yyval).reset();;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2558 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2564 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2565 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2569 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2570 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2574 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2576 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2581 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2583 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2588 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2589 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2590 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2591 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2594 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2597 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2599 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2613 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2618 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2619 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2626 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2629 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2632 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2640 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2644 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2655 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2659 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2663 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2665 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2671 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2672 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2675 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2676 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2679 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2680 "hphp.y"
    { (yyval).reset();;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2684 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2685 "hphp.y"
    { (yyval)++;;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2689 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2690 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2691 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2693 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2696 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2697 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2701 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2703 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2705 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2706 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2710 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2711 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2713 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2715 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2716 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2721 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2722 "hphp.y"
    { (yyval).reset();;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2726 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2727 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2728 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2729 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2732 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2735 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2741 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2742 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2746 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2747 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2748 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2749 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2755 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2760 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2762 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2764 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2765 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2771 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2776 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2780 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2781 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2784 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2785 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2791 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2796 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2803 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2807 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2808 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2812 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2814 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2815 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2817 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2822 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2824 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2826 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2828 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2830 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2831 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2835 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2840 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2841 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2842 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2843 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2844 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2845 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2846 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2847 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2848 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2852 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2853 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2858 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2860 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2874 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2878 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2884 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2885 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2891 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2895 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2901 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2902 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2906 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2909 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2915 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2920 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2922 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2923 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2927 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2928 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2933 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (4)]).text()); ;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2934 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (2)]).text()); ;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2937 "hphp.y"
    { _p->addTypeVar((yyvsp[(4) - (6)]).text()); ;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2939 "hphp.y"
    { _p->addTypeVar((yyvsp[(2) - (4)]).text()); ;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    {;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2944 "hphp.y"
    {;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2945 "hphp.y"
    {;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 2951 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 2956 "hphp.y"
    { ;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 2968 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 2970 "hphp.y"
    {;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 2974 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 2981 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 2987 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 2988 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 2991 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 2994 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 2996 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 2999 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3002 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3008 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 920:

/* Line 1455 of yacc.c  */
#line 3014 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 921:

/* Line 1455 of yacc.c  */
#line 3022 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 922:

/* Line 1455 of yacc.c  */
#line 3023 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13544 "hphp.tab.cpp"
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
#line 3026 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

