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
#line 644 "hphp.tab.cpp"

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
#define YYLAST   16689

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  209
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  272
/* YYNRULES -- Number of rules.  */
#define YYNRULES  919
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1742

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
    2997,  3001,  3003,  3005,  3006,  3007,  3010,  3014,  3016,  3022,
    3026,  3030,  3036,  3040,  3042,  3045,  3046,  3051,  3054,  3057,
    3059,  3061,  3063,  3065,  3070,  3077,  3079,  3088,  3095,  3097
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
     216,   253,    30,   479,   465,   202,   295,   203,    -1,    -1,
     425,   158,   216,   254,    30,   479,   465,   202,   295,   203,
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
     287,     9,   426,   326,   480,   161,    79,    -1,   287,     9,
     426,   326,   480,   161,    -1,   287,   410,    -1,   426,   326,
     480,   161,    79,    -1,   426,   326,   480,   161,    -1,    -1,
     426,   326,   480,    79,    -1,   426,   326,   480,    35,    79,
      -1,   426,   326,   480,    35,    79,    14,   405,    -1,   426,
     326,   480,    79,    14,   405,    -1,   287,     9,   426,   326,
     480,    79,    -1,   287,     9,   426,   326,   480,    35,    79,
      -1,   287,     9,   426,   326,   480,    35,    79,    14,   405,
      -1,   287,     9,   426,   326,   480,    79,    14,   405,    -1,
     289,     9,   426,   480,   161,    79,    -1,   289,     9,   426,
     480,   161,    -1,   289,   410,    -1,   426,   480,   161,    79,
      -1,   426,   480,   161,    -1,    -1,   426,   480,    79,    -1,
     426,   480,    35,    79,    -1,   426,   480,    35,    79,    14,
     405,    -1,   426,   480,    79,    14,   405,    -1,   289,     9,
     426,   480,    79,    -1,   289,     9,   426,   480,    35,    79,
      -1,   289,     9,   426,   480,    35,    79,    14,   405,    -1,
     289,     9,   426,   480,    79,    14,   405,    -1,   291,   410,
      -1,    -1,   340,    -1,    35,   432,    -1,   161,   340,    -1,
     291,     9,   340,    -1,   291,     9,   161,   340,    -1,   291,
       9,    35,   432,    -1,   292,     9,   293,    -1,   293,    -1,
      79,    -1,   204,   432,    -1,   204,   202,   340,   203,    -1,
     294,     9,    79,    -1,   294,     9,    79,    14,   405,    -1,
      79,    -1,    79,    14,   405,    -1,   295,   296,    -1,    -1,
     297,   201,    -1,   467,    14,   405,    -1,   298,   299,    -1,
      -1,    -1,   322,   300,   328,   201,    -1,    -1,   324,   479,
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
     399,   147,   123,    -1,    -1,    96,   479,    -1,   176,   468,
      14,   479,   201,    -1,   178,   468,   465,    14,   479,   201,
      -1,   216,    -1,   479,   216,    -1,   216,    -1,   216,   168,
     474,   169,    -1,   168,   471,   169,    -1,    -1,   479,    -1,
     470,     9,   479,    -1,   470,   409,    -1,   470,     9,   161,
      -1,   471,    -1,   161,    -1,    -1,    -1,    30,   479,    -1,
     474,     9,   216,    -1,   216,    -1,   474,     9,   216,    96,
     479,    -1,   216,    96,   479,    -1,    85,   128,   479,    -1,
     227,   147,   216,   128,   479,    -1,   476,     9,   475,    -1,
     475,    -1,   476,   409,    -1,    -1,   172,   199,   477,   200,
      -1,    29,   479,    -1,    55,   479,    -1,   227,    -1,   130,
      -1,   131,    -1,   478,    -1,   130,   168,   479,   169,    -1,
     130,   168,   479,     9,   479,   169,    -1,   152,    -1,   199,
     104,   199,   472,   200,    30,   479,   200,    -1,   199,   479,
       9,   470,   409,   200,    -1,   479,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   733,   733,   733,   742,   744,   747,   748,   749,   750,
     751,   752,   753,   756,   758,   758,   760,   760,   762,   763,
     765,   767,   772,   773,   774,   775,   776,   777,   778,   779,
     780,   781,   782,   783,   784,   785,   786,   787,   788,   789,
     790,   794,   796,   800,   802,   806,   808,   812,   813,   814,
     815,   820,   821,   822,   823,   828,   829,   830,   831,   836,
     837,   841,   842,   844,   847,   853,   860,   867,   871,   877,
     879,   882,   883,   884,   885,   888,   889,   893,   898,   898,
     904,   904,   911,   910,   916,   916,   921,   922,   923,   924,
     925,   926,   927,   928,   929,   930,   931,   932,   933,   936,
     934,   943,   941,   948,   956,   950,   960,   958,   962,   963,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   985,   985,   990,   996,  1000,  1000,  1008,  1009,  1013,
    1014,  1018,  1023,  1022,  1035,  1033,  1047,  1045,  1061,  1060,
    1069,  1067,  1079,  1078,  1097,  1095,  1114,  1113,  1122,  1120,
    1132,  1131,  1143,  1141,  1154,  1155,  1159,  1162,  1165,  1166,
    1167,  1170,  1172,  1175,  1176,  1179,  1180,  1183,  1184,  1188,
    1189,  1194,  1195,  1198,  1199,  1200,  1204,  1205,  1209,  1210,
    1214,  1215,  1219,  1220,  1225,  1226,  1231,  1232,  1233,  1234,
    1237,  1240,  1242,  1245,  1246,  1250,  1252,  1255,  1258,  1261,
    1262,  1265,  1266,  1270,  1276,  1283,  1285,  1290,  1296,  1300,
    1304,  1308,  1313,  1318,  1323,  1328,  1334,  1343,  1348,  1354,
    1356,  1360,  1365,  1369,  1372,  1375,  1379,  1383,  1387,  1391,
    1396,  1404,  1406,  1409,  1410,  1411,  1412,  1414,  1416,  1421,
    1422,  1425,  1426,  1427,  1431,  1432,  1434,  1435,  1439,  1441,
    1444,  1448,  1454,  1456,  1459,  1459,  1463,  1462,  1466,  1470,
    1468,  1483,  1480,  1493,  1495,  1497,  1499,  1501,  1503,  1505,
    1509,  1510,  1511,  1514,  1520,  1523,  1529,  1532,  1537,  1539,
    1544,  1549,  1553,  1554,  1560,  1561,  1563,  1567,  1568,  1573,
    1574,  1578,  1579,  1583,  1585,  1591,  1596,  1597,  1599,  1603,
    1604,  1605,  1606,  1610,  1611,  1612,  1613,  1614,  1615,  1617,
    1622,  1625,  1626,  1630,  1631,  1635,  1636,  1639,  1640,  1643,
    1644,  1647,  1648,  1652,  1653,  1654,  1655,  1656,  1657,  1658,
    1662,  1663,  1666,  1667,  1668,  1671,  1673,  1675,  1676,  1679,
    1681,  1686,  1687,  1689,  1690,  1691,  1694,  1698,  1699,  1703,
    1704,  1708,  1709,  1713,  1717,  1722,  1726,  1730,  1735,  1736,
    1737,  1740,  1742,  1743,  1744,  1747,  1748,  1749,  1750,  1751,
    1752,  1753,  1754,  1755,  1756,  1757,  1758,  1759,  1760,  1761,
    1762,  1763,  1764,  1765,  1766,  1767,  1768,  1769,  1770,  1771,
    1772,  1773,  1774,  1775,  1776,  1777,  1778,  1779,  1780,  1781,
    1782,  1783,  1784,  1785,  1786,  1787,  1788,  1789,  1791,  1792,
    1794,  1796,  1797,  1798,  1799,  1800,  1801,  1802,  1803,  1804,
    1805,  1806,  1807,  1808,  1809,  1810,  1811,  1812,  1813,  1814,
    1815,  1816,  1817,  1818,  1822,  1826,  1831,  1830,  1845,  1843,
    1860,  1860,  1876,  1875,  1893,  1893,  1909,  1908,  1929,  1930,
    1931,  1936,  1938,  1942,  1946,  1952,  1956,  1962,  1964,  1968,
    1970,  1974,  1978,  1979,  1983,  1990,  1991,  1995,  1999,  2001,
    2006,  2011,  2018,  2020,  2025,  2026,  2027,  2029,  2033,  2037,
    2041,  2045,  2047,  2049,  2051,  2056,  2057,  2062,  2063,  2064,
    2065,  2066,  2067,  2071,  2075,  2079,  2083,  2088,  2093,  2097,
    2098,  2102,  2103,  2107,  2108,  2112,  2113,  2117,  2121,  2125,
    2129,  2130,  2131,  2132,  2136,  2142,  2151,  2164,  2165,  2168,
    2171,  2174,  2175,  2178,  2182,  2185,  2188,  2195,  2196,  2200,
    2201,  2203,  2207,  2208,  2209,  2210,  2211,  2212,  2213,  2214,
    2215,  2216,  2217,  2218,  2219,  2220,  2221,  2222,  2223,  2224,
    2225,  2226,  2227,  2228,  2229,  2230,  2231,  2232,  2233,  2234,
    2235,  2236,  2237,  2238,  2239,  2240,  2241,  2242,  2243,  2244,
    2245,  2246,  2247,  2248,  2249,  2250,  2251,  2252,  2253,  2254,
    2255,  2256,  2257,  2258,  2259,  2260,  2261,  2262,  2263,  2264,
    2265,  2266,  2267,  2268,  2269,  2270,  2271,  2272,  2273,  2274,
    2275,  2276,  2277,  2278,  2279,  2280,  2281,  2282,  2283,  2284,
    2285,  2286,  2290,  2295,  2296,  2299,  2300,  2301,  2305,  2306,
    2307,  2311,  2312,  2313,  2317,  2318,  2319,  2322,  2324,  2328,
    2329,  2330,  2331,  2333,  2334,  2335,  2336,  2337,  2338,  2339,
    2340,  2341,  2342,  2345,  2350,  2351,  2352,  2354,  2355,  2357,
    2358,  2359,  2360,  2361,  2362,  2364,  2366,  2368,  2370,  2372,
    2373,  2374,  2375,  2376,  2377,  2378,  2379,  2380,  2381,  2382,
    2383,  2384,  2385,  2386,  2387,  2388,  2390,  2392,  2394,  2396,
    2397,  2400,  2401,  2405,  2407,  2411,  2414,  2417,  2423,  2424,
    2425,  2426,  2427,  2428,  2429,  2434,  2436,  2440,  2441,  2444,
    2445,  2449,  2452,  2454,  2456,  2460,  2461,  2462,  2463,  2465,
    2468,  2472,  2473,  2474,  2475,  2478,  2479,  2480,  2481,  2482,
    2484,  2485,  2490,  2492,  2495,  2498,  2500,  2502,  2505,  2507,
    2511,  2513,  2516,  2519,  2525,  2527,  2530,  2531,  2536,  2539,
    2543,  2543,  2548,  2551,  2552,  2556,  2557,  2562,  2563,  2567,
    2568,  2572,  2573,  2578,  2580,  2585,  2586,  2587,  2588,  2589,
    2590,  2591,  2593,  2596,  2598,  2602,  2603,  2604,  2605,  2606,
    2608,  2610,  2612,  2616,  2617,  2618,  2622,  2625,  2628,  2631,
    2635,  2639,  2646,  2650,  2654,  2661,  2662,  2667,  2669,  2670,
    2673,  2674,  2677,  2678,  2682,  2683,  2687,  2688,  2689,  2690,
    2692,  2695,  2698,  2699,  2700,  2702,  2704,  2708,  2709,  2710,
    2712,  2713,  2714,  2718,  2720,  2723,  2725,  2726,  2727,  2728,
    2731,  2733,  2734,  2738,  2740,  2743,  2745,  2746,  2747,  2751,
    2753,  2756,  2759,  2761,  2763,  2767,  2769,  2772,  2774,  2777,
    2779,  2782,  2783,  2787,  2789,  2792,  2794,  2797,  2800,  2804,
    2806,  2810,  2811,  2813,  2814,  2820,  2821,  2823,  2825,  2827,
    2829,  2832,  2833,  2834,  2838,  2839,  2840,  2841,  2842,  2843,
    2844,  2845,  2846,  2850,  2851,  2855,  2857,  2865,  2867,  2871,
    2875,  2882,  2883,  2889,  2890,  2897,  2900,  2904,  2907,  2912,
    2917,  2919,  2920,  2921,  2925,  2926,  2930,  2932,  2933,  2935,
    2939,  2942,  2951,  2953,  2957,  2960,  2963,  2971,  2974,  2977,
    2978,  2981,  2984,  2985,  2988,  2992,  2996,  3002,  3012,  3013
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
  "hh_shape_member_type", "hh_non_empty_shape_member_list",
  "hh_shape_member_list", "hh_shape_type", "hh_type", "hh_type_opt", 0
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
     475,   475,   476,   476,   477,   477,   478,   479,   479,   479,
     479,   479,   479,   479,   479,   479,   479,   479,   480,   480
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
       3,     1,     1,     0,     0,     2,     3,     1,     5,     3,
       3,     5,     3,     1,     2,     0,     4,     2,     2,     1,
       1,     1,     1,     4,     6,     1,     8,     6,     1,     0
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
       0,     0,     0,   910,   911,   915,     0,     0,    59,   909,
       0,   912,     0,     0,    90,     0,     0,     0,     0,   121,
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
       0,     0,     0,    87,   109,    89,   907,   908,     0,   905,
       0,     0,     0,   882,     0,   116,    91,   119,     0,     0,
       0,     0,     0,     0,     0,   478,     0,   485,   487,   488,
     489,   490,   491,   492,   483,   505,   506,    70,     0,   106,
     108,     0,     0,    44,    51,     0,     0,    46,    55,    48,
       0,    18,     0,     0,   242,     0,    93,     0,     0,    94,
     873,     0,     0,   360,   358,   359,     0,     0,   166,     0,
     811,     0,     0,     0,     0,   642,   854,   689,     0,     0,
     852,   694,   851,    62,     5,    13,    14,     0,   164,     0,
       0,   451,     0,     0,   698,     0,     0,   617,   452,   842,
       0,   698,     0,     0,   698,     0,     0,     0,     0,     0,
     744,     0,   700,   743,   919,   341,   411,   764,    75,    69,
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
       0,     0,   192,   192,    85,     0,     0,     0,   903,   698,
       0,   893,     0,     0,     0,     0,     0,   696,   632,     0,
       0,   614,     0,     0,     0,     0,     0,    64,   645,   613,
     651,   652,   650,     0,   644,    68,   649,     0,     0,   495,
       0,     0,   501,   498,   499,   507,     0,   486,   481,     0,
     484,     0,     0,     0,    52,    19,     0,     0,    56,    20,
       0,     0,     0,    41,    49,     0,   239,   247,   244,     0,
       0,   864,   867,   866,   865,    12,   897,     0,     0,     0,
     810,   807,     0,   462,   863,   862,   861,     0,   857,     0,
     858,   860,     0,     5,     0,     0,     0,   514,   515,   523,
     522,     0,     0,   697,   457,   461,     0,   467,   697,   837,
       0,   465,   697,   835,   466,     0,   878,     0,   443,     0,
     894,   744,   219,   918,     0,     0,   761,   745,   752,   791,
     697,   889,   885,   234,   235,   612,   699,   231,     0,   744,
       0,     0,   164,   438,   134,   413,     0,   472,   473,     0,
     464,   697,   823,     0,     0,   232,   166,     0,   164,   162,
       0,   802,   363,     0,     0,   232,   749,   750,   763,   787,
     788,     0,     0,     0,   723,   705,   706,   707,   708,     0,
       0,     0,   716,   715,   729,   698,     0,   737,   821,   820,
       0,   799,   745,   754,   627,     0,   202,     0,     0,    76,
       0,     0,     0,     0,     0,     0,     0,   172,   173,   184,
       0,    70,   182,   103,   192,     0,   192,     0,     0,   913,
       0,     0,   697,   904,   906,   892,   698,   891,     0,   698,
     673,   674,   671,   672,   704,     0,   698,   696,     0,     0,
     460,   846,   844,   844,     0,     0,   830,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   480,     0,     0,     0,   503,   504,   502,
       0,     0,   482,     0,   122,     0,   125,   107,     0,    43,
      53,     0,    45,    57,    50,   243,     0,   874,    96,     0,
       0,   884,   165,   167,   253,     0,     0,   808,     0,   856,
       0,    17,     0,   877,   163,   253,     0,     0,   454,     0,
     875,   841,   840,     0,   879,     0,   894,     0,     0,   919,
       0,   223,   221,     0,   747,   746,   888,     0,     0,   236,
      67,     0,   744,   161,     0,   744,     0,   412,   827,   826,
       0,   232,     0,     0,     0,     0,   164,   136,   628,   748,
     232,     0,     0,   711,   712,   713,   714,   717,   718,   727,
       0,   698,   723,     0,   710,   731,   697,   734,   736,   738,
       0,   815,   748,     0,     0,     0,     0,   199,   450,    81,
       0,   350,   172,   174,   810,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   186,     0,   900,     0,   902,   697,
       0,     0,     0,   647,   697,   695,     0,   686,     0,   698,
       0,   850,     0,   698,     0,     0,   698,     0,   653,   687,
     685,   834,     0,   698,   656,   658,   657,     0,     0,   654,
     655,   659,   661,   660,   676,   675,   678,   677,   679,   681,
     682,   680,   669,   668,   663,   664,   662,   665,   666,   667,
     670,   493,     0,   494,   500,   508,   509,     0,    70,    54,
      58,   245,   899,   896,     0,   320,   812,   810,   354,   357,
     361,     0,    15,     0,   320,   526,     0,     0,   528,   521,
     524,     0,   519,     0,     0,   880,     0,   895,   445,     0,
     224,     0,   220,     0,     0,   232,   238,   237,   894,     0,
     253,     0,   744,     0,   232,     0,   783,   253,   877,   253,
       0,     0,   364,   232,     0,   777,     0,   720,   697,   722,
       0,   709,     0,     0,   698,   728,   819,     0,    70,     0,
     195,   181,     0,     0,     0,   171,    99,   185,     0,     0,
     188,     0,   193,   194,    70,   187,   914,     0,   890,     0,
     917,   703,   702,   646,     0,   697,   459,   648,   470,   697,
     845,     0,   468,   697,   843,   469,     0,   471,   697,   829,
     684,     0,     0,     0,     0,     0,   168,     0,     0,     0,
     318,     0,     0,     0,   147,   252,   254,     0,   317,     0,
     320,     0,   859,   249,   151,   517,     0,     0,   453,   839,
     447,     0,   227,   218,     0,   226,   748,   232,     0,   435,
     894,   320,   894,     0,   825,     0,   782,   320,     0,   320,
     253,   744,     0,   776,   726,   725,   719,     0,   721,   697,
     730,    70,   201,    77,    82,   101,   175,     0,   183,   189,
      70,   191,   901,     0,     0,   456,     0,   849,   848,     0,
     833,   832,   683,     0,    70,   126,   898,     0,     0,     0,
       0,   169,   284,   282,   286,   614,    27,     0,   278,     0,
     283,   295,     0,   293,   298,     0,   297,     0,   296,     0,
     130,   256,     0,   258,     0,   809,     0,   518,   516,   527,
     525,   228,     0,   217,   225,   232,     0,   780,     0,     0,
       0,   143,   435,   894,   784,   149,   249,   153,   320,     0,
     778,     0,   733,     0,   197,     0,     0,    70,   178,   100,
     190,   916,   701,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   268,   272,     0,     0,   263,   578,   577,   574,
     576,   575,   595,   597,   596,   566,   537,   538,   556,   572,
     571,   533,   543,   544,   546,   545,   565,   549,   547,   548,
     550,   551,   552,   553,   554,   555,   557,   558,   559,   560,
     561,   562,   564,   563,   534,   535,   536,   539,   540,   542,
     580,   581,   590,   589,   588,   587,   586,   585,   573,   592,
     582,   583,   584,   567,   568,   569,   570,   593,   594,   598,
     600,   599,   601,   602,   579,   604,   603,   606,   608,   607,
     541,   611,   609,   610,   605,   591,   532,   290,   529,     0,
     264,   311,   312,   310,   303,     0,   304,   265,   337,     0,
       0,     0,     0,   130,   139,   248,     0,     0,     0,   230,
       0,   779,     0,    70,   313,    70,   133,     0,     0,     0,
     145,   894,   724,     0,    70,   176,    83,   102,     0,   455,
     847,   831,   496,   124,   266,   267,   340,   170,     0,     0,
     287,   279,     0,     0,     0,   292,   294,     0,     0,   299,
     306,   307,   305,     0,     0,   255,     0,     0,     0,     0,
     250,     0,   229,   781,     0,   512,   700,     0,     0,    70,
     135,   141,     0,   732,     0,     0,     0,   104,   269,    59,
       0,   270,   271,     0,     0,   285,   289,   530,   531,     0,
     280,   308,   309,   301,   302,   300,   338,   335,   259,   257,
     339,     0,   251,   513,   699,     0,   437,   314,     0,   137,
       0,   179,   497,     0,   128,     0,   320,   288,   291,     0,
     744,   261,     0,   510,   434,   439,   177,     0,     0,   105,
     276,     0,   319,   336,     0,   700,   331,   744,   511,     0,
     127,     0,     0,   275,   894,   744,   205,   332,   333,   334,
     919,   330,     0,     0,     0,   274,     0,   331,     0,   894,
       0,   273,   315,    70,   260,   919,     0,   209,   207,     0,
      70,     0,     0,   210,     0,   206,   262,     0,   316,     0,
     213,   204,     0,   212,   123,   214,     0,   203,   211,     0,
     216,   215
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   120,   803,   544,   182,   275,   502,
     506,   276,   503,   507,   122,   123,   124,   125,   126,   127,
     323,   579,   580,   456,   240,  1435,   462,  1357,  1436,  1664,
     763,   270,   497,  1627,   997,  1178,  1679,   339,   183,   581,
     850,  1056,  1231,   131,   547,   867,   582,   601,   869,   528,
     866,   583,   548,   868,   341,   291,   307,   134,   852,   806,
     789,  1012,  1380,  1106,   917,  1576,  1439,   705,   923,   461,
     714,   925,  1264,   697,   906,   909,  1095,  1684,  1685,   571,
     572,   595,   596,   280,   281,   285,  1406,  1555,  1556,  1185,
    1305,  1399,  1551,  1670,  1687,  1588,  1631,  1632,  1633,  1387,
    1388,  1389,  1589,  1595,  1640,  1392,  1393,  1397,  1544,  1545,
    1546,  1566,  1714,  1306,  1307,   184,   136,  1700,  1701,  1549,
    1309,   137,   233,   457,   458,   138,   139,   140,   141,   142,
     143,   144,   145,  1419,   146,   849,  1055,   147,   237,   569,
     318,   570,   452,   553,   554,  1129,   555,  1130,   148,   149,
     150,   151,   152,   740,   741,   742,   153,   154,   267,   155,
     268,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     753,   754,   989,   494,   495,   496,   760,  1616,   156,   549,
    1408,   550,  1026,   811,  1202,  1199,  1537,  1538,   157,   158,
     159,   227,   234,   326,   442,   160,   944,   746,   161,   945,
     841,   832,   946,   893,  1077,  1079,  1080,  1081,   895,  1243,
    1244,   896,   678,   427,   195,   196,   584,   574,   408,   409,
     838,   163,   228,   186,   165,   166,   167,   168,   169,   170,
     171,   632,   172,   230,   231,   531,   219,   220,   635,   636,
    1142,  1143,   563,   560,   564,   561,  1135,  1132,  1136,  1133,
     300,   301,   797,   173,   521,   174,   568,   175,  1557,   292,
     334,   590,   591,   938,  1038,   787,   718,   719,   720,   261,
     262,   834
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1452
static const yytype_int16 yypact[] =
{
   -1452,   146, -1452, -1452,  6213, 13625, 13625,   -74, 13625, 13625,
   13625, 11733, 13625, -1452, 13625, 13625, 13625, 13625, 13625, 13625,
   13625, 13625, 13625, 13625, 13625, 13625, 14746, 14746, 11939, 13625,
   14896,   -70,   -42, -1452, -1452, -1452, -1452, -1452,   143, -1452,
     113, 13625, -1452,   -42,   -26,   -11,   126,   -42, 12103, 16252,
   12267, -1452, 14686, 10745,   151, 13625, 16030,     7, -1452, -1452,
   -1452,   105,   194,    63,   163,   169,   171,   181, -1452, 16252,
     210,   213, -1452, -1452, -1452, -1452, -1452,   310,  3733, -1452,
   -1452, 16252, -1452, -1452, -1452, -1452, 16252, -1452, 16252, -1452,
     266,   222,   230,   243,   254, 16252, 16252, -1452,    31, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, 13625, -1452, -1452,   255,   344,   392,   392,
   -1452,   185,   308,   364, -1452,   262, -1452,    44, -1452,   440,
   -1452, -1452, -1452, -1452, 16105,    24, -1452, -1452,   276,   282,
     286,   306,   309,   312,  2992, -1452, -1452, -1452, -1452,   450,
   -1452, -1452, -1452,   452,   470,   338, -1452,    99,   339,   396,
   -1452, -1452,   632,     5,  1839,   100,   345,   110,   114,   348,
      11, -1452,    26, -1452,   482, -1452, -1452, -1452,   402,   351,
     407, -1452, -1452,   440,    24, 16508,  2234, 16508, 13625, 16508,
   16508, 10934,   358, 15443, 10934,   517, 16252,   500,   500,    95,
     500,   500,   500,   500,   500,   500,   500,   500,   500, -1452,
   -1452,  2165,   403, -1452,   426,   448,   448, 14746, 15487,   379,
     577, -1452,   402,  2165,   403,   449,   454,   388,   116, -1452,
     468,   100, 12431, -1452, -1452, 13625,  9509,   591,    46, 16508,
   10539, -1452, 13625, 13625, 16252, -1452, -1452,  4867,   401, -1452,
    4940, 14686, 14686,   436, -1452, -1452,   415, 14566,   592, -1452,
     601, -1452, 16252,   537, -1452,   419,  5234,   434,   475, -1452,
      36,  5278, 14919, 16118, 16252,    52, -1452,    27, -1452,  3949,
      60, -1452, -1452, -1452,   603,    66, 14746, 14746, 13625,   442,
     476, -1452, -1452, 14295, 11939,    79,   260, -1452, 13789, 14746,
     359, -1452, 16252, -1452,   191,   308, -1452, -1452, -1452, -1452,
   15211, 13625, 13625, 13625,   637,   550, -1452, -1452,    29,   453,
   16508,   456,   608,  6419, 13625,   232,   455,   438,   232,   285,
     265, -1452, 16252, 14686,   463, 10951, 14686, -1452, -1452, 15233,
   -1452, -1452, -1452, -1452, -1452,   440, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, 13625, 13625, 13625, 12637, 13625, 13625, 13625,
   13625, 13625, 13625, 13625, 13625, 13625, 13625, 13625, 13625, 13625,
   13625, 13625, 13625, 13625, 13625, 13625, 13625, 13625, 14896, 13625,
   -1452, 13625, 13625, -1452, 13625,  2413, 16252, 16252, 16252, 16105,
     559,   350,  5572, 13625, 13625, 13625, 13625, 13625, 13625, 13625,
   13625, 13625, 13625, 13625, 13625, -1452, -1452,  1691, -1452,   118,
   13625, 13625, -1452, 10951, 13625, 13625,   255,   120, 14295,   467,
     440, 12843,  5347, -1452, 13625, -1452,   480,   645,  2165,   487,
      -7,  3223,   448, 13049, -1452, 13255, -1452,   488,    -1, -1452,
      45, 10951, -1452,  4290, -1452,   124, -1452, -1452,  5416, -1452,
   -1452, 11157, -1452, 13625, -1452,   602,  9715,   680,   493, 16420,
     681,    78,    33, -1452, -1452, -1452, -1452, -1452, 14686, 14769,
     497,   688, 14986, -1452,   514, -1452, -1452, -1452,   624, 13625,
     625,   629, 13625, 13625, 13625, -1452,   475, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452,   523, -1452, -1452, -1452,   513, -1452,
   -1452, 16252,   512,   705,    57, 16252,   522,   706,    65,   187,
   16168, -1452, 16252, 13625,   448,     7, -1452, 14986,   647, -1452,
     448,    81,    84,   527,   528,  1245,   529, 16252,   604,   533,
     448,    89,   534, 16049, 16252, -1452, -1452,   667,  1889,   -18,
   -1452, -1452, -1452,   308, -1452, -1452, -1452,   707,   612,   573,
     246, -1452,   255,   613,   733,   543,   606,   120, -1452, 16508,
     551,   741, 15543,   558,   750,   560, 14686, 14686,   747,   591,
      29,   565,   757, -1452, 14686,    40,   703,    98, -1452, -1452,
   -1452, -1452, -1452, -1452,   524,  2686, -1452, -1452, -1452, -1452,
     761,   611, -1452, 14746, 13625,   571,   764, 16508,   763, -1452,
   -1452,   656, 15253, 11346, 12826, 10934, 13625, 16464,  2480, 14641,
    3120,  3459,  1665, 12615, 12615, 12615, 12615,  2059,  2059,  2059,
    2059,   671,   671,   503,   503,   503,    95,    95,    95, -1452,
     500, 16508,   578,   579, 15587,   580,   777, -1452, 13625,   -50,
     589,   120, -1452, -1452, -1452, -1452,   440, 13625,  3635, -1452,
   -1452, 10934, -1452, 10934, 10934, 10934, 10934, 10934, 10934, 10934,
   10934, 10934, 10934, 10934, 10934, 13625,   -50,   590,   585,  2892,
     594,   597,  3086,    90,   596, -1452, 16508, 15080, -1452, 16252,
   -1452,   453,    40,   403, 14746, 16508, 14746, 15643,    42,   125,
   -1452,   605, 13625, -1452, -1452, -1452,  9303,    83, -1452, -1452,
   16508, 16508,   -42, -1452, -1452, -1452, 13625,   700,  1878, 14986,
   16252,  9921,   609,   617, -1452,    51,   669,   657, -1452,   797,
     621,  4171, 14686, 14986, 14986, 14986, 14986, 14986, -1452,   623,
      28,   676,   627,   631,   633,   634, 14986,    15, -1452,   677,
   -1452, -1452, -1452,   642, -1452, 16594, -1452, 13625,   649, 16508,
     650,   831, 14225,   839, -1452, 16508, 14181, -1452,   523,   771,
   -1452,  6625, 15966,   651,   190, -1452, 14919, 16252,   207, -1452,
   16118, 16252, 16252, -1452, -1452,  3229, -1452, 16594,   837, 14746,
     653, -1452, -1452, -1452, -1452, -1452,   759,    55, 15966,   654,
   14295, 15128,   846, -1452, -1452, -1452, -1452,   655, -1452, 13625,
   -1452, -1452,  5801, -1452, 14686, 15966,   660, -1452, -1452, -1452,
   -1452,   849, 13625, 15211, -1452, -1452, 16191, -1452, 13625, -1452,
   13625, -1452, 13625, -1452, -1452,   664, -1452, 14686, -1452,   672,
     838,    21, -1452, -1452,    53,  5339, -1452,   129, -1452, -1452,
   14686, -1452, -1452,   448, 16508, -1452, 11363, -1452, 14986,    22,
     674, 15966,   612, -1452, -1452, 13444, 13625, -1452, -1452, 13625,
   -1452, 13625, -1452,  3428,   675, 10951,   604,   841,   612,   656,
   16252, 14896,   448,  3538,   678, 10951, -1452, -1452,   131, -1452,
   -1452,   861, 15276, 15276, 15080, -1452, -1452, -1452, -1452,   682,
      30,   686, -1452, -1452, -1452,   867,   687,   480,   448,   448,
   13461, -1452,   132, -1452, -1452,  3726,   274,   -42, 10539, -1452,
    6831,   685,  7037,   690,  1878, 14746,   689,   765,   448, 16594,
     875, -1452, -1452, -1452, -1452,   381, -1452,    16, 14686, -1452,
   14686, 16252, 14769, -1452, -1452, -1452,   885, -1452,   695,   761,
     386,   386,   832,   832, 15787,   692,   887, 14986,   755, 16252,
   15211, 14986, 14986, 14986,  2162, 16210, 14986, 14986, 14986, 14986,
   14836, 14986, 14986, 14986, 14986, 14986, 14986, 14986, 14986, 14986,
   14986, 14986, 14986, 14986, 14986, 14986, 14986, 14986, 14986, 14986,
   14986, 14986, 14986, 16508, 13625, 13625, 13625, -1452, -1452, -1452,
   13625, 13625, -1452,   475, -1452,   821, -1452, -1452, 16252, -1452,
   -1452, 16252, -1452, -1452, -1452, -1452, 14986,   448, -1452, 14686,
   16252, -1452,   892, -1452, -1452,    92,   704,   448, 11569, -1452,
    1772, -1452,  6007,   550,   892, -1452,   197,   -32, 16508,   774,
   -1452, 16508, 16508, 15687, -1452,   709,   838, 14686,   591, 14686,
     834,   897,   835, 13625,   -50,   716, -1452, 14746, 13625, 16508,
   16594,   718,    22, -1452,   714,    22,   720, 13444, 16508, 15743,
     721, 10951,   724,   726, 14686,   727,   612, -1452,   388,   731,
   10951,   732, 13625, -1452, -1452, -1452, -1452, -1452, -1452,   798,
     725,   924, 15080,   790, -1452, 15211, 15080, -1452, -1452, -1452,
   14746, 16508, -1452,   -42,   906,   864, 10539, -1452, -1452, -1452,
     738, 13625,   765,   448, 14295,  1878,   742, 14986,  7243,   427,
     740, 13625,    32,    34, -1452,   775, -1452,   815, -1452, 14616,
     916,   748, 14986, -1452, 14986, -1452,   749, -1452,   819,   944,
     754, 16594,   766,   955, 15843,   767,   961,   773, -1452, -1452,
   -1452, 15885,   768,   966, 11922, 13032, 13238, 14986, 16552,  5523,
   15153,  3681,  4971, 14426, 16624, 16624, 16624, 16624,  2368,  2368,
    2368,  2368,   792,   792,   386,   386,   386,   832,   832,   832,
     832, 16508, 14357, 16508, -1452, 16508, -1452,   781, -1452, -1452,
   -1452, 16594, -1452,   881, 15966,   464, -1452, 14295, -1452, -1452,
   10934,   782, -1452,   784,   495, -1452,    77, 13625, -1452, -1452,
   -1452, 13625, -1452, 13625, 13625, -1452,   591, -1452, -1452,   108,
     970, 14986, -1452,  4148,   788, 10951,   448, 16508,   838,   789,
   -1452,   791,    22, 13625, 10951,   793, -1452, -1452,   550, -1452,
     786,   796, -1452, 10951,   801, -1452, 15080, -1452, 15080, -1452,
     803, -1452,   862,   804,   983, -1452,   448,   975, -1452,   795,
   -1452, -1452,   807,   808,    93, -1452, -1452, 16594,   809,   810,
   -1452,  4634, -1452, -1452, -1452, -1452, -1452, 14686, -1452, 14686,
   -1452, 16594, 15941, -1452, 14986, 15211, -1452, -1452, -1452, 14986,
   -1452, 14986, -1452, 14986, -1452, -1452, 14986, -1452, 14986, -1452,
   13993, 14986, 13625,   812,  7449, 14686, -1452,   240, 14686, 15966,
   -1452, 15985,   853, 14425, -1452, -1452, -1452,   559, 14474,    67,
     350,    94, -1452, -1452, -1452,   860,  4222,  4557, 16508, 16508,
   -1452,   937,  1003,   940, 14986, 16594,   822, 10951,   820,   912,
     838,   845,   838,   823, 16508,   824, -1452,  1039,   827,  1072,
   -1452,    22,   825, -1452, -1452,   903, -1452, 15080, -1452, 15211,
   -1452, -1452,  9303, -1452, -1452, -1452, -1452, 10127, -1452, -1452,
   -1452,  9303, -1452,   833, 14986, 16594,   909, 16594, 16594, 15983,
   16594, 16039, 13993, 14313, -1452, -1452, -1452, 15966, 15966,  1024,
      64, -1452, -1452, -1452, -1452,    69,   842,    70, -1452, 13995,
   -1452, -1452,    71, -1452, -1452, 15150, -1452,   840, -1452,   967,
     440, -1452, 14686, -1452,   559, -1452,  4968, -1452, -1452, -1452,
   -1452,  1031, 14986, -1452, 16594, 10951,   847, -1452,   850,   848,
     214, -1452,   912,   838, -1452, -1452, -1452, -1452,  1201,   851,
   -1452, 15080, -1452,   925,  9303, 10333, 10127, -1452, -1452, -1452,
    9303, -1452, 16594, 14986, 14986, 14986, 13625,  7655,   854,   855,
   14986, 15966, -1452, -1452,   998, 15985, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452,   387, -1452,   853,
   -1452, -1452, -1452, -1452, -1452,    80,   311, -1452,  1038,    72,
   16252,   967,  1040,   440, -1452, -1452,   856,  1045, 14986, 16594,
     863, -1452,   117, -1452, -1452, -1452, -1452,   858,   214, 14382,
   -1452,   838, -1452, 15080, -1452, -1452, -1452, -1452,  7861, 16594,
   16594, 16594, 14269, -1452, -1452, -1452, 16594, -1452,  2678,    38,
   -1452, -1452, 14986, 13995, 13995,  1006, -1452, 15150, 15150,   373,
   -1452, -1452, -1452, 14986,   985, -1452,   866,    74, 14986, 16252,
   -1452, 14986, 16594, -1452,   987, -1452,  1053,  8067,  8273, -1452,
   -1452, -1452,   214, -1452,  8479,   869,   988,   960, -1452,   979,
     930, -1452, -1452,   989,   998, -1452, 16594, -1452, -1452,   920,
   -1452,  1048, -1452, -1452, -1452, -1452, 16594,  1073, -1452, -1452,
   16594,   889, 16594, -1452,   316,   886, -1452, -1452,  8685, -1452,
     888, -1452, -1452,   894,   926, 16252,   350, -1452, -1452, 14986,
      23, -1452,  1015, -1452, -1452, -1452, -1452, 15966,   651, -1452,
     933, 16252,   259, 16594,   898,  1088,   336,    23, -1452,  1020,
   -1452, 15966,   899, -1452,   838,    25, -1452, -1452, -1452, -1452,
   14686, -1452,   901,   902,    76, -1452,   244,   336,   115,   838,
     904, -1452, -1452, -1452, -1452, 14686,  1026,  1089,  1028,   244,
   -1452,  8891,   121,  1094, 14986, -1452, -1452,  9097, -1452,  1030,
    1097,  1033, 14986, 16594, -1452,  1099, 14986, -1452, 16594, 14986,
   16594, 16594
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1452, -1452, -1452,  -504, -1452, -1452, -1452,    -4, -1452, -1452,
   -1452,   610,   352,   346,    41,  1183,  3814, -1452,  2244, -1452,
    -439, -1452,    14, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452,  -564, -1452, -1452,  -177,   103,    12,
   -1452, -1452, -1452, -1452, -1452, -1452,    17, -1452, -1452, -1452,
   -1452,    18, -1452, -1452,   730,   736,   735,  -126,   258,  -811,
     270,   323,  -554,    37,  -810, -1452,  -298, -1452, -1452, -1452,
   -1452,  -664,  -121, -1452, -1452, -1452, -1452,  -544, -1452,  -540,
   -1452,  -387, -1452, -1452,   630, -1452,  -280, -1452, -1452,  -981,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,
    -295, -1452, -1452, -1452, -1452, -1452,  -376, -1452,  -138, -1395,
   -1452, -1451,  -553, -1452,  -160,    13,  -134,  -539, -1452,  -384,
   -1452,   -76,   -24,  1128,  -668,  -363, -1452, -1452,   -44, -1452,
   -1452,  3643,   -54,  -252, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452,  -536,  -761, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1452,   779, -1452,
   -1452,   182, -1452,   693, -1452, -1452, -1452, -1452, -1452, -1452,
   -1452,   193, -1452,   711, -1452, -1452,   423, -1452,   165, -1452,
   -1452, -1452, -1452, -1452, -1452, -1452, -1452, -1096, -1452,  1698,
    3025,  -343, -1452, -1452,   127,  3440,  4230, -1452, -1452,   245,
    -164,  -589, -1452, -1452,   317,  -666,   119, -1452, -1452, -1452,
   -1452, -1452,   302, -1452, -1452, -1452,   107,  -803,  -188,  -187,
    -131, -1452, -1452,    68, -1452, -1452, -1452, -1452,     1,  -133,
   -1452,  -251, -1452, -1452, -1452,  -394,   908, -1452, -1452, -1452,
   -1452, -1452,   890, -1452, -1452, -1452,   251, -1452, -1452, -1452,
     416,   264, -1452, -1452,   919,  -307,  -977, -1452,   -48,   -83,
    -192,  -201,   491, -1452, -1009, -1452,   283, -1452, -1452, -1452,
    -237, -1014
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -887
static const yytype_int16 yytable[] =
{
     121,   346,   390,   558,   260,   308,   419,   847,   236,   265,
     696,   894,   314,   315,   466,   467,   130,   135,   128,   241,
     471,   132,   133,   245,   673,  1209,   670,  1206,  1039,   649,
     829,   229,   439,   828,   412,   629,   389,   319,   913,   417,
     802,  1054,   444,   445,  1194,   248,  1193,  1634,   258,   927,
     346,   342,  1029,   336,   691,   453,   436,  1065,   761,   321,
     928,   510,  1262,   712,  1010,   290,    13,    13,    13,   515,
      13,  -768,   164,  1451,    13,   518,  1402,   414,  -281,  1455,
    1539,  1604,   306,  1604,   290,  1451,   278,   710,  1040,  1597,
     779,   290,   290,   779,   215,   216,   592,   277,   791,   791,
     446,   791,   791,   791,  1102,   209,   410,   129,   410,   407,
     316,   162,   948,  1598,  1083,  1111,  1112,  1620,   333,   304,
     407,  1200,   305,   512,   209,   188,   407,  1315,    51,   232,
     290,   633,  1041,  1111,  1112,   429,    58,    59,    60,   176,
     177,   343,   284,  1321,   498,   533,     3,   437,   378,  -886,
    1716,  -440,  1614,   767,   907,   908,  1729,   235,  -617,   668,
     379,   771,  -886,   671,  -772,  -765,   410,   835,   602,   443,
    1201,  1659,   297,   242,  1084,   391,  -766,   557,   332,  -476,
    -767,   322,  -803,   333,  -769,   801,   414,  1322,   243,  1128,
    -804,  -806,   426,   577,  1717,  -770,  1615,  -771,  -805,   689,
    1730,   499,  1641,  1642,  -774,   344,   534,  -768,   332,  1329,
    -616,   279,   523,   415,  1042,   331,   332,  -886,  -699,  1114,
     929,  -699,  -222,  -208,  1011,  -699,  -222,   317,   282,   309,
     416,   715,   121,  1263,   524,   713,   121,  1265,   345,  1331,
     460,  1635,   411,   674,   411,   337,  1337,   454,  1339,   116,
     450,  1338,   641,   511,   455,  1230,   600,   346,   473,   910,
    1109,   516,  1113,   680,   912,  1452,  1453,   519,  1403,  1323,
    -281,  1456,  1540,  1605,   641,  1649,  1718,  1711,   711,   430,
    1599,   780,  1731,   772,   781,   432,   998,   420,   598,   792,
     881,   438,  1186,  1356,  1405,  1255,   808,  -775,   641,  1022,
    -772,  -765,   411,  1001,   164,   308,   342,   641,   164,  1051,
     641,   295,  -766,   504,   508,   509,  -767,   283,  -803,   121,
    -769,  1420,   415,  1422,  1242,   244,  -804,  -806,   589,   825,
     826,  -770,   258,  -771,  -805,   290,   135,   833,   332,   295,
    1600,   332,   332,   543,   295,  1093,  1094,   514,   650,   540,
    1195,  1672,   681,   269,   520,   520,   525,  1601,   332,  1428,
    1602,   530,   286,  1196,   295,  1377,  1378,   539,   287,   540,
     288,    58,    59,    60,   176,   177,   343,   298,   299,   229,
     289,   639,   290,   643,   290,   290,   640,   836,   837,   295,
     814,   164,   545,   546,   296,  1673,  1015,   819,   809,  1197,
     823,  1592,  1643,   666,   535,   298,   299,   699,   667,   293,
     298,   299,   294,   810,  1568,  1564,  1565,  1593,   309,  1644,
    1245,   310,  1645,   295,   390,   573,   129,   683,   325,   311,
     298,   299,   640,  1252,  1594,   979,   980,   981,   295,   693,
     344,   690,   312,   540,   694,  1712,  1713,   864,  1697,  1698,
    1699,   982,   121,   313,   297,   298,   299,   324,   389,   332,
    1693,   335,    58,    59,    60,   176,   177,   343,  1297,   870,
     704,   295,   862,   588,   874,   338,   328,   347,  1062,  1110,
    1111,  1112,  1108,   348,   592,   592,   530,   349,  1071,   298,
     299,   864,   587,   646,   836,   837,   430,  1637,  1638,  1297,
     901,   902,  1208,   541,   298,   299,   558,   350,   774,    13,
     351,  -886,  1219,   352,  1366,  1221,  -474,   295,   381,   854,
     936,   939,   540,   786,   164,  1259,  1111,  1112,  1068,   796,
     798,   344,   333,   327,   329,   330,   382,   298,   299,   383,
      13,   384,   764,   385,   413,   439,   768,  -773,  -475,  -616,
     418,   277,   375,   376,   377,   933,   378,   903,   302,   423,
     536,   425,  1622,  -886,   542,   379,  -886,  1023,   379,  1298,
    1344,   333,  1345,   431,  1299,   407,    58,    59,    60,   176,
     177,   343,  1300,   298,   299,   434,   435,   441,  1433,   536,
    1035,   542,   536,   542,   542,   443,  -615,   391,   290,   451,
    1298,   440,   464,  1046,   468,  1299,  -881,    58,    59,    60,
     176,   177,   343,  1300,   469,   472,   474,   517,  1301,  1302,
     475,  1303,   421,   393,   394,   395,   396,   397,   398,   399,
     400,   401,   402,   403,   404,   477,    58,    59,    60,    61,
      62,   343,   526,   558,   527,   344,   567,    68,   386,  1301,
    1302,   566,  1303,   575,   679,  1188,   576,   478,   479,   480,
     586,   843,   -65,    51,   481,   482,   599,  1304,   483,   484,
    1320,   405,   406,   892,  1225,   897,   344,   573,   911,   677,
     557,  1432,  1333,  1234,   388,  1706,  1708,   682,   688,   453,
     702,  1115,   121,  1116,   706,   709,   721,   722,  1314,   747,
    1719,  1722,   641,   748,   750,   344,   920,   121,   751,   135,
    1254,   759,   762,   765,   766,   770,   872,   372,   373,   374,
     375,   376,   377,   769,   378,   922,   778,   782,   783,   788,
     785,  1087,   790,   799,   793,   407,   379,   804,   805,  1294,
     807,   812,   813,   815,    58,    59,    60,    61,    62,   343,
     818,   817,   898,   816,   899,    68,   386,   121,   821,   822,
     824,   827,   830,  1000,   164,  1572,   831,  1003,  1004,  -477,
     840,   845,  1182,   846,   135,  1121,   918,   848,   558,   164,
     842,   851,  1125,   860,   857,   858,   861,  1067,   865,   875,
     387,   876,   388,  1311,   878,   853,   914,   930,   121,   129,
    1207,  1429,   833,   879,   931,   904,   932,   504,   577,  1352,
     924,   508,  1030,   344,   130,   135,   128,   557,   926,   132,
     133,   934,   947,   949,   955,  1361,   950,  1228,  1328,   164,
     951,  1044,   952,   953,   984,   985,  1045,  1335,   976,   977,
     978,   979,   980,   981,   956,   986,  1342,  1007,   990,  1297,
     993,  1006,  1214,   996,  1008,  1009,  1014,   982,   530,  1017,
    1018,  1019,  1025,  1027,   129,  1034,   290,  1686,  1037,  1036,
     164,  1064,   229,  1052,  1061,  1072,  1086,  1070,  1076,  1076,
     892,  1082,  1046,  1096,  1686,  1085,  1099,  1088,  1104,  1107,
      13,  1101,  1707,  1105,  1119,  1120,  1124,   982,  1123,   535,
    1177,  1184,  1203,  1187,   121,   129,   121,  1623,   121,   162,
    1205,  1211,  1434,  1210,  1212,  1215,  1220,  1239,  1218,  1222,
    1224,  1440,  1097,   135,  1226,   135,  1236,  1117,  1227,  1229,
    1233,  1237,  1235,  1238,  1241,  1447,  1248,  1249,   573,  1251,
    1416,  1260,  1256,  1267,  1266,  1127,  1269,  1274,  1270,  1273,
    1298,  1140,   557,  1275,  1277,  1299,   573,    58,    59,    60,
     176,   177,   343,  1300,  1279,  1276,  1278,  1282,   558,  1280,
    1283,  1287,  1284,  1285,  1189,  1288,   164,  1295,   164,  1289,
     164,  1293,   918,  1103,  1324,  1312,  1313,  1327,  1340,  1330,
    1347,  1332,  1349,  1336,  1179,  1341,  1353,  1180,  1578,  1301,
    1302,  1343,  1303,  1346,  1348,  1351,  1183,  1354,  1355,  1391,
    1358,  1359,  1407,   129,  1374,   129,  1411,  1412,   121,  1413,
    1417,  1415,  1418,  1423,  1424,  1430,   344,  1655,  1560,  1426,
    1362,  1431,  1363,  1441,   130,   135,   128,  1443,  1450,   132,
     133,  1547,   558,  1297,  1454,  1558,  1548,  1561,  1421,  1562,
    1563,  1571,  1603,  1573,  1608,  1584,  1585,  1610,  1376,  1611,
    1619,  1639,  1654,  1613,  1647,  1648,  1653,  1662,  1663,  1247,
    1661,  1401,    33,    34,    35,  -277,  1297,  1665,   892,  1668,
    1350,  1598,   892,   728,    13,  1666,  1674,  1669,  1671,  1676,
     164,  1678,   121,  1677,  1688,  1691,  1696,  1695,  1694,  1703,
    1705,  1709,  1710,  1724,   121,  1723,  1720,  1725,  1732,  1735,
    1250,  1736,  1737,  1739,  1690,  1216,  1002,    13,   999,   645,
     773,   135,   642,   644,  1617,   129,  1618,  1066,  1024,   162,
      72,    73,    74,    75,    76,  1624,  1063,  1704,  1577,  1253,
    1360,   730,   557,  1702,  1298,   776,  1569,    79,    80,  1299,
    1404,    58,    59,    60,   176,   177,   343,  1300,  1246,   573,
    1591,    89,   573,  1596,   164,  1398,  1726,  1607,  1715,   238,
    1567,   652,   530,   918,   346,  1176,   164,  1298,    97,   757,
    1658,   992,  1299,  1174,    58,    59,    60,   176,   177,   343,
    1300,  1198,  1126,  1301,  1302,  1232,  1303,   758,  1308,  1089,
    1078,  1240,   532,   565,  1137,  1297,   522,  1308,     0,   212,
     212,   129,   937,   224,     0,  1118,   557,     0,     0,     0,
     344,     0,     0,  1550,     0,     0,  1301,  1302,     0,  1303,
       0,     0,   892,     0,   892,   224,     0,     0,     0,     0,
       0,     0,  1425,     0,     0,     0,    13,     0,     0,     0,
    1379,     0,     0,   344,     0,   530,     0,     0,     0,   421,
     393,   394,   395,   396,   397,   398,   399,   400,   401,   402,
     403,   404,     0,     0,  1721,  1427,     0,     0,     0,     0,
       0,  1727,     0,     0,     0,     0,     0,     0,     0,     0,
     121,     0,  1310,     0,   258,     0,     0,     0,     0,  1396,
       0,  1310,     0,     0,     0,     0,  1298,   135,   405,   406,
       0,  1299,     0,    58,    59,    60,   176,   177,   343,  1300,
       0,     0,     0,   391,     0,     0,     0,     0,     0,   573,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   892,  1308,     0,     0,     0,   121,     0,
    1308,     0,  1308,   121,  1552,  1301,  1302,   121,  1303,     0,
       0,     0,   164,     0,     0,   135,     0,     0,     0,     0,
       0,  1438,   407,     0,   135,     0,  1609,     0,     0,     0,
       0,     0,   344,     0,     0,  1536,     0,     0,     0,     0,
       0,  1543,     0,     0,   212,     0,     0,   129,   258,     0,
     212,     0,   258,     0,  1570,     0,   212,     0,     0,     0,
    1400,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     164,     0,     0,     0,     0,   164,     0,   892,     0,   164,
     121,   121,   121,     0,   224,   224,   121,     0,  1310,     0,
     224,  1308,     0,   121,  1310,   784,  1310,   135,   573,  1575,
    1438,     0,     0,   135,     0,   129,     0,     0,     0,     0,
     135,     0,   212,   833,   129,     0,     0,  1606,     0,   212,
     212,     0,     0,     0,     0,     0,   212,     0,   833,     0,
       0,     0,   212,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   224,     0,     0,     0,     0,     0,     0,
       0,     0,   164,   164,   164,     0,  1681,  1553,   164,     0,
       0,     0,     0,     0,     0,   164,   224,     0,     0,   224,
       0,     0,     0,     0,     0,     0,  1651,     0,     0,     0,
       0,     0,     0,     0,     0,  1310,     0,   129,     0,     0,
       0,     0,     0,   129,     0,     0,   290,     0,   346,     0,
     129,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   224,     0,     0,     0,   258,     0,     0,     0,   892,
       0,     0,     0,     0,   121,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1629,     0,     0,     0,     0,  1536,
    1536,   135,     0,  1543,  1543,     0,     0,     0,     0,     0,
       0,   212,     0,     0,     0,   290,     0,     0,     0,     0,
       0,   212,     0,   121,   121,     0,     0,     0,     0,     0,
     121,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     135,   135,     0,     0,     0,     0,     0,   135,     0,     0,
       0,     0,     0,     0,     0,     0,   164,     0,     0,     0,
       0,   224,   224,     0,   121,   737,     0,     0,     0,     0,
       0,  1680,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   135,     0,     0,     0,     0,     0,  1692,     0,  1682,
       0,   129,     0,     0,     0,   164,   164,     0,     0,     0,
       0,     0,   164,     0,     0,     0,     0,     0,     0,     0,
     737,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   121,   378,     0,
     129,   129,     0,   121,   213,   213,   164,   129,   225,     0,
     379,     0,     0,     0,   135,     0,     0,     0,     0,     0,
     135,     0,     0,     0,     0,     0,     0,     0,     0,   224,
     224,     0,     0,     0,     0,     0,     0,   224,     0,     0,
       0,   129,     0,     0,     0,     0,     0,     0,    36,     0,
     209,     0,     0,     0,     0,     0,   212,   573,     0,     0,
       0,     0,   353,   354,   355,     0,     0,     0,     0,   164,
       0,     0,     0,     0,   573,   164,     0,     0,     0,     0,
       0,   356,   573,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   129,   378,     0,     0,     0,     0,
     129,   212,     0,     0,     0,     0,     0,   379,     0,     0,
       0,     0,     0,     0,     0,    83,    84,     0,    85,   181,
      87,     0,     0,   392,   393,   394,   395,   396,   397,   398,
     399,   400,   401,   402,   403,   404,     0,   212,     0,   212,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,     0,
       0,   212,   737,   665,     0,   116,     0,     0,     0,   353,
     354,   355,   405,   406,   224,   224,   737,   737,   737,   737,
     737,     0,     0,   915,     0,   213,     0,     0,   356,   737,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,     0,   378,     0,     0,   224,     0,     0,     0,     0,
       0,     0,     0,     0,   379,    36,     0,   209,     0,     0,
       0,     0,   212,     0,     0,     0,   407,     0,     0,     0,
       0,   224,     0,   212,   212,     0,     0,   213,  1191,     0,
       0,     0,     0,     0,   213,   213,     0,   224,   224,     0,
       0,   213,     0,     0,     0,   210,   224,   213,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   916,   213,     0,
     224,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   224,     0,     0,   180,     0,     0,    81,
      82,   737,    83,    84,   224,    85,   181,    87,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   224,     0,     0,     0,     0,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,     0,   225,   211,     0,     0,
       0,     0,   116,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   800,     0,     0,     0,     0,   212,   212,  -887,
    -887,  -887,  -887,   370,   371,   372,   373,   374,   375,   376,
     377,   224,   378,   224,     0,   224,   213,     0,     0,     0,
       0,     0,     0,     0,   379,     0,     0,     0,     0,     0,
     737,     0,     0,   224,   737,   737,   737,     0,     0,   737,
     737,   737,   737,   737,   737,   737,   737,   737,   737,   737,
     737,   737,   737,   737,   737,   737,   737,   737,   737,   737,
     737,   737,   737,   737,   737,   737,     0,     0,     0,     0,
     743,     0,   957,   958,   959,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   737,
       0,   960,   224,   961,   962,   963,   964,   965,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     978,   979,   980,   981,     0,   743,     0,     0,     0,     0,
     224,     0,   224,     0,     0,     0,     0,   982,     0,     0,
     212,     0,    29,    30,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,   209,     0,     0,   224,   421,   393,
     394,   395,   396,   397,   398,   399,   400,   401,   402,   403,
     404,     0,     0,     0,     0,     0,     0,     0,   224,     0,
       0,     0,     0,   212,     0,     0,     0,     0,     0,     0,
       0,     0,   210,     0,     0,     0,     0,   212,   212,     0,
     737,   213,     0,     0,     0,     0,   259,   405,   406,     0,
       0,     0,   224,     0,     0,   737,     0,   737,     0,     0,
       0,     0,     0,   180,     0,     0,    81,    82,     0,    83,
      84,     0,    85,   181,    87,     0,     0,     0,     0,     0,
     737,    90,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   213,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   407,  1138,     0,   428,     0,     0,   224,     0,   116,
     212,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   213,     0,   213,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   737,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   213,   743,  -887,  -887,
    -887,  -887,   974,   975,   976,   977,   978,   979,   980,   981,
       0,   743,   743,   743,   743,   743,     0,     0,     0,     0,
       0,     0,     0,   982,   743,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     224,     0,   224,     0,     0,     0,     0,   737,   224,     0,
     995,     0,   737,     0,   737,     0,   737,     0,     0,   737,
       0,   737,     0,     0,   737,     0,     0,   213,   224,     0,
       0,   224,   224,     0,   224,     0,  1013,     0,   213,   213,
      36,   224,   209,     0,     0,   259,   259,     0,     0,     0,
       0,   259,     0,  1013,     0,     0,     0,   737,     0,     0,
       0,   213,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   224,   378,     0,     0,   637,     0,     0,     0,
       0,     0,     0,     0,     0,   379,   743,   737,     0,  1053,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     224,   224,     0,     0,     0,     0,     0,    83,    84,   225,
      85,   181,    87,     0,     0,     0,     0,   259,     0,     0,
     259,     0,     0,     0,     0,   224,     0,     0,     0,   224,
       0,     0,     0,     0,     0,   737,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,     0,   213,   213,     0,   638,     0,   116,     0,     0,
       0,     0,     0,     0,     0,     0,   737,   737,   737,     0,
       0,     0,     0,   737,   224,     0,     0,     0,   224,     0,
       0,     0,     0,     0,     0,   743,     0,     0,   213,   743,
     743,   743,     0,     0,   743,   743,   743,   743,   743,   743,
     743,   743,   743,   743,   743,   743,   743,   743,   743,   743,
     743,   743,   743,   743,   743,   743,   743,   743,   743,   743,
     743,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   353,   354,   355,     0,
       0,     0,     0,     0,   743,     0,     0,     0,     0,     0,
       0,     0,   259,   717,     0,   356,   739,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,     0,   378,
       0,   737,     0,     0,     0,   213,     0,     0,     0,     0,
       0,   379,   224,     0,     0,    36,     0,     0,     0,     0,
       0,   739,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   224,     0,     0,     0,   737,     0,     0,     0,     0,
       0,     0,     0,   213,     0,     0,   737,     0,   213,     0,
       0,   737,     0,     0,   737,     0,     0,     0,     0,     0,
       0,     0,   213,   213,     0,   743,     0,     0,     0,     0,
     259,   259,     0,     0,     0,     0,     0,     0,   259,     0,
     743,     0,   743,     0,     0,     0,   180,     0,     0,    81,
       0,     0,    83,    84,     0,    85,   181,    87,     0,     0,
       0,     0,     0,     0,     0,   743,     0,     0,     0,     0,
       0,     0,   737,     0,     0,     0,     0,     0,     0,     0,
     224,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   224,     0,     0,     0,     0,     0,
       0,  1628,  1296,   224,     0,   213,     0,     0,     0,   839,
       0,     0,     0,     0,     0,     0,     0,     0,   224,     0,
       0,     0,   353,   354,   355,     0,     0,   737,     0,   743,
       0,     0,     0,     0,     0,   737,     0,     0,     0,   737,
       0,   356,   737,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,     0,   378,     0,     0,     0,     0,
       0,     0,     0,   739,     0,     0,     0,   379,     0,     0,
       0,     0,     0,     0,     0,   259,   259,   739,   739,   739,
     739,   739,   743,   213,     0,     0,     0,   743,     0,   743,
     739,   743,     0,     0,   743,     0,   743,     0,     0,   743,
       0,     0,     0,     0,     0,     0,     0,  1381,     0,  1390,
       0,     0,   353,   354,   355,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   356,   743,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,     0,   378,     0,   213,   259,     0,
       0,   214,   214,     0,     0,   226,     0,   379,     0,     0,
       0,     0,   743,     0,     0,     0,     0,     0,     0,     0,
       0,   259,     0,     0,     0,  1448,  1449,     0,     0,     0,
       0,     0,     0,     0,   259,     0,     0,     0,     0,     0,
       0,     0,   739,     0,     0,   877,   353,   354,   355,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     743,     0,     0,     0,     0,   356,     0,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,     0,   378,
       0,   743,   743,   743,     0,     0,     0,     0,   743,  1587,
       0,   379,     0,  1390,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   259,   378,   259,     0,   717,     0,     0,     0,
       0,     0,     0,     0,     0,   379,     0,     0,     0,     0,
       0,   739,     0,   380,     0,   739,   739,   739,     0,     0,
     739,   739,   739,   739,   739,   739,   739,   739,   739,   739,
     739,   739,   739,   739,   739,   739,   739,   739,   739,   739,
     739,   739,   739,   739,   739,   739,   739,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   214,     0,     0,   353,
     354,   355,   214,     0,     0,     0,     0,     0,   214,     0,
     739,     0,     0,   259,     0,     0,   743,     0,   356,     0,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   259,   378,   259,     0,     0,     0,     0,     0,   880,
     743,     0,     0,     0,   379,     0,     0,     0,     0,     0,
      36,   743,   209,     0,   214,     0,   743,     0,   259,   743,
       0,   214,   214,     0,     0,     0,     0,     0,   214,     0,
       0,     0,     0,     0,   214,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   556,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   739,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   259,     0,     0,   739,   743,   739,     0,
       0,     0,     0,     0,     0,  1689,     0,    83,    84,     0,
      85,   181,    87,     0,     0,     0,     0,     0,     0,  1381,
       0,   739,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   226,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,     0,   743,     0,     0,   638,     0,   116,     0,     0,
     743,     0,  1005,     0,   743,     0,     0,   743,   353,   354,
     355,     0,     0,   214,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   214,     0,   739,     0,   356,     0,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
       0,   378,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   379,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   259,   378,   259,     0,     0,     0,     0,   739,     0,
       0,     0,     0,   739,   379,   739,     0,   739,     0,     0,
     739,     0,   739,     0,     0,   739,     0,     0,     0,   259,
       0,     0,   259,     0,     0,     0,     0,     0,   353,   354,
     355,     0,   259,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   356,   739,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
       0,   378,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   379,     0,     0,     0,     0,   739,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   214,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1060,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   259,     0,   185,   187,
     259,   189,   190,   191,   193,   194,   739,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,     0,
       0,   218,   221,   214,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   239,     0,     0,   739,   739,   739,
       0,   247,     0,   250,   739,     0,   266,     0,   271,     0,
       0,     0,     0,   871,     0,     0,     0,     0,     0,   214,
       0,   214,    36,     0,   209,   964,   965,   966,   967,   968,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   980,   981,   214,     0,     0,   353,   354,   355,     0,
       0,  1069,     0,     0,     0,     0,   982,     0,     0,     0,
       0,     0,   210,     0,     0,   356,   320,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,     0,   378,
       0,     0,     0,   180,     0,     0,    81,    82,     0,    83,
      84,   379,    85,   181,    87,     0,     0,     0,     0,     0,
       0,     0,   739,     0,   214,     0,     0,     0,     0,     0,
      36,     0,     0,   259,     0,   214,   214,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   422,  1630,     0,   211,     0,   739,     0,   556,   116,
       0,     0,     0,     0,     0,     0,     0,   739,     0,     0,
       0,     0,   739,     0,     0,   739,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   448,     0,     0,   448,     0,
       0,     0,     0,     0,   302,   239,   459,    83,    84,     0,
      85,   181,    87,     0,     0,     0,   226,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   744,   739,     0,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,  1092,
       0,   320,     0,     0,     0,   303,     0,   218,     0,   214,
     214,   538,     0,     0,   259,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   559,   562,   562,   744,     0,   259,
       0,     0,     0,     0,     0,     0,     0,   585,   739,     0,
       0,     0,     0,     0,     0,   556,   739,     0,   597,     0,
     739,     0,     0,   739,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   603,   604,   605,   607,
     608,   609,   610,   611,   612,   613,   614,   615,   616,   617,
     618,   619,   620,   621,   622,   623,   624,   625,   626,   627,
     628,     0,   630,     0,   631,   631,    36,   634,   209,     0,
       0,     0,     0,     0,     0,   651,   653,   654,   655,   656,
     657,   658,   659,   660,   661,   662,   663,   664,     0,     0,
       0,     0,     0,   631,   669,     0,   597,   631,   672,     0,
       0,     0,     0,     0,   651,     0,   210,   676,     0,     0,
       0,     0,   214,     0,     0,     0,   685,     0,   687,     0,
       0,     0,     0,     0,   597,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   700,     0,   701,   180,     0,     0,
      81,    82,     0,    83,    84,     0,    85,   181,    87,     0,
     556,     0,     0,     0,     0,   214,     0,     0,     0,     0,
       0,     0,   749,     0,     0,   752,   755,   756,     0,   214,
     214,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,     0,   211,   744,
       0,   513,     0,   116,     0,     0,   775,     0,   353,   354,
     355,     0,     0,   744,   744,   744,   744,   744,     0,     0,
       0,     0,     0,     0,     0,     0,   744,   356,     0,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     251,   378,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   214,   379,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   252,     0,     0,     0,
       0,     0,   353,   354,   355,     0,     0,   844,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,   855,
       0,   356,     0,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,     0,   378,     0,     0,     0,     0,
       0,   863,     0,     0,     0,     0,   738,   379,   744,     0,
     193,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     556,   253,   254,     0,     0,     0,     0,     0,   873,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   180,
       0,     0,    81,   255,     0,    83,    84,     0,    85,   181,
      87,   738,   935,     0,     0,   905,     0,     0,     0,     0,
       0,     0,     0,   256,     0,     0,     0,     0,     0,   239,
       0,  1326,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,    36,     0,   209,
     257,     0,     0,     0,   556,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   744,     0,     0,
     983,   744,   744,   744,     0,     0,   744,   744,   744,   744,
     744,   744,   744,   744,   744,   744,   744,   744,   744,   744,
     744,   744,   744,   744,   744,   744,   744,   744,   744,   744,
     744,   744,   744,     0,     0,  1409,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1020,     0,    83,    84,   744,    85,   181,    87,
       0,     0,     0,     0,     0,  1028,     0,     0,     0,     0,
       0,  1031,     0,  1032,     0,  1033,     0,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,     0,  1049,
       0,     0,   692,     0,   116,     0,     0,     0,     0,  1057,
       0,     0,  1058,     0,  1059,     0,     0,     0,   597,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   597,     0,
       0,     0,     0,   738,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   738,   738,   738,
     738,   738,     0,  1091,     0,     0,     0,   744,     0,     0,
     738,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   744,     0,   744,     0,     0,   353,   354,   355,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   356,   744,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,     0,
     378,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   379,     0,     0,     0,     0,  1171,  1172,  1173,
       0,     0,     0,   752,  1175,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   353,   354,   355,     0,     0,     0,
       0,   744,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1190,   738,   356,  1262,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,  1213,   378,     0,     0,
       0,  1217,     0,     0,     0,     0,     0,     0,     0,   379,
       0,     0,   745,     0,   597,     0,     0,     0,     0,     0,
       0,     0,     0,   597,   744,  1190,     0,     0,     0,   744,
       0,   744,     0,   744,     0,     0,   744,     0,   744,     0,
       0,   744,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   239,     0,     0,   777,     0,     0,
       0,     0,     0,     0,  1261,     0,     0,     0,     0,     0,
    1410,   738,     0,     0,   744,   738,   738,   738,     0,     0,
     738,   738,   738,   738,   738,   738,   738,   738,   738,   738,
     738,   738,   738,   738,   738,   738,   738,   738,   738,   738,
     738,   738,   738,   738,   738,   738,   738,     0,     0,     0,
       0,     0,     0,     0,   744,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     738,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1263,     0,     0,     0,     0,
    1316,     0,     0,     0,  1317,     0,  1318,  1319,     0,     0,
       0,     0,   744,     0,     0,     0,     0,     0,   597,     0,
       0,     0,     0,     0,     0,     0,  1334,   597,     0,     0,
       0,     0,     0,     0,     0,     0,   597,   353,   354,   355,
       0,     0,     0,   744,   744,   744,     0,     0,     0,     0,
     744,     0,     0,     0,  1590,     0,   356,     0,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,     0,
     378,   738,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   379,     0,     0,  1373,   738,     0,   738,   919,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     353,   354,   355,   940,   941,   942,   943,     0,     0,     0,
       0,   738,     0,     0,     0,     0,   954,     0,     0,   356,
     597,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,     0,   378,     0,     0,     0,   251,   744,     0,
       0,     0,     0,     0,     0,   379,   965,   966,   967,   968,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   980,   981,   252,     0,   738,     0,     0,     0,     0,
       0,     0,   744,     0,     0,     0,   982,     0,     0,     0,
       0,     0,     0,   744,     0,    36,     0,     0,   744,     0,
       0,   744,     0,     0,     0,     0,     0,     0,   597,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   463,     0,
       0,     0,     0,     0,  1667,     0,     0,     0,  1050,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   738,  1582,
       0,     0,     0,   738,     0,   738,     0,   738,   253,   254,
     738,     0,   738,     0,     0,   738,     0,     0,     0,   744,
       0,     0,     0,     0,     0,     0,   180,     0,     0,    81,
     255,     0,    83,    84,     0,    85,   181,    87,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   738,     0,
     256,   465,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   744,     0,     0,   257,     0,     0,
       0,  1554,   744,     0,     0,     0,   744,     0,   738,   744,
       0,  1131,  1134,  1134,     0,     0,  1141,  1144,  1145,  1146,
    1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,
    1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   738,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1181,     0,     0,     0,
       0,     0,     0,     0,   353,   354,   355,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   738,   738,   738,
       0,     0,     0,   356,   738,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,     0,   378,   353,   354,
     355,     0,     0,     0,     0,     0,     0,     0,     0,   379,
       0,     0,     0,     0,     0,     0,     0,   356,     0,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
       0,   378,     0,     0,     0,     0,     0,  1257,     0,     0,
       0,     0,     0,   379,     0,     0,     0,     0,     0,     0,
       0,     0,  1271,     0,  1272,     0,     0,   353,   354,   355,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   738,     0,     0,     0,   356,  1290,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,     0,
     378,     0,     0,     0,     0,     0,   738,     0,     0,     0,
       0,     0,   379,     0,     0,     0,    36,   738,   209,     0,
       0,     0,   738,     0,     0,   738,   353,   354,   355,     0,
       0,     0,     0,     0,     0,   476,     0,     0,     0,     0,
       0,  1325,     0,     0,     0,   356,     0,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,     0,   378,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   500,
       0,   379,     0,   738,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    83,    84,     0,    85,   181,    87,     0,
       0,     0,     0,     0,  1365,     0,     0,     0,     0,  1367,
       0,  1368,     0,  1369,     0,     0,  1370,     0,  1371,     0,
       0,  1372,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,     0,   738,     0,
       0,  1043,     0,   116,     0,     0,   738,   675,     0,     0,
     738,     0,     0,   738,  1414,   962,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,   982,     0,
       0,     0,     0,     0,  1442,     0,     0,     0,     0,   647,
      12,     0,     0,     0,     0,     0,     0,   648,     0,     0,
       0,     0,     0,     0,     0,     0,   695,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,  1559,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,    40,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1579,  1580,  1581,    51,     0,     0,     0,
    1586,     0,     0,     0,    58,    59,    60,   176,   177,   178,
       0,     0,    65,    66,     0,     0,     0,     0,     0,     0,
       0,   179,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     180,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     181,    87,     0,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,    94,     0,     0,
       0,     0,    97,    98,   263,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     0,
       0,   113,     0,     0,     0,     0,   116,   117,     0,   118,
     119,     0,     0,     0,     0,     0,     0,     0,  1612,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,  1636,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,  1646,     0,     0,     0,     0,  1650,     0,
       0,  1652,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,     0,     0,     0,    40,    41,    42,    43,
       0,    44,     0,    45,     0,    46,     0,     0,    47,  1683,
       0,     0,    48,    49,    50,    51,    52,    53,    54,     0,
      55,    56,    57,    58,    59,    60,    61,    62,    63,     0,
      64,    65,    66,    67,    68,    69,     0,     0,     0,     0,
      70,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,    78,
      79,    80,    81,    82,  1733,    83,    84,     0,    85,    86,
      87,    88,  1738,     0,    89,     0,  1740,    90,     0,  1741,
       0,     0,     0,    91,    92,    93,    94,    95,     0,    96,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,  1021,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,     0,
       0,     0,    40,    41,    42,    43,     0,    44,     0,    45,
       0,    46,     0,     0,    47,     0,     0,     0,    48,    49,
      50,    51,    52,    53,    54,     0,    55,    56,    57,    58,
      59,    60,    61,    62,    63,     0,    64,    65,    66,    67,
      68,    69,     0,     0,     0,     0,    70,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,    78,    79,    80,    81,    82,
       0,    83,    84,     0,    85,    86,    87,    88,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,    94,    95,     0,    96,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
    1192,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,     0,     0,     0,    40,    41,
      42,    43,     0,    44,     0,    45,     0,    46,     0,     0,
      47,     0,     0,     0,    48,    49,    50,    51,    52,    53,
      54,     0,    55,    56,    57,    58,    59,    60,    61,    62,
      63,     0,    64,    65,    66,    67,    68,    69,     0,     0,
       0,     0,    70,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,    78,    79,    80,    81,    82,     0,    83,    84,     0,
      85,    86,    87,    88,     0,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,    92,    93,    94,    95,
       0,    96,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,     0,     0,     0,    40,    41,    42,    43,     0,    44,
       0,    45,     0,    46,     0,     0,    47,     0,     0,     0,
      48,    49,    50,    51,     0,    53,    54,     0,    55,     0,
      57,    58,    59,    60,    61,    62,    63,     0,    64,    65,
      66,     0,    68,    69,     0,     0,     0,     0,    70,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,   180,    79,    80,
      81,    82,     0,    83,    84,     0,    85,   181,    87,    88,
       0,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,    91,    92,    93,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,   578,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,     0,     0,     0,
      40,    41,    42,    43,     0,    44,     0,    45,     0,    46,
       0,     0,    47,     0,     0,     0,    48,    49,    50,    51,
       0,    53,    54,     0,    55,     0,    57,    58,    59,    60,
      61,    62,    63,     0,    64,    65,    66,     0,    68,    69,
       0,     0,     0,     0,    70,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   180,    79,    80,    81,    82,     0,    83,
      84,     0,    85,   181,    87,    88,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   114,   115,   994,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,     0,     0,     0,    40,    41,    42,    43,
       0,    44,     0,    45,     0,    46,     0,     0,    47,     0,
       0,     0,    48,    49,    50,    51,     0,    53,    54,     0,
      55,     0,    57,    58,    59,    60,    61,    62,    63,     0,
      64,    65,    66,     0,    68,    69,     0,     0,     0,     0,
      70,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   180,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   181,
      87,    88,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,  1098,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,     0,
       0,     0,    40,    41,    42,    43,  1100,    44,     0,    45,
       0,    46,     0,     0,    47,     0,     0,     0,    48,    49,
      50,    51,     0,    53,    54,     0,    55,     0,    57,    58,
      59,    60,    61,    62,    63,     0,    64,    65,    66,     0,
      68,    69,     0,     0,     0,     0,    70,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   180,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   181,    87,    88,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
       0,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,     0,     0,     0,    40,    41,
      42,    43,     0,    44,     0,    45,     0,    46,  1258,     0,
      47,     0,     0,     0,    48,    49,    50,    51,     0,    53,
      54,     0,    55,     0,    57,    58,    59,    60,    61,    62,
      63,     0,    64,    65,    66,     0,    68,    69,     0,     0,
       0,     0,    70,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   180,    79,    80,    81,    82,     0,    83,    84,     0,
      85,   181,    87,    88,     0,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,    92,    93,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,     0,     0,     0,    40,    41,    42,    43,     0,    44,
       0,    45,     0,    46,     0,     0,    47,     0,     0,     0,
      48,    49,    50,    51,     0,    53,    54,     0,    55,     0,
      57,    58,    59,    60,    61,    62,    63,     0,    64,    65,
      66,     0,    68,    69,     0,     0,     0,     0,    70,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,   180,    79,    80,
      81,    82,     0,    83,    84,     0,    85,   181,    87,    88,
       0,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,    91,    92,    93,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,  1375,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,     0,     0,     0,
      40,    41,    42,    43,     0,    44,     0,    45,     0,    46,
       0,     0,    47,     0,     0,     0,    48,    49,    50,    51,
       0,    53,    54,     0,    55,     0,    57,    58,    59,    60,
      61,    62,    63,     0,    64,    65,    66,     0,    68,    69,
       0,     0,     0,     0,    70,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   180,    79,    80,    81,    82,     0,    83,
      84,     0,    85,   181,    87,    88,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   114,   115,  1583,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,     0,     0,     0,    40,    41,    42,    43,
       0,    44,     0,    45,  1625,    46,     0,     0,    47,     0,
       0,     0,    48,    49,    50,    51,     0,    53,    54,     0,
      55,     0,    57,    58,    59,    60,    61,    62,    63,     0,
      64,    65,    66,     0,    68,    69,     0,     0,     0,     0,
      70,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   180,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   181,
      87,    88,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,     0,
       0,     0,    40,    41,    42,    43,     0,    44,     0,    45,
       0,    46,     0,     0,    47,     0,     0,     0,    48,    49,
      50,    51,     0,    53,    54,     0,    55,     0,    57,    58,
      59,    60,    61,    62,    63,     0,    64,    65,    66,     0,
      68,    69,     0,     0,     0,     0,    70,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   180,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   181,    87,    88,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
    1656,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,     0,     0,     0,    40,    41,
      42,    43,     0,    44,     0,    45,     0,    46,     0,     0,
      47,     0,     0,     0,    48,    49,    50,    51,     0,    53,
      54,     0,    55,     0,    57,    58,    59,    60,    61,    62,
      63,     0,    64,    65,    66,     0,    68,    69,     0,     0,
       0,     0,    70,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   180,    79,    80,    81,    82,     0,    83,    84,     0,
      85,   181,    87,    88,     0,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,    92,    93,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,  1657,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,     0,     0,     0,    40,    41,    42,    43,     0,    44,
    1660,    45,     0,    46,     0,     0,    47,     0,     0,     0,
      48,    49,    50,    51,     0,    53,    54,     0,    55,     0,
      57,    58,    59,    60,    61,    62,    63,     0,    64,    65,
      66,     0,    68,    69,     0,     0,     0,     0,    70,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,   180,    79,    80,
      81,    82,     0,    83,    84,     0,    85,   181,    87,    88,
       0,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,    91,    92,    93,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,     0,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,     0,     0,     0,
      40,    41,    42,    43,     0,    44,     0,    45,     0,    46,
       0,     0,    47,     0,     0,     0,    48,    49,    50,    51,
       0,    53,    54,     0,    55,     0,    57,    58,    59,    60,
      61,    62,    63,     0,    64,    65,    66,     0,    68,    69,
       0,     0,     0,     0,    70,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   180,    79,    80,    81,    82,     0,    83,
      84,     0,    85,   181,    87,    88,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   114,   115,  1675,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,     0,     0,     0,    40,    41,    42,    43,
       0,    44,     0,    45,     0,    46,     0,     0,    47,     0,
       0,     0,    48,    49,    50,    51,     0,    53,    54,     0,
      55,     0,    57,    58,    59,    60,    61,    62,    63,     0,
      64,    65,    66,     0,    68,    69,     0,     0,     0,     0,
      70,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   180,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   181,
      87,    88,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,  1728,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,     0,
       0,     0,    40,    41,    42,    43,     0,    44,     0,    45,
       0,    46,     0,     0,    47,     0,     0,     0,    48,    49,
      50,    51,     0,    53,    54,     0,    55,     0,    57,    58,
      59,    60,    61,    62,    63,     0,    64,    65,    66,     0,
      68,    69,     0,     0,     0,     0,    70,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   180,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   181,    87,    88,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
    1734,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    13,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,     0,     0,     0,    40,    41,
      42,    43,     0,    44,     0,    45,     0,    46,     0,     0,
      47,     0,     0,     0,    48,    49,    50,    51,     0,    53,
      54,     0,    55,     0,    57,    58,    59,    60,    61,    62,
      63,     0,    64,    65,    66,     0,    68,    69,     0,     0,
       0,     0,    70,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   180,    79,    80,    81,    82,     0,    83,    84,     0,
      85,   181,    87,    88,     0,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,    92,    93,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,   449,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,     0,     0,     0,    40,    41,    42,    43,     0,    44,
       0,    45,     0,    46,     0,     0,    47,     0,     0,     0,
      48,    49,    50,    51,     0,    53,    54,     0,    55,     0,
      57,    58,    59,    60,   176,   177,    63,     0,    64,    65,
      66,     0,     0,     0,     0,     0,     0,     0,    70,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,   180,    79,    80,
      81,    82,     0,    83,    84,     0,    85,   181,    87,     0,
       0,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,    91,    92,    93,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,     0,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,   703,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,     0,     0,     0,
      40,    41,    42,    43,     0,    44,     0,    45,     0,    46,
       0,     0,    47,     0,     0,     0,    48,    49,    50,    51,
       0,    53,    54,     0,    55,     0,    57,    58,    59,    60,
     176,   177,    63,     0,    64,    65,    66,     0,     0,     0,
       0,     0,     0,     0,    70,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   180,    79,    80,    81,    82,     0,    83,
      84,     0,    85,   181,    87,     0,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   114,   115,     0,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,   921,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,     0,     0,     0,    40,    41,    42,    43,
       0,    44,     0,    45,     0,    46,     0,     0,    47,     0,
       0,     0,    48,    49,    50,    51,     0,    53,    54,     0,
      55,     0,    57,    58,    59,    60,   176,   177,    63,     0,
      64,    65,    66,     0,     0,     0,     0,     0,     0,     0,
      70,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   180,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   181,
      87,     0,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,   114,   115,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,  1437,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,     0,
       0,     0,    40,    41,    42,    43,     0,    44,     0,    45,
       0,    46,     0,     0,    47,     0,     0,     0,    48,    49,
      50,    51,     0,    53,    54,     0,    55,     0,    57,    58,
      59,    60,   176,   177,    63,     0,    64,    65,    66,     0,
       0,     0,     0,     0,     0,     0,    70,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   180,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   181,    87,     0,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   114,   115,
       0,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,     0,  1574,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,    32,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,    39,     0,     0,     0,    40,    41,
      42,    43,     0,    44,     0,    45,     0,    46,     0,     0,
      47,     0,     0,     0,    48,    49,    50,    51,     0,    53,
      54,     0,    55,     0,    57,    58,    59,    60,   176,   177,
      63,     0,    64,    65,    66,     0,     0,     0,     0,     0,
       0,     0,    70,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   180,    79,    80,    81,    82,     0,    83,    84,     0,
      85,   181,    87,     0,     0,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,    92,    93,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   114,   115,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,     0,     0,     0,    40,    41,    42,    43,     0,    44,
       0,    45,     0,    46,     0,     0,    47,     0,     0,     0,
      48,    49,    50,    51,     0,    53,    54,     0,    55,     0,
      57,    58,    59,    60,   176,   177,    63,     0,    64,    65,
      66,     0,     0,     0,     0,     0,     0,     0,    70,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,   180,    79,    80,
      81,    82,     0,    83,    84,     0,    85,   181,    87,     0,
       0,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,    91,    92,    93,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
     114,   115,     0,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
      40,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    51,
       0,     0,     0,     0,     0,     0,     0,    58,    59,    60,
     176,   177,   178,     0,     0,    65,    66,     0,     0,     0,
       0,     0,     0,     0,   179,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   180,    79,    80,    81,    82,     0,    83,
      84,     0,    85,   181,    87,     0,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
      94,     0,     0,     0,     0,    97,    98,   263,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,   264,     0,     0,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,   356,    10,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   593,   378,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,   379,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,    40,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    51,     0,     0,     0,     0,
       0,     0,     0,    58,    59,    60,   176,   177,   178,     0,
       0,    65,    66,     0,     0,     0,     0,     0,     0,     0,
     179,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   180,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   181,
      87,     0,   594,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,     0,     0,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,    40,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    51,     0,     0,     0,     0,     0,     0,     0,    58,
      59,    60,   176,   177,   178,     0,     0,    65,    66,     0,
       0,     0,     0,     0,     0,     0,   179,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   180,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   181,    87,     0,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,   354,   355,   698,
       0,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,   356,    10,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,  1047,   378,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,   379,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,    40,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    51,     0,     0,
       0,     0,     0,     0,     0,    58,    59,    60,   176,   177,
     178,     0,     0,    65,    66,     0,     0,     0,     0,     0,
       0,     0,   179,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   180,    79,    80,    81,    82,     0,    83,    84,     0,
      85,   181,    87,     0,  1048,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,    92,    93,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,     0,     0,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   647,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,    40,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    51,     0,     0,     0,     0,     0,     0,
       0,    58,    59,    60,   176,   177,   178,     0,     0,    65,
      66,     0,     0,     0,     0,     0,     0,     0,   179,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,   180,    79,    80,
      81,    82,     0,    83,    84,     0,    85,   181,    87,     0,
       0,     0,    89,     0,     0,    90,     5,     6,     7,     8,
       9,    91,    92,    93,    94,     0,    10,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
       0,     0,     0,   116,   117,     0,   118,   119,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,    40,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   192,     0,     0,    51,     0,     0,
       0,     0,     0,     0,     0,    58,    59,    60,   176,   177,
     178,     0,     0,    65,    66,     0,     0,     0,     0,     0,
       0,     0,   179,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   180,    79,    80,    81,    82,     0,    83,    84,     0,
      85,   181,    87,     0,     0,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,    92,    93,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,   958,   959,     0,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,   960,    10,   961,   962,   963,   964,   965,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     978,   979,   980,   981,   217,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,   982,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,    40,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    51,     0,     0,     0,     0,     0,     0,
       0,    58,    59,    60,   176,   177,   178,     0,     0,    65,
      66,     0,     0,     0,     0,     0,     0,     0,   179,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,   180,    79,    80,
      81,    82,     0,    83,    84,     0,    85,   181,    87,     0,
       0,     0,    89,     0,     0,    90,     5,     6,     7,     8,
       9,    91,    92,    93,    94,     0,    10,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
       0,     0,     0,   116,   117,     0,   118,   119,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,    40,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    51,     0,     0,
       0,     0,     0,     0,     0,    58,    59,    60,   176,   177,
     178,     0,     0,    65,    66,     0,     0,     0,     0,     0,
       0,     0,   179,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   180,    79,    80,    81,    82,     0,    83,    84,     0,
      85,   181,    87,     0,     0,     0,    89,     0,     0,    90,
       5,     6,     7,     8,     9,    91,    92,    93,    94,     0,
      10,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   246,     0,     0,   116,   117,     0,
     118,   119,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,    40,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    51,     0,     0,     0,     0,     0,     0,     0,    58,
      59,    60,   176,   177,   178,     0,     0,    65,    66,     0,
       0,     0,     0,     0,     0,     0,   179,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   180,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   181,    87,     0,     0,     0,
      89,     0,     0,    90,     5,     6,     7,     8,     9,    91,
      92,    93,    94,     0,    10,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   249,     0,
       0,   116,   117,     0,   118,   119,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,    40,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    51,     0,     0,     0,     0,
       0,     0,     0,    58,    59,    60,   176,   177,   178,     0,
       0,    65,    66,     0,     0,     0,     0,     0,     0,     0,
     179,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   180,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   181,
      87,     0,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,    94,     0,     0,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,   447,     0,     0,     0,   116,   117,     0,   118,   119,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,  -887,  -887,  -887,  -887,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   606,   378,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     379,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,    40,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    51,     0,     0,     0,     0,     0,     0,     0,    58,
      59,    60,   176,   177,   178,     0,     0,    65,    66,     0,
       0,     0,     0,     0,     0,     0,   179,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   180,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   181,    87,     0,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,    94,     0,     0,     0,     0,    97,    98,     0,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     0,     0,   113,     0,   355,     0,
       0,   116,   117,     0,   118,   119,     5,     6,     7,     8,
       9,     0,     0,     0,     0,   356,    10,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   648,   378,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,   379,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,    40,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    51,     0,     0,
       0,     0,     0,     0,     0,    58,    59,    60,   176,   177,
     178,     0,     0,    65,    66,     0,     0,     0,     0,     0,
       0,     0,   179,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   180,    79,    80,    81,    82,     0,    83,    84,     0,
      85,   181,    87,     0,     0,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,    92,    93,    94,     0,
       0,     0,     0,    97,    98,     0,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
       0,     0,   113,     0,   959,     0,     0,   116,   117,     0,
     118,   119,     5,     6,     7,     8,     9,     0,     0,     0,
       0,   960,    10,   961,   962,   963,   964,   965,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     978,   979,   980,   981,   684,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,   982,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,    40,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    51,     0,     0,     0,     0,     0,     0,
       0,    58,    59,    60,   176,   177,   178,     0,     0,    65,
      66,     0,     0,     0,     0,     0,     0,     0,   179,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,   180,    79,    80,
      81,    82,     0,    83,    84,     0,    85,   181,    87,     0,
       0,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,    91,    92,    93,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
       0,     0,     0,   116,   117,     0,   118,   119,     5,     6,
       7,     8,     9,     0,     0,     0,     0,   960,    10,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,   980,   981,
     686,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,   982,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
      40,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    51,
       0,     0,     0,     0,     0,     0,     0,    58,    59,    60,
     176,   177,   178,     0,     0,    65,    66,     0,     0,     0,
       0,     0,     0,     0,   179,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   180,    79,    80,    81,    82,     0,    83,
      84,     0,    85,   181,    87,     0,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
      94,     0,     0,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,     0,     0,     0,   116,
     117,     0,   118,   119,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,  1090,   378,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,   379,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,    40,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    51,     0,     0,     0,     0,
       0,     0,     0,    58,    59,    60,   176,   177,   178,     0,
       0,    65,    66,     0,     0,     0,     0,     0,     0,     0,
     179,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   180,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   181,
      87,     0,     0,     0,    89,     0,     0,    90,     5,     6,
       7,     8,     9,    91,    92,    93,    94,     0,    10,     0,
       0,    97,    98,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,     0,     0,
     113,     0,     0,     0,     0,   116,   117,     0,   118,   119,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
      40,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    51,
       0,     0,     0,     0,     0,     0,     0,    58,    59,    60,
     176,   177,   178,     0,     0,    65,    66,     0,     0,     0,
       0,     0,     0,     0,   179,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   180,    79,    80,    81,    82,     0,    83,
      84,     0,    85,   181,    87,     0,     0,     0,    89,     0,
       0,    90,     5,     6,     7,     8,     9,    91,    92,    93,
      94,     0,    10,     0,     0,    97,    98,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,     0,     0,   113,     0,     0,     0,     0,   116,
     117,     0,   118,   119,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,   537,    38,     0,
       0,     0,     0,     0,    40,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    51,     0,     0,     0,     0,     0,     0,
       0,    58,    59,    60,   176,   177,   178,     0,     0,    65,
      66,     0,     0,     0,     0,     0,     0,     0,   179,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,   180,    79,    80,
      81,    82,     0,    83,    84,     0,    85,   181,    87,     0,
       0,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,    91,    92,    93,    94,     0,     0,     0,     0,    97,
      98,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,     0,     0,   113,     0,
       0,     0,     0,   116,   117,     0,   118,   119,  1457,  1458,
    1459,  1460,  1461,     0,     0,  1462,  1463,  1464,  1465,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1466,  1467,   961,   962,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,     0,     0,     0,  1468,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   982,     0,
       0,     0,  1469,  1470,  1471,  1472,  1473,  1474,  1475,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,  1476,  1477,  1478,  1479,  1480,  1481,  1482,  1483,  1484,
    1485,  1486,  1487,  1488,  1489,  1490,  1491,  1492,  1493,  1494,
    1495,  1496,  1497,  1498,  1499,  1500,  1501,  1502,  1503,  1504,
    1505,  1506,  1507,  1508,  1509,  1510,  1511,  1512,  1513,  1514,
    1515,  1516,     0,     0,  1517,  1518,     0,  1519,  1520,  1521,
    1522,  1523,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1524,  1525,  1526,     0,     0,     0,    83,
      84,     0,    85,   181,    87,  1527,     0,  1528,  1529,     0,
    1530,     0,     0,     0,     0,     0,     0,  1531,     0,     0,
       0,  1532,     0,  1533,     0,  1534,  1535,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   353,   354,   355,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     356,     0,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,     0,   378,   353,   354,   355,     0,     0,
       0,     0,     0,     0,     0,     0,   379,     0,     0,     0,
       0,     0,     0,     0,   356,     0,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,     0,   378,   353,
     354,   355,     0,     0,     0,     0,     0,     0,     0,     0,
     379,     0,     0,     0,     0,     0,     0,     0,   356,     0,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,     0,   378,   353,   354,   355,     0,     0,     0,     0,
       0,     0,     0,     0,   379,     0,     0,     0,     0,     0,
       0,     0,   356,     0,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,     0,   378,   353,   354,   355,
       0,     0,    36,     0,   209,     0,   991,     0,   379,     0,
       0,     0,     0,     0,     0,     0,   356,     0,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,     0,
     378,   251,   210,     0,     0,     0,   987,   988,     0,     0,
       0,     0,   379,     0,   529,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   252,     0,     0,
       0,     0,     0,   180,     0,     0,    81,    82,     0,    83,
      84,     0,    85,   181,    87,     0,     0,  1626,     0,    36,
       0,     0,   966,   967,   968,   969,   970,   971,   972,   973,
     974,   975,   976,   977,   978,   979,   980,   981,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   982,     0,     0,   211,     0,     0,     0,     0,   116,
    1446,     0,    36,   251,     0,     0,     0,     0,     0,     0,
       0,     0,   253,   254,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   252,
     180,     0,     0,    81,   255,     0,    83,    84,     0,    85,
     181,    87,     0,  1292,     0,     0,  1394,     0,     0,     0,
       0,    36,     0,     0,   256,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,  -319,    83,
      84,   257,    85,   181,    87,  1621,    58,    59,    60,   176,
     177,   343,     0,     0,     0,   251,     0,     0,     0,     0,
       0,     0,     0,     0,   253,   254,     0,     0,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   252,   180,     0,  1395,    81,   255,     0,    83,    84,
       0,    85,   181,    87,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,   251,   256,     0,     0,     0,
       0,     0,     0,     0,     0,   344,     0,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     470,   252,     0,   257,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,    36,   378,     0,   253,   254,     0,     0,
       0,     0,     0,     0,     0,     0,   379,     0,     0,     0,
       0,     0,     0,     0,   180,   251,     0,    81,   255,     0,
      83,    84,     0,    85,   181,    87,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   256,     0,
       0,   252,     0,     0,     0,     0,   253,   254,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,    36,   180,   257,     0,    81,   255,     0,
      83,    84,     0,    85,   181,    87,     0,  1268,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   256,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,   257,   253,   254,     0,     0,
       0,     0,     0,    36,     0,   209,     0,     0,     0,     0,
       0,     0,     0,     0,   180,     0,     0,    81,   255,     0,
      83,    84,     0,    85,   181,    87,    36,     0,     0,     0,
       0,     0,     0,     0,   716,     0,     0,     0,   256,     0,
       0,     0,     0,   210,     0,     0,  1147,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   723,   724,     0,   257,     0,     0,   725,     0,
     726,     0,     0,     0,   180,     0,     0,    81,    82,     0,
      83,    84,   727,    85,   181,    87,     0,     0,     0,     0,
      33,    34,    35,    36,     0,     0,     0,   180,     0,     0,
      81,   728,     0,    83,    84,     0,    85,   181,    87,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,   211,     0,     0,     0,     0,
     116,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,   729,     0,    72,    73,
      74,    75,    76,    36,     0,   209,     0,     0,     0,   730,
       0,     0,     0,     0,   180,    79,    80,    81,   731,     0,
      83,    84,     0,    85,   181,    87,    36,     0,     0,    89,
       0,     0,     0,     0,     0,     0,     0,     0,   732,   733,
     734,   735,     0,   222,     0,     0,    97,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   723,   724,     0,   736,     0,     0,   725,     0,
     726,     0,     0,     0,   180,     0,     0,    81,    82,     0,
      83,    84,   727,    85,   181,    87,     0,     0,     0,     0,
      33,    34,    35,    36,     0,     0,     0,     0,     0,     0,
     501,   728,     0,    83,    84,     0,    85,   181,    87,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,   223,     0,     0,     0,     0,
     116,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,   729,     0,    72,    73,
      74,    75,    76,     0,     0,     0,   882,   883,     0,   730,
       0,     0,     0,     0,   180,    79,    80,    81,   731,     0,
      83,    84,     0,    85,   181,    87,   884,     0,     0,    89,
       0,     0,     0,     0,   885,   886,   887,    36,   732,   733,
     734,   735,     0,     0,     0,   888,    97,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,   736,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,    36,     0,   209,     0,     0,
     889,     0,     0,     0,     0,     0,     0,     0,   982,     0,
       0,     0,     0,   890,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,    83,    84,     0,    85,   181,    87,
       0,     0,     0,     0,     0,   210,     0,     0,     0,     0,
       0,     0,   891,     0,     0,     0,     0,  1016,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   180,     0,     0,    81,
      82,     0,    83,    84,     0,    85,   181,    87,    36,     0,
     209,     0,     0,     0,     0,     0,   551,     0,     0,     0,
       0,     0,  1541,     0,    83,    84,  1542,    85,   181,    87,
      36,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,     0,     0,   211,   210,     0,
      36,     0,   116,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,     0,  1395,
    1073,  1074,  1075,    36,     0,     0,     0,     0,     0,   180,
       0,     0,    81,    82,     0,    83,    84,     0,    85,   181,
      87,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    83,    84,     0,
      85,   181,    87,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,    83,    84,     0,
      85,   181,    87,     0,     0,   552,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
      83,    84,   599,    85,   181,    87,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,     0,   853,   353,   354,   355,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   356,     0,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,     0,   378,   353,   354,   355,
       0,     0,     0,     0,     0,     0,     0,     0,   379,     0,
       0,     0,     0,     0,     0,     0,   356,     0,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,     0,
     378,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   379,   353,   354,   355,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   424,   356,     0,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,     0,   378,   353,   354,   355,
       0,     0,     0,     0,     0,     0,     0,     0,   379,     0,
       0,     0,     0,     0,     0,   433,   356,     0,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,     0,
     378,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   379,   353,   354,   355,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   820,   356,     0,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,     0,   378,   353,   354,   355,
       0,     0,     0,     0,     0,     0,     0,     0,   379,     0,
       0,     0,     0,     0,     0,   859,   356,     0,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,     0,
     378,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   379,   353,   354,   355,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   900,   356,     0,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,     0,   378,   957,   958,   959,
       0,     0,     0,     0,     0,     0,     0,     0,   379,     0,
       0,     0,     0,     0,     0,  1204,   960,     0,   961,   962,
     963,   964,   965,   966,   967,   968,   969,   970,   971,   972,
     973,   974,   975,   976,   977,   978,   979,   980,   981,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   982,   957,   958,   959,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1223,   960,     0,   961,   962,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,   957,   958,   959,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   982,     0,
       0,     0,     0,     0,   960,  1122,   961,   962,   963,   964,
     965,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   978,   979,   980,   981,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     982,   957,   958,   959,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     960,  1281,   961,   962,   963,   964,   965,   966,   967,   968,
     969,   970,   971,   972,   973,   974,   975,   976,   977,   978,
     979,   980,   981,   957,   958,   959,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   982,     0,     0,     0,
       0,     0,   960,  1286,   961,   962,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,     0,     0,     0,   982,   957,
     958,   959,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,     0,     0,     0,     0,   960,  1364,
     961,   962,   963,   964,   965,   966,   967,   968,   969,   970,
     971,   972,   973,   974,   975,   976,   977,   978,   979,   980,
     981,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1382,   982,     0,     0,    36,     0,     0,
       0,  1444,     0,     0,   180,  1383,  1384,    81,    82,     0,
      83,    84,     0,    85,   181,    87,    36,     0,   794,   795,
       0,     0,     0,   180,   272,   273,    81,  1385,     0,    83,
      84,     0,    85,  1386,    87,     0,     0,     0,     0,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,     0,     0,     0,  1445,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   274,    36,     0,    83,    84,     0,    85,   181,    87,
       0,     0,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,     0,    83,    84,     0,    85,   181,    87,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,    36,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   340,     0,    83,
      84,     0,    85,   181,    87,     0,     0,     0,    36,   505,
       0,     0,    83,    84,     0,    85,   181,    87,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    36,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   637,     0,     0,     0,     0,   274,
       0,     0,    83,    84,     0,    85,   181,    87,     0,    36,
       0,     0,     0,  1139,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    83,    84,     0,    85,   181,
      87,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,    83,    84,     0,    85,   181,    87,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,    83,    84,     0,    85,
     181,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     353,   354,   355,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   707,   356,
       0,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,     0,   378,   353,   354,   355,     0,     0,     0,
       0,     0,     0,     0,     0,   379,     0,     0,     0,     0,
       0,     0,     0,   356,   856,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   708,   378,   353,   354,
     355,     0,     0,     0,     0,     0,     0,     0,     0,   379,
       0,     0,     0,     0,     0,     0,     0,   356,     0,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
       0,   378,   957,   958,   959,     0,     0,     0,     0,     0,
       0,     0,     0,   379,     0,     0,     0,     0,     0,     0,
       0,   960,  1291,   961,   962,   963,   964,   965,   966,   967,
     968,   969,   970,   971,   972,   973,   974,   975,   976,   977,
     978,   979,   980,   981,   957,   958,   959,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   982,     0,     0,
       0,     0,     0,   960,     0,   961,   962,   963,   964,   965,
     966,   967,   968,   969,   970,   971,   972,   973,   974,   975,
     976,   977,   978,   979,   980,   981,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   982,
    -887,  -887,  -887,  -887,   970,   971,   972,   973,   974,   975,
     976,   977,   978,   979,   980,   981,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   982
};

static const yytype_int16 yycheck[] =
{
       4,   135,   162,   310,    52,    88,   183,   596,    32,    53,
     449,   677,    95,    96,   251,   252,     4,     4,     4,    43,
     257,     4,     4,    47,   418,  1039,   413,  1036,   831,   392,
     570,    30,   224,   569,   165,   378,   162,   113,   706,   172,
     544,   852,   230,   230,  1025,    49,  1023,     9,    52,   713,
     184,   134,   813,     9,   441,     9,   220,   868,   497,   113,
       9,     9,    30,    30,     9,    69,    45,    45,    45,     9,
      45,    66,     4,     9,    45,     9,     9,    66,     9,     9,
       9,     9,    86,     9,    88,     9,    79,     9,    35,     9,
       9,    95,    96,     9,    26,    27,   333,    56,     9,     9,
     231,     9,     9,     9,   914,    79,    66,     4,    66,   127,
      79,     4,    84,    33,    84,    99,   100,  1568,   168,    78,
     127,   153,    81,    96,    79,   199,   127,    50,   104,   199,
     134,   382,    79,    99,   100,   211,   112,   113,   114,   115,
     116,   117,    79,    35,   108,    66,     0,   223,    53,   199,
      35,     8,    35,    96,    71,    72,    35,   199,   147,   410,
      65,    96,   147,   414,    66,    66,    66,   127,   345,   127,
     202,  1622,   144,   199,   144,   162,    66,   310,   151,    66,
      66,   113,    66,   168,    66,   203,    66,    79,   199,   950,
      66,    66,   196,   200,    79,    66,    79,    66,    66,   200,
      79,   165,  1597,  1598,   199,   181,   127,   202,   151,  1218,
     147,   204,   288,   202,   161,    30,   151,   202,   197,   203,
     169,   200,   200,   200,   169,   200,   197,   196,   123,   152,
     204,   468,   236,   201,   288,   202,   240,   203,   135,  1220,
     244,   203,   202,   420,   202,   201,  1227,   201,  1229,   204,
     236,  1228,   385,   201,   240,  1066,   339,   391,   262,   698,
     924,   201,   926,   427,   703,   201,   202,   201,   201,   161,
     201,   201,   201,   201,   407,   201,   161,   201,   200,   211,
     200,   200,   161,    96,   200,   217,    96,   184,   336,   200,
     200,   223,   200,   200,   200,  1105,    50,   199,   431,   803,
     202,   202,   202,    96,   236,   388,   389,   440,   240,   849,
     443,    79,   202,   272,   273,   274,   202,   123,   202,   323,
     202,  1330,   202,  1332,  1085,   199,   202,   202,   332,   566,
     567,   202,   336,   202,   202,   339,   323,   574,   151,    79,
      29,   151,   151,   302,    79,    71,    72,   279,   392,    84,
     153,    35,   428,   202,   286,   287,   288,    46,   151,  1340,
      49,   293,   199,   166,    79,   125,   126,   299,   199,    84,
     199,   112,   113,   114,   115,   116,   117,   145,   146,   378,
     199,   385,   386,   387,   388,   389,   385,   575,   575,    79,
     554,   323,   201,   202,    84,    79,   790,   561,   152,   202,
     564,    14,    29,   407,   144,   145,   146,   451,   407,   199,
     145,   146,   199,   167,  1423,   201,   202,    30,   152,    46,
    1086,   199,    49,    79,   584,   318,   323,   431,    84,   199,
     145,   146,   431,  1101,    47,    49,    50,    51,    79,   443,
     181,   440,   199,    84,   443,   201,   202,   639,   112,   113,
     114,    65,   456,   199,   144,   145,   146,   202,   584,   151,
     201,   199,   112,   113,   114,   115,   116,   117,     4,   646,
     456,    79,   636,   208,   666,    35,    84,   201,   865,    98,
      99,   100,   921,   201,   721,   722,   418,   201,   875,   145,
     146,   683,   207,   390,   682,   682,   428,  1593,  1594,     4,
     688,   688,  1038,   144,   145,   146,   813,   201,   512,    45,
     201,   147,  1052,   201,  1275,  1055,    66,    79,    66,   602,
     721,   722,    84,   527,   456,    98,    99,   100,   871,   533,
     534,   181,   168,   117,   118,   119,    66,   145,   146,   201,
      45,   202,   501,   147,   199,   737,   505,   199,    66,   147,
     199,   510,    49,    50,    51,   719,    53,   688,   151,   201,
     296,    44,  1571,   199,   300,    65,   202,   804,    65,   105,
    1236,   168,  1238,   147,   110,   127,   112,   113,   114,   115,
     116,   117,   118,   145,   146,   206,     9,   199,  1349,   325,
     827,   327,   328,   329,   330,   127,   147,   584,   602,     8,
     105,   147,   201,   840,   168,   110,    14,   112,   113,   114,
     115,   116,   117,   118,   199,    14,    79,    14,   154,   155,
     201,   157,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,   201,   112,   113,   114,   115,
     116,   117,   200,   950,   168,   181,    96,   123,   124,   154,
     155,    14,   157,   200,     9,  1018,   200,   182,   183,   184,
     205,   593,   199,   104,   189,   190,   199,   203,   193,   194,
    1206,    63,    64,   677,  1061,   679,   181,   570,   702,   199,
     813,  1347,  1222,  1070,   160,  1694,  1700,   200,   200,     9,
      88,   928,   696,   930,   201,    14,   199,     9,   203,   185,
    1709,  1715,   835,    79,    79,   181,   710,   711,    79,   696,
    1104,   188,   199,   201,     9,     9,   648,    46,    47,    48,
      49,    50,    51,   201,    53,   711,    79,   200,   200,   125,
     201,   895,   199,    66,   200,   127,    65,    30,   126,  1178,
     167,   128,     9,   200,   112,   113,   114,   115,   116,   117,
       9,   200,   684,   147,   686,   123,   124,   761,   200,     9,
     200,    14,   197,   767,   696,  1431,     9,   771,   772,    66,
       9,   200,  1009,     9,   761,   939,   708,    14,  1085,   711,
     169,   125,   946,   203,   206,   206,     9,   870,   199,   199,
     158,   206,   160,  1187,   200,   199,    96,   128,   802,   696,
    1037,  1341,  1039,   206,   147,   200,     9,   766,   200,  1248,
     201,   770,   816,   181,   802,   802,   802,   950,   201,   802,
     802,   200,   199,   147,   147,  1264,   199,  1064,  1215,   761,
     199,   835,   199,   199,   185,   185,   835,  1224,    46,    47,
      48,    49,    50,    51,   202,    14,  1233,   779,     9,     4,
      79,    14,  1044,   202,   201,    96,   202,    65,   790,   791,
      14,   206,   202,    14,   761,   201,   870,  1670,    30,   197,
     802,    30,   871,   199,   199,    14,     9,   199,   882,   883,
     884,   199,  1119,   907,  1687,   199,   201,   200,   199,    14,
      45,   201,  1695,   128,     9,   200,     9,    65,   206,   144,
      79,     9,   128,   199,   908,   802,   910,  1573,   912,   802,
     201,    14,  1351,    79,    79,   199,   202,  1081,   200,   199,
     199,  1360,   908,   910,   200,   912,   128,   931,   202,   202,
     199,   206,   200,     9,   144,  1374,    30,    73,   831,   201,
    1327,   201,   200,   128,   169,   949,    30,   128,   200,   200,
     105,   955,  1085,     9,   200,   110,   849,   112,   113,   114,
     115,   116,   117,   118,     9,  1129,   200,   200,  1275,  1133,
       9,   203,  1136,   200,  1018,     9,   908,    96,   910,  1143,
     912,   200,   914,   915,    14,   203,   202,   199,   202,   200,
     128,   200,     9,   200,   998,   199,   201,  1001,  1437,   154,
     155,   200,   157,   200,   200,    30,  1010,   200,   200,   156,
     201,   201,   152,   910,   202,   912,    79,    14,  1022,    79,
     200,   199,   110,   200,   200,   200,   181,  1616,  1415,   202,
    1267,   128,  1269,   200,  1022,  1022,  1022,   128,    14,  1022,
    1022,   201,  1349,     4,   202,    14,    79,   200,   203,   199,
     202,   200,    14,   128,    14,   201,   201,   201,  1295,    14,
     202,    55,     9,   200,    79,   199,    79,    79,   108,  1093,
     201,  1308,    74,    75,    76,    96,     4,   147,  1082,   159,
    1244,    33,  1086,    85,    45,    96,   200,    14,   199,   201,
    1022,   165,  1096,   199,    79,   162,  1685,     9,   200,    79,
     201,   200,   200,    14,  1108,    79,   202,    79,    14,    79,
    1096,    14,    79,    14,  1678,  1047,   770,    45,   766,   389,
     510,  1108,   386,   388,  1563,  1022,  1565,   869,   805,  1022,
     132,   133,   134,   135,   136,  1574,   866,  1691,  1436,  1102,
    1261,   143,  1275,  1687,   105,   515,  1426,   149,   150,   110,
    1310,   112,   113,   114,   115,   116,   117,   118,  1090,  1052,
    1455,   163,  1055,  1539,  1096,  1303,  1719,  1551,  1707,    41,
    1422,   392,  1104,  1105,  1308,   993,  1108,   105,   180,   486,
    1619,   758,   110,   990,   112,   113,   114,   115,   116,   117,
     118,  1026,   947,   154,   155,  1068,   157,   486,  1185,   897,
     883,  1082,   294,   313,   953,     4,   287,  1194,    -1,    26,
      27,  1108,   721,    30,    -1,   932,  1349,    -1,    -1,    -1,
     181,    -1,    -1,  1400,    -1,    -1,   154,   155,    -1,   157,
      -1,    -1,  1236,    -1,  1238,    52,    -1,    -1,    -1,    -1,
      -1,    -1,   203,    -1,    -1,    -1,    45,    -1,    -1,    -1,
    1298,    -1,    -1,   181,    -1,  1187,    -1,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,  1713,   203,    -1,    -1,    -1,    -1,
      -1,  1720,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1294,    -1,  1185,    -1,  1298,    -1,    -1,    -1,    -1,  1303,
      -1,  1194,    -1,    -1,    -1,    -1,   105,  1294,    63,    64,
      -1,   110,    -1,   112,   113,   114,   115,   116,   117,   118,
      -1,    -1,    -1,  1310,    -1,    -1,    -1,    -1,    -1,  1222,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1347,  1331,    -1,    -1,    -1,  1352,    -1,
    1337,    -1,  1339,  1357,  1402,   154,   155,  1361,   157,    -1,
      -1,    -1,  1294,    -1,    -1,  1352,    -1,    -1,    -1,    -1,
      -1,  1357,   127,    -1,  1361,    -1,  1553,    -1,    -1,    -1,
      -1,    -1,   181,    -1,    -1,  1389,    -1,    -1,    -1,    -1,
      -1,  1395,    -1,    -1,   211,    -1,    -1,  1294,  1402,    -1,
     217,    -1,  1406,    -1,   203,    -1,   223,    -1,    -1,    -1,
    1307,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1352,    -1,    -1,    -1,    -1,  1357,    -1,  1431,    -1,  1361,
    1434,  1435,  1436,    -1,   251,   252,  1440,    -1,  1331,    -1,
     257,  1428,    -1,  1447,  1337,   200,  1339,  1434,  1341,  1435,
    1436,    -1,    -1,  1440,    -1,  1352,    -1,    -1,    -1,    -1,
    1447,    -1,   279,  1700,  1361,    -1,    -1,  1550,    -1,   286,
     287,    -1,    -1,    -1,    -1,    -1,   293,    -1,  1715,    -1,
      -1,    -1,   299,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   310,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1434,  1435,  1436,    -1,  1666,  1404,  1440,    -1,
      -1,    -1,    -1,    -1,    -1,  1447,   333,    -1,    -1,   336,
      -1,    -1,    -1,    -1,    -1,    -1,  1609,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1428,    -1,  1434,    -1,    -1,
      -1,    -1,    -1,  1440,    -1,    -1,  1550,    -1,  1682,    -1,
    1447,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   378,    -1,    -1,    -1,  1569,    -1,    -1,    -1,  1573,
      -1,    -1,    -1,    -1,  1578,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1588,    -1,    -1,    -1,    -1,  1593,
    1594,  1578,    -1,  1597,  1598,    -1,    -1,    -1,    -1,    -1,
      -1,   418,    -1,    -1,    -1,  1609,    -1,    -1,    -1,    -1,
      -1,   428,    -1,  1617,  1618,    -1,    -1,    -1,    -1,    -1,
    1624,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1617,  1618,    -1,    -1,    -1,    -1,    -1,  1624,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1578,    -1,    -1,    -1,
      -1,   468,   469,    -1,  1658,   472,    -1,    -1,    -1,    -1,
      -1,  1665,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1658,    -1,    -1,    -1,    -1,    -1,  1681,    -1,  1666,
      -1,  1578,    -1,    -1,    -1,  1617,  1618,    -1,    -1,    -1,
      -1,    -1,  1624,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     517,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,  1721,    53,    -1,
    1617,  1618,    -1,  1727,    26,    27,  1658,  1624,    30,    -1,
      65,    -1,    -1,    -1,  1721,    -1,    -1,    -1,    -1,    -1,
    1727,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   566,
     567,    -1,    -1,    -1,    -1,    -1,    -1,   574,    -1,    -1,
      -1,  1658,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,
      79,    -1,    -1,    -1,    -1,    -1,   593,  1670,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,  1721,
      -1,    -1,    -1,    -1,  1687,  1727,    -1,    -1,    -1,    -1,
      -1,    29,  1695,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,  1721,    53,    -1,    -1,    -1,    -1,
    1727,   648,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,   684,    -1,   686,
      -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,    -1,
      -1,   708,   709,   202,    -1,   204,    -1,    -1,    -1,    10,
      11,    12,    63,    64,   721,   722,   723,   724,   725,   726,
     727,    -1,    -1,    35,    -1,   217,    -1,    -1,    29,   736,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,   762,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    77,    -1,    79,    -1,    -1,
      -1,    -1,   779,    -1,    -1,    -1,   127,    -1,    -1,    -1,
      -1,   788,    -1,   790,   791,    -1,    -1,   279,   206,    -1,
      -1,    -1,    -1,    -1,   286,   287,    -1,   804,   805,    -1,
      -1,   293,    -1,    -1,    -1,   117,   813,   299,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   310,    -1,
     827,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   840,    -1,    -1,   148,    -1,    -1,   151,
     152,   848,   154,   155,   851,   157,   158,   159,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   871,    -1,    -1,    -1,    -1,    -1,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   378,   199,    -1,    -1,
      -1,    -1,   204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   203,    -1,    -1,    -1,    -1,   914,   915,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,   928,    53,   930,    -1,   932,   418,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
     947,    -1,    -1,   950,   951,   952,   953,    -1,    -1,   956,
     957,   958,   959,   960,   961,   962,   963,   964,   965,   966,
     967,   968,   969,   970,   971,   972,   973,   974,   975,   976,
     977,   978,   979,   980,   981,   982,    -1,    -1,    -1,    -1,
     472,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1006,
      -1,    29,  1009,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,   517,    -1,    -1,    -1,    -1,
    1037,    -1,  1039,    -1,    -1,    -1,    -1,    65,    -1,    -1,
    1047,    -1,    67,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    79,    -1,    -1,  1064,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1085,    -1,
      -1,    -1,    -1,  1090,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   117,    -1,    -1,    -1,    -1,  1104,  1105,    -1,
    1107,   593,    -1,    -1,    -1,    -1,    52,    63,    64,    -1,
      -1,    -1,  1119,    -1,    -1,  1122,    -1,  1124,    -1,    -1,
      -1,    -1,    -1,   148,    -1,    -1,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,
    1147,   166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   648,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   127,   200,    -1,   199,    -1,    -1,  1184,    -1,   204,
    1187,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   684,    -1,   686,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1211,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   708,   709,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,   723,   724,   725,   726,   727,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,   736,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1267,    -1,  1269,    -1,    -1,    -1,    -1,  1274,  1275,    -1,
     762,    -1,  1279,    -1,  1281,    -1,  1283,    -1,    -1,  1286,
      -1,  1288,    -1,    -1,  1291,    -1,    -1,   779,  1295,    -1,
      -1,  1298,  1299,    -1,  1301,    -1,   788,    -1,   790,   791,
      77,  1308,    79,    -1,    -1,   251,   252,    -1,    -1,    -1,
      -1,   257,    -1,   805,    -1,    -1,    -1,  1324,    -1,    -1,
      -1,   813,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,  1349,    53,    -1,    -1,   123,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,   848,  1364,    -1,   851,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1377,  1378,    -1,    -1,    -1,    -1,    -1,   154,   155,   871,
     157,   158,   159,    -1,    -1,    -1,    -1,   333,    -1,    -1,
     336,    -1,    -1,    -1,    -1,  1402,    -1,    -1,    -1,  1406,
      -1,    -1,    -1,    -1,    -1,  1412,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,    -1,   914,   915,    -1,   202,    -1,   204,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1443,  1444,  1445,    -1,
      -1,    -1,    -1,  1450,  1451,    -1,    -1,    -1,  1455,    -1,
      -1,    -1,    -1,    -1,    -1,   947,    -1,    -1,   950,   951,
     952,   953,    -1,    -1,   956,   957,   958,   959,   960,   961,
     962,   963,   964,   965,   966,   967,   968,   969,   970,   971,
     972,   973,   974,   975,   976,   977,   978,   979,   980,   981,
     982,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,  1006,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   468,   469,    -1,    29,   472,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,  1558,    -1,    -1,    -1,  1047,    -1,    -1,    -1,    -1,
      -1,    65,  1569,    -1,    -1,    77,    -1,    -1,    -1,    -1,
      -1,   517,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1588,    -1,    -1,    -1,  1592,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1085,    -1,    -1,  1603,    -1,  1090,    -1,
      -1,  1608,    -1,    -1,  1611,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1104,  1105,    -1,  1107,    -1,    -1,    -1,    -1,
     566,   567,    -1,    -1,    -1,    -1,    -1,    -1,   574,    -1,
    1122,    -1,  1124,    -1,    -1,    -1,   148,    -1,    -1,   151,
      -1,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1147,    -1,    -1,    -1,    -1,
      -1,    -1,  1669,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1677,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,  1691,    -1,    -1,    -1,    -1,    -1,
      -1,   203,  1184,  1700,    -1,  1187,    -1,    -1,    -1,   203,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1715,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,  1724,    -1,  1211,
      -1,    -1,    -1,    -1,    -1,  1732,    -1,    -1,    -1,  1736,
      -1,    29,  1739,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   709,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   721,   722,   723,   724,   725,
     726,   727,  1274,  1275,    -1,    -1,    -1,  1279,    -1,  1281,
     736,  1283,    -1,    -1,  1286,    -1,  1288,    -1,    -1,  1291,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1299,    -1,  1301,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,  1324,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,  1349,   804,    -1,
      -1,    26,    27,    -1,    -1,    30,    -1,    65,    -1,    -1,
      -1,    -1,  1364,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   827,    -1,    -1,    -1,  1377,  1378,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   840,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   848,    -1,    -1,   203,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1412,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,  1443,  1444,  1445,    -1,    -1,    -1,    -1,  1450,  1451,
      -1,    65,    -1,  1455,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,   928,    53,   930,    -1,   932,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,   947,    -1,   201,    -1,   951,   952,   953,    -1,    -1,
     956,   957,   958,   959,   960,   961,   962,   963,   964,   965,
     966,   967,   968,   969,   970,   971,   972,   973,   974,   975,
     976,   977,   978,   979,   980,   981,   982,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   211,    -1,    -1,    10,
      11,    12,   217,    -1,    -1,    -1,    -1,    -1,   223,    -1,
    1006,    -1,    -1,  1009,    -1,    -1,  1558,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,  1037,    53,  1039,    -1,    -1,    -1,    -1,    -1,   203,
    1592,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      77,  1603,    79,    -1,   279,    -1,  1608,    -1,  1064,  1611,
      -1,   286,   287,    -1,    -1,    -1,    -1,    -1,   293,    -1,
      -1,    -1,    -1,    -1,   299,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   310,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1119,    -1,    -1,  1122,  1669,  1124,    -1,
      -1,    -1,    -1,    -1,    -1,  1677,    -1,   154,   155,    -1,
     157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,  1691,
      -1,  1147,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   378,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,    -1,  1724,    -1,    -1,   202,    -1,   204,    -1,    -1,
    1732,    -1,   203,    -1,  1736,    -1,    -1,  1739,    10,    11,
      12,    -1,    -1,   418,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   428,    -1,  1211,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,  1267,    53,  1269,    -1,    -1,    -1,    -1,  1274,    -1,
      -1,    -1,    -1,  1279,    65,  1281,    -1,  1283,    -1,    -1,
    1286,    -1,  1288,    -1,    -1,  1291,    -1,    -1,    -1,  1295,
      -1,    -1,  1298,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,  1308,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,  1324,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,  1364,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   593,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1402,    -1,     5,     6,
    1406,     8,     9,    10,    11,    12,  1412,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    -1,
      -1,    28,    29,   648,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    -1,    -1,  1443,  1444,  1445,
      -1,    48,    -1,    50,  1450,    -1,    53,    -1,    55,    -1,
      -1,    -1,    -1,    68,    -1,    -1,    -1,    -1,    -1,   684,
      -1,   686,    77,    -1,    79,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,   708,    -1,    -1,    10,    11,    12,    -1,
      -1,   203,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,   117,    -1,    -1,    29,   113,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,   148,    -1,    -1,   151,   152,    -1,   154,
     155,    65,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1558,    -1,   779,    -1,    -1,    -1,    -1,    -1,
      77,    -1,    -1,  1569,    -1,   790,   791,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   188,  1588,    -1,   199,    -1,  1592,    -1,   813,   204,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1603,    -1,    -1,
      -1,    -1,  1608,    -1,    -1,  1611,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   232,    -1,    -1,   235,    -1,
      -1,    -1,    -1,    -1,   151,   242,   243,   154,   155,    -1,
     157,   158,   159,    -1,    -1,    -1,   871,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   472,  1669,    -1,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   203,
      -1,   288,    -1,    -1,    -1,   202,    -1,   294,    -1,   914,
     915,   298,    -1,    -1,  1700,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   311,   312,   313,   517,    -1,  1715,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   324,  1724,    -1,
      -1,    -1,    -1,    -1,    -1,   950,  1732,    -1,   335,    -1,
    1736,    -1,    -1,  1739,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,    -1,   379,    -1,   381,   382,    77,   384,    79,    -1,
      -1,    -1,    -1,    -1,    -1,   392,   393,   394,   395,   396,
     397,   398,   399,   400,   401,   402,   403,   404,    -1,    -1,
      -1,    -1,    -1,   410,   411,    -1,   413,   414,   415,    -1,
      -1,    -1,    -1,    -1,   421,    -1,   117,   424,    -1,    -1,
      -1,    -1,  1047,    -1,    -1,    -1,   433,    -1,   435,    -1,
      -1,    -1,    -1,    -1,   441,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   451,    -1,   453,   148,    -1,    -1,
     151,   152,    -1,   154,   155,    -1,   157,   158,   159,    -1,
    1085,    -1,    -1,    -1,    -1,  1090,    -1,    -1,    -1,    -1,
      -1,    -1,   479,    -1,    -1,   482,   483,   484,    -1,  1104,
    1105,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,    -1,    -1,   199,   709,
      -1,   202,    -1,   204,    -1,    -1,   513,    -1,    10,    11,
      12,    -1,    -1,   723,   724,   725,   726,   727,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   736,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      29,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1187,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,   594,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,   606,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,   638,    -1,    -1,    -1,    -1,   472,    65,   848,    -1,
     647,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1275,   130,   131,    -1,    -1,    -1,    -1,    -1,   665,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   148,
      -1,    -1,   151,   152,    -1,   154,   155,    -1,   157,   158,
     159,   517,   161,    -1,    -1,   692,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   172,    -1,    -1,    -1,    -1,    -1,   706,
      -1,   203,    -1,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    77,    -1,    79,
     199,    -1,    -1,    -1,  1349,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   947,    -1,    -1,
     747,   951,   952,   953,    -1,    -1,   956,   957,   958,   959,
     960,   961,   962,   963,   964,   965,   966,   967,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   977,   978,   979,
     980,   981,   982,    -1,    -1,   203,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   799,    -1,   154,   155,  1006,   157,   158,   159,
      -1,    -1,    -1,    -1,    -1,   812,    -1,    -1,    -1,    -1,
      -1,   818,    -1,   820,    -1,   822,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,    -1,   846,
      -1,    -1,   202,    -1,   204,    -1,    -1,    -1,    -1,   856,
      -1,    -1,   859,    -1,   861,    -1,    -1,    -1,   865,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   875,    -1,
      -1,    -1,    -1,   709,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   723,   724,   725,
     726,   727,    -1,   900,    -1,    -1,    -1,  1107,    -1,    -1,
     736,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1122,    -1,  1124,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,  1147,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,   984,   985,   986,
      -1,    -1,    -1,   990,   991,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,  1211,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1018,   848,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,  1043,    53,    -1,    -1,
      -1,  1048,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,   472,    -1,  1061,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1070,  1274,  1072,    -1,    -1,    -1,  1279,
      -1,  1281,    -1,  1283,    -1,    -1,  1286,    -1,  1288,    -1,
      -1,  1291,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1101,    -1,    -1,   517,    -1,    -1,
      -1,    -1,    -1,    -1,  1111,    -1,    -1,    -1,    -1,    -1,
     203,   947,    -1,    -1,  1324,   951,   952,   953,    -1,    -1,
     956,   957,   958,   959,   960,   961,   962,   963,   964,   965,
     966,   967,   968,   969,   970,   971,   972,   973,   974,   975,
     976,   977,   978,   979,   980,   981,   982,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1364,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1006,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,
    1197,    -1,    -1,    -1,  1201,    -1,  1203,  1204,    -1,    -1,
      -1,    -1,  1412,    -1,    -1,    -1,    -1,    -1,  1215,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1223,  1224,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1233,    10,    11,    12,
      -1,    -1,    -1,  1443,  1444,  1445,    -1,    -1,    -1,    -1,
    1450,    -1,    -1,    -1,  1454,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,  1107,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,  1292,  1122,    -1,  1124,   709,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,   723,   724,   725,   726,    -1,    -1,    -1,
      -1,  1147,    -1,    -1,    -1,    -1,   736,    -1,    -1,    29,
    1327,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,    29,  1558,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    55,    -1,  1211,    -1,    -1,    -1,    -1,
      -1,    -1,  1592,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,  1603,    -1,    77,    -1,    -1,  1608,    -1,
      -1,  1611,    -1,    -1,    -1,    -1,    -1,    -1,  1415,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   201,    -1,
      -1,    -1,    -1,    -1,  1634,    -1,    -1,    -1,   848,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1274,  1446,
      -1,    -1,    -1,  1279,    -1,  1281,    -1,  1283,   130,   131,
    1286,    -1,  1288,    -1,    -1,  1291,    -1,    -1,    -1,  1669,
      -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1324,    -1,
     172,   201,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,  1724,    -1,    -1,   199,    -1,    -1,
      -1,   203,  1732,    -1,    -1,    -1,  1736,    -1,  1364,  1739,
      -1,   951,   952,   953,    -1,    -1,   956,   957,   958,   959,
     960,   961,   962,   963,   964,   965,   966,   967,   968,   969,
     970,   971,   972,   973,   974,   975,   976,   977,   978,   979,
     980,   981,   982,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1412,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1006,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1443,  1444,  1445,
      -1,    -1,    -1,    29,  1450,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,  1107,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1122,    -1,  1124,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1558,    -1,    -1,    -1,    29,  1147,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,  1592,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    77,  1603,    79,    -1,
      -1,    -1,  1608,    -1,    -1,  1611,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,
      -1,  1211,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   201,
      -1,    65,    -1,  1669,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   154,   155,    -1,   157,   158,   159,    -1,
      -1,    -1,    -1,    -1,  1274,    -1,    -1,    -1,    -1,  1279,
      -1,  1281,    -1,  1283,    -1,    -1,  1286,    -1,  1288,    -1,
      -1,  1291,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,    -1,    -1,  1724,    -1,
      -1,   202,    -1,   204,    -1,    -1,  1732,   200,    -1,    -1,
    1736,    -1,    -1,  1739,  1324,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,  1364,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,  1412,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1443,  1444,  1445,   104,    -1,    -1,    -1,
    1450,    -1,    -1,    -1,   112,   113,   114,   115,   116,   117,
      -1,    -1,   120,   121,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,   175,    -1,    -1,
      -1,    -1,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,    -1,
      -1,   199,    -1,    -1,    -1,    -1,   204,   205,    -1,   207,
     208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1558,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1592,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,  1603,    -1,    -1,    -1,    -1,  1608,    -1,
      -1,  1611,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,    88,
      -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,  1669,
      -1,    -1,   101,   102,   103,   104,   105,   106,   107,    -1,
     109,   110,   111,   112,   113,   114,   115,   116,   117,    -1,
     119,   120,   121,   122,   123,   124,    -1,    -1,    -1,    -1,
     129,   130,    -1,   132,   133,   134,   135,   136,    -1,    -1,
      -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,
     149,   150,   151,   152,  1724,   154,   155,    -1,   157,   158,
     159,   160,  1732,    -1,   163,    -1,  1736,   166,    -1,  1739,
      -1,    -1,    -1,   172,   173,   174,   175,   176,    -1,   178,
      -1,   180,   181,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
     199,    -1,   201,   202,   203,   204,   205,    -1,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    -1,
      -1,    -1,    85,    86,    87,    88,    -1,    90,    -1,    92,
      -1,    94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,
     103,   104,   105,   106,   107,    -1,   109,   110,   111,   112,
     113,   114,   115,   116,   117,    -1,   119,   120,   121,   122,
     123,   124,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,
     133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,
     143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,   160,    -1,    -1,
     163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,
     173,   174,   175,   176,    -1,   178,    -1,   180,   181,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,   199,    -1,   201,   202,
     203,   204,   205,    -1,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,
      87,    88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,
      97,    -1,    -1,    -1,   101,   102,   103,   104,   105,   106,
     107,    -1,   109,   110,   111,   112,   113,   114,   115,   116,
     117,    -1,   119,   120,   121,   122,   123,   124,    -1,    -1,
      -1,    -1,   129,   130,    -1,   132,   133,   134,   135,   136,
      -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,
      -1,   148,   149,   150,   151,   152,    -1,   154,   155,    -1,
     157,   158,   159,   160,    -1,    -1,   163,    -1,    -1,   166,
      -1,    -1,    -1,    -1,    -1,   172,   173,   174,   175,   176,
      -1,   178,    -1,   180,   181,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,   199,    -1,   201,   202,    -1,   204,   205,    -1,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      81,    -1,    -1,    -1,    85,    86,    87,    88,    -1,    90,
      -1,    92,    -1,    94,    -1,    -1,    97,    -1,    -1,    -1,
     101,   102,   103,   104,    -1,   106,   107,    -1,   109,    -1,
     111,   112,   113,   114,   115,   116,   117,    -1,   119,   120,
     121,    -1,   123,   124,    -1,    -1,    -1,    -1,   129,   130,
      -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,
      -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,
     151,   152,    -1,   154,   155,    -1,   157,   158,   159,   160,
      -1,    -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,
      -1,   172,   173,   174,   175,    -1,    -1,    -1,    -1,   180,
     181,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,   199,    -1,
     201,   202,   203,   204,   205,    -1,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,
      85,    86,    87,    88,    -1,    90,    -1,    92,    -1,    94,
      -1,    -1,    97,    -1,    -1,    -1,   101,   102,   103,   104,
      -1,   106,   107,    -1,   109,    -1,   111,   112,   113,   114,
     115,   116,   117,    -1,   119,   120,   121,    -1,   123,   124,
      -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,
     135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,
      -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,   160,    -1,    -1,   163,    -1,
      -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,   174,
     175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,   199,    -1,   201,   202,   203,   204,
     205,    -1,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,    88,
      -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,    -1,
      -1,    -1,   101,   102,   103,   104,    -1,   106,   107,    -1,
     109,    -1,   111,   112,   113,   114,   115,   116,   117,    -1,
     119,   120,   121,    -1,   123,   124,    -1,    -1,    -1,    -1,
     129,   130,    -1,   132,   133,   134,   135,   136,    -1,    -1,
      -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,
     149,   150,   151,   152,    -1,   154,   155,    -1,   157,   158,
     159,   160,    -1,    -1,   163,    -1,    -1,   166,    -1,    -1,
      -1,    -1,    -1,   172,   173,   174,   175,    -1,    -1,    -1,
      -1,   180,   181,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
     199,    -1,   201,   202,   203,   204,   205,    -1,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    -1,
      -1,    -1,    85,    86,    87,    88,    89,    90,    -1,    92,
      -1,    94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,
     103,   104,    -1,   106,   107,    -1,   109,    -1,   111,   112,
     113,   114,   115,   116,   117,    -1,   119,   120,   121,    -1,
     123,   124,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,
     133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,
     143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,   160,    -1,    -1,
     163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,
     173,   174,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,   199,    -1,   201,   202,
      -1,   204,   205,    -1,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,
      87,    88,    -1,    90,    -1,    92,    -1,    94,    95,    -1,
      97,    -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,
     107,    -1,   109,    -1,   111,   112,   113,   114,   115,   116,
     117,    -1,   119,   120,   121,    -1,   123,   124,    -1,    -1,
      -1,    -1,   129,   130,    -1,   132,   133,   134,   135,   136,
      -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,
      -1,   148,   149,   150,   151,   152,    -1,   154,   155,    -1,
     157,   158,   159,   160,    -1,    -1,   163,    -1,    -1,   166,
      -1,    -1,    -1,    -1,    -1,   172,   173,   174,   175,    -1,
      -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,   199,    -1,   201,   202,    -1,   204,   205,    -1,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      81,    -1,    -1,    -1,    85,    86,    87,    88,    -1,    90,
      -1,    92,    -1,    94,    -1,    -1,    97,    -1,    -1,    -1,
     101,   102,   103,   104,    -1,   106,   107,    -1,   109,    -1,
     111,   112,   113,   114,   115,   116,   117,    -1,   119,   120,
     121,    -1,   123,   124,    -1,    -1,    -1,    -1,   129,   130,
      -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,
      -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,
     151,   152,    -1,   154,   155,    -1,   157,   158,   159,   160,
      -1,    -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,
      -1,   172,   173,   174,   175,    -1,    -1,    -1,    -1,   180,
     181,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,   199,    -1,
     201,   202,   203,   204,   205,    -1,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,
      85,    86,    87,    88,    -1,    90,    -1,    92,    -1,    94,
      -1,    -1,    97,    -1,    -1,    -1,   101,   102,   103,   104,
      -1,   106,   107,    -1,   109,    -1,   111,   112,   113,   114,
     115,   116,   117,    -1,   119,   120,   121,    -1,   123,   124,
      -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,
     135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,
      -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,   160,    -1,    -1,   163,    -1,
      -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,   174,
     175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,   199,    -1,   201,   202,   203,   204,
     205,    -1,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,    88,
      -1,    90,    -1,    92,    93,    94,    -1,    -1,    97,    -1,
      -1,    -1,   101,   102,   103,   104,    -1,   106,   107,    -1,
     109,    -1,   111,   112,   113,   114,   115,   116,   117,    -1,
     119,   120,   121,    -1,   123,   124,    -1,    -1,    -1,    -1,
     129,   130,    -1,   132,   133,   134,   135,   136,    -1,    -1,
      -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,
     149,   150,   151,   152,    -1,   154,   155,    -1,   157,   158,
     159,   160,    -1,    -1,   163,    -1,    -1,   166,    -1,    -1,
      -1,    -1,    -1,   172,   173,   174,   175,    -1,    -1,    -1,
      -1,   180,   181,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
     199,    -1,   201,   202,    -1,   204,   205,    -1,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    -1,
      -1,    -1,    85,    86,    87,    88,    -1,    90,    -1,    92,
      -1,    94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,
     103,   104,    -1,   106,   107,    -1,   109,    -1,   111,   112,
     113,   114,   115,   116,   117,    -1,   119,   120,   121,    -1,
     123,   124,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,
     133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,
     143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,   160,    -1,    -1,
     163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,
     173,   174,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,   199,    -1,   201,   202,
     203,   204,   205,    -1,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,
      87,    88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,
      97,    -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,
     107,    -1,   109,    -1,   111,   112,   113,   114,   115,   116,
     117,    -1,   119,   120,   121,    -1,   123,   124,    -1,    -1,
      -1,    -1,   129,   130,    -1,   132,   133,   134,   135,   136,
      -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,
      -1,   148,   149,   150,   151,   152,    -1,   154,   155,    -1,
     157,   158,   159,   160,    -1,    -1,   163,    -1,    -1,   166,
      -1,    -1,    -1,    -1,    -1,   172,   173,   174,   175,    -1,
      -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,   199,    -1,   201,   202,   203,   204,   205,    -1,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      81,    -1,    -1,    -1,    85,    86,    87,    88,    -1,    90,
      91,    92,    -1,    94,    -1,    -1,    97,    -1,    -1,    -1,
     101,   102,   103,   104,    -1,   106,   107,    -1,   109,    -1,
     111,   112,   113,   114,   115,   116,   117,    -1,   119,   120,
     121,    -1,   123,   124,    -1,    -1,    -1,    -1,   129,   130,
      -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,
      -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,
     151,   152,    -1,   154,   155,    -1,   157,   158,   159,   160,
      -1,    -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,
      -1,   172,   173,   174,   175,    -1,    -1,    -1,    -1,   180,
     181,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,   199,    -1,
     201,   202,    -1,   204,   205,    -1,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,
      85,    86,    87,    88,    -1,    90,    -1,    92,    -1,    94,
      -1,    -1,    97,    -1,    -1,    -1,   101,   102,   103,   104,
      -1,   106,   107,    -1,   109,    -1,   111,   112,   113,   114,
     115,   116,   117,    -1,   119,   120,   121,    -1,   123,   124,
      -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,
     135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,
      -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,   160,    -1,    -1,   163,    -1,
      -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,   174,
     175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,   199,    -1,   201,   202,   203,   204,
     205,    -1,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,    88,
      -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,    -1,
      -1,    -1,   101,   102,   103,   104,    -1,   106,   107,    -1,
     109,    -1,   111,   112,   113,   114,   115,   116,   117,    -1,
     119,   120,   121,    -1,   123,   124,    -1,    -1,    -1,    -1,
     129,   130,    -1,   132,   133,   134,   135,   136,    -1,    -1,
      -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,
     149,   150,   151,   152,    -1,   154,   155,    -1,   157,   158,
     159,   160,    -1,    -1,   163,    -1,    -1,   166,    -1,    -1,
      -1,    -1,    -1,   172,   173,   174,   175,    -1,    -1,    -1,
      -1,   180,   181,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
     199,    -1,   201,   202,   203,   204,   205,    -1,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    -1,
      -1,    -1,    85,    86,    87,    88,    -1,    90,    -1,    92,
      -1,    94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,
     103,   104,    -1,   106,   107,    -1,   109,    -1,   111,   112,
     113,   114,   115,   116,   117,    -1,   119,   120,   121,    -1,
     123,   124,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,
     133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,
     143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,   160,    -1,    -1,
     163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,
     173,   174,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,   199,    -1,   201,   202,
     203,   204,   205,    -1,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,
      87,    88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,
      97,    -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,
     107,    -1,   109,    -1,   111,   112,   113,   114,   115,   116,
     117,    -1,   119,   120,   121,    -1,   123,   124,    -1,    -1,
      -1,    -1,   129,   130,    -1,   132,   133,   134,   135,   136,
      -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,
      -1,   148,   149,   150,   151,   152,    -1,   154,   155,    -1,
     157,   158,   159,   160,    -1,    -1,   163,    -1,    -1,   166,
      -1,    -1,    -1,    -1,    -1,   172,   173,   174,   175,    -1,
      -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,   199,    -1,   201,   202,    -1,   204,   205,    -1,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    30,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      81,    -1,    -1,    -1,    85,    86,    87,    88,    -1,    90,
      -1,    92,    -1,    94,    -1,    -1,    97,    -1,    -1,    -1,
     101,   102,   103,   104,    -1,   106,   107,    -1,   109,    -1,
     111,   112,   113,   114,   115,   116,   117,    -1,   119,   120,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,
      -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,
      -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,
     151,   152,    -1,   154,   155,    -1,   157,   158,   159,    -1,
      -1,    -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,
      -1,   172,   173,   174,   175,    -1,    -1,    -1,    -1,   180,
     181,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,   199,    -1,
     201,   202,    -1,   204,   205,    -1,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,
      85,    86,    87,    88,    -1,    90,    -1,    92,    -1,    94,
      -1,    -1,    97,    -1,    -1,    -1,   101,   102,   103,   104,
      -1,   106,   107,    -1,   109,    -1,   111,   112,   113,   114,
     115,   116,   117,    -1,   119,   120,   121,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,
     135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,
      -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,    -1,    -1,    -1,   163,    -1,
      -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,   174,
     175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,   199,    -1,   201,   202,    -1,   204,
     205,    -1,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,    88,
      -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,    -1,
      -1,    -1,   101,   102,   103,   104,    -1,   106,   107,    -1,
     109,    -1,   111,   112,   113,   114,   115,   116,   117,    -1,
     119,   120,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     129,   130,    -1,   132,   133,   134,   135,   136,    -1,    -1,
      -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,
     149,   150,   151,   152,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    -1,   163,    -1,    -1,   166,    -1,    -1,
      -1,    -1,    -1,   172,   173,   174,   175,    -1,    -1,    -1,
      -1,   180,   181,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
     199,    -1,   201,   202,    -1,   204,   205,    -1,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    30,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    70,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    -1,
      -1,    -1,    85,    86,    87,    88,    -1,    90,    -1,    92,
      -1,    94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,
     103,   104,    -1,   106,   107,    -1,   109,    -1,   111,   112,
     113,   114,   115,   116,   117,    -1,   119,   120,   121,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,
     133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,
     143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,
     163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,
     173,   174,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,   199,    -1,   201,   202,
      -1,   204,   205,    -1,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,
      87,    88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,
      97,    -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,
     107,    -1,   109,    -1,   111,   112,   113,   114,   115,   116,
     117,    -1,   119,   120,   121,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,   130,    -1,   132,   133,   134,   135,   136,
      -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,
      -1,   148,   149,   150,   151,   152,    -1,   154,   155,    -1,
     157,   158,   159,    -1,    -1,    -1,   163,    -1,    -1,   166,
      -1,    -1,    -1,    -1,    -1,   172,   173,   174,   175,    -1,
      -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,   199,    -1,   201,   202,    -1,   204,   205,    -1,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      81,    -1,    -1,    -1,    85,    86,    87,    88,    -1,    90,
      -1,    92,    -1,    94,    -1,    -1,    97,    -1,    -1,    -1,
     101,   102,   103,   104,    -1,   106,   107,    -1,   109,    -1,
     111,   112,   113,   114,   115,   116,   117,    -1,   119,   120,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,
      -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,
      -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,
     151,   152,    -1,   154,   155,    -1,   157,   158,   159,    -1,
      -1,    -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,
      -1,   172,   173,   174,   175,    -1,    -1,    -1,    -1,   180,
     181,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,   199,    -1,
     201,   202,    -1,   204,   205,    -1,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,   113,   114,
     115,   116,   117,    -1,    -1,   120,   121,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,
     135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,
      -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,    -1,    -1,    -1,   163,    -1,
      -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,   174,
     175,    -1,    -1,    -1,    -1,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,   199,    -1,   201,    -1,    -1,   204,
     205,    -1,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    29,    13,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    35,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    65,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   112,   113,   114,   115,   116,   117,    -1,
      -1,   120,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     129,   130,    -1,   132,   133,   134,   135,   136,    -1,    -1,
      -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,
     149,   150,   151,   152,    -1,   154,   155,    -1,   157,   158,
     159,    -1,   161,    -1,   163,    -1,    -1,   166,    -1,    -1,
      -1,    -1,    -1,   172,   173,   174,   175,    -1,    -1,    -1,
      -1,   180,   181,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
     199,    -1,    -1,    -1,    -1,   204,   205,    -1,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,
     113,   114,   115,   116,   117,    -1,    -1,   120,   121,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,
     133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,
     143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,
     163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,
     173,   174,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,   199,    11,    12,   202,
      -1,   204,   205,    -1,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    29,    13,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    35,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    65,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   112,   113,   114,   115,   116,
     117,    -1,    -1,   120,   121,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,   130,    -1,   132,   133,   134,   135,   136,
      -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,
      -1,   148,   149,   150,   151,   152,    -1,   154,   155,    -1,
     157,   158,   159,    -1,   161,    -1,   163,    -1,    -1,   166,
      -1,    -1,    -1,    -1,    -1,   172,   173,   174,   175,    -1,
      -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,   199,    -1,    -1,    -1,    -1,   204,   205,    -1,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,   113,   114,   115,   116,   117,    -1,    -1,   120,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,
      -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,
      -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,
     151,   152,    -1,   154,   155,    -1,   157,   158,   159,    -1,
      -1,    -1,   163,    -1,    -1,   166,     3,     4,     5,     6,
       7,   172,   173,   174,   175,    -1,    13,    -1,    -1,   180,
     181,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,   199,    -1,
      -1,    -1,    -1,   204,   205,    -1,   207,   208,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   101,    -1,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   112,   113,   114,   115,   116,
     117,    -1,    -1,   120,   121,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,   130,    -1,   132,   133,   134,   135,   136,
      -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,
      -1,   148,   149,   150,   151,   152,    -1,   154,   155,    -1,
     157,   158,   159,    -1,    -1,    -1,   163,    -1,    -1,   166,
      -1,    -1,    -1,    -1,    -1,   172,   173,   174,   175,    -1,
      -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,   199,    11,    12,    -1,    -1,   204,   205,    -1,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    29,    13,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    65,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,   113,   114,   115,   116,   117,    -1,    -1,   120,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,
      -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,
      -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,
     151,   152,    -1,   154,   155,    -1,   157,   158,   159,    -1,
      -1,    -1,   163,    -1,    -1,   166,     3,     4,     5,     6,
       7,   172,   173,   174,   175,    -1,    13,    -1,    -1,   180,
     181,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,   199,    -1,
      -1,    -1,    -1,   204,   205,    -1,   207,   208,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   112,   113,   114,   115,   116,
     117,    -1,    -1,   120,   121,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,   130,    -1,   132,   133,   134,   135,   136,
      -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,
      -1,   148,   149,   150,   151,   152,    -1,   154,   155,    -1,
     157,   158,   159,    -1,    -1,    -1,   163,    -1,    -1,   166,
       3,     4,     5,     6,     7,   172,   173,   174,   175,    -1,
      13,    -1,    -1,   180,   181,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,   199,    -1,   201,    -1,    -1,   204,   205,    -1,
     207,   208,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,
     113,   114,   115,   116,   117,    -1,    -1,   120,   121,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,
     133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,
     143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,
     163,    -1,    -1,   166,     3,     4,     5,     6,     7,   172,
     173,   174,   175,    -1,    13,    -1,    -1,   180,   181,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,   199,    -1,   201,    -1,
      -1,   204,   205,    -1,   207,   208,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   112,   113,   114,   115,   116,   117,    -1,
      -1,   120,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     129,   130,    -1,   132,   133,   134,   135,   136,    -1,    -1,
      -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,
     149,   150,   151,   152,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    -1,   163,    -1,    -1,   166,    -1,    -1,
      -1,    -1,    -1,   172,   173,   174,   175,    -1,    -1,    -1,
      -1,   180,   181,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
     199,   200,    -1,    -1,    -1,   204,   205,    -1,   207,   208,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    30,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,
     113,   114,   115,   116,   117,    -1,    -1,   120,   121,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,
     133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,
     143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,
     163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,
     173,   174,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,    -1,    -1,   199,    -1,    12,    -1,
      -1,   204,   205,    -1,   207,   208,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    29,    13,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    35,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    65,    -1,    -1,    -1,    52,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   112,   113,   114,   115,   116,
     117,    -1,    -1,   120,   121,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,   130,    -1,   132,   133,   134,   135,   136,
      -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,
      -1,   148,   149,   150,   151,   152,    -1,   154,   155,    -1,
     157,   158,   159,    -1,    -1,    -1,   163,    -1,    -1,   166,
      -1,    -1,    -1,    -1,    -1,   172,   173,   174,   175,    -1,
      -1,    -1,    -1,   180,   181,    -1,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
      -1,    -1,   199,    -1,    12,    -1,    -1,   204,   205,    -1,
     207,   208,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    29,    13,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    65,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,   113,   114,   115,   116,   117,    -1,    -1,   120,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,
      -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,
      -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,
     151,   152,    -1,   154,   155,    -1,   157,   158,   159,    -1,
      -1,    -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,
      -1,   172,   173,   174,   175,    -1,    -1,    -1,    -1,   180,
     181,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,   199,    -1,
      -1,    -1,    -1,   204,   205,    -1,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    29,    13,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    47,    65,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,   113,   114,
     115,   116,   117,    -1,    -1,   120,   121,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,
     135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,
      -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,    -1,    -1,    -1,   163,    -1,
      -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,   174,
     175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,   199,    -1,    -1,    -1,    -1,   204,
     205,    -1,   207,   208,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    35,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    65,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   112,   113,   114,   115,   116,   117,    -1,
      -1,   120,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     129,   130,    -1,   132,   133,   134,   135,   136,    -1,    -1,
      -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,
     149,   150,   151,   152,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    -1,   163,    -1,    -1,   166,     3,     4,
       5,     6,     7,   172,   173,   174,   175,    -1,    13,    -1,
      -1,   180,   181,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,    -1,    -1,
     199,    -1,    -1,    -1,    -1,   204,   205,    -1,   207,   208,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,   113,   114,
     115,   116,   117,    -1,    -1,   120,   121,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,
     135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,
      -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,    -1,    -1,    -1,   163,    -1,
      -1,   166,     3,     4,     5,     6,     7,   172,   173,   174,
     175,    -1,    13,    -1,    -1,   180,   181,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,    -1,    -1,   199,    -1,    -1,    -1,    -1,   204,
     205,    -1,   207,   208,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,   113,   114,   115,   116,   117,    -1,    -1,   120,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,
      -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,
      -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,
     151,   152,    -1,   154,   155,    -1,   157,   158,   159,    -1,
      -1,    -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,
      -1,   172,   173,   174,   175,    -1,    -1,    -1,    -1,   180,
     181,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,    -1,    -1,   199,    -1,
      -1,    -1,    -1,   204,   205,    -1,   207,   208,     3,     4,
       5,     6,     7,    -1,    -1,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    -1,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    67,    68,    69,    70,    71,    72,    73,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,    -1,    -1,   129,   130,    -1,   132,   133,   134,
     135,   136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   148,   149,   150,    -1,    -1,    -1,   154,
     155,    -1,   157,   158,   159,   160,    -1,   162,   163,    -1,
     165,    -1,    -1,    -1,    -1,    -1,    -1,   172,    -1,    -1,
      -1,   176,    -1,   178,    -1,   180,   181,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    77,    -1,    79,    -1,   195,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    29,   117,    -1,    -1,    -1,   191,   192,    -1,    -1,
      -1,    -1,    65,    -1,   129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,
      -1,    -1,    -1,   148,    -1,    -1,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,    -1,    -1,   188,    -1,    77,
      -1,    -1,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    65,    -1,    -1,   199,    -1,    -1,    -1,    -1,   204,
     187,    -1,    77,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,
     148,    -1,    -1,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,    -1,   186,    -1,    -1,   121,    -1,    -1,    -1,
      -1,    77,    -1,    -1,   172,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   104,   154,
     155,   199,   157,   158,   159,   203,   112,   113,   114,   115,
     116,   117,    -1,    -1,    -1,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   130,   131,    -1,    -1,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    55,   148,    -1,   199,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    29,   172,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   181,    -1,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     104,    55,    -1,   199,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    77,    53,    -1,   130,   131,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   148,    29,    -1,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   172,    -1,
      -1,    55,    -1,    -1,    -1,    -1,   130,   131,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    77,   148,   199,    -1,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,    -1,   161,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   172,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,    -1,   199,   130,   131,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   148,    -1,    -1,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,    77,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,   172,    -1,
      -1,    -1,    -1,   117,    -1,    -1,    30,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    46,    47,    -1,   199,    -1,    -1,    52,    -1,
      54,    -1,    -1,    -1,   148,    -1,    -1,   151,   152,    -1,
     154,   155,    66,   157,   158,   159,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    -1,    -1,    -1,   148,    -1,    -1,
     151,    85,    -1,   154,   155,    -1,   157,   158,   159,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,    -1,   199,    -1,    -1,    -1,    -1,
     204,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,   130,    -1,   132,   133,
     134,   135,   136,    77,    -1,    79,    -1,    -1,    -1,   143,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,    77,    -1,    -1,   163,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   172,   173,
     174,   175,    -1,   117,    -1,    -1,   180,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    46,    47,    -1,   199,    -1,    -1,    52,    -1,
      54,    -1,    -1,    -1,   148,    -1,    -1,   151,   152,    -1,
     154,   155,    66,   157,   158,   159,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,    -1,
     151,    85,    -1,   154,   155,    -1,   157,   158,   159,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,    -1,   199,    -1,    -1,    -1,    -1,
     204,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,   130,    -1,   132,   133,
     134,   135,   136,    -1,    -1,    -1,    46,    47,    -1,   143,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,    66,    -1,    -1,   163,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,   172,   173,
     174,   175,    -1,    -1,    -1,    85,   180,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,    -1,   199,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    77,    -1,    79,    -1,    -1,
     130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,   143,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    -1,    -1,   154,   155,    -1,   157,   158,   159,
      -1,    -1,    -1,    -1,    -1,   117,    -1,    -1,    -1,    -1,
      -1,    -1,   172,    -1,    -1,    -1,    -1,   129,    -1,    -1,
      -1,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   148,    -1,    -1,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,    77,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,
      -1,    -1,   152,    -1,   154,   155,   156,   157,   158,   159,
      77,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,    -1,   199,   117,    -1,
      77,    -1,   204,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,    -1,   199,
      74,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,   148,
      -1,    -1,   151,   152,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,   155,    -1,
     157,   158,   159,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   154,   155,    -1,
     157,   158,   159,    -1,    -1,   204,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
     154,   155,   199,   157,   158,   159,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,    -1,   199,    10,    11,    12,    -1,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   128,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,   128,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   128,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,   128,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   128,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,   128,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   128,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    29,   128,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,   128,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    29,   128,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    65,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    29,   128,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   118,    65,    -1,    -1,    77,    -1,    -1,
      -1,   128,    -1,    -1,   148,   130,   131,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,    77,    -1,    79,    80,
      -1,    -1,    -1,   148,   104,   105,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,    -1,    -1,    -1,    -1,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,    -1,    -1,    -1,   128,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   151,    77,    -1,   154,   155,    -1,   157,   158,   159,
      -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   154,   155,    -1,   157,   158,   159,    -1,
      -1,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,    -1,    -1,
      -1,    -1,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    77,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   152,    -1,   154,
     155,    -1,   157,   158,   159,    -1,    -1,    -1,    77,   151,
      -1,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   123,    -1,    -1,    -1,    -1,   151,
      -1,    -1,   154,   155,    -1,   157,   158,   159,    -1,    77,
      -1,    -1,    -1,   123,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   154,   155,    -1,   157,   158,
     159,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   154,   155,    -1,   157,   158,   159,
      -1,    -1,    -1,    -1,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,    -1,
      -1,    -1,    -1,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    28,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    96,    53,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65
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
     467,   478,   479,   182,   201,   337,   340,   367,   369,   202,
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
     216,   278,   235,   201,   201,   201,   479,   479,   168,   199,
     104,   479,    14,   216,    79,   201,   201,   201,   182,   183,
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
     470,   471,   479,    35,   161,   290,   291,   340,   467,   199,
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
       9,   200,    30,   202,   279,   479,    85,   227,   475,   476,
     477,   199,     9,    46,    47,    52,    54,    66,    85,   130,
     143,   152,   172,   173,   174,   175,   199,   224,   225,   227,
     362,   363,   364,   398,   404,   405,   406,   185,    79,   340,
      79,    79,   340,   379,   380,   340,   340,   372,   382,   188,
     385,   229,   199,   239,   223,   201,     9,    96,   223,   201,
       9,    96,    96,   220,   216,   340,   293,   405,    79,     9,
     200,   200,   200,   200,   200,   201,   216,   474,   125,   269,
     199,     9,   200,   200,    79,    80,   216,   461,   216,    66,
     203,   203,   212,   214,    30,   126,   268,   167,    50,   152,
     167,   392,   128,     9,   409,   200,   147,   200,     9,   409,
     128,   200,     9,   409,   200,   479,   479,    14,   351,   288,
     197,     9,   410,   479,   480,   127,   427,   428,   429,   203,
       9,   409,   169,   432,   340,   200,     9,   410,    14,   344,
     249,   125,   267,   199,   468,   340,    30,   206,   206,   128,
     203,     9,   409,   340,   469,   199,   259,   254,   262,   257,
     246,    68,   432,   340,   469,   199,   206,   203,   200,   206,
     203,   200,    46,    47,    66,    74,    75,    76,    85,   130,
     143,   172,   216,   412,   414,   417,   420,   216,   432,   432,
     128,   427,   428,   429,   200,   340,   283,    71,    72,   284,
     229,   331,   229,   333,    96,    35,   129,   273,   432,   405,
     216,    30,   231,   277,   201,   280,   201,   280,     9,   169,
     128,   147,     9,   409,   200,   161,   470,   471,   472,   470,
     405,   405,   405,   405,   405,   408,   411,   199,    84,   147,
     199,   199,   199,   199,   405,   147,   202,    10,    11,    12,
      29,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    65,   340,   185,   185,    14,   191,   192,   381,
       9,   195,   385,    79,   203,   398,   202,   243,    96,   221,
     216,    96,   222,   216,   216,   203,    14,   432,   201,    96,
       9,   169,   270,   398,   202,   444,   129,   432,    14,   206,
     340,   203,   212,   479,   270,   202,   391,    14,   340,   352,
     216,   340,   340,   340,   201,   479,   197,    30,   473,   426,
      35,    79,   161,   202,   216,   437,   479,    35,   161,   340,
     405,   288,   199,   398,   268,   345,   250,   340,   340,   340,
     203,   199,   290,   269,    30,   268,   267,   468,   400,   203,
     199,   290,    14,    74,    75,    76,   216,   413,   413,   414,
     415,   416,   199,    84,   144,   199,     9,   409,   200,   421,
      35,   340,   203,    71,    72,   285,   331,   231,   203,   201,
      89,   201,   273,   432,   199,   128,   272,    14,   229,   280,
      98,    99,   100,   280,   203,   479,   479,   216,   475,     9,
     200,   409,   128,   206,     9,   409,   408,   216,   352,   354,
     356,   405,   456,   458,   405,   455,   457,   455,   200,   123,
     216,   405,   449,   450,   405,   405,   405,    30,   405,   405,
     405,   405,   405,   405,   405,   405,   405,   405,   405,   405,
     405,   405,   405,   405,   405,   405,   405,   405,   405,   405,
     405,   340,   340,   340,   380,   340,   370,    79,   244,   216,
     216,   405,   479,   216,     9,   298,   200,   199,   334,   337,
     340,   206,   203,   465,   298,   153,   166,   202,   387,   394,
     153,   202,   393,   128,   128,   201,   473,   479,   351,   480,
      79,    14,    79,   340,   469,   199,   432,   340,   200,   288,
     202,   288,   199,   128,   199,   290,   200,   202,   479,   202,
     268,   251,   403,   199,   290,   200,   128,   206,     9,   409,
     415,   144,   352,   418,   419,   414,   432,   331,    30,    73,
     231,   201,   333,   272,   444,   273,   200,   405,    95,    98,
     201,   340,    30,   201,   281,   203,   169,   128,   161,    30,
     200,   405,   405,   200,   128,     9,   409,   200,   200,     9,
     409,   128,   200,     9,   409,   200,   128,   203,     9,   409,
     405,    30,   186,   200,   229,    96,   398,     4,   105,   110,
     118,   154,   155,   157,   203,   299,   322,   323,   324,   329,
     425,   444,   203,   202,   203,    50,   340,   340,   340,   340,
     351,    35,    79,   161,    14,   405,   203,   199,   290,   473,
     200,   298,   200,   288,   340,   290,   200,   298,   465,   298,
     202,   199,   290,   200,   414,   414,   200,   128,   200,     9,
     409,    30,   229,   201,   200,   200,   200,   236,   201,   201,
     281,   229,   479,   479,   128,   405,   352,   405,   405,   405,
     405,   405,   405,   340,   202,   203,   479,   125,   126,   467,
     271,   398,   118,   130,   131,   152,   158,   308,   309,   310,
     398,   156,   314,   315,   121,   199,   216,   316,   317,   300,
     247,   479,     9,   201,   323,   200,   295,   152,   389,   203,
     203,    79,    14,    79,   405,   199,   290,   200,   110,   342,
     473,   203,   473,   200,   200,   203,   202,   203,   298,   288,
     200,   128,   414,   352,   229,   234,   237,    30,   231,   275,
     229,   200,   405,   128,   128,   128,   187,   229,   398,   398,
      14,     9,   201,   202,   202,     9,   201,     3,     4,     5,
       6,     7,    10,    11,    12,    13,    27,    28,    53,    67,
      68,    69,    70,    71,    72,    73,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   129,   130,   132,
     133,   134,   135,   136,   148,   149,   150,   160,   162,   163,
     165,   172,   176,   178,   180,   181,   216,   395,   396,     9,
     201,   152,   156,   216,   317,   318,   319,   201,    79,   328,
     246,   301,   467,   247,   203,   296,   297,   467,    14,   405,
     290,   200,   199,   202,   201,   202,   320,   342,   473,   295,
     203,   200,   414,   128,    30,   231,   274,   275,   229,   405,
     405,   405,   340,   203,   201,   201,   405,   398,   304,   311,
     404,   309,    14,    30,    47,   312,   315,     9,    33,   200,
      29,    46,    49,    14,     9,   201,   468,   328,    14,   246,
     201,    14,   405,   200,    35,    79,   386,   229,   229,   202,
     320,   203,   473,   414,   229,    93,   188,   242,   203,   216,
     227,   305,   306,   307,     9,   203,   405,   396,   396,    55,
     313,   318,   318,    29,    46,    49,   405,    79,   199,   201,
     405,   468,   405,    79,     9,   410,   203,   203,   229,   320,
      91,   201,    79,   108,   238,   147,    96,   404,   159,    14,
     302,   199,    35,    79,   200,   203,   201,   199,   165,   245,
     216,   323,   324,   405,   286,   287,   426,   303,    79,   398,
     243,   162,   216,   201,   200,     9,   410,   112,   113,   114,
     326,   327,   286,    79,   271,   201,   473,   426,   480,   200,
     200,   201,   201,   202,   321,   326,    35,    79,   161,   473,
     202,   229,   480,    79,    14,    79,   321,   229,   203,    35,
      79,   161,    14,   405,   203,    79,    14,    79,   405,    14,
     405,   405
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
#line 733 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 736 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 743 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 744 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 747 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 748 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 749 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 750 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 751 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 752 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 753 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 756 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 758 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 759 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 760 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 761 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 762 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 764 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 766 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 767 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 778 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 780 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 781 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 782 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 783 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 795 "hphp.y"
    { ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 796 "hphp.y"
    { ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 801 "hphp.y"
    { ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 802 "hphp.y"
    { ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 807 "hphp.y"
    { ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 808 "hphp.y"
    { ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 812 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 813 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 814 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 816 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 820 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 821 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 822 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 824 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 828 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 829 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 830 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 832 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 843 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 844 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 847 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 854 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 861 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 869 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 872 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 878 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 879 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 882 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 883 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 884 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 885 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 888 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 892 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 897 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 898 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 900 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 904 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 907 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 911 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 913 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 916 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 918 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 921 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 922 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 923 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 924 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 925 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 926 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 928 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 929 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 930 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 931 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 933 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 936 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 938 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 943 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 945 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 949 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 956 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 957 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 960 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 961 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 962 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 963 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval));;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 967 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 968 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 969 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 970 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 971 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 972 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 973 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 974 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 976 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 977 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 985 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 986 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 995 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 996 "hphp.y"
    { (yyval).reset();;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1000 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1008 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1009 "hphp.y"
    { (yyval).reset();;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1013 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1014 "hphp.y"
    { (yyval).reset();;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1018 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1023 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1029 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1035 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1041 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1047 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1053 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1061 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1065 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1069 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1073 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1079 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1082 "hphp.y"
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
#line 1097 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1100 "hphp.y"
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
#line 1114 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1117 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1122 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1125 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1132 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1135 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1143 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1146 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1154 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1155 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1159 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1162 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1165 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1166 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1167 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1171 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1172 "hphp.y"
    { (yyval).reset();;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1175 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1176 "hphp.y"
    { (yyval).reset();;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1179 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1180 "hphp.y"
    { (yyval).reset();;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1183 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1185 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1188 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1190 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1194 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1195 "hphp.y"
    { (yyval).reset();;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1198 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1199 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1200 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1204 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1206 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1209 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1211 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1214 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1216 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1219 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1221 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1231 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1232 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1233 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1234 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1239 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1241 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1242 "hphp.y"
    { (yyval).reset();;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1245 "hphp.y"
    { (yyval).reset();;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1246 "hphp.y"
    { (yyval).reset();;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1251 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1252 "hphp.y"
    { (yyval).reset();;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1257 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1258 "hphp.y"
    { (yyval).reset();;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1261 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1262 "hphp.y"
    { (yyval).reset();;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1265 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1266 "hphp.y"
    { (yyval).reset();;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1274 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1280 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1288 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1293 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1296 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1302 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1311 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1316 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1321 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1326 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1332 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1338 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1346 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1351 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1355 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1358 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1362 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1365 "hphp.y"
    { (yyval).reset();;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1370 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1373 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1377 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1381 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1385 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1389 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1394 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1399 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1405 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1406 "hphp.y"
    { (yyval).reset();;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1409 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1410 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1411 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1413 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1415 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1417 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1421 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1422 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1425 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1426 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1427 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1431 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1433 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1434 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1435 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1440 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1441 "hphp.y"
    { (yyval).reset();;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1444 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1449 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1455 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1456 "hphp.y"
    { (yyval).reset();;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1459 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1460 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1463 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1464 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1466 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1470 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1476 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1483 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1489 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1494 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1498 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1500 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1502 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1503 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1506 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1510 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1511 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1517 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1521 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1524 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1531 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1537 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1540 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1547 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1553 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1560 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1563 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1567 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1573 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1578 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1586 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1591 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1596 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1597 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1603 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1604 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1605 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1606 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1610 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1618 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1622 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1625 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1626 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1630 "hphp.y"
    { (yyval).reset();;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1631 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1635 "hphp.y"
    { (yyval).reset();;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1636 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1639 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1640 "hphp.y"
    { (yyval).reset();;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1643 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1644 "hphp.y"
    { (yyval).reset();;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1647 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1649 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1652 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1654 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1655 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1657 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1658 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1662 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1663 "hphp.y"
    { (yyval).reset();;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1666 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1667 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1668 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1672 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1675 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1676 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1680 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1682 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1686 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1688 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1689 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1690 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1694 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1698 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1699 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1703 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1704 "hphp.y"
    { (yyval).reset();;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1708 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1709 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1713 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1718 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1722 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1726 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1731 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1735 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1736 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1748 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1749 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1751 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1752 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1761 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { (yyval).reset();;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
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
#line 1868 "hphp.y"
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
#line 1876 "hphp.y"
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
#line 1884 "hphp.y"
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
#line 1893 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
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
#line 1909 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
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
#line 1929 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1969 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1970 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1974 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1978 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1979 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1984 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1990 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1991 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1995 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_VARRAY);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 2000 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 2002 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 2007 "hphp.y"
    { _p->onCheckedArray((yyval),(yyvsp[(3) - (4)]),T_VARRAY);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 2012 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 2019 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 2021 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 2025 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 2026 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 2027 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 2029 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 2033 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 2037 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 2041 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2046 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2048 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2050 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2052 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2056 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2058 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2062 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2063 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2064 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2065 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2066 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2067 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2071 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2075 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2079 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2084 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2089 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2093 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2097 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2098 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2102 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2103 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2107 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2108 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2112 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2117 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2121 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2125 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2129 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2132 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2139 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2142 "hphp.y"
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
#line 2153 "hphp.y"
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
#line 2164 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2165 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2170 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2171 "hphp.y"
    { (yyval).reset();;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2174 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2175 "hphp.y"
    { (yyval).reset();;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2178 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2182 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2185 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
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
#line 2195 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2200 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2204 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2207 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2210 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2211 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2213 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2214 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2215 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2216 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2219 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2221 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2222 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2227 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2236 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval).reset();;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval).reset();;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { (yyval).reset();;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { (yyval).reset();;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2337 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2338 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2344 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2353 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { (yyval).reset();;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { (yyval).reset();;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { (yyval).reset();;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { (yyval).reset();;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2467 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2473 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2475 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2479 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2481 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2486 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2492 "hphp.y"
    { (yyval).reset();;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2497 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2506 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2512 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2513 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2518 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2521 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2526 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { (yyval).reset();;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2538 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2540 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2545 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2551 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2552 "hphp.y"
    { (yyval).reset();;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2558 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2562 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2567 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2572 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2574 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2579 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2581 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2588 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2589 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2590 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2597 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2598 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2602 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2603 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2616 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2617 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2618 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2624 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2627 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2630 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2634 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2638 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2649 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2653 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2657 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2661 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2663 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2669 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2673 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2674 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2677 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2678 "hphp.y"
    { (yyval).reset();;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2682 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2683 "hphp.y"
    { (yyval)++;;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2687 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2688 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2689 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2691 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2694 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2695 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2699 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2701 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2703 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2704 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2708 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2709 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2711 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2712 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2713 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2720 "hphp.y"
    { (yyval).reset();;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2724 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2725 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2726 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2727 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2730 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2732 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2733 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2739 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2740 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2744 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2745 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2746 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2747 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2752 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2753 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2758 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2760 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2762 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2763 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2769 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2774 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2778 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2779 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2782 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2783 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2789 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2794 "hphp.y"
    { _p->onEmptyCheckedArray((yyval));;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2805 "hphp.y"
    { _p->onCheckedArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2806 "hphp.y"
    { _p->onCheckedArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2810 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2812 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2813 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2815 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2820 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2822 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2824 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2826 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2828 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2829 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2832 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2833 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2839 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2840 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2841 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2842 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2843 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2844 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2845 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2846 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2850 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2851 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2856 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2858 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2872 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2876 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2882 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2883 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2889 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2893 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2899 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2900 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2904 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2907 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2913 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2918 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2919 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2920 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2926 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2931 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); ;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2932 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); ;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2934 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); ;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2935 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2941 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2946 "hphp.y"
    { ;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 2958 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 2960 "hphp.y"
    {;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 2964 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 2971 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 908:

/* Line 1455 of yacc.c  */
#line 2974 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 909:

/* Line 1455 of yacc.c  */
#line 2977 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 910:

/* Line 1455 of yacc.c  */
#line 2978 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 911:

/* Line 1455 of yacc.c  */
#line 2981 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 912:

/* Line 1455 of yacc.c  */
#line 2984 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 913:

/* Line 1455 of yacc.c  */
#line 2986 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 914:

/* Line 1455 of yacc.c  */
#line 2989 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 915:

/* Line 1455 of yacc.c  */
#line 2992 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 916:

/* Line 1455 of yacc.c  */
#line 2998 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 917:

/* Line 1455 of yacc.c  */
#line 3004 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 918:

/* Line 1455 of yacc.c  */
#line 3012 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 919:

/* Line 1455 of yacc.c  */
#line 3013 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13531 "hphp.tab.cpp"
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
#line 3016 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

