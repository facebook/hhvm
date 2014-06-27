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
     T_XHP_ENUM = 394,
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
     T_TYPE = 409,
     T_UNRESOLVED_TYPE = 410,
     T_NEWTYPE = 411,
     T_UNRESOLVED_NEWTYPE = 412,
     T_COMPILER_HALT_OFFSET = 413,
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
#line 875 "hphp.tab.cpp"

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
#define YYLAST   15922

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  206
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  252
/* YYNRULES -- Number of rules.  */
#define YYNRULES  877
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1641

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
       2,     2,     2,    52,   204,     2,   201,    51,    35,   205,
     196,   197,    49,    46,     9,    47,    48,    50,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    30,   198,
      40,    14,    41,    29,    55,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    66,     2,   203,    34,     2,   202,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   199,    33,   200,    54,     2,     2,     2,
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
     801,   807,   814,   823,   831,   834,   835,   837,   840,   843,
     847,   852,   857,   861,   863,   865,   868,   873,   877,   883,
     885,   889,   892,   893,   894,   899,   900,   906,   909,   910,
     921,   922,   934,   938,   942,   946,   951,   956,   960,   966,
     969,   972,   973,   980,   986,   991,   995,   997,   999,  1003,
    1008,  1010,  1012,  1014,  1016,  1021,  1023,  1025,  1029,  1032,
    1033,  1036,  1037,  1039,  1043,  1045,  1047,  1049,  1051,  1055,
    1060,  1065,  1070,  1072,  1074,  1077,  1080,  1083,  1087,  1091,
    1093,  1095,  1097,  1099,  1103,  1105,  1109,  1111,  1113,  1115,
    1116,  1118,  1121,  1123,  1125,  1127,  1129,  1131,  1133,  1135,
    1137,  1138,  1140,  1142,  1144,  1148,  1154,  1156,  1160,  1166,
    1171,  1175,  1179,  1182,  1184,  1186,  1190,  1194,  1196,  1198,
    1199,  1202,  1207,  1211,  1218,  1221,  1225,  1232,  1234,  1236,
    1238,  1245,  1249,  1254,  1261,  1265,  1269,  1273,  1277,  1281,
    1285,  1289,  1293,  1297,  1301,  1305,  1309,  1312,  1315,  1318,
    1321,  1325,  1329,  1333,  1337,  1341,  1345,  1349,  1353,  1357,
    1361,  1365,  1369,  1373,  1377,  1381,  1385,  1389,  1392,  1395,
    1398,  1401,  1405,  1409,  1413,  1417,  1421,  1425,  1429,  1433,
    1437,  1441,  1447,  1452,  1454,  1457,  1460,  1463,  1466,  1469,
    1472,  1475,  1478,  1481,  1483,  1485,  1487,  1491,  1494,  1496,
    1498,  1500,  1506,  1507,  1508,  1520,  1521,  1534,  1535,  1539,
    1540,  1547,  1550,  1555,  1557,  1559,  1565,  1569,  1575,  1579,
    1582,  1583,  1586,  1587,  1592,  1597,  1601,  1606,  1611,  1616,
    1621,  1623,  1625,  1627,  1631,  1634,  1638,  1643,  1646,  1650,
    1652,  1655,  1657,  1660,  1662,  1664,  1666,  1668,  1670,  1672,
    1677,  1682,  1685,  1694,  1705,  1708,  1710,  1714,  1716,  1719,
    1721,  1723,  1725,  1727,  1730,  1735,  1739,  1743,  1748,  1750,
    1753,  1758,  1761,  1768,  1769,  1771,  1776,  1777,  1780,  1781,
    1783,  1785,  1789,  1791,  1795,  1797,  1799,  1803,  1807,  1809,
    1811,  1813,  1815,  1817,  1819,  1821,  1823,  1825,  1827,  1829,
    1831,  1833,  1835,  1837,  1839,  1841,  1843,  1845,  1847,  1849,
    1851,  1853,  1855,  1857,  1859,  1861,  1863,  1865,  1867,  1869,
    1871,  1873,  1875,  1877,  1879,  1881,  1883,  1885,  1887,  1889,
    1891,  1893,  1895,  1897,  1899,  1901,  1903,  1905,  1907,  1909,
    1911,  1913,  1915,  1917,  1919,  1921,  1923,  1925,  1927,  1929,
    1931,  1933,  1935,  1937,  1939,  1941,  1943,  1945,  1947,  1949,
    1951,  1953,  1955,  1957,  1959,  1961,  1963,  1965,  1967,  1972,
    1974,  1976,  1978,  1980,  1982,  1984,  1986,  1988,  1991,  1993,
    1994,  1995,  1997,  1999,  2003,  2004,  2006,  2008,  2010,  2012,
    2014,  2016,  2018,  2020,  2022,  2024,  2026,  2028,  2030,  2034,
    2037,  2039,  2041,  2044,  2047,  2052,  2056,  2061,  2063,  2065,
    2067,  2071,  2075,  2079,  2083,  2087,  2091,  2095,  2099,  2103,
    2107,  2111,  2115,  2119,  2123,  2127,  2131,  2135,  2138,  2141,
    2145,  2149,  2153,  2157,  2161,  2165,  2169,  2173,  2179,  2184,
    2188,  2192,  2196,  2198,  2200,  2202,  2204,  2208,  2212,  2216,
    2219,  2220,  2222,  2223,  2225,  2226,  2232,  2236,  2240,  2242,
    2244,  2246,  2248,  2250,  2254,  2257,  2259,  2261,  2263,  2265,
    2267,  2269,  2272,  2275,  2280,  2284,  2289,  2292,  2293,  2299,
    2303,  2307,  2309,  2313,  2315,  2318,  2319,  2325,  2329,  2332,
    2333,  2337,  2338,  2343,  2346,  2347,  2351,  2355,  2357,  2358,
    2360,  2363,  2366,  2371,  2375,  2379,  2382,  2387,  2390,  2395,
    2397,  2399,  2401,  2403,  2405,  2408,  2413,  2417,  2422,  2426,
    2428,  2430,  2432,  2434,  2437,  2442,  2447,  2451,  2453,  2455,
    2459,  2467,  2474,  2483,  2493,  2502,  2513,  2521,  2528,  2537,
    2539,  2542,  2547,  2552,  2554,  2556,  2561,  2563,  2564,  2566,
    2569,  2571,  2573,  2576,  2581,  2585,  2589,  2590,  2592,  2595,
    2600,  2604,  2607,  2611,  2618,  2619,  2621,  2626,  2629,  2630,
    2636,  2640,  2644,  2646,  2653,  2658,  2663,  2666,  2669,  2670,
    2676,  2680,  2684,  2686,  2689,  2690,  2696,  2700,  2704,  2706,
    2709,  2712,  2714,  2717,  2719,  2724,  2728,  2732,  2739,  2743,
    2745,  2747,  2749,  2754,  2759,  2764,  2769,  2772,  2775,  2780,
    2783,  2786,  2788,  2792,  2796,  2800,  2801,  2804,  2810,  2817,
    2819,  2822,  2824,  2829,  2833,  2834,  2836,  2840,  2843,  2847,
    2849,  2851,  2852,  2853,  2856,  2860,  2862,  2868,  2872,  2876,
    2882,  2886,  2888,  2891,  2892,  2897,  2900,  2903,  2905,  2907,
    2909,  2911,  2916,  2923,  2925,  2934,  2941,  2943
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     207,     0,    -1,    -1,   208,   209,    -1,   209,   210,    -1,
      -1,   228,    -1,   244,    -1,   248,    -1,   253,    -1,   443,
      -1,   122,   196,   197,   198,    -1,   148,   220,   198,    -1,
      -1,   148,   220,   199,   211,   209,   200,    -1,    -1,   148,
     199,   212,   209,   200,    -1,   110,   214,   198,    -1,   110,
     104,   215,   198,    -1,   110,   105,   216,   198,    -1,   225,
     198,    -1,    77,    -1,   154,    -1,   155,    -1,   157,    -1,
     159,    -1,   158,    -1,   180,    -1,   181,    -1,   183,    -1,
     182,    -1,   184,    -1,   185,    -1,   186,    -1,   187,    -1,
     188,    -1,   189,    -1,   190,    -1,   191,    -1,   192,    -1,
     214,     9,   217,    -1,   217,    -1,   218,     9,   218,    -1,
     218,    -1,   219,     9,   219,    -1,   219,    -1,   220,    -1,
     151,   220,    -1,   220,    96,   213,    -1,   151,   220,    96,
     213,    -1,   220,    -1,   151,   220,    -1,   220,    96,   213,
      -1,   151,   220,    96,   213,    -1,   220,    -1,   151,   220,
      -1,   220,    96,   213,    -1,   151,   220,    96,   213,    -1,
     213,    -1,   220,   151,   213,    -1,   220,    -1,   148,   151,
     220,    -1,   151,   220,    -1,   221,    -1,   221,   446,    -1,
     221,   446,    -1,   225,     9,   444,    14,   390,    -1,   105,
     444,    14,   390,    -1,   226,   227,    -1,    -1,   228,    -1,
     244,    -1,   248,    -1,   253,    -1,   199,   226,   200,    -1,
      70,   321,   228,   275,   277,    -1,    70,   321,    30,   226,
     276,   278,    73,   198,    -1,    -1,    88,   321,   229,   269,
      -1,    -1,    87,   230,   228,    88,   321,   198,    -1,    -1,
      90,   196,   323,   198,   323,   198,   323,   197,   231,   267,
      -1,    -1,    97,   321,   232,   272,    -1,   101,   198,    -1,
     101,   330,   198,    -1,   103,   198,    -1,   103,   330,   198,
      -1,   106,   198,    -1,   106,   330,   198,    -1,    27,   101,
     198,    -1,   111,   285,   198,    -1,   117,   287,   198,    -1,
      86,   322,   198,    -1,   119,   196,   440,   197,   198,    -1,
     198,    -1,    81,    -1,    -1,    92,   196,   330,    96,   266,
     265,   197,   233,   268,    -1,    94,   196,   271,   197,   270,
      -1,    -1,   107,   236,   108,   196,   382,    79,   197,   199,
     226,   200,   238,   234,   241,    -1,    -1,   107,   236,   165,
     235,   239,    -1,   109,   330,   198,    -1,   102,   213,   198,
      -1,   330,   198,    -1,   324,   198,    -1,   325,   198,    -1,
     326,   198,    -1,   327,   198,    -1,   328,   198,    -1,   106,
     327,   198,    -1,   329,   198,    -1,   352,   198,    -1,   106,
     351,   198,    -1,   213,    30,    -1,    -1,   199,   237,   226,
     200,    -1,   238,   108,   196,   382,    79,   197,   199,   226,
     200,    -1,    -1,    -1,   199,   240,   226,   200,    -1,   165,
     239,    -1,    -1,    35,    -1,    -1,   104,    -1,    -1,   243,
     242,   445,   245,   196,   281,   197,   450,   310,    -1,    -1,
     314,   243,   242,   445,   246,   196,   281,   197,   450,   310,
      -1,    -1,   410,   313,   243,   242,   445,   247,   196,   281,
     197,   450,   310,    -1,    -1,   259,   256,   249,   260,   261,
     199,   288,   200,    -1,    -1,   410,   259,   256,   250,   260,
     261,   199,   288,   200,    -1,    -1,   124,   257,   251,   262,
     199,   288,   200,    -1,    -1,   410,   124,   257,   252,   262,
     199,   288,   200,    -1,    -1,   160,   258,   254,   261,   199,
     288,   200,    -1,    -1,   410,   160,   258,   255,   261,   199,
     288,   200,    -1,   445,    -1,   152,    -1,   445,    -1,   445,
      -1,   123,    -1,   116,   123,    -1,   115,   123,    -1,   125,
     382,    -1,    -1,   126,   263,    -1,    -1,   125,   263,    -1,
      -1,   382,    -1,   263,     9,   382,    -1,   382,    -1,   264,
       9,   382,    -1,   128,   266,    -1,    -1,   417,    -1,    35,
     417,    -1,   129,   196,   429,   197,    -1,   228,    -1,    30,
     226,    91,   198,    -1,   228,    -1,    30,   226,    93,   198,
      -1,   228,    -1,    30,   226,    89,   198,    -1,   228,    -1,
      30,   226,    95,   198,    -1,   213,    14,   390,    -1,   271,
       9,   213,    14,   390,    -1,   199,   273,   200,    -1,   199,
     198,   273,   200,    -1,    30,   273,    98,   198,    -1,    30,
     198,   273,    98,   198,    -1,   273,    99,   330,   274,   226,
      -1,   273,   100,   274,   226,    -1,    -1,    30,    -1,   198,
      -1,   275,    71,   321,   228,    -1,    -1,   276,    71,   321,
      30,   226,    -1,    -1,    72,   228,    -1,    -1,    72,    30,
     226,    -1,    -1,   280,     9,   411,   316,   457,   161,    79,
      -1,   280,     9,   411,   316,   457,   161,    -1,   280,   395,
      -1,   411,   316,   457,   161,    79,    -1,   411,   316,   457,
     161,    -1,    -1,   411,   316,   457,    79,    -1,   411,   316,
     457,    35,    79,    -1,   411,   316,   457,    35,    79,    14,
     390,    -1,   411,   316,   457,    79,    14,   390,    -1,   280,
       9,   411,   316,   457,    79,    -1,   280,     9,   411,   316,
     457,    35,    79,    -1,   280,     9,   411,   316,   457,    35,
      79,    14,   390,    -1,   280,     9,   411,   316,   457,    79,
      14,   390,    -1,   282,     9,   411,   457,   161,    79,    -1,
     282,     9,   411,   457,   161,    -1,   282,   395,    -1,   411,
     457,   161,    79,    -1,   411,   457,   161,    -1,    -1,   411,
     457,    79,    -1,   411,   457,    35,    79,    -1,   411,   457,
      35,    79,    14,   390,    -1,   411,   457,    79,    14,   390,
      -1,   282,     9,   411,   457,    79,    -1,   282,     9,   411,
     457,    35,    79,    -1,   282,     9,   411,   457,    35,    79,
      14,   390,    -1,   282,     9,   411,   457,    79,    14,   390,
      -1,   284,   395,    -1,    -1,   330,    -1,    35,   417,    -1,
     161,   330,    -1,   284,     9,   330,    -1,   284,     9,   161,
     330,    -1,   284,     9,    35,   417,    -1,   285,     9,   286,
      -1,   286,    -1,    79,    -1,   201,   417,    -1,   201,   199,
     330,   200,    -1,   287,     9,    79,    -1,   287,     9,    79,
      14,   390,    -1,    79,    -1,    79,    14,   390,    -1,   288,
     289,    -1,    -1,    -1,   312,   290,   318,   198,    -1,    -1,
     314,   456,   291,   318,   198,    -1,   319,   198,    -1,    -1,
     313,   243,   242,   445,   196,   292,   279,   197,   450,   311,
      -1,    -1,   410,   313,   243,   242,   445,   196,   293,   279,
     197,   450,   311,    -1,   154,   298,   198,    -1,   155,   304,
     198,    -1,   157,   306,   198,    -1,     4,   125,   382,   198,
      -1,     4,   126,   382,   198,    -1,   110,   264,   198,    -1,
     110,   264,   199,   294,   200,    -1,   294,   295,    -1,   294,
     296,    -1,    -1,   224,   147,   213,   162,   264,   198,    -1,
     297,    96,   313,   213,   198,    -1,   297,    96,   314,   198,
      -1,   224,   147,   213,    -1,   213,    -1,   299,    -1,   298,
       9,   299,    -1,   300,   379,   302,   303,    -1,   152,    -1,
     130,    -1,   382,    -1,   118,    -1,   158,   199,   301,   200,
      -1,   131,    -1,   388,    -1,   301,     9,   388,    -1,    14,
     390,    -1,    -1,    55,   159,    -1,    -1,   305,    -1,   304,
       9,   305,    -1,   156,    -1,   307,    -1,   213,    -1,   121,
      -1,   196,   308,   197,    -1,   196,   308,   197,    49,    -1,
     196,   308,   197,    29,    -1,   196,   308,   197,    46,    -1,
     307,    -1,   309,    -1,   309,    49,    -1,   309,    29,    -1,
     309,    46,    -1,   308,     9,   308,    -1,   308,    33,   308,
      -1,   213,    -1,   152,    -1,   156,    -1,   198,    -1,   199,
     226,   200,    -1,   198,    -1,   199,   226,   200,    -1,   314,
      -1,   118,    -1,   314,    -1,    -1,   315,    -1,   314,   315,
      -1,   112,    -1,   113,    -1,   114,    -1,   117,    -1,   116,
      -1,   115,    -1,   178,    -1,   317,    -1,    -1,   112,    -1,
     113,    -1,   114,    -1,   318,     9,    79,    -1,   318,     9,
      79,    14,   390,    -1,    79,    -1,    79,    14,   390,    -1,
     319,     9,   444,    14,   390,    -1,   105,   444,    14,   390,
      -1,   196,   320,   197,    -1,    68,   384,   387,    -1,    67,
     330,    -1,   371,    -1,   347,    -1,   196,   330,   197,    -1,
     322,     9,   330,    -1,   330,    -1,   322,    -1,    -1,    27,
     330,    -1,    27,   330,   128,   330,    -1,   417,    14,   324,
      -1,   129,   196,   429,   197,    14,   324,    -1,    28,   330,
      -1,   417,    14,   327,    -1,   129,   196,   429,   197,    14,
     327,    -1,   331,    -1,   417,    -1,   320,    -1,   129,   196,
     429,   197,    14,   330,    -1,   417,    14,   330,    -1,   417,
      14,    35,   417,    -1,   417,    14,    35,    68,   384,   387,
      -1,   417,    26,   330,    -1,   417,    25,   330,    -1,   417,
      24,   330,    -1,   417,    23,   330,    -1,   417,    22,   330,
      -1,   417,    21,   330,    -1,   417,    20,   330,    -1,   417,
      19,   330,    -1,   417,    18,   330,    -1,   417,    17,   330,
      -1,   417,    16,   330,    -1,   417,    15,   330,    -1,   417,
      64,    -1,    64,   417,    -1,   417,    63,    -1,    63,   417,
      -1,   330,    31,   330,    -1,   330,    32,   330,    -1,   330,
      10,   330,    -1,   330,    12,   330,    -1,   330,    11,   330,
      -1,   330,    33,   330,    -1,   330,    35,   330,    -1,   330,
      34,   330,    -1,   330,    48,   330,    -1,   330,    46,   330,
      -1,   330,    47,   330,    -1,   330,    49,   330,    -1,   330,
      50,   330,    -1,   330,    65,   330,    -1,   330,    51,   330,
      -1,   330,    45,   330,    -1,   330,    44,   330,    -1,    46,
     330,    -1,    47,   330,    -1,    52,   330,    -1,    54,   330,
      -1,   330,    37,   330,    -1,   330,    36,   330,    -1,   330,
      39,   330,    -1,   330,    38,   330,    -1,   330,    40,   330,
      -1,   330,    43,   330,    -1,   330,    41,   330,    -1,   330,
      42,   330,    -1,   330,    53,   384,    -1,   196,   331,   197,
      -1,   330,    29,   330,    30,   330,    -1,   330,    29,    30,
     330,    -1,   439,    -1,    62,   330,    -1,    61,   330,    -1,
      60,   330,    -1,    59,   330,    -1,    58,   330,    -1,    57,
     330,    -1,    56,   330,    -1,    69,   385,    -1,    55,   330,
      -1,   392,    -1,   346,    -1,   345,    -1,   202,   386,   202,
      -1,    13,   330,    -1,   333,    -1,   336,    -1,   349,    -1,
     110,   196,   370,   395,   197,    -1,    -1,    -1,   243,   242,
     196,   334,   281,   197,   450,   332,   199,   226,   200,    -1,
      -1,   314,   243,   242,   196,   335,   281,   197,   450,   332,
     199,   226,   200,    -1,    -1,    79,   337,   339,    -1,    -1,
     193,   338,   281,   194,   450,   339,    -1,     8,   330,    -1,
       8,   199,   226,   200,    -1,    85,    -1,   441,    -1,   341,
       9,   340,   128,   330,    -1,   340,   128,   330,    -1,   342,
       9,   340,   128,   390,    -1,   340,   128,   390,    -1,   341,
     394,    -1,    -1,   342,   394,    -1,    -1,   172,   196,   343,
     197,    -1,   130,   196,   430,   197,    -1,    66,   430,   203,
      -1,   382,   199,   432,   200,    -1,   382,   199,   434,   200,
      -1,   349,    66,   425,   203,    -1,   350,    66,   425,   203,
      -1,   346,    -1,   441,    -1,    85,    -1,   196,   331,   197,
      -1,   353,   354,    -1,   417,    14,   351,    -1,   179,    79,
     182,   330,    -1,   355,   366,    -1,   355,   366,   369,    -1,
     366,    -1,   366,   369,    -1,   356,    -1,   355,   356,    -1,
     357,    -1,   358,    -1,   359,    -1,   360,    -1,   361,    -1,
     362,    -1,   179,    79,   182,   330,    -1,   186,    79,    14,
     330,    -1,   180,   330,    -1,   181,    79,   182,   330,   183,
     330,   184,   330,    -1,   181,    79,   182,   330,   183,   330,
     184,   330,   185,    79,    -1,   187,   363,    -1,   364,    -1,
     363,     9,   364,    -1,   330,    -1,   330,   365,    -1,   188,
      -1,   189,    -1,   367,    -1,   368,    -1,   190,   330,    -1,
     191,   330,   192,   330,    -1,   185,    79,   354,    -1,   370,
       9,    79,    -1,   370,     9,    35,    79,    -1,    79,    -1,
      35,    79,    -1,   166,   152,   372,   167,    -1,   374,    50,
      -1,   374,   167,   375,   166,    50,   373,    -1,    -1,   152,
      -1,   374,   376,    14,   377,    -1,    -1,   375,   378,    -1,
      -1,   152,    -1,   153,    -1,   199,   330,   200,    -1,   153,
      -1,   199,   330,   200,    -1,   371,    -1,   380,    -1,   379,
      30,   380,    -1,   379,    47,   380,    -1,   213,    -1,    69,
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
      -1,   113,    -1,   112,    -1,   178,    -1,   119,    -1,   129,
      -1,   130,    -1,    10,    -1,    12,    -1,    11,    -1,   132,
      -1,   134,    -1,   133,    -1,   135,    -1,   136,    -1,   150,
      -1,   149,    -1,   177,    -1,   160,    -1,   163,    -1,   162,
      -1,   173,    -1,   175,    -1,   172,    -1,   223,   196,   283,
     197,    -1,   224,    -1,   152,    -1,   382,    -1,   117,    -1,
     423,    -1,   382,    -1,   117,    -1,   427,    -1,   196,   197,
      -1,   321,    -1,    -1,    -1,    84,    -1,   436,    -1,   196,
     283,   197,    -1,    -1,    74,    -1,    75,    -1,    76,    -1,
      85,    -1,   135,    -1,   136,    -1,   150,    -1,   132,    -1,
     163,    -1,   133,    -1,   134,    -1,   149,    -1,   177,    -1,
     143,    84,   144,    -1,   143,   144,    -1,   388,    -1,   222,
      -1,    46,   389,    -1,    47,   389,    -1,   130,   196,   393,
     197,    -1,    66,   393,   203,    -1,   172,   196,   344,   197,
      -1,   391,    -1,   348,    -1,   389,    -1,   196,   390,   197,
      -1,   390,    31,   390,    -1,   390,    32,   390,    -1,   390,
      10,   390,    -1,   390,    12,   390,    -1,   390,    11,   390,
      -1,   390,    33,   390,    -1,   390,    35,   390,    -1,   390,
      34,   390,    -1,   390,    48,   390,    -1,   390,    46,   390,
      -1,   390,    47,   390,    -1,   390,    49,   390,    -1,   390,
      50,   390,    -1,   390,    51,   390,    -1,   390,    45,   390,
      -1,   390,    44,   390,    -1,    52,   390,    -1,    54,   390,
      -1,   390,    37,   390,    -1,   390,    36,   390,    -1,   390,
      39,   390,    -1,   390,    38,   390,    -1,   390,    40,   389,
      -1,   390,    43,   390,    -1,   390,    41,   389,    -1,   390,
      42,   390,    -1,   390,    29,   390,    30,   390,    -1,   390,
      29,    30,   390,    -1,   224,   147,   213,    -1,   152,   147,
     213,    -1,   224,   147,   123,    -1,   222,    -1,    78,    -1,
     441,    -1,   388,    -1,   204,   436,   204,    -1,   205,   436,
     205,    -1,   143,   436,   144,    -1,   396,   394,    -1,    -1,
       9,    -1,    -1,     9,    -1,    -1,   396,     9,   390,   128,
     390,    -1,   396,     9,   390,    -1,   390,   128,   390,    -1,
     390,    -1,    74,    -1,    75,    -1,    76,    -1,    85,    -1,
     143,    84,   144,    -1,   143,   144,    -1,    74,    -1,    75,
      -1,    76,    -1,   213,    -1,   397,    -1,   213,    -1,    46,
     398,    -1,    47,   398,    -1,   130,   196,   400,   197,    -1,
      66,   400,   203,    -1,   172,   196,   403,   197,    -1,   401,
     394,    -1,    -1,   401,     9,   399,   128,   399,    -1,   401,
       9,   399,    -1,   399,   128,   399,    -1,   399,    -1,   402,
       9,   399,    -1,   399,    -1,   404,   394,    -1,    -1,   404,
       9,   340,   128,   399,    -1,   340,   128,   399,    -1,   402,
     394,    -1,    -1,   196,   405,   197,    -1,    -1,   407,     9,
     213,   406,    -1,   213,   406,    -1,    -1,   409,   407,   394,
      -1,    45,   408,    44,    -1,   410,    -1,    -1,   413,    -1,
     127,   422,    -1,   127,   213,    -1,   127,   199,   330,   200,
      -1,    66,   425,   203,    -1,   199,   330,   200,    -1,   418,
     414,    -1,   196,   320,   197,   414,    -1,   428,   414,    -1,
     196,   320,   197,   414,    -1,   422,    -1,   381,    -1,   420,
      -1,   421,    -1,   415,    -1,   417,   412,    -1,   196,   320,
     197,   412,    -1,   383,   147,   422,    -1,   419,   196,   283,
     197,    -1,   196,   417,   197,    -1,   381,    -1,   420,    -1,
     421,    -1,   415,    -1,   417,   413,    -1,   196,   320,   197,
     413,    -1,   419,   196,   283,   197,    -1,   196,   417,   197,
      -1,   422,    -1,   415,    -1,   196,   417,   197,    -1,   417,
     127,   213,   446,   196,   283,   197,    -1,   417,   127,   422,
     196,   283,   197,    -1,   417,   127,   199,   330,   200,   196,
     283,   197,    -1,   196,   320,   197,   127,   213,   446,   196,
     283,   197,    -1,   196,   320,   197,   127,   422,   196,   283,
     197,    -1,   196,   320,   197,   127,   199,   330,   200,   196,
     283,   197,    -1,   383,   147,   213,   446,   196,   283,   197,
      -1,   383,   147,   422,   196,   283,   197,    -1,   383,   147,
     199,   330,   200,   196,   283,   197,    -1,   423,    -1,   426,
     423,    -1,   423,    66,   425,   203,    -1,   423,   199,   330,
     200,    -1,   424,    -1,    79,    -1,   201,   199,   330,   200,
      -1,   330,    -1,    -1,   201,    -1,   426,   201,    -1,   422,
      -1,   416,    -1,   427,   412,    -1,   196,   320,   197,   412,
      -1,   383,   147,   422,    -1,   196,   417,   197,    -1,    -1,
     416,    -1,   427,   413,    -1,   196,   320,   197,   413,    -1,
     196,   417,   197,    -1,   429,     9,    -1,   429,     9,   417,
      -1,   429,     9,   129,   196,   429,   197,    -1,    -1,   417,
      -1,   129,   196,   429,   197,    -1,   431,   394,    -1,    -1,
     431,     9,   330,   128,   330,    -1,   431,     9,   330,    -1,
     330,   128,   330,    -1,   330,    -1,   431,     9,   330,   128,
      35,   417,    -1,   431,     9,    35,   417,    -1,   330,   128,
      35,   417,    -1,    35,   417,    -1,   433,   394,    -1,    -1,
     433,     9,   330,   128,   330,    -1,   433,     9,   330,    -1,
     330,   128,   330,    -1,   330,    -1,   435,   394,    -1,    -1,
     435,     9,   390,   128,   390,    -1,   435,     9,   390,    -1,
     390,   128,   390,    -1,   390,    -1,   436,   437,    -1,   436,
      84,    -1,   437,    -1,    84,   437,    -1,    79,    -1,    79,
      66,   438,   203,    -1,    79,   127,   213,    -1,   145,   330,
     200,    -1,   145,    78,    66,   330,   203,   200,    -1,   146,
     417,   200,    -1,   213,    -1,    80,    -1,    79,    -1,   120,
     196,   440,   197,    -1,   121,   196,   417,   197,    -1,   121,
     196,   331,   197,    -1,   121,   196,   320,   197,    -1,     7,
     330,    -1,     6,   330,    -1,     5,   196,   330,   197,    -1,
       4,   330,    -1,     3,   330,    -1,   417,    -1,   440,     9,
     417,    -1,   383,   147,   213,    -1,   383,   147,   123,    -1,
      -1,    96,   456,    -1,   173,   445,    14,   456,   198,    -1,
     175,   445,   442,    14,   456,   198,    -1,   213,    -1,   456,
     213,    -1,   213,    -1,   213,   168,   451,   169,    -1,   168,
     448,   169,    -1,    -1,   456,    -1,   447,     9,   456,    -1,
     447,   394,    -1,   447,     9,   161,    -1,   448,    -1,   161,
      -1,    -1,    -1,    30,   456,    -1,   451,     9,   213,    -1,
     213,    -1,   451,     9,   213,    96,   456,    -1,   213,    96,
     456,    -1,    85,   128,   456,    -1,   224,   147,   213,   128,
     456,    -1,   453,     9,   452,    -1,   452,    -1,   453,   394,
      -1,    -1,   172,   196,   454,   197,    -1,    29,   456,    -1,
      55,   456,    -1,   224,    -1,   130,    -1,   131,    -1,   455,
      -1,   130,   168,   456,   169,    -1,   130,   168,   456,     9,
     456,   169,    -1,   152,    -1,   196,   104,   196,   449,   197,
      30,   456,   197,    -1,   196,   456,     9,   447,   394,   197,
      -1,   456,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   730,   730,   730,   739,   741,   744,   745,   746,   747,
     748,   749,   752,   754,   754,   756,   756,   758,   759,   761,
     763,   768,   769,   770,   771,   772,   773,   774,   775,   776,
     777,   778,   779,   780,   781,   782,   783,   784,   785,   786,
     790,   792,   796,   798,   802,   804,   808,   809,   810,   811,
     816,   817,   818,   819,   824,   825,   826,   827,   832,   833,
     837,   838,   840,   843,   849,   856,   863,   867,   873,   875,
     878,   879,   880,   881,   884,   885,   889,   894,   894,   900,
     900,   907,   906,   912,   912,   917,   918,   919,   920,   921,
     922,   923,   924,   925,   926,   927,   928,   929,   932,   930,
     937,   945,   939,   949,   947,   951,   952,   956,   957,   958,
     959,   960,   961,   962,   963,   964,   965,   966,   974,   974,
     979,   985,   989,   989,   997,   998,  1002,  1003,  1007,  1012,
    1011,  1024,  1022,  1036,  1034,  1050,  1049,  1068,  1066,  1085,
    1084,  1093,  1091,  1103,  1102,  1114,  1112,  1125,  1126,  1130,
    1133,  1136,  1137,  1138,  1141,  1143,  1146,  1147,  1150,  1151,
    1154,  1155,  1159,  1160,  1165,  1166,  1169,  1170,  1171,  1175,
    1176,  1180,  1181,  1185,  1186,  1190,  1191,  1196,  1197,  1202,
    1203,  1204,  1205,  1208,  1211,  1213,  1216,  1217,  1221,  1223,
    1226,  1229,  1232,  1233,  1236,  1237,  1241,  1247,  1254,  1256,
    1261,  1267,  1271,  1275,  1279,  1284,  1289,  1294,  1299,  1305,
    1314,  1319,  1325,  1327,  1331,  1336,  1340,  1343,  1346,  1350,
    1354,  1358,  1362,  1367,  1375,  1377,  1380,  1381,  1382,  1383,
    1385,  1387,  1392,  1393,  1396,  1397,  1398,  1402,  1403,  1405,
    1406,  1410,  1412,  1415,  1415,  1419,  1418,  1422,  1426,  1424,
    1439,  1436,  1449,  1451,  1453,  1455,  1457,  1459,  1461,  1465,
    1466,  1467,  1470,  1476,  1479,  1485,  1488,  1493,  1495,  1500,
    1505,  1509,  1510,  1516,  1517,  1519,  1523,  1524,  1529,  1530,
    1534,  1535,  1539,  1541,  1547,  1552,  1553,  1555,  1559,  1560,
    1561,  1562,  1566,  1567,  1568,  1569,  1570,  1571,  1573,  1578,
    1581,  1582,  1586,  1587,  1591,  1592,  1595,  1596,  1599,  1600,
    1603,  1604,  1608,  1609,  1610,  1611,  1612,  1613,  1614,  1618,
    1619,  1622,  1623,  1624,  1627,  1629,  1631,  1632,  1635,  1637,
    1642,  1643,  1645,  1646,  1647,  1650,  1654,  1655,  1659,  1660,
    1664,  1665,  1669,  1673,  1678,  1682,  1686,  1691,  1692,  1693,
    1696,  1698,  1699,  1700,  1703,  1704,  1705,  1706,  1707,  1708,
    1709,  1710,  1711,  1712,  1713,  1714,  1715,  1716,  1717,  1718,
    1719,  1720,  1721,  1722,  1723,  1724,  1725,  1726,  1727,  1728,
    1729,  1730,  1731,  1732,  1733,  1734,  1735,  1736,  1737,  1738,
    1739,  1740,  1741,  1742,  1743,  1744,  1745,  1747,  1748,  1750,
    1752,  1753,  1754,  1755,  1756,  1757,  1758,  1759,  1760,  1761,
    1762,  1763,  1764,  1765,  1766,  1767,  1768,  1769,  1770,  1771,
    1772,  1776,  1780,  1785,  1784,  1799,  1797,  1814,  1814,  1829,
    1829,  1847,  1848,  1853,  1855,  1859,  1863,  1869,  1873,  1879,
    1881,  1885,  1887,  1891,  1895,  1896,  1900,  1907,  1914,  1916,
    1921,  1922,  1923,  1925,  1929,  1933,  1937,  1941,  1943,  1945,
    1947,  1952,  1953,  1958,  1959,  1960,  1961,  1962,  1963,  1967,
    1971,  1975,  1979,  1984,  1989,  1993,  1994,  1998,  1999,  2003,
    2004,  2008,  2009,  2013,  2017,  2021,  2025,  2026,  2027,  2028,
    2032,  2038,  2047,  2060,  2061,  2064,  2067,  2070,  2071,  2074,
    2078,  2081,  2084,  2091,  2092,  2096,  2097,  2099,  2103,  2104,
    2105,  2106,  2107,  2108,  2109,  2110,  2111,  2112,  2113,  2114,
    2115,  2116,  2117,  2118,  2119,  2120,  2121,  2122,  2123,  2124,
    2125,  2126,  2127,  2128,  2129,  2130,  2131,  2132,  2133,  2134,
    2135,  2136,  2137,  2138,  2139,  2140,  2141,  2142,  2143,  2144,
    2145,  2146,  2147,  2148,  2149,  2150,  2151,  2152,  2153,  2154,
    2155,  2156,  2157,  2158,  2159,  2160,  2161,  2162,  2163,  2164,
    2165,  2166,  2167,  2168,  2169,  2170,  2171,  2172,  2173,  2174,
    2175,  2176,  2177,  2178,  2179,  2180,  2181,  2182,  2186,  2191,
    2192,  2195,  2196,  2197,  2201,  2202,  2203,  2207,  2208,  2209,
    2213,  2214,  2215,  2218,  2220,  2224,  2225,  2226,  2227,  2229,
    2230,  2231,  2232,  2233,  2234,  2235,  2236,  2237,  2238,  2241,
    2246,  2247,  2248,  2249,  2250,  2252,  2253,  2255,  2256,  2260,
    2261,  2262,  2264,  2266,  2268,  2270,  2272,  2273,  2274,  2275,
    2276,  2277,  2278,  2279,  2280,  2281,  2282,  2283,  2284,  2285,
    2287,  2289,  2291,  2293,  2294,  2297,  2298,  2302,  2304,  2308,
    2311,  2314,  2320,  2321,  2322,  2323,  2324,  2325,  2326,  2331,
    2333,  2337,  2338,  2341,  2342,  2346,  2349,  2351,  2353,  2357,
    2358,  2359,  2360,  2362,  2365,  2369,  2370,  2371,  2372,  2375,
    2376,  2377,  2378,  2379,  2381,  2382,  2387,  2389,  2392,  2395,
    2397,  2399,  2402,  2404,  2408,  2410,  2413,  2416,  2422,  2424,
    2427,  2428,  2433,  2436,  2440,  2440,  2445,  2448,  2449,  2453,
    2454,  2459,  2460,  2464,  2465,  2469,  2470,  2475,  2477,  2482,
    2483,  2484,  2485,  2486,  2487,  2488,  2490,  2493,  2495,  2499,
    2500,  2501,  2502,  2503,  2505,  2507,  2509,  2513,  2514,  2515,
    2519,  2522,  2525,  2528,  2532,  2536,  2543,  2547,  2551,  2558,
    2559,  2564,  2566,  2567,  2570,  2571,  2574,  2575,  2579,  2580,
    2584,  2585,  2586,  2587,  2589,  2592,  2595,  2596,  2597,  2599,
    2601,  2605,  2606,  2607,  2609,  2610,  2611,  2615,  2617,  2620,
    2622,  2623,  2624,  2625,  2628,  2630,  2631,  2635,  2637,  2640,
    2642,  2643,  2644,  2648,  2650,  2653,  2656,  2658,  2660,  2664,
    2665,  2667,  2668,  2674,  2675,  2677,  2679,  2681,  2683,  2686,
    2687,  2688,  2692,  2693,  2694,  2695,  2696,  2697,  2698,  2699,
    2700,  2704,  2705,  2709,  2711,  2719,  2721,  2725,  2729,  2736,
    2737,  2743,  2744,  2751,  2754,  2758,  2761,  2766,  2771,  2773,
    2774,  2775,  2779,  2780,  2784,  2786,  2787,  2789,  2793,  2796,
    2805,  2807,  2811,  2814,  2817,  2825,  2828,  2831,  2832,  2835,
    2838,  2839,  2842,  2846,  2850,  2856,  2866,  2867
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
  "T_XHP_CATEGORY_LABEL", "T_XHP_CHILDREN", "T_XHP_ENUM", "T_XHP_REQUIRED",
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
     284,   284,   285,   285,   286,   286,   286,   287,   287,   287,
     287,   288,   288,   290,   289,   291,   289,   289,   292,   289,
     293,   289,   289,   289,   289,   289,   289,   289,   289,   294,
     294,   294,   295,   296,   296,   297,   297,   298,   298,   299,
     299,   300,   300,   300,   300,   300,   301,   301,   302,   302,
     303,   303,   304,   304,   305,   306,   306,   306,   307,   307,
     307,   307,   308,   308,   308,   308,   308,   308,   308,   309,
     309,   309,   310,   310,   311,   311,   312,   312,   313,   313,
     314,   314,   315,   315,   315,   315,   315,   315,   315,   316,
     316,   317,   317,   317,   318,   318,   318,   318,   319,   319,
     320,   320,   320,   320,   320,   321,   322,   322,   323,   323,
     324,   324,   325,   326,   327,   328,   329,   330,   330,   330,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   332,   332,   334,   333,   335,   333,   337,   336,   338,
     336,   339,   339,   340,   340,   341,   341,   342,   342,   343,
     343,   344,   344,   345,   346,   346,   347,   348,   349,   349,
     350,   350,   350,   350,   351,   352,   353,   354,   354,   354,
     354,   355,   355,   356,   356,   356,   356,   356,   356,   357,
     358,   359,   360,   361,   362,   363,   363,   364,   364,   365,
     365,   366,   366,   367,   368,   369,   370,   370,   370,   370,
     371,   372,   372,   373,   373,   374,   374,   375,   375,   376,
     377,   377,   378,   378,   378,   379,   379,   379,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   381,   382,
     382,   383,   383,   383,   384,   384,   384,   385,   385,   385,
     386,   386,   386,   387,   387,   388,   388,   388,   388,   388,
     388,   388,   388,   388,   388,   388,   388,   388,   388,   388,
     389,   389,   389,   389,   389,   389,   389,   389,   389,   390,
     390,   390,   390,   390,   390,   390,   390,   390,   390,   390,
     390,   390,   390,   390,   390,   390,   390,   390,   390,   390,
     390,   390,   390,   390,   390,   390,   390,   390,   390,   391,
     391,   391,   392,   392,   392,   392,   392,   392,   392,   393,
     393,   394,   394,   395,   395,   396,   396,   396,   396,   397,
     397,   397,   397,   397,   397,   398,   398,   398,   398,   399,
     399,   399,   399,   399,   399,   399,   400,   400,   401,   401,
     401,   401,   402,   402,   403,   403,   404,   404,   405,   405,
     406,   406,   407,   407,   409,   408,   410,   411,   411,   412,
     412,   413,   413,   414,   414,   415,   415,   416,   416,   417,
     417,   417,   417,   417,   417,   417,   417,   417,   417,   418,
     418,   418,   418,   418,   418,   418,   418,   419,   419,   419,
     420,   420,   420,   420,   420,   420,   421,   421,   421,   422,
     422,   423,   423,   423,   424,   424,   425,   425,   426,   426,
     427,   427,   427,   427,   427,   427,   428,   428,   428,   428,
     428,   429,   429,   429,   429,   429,   429,   430,   430,   431,
     431,   431,   431,   431,   431,   431,   431,   432,   432,   433,
     433,   433,   433,   434,   434,   435,   435,   435,   435,   436,
     436,   436,   436,   437,   437,   437,   437,   437,   437,   438,
     438,   438,   439,   439,   439,   439,   439,   439,   439,   439,
     439,   440,   440,   441,   441,   442,   442,   443,   443,   444,
     444,   445,   445,   446,   446,   447,   447,   448,   449,   449,
     449,   449,   450,   450,   451,   451,   451,   451,   452,   452,
     453,   453,   454,   454,   455,   456,   456,   456,   456,   456,
     456,   456,   456,   456,   456,   456,   457,   457
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
       5,     6,     8,     7,     2,     0,     1,     2,     2,     3,
       4,     4,     3,     1,     1,     2,     4,     3,     5,     1,
       3,     2,     0,     0,     4,     0,     5,     2,     0,    10,
       0,    11,     3,     3,     3,     4,     4,     3,     5,     2,
       2,     0,     6,     5,     4,     3,     1,     1,     3,     4,
       1,     1,     1,     1,     4,     1,     1,     3,     2,     0,
       2,     0,     1,     3,     1,     1,     1,     1,     3,     4,
       4,     4,     1,     1,     2,     2,     2,     3,     3,     1,
       1,     1,     1,     3,     1,     3,     1,     1,     1,     0,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     1,     1,     1,     3,     5,     1,     3,     5,     4,
       3,     3,     2,     1,     1,     3,     3,     1,     1,     0,
       2,     4,     3,     6,     2,     3,     6,     1,     1,     1,
       6,     3,     4,     6,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     2,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     5,     4,     1,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     1,     1,     1,     3,     2,     1,     1,
       1,     5,     0,     0,    11,     0,    12,     0,     3,     0,
       6,     2,     4,     1,     1,     5,     3,     5,     3,     2,
       0,     2,     0,     4,     4,     3,     4,     4,     4,     4,
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
       1,     1,     2,     2,     4,     3,     4,     1,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     5,     4,     3,
       3,     3,     1,     1,     1,     1,     3,     3,     3,     2,
       0,     1,     0,     1,     0,     5,     3,     3,     1,     1,
       1,     1,     1,     3,     2,     1,     1,     1,     1,     1,
       1,     2,     2,     4,     3,     4,     2,     0,     5,     3,
       3,     1,     3,     1,     2,     0,     5,     3,     2,     0,
       3,     0,     4,     2,     0,     3,     3,     1,     0,     1,
       2,     2,     4,     3,     3,     2,     4,     2,     4,     1,
       1,     1,     1,     1,     2,     4,     3,     4,     3,     1,
       1,     1,     1,     2,     4,     4,     3,     1,     1,     3,
       7,     6,     8,     9,     8,    10,     7,     6,     8,     1,
       2,     4,     4,     1,     1,     4,     1,     0,     1,     2,
       1,     1,     2,     4,     3,     3,     0,     1,     2,     4,
       3,     2,     3,     6,     0,     1,     4,     2,     0,     5,
       3,     3,     1,     6,     4,     4,     2,     2,     0,     5,
       3,     3,     1,     2,     0,     5,     3,     3,     1,     2,
       2,     1,     2,     1,     4,     3,     3,     6,     3,     1,
       1,     1,     4,     4,     4,     4,     2,     2,     4,     2,
       2,     1,     3,     3,     3,     0,     2,     5,     6,     1,
       2,     1,     4,     3,     0,     1,     3,     2,     3,     1,
       1,     0,     0,     2,     3,     1,     5,     3,     3,     5,
       3,     1,     2,     0,     4,     2,     2,     1,     1,     1,
       1,     4,     6,     1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,     0,     0,   714,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   788,     0,
     776,   599,     0,   605,   606,   607,    21,   663,   764,    97,
     608,     0,    79,     0,     0,     0,     0,     0,     0,     0,
       0,   128,     0,     0,     0,     0,     0,     0,   312,   313,
     314,   317,   316,   315,     0,     0,     0,     0,   151,     0,
       0,     0,   612,   614,   615,   609,   610,     0,     0,   616,
     611,     0,   590,    22,    23,    24,    26,    25,     0,   613,
       0,     0,     0,     0,   617,   318,    27,    28,    30,    29,
      31,    32,    33,    34,    35,    36,    37,    38,    39,   429,
       0,    96,    69,   768,   600,     0,     0,     4,    58,    60,
      63,   662,     0,   589,     0,     6,   127,     7,     8,     9,
       0,     0,   310,   349,     0,     0,     0,     0,     0,     0,
       0,   347,   418,   419,   415,   414,   334,   420,     0,     0,
     333,   730,   591,     0,   665,   413,   309,   733,   348,     0,
       0,   731,   732,   729,   759,   763,     0,   403,   664,    10,
     317,   316,   315,     0,     0,    58,   127,     0,   830,   348,
     829,     0,   827,   826,   417,     0,   340,   344,     0,     0,
     387,   388,   389,   390,   412,   410,   409,   408,   407,   406,
     405,   404,   764,   592,     0,   844,   591,     0,   369,   367,
       0,   792,     0,   672,   332,   595,     0,   844,   594,     0,
     604,   771,   770,   596,     0,     0,   598,   411,     0,     0,
       0,     0,   337,     0,    77,   339,     0,     0,    83,    85,
       0,     0,    87,     0,     0,     0,   868,   869,   873,     0,
       0,    58,   867,     0,   870,     0,     0,    89,     0,     0,
       0,     0,   118,     0,     0,     0,     0,     0,     0,    41,
      46,   234,     0,     0,   233,   153,   152,   239,     0,     0,
       0,     0,     0,   841,   139,   149,   784,   788,   813,     0,
     619,     0,     0,     0,   811,     0,    15,     0,    62,   143,
     150,   496,   440,     0,   835,   718,   349,     0,   347,   348,
       0,     0,   601,     0,   602,     0,     0,     0,   117,     0,
       0,    65,   225,     0,    20,   126,     0,   148,   135,   147,
     315,   127,   311,   108,   109,   110,   111,   112,   114,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   776,     0,   107,   767,   767,   115,
     798,     0,     0,     0,     0,     0,   308,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     368,   366,     0,   734,   719,   767,     0,   725,   225,   767,
       0,   769,   760,   784,     0,   127,     0,     0,    91,     0,
     716,   711,   672,     0,     0,     0,     0,   796,     0,   445,
     671,   787,     0,     0,    65,     0,   225,   331,     0,   772,
     719,   727,   597,     0,    69,   189,     0,   428,     0,    94,
       0,     0,   338,     0,     0,     0,     0,     0,    86,   106,
      88,   865,   866,     0,   863,     0,     0,     0,   840,     0,
     113,    90,   116,     0,     0,     0,     0,     0,     0,     0,
     454,     0,   461,   463,   464,   465,   466,   467,   468,   459,
     481,   482,    69,     0,   103,   105,     0,     0,    43,    50,
       0,     0,    45,    54,    47,     0,    17,     0,     0,   235,
       0,    92,     0,     0,    93,   831,     0,     0,   349,   347,
     348,     0,     0,   159,     0,   785,     0,     0,     0,     0,
     618,   812,   663,     0,     0,   810,   668,   809,    61,     5,
      12,    13,   157,     0,     0,   433,     0,     0,   672,     0,
       0,   593,   434,     0,     0,     0,     0,   674,   717,   877,
     330,   400,   738,    74,    68,    70,    71,    72,    73,     0,
     416,   666,   667,    59,   672,     0,   845,     0,     0,     0,
     674,   226,     0,   423,   129,   155,     0,   372,   374,   373,
       0,     0,   370,   371,   375,   377,   376,   392,   391,   394,
     393,   395,   397,   398,   396,   386,   385,   379,   380,   378,
     381,   382,   384,   399,   383,   766,     0,     0,   802,     0,
     672,   834,     0,   833,   736,   759,   141,   145,   137,   127,
       0,     0,   342,   345,   351,   455,   365,   364,   363,   362,
     361,   360,   359,   358,   357,   356,   355,   354,     0,   721,
     720,     0,     0,     0,     0,     0,     0,     0,   828,   341,
     709,   713,   671,   715,     0,     0,   844,     0,   791,     0,
     790,     0,   775,   774,     0,     0,   721,   720,   335,   191,
     193,    69,   431,   336,     0,    69,   173,    78,   339,     0,
       0,     0,     0,   185,   185,    84,     0,     0,     0,   861,
     672,     0,   851,     0,     0,     0,     0,     0,   670,   608,
       0,     0,   590,     0,     0,    63,   621,   589,   628,     0,
     620,   629,    67,   627,     0,     0,   471,     0,     0,   477,
     474,   475,   483,     0,   462,   457,     0,   460,     0,     0,
       0,    51,    18,     0,     0,    55,    19,     0,     0,     0,
      40,    48,     0,   232,   240,   237,     0,     0,   822,   825,
     824,   823,    11,   855,     0,     0,     0,   784,   781,     0,
     444,   821,   820,   819,     0,   815,     0,   816,   818,     0,
       5,     0,     0,   490,   491,   499,   498,     0,     0,   671,
     439,   443,     0,     0,   836,     0,   852,   718,   212,   876,
       0,     0,   735,   719,   726,   765,   671,   847,   843,   227,
     228,   588,   673,   224,     0,   718,     0,     0,   157,   425,
     131,   402,     0,   448,   449,     0,   446,   671,   797,     0,
       0,   225,   159,   157,   155,     0,   776,   352,     0,     0,
     225,   723,   724,   737,   761,   762,     0,     0,     0,   697,
     679,   680,   681,   682,     0,     0,     0,   690,   689,   703,
     672,     0,   711,   795,   794,     0,   773,   719,   728,   603,
       0,   195,     0,     0,    75,     0,     0,     0,     0,     0,
       0,   165,   166,   177,     0,    69,   175,   100,   185,     0,
     185,     0,     0,   871,     0,     0,   671,   862,   864,   850,
     672,   849,     0,   672,   622,   623,   647,   648,   678,     0,
     672,   670,     0,     0,   442,     0,     0,   804,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   456,     0,     0,     0,   479,   480,   478,
       0,     0,   458,     0,   119,     0,   122,   104,     0,    42,
      52,     0,    44,    56,    49,   236,     0,   832,    95,     0,
       0,   842,   158,   160,   242,     0,     0,   782,     0,   814,
       0,    16,     0,   156,   242,     0,     0,   436,     0,   833,
     837,     0,     0,     0,   877,     0,   216,   214,     0,   721,
     720,   846,     0,     0,   229,    66,     0,   718,   154,     0,
     718,     0,   401,   801,   800,     0,   225,     0,     0,     0,
     157,   133,   604,   722,   225,     0,     0,   685,   686,   687,
     688,   691,   692,   701,     0,   672,   697,     0,   684,   705,
     671,   708,   710,   712,     0,   789,   722,     0,     0,     0,
       0,   192,   432,    80,     0,   339,   167,   784,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   179,     0,   858,
       0,   860,   671,     0,     0,     0,   625,   671,   669,     0,
     660,     0,   672,     0,   630,   661,   659,   808,     0,   672,
     633,   635,   634,     0,     0,   631,   632,   636,   638,   637,
     650,   649,   652,   651,   653,   655,   656,   654,   646,   645,
     640,   641,   639,   642,   643,   644,   469,     0,   470,   476,
     484,   485,     0,    69,    53,    57,   238,   857,   854,     0,
     309,   786,   784,   343,   346,   350,     0,    14,   309,   502,
       0,     0,   504,   497,   500,     0,   495,     0,   838,   853,
     430,     0,   217,     0,   213,     0,     0,   225,   231,   230,
     852,     0,   242,     0,   718,     0,   225,     0,   757,   242,
     242,     0,     0,   353,   225,     0,   751,     0,   694,   671,
     696,     0,   683,     0,     0,   672,   702,   793,     0,    69,
       0,   188,   174,     0,     0,   164,    98,   178,     0,     0,
     181,     0,   186,   187,    69,   180,   872,     0,   848,     0,
     875,   677,   676,   624,     0,   671,   441,   626,     0,   447,
     671,   803,   658,     0,     0,     0,     0,     0,   161,     0,
       0,     0,   307,     0,     0,     0,   140,   241,   243,     0,
     306,     0,   309,     0,   817,   144,   493,     0,     0,   435,
       0,   220,   211,     0,   219,   722,   225,     0,   422,   852,
     309,   852,     0,   799,     0,   756,   309,   309,   242,   718,
       0,   750,   700,   699,   693,     0,   695,   671,   704,    69,
     194,    76,    81,   168,     0,   176,   182,    69,   184,   859,
       0,     0,   438,     0,   807,   806,   657,     0,    69,   123,
     856,     0,     0,     0,     0,   162,   273,   271,   275,   590,
      26,     0,   267,     0,   272,   284,     0,   282,   287,     0,
     286,     0,   285,     0,   127,   245,     0,   247,     0,   783,
     494,   492,   503,   501,   221,     0,   210,   218,   225,     0,
     754,     0,     0,     0,   136,   422,   852,   758,   142,   146,
     309,     0,   752,     0,   707,     0,   190,     0,    69,   171,
      99,   183,   874,   675,     0,     0,     0,     0,     0,     0,
       0,     0,   257,   261,     0,     0,   252,   554,   553,   550,
     552,   551,   571,   573,   572,   542,   513,   514,   532,   548,
     547,   509,   519,   520,   522,   521,   541,   525,   523,   524,
     526,   527,   528,   529,   530,   531,   533,   534,   535,   536,
     537,   538,   540,   539,   510,   511,   512,   515,   516,   518,
     556,   557,   566,   565,   564,   563,   562,   561,   549,   568,
     558,   559,   560,   543,   544,   545,   546,   569,   570,   574,
     576,   575,   577,   578,   555,   580,   579,   582,   584,   583,
     517,   587,   585,   586,   581,   567,   508,   279,   505,     0,
     253,   300,   301,   299,   292,     0,   293,   254,   326,     0,
       0,     0,     0,   127,     0,   223,     0,   753,     0,    69,
     302,    69,   130,     0,     0,   138,   852,   698,     0,    69,
     169,    82,     0,   437,   805,   472,   121,   255,   256,   329,
     163,     0,     0,   276,   268,     0,     0,     0,   281,   283,
       0,     0,   288,   295,   296,   294,     0,     0,   244,     0,
       0,     0,     0,   222,   755,     0,   488,   674,     0,     0,
      69,   132,     0,   706,     0,     0,     0,   101,   258,    58,
       0,   259,   260,     0,     0,   274,   278,   506,   507,     0,
     269,   297,   298,   290,   291,   289,   327,   324,   248,   246,
     328,     0,   489,   673,     0,   424,   303,     0,   134,     0,
     172,   473,     0,   125,     0,   309,   277,   280,     0,   718,
     250,     0,   486,   421,   426,   170,     0,     0,   102,   265,
       0,   308,   325,     0,   674,   320,   718,   487,     0,   124,
       0,     0,   264,   852,   718,   198,   321,   322,   323,   877,
     319,     0,     0,     0,   263,     0,   320,     0,   852,     0,
     262,   304,    69,   249,   877,     0,   202,   200,     0,    69,
       0,     0,   203,     0,   199,   251,     0,   305,     0,   206,
     197,     0,   205,   120,   207,     0,   196,   204,     0,   209,
     208
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   117,   770,   529,   175,   268,   487,
     491,   269,   488,   492,   119,   120,   121,   122,   123,   124,
     310,   554,   555,   441,   233,  1347,   447,  1274,  1563,   730,
     263,   482,  1527,   947,  1113,  1578,   326,   176,   556,   806,
    1001,  1162,   557,   575,   824,   513,   822,   558,   532,   823,
     328,   284,   299,   130,   808,   772,   756,   962,  1294,  1049,
     871,  1481,  1350,   677,   877,   446,   685,   879,  1194,   670,
     861,   864,  1039,  1583,  1584,   546,   547,   569,   570,   273,
     274,   278,  1120,  1227,  1313,  1461,  1569,  1586,  1491,  1531,
    1532,  1533,  1301,  1302,  1303,  1492,  1498,  1540,  1306,  1307,
    1311,  1454,  1455,  1456,  1472,  1613,  1228,  1229,   177,   132,
    1599,  1600,  1459,  1231,   133,   226,   442,   443,   134,   135,
     136,   137,   138,   139,   140,   141,  1332,   142,   805,  1000,
     143,   230,   305,   437,   537,   538,  1072,   539,  1073,   144,
     145,   146,   708,   147,   148,   260,   149,   261,   470,   471,
     472,   473,   474,   475,   476,   477,   478,   720,   721,   939,
     479,   480,   481,   727,  1517,   150,   533,  1321,   534,   975,
     777,  1136,  1133,  1447,  1448,   151,   152,   153,   220,   227,
     313,   427,   154,   711,   898,   713,   155,   899,   797,   788,
     900,   848,  1021,  1023,  1024,  1025,   850,  1174,  1175,   851,
     651,   412,   188,   189,   156,   549,   393,   394,   794,   157,
     221,   179,   159,   160,   161,   162,   163,   164,   165,   606,
     166,   223,   224,   516,   212,   213,   609,   610,  1078,  1079,
     293,   294,   764,   167,   506,   168,   545,   169,   253,   285,
     321,   564,   565,   892,   983,   754,   689,   690,   691,   254,
     255,   790
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1121
static const yytype_int16 yypact[] =
{
   -1121,   120, -1121, -1121,  5394, 13108, 13108,   -52, 13108, 13108,
   13108, 10875, 13108, -1121, 13108, 13108, 13108, 13108, 13108, 13108,
   13108, 13108, 13108, 13108, 13108, 13108, 14542, 14542, 11078, 13108,
   14625,   -49,   -23, -1121, -1121, -1121, -1121, -1121,   169, -1121,
     117, 13108, -1121,   -23,   -17,    89,   162,   -23, 11281, 15621,
   11484, -1121, 13830, 10063,   164, 13108,  3526,    69, -1121, -1121,
   -1121,   249,   260,     4,   190,   196,   202,   209, -1121, 15621,
     212,   224, -1121, -1121, -1121, -1121, -1121,   338,  4657, -1121,
   -1121, 15621, -1121, -1121, -1121, -1121, -1121, -1121, 15621, -1121,
     288,   271, 15621, 15621, -1121, -1121, -1121, -1121, -1121, -1121,
   -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121,
   13108, -1121, -1121,   253,   287,   300,   300, -1121,   434,   326,
     353, -1121,   291, -1121,    32, -1121,   476, -1121, -1121, -1121,
   15361,   461, -1121, -1121,   315,   328,   332,   334,   337,   340,
     157, -1121, -1121, -1121, -1121,   482, -1121,   500,   513,   392,
   -1121,   -29,   355,   438, -1121, -1121,   558,    95,  2213,    88,
     403,    93,    94,   413,   155, -1121,   103, -1121,   547, -1121,
   -1121, -1121,   469,   428,   490, -1121,   476,   461, 15785,  2435,
   15785, 13108, 15785, 15785, 10249,   449, 14907, 15785,   606, 15621,
     587,   587,    90,   587,   587,   587,   587,   587,   587,   587,
     587,   587, -1121, -1121, 14357,   493, -1121,   532,   531,   531,
   14542, 14951,   473,   655, -1121,   469, 14357,   493,   551,   552,
     492,    98, -1121,   563,    88, 11687, -1121, -1121, 13108,  8642,
     684,    52, 15785,  9657, -1121, 13108, 13108, 15621, -1121, -1121,
    4628,   504, -1121,  4706, 13830, 13830,   535, -1121, -1121,   508,
   13784,   692, -1121,   694, -1121, 15621,   632, -1121,   521,  4753,
     525,   541, -1121,   245,  4931, 15444, 15486, 15621,    55, -1121,
     248, -1121, 14408,    57, -1121, -1121, -1121,   711,    58, 14542,
   14542, 13108,   555,   561, -1121, -1121,  3675, 11078,    53,   374,
   -1121, 13311, 14542,   396, -1121, 15621, -1121,   -24,   326, -1121,
   -1121, -1121, 13913,   712,   644,    92,   556, 15785,   557,  1401,
    5597, 13108,   244,   546,   303,   244,   268,   256, -1121, 15621,
   13830,   554, 10266, 13830, -1121, -1121, 14697, -1121, -1121, -1121,
   -1121,   476, -1121, -1121, -1121, -1121, -1121, -1121, -1121, 13108,
   13108, 13108, 11890, 13108, 13108, 13108, 13108, 13108, 13108, 13108,
   13108, 13108, 13108, 13108, 13108, 13108, 13108, 13108, 13108, 13108,
   13108, 13108, 13108, 13108, 14625, 13108, -1121, 13108, 13108, -1121,
   13108,  4730, 15621, 15621, 15361,   645,   630,  9860, 13108, 13108,
   13108, 13108, 13108, 13108, 13108, 13108, 13108, 13108, 13108, 13108,
   -1121, -1121,  3851, -1121,    99, 13108, 13108, -1121, 10266, 13108,
   13108,   253,   100,  3675,   564,   476, 12093,  3547, -1121, 13108,
   -1121,   565,   748, 14357,   566,   219, 13374,   531, 12296, -1121,
   12499, -1121,   567,   222, -1121,   203, 10266, -1121, 13642, -1121,
     106, -1121, -1121, 13697, -1121, -1121, 12702, -1121, 13108, -1121,
     674,  8845,   758,   571, 13093,   756,    82,    38, -1121, -1121,
   -1121, -1121, -1121, 13830, 15286,   576,   767, 14124, -1121,   601,
   -1121, -1121, -1121,   709, 13108,   710,   723, 13108, 13108, 13108,
   -1121,   541, -1121, -1121, -1121, -1121, -1121, -1121, -1121,   618,
   -1121, -1121, -1121,   608, -1121, -1121, 15621,   607,   797,   278,
   15621,   609,   802,   327,   330, 15499, -1121, 15621, 13108,   531,
      69, -1121, 14124,   733, -1121,   531,    86,   113,   616,   617,
    2189,   619, 15621,   690,   622,   531,   114,   623, 15430, 15621,
   -1121, -1121,   757,  1669,   -48, -1121, -1121, -1121,   326, -1121,
   -1121, -1121,   696,   658,   184, -1121,   253,   699,   819,   633,
     682,   100, -1121, 13830, 13830,   817,   642,   828, -1121, 13830,
      97,   772,   198, -1121, -1121, -1121, -1121, -1121, -1121,  1750,
   -1121, -1121, -1121, -1121,   830,   671, -1121, 14542, 13108,   646,
     833, 15785,   831, -1121, -1121,   719, 14741, 10452, 11061, 10249,
   13108, 13296,  2832, 12279,  2488,  1701,  2961,  3464,  3464,  3464,
    3464,  2305,  2305,  2305,  2305,   745,   745,   543,   543,   543,
      90,    90,    90, -1121,   587, 15785,   643,   647, 15007,   649,
     842, -1121, 13108,   -18,   656,   100, -1121, -1121, -1121,   476,
   13108,  3980, -1121, -1121, 10249, -1121, 10249, 10249, 10249, 10249,
   10249, 10249, 10249, 10249, 10249, 10249, 10249, 10249, 13108,   -18,
     657,   652,  1874,   659,   654,  2051,   115,   662, -1121, 15785,
   14312, -1121, 15621, -1121,   556,    97,   493, 14542, 15785, 14542,
   15051,   207,   118, -1121,   663, 13108, -1121, -1121, -1121,  8439,
      70, -1121, 15785, 15785,   -23, -1121, -1121, -1121, 13108,  2915,
   14124, 15621,  9048,   664,   666, -1121,    50,   738,   720, -1121,
     859,   672,  4174, 13830, 14218, 14218, 14124, 14124, 14124, -1121,
     675,    37,   727,   679, 14124,   387, -1121,   735, -1121,   685,
   -1121, -1121, 15871, -1121, 13108,   688, 15785,   701,   871, 10860,
     878, -1121, 15785, 13741, -1121,   618,   809, -1121,  5800, 15347,
     691,   346, -1121, 15444, 15621,   359, -1121, 15486, 15621, 15621,
   -1121, -1121,  2134, -1121, 15871,   875, 14542,   703, -1121, -1121,
   -1121, -1121, -1121,   796,    83, 15347,   695,  3675, 14491,   889,
   -1121, -1121, -1121, -1121,   705, -1121, 13108, -1121, -1121,  4985,
   -1121, 15347,   707, -1121, -1121, -1121, -1121,   890, 13108, 13913,
   -1121, -1121, 15569,   713, -1121, 13830,   879,    15, -1121, -1121,
     266, 14038, -1121,   237, -1121, -1121, 13830, -1121, -1121,   531,
   15785, -1121, 10469, -1121, 14124,    17,   714, 15347,   696, -1121,
   -1121, 12076, 13108, -1121, -1121, 13108, -1121, 13108, -1121,  2373,
     717, 10266,   690,   696,   719, 15621, 14625,   531,  2684,   718,
   10266, -1121, -1121,   259, -1121, -1121,   902, 15227, 15227, 14312,
   -1121, -1121, -1121, -1121,   721,   234,   722, -1121, -1121, -1121,
     910,   724,   565,   531,   531, 12905, -1121,   261, -1121, -1121,
    3179,   324,   -23,  9657, -1121,  6003,   725,  6206,   726, 14542,
     729,   792,   531, 15871,   912, -1121, -1121, -1121, -1121,   529,
   -1121,   230, 13830, -1121, 13830, 15621, 15286, -1121, -1121, -1121,
     918, -1121,   731,   830, -1121, -1121, -1121, -1121, 15151,   728,
     920, 14124,   788, 15621, 13913,  2005, 15582, 14124, 14124, 14124,
   14124, 13977, 14124, 14124, 14124, 14124, 14124, 14124, 14124, 14124,
   14124, 14218, 14218, 14124, 14124, 14124, 14124, 14124, 14124, 14124,
   14124, 14124, 14124, 15785, 13108, 13108, 13108, -1121, -1121, -1121,
   13108, 13108, -1121,   541, -1121,   855, -1121, -1121, 15621, -1121,
   -1121, 15621, -1121, -1121, -1121, -1121, 14124,   531, -1121, 13830,
   15621, -1121,   926, -1121, -1121,   116,   740,   531, 10672, -1121,
      65, -1121,  5191,   926, -1121,   313,   275, 15785,   810, -1121,
   -1121,   739, 13830,   684, 13830,   860,   928,   861, 13108,   -18,
     747, -1121, 14542, 13108, 15785, 15871,   749,    17, -1121,   746,
      17,   752, 12076, 15785, 15107,   753, 10266,   754,   751,   759,
     696, -1121,   492,   763, 10266,   764, 13108, -1121, -1121, -1121,
   -1121, -1121, -1121,   825,   760,   946, 14312,   816, -1121, 13913,
   14312, -1121, -1121, -1121, 14542, 15785, -1121,   -23,   932,   891,
    9657, -1121, -1121, -1121,   768, 13108,   531,  3675,  2915,   770,
   14124,  6409,   545,   771, 13108,    56,   262, -1121,   801, -1121,
     837, -1121,  4415,   941,   775, 14124, -1121, 14124, -1121,   776,
   -1121,   847,   970,   783, -1121, -1121, -1121, 15205,   781,   973,
   11469, 11672, 12482, 14124, 15829, 14041, 14288,  4177,  9651,  4978,
    5184,  5184,  5184,  5184, -1121, -1121,  1093,  1093,   511,   511,
     107,   107,   107, -1121, -1121, -1121, 15785, 12687, 15785, -1121,
   15785, -1121,   789, -1121, -1121, -1121, 15871, -1121,   892, 15347,
     389, -1121,  3675, -1121, -1121, 10249,   785, -1121,   505, -1121,
      24, 13108, -1121, -1121, -1121, 13108, -1121, 13108, -1121, -1121,
   -1121,   296,   975, 14124, -1121,  3272,   791, 10266,   531, 15785,
     879,   793, -1121,   794,    17, 13108, 10266,   795, -1121, -1121,
   -1121,   798,   799, -1121, 10266,   805, -1121, 14312, -1121, 14312,
   -1121,   807, -1121,   865,   808,   985, -1121,   531,   968, -1121,
     811, -1121, -1121,   813,   119, -1121, -1121, 15871,   815,   820,
   -1121,  3932, -1121, -1121, -1121, -1121, -1121, 13830, -1121, 13830,
   -1121, 15871, 15249, -1121, 14124, 13913, -1121, -1121, 14124, -1121,
   14124, -1121, 12888, 14124, 13108,   800,  6612, 13830, -1121,   444,
   13830, 15347, -1121, 15302,   850, 14679, -1121, -1121, -1121,   645,
   13738,    60,   630,   124, -1121, -1121,   869,  3316,  3414, 15785,
     944,   997,   945, 14124, 15871,   829, 10266,   832,   917,   879,
    1003,   879,   834, 15785,   835, -1121,  1082,  1426, -1121,    17,
     836, -1121, -1121,   900, -1121, 14312, -1121, 13913, -1121, -1121,
    8439, -1121, -1121, -1121,  9251, -1121, -1121, -1121,  8439, -1121,
     838, 14124, 15871,   906, 15871, 15303, 12888, 11266, -1121, -1121,
   -1121, 15347, 15347,  1016,    42, -1121, -1121, -1121, -1121,    61,
     839,    62, -1121, 13514, -1121, -1121,    64, -1121, -1121, 14607,
   -1121,   841, -1121,   958,   476, -1121, 13830, -1121,   645, -1121,
   -1121, -1121, -1121, -1121,  1026, 14124, -1121, 15871, 10266,   844,
   -1121,   848,   846,   408, -1121,   917,   879, -1121, -1121, -1121,
    1456,   849, -1121, 14312, -1121,   915,  8439,  9454, -1121, -1121,
   -1121,  8439, -1121, 15871, 14124, 14124, 13108,  6815,   851,   852,
   14124, 15347, -1121, -1121,   448, 15302, -1121, -1121, -1121, -1121,
   -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121,
   -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121,
   -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121,
   -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121,
   -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121,
   -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121,
   -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121, -1121,
   -1121, -1121, -1121, -1121, -1121, -1121, -1121,   517, -1121,   850,
   -1121, -1121, -1121, -1121, -1121,    84,   507, -1121,  1037,    71,
   15621,   958,  1039,   476, 14124, 15871,   857, -1121,   297, -1121,
   -1121, -1121, -1121,   856,   408, -1121,   879, -1121, 14312, -1121,
   -1121, -1121,  7018, 15871, 15871,  2780, -1121, -1121, -1121, 15871,
   -1121,  2865,    48, -1121, -1121, 14124, 13514, 13514,  1001, -1121,
   14607, 14607,   522, -1121, -1121, -1121, 14124,   978, -1121,   862,
      73, 14124, 15621, 15871, -1121,   980, -1121,  1051,  7221,  7424,
   -1121, -1121,   408, -1121,  7627,   863,   984,   956, -1121,   969,
     919, -1121, -1121,   971,   448, -1121, 15871, -1121, -1121,   911,
   -1121,  1036, -1121, -1121, -1121, -1121, 15871,  1057, -1121, -1121,
   15871,   876, -1121,   356,   877, -1121, -1121,  7830, -1121,   882,
   -1121, -1121,   881,   908, 15621,   630, -1121, -1121, 14124,    87,
   -1121,  1002, -1121, -1121, -1121, -1121, 15347,   691, -1121,   921,
   15621,   488, 15871,   887,  1078,   542,    87, -1121,  1006, -1121,
   15347,   895, -1121,   879,    91, -1121, -1121, -1121, -1121, 13830,
   -1121,   893,   898,    76, -1121,   432,   542,   298,   879,   897,
   -1121, -1121, -1121, -1121, 13830,  1009,  1075,  1018,   432, -1121,
    8033,   302,  1085, 14124, -1121, -1121,  8236, -1121,  1022,  1090,
    1028, 14124, 15871, -1121,  1091, 14124, -1121, 15871, 14124, 15871,
   15871
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1121, -1121, -1121,  -490, -1121, -1121, -1121,    -4, -1121, -1121,
   -1121,   614,   377,   386,     0,  1076,  3371, -1121,  2059, -1121,
    -347, -1121,     2, -1121, -1121, -1121, -1121, -1121, -1121, -1121,
   -1121, -1121, -1121,  -466, -1121, -1121,  -169,    40,     6, -1121,
   -1121, -1121,    11, -1121, -1121, -1121, -1121,    12, -1121, -1121,
     755,   774,   779,   974,   307,  -765,   304,   376,  -442, -1121,
     101, -1121, -1121, -1121, -1121, -1121, -1121,  -631,   -37, -1121,
   -1121, -1121, -1121,  -433, -1121,  -774, -1121,  -374, -1121, -1121,
     661, -1121,  -932, -1121, -1121, -1121, -1121, -1121, -1121, -1121,
   -1121, -1121, -1121,  -210, -1121, -1121, -1121, -1121, -1121,  -293,
   -1121,   -63,  -867, -1121, -1115,  -454, -1121,  -154,    31,  -130,
    -440, -1121,  -294, -1121,   -70,   -21,  1127,  -642,  -356, -1121,
   -1121,   -35, -1121, -1121,  3380,   -38,  -166, -1121, -1121, -1121,
   -1121, -1121, -1121,   187,  -751, -1121, -1121, -1121, -1121, -1121,
   -1121, -1121, -1121, -1121, -1121,   803, -1121, -1121,   228, -1121,
     704, -1121, -1121, -1121, -1121, -1121, -1121, -1121,   232, -1121,
     706, -1121, -1121,   451, -1121,   199, -1121, -1121, -1121, -1121,
   -1121, -1121, -1121, -1121,  -860, -1121,  1565,  2877,  -330, -1121,
   -1121,   166,  3194,  -645,  3570, -1121, -1121,   281,  -199,  -565,
   -1121, -1121,   341,  -630,   158, -1121, -1121, -1121, -1121, -1121,
     331, -1121, -1121, -1121,  -242,  -770,  -190,  -185,  -134, -1121,
   -1121,    28, -1121, -1121, -1121, -1121,    -7,  -153, -1121,   130,
   -1121, -1121, -1121,  -384,   899, -1121, -1121, -1121, -1121, -1121,
     553,   400, -1121, -1121,   905,  -290, -1121, -1121,  -320,   -84,
    -188,     3,   496, -1121, -1120, -1121,   305, -1121, -1121, -1121,
    -105,  -957
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -845
static const yytype_int16 yytable[] =
{
     118,   332,   375,   572,   300,   803,   125,   404,   303,   304,
     127,   229,   542,   402,   421,   128,   129,   984,   258,   646,
     849,   622,   234,   222,   643,   397,   238,  1141,   978,   424,
    1248,   996,   158,   429,   603,   131,   868,  -739,   430,   769,
     306,   323,  1128,   999,   126,   241,   329,   332,   251,   894,
     895,  1361,   664,   881,   208,   209,   270,  1534,  1009,   882,
      13,   438,    13,   548,   495,   283,   500,   503,   683,  1316,
    -270,  1365,   308,  1449,  1236,   339,   340,   341,   297,   392,
    1507,   298,  1507,   277,   283,  1361,  1192,   669,   283,   283,
     431,   681,   960,  1500,   342,   746,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,  1501,   364,   518,
       3,   902,   746,   758,   758,   758,   283,   319,   758,  1333,
     365,  1335,    13,   758,   414,   728,    13,    13,   309,   451,
     452,   862,   863,   364,   181,   456,   422,   225,   271,   541,
     320,  -592,   768,  1071,   395,   365,   930,   931,   932,  -740,
    -741,  -742,   576,   395,  -777,  -743,   399,   339,   340,   341,
    -739,   331,  -778,   228,   530,   531,   301,  -427,  -844,   235,
     519,   290,   202,  -452,  -780,   411,   342,   376,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,  -673,
     364,   508,  -673,   653,  -215,   566,  1474,   405,   615,   883,
    1250,   399,   365,  1151,   791,   118,  1153,  1256,  1257,   118,
     324,   435,   415,   445,   774,   440,   647,   684,   417,   615,
    1362,  1363,   574,   509,   423,  1161,   332,  1052,  1535,  1056,
     439,   458,   961,   496,  1193,   501,   504,   158,  1317,  -270,
    1366,   158,  1450,   615,  -746,   489,   493,   494,  1126,  1508,
     272,  1549,   615,   395,  1610,   615,  1094,  1095,  1173,   682,
     972,  1502,   202,   747,  -201,   236,  -215,   396,  -673,   300,
     329,  -748,  -740,  -741,  -742,   528,   396,  -777,  -743,   400,
     499,   985,  -593,  -744,   401,  -778,   118,   505,   505,   510,
     748,   759,   836,  1121,   515,   563,  1273,  -780,  1027,   251,
     524,  1319,   283,   288,   865,  -745,  1340,  -779,   867,  1054,
    1055,  1240,  1515,  1615,   428,   288,   775,  1628,   158,   780,
     525,   131,   623,   654,   497,   986,   392,   288,   686,   392,
     126,   776,   525,   483,   400,   366,  1522,   222,   237,  1521,
     792,  1054,  1055,   262,   614,   793,   288,   613,   283,   283,
     283,   312,   275,   965,   734,  1241,  1516,  1616,  1028,   288,
    1252,  1629,   288,   276,   315,   640,   279,   525,   639,   291,
     292,  1571,   280,  1219,  -749,  1037,  1038,  -746,   281,   319,
    1176,   291,   292,  1183,   113,   282,   396,  1558,   286,   614,
     484,   818,   656,   291,   292,   619,   552,   288,   663,   662,
     287,   667,   289,   738,   666,   820,   739,   987,  1134,   319,
    1057,   515,   291,   292,    13,  1572,  -744,   118,   783,   784,
     301,   415,   948,   676,   789,   291,   292,  1007,   291,   292,
     825,   829,   311,   288,  1283,   951,  1015,  1242,  -745,  1617,
    -779,   562,  1195,  1630,   318,   792,  1129,   302,   820,   158,
     793,   856,   561,  1605,  1135,   288,   857,   319,   319,  1130,
     525,   319,   290,   291,   292,  1341,   731,   322,  1618,   542,
     735,   887,   810,   741,  1220,   270,  1012,   319,   607,  1221,
    -844,    58,    59,    60,   170,   171,   330,  1222,   753,  1219,
     319,   325,  1131,   333,   763,   765,  1345,   424,   520,   291,
     292,   320,    33,    34,    35,   641,   334,   858,  1051,   644,
     335,  1495,   336,   699,  -844,   337,  1503,  1262,   338,  1263,
     526,   291,   292,  1223,  1224,   548,  1225,  1496,  -450,  -844,
      13,  1543,  -844,  1504,   370,   320,  1505,   927,   928,   929,
     930,   931,   932,   548,  1497,    51,   367,    95,  1544,  1291,
    1292,  1545,   283,    58,    59,    60,   170,   171,   330,   368,
      72,    73,    74,    75,    76,   371,  -844,   566,   566,  1226,
     369,   701,   361,   362,   363,   799,   364,    79,    80,   398,
      58,    59,    60,   170,   171,   330,  1470,  1471,   365,  -747,
    1220,    89,  1123,  -451,   542,  1221,  -592,    58,    59,    60,
     170,   171,   330,  1222,   403,    94,   541,  1053,  1054,  1055,
    1611,  1612,  1157,  1541,  1542,  1344,  1537,  1538,   615,    95,
    1165,   295,  1607,  1189,  1054,  1055,   847,   408,   852,   827,
     410,  1031,   365,   866,  1596,  1597,  1598,  1621,   392,  1223,
    1224,   320,  1225,  1184,   420,   118,    95,   314,   316,   317,
      58,    59,    60,    61,    62,   330,   419,   874,   118,   416,
     981,    68,   372,    95,   876,   853,  1592,   854,   426,   521,
     428,   991,   436,   527,  1064,   890,   893,   158,  -591,   425,
     131,  1068,   449,   453,   454,  1235,  -839,   872,   457,   126,
     158,   459,   521,  1477,   527,   521,   527,   527,   373,   460,
     463,   464,   465,   462,   118,   502,   543,   466,   467,   512,
     950,   468,   469,   489,   953,   954,    95,   493,  1233,   542,
     544,  1011,    58,    59,    60,   170,   171,   330,   560,    51,
     -64,   541,   511,   550,   551,   548,   158,   652,   548,   131,
     573,   650,   674,   655,   661,   118,  1216,   438,   126,   678,
     680,   125,   692,  1247,   957,   127,   693,  1058,   979,  1059,
     128,   129,  1254,   714,   990,   515,   967,   989,   715,   717,
    1260,   358,   359,   360,   361,   362,   363,   158,   364,  1585,
     131,  1146,   718,   726,   729,   732,   733,   736,    95,   126,
     365,   737,   745,   749,   750,   755,  1585,   752,   757,   222,
     760,   283,   771,   766,  1606,   773,  1170,   778,   779,   782,
     781,   785,  1270,  1020,  1020,   847,   786,   787,  -453,   796,
     798,  1040,   802,   801,   807,   804,   813,  1278,  1523,   816,
     814,   817,   821,   830,  1117,   831,   833,   834,   809,   118,
     859,   118,   878,   118,   880,  1041,   884,   885,   886,   888,
     934,   901,  1329,  1206,   903,   904,   541,  1139,  1232,   789,
    1211,  1060,   906,   935,   907,   936,  1232,   940,   943,   956,
     946,   158,   959,   158,   964,   158,   131,  1046,   131,  1070,
    1293,   958,  1076,   968,   976,   126,   974,   126,   969,   982,
     997,   980,   548,  1006,  1014,   542,  1016,  1026,  1029,  1030,
    1048,  1032,  1346,  1043,  1045,  1047,  1050,  1062,  1063,  1067,
    1351,  1066,   520,  1124,  1112,  1119,  1122,  1138,  1137,  1142,
    1144,  1357,  1143,  1147,  1114,  1152,  1150,  1115,  1154,  1156,
    1159,  1158,  1554,  1167,  1466,  1169,  1118,   991,  1160,  1164,
    1172,  1166,  1179,  1168,  1180,  1197,  1182,  1186,   118,  1190,
    1196,  1199,  1200,  1203,   125,  1204,  1268,   542,   127,  1205,
    1207,  1209,  1210,   128,   129,  1234,  1215,  1246,  1217,  1243,
    1249,  1251,  1255,  1265,  1267,  1259,  1462,  1258,  1269,  1288,
     158,  1482,  1261,   131,  1264,  1266,  1305,  1219,  1232,  1271,
    1272,  1325,   126,  1275,  1232,  1232,  1178,   548,  1276,  1595,
    1148,  1320,   847,  1324,  1326,  1328,   847,  1331,  1343,  1330,
    1360,  1336,  1337,  1342,  1354,  1352,   118,  1458,  1364,  1457,
    1464,  1467,  1181,  1478,  1468,  1469,  1476,   118,    13,  1487,
    1488,  1506,   541,  1511,  1514,  1520,  1539,  1547,  1548,  1552,
    1553,  1560,  1177,  1561,  1562,  -266,  1564,  1565,   158,  1501,
    1567,  1568,  1570,  1577,  1573,   515,   872,  1576,  1318,   158,
    1575,  1587,   131,  1590,  1593,  1602,  1219,  1594,  1622,  1623,
    1608,   126,  1279,  1604,  1280,  1609,  1619,  1624,  1232,  1631,
     332,  1634,   205,   205,  1635,  1638,   217,  1636,  1220,   740,
     949,  1589,  1290,  1221,   541,    58,    59,    60,   170,   171,
     330,  1222,  1518,   952,  1519,  1315,  1008,    13,   217,   618,
     374,  1010,  1524,  -845,  -845,  -845,  -845,   925,   926,   927,
     928,   929,   930,   931,   932,  1460,   616,   973,  1603,  1185,
     515,  1230,   617,  1601,  1277,  1494,  1499,  1223,  1224,  1230,
    1225,   743,  1312,   847,  1625,   847,  1614,  1510,   231,  1473,
    1140,  1111,  1109,  1557,  1132,   724,   942,   725,  1163,  1022,
     625,    95,  1069,  1033,  1171,   507,   517,  1220,   891,     0,
       0,  1061,  1221,     0,    58,    59,    60,   170,   171,   330,
    1222,     0,     0,  1334,     0,     0,     0,     0,     0,     0,
       0,     0,   118,     0,     0,     0,   251,     0,     0,     0,
       0,  1310,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1223,  1224,     0,  1225,
       0,     0,     0,     0,   158,     0,     0,   131,     0,     0,
       0,     0,     0,     0,     0,     0,   126,     0,     0,     0,
      95,   847,     0,   376,     0,  1620,   118,     0,     0,  1314,
     118,     0,  1626,     0,   118,     0,  1349,     0,     0,     0,
     205,  1230,  1338,     0,     0,     0,   205,  1230,  1230,     0,
       0,     0,   205,     0,  1512,     0,     0,     0,   158,  1446,
       0,   131,   158,     0,     0,  1453,   158,     0,     0,   131,
     126,     0,   251,     0,     0,     0,     0,     0,   126,     0,
     217,   217,     0,     0,     0,     0,   217,   548,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   847,
       0,     0,   118,   118,   548,     0,     0,   118,   205,  1480,
       0,     0,   548,   118,     0,   205,   205,     0,  1463,     0,
       0,     0,   205,     0,     0,     0,     0,     0,   205,     0,
       0,  1230,     0,     0,   158,   158,  1509,   131,   217,   158,
       0,     0,   131,     0,     0,   158,   126,     0,   131,     0,
       0,   126,     0,     0,     0,     0,   217,   126,     0,   217,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1580,     0,     0,     0,   406,   378,   379,   380,   381,
     382,   383,   384,   385,   386,   387,   388,   389,  1551,     0,
    1219,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     217,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   332,     0,     0,     0,     0,   283,     0,     0,     0,
    1219,     0,     0,     0,   390,   391,     0,     0,     0,     0,
       0,    13,     0,     0,   847,     0,     0,     0,   118,   205,
       0,     0,     0,     0,     0,     0,     0,  1529,     0,   205,
       0,     0,  1446,  1446,   789,     0,  1453,  1453,     0,     0,
       0,    13,     0,     0,     0,     0,     0,     0,   283,   789,
     158,     0,     0,   131,   118,   118,     0,     0,     0,     0,
     118,     0,   126,     0,     0,     0,     0,     0,   392,   217,
     217,  1220,     0,   705,     0,     0,  1221,     0,    58,    59,
      60,   170,   171,   330,  1222,     0,   158,   158,     0,   131,
     131,     0,   158,   118,     0,   131,     0,     0,   126,   126,
    1579,  1220,     0,     0,   126,     0,  1221,     0,    58,    59,
      60,   170,   171,   330,  1222,     0,  1591,     0,   705,     0,
    1223,  1224,     0,  1225,     0,   158,     0,     0,   131,     0,
       0,   206,   206,     0,     0,   218,  1581,   126,   552,     0,
       0,     0,     0,     0,    95,     0,     0,     0,     0,     0,
    1223,  1224,     0,  1225,     0,     0,   118,     0,     0,   217,
     217,     0,   118,     0,     0,   217,  1339,     0,     0,     0,
       0,     0,     0,     0,    95,     0,     0,     0,     0,     0,
       0,     0,     0,   205,     0,     0,     0,     0,   158,     0,
       0,   131,     0,     0,   158,     0,  1475,   131,     0,     0,
     126,     0,     0,     0,     0,     0,   126,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   339,
     340,   341,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   205,   342,     0,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,     0,   364,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   205,   365,   205,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,     0,   364,   205,   705,     0,     0,     0,
     339,   340,   341,     0,     0,     0,   365,     0,   217,   217,
     705,   705,   705,   705,   705,   206,     0,     0,     0,   342,
     705,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,     0,   364,     0,   217,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   365,     0,     0,     0,     0,
       0,     0,   205,     0,     0,     0,     0,     0,     0,     0,
       0,   217,     0,   205,   205,     0,     0,   206,     0,     0,
       0,     0,     0,     0,   206,   206,     0,   217,     0,     0,
       0,   206,     0,     0,     0,   217,     0,   206,     0,     0,
       0,   217,     0,     0,     0,     0,     0,   206,     0,   767,
       0,     0,   217,     0,     0,     0,     0,     0,     0,     0,
     705,     0,     0,   217,   339,   340,   341,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   217,   342,     0,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,     0,   364,     0,   218,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   365,
       0,     0,     0,     0,     0,   205,     0,     0,     0,     0,
     795,     0,     0,     0,     0,     0,     0,     0,   217,     0,
     217,     0,   217,     0,     0,     0,     0,     0,   206,     0,
       0,     0,     0,     0,     0,     0,     0,   705,     0,     0,
     217,     0,     0,   705,   705,   705,   705,   705,   705,   705,
     705,   705,   705,   705,   705,   705,   705,   705,   705,   705,
     705,   705,   705,   705,   705,   705,   705,   705,   705,     0,
       0,     0,     0,     0,     0,   908,   909,   910,     0,     0,
       0,     0,   709,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   705,     0,   911,   217,   912,   913,   914,   915,
     916,   917,   918,   919,   920,   921,   922,   923,   924,   925,
     926,   927,   928,   929,   930,   931,   932,     0,   217,     0,
     217,   339,   340,   341,     0,     0,     0,   709,   205,     0,
       0,     0,     0,     0,   832,     0,     0,     0,     0,     0,
     342,     0,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,     0,   364,   217,     0,     0,     0,     0,
     205,   252,     0,     0,     0,     0,   365,     0,     0,     0,
       0,     0,     0,   205,   205,     0,   705,     0,     0,     0,
       0,     0,   206,     0,     0,     0,     0,     0,   217,     0,
       0,   705,     0,   705,   339,   340,   341,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   705,
       0,     0,     0,   342,     0,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   206,   364,     0,     0,
       0,     0,     0,     0,     0,   217,     0,     0,   205,   365,
       0,     0,  1074,   406,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,   389,     0,     0,     0,   705,
       0,     0,   206,     0,   206,     0,     0,   377,   378,   379,
     380,   381,   382,   383,   384,   385,   386,   387,   388,   389,
       0,     0,     0,     0,   206,   709,     0,     0,     0,     0,
       0,   835,   390,   391,     0,     0,     0,     0,     0,   709,
     709,   709,   709,   709,     0,     0,     0,     0,     0,   709,
       0,     0,     0,   217,     0,   217,   390,   391,     0,     0,
     705,   217,     0,     0,   705,     0,   705,     0,     0,   705,
       0,     0,     0,   217,   945,     0,   217,   217,     0,   217,
       0,     0,     0,   252,   252,     0,   217,     0,     0,   252,
       0,   206,     0,     0,     0,     0,   392,     0,     0,   705,
     963,     0,   206,   206,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   955,     0,   963,     0,     0,     0,
     392,     0,     0,   217,   206,  -845,  -845,  -845,  -845,   356,
     357,   358,   359,   360,   361,   362,   363,   705,   364,     0,
       0,     0,     0,     0,     0,     0,     0,   217,   217,   709,
     365,     0,   998,     0,     0,     0,     0,     0,     0,   252,
       0,     0,   252,   339,   340,   341,   751,     0,     0,     0,
       0,   218,   217,     0,     0,     0,     0,     0,     0,     0,
       0,   705,   342,     0,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,     0,   364,     0,     0,     0,
     705,   705,     0,     0,   206,     0,   705,   217,   365,     0,
       0,   217,     0,     0,     0,     0,     0,     0,     0,   406,
     378,   379,   380,   381,   382,   383,   384,   385,   386,   387,
     388,   389,     0,     0,     0,     0,   709,     0,     0,   206,
       0,     0,   709,   709,   709,   709,   709,   709,   709,   709,
     709,   709,   709,   709,   709,   709,   709,   709,   709,   709,
     709,   709,   709,   709,   709,   709,   709,   709,   390,   391,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   252,   688,     0,     0,   707,     0,     0,     0,
       0,   709,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     705,   364,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   365,     0,     0,     0,   206,     0,     0,
       0,   707,   392,     0,     0,     0,     0,   217,     0,     0,
       0,   705,     0,  1005,     0,     0,     0,     0,     0,     0,
       0,     0,   705,     0,     0,     0,     0,   705,     0,     0,
       0,     0,     0,     0,   206,     0,     0,     0,     0,   206,
       0,     0,   252,   252,     0,     0,     0,     0,   252,     0,
       0,     0,   206,   206,     0,   709,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     709,     0,   709,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   705,     0,     0,     0,   709,     0,
       0,     0,   217,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   217,     0,     0,     0,
       0,     0,     0,     0,     0,   217,     0,     0,     0,     0,
       0,     0,     0,     0,  1218,     0,     0,   206,     0,     0,
     217,     0,     0,     0,   339,   340,   341,     0,     0,   705,
       0,     0,     0,     0,     0,     0,     0,   705,   709,     0,
       0,   705,     0,   342,   705,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,     0,   364,     0,   707,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   365,
       0,   252,   252,   707,   707,   707,   707,   707,     0,     0,
       0,     0,     0,   707,     0,     0,     0,     0,     0,   709,
     206,     0,     0,   709,     0,   709,     0,     0,   709,     0,
       0,     0,     0,     0,     0,     0,  1295,     0,  1304,     0,
     339,   340,   341,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   709,   342,
       0,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   206,   364,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   252,   365,   709,     0,     0,     0,
       0,     0,     0,     0,     0,   252,  1358,  1359,     0,     0,
       0,     0,     0,   707,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,  1013,   364,     0,     0,     0,     0,
     709,     0,     0,     0,     0,     0,     0,   365,     0,     0,
       0,     0,     0,   207,   207,     0,     0,   219,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   709,
     709,     0,     0,     0,     0,   709,  1490,     0,     0,     0,
    1304,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   252,    36,   252,     0,   688,     0,     0,     0,     0,
     869,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     707,     0,     0,     0,     0,  1526,   707,   707,   707,   707,
     707,   707,   707,   707,   707,   707,   707,   707,   707,   707,
     707,   707,   707,   707,   707,   707,   707,   707,   707,   707,
     707,   707,    36,     0,   202,     0,     0,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   174,   364,   707,    81,     0,   252,    83,
      84,     0,    85,    86,    87,     0,   365,     0,     0,   709,
       0,     0,   203,     0,     0,     0,     0,     0,     0,     0,
       0,   252,     0,   252,   870,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,     0,     0,
     709,     0,     0,   174,     0,  1528,    81,    82,     0,    83,
      84,   709,    85,    86,    87,     0,   709,     0,     0,     0,
       0,   207,     0,     0,     0,     0,     0,   207,     0,     0,
       0,     0,     0,   207,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,     0,   707,
       0,   204,     0,     0,     0,     0,   113,     0,     0,     0,
       0,   252,     0,     0,   707,     0,   707,     0,     0,     0,
       0,     0,     0,   709,     0,     0,     0,     0,     0,     0,
       0,  1588,   707,     0,     0,     0,     0,     0,     0,   207,
       0,     0,     0,     0,     0,  1295,   207,   207,     0,     0,
       0,     0,     0,   207,     0,     0,     0,     0,     0,   207,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   540,
       0,     0,     0,     0,     0,     0,     0,     0,   709,   339,
     340,   341,     0,     0,     0,     0,   709,     0,     0,     0,
     709,     0,   707,   709,     0,     0,     0,     0,   342,     0,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,     0,   364,     0,     0,     0,     0,     0,     0,     0,
       0,   219,     0,     0,   365,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   252,     0,   252,     0,
       0,     0,     0,   707,     0,     0,     0,   707,     0,   707,
       0,     0,   707,     0,     0,     0,   252,     0,     0,   252,
     207,     0,   339,   340,   341,     0,     0,     0,     0,   252,
     207,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   342,   707,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,     0,   364,   339,   340,   341,     0,
       0,     0,     0,     0,     0,     0,     0,   365,     0,     0,
     707,     0,     0,     0,     0,   342,     0,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,     0,   364,
       0,     0,     0,     0,     0,   252,     0,     0,     0,  1036,
       0,   365,     0,     0,   707,   178,   180,     0,   182,   183,
     184,   186,   187,     0,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,     0,     0,   211,   214,
       0,     0,     0,   707,   707,     0,     0,     0,     0,   707,
       0,   232,     0,     0,   339,   340,   341,     0,   240,     0,
     243,     0,     0,   259,     0,   264,     0,     0,     0,     0,
       0,     0,     0,   342,   207,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,     0,   364,     0,     0,
       0,     0,  1245,     0,     0,     0,     0,     0,     0,   365,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     307,     0,     0,     0,     0,     0,     0,     0,   207,     0,
    -845,  -845,  -845,  -845,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,  1322,   364,     0,     0,
       0,     0,     0,   707,     0,     0,     0,     0,     0,   365,
       0,     0,     0,     0,   207,     0,   207,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1530,     0,     0,     0,   707,     0,   207,   339,   340,   341,
       0,   407,     0,     0,     0,   707,     0,     0,     0,     0,
     707,     0,     0,     0,     0,     0,   342,     0,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,     0,
     364,     0,     0,    36,     0,   433,     0,     0,   433,     0,
       0,     0,   365,     0,  1323,   232,   444,     0,     0,     0,
       0,     0,     0,   207,     0,     0,     0,   707,     0,     0,
     265,   266,     0,     0,   207,   207,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   710,     0,     0,     0,     0,   540,     0,   252,     0,
       0,   307,     0,     0,     0,     0,     0,   211,     0,     0,
       0,   523,     0,   252,     0,     0,     0,   267,     0,     0,
      83,    84,   707,    85,    86,    87,     0,     0,     0,     0,
     707,   559,     0,     0,   707,     0,   710,   707,     0,     0,
       0,     0,   571,   219,     0,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   577,
     578,   579,   581,   582,   583,   584,   585,   586,   587,   588,
     589,   590,   591,   592,   593,   594,   595,   596,   597,   598,
     599,   600,   601,   602,   648,   604,   207,   605,   605,     0,
     608,     0,    36,     0,   202,     0,     0,   624,   626,   627,
     628,   629,   630,   631,   632,   633,   634,   635,   636,   637,
       0,     0,     0,     0,     0,   605,   642,     0,   571,   605,
     645,   540,     0,     0,     0,     0,   624,     0,     0,   649,
       0,     0,   203,     0,     0,     0,     0,     0,   658,     0,
     660,     0,     0,     0,   514,     0,   571,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   672,     0,   673,     0,
       0,     0,     0,   174,     0,     0,    81,    82,   706,    83,
      84,     0,    85,    86,    87,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   716,     0,     0,   719,   722,   723,
       0,     0,     0,     0,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,     0,   207,
       0,   204,     0,   706,   710,     0,   113,     0,   742,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   710,   710,
     710,   710,   710,     0,     0,     0,     0,     0,   710,     0,
       0,     0,     0,     0,     0,     0,   540,     0,     0,     0,
       0,   207,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   207,   207,     0,     0,    36,     0,
     202,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   339,   340,   341,     0,     0,     0,   800,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     811,   342,  1192,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,     0,   364,     0,     0,     0,     0,
       0,     0,   819,     0,     0,     0,     0,   365,   710,   207,
     186,     0,     0,     0,     0,    83,    84,     0,    85,    86,
      87,     0,     0,     0,     0,     0,     0,     0,   828,     0,
       0,     0,     0,     0,     0,     0,     0,   712,     0,     0,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,     0,   860,     0,     0,   826,     0,
     638,   706,   113,     0,     0,     0,     0,    36,   232,   202,
       0,     0,     0,     0,     0,   706,   706,   706,   706,   706,
       0,     0,   744,     0,     0,   706,     0,     0,     0,     0,
       0,     0,   540,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   933,   710,     0,   203,     0,     0,
       0,   710,   710,   710,   710,   710,   710,   710,   710,   710,
     710,   710,   710,   710,   710,   710,   710,   710,   710,   710,
     710,   710,   710,   710,   710,   710,   710,     0,   174,     0,
    1193,    81,    82,     0,    83,    84,     0,    85,    86,    87,
       0,     0,     0,     0,   540,     0,   970,     0,     0,     0,
     710,     0,     0,     0,     0,     0,     0,     0,   977,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,     0,     0,   706,   204,     0,     0,     0,
       0,   113,   994,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1002,     0,     0,  1003,     0,  1004,     0,     0,
       0,   571,     0,   244,     0,     0,     0,     0,     0,     0,
     571,   915,   916,   917,   918,   919,   920,   921,   922,   923,
     924,   925,   926,   927,   928,   929,   930,   931,   932,   245,
       0,     0,     0,     0,     0,  1035,     0,     0,     0,     0,
       0,     0,     0,     0,   710,     0,     0,     0,     0,     0,
     873,    36,     0,     0,     0,     0,     0,     0,     0,   710,
       0,   710,     0,     0,     0,     0,   896,   897,     0,     0,
       0,     0,   706,     0,   905,     0,     0,   710,   706,   706,
     706,   706,   706,   706,   706,   706,   706,   706,   706,   706,
     706,   706,   706,   706,   706,   706,   706,   706,   706,   706,
     706,   706,   706,   706,   246,   247,     0,     0,     0,     0,
       0,     0,     0,     0,  1106,  1107,  1108,     0,     0,     0,
     719,  1110,   174,     0,     0,    81,   248,   706,    83,    84,
       0,    85,    86,    87,     0,   889,     0,   710,     0,     0,
       0,     0,     0,     0,     0,     0,   249,     0,  1125,     0,
       0,     0,     0,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,     0,  1145,     0,
     250,     0,     0,  1149,   995,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   571,     0,     0,     0,
       0,     0,     0,     0,   571,     0,  1125,     0,   710,     0,
       0,     0,   710,     0,   710,     0,     0,   710,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   706,     0,     0,     0,   232,     0,     0,     0,     0,
       0,     0,     0,     0,  1191,     0,   706,   710,   706,     0,
       0,     0,     0,     0,   244,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   706,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     245,     0,     0,     0,     0,   710,     0,  1077,  1080,  1081,
    1082,  1084,  1085,  1086,  1087,  1088,  1089,  1090,  1091,  1092,
    1093,     0,    36,  1096,  1097,  1098,  1099,  1100,  1101,  1102,
    1103,  1104,  1105,     0,     0,     0,     0,     0,     0,     0,
       0,  1237,     0,     0,   706,  1238,     0,  1239,     0,   710,
       0,     0,     0,     0,     0,     0,  1116,   571,     0,     0,
       0,     0,     0,     0,     0,  1253,   571,     0,     0,     0,
       0,     0,     0,     0,   571,   246,   247,     0,   710,   710,
       0,     0,     0,     0,   710,     0,     0,     0,  1493,     0,
       0,     0,     0,   174,     0,     0,    81,   248,     0,    83,
      84,     0,    85,    86,    87,   706,  1198,     0,     0,   706,
       0,   706,     0,     0,   706,     0,     0,   249,     0,     0,
       0,     0,     0,     0,  1287,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,     0,     0,
       0,   250,     0,     0,   706,     0,     0,     0,     0,     0,
    1187,     0,     0,     0,     0,     0,   571,     0,     0,     0,
       0,     0,     0,     0,     0,  1201,     0,  1202,   339,   340,
     341,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   706,  1212,     0,     0,     0,   342,   710,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
       0,   364,     0,     0,     0,     0,     0,     0,     0,   710,
       0,     0,     0,   365,     0,     0,   706,     0,     0,     0,
     710,     0,     0,     0,     0,   710,     0,     0,   571,     0,
       0,     0,     0,  1244,     0,     0,   339,   340,   341,     0,
       0,     0,     0,     0,     0,   706,   706,     0,  1566,     0,
       0,   706,     0,     0,    36,   342,  1485,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,     0,   364,
       0,     0,   710,   339,   340,   341,     0,     0,     0,     0,
       0,   365,     0,     0,  1282,     0,     0,     0,  1284,     0,
    1285,     0,   342,  1286,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,     0,   364,    36,   295,   202,
       0,    83,    84,  1327,    85,    86,    87,   710,   365,     0,
       0,     0,     0,     0,     0,   710,   448,     0,     0,   710,
       0,     0,   710,     0,     0,   706,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
       0,  1353,     0,   611,     0,     0,   296,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   706,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   706,     0,     0,
       0,     0,   706,     0,    83,    84,     0,    85,    86,    87,
       0,     0,     0,     0,     0,  1465,     0,     0,     0,     0,
       0,     0,     0,     0,   450,     0,     0,     0,     0,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,     0,  1483,  1484,     0,     0,     0,   612,
    1489,   113,     0,     0,     0,     0,     0,     0,     0,   706,
       0,   339,   340,   341,     0,     0,     0,     0,     0,     0,
       0,   461,     0,     0,     0,     0,     0,     0,     0,     0,
     342,     0,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,     0,   364,     0,     0,     0,     5,     6,
       7,     8,     9,     0,   706,     0,   365,     0,    10,     0,
       0,     0,   706,     0,     0,     0,   706,     0,     0,   706,
       0,     0,    11,    12,   917,   918,   919,   920,   921,   922,
     923,   924,   925,   926,   927,   928,   929,   930,   931,   932,
      13,    14,    15,     0,  1513,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,  1536,    39,     0,     0,     0,
      40,    41,    42,    43,     0,    44,  1546,    45,     0,    46,
       0,  1550,    47,     0,     0,     0,    48,    49,    50,    51,
      52,    53,    54,     0,    55,    56,    57,    58,    59,    60,
      61,    62,    63,     0,    64,    65,    66,    67,    68,    69,
       0,     0,     0,     0,    70,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,   485,
       0,     0,     0,    78,    79,    80,    81,    82,  1582,    83,
      84,     0,    85,    86,    87,    88,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,     0,
      93,     0,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,   110,     0,   111,   112,   971,   113,   114,     0,   115,
     116,     0,     0,  1632,     5,     6,     7,     8,     9,     0,
       0,  1637,     0,     0,    10,  1639,     0,     0,  1640,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
    -845,  -845,  -845,  -845,   921,   922,   923,   924,   925,   926,
     927,   928,   929,   930,   931,   932,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,     0,     0,     0,    40,    41,    42,    43,
       0,    44,     0,    45,     0,    46,     0,     0,    47,     0,
       0,     0,    48,    49,    50,    51,    52,    53,    54,     0,
      55,    56,    57,    58,    59,    60,    61,    62,    63,     0,
      64,    65,    66,    67,    68,    69,     0,     0,     0,     0,
      70,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,    78,
      79,    80,    81,    82,     0,    83,    84,     0,    85,    86,
      87,    88,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,     0,    93,     0,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,   110,     0,   111,
     112,  1127,   113,   114,     0,   115,   116,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    13,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,     0,     0,     0,    40,
      41,    42,    43,     0,    44,     0,    45,     0,    46,     0,
       0,    47,     0,     0,     0,    48,    49,    50,    51,    52,
      53,    54,     0,    55,    56,    57,    58,    59,    60,    61,
      62,    63,     0,    64,    65,    66,    67,    68,    69,     0,
       0,     0,     0,    70,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,    78,    79,    80,    81,    82,     0,    83,    84,
       0,    85,    86,    87,    88,     0,     0,    89,     0,     0,
      90,     0,     0,     0,     0,     0,    91,    92,     0,    93,
       0,    94,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
     110,     0,   111,   112,     0,   113,   114,     0,   115,   116,
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
      77,     0,     0,     0,     0,   174,    79,    80,    81,    82,
       0,    83,    84,     0,    85,    86,    87,    88,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
       0,     0,     0,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,   553,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
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
       0,     0,     0,    77,     0,     0,     0,     0,   174,    79,
      80,    81,    82,     0,    83,    84,     0,    85,    86,    87,
      88,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,     0,     0,     0,     0,    94,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,   110,     0,   111,   112,
     944,   113,   114,     0,   115,   116,     5,     6,     7,     8,
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
       0,   174,    79,    80,    81,    82,     0,    83,    84,     0,
      85,    86,    87,    88,     0,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,     0,     0,     0,     0,
      94,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,   110,
       0,   111,   112,  1042,   113,   114,     0,   115,   116,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,     0,     0,
       0,    40,    41,    42,    43,  1044,    44,     0,    45,     0,
      46,     0,     0,    47,     0,     0,     0,    48,    49,    50,
      51,     0,    53,    54,     0,    55,     0,    57,    58,    59,
      60,    61,    62,    63,     0,    64,    65,    66,     0,    68,
      69,     0,     0,     0,     0,    70,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   174,    79,    80,    81,    82,     0,
      83,    84,     0,    85,    86,    87,    88,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,     0,
       0,     0,     0,    94,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,   110,     0,   111,   112,     0,   113,   114,     0,
     115,   116,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,    32,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
      39,     0,     0,     0,    40,    41,    42,    43,     0,    44,
       0,    45,     0,    46,  1188,     0,    47,     0,     0,     0,
      48,    49,    50,    51,     0,    53,    54,     0,    55,     0,
      57,    58,    59,    60,    61,    62,    63,     0,    64,    65,
      66,     0,    68,    69,     0,     0,     0,     0,    70,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,   174,    79,    80,
      81,    82,     0,    83,    84,     0,    85,    86,    87,    88,
       0,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,    91,     0,     0,     0,     0,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,   110,     0,   111,   112,     0,
     113,   114,     0,   115,   116,     5,     6,     7,     8,     9,
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
     174,    79,    80,    81,    82,     0,    83,    84,     0,    85,
      86,    87,    88,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,     0,     0,     0,     0,    94,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,   110,     0,
     111,   112,  1289,   113,   114,     0,   115,   116,     5,     6,
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
       0,     0,     0,   174,    79,    80,    81,    82,     0,    83,
      84,     0,    85,    86,    87,    88,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,     0,     0,
       0,     0,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,   110,     0,   111,   112,  1486,   113,   114,     0,   115,
     116,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
       0,     0,     0,    40,    41,    42,    43,     0,    44,     0,
      45,  1525,    46,     0,     0,    47,     0,     0,     0,    48,
      49,    50,    51,     0,    53,    54,     0,    55,     0,    57,
      58,    59,    60,    61,    62,    63,     0,    64,    65,    66,
       0,    68,    69,     0,     0,     0,     0,    70,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   174,    79,    80,    81,
      82,     0,    83,    84,     0,    85,    86,    87,    88,     0,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,     0,     0,     0,     0,    94,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,   110,     0,   111,   112,     0,   113,
     114,     0,   115,   116,     5,     6,     7,     8,     9,     0,
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
       0,     0,     0,     0,    77,     0,     0,     0,     0,   174,
      79,    80,    81,    82,     0,    83,    84,     0,    85,    86,
      87,    88,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,     0,     0,     0,     0,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,   110,     0,   111,
     112,  1555,   113,   114,     0,   115,   116,     5,     6,     7,
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
       0,     0,   174,    79,    80,    81,    82,     0,    83,    84,
       0,    85,    86,    87,    88,     0,     0,    89,     0,     0,
      90,     0,     0,     0,     0,     0,    91,     0,     0,     0,
       0,    94,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
     110,     0,   111,   112,  1556,   113,   114,     0,   115,   116,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,     0,
       0,     0,    40,    41,    42,    43,     0,    44,  1559,    45,
       0,    46,     0,     0,    47,     0,     0,     0,    48,    49,
      50,    51,     0,    53,    54,     0,    55,     0,    57,    58,
      59,    60,    61,    62,    63,     0,    64,    65,    66,     0,
      68,    69,     0,     0,     0,     0,    70,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   174,    79,    80,    81,    82,
       0,    83,    84,     0,    85,    86,    87,    88,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
       0,     0,     0,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
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
       0,     0,     0,    77,     0,     0,     0,     0,   174,    79,
      80,    81,    82,     0,    83,    84,     0,    85,    86,    87,
      88,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,     0,     0,     0,     0,    94,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,   110,     0,   111,   112,
    1574,   113,   114,     0,   115,   116,     5,     6,     7,     8,
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
       0,   174,    79,    80,    81,    82,     0,    83,    84,     0,
      85,    86,    87,    88,     0,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,     0,     0,     0,     0,
      94,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,   110,
       0,   111,   112,  1627,   113,   114,     0,   115,   116,     5,
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
       0,     0,     0,     0,   174,    79,    80,    81,    82,     0,
      83,    84,     0,    85,    86,    87,    88,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,     0,
       0,     0,     0,    94,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,   110,     0,   111,   112,  1633,   113,   114,     0,
     115,   116,     5,     6,     7,     8,     9,     0,     0,     0,
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
       0,     0,    77,     0,     0,     0,     0,   174,    79,    80,
      81,    82,     0,    83,    84,     0,    85,    86,    87,    88,
       0,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,    91,     0,     0,     0,     0,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,   110,     0,   111,   112,     0,
     113,   114,     0,   115,   116,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,   434,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,     0,     0,     0,    40,    41,    42,
      43,     0,    44,     0,    45,     0,    46,     0,     0,    47,
       0,     0,     0,    48,    49,    50,    51,     0,    53,    54,
       0,    55,     0,    57,    58,    59,    60,   170,   171,    63,
       0,    64,    65,    66,     0,     0,     0,     0,     0,     0,
       0,    70,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     174,    79,    80,    81,    82,     0,    83,    84,     0,    85,
      86,    87,     0,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,     0,     0,     0,     0,    94,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,   110,     0,
     111,   112,     0,   113,   114,     0,   115,   116,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,   675,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,     0,     0,     0,
      40,    41,    42,    43,     0,    44,     0,    45,     0,    46,
       0,     0,    47,     0,     0,     0,    48,    49,    50,    51,
       0,    53,    54,     0,    55,     0,    57,    58,    59,    60,
     170,   171,    63,     0,    64,    65,    66,     0,     0,     0,
       0,     0,     0,     0,    70,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   174,    79,    80,    81,    82,     0,    83,
      84,     0,    85,    86,    87,     0,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,     0,     0,
       0,     0,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,   110,     0,   111,   112,     0,   113,   114,     0,   115,
     116,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,   875,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,    32,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,    39,
       0,     0,     0,    40,    41,    42,    43,     0,    44,     0,
      45,     0,    46,     0,     0,    47,     0,     0,     0,    48,
      49,    50,    51,     0,    53,    54,     0,    55,     0,    57,
      58,    59,    60,   170,   171,    63,     0,    64,    65,    66,
       0,     0,     0,     0,     0,     0,     0,    70,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   174,    79,    80,    81,
      82,     0,    83,    84,     0,    85,    86,    87,     0,     0,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,     0,     0,     0,     0,    94,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,   110,     0,   111,   112,     0,   113,
     114,     0,   115,   116,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,  1348,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,     0,     0,     0,    40,    41,    42,    43,
       0,    44,     0,    45,     0,    46,     0,     0,    47,     0,
       0,     0,    48,    49,    50,    51,     0,    53,    54,     0,
      55,     0,    57,    58,    59,    60,   170,   171,    63,     0,
      64,    65,    66,     0,     0,     0,     0,     0,     0,     0,
      70,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   174,
      79,    80,    81,    82,     0,    83,    84,     0,    85,    86,
      87,     0,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,     0,     0,     0,     0,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,   110,     0,   111,
     112,     0,   113,   114,     0,   115,   116,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,  1479,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,    32,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,    39,     0,     0,     0,    40,
      41,    42,    43,     0,    44,     0,    45,     0,    46,     0,
       0,    47,     0,     0,     0,    48,    49,    50,    51,     0,
      53,    54,     0,    55,     0,    57,    58,    59,    60,   170,
     171,    63,     0,    64,    65,    66,     0,     0,     0,     0,
       0,     0,     0,    70,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   174,    79,    80,    81,    82,     0,    83,    84,
       0,    85,    86,    87,     0,     0,     0,    89,     0,     0,
      90,     0,     0,     0,     0,     0,    91,     0,     0,     0,
       0,    94,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
     110,     0,   111,   112,     0,   113,   114,     0,   115,   116,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,   916,   917,   918,   919,
     920,   921,   922,   923,   924,   925,   926,   927,   928,   929,
     930,   931,   932,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,     0,
       0,     0,    40,    41,    42,    43,     0,    44,     0,    45,
       0,    46,     0,     0,    47,     0,     0,     0,    48,    49,
      50,    51,     0,    53,    54,     0,    55,     0,    57,    58,
      59,    60,   170,   171,    63,     0,    64,    65,    66,     0,
       0,     0,     0,     0,     0,     0,    70,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   174,    79,    80,    81,    82,
       0,    83,    84,     0,    85,    86,    87,     0,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
       0,     0,     0,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   620,    12,     0,
       0,     0,     0,     0,     0,   621,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,    40,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    51,     0,     0,     0,     0,     0,
       0,     0,    58,    59,    60,   170,   171,   172,     0,     0,
      65,    66,     0,     0,     0,     0,     0,     0,     0,   173,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   174,    79,
      80,    81,    82,     0,    83,    84,     0,    85,    86,    87,
       0,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,     0,     0,     0,     0,    94,    95,   256,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,   110,     0,     0,     0,
       0,   113,   114,     0,   115,   116,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,    40,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    51,     0,     0,
       0,     0,     0,     0,     0,    58,    59,    60,   170,   171,
     172,     0,     0,    65,    66,     0,     0,     0,     0,     0,
       0,     0,   173,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   174,    79,    80,    81,    82,     0,    83,    84,     0,
      85,    86,    87,     0,     0,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,     0,     0,     0,     0,
      94,    95,   256,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,   110,
       0,   257,     0,     0,   113,   114,     0,   115,   116,     5,
       6,     7,     8,     9,     0,     0,     0,     0,   342,    10,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   567,   364,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,   365,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,    40,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      51,     0,     0,     0,     0,     0,     0,     0,    58,    59,
      60,   170,   171,   172,     0,     0,    65,    66,     0,     0,
       0,     0,     0,     0,     0,   173,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   174,    79,    80,    81,    82,     0,
      83,    84,     0,    85,    86,    87,     0,   568,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,     0,
       0,     0,     0,    94,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,   110,   340,   341,     0,     0,   113,   114,     0,
     115,   116,     5,     6,     7,     8,     9,     0,     0,     0,
       0,   342,    10,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   992,   364,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,   365,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,    40,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    51,     0,     0,     0,     0,     0,     0,
       0,    58,    59,    60,   170,   171,   172,     0,     0,    65,
      66,     0,     0,     0,     0,     0,     0,     0,   173,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,   174,    79,    80,
      81,    82,     0,    83,    84,     0,    85,    86,    87,     0,
     993,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,    91,     0,     0,     0,     0,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,   110,     0,     0,     0,     0,
     113,   114,     0,   115,   116,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   620,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,    40,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    51,     0,     0,     0,
       0,     0,     0,     0,    58,    59,    60,   170,   171,   172,
       0,     0,    65,    66,     0,     0,     0,     0,     0,     0,
       0,   173,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     174,    79,    80,    81,    82,     0,    83,    84,     0,    85,
      86,    87,     0,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,     0,     0,     0,     0,    94,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,   110,     0,
     339,   340,   341,   113,   114,     0,   115,   116,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   342,
       0,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,     0,   364,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,   365,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
      40,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   185,     0,     0,    51,
       0,     0,     0,     0,     0,     0,     0,    58,    59,    60,
     170,   171,   172,     0,     0,    65,    66,     0,     0,     0,
       0,     0,     0,     0,   173,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   174,    79,    80,    81,    82,     0,    83,
      84,     0,    85,    86,    87,     0,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,   937,   938,
       0,     0,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,   110,     0,   341,     0,     0,   113,   114,     0,   115,
     116,     5,     6,     7,     8,     9,     0,     0,     0,     0,
     342,    10,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   210,   364,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    14,    15,   365,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,    40,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    51,     0,     0,     0,     0,     0,     0,     0,
      58,    59,    60,   170,   171,   172,     0,     0,    65,    66,
       0,     0,     0,     0,     0,     0,     0,   173,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   174,    79,    80,    81,
      82,     0,    83,    84,     0,    85,    86,    87,     0,     0,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,     0,     0,     0,     0,    94,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,   110,     0,   339,   340,   341,   113,
     114,     0,   115,   116,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   342,     0,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,     0,   364,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,   365,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,    40,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    51,     0,     0,     0,     0,
       0,     0,     0,    58,    59,    60,   170,   171,   172,     0,
       0,    65,    66,     0,     0,     0,     0,     0,     0,     0,
     173,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   174,
      79,    80,    81,    82,     0,    83,    84,     0,    85,    86,
      87,     0,     0,     0,    89,     0,     0,    90,     0,     0,
    1356,     0,     0,    91,     0,     0,     0,     0,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,   110,     0,   239,
     909,   910,   113,   114,     0,   115,   116,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   911,     0,
     912,   913,   914,   915,   916,   917,   918,   919,   920,   921,
     922,   923,   924,   925,   926,   927,   928,   929,   930,   931,
     932,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,    40,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    51,     0,
       0,     0,     0,     0,     0,     0,    58,    59,    60,   170,
     171,   172,     0,     0,    65,    66,     0,     0,     0,     0,
       0,     0,     0,   173,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   174,    79,    80,    81,    82,     0,    83,    84,
       0,    85,    86,    87,     0,     0,     0,    89,     0,     0,
      90,     0,     0,     0,     0,     0,    91,     0,     0,     0,
       0,    94,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
     110,     0,   242,     0,   910,   113,   114,     0,   115,   116,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   911,     0,   912,   913,   914,   915,   916,   917,   918,
     919,   920,   921,   922,   923,   924,   925,   926,   927,   928,
     929,   930,   931,   932,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,    40,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    51,     0,     0,     0,     0,     0,     0,     0,    58,
      59,    60,   170,   171,   172,     0,     0,    65,    66,     0,
       0,     0,     0,     0,     0,     0,   173,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   174,    79,    80,    81,    82,
       0,    83,    84,     0,    85,    86,    87,     0,     0,     0,
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
       0,     0,     0,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,   432,     0,     0,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     580,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,    40,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    51,     0,     0,     0,     0,     0,
       0,     0,    58,    59,    60,   170,   171,   172,     0,     0,
      65,    66,     0,     0,     0,     0,     0,     0,     0,   173,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   174,    79,
      80,    81,    82,     0,    83,    84,     0,    85,    86,    87,
       0,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,     0,     0,     0,     0,    94,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,   110,     0,     0,     0,
       0,   113,   114,     0,   115,   116,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   621,   364,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,   365,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,    40,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    51,     0,     0,
       0,     0,     0,     0,     0,    58,    59,    60,   170,   171,
     172,     0,     0,    65,    66,     0,     0,     0,     0,     0,
       0,     0,   173,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   174,    79,    80,    81,    82,     0,    83,    84,     0,
      85,    86,    87,     0,     0,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,     0,     0,     0,     0,
      94,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,   110,
       0,     0,     0,     0,   113,   114,     0,   115,   116,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   657,   364,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,   365,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,    40,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      51,     0,     0,     0,     0,     0,     0,     0,    58,    59,
      60,   170,   171,   172,     0,     0,    65,    66,     0,     0,
       0,     0,     0,     0,     0,   173,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   174,    79,    80,    81,    82,     0,
      83,    84,     0,    85,    86,    87,     0,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,     0,
       0,     0,     0,    94,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,   110,     0,     0,     0,     0,   113,   114,     0,
     115,   116,     5,     6,     7,     8,     9,     0,     0,     0,
       0,   911,    10,   912,   913,   914,   915,   916,   917,   918,
     919,   920,   921,   922,   923,   924,   925,   926,   927,   928,
     929,   930,   931,   932,   659,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,    40,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    51,     0,     0,     0,     0,     0,     0,
       0,    58,    59,    60,   170,   171,   172,     0,     0,    65,
      66,     0,     0,     0,     0,     0,     0,     0,   173,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,   174,    79,    80,
      81,    82,     0,    83,    84,     0,    85,    86,    87,     0,
       0,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,    91,     0,     0,     0,     0,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,   110,     0,   339,   340,   341,
     113,   114,     0,   115,   116,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   342,     0,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,     0,
     364,     0,     0,     0,     0,     0,     0,     0,    14,    15,
       0,     0,   365,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,    40,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    51,     0,     0,     0,
       0,     0,     0,     0,    58,    59,    60,   170,   171,   172,
       0,     0,    65,    66,     0,     0,     0,     0,     0,     0,
       0,   173,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     174,    79,    80,    81,    82,     0,    83,    84,     0,    85,
      86,    87,     0,     0,     0,    89,     0,     0,    90,     0,
    1214,     0,     0,     0,    91,     0,     0,     0,     0,    94,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,   110,     0,
       0,   671,     0,   113,   114,     0,   115,   116,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   912,
     913,   914,   915,   916,   917,   918,   919,   920,   921,   922,
     923,   924,   925,   926,   927,   928,   929,   930,   931,   932,
    1034,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,     0,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,     0,     0,     0,     0,
      40,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    51,
       0,     0,     0,     0,     0,     0,     0,    58,    59,    60,
     170,   171,   172,     0,     0,    65,    66,     0,     0,     0,
       0,     0,     0,     0,   173,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   174,    79,    80,    81,    82,     0,    83,
      84,     0,    85,    86,    87,     0,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,     0,     0,
       0,     0,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,   110,     0,   339,   340,   341,   113,   114,     0,   115,
     116,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   342,     0,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,     0,   364,     0,     0,     0,
       0,     0,     0,     0,    14,    15,     0,     0,   365,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,   679,
       0,     0,     0,    40,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    51,     0,     0,     0,     0,     0,     0,     0,
      58,    59,    60,   170,   171,   172,     0,     0,    65,    66,
       0,     0,     0,     0,     0,     0,     0,   173,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   174,    79,    80,    81,
      82,     0,    83,    84,     0,    85,    86,    87,     0,     0,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,     0,     0,     0,     0,    94,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,   110,     0,   339,   340,   341,   113,
     114,     0,   115,   116,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   342,   812,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,     0,   364,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,   365,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,   522,
      38,     0,     0,     0,     0,     0,    40,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    51,     0,     0,     0,     0,
       0,     0,     0,    58,    59,    60,   170,   171,   172,     0,
       0,    65,    66,     0,     0,     0,     0,     0,     0,     0,
     173,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,    36,     0,   202,    77,     0,     0,     0,     0,   174,
      79,    80,    81,    82,     0,    83,    84,     0,    85,    86,
      87,     0,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,     0,     0,     0,     0,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,   110,     0,     0,
       0,     0,   113,   114,     0,   115,   116,  1367,  1368,  1369,
    1370,  1371,     0,     0,  1372,  1373,  1374,  1375,    83,    84,
       0,    85,    86,    87,     0,     0,     0,     0,     0,     0,
       0,  1376,  1377,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,  1378,     0,     0,
       0,     0,     0,   612,     0,   113,     0,     0,     0,     0,
       0,  1379,  1380,  1381,  1382,  1383,  1384,  1385,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
    1386,  1387,  1388,  1389,  1390,  1391,  1392,  1393,  1394,  1395,
    1396,  1397,  1398,  1399,  1400,  1401,  1402,  1403,  1404,  1405,
    1406,  1407,  1408,  1409,  1410,  1411,  1412,  1413,  1414,  1415,
    1416,  1417,  1418,  1419,  1420,  1421,  1422,  1423,  1424,  1425,
    1426,     0,     0,  1427,  1428,     0,  1429,  1430,  1431,  1432,
    1433,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1434,  1435,  1436,     0,     0,     0,    83,    84,
       0,    85,    86,    87,  1437,     0,  1438,  1439,     0,  1440,
       0,     0,     0,     0,     0,     0,  1441,  1442,     0,  1443,
       0,  1444,  1445,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   339,   340,   341,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    36,
       0,   202,     0,     0,     0,     0,   342,     0,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,     0,
     364,   339,   340,   341,     0,     0,     0,     0,     0,     0,
       0,     0,   365,     0,     0,     0,     0,   244,     0,     0,
     342,     0,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   245,   364,     0,    83,    84,     0,    85,
      86,    87,     0,     0,     0,     0,   365,     0,     0,     0,
       0,     0,     0,   244,     0,    36,     0,     0,     0,     0,
       0,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,     0,     0,     0,     0,   245,
       0,   665,  -308,   113,     0,     0,     0,     0,     0,     0,
      58,    59,    60,   170,   171,   330,     0,     0,     0,   244,
       0,    36,     0,     0,     0,     0,     0,     0,   246,   247,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   245,   174,     0,   455,    81,
     248,     0,    83,    84,   668,    85,    86,    87,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    36,     0,     0,
     249,     0,     0,     0,   246,   247,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,     0,   174,   941,   250,    81,   248,     0,    83,    84,
       0,    85,    86,    87,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   249,     0,     0,     0,
     246,   247,     0,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,     0,   174,     0,
     250,    81,   248,     0,    83,    84,     0,    85,    86,    87,
      36,     0,   202,     0,     0,     0,     0,     0,   535,     0,
       0,     0,   249,     0,     0,     0,     0,  1083,     0,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   694,   695,     0,   250,     0,     0,   696,
     203,   697,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   698,     0,     0,     0,     0,     0,     0,
       0,    33,    34,    35,    36,     0,     0,     0,     0,     0,
       0,   174,   699,     0,    81,    82,     0,    83,    84,     0,
      85,    86,    87,   913,   914,   915,   916,   917,   918,   919,
     920,   921,   922,   923,   924,   925,   926,   927,   928,   929,
     930,   931,   932,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,     0,   700,     0,    72,
      73,    74,    75,    76,   536,    36,     0,   202,     0,     0,
     701,     0,     0,     0,     0,   174,    79,    80,    81,   702,
       0,    83,    84,     0,    85,    86,    87,     0,     0,     0,
      89,     0,     0,     0,     0,     0,     0,     0,     0,   703,
       0,     0,     0,     0,    94,     0,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     694,   695,     0,   704,     0,     0,   696,     0,   697,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     698,     0,    83,    84,     0,    85,    86,    87,    33,    34,
      35,    36,     0,     0,     0,     0,     0,     0,     0,   699,
       0,     0,     0,     0,     0,     0,     0,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,     0,     0,     0,     0,     0,     0,   988,     0,   113,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   700,     0,    72,    73,    74,    75,
      76,     0,     0,     0,   694,   695,     0,   701,     0,     0,
       0,     0,   174,    79,    80,    81,   702,     0,    83,    84,
       0,    85,    86,    87,   698,     0,     0,    89,     0,     0,
       0,     0,    33,    34,    35,    36,   703,     0,     0,     0,
       0,    94,     0,   699,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,     0,     0,     0,
     704,   914,   915,   916,   917,   918,   919,   920,   921,   922,
     923,   924,   925,   926,   927,   928,   929,   930,   931,   932,
       0,     0,     0,     0,     0,     0,     0,     0,   700,     0,
      72,    73,    74,    75,    76,     0,     0,     0,   837,   838,
       0,   701,     0,     0,     0,     0,   174,    79,    80,    81,
     702,     0,    83,    84,     0,    85,    86,    87,   839,     0,
       0,    89,     0,     0,     0,     0,   840,   841,   842,    36,
     703,     0,     0,     0,     0,    94,     0,   843,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    29,    30,     0,     0,     0,     0,
       0,     0,     0,     0,    36,     0,   202,     0,     0,     0,
       0,     0,   844,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   845,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    83,    84,     0,    85,
      86,    87,     0,     0,   203,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   846,    36,     0,   202,     0,     0,
       0,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   174,     0,     0,    81,    82,
       0,    83,    84,     0,    85,    86,    87,     0,     0,     0,
       0,     0,     0,    90,     0,   203,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
       0,     0,     0,   413,     0,     0,   174,     0,   113,    81,
      82,     0,    83,    84,     0,    85,    86,    87,    36,     0,
     202,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,     0,     0,     0,   204,     0,     0,   498,   203,   113,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    36,
     966,   202,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   174,
       0,     0,    81,    82,     0,    83,    84,     0,    85,    86,
      87,     0,     0,     0,     0,     0,     0,     0,     0,   203,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,    36,     0,     0,   204,     0,     0,
     174,     0,   113,    81,    82,     0,    83,    84,     0,    85,
      86,    87,    36,     0,   202,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,     0,     0,     0,   204,     0,
       0,     0,   215,   113,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,     0,  1451,
       0,    83,    84,  1452,    85,    86,    87,     0,     0,     0,
       0,     0,     0,   174,    36,     0,    81,    82,     0,    83,
      84,     0,    85,    86,    87,     0,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
    1308,     0,     0,  1309,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,    36,     0,
       0,   216,     0,     0,     0,     0,   113,     0,     0,     0,
       0,     0,     0,    83,    84,     0,    85,    86,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    83,    84,     0,    85,    86,    87,     0,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,     0,     0,     0,  1309,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
       0,     0,     0,   573,     0,    83,    84,     0,    85,    86,
      87,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   339,   340,   341,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,     0,     0,   342,   809,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,     0,
     364,   339,   340,   341,     0,     0,     0,     0,     0,     0,
       0,     0,   365,     0,     0,     0,     0,     0,     0,     0,
     342,     0,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,     0,   364,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   365,   339,   340,   341,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   409,   342,     0,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,     0,
     364,   339,   340,   341,     0,     0,     0,     0,     0,     0,
       0,     0,   365,     0,     0,     0,     0,     0,     0,   418,
     342,     0,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,     0,   364,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   365,   339,   340,   341,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   815,   342,     0,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,     0,
     364,   908,   909,   910,     0,     0,     0,     0,     0,     0,
       0,     0,   365,     0,     0,     0,     0,     0,     0,   855,
     911,     0,   912,   913,   914,   915,   916,   917,   918,   919,
     920,   921,   922,   923,   924,   925,   926,   927,   928,   929,
     930,   931,   932,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   908,   909,   910,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   911,  1155,   912,   913,   914,   915,
     916,   917,   918,   919,   920,   921,   922,   923,   924,   925,
     926,   927,   928,   929,   930,   931,   932,     0,     0,   908,
     909,   910,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   911,  1065,
     912,   913,   914,   915,   916,   917,   918,   919,   920,   921,
     922,   923,   924,   925,   926,   927,   928,   929,   930,   931,
     932,  1017,  1018,  1019,    36,     0,     0,     0,     0,     0,
       0,     0,     0,   908,   909,   910,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   911,  1208,   912,   913,   914,   915,   916,   917,
     918,   919,   920,   921,   922,   923,   924,   925,   926,   927,
     928,   929,   930,   931,   932,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,   687,     0,     0,     0,     0,     0,  1281,     0,    36,
       0,    83,    84,     0,    85,    86,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
    1296,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,  1355,  1297,  1298,   174,     0,     0,    81,    36,     0,
      83,    84,     0,    85,    86,    87,     0,     0,     0,     0,
     174,     0,     0,    81,  1299,     0,    83,    84,     0,    85,
    1300,    87,     0,     0,     0,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,     0,
       0,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   174,     0,     0,    81,    82,
       0,    83,    84,     0,    85,    86,    87,    36,     0,   761,
     762,     0,     0,   327,     0,    83,    84,     0,    85,    86,
      87,    36,     0,     0,     0,     0,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,     0,    83,    84,     0,    85,    86,    87,
       0,     0,     0,     0,     0,   486,     0,     0,    83,    84,
       0,    85,    86,    87,     0,     0,     0,     0,     0,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   490,     0,     0,
      83,    84,     0,    85,    86,    87,    36,     0,     0,     0,
     267,     0,     0,    83,    84,     0,    85,    86,    87,    36,
       0,     0,     0,     0,     0,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   611,     0,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,  1075,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    83,    84,     0,    85,    86,    87,     0,
       0,     0,     0,     0,     0,     0,    83,    84,     0,    85,
      86,    87,     0,     0,     0,     0,     0,     0,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,    83,    84,     0,    85,    86,
      87,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   339,   340,   341,     0,     0,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   342,     0,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,     0,   364,   908,
     909,   910,     0,     0,     0,     0,     0,     0,     0,     0,
     365,     0,     0,     0,     0,     0,     0,     0,   911,  1213,
     912,   913,   914,   915,   916,   917,   918,   919,   920,   921,
     922,   923,   924,   925,   926,   927,   928,   929,   930,   931,
     932,   908,   909,   910,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     911,     0,   912,   913,   914,   915,   916,   917,   918,   919,
     920,   921,   922,   923,   924,   925,   926,   927,   928,   929,
     930,   931,   932
};

static const yytype_int16 yycheck[] =
{
       4,   131,   156,   323,    88,   570,     4,   176,    92,    93,
       4,    32,   302,   166,   213,     4,     4,   787,    53,   403,
     650,   377,    43,    30,   398,   159,    47,   984,   779,   217,
    1150,   805,     4,   223,   364,     4,   678,    66,   223,   529,
     110,     9,   974,   808,     4,    49,   130,   177,    52,   694,
     695,     9,   426,   684,    26,    27,    56,     9,   823,     9,
      45,     9,    45,   305,     9,    69,     9,     9,    30,     9,
       9,     9,   110,     9,    50,    10,    11,    12,    78,   127,
       9,    81,     9,    79,    88,     9,    30,   434,    92,    93,
     224,     9,     9,     9,    29,     9,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    33,    53,    66,
       0,    84,     9,     9,     9,     9,   130,   151,     9,  1249,
      65,  1251,    45,     9,   204,   482,    45,    45,   110,   244,
     245,    71,    72,    53,   196,   250,   216,   196,    79,   302,
     168,   147,   200,   904,    66,    65,    49,    50,    51,    66,
      66,    66,   331,    66,    66,    66,    66,    10,    11,    12,
     199,   131,    66,   196,   198,   199,   152,     8,   196,   196,
     127,   144,    79,    66,    66,   189,    29,   156,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,   194,
      53,   281,   197,   412,   197,   320,  1336,   177,   371,   169,
    1152,    66,    65,   997,   127,   229,  1000,  1159,  1160,   233,
     198,   229,   204,   237,    50,   233,   405,   199,   210,   392,
     198,   199,   326,   281,   216,  1010,   376,   878,   200,   880,
     198,   255,   169,   198,   198,   198,   198,   229,   198,   198,
     198,   233,   198,   416,    66,   265,   266,   267,   203,   198,
     201,   198,   425,    66,   198,   428,   921,   922,  1029,   197,
     770,   197,    79,   197,   197,   196,   194,   199,   197,   373,
     374,   196,   199,   199,   199,   295,   199,   199,   199,   199,
     272,    35,   147,    66,   201,   199,   310,   279,   280,   281,
     197,   197,   197,   197,   286,   319,   197,   199,    84,   323,
     292,   197,   326,    79,   671,    66,  1258,    66,   675,    99,
     100,    35,    35,    35,   127,    79,   152,    35,   310,   538,
      84,   310,   377,   413,    96,    79,   127,    79,   453,   127,
     310,   167,    84,   108,   199,   198,  1476,   364,   196,  1474,
     550,    99,   100,   199,   371,   550,    79,   371,   372,   373,
     374,    84,   123,   757,    96,    79,    79,    79,   144,    79,
    1154,    79,    79,   123,    84,   392,   196,    84,   392,   145,
     146,    35,   196,     4,   196,    71,    72,   199,   196,   151,
    1030,   145,   146,  1045,   201,   196,   199,  1522,   196,   416,
     165,   610,   416,   145,   146,   375,   197,    79,   425,   197,
     196,   428,    84,    96,   428,   613,    96,   161,   153,   151,
     200,   403,   145,   146,    45,    79,   199,   441,   543,   544,
     152,   413,    96,   441,   549,   145,   146,   821,   145,   146,
     619,   639,   199,    79,  1205,    96,   830,   161,   199,   161,
     199,   205,   200,   161,    30,   655,   153,   196,   656,   441,
     655,   661,   204,  1593,   199,    79,   661,   151,   151,   166,
      84,   151,   144,   145,   146,  1259,   486,   196,  1608,   779,
     490,   690,   576,   497,   105,   495,   826,   151,   368,   110,
     147,   112,   113,   114,   115,   116,   117,   118,   512,     4,
     151,    35,   199,   198,   518,   519,  1267,   705,   144,   145,
     146,   168,    74,    75,    76,   395,   198,   661,   875,   399,
     198,    14,   198,    85,   147,   198,    29,  1167,   198,  1169,
     144,   145,   146,   154,   155,   787,   157,    30,    66,   196,
      45,    29,   199,    46,   199,   168,    49,    46,    47,    48,
      49,    50,    51,   805,    47,   104,    66,   178,    46,   125,
     126,    49,   576,   112,   113,   114,   115,   116,   117,    66,
     132,   133,   134,   135,   136,   147,   199,   692,   693,   200,
     198,   143,    49,    50,    51,   567,    53,   149,   150,   196,
     112,   113,   114,   115,   116,   117,   198,   199,    65,   196,
     105,   163,   968,    66,   904,   110,   147,   112,   113,   114,
     115,   116,   117,   118,   196,   177,   779,    98,    99,   100,
     198,   199,  1006,  1500,  1501,  1265,  1496,  1497,   791,   178,
    1014,   151,  1599,    98,    99,   100,   650,   198,   652,   621,
      44,   850,    65,   674,   112,   113,   114,  1614,   127,   154,
     155,   168,   157,  1047,     9,   669,   178,   114,   115,   116,
     112,   113,   114,   115,   116,   117,   203,   681,   682,   147,
     785,   123,   124,   178,   682,   657,   198,   659,   196,   289,
     127,   796,     8,   293,   893,   692,   693,   669,   147,   147,
     669,   900,   198,   168,   196,   200,    14,   679,    14,   669,
     682,    79,   312,  1343,   314,   315,   316,   317,   160,   198,
     179,   180,   181,   198,   728,    14,    14,   186,   187,   168,
     734,   190,   191,   733,   738,   739,   178,   737,  1122,  1029,
      96,   825,   112,   113,   114,   115,   116,   117,   202,   104,
     196,   904,   197,   197,   197,   997,   728,     9,  1000,   728,
     196,   196,    88,   197,   197,   769,  1113,     9,   728,   198,
      14,   769,   196,  1147,   746,   769,     9,   882,   782,   884,
     769,   769,  1156,   182,   791,   757,   758,   791,    79,    79,
    1164,    46,    47,    48,    49,    50,    51,   769,    53,  1569,
     769,   989,    79,   185,   196,   198,     9,   198,   178,   769,
      65,     9,    79,   197,   197,   125,  1586,   198,   196,   826,
     197,   825,   126,    66,  1594,   167,  1025,   128,     9,   147,
     197,    14,  1179,   837,   838,   839,   194,     9,    66,     9,
     169,   862,     9,   197,   125,    14,   203,  1194,  1478,   200,
     203,     9,   196,   196,   959,   203,   197,   203,   196,   863,
     197,   865,   198,   867,   198,   863,   128,   147,     9,   197,
     182,   196,  1246,  1072,   147,   196,  1029,   982,  1120,   984,
    1079,   885,   147,   182,   199,    14,  1128,     9,    79,    14,
     199,   863,    96,   865,   199,   867,   865,   869,   867,   903,
    1220,   198,   906,    14,    14,   865,   199,   867,   203,    30,
     196,   198,  1154,   196,   196,  1205,    14,   196,   196,     9,
     128,   197,  1269,   198,   198,   196,    14,     9,   197,     9,
    1277,   203,   144,   968,    79,     9,   196,   198,   128,    79,
      79,  1288,    14,   196,   948,   199,   197,   951,   196,   196,
     199,   197,  1517,   128,  1328,     9,   960,  1062,   199,   196,
     144,   197,    30,   203,    73,   128,   198,   197,   972,   198,
     169,    30,   197,   197,   972,   128,  1175,  1267,   972,     9,
     197,   200,     9,   972,   972,   200,   197,   196,    96,    14,
     197,   197,   197,   128,     9,   196,  1316,   199,    30,   199,
     972,  1348,   197,   972,   197,   197,   156,     4,  1250,   198,
     197,    14,   972,   198,  1256,  1257,  1037,  1259,   198,  1584,
     992,   152,  1026,    79,    79,   196,  1030,   110,   128,   197,
      14,   197,   197,   197,   128,   197,  1040,    79,   199,   198,
      14,   197,  1040,   128,   196,   199,   197,  1051,    45,   198,
     198,    14,  1205,    14,   197,   199,    55,    79,   196,    79,
       9,   198,  1034,    79,   108,    96,   147,    96,  1040,    33,
     159,    14,   196,   165,   197,  1047,  1048,   196,  1232,  1051,
     198,    79,  1051,   162,   197,    79,     4,     9,    79,    14,
     197,  1051,  1197,   198,  1199,   197,   199,    79,  1340,    14,
    1230,    79,    26,    27,    14,    14,    30,    79,   105,   495,
     733,  1577,  1217,   110,  1267,   112,   113,   114,   115,   116,
     117,   118,  1469,   737,  1471,  1230,   822,    45,    52,   374,
     156,   824,  1479,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,  1314,   372,   771,  1590,  1048,
    1122,  1120,   373,  1586,  1191,  1365,  1449,   154,   155,  1128,
     157,   500,  1225,  1167,  1618,  1169,  1606,  1461,    41,  1335,
     983,   943,   940,  1520,   975,   471,   725,   471,  1012,   838,
     377,   178,   901,   852,  1026,   280,   287,   105,   692,    -1,
      -1,   886,   110,    -1,   112,   113,   114,   115,   116,   117,
     118,    -1,    -1,   200,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1216,    -1,    -1,    -1,  1220,    -1,    -1,    -1,
      -1,  1225,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   154,   155,    -1,   157,
      -1,    -1,    -1,    -1,  1216,    -1,    -1,  1216,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1216,    -1,    -1,    -1,
     178,  1265,    -1,  1232,    -1,  1612,  1270,    -1,    -1,  1229,
    1274,    -1,  1619,    -1,  1278,    -1,  1274,    -1,    -1,    -1,
     204,  1250,   200,    -1,    -1,    -1,   210,  1256,  1257,    -1,
      -1,    -1,   216,    -1,  1463,    -1,    -1,    -1,  1270,  1303,
      -1,  1270,  1274,    -1,    -1,  1309,  1278,    -1,    -1,  1278,
    1270,    -1,  1316,    -1,    -1,    -1,    -1,    -1,  1278,    -1,
     244,   245,    -1,    -1,    -1,    -1,   250,  1569,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1343,
      -1,    -1,  1346,  1347,  1586,    -1,    -1,  1351,   272,  1347,
      -1,    -1,  1594,  1357,    -1,   279,   280,    -1,  1318,    -1,
      -1,    -1,   286,    -1,    -1,    -1,    -1,    -1,   292,    -1,
      -1,  1340,    -1,    -1,  1346,  1347,  1460,  1346,   302,  1351,
      -1,    -1,  1351,    -1,    -1,  1357,  1346,    -1,  1357,    -1,
      -1,  1351,    -1,    -1,    -1,    -1,   320,  1357,    -1,   323,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1565,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,  1512,    -1,
       4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     364,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1581,    -1,    -1,    -1,    -1,  1460,    -1,    -1,    -1,
       4,    -1,    -1,    -1,    63,    64,    -1,    -1,    -1,    -1,
      -1,    45,    -1,    -1,  1478,    -1,    -1,    -1,  1482,   403,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1491,    -1,   413,
      -1,    -1,  1496,  1497,  1599,    -1,  1500,  1501,    -1,    -1,
      -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,  1512,  1614,
    1482,    -1,    -1,  1482,  1518,  1519,    -1,    -1,    -1,    -1,
    1524,    -1,  1482,    -1,    -1,    -1,    -1,    -1,   127,   453,
     454,   105,    -1,   457,    -1,    -1,   110,    -1,   112,   113,
     114,   115,   116,   117,   118,    -1,  1518,  1519,    -1,  1518,
    1519,    -1,  1524,  1557,    -1,  1524,    -1,    -1,  1518,  1519,
    1564,   105,    -1,    -1,  1524,    -1,   110,    -1,   112,   113,
     114,   115,   116,   117,   118,    -1,  1580,    -1,   502,    -1,
     154,   155,    -1,   157,    -1,  1557,    -1,    -1,  1557,    -1,
      -1,    26,    27,    -1,    -1,    30,  1565,  1557,   197,    -1,
      -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,    -1,    -1,
     154,   155,    -1,   157,    -1,    -1,  1620,    -1,    -1,   543,
     544,    -1,  1626,    -1,    -1,   549,   200,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   567,    -1,    -1,    -1,    -1,  1620,    -1,
      -1,  1620,    -1,    -1,  1626,    -1,   200,  1626,    -1,    -1,
    1620,    -1,    -1,    -1,    -1,    -1,  1626,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   621,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   657,    65,   659,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,   679,   680,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    65,    -1,   692,   693,
     694,   695,   696,   697,   698,   210,    -1,    -1,    -1,    29,
     704,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,   729,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,   746,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   755,    -1,   757,   758,    -1,    -1,   272,    -1,    -1,
      -1,    -1,    -1,    -1,   279,   280,    -1,   771,    -1,    -1,
      -1,   286,    -1,    -1,    -1,   779,    -1,   292,    -1,    -1,
      -1,   785,    -1,    -1,    -1,    -1,    -1,   302,    -1,   200,
      -1,    -1,   796,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     804,    -1,    -1,   807,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   826,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,   364,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,   869,    -1,    -1,    -1,    -1,
     200,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   882,    -1,
     884,    -1,   886,    -1,    -1,    -1,    -1,    -1,   403,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   901,    -1,    -1,
     904,    -1,    -1,   907,   908,   909,   910,   911,   912,   913,
     914,   915,   916,   917,   918,   919,   920,   921,   922,   923,
     924,   925,   926,   927,   928,   929,   930,   931,   932,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,   457,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   956,    -1,    29,   959,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,   982,    -1,
     984,    10,    11,    12,    -1,    -1,    -1,   502,   992,    -1,
      -1,    -1,    -1,    -1,   200,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,  1029,    -1,    -1,    -1,    -1,
    1034,    52,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,  1047,  1048,    -1,  1050,    -1,    -1,    -1,
      -1,    -1,   567,    -1,    -1,    -1,    -1,    -1,  1062,    -1,
      -1,  1065,    -1,  1067,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1083,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,   621,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1119,    -1,    -1,  1122,    65,
      -1,    -1,   197,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    -1,    -1,    -1,  1143,
      -1,    -1,   657,    -1,   659,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      -1,    -1,    -1,    -1,   679,   680,    -1,    -1,    -1,    -1,
      -1,   200,    63,    64,    -1,    -1,    -1,    -1,    -1,   694,
     695,   696,   697,   698,    -1,    -1,    -1,    -1,    -1,   704,
      -1,    -1,    -1,  1197,    -1,  1199,    63,    64,    -1,    -1,
    1204,  1205,    -1,    -1,  1208,    -1,  1210,    -1,    -1,  1213,
      -1,    -1,    -1,  1217,   729,    -1,  1220,  1221,    -1,  1223,
      -1,    -1,    -1,   244,   245,    -1,  1230,    -1,    -1,   250,
      -1,   746,    -1,    -1,    -1,    -1,   127,    -1,    -1,  1243,
     755,    -1,   757,   758,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   200,    -1,   771,    -1,    -1,    -1,
     127,    -1,    -1,  1267,   779,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,  1281,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1291,  1292,   804,
      65,    -1,   807,    -1,    -1,    -1,    -1,    -1,    -1,   320,
      -1,    -1,   323,    10,    11,    12,   197,    -1,    -1,    -1,
      -1,   826,  1316,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1325,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
    1354,  1355,    -1,    -1,   869,    -1,  1360,  1361,    65,    -1,
      -1,  1365,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    -1,    -1,    -1,    -1,   901,    -1,    -1,   904,
      -1,    -1,   907,   908,   909,   910,   911,   912,   913,   914,
     915,   916,   917,   918,   919,   920,   921,   922,   923,   924,
     925,   926,   927,   928,   929,   930,   931,   932,    63,    64,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   453,   454,    -1,    -1,   457,    -1,    -1,    -1,
      -1,   956,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
    1464,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,   992,    -1,    -1,
      -1,   502,   127,    -1,    -1,    -1,    -1,  1491,    -1,    -1,
      -1,  1495,    -1,   200,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1506,    -1,    -1,    -1,    -1,  1511,    -1,    -1,
      -1,    -1,    -1,    -1,  1029,    -1,    -1,    -1,    -1,  1034,
      -1,    -1,   543,   544,    -1,    -1,    -1,    -1,   549,    -1,
      -1,    -1,  1047,  1048,    -1,  1050,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1065,    -1,  1067,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1568,    -1,    -1,    -1,  1083,    -1,
      -1,    -1,  1576,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1590,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1599,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1119,    -1,    -1,  1122,    -1,    -1,
    1614,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,  1623,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1631,  1143,    -1,
      -1,  1635,    -1,    29,  1638,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,   680,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,   692,   693,   694,   695,   696,   697,   698,    -1,    -1,
      -1,    -1,    -1,   704,    -1,    -1,    -1,    -1,    -1,  1204,
    1205,    -1,    -1,  1208,    -1,  1210,    -1,    -1,  1213,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1221,    -1,  1223,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1243,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,  1267,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   785,    65,  1281,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   796,  1291,  1292,    -1,    -1,
      -1,    -1,    -1,   804,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,   200,    53,    -1,    -1,    -1,    -1,
    1325,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    26,    27,    -1,    -1,    30,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1354,
    1355,    -1,    -1,    -1,    -1,  1360,  1361,    -1,    -1,    -1,
    1365,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   882,    77,   884,    -1,   886,    -1,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     901,    -1,    -1,    -1,    -1,   185,   907,   908,   909,   910,
     911,   912,   913,   914,   915,   916,   917,   918,   919,   920,
     921,   922,   923,   924,   925,   926,   927,   928,   929,   930,
     931,   932,    77,    -1,    79,    -1,    -1,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,   148,    53,   956,   151,    -1,   959,   154,
     155,    -1,   157,   158,   159,    -1,    65,    -1,    -1,  1464,
      -1,    -1,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   982,    -1,   984,   129,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    -1,    -1,
    1495,    -1,    -1,   148,    -1,   200,   151,   152,    -1,   154,
     155,  1506,   157,   158,   159,    -1,  1511,    -1,    -1,    -1,
      -1,   204,    -1,    -1,    -1,    -1,    -1,   210,    -1,    -1,
      -1,    -1,    -1,   216,    -1,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    -1,  1050,
      -1,   196,    -1,    -1,    -1,    -1,   201,    -1,    -1,    -1,
      -1,  1062,    -1,    -1,  1065,    -1,  1067,    -1,    -1,    -1,
      -1,    -1,    -1,  1568,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1576,  1083,    -1,    -1,    -1,    -1,    -1,    -1,   272,
      -1,    -1,    -1,    -1,    -1,  1590,   279,   280,    -1,    -1,
      -1,    -1,    -1,   286,    -1,    -1,    -1,    -1,    -1,   292,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   302,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1623,    10,
      11,    12,    -1,    -1,    -1,    -1,  1631,    -1,    -1,    -1,
    1635,    -1,  1143,  1638,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   364,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1197,    -1,  1199,    -1,
      -1,    -1,    -1,  1204,    -1,    -1,    -1,  1208,    -1,  1210,
      -1,    -1,  1213,    -1,    -1,    -1,  1217,    -1,    -1,  1220,
     403,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,  1230,
     413,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,  1243,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
    1281,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,    -1,  1316,    -1,    -1,    -1,   200,
      -1,    65,    -1,    -1,  1325,     5,     6,    -1,     8,     9,
      10,    11,    12,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    -1,    -1,    28,    29,
      -1,    -1,    -1,  1354,  1355,    -1,    -1,    -1,    -1,  1360,
      -1,    41,    -1,    -1,    10,    11,    12,    -1,    48,    -1,
      50,    -1,    -1,    53,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   567,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      -1,    -1,   200,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     110,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   621,    -1,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,   200,    53,    -1,    -1,
      -1,    -1,    -1,  1464,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,   657,    -1,   659,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1491,    -1,    -1,    -1,  1495,    -1,   679,    10,    11,    12,
      -1,   181,    -1,    -1,    -1,  1506,    -1,    -1,    -1,    -1,
    1511,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    77,    -1,   225,    -1,    -1,   228,    -1,
      -1,    -1,    65,    -1,   200,   235,   236,    -1,    -1,    -1,
      -1,    -1,    -1,   746,    -1,    -1,    -1,  1568,    -1,    -1,
     104,   105,    -1,    -1,   757,   758,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   457,    -1,    -1,    -1,    -1,   779,    -1,  1599,    -1,
      -1,   281,    -1,    -1,    -1,    -1,    -1,   287,    -1,    -1,
      -1,   291,    -1,  1614,    -1,    -1,    -1,   151,    -1,    -1,
     154,   155,  1623,   157,   158,   159,    -1,    -1,    -1,    -1,
    1631,   311,    -1,    -1,  1635,    -1,   502,  1638,    -1,    -1,
      -1,    -1,   322,   826,    -1,    -1,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   339,
     340,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   197,   365,   869,   367,   368,    -1,
     370,    -1,    77,    -1,    79,    -1,    -1,   377,   378,   379,
     380,   381,   382,   383,   384,   385,   386,   387,   388,   389,
      -1,    -1,    -1,    -1,    -1,   395,   396,    -1,   398,   399,
     400,   904,    -1,    -1,    -1,    -1,   406,    -1,    -1,   409,
      -1,    -1,   117,    -1,    -1,    -1,    -1,    -1,   418,    -1,
     420,    -1,    -1,    -1,   129,    -1,   426,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   436,    -1,   438,    -1,
      -1,    -1,    -1,   148,    -1,    -1,   151,   152,   457,   154,
     155,    -1,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   464,    -1,    -1,   467,   468,   469,
      -1,    -1,    -1,    -1,    -1,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    -1,   992,
      -1,   196,    -1,   502,   680,    -1,   201,    -1,   498,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   694,   695,
     696,   697,   698,    -1,    -1,    -1,    -1,    -1,   704,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1029,    -1,    -1,    -1,
      -1,  1034,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1047,  1048,    -1,    -1,    77,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,   568,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     580,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,   612,    -1,    -1,    -1,    -1,    65,   804,  1122,
     620,    -1,    -1,    -1,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   638,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   457,    -1,    -1,
      -1,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,    -1,   665,    -1,    -1,    68,    -1,
     199,   680,   201,    -1,    -1,    -1,    -1,    77,   678,    79,
      -1,    -1,    -1,    -1,    -1,   694,   695,   696,   697,   698,
      -1,    -1,   502,    -1,    -1,   704,    -1,    -1,    -1,    -1,
      -1,    -1,  1205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   714,   901,    -1,   117,    -1,    -1,
      -1,   907,   908,   909,   910,   911,   912,   913,   914,   915,
     916,   917,   918,   919,   920,   921,   922,   923,   924,   925,
     926,   927,   928,   929,   930,   931,   932,    -1,   148,    -1,
     198,   151,   152,    -1,   154,   155,    -1,   157,   158,   159,
      -1,    -1,    -1,    -1,  1267,    -1,   766,    -1,    -1,    -1,
     956,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   778,    -1,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,    -1,    -1,   804,   196,    -1,    -1,    -1,
      -1,   201,   802,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   812,    -1,    -1,   815,    -1,   817,    -1,    -1,
      -1,   821,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,
     830,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    55,
      -1,    -1,    -1,    -1,    -1,   855,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1050,    -1,    -1,    -1,    -1,    -1,
     680,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1065,
      -1,  1067,    -1,    -1,    -1,    -1,   696,   697,    -1,    -1,
      -1,    -1,   901,    -1,   704,    -1,    -1,  1083,   907,   908,
     909,   910,   911,   912,   913,   914,   915,   916,   917,   918,
     919,   920,   921,   922,   923,   924,   925,   926,   927,   928,
     929,   930,   931,   932,   130,   131,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   934,   935,   936,    -1,    -1,    -1,
     940,   941,   148,    -1,    -1,   151,   152,   956,   154,   155,
      -1,   157,   158,   159,    -1,   161,    -1,  1143,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   172,    -1,   968,    -1,
      -1,    -1,    -1,    -1,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,    -1,   988,    -1,
     196,    -1,    -1,   993,   804,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1006,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1014,    -1,  1016,    -1,  1204,    -1,
      -1,    -1,  1208,    -1,  1210,    -1,    -1,  1213,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1050,    -1,    -1,    -1,  1045,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1054,    -1,  1065,  1243,  1067,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1083,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      55,    -1,    -1,    -1,    -1,  1281,    -1,   907,   908,   909,
     910,   911,   912,   913,   914,   915,   916,   917,   918,   919,
     920,    -1,    77,   923,   924,   925,   926,   927,   928,   929,
     930,   931,   932,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1131,    -1,    -1,  1143,  1135,    -1,  1137,    -1,  1325,
      -1,    -1,    -1,    -1,    -1,    -1,   956,  1147,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1155,  1156,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1164,   130,   131,    -1,  1354,  1355,
      -1,    -1,    -1,    -1,  1360,    -1,    -1,    -1,  1364,    -1,
      -1,    -1,    -1,   148,    -1,    -1,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,  1204,   161,    -1,    -1,  1208,
      -1,  1210,    -1,    -1,  1213,    -1,    -1,   172,    -1,    -1,
      -1,    -1,    -1,    -1,  1214,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    -1,    -1,
      -1,   196,    -1,    -1,  1243,    -1,    -1,    -1,    -1,    -1,
    1050,    -1,    -1,    -1,    -1,    -1,  1246,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1065,    -1,  1067,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1281,  1083,    -1,    -1,    -1,    29,  1464,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1495,
      -1,    -1,    -1,    65,    -1,    -1,  1325,    -1,    -1,    -1,
    1506,    -1,    -1,    -1,    -1,  1511,    -1,    -1,  1328,    -1,
      -1,    -1,    -1,  1143,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,  1354,  1355,    -1,  1534,    -1,
      -1,  1360,    -1,    -1,    77,    29,  1356,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,  1568,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,  1204,    -1,    -1,    -1,  1208,    -1,
    1210,    -1,    29,  1213,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    77,   151,    79,
      -1,   154,   155,  1243,   157,   158,   159,  1623,    65,    -1,
      -1,    -1,    -1,    -1,    -1,  1631,   198,    -1,    -1,  1635,
      -1,    -1,  1638,    -1,    -1,  1464,    -1,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
      -1,  1281,    -1,   123,    -1,    -1,   199,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1495,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1506,    -1,    -1,
      -1,    -1,  1511,    -1,   154,   155,    -1,   157,   158,   159,
      -1,    -1,    -1,    -1,    -1,  1325,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,    -1,    -1,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,    -1,  1354,  1355,    -1,    -1,    -1,   199,
    1360,   201,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1568,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   198,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,     3,     4,
       5,     6,     7,    -1,  1623,    -1,    65,    -1,    13,    -1,
      -1,    -1,  1631,    -1,    -1,    -1,  1635,    -1,    -1,  1638,
      -1,    -1,    27,    28,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      45,    46,    47,    -1,  1464,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,  1495,    81,    -1,    -1,    -1,
      85,    86,    87,    88,    -1,    90,  1506,    92,    -1,    94,
      -1,  1511,    97,    -1,    -1,    -1,   101,   102,   103,   104,
     105,   106,   107,    -1,   109,   110,   111,   112,   113,   114,
     115,   116,   117,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,
     135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,   198,
      -1,    -1,    -1,   148,   149,   150,   151,   152,  1568,   154,
     155,    -1,   157,   158,   159,   160,    -1,    -1,   163,    -1,
      -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,    -1,
     175,    -1,   177,   178,    -1,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,    -1,
      -1,   196,    -1,   198,   199,   200,   201,   202,    -1,   204,
     205,    -1,    -1,  1623,     3,     4,     5,     6,     7,    -1,
      -1,  1631,    -1,    -1,    13,  1635,    -1,    -1,  1638,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    45,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,    88,
      -1,    90,    -1,    92,    -1,    94,    -1,    -1,    97,    -1,
      -1,    -1,   101,   102,   103,   104,   105,   106,   107,    -1,
     109,   110,   111,   112,   113,   114,   115,   116,   117,    -1,
     119,   120,   121,   122,   123,   124,    -1,    -1,    -1,    -1,
     129,   130,    -1,   132,   133,   134,   135,   136,    -1,    -1,
      -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,
     149,   150,   151,   152,    -1,   154,   155,    -1,   157,   158,
     159,   160,    -1,    -1,   163,    -1,    -1,   166,    -1,    -1,
      -1,    -1,    -1,   172,   173,    -1,   175,    -1,   177,   178,
      -1,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,    -1,    -1,   196,    -1,   198,
     199,   200,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    69,    70,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,    85,
      86,    87,    88,    -1,    90,    -1,    92,    -1,    94,    -1,
      -1,    97,    -1,    -1,    -1,   101,   102,   103,   104,   105,
     106,   107,    -1,   109,   110,   111,   112,   113,   114,   115,
     116,   117,    -1,   119,   120,   121,   122,   123,   124,    -1,
      -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,   135,
     136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,   160,    -1,    -1,   163,    -1,    -1,
     166,    -1,    -1,    -1,    -1,    -1,   172,   173,    -1,   175,
      -1,   177,   178,    -1,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,    -1,    -1,
     196,    -1,   198,   199,    -1,   201,   202,    -1,   204,   205,
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
      -1,    -1,    -1,    -1,   177,   178,    -1,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,    -1,    -1,   196,    -1,   198,   199,   200,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
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
      -1,    -1,   172,    -1,    -1,    -1,    -1,   177,   178,    -1,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,    -1,    -1,   196,    -1,   198,   199,
     200,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
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
      -1,    -1,    -1,    -1,    -1,   172,    -1,    -1,    -1,    -1,
     177,   178,    -1,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,    -1,    -1,   196,
      -1,   198,   199,   200,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    -1,    -1,
      -1,    85,    86,    87,    88,    89,    90,    -1,    92,    -1,
      94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,   103,
     104,    -1,   106,   107,    -1,   109,    -1,   111,   112,   113,
     114,   115,   116,   117,    -1,   119,   120,   121,    -1,   123,
     124,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,
     134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,   160,    -1,    -1,   163,
      -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,    -1,
      -1,    -1,    -1,   177,   178,    -1,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
      -1,    -1,   196,    -1,   198,   199,    -1,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,
      -1,    52,    -1,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,    -1,
      81,    -1,    -1,    -1,    85,    86,    87,    88,    -1,    90,
      -1,    92,    -1,    94,    95,    -1,    97,    -1,    -1,    -1,
     101,   102,   103,   104,    -1,   106,   107,    -1,   109,    -1,
     111,   112,   113,   114,   115,   116,   117,    -1,   119,   120,
     121,    -1,   123,   124,    -1,    -1,    -1,    -1,   129,   130,
      -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,
      -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,
     151,   152,    -1,   154,   155,    -1,   157,   158,   159,   160,
      -1,    -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,
      -1,   172,    -1,    -1,    -1,    -1,   177,   178,    -1,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,    -1,    -1,   196,    -1,   198,   199,    -1,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
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
      -1,    -1,    -1,    -1,   172,    -1,    -1,    -1,    -1,   177,
     178,    -1,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,    -1,    -1,   196,    -1,
     198,   199,   200,   201,   202,    -1,   204,   205,     3,     4,
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
      -1,   166,    -1,    -1,    -1,    -1,    -1,   172,    -1,    -1,
      -1,    -1,   177,   178,    -1,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,    -1,
      -1,   196,    -1,   198,   199,   200,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    81,
      -1,    -1,    -1,    85,    86,    87,    88,    -1,    90,    -1,
      92,    93,    94,    -1,    -1,    97,    -1,    -1,    -1,   101,
     102,   103,   104,    -1,   106,   107,    -1,   109,    -1,   111,
     112,   113,   114,   115,   116,   117,    -1,   119,   120,   121,
      -1,   123,   124,    -1,    -1,    -1,    -1,   129,   130,    -1,
     132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,
      -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,   160,    -1,
      -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,
     172,    -1,    -1,    -1,    -1,   177,   178,    -1,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,    -1,    -1,   196,    -1,   198,   199,    -1,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
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
      -1,    -1,    -1,   172,    -1,    -1,    -1,    -1,   177,   178,
      -1,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,    -1,    -1,   196,    -1,   198,
     199,   200,   201,   202,    -1,   204,   205,     3,     4,     5,
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
     166,    -1,    -1,    -1,    -1,    -1,   172,    -1,    -1,    -1,
      -1,   177,   178,    -1,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,    -1,    -1,
     196,    -1,   198,   199,   200,   201,   202,    -1,   204,   205,
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
      -1,    -1,    -1,    -1,   177,   178,    -1,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,    -1,    -1,   196,    -1,   198,   199,    -1,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
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
      -1,    -1,   172,    -1,    -1,    -1,    -1,   177,   178,    -1,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,    -1,    -1,   196,    -1,   198,   199,
     200,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
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
      -1,    -1,    -1,    -1,    -1,   172,    -1,    -1,    -1,    -1,
     177,   178,    -1,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,    -1,    -1,   196,
      -1,   198,   199,   200,   201,   202,    -1,   204,   205,     3,
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
      -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,    -1,
      -1,    -1,    -1,   177,   178,    -1,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
      -1,    -1,   196,    -1,   198,   199,   200,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
      -1,   172,    -1,    -1,    -1,    -1,   177,   178,    -1,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,    -1,    -1,   196,    -1,   198,   199,    -1,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
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
      -1,    -1,    -1,    -1,   172,    -1,    -1,    -1,    -1,   177,
     178,    -1,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,    -1,    -1,   196,    -1,
     198,   199,    -1,   201,   202,    -1,   204,   205,     3,     4,
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
      -1,   166,    -1,    -1,    -1,    -1,    -1,   172,    -1,    -1,
      -1,    -1,   177,   178,    -1,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,    -1,
      -1,   196,    -1,   198,   199,    -1,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
     172,    -1,    -1,    -1,    -1,   177,   178,    -1,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,    -1,    -1,   196,    -1,   198,   199,    -1,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
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
      -1,    -1,    -1,   172,    -1,    -1,    -1,    -1,   177,   178,
      -1,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,    -1,    -1,   196,    -1,   198,
     199,    -1,   201,   202,    -1,   204,   205,     3,     4,     5,
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
     166,    -1,    -1,    -1,    -1,    -1,   172,    -1,    -1,    -1,
      -1,   177,   178,    -1,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,    -1,    -1,
     196,    -1,   198,   199,    -1,   201,   202,    -1,   204,   205,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    46,    47,    -1,    -1,    -1,    -1,    52,
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
      -1,    -1,    -1,    -1,   177,   178,    -1,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,    -1,    -1,   196,    -1,   198,   199,    -1,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,
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
      -1,    -1,   172,    -1,    -1,    -1,    -1,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,    -1,    -1,   196,    -1,    -1,    -1,
      -1,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
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
      -1,    -1,    -1,    -1,    -1,   172,    -1,    -1,    -1,    -1,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,    -1,    -1,   196,
      -1,   198,    -1,    -1,   201,   202,    -1,   204,   205,     3,
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
      -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,    -1,
      -1,    -1,    -1,   177,   178,    -1,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
      -1,    -1,   196,    11,    12,    -1,    -1,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    29,    13,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    35,    53,    -1,    -1,    -1,    -1,
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
     161,    -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,
      -1,   172,    -1,    -1,    -1,    -1,   177,   178,    -1,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,    -1,    -1,   196,    -1,    -1,    -1,    -1,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
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
      -1,    -1,    -1,    -1,   172,    -1,    -1,    -1,    -1,   177,
     178,    -1,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,    -1,    -1,   196,    -1,
      10,    11,    12,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    47,    -1,    -1,    65,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   101,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,   113,   114,
     115,   116,   117,    -1,    -1,   120,   121,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,
     135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,
      -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,    -1,    -1,    -1,   163,    -1,
      -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   188,   189,
      -1,    -1,   177,   178,    -1,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,    -1,
      -1,   196,    -1,    12,    -1,    -1,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
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
     152,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,
     172,    -1,    -1,    -1,    -1,   177,   178,    -1,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,    -1,    -1,   196,    -1,    10,    11,    12,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    65,    -1,    52,    -1,    54,    55,    56,    57,    58,
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
     184,    -1,    -1,   172,    -1,    -1,    -1,    -1,   177,   178,
      -1,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,    -1,    -1,   196,    -1,   198,
      11,    12,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     166,    -1,    -1,    -1,    -1,    -1,   172,    -1,    -1,    -1,
      -1,   177,   178,    -1,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,    -1,    -1,
     196,    -1,   198,    -1,    12,   201,   202,    -1,   204,   205,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,   177,   178,    -1,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,    -1,    -1,   196,   197,    -1,    -1,    -1,   201,   202,
      -1,   204,   205,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   172,    -1,    -1,    -1,    -1,   177,   178,    -1,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,    -1,    -1,   196,    -1,    -1,    -1,
      -1,   201,   202,    -1,   204,   205,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    31,    32,    33,
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
      -1,    -1,    -1,    -1,    -1,   172,    -1,    -1,    -1,    -1,
     177,   178,    -1,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,    -1,    -1,   196,
      -1,    -1,    -1,    -1,   201,   202,    -1,   204,   205,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
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
     154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,   163,
      -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,    -1,
      -1,    -1,    -1,   177,   178,    -1,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
      -1,    -1,   196,    -1,    -1,    -1,    -1,   201,   202,    -1,
     204,   205,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    29,    13,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    35,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,
      -1,   172,    -1,    -1,    -1,    -1,   177,   178,    -1,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,    -1,    -1,   196,    -1,    10,    11,    12,
     201,   202,    -1,   204,   205,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    65,    -1,    52,    -1,    54,    55,    56,    57,
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
     183,    -1,    -1,    -1,   172,    -1,    -1,    -1,    -1,   177,
     178,    -1,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,    -1,    -1,   196,    -1,
      -1,   199,    -1,   201,   202,    -1,   204,   205,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   166,    -1,    -1,    -1,    -1,    -1,   172,    -1,    -1,
      -1,    -1,   177,   178,    -1,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,    -1,
      -1,   196,    -1,    10,    11,    12,   201,   202,    -1,   204,
     205,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    65,    -1,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,    -1,    96,
      -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     112,   113,   114,   115,   116,   117,    -1,    -1,   120,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,    -1,
     132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,
      -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,
     172,    -1,    -1,    -1,    -1,   177,   178,    -1,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,    -1,    -1,   196,    -1,    10,    11,    12,   201,
     202,    -1,   204,   205,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    65,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   112,   113,   114,   115,   116,   117,    -1,
      -1,   120,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     129,   130,    -1,   132,   133,   134,   135,   136,    -1,    -1,
      -1,    77,    -1,    79,   143,    -1,    -1,    -1,    -1,   148,
     149,   150,   151,   152,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    -1,   163,    -1,    -1,   166,    -1,    -1,
      -1,    -1,    -1,   172,    -1,    -1,    -1,    -1,   177,   178,
      -1,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,    -1,    -1,   196,    -1,    -1,
      -1,    -1,   201,   202,    -1,   204,   205,     3,     4,     5,
       6,     7,    -1,    -1,    10,    11,    12,    13,   154,   155,
      -1,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,    53,    -1,    -1,
      -1,    -1,    -1,   199,    -1,   201,    -1,    -1,    -1,    -1,
      -1,    67,    68,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,    -1,    -1,   129,   130,    -1,   132,   133,   134,   135,
     136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   148,   149,   150,    -1,    -1,    -1,   154,   155,
      -1,   157,   158,   159,   160,    -1,   162,   163,    -1,   165,
      -1,    -1,    -1,    -1,    -1,    -1,   172,   173,    -1,   175,
      -1,   177,   178,    -1,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,
      -1,    79,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    29,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    55,    53,    -1,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    77,    -1,    -1,    -1,    -1,
      -1,    -1,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,    -1,    -1,    -1,    -1,    55,
      -1,   199,   104,   201,    -1,    -1,    -1,    -1,    -1,    -1,
     112,   113,   114,   115,   116,   117,    -1,    -1,    -1,    29,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    55,   148,    -1,   104,   151,
     152,    -1,   154,   155,   197,   157,   158,   159,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,
     172,    -1,    -1,    -1,   130,   131,   178,    -1,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,    -1,   148,   192,   196,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   172,    -1,    -1,    -1,
     130,   131,    -1,    -1,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,    -1,   148,    -1,
     196,   151,   152,    -1,   154,   155,    -1,   157,   158,   159,
      77,    -1,    79,    -1,    -1,    -1,    -1,    -1,    85,    -1,
      -1,    -1,   172,    -1,    -1,    -1,    -1,    30,    -1,    -1,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,    46,    47,    -1,   196,    -1,    -1,    52,
     117,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,
      -1,   148,    85,    -1,   151,   152,    -1,   154,   155,    -1,
     157,   158,   159,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,    -1,   130,    -1,   132,
     133,   134,   135,   136,   201,    77,    -1,    79,    -1,    -1,
     143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,
     163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   172,
      -1,    -1,    -1,    -1,   177,    -1,    -1,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
      46,    47,    -1,   196,    -1,    -1,    52,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,   154,   155,    -1,   157,   158,   159,    74,    75,
      76,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    85,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,    -1,    -1,    -1,    -1,    -1,    -1,   199,    -1,   201,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   130,    -1,   132,   133,   134,   135,
     136,    -1,    -1,    -1,    46,    47,    -1,   143,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,    66,    -1,    -1,   163,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,   172,    -1,    -1,    -1,
      -1,   177,    -1,    85,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,    -1,    -1,    -1,
     196,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,    -1,
     132,   133,   134,   135,   136,    -1,    -1,    -1,    46,    47,
      -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,    66,    -1,
      -1,   163,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
     172,    -1,    -1,    -1,    -1,   177,    -1,    85,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    67,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    77,    -1,    79,    -1,    -1,    -1,
      -1,    -1,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   154,   155,    -1,   157,
     158,   159,    -1,    -1,   117,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   172,    77,    -1,    79,    -1,    -1,
      -1,    -1,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   148,    -1,    -1,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,
      -1,    -1,    -1,   166,    -1,   117,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
      -1,    -1,    -1,   196,    -1,    -1,   148,    -1,   201,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,    77,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,    -1,    -1,    -1,   196,    -1,    -1,   199,   117,   201,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,
     129,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   148,
      -1,    -1,   151,   152,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   117,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,    77,    -1,    -1,   196,    -1,    -1,
     148,    -1,   201,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,    -1,    -1,    -1,   196,    -1,
      -1,    -1,   117,   201,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,   152,
      -1,   154,   155,   156,   157,   158,   159,    -1,    -1,    -1,
      -1,    -1,    -1,   148,    77,    -1,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,    -1,    -1,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     121,    -1,    -1,   196,    -1,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    77,    -1,
      -1,   196,    -1,    -1,    -1,    -1,   201,    -1,    -1,    -1,
      -1,    -1,    -1,   154,   155,    -1,   157,   158,   159,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,    -1,    -1,    -1,   196,    -1,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
      -1,    -1,    -1,   196,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,    -1,    -1,    29,   196,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   128,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,   128,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   128,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,   128,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,   128,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   128,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    74,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,   128,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    85,    -1,    -1,    -1,    -1,    -1,   128,    -1,    77,
      -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     118,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,
      -1,   128,   130,   131,   148,    -1,    -1,   151,    77,    -1,
     154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,    -1,
     148,    -1,    -1,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    -1,    -1,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,    -1,
      -1,    -1,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   148,    -1,    -1,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,    77,    -1,    79,
      80,    -1,    -1,   152,    -1,   154,   155,    -1,   157,   158,
     159,    77,    -1,    -1,    -1,    -1,    -1,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
      -1,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   154,   155,    -1,   157,   158,   159,
      -1,    -1,    -1,    -1,    -1,   151,    -1,    -1,   154,   155,
      -1,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,    -1,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   151,    -1,    -1,
     154,   155,    -1,   157,   158,   159,    77,    -1,    -1,    -1,
     151,    -1,    -1,   154,   155,    -1,   157,   158,   159,    77,
      -1,    -1,    -1,    -1,    -1,    -1,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   123,    -1,    -1,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    -1,    -1,   123,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   154,   155,    -1,   157,   158,   159,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   207,   208,     0,   209,     3,     4,     5,     6,     7,
      13,    27,    28,    45,    46,    47,    52,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    66,    67,
      68,    69,    70,    74,    75,    76,    77,    78,    79,    81,
      85,    86,    87,    88,    90,    92,    94,    97,   101,   102,
     103,   104,   105,   106,   107,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   119,   120,   121,   122,   123,   124,
     129,   130,   132,   133,   134,   135,   136,   143,   148,   149,
     150,   151,   152,   154,   155,   157,   158,   159,   160,   163,
     166,   172,   173,   175,   177,   178,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     196,   198,   199,   201,   202,   204,   205,   210,   213,   220,
     221,   222,   223,   224,   225,   228,   243,   244,   248,   253,
     259,   314,   315,   320,   324,   325,   326,   327,   328,   329,
     330,   331,   333,   336,   345,   346,   347,   349,   350,   352,
     371,   381,   382,   383,   388,   392,   410,   415,   417,   418,
     419,   420,   421,   422,   423,   424,   426,   439,   441,   443,
     115,   116,   117,   129,   148,   213,   243,   314,   330,   417,
     330,   196,   330,   330,   330,   101,   330,   330,   408,   409,
     330,   330,   330,   330,   330,   330,   330,   330,   330,   330,
     330,   330,    79,   117,   196,   221,   382,   383,   417,   417,
      35,   330,   430,   431,   330,   117,   196,   221,   382,   383,
     384,   416,   422,   427,   428,   196,   321,   385,   196,   321,
     337,   322,   330,   230,   321,   196,   196,   196,   321,   198,
     330,   213,   198,   330,    29,    55,   130,   131,   152,   172,
     196,   213,   224,   444,   455,   456,   179,   198,   327,   330,
     351,   353,   199,   236,   330,   104,   105,   151,   214,   217,
     220,    79,   201,   285,   286,   123,   123,    79,   287,   196,
     196,   196,   196,   213,   257,   445,   196,   196,    79,    84,
     144,   145,   146,   436,   437,   151,   199,   220,   220,   258,
     445,   152,   196,   445,   445,   338,   320,   330,   331,   417,
     226,   199,    84,   386,   436,    84,   436,   436,    30,   151,
     168,   446,   196,     9,   198,    35,   242,   152,   256,   445,
     117,   243,   315,   198,   198,   198,   198,   198,   198,    10,
      11,    12,    29,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    53,    65,   198,    66,    66,   198,
     199,   147,   124,   160,   259,   313,   314,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      63,    64,   127,   412,   413,    66,   199,   414,   196,    66,
     199,   201,   423,   196,   242,   243,    14,   330,   198,   128,
      44,   213,   407,   196,   320,   417,   147,   417,   128,   203,
       9,   394,   320,   417,   446,   147,   196,   387,   127,   412,
     413,   414,   197,   330,    30,   228,     8,   339,     9,   198,
     228,   229,   322,   323,   330,   213,   271,   232,   198,   198,
     198,   456,   456,   168,   196,   104,   456,    14,   213,    79,
     198,   198,   198,   179,   180,   181,   186,   187,   190,   191,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   366,
     367,   368,   237,   108,   165,   198,   151,   215,   218,   220,
     151,   216,   219,   220,   220,     9,   198,    96,   199,   417,
       9,   198,    14,     9,   198,   417,   440,   440,   320,   331,
     417,   197,   168,   251,   129,   417,   429,   430,    66,   127,
     144,   437,    78,   330,   417,    84,   144,   437,   220,   212,
     198,   199,   254,   372,   374,    85,   201,   340,   341,   343,
     383,   423,   441,    14,    96,   442,   281,   282,   410,   411,
     197,   197,   197,   200,   227,   228,   244,   248,   253,   330,
     202,   204,   205,   213,   447,   448,   456,    35,   161,   283,
     284,   330,   444,   196,   445,   249,   242,   330,   330,   330,
      30,   330,   330,   330,   330,   330,   330,   330,   330,   330,
     330,   330,   330,   330,   330,   330,   330,   330,   330,   330,
     330,   330,   330,   384,   330,   330,   425,   425,   330,   432,
     433,   123,   199,   213,   422,   423,   257,   258,   256,   243,
      27,    35,   324,   327,   330,   351,   330,   330,   330,   330,
     330,   330,   330,   330,   330,   330,   330,   330,   199,   213,
     422,   425,   330,   283,   425,   330,   429,   242,   197,   330,
     196,   406,     9,   394,   320,   197,   213,    35,   330,    35,
     330,   197,   197,   422,   283,   199,   213,   422,   197,   226,
     275,   199,   330,   330,    88,    30,   228,   269,   198,    96,
      14,     9,   197,    30,   199,   272,   456,    85,   224,   452,
     453,   454,   196,     9,    46,    47,    52,    54,    66,    85,
     130,   143,   152,   172,   196,   221,   222,   224,   348,   382,
     388,   389,   390,   391,   182,    79,   330,    79,    79,   330,
     363,   364,   330,   330,   356,   366,   185,   369,   226,   196,
     235,   220,   198,     9,    96,   220,   198,     9,    96,    96,
     217,   213,   330,   286,   390,    79,     9,   197,   197,   197,
     197,   197,   198,   213,   451,   125,   262,   196,     9,   197,
     197,    79,    80,   213,   438,   213,    66,   200,   200,   209,
     211,   126,   261,   167,    50,   152,   167,   376,   128,     9,
     394,   197,   147,   456,   456,    14,   194,     9,   395,   456,
     457,   127,   412,   413,   414,   200,     9,   394,   169,   417,
     330,   197,     9,   395,    14,   334,   245,   125,   260,   196,
     445,   330,    30,   203,   203,   128,   200,     9,   394,   330,
     446,   196,   252,   255,   250,   242,    68,   417,   330,   446,
     196,   203,   200,   197,   203,   200,   197,    46,    47,    66,
      74,    75,    76,    85,   130,   143,   172,   213,   397,   399,
     402,   405,   213,   417,   417,   128,   412,   413,   414,   197,
     330,   276,    71,    72,   277,   226,   321,   226,   323,    35,
     129,   266,   417,   390,   213,    30,   228,   270,   198,   273,
     198,   273,     9,   169,   128,   147,     9,   394,   197,   161,
     447,   448,   449,   447,   389,   389,   390,   390,   390,   393,
     396,   196,    84,   147,   196,   390,   147,   199,    10,    11,
      12,    29,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,   330,   182,   182,    14,   188,   189,   365,
       9,   192,   369,    79,   200,   382,   199,   239,    96,   218,
     213,    96,   219,   213,   213,   200,    14,   417,   198,    96,
       9,   169,   263,   382,   199,   429,   129,   417,    14,   203,
     330,   200,   209,   263,   199,   375,    14,   330,   340,   213,
     198,   456,    30,   450,   411,    35,    79,   161,   199,   213,
     422,   456,    35,   161,   330,   390,   281,   196,   382,   261,
     335,   246,   330,   330,   330,   200,   196,   283,   262,   261,
     260,   445,   384,   200,   196,   283,    14,    74,    75,    76,
     213,   398,   398,   399,   400,   401,   196,    84,   144,   196,
       9,   394,   197,   406,    35,   330,   200,    71,    72,   278,
     321,   228,   200,   198,    89,   198,   417,   196,   128,   265,
      14,   226,   273,    98,    99,   100,   273,   200,   456,   456,
     213,   452,     9,   197,   394,   128,   203,     9,   394,   393,
     213,   340,   342,   344,   197,   123,   213,   390,   434,   435,
     390,   390,   390,    30,   390,   390,   390,   390,   390,   390,
     390,   390,   390,   390,   389,   389,   390,   390,   390,   390,
     390,   390,   390,   390,   390,   390,   330,   330,   330,   364,
     330,   354,    79,   240,   213,   213,   390,   456,   213,     9,
     288,   197,   196,   324,   327,   330,   203,   200,   288,   153,
     166,   199,   371,   378,   153,   199,   377,   128,   198,   456,
     339,   457,    79,    14,    79,   330,   446,   196,   417,   330,
     197,   281,   199,   281,   196,   128,   196,   283,   197,   199,
     199,   261,   247,   387,   196,   283,   197,   128,   203,     9,
     394,   400,   144,   340,   403,   404,   399,   417,   321,    30,
      73,   228,   198,   323,   429,   266,   197,   390,    95,    98,
     198,   330,    30,   198,   274,   200,   169,   128,   161,    30,
     197,   390,   390,   197,   128,     9,   394,   197,   128,   200,
       9,   394,   390,    30,   183,   197,   226,    96,   382,     4,
     105,   110,   118,   154,   155,   157,   200,   289,   312,   313,
     314,   319,   410,   429,   200,   200,    50,   330,   330,   330,
      35,    79,   161,    14,   390,   200,   196,   283,   450,   197,
     288,   197,   281,   330,   283,   197,   288,   288,   199,   196,
     283,   197,   399,   399,   197,   128,   197,     9,   394,    30,
     226,   198,   197,   197,   233,   198,   198,   274,   226,   456,
     456,   128,   390,   340,   390,   390,   390,   330,   199,   200,
     456,   125,   126,   444,   264,   382,   118,   130,   131,   152,
     158,   298,   299,   300,   382,   156,   304,   305,   121,   196,
     213,   306,   307,   290,   243,   456,     9,   198,   313,   197,
     152,   373,   200,   200,    79,    14,    79,   390,   196,   283,
     197,   110,   332,   450,   200,   450,   197,   197,   200,   200,
     288,   281,   197,   128,   399,   340,   226,   231,    30,   228,
     268,   226,   197,   390,   128,   128,   184,   226,   382,   382,
      14,     9,   198,   199,   199,     9,   198,     3,     4,     5,
       6,     7,    10,    11,    12,    13,    27,    28,    53,    67,
      68,    69,    70,    71,    72,    73,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   129,   130,   132,
     133,   134,   135,   136,   148,   149,   150,   160,   162,   163,
     165,   172,   173,   175,   177,   178,   213,   379,   380,     9,
     198,   152,   156,   213,   307,   308,   309,   198,    79,   318,
     242,   291,   444,   243,    14,   390,   283,   197,   196,   199,
     198,   199,   310,   332,   450,   200,   197,   399,   128,    30,
     228,   267,   226,   390,   390,   330,   200,   198,   198,   390,
     382,   294,   301,   388,   299,    14,    30,    47,   302,   305,
       9,    33,   197,    29,    46,    49,    14,     9,   198,   445,
     318,    14,   242,   390,   197,    35,    79,   370,   226,   226,
     199,   310,   450,   399,   226,    93,   185,   238,   200,   213,
     224,   295,   296,   297,     9,   200,   390,   380,   380,    55,
     303,   308,   308,    29,    46,    49,   390,    79,   196,   198,
     390,   445,    79,     9,   395,   200,   200,   226,   310,    91,
     198,    79,   108,   234,   147,    96,   388,   159,    14,   292,
     196,    35,    79,   197,   200,   198,   196,   165,   241,   213,
     313,   314,   390,   279,   280,   411,   293,    79,   382,   239,
     162,   213,   198,   197,     9,   395,   112,   113,   114,   316,
     317,   279,    79,   264,   198,   450,   411,   457,   197,   197,
     198,   198,   199,   311,   316,    35,    79,   161,   450,   199,
     226,   457,    79,    14,    79,   311,   226,   200,    35,    79,
     161,    14,   390,   200,    79,    14,    79,   390,    14,   390,
     390
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
#line 730 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 733 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 740 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 741 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 744 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 745 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 746 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 747 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 748 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 749 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 752 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 754 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 755 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 756 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 757 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 758 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 760 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 762 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 763 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 769 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 771 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 778 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 780 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 781 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 782 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 783 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 791 "hphp.y"
    { ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 792 "hphp.y"
    { ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 797 "hphp.y"
    { ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 798 "hphp.y"
    { ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 803 "hphp.y"
    { ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 804 "hphp.y"
    { ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 808 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 809 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 810 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 812 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 816 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 817 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 818 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 820 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 824 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 825 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 826 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 828 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 832 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 839 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 840 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 843 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 850 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 857 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 865 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 868 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 874 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 875 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 878 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 879 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 880 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 881 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 884 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 888 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 893 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 894 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 896 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 900 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 903 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
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
#line 909 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 912 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 914 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 917 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 918 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 919 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 920 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 921 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 922 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 923 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 924 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 925 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 926 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 928 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 929 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 934 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 938 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 945 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 946 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 949 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 950 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 951 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 952 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval));;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 956 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 957 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 958 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 959 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 960 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 961 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 962 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 963 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 964 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 965 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 966 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 974 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 984 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 985 "hphp.y"
    { (yyval).reset();;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 989 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 991 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 997 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 998 "hphp.y"
    { (yyval).reset();;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1003 "hphp.y"
    { (yyval).reset();;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1018 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1024 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1030 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1036 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1042 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1050 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1053 "hphp.y"
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
#line 1068 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1071 "hphp.y"
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
#line 1085 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1088 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1093 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1096 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1103 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1106 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1114 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1117 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1125 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1126 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1130 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1133 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1136 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1137 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1138 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1142 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1143 "hphp.y"
    { (yyval).reset();;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1146 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1147 "hphp.y"
    { (yyval).reset();;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1150 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1151 "hphp.y"
    { (yyval).reset();;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1154 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1156 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1159 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1161 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1165 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1166 "hphp.y"
    { (yyval).reset();;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1169 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1170 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1171 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1175 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1177 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1180 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1182 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1185 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1187 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1190 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1192 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1202 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1203 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1204 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1205 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1210 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1212 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1213 "hphp.y"
    { (yyval).reset();;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1216 "hphp.y"
    { (yyval).reset();;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1217 "hphp.y"
    { (yyval).reset();;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1222 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1223 "hphp.y"
    { (yyval).reset();;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1228 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1229 "hphp.y"
    { (yyval).reset();;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1232 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1233 "hphp.y"
    { (yyval).reset();;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1236 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1237 "hphp.y"
    { (yyval).reset();;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1245 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1251 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1259 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1264 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1267 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1277 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1282 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1287 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1292 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1297 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1303 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1309 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1317 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1322 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1326 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1329 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1333 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1336 "hphp.y"
    { (yyval).reset();;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1341 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1344 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1348 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1352 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1356 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1360 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1365 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1370 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1376 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1377 "hphp.y"
    { (yyval).reset();;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1380 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1381 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1382 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1384 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1386 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1388 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1392 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1393 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1396 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1397 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1398 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1402 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1404 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1405 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1406 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1411 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1412 "hphp.y"
    { (yyval).reset();;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1415 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1416 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1419 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1420 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1422 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1426 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1432 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1439 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1445 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1450 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1452 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1454 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1456 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1458 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1459 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1462 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1465 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1466 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1467 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1473 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1477 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1480 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1487 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1488 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1493 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1503 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1505 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1510 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1516 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1518 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1519 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1523 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1525 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1529 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1530 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1534 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1535 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1539 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1542 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1547 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1552 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1553 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1555 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1559 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1560 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1566 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1567 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1568 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1578 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1581 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1582 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1586 "hphp.y"
    { (yyval).reset();;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1587 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1591 "hphp.y"
    { (yyval).reset();;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1592 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1595 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1596 "hphp.y"
    { (yyval).reset();;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1600 "hphp.y"
    { (yyval).reset();;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1603 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1605 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1608 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1610 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1618 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1619 "hphp.y"
    { (yyval).reset();;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1622 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1623 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1624 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1628 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1630 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1631 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1632 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1636 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1638 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1642 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1644 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1645 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1646 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1647 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1650 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1654 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1655 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1659 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1660 "hphp.y"
    { (yyval).reset();;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1664 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1665 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1669 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1678 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1682 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1687 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1693 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1697 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1698 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1699 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1702 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1703 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1704 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1708 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1709 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1710 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1713 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1714 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1716 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1717 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1718 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1719 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1721 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1722 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1723 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1724 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1725 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1726 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1727 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1728 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1729 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1730 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1731 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1732 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1735 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1736 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1739 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1744 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1745 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1748 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1751 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1752 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1761 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { (yyval).reset();;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { Token v; Token w;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]) ;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1872 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1875 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1880 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1886 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1891 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1895 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1944 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1967 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1971 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1975 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1980 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1985 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1989 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1993 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1994 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1998 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 2003 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 2004 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2008 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2009 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2013 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2017 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2021 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2025 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2026 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2027 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2028 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2035 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2038 "hphp.y"
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

  case 492:

/* Line 1455 of yacc.c  */
#line 2049 "hphp.y"
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

  case 493:

/* Line 1455 of yacc.c  */
#line 2060 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2061 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2066 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2067 "hphp.y"
    { (yyval).reset();;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2070 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2071 "hphp.y"
    { (yyval).reset();;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2074 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2078 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2081 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2084 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2091 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2092 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2096 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2098 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2100 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2103 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2104 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2105 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2106 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2107 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2108 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2109 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2110 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2111 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2112 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2114 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2115 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2116 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2117 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2118 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2120 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2121 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2123 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2124 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2125 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2126 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2127 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2128 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2129 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2132 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2133 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2135 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2136 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2137 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2139 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2140 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2141 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2142 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2146 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2147 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2148 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2149 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2150 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2151 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2153 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2154 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2155 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2156 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2159 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2160 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2161 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2162 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2163 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2164 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2165 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2166 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2167 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2168 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2169 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2170 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2171 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2173 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2174 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2175 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2176 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2177 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2178 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2179 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2181 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2182 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2187 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2191 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2195 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2201 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2207 "hphp.y"
    { (yyval).reset();;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { (yyval).reset();;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2213 "hphp.y"
    { (yyval).reset();;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2214 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2215 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2219 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
    { (yyval).reset();;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2227 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2236 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2332 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { (yyval).reset();;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2337 "hphp.y"
    { (yyval).reset();;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2338 "hphp.y"
    { (yyval).reset();;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { (yyval).reset();;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2348 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2352 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2353 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { (yyval).reset();;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2410 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { (yyval).reset();;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { (yyval).reset();;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2459 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2465 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2471 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2476 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2485 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2486 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2487 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2489 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2492 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2494 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2506 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2513 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2521 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2524 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2535 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2539 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2550 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2554 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2558 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2565 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2566 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2567 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2570 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2574 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    { (yyval).reset();;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2579 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2580 "hphp.y"
    { (yyval)++;;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2584 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2588 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2591 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2596 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2598 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2601 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2608 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2610 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2616 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2617 "hphp.y"
    { (yyval).reset();;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2621 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2624 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2627 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2629 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2630 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2631 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2643 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2644 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2649 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2650 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2655 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2657 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2659 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2660 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2664 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2666 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2667 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2669 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2674 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2676 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2678 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2680 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2682 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2683 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2686 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2687 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2688 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2692 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2693 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2694 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2695 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2696 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2697 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2698 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2699 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2700 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2704 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2705 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2710 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2712 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2726 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2730 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2737 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2743 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2747 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2753 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2758 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2761 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2774 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2775 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2779 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2780 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2785 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); ;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2786 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); ;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2788 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); ;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2789 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2795 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2800 "hphp.y"
    { ;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2814 "hphp.y"
    {;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2818 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2825 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2828 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2831 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2832 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2835 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2840 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2843 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2846 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2852 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2858 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2866 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2867 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 12976 "hphp.tab.cpp"
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
#line 2870 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

