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
#define YYLAST   16575

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  208
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  266
/* YYNRULES -- Number of rules.  */
#define YYNRULES  907
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1722

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
    1585,  1589,  1590,  1595,  1596,  1603,  1604,  1612,  1615,  1618,
    1623,  1625,  1627,  1633,  1637,  1643,  1647,  1650,  1651,  1654,
    1655,  1660,  1665,  1669,  1674,  1679,  1684,  1689,  1694,  1699,
    1704,  1709,  1711,  1713,  1715,  1719,  1722,  1726,  1731,  1734,
    1738,  1740,  1743,  1745,  1748,  1750,  1752,  1754,  1756,  1758,
    1760,  1765,  1770,  1773,  1782,  1793,  1796,  1798,  1802,  1804,
    1807,  1809,  1811,  1813,  1815,  1818,  1823,  1827,  1831,  1836,
    1838,  1841,  1846,  1849,  1856,  1857,  1859,  1864,  1865,  1868,
    1869,  1871,  1873,  1877,  1879,  1883,  1885,  1887,  1891,  1895,
    1897,  1899,  1901,  1903,  1905,  1907,  1909,  1911,  1913,  1915,
    1917,  1919,  1921,  1923,  1925,  1927,  1929,  1931,  1933,  1935,
    1937,  1939,  1941,  1943,  1945,  1947,  1949,  1951,  1953,  1955,
    1957,  1959,  1961,  1963,  1965,  1967,  1969,  1971,  1973,  1975,
    1977,  1979,  1981,  1983,  1985,  1987,  1989,  1991,  1993,  1995,
    1997,  1999,  2001,  2003,  2005,  2007,  2009,  2011,  2013,  2015,
    2017,  2019,  2021,  2023,  2025,  2027,  2029,  2031,  2033,  2035,
    2037,  2039,  2041,  2043,  2045,  2047,  2049,  2051,  2053,  2055,
    2060,  2062,  2064,  2066,  2068,  2070,  2072,  2074,  2076,  2079,
    2081,  2082,  2083,  2085,  2087,  2091,  2092,  2094,  2096,  2098,
    2100,  2102,  2104,  2106,  2108,  2110,  2112,  2114,  2116,  2118,
    2122,  2125,  2127,  2129,  2134,  2138,  2143,  2145,  2147,  2149,
    2153,  2157,  2161,  2165,  2169,  2173,  2177,  2181,  2185,  2189,
    2193,  2197,  2201,  2205,  2209,  2213,  2217,  2221,  2224,  2227,
    2230,  2233,  2237,  2241,  2245,  2249,  2253,  2257,  2261,  2265,
    2271,  2276,  2280,  2284,  2288,  2290,  2292,  2294,  2296,  2300,
    2304,  2308,  2311,  2312,  2314,  2315,  2317,  2318,  2324,  2328,
    2332,  2334,  2336,  2338,  2340,  2342,  2346,  2349,  2351,  2353,
    2355,  2357,  2359,  2361,  2364,  2367,  2372,  2376,  2381,  2384,
    2385,  2391,  2395,  2399,  2401,  2405,  2407,  2410,  2411,  2417,
    2421,  2424,  2425,  2429,  2430,  2435,  2438,  2439,  2443,  2447,
    2449,  2450,  2452,  2455,  2458,  2463,  2467,  2471,  2474,  2479,
    2482,  2487,  2489,  2491,  2493,  2495,  2497,  2500,  2505,  2509,
    2514,  2518,  2520,  2522,  2524,  2526,  2529,  2534,  2539,  2543,
    2545,  2547,  2551,  2559,  2566,  2575,  2585,  2594,  2605,  2613,
    2620,  2629,  2631,  2634,  2639,  2644,  2646,  2648,  2653,  2655,
    2656,  2658,  2661,  2663,  2665,  2668,  2673,  2677,  2681,  2682,
    2684,  2687,  2692,  2696,  2699,  2703,  2710,  2711,  2713,  2718,
    2721,  2722,  2728,  2732,  2736,  2738,  2745,  2750,  2755,  2758,
    2761,  2762,  2768,  2772,  2776,  2778,  2781,  2782,  2788,  2792,
    2796,  2798,  2801,  2802,  2808,  2812,  2815,  2816,  2822,  2826,
    2829,  2832,  2834,  2837,  2839,  2844,  2848,  2852,  2859,  2863,
    2865,  2867,  2869,  2874,  2879,  2884,  2889,  2892,  2895,  2900,
    2903,  2906,  2908,  2912,  2916,  2920,  2921,  2924,  2930,  2937,
    2939,  2942,  2944,  2949,  2953,  2954,  2956,  2960,  2963,  2967,
    2969,  2971,  2972,  2973,  2976,  2980,  2982,  2988,  2992,  2996,
    3002,  3006,  3008,  3011,  3012,  3017,  3020,  3023,  3025,  3027,
    3029,  3031,  3036,  3043,  3045,  3054,  3061,  3063
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     209,     0,    -1,    -1,   210,   211,    -1,   211,   212,    -1,
      -1,   230,    -1,   247,    -1,   254,    -1,   251,    -1,   259,
      -1,   459,    -1,   122,   198,   199,   200,    -1,   148,   222,
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
     462,    -1,   223,   462,    -1,   227,     9,   460,    14,   402,
      -1,   105,   460,    14,   402,    -1,   228,   229,    -1,    -1,
     230,    -1,   247,    -1,   254,    -1,   259,    -1,   201,   228,
     202,    -1,    70,   330,   230,   281,   283,    -1,    70,   330,
      30,   228,   282,   284,    73,   200,    -1,    -1,    88,   330,
     231,   275,    -1,    -1,    87,   232,   230,    88,   330,   200,
      -1,    -1,    90,   198,   332,   200,   332,   200,   332,   199,
     233,   273,    -1,    -1,    97,   330,   234,   278,    -1,   101,
     200,    -1,   101,   339,   200,    -1,   103,   200,    -1,   103,
     339,   200,    -1,   106,   200,    -1,   106,   339,   200,    -1,
      27,   101,   200,    -1,   111,   291,   200,    -1,   117,   293,
     200,    -1,    86,   331,   200,    -1,   119,   198,   456,   199,
     200,    -1,   200,    -1,    81,    -1,    -1,    92,   198,   339,
      96,   272,   271,   199,   235,   274,    -1,    -1,    92,   198,
     339,    28,    96,   272,   271,   199,   236,   274,    -1,    94,
     198,   277,   199,   276,    -1,    -1,   107,   239,   108,   198,
     395,    79,   199,   201,   228,   202,   241,   237,   244,    -1,
      -1,   107,   239,   165,   238,   242,    -1,   109,   339,   200,
      -1,   102,   215,   200,    -1,   339,   200,    -1,   333,   200,
      -1,   334,   200,    -1,   335,   200,    -1,   336,   200,    -1,
     337,   200,    -1,   106,   336,   200,    -1,   338,   200,    -1,
     365,   200,    -1,   106,   364,   200,    -1,   215,    30,    -1,
      -1,   201,   240,   228,   202,    -1,   241,   108,   198,   395,
      79,   199,   201,   228,   202,    -1,    -1,    -1,   201,   243,
     228,   202,    -1,   165,   242,    -1,    -1,    35,    -1,    -1,
     104,    -1,    -1,   246,   245,   461,   248,   198,   287,   199,
     466,   319,    -1,    -1,   323,   246,   245,   461,   249,   198,
     287,   199,   466,   319,    -1,    -1,   422,   322,   246,   245,
     461,   250,   198,   287,   199,   466,   319,    -1,    -1,   158,
     215,   252,    30,   472,   458,   201,   294,   202,    -1,    -1,
     422,   158,   215,   253,    30,   472,   458,   201,   294,   202,
      -1,    -1,   265,   262,   255,   266,   267,   201,   297,   202,
      -1,    -1,   422,   265,   262,   256,   266,   267,   201,   297,
     202,    -1,    -1,   124,   263,   257,   268,   201,   297,   202,
      -1,    -1,   422,   124,   263,   258,   268,   201,   297,   202,
      -1,    -1,   160,   264,   260,   267,   201,   297,   202,    -1,
      -1,   422,   160,   264,   261,   267,   201,   297,   202,    -1,
     461,    -1,   152,    -1,   461,    -1,   461,    -1,   123,    -1,
     116,   123,    -1,   115,   123,    -1,   125,   395,    -1,    -1,
     126,   269,    -1,    -1,   125,   269,    -1,    -1,   395,    -1,
     269,     9,   395,    -1,   395,    -1,   270,     9,   395,    -1,
     128,   272,    -1,    -1,   429,    -1,    35,   429,    -1,   129,
     198,   441,   199,    -1,   230,    -1,    30,   228,    91,   200,
      -1,   230,    -1,    30,   228,    93,   200,    -1,   230,    -1,
      30,   228,    89,   200,    -1,   230,    -1,    30,   228,    95,
     200,    -1,   215,    14,   402,    -1,   277,     9,   215,    14,
     402,    -1,   201,   279,   202,    -1,   201,   200,   279,   202,
      -1,    30,   279,    98,   200,    -1,    30,   200,   279,    98,
     200,    -1,   279,    99,   339,   280,   228,    -1,   279,   100,
     280,   228,    -1,    -1,    30,    -1,   200,    -1,   281,    71,
     330,   230,    -1,    -1,   282,    71,   330,    30,   228,    -1,
      -1,    72,   230,    -1,    -1,    72,    30,   228,    -1,    -1,
     286,     9,   423,   325,   473,   161,    79,    -1,   286,     9,
     423,   325,   473,   161,    -1,   286,   407,    -1,   423,   325,
     473,   161,    79,    -1,   423,   325,   473,   161,    -1,    -1,
     423,   325,   473,    79,    -1,   423,   325,   473,    35,    79,
      -1,   423,   325,   473,    35,    79,    14,   402,    -1,   423,
     325,   473,    79,    14,   402,    -1,   286,     9,   423,   325,
     473,    79,    -1,   286,     9,   423,   325,   473,    35,    79,
      -1,   286,     9,   423,   325,   473,    35,    79,    14,   402,
      -1,   286,     9,   423,   325,   473,    79,    14,   402,    -1,
     288,     9,   423,   473,   161,    79,    -1,   288,     9,   423,
     473,   161,    -1,   288,   407,    -1,   423,   473,   161,    79,
      -1,   423,   473,   161,    -1,    -1,   423,   473,    79,    -1,
     423,   473,    35,    79,    -1,   423,   473,    35,    79,    14,
     402,    -1,   423,   473,    79,    14,   402,    -1,   288,     9,
     423,   473,    79,    -1,   288,     9,   423,   473,    35,    79,
      -1,   288,     9,   423,   473,    35,    79,    14,   402,    -1,
     288,     9,   423,   473,    79,    14,   402,    -1,   290,   407,
      -1,    -1,   339,    -1,    35,   429,    -1,   161,   339,    -1,
     290,     9,   339,    -1,   290,     9,   161,   339,    -1,   290,
       9,    35,   429,    -1,   291,     9,   292,    -1,   292,    -1,
      79,    -1,   203,   429,    -1,   203,   201,   339,   202,    -1,
     293,     9,    79,    -1,   293,     9,    79,    14,   402,    -1,
      79,    -1,    79,    14,   402,    -1,   294,   295,    -1,    -1,
     296,   200,    -1,   460,    14,   402,    -1,   297,   298,    -1,
      -1,    -1,   321,   299,   327,   200,    -1,    -1,   323,   472,
     300,   327,   200,    -1,   328,   200,    -1,    -1,   322,   246,
     245,   461,   198,   301,   285,   199,   466,   320,    -1,    -1,
     422,   322,   246,   245,   461,   198,   302,   285,   199,   466,
     320,    -1,   154,   307,   200,    -1,   155,   313,   200,    -1,
     157,   315,   200,    -1,     4,   125,   395,   200,    -1,     4,
     126,   395,   200,    -1,   110,   270,   200,    -1,   110,   270,
     201,   303,   202,    -1,   303,   304,    -1,   303,   305,    -1,
      -1,   226,   147,   215,   162,   270,   200,    -1,   306,    96,
     322,   215,   200,    -1,   306,    96,   323,   200,    -1,   226,
     147,   215,    -1,   215,    -1,   308,    -1,   307,     9,   308,
      -1,   309,   392,   311,   312,    -1,   152,    -1,   130,    -1,
     395,    -1,   118,    -1,   158,   201,   310,   202,    -1,   131,
      -1,   401,    -1,   310,     9,   401,    -1,    14,   402,    -1,
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
     402,    -1,    79,    -1,    79,    14,   402,    -1,   328,     9,
     460,    14,   402,    -1,   105,   460,    14,   402,    -1,   198,
     329,   199,    -1,    68,   397,   400,    -1,    67,   339,    -1,
     384,    -1,   358,    -1,   198,   339,   199,    -1,   331,     9,
     339,    -1,   339,    -1,   331,    -1,    -1,    27,   339,    -1,
      27,   339,   128,   339,    -1,   429,    14,   333,    -1,   129,
     198,   441,   199,    14,   333,    -1,    28,   339,    -1,   429,
      14,   336,    -1,   129,   198,   441,   199,    14,   336,    -1,
     340,    -1,   429,    -1,   329,    -1,   129,   198,   441,   199,
      14,   339,    -1,   429,    14,   339,    -1,   429,    14,    35,
     429,    -1,   429,    14,    35,    68,   397,   400,    -1,   429,
      26,   339,    -1,   429,    25,   339,    -1,   429,    24,   339,
      -1,   429,    23,   339,    -1,   429,    22,   339,    -1,   429,
      21,   339,    -1,   429,    20,   339,    -1,   429,    19,   339,
      -1,   429,    18,   339,    -1,   429,    17,   339,    -1,   429,
      16,   339,    -1,   429,    15,   339,    -1,   429,    64,    -1,
      64,   429,    -1,   429,    63,    -1,    63,   429,    -1,   339,
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
      -1,   339,    53,   397,    -1,   198,   340,   199,    -1,   339,
      29,   339,    30,   339,    -1,   339,    29,    30,   339,    -1,
     455,    -1,    62,   339,    -1,    61,   339,    -1,    60,   339,
      -1,    59,   339,    -1,    58,   339,    -1,    57,   339,    -1,
      56,   339,    -1,    69,   398,    -1,    55,   339,    -1,   404,
      -1,   357,    -1,   356,    -1,   359,    -1,   204,   399,   204,
      -1,    13,   339,    -1,   342,    -1,   345,    -1,   362,    -1,
     110,   198,   383,   407,   199,    -1,    -1,    -1,   246,   245,
     198,   343,   287,   199,   466,   341,   201,   228,   202,    -1,
      -1,   323,   246,   245,   198,   344,   287,   199,   466,   341,
     201,   228,   202,    -1,    -1,    79,   346,   350,    -1,    -1,
     180,    79,   347,   350,    -1,    -1,   195,   348,   287,   196,
     466,   350,    -1,    -1,   180,   195,   349,   287,   196,   466,
     350,    -1,     8,   339,    -1,     8,   336,    -1,     8,   201,
     228,   202,    -1,    85,    -1,   457,    -1,   352,     9,   351,
     128,   339,    -1,   351,   128,   339,    -1,   353,     9,   351,
     128,   402,    -1,   351,   128,   402,    -1,   352,   406,    -1,
      -1,   353,   406,    -1,    -1,   172,   198,   354,   199,    -1,
     130,   198,   442,   199,    -1,    66,   442,   205,    -1,   395,
     201,   444,   202,    -1,   173,   198,   448,   199,    -1,   174,
     198,   448,   199,    -1,   173,   198,   450,   199,    -1,   174,
     198,   450,   199,    -1,   395,   201,   446,   202,    -1,   362,
      66,   437,   205,    -1,   363,    66,   437,   205,    -1,   357,
      -1,   457,    -1,    85,    -1,   198,   340,   199,    -1,   366,
     367,    -1,   429,    14,   364,    -1,   181,    79,   184,   339,
      -1,   368,   379,    -1,   368,   379,   382,    -1,   379,    -1,
     379,   382,    -1,   369,    -1,   368,   369,    -1,   370,    -1,
     371,    -1,   372,    -1,   373,    -1,   374,    -1,   375,    -1,
     181,    79,   184,   339,    -1,   188,    79,    14,   339,    -1,
     182,   339,    -1,   183,    79,   184,   339,   185,   339,   186,
     339,    -1,   183,    79,   184,   339,   185,   339,   186,   339,
     187,    79,    -1,   189,   376,    -1,   377,    -1,   376,     9,
     377,    -1,   339,    -1,   339,   378,    -1,   190,    -1,   191,
      -1,   380,    -1,   381,    -1,   192,   339,    -1,   193,   339,
     194,   339,    -1,   187,    79,   367,    -1,   383,     9,    79,
      -1,   383,     9,    35,    79,    -1,    79,    -1,    35,    79,
      -1,   166,   152,   385,   167,    -1,   387,    50,    -1,   387,
     167,   388,   166,    50,   386,    -1,    -1,   152,    -1,   387,
     389,    14,   390,    -1,    -1,   388,   391,    -1,    -1,   152,
      -1,   153,    -1,   201,   339,   202,    -1,   153,    -1,   201,
     339,   202,    -1,   384,    -1,   393,    -1,   392,    30,   393,
      -1,   392,    47,   393,    -1,   215,    -1,    69,    -1,   104,
      -1,   105,    -1,   106,    -1,    27,    -1,    28,    -1,   107,
      -1,   108,    -1,   165,    -1,   109,    -1,    70,    -1,    71,
      -1,    73,    -1,    72,    -1,    88,    -1,    89,    -1,    87,
      -1,    90,    -1,    91,    -1,    92,    -1,    93,    -1,    94,
      -1,    95,    -1,    53,    -1,    96,    -1,    97,    -1,    98,
      -1,    99,    -1,   100,    -1,   101,    -1,   103,    -1,   102,
      -1,    86,    -1,    13,    -1,   123,    -1,   124,    -1,   125,
      -1,   126,    -1,    68,    -1,    67,    -1,   118,    -1,     5,
      -1,     7,    -1,     6,    -1,     4,    -1,     3,    -1,   148,
      -1,   110,    -1,   111,    -1,   120,    -1,   121,    -1,   122,
      -1,   117,    -1,   116,    -1,   115,    -1,   114,    -1,   113,
      -1,   112,    -1,   180,    -1,   119,    -1,   129,    -1,   130,
      -1,    10,    -1,    12,    -1,    11,    -1,   132,    -1,   134,
      -1,   133,    -1,   135,    -1,   136,    -1,   150,    -1,   149,
      -1,   179,    -1,   160,    -1,   163,    -1,   162,    -1,   175,
      -1,   177,    -1,   172,    -1,   225,   198,   289,   199,    -1,
     226,    -1,   152,    -1,   395,    -1,   117,    -1,   435,    -1,
     395,    -1,   117,    -1,   439,    -1,   198,   199,    -1,   330,
      -1,    -1,    -1,    84,    -1,   452,    -1,   198,   289,   199,
      -1,    -1,    74,    -1,    75,    -1,    76,    -1,    85,    -1,
     135,    -1,   136,    -1,   150,    -1,   132,    -1,   163,    -1,
     133,    -1,   134,    -1,   149,    -1,   179,    -1,   143,    84,
     144,    -1,   143,   144,    -1,   401,    -1,   224,    -1,   130,
     198,   405,   199,    -1,    66,   405,   205,    -1,   172,   198,
     355,   199,    -1,   403,    -1,   361,    -1,   360,    -1,   198,
     402,   199,    -1,   402,    31,   402,    -1,   402,    32,   402,
      -1,   402,    10,   402,    -1,   402,    12,   402,    -1,   402,
      11,   402,    -1,   402,    33,   402,    -1,   402,    35,   402,
      -1,   402,    34,   402,    -1,   402,    48,   402,    -1,   402,
      46,   402,    -1,   402,    47,   402,    -1,   402,    49,   402,
      -1,   402,    50,   402,    -1,   402,    51,   402,    -1,   402,
      45,   402,    -1,   402,    44,   402,    -1,   402,    65,   402,
      -1,    52,   402,    -1,    54,   402,    -1,    46,   402,    -1,
      47,   402,    -1,   402,    37,   402,    -1,   402,    36,   402,
      -1,   402,    39,   402,    -1,   402,    38,   402,    -1,   402,
      40,   402,    -1,   402,    43,   402,    -1,   402,    41,   402,
      -1,   402,    42,   402,    -1,   402,    29,   402,    30,   402,
      -1,   402,    29,    30,   402,    -1,   226,   147,   215,    -1,
     152,   147,   215,    -1,   226,   147,   123,    -1,   224,    -1,
      78,    -1,   457,    -1,   401,    -1,   206,   452,   206,    -1,
     207,   452,   207,    -1,   143,   452,   144,    -1,   408,   406,
      -1,    -1,     9,    -1,    -1,     9,    -1,    -1,   408,     9,
     402,   128,   402,    -1,   408,     9,   402,    -1,   402,   128,
     402,    -1,   402,    -1,    74,    -1,    75,    -1,    76,    -1,
      85,    -1,   143,    84,   144,    -1,   143,   144,    -1,    74,
      -1,    75,    -1,    76,    -1,   215,    -1,   409,    -1,   215,
      -1,    46,   410,    -1,    47,   410,    -1,   130,   198,   412,
     199,    -1,    66,   412,   205,    -1,   172,   198,   415,   199,
      -1,   413,   406,    -1,    -1,   413,     9,   411,   128,   411,
      -1,   413,     9,   411,    -1,   411,   128,   411,    -1,   411,
      -1,   414,     9,   411,    -1,   411,    -1,   416,   406,    -1,
      -1,   416,     9,   351,   128,   411,    -1,   351,   128,   411,
      -1,   414,   406,    -1,    -1,   198,   417,   199,    -1,    -1,
     419,     9,   215,   418,    -1,   215,   418,    -1,    -1,   421,
     419,   406,    -1,    45,   420,    44,    -1,   422,    -1,    -1,
     425,    -1,   127,   434,    -1,   127,   215,    -1,   127,   201,
     339,   202,    -1,    66,   437,   205,    -1,   201,   339,   202,
      -1,   430,   426,    -1,   198,   329,   199,   426,    -1,   440,
     426,    -1,   198,   329,   199,   426,    -1,   434,    -1,   394,
      -1,   432,    -1,   433,    -1,   427,    -1,   429,   424,    -1,
     198,   329,   199,   424,    -1,   396,   147,   434,    -1,   431,
     198,   289,   199,    -1,   198,   429,   199,    -1,   394,    -1,
     432,    -1,   433,    -1,   427,    -1,   429,   425,    -1,   198,
     329,   199,   425,    -1,   431,   198,   289,   199,    -1,   198,
     429,   199,    -1,   434,    -1,   427,    -1,   198,   429,   199,
      -1,   429,   127,   215,   462,   198,   289,   199,    -1,   429,
     127,   434,   198,   289,   199,    -1,   429,   127,   201,   339,
     202,   198,   289,   199,    -1,   198,   329,   199,   127,   215,
     462,   198,   289,   199,    -1,   198,   329,   199,   127,   434,
     198,   289,   199,    -1,   198,   329,   199,   127,   201,   339,
     202,   198,   289,   199,    -1,   396,   147,   215,   462,   198,
     289,   199,    -1,   396,   147,   434,   198,   289,   199,    -1,
     396,   147,   201,   339,   202,   198,   289,   199,    -1,   435,
      -1,   438,   435,    -1,   435,    66,   437,   205,    -1,   435,
     201,   339,   202,    -1,   436,    -1,    79,    -1,   203,   201,
     339,   202,    -1,   339,    -1,    -1,   203,    -1,   438,   203,
      -1,   434,    -1,   428,    -1,   439,   424,    -1,   198,   329,
     199,   424,    -1,   396,   147,   434,    -1,   198,   429,   199,
      -1,    -1,   428,    -1,   439,   425,    -1,   198,   329,   199,
     425,    -1,   198,   429,   199,    -1,   441,     9,    -1,   441,
       9,   429,    -1,   441,     9,   129,   198,   441,   199,    -1,
      -1,   429,    -1,   129,   198,   441,   199,    -1,   443,   406,
      -1,    -1,   443,     9,   339,   128,   339,    -1,   443,     9,
     339,    -1,   339,   128,   339,    -1,   339,    -1,   443,     9,
     339,   128,    35,   429,    -1,   443,     9,    35,   429,    -1,
     339,   128,    35,   429,    -1,    35,   429,    -1,   445,   406,
      -1,    -1,   445,     9,   339,   128,   339,    -1,   445,     9,
     339,    -1,   339,   128,   339,    -1,   339,    -1,   447,   406,
      -1,    -1,   447,     9,   402,   128,   402,    -1,   447,     9,
     402,    -1,   402,   128,   402,    -1,   402,    -1,   449,   406,
      -1,    -1,   449,     9,   339,   128,   339,    -1,   339,   128,
     339,    -1,   451,   406,    -1,    -1,   451,     9,   402,   128,
     402,    -1,   402,   128,   402,    -1,   452,   453,    -1,   452,
      84,    -1,   453,    -1,    84,   453,    -1,    79,    -1,    79,
      66,   454,   205,    -1,    79,   127,   215,    -1,   145,   339,
     202,    -1,   145,    78,    66,   339,   205,   202,    -1,   146,
     429,   202,    -1,   215,    -1,    80,    -1,    79,    -1,   120,
     198,   456,   199,    -1,   121,   198,   429,   199,    -1,   121,
     198,   340,   199,    -1,   121,   198,   329,   199,    -1,     7,
     339,    -1,     6,   339,    -1,     5,   198,   339,   199,    -1,
       4,   339,    -1,     3,   339,    -1,   429,    -1,   456,     9,
     429,    -1,   396,   147,   215,    -1,   396,   147,   123,    -1,
      -1,    96,   472,    -1,   175,   461,    14,   472,   200,    -1,
     177,   461,   458,    14,   472,   200,    -1,   215,    -1,   472,
     215,    -1,   215,    -1,   215,   168,   467,   169,    -1,   168,
     464,   169,    -1,    -1,   472,    -1,   463,     9,   472,    -1,
     463,   406,    -1,   463,     9,   161,    -1,   464,    -1,   161,
      -1,    -1,    -1,    30,   472,    -1,   467,     9,   215,    -1,
     215,    -1,   467,     9,   215,    96,   472,    -1,   215,    96,
     472,    -1,    85,   128,   472,    -1,   226,   147,   215,   128,
     472,    -1,   469,     9,   468,    -1,   468,    -1,   469,   406,
      -1,    -1,   172,   198,   470,   199,    -1,    29,   472,    -1,
      55,   472,    -1,   226,    -1,   130,    -1,   131,    -1,   471,
      -1,   130,   168,   472,   169,    -1,   130,   168,   472,     9,
     472,   169,    -1,   152,    -1,   198,   104,   198,   465,   199,
      30,   472,   199,    -1,   198,   472,     9,   463,   406,   199,
      -1,   472,    -1,    -1
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
    1858,  1874,  1873,  1891,  1891,  1907,  1906,  1927,  1928,  1929,
    1934,  1936,  1940,  1944,  1950,  1954,  1960,  1962,  1966,  1968,
    1972,  1976,  1977,  1981,  1988,  1989,  1993,  1994,  1999,  2006,
    2008,  2013,  2014,  2015,  2017,  2021,  2025,  2029,  2033,  2035,
    2037,  2039,  2044,  2045,  2050,  2051,  2052,  2053,  2054,  2055,
    2059,  2063,  2067,  2071,  2076,  2081,  2085,  2086,  2090,  2091,
    2095,  2096,  2100,  2101,  2105,  2109,  2113,  2117,  2118,  2119,
    2120,  2124,  2130,  2139,  2152,  2153,  2156,  2159,  2162,  2163,
    2166,  2170,  2173,  2176,  2183,  2184,  2188,  2189,  2191,  2195,
    2196,  2197,  2198,  2199,  2200,  2201,  2202,  2203,  2204,  2205,
    2206,  2207,  2208,  2209,  2210,  2211,  2212,  2213,  2214,  2215,
    2216,  2217,  2218,  2219,  2220,  2221,  2222,  2223,  2224,  2225,
    2226,  2227,  2228,  2229,  2230,  2231,  2232,  2233,  2234,  2235,
    2236,  2237,  2238,  2239,  2240,  2241,  2242,  2243,  2244,  2245,
    2246,  2247,  2248,  2249,  2250,  2251,  2252,  2253,  2254,  2255,
    2256,  2257,  2258,  2259,  2260,  2261,  2262,  2263,  2264,  2265,
    2266,  2267,  2268,  2269,  2270,  2271,  2272,  2273,  2274,  2278,
    2283,  2284,  2287,  2288,  2289,  2293,  2294,  2295,  2299,  2300,
    2301,  2305,  2306,  2307,  2310,  2312,  2316,  2317,  2318,  2319,
    2321,  2322,  2323,  2324,  2325,  2326,  2327,  2328,  2329,  2330,
    2333,  2338,  2339,  2340,  2342,  2343,  2345,  2346,  2347,  2349,
    2350,  2352,  2354,  2356,  2358,  2360,  2361,  2362,  2363,  2364,
    2365,  2366,  2367,  2368,  2369,  2370,  2371,  2372,  2373,  2374,
    2375,  2376,  2378,  2380,  2382,  2384,  2385,  2388,  2389,  2393,
    2395,  2399,  2402,  2405,  2411,  2412,  2413,  2414,  2415,  2416,
    2417,  2422,  2424,  2428,  2429,  2432,  2433,  2437,  2440,  2442,
    2444,  2448,  2449,  2450,  2451,  2453,  2456,  2460,  2461,  2462,
    2463,  2466,  2467,  2468,  2469,  2470,  2472,  2473,  2478,  2480,
    2483,  2486,  2488,  2490,  2493,  2495,  2499,  2501,  2504,  2507,
    2513,  2515,  2518,  2519,  2524,  2527,  2531,  2531,  2536,  2539,
    2540,  2544,  2545,  2550,  2551,  2555,  2556,  2560,  2561,  2566,
    2568,  2573,  2574,  2575,  2576,  2577,  2578,  2579,  2581,  2584,
    2586,  2590,  2591,  2592,  2593,  2594,  2596,  2598,  2600,  2604,
    2605,  2606,  2610,  2613,  2616,  2619,  2623,  2627,  2634,  2638,
    2642,  2649,  2650,  2655,  2657,  2658,  2661,  2662,  2665,  2666,
    2670,  2671,  2675,  2676,  2677,  2678,  2680,  2683,  2686,  2687,
    2688,  2690,  2692,  2696,  2697,  2698,  2700,  2701,  2702,  2706,
    2708,  2711,  2713,  2714,  2715,  2716,  2719,  2721,  2722,  2726,
    2728,  2731,  2733,  2734,  2735,  2739,  2741,  2744,  2747,  2749,
    2751,  2755,  2757,  2760,  2762,  2766,  2768,  2771,  2774,  2779,
    2780,  2782,  2783,  2789,  2790,  2792,  2794,  2796,  2798,  2801,
    2802,  2803,  2807,  2808,  2809,  2810,  2811,  2812,  2813,  2814,
    2815,  2819,  2820,  2824,  2826,  2834,  2836,  2840,  2844,  2851,
    2852,  2858,  2859,  2866,  2869,  2873,  2876,  2881,  2886,  2888,
    2889,  2890,  2894,  2895,  2899,  2901,  2902,  2904,  2908,  2911,
    2920,  2922,  2926,  2929,  2932,  2940,  2943,  2946,  2947,  2950,
    2953,  2954,  2957,  2961,  2965,  2971,  2981,  2982
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
  "$@30", "lambda_expression", "$@31", "$@32", "$@33", "$@34",
  "lambda_body", "shape_keyname", "non_empty_shape_pair_list",
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
     345,   347,   345,   348,   345,   349,   345,   350,   350,   350,
     351,   351,   352,   352,   353,   353,   354,   354,   355,   355,
     356,   357,   357,   358,   359,   359,   360,   360,   361,   362,
     362,   363,   363,   363,   363,   364,   365,   366,   367,   367,
     367,   367,   368,   368,   369,   369,   369,   369,   369,   369,
     370,   371,   372,   373,   374,   375,   376,   376,   377,   377,
     378,   378,   379,   379,   380,   381,   382,   383,   383,   383,
     383,   384,   385,   385,   386,   386,   387,   387,   388,   388,
     389,   390,   390,   391,   391,   391,   392,   392,   392,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   394,
     395,   395,   396,   396,   396,   397,   397,   397,   398,   398,
     398,   399,   399,   399,   400,   400,   401,   401,   401,   401,
     401,   401,   401,   401,   401,   401,   401,   401,   401,   401,
     401,   402,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   403,   403,   403,   404,   404,   404,   404,   404,   404,
     404,   405,   405,   406,   406,   407,   407,   408,   408,   408,
     408,   409,   409,   409,   409,   409,   409,   410,   410,   410,
     410,   411,   411,   411,   411,   411,   411,   411,   412,   412,
     413,   413,   413,   413,   414,   414,   415,   415,   416,   416,
     417,   417,   418,   418,   419,   419,   421,   420,   422,   423,
     423,   424,   424,   425,   425,   426,   426,   427,   427,   428,
     428,   429,   429,   429,   429,   429,   429,   429,   429,   429,
     429,   430,   430,   430,   430,   430,   430,   430,   430,   431,
     431,   431,   432,   432,   432,   432,   432,   432,   433,   433,
     433,   434,   434,   435,   435,   435,   436,   436,   437,   437,
     438,   438,   439,   439,   439,   439,   439,   439,   440,   440,
     440,   440,   440,   441,   441,   441,   441,   441,   441,   442,
     442,   443,   443,   443,   443,   443,   443,   443,   443,   444,
     444,   445,   445,   445,   445,   446,   446,   447,   447,   447,
     447,   448,   448,   449,   449,   450,   450,   451,   451,   452,
     452,   452,   452,   453,   453,   453,   453,   453,   453,   454,
     454,   454,   455,   455,   455,   455,   455,   455,   455,   455,
     455,   456,   456,   457,   457,   458,   458,   459,   459,   460,
     460,   461,   461,   462,   462,   463,   463,   464,   465,   465,
     465,   465,   466,   466,   467,   467,   467,   467,   468,   468,
     469,   469,   470,   470,   471,   472,   472,   472,   472,   472,
     472,   472,   472,   472,   472,   472,   473,   473
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
       3,     0,     4,     0,     6,     0,     7,     2,     2,     4,
       1,     1,     5,     3,     5,     3,     2,     0,     2,     0,
       4,     4,     3,     4,     4,     4,     4,     4,     4,     4,
       4,     1,     1,     1,     3,     2,     3,     4,     2,     3,
       1,     2,     1,     2,     1,     1,     1,     1,     1,     1,
       4,     4,     2,     8,    10,     2,     1,     3,     1,     2,
       1,     1,     1,     1,     2,     4,     3,     3,     4,     1,
       2,     4,     2,     6,     0,     1,     4,     0,     2,     0,
       1,     1,     3,     1,     3,     1,     1,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     1,
       0,     0,     1,     1,     3,     0,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       2,     1,     1,     4,     3,     4,     1,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     5,
       4,     3,     3,     3,     1,     1,     1,     1,     3,     3,
       3,     2,     0,     1,     0,     1,     0,     5,     3,     3,
       1,     1,     1,     1,     1,     3,     2,     1,     1,     1,
       1,     1,     1,     2,     2,     4,     3,     4,     2,     0,
       5,     3,     3,     1,     3,     1,     2,     0,     5,     3,
       2,     0,     3,     0,     4,     2,     0,     3,     3,     1,
       0,     1,     2,     2,     4,     3,     3,     2,     4,     2,
       4,     1,     1,     1,     1,     1,     2,     4,     3,     4,
       3,     1,     1,     1,     1,     2,     4,     4,     3,     1,
       1,     3,     7,     6,     8,     9,     8,    10,     7,     6,
       8,     1,     2,     4,     4,     1,     1,     4,     1,     0,
       1,     2,     1,     1,     2,     4,     3,     3,     0,     1,
       2,     4,     3,     2,     3,     6,     0,     1,     4,     2,
       0,     5,     3,     3,     1,     6,     4,     4,     2,     2,
       0,     5,     3,     3,     1,     2,     0,     5,     3,     3,
       1,     2,     0,     5,     3,     2,     0,     5,     3,     2,
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
       0,     0,     0,   736,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   810,     0,
     798,   620,     0,   626,   627,   628,    22,   685,   786,    98,
     629,     0,    80,     0,     0,     0,     0,     0,     0,     0,
       0,   131,     0,     0,     0,     0,     0,     0,   323,   324,
     325,   328,   327,   326,     0,     0,     0,     0,   158,     0,
       0,     0,   633,   635,   636,   630,   631,     0,     0,   637,
     632,     0,   611,    23,    24,    25,    27,    26,     0,   634,
       0,     0,     0,     0,     0,     0,   638,   329,    28,    29,
      31,    30,    32,    33,    34,    35,    36,    37,    38,    39,
      40,   443,     0,    97,    70,   790,   621,     0,     0,     4,
      59,    61,    64,   684,     0,   610,     0,     6,   130,     7,
       9,     8,    10,     0,     0,   321,   360,     0,     0,     0,
       0,     0,     0,     0,   358,   430,   431,   426,   425,   345,
     427,   432,     0,     0,   344,   752,   612,     0,   687,   424,
     320,   755,   359,     0,     0,   753,   754,   751,   781,   785,
       0,   414,   686,    11,   328,   327,   326,     0,     0,    27,
      59,   130,     0,   860,   359,   859,     0,   857,   856,   429,
       0,   351,   355,     0,     0,   398,   399,   400,   401,   423,
     421,   420,   419,   418,   417,   416,   415,   786,   613,     0,
     874,   612,     0,   380,   378,     0,   814,     0,   694,   343,
     616,     0,   874,   615,     0,   625,   793,   792,   617,     0,
       0,   619,   422,     0,     0,     0,     0,   348,     0,    78,
     350,     0,     0,    84,    86,     0,     0,    88,     0,     0,
       0,   898,   899,   903,     0,     0,    59,   897,     0,   900,
       0,     0,    90,     0,     0,     0,     0,   121,     0,     0,
       0,     0,     0,     0,    42,    47,   241,     0,     0,   240,
     160,   159,   246,     0,     0,     0,     0,     0,   871,   146,
     156,   806,   810,   843,     0,   640,     0,     0,     0,   841,
       0,    16,     0,    63,   138,   150,   157,   517,   457,   832,
     832,     0,   865,   441,   445,   740,   360,     0,   358,   359,
       0,     0,   622,     0,   623,     0,     0,     0,   120,     0,
       0,    66,   232,     0,    21,   129,     0,   155,   142,   154,
     326,   329,   130,   322,   111,   112,   113,   114,   115,   117,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   798,     0,   110,   789,   789,
     118,   820,     0,     0,     0,     0,     0,     0,   319,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   379,   377,     0,   756,   741,   789,     0,   747,
     232,   789,     0,   791,   782,   806,     0,   130,     0,     0,
      92,     0,   738,   733,   694,     0,     0,     0,     0,   818,
       0,   462,   693,   809,     0,     0,    66,     0,   232,   342,
       0,   794,   741,   749,   618,     0,    70,   196,     0,   440,
       0,    95,     0,     0,   349,     0,     0,     0,     0,     0,
      87,   109,    89,   895,   896,     0,   893,     0,     0,     0,
     870,     0,   116,    91,   119,     0,     0,     0,     0,     0,
       0,     0,   475,     0,   482,   484,   485,   486,   487,   488,
     489,   480,   502,   503,    70,     0,   106,   108,     0,     0,
      44,    51,     0,     0,    46,    55,    48,     0,    18,     0,
       0,   242,     0,    93,     0,     0,    94,   861,     0,     0,
     360,   358,   359,     0,     0,   166,     0,   807,     0,     0,
       0,     0,   639,   842,   685,     0,     0,   840,   690,   839,
      62,     5,    13,    14,     0,   164,     0,     0,   450,     0,
       0,   694,     0,     0,   614,   451,     0,     0,   694,     0,
       0,     0,     0,     0,   740,     0,   696,   739,   907,   341,
     411,   760,    75,    69,    71,    72,    73,    74,   320,     0,
     428,   688,   689,    60,   694,     0,   875,     0,     0,     0,
     696,   233,     0,   435,   132,   162,     0,   383,   385,   384,
       0,     0,   381,   382,   386,   388,   387,   403,   402,   405,
     404,   406,   408,   409,   407,   397,   396,   390,   391,   389,
     392,   393,   395,   410,   394,   788,     0,     0,   824,     0,
     694,   864,     0,   863,   758,   781,   148,   140,   152,   144,
     130,     0,     0,   353,   356,   362,   476,   376,   375,   374,
     373,   372,   371,   370,   369,   368,   367,   366,   365,     0,
     743,   742,     0,     0,     0,     0,     0,     0,     0,   858,
     352,   731,   735,   693,   737,     0,     0,   874,     0,   813,
       0,   812,     0,   797,   796,     0,     0,   743,   742,   346,
     198,   200,    70,   448,   447,   347,     0,    70,   180,    79,
     350,     0,     0,     0,     0,     0,   192,   192,    85,     0,
       0,     0,   891,   694,     0,   881,     0,     0,     0,     0,
       0,   692,   629,     0,     0,   611,     0,     0,     0,     0,
      64,   642,   610,   648,   647,     0,   641,    68,   646,     0,
       0,   492,     0,     0,   498,   495,   496,   504,     0,   483,
     478,     0,   481,     0,     0,     0,    52,    19,     0,     0,
      56,    20,     0,     0,     0,    41,    49,     0,   239,   247,
     244,     0,     0,   852,   855,   854,   853,    12,   885,     0,
       0,     0,   806,   803,     0,   461,   851,   850,   849,     0,
     845,     0,   846,   848,     0,     5,     0,     0,     0,   511,
     512,   520,   519,     0,     0,   693,   456,   460,     0,     0,
     464,   693,   831,   465,     0,   866,     0,   442,     0,   882,
     740,   219,   906,     0,     0,   757,   741,   748,   787,   693,
     877,   873,   234,   235,   609,   695,   231,     0,   740,     0,
       0,   164,   437,   134,   413,     0,   469,   470,     0,   463,
     693,   819,     0,     0,   232,   166,     0,   164,   162,     0,
     798,   363,     0,     0,   232,   745,   746,   759,   783,   784,
       0,     0,     0,   719,   701,   702,   703,   704,     0,     0,
       0,   712,   711,   725,   694,     0,   733,   817,   816,     0,
     795,   741,   750,   624,     0,   202,     0,     0,    76,     0,
       0,     0,     0,     0,     0,     0,   172,   173,   184,     0,
      70,   182,   103,   192,     0,   192,     0,     0,   901,     0,
       0,   693,   892,   894,   880,   694,   879,     0,   694,   669,
     670,   667,   668,   700,     0,   694,   692,     0,     0,   459,
     836,   836,     0,     0,   826,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   477,     0,     0,     0,   500,   501,   499,     0,     0,
     479,     0,   122,     0,   125,   107,     0,    43,    53,     0,
      45,    57,    50,   243,     0,   862,    96,     0,     0,   872,
     165,   167,   253,     0,     0,   804,     0,   844,     0,    17,
       0,   865,   163,   253,     0,     0,   453,     0,   863,   834,
       0,   867,     0,   882,     0,     0,   907,     0,   223,   221,
       0,   743,   742,   876,     0,     0,   236,    67,     0,   740,
     161,     0,   740,     0,   412,   823,   822,     0,   232,     0,
       0,     0,     0,   164,   136,   625,   744,   232,     0,     0,
     707,   708,   709,   710,   713,   714,   723,     0,   694,   719,
       0,   706,   727,   693,   730,   732,   734,     0,   811,   744,
       0,     0,     0,     0,   199,   449,    81,     0,   350,   172,
     174,   806,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   186,     0,   888,     0,   890,   693,     0,     0,     0,
     644,   693,   691,     0,   682,     0,   694,     0,     0,     0,
     694,     0,   649,   683,   681,   830,     0,   694,   652,   654,
     653,     0,     0,   650,   651,   655,   657,   656,   672,   671,
     674,   673,   675,   677,   678,   676,   665,   664,   659,   660,
     658,   661,   662,   663,   666,   490,     0,   491,   497,   505,
     506,     0,    70,    54,    58,   245,   887,   884,     0,   320,
     808,   806,   354,   357,   361,     0,    15,     0,   320,   523,
       0,     0,   525,   518,   521,     0,   516,     0,     0,   868,
       0,   883,   444,     0,   224,     0,   220,     0,     0,   232,
     238,   237,   882,     0,   253,     0,   740,     0,   232,     0,
     779,   253,   865,   253,     0,     0,   364,   232,     0,   773,
       0,   716,   693,   718,     0,   705,     0,     0,   694,   724,
     815,     0,    70,     0,   195,   181,     0,     0,     0,   171,
      99,   185,     0,     0,   188,     0,   193,   194,    70,   187,
     902,     0,   878,     0,   905,   699,   698,   643,     0,   693,
     458,   645,     0,   466,   693,   835,   467,     0,   468,   693,
     825,   680,     0,     0,     0,     0,     0,   168,     0,     0,
       0,   318,     0,     0,     0,   147,   252,   254,     0,   317,
       0,   320,     0,   847,   249,   151,   514,     0,     0,   452,
     833,   446,     0,   227,   218,     0,   226,   744,   232,     0,
     434,   882,   320,   882,     0,   821,     0,   778,   320,     0,
     320,   253,   740,     0,   772,   722,   721,   715,     0,   717,
     693,   726,    70,   201,    77,    82,   101,   175,     0,   183,
     189,    70,   191,   889,     0,     0,   455,     0,   838,     0,
     829,   828,   679,     0,    70,   126,   886,     0,     0,     0,
       0,   169,   284,   282,   286,   611,    27,     0,   278,     0,
     283,   295,     0,   293,   298,     0,   297,     0,   296,     0,
     130,   256,     0,   258,     0,   805,     0,   515,   513,   524,
     522,   228,     0,   217,   225,   232,     0,   776,     0,     0,
       0,   143,   434,   882,   780,   149,   249,   153,   320,     0,
     774,     0,   729,     0,   197,     0,     0,    70,   178,   100,
     190,   904,   697,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   268,   272,     0,     0,   263,   575,   574,   571,
     573,   572,   592,   594,   593,   563,   534,   535,   553,   569,
     568,   530,   540,   541,   543,   542,   562,   546,   544,   545,
     547,   548,   549,   550,   551,   552,   554,   555,   556,   557,
     558,   559,   561,   560,   531,   532,   533,   536,   537,   539,
     577,   578,   587,   586,   585,   584,   583,   582,   570,   589,
     579,   580,   581,   564,   565,   566,   567,   590,   591,   595,
     597,   596,   598,   599,   576,   601,   600,   603,   605,   604,
     538,   608,   606,   607,   602,   588,   529,   290,   526,     0,
     264,   311,   312,   310,   303,     0,   304,   265,   337,     0,
       0,     0,     0,   130,   139,   248,     0,     0,     0,   230,
       0,   775,     0,    70,   313,    70,   133,     0,     0,     0,
     145,   882,   720,     0,    70,   176,    83,   102,     0,   454,
     837,   827,   493,   124,   266,   267,   340,   170,     0,     0,
     287,   279,     0,     0,     0,   292,   294,     0,     0,   299,
     306,   307,   305,     0,     0,   255,     0,     0,     0,     0,
     250,     0,   229,   777,     0,   509,   696,     0,     0,    70,
     135,   141,     0,   728,     0,     0,     0,   104,   269,    59,
       0,   270,   271,     0,     0,   285,   289,   527,   528,     0,
     280,   308,   309,   301,   302,   300,   338,   335,   259,   257,
     339,     0,   251,   510,   695,     0,   436,   314,     0,   137,
       0,   179,   494,     0,   128,     0,   320,   288,   291,     0,
     740,   261,     0,   507,   433,   438,   177,     0,     0,   105,
     276,     0,   319,   336,     0,   696,   331,   740,   508,     0,
     127,     0,     0,   275,   882,   740,   205,   332,   333,   334,
     907,   330,     0,     0,     0,   274,     0,   331,     0,   882,
       0,   273,   315,    70,   260,   907,     0,   209,   207,     0,
      70,     0,     0,   210,     0,   206,   262,     0,   316,     0,
     213,   204,     0,   212,   123,   214,     0,   203,   211,     0,
     216,   215
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   119,   795,   541,   180,   273,   499,
     503,   274,   500,   504,   121,   122,   123,   124,   125,   126,
     320,   573,   574,   453,   238,  1415,   459,  1338,  1416,  1644,
     755,   268,   494,  1607,   985,  1162,  1659,   336,   181,   575,
     839,  1043,  1215,   130,   544,   856,   576,   595,   858,   525,
     855,   577,   545,   857,   338,   289,   305,   133,   841,   798,
     781,  1000,  1360,  1093,   906,  1556,  1419,   699,   912,   458,
     708,   914,  1248,   691,   895,   898,  1082,  1664,  1665,   565,
     566,   589,   590,   278,   279,   283,  1386,  1535,  1536,  1169,
    1286,  1379,  1531,  1650,  1667,  1568,  1611,  1612,  1613,  1367,
    1368,  1369,  1569,  1575,  1620,  1372,  1373,  1377,  1524,  1525,
    1526,  1546,  1694,  1287,  1288,   182,   135,  1680,  1681,  1529,
    1290,   136,   231,   454,   455,   137,   138,   139,   140,   141,
     142,   143,   144,  1399,   145,   838,  1042,   146,   235,   563,
     315,   564,   449,   550,   551,  1116,   552,  1117,   147,   148,
     149,   150,   733,   734,   151,   152,   265,   153,   266,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   745,   746,
     977,   491,   492,   493,   752,  1596,   154,   546,  1388,   547,
    1014,   803,  1186,  1183,  1517,  1518,   155,   156,   157,   225,
     232,   323,   439,   158,   933,   738,   159,   934,   830,   821,
     935,   882,  1064,  1066,  1067,  1068,   884,  1227,  1228,   885,
     672,   424,   193,   194,   578,   568,   405,   406,   827,   161,
     226,   184,   163,   164,   165,   166,   167,   168,   169,   626,
     170,   228,   229,   528,   217,   218,   629,   630,  1126,  1127,
     557,   558,  1119,  1120,   298,   299,   789,   171,   518,   172,
     562,   173,  1537,   290,   331,   584,   585,   927,  1025,   779,
     712,   713,   714,   259,   260,   823
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1432
static const yytype_int16 yypact[] =
{
   -1432,   108, -1432, -1432,  5667, 12965, 12965,   -88, 12965, 12965,
   12965, 11284, 12965, -1432, 12965, 12965, 12965, 12965, 12965, 12965,
   12965, 12965, 12965, 12965, 12965, 12965, 14741, 14741, 11448, 12965,
   14824,   -70,   -68, -1432, -1432, -1432, -1432, -1432,   124, -1432,
     128, 12965, -1432,   -68,   -55,   -17,     6,   -68, 11612, 15975,
   11776, -1432, 14138, 10382,     5, 12965, 15699,     0, -1432, -1432,
   -1432,   157,   173,    41,   171,   201,   213,   233, -1432, 15975,
     237,   253, -1432, -1432, -1432, -1432, -1432,   358, 14888, -1432,
   -1432, 15975, -1432, -1432, -1432, -1432, 15975, -1432, 15975, -1432,
     319,   257,   284,   292, 15975, 15975, -1432,    19, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432, -1432, 12965, -1432, -1432,   297,    52,   308,   308, -1432,
     434,   324,   265, -1432,   348, -1432,    32, -1432,   488, -1432,
   -1432, -1432, -1432, 15758,   418, -1432, -1432,   351,   364,   368,
     371,   373,   385,  4776, -1432, -1432, -1432, -1432,   482, -1432,
   -1432,   527,   528,   395, -1432,   109,   400,   455, -1432, -1432,
     463,   110,  1887,   118,   405,   119,   121,   408,    35, -1432,
     141, -1432,   548, -1432, -1432, -1432,   477,   428,   474, -1432,
   -1432,   488,   418, 16248,  2186, 16248, 12965, 16248, 16248,  4531,
     440, 15110,  4531,   598, 15975,   581,   581,    59,   581,   581,
     581,   581,   581,   581,   581,   581,   581, -1432, -1432,  4714,
     479, -1432,   501,   531,   531, 14741, 15154,   449,   646, -1432,
     477,  4714,   479,   512,   513,   466,   123, -1432,   538,   118,
   11940, -1432, -1432, 12965,  8947,   658,    57, 16248,  9972, -1432,
   12965, 12965, 15975, -1432, -1432,  4904,   468, -1432,  4951, 14138,
   14138,   502, -1432, -1432,   475, 13915,   664, -1432,   665, -1432,
   15975,   612, -1432,   492,  5014,   498,   755, -1432,   -31, 13519,
   15817, 15832, 15975,    60, -1432,   131, -1432, 14522,    62, -1432,
   -1432, -1432,   680,    65, 14741, 14741, 12965,   506,   534, -1432,
   -1432, 14605, 11448,   235,   316, -1432, 13129, 14741,   366, -1432,
   15975, -1432,   296,   324, -1432, -1432, -1432, -1432, 14438, 12965,
   12965,   692,   611, -1432, -1432,    44,   510, 16248,   514,   613,
    5872, 12965,   263,   508,   339,   263,   274,   261, -1432, 15975,
   14138,   522, 10546, 14138, -1432, -1432,  3043, -1432, -1432, -1432,
   -1432, -1432,   488, -1432, -1432, -1432, -1432, -1432, -1432, -1432,
   12965, 12965, 12965, 12145, 12965, 12965, 12965, 12965, 12965, 12965,
   12965, 12965, 12965, 12965, 12965, 12965, 12965, 12965, 12965, 12965,
   12965, 12965, 12965, 12965, 12965, 14824, 12965, -1432, 12965, 12965,
   -1432, 12965,  2659, 15975, 15975, 15975, 15758,   617,   503, 10177,
   12965, 12965, 12965, 12965, 12965, 12965, 12965, 12965, 12965, 12965,
   12965, 12965, -1432, -1432,  2993, -1432,   126, 12965, 12965, -1432,
   10546, 12965, 12965,   297,   127, 14605,   524,   488, 12309, 13563,
   -1432, 12965, -1432,   525,   718,  4714,   529,   -20,  4092,   531,
   12473, -1432, 12637, -1432,   532,   -13, -1432,   156, 10546, -1432,
   14290, -1432,   129, -1432, -1432, 13607, -1432, -1432, 10751, -1432,
   12965, -1432,   642,  9152,   723,   533, 16160,   721,    46,    28,
   -1432, -1432, -1432, -1432, -1432, 14138, 15635,   541,   727, 14377,
   -1432,   553, -1432, -1432, -1432,   662, 12965,   663,   666, 12965,
   12965, 12965, -1432,   755, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432,   556, -1432, -1432, -1432,   549, -1432, -1432, 15975,   550,
     739,   243, 15975,   551,   743,   252,   259, 15876, -1432, 15975,
   12965,   531,     0, -1432, 14377,   674, -1432,   531,    85,    86,
     558,   559,  1516,   564, 15975,   640,   569,   531,    87,   570,
   15772, 15975, -1432, -1432,   704,  2187,   -30, -1432, -1432, -1432,
     324, -1432, -1432, -1432,   745,   647,   610,    61, -1432,   297,
     650,   770,   582,   633,   127, -1432, 15210,   585,   777,   589,
   14138, 14138,   775,   658,    44,   599,   785, -1432, 14138,   104,
     730,   117, -1432, -1432, -1432, -1432, -1432, -1432,   602,  2325,
   -1432, -1432, -1432, -1432,   788,   629, -1432, 14741, 12965,   601,
     793, 16248,   787, -1432, -1432,   678, 14942, 16375, 16059,  4531,
   12965, 16204, 16449,  3274,  2240, 16480,  3539, 12123, 12123, 12123,
   12123,  1250,  1250,  1250,  1250,   792,   792,   158,   158,   158,
      59,    59,    59, -1432,   581, 16248,   605,   606, 15254,   614,
     804, -1432, 12965,   -63,   619,   127, -1432, -1432, -1432, -1432,
     488, 12965,  4954, -1432, -1432,  4531, -1432,  4531,  4531,  4531,
    4531,  4531,  4531,  4531,  4531,  4531,  4531,  4531,  4531, 12965,
     -63,   623,   621,  2385,   625,   622,  2587,    90,   624, -1432,
   16248, 14506, -1432, 15975, -1432,   510,   104,   479, 14741, 16248,
   14741, 15310,   125,   133, -1432,   630, 12965, -1432, -1432, -1432,
    8742,    51, -1432, -1432, 16248, 16248,   -68, -1432, -1432, -1432,
   12965,   735,  3443, 14377, 15975,  9357,   634,   635, -1432,    50,
     705,   697, -1432,   837,   648, 13994, 14138, 14377, 14377, 14377,
   14377, 14377, -1432,   652,   228,   706,   653,   654,   656, 14377,
     341, -1432,   711, -1432, -1432,   661, -1432, 16334, -1432, 12965,
     675, 16248,   681,   850, 13695,   857, -1432, 16248, 13651, -1432,
     556,   791, -1432,  6077, 15714,   671,   277, -1432, 15817, 15975,
     293, -1432, 15832, 15975, 15975, -1432, -1432,  2636, -1432, 16334,
     860, 14741,   676, -1432, -1432, -1432, -1432, -1432,   779,    55,
   15714,   677, 14605, 14688,   863, -1432, -1432, -1432, -1432,   683,
   -1432, 12965, -1432, -1432,  5257, -1432, 14138, 15714,   679, -1432,
   -1432, -1432, -1432,   865, 12965, 14438, -1432, -1432, 15890, 12965,
   -1432, 12965, -1432, -1432,   689, -1432, 14138, -1432,   695,   868,
      22, -1432, -1432,   282, 14866, -1432,   134, -1432, -1432, 14138,
   -1432, -1432,   531, 16248, -1432, 10915, -1432, 14377,    23,   702,
   15714,   647, -1432, -1432,  4727, 12965, -1432, -1432, 12965, -1432,
   12965, -1432,  3044,   708, 10546,   640,   872,   647,   678, 15975,
   14824,   531,  3847,   709, 10546, -1432, -1432,   135, -1432, -1432,
     889, 14592, 14592, 14506, -1432, -1432, -1432, -1432,   712,   254,
     713, -1432, -1432, -1432,   895,   710,   525,   531,   531, 12801,
   -1432,   136, -1432, -1432,  3917,   303,   -68,  9972, -1432,  6282,
     715,  6487,   719,  3443, 14741,   720,   794,   531, 16334,   899,
   -1432, -1432, -1432, -1432,   266, -1432,   199, 14138, -1432, 14138,
   15975, 15635, -1432, -1432, -1432,   911, -1432,   725,   788,   539,
     539,   861,   861, 15454,   722,   916, 14377,   784, 15975, 14438,
   14377, 14377,   115, 15962, 14377, 14377, 14377, 14377, 14228, 14377,
   14377, 14377, 14377, 14377, 14377, 14377, 14377, 14377, 14377, 14377,
   14377, 14377, 14377, 14377, 14377, 14377, 14377, 14377, 14377, 14377,
   14377, 16248, 12965, 12965, 12965, -1432, -1432, -1432, 12965, 12965,
   -1432,   755, -1432,   853, -1432, -1432, 15975, -1432, -1432, 15975,
   -1432, -1432, -1432, -1432, 14377,   531, -1432, 14138, 15975, -1432,
     936, -1432, -1432,    93,   751,   531, 11120, -1432,  2057, -1432,
    5462,   611,   936, -1432,   362,    16, 16248,   822, -1432, 16248,
   15354, -1432,   752,   868, 14138,   658, 14138,   875,   937,   876,
   12965,   -63,   758, -1432, 14741, 12965, 16248, 16334,   759,    23,
   -1432,   756,    23,   761,  4727, 16248, 15410,   762, 10546,   763,
     765, 14138,   767,   647, -1432,   466,   766, 10546,   772, 12965,
   -1432, -1432, -1432, -1432, -1432, -1432,   846,   771,   966, 14506,
     833, -1432, 14438, 14506, -1432, -1432, -1432, 14741, 16248, -1432,
     -68,   948,   906,  9972, -1432, -1432, -1432,   780, 12965,   794,
     531, 14605,  3443,   782, 14377,  6692,   438,   783, 12965,    43,
     268, -1432,   817, -1432,   864, -1432, 14059,   963,   796, 14377,
   -1432, 14377, -1432,   797, -1432,   869,   990,   801, 15510,   802,
     993,   808, -1432, -1432, -1432, 15552,   809,   994,  4603, 16415,
    2674, 14377, 16292, 13874,  2418, 14568, 14871, 14912, 16510, 16510,
   16510, 16510,  1682,  1682,  1682,  1682,   836,   836,   539,   539,
     539,   861,   861,   861,   861, 16248, 13827, 16248, -1432, 16248,
   -1432,   813, -1432, -1432, -1432, 16334, -1432,   917, 15714,   442,
   -1432, 14605, -1432, -1432,  4531,   814, -1432,   816,   495, -1432,
      63, 12965, -1432, -1432, -1432, 12965, -1432, 12965, 12965, -1432,
     658, -1432, -1432,   306,  1001, 14377, -1432,  4324,   821, 10546,
     531, 16248,   868,   825, -1432,   827,    23, 12965, 10546,   828,
   -1432, -1432,   611, -1432,   819,   830, -1432, 10546,   831, -1432,
   14506, -1432, 14506, -1432,   832, -1432,   901,   834,  1026, -1432,
     531,  1006, -1432,   838, -1432, -1432,   840,   842,    94, -1432,
   -1432, 16334,   843,   845, -1432,  4692, -1432, -1432, -1432, -1432,
   -1432, 14138, -1432, 14138, -1432, 16334, 15608, -1432, 14377, 14438,
   -1432, -1432, 14377, -1432, 14377, -1432, -1432, 14377, -1432, 14377,
   -1432, 13332, 14377, 12965,   841,  6897, 14138, -1432,    -7, 14138,
   15714, -1432, 15654,   890, 14157, -1432, -1432, -1432,   617, 13850,
      67,   503,    95, -1432, -1432, -1432,   896,  4402,  4454, 16248,
   16248, -1432,   968,  1035,   971, 14377, 16334,   854, 10546,   852,
     943,   868,  1109,   868,   858, 16248,   859, -1432,  1564,   862,
    1595, -1432,    23,   867, -1432, -1432,   928, -1432, 14506, -1432,
   14438, -1432, -1432,  8742, -1432, -1432, -1432, -1432,  9562, -1432,
   -1432, -1432,  8742, -1432,   871, 14377, 16334,   931, 16334, 15650,
   16334, 15706, 13332, 13783, -1432, -1432, -1432, 15714, 15714,  1046,
      69, -1432, -1432, -1432, -1432,    71,   866,    72, -1432, 13334,
   -1432, -1432,    74, -1432, -1432,  1960, -1432,   873, -1432,   982,
     488, -1432, 14138, -1432,   617, -1432,  1688, -1432, -1432, -1432,
   -1432,  1048, 14377, -1432, 16334, 10546,   878, -1432,   870,   874,
     196, -1432,   943,   868, -1432, -1432, -1432, -1432,  1646,   879,
   -1432, 14506, -1432,   944,  8742,  9767,  9562, -1432, -1432, -1432,
    8742, -1432, 16334, 14377, 14377, 14377, 12965,  7102,   880,   881,
   14377, 15714, -1432, -1432,   855, 15654, -1432, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432,   494, -1432,   890,
   -1432, -1432, -1432, -1432, -1432,    42,   520, -1432,  1050,    77,
   15975,   982,  1057,   488, -1432, -1432,   884,  1060, 14377, 16334,
     886, -1432,   325, -1432, -1432, -1432, -1432,   885,   196,  5037,
   -1432,   868, -1432, 14506, -1432, -1432, -1432, -1432,  7307, 16334,
   16334, 16334, 13739, -1432, -1432, -1432, 16334, -1432, 14312,    47,
   -1432, -1432, 14377, 13334, 13334,  1032, -1432,  1960,  1960,   521,
   -1432, -1432, -1432, 14377,  1009, -1432,   891,    78, 14377, 15975,
   -1432, 14377, 16334, -1432,  1011, -1432,  1083,  7512,  7717, -1432,
   -1432, -1432,   196, -1432,  7922,   893,  1015,   987, -1432,  1000,
     950, -1432, -1432,  1002,   855, -1432, 16334, -1432, -1432,   940,
   -1432,  1068, -1432, -1432, -1432, -1432, 16334,  1088, -1432, -1432,
   16334,   905, 16334, -1432,   335,   907, -1432, -1432,  8127, -1432,
     904, -1432, -1432,   909,   946, 15975,   503, -1432, -1432, 14377,
      27, -1432,  1030, -1432, -1432, -1432, -1432, 15714,   671, -1432,
     952, 15975,    25, 16334,   919,  1096,   404,    27, -1432,  1036,
   -1432, 15714,   920, -1432,   868,    40, -1432, -1432, -1432, -1432,
   14138, -1432,   923,   924,    79, -1432,   216,   404,   311,   868,
     915, -1432, -1432, -1432, -1432, 14138,  1045,  1111,  1047,   216,
   -1432,  8332,   312,  1113, 14377, -1432, -1432,  8537, -1432,  1051,
    1117,  1053, 14377, 16334, -1432,  1119, 14377, -1432, 16334, 14377,
   16334, 16334
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1432, -1432, -1432,  -504, -1432, -1432, -1432,    -4, -1432, -1432,
   -1432,   627,   378,   375,   -24,  1220,  2080, -1432,  2443, -1432,
    -433, -1432,    -1, -1432, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432,  -520, -1432, -1432,  -175,    34,    11,
   -1432, -1432, -1432, -1432, -1432, -1432,    12, -1432, -1432, -1432,
   -1432,    15, -1432, -1432,   753,   757,   760,  -130,   283,  -795,
     289,   349,  -524,    64,  -797, -1432,  -268, -1432, -1432, -1432,
   -1432,  -647,   -95, -1432, -1432, -1432, -1432,  -516, -1432,  -550,
   -1432,  -385, -1432, -1432,   651, -1432,  -254, -1432, -1432,  -950,
   -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432, -1432,
    -276, -1432, -1432, -1432, -1432, -1432,  -359, -1432,  -122, -1138,
   -1432, -1431,  -535, -1432,  -156,    13,  -133,  -522, -1432,  -363,
   -1432,   -76,   -25,  1128,  -658,  -361, -1432, -1432,   -33, -1432,
   -1432,  3822,    -3,  -232, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432, -1432,  -539,  -765, -1432, -1432, -1432, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1432,   786, -1432, -1432,   190,
   -1432,   690, -1432, -1432, -1432, -1432, -1432, -1432, -1432,   194,
   -1432,   691, -1432, -1432,   429, -1432,   163, -1432, -1432, -1432,
   -1432, -1432, -1432, -1432, -1432, -1116, -1432,  2024,  1181,  -346,
   -1432, -1432,   130,  3589,  2812, -1432, -1432,   242,  -174,  -582,
   -1432, -1432,   309,  -648,   111, -1432, -1432, -1432, -1432, -1432,
     298, -1432, -1432, -1432,    -2,  -794,  -181,  -176,  -129, -1432,
   -1432,    66, -1432, -1432, -1432, -1432,     1,  -131, -1432,   114,
   -1432, -1432, -1432,  -380,   894, -1432, -1432, -1432, -1432, -1432,
     877, -1432,   247, -1432,   427,   363, -1432, -1432,   898,  -298,
    -968, -1432,   -43,   -83,  -201,  -132,   478, -1432,  -990, -1432,
     269, -1432, -1432, -1432,  -134,  -999
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -875
static const yytype_int16 yytable[] =
{
     120,   343,   160,   127,   387,   306,   416,   234,   836,   258,
     555,   311,   312,   690,   818,   129,   131,   134,   239,   132,
     263,   436,   243,   883,   817,   664,  1026,  1193,   643,   623,
     386,   227,   275,  1190,   409,   667,   316,   794,   128,   414,
    1017,   333,   902,  1177,   433,   246,  1041,   441,   256,   343,
     339,  1577,   442,   685,   302,   704,  1614,   303,   706,   917,
     916,   753,  1052,  1178,   998,   288,   450,    13,    13,   507,
     162,   512,    13,  1246,   515,  1578,  1382,   495,  1431,   276,
    -281,  1435,   304,  1519,   288,    13,  1584,  1584,  1431,    13,
     288,   288,   213,   214,   771,   771,   783,   404,   313,   783,
     443,   411,   783,   783,   783,   330,  1089,   404,     3,   318,
     186,   800,   375,  1296,   404,   463,   464,  1600,  1357,  1358,
     282,   468,   896,   897,   376,   945,   946,   947,   230,   288,
     233,   293,  -439,   426,   496,  -874,   322,    58,    59,    60,
     174,   175,   340,   240,   948,   434,   949,   950,   951,   952,
     953,   954,   955,   956,   957,   958,   959,   960,   961,   962,
     963,   964,   965,   966,   967,   968,   969,   596,   342,  1184,
     407,  1639,   793,   388,  1115,  -761,  -764,   554,   319,   571,
     970,   241,  -614,  -768,   407,  -762,   683,  -763,  -613,  -799,
     423,   407,  -765,   411,  -473,  -800,   586,   296,   297,  -802,
    -766,  -767,  -801,   277,   242,   341,   267,   372,   373,   374,
     520,   375,  1310,   801,   314,   307,   417,  1185,  -695,   918,
     207,  -695,  -222,   376,   999,  1673,  -208,   509,   802,   707,
     120,   824,   334,   447,   120,   207,   412,   452,   457,  -695,
    -222,  1579,   668,  1247,  1319,   705,   501,   505,   506,  1615,
     674,   635,   440,   594,  1312,   343,   470,   451,  1214,   899,
     508,  1318,   513,  1320,   901,   516,  1096,  1383,  1100,  1432,
    1433,  -281,  1436,   635,  1520,   427,   540,  1585,  1629,  1691,
     280,   429,   329,   521,   772,   773,   784,   435,  1038,   870,
     592,  1010,  1170,  1337,  1385,  1239,   281,   635,  1098,  1099,
     162,   530,   306,   339,   162,   408,   635,  1226,  -770,   635,
    -761,  -764,   937,   567,  1122,  -771,   120,  1027,  -768,   408,
    -762,  1400,  -763,  1402,  -799,   583,   408,  -765,   412,   256,
    -800,   709,   288,   134,  -802,  -766,  -767,  -801,  1070,   759,
     293,  1302,   293,   511,   413,   537,  1696,  1709,   763,   675,
     517,   517,   522,   293,   128,   764,   644,   527,   537,   115,
    1594,  1028,   531,   536,  1097,  1098,  1099,  1098,  1099,   284,
    1652,  1408,   295,   986,  1080,  1081,   227,   806,   633,   288,
     637,   288,   288,   634,   812,  1303,   162,   293,   825,   989,
    1697,  1710,   325,   826,   329,   293,  1544,  1545,  1071,   285,
     660,  1101,  1003,   329,  1595,   661,   296,   297,   296,   297,
     329,   286,  -874,  1548,  1653,   693,  1692,  1693,   293,   296,
     297,   640,   387,   537,   677,  1229,   814,   815,   329,   634,
    1236,   287,   853,   330,   822,   291,   687,   293,   684,  1621,
    1622,   688,   294,  1029,   329,   293,  1278,   329,   386,   120,
     537,   292,   698,   296,   297,   308,   851,  1617,  1618,   863,
     532,   296,   297,  -874,   328,   859,  -874,  1304,   582,  1049,
    1249,   307,  1698,  1711,   756,   329,   853,  1095,   760,  1058,
     581,   527,   309,   275,   296,   297,  1192,    13,  -874,  1203,
     310,   427,  1205,   627,  1347,   825,   542,   543,   321,  1278,
     826,   890,   295,   296,   297,   766,   891,   555,  1572,   330,
     538,   296,   297,   843,  1055,  1179,  1677,  1678,  1679,   162,
     778,   662,    51,   335,  1573,   665,   788,   790,  1180,   436,
      58,    59,    60,   174,   175,   340,  1243,  1098,  1099,   922,
      13,  1574,  -874,   324,   326,   327,   332,  1279,  -471,  1580,
    1623,   344,  1280,   892,    58,    59,    60,   174,   175,   340,
    1281,  1602,   567,  1181,   345,  1413,  1581,  1624,   346,  1582,
    1625,   347,  1325,   348,  1326,    58,    59,    60,    61,    62,
     340,   586,   586,   925,   928,   349,    68,   383,   967,   968,
     969,   388,   288,   378,   379,   380,  1282,  1283,   341,  1284,
    1279,   381,   382,   410,   970,  1280,  -769,    58,    59,    60,
     174,   175,   340,  1281,  -472,    58,    59,    60,   174,   175,
     340,   384,   341,   385,  -613,   300,   415,   418,   390,   391,
     392,   393,   394,   395,   396,   397,   398,   399,   400,   401,
     420,   555,   422,   341,  1285,  1172,   376,   330,   428,  1282,
    1283,  1301,  1284,   832,   431,   432,  1314,   533,   404,  -612,
     437,   539,  1011,  1209,   438,   440,   448,   881,   461,   886,
     465,   900,  1218,   466,   554,   341,   402,   403,  -869,   469,
    1412,  1688,  1022,   341,  1686,   533,   120,   539,   533,   539,
     539,   471,   472,   635,   514,  1033,  1702,  1295,   474,  1699,
     909,   120,   524,   134,   911,   523,   560,   561,   861,   569,
    1074,  1238,   580,   570,    58,    59,    60,    61,    62,   340,
     -65,    51,   593,   671,   128,    68,   383,   673,   676,  1275,
     696,   682,   450,   700,   501,   703,   716,   739,   505,   715,
     404,   740,   742,   751,   887,   743,   888,   754,   758,   120,
     757,   761,   762,   770,  1108,   988,   162,   774,   775,   991,
     992,  1112,   385,  1552,   777,   780,   134,   782,   907,   785,
     791,   162,  1409,   797,   555,   796,  1054,   799,   804,   805,
     808,   807,   341,  1102,   810,  1103,   811,   128,   813,   816,
     120,  1292,   160,   127,   820,   819,  -474,   829,   831,  1333,
     834,   837,   835,   840,  1018,   129,   131,   134,   554,   132,
     846,   847,   571,   850,  1309,  1342,   849,   854,   567,   162,
    1031,   864,   842,  1316,   867,  1032,   865,   868,   128,   893,
    1198,   903,  1323,   919,   913,   915,   567,   995,   369,   370,
     371,   372,   373,   374,   920,   375,   921,   923,   527,  1005,
     936,   939,   940,   938,   941,   288,  1666,   376,   943,   972,
     162,   227,   944,  1166,   974,   973,   978,  1063,  1063,   881,
     981,  1083,   984,  1666,   994,   997,   996,  1006,  1002,  1015,
    1013,  1687,   964,   965,   966,   967,   968,   969,  1007,  1021,
    1191,  1023,   822,   120,  1223,   120,  1084,   120,  1024,  1414,
    1039,   970,  1051,  1059,  1073,  1603,  1048,  1057,  1420,  1075,
    1069,  1072,   134,  1094,   134,  1086,  1104,  1212,  1091,  1088,
    1106,  1427,  1092,  1396,  1107,  1111,   970,  1110,   532,    33,
      34,    35,  1161,   128,  1114,   128,   475,   476,   477,  1124,
     722,   554,  1260,   478,   479,  1168,  1265,   480,   481,  1171,
    1187,  1195,  1189,  1270,  1194,  1196,  1199,  1204,  1202,  1206,
    1208,   555,  1210,   162,  1217,   162,  1211,   162,  1213,   907,
    1090,  1219,  1033,  1173,  1220,  1222,  1221,  1225,  1232,  1233,
    1235,  1240,  1163,  1244,  1558,  1164,  1250,    72,    73,    74,
      75,    76,  1251,  1253,  1167,  1254,  1257,  1258,   724,  1259,
    1261,  1263,  1264,  1269,    79,    80,   120,  1266,   160,   127,
    1540,  1268,  1274,  1276,  1635,  1305,  1293,  1294,    89,  1308,
    1321,   129,   131,   134,  1311,   132,  1313,  1317,  1322,  1328,
    1324,  1327,   555,  1329,    96,  1330,  1332,   567,  1334,  1335,
     567,  1336,  1354,  1339,   128,  1340,  1371,  1391,  1387,  1392,
    1393,  1397,  1395,  1398,  1331,  1231,  1411,  1403,  1404,  1423,
    1430,  1528,  1538,  1406,  1583,   881,  1410,  1434,  1542,   881,
    1421,  1588,  1553,  1527,  1591,  1543,   162,  1541,  1551,   120,
    1564,  1565,  1234,  1676,  1590,  1593,  1599,  1619,  1627,  1628,
    1633,   120,  1634,  1641,  1642,  1643,  -277,  1645,  1646,  1648,
    1200,  1578,  1649,  1651,  1656,  1675,  1654,  1657,   134,  1668,
    1597,  1658,  1598,  1278,  1671,  1683,  1700,  1343,  1674,  1344,
    1685,  1604,  1689,  1690,  1703,  1704,  1705,  1712,   554,   128,
    1715,  1716,  1717,  1719,   765,  1384,   987,   990,  1670,   639,
     636,  1053,  1356,  1230,  1050,   638,  1012,  1684,  1557,   162,
    1341,  1682,  1549,  1237,    13,  1381,   343,   527,   907,  1571,
    1576,   162,  1378,   768,  1706,  1695,  1638,  1291,  1587,   236,
    1547,  1160,  1158,   749,   750,   646,  1291,  1182,  1113,   980,
    1224,  1065,  1289,   519,  1076,  1216,   529,   559,  1121,     0,
    1105,  1289,     0,   926,     0,     0,     0,     0,     0,   554,
       0,     0,     0,     0,   567,  1530,     0,   212,   212,     0,
       0,   224,     0,     0,  1279,     0,   881,     0,   881,  1280,
       0,    58,    59,    60,   174,   175,   340,  1281,     0,     0,
       0,     0,     0,     0,     0,     0,  1359,   527,     0,     0,
       0,     0,     0,     0,     0,     0,   210,   210,     0,     0,
     222,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1701,     0,     0,  1282,  1283,     0,  1284,  1707,     0,     0,
       0,   120,   222,     0,     0,   256,     0,     0,     0,     0,
    1376,     0,     0,     0,     0,     0,     0,     0,   134,   341,
    -875,  -875,  -875,  -875,   367,   368,   369,   370,   371,   372,
     373,   374,     0,   375,   388,     0,     0,     0,     0,   128,
    1291,  1401,     0,     0,     0,   376,  1291,     0,  1291,     0,
     567,     0,  1380,     0,   881,  1289,     0,     0,     0,   120,
       0,  1289,     0,  1289,   120,     0,     0,  1418,   120,  1532,
       0,   162,     0,     0,     0,     0,   134,     0,     0,     0,
       0,     0,     0,     0,     0,   134,     0,     0,  1589,     0,
       0,     0,     0,     0,     0,  1516,     0,   128,     0,     0,
       0,  1523,     0,     0,     0,     0,   128,     0,   256,     0,
       0,     0,   256,     0,     0,     0,     0,     0,     0,     0,
     212,     0,     0,     0,     0,     0,   212,     0,     0,   162,
       0,     0,   212,     0,   162,     0,  1291,   881,   162,     0,
     120,   120,   120,     0,  1555,  1418,   120,     0,  1533,     0,
       0,  1289,     0,   120,     0,     0,     0,   134,     0,   210,
       0,     0,     0,   134,     0,   210,     0,     0,     0,     0,
     134,   210,     0,     0,     0,     0,     0,  1586,   128,     0,
       0,     0,     0,     0,   128,     0,     0,     0,   212,     0,
       0,   128,     0,     0,     0,   212,   212,     0,     0,   222,
     222,     0,   212,     0,     0,   222,     0,     0,   212,     0,
     162,   162,   162,     0,     0,     0,   162,     0,     0,   553,
    1661,     0,     0,   162,     0,     0,     0,   210,     0,     0,
       0,     0,     0,     0,   210,   210,  1631,     0,     0,     0,
       0,   210,     0,     0,     0,     0,     0,   210,     0,     0,
       0,     0,     0,     0,     0,     0,   288,     0,   222,   343,
     418,   390,   391,   392,   393,   394,   395,   396,   397,   398,
     399,   400,   401,     0,     0,   256,   822,     0,     0,   881,
     222,     0,     0,   222,   120,     0,   224,     0,     0,     0,
       0,   822,     0,     0,  1609,     0,     0,     0,  1278,  1516,
    1516,   134,     0,  1523,  1523,     0,     0,     0,     0,   402,
     403,     0,     0,     0,     0,   288,     0,     0,     0,     0,
       0,     0,   128,   120,   120,   222,   212,     0,     0,  1278,
     120,     0,     0,     0,     0,     0,   212,     0,     0,    13,
     134,   134,     0,     0,     0,     0,     0,   134,     0,     0,
       0,     0,     0,     0,   162,     0,     0,     0,     0,     0,
       0,   128,   128,     0,   120,   210,     0,     0,   128,     0,
      13,  1660,     0,   404,     0,   210,     0,     0,   567,     0,
    1278,   134,     0,     0,     0,     0,     0,  1672,     0,  1662,
       0,     0,     0,   162,   162,   567,     0,     0,     0,  1279,
     162,     0,   128,   567,  1280,     0,    58,    59,    60,   174,
     175,   340,  1281,     0,     0,   222,   222,     0,     0,   730,
       0,    13,     0,     0,     0,     0,     0,   120,     0,     0,
    1279,     0,     0,   120,   162,  1280,     0,    58,    59,    60,
     174,   175,   340,  1281,   134,   776,     0,   249,  1282,  1283,
     134,  1284,  -875,  -875,  -875,  -875,   962,   963,   964,   965,
     966,   967,   968,   969,   730,   128,     0,     0,     0,     0,
       0,   128,     0,   250,   341,     0,     0,   970,     0,  1282,
    1283,  1279,  1284,     0,     0,     0,  1280,     0,    58,    59,
      60,   174,   175,   340,  1281,    36,  1405,   162,   212,     0,
       0,     0,     0,   162,     0,   341,     0,     0,     0,     0,
     222,   222,     0,     0,     0,     0,     0,     0,   222,     0,
       0,     0,     0,     0,     0,     0,     0,  1407,     0,     0,
    1282,  1283,     0,  1284,     0,     0,     0,   210,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   251,   252,
       0,     0,     0,   212,     0,     0,   341,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   178,     0,     0,    81,
     253,     0,    83,    84,     0,    85,   179,    87,  1550,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   212,
     254,   212,   210,     0,     0,     0,     0,     0,     0,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   212,     0,     0,   255,     0,     0,     0,
    1534,     0,     0,     0,     0,     0,     0,     0,   210,     0,
     210,   389,   390,   391,   392,   393,   394,   395,   396,   397,
     398,   399,   400,   401,     0,     0,     0,     0,     0,     0,
       0,     0,   210,   730,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   222,   222,   730,   730,   730,
     730,   730,     0,     0,     0,     0,     0,     0,     0,   730,
     402,   403,   212,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   212,   212,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   222,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   553,     0,     0,     0,
       0,   210,     0,     0,     0,     0,     0,     0,     0,     0,
     222,     0,   210,   210,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   404,     0,   222,   222,     0,     0,
       0,     0,     0,     0,     0,   222,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   222,    36,     0,     0,
       0,   224,     0,     0,     0,     0,     0,     0,     0,   222,
     211,   211,     0,     0,   223,     0,     0,   730,     0,     0,
     222,     0,     0,     0,     0,     0,     0,   350,   351,   352,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     222,     0,     0,     0,   212,   212,   353,     0,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,     0,
     375,     0,  1521,     0,    83,    84,  1522,    85,   179,    87,
     553,     0,   376,   210,   210,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   222,     0,   222,
       0,   222,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,   730,     0,  1375,   222,
     730,   730,     0,     0,   730,   730,   730,   730,   730,   730,
     730,   730,   730,   730,   730,   730,   730,   730,   730,   730,
     730,   730,   730,   730,   730,   730,   730,   730,   730,   730,
     730,     0,     0,     0,     0,     0,     0,   350,   351,   352,
     418,   390,   391,   392,   393,   394,   395,   396,   397,   398,
     399,   400,   401,     0,   730,   212,   353,   222,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   211,
     375,     0,     0,     0,   222,     0,   222,     0,     0,   402,
     403,     0,   376,   553,   210,     0,     0,     0,   212,     0,
       0,     0,  1175,     0,     0,     0,     0,     0,     0,     0,
       0,   222,   212,   212,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   222,   375,     0,     0,     0,   210,     0,     0,
       0,   211,     0,     0,     0,   376,     0,     0,   211,   211,
       0,   210,   210,   404,   730,   211,     0,     0,     0,     0,
       0,   211,     0,     0,     0,     0,   222,     0,     0,   730,
       0,   730,   211,     0,     0,   350,   351,   352,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   730,   212,     0,   353,     0,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,     0,   375,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   222,   792,
     376,   210,     0,     0,     0,   350,   351,   352,     0,   223,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   353,   730,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,     0,   375,   211,
     553,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     376,   951,   952,   953,   954,   955,   956,   957,   958,   959,
     960,   961,   962,   963,   964,   965,   966,   967,   968,   969,
       0,   222,     0,   222,     0,     0,     0,     0,   730,   222,
       0,     0,   730,   970,   730,     0,     0,   730,     0,   730,
       0,     0,   730,   735,     0,   257,   222,     0,     0,   222,
     222,     0,   222,     0,     0,     0,     0,     0,     0,   222,
       0,   553,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   730,     0,   828,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   735,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   731,
     222,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   730,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   222,   222,     0,
       0,     0,     0,     0,     0,     0,     0,   866,     0,     0,
       0,     0,     0,     0,   731,     0,     0,   350,   351,   352,
       0,     0,   222,     0,     0,     0,   222,     0,     0,     0,
       0,   211,   730,     0,     0,     0,   353,     0,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,     0,
     375,     0,     0,   730,   730,   730,   350,   351,   352,     0,
     730,   222,   376,     0,     0,   222,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   353,   211,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,     0,   375,
       0,     0,   257,   257,     0,     0,     0,     0,   257,     0,
       0,   376,   211,   948,   211,   949,   950,   951,   952,   953,
     954,   955,   956,   957,   958,   959,   960,   961,   962,   963,
     964,   965,   966,   967,   968,   969,   211,   735,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,   207,   970,
       0,   735,   735,   735,   735,   735,     0,     0,     0,     0,
       0,     0,     0,   735,     0,     0,     0,     0,   730,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   222,
       0,     0,     0,   257,     0,     0,   257,     0,   983,     0,
       0,     0,   631,   731,     0,     0,     0,     0,   222,   869,
       0,     0,   730,     0,     0,   211,     0,   731,   731,   731,
     731,   731,     0,   730,  1001,     0,   211,   211,   730,   731,
       0,   730,     0,    83,    84,     0,    85,   179,    87,     0,
       0,  1001,     0,     0,     0,     0,     0,     0,     0,   211,
       0,     0,     0,     0,     0,     0,     0,     0,   993,     0,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,     0,     0,     0,     0,
     632,   735,   115,     0,  1040,     0,     0,     0,     0,   730,
       0,     0,     0,     0,     0,     0,     0,   222,     0,     0,
       0,     0,     0,     0,   223,     0,     0,     0,     0,     0,
       0,   222,     0,     0,     0,     0,     0,     0,     0,     0,
     222,     0,     0,     0,     0,     0,     0,     0,   257,   711,
       0,     0,   732,     0,     0,   222,     0,   731,     0,     0,
       0,     0,     0,     0,   730,     0,     0,   211,   211,     0,
       0,     0,   730,     0,     0,     0,   730,     0,     0,   730,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   732,     0,     0,
     735,     0,     0,   211,   735,   735,     0,     0,   735,   735,
     735,   735,   735,   735,   735,   735,   735,   735,   735,   735,
     735,   735,   735,   735,   735,   735,   735,   735,   735,   735,
     735,   735,   735,   735,   735,     0,     0,     0,     0,     0,
       0,     0,     0,   257,   257,     0,     0,     0,     0,     0,
       0,   257,     0,     0,     0,     0,   731,     0,   735,     0,
     731,   731,     0,     0,   731,   731,   731,   731,   731,   731,
     731,   731,   731,   731,   731,   731,   731,   731,   731,   731,
     731,   731,   731,   731,   731,   731,   731,   731,   731,   731,
     731,     0,     0,     0,   350,   351,   352,     0,   211,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      36,     0,   207,   353,   731,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   211,   375,     0,     0,
       0,   211,     0,     0,     0,     0,     0,     0,     0,   376,
       0,     0,     0,     0,     0,   211,   211,     0,   735,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   735,     0,   735,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   732,    83,    84,     0,
      85,   179,    87,     0,     0,   735,     0,     0,   257,   257,
     732,   732,   732,   732,   732,     0,     0,     0,     0,     0,
       0,     0,   732,     0,   731,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,   731,
       0,   731,  1277,     0,   659,   211,   115,    83,    84,     0,
      85,   179,    87,     0,     0,     0,     0,     0,     0,     0,
       0,   731,     0,     0,     0,     0,     0,     0,     0,   735,
       0,     0,     0,     0,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,   257,
       0,   593,     0,     0,     0,     0,  1047,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   257,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   257,     0,     0,   731,     0,     0,     0,     0,
     732,   737,   735,   211,     0,     0,   735,     0,   735,     0,
       0,   735,     0,   735,     0,     0,   735,     0,     0,     0,
       0,     0,     0,     0,  1361,     0,  1370,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   769,   375,     0,   735,
       0,     0,     0,     0,     0,     0,     0,     0,   731,   376,
       0,     0,   731,     0,   731,     0,     0,   731,     0,   731,
       0,     0,   731,     0,   211,     0,     0,     0,     0,     0,
     257,     0,   257,     0,   711,     0,     0,     0,     0,   735,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   732,
       0,  1428,  1429,   732,   732,   731,     0,   732,   732,   732,
     732,   732,   732,   732,   732,   732,   732,   732,   732,   732,
     732,   732,   732,   732,   732,   732,   732,   732,   732,   732,
     732,   732,   732,   732,     0,     0,   735,     0,     0,     0,
       0,     0,     0,     0,     0,   731,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   732,     0,     0,
     257,     0,     0,     0,     0,     0,     0,   735,   735,   735,
       0,     0,     0,     0,   735,  1567,     0,     0,     0,  1370,
       0,     0,     0,     0,     0,     0,     0,   257,     0,   257,
       0,     0,   731,     0,     0,     0,     0,     0,   904,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   257,     0,     0,     0,     0,     0,
       0,     0,     0,   731,   731,   731,     0,     0,     0,     0,
     731,     0,     0,     0,     0,   908,     0,     0,     0,     0,
      36,     0,   207,     0,     0,     0,     0,     0,     0,   929,
     930,   931,   932,     0,     0,     0,     0,   732,     0,     0,
       0,   942,     0,     0,     0,     0,     0,     0,     0,   257,
       0,     0,   732,     0,   732,     0,     0,     0,     0,     0,
     208,     0,   735,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   905,     0,   732,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   178,   375,     0,    81,    82,   735,    83,    84,     0,
      85,   179,    87,     0,   376,     0,     0,   735,     0,     0,
       0,     0,   735,     0,     0,   735,     0,     0,   731,     0,
       0,     0,     0,     0,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   732,     0,
       0,   209,     0,     0,     0,     0,   115,     0,     0,  1037,
       0,     0,   731,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   731,     0,     0,     0,     0,   731,     0,
       0,   731,     0,   735,     0,     0,     0,     0,     0,     0,
       0,  1669,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   257,  1361,   257,     0,     0,     0,
       0,   732,     0,     0,     0,   732,     0,   732,     0,     0,
     732,     0,   732,     0,     0,   732,     0,     0,     0,   257,
       0,     0,   257,     0,     0,     0,     0,     0,   735,   731,
       0,     0,   257,     0,     0,     0,   735,     0,     0,     0,
     735,     0,     0,   735,     0,     0,     0,     0,   732,     0,
       0,     0,  1118,  1118,     0,     0,  1125,  1128,  1129,  1130,
    1132,  1133,  1134,  1135,  1136,  1137,  1138,  1139,  1140,  1141,
    1142,  1143,  1144,  1145,  1146,  1147,  1148,  1149,  1150,  1151,
    1152,  1153,  1154,     0,   731,     0,     0,     0,   732,     0,
       0,     0,   731,     0,     0,     0,   731,     0,     0,   731,
       0,     0,     0,     0,     0,     0,  1165,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   257,     0,   183,   185,   257,
     187,   188,   189,   191,   192,   732,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,     0,     0,
     216,   219,     0,     0,     0,     0,     0,   350,   351,   352,
       0,     0,     0,   237,     0,     0,   732,   732,   732,     0,
     245,     0,   248,   732,     0,   264,   353,   269,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,     0,
     375,     0,     0,     0,     0,     0,  1241,     0,     0,     0,
       0,     0,   376,     0,     0,     0,     0,     0,     0,     0,
       0,  1255,     0,  1256,     0,     0,     0,   350,   351,   352,
       0,     0,     0,     0,   317,     0,     0,     0,     0,     0,
       0,     0,     0,  1271,     0,     0,   353,     0,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,     0,
     375,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   732,   376,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   257,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1306,   419,     0,
       0,  1610,     0,     0,     0,   732,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   732,     0,     0,     0,
       0,   732,     0,     0,   732,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1056,
       0,     0,   445,     0,     0,   445,     0,     0,   736,     0,
       0,     0,   237,   456,     0,     0,     0,     0,     0,     0,
    1346,     0,     0,     0,  1348,     0,  1349,     0,     0,  1350,
       0,  1351,     0,     0,  1352,     0,     0,     0,     0,     0,
       0,     0,   732,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   736,     0,     0,     0,     0,   317,     0,
       0,     0,     0,     0,   216,     0,     0,  1394,   535,  1079,
       0,     0,     0,   257,     0,     0,     0,     0,     0,     0,
       0,   556,   556,     0,     0,     0,     0,     0,   257,     0,
       0,     0,     0,   579,     0,     0,     0,   732,     0,     0,
       0,     0,     0,     0,   591,   732,     0,  1422,     0,   732,
       0,     0,   732,     0,     0,     0,     0,     0,     0,    36,
       0,   207,   597,   598,   599,   601,   602,   603,   604,   605,
     606,   607,   608,   609,   610,   611,   612,   613,   614,   615,
     616,   617,   618,   619,   620,   621,   622,     0,   624,     0,
     625,   625,     0,   628,  1539,     0,     0,     0,     0,     0,
       0,   645,   647,   648,   649,   650,   651,   652,   653,   654,
     655,   656,   657,   658,     0,     0,     0,     0,     0,   625,
     663,     0,   591,   625,   666,  1559,  1560,  1561,     0,     0,
     645,     0,  1566,   670,     0,     0,    83,    84,     0,    85,
     179,    87,   679,     0,   681,     0,     0,     0,     0,     0,
     591,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     694,     0,   695,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,     0,     0,     0,
       0,     0,   736,   632,     0,   115,     0,     0,   741,     0,
       0,   744,   747,   748,     0,     0,   736,   736,   736,   736,
     736,     0,     0,     0,     0,     0,     0,     0,   736,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   767,     0,   350,   351,   352,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1592,     0,     0,   353,     0,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,     0,   375,     0,     0,
       0,     0,     0,     0,  1616,     0,     0,     0,     0,   376,
       0,     0,     0,     0,     0,  1626,     0,     0,     0,     0,
    1630,     0,     0,  1632,     0,     0,     0,     0,     0,     0,
     833,     0,   350,   351,   352,     0,     0,     0,     0,     0,
       0,     0,   844,     0,     0,     0,   736,     0,     0,     0,
       0,   353,     0,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   852,   375,     0,     0,     0,     0,
       0,  1663,     0,   191,   350,   351,   352,   376,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   862,     0,   353,     0,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,     0,   375,   894,     0,
       0,     0,     0,     0,     0,     0,  1713,     0,     0,   376,
       0,     0,   237,     0,  1718,   736,  1307,     0,  1720,   736,
     736,  1721,     0,   736,   736,   736,   736,   736,   736,   736,
     736,   736,   736,   736,   736,   736,   736,   736,   736,   736,
     736,   736,   736,   736,   736,   736,   736,   736,   736,   736,
     353,   971,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   736,   375,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   376,     0,     0,     0,
       0,     0,     0,     0,  1389,     0,     0,     0,     0,     0,
       0,     0,     0,  1008,   946,   947,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1016,     0,     0,     0,
       0,  1019,   948,  1020,   949,   950,   951,   952,   953,   954,
     955,   956,   957,   958,   959,   960,   961,   962,   963,   964,
     965,   966,   967,   968,   969,     0,  1390,  1036,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1044,   970,     0,
    1045,     0,  1046,     0,     0,     0,   591,     0,     0,     0,
       0,     0,     0,   736,     0,     0,   591,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   736,     0,
     736,     0,   350,   351,   352,     0,     0,     0,     0,     0,
       0,  1078,     0,     0,     0,     0,     0,     0,     0,     0,
     736,   353,  1246,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,     0,   375,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   376,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,     0,
     375,    29,    30,     0,   736,     0,   350,   351,   352,     0,
       0,    36,   376,   207,  1155,  1156,  1157,     0,     0,     0,
     744,  1159,     0,     0,     0,   353,     0,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,   372,   373,   374,  1174,   375,
       0,   208,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   376,     0,     0,     0,     0,     0,   736,     0,     0,
       0,   736,  1197,   736,     0,     0,   736,  1201,   736,     0,
       0,   736,   178,     0,     0,    81,    82,     0,    83,    84,
     591,    85,   179,    87,     0,     0,     0,     0,     0,   591,
      90,  1174,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1247,     0,   736,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
     237,     0,   425,     0,   350,   351,   352,   115,     0,     0,
    1245,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   353,   736,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,     0,   375,     0,     0,
       0,   350,   351,   352,     0,     0,     0,     0,     0,   376,
       0,     0,     0,     0,     0,     0,   377,     0,     0,     0,
     353,   736,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,  1297,   375,     0,     0,  1298,     0,  1299,
    1300,     0,   736,   736,   736,     0,   376,     0,     0,   736,
       0,   591,   860,  1570,   350,   351,   352,     0,     0,  1315,
     591,    36,     0,   207,     0,     0,     0,     0,     0,   591,
       0,     0,     0,   353,     0,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   249,   375,     0,     0,
       0,   208,     0,     0,     0,     0,     0,     0,     0,   376,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   250,     0,     0,  1353,     0,     0,     0,     0,
       0,     0,   178,     0,   460,    81,    82,     0,    83,    84,
       0,    85,   179,    87,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   736,     0,     0,
     591,     0,     0,     0,     0,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   462,   209,     0,     0,     0,     0,   115,     0,     0,
       0,   736,     0,     0,     0,     0,     0,   251,   252,     0,
       0,     0,   736,     0,     0,     0,     0,   736,     0,     0,
     736,     0,     0,     0,     0,   178,     0,     0,    81,   253,
       0,    83,    84,     0,    85,   179,    87,     0,     0,     0,
       0,     0,     0,  1647,     0,     0,     0,     0,     0,   254,
       0,     0,     0,     0,   473,     0,     0,   591,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,     0,   255,     0,     0,   736,  1601,
       0,     0,     0,     0,     0,     0,     0,     0,  1562,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,   736,     0,     0,     0,     0,     0,     0,
       0,   736,    13,    14,    15,   736,     0,     0,   736,    16,
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
     109,   110,   111,     0,     0,   112,     0,   113,   114,  1009,
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
     112,     0,   113,   114,  1176,   115,   116,     0,   117,   118,
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
     112,     0,   113,   114,   572,   115,   116,     0,   117,   118,
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
     109,   110,   111,     0,     0,   112,     0,   113,   114,   982,
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
     112,     0,   113,   114,  1085,   115,   116,     0,   117,   118,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,     0,
       0,     0,    40,    41,    42,    43,  1087,    44,     0,    45,
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
      43,     0,    44,     0,    45,     0,    46,  1242,     0,    47,
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
     109,   110,   111,     0,     0,   112,     0,   113,   114,  1355,
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
     112,     0,   113,   114,  1563,   115,   116,     0,   117,   118,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,    32,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,    39,     0,
       0,     0,    40,    41,    42,    43,     0,    44,     0,    45,
    1605,    46,     0,     0,    47,     0,     0,     0,    48,    49,
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
     112,     0,   113,   114,  1636,   115,   116,     0,   117,   118,
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
     109,   110,   111,     0,     0,   112,     0,   113,   114,  1637,
     115,   116,     0,   117,   118,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,     0,     0,     0,    40,    41,    42,
      43,     0,    44,  1640,    45,     0,    46,     0,     0,    47,
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
     109,   110,   111,     0,     0,   112,     0,   113,   114,  1655,
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
     112,     0,   113,   114,  1708,   115,   116,     0,   117,   118,
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
     109,   110,   111,     0,     0,   112,     0,   113,   114,  1714,
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
     112,     0,   113,   114,     0,   115,   116,     0,   117,   118,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,     0,   446,     0,     0,
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
      12,     0,   697,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,    11,    12,     0,   910,     0,     0,
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
      12,     0,  1417,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,    11,    12,     0,  1554,     0,     0,
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
      12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,   641,    12,     0,     0,     0,     0,
       0,     0,   642,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    14,    15,     0,     0,     0,     0,    16,
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
      89,     0,     0,    90,     0,     0,     0,     0,     0,    91,
      92,    93,     0,     0,     0,     0,    96,    97,   261,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,     0,     0,     0,     0,
     115,   116,     0,   117,   118,     5,     6,     7,     8,     9,
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
       0,     0,     0,     0,    58,    59,    60,   174,   175,   176,
       0,     0,    65,    66,     0,     0,     0,     0,     0,     0,
       0,   177,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     178,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     179,    87,     0,     0,     0,    89,     0,     0,    90,     5,
       6,     7,     8,     9,    91,    92,    93,     0,     0,    10,
       0,    96,    97,   261,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
     112,   587,   262,     0,     0,   115,   116,     0,   117,   118,
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
      83,    84,     0,    85,   179,    87,     0,   588,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,     0,     0,     0,     0,    96,    97,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   112,     0,     0,     0,     0,   115,
     116,     0,   117,   118,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
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
    1034,     0,   692,     0,   115,   116,     0,   117,   118,     0,
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
      84,     0,    85,   179,    87,     0,  1035,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
       0,     0,     0,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,     0,     0,     0,   115,   116,
       0,   117,   118,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   641,    12,     0,
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
     106,   107,   108,   109,   110,   111,     0,     0,   112,   444,
       0,     0,     0,   115,   116,     0,   117,   118,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,  -875,
    -875,  -875,  -875,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   600,   375,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   376,     0,
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
     111,     0,     0,   112,   642,     0,     0,     0,   115,   116,
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
     107,   108,   109,   110,   111,     0,     0,   112,   678,     0,
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
       0,   112,   680,     0,     0,     0,   115,   116,     0,   117,
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
     109,   110,   111,     0,     0,   112,  1077,     0,     0,     0,
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
       0,     0,     0,     0,   115,   116,     0,   117,   118,     0,
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
       0,     0,     0,    33,    34,    35,    36,   534,    38,     0,
       0,     0,     0,     0,    40,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    51,     0,     0,     0,     0,     0,     0,
       0,    58,    59,    60,   174,   175,   176,     0,     0,    65,
      66,     0,     0,     0,     0,     0,     0,     0,   177,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,   178,    79,    80,
      81,    82,     0,    83,    84,     0,    85,   179,    87,     0,
       0,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,    91,    92,    93,     0,     0,     0,     0,    96,    97,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,   112,     0,     0,
       0,     0,   115,   116,     0,   117,   118,  1437,  1438,  1439,
    1440,  1441,     0,     0,  1442,  1443,  1444,  1445,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1446,  1447,   949,   950,   951,   952,   953,   954,   955,
     956,   957,   958,   959,   960,   961,   962,   963,   964,   965,
     966,   967,   968,   969,     0,     0,     0,  1448,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   970,     0,     0,
       0,  1449,  1450,  1451,  1452,  1453,  1454,  1455,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
    1456,  1457,  1458,  1459,  1460,  1461,  1462,  1463,  1464,  1465,
    1466,  1467,  1468,  1469,  1470,  1471,  1472,  1473,  1474,  1475,
    1476,  1477,  1478,  1479,  1480,  1481,  1482,  1483,  1484,  1485,
    1486,  1487,  1488,  1489,  1490,  1491,  1492,  1493,  1494,  1495,
    1496,     0,     0,  1497,  1498,     0,  1499,  1500,  1501,  1502,
    1503,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1504,  1505,  1506,     0,     0,     0,    83,    84,
       0,    85,   179,    87,  1507,     0,  1508,  1509,     0,  1510,
       0,     0,     0,     0,     0,     0,  1511,     0,     0,  1512,
       0,  1513,     0,  1514,  1515,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   350,
     351,   352,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   353,     0,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,     0,   375,   350,   351,   352,     0,     0,     0,     0,
       0,     0,     0,     0,   376,     0,     0,     0,     0,     0,
       0,     0,   353,     0,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,     0,   375,   350,   351,   352,
       0,     0,     0,     0,     0,     0,     0,     0,   376,     0,
       0,     0,     0,     0,     0,     0,   353,     0,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,     0,
     375,   350,   351,   352,     0,     0,     0,     0,     0,     0,
       0,     0,   376,     0,     0,     0,     0,     0,     0,     0,
     353,     0,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,     0,   375,   350,   351,   352,     0,     0,
       0,     0,     0,     0,     0,     0,   376,     0,     0,   497,
       0,     0,     0,     0,   353,     0,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,     0,   375,   350,
     351,   352,     0,     0,     0,     0,     0,     0,     0,     0,
     376,     0,   669,     0,     0,     0,     0,     0,   353,     0,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,     0,   375,   350,   351,   352,     0,     0,     0,     0,
       0,     0,     0,     0,   376,     0,   689,     0,     0,     0,
       0,     0,   353,     0,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,     0,   375,   350,   351,   352,
       0,     0,     0,     0,     0,   979,     0,     0,   376,     0,
       0,     0,     0,     0,     0,     0,   353,     0,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   249,
     375,     0,     0,     0,     0,   975,   976,     0,     0,     0,
       0,     0,   376,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   250,   950,   951,   952,   953,
     954,   955,   956,   957,   958,   959,   960,   961,   962,   963,
     964,   965,   966,   967,   968,   969,  1606,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   970,
       0,     0,     0,     0,   249,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  -319,     0,     0,     0,     0,     0,
       0,     0,    58,    59,    60,   174,   175,   340,     0,  1426,
     250,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     251,   252,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,     0,     0,     0,     0,   178,     0,
       0,    81,   253,     0,    83,    84,     0,    85,   179,    87,
       0,     0,  1273,     0,     0,     0,     0,     0,     0,   467,
       0,     0,   254,   249,     0,     0,     0,     0,     0,     0,
     341,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   251,   252,     0,   255,   250,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   178,     0,     0,    81,   253,     0,    83,
      84,    36,    85,   179,    87,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   254,   249,     0,
       0,     0,     0,     0,     0,     0,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,     0,   255,   250,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   251,   252,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,   178,     0,     0,    81,   253,     0,    83,    84,
       0,    85,   179,    87,     0,   924,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   254,   249,     0,     0,
       0,     0,     0,     0,     0,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   251,
     252,     0,   255,   250,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   178,     0,     0,
      81,   253,     0,    83,    84,    36,    85,   179,    87,     0,
    1252,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   254,     0,     0,    36,     0,     0,     0,     0,     0,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,     0,   255,  1131,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   251,   252,
       0,     0,     0,     0,   717,   718,     0,     0,  1374,     0,
     719,     0,   720,     0,     0,     0,   178,     0,     0,    81,
     253,     0,    83,    84,   721,    85,   179,    87,     0,     0,
       0,     0,    33,    34,    35,    36,     0,     0,     0,     0,
     254,    83,    84,   722,    85,   179,    87,     0,     0,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,     0,   255,     0,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,     0,  1375,     0,     0,   723,     0,
      72,    73,    74,    75,    76,     0,     0,    36,     0,   207,
       0,   724,     0,     0,     0,     0,   178,    79,    80,    81,
     725,     0,    83,    84,     0,    85,   179,    87,     0,    36,
       0,    89,     0,     0,     0,     0,     0,     0,     0,     0,
     726,   727,   728,     0,     0,     0,     0,    96,     0,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   717,   718,     0,   729,     0,     0,   719,
       0,   720,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   721,    83,    84,     0,    85,   179,    87,
       0,    33,    34,    35,    36,     0,     0,     0,     0,     0,
     178,     0,   722,    81,     0,     0,    83,    84,     0,    85,
     179,    87,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,     0,     0,     0,
       0,   686,     0,   115,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   723,     0,    72,
      73,    74,    75,    76,  1608,    36,     0,   207,     0,     0,
     724,     0,     0,   548,     0,   178,    79,    80,    81,   725,
       0,    83,    84,     0,    85,   179,    87,     0,     0,     0,
      89,     0,     0,     0,     0,     0,     0,     0,     0,   726,
     727,   728,   871,   872,     0,   208,    96,     0,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   873,     0,     0,   729,     0,     0,     0,     0,
     874,   875,   876,    36,     0,     0,   178,     0,     0,    81,
      82,   877,    83,    84,     0,    85,   179,    87,     0,    36,
       0,   207,   952,   953,   954,   955,   956,   957,   958,   959,
     960,   961,   962,   963,   964,   965,   966,   967,   968,   969,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   970,     0,     0,   878,     0,     0,   208,
       0,   549,     0,     0,     0,     0,     0,     0,     0,   879,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      83,    84,     0,    85,   179,    87,  1060,  1061,  1062,    36,
     178,     0,     0,    81,    82,     0,    83,    84,   880,    85,
     179,    87,    36,     0,   207,     0,     0,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,     0,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,     0,     0,     0,
     209,     0,   208,   510,     0,   115,     0,     0,     0,     0,
       0,     0,     0,     0,   526,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    83,    84,     0,    85,
     179,    87,     0,   178,     0,     0,    81,    82,     0,    83,
      84,     0,    85,   179,    87,    36,     0,   207,     0,     0,
       0,     0,     0,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,     0,   209,     0,   208,     0,     0,   115,     0,
       0,     0,     0,     0,     0,     0,     0,  1004,    36,     0,
     207,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   178,     0,     0,    81,
      82,     0,    83,    84,     0,    85,   179,    87,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   208,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,     0,   209,     0,     0,   178,
       0,   115,    81,    82,     0,    83,    84,     0,    85,   179,
      87,    36,     0,   207,     0,     0,   953,   954,   955,   956,
     957,   958,   959,   960,   961,   962,   963,   964,   965,   966,
     967,   968,   969,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   970,     0,     0,   209,
       0,   220,     0,    36,   115,   207,     0,     0,   954,   955,
     956,   957,   958,   959,   960,   961,   962,   963,   964,   965,
     966,   967,   968,   969,     0,    36,     0,     0,     0,     0,
       0,     0,   178,     0,     0,    81,    82,   970,    83,    84,
       0,    85,   179,    87,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,    36,
      83,    84,   221,    85,   179,    87,     0,   115,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   300,
       0,     0,    83,    84,     0,    85,   179,    87,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,     0,     0,     0,     0,     0,  1030,     0,   115,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,     0,     0,     0,     0,   301,
       0,     0,     0,     0,     0,     0,    83,    84,     0,    85,
     179,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     350,   351,   352,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,     0,     0,   353,
     842,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,     0,   375,   350,   351,   352,     0,     0,     0,
       0,     0,     0,     0,     0,   376,     0,     0,     0,     0,
       0,     0,     0,   353,     0,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,     0,   375,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   376,
     350,   351,   352,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   421,   353,
       0,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,     0,   375,   350,   351,   352,     0,     0,     0,
       0,     0,     0,     0,     0,   376,     0,     0,     0,     0,
       0,     0,   430,   353,     0,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,     0,   375,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   376,
     350,   351,   352,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   809,   353,
       0,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,     0,   375,   350,   351,   352,     0,     0,     0,
       0,     0,     0,     0,     0,   376,     0,     0,     0,     0,
       0,     0,   848,   353,     0,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,     0,   375,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   376,
     350,   351,   352,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   889,   353,
       0,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,     0,   375,   945,   946,   947,     0,     0,     0,
       0,     0,     0,     0,     0,   376,     0,     0,     0,     0,
       0,     0,  1188,   948,     0,   949,   950,   951,   952,   953,
     954,   955,   956,   957,   958,   959,   960,   961,   962,   963,
     964,   965,   966,   967,   968,   969,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   970,
     945,   946,   947,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1207,   948,
       0,   949,   950,   951,   952,   953,   954,   955,   956,   957,
     958,   959,   960,   961,   962,   963,   964,   965,   966,   967,
     968,   969,   945,   946,   947,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   970,     0,     0,     0,     0,
       0,   948,  1109,   949,   950,   951,   952,   953,   954,   955,
     956,   957,   958,   959,   960,   961,   962,   963,   964,   965,
     966,   967,   968,   969,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   970,   945,   946,
     947,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   948,  1262,   949,
     950,   951,   952,   953,   954,   955,   956,   957,   958,   959,
     960,   961,   962,   963,   964,   965,   966,   967,   968,   969,
     945,   946,   947,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   970,     0,     0,     0,     0,     0,   948,
    1267,   949,   950,   951,   952,   953,   954,   955,   956,   957,
     958,   959,   960,   961,   962,   963,   964,   965,   966,   967,
     968,   969,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,     0,   970,   945,   946,   947,     0,
     710,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    36,     0,     0,     0,   948,  1345,   949,   950,   951,
     952,   953,   954,   955,   956,   957,   958,   959,   960,   961,
     962,   963,   964,   965,   966,   967,   968,   969,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   970,  1362,     0,     0,     0,    36,     0,  1424,     0,
       0,     0,     0,   178,  1363,  1364,    81,     0,     0,    83,
      84,    36,    85,   179,    87,     0,     0,     0,     0,     0,
       0,     0,   178,   270,   271,    81,  1365,     0,    83,    84,
       0,    85,  1366,    87,     0,     0,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,     0,     0,  1425,    36,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,    36,
     272,   786,   787,    83,    84,     0,    85,   179,    87,     0,
       0,     0,   178,     0,     0,    81,    82,     0,    83,    84,
       0,    85,   179,    87,     0,     0,     0,     0,     0,     0,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,    36,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,    36,
     337,     0,    83,    84,     0,    85,   179,    87,     0,     0,
       0,     0,     0,     0,     0,     0,    83,    84,     0,    85,
     179,    87,     0,     0,     0,     0,     0,     0,     0,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,    36,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,    36,   498,     0,
       0,    83,    84,     0,    85,   179,    87,     0,     0,     0,
       0,     0,     0,   502,     0,     0,    83,    84,     0,    85,
     179,    87,     0,     0,     0,     0,     0,     0,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,   631,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   272,     0,     0,
      83,    84,     0,    85,   179,    87,     0,     0,     0,    36,
       0,     0,     0,     0,    83,    84,     0,    85,   179,    87,
       0,     0,    36,     0,     0,     0,     0,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   352,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,  1123,     0,     0,   353,     0,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,     0,   375,     0,     0,     0,    83,    84,     0,    85,
     179,    87,     0,     0,   376,     0,     0,     0,     0,    83,
      84,     0,    85,   179,    87,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     350,   351,   352,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   701,   353,
       0,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,     0,   375,   350,   351,   352,     0,     0,     0,
       0,     0,     0,     0,     0,   376,     0,     0,     0,     0,
       0,     0,     0,   353,   845,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   702,   375,   350,   351,
     352,     0,     0,     0,     0,     0,     0,     0,     0,   376,
       0,     0,     0,     0,     0,     0,     0,   353,     0,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
       0,   375,   945,   946,   947,     0,     0,     0,     0,     0,
       0,     0,     0,   376,     0,     0,     0,     0,     0,     0,
       0,   948,  1272,   949,   950,   951,   952,   953,   954,   955,
     956,   957,   958,   959,   960,   961,   962,   963,   964,   965,
     966,   967,   968,   969,   945,   946,   947,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   970,     0,     0,
       0,     0,     0,   948,     0,   949,   950,   951,   952,   953,
     954,   955,   956,   957,   958,   959,   960,   961,   962,   963,
     964,   965,   966,   967,   968,   969,   351,   352,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   970,
       0,     0,     0,     0,   353,     0,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   947,   375,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     376,     0,     0,     0,   948,     0,   949,   950,   951,   952,
     953,   954,   955,   956,   957,   958,   959,   960,   961,   962,
     963,   964,   965,   966,   967,   968,   969,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     970,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,     0,   375,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   376,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,     0,   375,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   376,  -875,  -875,  -875,  -875,
     958,   959,   960,   961,   962,   963,   964,   965,   966,   967,
     968,   969,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   970
};

static const yytype_int16 yycheck[] =
{
       4,   134,     4,     4,   160,    88,   181,    32,   590,    52,
     308,    94,    95,   446,   564,     4,     4,     4,    43,     4,
      53,   222,    47,   671,   563,   410,   820,  1026,   389,   375,
     160,    30,    56,  1023,   163,   415,   112,   541,     4,   170,
     805,     9,   700,  1011,   218,    49,   841,   228,    52,   182,
     133,     9,   228,   438,    78,     9,     9,    81,    30,     9,
     707,   494,   857,  1013,     9,    69,     9,    45,    45,     9,
       4,     9,    45,    30,     9,    33,     9,   108,     9,    79,
       9,     9,    86,     9,    88,    45,     9,     9,     9,    45,
      94,    95,    26,    27,     9,     9,     9,   127,    79,     9,
     229,    66,     9,     9,     9,   168,   903,   127,     0,   112,
     198,    50,    53,    50,   127,   249,   250,  1548,   125,   126,
      79,   255,    71,    72,    65,    10,    11,    12,   198,   133,
     198,    79,     8,   209,   165,   198,    84,   112,   113,   114,
     115,   116,   117,   198,    29,   221,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,   342,   134,   153,
      66,  1602,   202,   160,   939,    66,    66,   308,   112,   199,
      65,   198,   147,    66,    66,    66,   199,    66,   147,    66,
     194,    66,    66,    66,    66,    66,   330,   145,   146,    66,
      66,    66,    66,   203,   198,   180,   201,    49,    50,    51,
     286,    53,  1202,   152,   195,   152,   182,   201,   196,   169,
      79,   199,   199,    65,   169,   200,   199,    96,   167,   201,
     234,   127,   200,   234,   238,    79,   201,   238,   242,   199,
     196,   199,   417,   200,  1212,   199,   270,   271,   272,   202,
     424,   382,   127,   336,  1204,   388,   260,   200,  1053,   692,
     200,  1211,   200,  1213,   697,   200,   913,   200,   915,   200,
     201,   200,   200,   404,   200,   209,   300,   200,   200,   200,
     123,   215,   151,   286,   199,   199,   199,   221,   838,   199,
     333,   795,   199,   199,   199,  1092,   123,   428,    99,   100,
     234,    66,   385,   386,   238,   201,   437,  1072,   198,   440,
     201,   201,    84,   315,   199,   198,   320,    35,   201,   201,
     201,  1311,   201,  1313,   201,   329,   201,   201,   201,   333,
     201,   465,   336,   320,   201,   201,   201,   201,    84,    96,
      79,    35,    79,   277,   203,    84,    35,    35,    96,   425,
     284,   285,   286,    79,   320,    96,   389,   291,    84,   203,
      35,    79,   127,   297,    98,    99,   100,    99,   100,   198,
      35,  1321,   144,    96,    71,    72,   375,   551,   382,   383,
     384,   385,   386,   382,   558,    79,   320,    79,   569,    96,
      79,    79,    84,   569,   151,    79,   200,   201,   144,   198,
     404,   202,   782,   151,    79,   404,   145,   146,   145,   146,
     151,   198,   147,  1403,    79,   448,   200,   201,    79,   145,
     146,   387,   578,    84,   428,  1073,   560,   561,   151,   428,
    1088,   198,   633,   168,   568,   198,   440,    79,   437,  1577,
    1578,   440,    84,   161,   151,    79,     4,   151,   578,   453,
      84,   198,   453,   145,   146,   198,   630,  1573,  1574,   660,
     144,   145,   146,   198,    30,   640,   201,   161,   207,   854,
     202,   152,   161,   161,   498,   151,   677,   910,   502,   864,
     206,   415,   198,   507,   145,   146,  1025,    45,   147,  1039,
     198,   425,  1042,   379,  1259,   676,   200,   201,   201,     4,
     676,   682,   144,   145,   146,   509,   682,   805,    14,   168,
     144,   145,   146,   596,   860,   153,   112,   113,   114,   453,
     524,   407,   104,    35,    30,   411,   530,   531,   166,   730,
     112,   113,   114,   115,   116,   117,    98,    99,   100,   713,
      45,    47,   201,   116,   117,   118,   198,   105,    66,    29,
      29,   200,   110,   682,   112,   113,   114,   115,   116,   117,
     118,  1551,   564,   201,   200,  1330,    46,    46,   200,    49,
      49,   200,  1220,   200,  1222,   112,   113,   114,   115,   116,
     117,   715,   716,   715,   716,   200,   123,   124,    49,    50,
      51,   578,   596,    66,    66,   200,   154,   155,   180,   157,
     105,   201,   147,   198,    65,   110,   198,   112,   113,   114,
     115,   116,   117,   118,    66,   112,   113,   114,   115,   116,
     117,   158,   180,   160,   147,   151,   198,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     200,   939,    44,   180,   202,  1006,    65,   168,   147,   154,
     155,  1190,   157,   587,   205,     9,  1206,   294,   127,   147,
     147,   298,   796,  1048,   198,   127,     8,   671,   200,   673,
     168,   696,  1057,   198,   805,   180,    63,    64,    14,    14,
    1328,  1680,   816,   180,  1674,   322,   690,   324,   325,   326,
     327,    79,   200,   824,    14,   829,  1695,   202,   200,  1689,
     704,   705,   168,   690,   705,   199,    14,    96,   642,   199,
     884,  1091,   204,   199,   112,   113,   114,   115,   116,   117,
     198,   104,   198,   198,   690,   123,   124,     9,   199,  1162,
      88,   199,     9,   200,   758,    14,     9,   184,   762,   198,
     127,    79,    79,   187,   678,    79,   680,   198,     9,   753,
     200,   200,     9,    79,   928,   759,   690,   199,   199,   763,
     764,   935,   160,  1411,   200,   125,   753,   198,   702,   199,
      66,   705,  1322,   126,  1072,    30,   859,   167,   128,     9,
     147,   199,   180,   917,   199,   919,     9,   753,   199,    14,
     794,  1171,   794,   794,     9,   196,    66,     9,   169,  1232,
     199,    14,     9,   125,   808,   794,   794,   794,   939,   794,
     205,   205,   199,     9,  1199,  1248,   202,   198,   820,   753,
     824,   198,   198,  1208,   199,   824,   205,   205,   794,   199,
    1031,    96,  1217,   128,   200,   200,   838,   771,    46,    47,
      48,    49,    50,    51,   147,    53,     9,   199,   782,   783,
     198,   198,   198,   147,   198,   859,  1650,    65,   147,   184,
     794,   860,   201,   997,    14,   184,     9,   871,   872,   873,
      79,   896,   201,  1667,    14,    96,   200,    14,   201,    14,
     201,  1675,    46,    47,    48,    49,    50,    51,   205,   200,
    1024,   196,  1026,   897,  1068,   899,   897,   901,    30,  1332,
     198,    65,    30,    14,     9,  1553,   198,   198,  1341,   199,
     198,   198,   899,    14,   901,   200,   920,  1051,   198,   200,
       9,  1354,   128,  1308,   199,     9,    65,   205,   144,    74,
      75,    76,    79,   899,   938,   901,   181,   182,   183,   943,
      85,  1072,  1116,   188,   189,     9,  1120,   192,   193,   198,
     128,    14,   200,  1127,    79,    79,   198,   201,   199,   198,
     198,  1259,   199,   897,   198,   899,   201,   901,   201,   903,
     904,   199,  1106,  1006,   128,     9,   205,   144,    30,    73,
     200,   199,   986,   200,  1417,   989,   169,   132,   133,   134,
     135,   136,   128,    30,   998,   199,   199,   128,   143,     9,
     199,   199,     9,     9,   149,   150,  1010,   199,  1010,  1010,
    1395,   202,   199,    96,  1596,    14,   202,   201,   163,   198,
     201,  1010,  1010,  1010,   199,  1010,   199,   199,   198,   128,
     199,   199,  1330,   199,   179,     9,    30,  1039,   200,   199,
    1042,   199,   201,   200,  1010,   200,   156,    79,   152,    14,
      79,   199,   198,   110,  1228,  1080,   128,   199,   199,   128,
      14,    79,    14,   201,    14,  1069,   199,   201,   198,  1073,
     199,    14,   128,   200,    14,   201,  1010,   199,   199,  1083,
     200,   200,  1083,  1665,   200,   199,   201,    55,    79,   198,
      79,  1095,     9,   200,    79,   108,    96,   147,    96,   159,
    1034,    33,    14,   198,   200,     9,   199,   198,  1095,    79,
    1543,   165,  1545,     4,   162,    79,   201,  1251,   199,  1253,
     200,  1554,   199,   199,    79,    14,    79,    14,  1259,  1095,
      79,    14,    79,    14,   507,  1291,   758,   762,  1658,   386,
     383,   858,  1276,  1077,   855,   385,   797,  1671,  1416,  1083,
    1245,  1667,  1406,  1089,    45,  1289,  1289,  1091,  1092,  1435,
    1519,  1095,  1284,   512,  1699,  1687,  1599,  1169,  1531,    41,
    1402,   981,   978,   483,   483,   389,  1178,  1014,   936,   750,
    1069,   872,  1169,   285,   886,  1055,   292,   310,   941,    -1,
     921,  1178,    -1,   715,    -1,    -1,    -1,    -1,    -1,  1330,
      -1,    -1,    -1,    -1,  1206,  1380,    -1,    26,    27,    -1,
      -1,    30,    -1,    -1,   105,    -1,  1220,    -1,  1222,   110,
      -1,   112,   113,   114,   115,   116,   117,   118,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1279,  1171,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    26,    27,    -1,    -1,
      30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1693,    -1,    -1,   154,   155,    -1,   157,  1700,    -1,    -1,
      -1,  1275,    52,    -1,    -1,  1279,    -1,    -1,    -1,    -1,
    1284,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1275,   180,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,  1291,    -1,    -1,    -1,    -1,  1275,
    1312,   202,    -1,    -1,    -1,    65,  1318,    -1,  1320,    -1,
    1322,    -1,  1288,    -1,  1328,  1312,    -1,    -1,    -1,  1333,
      -1,  1318,    -1,  1320,  1338,    -1,    -1,  1338,  1342,  1382,
      -1,  1275,    -1,    -1,    -1,    -1,  1333,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1342,    -1,    -1,  1533,    -1,
      -1,    -1,    -1,    -1,    -1,  1369,    -1,  1333,    -1,    -1,
      -1,  1375,    -1,    -1,    -1,    -1,  1342,    -1,  1382,    -1,
      -1,    -1,  1386,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     209,    -1,    -1,    -1,    -1,    -1,   215,    -1,    -1,  1333,
      -1,    -1,   221,    -1,  1338,    -1,  1408,  1411,  1342,    -1,
    1414,  1415,  1416,    -1,  1415,  1416,  1420,    -1,  1384,    -1,
      -1,  1408,    -1,  1427,    -1,    -1,    -1,  1414,    -1,   209,
      -1,    -1,    -1,  1420,    -1,   215,    -1,    -1,    -1,    -1,
    1427,   221,    -1,    -1,    -1,    -1,    -1,  1530,  1414,    -1,
      -1,    -1,    -1,    -1,  1420,    -1,    -1,    -1,   277,    -1,
      -1,  1427,    -1,    -1,    -1,   284,   285,    -1,    -1,   249,
     250,    -1,   291,    -1,    -1,   255,    -1,    -1,   297,    -1,
    1414,  1415,  1416,    -1,    -1,    -1,  1420,    -1,    -1,   308,
    1646,    -1,    -1,  1427,    -1,    -1,    -1,   277,    -1,    -1,
      -1,    -1,    -1,    -1,   284,   285,  1589,    -1,    -1,    -1,
      -1,   291,    -1,    -1,    -1,    -1,    -1,   297,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1530,    -1,   308,  1662,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    -1,  1549,  1680,    -1,    -1,  1553,
     330,    -1,    -1,   333,  1558,    -1,   375,    -1,    -1,    -1,
      -1,  1695,    -1,    -1,  1568,    -1,    -1,    -1,     4,  1573,
    1574,  1558,    -1,  1577,  1578,    -1,    -1,    -1,    -1,    63,
      64,    -1,    -1,    -1,    -1,  1589,    -1,    -1,    -1,    -1,
      -1,    -1,  1558,  1597,  1598,   375,   415,    -1,    -1,     4,
    1604,    -1,    -1,    -1,    -1,    -1,   425,    -1,    -1,    45,
    1597,  1598,    -1,    -1,    -1,    -1,    -1,  1604,    -1,    -1,
      -1,    -1,    -1,    -1,  1558,    -1,    -1,    -1,    -1,    -1,
      -1,  1597,  1598,    -1,  1638,   415,    -1,    -1,  1604,    -1,
      45,  1645,    -1,   127,    -1,   425,    -1,    -1,  1650,    -1,
       4,  1638,    -1,    -1,    -1,    -1,    -1,  1661,    -1,  1646,
      -1,    -1,    -1,  1597,  1598,  1667,    -1,    -1,    -1,   105,
    1604,    -1,  1638,  1675,   110,    -1,   112,   113,   114,   115,
     116,   117,   118,    -1,    -1,   465,   466,    -1,    -1,   469,
      -1,    45,    -1,    -1,    -1,    -1,    -1,  1701,    -1,    -1,
     105,    -1,    -1,  1707,  1638,   110,    -1,   112,   113,   114,
     115,   116,   117,   118,  1701,   199,    -1,    29,   154,   155,
    1707,   157,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,   514,  1701,    -1,    -1,    -1,    -1,
      -1,  1707,    -1,    55,   180,    -1,    -1,    65,    -1,   154,
     155,   105,   157,    -1,    -1,    -1,   110,    -1,   112,   113,
     114,   115,   116,   117,   118,    77,   202,  1701,   587,    -1,
      -1,    -1,    -1,  1707,    -1,   180,    -1,    -1,    -1,    -1,
     560,   561,    -1,    -1,    -1,    -1,    -1,    -1,   568,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   202,    -1,    -1,
     154,   155,    -1,   157,    -1,    -1,    -1,   587,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,
      -1,    -1,    -1,   642,    -1,    -1,   180,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,   202,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   678,
     172,   680,   642,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   702,    -1,    -1,   198,    -1,    -1,    -1,
     202,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   678,    -1,
     680,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   702,   703,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   715,   716,   717,   718,   719,
     720,   721,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   729,
      63,    64,   771,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   782,   783,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   754,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   805,    -1,    -1,    -1,
      -1,   771,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     780,    -1,   782,   783,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   127,    -1,   796,   797,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   805,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   816,    77,    -1,    -1,
      -1,   860,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   829,
      26,    27,    -1,    -1,    30,    -1,    -1,   837,    -1,    -1,
     840,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     860,    -1,    -1,    -1,   903,   904,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,   152,    -1,   154,   155,   156,   157,   158,   159,
     939,    -1,    65,   903,   904,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   917,    -1,   919,
      -1,   921,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    -1,   936,    -1,   198,   939,
     940,   941,    -1,    -1,   944,   945,   946,   947,   948,   949,
     950,   951,   952,   953,   954,   955,   956,   957,   958,   959,
     960,   961,   962,   963,   964,   965,   966,   967,   968,   969,
     970,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,   994,  1034,    29,   997,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,   215,
      53,    -1,    -1,    -1,  1024,    -1,  1026,    -1,    -1,    63,
      64,    -1,    65,  1072,  1034,    -1,    -1,    -1,  1077,    -1,
      -1,    -1,   205,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1051,  1091,  1092,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,  1072,    53,    -1,    -1,    -1,  1077,    -1,    -1,
      -1,   277,    -1,    -1,    -1,    65,    -1,    -1,   284,   285,
      -1,  1091,  1092,   127,  1094,   291,    -1,    -1,    -1,    -1,
      -1,   297,    -1,    -1,    -1,    -1,  1106,    -1,    -1,  1109,
      -1,  1111,   308,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1131,  1171,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1168,   202,
      65,  1171,    -1,    -1,    -1,    10,    11,    12,    -1,   375,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,  1195,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,   415,
    1259,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,  1251,    -1,  1253,    -1,    -1,    -1,    -1,  1258,  1259,
      -1,    -1,  1262,    65,  1264,    -1,    -1,  1267,    -1,  1269,
      -1,    -1,  1272,   469,    -1,    52,  1276,    -1,    -1,  1279,
    1280,    -1,  1282,    -1,    -1,    -1,    -1,    -1,    -1,  1289,
      -1,  1330,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1305,    -1,   202,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   514,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   469,
    1330,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1345,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1357,  1358,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   202,    -1,    -1,
      -1,    -1,    -1,    -1,   514,    -1,    -1,    10,    11,    12,
      -1,    -1,  1382,    -1,    -1,    -1,  1386,    -1,    -1,    -1,
      -1,   587,  1392,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,  1423,  1424,  1425,    10,    11,    12,    -1,
    1430,  1431,    65,    -1,    -1,  1435,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,   642,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,   249,   250,    -1,    -1,    -1,    -1,   255,    -1,
      -1,    65,   678,    29,   680,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,   702,   703,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    79,    65,
      -1,   717,   718,   719,   720,   721,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   729,    -1,    -1,    -1,    -1,  1538,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1549,
      -1,    -1,    -1,   330,    -1,    -1,   333,    -1,   754,    -1,
      -1,    -1,   123,   703,    -1,    -1,    -1,    -1,  1568,   202,
      -1,    -1,  1572,    -1,    -1,   771,    -1,   717,   718,   719,
     720,   721,    -1,  1583,   780,    -1,   782,   783,  1588,   729,
      -1,  1591,    -1,   154,   155,    -1,   157,   158,   159,    -1,
      -1,   797,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   805,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   202,    -1,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    -1,    -1,    -1,    -1,    -1,    -1,
     201,   837,   203,    -1,   840,    -1,    -1,    -1,    -1,  1649,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1657,    -1,    -1,
      -1,    -1,    -1,    -1,   860,    -1,    -1,    -1,    -1,    -1,
      -1,  1671,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1680,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   465,   466,
      -1,    -1,   469,    -1,    -1,  1695,    -1,   837,    -1,    -1,
      -1,    -1,    -1,    -1,  1704,    -1,    -1,   903,   904,    -1,
      -1,    -1,  1712,    -1,    -1,    -1,  1716,    -1,    -1,  1719,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   514,    -1,    -1,
     936,    -1,    -1,   939,   940,   941,    -1,    -1,   944,   945,
     946,   947,   948,   949,   950,   951,   952,   953,   954,   955,
     956,   957,   958,   959,   960,   961,   962,   963,   964,   965,
     966,   967,   968,   969,   970,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   560,   561,    -1,    -1,    -1,    -1,    -1,
      -1,   568,    -1,    -1,    -1,    -1,   936,    -1,   994,    -1,
     940,   941,    -1,    -1,   944,   945,   946,   947,   948,   949,
     950,   951,   952,   953,   954,   955,   956,   957,   958,   959,
     960,   961,   962,   963,   964,   965,   966,   967,   968,   969,
     970,    -1,    -1,    -1,    10,    11,    12,    -1,  1034,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      77,    -1,    79,    29,   994,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,  1072,    53,    -1,    -1,
      -1,  1077,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,  1091,  1092,    -1,  1094,    -1,
      77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1109,    -1,  1111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   703,   154,   155,    -1,
     157,   158,   159,    -1,    -1,  1131,    -1,    -1,   715,   716,
     717,   718,   719,   720,   721,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   729,    -1,  1094,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,  1109,
      -1,  1111,  1168,    -1,   201,  1171,   203,   154,   155,    -1,
     157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1131,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1195,
      -1,    -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,   796,
      -1,   198,    -1,    -1,    -1,    -1,   202,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   816,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   829,    -1,    -1,  1195,    -1,    -1,    -1,    -1,
     837,   469,  1258,  1259,    -1,    -1,  1262,    -1,  1264,    -1,
      -1,  1267,    -1,  1269,    -1,    -1,  1272,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1280,    -1,  1282,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,   514,    53,    -1,  1305,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1258,    65,
      -1,    -1,  1262,    -1,  1264,    -1,    -1,  1267,    -1,  1269,
      -1,    -1,  1272,    -1,  1330,    -1,    -1,    -1,    -1,    -1,
     917,    -1,   919,    -1,   921,    -1,    -1,    -1,    -1,  1345,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   936,
      -1,  1357,  1358,   940,   941,  1305,    -1,   944,   945,   946,
     947,   948,   949,   950,   951,   952,   953,   954,   955,   956,
     957,   958,   959,   960,   961,   962,   963,   964,   965,   966,
     967,   968,   969,   970,    -1,    -1,  1392,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1345,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   994,    -1,    -1,
     997,    -1,    -1,    -1,    -1,    -1,    -1,  1423,  1424,  1425,
      -1,    -1,    -1,    -1,  1430,  1431,    -1,    -1,    -1,  1435,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1024,    -1,  1026,
      -1,    -1,  1392,    -1,    -1,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1051,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1423,  1424,  1425,    -1,    -1,    -1,    -1,
    1430,    -1,    -1,    -1,    -1,   703,    -1,    -1,    -1,    -1,
      77,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,   717,
     718,   719,   720,    -1,    -1,    -1,    -1,  1094,    -1,    -1,
      -1,   729,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1106,
      -1,    -1,  1109,    -1,  1111,    -1,    -1,    -1,    -1,    -1,
     117,    -1,  1538,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,    -1,  1131,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,   148,    53,    -1,   151,   152,  1572,   154,   155,    -1,
     157,   158,   159,    -1,    65,    -1,    -1,  1583,    -1,    -1,
      -1,    -1,  1588,    -1,    -1,  1591,    -1,    -1,  1538,    -1,
      -1,    -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,  1195,    -1,
      -1,   198,    -1,    -1,    -1,    -1,   203,    -1,    -1,   837,
      -1,    -1,  1572,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1583,    -1,    -1,    -1,    -1,  1588,    -1,
      -1,  1591,    -1,  1649,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1657,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1251,  1671,  1253,    -1,    -1,    -1,
      -1,  1258,    -1,    -1,    -1,  1262,    -1,  1264,    -1,    -1,
    1267,    -1,  1269,    -1,    -1,  1272,    -1,    -1,    -1,  1276,
      -1,    -1,  1279,    -1,    -1,    -1,    -1,    -1,  1704,  1649,
      -1,    -1,  1289,    -1,    -1,    -1,  1712,    -1,    -1,    -1,
    1716,    -1,    -1,  1719,    -1,    -1,    -1,    -1,  1305,    -1,
      -1,    -1,   940,   941,    -1,    -1,   944,   945,   946,   947,
     948,   949,   950,   951,   952,   953,   954,   955,   956,   957,
     958,   959,   960,   961,   962,   963,   964,   965,   966,   967,
     968,   969,   970,    -1,  1704,    -1,    -1,    -1,  1345,    -1,
      -1,    -1,  1712,    -1,    -1,    -1,  1716,    -1,    -1,  1719,
      -1,    -1,    -1,    -1,    -1,    -1,   994,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1382,    -1,     5,     6,  1386,
       8,     9,    10,    11,    12,  1392,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    -1,    -1,
      28,    29,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    41,    -1,    -1,  1423,  1424,  1425,    -1,
      48,    -1,    50,  1430,    -1,    53,    29,    55,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,  1094,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1109,    -1,  1111,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1131,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1538,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1549,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1195,   186,    -1,
      -1,  1568,    -1,    -1,    -1,  1572,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1583,    -1,    -1,    -1,
      -1,  1588,    -1,    -1,  1591,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   202,
      -1,    -1,   230,    -1,    -1,   233,    -1,    -1,   469,    -1,
      -1,    -1,   240,   241,    -1,    -1,    -1,    -1,    -1,    -1,
    1258,    -1,    -1,    -1,  1262,    -1,  1264,    -1,    -1,  1267,
      -1,  1269,    -1,    -1,  1272,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1649,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   514,    -1,    -1,    -1,    -1,   286,    -1,
      -1,    -1,    -1,    -1,   292,    -1,    -1,  1305,   296,   202,
      -1,    -1,    -1,  1680,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   309,   310,    -1,    -1,    -1,    -1,    -1,  1695,    -1,
      -1,    -1,    -1,   321,    -1,    -1,    -1,  1704,    -1,    -1,
      -1,    -1,    -1,    -1,   332,  1712,    -1,  1345,    -1,  1716,
      -1,    -1,  1719,    -1,    -1,    -1,    -1,    -1,    -1,    77,
      -1,    79,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,    -1,   376,    -1,
     378,   379,    -1,   381,  1392,    -1,    -1,    -1,    -1,    -1,
      -1,   389,   390,   391,   392,   393,   394,   395,   396,   397,
     398,   399,   400,   401,    -1,    -1,    -1,    -1,    -1,   407,
     408,    -1,   410,   411,   412,  1423,  1424,  1425,    -1,    -1,
     418,    -1,  1430,   421,    -1,    -1,   154,   155,    -1,   157,
     158,   159,   430,    -1,   432,    -1,    -1,    -1,    -1,    -1,
     438,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     448,    -1,   450,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,    -1,    -1,    -1,
      -1,    -1,   703,   201,    -1,   203,    -1,    -1,   476,    -1,
      -1,   479,   480,   481,    -1,    -1,   717,   718,   719,   720,
     721,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   729,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   510,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1538,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    -1,  1572,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,  1583,    -1,    -1,    -1,    -1,
    1588,    -1,    -1,  1591,    -1,    -1,    -1,    -1,    -1,    -1,
     588,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   600,    -1,    -1,    -1,   837,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,   632,    53,    -1,    -1,    -1,    -1,
      -1,  1649,    -1,   641,    10,    11,    12,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   659,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,   686,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1704,    -1,    -1,    65,
      -1,    -1,   700,    -1,  1712,   936,   202,    -1,  1716,   940,
     941,  1719,    -1,   944,   945,   946,   947,   948,   949,   950,
     951,   952,   953,   954,   955,   956,   957,   958,   959,   960,
     961,   962,   963,   964,   965,   966,   967,   968,   969,   970,
      29,   739,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,   994,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   202,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   791,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   804,    -1,    -1,    -1,
      -1,   809,    29,   811,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,   202,   835,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   845,    65,    -1,
     848,    -1,   850,    -1,    -1,    -1,   854,    -1,    -1,    -1,
      -1,    -1,    -1,  1094,    -1,    -1,   864,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1109,    -1,
    1111,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,   889,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1131,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    67,    68,    -1,  1195,    -1,    10,    11,    12,    -1,
      -1,    77,    65,    79,   972,   973,   974,    -1,    -1,    -1,
     978,   979,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,  1006,    53,
      -1,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,  1258,    -1,    -1,
      -1,  1262,  1030,  1264,    -1,    -1,  1267,  1035,  1269,    -1,
      -1,  1272,   148,    -1,    -1,   151,   152,    -1,   154,   155,
    1048,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,  1057,
     166,  1059,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   200,    -1,  1305,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,    -1,
    1088,    -1,   198,    -1,    10,    11,    12,   203,    -1,    -1,
    1098,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,  1345,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,   200,    -1,    -1,    -1,
      29,  1392,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,  1181,    53,    -1,    -1,  1185,    -1,  1187,
    1188,    -1,  1423,  1424,  1425,    -1,    65,    -1,    -1,  1430,
      -1,  1199,    68,  1434,    10,    11,    12,    -1,    -1,  1207,
    1208,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,  1217,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    29,    53,    -1,    -1,
      -1,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    55,    -1,    -1,  1273,    -1,    -1,    -1,    -1,
      -1,    -1,   148,    -1,   200,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,    77,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1538,    -1,    -1,
    1308,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,    -1,
      -1,   200,   198,    -1,    -1,    -1,    -1,   203,    -1,    -1,
      -1,  1572,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,
      -1,    -1,  1583,    -1,    -1,    -1,    -1,  1588,    -1,    -1,
    1591,    -1,    -1,    -1,    -1,   148,    -1,    -1,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,
      -1,    -1,    -1,  1614,    -1,    -1,    -1,    -1,    -1,   172,
      -1,    -1,    -1,    -1,   200,    -1,    -1,  1395,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,    -1,    -1,   198,    -1,    -1,  1649,   202,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1426,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1704,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1712,    45,    46,    47,  1716,    -1,    -1,  1719,    52,
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
      -1,    -1,    85,    86,    87,    88,    89,    90,    -1,    92,
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
      88,    -1,    90,    -1,    92,    -1,    94,    95,    -1,    97,
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
      -1,    -1,    85,    86,    87,    88,    -1,    90,    -1,    92,
      93,    94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,
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
      88,    -1,    90,    91,    92,    -1,    94,    -1,    -1,    97,
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
      28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     173,   174,    -1,    -1,    -1,    -1,   179,   180,   181,   182,
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
      -1,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    35,   200,    -1,    -1,   203,   204,    -1,   206,   207,
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
     154,   155,    -1,   157,   158,   159,    -1,   161,    -1,   163,
      -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,
     174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,   198,    -1,    -1,    -1,    -1,   203,
     204,    -1,   206,   207,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
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
      47,    48,    49,    50,    51,    30,    53,    -1,    -1,    -1,
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
     193,   194,   195,    -1,    -1,   198,    35,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,   203,   204,    -1,   206,   207,    -1,
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
      -1,    -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,
      -1,   172,   173,   174,    -1,    -1,    -1,    -1,   179,   180,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,    -1,   198,    -1,    -1,
      -1,    -1,   203,   204,    -1,   206,   207,     3,     4,     5,
       6,     7,    -1,    -1,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    28,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    -1,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,   172,    -1,    -1,   175,
      -1,   177,    -1,   179,   180,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,   200,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,   199,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,   199,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,   194,    -1,    -1,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    29,
      53,    -1,    -1,    -1,    -1,   190,   191,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    55,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,   187,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   112,   113,   114,   115,   116,   117,    -1,   186,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     130,   131,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,   148,    -1,
      -1,   151,   152,    -1,   154,   155,    -1,   157,   158,   159,
      -1,    -1,   185,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,   172,    29,    -1,    -1,    -1,    -1,    -1,    -1,
     180,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   130,   131,    -1,   198,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   148,    -1,    -1,   151,   152,    -1,   154,
     155,    77,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   172,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      -1,    -1,    -1,   198,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   130,   131,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    -1,   148,    -1,    -1,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,    -1,   161,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   172,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   130,
     131,    -1,   198,    55,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,
     151,   152,    -1,   154,   155,    77,   157,   158,   159,    -1,
     161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   172,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    -1,    -1,    -1,   198,    30,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,   121,    -1,
      52,    -1,    54,    -1,    -1,    -1,   148,    -1,    -1,   151,
     152,    -1,   154,   155,    66,   157,   158,   159,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    -1,    -1,    -1,    -1,
     172,   154,   155,    85,   157,   158,   159,    -1,    -1,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    -1,    -1,    -1,   198,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,    -1,    -1,   198,    -1,    -1,   130,    -1,
     132,   133,   134,   135,   136,    -1,    -1,    77,    -1,    79,
      -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,    -1,    77,
      -1,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     172,   173,   174,    -1,    -1,    -1,    -1,   179,    -1,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    46,    47,    -1,   198,    -1,    -1,    52,
      -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    66,   154,   155,    -1,   157,   158,   159,
      -1,    74,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,
     148,    -1,    85,   151,    -1,    -1,   154,   155,    -1,   157,
     158,   159,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    -1,    -1,    -1,    -1,    -1,
      -1,   201,    -1,   203,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   130,    -1,   132,
     133,   134,   135,   136,   202,    77,    -1,    79,    -1,    -1,
     143,    -1,    -1,    85,    -1,   148,   149,   150,   151,   152,
      -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,
     163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   172,
     173,   174,    46,    47,    -1,   117,   179,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    66,    -1,    -1,   198,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    -1,    -1,   148,    -1,    -1,   151,
     152,    85,   154,   155,    -1,   157,   158,   159,    -1,    77,
      -1,    79,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    65,    -1,    -1,   130,    -1,    -1,   117,
      -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   143,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     154,   155,    -1,   157,   158,   159,    74,    75,    76,    77,
     148,    -1,    -1,   151,   152,    -1,   154,   155,   172,   157,
     158,   159,    77,    -1,    79,    -1,    -1,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,    -1,    -1,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,    -1,    -1,    -1,
     198,    -1,   117,   201,    -1,   203,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   154,   155,    -1,   157,
     158,   159,    -1,   148,    -1,    -1,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,    77,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      -1,    -1,    -1,   198,    -1,   117,    -1,    -1,   203,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,    77,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   117,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    -1,    -1,    -1,   198,    -1,    -1,   148,
      -1,   203,   151,   152,    -1,   154,   155,    -1,   157,   158,
     159,    77,    -1,    79,    -1,    -1,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,    65,    -1,    -1,   198,
      -1,   117,    -1,    77,   203,    79,    -1,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    77,    -1,    -1,    -1,    -1,
      -1,    -1,   148,    -1,    -1,   151,   152,    65,   154,   155,
      -1,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,    77,
     154,   155,   198,   157,   158,   159,    -1,   203,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   151,
      -1,    -1,   154,   155,    -1,   157,   158,   159,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,    -1,    -1,    -1,    -1,    -1,    -1,   201,    -1,   203,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    -1,    -1,    -1,    -1,    -1,    -1,   201,
      -1,    -1,    -1,    -1,    -1,    -1,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,    -1,    -1,    29,
     198,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   128,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,   128,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   128,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,   128,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   128,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,   128,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   128,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    29,   128,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   128,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    29,
     128,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    -1,    65,    10,    11,    12,    -1,
      85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    29,   128,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,   118,    -1,    -1,    -1,    77,    -1,   128,    -1,
      -1,    -1,    -1,   148,   130,   131,   151,    -1,    -1,   154,
     155,    77,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   148,   104,   105,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,    -1,    -1,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      -1,    -1,    -1,    -1,   128,    77,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,    77,
     151,    79,    80,   154,   155,    -1,   157,   158,   159,    -1,
      -1,    -1,   148,    -1,    -1,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    77,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,    77,
     152,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    77,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,    77,   151,    -1,
      -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,
      -1,    -1,    -1,   151,    -1,    -1,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,   123,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   151,    -1,    -1,
     154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,    77,
      -1,    -1,    -1,    -1,   154,   155,    -1,   157,   158,   159,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,    12,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   123,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    65,    -1,    -1,    -1,    -1,   154,
     155,    -1,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    29,
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
      46,    47,    48,    49,    50,    51,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    12,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65
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
     336,   337,   338,   339,   340,   342,   345,   356,   357,   358,
     359,   362,   363,   365,   384,   394,   395,   396,   401,   404,
     422,   427,   429,   430,   431,   432,   433,   434,   435,   436,
     438,   455,   457,   459,   115,   116,   117,   129,   148,   158,
     215,   246,   323,   339,   429,   339,   198,   339,   339,   339,
     101,   339,   339,   420,   421,   339,   339,   339,   339,   339,
     339,   339,   339,   339,   339,   339,   339,    79,   117,   198,
     223,   395,   396,   429,   429,    35,   339,   442,   443,   339,
     117,   198,   223,   395,   396,   397,   428,   434,   439,   440,
     198,   330,   398,   198,   330,   346,   331,   339,   232,   330,
     198,   198,   198,   330,   200,   339,   215,   200,   339,    29,
      55,   130,   131,   152,   172,   198,   215,   226,   460,   471,
     472,   181,   200,   336,   339,   364,   366,   201,   239,   339,
     104,   105,   151,   216,   219,   222,    79,   203,   291,   292,
     123,   123,    79,   293,   198,   198,   198,   198,   215,   263,
     461,   198,   198,    79,    84,   144,   145,   146,   452,   453,
     151,   201,   222,   222,   215,   264,   461,   152,   198,   198,
     198,   461,   461,    79,   195,   348,   329,   339,   340,   429,
     228,   201,    84,   399,   452,    84,   452,   452,    30,   151,
     168,   462,   198,     9,   200,    35,   245,   152,   262,   461,
     117,   180,   246,   324,   200,   200,   200,   200,   200,   200,
      10,    11,    12,    29,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    53,    65,   200,    66,    66,
     200,   201,   147,   124,   158,   160,   265,   322,   323,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    63,    64,   127,   424,   425,    66,   201,   426,
     198,    66,   201,   203,   435,   198,   245,   246,    14,   339,
     200,   128,    44,   215,   419,   198,   329,   429,   147,   429,
     128,   205,     9,   406,   329,   429,   462,   147,   198,   400,
     127,   424,   425,   426,   199,   339,    30,   230,     8,   350,
       9,   200,   230,   231,   331,   332,   339,   215,   277,   234,
     200,   200,   200,   472,   472,   168,   198,   104,   472,    14,
     215,    79,   200,   200,   200,   181,   182,   183,   188,   189,
     192,   193,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   379,   380,   381,   240,   108,   165,   200,   151,   217,
     220,   222,   151,   218,   221,   222,   222,     9,   200,    96,
     201,   429,     9,   200,    14,     9,   200,   429,   456,   456,
     329,   340,   429,   199,   168,   257,   129,   429,   441,   442,
      66,   127,   144,   453,    78,   339,   429,    84,   144,   453,
     222,   214,   200,   201,   252,   260,   385,   387,    85,   203,
     351,   352,   354,   396,   435,   457,   339,   448,   449,   448,
      14,    96,   458,   347,   349,   287,   288,   422,   423,   199,
     199,   199,   202,   229,   230,   247,   254,   259,   422,   339,
     204,   206,   207,   215,   463,   464,   472,    35,   161,   289,
     290,   339,   460,   198,   461,   255,   245,   339,   339,   339,
      30,   339,   339,   339,   339,   339,   339,   339,   339,   339,
     339,   339,   339,   339,   339,   339,   339,   339,   339,   339,
     339,   339,   339,   397,   339,   339,   437,   437,   339,   444,
     445,   123,   201,   215,   434,   435,   263,   215,   264,   262,
     246,    27,    35,   333,   336,   339,   364,   339,   339,   339,
     339,   339,   339,   339,   339,   339,   339,   339,   339,   201,
     215,   434,   437,   339,   289,   437,   339,   441,   245,   199,
     339,   198,   418,     9,   406,   329,   199,   215,    35,   339,
      35,   339,   199,   199,   434,   289,   201,   215,   434,   199,
     228,   281,   201,   336,   339,   339,    88,    30,   230,   275,
     200,    28,    96,    14,     9,   199,    30,   201,   278,   472,
      85,   226,   468,   469,   470,   198,     9,    46,    47,    52,
      54,    66,    85,   130,   143,   152,   172,   173,   174,   198,
     223,   224,   226,   360,   361,   395,   401,   402,   403,   184,
      79,   339,    79,    79,   339,   376,   377,   339,   339,   369,
     379,   187,   382,   228,   198,   238,   222,   200,     9,    96,
     222,   200,     9,    96,    96,   219,   215,   339,   292,   402,
      79,     9,   199,   199,   199,   199,   199,   200,   215,   467,
     125,   268,   198,     9,   199,   199,    79,    80,   215,   454,
     215,    66,   202,   202,   211,   213,    30,   126,   267,   167,
      50,   152,   167,   389,   128,     9,   406,   199,   147,   128,
     199,     9,   406,   199,   472,   472,    14,   350,   287,   196,
       9,   407,   472,   473,   127,   424,   425,   426,   202,     9,
     406,   169,   429,   339,   199,     9,   407,    14,   343,   248,
     125,   266,   198,   461,   339,    30,   205,   205,   128,   202,
       9,   406,   339,   462,   198,   258,   253,   261,   256,   245,
      68,   429,   339,   462,   198,   205,   202,   199,   205,   202,
     199,    46,    47,    66,    74,    75,    76,    85,   130,   143,
     172,   215,   409,   411,   414,   417,   215,   429,   429,   128,
     424,   425,   426,   199,   339,   282,    71,    72,   283,   228,
     330,   228,   332,    96,    35,   129,   272,   429,   402,   215,
      30,   230,   276,   200,   279,   200,   279,     9,   169,   128,
     147,     9,   406,   199,   161,   463,   464,   465,   463,   402,
     402,   402,   402,   402,   405,   408,   198,    84,   147,   198,
     198,   198,   402,   147,   201,    10,    11,    12,    29,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      65,   339,   184,   184,    14,   190,   191,   378,     9,   194,
     382,    79,   202,   395,   201,   242,    96,   220,   215,    96,
     221,   215,   215,   202,    14,   429,   200,    96,     9,   169,
     269,   395,   201,   441,   129,   429,    14,   205,   339,   202,
     211,   472,   269,   201,   388,    14,   339,   351,   215,   339,
     339,   200,   472,   196,    30,   466,   423,    35,    79,   161,
     201,   215,   434,   472,    35,   161,   339,   402,   287,   198,
     395,   267,   344,   249,   339,   339,   339,   202,   198,   289,
     268,    30,   267,   266,   461,   397,   202,   198,   289,    14,
      74,    75,    76,   215,   410,   410,   411,   412,   413,   198,
      84,   144,   198,     9,   406,   199,   418,    35,   339,   202,
      71,    72,   284,   330,   230,   202,   200,    89,   200,   272,
     429,   198,   128,   271,    14,   228,   279,    98,    99,   100,
     279,   202,   472,   472,   215,   468,     9,   199,   406,   128,
     205,     9,   406,   405,   215,   351,   353,   355,   402,   450,
     451,   450,   199,   123,   215,   402,   446,   447,   402,   402,
     402,    30,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   402,   402,   402,   402,   402,
     402,   402,   402,   402,   402,   339,   339,   339,   377,   339,
     367,    79,   243,   215,   215,   402,   472,   215,     9,   297,
     199,   198,   333,   336,   339,   205,   202,   458,   297,   153,
     166,   201,   384,   391,   153,   201,   390,   128,   128,   200,
     466,   472,   350,   473,    79,    14,    79,   339,   462,   198,
     429,   339,   199,   287,   201,   287,   198,   128,   198,   289,
     199,   201,   472,   201,   267,   250,   400,   198,   289,   199,
     128,   205,     9,   406,   412,   144,   351,   415,   416,   411,
     429,   330,    30,    73,   230,   200,   332,   271,   441,   272,
     199,   402,    95,    98,   200,   339,    30,   200,   280,   202,
     169,   128,   161,    30,   199,   402,   402,   199,   128,     9,
     406,   199,   128,   199,     9,   406,   199,   128,   202,     9,
     406,   402,    30,   185,   199,   228,    96,   395,     4,   105,
     110,   118,   154,   155,   157,   202,   298,   321,   322,   323,
     328,   422,   441,   202,   201,   202,    50,   339,   339,   339,
     339,   350,    35,    79,   161,    14,   402,   202,   198,   289,
     466,   199,   297,   199,   287,   339,   289,   199,   297,   458,
     297,   201,   198,   289,   199,   411,   411,   199,   128,   199,
       9,   406,    30,   228,   200,   199,   199,   199,   235,   200,
     200,   280,   228,   472,   472,   128,   402,   351,   402,   402,
     402,   402,   402,   339,   201,   202,   472,   125,   126,   460,
     270,   395,   118,   130,   131,   152,   158,   307,   308,   309,
     395,   156,   313,   314,   121,   198,   215,   315,   316,   299,
     246,   472,     9,   200,   322,   199,   294,   152,   386,   202,
     202,    79,    14,    79,   402,   198,   289,   199,   110,   341,
     466,   202,   466,   199,   199,   202,   201,   202,   297,   287,
     199,   128,   411,   351,   228,   233,   236,    30,   230,   274,
     228,   199,   402,   128,   128,   128,   186,   228,   395,   395,
      14,     9,   200,   201,   201,     9,   200,     3,     4,     5,
       6,     7,    10,    11,    12,    13,    27,    28,    53,    67,
      68,    69,    70,    71,    72,    73,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   129,   130,   132,
     133,   134,   135,   136,   148,   149,   150,   160,   162,   163,
     165,   172,   175,   177,   179,   180,   215,   392,   393,     9,
     200,   152,   156,   215,   316,   317,   318,   200,    79,   327,
     245,   300,   460,   246,   202,   295,   296,   460,    14,   402,
     289,   199,   198,   201,   200,   201,   319,   341,   466,   294,
     202,   199,   411,   128,    30,   230,   273,   274,   228,   402,
     402,   402,   339,   202,   200,   200,   402,   395,   303,   310,
     401,   308,    14,    30,    47,   311,   314,     9,    33,   199,
      29,    46,    49,    14,     9,   200,   461,   327,    14,   245,
     200,    14,   402,   199,    35,    79,   383,   228,   228,   201,
     319,   202,   466,   411,   228,    93,   187,   241,   202,   215,
     226,   304,   305,   306,     9,   202,   402,   393,   393,    55,
     312,   317,   317,    29,    46,    49,   402,    79,   198,   200,
     402,   461,   402,    79,     9,   407,   202,   202,   228,   319,
      91,   200,    79,   108,   237,   147,    96,   401,   159,    14,
     301,   198,    35,    79,   199,   202,   200,   198,   165,   244,
     215,   322,   323,   402,   285,   286,   423,   302,    79,   395,
     242,   162,   215,   200,   199,     9,   407,   112,   113,   114,
     325,   326,   285,    79,   270,   200,   466,   423,   473,   199,
     199,   200,   200,   201,   320,   325,    35,    79,   161,   466,
     201,   228,   473,    79,    14,    79,   320,   228,   202,    35,
      79,   161,    14,   402,   202,    79,    14,    79,   402,    14,
     402,   402
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
#line 1874 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(2) - (2)]),NULL,u,(yyvsp[(2) - (2)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
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

  case 443:

/* Line 1455 of yacc.c  */
#line 1891 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1899 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
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

  case 447:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1962 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1967 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1968 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1977 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1982 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1988 "hphp.y"
    { _p->onMapArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1989 "hphp.y"
    { _p->onMapArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1993 "hphp.y"
    { _p->onMapArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1994 "hphp.y"
    { _p->onMapArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 2000 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 2007 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 2009 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 2013 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 2014 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 2015 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 2017 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 2021 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 2025 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 2029 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 2034 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 2036 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 2038 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2040 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2044 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2046 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2050 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2051 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2052 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2053 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2054 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2055 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2059 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2063 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2067 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2072 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2077 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2081 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2085 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2086 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2090 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2091 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2095 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2096 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2100 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2101 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2105 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2109 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2117 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2118 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2120 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2127 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
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

  case 513:

/* Line 1455 of yacc.c  */
#line 2141 "hphp.y"
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

  case 514:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2153 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2159 "hphp.y"
    { (yyval).reset();;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2162 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2163 "hphp.y"
    { (yyval).reset();;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2166 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2170 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2173 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2176 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2183 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2184 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2190 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2195 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2199 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2200 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2201 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2204 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2205 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2207 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2210 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2211 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2213 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2214 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2215 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2216 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2219 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2221 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2222 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2227 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2236 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { (yyval).reset();;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval).reset();;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval).reset();;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval).reset();;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2332 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2338 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2344 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2345 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2349 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2353 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2355 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { (yyval).reset();;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { (yyval).reset();;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { (yyval).reset();;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { (yyval).reset();;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2466 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2467 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2471 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2479 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2480 "hphp.y"
    { (yyval).reset();;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2485 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2487 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2489 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2490 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2494 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2501 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2506 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2509 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { (yyval).reset();;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2518 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2519 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2526 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2528 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2539 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2540 "hphp.y"
    { (yyval).reset();;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2544 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2550 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2551 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2562 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2567 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2569 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2574 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2575 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2576 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2577 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2578 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2580 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2583 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2585 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2590 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2591 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2593 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2595 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2597 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2599 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2600 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2606 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2615 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2618 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2626 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2630 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2641 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2645 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2649 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2651 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2656 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2657 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2661 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2662 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2665 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2666 "hphp.y"
    { (yyval).reset();;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2671 "hphp.y"
    { (yyval)++;;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2675 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2676 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2677 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2679 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2682 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2683 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2687 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2689 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2691 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2692 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2696 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2697 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2699 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2700 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2701 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2702 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2707 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2708 "hphp.y"
    { (yyval).reset();;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2712 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2713 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2714 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2715 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2718 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2720 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2721 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2722 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2727 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2728 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2732 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2733 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2735 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2740 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2741 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2746 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2748 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2750 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2751 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2756 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2757 "hphp.y"
    { _p->onEmptyMapArray((yyval));;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2761 "hphp.y"
    { _p->onMapArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2762 "hphp.y"
    { _p->onMapArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { _p->onEmptyMapArray((yyval));;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2773 "hphp.y"
    { _p->onMapArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2775 "hphp.y"
    { _p->onMapArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2779 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2781 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2782 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2784 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2789 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2791 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2793 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2795 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2797 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2798 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2802 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2803 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2807 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2808 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2809 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2810 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2811 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2812 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2813 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2814 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2815 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2819 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2820 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2825 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2827 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2841 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2845 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2851 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2852 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2858 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2862 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2868 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2869 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2873 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2876 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2882 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2887 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2888 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2889 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2890 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2894 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2895 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2900 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); ;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2901 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); ;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2903 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); ;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2904 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2910 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2915 "hphp.y"
    { ;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2927 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2929 "hphp.y"
    {;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2933 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2940 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 896:

/* Line 1455 of yacc.c  */
#line 2943 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 897:

/* Line 1455 of yacc.c  */
#line 2946 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 898:

/* Line 1455 of yacc.c  */
#line 2947 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 899:

/* Line 1455 of yacc.c  */
#line 2950 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 900:

/* Line 1455 of yacc.c  */
#line 2953 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 901:

/* Line 1455 of yacc.c  */
#line 2955 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 902:

/* Line 1455 of yacc.c  */
#line 2958 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 903:

/* Line 1455 of yacc.c  */
#line 2961 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 904:

/* Line 1455 of yacc.c  */
#line 2967 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 905:

/* Line 1455 of yacc.c  */
#line 2973 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 906:

/* Line 1455 of yacc.c  */
#line 2981 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 907:

/* Line 1455 of yacc.c  */
#line 2982 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13407 "hphp.tab.cpp"
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
#line 2985 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

