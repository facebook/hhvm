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
     T_MIARRAY = 409,
     T_MSARRAY = 410,
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
#line 877 "hphp.tab.cpp"

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
#define YYLAST   16497

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  208
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  264
/* YYNRULES -- Number of rules.  */
#define YYNRULES  902
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1711

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
    1517,  1520,  1523,  1526,  1529,  1531,  1533,  1535,  1537,  1541,
    1544,  1546,  1548,  1550,  1556,  1557,  1558,  1570,  1571,  1584,
    1585,  1589,  1590,  1597,  1600,  1605,  1607,  1609,  1615,  1619,
    1625,  1629,  1632,  1633,  1636,  1637,  1642,  1647,  1651,  1656,
    1661,  1666,  1671,  1676,  1681,  1686,  1691,  1693,  1695,  1697,
    1701,  1704,  1708,  1713,  1716,  1720,  1722,  1725,  1727,  1730,
    1732,  1734,  1736,  1738,  1740,  1742,  1747,  1752,  1755,  1764,
    1775,  1778,  1780,  1784,  1786,  1789,  1791,  1793,  1795,  1797,
    1800,  1805,  1809,  1813,  1818,  1820,  1823,  1828,  1831,  1838,
    1839,  1841,  1846,  1847,  1850,  1851,  1853,  1855,  1859,  1861,
    1865,  1867,  1869,  1873,  1877,  1879,  1881,  1883,  1885,  1887,
    1889,  1891,  1893,  1895,  1897,  1899,  1901,  1903,  1905,  1907,
    1909,  1911,  1913,  1915,  1917,  1919,  1921,  1923,  1925,  1927,
    1929,  1931,  1933,  1935,  1937,  1939,  1941,  1943,  1945,  1947,
    1949,  1951,  1953,  1955,  1957,  1959,  1961,  1963,  1965,  1967,
    1969,  1971,  1973,  1975,  1977,  1979,  1981,  1983,  1985,  1987,
    1989,  1991,  1993,  1995,  1997,  1999,  2001,  2003,  2005,  2007,
    2009,  2011,  2013,  2015,  2017,  2019,  2021,  2023,  2025,  2027,
    2029,  2031,  2033,  2035,  2037,  2042,  2044,  2046,  2048,  2050,
    2052,  2054,  2056,  2058,  2061,  2063,  2064,  2065,  2067,  2069,
    2073,  2074,  2076,  2078,  2080,  2082,  2084,  2086,  2088,  2090,
    2092,  2094,  2096,  2098,  2100,  2104,  2107,  2109,  2111,  2116,
    2120,  2125,  2127,  2129,  2131,  2135,  2139,  2143,  2147,  2151,
    2155,  2159,  2163,  2167,  2171,  2175,  2179,  2183,  2187,  2191,
    2195,  2199,  2203,  2206,  2209,  2212,  2215,  2219,  2223,  2227,
    2231,  2235,  2239,  2243,  2247,  2253,  2258,  2262,  2266,  2270,
    2272,  2274,  2276,  2278,  2282,  2286,  2290,  2293,  2294,  2296,
    2297,  2299,  2300,  2306,  2310,  2314,  2316,  2318,  2320,  2322,
    2324,  2328,  2331,  2333,  2335,  2337,  2339,  2341,  2343,  2346,
    2349,  2354,  2358,  2363,  2366,  2367,  2373,  2377,  2381,  2383,
    2387,  2389,  2392,  2393,  2399,  2403,  2406,  2407,  2411,  2412,
    2417,  2420,  2421,  2425,  2429,  2431,  2432,  2434,  2437,  2440,
    2445,  2449,  2453,  2456,  2461,  2464,  2469,  2471,  2473,  2475,
    2477,  2479,  2482,  2487,  2491,  2496,  2500,  2502,  2504,  2506,
    2508,  2511,  2516,  2521,  2525,  2527,  2529,  2533,  2541,  2548,
    2557,  2567,  2576,  2587,  2595,  2602,  2611,  2613,  2616,  2621,
    2626,  2628,  2630,  2635,  2637,  2638,  2640,  2643,  2645,  2647,
    2650,  2655,  2659,  2663,  2664,  2666,  2669,  2674,  2678,  2681,
    2685,  2692,  2693,  2695,  2700,  2703,  2704,  2710,  2714,  2718,
    2720,  2727,  2732,  2737,  2740,  2743,  2744,  2750,  2754,  2758,
    2760,  2763,  2764,  2770,  2774,  2778,  2780,  2783,  2784,  2790,
    2794,  2797,  2798,  2804,  2808,  2811,  2814,  2816,  2819,  2821,
    2826,  2830,  2834,  2841,  2845,  2847,  2849,  2851,  2856,  2861,
    2866,  2871,  2874,  2877,  2882,  2885,  2888,  2890,  2894,  2898,
    2902,  2903,  2906,  2912,  2919,  2921,  2924,  2926,  2931,  2935,
    2936,  2938,  2942,  2945,  2949,  2951,  2953,  2954,  2955,  2958,
    2962,  2964,  2970,  2974,  2978,  2984,  2988,  2990,  2993,  2994,
    2999,  3002,  3005,  3007,  3009,  3011,  3013,  3018,  3025,  3027,
    3036,  3043,  3045
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     209,     0,    -1,    -1,   210,   211,    -1,   211,   212,    -1,
      -1,   230,    -1,   247,    -1,   254,    -1,   251,    -1,   259,
      -1,   457,    -1,   122,   198,   199,   200,    -1,   148,   222,
     200,    -1,    -1,   148,   222,   201,   213,   211,   202,    -1,
      -1,   148,   201,   214,   211,   202,    -1,   110,   216,   200,
      -1,   110,   104,   217,   200,    -1,   110,   105,   218,   200,
      -1,   227,   200,    -1,    77,    -1,   154,    -1,   155,    -1,
     157,    -1,   159,    -1,   158,    -1,   182,    -1,   183,    -1,
     185,    -1,   184,    -1,   186,    -1,   187,    -1,   188,    -1,
     189,    -1,   190,    -1,   191,    -1,   192,    -1,   193,    -1,
     194,    -1,   216,     9,   219,    -1,   219,    -1,   220,     9,
     220,    -1,   220,    -1,   221,     9,   221,    -1,   221,    -1,
     222,    -1,   151,   222,    -1,   222,    96,   215,    -1,   151,
     222,    96,   215,    -1,   222,    -1,   151,   222,    -1,   222,
      96,   215,    -1,   151,   222,    96,   215,    -1,   222,    -1,
     151,   222,    -1,   222,    96,   215,    -1,   151,   222,    96,
     215,    -1,   215,    -1,   222,   151,   215,    -1,   222,    -1,
     148,   151,   222,    -1,   151,   222,    -1,   223,    -1,   223,
     460,    -1,   223,   460,    -1,   227,     9,   458,    14,   400,
      -1,   105,   458,    14,   400,    -1,   228,   229,    -1,    -1,
     230,    -1,   247,    -1,   254,    -1,   259,    -1,   201,   228,
     202,    -1,    70,   330,   230,   281,   283,    -1,    70,   330,
      30,   228,   282,   284,    73,   200,    -1,    -1,    88,   330,
     231,   275,    -1,    -1,    87,   232,   230,    88,   330,   200,
      -1,    -1,    90,   198,   332,   200,   332,   200,   332,   199,
     233,   273,    -1,    -1,    97,   330,   234,   278,    -1,   101,
     200,    -1,   101,   339,   200,    -1,   103,   200,    -1,   103,
     339,   200,    -1,   106,   200,    -1,   106,   339,   200,    -1,
      27,   101,   200,    -1,   111,   291,   200,    -1,   117,   293,
     200,    -1,    86,   331,   200,    -1,   119,   198,   454,   199,
     200,    -1,   200,    -1,    81,    -1,    -1,    92,   198,   339,
      96,   272,   271,   199,   235,   274,    -1,    -1,    92,   198,
     339,    28,    96,   272,   271,   199,   236,   274,    -1,    94,
     198,   277,   199,   276,    -1,    -1,   107,   239,   108,   198,
     393,    79,   199,   201,   228,   202,   241,   237,   244,    -1,
      -1,   107,   239,   165,   238,   242,    -1,   109,   339,   200,
      -1,   102,   215,   200,    -1,   339,   200,    -1,   333,   200,
      -1,   334,   200,    -1,   335,   200,    -1,   336,   200,    -1,
     337,   200,    -1,   106,   336,   200,    -1,   338,   200,    -1,
     363,   200,    -1,   106,   362,   200,    -1,   215,    30,    -1,
      -1,   201,   240,   228,   202,    -1,   241,   108,   198,   393,
      79,   199,   201,   228,   202,    -1,    -1,    -1,   201,   243,
     228,   202,    -1,   165,   242,    -1,    -1,    35,    -1,    -1,
     104,    -1,    -1,   246,   245,   459,   248,   198,   287,   199,
     464,   319,    -1,    -1,   323,   246,   245,   459,   249,   198,
     287,   199,   464,   319,    -1,    -1,   420,   322,   246,   245,
     459,   250,   198,   287,   199,   464,   319,    -1,    -1,   158,
     215,   252,    30,   470,   456,   201,   294,   202,    -1,    -1,
     420,   158,   215,   253,    30,   470,   456,   201,   294,   202,
      -1,    -1,   265,   262,   255,   266,   267,   201,   297,   202,
      -1,    -1,   420,   265,   262,   256,   266,   267,   201,   297,
     202,    -1,    -1,   124,   263,   257,   268,   201,   297,   202,
      -1,    -1,   420,   124,   263,   258,   268,   201,   297,   202,
      -1,    -1,   160,   264,   260,   267,   201,   297,   202,    -1,
      -1,   420,   160,   264,   261,   267,   201,   297,   202,    -1,
     459,    -1,   152,    -1,   459,    -1,   459,    -1,   123,    -1,
     116,   123,    -1,   115,   123,    -1,   125,   393,    -1,    -1,
     126,   269,    -1,    -1,   125,   269,    -1,    -1,   393,    -1,
     269,     9,   393,    -1,   393,    -1,   270,     9,   393,    -1,
     128,   272,    -1,    -1,   427,    -1,    35,   427,    -1,   129,
     198,   439,   199,    -1,   230,    -1,    30,   228,    91,   200,
      -1,   230,    -1,    30,   228,    93,   200,    -1,   230,    -1,
      30,   228,    89,   200,    -1,   230,    -1,    30,   228,    95,
     200,    -1,   215,    14,   400,    -1,   277,     9,   215,    14,
     400,    -1,   201,   279,   202,    -1,   201,   200,   279,   202,
      -1,    30,   279,    98,   200,    -1,    30,   200,   279,    98,
     200,    -1,   279,    99,   339,   280,   228,    -1,   279,   100,
     280,   228,    -1,    -1,    30,    -1,   200,    -1,   281,    71,
     330,   230,    -1,    -1,   282,    71,   330,    30,   228,    -1,
      -1,    72,   230,    -1,    -1,    72,    30,   228,    -1,    -1,
     286,     9,   421,   325,   471,   161,    79,    -1,   286,     9,
     421,   325,   471,   161,    -1,   286,   405,    -1,   421,   325,
     471,   161,    79,    -1,   421,   325,   471,   161,    -1,    -1,
     421,   325,   471,    79,    -1,   421,   325,   471,    35,    79,
      -1,   421,   325,   471,    35,    79,    14,   400,    -1,   421,
     325,   471,    79,    14,   400,    -1,   286,     9,   421,   325,
     471,    79,    -1,   286,     9,   421,   325,   471,    35,    79,
      -1,   286,     9,   421,   325,   471,    35,    79,    14,   400,
      -1,   286,     9,   421,   325,   471,    79,    14,   400,    -1,
     288,     9,   421,   471,   161,    79,    -1,   288,     9,   421,
     471,   161,    -1,   288,   405,    -1,   421,   471,   161,    79,
      -1,   421,   471,   161,    -1,    -1,   421,   471,    79,    -1,
     421,   471,    35,    79,    -1,   421,   471,    35,    79,    14,
     400,    -1,   421,   471,    79,    14,   400,    -1,   288,     9,
     421,   471,    79,    -1,   288,     9,   421,   471,    35,    79,
      -1,   288,     9,   421,   471,    35,    79,    14,   400,    -1,
     288,     9,   421,   471,    79,    14,   400,    -1,   290,   405,
      -1,    -1,   339,    -1,    35,   427,    -1,   161,   339,    -1,
     290,     9,   339,    -1,   290,     9,   161,   339,    -1,   290,
       9,    35,   427,    -1,   291,     9,   292,    -1,   292,    -1,
      79,    -1,   203,   427,    -1,   203,   201,   339,   202,    -1,
     293,     9,    79,    -1,   293,     9,    79,    14,   400,    -1,
      79,    -1,    79,    14,   400,    -1,   294,   295,    -1,    -1,
     296,   200,    -1,   458,    14,   400,    -1,   297,   298,    -1,
      -1,    -1,   321,   299,   327,   200,    -1,    -1,   323,   470,
     300,   327,   200,    -1,   328,   200,    -1,    -1,   322,   246,
     245,   459,   198,   301,   285,   199,   464,   320,    -1,    -1,
     420,   322,   246,   245,   459,   198,   302,   285,   199,   464,
     320,    -1,   154,   307,   200,    -1,   155,   313,   200,    -1,
     157,   315,   200,    -1,     4,   125,   393,   200,    -1,     4,
     126,   393,   200,    -1,   110,   270,   200,    -1,   110,   270,
     201,   303,   202,    -1,   303,   304,    -1,   303,   305,    -1,
      -1,   226,   147,   215,   162,   270,   200,    -1,   306,    96,
     322,   215,   200,    -1,   306,    96,   323,   200,    -1,   226,
     147,   215,    -1,   215,    -1,   308,    -1,   307,     9,   308,
      -1,   309,   390,   311,   312,    -1,   152,    -1,   130,    -1,
     393,    -1,   118,    -1,   158,   201,   310,   202,    -1,   131,
      -1,   399,    -1,   310,     9,   399,    -1,    14,   400,    -1,
      -1,    55,   159,    -1,    -1,   314,    -1,   313,     9,   314,
      -1,   156,    -1,   316,    -1,   215,    -1,   121,    -1,   198,
     317,   199,    -1,   198,   317,   199,    49,    -1,   198,   317,
     199,    29,    -1,   198,   317,   199,    46,    -1,   316,    -1,
     318,    -1,   318,    49,    -1,   318,    29,    -1,   318,    46,
      -1,   317,     9,   317,    -1,   317,    33,   317,    -1,   215,
      -1,   152,    -1,   156,    -1,   200,    -1,   201,   228,   202,
      -1,   200,    -1,   201,   228,   202,    -1,   323,    -1,   118,
      -1,   323,    -1,    -1,   324,    -1,   323,   324,    -1,   112,
      -1,   113,    -1,   114,    -1,   117,    -1,   116,    -1,   115,
      -1,   180,    -1,   326,    -1,    -1,   112,    -1,   113,    -1,
     114,    -1,   327,     9,    79,    -1,   327,     9,    79,    14,
     400,    -1,    79,    -1,    79,    14,   400,    -1,   328,     9,
     458,    14,   400,    -1,   105,   458,    14,   400,    -1,   198,
     329,   199,    -1,    68,   395,   398,    -1,    67,   339,    -1,
     382,    -1,   356,    -1,   198,   339,   199,    -1,   331,     9,
     339,    -1,   339,    -1,   331,    -1,    -1,    27,   339,    -1,
      27,   339,   128,   339,    -1,   427,    14,   333,    -1,   129,
     198,   439,   199,    14,   333,    -1,    28,   339,    -1,   427,
      14,   336,    -1,   129,   198,   439,   199,    14,   336,    -1,
     340,    -1,   427,    -1,   329,    -1,   129,   198,   439,   199,
      14,   339,    -1,   427,    14,   339,    -1,   427,    14,    35,
     427,    -1,   427,    14,    35,    68,   395,   398,    -1,   427,
      26,   339,    -1,   427,    25,   339,    -1,   427,    24,   339,
      -1,   427,    23,   339,    -1,   427,    22,   339,    -1,   427,
      21,   339,    -1,   427,    20,   339,    -1,   427,    19,   339,
      -1,   427,    18,   339,    -1,   427,    17,   339,    -1,   427,
      16,   339,    -1,   427,    15,   339,    -1,   427,    64,    -1,
      64,   427,    -1,   427,    63,    -1,    63,   427,    -1,   339,
      31,   339,    -1,   339,    32,   339,    -1,   339,    10,   339,
      -1,   339,    12,   339,    -1,   339,    11,   339,    -1,   339,
      33,   339,    -1,   339,    35,   339,    -1,   339,    34,   339,
      -1,   339,    48,   339,    -1,   339,    46,   339,    -1,   339,
      47,   339,    -1,   339,    49,   339,    -1,   339,    50,   339,
      -1,   339,    65,   339,    -1,   339,    51,   339,    -1,   339,
      45,   339,    -1,   339,    44,   339,    -1,    46,   339,    -1,
      47,   339,    -1,    52,   339,    -1,    54,   339,    -1,   339,
      37,   339,    -1,   339,    36,   339,    -1,   339,    39,   339,
      -1,   339,    38,   339,    -1,   339,    40,   339,    -1,   339,
      43,   339,    -1,   339,    41,   339,    -1,   339,    42,   339,
      -1,   339,    53,   395,    -1,   198,   340,   199,    -1,   339,
      29,   339,    30,   339,    -1,   339,    29,    30,   339,    -1,
     453,    -1,    62,   339,    -1,    61,   339,    -1,    60,   339,
      -1,    59,   339,    -1,    58,   339,    -1,    57,   339,    -1,
      56,   339,    -1,    69,   396,    -1,    55,   339,    -1,   402,
      -1,   355,    -1,   354,    -1,   357,    -1,   204,   397,   204,
      -1,    13,   339,    -1,   342,    -1,   345,    -1,   360,    -1,
     110,   198,   381,   405,   199,    -1,    -1,    -1,   246,   245,
     198,   343,   287,   199,   464,   341,   201,   228,   202,    -1,
      -1,   323,   246,   245,   198,   344,   287,   199,   464,   341,
     201,   228,   202,    -1,    -1,    79,   346,   348,    -1,    -1,
     195,   347,   287,   196,   464,   348,    -1,     8,   339,    -1,
       8,   201,   228,   202,    -1,    85,    -1,   455,    -1,   350,
       9,   349,   128,   339,    -1,   349,   128,   339,    -1,   351,
       9,   349,   128,   400,    -1,   349,   128,   400,    -1,   350,
     404,    -1,    -1,   351,   404,    -1,    -1,   172,   198,   352,
     199,    -1,   130,   198,   440,   199,    -1,    66,   440,   205,
      -1,   393,   201,   442,   202,    -1,   173,   198,   446,   199,
      -1,   174,   198,   446,   199,    -1,   173,   198,   448,   199,
      -1,   174,   198,   448,   199,    -1,   393,   201,   444,   202,
      -1,   360,    66,   435,   205,    -1,   361,    66,   435,   205,
      -1,   355,    -1,   455,    -1,    85,    -1,   198,   340,   199,
      -1,   364,   365,    -1,   427,    14,   362,    -1,   181,    79,
     184,   339,    -1,   366,   377,    -1,   366,   377,   380,    -1,
     377,    -1,   377,   380,    -1,   367,    -1,   366,   367,    -1,
     368,    -1,   369,    -1,   370,    -1,   371,    -1,   372,    -1,
     373,    -1,   181,    79,   184,   339,    -1,   188,    79,    14,
     339,    -1,   182,   339,    -1,   183,    79,   184,   339,   185,
     339,   186,   339,    -1,   183,    79,   184,   339,   185,   339,
     186,   339,   187,    79,    -1,   189,   374,    -1,   375,    -1,
     374,     9,   375,    -1,   339,    -1,   339,   376,    -1,   190,
      -1,   191,    -1,   378,    -1,   379,    -1,   192,   339,    -1,
     193,   339,   194,   339,    -1,   187,    79,   365,    -1,   381,
       9,    79,    -1,   381,     9,    35,    79,    -1,    79,    -1,
      35,    79,    -1,   166,   152,   383,   167,    -1,   385,    50,
      -1,   385,   167,   386,   166,    50,   384,    -1,    -1,   152,
      -1,   385,   387,    14,   388,    -1,    -1,   386,   389,    -1,
      -1,   152,    -1,   153,    -1,   201,   339,   202,    -1,   153,
      -1,   201,   339,   202,    -1,   382,    -1,   391,    -1,   390,
      30,   391,    -1,   390,    47,   391,    -1,   215,    -1,    69,
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
      -1,   113,    -1,   112,    -1,   180,    -1,   119,    -1,   129,
      -1,   130,    -1,    10,    -1,    12,    -1,    11,    -1,   132,
      -1,   134,    -1,   133,    -1,   135,    -1,   136,    -1,   150,
      -1,   149,    -1,   179,    -1,   160,    -1,   163,    -1,   162,
      -1,   175,    -1,   177,    -1,   172,    -1,   225,   198,   289,
     199,    -1,   226,    -1,   152,    -1,   393,    -1,   117,    -1,
     433,    -1,   393,    -1,   117,    -1,   437,    -1,   198,   199,
      -1,   330,    -1,    -1,    -1,    84,    -1,   450,    -1,   198,
     289,   199,    -1,    -1,    74,    -1,    75,    -1,    76,    -1,
      85,    -1,   135,    -1,   136,    -1,   150,    -1,   132,    -1,
     163,    -1,   133,    -1,   134,    -1,   149,    -1,   179,    -1,
     143,    84,   144,    -1,   143,   144,    -1,   399,    -1,   224,
      -1,   130,   198,   403,   199,    -1,    66,   403,   205,    -1,
     172,   198,   353,   199,    -1,   401,    -1,   359,    -1,   358,
      -1,   198,   400,   199,    -1,   400,    31,   400,    -1,   400,
      32,   400,    -1,   400,    10,   400,    -1,   400,    12,   400,
      -1,   400,    11,   400,    -1,   400,    33,   400,    -1,   400,
      35,   400,    -1,   400,    34,   400,    -1,   400,    48,   400,
      -1,   400,    46,   400,    -1,   400,    47,   400,    -1,   400,
      49,   400,    -1,   400,    50,   400,    -1,   400,    51,   400,
      -1,   400,    45,   400,    -1,   400,    44,   400,    -1,   400,
      65,   400,    -1,    52,   400,    -1,    54,   400,    -1,    46,
     400,    -1,    47,   400,    -1,   400,    37,   400,    -1,   400,
      36,   400,    -1,   400,    39,   400,    -1,   400,    38,   400,
      -1,   400,    40,   400,    -1,   400,    43,   400,    -1,   400,
      41,   400,    -1,   400,    42,   400,    -1,   400,    29,   400,
      30,   400,    -1,   400,    29,    30,   400,    -1,   226,   147,
     215,    -1,   152,   147,   215,    -1,   226,   147,   123,    -1,
     224,    -1,    78,    -1,   455,    -1,   399,    -1,   206,   450,
     206,    -1,   207,   450,   207,    -1,   143,   450,   144,    -1,
     406,   404,    -1,    -1,     9,    -1,    -1,     9,    -1,    -1,
     406,     9,   400,   128,   400,    -1,   406,     9,   400,    -1,
     400,   128,   400,    -1,   400,    -1,    74,    -1,    75,    -1,
      76,    -1,    85,    -1,   143,    84,   144,    -1,   143,   144,
      -1,    74,    -1,    75,    -1,    76,    -1,   215,    -1,   407,
      -1,   215,    -1,    46,   408,    -1,    47,   408,    -1,   130,
     198,   410,   199,    -1,    66,   410,   205,    -1,   172,   198,
     413,   199,    -1,   411,   404,    -1,    -1,   411,     9,   409,
     128,   409,    -1,   411,     9,   409,    -1,   409,   128,   409,
      -1,   409,    -1,   412,     9,   409,    -1,   409,    -1,   414,
     404,    -1,    -1,   414,     9,   349,   128,   409,    -1,   349,
     128,   409,    -1,   412,   404,    -1,    -1,   198,   415,   199,
      -1,    -1,   417,     9,   215,   416,    -1,   215,   416,    -1,
      -1,   419,   417,   404,    -1,    45,   418,    44,    -1,   420,
      -1,    -1,   423,    -1,   127,   432,    -1,   127,   215,    -1,
     127,   201,   339,   202,    -1,    66,   435,   205,    -1,   201,
     339,   202,    -1,   428,   424,    -1,   198,   329,   199,   424,
      -1,   438,   424,    -1,   198,   329,   199,   424,    -1,   432,
      -1,   392,    -1,   430,    -1,   431,    -1,   425,    -1,   427,
     422,    -1,   198,   329,   199,   422,    -1,   394,   147,   432,
      -1,   429,   198,   289,   199,    -1,   198,   427,   199,    -1,
     392,    -1,   430,    -1,   431,    -1,   425,    -1,   427,   423,
      -1,   198,   329,   199,   423,    -1,   429,   198,   289,   199,
      -1,   198,   427,   199,    -1,   432,    -1,   425,    -1,   198,
     427,   199,    -1,   427,   127,   215,   460,   198,   289,   199,
      -1,   427,   127,   432,   198,   289,   199,    -1,   427,   127,
     201,   339,   202,   198,   289,   199,    -1,   198,   329,   199,
     127,   215,   460,   198,   289,   199,    -1,   198,   329,   199,
     127,   432,   198,   289,   199,    -1,   198,   329,   199,   127,
     201,   339,   202,   198,   289,   199,    -1,   394,   147,   215,
     460,   198,   289,   199,    -1,   394,   147,   432,   198,   289,
     199,    -1,   394,   147,   201,   339,   202,   198,   289,   199,
      -1,   433,    -1,   436,   433,    -1,   433,    66,   435,   205,
      -1,   433,   201,   339,   202,    -1,   434,    -1,    79,    -1,
     203,   201,   339,   202,    -1,   339,    -1,    -1,   203,    -1,
     436,   203,    -1,   432,    -1,   426,    -1,   437,   422,    -1,
     198,   329,   199,   422,    -1,   394,   147,   432,    -1,   198,
     427,   199,    -1,    -1,   426,    -1,   437,   423,    -1,   198,
     329,   199,   423,    -1,   198,   427,   199,    -1,   439,     9,
      -1,   439,     9,   427,    -1,   439,     9,   129,   198,   439,
     199,    -1,    -1,   427,    -1,   129,   198,   439,   199,    -1,
     441,   404,    -1,    -1,   441,     9,   339,   128,   339,    -1,
     441,     9,   339,    -1,   339,   128,   339,    -1,   339,    -1,
     441,     9,   339,   128,    35,   427,    -1,   441,     9,    35,
     427,    -1,   339,   128,    35,   427,    -1,    35,   427,    -1,
     443,   404,    -1,    -1,   443,     9,   339,   128,   339,    -1,
     443,     9,   339,    -1,   339,   128,   339,    -1,   339,    -1,
     445,   404,    -1,    -1,   445,     9,   400,   128,   400,    -1,
     445,     9,   400,    -1,   400,   128,   400,    -1,   400,    -1,
     447,   404,    -1,    -1,   447,     9,   339,   128,   339,    -1,
     339,   128,   339,    -1,   449,   404,    -1,    -1,   449,     9,
     400,   128,   400,    -1,   400,   128,   400,    -1,   450,   451,
      -1,   450,    84,    -1,   451,    -1,    84,   451,    -1,    79,
      -1,    79,    66,   452,   205,    -1,    79,   127,   215,    -1,
     145,   339,   202,    -1,   145,    78,    66,   339,   205,   202,
      -1,   146,   427,   202,    -1,   215,    -1,    80,    -1,    79,
      -1,   120,   198,   454,   199,    -1,   121,   198,   427,   199,
      -1,   121,   198,   340,   199,    -1,   121,   198,   329,   199,
      -1,     7,   339,    -1,     6,   339,    -1,     5,   198,   339,
     199,    -1,     4,   339,    -1,     3,   339,    -1,   427,    -1,
     454,     9,   427,    -1,   394,   147,   215,    -1,   394,   147,
     123,    -1,    -1,    96,   470,    -1,   175,   459,    14,   470,
     200,    -1,   177,   459,   456,    14,   470,   200,    -1,   215,
      -1,   470,   215,    -1,   215,    -1,   215,   168,   465,   169,
      -1,   168,   462,   169,    -1,    -1,   470,    -1,   461,     9,
     470,    -1,   461,   404,    -1,   461,     9,   161,    -1,   462,
      -1,   161,    -1,    -1,    -1,    30,   470,    -1,   465,     9,
     215,    -1,   215,    -1,   465,     9,   215,    96,   470,    -1,
     215,    96,   470,    -1,    85,   128,   470,    -1,   226,   147,
     215,   128,   470,    -1,   467,     9,   466,    -1,   466,    -1,
     467,   404,    -1,    -1,   172,   198,   468,   199,    -1,    29,
     470,    -1,    55,   470,    -1,   226,    -1,   130,    -1,   131,
      -1,   469,    -1,   130,   168,   470,   169,    -1,   130,   168,
     470,     9,   470,   169,    -1,   152,    -1,   198,   104,   198,
     463,   199,    30,   470,   199,    -1,   198,   470,     9,   461,
     404,   199,    -1,   470,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   732,   732,   732,   741,   743,   746,   747,   748,   749,
     750,   751,   752,   755,   757,   757,   759,   759,   761,   762,
     764,   766,   771,   772,   773,   774,   775,   776,   777,   778,
     779,   780,   781,   782,   783,   784,   785,   786,   787,   788,
     789,   793,   795,   799,   801,   805,   807,   811,   812,   813,
     814,   819,   820,   821,   822,   827,   828,   829,   830,   835,
     836,   840,   841,   843,   846,   852,   859,   866,   870,   876,
     878,   881,   882,   883,   884,   887,   888,   892,   897,   897,
     903,   903,   910,   909,   915,   915,   920,   921,   922,   923,
     924,   925,   926,   927,   928,   929,   930,   931,   932,   935,
     933,   942,   940,   947,   955,   949,   959,   957,   961,   962,
     966,   967,   968,   969,   970,   971,   972,   973,   974,   975,
     976,   984,   984,   989,   995,   999,   999,  1007,  1008,  1012,
    1013,  1017,  1022,  1021,  1034,  1032,  1046,  1044,  1060,  1059,
    1068,  1066,  1078,  1077,  1096,  1094,  1113,  1112,  1121,  1119,
    1131,  1130,  1142,  1140,  1153,  1154,  1158,  1161,  1164,  1165,
    1166,  1169,  1171,  1174,  1175,  1178,  1179,  1182,  1183,  1187,
    1188,  1193,  1194,  1197,  1198,  1199,  1203,  1204,  1208,  1209,
    1213,  1214,  1218,  1219,  1224,  1225,  1230,  1231,  1232,  1233,
    1236,  1239,  1241,  1244,  1245,  1249,  1251,  1254,  1257,  1260,
    1261,  1264,  1265,  1269,  1275,  1282,  1284,  1289,  1295,  1299,
    1303,  1307,  1312,  1317,  1322,  1327,  1333,  1342,  1347,  1353,
    1355,  1359,  1364,  1368,  1371,  1374,  1378,  1382,  1386,  1390,
    1395,  1403,  1405,  1408,  1409,  1410,  1411,  1413,  1415,  1420,
    1421,  1424,  1425,  1426,  1430,  1431,  1433,  1434,  1438,  1440,
    1443,  1447,  1453,  1455,  1458,  1458,  1462,  1461,  1465,  1469,
    1467,  1482,  1479,  1492,  1494,  1496,  1498,  1500,  1502,  1504,
    1508,  1509,  1510,  1513,  1519,  1522,  1528,  1531,  1536,  1538,
    1543,  1548,  1552,  1553,  1559,  1560,  1562,  1566,  1567,  1572,
    1573,  1577,  1578,  1582,  1584,  1590,  1595,  1596,  1598,  1602,
    1603,  1604,  1605,  1609,  1610,  1611,  1612,  1613,  1614,  1616,
    1621,  1624,  1625,  1629,  1630,  1634,  1635,  1638,  1639,  1642,
    1643,  1646,  1647,  1651,  1652,  1653,  1654,  1655,  1656,  1657,
    1661,  1662,  1665,  1666,  1667,  1670,  1672,  1674,  1675,  1678,
    1680,  1685,  1686,  1688,  1689,  1690,  1693,  1697,  1698,  1702,
    1703,  1707,  1708,  1712,  1716,  1721,  1725,  1729,  1734,  1735,
    1736,  1739,  1741,  1742,  1743,  1746,  1747,  1748,  1749,  1750,
    1751,  1752,  1753,  1754,  1755,  1756,  1757,  1758,  1759,  1760,
    1761,  1762,  1763,  1764,  1765,  1766,  1767,  1768,  1769,  1770,
    1771,  1772,  1773,  1774,  1775,  1776,  1777,  1778,  1779,  1780,
    1781,  1782,  1783,  1784,  1785,  1786,  1787,  1788,  1790,  1791,
    1793,  1795,  1796,  1797,  1798,  1799,  1800,  1801,  1802,  1803,
    1804,  1805,  1806,  1807,  1808,  1809,  1810,  1811,  1812,  1813,
    1814,  1815,  1816,  1820,  1824,  1829,  1828,  1843,  1841,  1858,
    1858,  1873,  1873,  1891,  1892,  1897,  1899,  1903,  1907,  1913,
    1917,  1923,  1925,  1929,  1931,  1935,  1939,  1940,  1944,  1951,
    1952,  1956,  1957,  1962,  1969,  1971,  1976,  1977,  1978,  1980,
    1984,  1988,  1992,  1996,  1998,  2000,  2002,  2007,  2008,  2013,
    2014,  2015,  2016,  2017,  2018,  2022,  2026,  2030,  2034,  2039,
    2044,  2048,  2049,  2053,  2054,  2058,  2059,  2063,  2064,  2068,
    2072,  2076,  2080,  2081,  2082,  2083,  2087,  2093,  2102,  2115,
    2116,  2119,  2122,  2125,  2126,  2129,  2133,  2136,  2139,  2146,
    2147,  2151,  2152,  2154,  2158,  2159,  2160,  2161,  2162,  2163,
    2164,  2165,  2166,  2167,  2168,  2169,  2170,  2171,  2172,  2173,
    2174,  2175,  2176,  2177,  2178,  2179,  2180,  2181,  2182,  2183,
    2184,  2185,  2186,  2187,  2188,  2189,  2190,  2191,  2192,  2193,
    2194,  2195,  2196,  2197,  2198,  2199,  2200,  2201,  2202,  2203,
    2204,  2205,  2206,  2207,  2208,  2209,  2210,  2211,  2212,  2213,
    2214,  2215,  2216,  2217,  2218,  2219,  2220,  2221,  2222,  2223,
    2224,  2225,  2226,  2227,  2228,  2229,  2230,  2231,  2232,  2233,
    2234,  2235,  2236,  2237,  2241,  2246,  2247,  2250,  2251,  2252,
    2256,  2257,  2258,  2262,  2263,  2264,  2268,  2269,  2270,  2273,
    2275,  2279,  2280,  2281,  2282,  2284,  2285,  2286,  2287,  2288,
    2289,  2290,  2291,  2292,  2293,  2296,  2301,  2302,  2303,  2305,
    2306,  2308,  2309,  2310,  2312,  2313,  2315,  2317,  2319,  2321,
    2323,  2324,  2325,  2326,  2327,  2328,  2329,  2330,  2331,  2332,
    2333,  2334,  2335,  2336,  2337,  2338,  2339,  2341,  2343,  2345,
    2347,  2348,  2351,  2352,  2356,  2358,  2362,  2365,  2368,  2374,
    2375,  2376,  2377,  2378,  2379,  2380,  2385,  2387,  2391,  2392,
    2395,  2396,  2400,  2403,  2405,  2407,  2411,  2412,  2413,  2414,
    2416,  2419,  2423,  2424,  2425,  2426,  2429,  2430,  2431,  2432,
    2433,  2435,  2436,  2441,  2443,  2446,  2449,  2451,  2453,  2456,
    2458,  2462,  2464,  2467,  2470,  2476,  2478,  2481,  2482,  2487,
    2490,  2494,  2494,  2499,  2502,  2503,  2507,  2508,  2513,  2514,
    2518,  2519,  2523,  2524,  2529,  2531,  2536,  2537,  2538,  2539,
    2540,  2541,  2542,  2544,  2547,  2549,  2553,  2554,  2555,  2556,
    2557,  2559,  2561,  2563,  2567,  2568,  2569,  2573,  2576,  2579,
    2582,  2586,  2590,  2597,  2601,  2605,  2612,  2613,  2618,  2620,
    2621,  2624,  2625,  2628,  2629,  2633,  2634,  2638,  2639,  2640,
    2641,  2643,  2646,  2649,  2650,  2651,  2653,  2655,  2659,  2660,
    2661,  2663,  2664,  2665,  2669,  2671,  2674,  2676,  2677,  2678,
    2679,  2682,  2684,  2685,  2689,  2691,  2694,  2696,  2697,  2698,
    2702,  2704,  2707,  2710,  2712,  2714,  2718,  2720,  2723,  2725,
    2729,  2731,  2734,  2737,  2742,  2743,  2745,  2746,  2752,  2753,
    2755,  2757,  2759,  2761,  2764,  2765,  2766,  2770,  2771,  2772,
    2773,  2774,  2775,  2776,  2777,  2778,  2782,  2783,  2787,  2789,
    2797,  2799,  2803,  2807,  2814,  2815,  2821,  2822,  2829,  2832,
    2836,  2839,  2844,  2849,  2851,  2852,  2853,  2857,  2858,  2862,
    2864,  2865,  2867,  2871,  2874,  2883,  2885,  2889,  2892,  2895,
    2903,  2906,  2909,  2910,  2913,  2916,  2917,  2920,  2924,  2928,
    2934,  2944,  2945
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
  "T_MIARRAY", "T_MSARRAY", "T_TYPE", "T_UNRESOLVED_TYPE", "T_NEWTYPE",
  "T_UNRESOLVED_NEWTYPE", "T_COMPILER_HALT_OFFSET", "T_ASYNC", "T_FROM",
  "T_WHERE", "T_JOIN", "T_IN", "T_ON", "T_EQUALS", "T_INTO", "T_LET",
  "T_ORDERBY", "T_ASCENDING", "T_DESCENDING", "T_SELECT", "T_GROUP",
  "T_BY", "T_LAMBDA_OP", "T_LAMBDA_CP", "T_UNRESOLVED_OP", "'('", "')'",
  "';'", "'{'", "'}'", "'$'", "'`'", "']'", "'\"'", "'\\''", "$accept",
  "start", "$@1", "top_statement_list", "top_statement", "$@2", "$@3",
  "ident", "use_declarations", "use_fn_declarations",
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
  "$@30", "lambda_expression", "$@31", "$@32", "lambda_body",
  "shape_keyname", "non_empty_shape_pair_list",
  "non_empty_static_shape_pair_list", "shape_pair_list",
  "static_shape_pair_list", "shape_literal", "array_literal",
  "collection_literal", "map_array_literal", "static_map_array_literal",
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
  "user_attribute_list", "$@33", "non_empty_user_attributes",
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
  "non_empty_static_collection_init", "map_array_init",
  "non_empty_map_array_init", "static_map_array_init",
  "non_empty_static_map_array_init", "encaps_list", "encaps_var",
  "encaps_var_offset", "internal_functions", "variable_list",
  "class_constant", "hh_opt_constraint", "hh_type_alias_statement",
  "hh_name_with_type", "hh_name_with_typevar", "hh_typeargs_opt",
  "hh_non_empty_type_list", "hh_type_list", "hh_func_type_list",
  "hh_opt_return_type", "hh_typevar_list", "hh_shape_member_type",
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
     230,   230,   230,   230,   230,   230,   230,   230,   230,   235,
     230,   236,   230,   230,   237,   230,   238,   230,   230,   230,
     230,   230,   230,   230,   230,   230,   230,   230,   230,   230,
     230,   240,   239,   241,   241,   243,   242,   244,   244,   245,
     245,   246,   248,   247,   249,   247,   250,   247,   252,   251,
     253,   251,   255,   254,   256,   254,   257,   254,   258,   254,
     260,   259,   261,   259,   262,   262,   263,   264,   265,   265,
     265,   266,   266,   267,   267,   268,   268,   269,   269,   270,
     270,   271,   271,   272,   272,   272,   273,   273,   274,   274,
     275,   275,   276,   276,   277,   277,   278,   278,   278,   278,
     279,   279,   279,   280,   280,   281,   281,   282,   282,   283,
     283,   284,   284,   285,   285,   285,   285,   285,   285,   286,
     286,   286,   286,   286,   286,   286,   286,   287,   287,   287,
     287,   287,   287,   288,   288,   288,   288,   288,   288,   288,
     288,   289,   289,   290,   290,   290,   290,   290,   290,   291,
     291,   292,   292,   292,   293,   293,   293,   293,   294,   294,
     295,   296,   297,   297,   299,   298,   300,   298,   298,   301,
     298,   302,   298,   298,   298,   298,   298,   298,   298,   298,
     303,   303,   303,   304,   305,   305,   306,   306,   307,   307,
     308,   308,   309,   309,   309,   309,   309,   310,   310,   311,
     311,   312,   312,   313,   313,   314,   315,   315,   315,   316,
     316,   316,   316,   317,   317,   317,   317,   317,   317,   317,
     318,   318,   318,   319,   319,   320,   320,   321,   321,   322,
     322,   323,   323,   324,   324,   324,   324,   324,   324,   324,
     325,   325,   326,   326,   326,   327,   327,   327,   327,   328,
     328,   329,   329,   329,   329,   329,   330,   331,   331,   332,
     332,   333,   333,   334,   335,   336,   337,   338,   339,   339,
     339,   340,   340,   340,   340,   340,   340,   340,   340,   340,
     340,   340,   340,   340,   340,   340,   340,   340,   340,   340,
     340,   340,   340,   340,   340,   340,   340,   340,   340,   340,
     340,   340,   340,   340,   340,   340,   340,   340,   340,   340,
     340,   340,   340,   340,   340,   340,   340,   340,   340,   340,
     340,   340,   340,   340,   340,   340,   340,   340,   340,   340,
     340,   340,   340,   340,   340,   340,   340,   340,   340,   340,
     340,   340,   340,   341,   341,   343,   342,   344,   342,   346,
     345,   347,   345,   348,   348,   349,   349,   350,   350,   351,
     351,   352,   352,   353,   353,   354,   355,   355,   356,   357,
     357,   358,   358,   359,   360,   360,   361,   361,   361,   361,
     362,   363,   364,   365,   365,   365,   365,   366,   366,   367,
     367,   367,   367,   367,   367,   368,   369,   370,   371,   372,
     373,   374,   374,   375,   375,   376,   376,   377,   377,   378,
     379,   380,   381,   381,   381,   381,   382,   383,   383,   384,
     384,   385,   385,   386,   386,   387,   388,   388,   389,   389,
     389,   390,   390,   390,   391,   391,   391,   391,   391,   391,
     391,   391,   391,   391,   391,   391,   391,   391,   391,   391,
     391,   391,   391,   391,   391,   391,   391,   391,   391,   391,
     391,   391,   391,   391,   391,   391,   391,   391,   391,   391,
     391,   391,   391,   391,   391,   391,   391,   391,   391,   391,
     391,   391,   391,   391,   391,   391,   391,   391,   391,   391,
     391,   391,   391,   391,   391,   391,   391,   391,   391,   391,
     391,   391,   391,   391,   391,   391,   391,   391,   391,   391,
     391,   391,   391,   391,   392,   393,   393,   394,   394,   394,
     395,   395,   395,   396,   396,   396,   397,   397,   397,   398,
     398,   399,   399,   399,   399,   399,   399,   399,   399,   399,
     399,   399,   399,   399,   399,   399,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   400,   401,   401,   401,   402,
     402,   402,   402,   402,   402,   402,   403,   403,   404,   404,
     405,   405,   406,   406,   406,   406,   407,   407,   407,   407,
     407,   407,   408,   408,   408,   408,   409,   409,   409,   409,
     409,   409,   409,   410,   410,   411,   411,   411,   411,   412,
     412,   413,   413,   414,   414,   415,   415,   416,   416,   417,
     417,   419,   418,   420,   421,   421,   422,   422,   423,   423,
     424,   424,   425,   425,   426,   426,   427,   427,   427,   427,
     427,   427,   427,   427,   427,   427,   428,   428,   428,   428,
     428,   428,   428,   428,   429,   429,   429,   430,   430,   430,
     430,   430,   430,   431,   431,   431,   432,   432,   433,   433,
     433,   434,   434,   435,   435,   436,   436,   437,   437,   437,
     437,   437,   437,   438,   438,   438,   438,   438,   439,   439,
     439,   439,   439,   439,   440,   440,   441,   441,   441,   441,
     441,   441,   441,   441,   442,   442,   443,   443,   443,   443,
     444,   444,   445,   445,   445,   445,   446,   446,   447,   447,
     448,   448,   449,   449,   450,   450,   450,   450,   451,   451,
     451,   451,   451,   451,   452,   452,   452,   453,   453,   453,
     453,   453,   453,   453,   453,   453,   454,   454,   455,   455,
     456,   456,   457,   457,   458,   458,   459,   459,   460,   460,
     461,   461,   462,   463,   463,   463,   463,   464,   464,   465,
     465,   465,   465,   466,   466,   467,   467,   468,   468,   469,
     470,   470,   470,   470,   470,   470,   470,   470,   470,   470,
     470,   471,   471
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
       2,     2,     2,     2,     1,     1,     1,     1,     3,     2,
       1,     1,     1,     5,     0,     0,    11,     0,    12,     0,
       3,     0,     6,     2,     4,     1,     1,     5,     3,     5,
       3,     2,     0,     2,     0,     4,     4,     3,     4,     4,
       4,     4,     4,     4,     4,     4,     1,     1,     1,     3,
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
       4,     1,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     3,     3,     3,     3,
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
       2,     0,     5,     3,     3,     1,     2,     0,     5,     3,
       2,     0,     5,     3,     2,     2,     1,     2,     1,     4,
       3,     3,     6,     3,     1,     1,     1,     4,     4,     4,
       4,     2,     2,     4,     2,     2,     1,     3,     3,     3,
       0,     2,     5,     6,     1,     2,     1,     4,     3,     0,
       1,     3,     2,     3,     1,     1,     0,     0,     2,     3,
       1,     5,     3,     3,     5,     3,     1,     2,     0,     4,
       2,     2,     1,     1,     1,     1,     4,     6,     1,     8,
       6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,     0,     0,   731,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   805,     0,
     793,   615,     0,   621,   622,   623,    22,   680,   781,    98,
     624,     0,    80,     0,     0,     0,     0,     0,     0,     0,
       0,   131,     0,     0,     0,     0,     0,     0,   323,   324,
     325,   328,   327,   326,     0,     0,     0,     0,   158,     0,
       0,     0,   628,   630,   631,   625,   626,     0,     0,   632,
     627,     0,   606,    23,    24,    25,    27,    26,     0,   629,
       0,     0,     0,     0,     0,     0,   633,   329,    28,    29,
      31,    30,    32,    33,    34,    35,    36,    37,    38,    39,
      40,   441,     0,    97,    70,   785,   616,     0,     0,     4,
      59,    61,    64,   679,     0,   605,     0,     6,   130,     7,
       9,     8,    10,     0,     0,   321,   360,     0,     0,     0,
       0,     0,     0,     0,   358,   430,   431,   426,   425,   345,
     427,   432,     0,     0,   344,   747,   607,     0,   682,   424,
     320,   750,   359,     0,     0,   748,   749,   746,   776,   780,
       0,   414,   681,    11,   328,   327,   326,     0,     0,    27,
      59,   130,     0,   855,   359,   854,     0,   852,   851,   429,
       0,   351,   355,     0,     0,   398,   399,   400,   401,   423,
     421,   420,   419,   418,   417,   416,   415,   781,   608,     0,
     869,   607,     0,   380,   378,     0,   809,     0,   689,   343,
     611,     0,   869,   610,     0,   620,   788,   787,   612,     0,
       0,   614,   422,     0,     0,     0,     0,   348,     0,    78,
     350,     0,     0,    84,    86,     0,     0,    88,     0,     0,
       0,   893,   894,   898,     0,     0,    59,   892,     0,   895,
       0,     0,    90,     0,     0,     0,     0,   121,     0,     0,
       0,     0,     0,     0,    42,    47,   241,     0,     0,   240,
     160,   159,   246,     0,     0,     0,     0,     0,   866,   146,
     156,   801,   805,   838,     0,   635,     0,     0,     0,   836,
       0,    16,     0,    63,   138,   150,   157,   512,   452,   827,
     827,     0,   860,   735,   360,     0,   358,   359,     0,     0,
     617,     0,   618,     0,     0,     0,   120,     0,     0,    66,
     232,     0,    21,   129,     0,   155,   142,   154,   326,   130,
     322,   111,   112,   113,   114,   115,   117,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   793,     0,   110,   784,   784,   118,   815,     0,
       0,     0,     0,     0,     0,   319,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   379,
     377,     0,   751,   736,   784,     0,   742,   232,   784,     0,
     786,   777,   801,     0,   130,     0,     0,    92,     0,   733,
     728,   689,     0,     0,     0,     0,   813,     0,   457,   688,
     804,     0,     0,    66,     0,   232,   342,     0,   789,   736,
     744,   613,     0,    70,   196,     0,   440,     0,    95,     0,
       0,   349,     0,     0,     0,     0,     0,    87,   109,    89,
     890,   891,     0,   888,     0,     0,     0,   865,     0,   116,
      91,   119,     0,     0,     0,     0,     0,     0,     0,   470,
       0,   477,   479,   480,   481,   482,   483,   484,   475,   497,
     498,    70,     0,   106,   108,     0,     0,    44,    51,     0,
       0,    46,    55,    48,     0,    18,     0,     0,   242,     0,
      93,     0,     0,    94,   856,     0,     0,   360,   358,   359,
       0,     0,   166,     0,   802,     0,     0,     0,     0,   634,
     837,   680,     0,     0,   835,   685,   834,    62,     5,    13,
      14,     0,   164,     0,     0,   445,     0,     0,   689,     0,
       0,   609,   446,     0,     0,   689,     0,     0,     0,     0,
       0,   691,   734,   902,   341,   411,   755,    75,    69,    71,
      72,    73,    74,   320,     0,   428,   683,   684,    60,   689,
       0,   870,     0,     0,     0,   691,   233,     0,   435,   132,
     162,     0,   383,   385,   384,     0,     0,   381,   382,   386,
     388,   387,   403,   402,   405,   404,   406,   408,   409,   407,
     397,   396,   390,   391,   389,   392,   393,   395,   410,   394,
     783,     0,     0,   819,     0,   689,   859,     0,   858,   753,
     776,   148,   140,   152,   144,   130,     0,     0,   353,   356,
     362,   471,   376,   375,   374,   373,   372,   371,   370,   369,
     368,   367,   366,   365,     0,   738,   737,     0,     0,     0,
       0,     0,     0,     0,   853,   352,   726,   730,   688,   732,
       0,     0,   869,     0,   808,     0,   807,     0,   792,   791,
       0,     0,   738,   737,   346,   198,   200,    70,   443,   347,
       0,    70,   180,    79,   350,     0,     0,     0,     0,     0,
     192,   192,    85,     0,     0,     0,   886,   689,     0,   876,
       0,     0,     0,     0,     0,   687,   624,     0,     0,   606,
       0,     0,     0,     0,    64,   637,   605,   643,   642,     0,
     636,    68,   641,     0,     0,   487,     0,     0,   493,   490,
     491,   499,     0,   478,   473,     0,   476,     0,     0,     0,
      52,    19,     0,     0,    56,    20,     0,     0,     0,    41,
      49,     0,   239,   247,   244,     0,     0,   847,   850,   849,
     848,    12,   880,     0,     0,     0,   801,   798,     0,   456,
     846,   845,   844,     0,   840,     0,   841,   843,     0,     5,
       0,     0,     0,   506,   507,   515,   514,     0,     0,   688,
     451,   455,     0,     0,   459,   688,   826,   460,     0,   861,
       0,   877,   735,   219,   901,     0,     0,   752,   736,   743,
     782,   688,   872,   868,   234,   235,   604,   690,   231,     0,
     735,     0,     0,   164,   437,   134,   413,     0,   464,   465,
       0,   458,   688,   814,     0,     0,   232,   166,     0,   164,
     162,     0,   793,   363,     0,     0,   232,   740,   741,   754,
     778,   779,     0,     0,     0,   714,   696,   697,   698,   699,
       0,     0,     0,   707,   706,   720,   689,     0,   728,   812,
     811,     0,   790,   736,   745,   619,     0,   202,     0,     0,
      76,     0,     0,     0,     0,     0,     0,     0,   172,   173,
     184,     0,    70,   182,   103,   192,     0,   192,     0,     0,
     896,     0,     0,   688,   887,   889,   875,   689,   874,     0,
     689,   664,   665,   662,   663,   695,     0,   689,   687,     0,
       0,   454,   831,   831,     0,     0,   821,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   472,     0,     0,     0,   495,   496,   494,
       0,     0,   474,     0,   122,     0,   125,   107,     0,    43,
      53,     0,    45,    57,    50,   243,     0,   857,    96,     0,
       0,   867,   165,   167,   253,     0,     0,   799,     0,   839,
       0,    17,     0,   860,   163,   253,     0,     0,   448,     0,
     858,   829,     0,   862,     0,     0,     0,   902,     0,   223,
     221,     0,   738,   737,   871,     0,     0,   236,    67,     0,
     735,   161,     0,   735,     0,   412,   818,   817,     0,   232,
       0,     0,     0,     0,   164,   136,   620,   739,   232,     0,
       0,   702,   703,   704,   705,   708,   709,   718,     0,   689,
     714,     0,   701,   722,   688,   725,   727,   729,     0,   806,
     739,     0,     0,     0,     0,   199,   444,    81,     0,   350,
     172,   174,   801,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   186,     0,   883,     0,   885,   688,     0,     0,
       0,   639,   688,   686,     0,   677,     0,   689,     0,     0,
       0,   689,     0,   644,   678,   676,   825,     0,   689,   647,
     649,   648,     0,     0,   645,   646,   650,   652,   651,   667,
     666,   669,   668,   670,   672,   673,   671,   660,   659,   654,
     655,   653,   656,   657,   658,   661,   485,     0,   486,   492,
     500,   501,     0,    70,    54,    58,   245,   882,   879,     0,
     320,   803,   801,   354,   357,   361,     0,    15,     0,   320,
     518,     0,     0,   520,   513,   516,     0,   511,     0,     0,
     863,   878,   442,     0,   224,     0,   220,     0,     0,   232,
     238,   237,   877,     0,   253,     0,   735,     0,   232,     0,
     774,   253,   860,   253,     0,     0,   364,   232,     0,   768,
       0,   711,   688,   713,     0,   700,     0,     0,   689,   719,
     810,     0,    70,     0,   195,   181,     0,     0,     0,   171,
      99,   185,     0,     0,   188,     0,   193,   194,    70,   187,
     897,     0,   873,     0,   900,   694,   693,   638,     0,   688,
     453,   640,     0,   461,   688,   830,   462,     0,   463,   688,
     820,   675,     0,     0,     0,     0,     0,   168,     0,     0,
       0,   318,     0,     0,     0,   147,   252,   254,     0,   317,
       0,   320,     0,   842,   249,   151,   509,     0,     0,   447,
     828,     0,   227,   218,     0,   226,   739,   232,     0,   434,
     877,   320,   877,     0,   816,     0,   773,   320,     0,   320,
     253,   735,     0,   767,   717,   716,   710,     0,   712,   688,
     721,    70,   201,    77,    82,   101,   175,     0,   183,   189,
      70,   191,   884,     0,     0,   450,     0,   833,     0,   824,
     823,   674,     0,    70,   126,   881,     0,     0,     0,     0,
     169,   284,   282,   286,   606,    27,     0,   278,     0,   283,
     295,     0,   293,   298,     0,   297,     0,   296,     0,   130,
     256,     0,   258,     0,   800,     0,   510,   508,   519,   517,
     228,     0,   217,   225,   232,     0,   771,     0,     0,     0,
     143,   434,   877,   775,   149,   249,   153,   320,     0,   769,
       0,   724,     0,   197,     0,     0,    70,   178,   100,   190,
     899,   692,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   268,   272,     0,     0,   263,   570,   569,   566,   568,
     567,   587,   589,   588,   558,   529,   530,   548,   564,   563,
     525,   535,   536,   538,   537,   557,   541,   539,   540,   542,
     543,   544,   545,   546,   547,   549,   550,   551,   552,   553,
     554,   556,   555,   526,   527,   528,   531,   532,   534,   572,
     573,   582,   581,   580,   579,   578,   577,   565,   584,   574,
     575,   576,   559,   560,   561,   562,   585,   586,   590,   592,
     591,   593,   594,   571,   596,   595,   598,   600,   599,   533,
     603,   601,   602,   597,   583,   524,   290,   521,     0,   264,
     311,   312,   310,   303,     0,   304,   265,   337,     0,     0,
       0,     0,   130,   139,   248,     0,     0,     0,   230,     0,
     770,     0,    70,   313,    70,   133,     0,     0,     0,   145,
     877,   715,     0,    70,   176,    83,   102,     0,   449,   832,
     822,   488,   124,   266,   267,   340,   170,     0,     0,   287,
     279,     0,     0,     0,   292,   294,     0,     0,   299,   306,
     307,   305,     0,     0,   255,     0,     0,     0,     0,   250,
       0,   229,   772,     0,   504,   691,     0,     0,    70,   135,
     141,     0,   723,     0,     0,     0,   104,   269,    59,     0,
     270,   271,     0,     0,   285,   289,   522,   523,     0,   280,
     308,   309,   301,   302,   300,   338,   335,   259,   257,   339,
       0,   251,   505,   690,     0,   436,   314,     0,   137,     0,
     179,   489,     0,   128,     0,   320,   288,   291,     0,   735,
     261,     0,   502,   433,   438,   177,     0,     0,   105,   276,
       0,   319,   336,     0,   691,   331,   735,   503,     0,   127,
       0,     0,   275,   877,   735,   205,   332,   333,   334,   902,
     330,     0,     0,     0,   274,     0,   331,     0,   877,     0,
     273,   315,    70,   260,   902,     0,   209,   207,     0,    70,
       0,     0,   210,     0,   206,   262,     0,   316,     0,   213,
     204,     0,   212,   123,   214,     0,   203,   211,     0,   216,
     215
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   119,   789,   538,   180,   273,   496,
     500,   274,   497,   501,   121,   122,   123,   124,   125,   126,
     318,   568,   569,   450,   238,  1404,   456,  1327,  1405,  1633,
     749,   268,   491,  1596,   977,  1153,  1648,   334,   181,   570,
     831,  1034,  1205,   130,   541,   848,   571,   590,   850,   522,
     847,   572,   542,   849,   336,   289,   305,   133,   833,   792,
     775,   992,  1349,  1084,   898,  1545,  1408,   693,   904,   455,
     702,   906,  1238,   686,   887,   890,  1073,  1653,  1654,   560,
     561,   584,   585,   278,   279,   283,  1375,  1524,  1525,  1160,
    1276,  1368,  1520,  1639,  1656,  1557,  1600,  1601,  1602,  1356,
    1357,  1358,  1558,  1564,  1609,  1361,  1362,  1366,  1513,  1514,
    1515,  1535,  1683,  1277,  1278,   182,   135,  1669,  1670,  1518,
    1280,   136,   231,   451,   452,   137,   138,   139,   140,   141,
     142,   143,   144,  1388,   145,   830,  1033,   146,   235,   313,
     446,   547,   548,  1107,   549,  1108,   147,   148,   149,   150,
     727,   728,   151,   152,   265,   153,   266,   479,   480,   481,
     482,   483,   484,   485,   486,   487,   739,   740,   969,   488,
     489,   490,   746,  1585,   154,   543,  1377,   544,  1006,   797,
    1177,  1174,  1506,  1507,   155,   156,   157,   225,   232,   321,
     436,   158,   925,   732,   159,   926,   822,   813,   927,   874,
    1055,  1057,  1058,  1059,   876,  1217,  1218,   877,   667,   421,
     193,   194,   573,   563,   402,   403,   819,   161,   226,   184,
     163,   164,   165,   166,   167,   168,   169,   621,   170,   228,
     229,   525,   217,   218,   624,   625,  1117,  1118,   554,   555,
    1110,  1111,   298,   299,   783,   171,   515,   172,   559,   173,
    1526,   290,   329,   579,   580,   919,  1016,   773,   706,   707,
     708,   259,   260,   815
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1408
static const yytype_int16 yypact[] =
{
   -1408,   118, -1408, -1408,  6092, 12939, 12939,   -78, 12939, 12939,
   12939, 11094, 12939, -1408, 12939, 12939, 12939, 12939, 12939, 12939,
   12939, 12939, 12939, 12939, 12939, 12939, 14418, 14418, 11258, 12939,
   14501,   -71,   -61, -1408, -1408, -1408, -1408, -1408,   138, -1408,
      87, 12939, -1408,   -61,   -59,    -3,     4,   -61, 11422, 15795,
   11586, -1408, 13751,  5308,     7, 12939,  3174,    95, -1408, -1408,
   -1408,    88,    92,    36,    22,    75,   107,   136, -1408, 15795,
     141,   150, -1408, -1408, -1408, -1408, -1408,   281, 14003, -1408,
   -1408, 15795, -1408, -1408, -1408, -1408, 15795, -1408, 15795, -1408,
     143,   156,   163,   166, 15795, 15795, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, 12939, -1408, -1408,   165,   257,   263,   263, -1408,
     351,   234,   212, -1408,   190, -1408,    57, -1408,   365, -1408,
   -1408, -1408, -1408, 15597,   506, -1408, -1408,   215,   235,   253,
     255,   279,   306,  3255, -1408, -1408, -1408, -1408,   406, -1408,
   -1408,   417,   425,   309, -1408,   100,   313,   371, -1408, -1408,
     753,    81,  2069,   105,   294,   106,   111,   328,    35, -1408,
     110, -1408,   462, -1408, -1408, -1408,   382,   334,   380, -1408,
   -1408,   365,   506, 16050,  2198, 16050, 12939, 16050, 16050,  5613,
     335, 14935, 16050,   496, 15795,   476,   476,   135,   476,   476,
     476,   476,   476,   476,   476,   476,   476, -1408, -1408, 14146,
     375, -1408,   397,   423,   423, 14418, 14979,   358,   570, -1408,
     382, 14146,   375,   434,   436,   386,   114, -1408,   461,   105,
   11750, -1408, -1408, 12939,  9372,   583,    59, 16050, 10397, -1408,
   12939, 12939, 15795, -1408, -1408,  3341,   400, -1408,  3541, 13751,
   13751,   433, -1408, -1408,   410,  2370,   603, -1408,   616, -1408,
   15795,   554, -1408,   438,  3687,   442,   652, -1408,     3,  4133,
   15610, 15661, 15795,    65, -1408,    45, -1408, 13810,    67, -1408,
   -1408, -1408,   622,    68, 14418, 14418, 12939,   445,   473, -1408,
   -1408, 14282, 11258,    42,   292, -1408, 13103, 14418,   355, -1408,
   15795, -1408,   195,   234, -1408, -1408, -1408, -1408,  3337, 12939,
   12939,   631,   550,    41,   450, 16050,   451,   703,  6297, 12939,
      73,   449,   274,    73,   261,   283, -1408, 15795, 13751,   457,
   10561, 13751, -1408, -1408, 14705, -1408, -1408, -1408, -1408,   365,
   -1408, -1408, -1408, -1408, -1408, -1408, -1408, 12939, 12939, 12939,
   11955, 12939, 12939, 12939, 12939, 12939, 12939, 12939, 12939, 12939,
   12939, 12939, 12939, 12939, 12939, 12939, 12939, 12939, 12939, 12939,
   12939, 12939, 14501, 12939, -1408, 12939, 12939, -1408, 12939,  4342,
   15795, 15795, 15795, 15597,   548,   460,  5102, 12939, 12939, 12939,
   12939, 12939, 12939, 12939, 12939, 12939, 12939, 12939, 12939, -1408,
   -1408, 14543, -1408,   119, 12939, 12939, -1408, 10561, 12939, 12939,
     165,   120, 14282,   458,   365, 12119,  4204, -1408, 12939, -1408,
     459,   650, 14146,   467,   -23, 14565,   423, 12283, -1408, 12447,
   -1408,   470,   -20, -1408,   113, 10561, -1408, 14626, -1408,   121,
   -1408, -1408,  4262, -1408, -1408, 12611, -1408, 12939, -1408,   573,
    9577,   654,   474, 15962,   657,    46,    27, -1408, -1408, -1408,
   -1408, -1408, 13751, 15460,   475,   666, 14098, -1408,   494, -1408,
   -1408, -1408,   601, 12939,   604,   608, 12939, 12939, 12939, -1408,
     652, -1408, -1408, -1408, -1408, -1408, -1408, -1408,   501, -1408,
   -1408, -1408,   491, -1408, -1408, 15795,   490,   682,    48, 15795,
     493,   688,    54,    58, 15676, -1408, 15795, 12939,   423,    95,
   -1408, 14098,   617, -1408,   423,    50,    52,   502,   503,  1488,
     504, 15795,   580,   508,   423,    86,   511, 15543, 15795, -1408,
   -1408,   641,  1776,   -39, -1408, -1408, -1408,   234, -1408, -1408,
   -1408,   681,   586,   546,   249, -1408,   165,   587,   705,   531,
     584,   120, -1408, 15035,   533,   731,   543, 13751, 13751,   730,
     549,   738, -1408, 13751,    43,   689,    96, -1408, -1408, -1408,
   -1408, -1408, -1408,   981,  1865, -1408, -1408, -1408, -1408,   739,
     588, -1408, 14418, 12939,   557,   749, 16050,   747, -1408, -1408,
     638, 14724, 16177, 16260,  5613, 12939, 16006, 16369,  2999, 16401,
   16432,  1809,  5488,  5488,  5488,  5488,  2672,  2672,  2672,  2672,
     578,   578,   454,   454,   454,   135,   135,   135, -1408,   476,
   16050,   559,   564, 15079,   568,   763, -1408, 12939,   -40,   575,
     120, -1408, -1408, -1408, -1408,   365, 12939, 14199, -1408, -1408,
    5613, -1408,  5613,  5613,  5613,  5613,  5613,  5613,  5613,  5613,
    5613,  5613,  5613,  5613, 12939,   -40,   576,   572,  1993,   582,
     577,  2128,    89,   590, -1408, 16050,  4751, -1408, 15795, -1408,
     450,    43,   375, 14418, 16050, 14418, 15135,    79,   127, -1408,
     592, 12939, -1408, -1408, -1408,  9167,   301, -1408, 16050, 16050,
     -61, -1408, -1408, -1408, 12939,   679, 13948, 14098, 15795,  9782,
     585,   589, -1408,    55,   664,   646, -1408,   786,   600,  4873,
   13751, 14098, 14098, 14098, 14098, 14098, -1408,   602,    15,   655,
     609,   611,   613, 14098,   -26, -1408,   659, -1408, -1408,   614,
   -1408, 16136, -1408, 12939,   632, 16050,   634,   805,  5533,   811,
   -1408, 16050,  4944, -1408,   501,   742, -1408,  6502, 15527,   621,
     239, -1408, 15610, 15795,   272, -1408, 15661, 15795, 15795, -1408,
   -1408,  2270, -1408, 16136,   809, 14418,   624, -1408, -1408, -1408,
   -1408, -1408,   729,    97, 15527,   626, 14282, 14365,   815, -1408,
   -1408, -1408, -1408,   627, -1408, 12939, -1408, -1408,  5682, -1408,
   13751, 15527,   636, -1408, -1408, -1408, -1408,   825, 12939,  3337,
   -1408, -1408, 15733, 12939, -1408, 12939, -1408, -1408,   649, -1408,
   13751,   821,    18, -1408, -1408,    61, 14683, -1408,   128, -1408,
   -1408, 13751, -1408, -1408,   423, 16050, -1408, 10725, -1408, 14098,
      26,   658, 15527,   586, -1408, -1408, 16335, 12939, -1408, -1408,
   12939, -1408, 12939, -1408,  2330,   674, 10561,   580,   827,   586,
     638, 15795, 14501,   423,  2565,   677, 10561, -1408, -1408,   131,
   -1408, -1408,   848, 14769, 14769,  4751, -1408, -1408, -1408, -1408,
     680,    21,   683, -1408, -1408, -1408,   855,   684,   459,   423,
     423, 12775, -1408,   132, -1408, -1408,  2609,   333,   -61, 10397,
   -1408,  6707,   686,  6912,   693, 13948, 14418,   690,   752,   423,
   16136,   868, -1408, -1408, -1408, -1408,   345, -1408,    14, 13751,
   -1408, 13751, 15795, 15460, -1408, -1408, -1408,   875, -1408,   695,
     739,   471,   471,   830,   830, 15279,   692,   890, 14098,   757,
   15795,  3337, 14098, 14098,  4719, 15748, 14098, 14098, 14098, 14098,
   13900, 14098, 14098, 14098, 14098, 14098, 14098, 14098, 14098, 14098,
   14098, 14098, 14098, 14098, 14098, 14098, 14098, 14098, 14098, 14098,
   14098, 14098, 14098, 16050, 12939, 12939, 12939, -1408, -1408, -1408,
   12939, 12939, -1408,   652, -1408,   824, -1408, -1408, 15795, -1408,
   -1408, 15795, -1408, -1408, -1408, -1408, 14098,   423, -1408, 13751,
   15795, -1408,   896, -1408, -1408,    91,   711,   423, 10930, -1408,
    1732, -1408,  5887,   550,   896, -1408,   246,   240, 16050,   784,
   -1408, 16050, 15179, -1408,   714, 13751,   583, 13751,   838,   904,
     840, 12939,   -40,   722, -1408, 14418, 12939, 16050, 16136,   724,
      26, -1408,   720,    26,   726, 16335, 16050, 15235,   727, 10561,
     728,   733, 13751,   734,   586, -1408,   386,   732, 10561,   737,
   12939, -1408, -1408, -1408, -1408, -1408, -1408,   800,   735,   920,
    4751,   788, -1408,  3337,  4751, -1408, -1408, -1408, 14418, 16050,
   -1408,   -61,   907,   870, 10397, -1408, -1408, -1408,   741, 12939,
     752,   423, 14282, 13948,   746, 14098,  7117,   448,   756, 12939,
      32,    24, -1408,   789, -1408,   832, -1408, 13686,   933,   765,
   14098, -1408, 14098, -1408,   767, -1408,   839,   960,   771, 15335,
     772,   963,   774, -1408, -1408, -1408, 15377,   773,   969, 16220,
   16300, 13309, 14098, 16094, 14078,  4601,  2754, 14329,  1674, 11933,
   11933, 11933, 11933,  1907,  1907,  1907,  1907,   687,   687,   471,
     471,   471,   830,   830,   830,   830, 16050, 13493, 16050, -1408,
   16050, -1408,   780, -1408, -1408, -1408, 16136, -1408,   884, 15527,
     452, -1408, 14282, -1408, -1408,  5613,   779, -1408,   781,   480,
   -1408,    60, 12939, -1408, -1408, -1408, 12939, -1408, 12939, 12939,
   -1408, -1408, -1408,   122,   970, 14098, -1408,  2857,   785, 10561,
     423, 16050,   821,   791, -1408,   793,    26, 12939, 10561,   794,
   -1408, -1408,   550, -1408,   795,   787, -1408, 10561,   796, -1408,
    4751, -1408,  4751, -1408,   798, -1408,   859,   806,   985, -1408,
     423,   978, -1408,   799, -1408, -1408,   813,   814,    93, -1408,
   -1408, 16136,   816,   817, -1408,  3145, -1408, -1408, -1408, -1408,
   -1408, 13751, -1408, 13751, -1408, 16136, 15433, -1408, 14098,  3337,
   -1408, -1408, 14098, -1408, 14098, -1408, -1408, 14098, -1408, 14098,
   -1408,  4083, 14098, 12939,   819,  7322, 13751, -1408,     0, 13751,
   15527, -1408, 15479,   866,  1484, -1408, -1408, -1408,   548, 13607,
      69,   460,    94, -1408, -1408, -1408,   871,  2956,  3083, 16050,
   16050,   945,  1011,   947, 14098, 16136,   835, 10561,   833,   924,
     821,   834,   821,   842, 16050,   843, -1408,  1005,   836,  1099,
   -1408,    26,   845, -1408, -1408,   910, -1408,  4751, -1408,  3337,
   -1408, -1408,  9167, -1408, -1408, -1408, -1408,  9987, -1408, -1408,
   -1408,  9167, -1408,   846, 14098, 16136,   911, 16136, 15475, 16136,
   15531,  4083,  5578, -1408, -1408, -1408, 15527, 15527,  1032,    63,
   -1408, -1408, -1408, -1408,    70,   847,    71, -1408, 13308, -1408,
   -1408,    72, -1408, -1408, 14264, -1408,   849, -1408,   968,   365,
   -1408, 13751, -1408,   548, -1408, 13431, -1408, -1408, -1408, -1408,
    1037, 14098, -1408, 16136, 10561,   853, -1408,   856,   852,   -69,
   -1408,   924,   821, -1408, -1408, -1408, -1408,  1375,   860, -1408,
    4751, -1408,   927,  9167, 10192,  9987, -1408, -1408, -1408,  9167,
   -1408, 16136, 14098, 14098, 14098, 12939,  7527,   858,   861, 14098,
   15527, -1408, -1408,  2114, 15479, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1408, -1408, -1408,   108, -1408,   866, -1408,
   -1408, -1408, -1408, -1408,    34,   431, -1408,  1048,    74, 15795,
     968,  1049,   365, -1408, -1408,   864,  1051, 14098, 16136,   867,
   -1408,    77, -1408, -1408, -1408, -1408,   873,   -69, 13516, -1408,
     821, -1408,  4751, -1408, -1408, -1408, -1408,  7732, 16136, 16136,
   16136,  3791, -1408, -1408, -1408, 16136, -1408,  3723,    33, -1408,
   -1408, 14098, 13308, 13308,  1012, -1408, 14264, 14264,   464, -1408,
   -1408, -1408, 14098,   990, -1408,   878,    76, 14098, 15795, -1408,
   14098, 16136, -1408,   992, -1408,  1068,  7937,  8142, -1408, -1408,
   -1408,   -69, -1408,  8347,   879,   999,   972, -1408,   988,   934,
   -1408, -1408,   991,  2114, -1408, 16136, -1408, -1408,   929, -1408,
    1053, -1408, -1408, -1408, -1408, 16136,  1075, -1408, -1408, 16136,
     892, 16136, -1408,    82,   893, -1408, -1408,  8552, -1408,   900,
   -1408, -1408,   909,   937, 15795,   460, -1408, -1408, 14098,    28,
   -1408,  1030, -1408, -1408, -1408, -1408, 15527,   621, -1408,   949,
   15795,   499, 16136,   913,  1104,   441,    28, -1408,  1035, -1408,
   15527,   916, -1408,   821,    30, -1408, -1408, -1408, -1408, 13751,
   -1408,   925,   928,    78, -1408,   -66,   441,   290,   821,   930,
   -1408, -1408, -1408, -1408, 13751,  1047,  1114,  1050,   -66, -1408,
    8757,   291,  1116, 14098, -1408, -1408,  8962, -1408,  1054,  1118,
    1056, 14098, 16136, -1408,  1122, 14098, -1408, 16136, 14098, 16136,
   16136
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1408, -1408, -1408,  -498, -1408, -1408, -1408,    -4, -1408, -1408,
   -1408,   633,   387,   389,    38,  1169,  3625, -1408,  2520, -1408,
    -300, -1408,     9, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1408,  -509, -1408, -1408,  -172,    -1,     8,
   -1408, -1408, -1408, -1408, -1408, -1408,    16, -1408, -1408, -1408,
   -1408,    17, -1408, -1408,   759,   770,   761,  -125,   297,  -789,
     304,   362,  -506,    83,  -843, -1408,  -250, -1408, -1408, -1408,
   -1408,  -645,   -79, -1408, -1408, -1408, -1408,  -499, -1408,  -772,
   -1408,  -381, -1408, -1408,   656, -1408,  -231, -1408, -1408,  -955,
   -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
    -258, -1408, -1408, -1408, -1408, -1408,  -340, -1408,  -105, -1136,
   -1408, -1407,  -518, -1408,  -156,    13,  -133,  -504, -1408,  -346,
   -1408,   -73,   -14,  1134,  -657,  -359, -1408, -1408,   -30, -1408,
   -1408,  3635,   -65,  -215, -1408, -1408, -1408, -1408, -1408, -1408,
     161,  -767, -1408, -1408, -1408, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1408,   792, -1408, -1408,   206, -1408,   700,
   -1408, -1408, -1408, -1408, -1408, -1408, -1408,   211, -1408,   704,
   -1408, -1408,   439, -1408,   180, -1408, -1408, -1408, -1408, -1408,
   -1408, -1408, -1408, -1113, -1408,  1988,   177,  -344, -1408, -1408,
     142,  3438,  4142, -1408, -1408,   259,  -211,  -579, -1408, -1408,
     325,  -632,   130, -1408, -1408, -1408, -1408, -1408,   314, -1408,
   -1408, -1408,    -2,  -793,  -190,  -182,  -132, -1408, -1408,    66,
   -1408, -1408, -1408, -1408,    39,  -148, -1408,    90, -1408, -1408,
   -1408,  -382,   899, -1408, -1408, -1408, -1408, -1408,   883, -1408,
     265, -1408,   487,   236, -1408, -1408,   915,  -284,  -952, -1408,
     -47,   -80,  -186,  -252,   492, -1408, -1151, -1408,   305, -1408,
   -1408, -1408,  -239,  -992
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -870
static const yytype_int16 yytable[] =
{
     120,   340,   160,   128,   384,   258,   828,   430,   306,   413,
     460,   461,   129,   127,   311,   312,   465,   134,   234,  1017,
     131,   132,   411,   263,   552,  1183,   659,   638,   618,   239,
     662,   406,  1009,   243,   875,   383,   433,   894,   438,   314,
     788,  1299,  1603,  1566,  1032,   246,   439,   316,   256,   340,
    1169,  1168,  1080,   337,   680,   698,   908,   700,  1029,   765,
    1043,   765,  1236,    13,   909,   288,   331,  1567,   447,   227,
     162,    13,  1420,    13,   504,    13,   509,   512,  1371,  -281,
    1424,  1508,   304,  1573,   288,  1573,    13,  1420,   401,   581,
     288,   288,   213,   214,   275,   777,  1018,   440,   777,   929,
     777,   408,   777,   777,   401,  1061,   990,   401,   527,   404,
    1286,   492,  1583,  1089,  1090,   282,   302,  1641,     3,   303,
     186,  -869,  1561,  1089,  1090,  1346,  1347,   230,   328,   288,
    1589,  1533,  1534,   339,  1681,  1682,   423,   233,  1562,   240,
    1019,   506,   328,   685,   753,   404,  -439,  -759,   431,  1389,
     757,  1391,   293,  -468,   758,  1563,  1584,  1291,  -869,   295,
     551,  1642,  -763,   787,  1106,  1062,  -756,   591,   493,   528,
     816,   404,  -757,   385,   276,  -869,   566,  -758,   317,   678,
    -794,   414,  -609,  -608,  1628,  -760,   408,  -795,   372,   207,
     420,   747,   207,  -797,  -761,   241,   327,  -762,  -796,   327,
     373,  1292,   242,   212,   212,   327,   437,   224,   267,   327,
     669,   280,   307,   517,  -690,   281,  1092,  -690,   296,   297,
     284,   518,  1020,   703,   910,  -222,  1239,  -208,   701,  -690,
     120,   630,  1237,  1568,   120,  1604,   409,  -222,   454,  1301,
    1229,  1537,   663,   444,   405,   699,  1307,   449,  1309,   766,
    1308,   767,   340,   630,   589,  1204,   467,   332,  1193,   448,
    1087,  1195,  1091,  1421,  1422,   505,   991,   510,   513,  1372,
    -281,  1425,  1509,   285,  1574,   424,  1618,   630,  1680,  -765,
     405,   426,  -759,  1293,   587,   778,   630,   432,   862,   630,
    1161,  1002,  1326,  1374,  -766,   307,  1216,  -763,   277,   794,
     162,  -756,   306,   337,   162,   286,   405,  -757,   498,   502,
     503,   562,  -758,   410,   120,  -794,   115,   128,   808,   809,
    -760,   409,  -795,   578,   814,  1685,  1698,   256,  -797,  -761,
     288,   134,  -762,  -796,   287,   978,   293,   800,   537,   291,
     293,   320,   293,   508,   806,   534,   327,   323,   292,   670,
     514,   514,   519,   293,   308,  1397,   639,   524,   534,  -869,
     293,   309,   293,   533,   310,   294,   319,   534,   981,  1686,
    1699,   293,   888,   889,   817,   628,   288,   632,   288,   288,
     328,   326,   818,   635,   162,   327,   212,   891,   330,  1591,
     327,   893,   212,  1175,   995,   539,   540,   655,   212,  1170,
     333,   795,   296,   297,  1071,  1072,   296,   297,   296,   297,
    -869,   227,  1171,  -869,   843,   341,   796,   384,   629,   296,
     297,   672,  1226,   327,  1303,   295,   296,   297,   296,   297,
    1610,  1611,  1219,   682,   293,   342,   529,   296,   297,   534,
     656,  1176,   845,  1088,  1089,  1090,   120,  1172,   383,  1606,
    1607,  1687,  1700,   343,   212,   344,  1268,   917,   920,   692,
    1569,   212,   212,   851,   629,  1040,   622,   576,   212,   855,
     581,   581,  -466,   679,   212,  1049,   683,  1570,   524,   345,
    1571,   817,  1336,   375,  1268,   550,   845,   882,   424,   818,
     577,   376,   407,  1612,   657,   883,   914,    13,   660,   535,
     296,   297,   760,   369,   370,   371,   346,   372,  1046,   377,
    1613,   835,  1675,  1614,   378,   552,   162,   772,   379,   373,
     959,   960,   961,   782,   784,    13,  -764,  1688,  -467,  -608,
     530,   300,   412,   750,   536,   417,   962,   754,   433,  1398,
     419,   373,   275,   328,   425,   884,  1233,  1089,  1090,   224,
     401,  1003,  1402,  1666,  1667,  1668,   530,  1269,   536,   530,
     536,   536,  1270,   428,    58,    59,    60,   174,   175,   338,
    1271,  1014,    58,    59,    60,   174,   175,   338,  1314,   429,
    1315,  -607,  1024,   434,   435,  1269,   385,   288,   437,   212,
    1270,   445,    58,    59,    60,   174,   175,   338,  1271,   212,
     458,   462,  1086,   322,   324,   325,  1272,  1273,   463,  1274,
      51,    58,    59,    60,   174,   175,   338,  -864,    58,    59,
      60,   174,   175,   338,   366,   367,   368,   369,   370,   371,
     466,   372,    97,   468,  1272,  1273,   511,  1274,   469,  1163,
      97,   521,   471,   373,   520,   557,   558,   552,   824,   564,
     565,   551,    51,   575,  1275,   -65,   588,   666,  1199,   668,
      97,   690,   873,   447,   878,  1065,   671,  1208,   630,   677,
    1093,   697,  1094,   709,   694,   710,   892,  1677,   733,    97,
     734,   120,  1285,   736,   128,  1401,    97,   737,   745,   748,
     751,   752,  1691,   755,   901,   120,   764,   756,   134,  1662,
    1228,   768,   769,   853,   771,   774,   776,   785,   903,  1099,
     779,   790,   791,   793,   799,   798,  1103,   415,   387,   388,
     389,   390,   391,   392,   393,   394,   395,   396,   397,   398,
     801,   802,   804,   956,   957,   958,   959,   960,   961,   879,
     805,   880,   807,   120,   810,   811,   128,   812,   821,   980,
    1157,   162,   962,   983,   984,  -469,   826,   823,   827,   212,
     134,   829,   899,   832,   838,   162,   399,   400,  1541,   839,
     841,  1045,   842,   846,   856,   895,  1181,   857,   814,   552,
    1282,   859,   860,   551,   120,   905,   160,   128,   834,   907,
     498,   885,   911,   912,   502,   913,   129,   127,  1010,   915,
     928,   134,   930,  1202,   131,   132,   935,   931,  1298,   932,
     562,   933,  1022,   162,   212,   936,   964,  1305,   965,   966,
     970,   973,   976,   986,   988,   989,  1312,   994,   562,   998,
     401,   987,   999,   472,   473,   474,  1188,  1005,  1268,  1007,
     475,   476,   524,   997,   477,   478,  1655,   288,  1213,  1013,
     212,  1015,   212,  1265,   162,  1023,  1030,  1042,  1024,  1054,
    1054,   873,  1050,  1655,  1064,    58,    59,    60,    61,    62,
     338,  1676,  1039,   212,  1074,  1048,    68,   380,  1060,    13,
    1083,  1063,  1085,  1066,  1097,   120,  1077,   120,  1082,   120,
     128,   227,   128,  1079,  1098,   962,  1250,  1101,  1075,  1102,
    1255,   529,   566,  1152,   134,  1159,   134,  1260,  1095,  1162,
    1592,   381,  1178,   382,  1180,   551,  1385,  1184,  1185,  1186,
    1189,  1194,  1322,  1192,  1196,  1198,  1105,  1200,  1210,  1212,
    1207,  1115,  1215,    97,  1201,  1203,  1209,  1222,  1331,  1269,
    1211,  1225,   212,  1223,  1270,  1230,    58,    59,    60,   174,
     175,   338,  1271,   212,   212,   162,  1234,   162,  1240,   162,
    1241,   899,  1081,  1243,  1244,   552,  1247,  1248,  1164,  1249,
    1251,  1253,  1254,  1256,  1154,  1258,   550,  1155,  1259,  1264,
    1266,  1283,  1284,  1297,  1294,  1311,  1158,  1317,  1272,  1273,
    1300,  1274,  1302,  1306,  1319,  1313,  1310,  1316,   120,  1323,
     160,   128,  1332,  1529,  1333,  1318,  1624,  1320,  1321,  1268,
     129,   127,  1324,  1325,    97,   134,  1328,  1329,   131,   132,
    1343,  1403,  1360,  1376,  1380,  1381,  1382,  1345,   562,   224,
    1409,   562,  1386,  1384,  1387,   552,  1390,  1395,  1400,  1412,
    1370,  1392,  1393,  1416,  1399,  1410,  1419,  1517,  1423,  1516,
      13,  1527,  1530,  1532,  1531,  1542,   873,  1221,  1553,  1540,
     873,  1554,  1572,  1577,  1579,  1580,  1582,  1608,   162,  1616,
     120,  1622,   212,   212,  1588,  1665,  1617,  1623,  1631,  1630,
    1632,  1634,   120,  1224,  -277,   128,  1567,  1635,  1637,  1638,
    1640,  1190,  1643,    58,    59,    60,    61,    62,   338,   134,
    1645,   551,  1647,  1268,    68,   380,  1547,  1646,   550,  1657,
    1269,  1660,  1663,  1664,  1672,  1270,  1674,    58,    59,    60,
     174,   175,   338,  1271,  1678,  1373,  1692,  1679,  1693,  1694,
    1701,  1689,  1705,  1704,  1220,  1706,  1708,   759,  1659,   979,
     162,   382,   634,   633,    13,   982,   340,  1044,   524,   899,
     631,  1041,   162,  1004,  1673,  1546,  1330,  1671,  1281,  1272,
    1273,    97,  1274,  1227,  1538,   762,  1560,  1281,  1565,  1367,
    1695,   551,  1684,  1279,  1576,   236,  1536,  1182,   641,  1151,
     743,  1149,  1279,   972,   744,    97,  1173,  1104,  1206,  1056,
    1214,   526,  1067,   556,   562,   210,   210,  1519,  1112,   222,
     516,   918,   212,     0,  1269,     0,   873,  1394,   873,  1270,
       0,    58,    59,    60,   174,   175,   338,  1271,  1096,     0,
       0,   222,  1348,     0,     0,     0,     0,     0,   524,     0,
       0,     0,  1586,     0,  1587,     0,     0,     0,     0,     0,
     550,     0,     0,  1593,     0,   212,     0,     0,     0,     0,
       0,     0,     0,  1272,  1273,     0,  1274,     0,     0,   212,
     212,   120,     0,     0,   128,   256,     0,     0,     0,     0,
    1365,     0,     0,     0,     0,     0,     0,  1369,   134,    97,
       0,     0,     0,     0,     0,     0,     0,     0,  1627,     0,
       0,     0,     0,     0,   385,     0,     0,     0,     0,  1281,
       0,  1396,     0,     0,     0,  1281,     0,  1281,     0,   562,
       0,     0,     0,   873,  1279,     0,     0,     0,   120,     0,
    1279,   128,  1279,   120,  1521,     0,     0,   120,     0,     0,
     128,   162,     0,     0,     0,   134,  1407,     0,     0,   212,
       0,     0,     0,     0,   134,     0,     0,     0,     0,     0,
    1578,     0,     0,     0,  1505,     0,     0,     0,     0,     0,
    1512,     0,     0,     0,     0,     0,     0,   256,     0,     0,
       0,   256,  1522,     0,     0,     0,     0,     0,   210,  1268,
       0,     0,  1690,     0,   210,     0,     0,     0,   162,  1696,
     210,     0,     0,   162,     0,  1281,   873,   162,     0,   120,
     120,   120,   128,     0,     0,   120,     0,     0,   128,     0,
    1279,     0,   120,  1544,  1407,   128,   134,     0,   222,   222,
      13,     0,   134,     0,   222,     0,   550,     0,     0,   134,
     814,     0,     0,     0,     0,     0,     0,     0,     0,  1575,
       0,     0,     0,     0,     0,   814,   210,     0,     0,     0,
       0,     0,     0,   210,   210,     0,     0,     0,     0,     0,
     210,     0,     0,     0,     0,     0,   210,     0,     0,   162,
     162,   162,     0,     0,     0,   162,     0,   222,     0,  1650,
    1269,     0,   162,     0,     0,  1270,     0,    58,    59,    60,
     174,   175,   338,  1271,     0,     0,   550,   222,  1620,     0,
     222,     0,   415,   387,   388,   389,   390,   391,   392,   393,
     394,   395,   396,   397,   398,   288,     0,     0,   340,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1272,
    1273,     0,  1274,     0,   256,     0,     0,     0,   873,     0,
       0,   222,     0,   120,     0,     0,   128,     0,     0,     0,
       0,   399,   400,  1598,     0,    97,     0,     0,  1505,  1505,
     134,    36,  1512,  1512,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   288,     0,     0,  1539,     0,     0,
       0,   210,   120,   120,     0,   128,   128,     0,     0,   120,
       0,   210,   128,     0,     0,     0,     0,     0,     0,   134,
     134,     0,     0,     0,     0,  1363,   134,     0,     0,     0,
       0,     0,     0,   162,     0,   401,     0,     0,     0,     0,
       0,     0,     0,   120,     0,     0,   128,     0,     0,     0,
    1649,   222,   222,     0,     0,   724,     0,   562,    83,    84,
     134,    85,   179,    87,     0,     0,  1661,     0,  1651,     0,
       0,     0,   162,   162,   562,     0,     0,     0,     0,   162,
       0,     0,   562,     0,     0,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
     724,     0,  1364,     0,     0,     0,   120,   770,     0,   128,
       0,     0,   120,   162,     0,   128,     0,     0,     0,     0,
       0,     0,     0,   134,     0,     0,     0,     0,     0,   134,
     946,   947,   948,   949,   950,   951,   952,   953,   954,   955,
     956,   957,   958,   959,   960,   961,   222,   222,     0,     0,
       0,     0,   222,     0,     0,     0,     0,     0,     0,   962,
       0,     0,   347,   348,   349,     0,     0,     0,     0,     0,
       0,   210,     0,     0,     0,     0,   162,     0,     0,     0,
       0,   350,   162,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,     0,   372,   347,   348,   349,     0,
       0,     0,     0,     0,     0,     0,     0,   373,     0,     0,
       0,     0,     0,     0,     0,   350,   210,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,     0,   372,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   373,   210,     0,   210,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,     0,   372,     0,     0,   210,   724,     0,     0,     0,
       0,     0,     0,     0,   373,   347,   348,   349,   222,   222,
     724,   724,   724,   724,   724,     0,     0,     0,     0,     0,
       0,     0,   724,     0,   350,     0,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   222,   372,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     373,     0,     0,     0,   210,     0,     0,  1166,     0,     0,
       0,     0,     0,   222,     0,   210,   210,  -870,  -870,  -870,
    -870,   954,   955,   956,   957,   958,   959,   960,   961,   222,
     222,     0,     0,     0,     0,     0,     0,     0,   222,     0,
       0,     0,   962,     0,     0,     0,     0,     0,   786,   222,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     222,     0,     0,     0,     0,     0,     0,     0,   724,     0,
       0,   222,     0,   347,   348,   349,     0,     0,     0,     0,
       0,     0,     0,     0,   211,   211,     0,     0,   223,     0,
       0,   222,   350,     0,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,     0,   372,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   373,     0,
       0,     0,     0,     0,   210,   210,     0,   820,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   222,     0,
     222,     0,   222,   386,   387,   388,   389,   390,   391,   392,
     393,   394,   395,   396,   397,   398,     0,   724,     0,     0,
     222,   724,   724,     0,     0,   724,   724,   724,   724,   724,
     724,   724,   724,   724,   724,   724,   724,   724,   724,   724,
     724,   724,   724,   724,   724,   724,   724,   724,   724,   724,
     724,   724,   399,   400,     0,     0,     0,     0,   347,   348,
     349,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   724,     0,   350,   222,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
       0,   372,     0,     0,   222,     0,   222,     0,    33,    34,
      35,     0,     0,   373,   210,   858,   401,     0,     0,   716,
       0,     0,     0,   211,     0,     0,     0,     0,     0,     0,
       0,   222,   415,   387,   388,   389,   390,   391,   392,   393,
     394,   395,   396,   397,   398,     0,     0,     0,     0,     0,
       0,     0,   222,     0,     0,     0,     0,   210,     0,     0,
       0,     0,     0,     0,     0,     0,    72,    73,    74,    75,
      76,   210,   210,     0,   724,     0,     0,   718,     0,     0,
       0,   399,   400,    79,    80,   211,   222,     0,     0,   724,
       0,   724,   211,   211,     0,     0,     0,    89,     0,   211,
     347,   348,   349,     0,     0,   211,     0,     0,     0,     0,
       0,   724,     0,    96,     0,     0,   211,     0,     0,   350,
       0,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,     0,   372,     0,   401,     0,     0,   222,     0,
     861,   210,     0,     0,     0,   373,     0,     0,     0,     0,
     347,   348,   349,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   724,     0,     0,     0,     0,   350,
     223,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,     0,   372,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   373,     0,     0,     0,   249,
     211,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     222,     0,   222,     0,     0,     0,     0,   724,   222,     0,
       0,   724,     0,   724,     0,   250,   724,     0,   724,     0,
       0,   724,     0,     0,     0,   222,     0,     0,   222,   222,
       0,   222,     0,     0,     0,     0,     0,    36,   222,     0,
       0,     0,     0,     0,   729,     0,     0,     0,     0,     0,
       0,     0,     0,   724,     0,     0,     0,     0,     0,     0,
       0,     0,   985,     0,   464,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   222,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   729,
     251,   252,     0,   724,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   222,   222,     0,   178,     0,
       0,    81,   253,     0,    83,    84,     0,    85,   179,    87,
       0,     0,  1038,     0,     0,     0,     0,     0,     0,     0,
     222,     0,   254,     0,   222,     0,     0,     0,     0,     0,
     724,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,     0,   255,     0,
     211,     0,   257,     0,     0,   347,   348,   349,     0,     0,
       0,   724,   724,   724,     0,     0,     0,     0,   724,   222,
       0,     0,     0,   222,   350,     0,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,     0,   372,   347,
     348,   349,     0,     0,     0,   211,     0,     0,     0,     0,
     373,     0,     0,     0,     0,     0,     0,     0,   350,     0,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   211,   372,   211,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   373,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   211,   729,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   724,     0,     0,   729,
     729,   729,   729,   729,     0,     0,     0,   222,     0,     0,
       0,   729,  -870,  -870,  -870,  -870,   364,   365,   366,   367,
     368,   369,   370,   371,     0,   372,   222,     0,     0,     0,
     724,     0,     0,     0,     0,     0,   975,   373,     0,     0,
       0,   724,     0,     0,     0,     0,   724,     0,     0,   724,
       0,     0,     0,   211,     0,     0,     0,     0,     0,     0,
       0,     0,   993,     0,   211,   211,     0,  1047,     0,   257,
     257,     0,     0,     0,     0,   257,     0,     0,     0,   993,
       0,     0,     0,     0,     0,     0,     0,   211,   944,   945,
     946,   947,   948,   949,   950,   951,   952,   953,   954,   955,
     956,   957,   958,   959,   960,   961,     0,   724,     0,     0,
       0,  1070,     0,     0,     0,   222,     0,   729,     0,   962,
    1031,     0,     0,     0,     0,     0,     0,     0,     0,   222,
       0,     0,     0,     0,     0,     0,     0,     0,   222,     0,
     223,     0,     0,     0,     0,     0,     0,     0,   257,     0,
       0,   257,     0,   222,     0,     0,     0,     0,     0,     0,
       0,     0,   724,     0,     0,     0,     0,   347,   348,   349,
     724,     0,     0,     0,   724,     0,     0,   724,     0,     0,
       0,     0,     0,   211,   211,     0,   350,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,     0,
     372,     0,     0,     0,     0,     0,   729,     0,     0,   211,
     729,   729,   373,     0,   729,   729,   729,   729,   729,   729,
     729,   729,   729,   729,   729,   729,   729,   729,   729,   729,
     729,   729,   729,   729,   729,   729,   729,   729,   729,   729,
     729,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   347,   348,   349,     0,
       0,     0,     0,     0,   729,     0,     0,     0,     0,     0,
       0,     0,   257,   705,     0,   350,   726,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,     0,   372,
       0,     0,     0,   211,     0,     0,     0,     0,     0,     0,
       0,   373,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   726,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   211,   372,     0,     0,     0,   211,     0,     0,  1296,
       0,     0,     0,     0,   373,     0,     0,     0,     0,     0,
     211,   211,     0,   729,     0,     0,     0,   257,   257,     0,
       0,     0,     0,   257,     0,     0,     0,     0,   729,     0,
     729,     0,     0,   347,   348,   349,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     729,     0,   350,     0,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,     0,   372,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1267,   373,     0,
     211,     0,     0,     0,     0,   347,   348,   349,  1378,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   729,   350,  1236,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,     0,   372,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     373,     0,     0,     0,     0,     0,     0,   726,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   257,
     257,   726,   726,   726,   726,   726,   729,   211,     0,     0,
     729,     0,   729,   726,     0,   729,     0,   729,     0,     0,
     729,    36,     0,     0,     0,     0,     0,     0,  1350,     0,
    1359,     0,     0,     0,     0,   347,   348,   349,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   270,   271,
       0,     0,   729,     0,   350,  1379,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   211,   372,     0,
     257,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     373,     0,   729,     0,     0,   272,     0,     0,    83,    84,
     257,    85,   179,    87,  1417,  1418,     0,     0,     0,     0,
       0,   257,     0,     0,     0,  1237,     0,     0,     0,   726,
       0,   347,   348,   349,     0,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   729,
     350,     0,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,     0,   372,     0,     0,     0,     0,     0,
     729,   729,   729,     0,     0,     0,   373,   729,  1556,     0,
       0,     0,  1359,     0,    36,     0,   207,     0,     0,     0,
       0,     0,   545,     0,     0,     0,     0,     0,     0,   257,
       0,   257,     0,   705,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   726,     0,
       0,     0,   726,   726,   208,   374,   726,   726,   726,   726,
     726,   726,   726,   726,   726,   726,   726,   726,   726,   726,
     726,   726,   726,   726,   726,   726,   726,   726,   726,   726,
     726,   726,   726,     0,     0,   178,     0,     0,    81,    82,
       0,    83,    84,     0,    85,   179,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   726,     0,     0,   257,
       0,     0,     0,     0,     0,   729,     0,     0,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,     0,   257,     0,   257,     0,     0,
     546,   457,     0,     0,     0,     0,     0,     0,     0,   729,
       0,   347,   348,   349,     0,     0,     0,     0,     0,     0,
     729,     0,   257,     0,     0,   729,     0,     0,   729,     0,
     350,     0,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,     0,   372,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   726,   373,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   257,     0,     0,
     726,     0,   726,     0,     0,     0,   729,     0,     0,     0,
       0,     0,     0,     0,  1658,     0,     0,     0,     0,     0,
     183,   185,   726,   187,   188,   189,   191,   192,  1350,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,     0,     0,   216,   219,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   237,     0,     0,     0,
       0,   729,     0,   245,     0,   248,     0,     0,   264,   729,
     269,     0,     0,   729,     0,     0,   729,   347,   348,   349,
       0,     0,     0,     0,     0,   726,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   350,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,     0,
     372,   459,     0,     0,     0,     0,     0,   315,     0,     0,
       0,     0,   373,     0,     0,     0,     0,     0,     0,     0,
       0,   257,     0,   257,     0,     0,     0,     0,   726,     0,
       0,     0,   726,     0,   726,     0,     0,   726,     0,   726,
       0,     0,   726,     0,     0,     0,   257,     0,     0,   257,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   257,
      36,   347,   348,   349,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   726,     0,     0,     0,     0,     0,
     350,   416,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,     0,   372,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   726,     0,   373,     0,     0,     0,
       0,     0,     0,     0,     0,   442,     0,     0,   442,     0,
       0,   178,     0,     0,    81,   237,   453,    83,    84,     0,
      85,   179,    87,     0,     0,     0,     0,   470,     0,     0,
       0,   257,     0,     0,     0,   257,     0,     0,     0,     0,
       0,   726,     0,     0,   730,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
       0,   315,     0,     0,     0,  1597,     0,   216,     0,     0,
       0,   532,   726,   726,   726,     0,     0,     0,     0,   726,
       0,     0,     0,     0,   553,   553,     0,     0,     0,   730,
       0,     0,     0,     0,   574,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   586,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1595,     0,
       0,     0,   592,   593,   594,   596,   597,   598,   599,   600,
     601,   602,   603,   604,   605,   606,   607,   608,   609,   610,
     611,   612,   613,   614,   615,   616,   617,     0,   619,     0,
     620,   620,     0,   623,     0,     0,     0,     0,     0,     0,
       0,   640,   642,   643,   644,   645,   646,   647,   648,   649,
     650,   651,   652,   653,     0,     0,     0,     0,     0,   620,
     658,     0,   586,   620,   661,     0,     0,   726,     0,     0,
     640,     0,     0,   665,     0,     0,     0,     0,   257,     0,
       0,     0,   674,     0,   676,     0,     0,     0,     0,     0,
     586,     0,     0,     0,     0,     0,     0,  1599,     0,     0,
     688,   726,   689,     0,     0,     0,     0,     0,     0,     0,
       0,   725,   726,     0,     0,     0,     0,   726,     0,     0,
     726,     0,     0,     0,     0,     0,     0,     0,   735,     0,
       0,   738,   741,   742,   941,   942,   943,   944,   945,   946,
     947,   948,   949,   950,   951,   952,   953,   954,   955,   956,
     957,   958,   959,   960,   961,   730,   725,     0,     0,     0,
       0,     0,   761,   347,   348,   349,     0,     0,   962,   730,
     730,   730,   730,   730,     0,     0,     0,     0,   726,     0,
       0,   730,   350,     0,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,     0,   372,     0,     0,   257,
       0,     0,     0,     0,     0,     0,     0,     0,   373,     0,
       0,     0,     0,     0,   257,     0,     0,     0,     0,     0,
       0,     0,     0,   726,   347,   348,   349,     0,   825,     0,
       0,   726,     0,     0,     0,   726,     0,     0,   726,     0,
     836,     0,     0,   350,     0,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,     0,   372,     0,     0,
       0,     0,   844,     0,     0,     0,     0,   730,     0,   373,
       0,   191,   347,   348,   349,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   854,
       0,   350,     0,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,     0,   372,   886,     0,     0,     0,
       0,     0,   725,     0,     0,     0,     0,   373,     0,   237,
       0,     0,     0,   494,     0,     0,   725,   725,   725,   725,
     725,     0,     0,     0,     0,     0,     0,     0,   725,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   730,     0,   963,     0,
     730,   730,     0,     0,   730,   730,   730,   730,   730,   730,
     730,   730,   730,   730,   730,   730,   730,   730,   730,   730,
     730,   730,   730,   730,   730,   730,   730,   730,   730,   730,
     730,     0,     0,   664,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    36,
    1000,   207,     0,     0,   730,     0,     0,     0,     0,     0,
       0,     0,     0,  1008,     0,     0,     0,     0,  1011,     0,
    1012,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   725,     0,     0,     0,     0,     0,
       0,   684,  1027,     0,     0,   626,     0,     0,     0,     0,
       0,     0,  1035,     0,     0,  1036,     0,  1037,     0,     0,
       0,   586,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   586,     0,     0,     0,     0,    83,    84,     0,    85,
     179,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1069,     0,     0,     0,
       0,     0,     0,   730,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,     0,   730,     0,
     730,     0,     0,   627,     0,   115,     0,     0,     0,     0,
       0,     0,     0,   725,     0,     0,     0,   725,   725,     0,
     730,   725,   725,   725,   725,   725,   725,   725,   725,   725,
     725,   725,   725,   725,   725,   725,   725,   725,   725,   725,
     725,   725,   725,   725,   725,   725,   725,   725,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1146,
    1147,  1148,     0,     0,     0,   738,  1150,     0,   731,     0,
       0,   725,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   730,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1165,   943,   944,   945,   946,   947,   948,
     949,   950,   951,   952,   953,   954,   955,   956,   957,   958,
     959,   960,   961,   763,     0,     0,  1187,     0,     0,     0,
       0,  1191,     0,     0,     0,     0,   962,     0,     0,     0,
       0,     0,     0,     0,   586,     0,     0,     0,     0,     0,
       0,     0,     0,   586,     0,  1165,   730,     0,     0,     0,
     730,     0,   730,     0,     0,   730,     0,   730,     0,     0,
     730,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     725,     0,     0,     0,   237,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1235,   725,     0,   725,     0,   937,
     938,   939,   730,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   725,   940,     0,
     941,   942,   943,   944,   945,   946,   947,   948,   949,   950,
     951,   952,   953,   954,   955,   956,   957,   958,   959,   960,
     961,     0,   730,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   962,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   863,   864,     0,
       0,     0,     0,     0,     0,     0,     0,  1287,     0,     0,
     725,  1288,     0,  1289,  1290,     0,     0,   865,     0,   730,
       0,     0,     0,     0,   586,   866,   867,   868,    36,     0,
       0,     0,  1304,   586,     0,     0,   869,     0,     0,   900,
       0,     0,   586,     0,     0,     0,     0,     0,     0,     0,
     730,   730,   730,   921,   922,   923,   924,   730,     0,     0,
       0,  1559,     0,     0,     0,   934,     0,     0,     0,     0,
       0,     0,     0,   725,     0,     0,     0,   725,     0,   725,
       0,   870,   725,     0,   725,     0,     0,   725,     0,     0,
       0,     0,     0,     0,   871,     0,     0,     0,  1342,     0,
       0,     0,   249,     0,     0,    83,    84,     0,    85,   179,
      87,     0,     0,     0,     0,     0,     0,     0,  1113,   725,
       0,     0,     0,   872,     0,     0,     0,     0,   250,     0,
       0,     0,   586,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,     0,     0,     0,
      36,     0,     0,     0,   347,   348,   349,     0,     0,   725,
       0,     0,     0,     0,     0,   730,     0,     0,     0,     0,
       0,  1028,     0,   350,     0,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,     0,   372,     0,   730,
       0,     0,     0,   251,   252,     0,   725,     0,     0,   373,
     730,     0,     0,     0,     0,   730,     0,     0,   730,   586,
       0,   178,     0,     0,    81,   253,     0,    83,    84,     0,
      85,   179,    87,     0,   916,     0,     0,   725,   725,   725,
       0,  1636,     0,     0,   725,   254,     0,     0,     0,     0,
    1551,     0,     0,     0,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
       0,   255,     0,     0,  1109,  1109,   730,     0,  1116,  1119,
    1120,  1121,  1123,  1124,  1125,  1126,  1127,  1128,  1129,  1130,
    1131,  1132,  1133,  1134,  1135,  1136,  1137,  1138,  1139,  1140,
    1141,  1142,  1143,  1144,  1145,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1156,   636,
      12,   730,     0,     0,     0,     0,     0,   637,   971,   730,
       0,     0,     0,   730,     0,     0,   730,     0,    14,    15,
       0,     0,   725,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,   725,    40,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   725,     0,     0,
       0,     0,   725,     0,     0,   725,    51,     0,     0,     0,
       0,     0,     0,     0,    58,    59,    60,   174,   175,   176,
       0,     0,    65,    66,     0,     0,     0,  1231,     0,     0,
       0,   177,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,  1245,     0,  1246,    77,     0,     0,     0,     0,
     178,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     179,    87,     0,   725,  1261,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,     0,     0,     0,
       0,    96,    97,   261,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
     112,     0,     0,     0,     0,   115,   116,     0,   117,   118,
       0,     5,     6,     7,     8,     9,     0,     0,   725,     0,
       0,    10,     0,     0,     0,     0,   725,  1295,     0,     0,
     725,     0,     0,   725,     0,     0,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
    1335,     0,     0,    40,  1337,     0,  1338,     0,     0,  1339,
       0,  1340,     0,     0,  1341,     0,     0,     0,     0,     0,
       0,     0,    51,     0,     0,     0,     0,     0,     0,     0,
      58,    59,    60,   174,   175,   176,     0,     0,    65,    66,
       0,     0,     0,     0,     0,     0,  1383,   177,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   178,    79,    80,    81,
      82,     0,    83,    84,     0,    85,   179,    87,     0,     0,
       0,    89,     0,     0,    90,     0,  1411,     0,     0,     0,
      91,    92,    93,     0,     0,     0,     0,    96,    97,   261,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,     0,   112,     0,   262,     0,
       0,   115,   116,     0,   117,   118,     0,     0,     0,     0,
       0,     0,     0,  1528,  -870,  -870,  -870,  -870,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
       0,   372,     0,   347,   348,   349,     0,     0,     0,     0,
       0,     0,     0,   373,  1548,  1549,  1550,     0,     0,     0,
       0,  1555,   350,     0,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,     0,   372,     0,   347,   348,
     349,     0,     0,     0,     0,     0,     0,     0,   373,     0,
       0,     0,     0,     0,     0,     0,     0,   350,     0,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
       0,   372,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   350,   373,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,     0,   372,     0,     0,  1581,
       0,     0,     0,     0,     0,     0,     0,     0,   373,     0,
       0,     0,     0,     0,     0,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,  1605,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,  1615,     0,     0,     0,     0,  1619,
       0,     0,  1621,   967,   968,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,  1415,     0,     0,    40,    41,    42,
      43,     0,    44,     0,    45,     0,    46,     0,     0,    47,
    1652,     0,     0,    48,    49,    50,    51,    52,    53,    54,
       0,    55,    56,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,    70,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
      78,    79,    80,    81,    82,  1702,    83,    84,     0,    85,
      86,    87,    88,  1707,     0,    89,     0,  1709,    90,     0,
    1710,     0,     0,     0,    91,    92,    93,    94,     0,    95,
       0,    96,    97,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
     112,     0,   113,   114,  1001,   115,   116,     0,   117,   118,
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
      92,    93,    94,     0,    95,     0,    96,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,     0,   113,   114,  1167,
     115,   116,     0,   117,   118,     5,     6,     7,     8,     9,
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
       0,     0,     0,     0,    91,    92,    93,    94,     0,    95,
       0,    96,    97,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
     112,     0,   113,   114,     0,   115,   116,     0,   117,   118,
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
      77,     0,     0,     0,     0,   178,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   179,    87,    88,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,     0,     0,     0,     0,    96,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,     0,   113,   114,   567,
     115,   116,     0,   117,   118,     5,     6,     7,     8,     9,
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
     178,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     179,    87,    88,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,     0,     0,     0,
       0,    96,    97,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
     112,     0,   113,   114,   974,   115,   116,     0,   117,   118,
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
      77,     0,     0,     0,     0,   178,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   179,    87,    88,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,     0,     0,     0,     0,    96,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,     0,   113,   114,  1076,
     115,   116,     0,   117,   118,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,     0,     0,     0,    40,    41,    42,
      43,  1078,    44,     0,    45,     0,    46,     0,     0,    47,
       0,     0,     0,    48,    49,    50,    51,     0,    53,    54,
       0,    55,     0,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,     0,    68,    69,     0,     0,     0,
       0,    70,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     178,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     179,    87,    88,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,     0,     0,     0,
       0,    96,    97,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
     112,     0,   113,   114,     0,   115,   116,     0,   117,   118,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,     0,
       0,     0,    40,    41,    42,    43,     0,    44,     0,    45,
       0,    46,  1232,     0,    47,     0,     0,     0,    48,    49,
      50,    51,     0,    53,    54,     0,    55,     0,    57,    58,
      59,    60,    61,    62,    63,     0,    64,    65,    66,     0,
      68,    69,     0,     0,     0,     0,    70,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   178,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   179,    87,    88,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,     0,     0,     0,     0,    96,    97,     0,    98,
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
      37,    38,     0,    39,     0,     0,     0,    40,    41,    42,
      43,     0,    44,     0,    45,     0,    46,     0,     0,    47,
       0,     0,     0,    48,    49,    50,    51,     0,    53,    54,
       0,    55,     0,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,     0,    68,    69,     0,     0,     0,
       0,    70,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     178,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     179,    87,    88,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,     0,     0,     0,
       0,    96,    97,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
     112,     0,   113,   114,  1344,   115,   116,     0,   117,   118,
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
      77,     0,     0,     0,     0,   178,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   179,    87,    88,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,     0,     0,     0,     0,    96,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,     0,   113,   114,  1552,
     115,   116,     0,   117,   118,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,     0,     0,     0,    40,    41,    42,
      43,     0,    44,     0,    45,  1594,    46,     0,     0,    47,
       0,     0,     0,    48,    49,    50,    51,     0,    53,    54,
       0,    55,     0,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,     0,    68,    69,     0,     0,     0,
       0,    70,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     178,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     179,    87,    88,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,     0,     0,     0,
       0,    96,    97,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
     112,     0,   113,   114,     0,   115,   116,     0,   117,   118,
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
      77,     0,     0,     0,     0,   178,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   179,    87,    88,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,     0,     0,     0,     0,    96,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,     0,   113,   114,  1625,
     115,   116,     0,   117,   118,     5,     6,     7,     8,     9,
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
     178,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     179,    87,    88,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,     0,     0,     0,
       0,    96,    97,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
     112,     0,   113,   114,  1626,   115,   116,     0,   117,   118,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,     0,
       0,     0,    40,    41,    42,    43,     0,    44,  1629,    45,
       0,    46,     0,     0,    47,     0,     0,     0,    48,    49,
      50,    51,     0,    53,    54,     0,    55,     0,    57,    58,
      59,    60,    61,    62,    63,     0,    64,    65,    66,     0,
      68,    69,     0,     0,     0,     0,    70,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   178,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   179,    87,    88,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,     0,     0,     0,     0,    96,    97,     0,    98,
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
      37,    38,     0,    39,     0,     0,     0,    40,    41,    42,
      43,     0,    44,     0,    45,     0,    46,     0,     0,    47,
       0,     0,     0,    48,    49,    50,    51,     0,    53,    54,
       0,    55,     0,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,     0,    68,    69,     0,     0,     0,
       0,    70,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     178,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     179,    87,    88,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,     0,     0,     0,
       0,    96,    97,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
     112,     0,   113,   114,  1644,   115,   116,     0,   117,   118,
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
      77,     0,     0,     0,     0,   178,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   179,    87,    88,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,     0,     0,     0,     0,    96,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,     0,   113,   114,  1697,
     115,   116,     0,   117,   118,     5,     6,     7,     8,     9,
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
     178,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     179,    87,    88,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,     0,     0,     0,
       0,    96,    97,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
     112,     0,   113,   114,  1703,   115,   116,     0,   117,   118,
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
      77,     0,     0,     0,     0,   178,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   179,    87,    88,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,     0,     0,     0,     0,    96,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,     0,   113,   114,     0,
     115,   116,     0,   117,   118,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,   443,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,     0,     0,     0,    40,    41,    42,
      43,     0,    44,     0,    45,     0,    46,     0,     0,    47,
       0,     0,     0,    48,    49,    50,    51,     0,    53,    54,
       0,    55,     0,    57,    58,    59,    60,   174,   175,    63,
       0,    64,    65,    66,     0,     0,     0,     0,     0,     0,
       0,    70,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     178,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     179,    87,     0,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,     0,     0,     0,
       0,    96,    97,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
     112,     0,   113,   114,     0,   115,   116,     0,   117,   118,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,   691,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,     0,
       0,     0,    40,    41,    42,    43,     0,    44,     0,    45,
       0,    46,     0,     0,    47,     0,     0,     0,    48,    49,
      50,    51,     0,    53,    54,     0,    55,     0,    57,    58,
      59,    60,   174,   175,    63,     0,    64,    65,    66,     0,
       0,     0,     0,     0,     0,     0,    70,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   178,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   179,    87,     0,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,     0,     0,     0,     0,    96,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,     0,   113,   114,     0,
     115,   116,     0,   117,   118,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,   902,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,     0,     0,     0,    40,    41,    42,
      43,     0,    44,     0,    45,     0,    46,     0,     0,    47,
       0,     0,     0,    48,    49,    50,    51,     0,    53,    54,
       0,    55,     0,    57,    58,    59,    60,   174,   175,    63,
       0,    64,    65,    66,     0,     0,     0,     0,     0,     0,
       0,    70,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     178,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     179,    87,     0,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,     0,     0,     0,
       0,    96,    97,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
     112,     0,   113,   114,     0,   115,   116,     0,   117,   118,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,  1406,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,     0,
       0,     0,    40,    41,    42,    43,     0,    44,     0,    45,
       0,    46,     0,     0,    47,     0,     0,     0,    48,    49,
      50,    51,     0,    53,    54,     0,    55,     0,    57,    58,
      59,    60,   174,   175,    63,     0,    64,    65,    66,     0,
       0,     0,     0,     0,     0,     0,    70,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   178,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   179,    87,     0,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,     0,     0,     0,     0,    96,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,     0,   113,   114,     0,
     115,   116,     0,   117,   118,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,  1543,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,     0,     0,     0,    40,    41,    42,
      43,     0,    44,     0,    45,     0,    46,     0,     0,    47,
       0,     0,     0,    48,    49,    50,    51,     0,    53,    54,
       0,    55,     0,    57,    58,    59,    60,   174,   175,    63,
       0,    64,    65,    66,     0,     0,     0,     0,     0,     0,
       0,    70,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     178,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     179,    87,     0,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,     0,     0,     0,
       0,    96,    97,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
     112,     0,   113,   114,     0,   115,   116,     0,   117,   118,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,     0,
       0,     0,    40,    41,    42,    43,     0,    44,     0,    45,
       0,    46,     0,     0,    47,     0,     0,     0,    48,    49,
      50,    51,     0,    53,    54,     0,    55,     0,    57,    58,
      59,    60,   174,   175,    63,     0,    64,    65,    66,     0,
       0,     0,     0,     0,     0,     0,    70,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   178,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   179,    87,     0,     0,     0,
      89,     0,     0,    90,     5,     6,     7,     8,     9,    91,
      92,    93,     0,     0,    10,     0,    96,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,   582,   113,   114,     0,
     115,   116,     0,   117,   118,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,    40,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    51,     0,     0,     0,     0,
       0,     0,     0,    58,    59,    60,   174,   175,   176,     0,
       0,    65,    66,     0,     0,     0,     0,     0,     0,     0,
     177,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   178,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   179,
      87,     0,   583,     0,    89,     0,     0,    90,     5,     6,
       7,     8,     9,    91,    92,    93,     0,     0,    10,     0,
      96,    97,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,   112,
    1025,     0,     0,     0,   115,   116,     0,   117,   118,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
      40,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    51,
       0,     0,     0,     0,     0,     0,     0,    58,    59,    60,
     174,   175,   176,     0,     0,    65,    66,     0,     0,     0,
       0,     0,     0,     0,   177,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   178,    79,    80,    81,    82,     0,    83,
      84,     0,    85,   179,    87,     0,  1026,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
       0,     0,     0,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,     0,     0,     0,   115,   116,
       0,   117,   118,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   636,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,    40,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    51,     0,     0,     0,     0,     0,
       0,     0,    58,    59,    60,   174,   175,   176,     0,     0,
      65,    66,     0,     0,     0,     0,     0,     0,     0,   177,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   178,    79,
      80,    81,    82,     0,    83,    84,     0,    85,   179,    87,
       0,     0,     0,    89,     0,     0,    90,     5,     6,     7,
       8,     9,    91,    92,    93,     0,     0,    10,     0,    96,
      97,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,   112,     0,
       0,     0,     0,   115,   116,     0,   117,   118,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,    40,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   190,     0,     0,    51,     0,
       0,     0,     0,     0,     0,     0,    58,    59,    60,   174,
     175,   176,     0,     0,    65,    66,     0,     0,     0,     0,
       0,     0,     0,   177,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   178,    79,    80,    81,    82,     0,    83,    84,
       0,    85,   179,    87,     0,     0,     0,    89,     0,     0,
      90,     5,     6,     7,     8,     9,    91,    92,    93,     0,
       0,    10,     0,    96,    97,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,     0,   112,   215,     0,     0,     0,   115,   116,     0,
     117,   118,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,    40,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    51,     0,     0,     0,     0,     0,     0,     0,
      58,    59,    60,   174,   175,   176,     0,     0,    65,    66,
       0,     0,     0,     0,     0,     0,     0,   177,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   178,    79,    80,    81,
      82,     0,    83,    84,     0,    85,   179,    87,     0,     0,
       0,    89,     0,     0,    90,     5,     6,     7,     8,     9,
      91,    92,    93,     0,     0,    10,     0,    96,    97,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,     0,   112,     0,     0,     0,
       0,   115,   116,     0,   117,   118,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,    40,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    51,     0,     0,     0,
       0,     0,     0,     0,    58,    59,    60,   174,   175,   176,
       0,     0,    65,    66,     0,     0,     0,     0,     0,     0,
       0,   177,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     178,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     179,    87,     0,     0,     0,    89,     0,     0,    90,     5,
       6,     7,     8,     9,    91,    92,    93,     0,     0,    10,
       0,    96,    97,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
     112,     0,   244,     0,     0,   115,   116,     0,   117,   118,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,    40,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      51,     0,     0,     0,     0,     0,     0,     0,    58,    59,
      60,   174,   175,   176,     0,     0,    65,    66,     0,     0,
       0,     0,     0,     0,     0,   177,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   178,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   179,    87,     0,     0,     0,    89,
       0,     0,    90,     5,     6,     7,     8,     9,    91,    92,
      93,     0,     0,    10,     0,    96,    97,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   112,     0,   247,     0,     0,   115,
     116,     0,   117,   118,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,    40,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    51,     0,     0,     0,     0,     0,
       0,     0,    58,    59,    60,   174,   175,   176,     0,     0,
      65,    66,     0,     0,     0,     0,     0,     0,     0,   177,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   178,    79,
      80,    81,    82,     0,    83,    84,     0,    85,   179,    87,
       0,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,     0,     0,     0,     0,    96,
      97,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,   112,   441,
       0,     0,     0,   115,   116,     0,   117,   118,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,  -870,
    -870,  -870,  -870,   950,   951,   952,   953,   954,   955,   956,
     957,   958,   959,   960,   961,   595,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   962,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
      40,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    51,
       0,     0,     0,     0,     0,     0,     0,    58,    59,    60,
     174,   175,   176,     0,     0,    65,    66,     0,     0,     0,
       0,     0,     0,     0,   177,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   178,    79,    80,    81,    82,     0,    83,
      84,     0,    85,   179,    87,     0,     0,     0,    89,     0,
       0,    90,     5,     6,     7,     8,     9,    91,    92,    93,
       0,     0,    10,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,   637,     0,     0,     0,   115,   116,
       0,   117,   118,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,    40,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    51,     0,     0,     0,     0,     0,     0,
       0,    58,    59,    60,   174,   175,   176,     0,     0,    65,
      66,     0,     0,     0,     0,     0,     0,     0,   177,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,   178,    79,    80,
      81,    82,     0,    83,    84,     0,    85,   179,    87,     0,
       0,     0,    89,     0,     0,    90,     5,     6,     7,     8,
       9,    91,    92,    93,     0,     0,    10,     0,    96,    97,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,   112,   673,     0,
       0,     0,   115,   116,     0,   117,   118,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,    40,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    51,     0,     0,
       0,     0,     0,     0,     0,    58,    59,    60,   174,   175,
     176,     0,     0,    65,    66,     0,     0,     0,     0,     0,
       0,     0,   177,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   178,    79,    80,    81,    82,     0,    83,    84,     0,
      85,   179,    87,     0,     0,     0,    89,     0,     0,    90,
       5,     6,     7,     8,     9,    91,    92,    93,     0,     0,
      10,     0,    96,    97,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,   112,   675,     0,     0,     0,   115,   116,     0,   117,
     118,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,    40,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    51,     0,     0,     0,     0,     0,     0,     0,    58,
      59,    60,   174,   175,   176,     0,     0,    65,    66,     0,
       0,     0,     0,     0,     0,     0,   177,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   178,    79,    80,    81,    82,
       0,    83,    84,     0,    85,   179,    87,     0,     0,     0,
      89,     0,     0,    90,     5,     6,     7,     8,     9,    91,
      92,    93,     0,     0,    10,     0,    96,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,     0,     0,     0,     0,
     115,   116,     0,   117,   118,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,    40,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    51,     0,     0,     0,     0,
       0,     0,     0,    58,    59,    60,   174,   175,   176,     0,
       0,    65,    66,     0,     0,     0,     0,     0,     0,     0,
     177,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   178,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   179,
      87,     0,     0,     0,    89,     0,     0,    90,     5,     6,
       7,     8,     9,    91,    92,    93,     0,     0,    10,     0,
      96,    97,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,   112,
    1068,     0,   687,     0,   115,   116,     0,   117,   118,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
      40,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    51,
       0,     0,     0,     0,     0,     0,     0,    58,    59,    60,
     174,   175,   176,     0,     0,    65,    66,     0,     0,     0,
       0,     0,     0,     0,   177,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   178,    79,    80,    81,    82,     0,    83,
      84,     0,    85,   179,    87,     0,     0,     0,    89,     0,
       0,    90,     5,     6,     7,     8,     9,    91,    92,    93,
       0,     0,    10,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,     0,     0,     0,   115,   116,
       0,   117,   118,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,    40,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    51,     0,     0,     0,     0,     0,     0,
       0,    58,    59,    60,   174,   175,   176,     0,     0,    65,
      66,     0,     0,     0,     0,     0,     0,     0,   177,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,   178,    79,    80,
      81,    82,     0,    83,    84,     0,    85,   179,    87,     0,
       0,     0,    89,     0,     0,    90,     5,     6,     7,     8,
       9,    91,    92,    93,     0,     0,    10,     0,    96,    97,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,   112,     0,     0,
       0,     0,   115,   116,     0,   117,   118,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,   531,    38,     0,     0,     0,     0,     0,    40,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    51,     0,     0,
       0,     0,     0,     0,     0,    58,    59,    60,   174,   175,
     176,     0,     0,    65,    66,     0,     0,     0,     0,     0,
       0,     0,   177,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   178,    79,    80,    81,    82,     0,    83,    84,     0,
      85,   179,    87,     0,     0,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,    92,    93,     0,     0,
       0,     0,    96,    97,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,   112,     0,     0,     0,     0,   115,   116,     0,   117,
     118,  1426,  1427,  1428,  1429,  1430,     0,     0,  1431,  1432,
    1433,  1434,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1435,  1436,     0,   940,     0,
     941,   942,   943,   944,   945,   946,   947,   948,   949,   950,
     951,   952,   953,   954,   955,   956,   957,   958,   959,   960,
     961,  1437,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   962,  1438,  1439,  1440,  1441,  1442,
    1443,  1444,     0,     0,     0,    36,     0,     0,     0,     0,
       0,     0,     0,     0,  1445,  1446,  1447,  1448,  1449,  1450,
    1451,  1452,  1453,  1454,  1455,  1456,  1457,  1458,  1459,  1460,
    1461,  1462,  1463,  1464,  1465,  1466,  1467,  1468,  1469,  1470,
    1471,  1472,  1473,  1474,  1475,  1476,  1477,  1478,  1479,  1480,
    1481,  1482,  1483,  1484,  1485,     0,     0,  1486,  1487,     0,
    1488,  1489,  1490,  1491,  1492,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1493,  1494,  1495,     0,
     249,     0,    83,    84,     0,    85,   179,    87,  1496,     0,
    1497,  1498,     0,  1499,     0,     0,     0,     0,     0,     0,
    1500,     0,     0,  1501,     0,  1502,   250,  1503,  1504,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   347,   348,   349,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   350,     0,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   249,   372,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   373,     0,
       0,   251,   252,     0,     0,     0,     0,     0,     0,     0,
       0,   250,     0,     0,     0,     0,     0,     0,     0,   178,
       0,     0,    81,   253,     0,    83,    84,     0,    85,   179,
      87,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   254,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,     0,     0,   255,
       0,     0,     0,  1523,     0,     0,   249,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   251,   252,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   250,     0,   178,     0,     0,    81,   253,     0,
      83,    84,     0,    85,   179,    87,     0,     0,  1263,     0,
       0,     0,     0,     0,    36,     0,     0,     0,   254,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,  -319,     0,     0,   255,   249,     0,     0,  1590,    58,
      59,    60,   174,   175,   338,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   251,   252,     0,
       0,   250,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   178,     0,     0,    81,   253,
       0,    83,    84,    36,    85,   179,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   254,
     249,     0,     0,     0,     0,     0,     0,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,     0,   255,   250,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   251,   252,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,   178,     0,     0,    81,   253,     0,
      83,    84,     0,    85,   179,    87,     0,  1242,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   254,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   251,   252,     0,   255,     0,     0,    36,     0,   207,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   178,
       0,     0,    81,   253,     0,    83,    84,     0,    85,   179,
      87,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   254,     0,     0,     0,   208,     0,     0,
    1122,     0,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   711,   712,     0,   255,
       0,     0,   713,     0,   714,     0,     0,     0,   178,     0,
       0,    81,    82,     0,    83,    84,   715,    85,   179,    87,
       0,     0,     0,     0,    33,    34,    35,    36,     0,     0,
       0,     0,     0,   896,     0,   716,     0,     0,     0,     0,
       0,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,     0,   209,     0,
       0,   507,     0,   115,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    36,     0,   207,     0,     0,
     717,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,   718,     0,     0,     0,     0,   178,    79,
      80,    81,   719,     0,    83,    84,     0,    85,   179,    87,
       0,     0,     0,    89,     0,   208,     0,     0,     0,     0,
       0,     0,   720,   721,   722,     0,     0,   897,     0,    96,
      36,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,   178,     0,   723,    81,
      82,     0,    83,    84,     0,    85,   179,    87,     0,     0,
     942,   943,   944,   945,   946,   947,   948,   949,   950,   951,
     952,   953,   954,   955,   956,   957,   958,   959,   960,   961,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   962,   711,   712,   209,     0,     0,     0,
     713,   115,   714,     0,   300,     0,     0,    83,    84,     0,
      85,   179,    87,     0,   715,     0,     0,     0,     0,     0,
       0,     0,    33,    34,    35,    36,     0,     0,     0,     0,
       0,     0,     0,   716,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
       0,     0,     0,     0,   301,     0,     0,     0,     0,     0,
       0,     0,     0,    29,    30,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,   207,     0,     0,   717,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,   718,     0,     0,     0,     0,   178,    79,    80,    81,
     719,     0,    83,    84,     0,    85,   179,    87,     0,     0,
       0,    89,     0,   208,     0,     0,     0,   852,     0,     0,
     720,   721,   722,     0,     0,     0,    36,    96,   207,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,   178,     0,   723,    81,    82,     0,
      83,    84,     0,    85,   179,    87,     0,     0,     0,     0,
       0,     0,    90,     0,     0,     0,   208,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,    36,     0,     0,   422,     0,     0,   178,     0,   115,
      81,    82,     0,    83,    84,     0,    85,   179,    87,    36,
       0,   207,     0,     0,   945,   946,   947,   948,   949,   950,
     951,   952,   953,   954,   955,   956,   957,   958,   959,   960,
     961,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   962,     0,     0,   209,     0,   208,
       0,     0,   115,     0,     0,     0,     0,     0,     0,     0,
       0,   523,     0,     0,     0,     0,  1510,     0,    83,    84,
    1511,    85,   179,    87,     0,     0,     0,     0,     0,     0,
     178,     0,     0,    81,    82,     0,    83,    84,     0,    85,
     179,    87,    36,     0,   207,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,     0,  1364,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,     0,     0,     0,
     209,     0,   208,     0,     0,   115,     0,     0,     0,     0,
       0,     0,     0,     0,   996,    36,     0,   207,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   178,     0,     0,    81,    82,     0,    83,
      84,     0,    85,   179,    87,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   208,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,     0,   209,     0,     0,   178,     0,   115,    81,
      82,     0,    83,    84,     0,    85,   179,    87,    36,     0,
     207,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,     0,   209,     0,   220,     0,
      36,   115,   207,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,   207,     0,     0,     0,     0,   178,
       0,     0,    81,    82,     0,    83,    84,     0,    85,   179,
      87,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,    83,    84,   221,
      85,   179,    87,    36,   115,   207,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    83,
      84,     0,    85,   179,    87,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
       0,     0,     0,     0,   654,     0,   115,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
      36,     0,   207,     0,     0,     0,   627,     0,   115,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      83,    84,    36,    85,   179,    87,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,     0,     0,     0,     0,     0,   681,     0,   115,
       0,     0,     0,     0,     0,     0,     0,    83,    84,     0,
      85,   179,    87,  1051,  1052,  1053,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    83,
      84,     0,    85,   179,    87,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,    83,    84,
       0,    85,   179,    87,  1021,     0,   115,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,     0,   588,     0,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,     0,   834,    83,    84,     0,    85,   179,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   347,   348,   349,     0,     0,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   350,     0,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,     0,   372,   347,
     348,   349,     0,     0,     0,     0,     0,     0,     0,     0,
     373,     0,     0,     0,     0,     0,     0,     0,   350,     0,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,     0,   372,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   373,   347,   348,   349,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   418,   350,     0,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,     0,   372,   347,
     348,   349,     0,     0,     0,     0,     0,     0,     0,     0,
     373,     0,     0,     0,     0,     0,     0,   427,   350,     0,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,     0,   372,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   373,   347,   348,   349,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   803,   350,     0,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,     0,   372,   347,
     348,   349,     0,     0,     0,     0,     0,     0,     0,     0,
     373,     0,     0,     0,     0,     0,     0,   840,   350,     0,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,     0,   372,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   373,   347,   348,   349,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   881,   350,     0,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,     0,   372,   937,
     938,   939,     0,     0,     0,     0,     0,     0,     0,     0,
     373,     0,     0,     0,     0,     0,     0,  1179,   940,     0,
     941,   942,   943,   944,   945,   946,   947,   948,   949,   950,
     951,   952,   953,   954,   955,   956,   957,   958,   959,   960,
     961,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   962,   937,   938,   939,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1197,   940,     0,   941,   942,   943,   944,
     945,   946,   947,   948,   949,   950,   951,   952,   953,   954,
     955,   956,   957,   958,   959,   960,   961,   937,   938,   939,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     962,     0,     0,     0,     0,     0,   940,  1100,   941,   942,
     943,   944,   945,   946,   947,   948,   949,   950,   951,   952,
     953,   954,   955,   956,   957,   958,   959,   960,   961,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   962,   937,   938,   939,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   940,  1252,   941,   942,   943,   944,   945,   946,
     947,   948,   949,   950,   951,   952,   953,   954,   955,   956,
     957,   958,   959,   960,   961,   937,   938,   939,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   962,     0,
       0,     0,     0,     0,   940,  1257,   941,   942,   943,   944,
     945,   946,   947,   948,   949,   950,   951,   952,   953,   954,
     955,   956,   957,   958,   959,   960,   961,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    36,     0,     0,
     962,   937,   938,   939,     0,   704,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,     0,     0,
     940,  1334,   941,   942,   943,   944,   945,   946,   947,   948,
     949,   950,   951,   952,   953,   954,   955,   956,   957,   958,
     959,   960,   961,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   962,  1351,     0,     0,
       0,     0,     0,  1413,    36,     0,     0,     0,   178,  1352,
    1353,    81,     0,     0,    83,    84,     0,    85,   179,    87,
      36,     0,   780,   781,     0,     0,     0,   178,     0,     0,
      81,  1354,     0,    83,    84,     0,    85,  1355,    87,     0,
       0,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,     0,     0,  1414,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,    36,   178,     0,     0,    81,    82,
       0,    83,    84,     0,    85,   179,    87,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    83,    84,     0,
      85,   179,    87,     0,     0,     0,     0,     0,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   335,
       0,    83,    84,    36,    85,   179,    87,     0,     0,     0,
       0,   495,     0,     0,    83,    84,     0,    85,   179,    87,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,     0,     0,     0,
      36,     0,   499,     0,     0,    83,    84,     0,    85,   179,
      87,     0,     0,     0,     0,    36,     0,   272,     0,     0,
      83,    84,     0,    85,   179,    87,     0,     0,     0,     0,
       0,     0,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   626,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,  1114,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    83,    84,     0,
      85,   179,    87,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    83,    84,     0,    85,   179,    87,     0,     0,
       0,     0,     0,     0,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,     0,     0,     0,     0,    83,
      84,     0,    85,   179,    87,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   347,   348,   349,     0,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     695,   350,     0,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,     0,   372,   347,   348,   349,     0,
       0,     0,     0,     0,     0,     0,     0,   373,     0,     0,
       0,     0,     0,     0,     0,   350,   837,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   696,   372,
     347,   348,   349,     0,     0,     0,     0,     0,     0,     0,
       0,   373,     0,     0,     0,     0,     0,     0,     0,   350,
       0,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,     0,   372,   937,   938,   939,     0,     0,     0,
       0,     0,     0,     0,     0,   373,     0,     0,     0,     0,
       0,     0,     0,   940,  1262,   941,   942,   943,   944,   945,
     946,   947,   948,   949,   950,   951,   952,   953,   954,   955,
     956,   957,   958,   959,   960,   961,   937,   938,   939,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   962,
       0,     0,     0,     0,     0,   940,     0,   941,   942,   943,
     944,   945,   946,   947,   948,   949,   950,   951,   952,   953,
     954,   955,   956,   957,   958,   959,   960,   961,   348,   349,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   962,     0,     0,     0,     0,   350,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,     0,
     372,   938,   939,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   373,     0,     0,     0,     0,     0,     0,   940,
       0,   941,   942,   943,   944,   945,   946,   947,   948,   949,
     950,   951,   952,   953,   954,   955,   956,   957,   958,   959,
     960,   961,   349,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   962,     0,     0,     0,   350,
       0,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   939,   372,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   373,     0,     0,     0,   940,
       0,   941,   942,   943,   944,   945,   946,   947,   948,   949,
     950,   951,   952,   953,   954,   955,   956,   957,   958,   959,
     960,   961,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   962,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,     0,   372,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     373,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,     0,   372,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   373,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,     0,   372,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   373,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,     0,   372,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   373
};

static const yytype_int16 yycheck[] =
{
       4,   134,     4,     4,   160,    52,   585,   218,    88,   181,
     249,   250,     4,     4,    94,    95,   255,     4,    32,   812,
       4,     4,   170,    53,   308,  1017,   407,   386,   372,    43,
     412,   163,   799,    47,   666,   160,   222,   694,   228,   112,
     538,  1192,     9,     9,   833,    49,   228,   112,    52,   182,
    1005,  1003,   895,   133,   435,     9,   701,    30,   830,     9,
     849,     9,    30,    45,     9,    69,     9,    33,     9,    30,
       4,    45,     9,    45,     9,    45,     9,     9,     9,     9,
       9,     9,    86,     9,    88,     9,    45,     9,   127,   328,
      94,    95,    26,    27,    56,     9,    35,   229,     9,    84,
       9,    66,     9,     9,   127,    84,     9,   127,    66,    66,
      50,   108,    35,    99,   100,    79,    78,    35,     0,    81,
     198,   147,    14,    99,   100,   125,   126,   198,   168,   133,
    1537,   200,   201,   134,   200,   201,   209,   198,    30,   198,
      79,    96,   168,   443,    96,    66,     8,    66,   221,  1300,
      96,  1302,    79,    66,    96,    47,    79,    35,   198,   144,
     308,    79,    66,   202,   931,   144,    66,   339,   165,   127,
     127,    66,    66,   160,    79,   201,   199,    66,   112,   199,
      66,   182,   147,   147,  1591,    66,    66,    66,    53,    79,
     194,   491,    79,    66,    66,   198,   151,    66,    66,   151,
      65,    79,   198,    26,    27,   151,   127,    30,   201,   151,
     421,   123,   152,   286,   196,   123,   202,   199,   145,   146,
     198,   286,   161,   462,   169,   199,   202,   199,   201,   199,
     234,   379,   200,   199,   238,   202,   201,   196,   242,  1194,
    1083,  1392,   414,   234,   201,   199,  1201,   238,  1203,   199,
    1202,   199,   385,   401,   334,  1044,   260,   200,  1030,   200,
     905,  1033,   907,   200,   201,   200,   169,   200,   200,   200,
     200,   200,   200,   198,   200,   209,   200,   425,   200,   198,
     201,   215,   201,   161,   331,   199,   434,   221,   199,   437,
     199,   789,   199,   199,   198,   152,  1063,   201,   203,    50,
     234,   201,   382,   383,   238,   198,   201,   201,   270,   271,
     272,   313,   201,   203,   318,   201,   203,   318,   557,   558,
     201,   201,   201,   327,   563,    35,    35,   331,   201,   201,
     334,   318,   201,   201,   198,    96,    79,   548,   300,   198,
      79,    84,    79,   277,   555,    84,   151,    84,   198,   422,
     284,   285,   286,    79,   198,  1310,   386,   291,    84,   147,
      79,   198,    79,   297,   198,    84,   201,    84,    96,    79,
      79,    79,    71,    72,   564,   379,   380,   381,   382,   383,
     168,    30,   564,   384,   318,   151,   209,   687,   198,  1540,
     151,   691,   215,   153,   776,   200,   201,   401,   221,   153,
      35,   152,   145,   146,    71,    72,   145,   146,   145,   146,
     198,   372,   166,   201,   625,   200,   167,   573,   379,   145,
     146,   425,  1079,   151,  1196,   144,   145,   146,   145,   146,
    1566,  1567,  1064,   437,    79,   200,   144,   145,   146,    84,
     401,   201,   628,    98,    99,   100,   450,   201,   573,  1562,
    1563,   161,   161,   200,   277,   200,     4,   709,   710,   450,
      29,   284,   285,   635,   425,   846,   376,   206,   291,   655,
     709,   710,    66,   434,   297,   856,   437,    46,   412,   200,
      49,   671,  1249,    66,     4,   308,   672,   677,   422,   671,
     207,    66,   198,    29,   404,   677,   707,    45,   408,   144,
     145,   146,   506,    49,    50,    51,   200,    53,   852,   200,
      46,   591,  1663,    49,   201,   799,   450,   521,   147,    65,
      49,    50,    51,   527,   528,    45,   198,  1678,    66,   147,
     294,   151,   198,   495,   298,   200,    65,   499,   724,  1311,
      44,    65,   504,   168,   147,   677,    98,    99,   100,   372,
     127,   790,  1319,   112,   113,   114,   320,   105,   322,   323,
     324,   325,   110,   205,   112,   113,   114,   115,   116,   117,
     118,   810,   112,   113,   114,   115,   116,   117,  1210,     9,
    1212,   147,   821,   147,   198,   105,   573,   591,   127,   412,
     110,     8,   112,   113,   114,   115,   116,   117,   118,   422,
     200,   168,   902,   116,   117,   118,   154,   155,   198,   157,
     104,   112,   113,   114,   115,   116,   117,    14,   112,   113,
     114,   115,   116,   117,    46,    47,    48,    49,    50,    51,
      14,    53,   180,    79,   154,   155,    14,   157,   200,   998,
     180,   168,   200,    65,   199,    14,    96,   931,   582,   199,
     199,   799,   104,   204,   202,   198,   198,   198,  1039,     9,
     180,    88,   666,     9,   668,   876,   199,  1048,   816,   199,
     909,    14,   911,   198,   200,     9,   690,  1669,   184,   180,
      79,   685,   202,    79,   685,  1317,   180,    79,   187,   198,
     200,     9,  1684,   200,   698,   699,    79,     9,   685,   200,
    1082,   199,   199,   637,   200,   125,   198,    66,   699,   920,
     199,    30,   126,   167,     9,   128,   927,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     199,   147,   199,    46,    47,    48,    49,    50,    51,   673,
       9,   675,   199,   747,    14,   196,   747,     9,     9,   753,
     989,   685,    65,   757,   758,    66,   199,   169,     9,   582,
     747,    14,   696,   125,   205,   699,    63,    64,  1400,   205,
     202,   851,     9,   198,   198,    96,  1015,   205,  1017,  1063,
    1162,   199,   205,   931,   788,   200,   788,   788,   198,   200,
     752,   199,   128,   147,   756,     9,   788,   788,   802,   199,
     198,   788,   147,  1042,   788,   788,   147,   198,  1189,   198,
     812,   198,   816,   747,   637,   201,   184,  1198,   184,    14,
       9,    79,   201,    14,   200,    96,  1207,   201,   830,    14,
     127,   765,   205,   181,   182,   183,  1022,   201,     4,    14,
     188,   189,   776,   777,   192,   193,  1639,   851,  1059,   200,
     673,    30,   675,  1153,   788,   816,   198,    30,  1097,   863,
     864,   865,    14,  1656,     9,   112,   113,   114,   115,   116,
     117,  1664,   198,   696,   888,   198,   123,   124,   198,    45,
     128,   198,    14,   199,     9,   889,   200,   891,   198,   893,
     891,   852,   893,   200,   199,    65,  1107,   205,   889,     9,
    1111,   144,   199,    79,   891,     9,   893,  1118,   912,   198,
    1542,   158,   128,   160,   200,  1063,  1297,    79,    14,    79,
     198,   201,  1222,   199,   198,   198,   930,   199,   128,     9,
     198,   935,   144,   180,   201,   201,   199,    30,  1238,   105,
     205,   200,   765,    73,   110,   199,   112,   113,   114,   115,
     116,   117,   118,   776,   777,   889,   200,   891,   169,   893,
     128,   895,   896,    30,   199,  1249,   199,   128,   998,     9,
     199,   199,     9,   199,   978,   202,   799,   981,     9,   199,
      96,   202,   201,   198,    14,   198,   990,   128,   154,   155,
     199,   157,   199,   199,     9,   199,   201,   199,  1002,   200,
    1002,  1002,  1241,  1384,  1243,   199,  1585,  1218,    30,     4,
    1002,  1002,   199,   199,   180,  1002,   200,   200,  1002,  1002,
     201,  1321,   156,   152,    79,    14,    79,  1266,  1030,   852,
    1330,  1033,   199,   198,   110,  1319,   202,   201,   128,   128,
    1279,   199,   199,  1343,   199,   199,    14,    79,   201,   200,
      45,    14,   199,   201,   198,   128,  1060,  1071,   200,   199,
    1064,   200,    14,    14,   200,    14,   199,    55,  1002,    79,
    1074,    79,   895,   896,   201,  1654,   198,     9,    79,   200,
     108,   147,  1086,  1074,    96,  1086,    33,    96,   159,    14,
     198,  1025,   199,   112,   113,   114,   115,   116,   117,  1086,
     200,  1249,   165,     4,   123,   124,  1406,   198,   931,    79,
     105,   162,   199,     9,    79,   110,   200,   112,   113,   114,
     115,   116,   117,   118,   199,  1281,    79,   199,    14,    79,
      14,   201,    14,    79,  1068,    79,    14,   504,  1647,   752,
    1074,   160,   383,   382,    45,   756,  1279,   850,  1082,  1083,
     380,   847,  1086,   791,  1660,  1405,  1235,  1656,  1160,   154,
     155,   180,   157,  1080,  1395,   509,  1424,  1169,  1508,  1274,
    1688,  1319,  1676,  1160,  1520,    41,  1391,  1016,   386,   973,
     480,   970,  1169,   744,   480,   180,  1006,   928,  1046,   864,
    1060,   292,   878,   310,  1196,    26,    27,  1369,   933,    30,
     285,   709,  1025,    -1,   105,    -1,  1210,   202,  1212,   110,
      -1,   112,   113,   114,   115,   116,   117,   118,   913,    -1,
      -1,    52,  1269,    -1,    -1,    -1,    -1,    -1,  1162,    -1,
      -1,    -1,  1532,    -1,  1534,    -1,    -1,    -1,    -1,    -1,
    1063,    -1,    -1,  1543,    -1,  1068,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   154,   155,    -1,   157,    -1,    -1,  1082,
    1083,  1265,    -1,    -1,  1265,  1269,    -1,    -1,    -1,    -1,
    1274,    -1,    -1,    -1,    -1,    -1,    -1,  1278,  1265,   180,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1588,    -1,
      -1,    -1,    -1,    -1,  1281,    -1,    -1,    -1,    -1,  1301,
      -1,   202,    -1,    -1,    -1,  1307,    -1,  1309,    -1,  1311,
      -1,    -1,    -1,  1317,  1301,    -1,    -1,    -1,  1322,    -1,
    1307,  1322,  1309,  1327,  1371,    -1,    -1,  1331,    -1,    -1,
    1331,  1265,    -1,    -1,    -1,  1322,  1327,    -1,    -1,  1162,
      -1,    -1,    -1,    -1,  1331,    -1,    -1,    -1,    -1,    -1,
    1522,    -1,    -1,    -1,  1358,    -1,    -1,    -1,    -1,    -1,
    1364,    -1,    -1,    -1,    -1,    -1,    -1,  1371,    -1,    -1,
      -1,  1375,  1373,    -1,    -1,    -1,    -1,    -1,   209,     4,
      -1,    -1,  1682,    -1,   215,    -1,    -1,    -1,  1322,  1689,
     221,    -1,    -1,  1327,    -1,  1397,  1400,  1331,    -1,  1403,
    1404,  1405,  1403,    -1,    -1,  1409,    -1,    -1,  1409,    -1,
    1397,    -1,  1416,  1404,  1405,  1416,  1403,    -1,   249,   250,
      45,    -1,  1409,    -1,   255,    -1,  1249,    -1,    -1,  1416,
    1669,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1519,
      -1,    -1,    -1,    -1,    -1,  1684,   277,    -1,    -1,    -1,
      -1,    -1,    -1,   284,   285,    -1,    -1,    -1,    -1,    -1,
     291,    -1,    -1,    -1,    -1,    -1,   297,    -1,    -1,  1403,
    1404,  1405,    -1,    -1,    -1,  1409,    -1,   308,    -1,  1635,
     105,    -1,  1416,    -1,    -1,   110,    -1,   112,   113,   114,
     115,   116,   117,   118,    -1,    -1,  1319,   328,  1578,    -1,
     331,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,  1519,    -1,    -1,  1651,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,
     155,    -1,   157,    -1,  1538,    -1,    -1,    -1,  1542,    -1,
      -1,   372,    -1,  1547,    -1,    -1,  1547,    -1,    -1,    -1,
      -1,    63,    64,  1557,    -1,   180,    -1,    -1,  1562,  1563,
    1547,    77,  1566,  1567,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1578,    -1,    -1,   202,    -1,    -1,
      -1,   412,  1586,  1587,    -1,  1586,  1587,    -1,    -1,  1593,
      -1,   422,  1593,    -1,    -1,    -1,    -1,    -1,    -1,  1586,
    1587,    -1,    -1,    -1,    -1,   121,  1593,    -1,    -1,    -1,
      -1,    -1,    -1,  1547,    -1,   127,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1627,    -1,    -1,  1627,    -1,    -1,    -1,
    1634,   462,   463,    -1,    -1,   466,    -1,  1639,   154,   155,
    1627,   157,   158,   159,    -1,    -1,  1650,    -1,  1635,    -1,
      -1,    -1,  1586,  1587,  1656,    -1,    -1,    -1,    -1,  1593,
      -1,    -1,  1664,    -1,    -1,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,    -1,
     511,    -1,   198,    -1,    -1,    -1,  1690,   199,    -1,  1690,
      -1,    -1,  1696,  1627,    -1,  1696,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1690,    -1,    -1,    -1,    -1,    -1,  1696,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,   557,   558,    -1,    -1,
      -1,    -1,   563,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,   582,    -1,    -1,    -1,    -1,  1690,    -1,    -1,    -1,
      -1,    29,  1696,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,   637,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,   673,    -1,   675,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,   696,   697,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    10,    11,    12,   709,   710,
     711,   712,   713,   714,   715,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   723,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,   748,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,   765,    -1,    -1,   205,    -1,    -1,
      -1,    -1,    -1,   774,    -1,   776,   777,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,   790,
     791,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   799,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,   202,   810,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     821,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   829,    -1,
      -1,   832,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    26,    27,    -1,    -1,    30,    -1,
      -1,   852,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,   895,   896,    -1,   202,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   909,    -1,
     911,    -1,   913,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,   928,    -1,    -1,
     931,   932,   933,    -1,    -1,   936,   937,   938,   939,   940,
     941,   942,   943,   944,   945,   946,   947,   948,   949,   950,
     951,   952,   953,   954,   955,   956,   957,   958,   959,   960,
     961,   962,    63,    64,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   986,    -1,    29,   989,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,  1015,    -1,  1017,    -1,    74,    75,
      76,    -1,    -1,    65,  1025,   202,   127,    -1,    -1,    85,
      -1,    -1,    -1,   215,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1042,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1063,    -1,    -1,    -1,    -1,  1068,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   132,   133,   134,   135,
     136,  1082,  1083,    -1,  1085,    -1,    -1,   143,    -1,    -1,
      -1,    63,    64,   149,   150,   277,  1097,    -1,    -1,  1100,
      -1,  1102,   284,   285,    -1,    -1,    -1,   163,    -1,   291,
      10,    11,    12,    -1,    -1,   297,    -1,    -1,    -1,    -1,
      -1,  1122,    -1,   179,    -1,    -1,   308,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,   127,    -1,    -1,  1159,    -1,
     202,  1162,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1185,    -1,    -1,    -1,    -1,    29,
     372,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    29,
     412,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1241,    -1,  1243,    -1,    -1,    -1,    -1,  1248,  1249,    -1,
      -1,  1252,    -1,  1254,    -1,    55,  1257,    -1,  1259,    -1,
      -1,  1262,    -1,    -1,    -1,  1266,    -1,    -1,  1269,  1270,
      -1,  1272,    -1,    -1,    -1,    -1,    -1,    77,  1279,    -1,
      -1,    -1,    -1,    -1,   466,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1294,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   202,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1319,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   511,
     130,   131,    -1,  1334,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1346,  1347,    -1,   148,    -1,
      -1,   151,   152,    -1,   154,   155,    -1,   157,   158,   159,
      -1,    -1,   202,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1371,    -1,   172,    -1,  1375,    -1,    -1,    -1,    -1,    -1,
    1381,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    -1,    -1,    -1,   198,    -1,
     582,    -1,    52,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,  1412,  1413,  1414,    -1,    -1,    -1,    -1,  1419,  1420,
      -1,    -1,    -1,  1424,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    10,
      11,    12,    -1,    -1,    -1,   637,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,   673,    53,   675,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   696,   697,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1527,    -1,    -1,   711,
     712,   713,   714,   715,    -1,    -1,    -1,  1538,    -1,    -1,
      -1,   723,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,  1557,    -1,    -1,    -1,
    1561,    -1,    -1,    -1,    -1,    -1,   748,    65,    -1,    -1,
      -1,  1572,    -1,    -1,    -1,    -1,  1577,    -1,    -1,  1580,
      -1,    -1,    -1,   765,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   774,    -1,   776,   777,    -1,   202,    -1,   249,
     250,    -1,    -1,    -1,    -1,   255,    -1,    -1,    -1,   791,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   799,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,  1638,    -1,    -1,
      -1,   202,    -1,    -1,    -1,  1646,    -1,   829,    -1,    65,
     832,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1660,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1669,    -1,
     852,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   328,    -1,
      -1,   331,    -1,  1684,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1693,    -1,    -1,    -1,    -1,    10,    11,    12,
    1701,    -1,    -1,    -1,  1705,    -1,    -1,  1708,    -1,    -1,
      -1,    -1,    -1,   895,   896,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,   928,    -1,    -1,   931,
     932,   933,    65,    -1,   936,   937,   938,   939,   940,   941,
     942,   943,   944,   945,   946,   947,   948,   949,   950,   951,
     952,   953,   954,   955,   956,   957,   958,   959,   960,   961,
     962,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,   986,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   462,   463,    -1,    29,   466,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,  1025,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   511,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,  1063,    53,    -1,    -1,    -1,  1068,    -1,    -1,   202,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
    1082,  1083,    -1,  1085,    -1,    -1,    -1,   557,   558,    -1,
      -1,    -1,    -1,   563,    -1,    -1,    -1,    -1,  1100,    -1,
    1102,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1122,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1159,    65,    -1,
    1162,    -1,    -1,    -1,    -1,    10,    11,    12,   202,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1185,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,   697,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   709,
     710,   711,   712,   713,   714,   715,  1248,  1249,    -1,    -1,
    1252,    -1,  1254,   723,    -1,  1257,    -1,  1259,    -1,    -1,
    1262,    77,    -1,    -1,    -1,    -1,    -1,    -1,  1270,    -1,
    1272,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,   105,
      -1,    -1,  1294,    -1,    29,   202,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,  1319,    53,    -1,
     790,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,  1334,    -1,    -1,   151,    -1,    -1,   154,   155,
     810,   157,   158,   159,  1346,  1347,    -1,    -1,    -1,    -1,
      -1,   821,    -1,    -1,    -1,   200,    -1,    -1,    -1,   829,
      -1,    10,    11,    12,    -1,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,  1381,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
    1412,  1413,  1414,    -1,    -1,    -1,    65,  1419,  1420,    -1,
      -1,    -1,  1424,    -1,    77,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,   909,
      -1,   911,    -1,   913,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   928,    -1,
      -1,    -1,   932,   933,   117,   200,   936,   937,   938,   939,
     940,   941,   942,   943,   944,   945,   946,   947,   948,   949,
     950,   951,   952,   953,   954,   955,   956,   957,   958,   959,
     960,   961,   962,    -1,    -1,   148,    -1,    -1,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   986,    -1,    -1,   989,
      -1,    -1,    -1,    -1,    -1,  1527,    -1,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,    -1,    -1,  1015,    -1,  1017,    -1,    -1,
     203,   200,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1561,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
    1572,    -1,  1042,    -1,    -1,  1577,    -1,    -1,  1580,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1085,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1097,    -1,    -1,
    1100,    -1,  1102,    -1,    -1,    -1,  1638,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1646,    -1,    -1,    -1,    -1,    -1,
       5,     6,  1122,     8,     9,    10,    11,    12,  1660,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    -1,    -1,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,
      -1,  1693,    -1,    48,    -1,    50,    -1,    -1,    53,  1701,
      55,    -1,    -1,  1705,    -1,    -1,  1708,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,  1185,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,   200,    -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1241,    -1,  1243,    -1,    -1,    -1,    -1,  1248,    -1,
      -1,    -1,  1252,    -1,  1254,    -1,    -1,  1257,    -1,  1259,
      -1,    -1,  1262,    -1,    -1,    -1,  1266,    -1,    -1,  1269,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1279,
      77,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1294,    -1,    -1,    -1,    -1,    -1,
      29,   186,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1334,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   230,    -1,    -1,   233,    -1,
      -1,   148,    -1,    -1,   151,   240,   241,   154,   155,    -1,
     157,   158,   159,    -1,    -1,    -1,    -1,   200,    -1,    -1,
      -1,  1371,    -1,    -1,    -1,  1375,    -1,    -1,    -1,    -1,
      -1,  1381,    -1,    -1,   466,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,    -1,
      -1,   286,    -1,    -1,    -1,   202,    -1,   292,    -1,    -1,
      -1,   296,  1412,  1413,  1414,    -1,    -1,    -1,    -1,  1419,
      -1,    -1,    -1,    -1,   309,   310,    -1,    -1,    -1,   511,
      -1,    -1,    -1,    -1,   319,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   330,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,
      -1,    -1,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,    -1,   373,    -1,
     375,   376,    -1,   378,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,    -1,    -1,    -1,    -1,    -1,   404,
     405,    -1,   407,   408,   409,    -1,    -1,  1527,    -1,    -1,
     415,    -1,    -1,   418,    -1,    -1,    -1,    -1,  1538,    -1,
      -1,    -1,   427,    -1,   429,    -1,    -1,    -1,    -1,    -1,
     435,    -1,    -1,    -1,    -1,    -1,    -1,  1557,    -1,    -1,
     445,  1561,   447,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   466,  1572,    -1,    -1,    -1,    -1,  1577,    -1,    -1,
    1580,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   473,    -1,
      -1,   476,   477,   478,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,   697,   511,    -1,    -1,    -1,
      -1,    -1,   507,    10,    11,    12,    -1,    -1,    65,   711,
     712,   713,   714,   715,    -1,    -1,    -1,    -1,  1638,    -1,
      -1,   723,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,  1669,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,  1684,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1693,    10,    11,    12,    -1,   583,    -1,
      -1,  1701,    -1,    -1,    -1,  1705,    -1,    -1,  1708,    -1,
     595,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      -1,    -1,   627,    -1,    -1,    -1,    -1,   829,    -1,    65,
      -1,   636,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   654,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,   681,    -1,    -1,    -1,
      -1,    -1,   697,    -1,    -1,    -1,    -1,    65,    -1,   694,
      -1,    -1,    -1,   200,    -1,    -1,   711,   712,   713,   714,
     715,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   723,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   928,    -1,   733,    -1,
     932,   933,    -1,    -1,   936,   937,   938,   939,   940,   941,
     942,   943,   944,   945,   946,   947,   948,   949,   950,   951,
     952,   953,   954,   955,   956,   957,   958,   959,   960,   961,
     962,    -1,    -1,   199,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,
     785,    79,    -1,    -1,   986,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   798,    -1,    -1,    -1,    -1,   803,    -1,
     805,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   829,    -1,    -1,    -1,    -1,    -1,
      -1,   199,   827,    -1,    -1,   123,    -1,    -1,    -1,    -1,
      -1,    -1,   837,    -1,    -1,   840,    -1,   842,    -1,    -1,
      -1,   846,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   856,    -1,    -1,    -1,    -1,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   881,    -1,    -1,    -1,
      -1,    -1,    -1,  1085,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,    -1,  1100,    -1,
    1102,    -1,    -1,   201,    -1,   203,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   928,    -1,    -1,    -1,   932,   933,    -1,
    1122,   936,   937,   938,   939,   940,   941,   942,   943,   944,
     945,   946,   947,   948,   949,   950,   951,   952,   953,   954,
     955,   956,   957,   958,   959,   960,   961,   962,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   964,
     965,   966,    -1,    -1,    -1,   970,   971,    -1,   466,    -1,
      -1,   986,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1185,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   998,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,   511,    -1,    -1,  1021,    -1,    -1,    -1,
      -1,  1026,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1039,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1048,    -1,  1050,  1248,    -1,    -1,    -1,
    1252,    -1,  1254,    -1,    -1,  1257,    -1,  1259,    -1,    -1,
    1262,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1085,    -1,    -1,    -1,  1079,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1089,  1100,    -1,  1102,    -1,    10,
      11,    12,  1294,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1122,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,  1334,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1172,    -1,    -1,
    1185,  1176,    -1,  1178,  1179,    -1,    -1,    66,    -1,  1381,
      -1,    -1,    -1,    -1,  1189,    74,    75,    76,    77,    -1,
      -1,    -1,  1197,  1198,    -1,    -1,    85,    -1,    -1,   697,
      -1,    -1,  1207,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1412,  1413,  1414,   711,   712,   713,   714,  1419,    -1,    -1,
      -1,  1423,    -1,    -1,    -1,   723,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1248,    -1,    -1,    -1,  1252,    -1,  1254,
      -1,   130,  1257,    -1,  1259,    -1,    -1,  1262,    -1,    -1,
      -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,  1263,    -1,
      -1,    -1,    29,    -1,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   199,  1294,
      -1,    -1,    -1,   172,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    -1,  1297,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,    -1,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,  1334,
      -1,    -1,    -1,    -1,    -1,  1527,    -1,    -1,    -1,    -1,
      -1,   829,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,  1561,
      -1,    -1,    -1,   130,   131,    -1,  1381,    -1,    -1,    65,
    1572,    -1,    -1,    -1,    -1,  1577,    -1,    -1,  1580,  1384,
      -1,   148,    -1,    -1,   151,   152,    -1,   154,   155,    -1,
     157,   158,   159,    -1,   161,    -1,    -1,  1412,  1413,  1414,
      -1,  1603,    -1,    -1,  1419,   172,    -1,    -1,    -1,    -1,
    1415,    -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,    -1,
      -1,   198,    -1,    -1,   932,   933,  1638,    -1,   936,   937,
     938,   939,   940,   941,   942,   943,   944,   945,   946,   947,
     948,   949,   950,   951,   952,   953,   954,   955,   956,   957,
     958,   959,   960,   961,   962,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   986,    27,
      28,  1693,    -1,    -1,    -1,    -1,    -1,    35,   194,  1701,
      -1,    -1,    -1,  1705,    -1,    -1,  1708,    -1,    46,    47,
      -1,    -1,  1527,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    -1,    -1,  1561,    85,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1572,    -1,    -1,
      -1,    -1,  1577,    -1,    -1,  1580,   104,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   112,   113,   114,   115,   116,   117,
      -1,    -1,   120,   121,    -1,    -1,    -1,  1085,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,  1100,    -1,  1102,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,    -1,  1638,  1122,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,
      -1,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,    -1,    -1,    -1,   203,   204,    -1,   206,   207,
      -1,     3,     4,     5,     6,     7,    -1,    -1,  1693,    -1,
      -1,    13,    -1,    -1,    -1,    -1,  1701,  1185,    -1,    -1,
    1705,    -1,    -1,  1708,    -1,    -1,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    -1,
    1248,    -1,    -1,    85,  1252,    -1,  1254,    -1,    -1,  1257,
      -1,  1259,    -1,    -1,  1262,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     112,   113,   114,   115,   116,   117,    -1,    -1,   120,   121,
      -1,    -1,    -1,    -1,    -1,    -1,  1294,   129,   130,    -1,
     132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,
      -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,   163,    -1,    -1,   166,    -1,  1334,    -1,    -1,    -1,
     172,   173,   174,    -1,    -1,    -1,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,   200,    -1,
      -1,   203,   204,    -1,   206,   207,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1381,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,  1412,  1413,  1414,    -1,    -1,    -1,
      -1,  1419,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    65,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,  1527,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1561,    -1,    -1,    -1,    -1,    -1,    27,
      28,    -1,    -1,    -1,  1572,    -1,    -1,    -1,    -1,  1577,
      -1,    -1,  1580,   190,   191,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,   186,    -1,    -1,    85,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
    1638,    -1,    -1,   101,   102,   103,   104,   105,   106,   107,
      -1,   109,   110,   111,   112,   113,   114,   115,   116,   117,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,  1693,   154,   155,    -1,   157,
     158,   159,   160,  1701,    -1,   163,    -1,  1705,   166,    -1,
    1708,    -1,    -1,    -1,   172,   173,   174,   175,    -1,   177,
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
     173,   174,   175,    -1,   177,    -1,   179,   180,    -1,   182,
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
      78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,   105,   106,   107,
      -1,   109,   110,   111,   112,   113,   114,   115,   116,   117,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,   160,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,   175,    -1,   177,
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
     173,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
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
      78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,   107,
      -1,   109,    -1,   111,   112,   113,   114,   115,   116,   117,
      -1,   119,   120,   121,    -1,   123,   124,    -1,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,   160,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,
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
     173,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
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
      78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,
      88,    89,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,   107,
      -1,   109,    -1,   111,   112,   113,   114,   115,   116,   117,
      -1,   119,   120,   121,    -1,   123,   124,    -1,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,   160,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,
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
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    -1,
      -1,    -1,    85,    86,    87,    88,    -1,    90,    -1,    92,
      -1,    94,    95,    -1,    97,    -1,    -1,    -1,   101,   102,
     103,   104,    -1,   106,   107,    -1,   109,    -1,   111,   112,
     113,   114,   115,   116,   117,    -1,   119,   120,   121,    -1,
     123,   124,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,
     133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,
     143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,   160,    -1,    -1,
     163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,
     173,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
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
      78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,   107,
      -1,   109,    -1,   111,   112,   113,   114,   115,   116,   117,
      -1,   119,   120,   121,    -1,   123,   124,    -1,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,   160,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,
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
     173,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
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
      78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,
      88,    -1,    90,    -1,    92,    93,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,   107,
      -1,   109,    -1,   111,   112,   113,   114,   115,   116,   117,
      -1,   119,   120,   121,    -1,   123,   124,    -1,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,   160,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,
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
     173,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
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
      78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,   107,
      -1,   109,    -1,   111,   112,   113,   114,   115,   116,   117,
      -1,   119,   120,   121,    -1,   123,   124,    -1,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,   160,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,
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
      -1,    74,    75,    76,    77,    78,    79,    -1,    81,    -1,
      -1,    -1,    85,    86,    87,    88,    -1,    90,    91,    92,
      -1,    94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,
     103,   104,    -1,   106,   107,    -1,   109,    -1,   111,   112,
     113,   114,   115,   116,   117,    -1,   119,   120,   121,    -1,
     123,   124,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,
     133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,
     143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,   160,    -1,    -1,
     163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,
     173,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
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
      78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,   107,
      -1,   109,    -1,   111,   112,   113,   114,   115,   116,   117,
      -1,   119,   120,   121,    -1,   123,   124,    -1,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,   160,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,
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
     173,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
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
      78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,   107,
      -1,   109,    -1,   111,   112,   113,   114,   115,   116,   117,
      -1,   119,   120,   121,    -1,   123,   124,    -1,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,   160,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,
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
     173,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
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
      78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,   107,
      -1,   109,    -1,   111,   112,   113,   114,   115,   116,   117,
      -1,   119,   120,   121,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,
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
     173,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
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
      78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,   107,
      -1,   109,    -1,   111,   112,   113,   114,   115,   116,   117,
      -1,   119,   120,   121,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,
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
     173,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,
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
      78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,    -1,   106,   107,
      -1,   109,    -1,   111,   112,   113,   114,   115,   116,   117,
      -1,   119,   120,   121,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,   200,   201,    -1,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
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
     163,    -1,    -1,   166,     3,     4,     5,     6,     7,   172,
     173,   174,    -1,    -1,    13,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    35,   200,   201,    -1,
     203,   204,    -1,   206,   207,    -1,    -1,    46,    47,    -1,
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
     159,    -1,   161,    -1,   163,    -1,    -1,   166,     3,     4,
       5,     6,     7,   172,   173,   174,    -1,    -1,    13,    -1,
     179,   180,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,   198,
      35,    -1,    -1,    -1,   203,   204,    -1,   206,   207,    -1,
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
     155,    -1,   157,   158,   159,    -1,   161,    -1,   163,    -1,
      -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,   174,
      -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,   198,    -1,    -1,    -1,    -1,   203,   204,
      -1,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
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
      -1,    -1,    -1,   163,    -1,    -1,   166,     3,     4,     5,
       6,     7,   172,   173,   174,    -1,    -1,    13,    -1,   179,
     180,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,   198,    -1,
      -1,    -1,    -1,   203,   204,    -1,   206,   207,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,   113,   114,   115,
     116,   117,    -1,    -1,   120,   121,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,   135,
     136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,    -1,    -1,    -1,   163,    -1,    -1,
     166,     3,     4,     5,     6,     7,   172,   173,   174,    -1,
      -1,    13,    -1,   179,   180,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      -1,    -1,   198,    35,    -1,    -1,    -1,   203,   204,    -1,
     206,   207,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
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
     172,   173,   174,    -1,    -1,    13,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,    -1,    -1,
      -1,   203,   204,    -1,   206,   207,    -1,    -1,    46,    47,
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
       4,     5,     6,     7,   172,   173,   174,    -1,    -1,    13,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,   200,    -1,    -1,   203,   204,    -1,   206,   207,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
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
     174,    -1,    -1,    13,    -1,   179,   180,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,   198,    -1,   200,    -1,    -1,   203,
     204,    -1,   206,   207,    -1,    -1,    46,    47,    -1,    -1,
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
      -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,   179,
     180,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,   198,   199,
      -1,    -1,    -1,   203,   204,    -1,   206,   207,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
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
      -1,    -1,    13,    -1,   179,   180,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,   198,    35,    -1,    -1,    -1,   203,   204,
      -1,   206,   207,    -1,    -1,    46,    47,    -1,    -1,    -1,
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
       7,   172,   173,   174,    -1,    -1,    13,    -1,   179,   180,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,    -1,   198,    35,    -1,
      -1,    -1,   203,   204,    -1,   206,   207,    -1,    -1,    46,
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
       3,     4,     5,     6,     7,   172,   173,   174,    -1,    -1,
      13,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    35,    -1,    -1,    -1,   203,   204,    -1,   206,
     207,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
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
     173,   174,    -1,    -1,    13,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,    -1,    -1,   198,    -1,    -1,    -1,    -1,
     203,   204,    -1,   206,   207,    -1,    -1,    46,    47,    -1,
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
       5,     6,     7,   172,   173,   174,    -1,    -1,    13,    -1,
     179,   180,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,   198,
      35,    -1,   201,    -1,   203,   204,    -1,   206,   207,    -1,
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
      -1,    -1,    13,    -1,   179,   180,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,   198,    -1,    -1,    -1,    -1,   203,   204,
      -1,   206,   207,    -1,    -1,    46,    47,    -1,    -1,    -1,
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
       7,   172,   173,   174,    -1,    -1,    13,    -1,   179,   180,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,    -1,   198,    -1,    -1,
      -1,    -1,   203,   204,    -1,   206,   207,    -1,    -1,    46,
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
      -1,    -1,    -1,    -1,    -1,   172,   173,   174,    -1,    -1,
      -1,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    -1,    -1,    -1,    -1,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    67,    68,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,    -1,    -1,   129,   130,    -1,
     132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   148,   149,   150,    -1,
      29,    -1,   154,   155,    -1,   157,   158,   159,   160,    -1,
     162,   163,    -1,   165,    -1,    -1,    -1,    -1,    -1,    -1,
     172,    -1,    -1,   175,    -1,   177,    55,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    10,    11,    12,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    29,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,   130,   131,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   148,
      -1,    -1,   151,   152,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   172,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,    -1,    -1,    -1,   198,
      -1,    -1,    -1,   202,    -1,    -1,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    55,    -1,   148,    -1,    -1,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,    -1,    -1,   185,    -1,
      -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,   172,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   104,    -1,    -1,   198,    29,    -1,    -1,   202,   112,
     113,   114,   115,   116,   117,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,   151,   152,
      -1,   154,   155,    77,   157,   158,   159,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   172,
      29,    -1,    -1,    -1,    -1,    -1,    -1,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,    -1,    -1,   198,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    -1,   148,    -1,    -1,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,    -1,   161,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   172,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   130,   131,    -1,   198,    -1,    -1,    77,    -1,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   148,
      -1,    -1,   151,   152,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   172,    -1,    -1,    -1,   117,    -1,    -1,
      30,    -1,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,    46,    47,    -1,   198,
      -1,    -1,    52,    -1,    54,    -1,    -1,    -1,   148,    -1,
      -1,   151,   152,    -1,   154,   155,    66,   157,   158,   159,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    -1,    -1,
      -1,    -1,    -1,    35,    -1,    85,    -1,    -1,    -1,    -1,
      -1,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    -1,    -1,    -1,   198,    -1,
      -1,   201,    -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    77,    -1,    79,    -1,    -1,
     130,    -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,
      -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,   152,    -1,   154,   155,    -1,   157,   158,   159,
      -1,    -1,    -1,   163,    -1,   117,    -1,    -1,    -1,    -1,
      -1,    -1,   172,   173,   174,    -1,    -1,   129,    -1,   179,
      77,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    -1,   148,    -1,   198,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    65,    46,    47,   198,    -1,    -1,    -1,
      52,   203,    54,    -1,   151,    -1,    -1,   154,   155,    -1,
     157,   158,   159,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    85,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,    -1,
      -1,    -1,    -1,    -1,   201,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    79,    -1,    -1,   130,    -1,
     132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,
      -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,   163,    -1,   117,    -1,    -1,    -1,    68,    -1,    -1,
     172,   173,   174,    -1,    -1,    -1,    77,   179,    79,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    -1,   148,    -1,   198,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,    -1,
      -1,    -1,   166,    -1,    -1,    -1,   117,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,    77,    -1,    -1,   198,    -1,    -1,   148,    -1,   203,
     151,   152,    -1,   154,   155,    -1,   157,   158,   159,    77,
      -1,    79,    -1,    -1,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    65,    -1,    -1,   198,    -1,   117,
      -1,    -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   129,    -1,    -1,    -1,    -1,   152,    -1,   154,   155,
     156,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,
     148,    -1,    -1,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,    77,    -1,    79,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,    -1,
      -1,    -1,   198,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,    -1,    -1,    -1,
     198,    -1,   117,    -1,    -1,   203,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   129,    77,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   148,    -1,    -1,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   117,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      -1,    -1,    -1,   198,    -1,    -1,   148,    -1,   203,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,    77,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    -1,    -1,    -1,   198,    -1,   117,    -1,
      77,   203,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    79,    -1,    -1,    -1,    -1,   148,
      -1,    -1,   151,   152,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,    -1,   154,   155,   198,
     157,   158,   159,    77,   203,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,
     155,    -1,   157,   158,   159,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,    -1,
      -1,    -1,    -1,    -1,   201,    -1,   203,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      77,    -1,    79,    -1,    -1,    -1,   201,    -1,   203,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     154,   155,    77,   157,   158,   159,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,    -1,    -1,    -1,    -1,    -1,    -1,   201,    -1,   203,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,   155,    -1,
     157,   158,   159,    74,    75,    76,    77,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,
     155,    -1,   157,   158,   159,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   154,   155,
      -1,   157,   158,   159,   201,    -1,   203,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      -1,    -1,    -1,   198,    -1,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,    -1,
      -1,    -1,   198,   154,   155,    -1,   157,   158,   159,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    29,    -1,    31,    32,    33,    34,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,
      65,    10,    11,    12,    -1,    85,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      29,   128,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,   118,    -1,    -1,
      -1,    -1,    -1,   128,    77,    -1,    -1,    -1,   148,   130,
     131,   151,    -1,    -1,   154,   155,    -1,   157,   158,   159,
      77,    -1,    79,    80,    -1,    -1,    -1,   148,    -1,    -1,
     151,   152,    -1,   154,   155,    -1,   157,   158,   159,    -1,
      -1,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    -1,    -1,    -1,    -1,   128,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    77,   148,    -1,    -1,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,   155,    -1,
     157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,    -1,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    77,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   152,
      -1,   154,   155,    77,   157,   158,   159,    -1,    -1,    -1,
      -1,   151,    -1,    -1,   154,   155,    -1,   157,   158,   159,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    -1,    -1,    -1,    -1,    -1,
      77,    -1,   151,    -1,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    -1,    -1,    77,    -1,   151,    -1,    -1,
     154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   123,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   123,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,   155,    -1,
     157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    -1,    -1,    -1,    -1,    -1,    -1,   154,
     155,    -1,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
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
      44,    45,    46,    47,    48,    49,    50,    51,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    12,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   209,   210,     0,   211,     3,     4,     5,     6,     7,
      13,    27,    28,    45,    46,    47,    52,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    66,    67,
      68,    69,    70,    74,    75,    76,    77,    78,    79,    81,
      85,    86,    87,    88,    90,    92,    94,    97,   101,   102,
     103,   104,   105,   106,   107,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   119,   120,   121,   122,   123,   124,
     129,   130,   132,   133,   134,   135,   136,   143,   148,   149,
     150,   151,   152,   154,   155,   157,   158,   159,   160,   163,
     166,   172,   173,   174,   175,   177,   179,   180,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   198,   200,   201,   203,   204,   206,   207,   212,
     215,   222,   223,   224,   225,   226,   227,   230,   246,   247,
     251,   254,   259,   265,   323,   324,   329,   333,   334,   335,
     336,   337,   338,   339,   340,   342,   345,   354,   355,   356,
     357,   360,   361,   363,   382,   392,   393,   394,   399,   402,
     420,   425,   427,   428,   429,   430,   431,   432,   433,   434,
     436,   453,   455,   457,   115,   116,   117,   129,   148,   158,
     215,   246,   323,   339,   427,   339,   198,   339,   339,   339,
     101,   339,   339,   418,   419,   339,   339,   339,   339,   339,
     339,   339,   339,   339,   339,   339,   339,    79,   117,   198,
     223,   393,   394,   427,   427,    35,   339,   440,   441,   339,
     117,   198,   223,   393,   394,   395,   426,   432,   437,   438,
     198,   330,   396,   198,   330,   346,   331,   339,   232,   330,
     198,   198,   198,   330,   200,   339,   215,   200,   339,    29,
      55,   130,   131,   152,   172,   198,   215,   226,   458,   469,
     470,   181,   200,   336,   339,   362,   364,   201,   239,   339,
     104,   105,   151,   216,   219,   222,    79,   203,   291,   292,
     123,   123,    79,   293,   198,   198,   198,   198,   215,   263,
     459,   198,   198,    79,    84,   144,   145,   146,   450,   451,
     151,   201,   222,   222,   215,   264,   459,   152,   198,   198,
     198,   459,   459,   347,   329,   339,   340,   427,   228,   201,
      84,   397,   450,    84,   450,   450,    30,   151,   168,   460,
     198,     9,   200,    35,   245,   152,   262,   459,   117,   246,
     324,   200,   200,   200,   200,   200,   200,    10,    11,    12,
      29,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    53,    65,   200,    66,    66,   200,   201,   147,
     124,   158,   160,   265,   322,   323,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    63,
      64,   127,   422,   423,    66,   201,   424,   198,    66,   201,
     203,   433,   198,   245,   246,    14,   339,   200,   128,    44,
     215,   417,   198,   329,   427,   147,   427,   128,   205,     9,
     404,   329,   427,   460,   147,   198,   398,   127,   422,   423,
     424,   199,   339,    30,   230,     8,   348,     9,   200,   230,
     231,   331,   332,   339,   215,   277,   234,   200,   200,   200,
     470,   470,   168,   198,   104,   470,    14,   215,    79,   200,
     200,   200,   181,   182,   183,   188,   189,   192,   193,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   377,   378,
     379,   240,   108,   165,   200,   151,   217,   220,   222,   151,
     218,   221,   222,   222,     9,   200,    96,   201,   427,     9,
     200,    14,     9,   200,   427,   454,   454,   329,   340,   427,
     199,   168,   257,   129,   427,   439,   440,    66,   127,   144,
     451,    78,   339,   427,    84,   144,   451,   222,   214,   200,
     201,   252,   260,   383,   385,    85,   203,   349,   350,   352,
     394,   433,   455,   339,   446,   447,   446,    14,    96,   456,
     287,   288,   420,   421,   199,   199,   199,   202,   229,   230,
     247,   254,   259,   420,   339,   204,   206,   207,   215,   461,
     462,   470,    35,   161,   289,   290,   339,   458,   198,   459,
     255,   245,   339,   339,   339,    30,   339,   339,   339,   339,
     339,   339,   339,   339,   339,   339,   339,   339,   339,   339,
     339,   339,   339,   339,   339,   339,   339,   339,   395,   339,
     339,   435,   435,   339,   442,   443,   123,   201,   215,   432,
     433,   263,   215,   264,   262,   246,    27,    35,   333,   336,
     339,   362,   339,   339,   339,   339,   339,   339,   339,   339,
     339,   339,   339,   339,   201,   215,   432,   435,   339,   289,
     435,   339,   439,   245,   199,   339,   198,   416,     9,   404,
     329,   199,   215,    35,   339,    35,   339,   199,   199,   432,
     289,   201,   215,   432,   199,   228,   281,   201,   339,   339,
      88,    30,   230,   275,   200,    28,    96,    14,     9,   199,
      30,   201,   278,   470,    85,   226,   466,   467,   468,   198,
       9,    46,    47,    52,    54,    66,    85,   130,   143,   152,
     172,   173,   174,   198,   223,   224,   226,   358,   359,   393,
     399,   400,   401,   184,    79,   339,    79,    79,   339,   374,
     375,   339,   339,   367,   377,   187,   380,   228,   198,   238,
     222,   200,     9,    96,   222,   200,     9,    96,    96,   219,
     215,   339,   292,   400,    79,     9,   199,   199,   199,   199,
     199,   200,   215,   465,   125,   268,   198,     9,   199,   199,
      79,    80,   215,   452,   215,    66,   202,   202,   211,   213,
      30,   126,   267,   167,    50,   152,   167,   387,   128,     9,
     404,   199,   147,   128,   199,     9,   404,   199,   470,   470,
      14,   196,     9,   405,   470,   471,   127,   422,   423,   424,
     202,     9,   404,   169,   427,   339,   199,     9,   405,    14,
     343,   248,   125,   266,   198,   459,   339,    30,   205,   205,
     128,   202,     9,   404,   339,   460,   198,   258,   253,   261,
     256,   245,    68,   427,   339,   460,   198,   205,   202,   199,
     205,   202,   199,    46,    47,    66,    74,    75,    76,    85,
     130,   143,   172,   215,   407,   409,   412,   415,   215,   427,
     427,   128,   422,   423,   424,   199,   339,   282,    71,    72,
     283,   228,   330,   228,   332,    96,    35,   129,   272,   427,
     400,   215,    30,   230,   276,   200,   279,   200,   279,     9,
     169,   128,   147,     9,   404,   199,   161,   461,   462,   463,
     461,   400,   400,   400,   400,   400,   403,   406,   198,    84,
     147,   198,   198,   198,   400,   147,   201,    10,    11,    12,
      29,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    65,   339,   184,   184,    14,   190,   191,   376,
       9,   194,   380,    79,   202,   393,   201,   242,    96,   220,
     215,    96,   221,   215,   215,   202,    14,   427,   200,    96,
       9,   169,   269,   393,   201,   439,   129,   427,    14,   205,
     339,   202,   211,   470,   269,   201,   386,    14,   339,   349,
     215,   339,   339,   200,   470,    30,   464,   421,    35,    79,
     161,   201,   215,   432,   470,    35,   161,   339,   400,   287,
     198,   393,   267,   344,   249,   339,   339,   339,   202,   198,
     289,   268,    30,   267,   266,   459,   395,   202,   198,   289,
      14,    74,    75,    76,   215,   408,   408,   409,   410,   411,
     198,    84,   144,   198,     9,   404,   199,   416,    35,   339,
     202,    71,    72,   284,   330,   230,   202,   200,    89,   200,
     272,   427,   198,   128,   271,    14,   228,   279,    98,    99,
     100,   279,   202,   470,   470,   215,   466,     9,   199,   404,
     128,   205,     9,   404,   403,   215,   349,   351,   353,   400,
     448,   449,   448,   199,   123,   215,   400,   444,   445,   400,
     400,   400,    30,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   400,   339,   339,   339,   375,
     339,   365,    79,   243,   215,   215,   400,   470,   215,     9,
     297,   199,   198,   333,   336,   339,   205,   202,   456,   297,
     153,   166,   201,   382,   389,   153,   201,   388,   128,   128,
     200,   470,   348,   471,    79,    14,    79,   339,   460,   198,
     427,   339,   199,   287,   201,   287,   198,   128,   198,   289,
     199,   201,   470,   201,   267,   250,   398,   198,   289,   199,
     128,   205,     9,   404,   410,   144,   349,   413,   414,   409,
     427,   330,    30,    73,   230,   200,   332,   271,   439,   272,
     199,   400,    95,    98,   200,   339,    30,   200,   280,   202,
     169,   128,   161,    30,   199,   400,   400,   199,   128,     9,
     404,   199,   128,   199,     9,   404,   199,   128,   202,     9,
     404,   400,    30,   185,   199,   228,    96,   393,     4,   105,
     110,   118,   154,   155,   157,   202,   298,   321,   322,   323,
     328,   420,   439,   202,   201,   202,    50,   339,   339,   339,
     339,    35,    79,   161,    14,   400,   202,   198,   289,   464,
     199,   297,   199,   287,   339,   289,   199,   297,   456,   297,
     201,   198,   289,   199,   409,   409,   199,   128,   199,     9,
     404,    30,   228,   200,   199,   199,   199,   235,   200,   200,
     280,   228,   470,   470,   128,   400,   349,   400,   400,   400,
     400,   400,   339,   201,   202,   470,   125,   126,   458,   270,
     393,   118,   130,   131,   152,   158,   307,   308,   309,   393,
     156,   313,   314,   121,   198,   215,   315,   316,   299,   246,
     470,     9,   200,   322,   199,   294,   152,   384,   202,   202,
      79,    14,    79,   400,   198,   289,   199,   110,   341,   464,
     202,   464,   199,   199,   202,   201,   202,   297,   287,   199,
     128,   409,   349,   228,   233,   236,    30,   230,   274,   228,
     199,   400,   128,   128,   128,   186,   228,   393,   393,    14,
       9,   200,   201,   201,     9,   200,     3,     4,     5,     6,
       7,    10,    11,    12,    13,    27,    28,    53,    67,    68,
      69,    70,    71,    72,    73,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   129,   130,   132,   133,
     134,   135,   136,   148,   149,   150,   160,   162,   163,   165,
     172,   175,   177,   179,   180,   215,   390,   391,     9,   200,
     152,   156,   215,   316,   317,   318,   200,    79,   327,   245,
     300,   458,   246,   202,   295,   296,   458,    14,   400,   289,
     199,   198,   201,   200,   201,   319,   341,   464,   294,   202,
     199,   409,   128,    30,   230,   273,   274,   228,   400,   400,
     400,   339,   202,   200,   200,   400,   393,   303,   310,   399,
     308,    14,    30,    47,   311,   314,     9,    33,   199,    29,
      46,    49,    14,     9,   200,   459,   327,    14,   245,   200,
      14,   400,   199,    35,    79,   381,   228,   228,   201,   319,
     202,   464,   409,   228,    93,   187,   241,   202,   215,   226,
     304,   305,   306,     9,   202,   400,   391,   391,    55,   312,
     317,   317,    29,    46,    49,   400,    79,   198,   200,   400,
     459,   400,    79,     9,   405,   202,   202,   228,   319,    91,
     200,    79,   108,   237,   147,    96,   399,   159,    14,   301,
     198,    35,    79,   199,   202,   200,   198,   165,   244,   215,
     322,   323,   400,   285,   286,   421,   302,    79,   393,   242,
     162,   215,   200,   199,     9,   405,   112,   113,   114,   325,
     326,   285,    79,   270,   200,   464,   421,   471,   199,   199,
     200,   200,   201,   320,   325,    35,    79,   161,   464,   201,
     228,   471,    79,    14,    79,   320,   228,   202,    35,    79,
     161,    14,   400,   202,    79,    14,    79,   400,    14,   400,
     400
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
#line 732 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 735 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 742 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 743 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 746 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 747 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 748 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 749 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 750 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 751 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 752 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 755 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 757 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 758 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 759 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 760 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 761 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 763 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 765 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 766 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 771 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 778 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 780 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 781 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 782 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 783 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 794 "hphp.y"
    { ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 795 "hphp.y"
    { ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 800 "hphp.y"
    { ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 801 "hphp.y"
    { ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 806 "hphp.y"
    { ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 807 "hphp.y"
    { ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 811 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 812 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 813 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 815 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 819 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 820 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 821 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 823 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 827 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 828 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 829 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 831 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 835 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 842 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 843 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 846 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 853 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 860 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 868 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 871 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 877 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 878 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 881 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 882 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 883 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 884 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 887 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 891 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 896 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 897 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 899 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 903 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 906 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 910 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 912 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 915 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 917 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 920 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 921 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 922 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 923 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 924 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 925 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 926 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 928 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 929 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 930 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 931 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 935 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 937 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 942 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 944 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 948 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 955 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 956 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 959 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 960 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 961 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 962 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval));;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 966 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 967 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 968 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 969 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 970 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 971 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 972 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 973 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 974 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 976 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 984 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 985 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 994 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 995 "hphp.y"
    { (yyval).reset();;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 999 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1001 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1008 "hphp.y"
    { (yyval).reset();;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1013 "hphp.y"
    { (yyval).reset();;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1017 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1022 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1028 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1034 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1040 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1046 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1052 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1060 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(2) - (2)]));;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1064 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]),0); ;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1068 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_ENUM,(yyvsp[(3) - (3)]));;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1072 "hphp.y"
    { _p->onEnum((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]),&(yyvsp[(1) - (10)])); ;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1078 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1081 "hphp.y"
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
#line 1096 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1099 "hphp.y"
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
#line 1113 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1116 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1121 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1124 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1131 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1134 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1142 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1145 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]), nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1153 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1154 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1158 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1161 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1164 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1165 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1166 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1170 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1171 "hphp.y"
    { (yyval).reset();;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1174 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1175 "hphp.y"
    { (yyval).reset();;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1178 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1179 "hphp.y"
    { (yyval).reset();;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1182 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1184 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1187 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1189 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1193 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1194 "hphp.y"
    { (yyval).reset();;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1197 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1198 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1199 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1203 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1205 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1208 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1210 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
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

  case 186:

/* Line 1455 of yacc.c  */
#line 1230 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1231 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1232 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1233 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1238 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1240 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1241 "hphp.y"
    { (yyval).reset();;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1244 "hphp.y"
    { (yyval).reset();;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1245 "hphp.y"
    { (yyval).reset();;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1250 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1251 "hphp.y"
    { (yyval).reset();;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1256 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1257 "hphp.y"
    { (yyval).reset();;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1260 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1261 "hphp.y"
    { (yyval).reset();;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1264 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1265 "hphp.y"
    { (yyval).reset();;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1279 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1287 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1292 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1295 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1301 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1305 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1310 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1315 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1320 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1325 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1331 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1337 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1345 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1350 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1354 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1357 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1361 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1364 "hphp.y"
    { (yyval).reset();;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1369 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1372 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1376 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1380 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1384 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1388 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1393 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1398 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1404 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1405 "hphp.y"
    { (yyval).reset();;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1408 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1409 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1410 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1412 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1414 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1416 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1420 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1421 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1424 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1425 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1426 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1430 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1432 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1433 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1434 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1439 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1440 "hphp.y"
    { (yyval).reset();;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1443 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1448 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1454 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1455 "hphp.y"
    { (yyval).reset();;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1458 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1459 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1462 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1463 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1465 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1469 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1475 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1482 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1488 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1493 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1495 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1497 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1499 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1501 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1502 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1505 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1508 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1510 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1516 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1520 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1523 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1530 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1531 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1539 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1546 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1552 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1553 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1559 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1566 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1568 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1573 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1578 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1582 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1585 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1590 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1595 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1596 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1598 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1603 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1604 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1605 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1610 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1615 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1617 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1621 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1624 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1625 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1629 "hphp.y"
    { (yyval).reset();;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1630 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1634 "hphp.y"
    { (yyval).reset();;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1635 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1638 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1639 "hphp.y"
    { (yyval).reset();;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1642 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1643 "hphp.y"
    { (yyval).reset();;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1646 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1651 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1652 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1654 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1655 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1657 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1661 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1662 "hphp.y"
    { (yyval).reset();;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1665 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1666 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1667 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1671 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1673 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1675 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1681 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1685 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1687 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1688 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1689 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1690 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1693 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1697 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1698 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1702 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1703 "hphp.y"
    { (yyval).reset();;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1708 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1717 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1721 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1725 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1730 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1735 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1736 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1745 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1748 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1749 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1751 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1752 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1761 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { (yyval).reset();;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1858 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { Token v; Token w;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1873 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1891 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1893 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1897 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1899 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { _p->onMapArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { _p->onMapArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { _p->onMapArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { _p->onMapArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1970 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1977 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1978 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1980 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1984 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1988 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1992 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1997 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 2001 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 2003 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 2007 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 2009 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 2013 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 2014 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2015 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2016 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2017 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2018 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2022 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2026 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2030 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2035 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2040 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2044 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2048 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2049 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2053 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2054 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2058 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2059 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2063 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2064 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2068 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2072 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2076 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2080 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2081 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2082 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2083 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2090 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2093 "hphp.y"
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

  case 508:

/* Line 1455 of yacc.c  */
#line 2104 "hphp.y"
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

  case 509:

/* Line 1455 of yacc.c  */
#line 2115 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2116 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2121 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { (yyval).reset();;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2125 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2126 "hphp.y"
    { (yyval).reset();;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2129 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2133 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2136 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2139 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2146 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2147 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2151 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2153 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2155 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2159 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2160 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2161 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2162 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2163 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2164 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2165 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2166 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2167 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2168 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2169 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2170 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2171 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2173 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2174 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2175 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2176 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2177 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2178 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2179 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2181 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2182 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2183 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2184 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2185 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2186 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2187 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2190 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2191 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2193 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2194 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2195 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2199 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2200 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2201 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2204 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2205 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2207 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2210 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2211 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2213 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2214 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2215 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2216 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2219 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2221 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2222 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2227 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2236 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval).reset();;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval).reset();;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval).reset();;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval).reset();;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2332 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2337 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2338 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2344 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2349 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { (yyval).reset();;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { (yyval).reset();;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { (yyval).reset();;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { (yyval).reset();;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { (yyval).reset();;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2452 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { (yyval).reset();;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2481 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2489 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2494 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2496 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2503 "hphp.y"
    { (yyval).reset();;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2513 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2518 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2519 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2523 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2538 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2539 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2540 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2541 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2543 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2549 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2553 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2558 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2562 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2567 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2569 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2578 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2581 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2589 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2593 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2608 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2619 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2620 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2621 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2624 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2625 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2628 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2629 "hphp.y"
    { (yyval).reset();;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2633 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2634 "hphp.y"
    { (yyval)++;;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2638 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2639 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2640 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2645 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2646 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2650 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2652 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2654 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2655 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2659 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2660 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2662 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2663 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2664 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2665 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2671 "hphp.y"
    { (yyval).reset();;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2675 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2676 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2677 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2678 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2681 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2683 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2684 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2685 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2690 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2691 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2695 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2696 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2697 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2698 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2703 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2704 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2709 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2711 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2713 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2720 "hphp.y"
    { _p->onEmptyMapArray((yyval));;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2724 "hphp.y"
    { _p->onMapArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2725 "hphp.y"
    { _p->onMapArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2730 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2731 "hphp.y"
    { _p->onEmptyMapArray((yyval));;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { _p->onMapArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2738 "hphp.y"
    { _p->onMapArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2742 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2744 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2745 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2747 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2752 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2756 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2758 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2760 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2761 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2764 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2765 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2766 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2770 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2771 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2772 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2773 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2774 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2775 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2776 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2777 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2778 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2782 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2783 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2788 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2790 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2804 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2808 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2814 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2815 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2821 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2825 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2831 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2832 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2836 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2839 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2845 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2850 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2851 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2852 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2853 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2857 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2858 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2863 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); ;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2864 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); ;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2866 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); ;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2867 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2873 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2878 "hphp.y"
    { ;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2890 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2892 "hphp.y"
    {;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2896 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2903 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2906 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2909 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2910 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2913 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2916 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2918 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2921 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2924 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2930 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2936 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2944 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2945 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13324 "hphp.tab.cpp"
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
#line 2948 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

