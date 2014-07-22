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
#define YYLAST   16201

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  208
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  255
/* YYNRULES -- Number of rules.  */
#define YYNRULES  886
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1667

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
      19,    21,    26,    30,    31,    38,    39,    45,    49,    54,
      59,    62,    64,    66,    68,    70,    72,    74,    76,    78,
      80,    82,    84,    86,    88,    90,    92,    94,    96,    98,
     100,   104,   106,   110,   112,   116,   118,   120,   123,   127,
     132,   134,   137,   141,   146,   148,   151,   155,   160,   162,
     166,   168,   172,   175,   177,   180,   183,   189,   194,   197,
     198,   200,   202,   204,   206,   210,   216,   225,   226,   231,
     232,   239,   240,   251,   252,   257,   260,   264,   267,   271,
     274,   278,   282,   286,   290,   294,   300,   302,   304,   305,
     315,   316,   327,   333,   334,   348,   349,   355,   359,   363,
     366,   369,   372,   375,   378,   381,   385,   388,   391,   395,
     398,   399,   404,   414,   415,   416,   421,   424,   425,   427,
     428,   430,   431,   441,   442,   453,   454,   466,   467,   476,
     477,   487,   488,   496,   497,   506,   507,   515,   516,   525,
     527,   529,   531,   533,   535,   538,   541,   544,   545,   548,
     549,   552,   553,   555,   559,   561,   565,   568,   569,   571,
     574,   579,   581,   586,   588,   593,   595,   600,   602,   607,
     611,   617,   621,   626,   631,   637,   643,   648,   649,   651,
     653,   658,   659,   665,   666,   669,   670,   674,   675,   683,
     690,   693,   699,   704,   705,   710,   716,   724,   731,   738,
     746,   756,   765,   772,   778,   781,   786,   790,   791,   795,
     800,   807,   813,   819,   826,   835,   843,   846,   847,   849,
     852,   855,   859,   864,   869,   873,   875,   877,   880,   885,
     889,   895,   897,   901,   904,   905,   906,   911,   912,   918,
     921,   922,   933,   934,   946,   950,   954,   958,   963,   968,
     972,   978,   981,   984,   985,   992,   998,  1003,  1007,  1009,
    1011,  1015,  1020,  1022,  1024,  1026,  1028,  1033,  1035,  1037,
    1041,  1044,  1045,  1048,  1049,  1051,  1055,  1057,  1059,  1061,
    1063,  1067,  1072,  1077,  1082,  1084,  1086,  1089,  1092,  1095,
    1099,  1103,  1105,  1107,  1109,  1111,  1115,  1117,  1121,  1123,
    1125,  1127,  1128,  1130,  1133,  1135,  1137,  1139,  1141,  1143,
    1145,  1147,  1149,  1150,  1152,  1154,  1156,  1160,  1166,  1168,
    1172,  1178,  1183,  1187,  1191,  1194,  1196,  1198,  1202,  1206,
    1208,  1210,  1211,  1214,  1219,  1223,  1230,  1233,  1237,  1244,
    1246,  1248,  1250,  1257,  1261,  1266,  1273,  1277,  1281,  1285,
    1289,  1293,  1297,  1301,  1305,  1309,  1313,  1317,  1321,  1324,
    1327,  1330,  1333,  1337,  1341,  1345,  1349,  1353,  1357,  1361,
    1365,  1369,  1373,  1377,  1381,  1385,  1389,  1393,  1397,  1401,
    1404,  1407,  1410,  1413,  1417,  1421,  1425,  1429,  1433,  1437,
    1441,  1445,  1449,  1453,  1459,  1464,  1466,  1469,  1472,  1475,
    1478,  1481,  1484,  1487,  1490,  1493,  1495,  1497,  1499,  1501,
    1505,  1508,  1510,  1512,  1514,  1520,  1521,  1522,  1534,  1535,
    1548,  1549,  1553,  1554,  1561,  1564,  1569,  1571,  1573,  1579,
    1583,  1589,  1593,  1596,  1597,  1600,  1601,  1606,  1611,  1615,
    1620,  1625,  1630,  1635,  1640,  1645,  1647,  1649,  1651,  1655,
    1658,  1662,  1667,  1670,  1674,  1676,  1679,  1681,  1684,  1686,
    1688,  1690,  1692,  1694,  1696,  1701,  1706,  1709,  1718,  1729,
    1732,  1734,  1738,  1740,  1743,  1745,  1747,  1749,  1751,  1754,
    1759,  1763,  1767,  1772,  1774,  1777,  1782,  1785,  1792,  1793,
    1795,  1800,  1801,  1804,  1805,  1807,  1809,  1813,  1815,  1819,
    1821,  1823,  1827,  1831,  1833,  1835,  1837,  1839,  1841,  1843,
    1845,  1847,  1849,  1851,  1853,  1855,  1857,  1859,  1861,  1863,
    1865,  1867,  1869,  1871,  1873,  1875,  1877,  1879,  1881,  1883,
    1885,  1887,  1889,  1891,  1893,  1895,  1897,  1899,  1901,  1903,
    1905,  1907,  1909,  1911,  1913,  1915,  1917,  1919,  1921,  1923,
    1925,  1927,  1929,  1931,  1933,  1935,  1937,  1939,  1941,  1943,
    1945,  1947,  1949,  1951,  1953,  1955,  1957,  1959,  1961,  1963,
    1965,  1967,  1969,  1971,  1973,  1975,  1977,  1979,  1981,  1983,
    1985,  1987,  1989,  1991,  1996,  1998,  2000,  2002,  2004,  2006,
    2008,  2010,  2012,  2015,  2017,  2018,  2019,  2021,  2023,  2027,
    2028,  2030,  2032,  2034,  2036,  2038,  2040,  2042,  2044,  2046,
    2048,  2050,  2052,  2054,  2058,  2061,  2063,  2065,  2070,  2074,
    2079,  2081,  2083,  2087,  2091,  2095,  2099,  2103,  2107,  2111,
    2115,  2119,  2123,  2127,  2131,  2135,  2139,  2143,  2147,  2151,
    2155,  2158,  2161,  2164,  2167,  2171,  2175,  2179,  2183,  2187,
    2191,  2195,  2199,  2205,  2210,  2214,  2218,  2222,  2224,  2226,
    2228,  2230,  2234,  2238,  2242,  2245,  2246,  2248,  2249,  2251,
    2252,  2258,  2262,  2266,  2268,  2270,  2272,  2274,  2276,  2280,
    2283,  2285,  2287,  2289,  2291,  2293,  2295,  2298,  2301,  2306,
    2310,  2315,  2318,  2319,  2325,  2329,  2333,  2335,  2339,  2341,
    2344,  2345,  2351,  2355,  2358,  2359,  2363,  2364,  2369,  2372,
    2373,  2377,  2381,  2383,  2384,  2386,  2389,  2392,  2397,  2401,
    2405,  2408,  2413,  2416,  2421,  2423,  2425,  2427,  2429,  2431,
    2434,  2439,  2443,  2448,  2452,  2454,  2456,  2458,  2460,  2463,
    2468,  2473,  2477,  2479,  2481,  2485,  2493,  2500,  2509,  2519,
    2528,  2539,  2547,  2554,  2563,  2565,  2568,  2573,  2578,  2580,
    2582,  2587,  2589,  2590,  2592,  2595,  2597,  2599,  2602,  2607,
    2611,  2615,  2616,  2618,  2621,  2626,  2630,  2633,  2637,  2644,
    2645,  2647,  2652,  2655,  2656,  2662,  2666,  2670,  2672,  2679,
    2684,  2689,  2692,  2695,  2696,  2702,  2706,  2710,  2712,  2715,
    2716,  2722,  2726,  2730,  2732,  2735,  2736,  2742,  2746,  2749,
    2752,  2754,  2757,  2759,  2764,  2768,  2772,  2779,  2783,  2785,
    2787,  2789,  2794,  2799,  2804,  2809,  2812,  2815,  2820,  2823,
    2826,  2828,  2832,  2836,  2840,  2841,  2844,  2850,  2857,  2859,
    2862,  2864,  2869,  2873,  2874,  2876,  2880,  2883,  2887,  2889,
    2891,  2892,  2893,  2896,  2900,  2902,  2908,  2912,  2916,  2922,
    2926,  2928,  2931,  2932,  2937,  2940,  2943,  2945,  2947,  2949,
    2951,  2956,  2963,  2965,  2974,  2981,  2983
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     209,     0,    -1,    -1,   210,   211,    -1,   211,   212,    -1,
      -1,   230,    -1,   247,    -1,   251,    -1,   256,    -1,   448,
      -1,   122,   198,   199,   200,    -1,   148,   222,   200,    -1,
      -1,   148,   222,   201,   213,   211,   202,    -1,    -1,   148,
     201,   214,   211,   202,    -1,   110,   216,   200,    -1,   110,
     104,   217,   200,    -1,   110,   105,   218,   200,    -1,   227,
     200,    -1,    77,    -1,   154,    -1,   155,    -1,   157,    -1,
     159,    -1,   158,    -1,   182,    -1,   183,    -1,   185,    -1,
     184,    -1,   186,    -1,   187,    -1,   188,    -1,   189,    -1,
     190,    -1,   191,    -1,   192,    -1,   193,    -1,   194,    -1,
     216,     9,   219,    -1,   219,    -1,   220,     9,   220,    -1,
     220,    -1,   221,     9,   221,    -1,   221,    -1,   222,    -1,
     151,   222,    -1,   222,    96,   215,    -1,   151,   222,    96,
     215,    -1,   222,    -1,   151,   222,    -1,   222,    96,   215,
      -1,   151,   222,    96,   215,    -1,   222,    -1,   151,   222,
      -1,   222,    96,   215,    -1,   151,   222,    96,   215,    -1,
     215,    -1,   222,   151,   215,    -1,   222,    -1,   148,   151,
     222,    -1,   151,   222,    -1,   223,    -1,   223,   451,    -1,
     223,   451,    -1,   227,     9,   449,    14,   393,    -1,   105,
     449,    14,   393,    -1,   228,   229,    -1,    -1,   230,    -1,
     247,    -1,   251,    -1,   256,    -1,   201,   228,   202,    -1,
      70,   324,   230,   278,   280,    -1,    70,   324,    30,   228,
     279,   281,    73,   200,    -1,    -1,    88,   324,   231,   272,
      -1,    -1,    87,   232,   230,    88,   324,   200,    -1,    -1,
      90,   198,   326,   200,   326,   200,   326,   199,   233,   270,
      -1,    -1,    97,   324,   234,   275,    -1,   101,   200,    -1,
     101,   333,   200,    -1,   103,   200,    -1,   103,   333,   200,
      -1,   106,   200,    -1,   106,   333,   200,    -1,    27,   101,
     200,    -1,   111,   288,   200,    -1,   117,   290,   200,    -1,
      86,   325,   200,    -1,   119,   198,   445,   199,   200,    -1,
     200,    -1,    81,    -1,    -1,    92,   198,   333,    96,   269,
     268,   199,   235,   271,    -1,    -1,    92,   198,   333,    28,
      96,   269,   268,   199,   236,   271,    -1,    94,   198,   274,
     199,   273,    -1,    -1,   107,   239,   108,   198,   386,    79,
     199,   201,   228,   202,   241,   237,   244,    -1,    -1,   107,
     239,   165,   238,   242,    -1,   109,   333,   200,    -1,   102,
     215,   200,    -1,   333,   200,    -1,   327,   200,    -1,   328,
     200,    -1,   329,   200,    -1,   330,   200,    -1,   331,   200,
      -1,   106,   330,   200,    -1,   332,   200,    -1,   356,   200,
      -1,   106,   355,   200,    -1,   215,    30,    -1,    -1,   201,
     240,   228,   202,    -1,   241,   108,   198,   386,    79,   199,
     201,   228,   202,    -1,    -1,    -1,   201,   243,   228,   202,
      -1,   165,   242,    -1,    -1,    35,    -1,    -1,   104,    -1,
      -1,   246,   245,   450,   248,   198,   284,   199,   455,   313,
      -1,    -1,   317,   246,   245,   450,   249,   198,   284,   199,
     455,   313,    -1,    -1,   413,   316,   246,   245,   450,   250,
     198,   284,   199,   455,   313,    -1,    -1,   262,   259,   252,
     263,   264,   201,   291,   202,    -1,    -1,   413,   262,   259,
     253,   263,   264,   201,   291,   202,    -1,    -1,   124,   260,
     254,   265,   201,   291,   202,    -1,    -1,   413,   124,   260,
     255,   265,   201,   291,   202,    -1,    -1,   160,   261,   257,
     264,   201,   291,   202,    -1,    -1,   413,   160,   261,   258,
     264,   201,   291,   202,    -1,   450,    -1,   152,    -1,   450,
      -1,   450,    -1,   123,    -1,   116,   123,    -1,   115,   123,
      -1,   125,   386,    -1,    -1,   126,   266,    -1,    -1,   125,
     266,    -1,    -1,   386,    -1,   266,     9,   386,    -1,   386,
      -1,   267,     9,   386,    -1,   128,   269,    -1,    -1,   420,
      -1,    35,   420,    -1,   129,   198,   432,   199,    -1,   230,
      -1,    30,   228,    91,   200,    -1,   230,    -1,    30,   228,
      93,   200,    -1,   230,    -1,    30,   228,    89,   200,    -1,
     230,    -1,    30,   228,    95,   200,    -1,   215,    14,   393,
      -1,   274,     9,   215,    14,   393,    -1,   201,   276,   202,
      -1,   201,   200,   276,   202,    -1,    30,   276,    98,   200,
      -1,    30,   200,   276,    98,   200,    -1,   276,    99,   333,
     277,   228,    -1,   276,   100,   277,   228,    -1,    -1,    30,
      -1,   200,    -1,   278,    71,   324,   230,    -1,    -1,   279,
      71,   324,    30,   228,    -1,    -1,    72,   230,    -1,    -1,
      72,    30,   228,    -1,    -1,   283,     9,   414,   319,   462,
     161,    79,    -1,   283,     9,   414,   319,   462,   161,    -1,
     283,   398,    -1,   414,   319,   462,   161,    79,    -1,   414,
     319,   462,   161,    -1,    -1,   414,   319,   462,    79,    -1,
     414,   319,   462,    35,    79,    -1,   414,   319,   462,    35,
      79,    14,   393,    -1,   414,   319,   462,    79,    14,   393,
      -1,   283,     9,   414,   319,   462,    79,    -1,   283,     9,
     414,   319,   462,    35,    79,    -1,   283,     9,   414,   319,
     462,    35,    79,    14,   393,    -1,   283,     9,   414,   319,
     462,    79,    14,   393,    -1,   285,     9,   414,   462,   161,
      79,    -1,   285,     9,   414,   462,   161,    -1,   285,   398,
      -1,   414,   462,   161,    79,    -1,   414,   462,   161,    -1,
      -1,   414,   462,    79,    -1,   414,   462,    35,    79,    -1,
     414,   462,    35,    79,    14,   393,    -1,   414,   462,    79,
      14,   393,    -1,   285,     9,   414,   462,    79,    -1,   285,
       9,   414,   462,    35,    79,    -1,   285,     9,   414,   462,
      35,    79,    14,   393,    -1,   285,     9,   414,   462,    79,
      14,   393,    -1,   287,   398,    -1,    -1,   333,    -1,    35,
     420,    -1,   161,   333,    -1,   287,     9,   333,    -1,   287,
       9,   161,   333,    -1,   287,     9,    35,   420,    -1,   288,
       9,   289,    -1,   289,    -1,    79,    -1,   203,   420,    -1,
     203,   201,   333,   202,    -1,   290,     9,    79,    -1,   290,
       9,    79,    14,   393,    -1,    79,    -1,    79,    14,   393,
      -1,   291,   292,    -1,    -1,    -1,   315,   293,   321,   200,
      -1,    -1,   317,   461,   294,   321,   200,    -1,   322,   200,
      -1,    -1,   316,   246,   245,   450,   198,   295,   282,   199,
     455,   314,    -1,    -1,   413,   316,   246,   245,   450,   198,
     296,   282,   199,   455,   314,    -1,   154,   301,   200,    -1,
     155,   307,   200,    -1,   157,   309,   200,    -1,     4,   125,
     386,   200,    -1,     4,   126,   386,   200,    -1,   110,   267,
     200,    -1,   110,   267,   201,   297,   202,    -1,   297,   298,
      -1,   297,   299,    -1,    -1,   226,   147,   215,   162,   267,
     200,    -1,   300,    96,   316,   215,   200,    -1,   300,    96,
     317,   200,    -1,   226,   147,   215,    -1,   215,    -1,   302,
      -1,   301,     9,   302,    -1,   303,   383,   305,   306,    -1,
     152,    -1,   130,    -1,   386,    -1,   118,    -1,   158,   201,
     304,   202,    -1,   131,    -1,   392,    -1,   304,     9,   392,
      -1,    14,   393,    -1,    -1,    55,   159,    -1,    -1,   308,
      -1,   307,     9,   308,    -1,   156,    -1,   310,    -1,   215,
      -1,   121,    -1,   198,   311,   199,    -1,   198,   311,   199,
      49,    -1,   198,   311,   199,    29,    -1,   198,   311,   199,
      46,    -1,   310,    -1,   312,    -1,   312,    49,    -1,   312,
      29,    -1,   312,    46,    -1,   311,     9,   311,    -1,   311,
      33,   311,    -1,   215,    -1,   152,    -1,   156,    -1,   200,
      -1,   201,   228,   202,    -1,   200,    -1,   201,   228,   202,
      -1,   317,    -1,   118,    -1,   317,    -1,    -1,   318,    -1,
     317,   318,    -1,   112,    -1,   113,    -1,   114,    -1,   117,
      -1,   116,    -1,   115,    -1,   180,    -1,   320,    -1,    -1,
     112,    -1,   113,    -1,   114,    -1,   321,     9,    79,    -1,
     321,     9,    79,    14,   393,    -1,    79,    -1,    79,    14,
     393,    -1,   322,     9,   449,    14,   393,    -1,   105,   449,
      14,   393,    -1,   198,   323,   199,    -1,    68,   388,   391,
      -1,    67,   333,    -1,   375,    -1,   350,    -1,   198,   333,
     199,    -1,   325,     9,   333,    -1,   333,    -1,   325,    -1,
      -1,    27,   333,    -1,    27,   333,   128,   333,    -1,   420,
      14,   327,    -1,   129,   198,   432,   199,    14,   327,    -1,
      28,   333,    -1,   420,    14,   330,    -1,   129,   198,   432,
     199,    14,   330,    -1,   334,    -1,   420,    -1,   323,    -1,
     129,   198,   432,   199,    14,   333,    -1,   420,    14,   333,
      -1,   420,    14,    35,   420,    -1,   420,    14,    35,    68,
     388,   391,    -1,   420,    26,   333,    -1,   420,    25,   333,
      -1,   420,    24,   333,    -1,   420,    23,   333,    -1,   420,
      22,   333,    -1,   420,    21,   333,    -1,   420,    20,   333,
      -1,   420,    19,   333,    -1,   420,    18,   333,    -1,   420,
      17,   333,    -1,   420,    16,   333,    -1,   420,    15,   333,
      -1,   420,    64,    -1,    64,   420,    -1,   420,    63,    -1,
      63,   420,    -1,   333,    31,   333,    -1,   333,    32,   333,
      -1,   333,    10,   333,    -1,   333,    12,   333,    -1,   333,
      11,   333,    -1,   333,    33,   333,    -1,   333,    35,   333,
      -1,   333,    34,   333,    -1,   333,    48,   333,    -1,   333,
      46,   333,    -1,   333,    47,   333,    -1,   333,    49,   333,
      -1,   333,    50,   333,    -1,   333,    65,   333,    -1,   333,
      51,   333,    -1,   333,    45,   333,    -1,   333,    44,   333,
      -1,    46,   333,    -1,    47,   333,    -1,    52,   333,    -1,
      54,   333,    -1,   333,    37,   333,    -1,   333,    36,   333,
      -1,   333,    39,   333,    -1,   333,    38,   333,    -1,   333,
      40,   333,    -1,   333,    43,   333,    -1,   333,    41,   333,
      -1,   333,    42,   333,    -1,   333,    53,   388,    -1,   198,
     334,   199,    -1,   333,    29,   333,    30,   333,    -1,   333,
      29,    30,   333,    -1,   444,    -1,    62,   333,    -1,    61,
     333,    -1,    60,   333,    -1,    59,   333,    -1,    58,   333,
      -1,    57,   333,    -1,    56,   333,    -1,    69,   389,    -1,
      55,   333,    -1,   395,    -1,   349,    -1,   348,    -1,   351,
      -1,   204,   390,   204,    -1,    13,   333,    -1,   336,    -1,
     339,    -1,   353,    -1,   110,   198,   374,   398,   199,    -1,
      -1,    -1,   246,   245,   198,   337,   284,   199,   455,   335,
     201,   228,   202,    -1,    -1,   317,   246,   245,   198,   338,
     284,   199,   455,   335,   201,   228,   202,    -1,    -1,    79,
     340,   342,    -1,    -1,   195,   341,   284,   196,   455,   342,
      -1,     8,   333,    -1,     8,   201,   228,   202,    -1,    85,
      -1,   446,    -1,   344,     9,   343,   128,   333,    -1,   343,
     128,   333,    -1,   345,     9,   343,   128,   393,    -1,   343,
     128,   393,    -1,   344,   397,    -1,    -1,   345,   397,    -1,
      -1,   172,   198,   346,   199,    -1,   130,   198,   433,   199,
      -1,    66,   433,   205,    -1,   386,   201,   435,   202,    -1,
     173,   198,   439,   199,    -1,   174,   198,   439,   199,    -1,
     386,   201,   437,   202,    -1,   353,    66,   428,   205,    -1,
     354,    66,   428,   205,    -1,   349,    -1,   446,    -1,    85,
      -1,   198,   334,   199,    -1,   357,   358,    -1,   420,    14,
     355,    -1,   181,    79,   184,   333,    -1,   359,   370,    -1,
     359,   370,   373,    -1,   370,    -1,   370,   373,    -1,   360,
      -1,   359,   360,    -1,   361,    -1,   362,    -1,   363,    -1,
     364,    -1,   365,    -1,   366,    -1,   181,    79,   184,   333,
      -1,   188,    79,    14,   333,    -1,   182,   333,    -1,   183,
      79,   184,   333,   185,   333,   186,   333,    -1,   183,    79,
     184,   333,   185,   333,   186,   333,   187,    79,    -1,   189,
     367,    -1,   368,    -1,   367,     9,   368,    -1,   333,    -1,
     333,   369,    -1,   190,    -1,   191,    -1,   371,    -1,   372,
      -1,   192,   333,    -1,   193,   333,   194,   333,    -1,   187,
      79,   358,    -1,   374,     9,    79,    -1,   374,     9,    35,
      79,    -1,    79,    -1,    35,    79,    -1,   166,   152,   376,
     167,    -1,   378,    50,    -1,   378,   167,   379,   166,    50,
     377,    -1,    -1,   152,    -1,   378,   380,    14,   381,    -1,
      -1,   379,   382,    -1,    -1,   152,    -1,   153,    -1,   201,
     333,   202,    -1,   153,    -1,   201,   333,   202,    -1,   375,
      -1,   384,    -1,   383,    30,   384,    -1,   383,    47,   384,
      -1,   215,    -1,    69,    -1,   104,    -1,   105,    -1,   106,
      -1,    27,    -1,    28,    -1,   107,    -1,   108,    -1,   165,
      -1,   109,    -1,    70,    -1,    71,    -1,    73,    -1,    72,
      -1,    88,    -1,    89,    -1,    87,    -1,    90,    -1,    91,
      -1,    92,    -1,    93,    -1,    94,    -1,    95,    -1,    53,
      -1,    96,    -1,    97,    -1,    98,    -1,    99,    -1,   100,
      -1,   101,    -1,   103,    -1,   102,    -1,    86,    -1,    13,
      -1,   123,    -1,   124,    -1,   125,    -1,   126,    -1,    68,
      -1,    67,    -1,   118,    -1,     5,    -1,     7,    -1,     6,
      -1,     4,    -1,     3,    -1,   148,    -1,   110,    -1,   111,
      -1,   120,    -1,   121,    -1,   122,    -1,   117,    -1,   116,
      -1,   115,    -1,   114,    -1,   113,    -1,   112,    -1,   180,
      -1,   119,    -1,   129,    -1,   130,    -1,    10,    -1,    12,
      -1,    11,    -1,   132,    -1,   134,    -1,   133,    -1,   135,
      -1,   136,    -1,   150,    -1,   149,    -1,   179,    -1,   160,
      -1,   163,    -1,   162,    -1,   175,    -1,   177,    -1,   172,
      -1,   225,   198,   286,   199,    -1,   226,    -1,   152,    -1,
     386,    -1,   117,    -1,   426,    -1,   386,    -1,   117,    -1,
     430,    -1,   198,   199,    -1,   324,    -1,    -1,    -1,    84,
      -1,   441,    -1,   198,   286,   199,    -1,    -1,    74,    -1,
      75,    -1,    76,    -1,    85,    -1,   135,    -1,   136,    -1,
     150,    -1,   132,    -1,   163,    -1,   133,    -1,   134,    -1,
     149,    -1,   179,    -1,   143,    84,   144,    -1,   143,   144,
      -1,   392,    -1,   224,    -1,   130,   198,   396,   199,    -1,
      66,   396,   205,    -1,   172,   198,   347,   199,    -1,   394,
      -1,   352,    -1,   198,   393,   199,    -1,   393,    31,   393,
      -1,   393,    32,   393,    -1,   393,    10,   393,    -1,   393,
      12,   393,    -1,   393,    11,   393,    -1,   393,    33,   393,
      -1,   393,    35,   393,    -1,   393,    34,   393,    -1,   393,
      48,   393,    -1,   393,    46,   393,    -1,   393,    47,   393,
      -1,   393,    49,   393,    -1,   393,    50,   393,    -1,   393,
      51,   393,    -1,   393,    45,   393,    -1,   393,    44,   393,
      -1,   393,    65,   393,    -1,    52,   393,    -1,    54,   393,
      -1,    46,   393,    -1,    47,   393,    -1,   393,    37,   393,
      -1,   393,    36,   393,    -1,   393,    39,   393,    -1,   393,
      38,   393,    -1,   393,    40,   393,    -1,   393,    43,   393,
      -1,   393,    41,   393,    -1,   393,    42,   393,    -1,   393,
      29,   393,    30,   393,    -1,   393,    29,    30,   393,    -1,
     226,   147,   215,    -1,   152,   147,   215,    -1,   226,   147,
     123,    -1,   224,    -1,    78,    -1,   446,    -1,   392,    -1,
     206,   441,   206,    -1,   207,   441,   207,    -1,   143,   441,
     144,    -1,   399,   397,    -1,    -1,     9,    -1,    -1,     9,
      -1,    -1,   399,     9,   393,   128,   393,    -1,   399,     9,
     393,    -1,   393,   128,   393,    -1,   393,    -1,    74,    -1,
      75,    -1,    76,    -1,    85,    -1,   143,    84,   144,    -1,
     143,   144,    -1,    74,    -1,    75,    -1,    76,    -1,   215,
      -1,   400,    -1,   215,    -1,    46,   401,    -1,    47,   401,
      -1,   130,   198,   403,   199,    -1,    66,   403,   205,    -1,
     172,   198,   406,   199,    -1,   404,   397,    -1,    -1,   404,
       9,   402,   128,   402,    -1,   404,     9,   402,    -1,   402,
     128,   402,    -1,   402,    -1,   405,     9,   402,    -1,   402,
      -1,   407,   397,    -1,    -1,   407,     9,   343,   128,   402,
      -1,   343,   128,   402,    -1,   405,   397,    -1,    -1,   198,
     408,   199,    -1,    -1,   410,     9,   215,   409,    -1,   215,
     409,    -1,    -1,   412,   410,   397,    -1,    45,   411,    44,
      -1,   413,    -1,    -1,   416,    -1,   127,   425,    -1,   127,
     215,    -1,   127,   201,   333,   202,    -1,    66,   428,   205,
      -1,   201,   333,   202,    -1,   421,   417,    -1,   198,   323,
     199,   417,    -1,   431,   417,    -1,   198,   323,   199,   417,
      -1,   425,    -1,   385,    -1,   423,    -1,   424,    -1,   418,
      -1,   420,   415,    -1,   198,   323,   199,   415,    -1,   387,
     147,   425,    -1,   422,   198,   286,   199,    -1,   198,   420,
     199,    -1,   385,    -1,   423,    -1,   424,    -1,   418,    -1,
     420,   416,    -1,   198,   323,   199,   416,    -1,   422,   198,
     286,   199,    -1,   198,   420,   199,    -1,   425,    -1,   418,
      -1,   198,   420,   199,    -1,   420,   127,   215,   451,   198,
     286,   199,    -1,   420,   127,   425,   198,   286,   199,    -1,
     420,   127,   201,   333,   202,   198,   286,   199,    -1,   198,
     323,   199,   127,   215,   451,   198,   286,   199,    -1,   198,
     323,   199,   127,   425,   198,   286,   199,    -1,   198,   323,
     199,   127,   201,   333,   202,   198,   286,   199,    -1,   387,
     147,   215,   451,   198,   286,   199,    -1,   387,   147,   425,
     198,   286,   199,    -1,   387,   147,   201,   333,   202,   198,
     286,   199,    -1,   426,    -1,   429,   426,    -1,   426,    66,
     428,   205,    -1,   426,   201,   333,   202,    -1,   427,    -1,
      79,    -1,   203,   201,   333,   202,    -1,   333,    -1,    -1,
     203,    -1,   429,   203,    -1,   425,    -1,   419,    -1,   430,
     415,    -1,   198,   323,   199,   415,    -1,   387,   147,   425,
      -1,   198,   420,   199,    -1,    -1,   419,    -1,   430,   416,
      -1,   198,   323,   199,   416,    -1,   198,   420,   199,    -1,
     432,     9,    -1,   432,     9,   420,    -1,   432,     9,   129,
     198,   432,   199,    -1,    -1,   420,    -1,   129,   198,   432,
     199,    -1,   434,   397,    -1,    -1,   434,     9,   333,   128,
     333,    -1,   434,     9,   333,    -1,   333,   128,   333,    -1,
     333,    -1,   434,     9,   333,   128,    35,   420,    -1,   434,
       9,    35,   420,    -1,   333,   128,    35,   420,    -1,    35,
     420,    -1,   436,   397,    -1,    -1,   436,     9,   333,   128,
     333,    -1,   436,     9,   333,    -1,   333,   128,   333,    -1,
     333,    -1,   438,   397,    -1,    -1,   438,     9,   393,   128,
     393,    -1,   438,     9,   393,    -1,   393,   128,   393,    -1,
     393,    -1,   440,   397,    -1,    -1,   440,     9,   333,   128,
     333,    -1,   333,   128,   333,    -1,   441,   442,    -1,   441,
      84,    -1,   442,    -1,    84,   442,    -1,    79,    -1,    79,
      66,   443,   205,    -1,    79,   127,   215,    -1,   145,   333,
     202,    -1,   145,    78,    66,   333,   205,   202,    -1,   146,
     420,   202,    -1,   215,    -1,    80,    -1,    79,    -1,   120,
     198,   445,   199,    -1,   121,   198,   420,   199,    -1,   121,
     198,   334,   199,    -1,   121,   198,   323,   199,    -1,     7,
     333,    -1,     6,   333,    -1,     5,   198,   333,   199,    -1,
       4,   333,    -1,     3,   333,    -1,   420,    -1,   445,     9,
     420,    -1,   387,   147,   215,    -1,   387,   147,   123,    -1,
      -1,    96,   461,    -1,   175,   450,    14,   461,   200,    -1,
     177,   450,   447,    14,   461,   200,    -1,   215,    -1,   461,
     215,    -1,   215,    -1,   215,   168,   456,   169,    -1,   168,
     453,   169,    -1,    -1,   461,    -1,   452,     9,   461,    -1,
     452,   397,    -1,   452,     9,   161,    -1,   453,    -1,   161,
      -1,    -1,    -1,    30,   461,    -1,   456,     9,   215,    -1,
     215,    -1,   456,     9,   215,    96,   461,    -1,   215,    96,
     461,    -1,    85,   128,   461,    -1,   226,   147,   215,   128,
     461,    -1,   458,     9,   457,    -1,   457,    -1,   458,   397,
      -1,    -1,   172,   198,   459,   199,    -1,    29,   461,    -1,
      55,   461,    -1,   226,    -1,   130,    -1,   131,    -1,   460,
      -1,   130,   168,   461,   169,    -1,   130,   168,   461,     9,
     461,   169,    -1,   152,    -1,   198,   104,   198,   454,   199,
      30,   461,   199,    -1,   198,   461,     9,   452,   397,   199,
      -1,   461,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   732,   732,   732,   741,   743,   746,   747,   748,   749,
     750,   751,   754,   756,   756,   758,   758,   760,   761,   763,
     765,   770,   771,   772,   773,   774,   775,   776,   777,   778,
     779,   780,   781,   782,   783,   784,   785,   786,   787,   788,
     792,   794,   798,   800,   804,   806,   810,   811,   812,   813,
     818,   819,   820,   821,   826,   827,   828,   829,   834,   835,
     839,   840,   842,   845,   851,   858,   865,   869,   875,   877,
     880,   881,   882,   883,   886,   887,   891,   896,   896,   902,
     902,   909,   908,   914,   914,   919,   920,   921,   922,   923,
     924,   925,   926,   927,   928,   929,   930,   931,   934,   932,
     941,   939,   946,   954,   948,   958,   956,   960,   961,   965,
     966,   967,   968,   969,   970,   971,   972,   973,   974,   975,
     983,   983,   988,   994,   998,   998,  1006,  1007,  1011,  1012,
    1016,  1021,  1020,  1033,  1031,  1045,  1043,  1059,  1058,  1077,
    1075,  1094,  1093,  1102,  1100,  1112,  1111,  1123,  1121,  1134,
    1135,  1139,  1142,  1145,  1146,  1147,  1150,  1152,  1155,  1156,
    1159,  1160,  1163,  1164,  1168,  1169,  1174,  1175,  1178,  1179,
    1180,  1184,  1185,  1189,  1190,  1194,  1195,  1199,  1200,  1205,
    1206,  1211,  1212,  1213,  1214,  1217,  1220,  1222,  1225,  1226,
    1230,  1232,  1235,  1238,  1241,  1242,  1245,  1246,  1250,  1256,
    1263,  1265,  1270,  1276,  1280,  1284,  1288,  1293,  1298,  1303,
    1308,  1314,  1323,  1328,  1334,  1336,  1340,  1345,  1349,  1352,
    1355,  1359,  1363,  1367,  1371,  1376,  1384,  1386,  1389,  1390,
    1391,  1392,  1394,  1396,  1401,  1402,  1405,  1406,  1407,  1411,
    1412,  1414,  1415,  1419,  1421,  1424,  1424,  1428,  1427,  1431,
    1435,  1433,  1448,  1445,  1458,  1460,  1462,  1464,  1466,  1468,
    1470,  1474,  1475,  1476,  1479,  1485,  1488,  1494,  1497,  1502,
    1504,  1509,  1514,  1518,  1519,  1525,  1526,  1528,  1532,  1533,
    1538,  1539,  1543,  1544,  1548,  1550,  1556,  1561,  1562,  1564,
    1568,  1569,  1570,  1571,  1575,  1576,  1577,  1578,  1579,  1580,
    1582,  1587,  1590,  1591,  1595,  1596,  1600,  1601,  1604,  1605,
    1608,  1609,  1612,  1613,  1617,  1618,  1619,  1620,  1621,  1622,
    1623,  1627,  1628,  1631,  1632,  1633,  1636,  1638,  1640,  1641,
    1644,  1646,  1651,  1652,  1654,  1655,  1656,  1659,  1663,  1664,
    1668,  1669,  1673,  1674,  1678,  1682,  1687,  1691,  1695,  1700,
    1701,  1702,  1705,  1707,  1708,  1709,  1712,  1713,  1714,  1715,
    1716,  1717,  1718,  1719,  1720,  1721,  1722,  1723,  1724,  1725,
    1726,  1727,  1728,  1729,  1730,  1731,  1732,  1733,  1734,  1735,
    1736,  1737,  1738,  1739,  1740,  1741,  1742,  1743,  1744,  1745,
    1746,  1747,  1748,  1749,  1750,  1751,  1752,  1753,  1754,  1756,
    1757,  1759,  1761,  1762,  1763,  1764,  1765,  1766,  1767,  1768,
    1769,  1770,  1771,  1772,  1773,  1774,  1775,  1776,  1777,  1778,
    1779,  1780,  1781,  1782,  1786,  1790,  1795,  1794,  1809,  1807,
    1824,  1824,  1839,  1839,  1857,  1858,  1863,  1865,  1869,  1873,
    1879,  1883,  1889,  1891,  1895,  1897,  1901,  1905,  1906,  1910,
    1917,  1918,  1922,  1929,  1931,  1936,  1937,  1938,  1940,  1944,
    1948,  1952,  1956,  1958,  1960,  1962,  1967,  1968,  1973,  1974,
    1975,  1976,  1977,  1978,  1982,  1986,  1990,  1994,  1999,  2004,
    2008,  2009,  2013,  2014,  2018,  2019,  2023,  2024,  2028,  2032,
    2036,  2040,  2041,  2042,  2043,  2047,  2053,  2062,  2075,  2076,
    2079,  2082,  2085,  2086,  2089,  2093,  2096,  2099,  2106,  2107,
    2111,  2112,  2114,  2118,  2119,  2120,  2121,  2122,  2123,  2124,
    2125,  2126,  2127,  2128,  2129,  2130,  2131,  2132,  2133,  2134,
    2135,  2136,  2137,  2138,  2139,  2140,  2141,  2142,  2143,  2144,
    2145,  2146,  2147,  2148,  2149,  2150,  2151,  2152,  2153,  2154,
    2155,  2156,  2157,  2158,  2159,  2160,  2161,  2162,  2163,  2164,
    2165,  2166,  2167,  2168,  2169,  2170,  2171,  2172,  2173,  2174,
    2175,  2176,  2177,  2178,  2179,  2180,  2181,  2182,  2183,  2184,
    2185,  2186,  2187,  2188,  2189,  2190,  2191,  2192,  2193,  2194,
    2195,  2196,  2197,  2201,  2206,  2207,  2210,  2211,  2212,  2216,
    2217,  2218,  2222,  2223,  2224,  2228,  2229,  2230,  2233,  2235,
    2239,  2240,  2241,  2242,  2244,  2245,  2246,  2247,  2248,  2249,
    2250,  2251,  2252,  2253,  2256,  2261,  2262,  2263,  2265,  2266,
    2268,  2269,  2271,  2272,  2274,  2276,  2278,  2280,  2282,  2283,
    2284,  2285,  2286,  2287,  2288,  2289,  2290,  2291,  2292,  2293,
    2294,  2295,  2296,  2297,  2298,  2300,  2302,  2304,  2306,  2307,
    2310,  2311,  2315,  2317,  2321,  2324,  2327,  2333,  2334,  2335,
    2336,  2337,  2338,  2339,  2344,  2346,  2350,  2351,  2354,  2355,
    2359,  2362,  2364,  2366,  2370,  2371,  2372,  2373,  2375,  2378,
    2382,  2383,  2384,  2385,  2388,  2389,  2390,  2391,  2392,  2394,
    2395,  2400,  2402,  2405,  2408,  2410,  2412,  2415,  2417,  2421,
    2423,  2426,  2429,  2435,  2437,  2440,  2441,  2446,  2449,  2453,
    2453,  2458,  2461,  2462,  2466,  2467,  2472,  2473,  2477,  2478,
    2482,  2483,  2488,  2490,  2495,  2496,  2497,  2498,  2499,  2500,
    2501,  2503,  2506,  2508,  2512,  2513,  2514,  2515,  2516,  2518,
    2520,  2522,  2526,  2527,  2528,  2532,  2535,  2538,  2541,  2545,
    2549,  2556,  2560,  2564,  2571,  2572,  2577,  2579,  2580,  2583,
    2584,  2587,  2588,  2592,  2593,  2597,  2598,  2599,  2600,  2602,
    2605,  2608,  2609,  2610,  2612,  2614,  2618,  2619,  2620,  2622,
    2623,  2624,  2628,  2630,  2633,  2635,  2636,  2637,  2638,  2641,
    2643,  2644,  2648,  2650,  2653,  2655,  2656,  2657,  2661,  2663,
    2666,  2669,  2671,  2673,  2677,  2679,  2682,  2684,  2688,  2689,
    2691,  2692,  2698,  2699,  2701,  2703,  2705,  2707,  2710,  2711,
    2712,  2716,  2717,  2718,  2719,  2720,  2721,  2722,  2723,  2724,
    2728,  2729,  2733,  2735,  2743,  2745,  2749,  2753,  2760,  2761,
    2767,  2768,  2775,  2778,  2782,  2785,  2790,  2795,  2797,  2798,
    2799,  2803,  2804,  2808,  2810,  2811,  2813,  2817,  2820,  2829,
    2831,  2835,  2838,  2841,  2849,  2852,  2855,  2856,  2859,  2862,
    2863,  2866,  2870,  2874,  2880,  2890,  2891
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
  "class_declaration_statement", "$@17", "$@18", "$@19", "$@20",
  "trait_declaration_statement", "$@21", "$@22", "class_decl_name",
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
  "class_statement_list", "class_statement", "$@23", "$@24", "$@25",
  "$@26", "trait_rules", "trait_precedence_rule", "trait_alias_rule",
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
  "expr_no_variable", "lambda_use_vars", "closure_expression", "$@27",
  "$@28", "lambda_expression", "$@29", "$@30", "lambda_body",
  "shape_keyname", "non_empty_shape_pair_list",
  "non_empty_static_shape_pair_list", "shape_pair_list",
  "static_shape_pair_list", "shape_literal", "array_literal",
  "collection_literal", "map_array_literal", "static_collection_literal",
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
  "static_numeric_scalar_ae", "static_scalar_ae",
  "static_array_pair_list_ae", "non_empty_static_array_pair_list_ae",
  "non_empty_static_scalar_list_ae", "static_shape_pair_list_ae",
  "non_empty_static_shape_pair_list_ae", "static_scalar_list_ae",
  "attribute_static_scalar_list", "non_empty_user_attribute_list",
  "user_attribute_list", "$@31", "non_empty_user_attributes",
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
  "non_empty_map_array_init", "encaps_list", "encaps_var",
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
     212,   212,   212,   213,   212,   214,   212,   212,   212,   212,
     212,   215,   215,   215,   215,   215,   215,   215,   215,   215,
     215,   215,   215,   215,   215,   215,   215,   215,   215,   215,
     216,   216,   217,   217,   218,   218,   219,   219,   219,   219,
     220,   220,   220,   220,   221,   221,   221,   221,   222,   222,
     223,   223,   223,   224,   225,   226,   227,   227,   228,   228,
     229,   229,   229,   229,   230,   230,   230,   231,   230,   232,
     230,   233,   230,   234,   230,   230,   230,   230,   230,   230,
     230,   230,   230,   230,   230,   230,   230,   230,   235,   230,
     236,   230,   230,   237,   230,   238,   230,   230,   230,   230,
     230,   230,   230,   230,   230,   230,   230,   230,   230,   230,
     240,   239,   241,   241,   243,   242,   244,   244,   245,   245,
     246,   248,   247,   249,   247,   250,   247,   252,   251,   253,
     251,   254,   251,   255,   251,   257,   256,   258,   256,   259,
     259,   260,   261,   262,   262,   262,   263,   263,   264,   264,
     265,   265,   266,   266,   267,   267,   268,   268,   269,   269,
     269,   270,   270,   271,   271,   272,   272,   273,   273,   274,
     274,   275,   275,   275,   275,   276,   276,   276,   277,   277,
     278,   278,   279,   279,   280,   280,   281,   281,   282,   282,
     282,   282,   282,   282,   283,   283,   283,   283,   283,   283,
     283,   283,   284,   284,   284,   284,   284,   284,   285,   285,
     285,   285,   285,   285,   285,   285,   286,   286,   287,   287,
     287,   287,   287,   287,   288,   288,   289,   289,   289,   290,
     290,   290,   290,   291,   291,   293,   292,   294,   292,   292,
     295,   292,   296,   292,   292,   292,   292,   292,   292,   292,
     292,   297,   297,   297,   298,   299,   299,   300,   300,   301,
     301,   302,   302,   303,   303,   303,   303,   303,   304,   304,
     305,   305,   306,   306,   307,   307,   308,   309,   309,   309,
     310,   310,   310,   310,   311,   311,   311,   311,   311,   311,
     311,   312,   312,   312,   313,   313,   314,   314,   315,   315,
     316,   316,   317,   317,   318,   318,   318,   318,   318,   318,
     318,   319,   319,   320,   320,   320,   321,   321,   321,   321,
     322,   322,   323,   323,   323,   323,   323,   324,   325,   325,
     326,   326,   327,   327,   328,   329,   330,   331,   332,   333,
     333,   333,   334,   334,   334,   334,   334,   334,   334,   334,
     334,   334,   334,   334,   334,   334,   334,   334,   334,   334,
     334,   334,   334,   334,   334,   334,   334,   334,   334,   334,
     334,   334,   334,   334,   334,   334,   334,   334,   334,   334,
     334,   334,   334,   334,   334,   334,   334,   334,   334,   334,
     334,   334,   334,   334,   334,   334,   334,   334,   334,   334,
     334,   334,   334,   334,   334,   334,   334,   334,   334,   334,
     334,   334,   334,   334,   335,   335,   337,   336,   338,   336,
     340,   339,   341,   339,   342,   342,   343,   343,   344,   344,
     345,   345,   346,   346,   347,   347,   348,   349,   349,   350,
     351,   351,   352,   353,   353,   354,   354,   354,   354,   355,
     356,   357,   358,   358,   358,   358,   359,   359,   360,   360,
     360,   360,   360,   360,   361,   362,   363,   364,   365,   366,
     367,   367,   368,   368,   369,   369,   370,   370,   371,   372,
     373,   374,   374,   374,   374,   375,   376,   376,   377,   377,
     378,   378,   379,   379,   380,   381,   381,   382,   382,   382,
     383,   383,   383,   384,   384,   384,   384,   384,   384,   384,
     384,   384,   384,   384,   384,   384,   384,   384,   384,   384,
     384,   384,   384,   384,   384,   384,   384,   384,   384,   384,
     384,   384,   384,   384,   384,   384,   384,   384,   384,   384,
     384,   384,   384,   384,   384,   384,   384,   384,   384,   384,
     384,   384,   384,   384,   384,   384,   384,   384,   384,   384,
     384,   384,   384,   384,   384,   384,   384,   384,   384,   384,
     384,   384,   384,   384,   384,   384,   384,   384,   384,   384,
     384,   384,   384,   385,   386,   386,   387,   387,   387,   388,
     388,   388,   389,   389,   389,   390,   390,   390,   391,   391,
     392,   392,   392,   392,   392,   392,   392,   392,   392,   392,
     392,   392,   392,   392,   392,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   394,   394,   394,   395,   395,   395,
     395,   395,   395,   395,   396,   396,   397,   397,   398,   398,
     399,   399,   399,   399,   400,   400,   400,   400,   400,   400,
     401,   401,   401,   401,   402,   402,   402,   402,   402,   402,
     402,   403,   403,   404,   404,   404,   404,   405,   405,   406,
     406,   407,   407,   408,   408,   409,   409,   410,   410,   412,
     411,   413,   414,   414,   415,   415,   416,   416,   417,   417,
     418,   418,   419,   419,   420,   420,   420,   420,   420,   420,
     420,   420,   420,   420,   421,   421,   421,   421,   421,   421,
     421,   421,   422,   422,   422,   423,   423,   423,   423,   423,
     423,   424,   424,   424,   425,   425,   426,   426,   426,   427,
     427,   428,   428,   429,   429,   430,   430,   430,   430,   430,
     430,   431,   431,   431,   431,   431,   432,   432,   432,   432,
     432,   432,   433,   433,   434,   434,   434,   434,   434,   434,
     434,   434,   435,   435,   436,   436,   436,   436,   437,   437,
     438,   438,   438,   438,   439,   439,   440,   440,   441,   441,
     441,   441,   442,   442,   442,   442,   442,   442,   443,   443,
     443,   444,   444,   444,   444,   444,   444,   444,   444,   444,
     445,   445,   446,   446,   447,   447,   448,   448,   449,   449,
     450,   450,   451,   451,   452,   452,   453,   454,   454,   454,
     454,   455,   455,   456,   456,   456,   456,   457,   457,   458,
     458,   459,   459,   460,   461,   461,   461,   461,   461,   461,
     461,   461,   461,   461,   461,   462,   462
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
       0,    10,     5,     0,    13,     0,     5,     3,     3,     2,
       2,     2,     2,     2,     2,     3,     2,     2,     3,     2,
       0,     4,     9,     0,     0,     4,     2,     0,     1,     0,
       1,     0,     9,     0,    10,     0,    11,     0,     8,     0,
       9,     0,     7,     0,     8,     0,     7,     0,     8,     1,
       1,     1,     1,     1,     2,     2,     2,     0,     2,     0,
       2,     0,     1,     3,     1,     3,     2,     0,     1,     2,
       4,     1,     4,     1,     4,     1,     4,     1,     4,     3,
       5,     3,     4,     4,     5,     5,     4,     0,     1,     1,
       4,     0,     5,     0,     2,     0,     3,     0,     7,     6,
       2,     5,     4,     0,     4,     5,     7,     6,     6,     7,
       9,     8,     6,     5,     2,     4,     3,     0,     3,     4,
       6,     5,     5,     6,     8,     7,     2,     0,     1,     2,
       2,     3,     4,     4,     3,     1,     1,     2,     4,     3,
       5,     1,     3,     2,     0,     0,     4,     0,     5,     2,
       0,    10,     0,    11,     3,     3,     3,     4,     4,     3,
       5,     2,     2,     0,     6,     5,     4,     3,     1,     1,
       3,     4,     1,     1,     1,     1,     4,     1,     1,     3,
       2,     0,     2,     0,     1,     3,     1,     1,     1,     1,
       3,     4,     4,     4,     1,     1,     2,     2,     2,     3,
       3,     1,     1,     1,     1,     3,     1,     3,     1,     1,
       1,     0,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     1,     1,     1,     3,     5,     1,     3,
       5,     4,     3,     3,     2,     1,     1,     3,     3,     1,
       1,     0,     2,     4,     3,     6,     2,     3,     6,     1,
       1,     1,     6,     3,     4,     6,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     5,     4,     1,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     1,     1,     1,     1,     3,
       2,     1,     1,     1,     5,     0,     0,    11,     0,    12,
       0,     3,     0,     6,     2,     4,     1,     1,     5,     3,
       5,     3,     2,     0,     2,     0,     4,     4,     3,     4,
       4,     4,     4,     4,     4,     1,     1,     1,     3,     2,
       3,     4,     2,     3,     1,     2,     1,     2,     1,     1,
       1,     1,     1,     1,     4,     4,     2,     8,    10,     2,
       1,     3,     1,     2,     1,     1,     1,     1,     2,     4,
       3,     3,     4,     1,     2,     4,     2,     6,     0,     1,
       4,     0,     2,     0,     1,     1,     3,     1,     3,     1,
       1,     3,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     4,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     1,     0,     0,     1,     1,     3,     0,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     2,     1,     1,     4,     3,     4,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     5,     4,     3,     3,     3,     1,     1,     1,
       1,     3,     3,     3,     2,     0,     1,     0,     1,     0,
       5,     3,     3,     1,     1,     1,     1,     1,     3,     2,
       1,     1,     1,     1,     1,     1,     2,     2,     4,     3,
       4,     2,     0,     5,     3,     3,     1,     3,     1,     2,
       0,     5,     3,     2,     0,     3,     0,     4,     2,     0,
       3,     3,     1,     0,     1,     2,     2,     4,     3,     3,
       2,     4,     2,     4,     1,     1,     1,     1,     1,     2,
       4,     3,     4,     3,     1,     1,     1,     1,     2,     4,
       4,     3,     1,     1,     3,     7,     6,     8,     9,     8,
      10,     7,     6,     8,     1,     2,     4,     4,     1,     1,
       4,     1,     0,     1,     2,     1,     1,     2,     4,     3,
       3,     0,     1,     2,     4,     3,     2,     3,     6,     0,
       1,     4,     2,     0,     5,     3,     3,     1,     6,     4,
       4,     2,     2,     0,     5,     3,     3,     1,     2,     0,
       5,     3,     3,     1,     2,     0,     5,     3,     2,     2,
       1,     2,     1,     4,     3,     3,     6,     3,     1,     1,
       1,     4,     4,     4,     4,     2,     2,     4,     2,     2,
       1,     3,     3,     3,     0,     2,     5,     6,     1,     2,
       1,     4,     3,     0,     1,     3,     2,     3,     1,     1,
       0,     0,     2,     3,     1,     5,     3,     3,     5,     3,
       1,     2,     0,     4,     2,     2,     1,     1,     1,     1,
       4,     6,     1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,     0,     0,   719,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   793,     0,
     781,   604,     0,   610,   611,   612,    21,   668,   769,    97,
     613,     0,    79,     0,     0,     0,     0,     0,     0,     0,
       0,   130,     0,     0,     0,     0,     0,     0,   314,   315,
     316,   319,   318,   317,     0,     0,     0,     0,   153,     0,
       0,     0,   617,   619,   620,   614,   615,     0,     0,   621,
     616,     0,   595,    22,    23,    24,    26,    25,     0,   618,
       0,     0,     0,     0,     0,     0,   622,   320,    27,    28,
      30,    29,    31,    32,    33,    34,    35,    36,    37,    38,
      39,   432,     0,    96,    69,   773,   605,     0,     0,     4,
      58,    60,    63,   667,     0,   594,     0,     6,   129,     7,
       8,     9,     0,     0,   312,   351,     0,     0,     0,     0,
       0,     0,     0,   349,   421,   422,   417,   416,   336,   418,
     423,     0,     0,   335,   735,   596,     0,   670,   415,   311,
     738,   350,     0,     0,   736,   737,   734,   764,   768,     0,
     405,   669,    10,   319,   318,   317,     0,     0,    58,   129,
       0,   839,   350,   838,     0,   836,   835,   420,     0,   342,
     346,     0,     0,   389,   390,   391,   392,   414,   412,   411,
     410,   409,   408,   407,   406,   769,   597,     0,   853,   596,
       0,   371,   369,     0,   797,     0,   677,   334,   600,     0,
     853,   599,     0,   609,   776,   775,   601,     0,     0,   603,
     413,     0,     0,     0,     0,   339,     0,    77,   341,     0,
       0,    83,    85,     0,     0,    87,     0,     0,     0,   877,
     878,   882,     0,     0,    58,   876,     0,   879,     0,     0,
      89,     0,     0,     0,     0,   120,     0,     0,     0,     0,
       0,     0,    41,    46,   236,     0,     0,   235,   155,   154,
     241,     0,     0,     0,     0,     0,   850,   141,   151,   789,
     793,   822,     0,   624,     0,     0,     0,   820,     0,    15,
       0,    62,   145,   152,   501,   443,   815,   815,     0,   844,
     723,   351,     0,   349,   350,     0,     0,   606,     0,   607,
       0,     0,     0,   119,     0,     0,    65,   227,     0,    20,
     128,     0,   150,   137,   149,   317,   129,   313,   110,   111,
     112,   113,   114,   116,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   781,
       0,   109,   772,   772,   117,   803,     0,     0,     0,     0,
       0,   310,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   370,   368,     0,   739,   724,
     772,     0,   730,   227,   772,     0,   774,   765,   789,     0,
     129,     0,     0,    91,     0,   721,   716,   677,     0,     0,
       0,     0,   801,     0,   448,   676,   792,     0,     0,    65,
       0,   227,   333,     0,   777,   724,   732,   602,     0,    69,
     191,     0,   431,     0,    94,     0,     0,   340,     0,     0,
       0,     0,     0,    86,   108,    88,   874,   875,     0,   872,
       0,     0,     0,   849,     0,   115,    90,   118,     0,     0,
       0,     0,     0,     0,     0,   459,     0,   466,   468,   469,
     470,   471,   472,   473,   464,   486,   487,    69,     0,   105,
     107,     0,     0,    43,    50,     0,     0,    45,    54,    47,
       0,    17,     0,     0,   237,     0,    92,     0,     0,    93,
     840,     0,     0,   351,   349,   350,     0,     0,   161,     0,
     790,     0,     0,     0,     0,   623,   821,   668,     0,     0,
     819,   673,   818,    61,     5,    12,    13,   159,     0,     0,
     436,     0,     0,   677,     0,     0,   598,   437,     0,     0,
     677,     0,     0,     0,     0,     0,   679,   722,   886,   332,
     402,   743,    74,    68,    70,    71,    72,    73,     0,   419,
     671,   672,    59,   677,     0,   854,     0,     0,     0,   679,
     228,     0,   426,   131,   157,     0,   374,   376,   375,     0,
       0,   372,   373,   377,   379,   378,   394,   393,   396,   395,
     397,   399,   400,   398,   388,   387,   381,   382,   380,   383,
     384,   386,   401,   385,   771,     0,     0,   807,     0,   677,
     843,     0,   842,   741,   764,   143,   147,   139,   129,     0,
       0,   344,   347,   353,   460,   367,   366,   365,   364,   363,
     362,   361,   360,   359,   358,   357,   356,     0,   726,   725,
       0,     0,     0,     0,     0,     0,     0,   837,   343,   714,
     718,   676,   720,     0,     0,   853,     0,   796,     0,   795,
       0,   780,   779,     0,     0,   726,   725,   337,   193,   195,
      69,   434,   338,     0,    69,   175,    78,   341,     0,     0,
       0,     0,     0,   187,   187,    84,     0,     0,     0,   870,
     677,     0,   860,     0,     0,     0,     0,     0,   675,   613,
       0,     0,   595,     0,     0,    63,   626,   594,   631,     0,
     625,    67,   630,     0,     0,   476,     0,     0,   482,   479,
     480,   488,     0,   467,   462,     0,   465,     0,     0,     0,
      51,    18,     0,     0,    55,    19,     0,     0,     0,    40,
      48,     0,   234,   242,   239,     0,     0,   831,   834,   833,
     832,    11,   864,     0,     0,     0,   789,   786,     0,   447,
     830,   829,   828,     0,   824,     0,   825,   827,     0,     5,
       0,     0,   495,   496,   504,   503,     0,     0,   676,   442,
     446,     0,     0,   450,   676,   814,   451,     0,   845,     0,
     861,   723,   214,   885,     0,     0,   740,   724,   731,   770,
     676,   856,   852,   229,   230,   593,   678,   226,     0,   723,
       0,     0,   159,   428,   133,   404,     0,   453,   454,     0,
     449,   676,   802,     0,     0,   227,   161,   159,   157,     0,
     781,   354,     0,     0,   227,   728,   729,   742,   766,   767,
       0,     0,     0,   702,   684,   685,   686,   687,     0,     0,
       0,   695,   694,   708,   677,     0,   716,   800,   799,     0,
     778,   724,   733,   608,     0,   197,     0,     0,    75,     0,
       0,     0,     0,     0,     0,     0,   167,   168,   179,     0,
      69,   177,   102,   187,     0,   187,     0,     0,   880,     0,
       0,   676,   871,   873,   859,   677,   858,     0,   677,   652,
     653,   650,   651,   683,     0,   677,   675,     0,     0,   445,
       0,     0,   809,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   461,
       0,     0,     0,   484,   485,   483,     0,     0,   463,     0,
     121,     0,   124,   106,     0,    42,    52,     0,    44,    56,
      49,   238,     0,   841,    95,     0,     0,   851,   160,   162,
     244,     0,     0,   787,     0,   823,     0,    16,     0,   158,
     244,     0,     0,   439,     0,   842,   817,     0,   846,     0,
       0,     0,   886,     0,   218,   216,     0,   726,   725,   855,
       0,     0,   231,    66,     0,   723,   156,     0,   723,     0,
     403,   806,   805,     0,   227,     0,     0,     0,   159,   135,
     609,   727,   227,     0,     0,   690,   691,   692,   693,   696,
     697,   706,     0,   677,   702,     0,   689,   710,   676,   713,
     715,   717,     0,   794,   727,     0,     0,     0,     0,   194,
     435,    80,     0,   341,   167,   169,   789,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   181,     0,   867,     0,
     869,   676,     0,     0,     0,   628,   676,   674,     0,   665,
       0,   677,     0,   632,   666,   664,   813,     0,   677,   635,
     637,   636,     0,     0,   633,   634,   638,   640,   639,   655,
     654,   657,   656,   658,   660,   661,   659,   648,   647,   642,
     643,   641,   644,   645,   646,   649,   474,     0,   475,   481,
     489,   490,     0,    69,    53,    57,   240,   866,   863,     0,
     311,   791,   789,   345,   348,   352,     0,    14,   311,   507,
       0,     0,   509,   502,   505,     0,   500,     0,     0,   847,
     862,   433,     0,   219,     0,   215,     0,     0,   227,   233,
     232,   861,     0,   244,     0,   723,     0,   227,     0,   762,
     244,   244,     0,     0,   355,   227,     0,   756,     0,   699,
     676,   701,     0,   688,     0,     0,   677,   707,   798,     0,
      69,     0,   190,   176,     0,     0,     0,   166,    98,   180,
       0,     0,   183,     0,   188,   189,    69,   182,   881,     0,
     857,     0,   884,   682,   681,   627,     0,   676,   444,   629,
       0,   452,   676,   808,   663,     0,     0,     0,     0,     0,
     163,     0,     0,     0,   309,     0,     0,     0,   142,   243,
     245,     0,   308,     0,   311,     0,   826,   146,   498,     0,
       0,   438,   816,     0,   222,   213,     0,   221,   727,   227,
       0,   425,   861,   311,   861,     0,   804,     0,   761,   311,
     311,   244,   723,     0,   755,   705,   704,   698,     0,   700,
     676,   709,    69,   196,    76,    81,   100,   170,     0,   178,
     184,    69,   186,   868,     0,     0,   441,     0,   812,   811,
     662,     0,    69,   125,   865,     0,     0,     0,     0,   164,
     275,   273,   277,   595,    26,     0,   269,     0,   274,   286,
       0,   284,   289,     0,   288,     0,   287,     0,   129,   247,
       0,   249,     0,   788,   499,   497,   508,   506,   223,     0,
     212,   220,   227,     0,   759,     0,     0,     0,   138,   425,
     861,   763,   144,   148,   311,     0,   757,     0,   712,     0,
     192,     0,     0,    69,   173,    99,   185,   883,   680,     0,
       0,     0,     0,     0,     0,     0,     0,   259,   263,     0,
       0,   254,   559,   558,   555,   557,   556,   576,   578,   577,
     547,   518,   519,   537,   553,   552,   514,   524,   525,   527,
     526,   546,   530,   528,   529,   531,   532,   533,   534,   535,
     536,   538,   539,   540,   541,   542,   543,   545,   544,   515,
     516,   517,   520,   521,   523,   561,   562,   571,   570,   569,
     568,   567,   566,   554,   573,   563,   564,   565,   548,   549,
     550,   551,   574,   575,   579,   581,   580,   582,   583,   560,
     585,   584,   587,   589,   588,   522,   592,   590,   591,   586,
     572,   513,   281,   510,     0,   255,   302,   303,   301,   294,
       0,   295,   256,   328,     0,     0,     0,     0,   129,     0,
     225,     0,   758,     0,    69,   304,    69,   132,     0,     0,
     140,   861,   703,     0,    69,   171,    82,   101,     0,   440,
     810,   477,   123,   257,   258,   331,   165,     0,     0,   278,
     270,     0,     0,     0,   283,   285,     0,     0,   290,   297,
     298,   296,     0,     0,   246,     0,     0,     0,     0,   224,
     760,     0,   493,   679,     0,     0,    69,   134,     0,   711,
       0,     0,     0,   103,   260,    58,     0,   261,   262,     0,
       0,   276,   280,   511,   512,     0,   271,   299,   300,   292,
     293,   291,   329,   326,   250,   248,   330,     0,   494,   678,
       0,   427,   305,     0,   136,     0,   174,   478,     0,   127,
       0,   311,   279,   282,     0,   723,   252,     0,   491,   424,
     429,   172,     0,     0,   104,   267,     0,   310,   327,     0,
     679,   322,   723,   492,     0,   126,     0,     0,   266,   861,
     723,   200,   323,   324,   325,   886,   321,     0,     0,     0,
     265,     0,   322,     0,   861,     0,   264,   306,    69,   251,
     886,     0,   204,   202,     0,    69,     0,     0,   205,     0,
     201,   253,     0,   307,     0,   208,   199,     0,   207,   122,
     209,     0,   198,   206,     0,   211,   210
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   119,   779,   534,   178,   271,   492,
     496,   272,   493,   497,   121,   122,   123,   124,   125,   126,
     315,   563,   564,   446,   236,  1371,   452,  1298,  1372,  1589,
     739,   266,   487,  1553,   963,  1133,  1604,   331,   179,   565,
     820,  1019,  1183,   566,   584,   838,   518,   836,   567,   537,
     837,   333,   287,   302,   132,   822,   781,   765,   978,  1318,
    1068,   886,  1506,  1375,   686,   892,   451,   695,   894,  1216,
     679,   875,   878,  1057,  1609,  1610,   555,   556,   578,   579,
     276,   277,   281,  1140,  1249,  1337,  1486,  1595,  1612,  1517,
    1557,  1558,  1559,  1325,  1326,  1327,  1518,  1524,  1566,  1330,
    1331,  1335,  1479,  1480,  1481,  1497,  1639,  1250,  1251,   180,
     134,  1625,  1626,  1484,  1253,   135,   229,   447,   448,   136,
     137,   138,   139,   140,   141,   142,   143,  1356,   144,   819,
    1018,   145,   233,   310,   442,   542,   543,  1091,   544,  1092,
     146,   147,   148,   149,   718,   150,   151,   263,   152,   264,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   729,
     730,   955,   484,   485,   486,   736,  1543,   153,   538,  1345,
     539,   991,   786,  1156,  1153,  1472,  1473,   154,   155,   156,
     223,   230,   318,   432,   157,   913,   722,   158,   914,   811,
     802,   915,   862,  1039,  1041,  1042,  1043,   864,  1195,  1196,
     865,   660,   417,   191,   192,   159,   558,   398,   399,   808,
     160,   224,   182,   162,   163,   164,   165,   166,   167,   168,
     615,   169,   226,   227,   521,   215,   216,   618,   619,  1097,
    1098,   549,   550,   296,   297,   773,   170,   511,   171,   554,
     172,   256,   288,   326,   573,   574,   907,  1001,   763,   699,
     700,   701,   257,   258,   804
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1389
static const yytype_int16 yypact[] =
{
   -1389,   142, -1389, -1389,  5700, 12752, 12752,   -51, 12752, 12752,
   12752, 10907, 12752, -1389, 12752, 12752, 12752, 12752, 12752, 12752,
   12752, 12752, 12752, 12752, 12752, 12752, 14425, 14425, 11071, 12752,
   14508,   -41,   -37, -1389, -1389, -1389, -1389, -1389,   218, -1389,
     260, 12752, -1389,   -37,   135,   152,   160,   -37, 11235, 15480,
   11399, -1389, 13714, 10210,   159, 12752, 15209,    -9, -1389, -1389,
   -1389,   263,   267,    40,   185,   197,   201,   210, -1389, 15480,
     220,   246, -1389, -1389, -1389, -1389, -1389,   269, 13966, -1389,
   -1389, 15480, -1389, -1389, -1389, -1389, -1389, -1389, 15480, -1389,
     275,   248,   274,   276, 15480, 15480, -1389, -1389, -1389, -1389,
   -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389,
   -1389, -1389, 12752, -1389, -1389,   238,    50,    53,    53, -1389,
     400,   282,   242, -1389,   285, -1389,    52, -1389,   426, -1389,
   -1389, -1389, 15282,   458, -1389, -1389,   291,   292,   303,   305,
     317,   318,  3685, -1389, -1389, -1389, -1389,   456, -1389, -1389,
     463,   481,   326, -1389,    37,   311,   401, -1389, -1389,   474,
      99,  2274,    61,   357,   101,   106,   378,    30, -1389,    -6,
   -1389,   514, -1389, -1389, -1389,   436,   384,   433, -1389,   426,
     458, 15735,  2544, 15735, 12752, 15735, 15735, 13122,   392, 14718,
   15735,   549, 15480,   529,   529,   329,   529,   529,   529,   529,
     529,   529,   529,   529,   529, -1389, -1389,  2163,   427, -1389,
     462,   480,   480, 14425, 14762,   405,   616, -1389,   436,  2163,
     427,   479,   483,   429,   107, -1389,   506,    61, 11563, -1389,
   -1389, 12752,  8980,   631,    60, 15735, 10005, -1389, 12752, 12752,
   15480, -1389, -1389,  3729,   440, -1389,  4046, 13714, 13714,   473,
   -1389, -1389,   447, 13570,   636, -1389,   637, -1389, 15480,   573,
   -1389,   465,  4097,   468,   454, -1389,    28,  4159, 15295, 15346,
   15480,    68, -1389,   189, -1389, 13773,    69, -1389, -1389, -1389,
     642,    70, 14425, 14425, 12752,   459,   494, -1389, -1389, 14289,
   11071,    44,   211, -1389, 12916, 14425,   341, -1389, 15480, -1389,
     -10,   282, -1389, -1389, -1389,  4855, 12752, 12752,   655,   574,
      19,   472, 15735,   477,   597,  5905, 12752,   224,   469,   312,
     224,   235,   257, -1389, 15480, 13714,   482, 10374, 13714, -1389,
   -1389, 14123, -1389, -1389, -1389, -1389,   426, -1389, -1389, -1389,
   -1389, -1389, -1389, -1389, 12752, 12752, 12752, 11768, 12752, 12752,
   12752, 12752, 12752, 12752, 12752, 12752, 12752, 12752, 12752, 12752,
   12752, 12752, 12752, 12752, 12752, 12752, 12752, 12752, 12752, 14508,
   12752, -1389, 12752, 12752, -1389, 12752,  3400, 15480, 15480, 15282,
     571,   487,  5048, 12752, 12752, 12752, 12752, 12752, 12752, 12752,
   12752, 12752, 12752, 12752, 12752, -1389, -1389,  2757, -1389,   108,
   12752, 12752, -1389, 10374, 12752, 12752,   238,   109, 14289,   484,
     426, 11932,  4554, -1389, 12752, -1389,   486,   668,  2163,   497,
     -30,  3632,   480, 12096, -1389, 12260, -1389,   498,   -14, -1389,
      10, 10374, -1389,  4076, -1389,   112, -1389, -1389,  4806, -1389,
   -1389, 12424, -1389, 12752, -1389,   593,  9185,   677,   499, 15647,
     686,    57,    24, -1389, -1389, -1389, -1389, -1389, 13714,  4805,
     503,   693, 14061, -1389,   519, -1389, -1389, -1389,   625, 12752,
     626,   628, 12752, 12752, 12752, -1389,   454, -1389, -1389, -1389,
   -1389, -1389, -1389, -1389,   524, -1389, -1389, -1389,   515, -1389,
   -1389, 15480,   512,   706,   203, 15480,   518,   710,   255,   265,
   15361, -1389, 15480, 12752,   480,    -9, -1389, 14061,   641, -1389,
     480,    84,    89,   522,   523,  1908,   525, 15480,   602,   530,
     480,    90,   531, 15228, 15480, -1389, -1389,   663,  1832,   -18,
   -1389, -1389, -1389,   282, -1389, -1389, -1389,   605,   565,    62,
   -1389,   238,   606,   726,   537,   591,   109, -1389, 14818,   542,
     733,   546, 13714, 13714,   734,   551,   741, -1389, 13714,    74,
     685,   100, -1389, -1389, -1389, -1389, -1389, -1389,  1968, -1389,
   -1389, -1389, -1389,   743,   584, -1389, 14425, 12752,   559,   750,
   15735,   746, -1389, -1389,   639, 14490, 15862, 15902, 13122, 12752,
   15691, 16011, 16044,  3806, 16075, 11746, 16136, 16136, 16136, 16136,
    1151,  1151,  1151,  1151,   492,   492,   105,   105,   105,   329,
     329,   329, -1389,   529, 15735,   557,   560, 14862,   564,   758,
   -1389, 12752,   -48,   572,   109, -1389, -1389, -1389,   426, 12752,
   14206, -1389, -1389, 13122, -1389, 13122, 13122, 13122, 13122, 13122,
   13122, 13122, 13122, 13122, 13122, 13122, 13122, 12752,   -48,   577,
     566,  2102,   570,   567,  2169,    92,   578, -1389, 15735, 14190,
   -1389, 15480, -1389,   472,    74,   427, 14425, 15735, 14425, 14918,
      80,   114, -1389,   580, 12752, -1389, -1389, -1389,  8775,    81,
   -1389, 15735, 15735,   -37, -1389, -1389, -1389, 12752,   681, 13911,
   14061, 15480,  9390,   583,   592, -1389,    49,   656,   646, -1389,
     785,   596,  3216, 13714, 14061, 14061, 14061, 14061, 14061, -1389,
     599,    32,   652,   604, 14061,   200, -1389,   653, -1389,   603,
   -1389, 15821, -1389, 12752,   621, 15735,   622,   793, 13350,   801,
   -1389, 15735, 13306, -1389,   524,   732, -1389,  6110, 15145,   611,
     268, -1389, 15295, 15480,   271, -1389, 15346, 15480, 15480, -1389,
   -1389,  2449, -1389, 15821,   804, 14425,   619, -1389, -1389, -1389,
   -1389, -1389,   724,    58, 15145,   620, 14289, 14372,   808, -1389,
   -1389, -1389, -1389,   618, -1389, 12752, -1389, -1389,  5290, -1389,
   15145,   623, -1389, -1389, -1389, -1389,   813, 12752,  4855, -1389,
   -1389, 15418, 12752, -1389, 12752, -1389, -1389,   629, -1389, 13714,
     798,     6, -1389, -1389,    51,  4219, -1389,   115, -1389, -1389,
   13714, -1389, -1389,   480, 15735, -1389, 10538, -1389, 14061,    17,
     632, 15145,   605, -1389, -1389, 15977, 12752, -1389, -1389, 12752,
   -1389, 12752, -1389,  2717,   633, 10374,   602,   605,   639, 15480,
   14508,   480,  2841,   634, 10374, -1389, -1389,   116, -1389, -1389,
     822, 14276, 14276, 14190, -1389, -1389, -1389, -1389,   640,   241,
     644, -1389, -1389, -1389,   828,   645,   486,   480,   480, 12588,
   -1389,   117, -1389, -1389,  3060,   132,   -37, 10005, -1389,  6315,
     650,  6520,   651, 13911, 14425,   648,   711,   480, 15821,   826,
   -1389, -1389, -1389, -1389,   416, -1389,     8, 13714, -1389, 13714,
   15480,  4805, -1389, -1389, -1389,   834, -1389,   654,   743,   460,
     460,   789,   789, 15062,   657,   846, 14061,   712, 15480,  4855,
    4874, 15433, 14061, 14061, 14061, 14061, 13863, 14061, 14061, 14061,
   14061, 14061, 14061, 14061, 14061, 14061, 14061, 14061, 14061, 14061,
   14061, 14061, 14061, 14061, 14061, 14061, 14061, 14061, 14061, 15735,
   12752, 12752, 12752, -1389, -1389, -1389, 12752, 12752, -1389,   454,
   -1389,   778, -1389, -1389, 15480, -1389, -1389, 15480, -1389, -1389,
   -1389, -1389, 14061,   480, -1389, 13714, 15480, -1389,   849, -1389,
   -1389,    93,   662,   480, 10743, -1389,  1704, -1389,  5495,   849,
   -1389,   231,   -38, 15735,   735, -1389, 15735, 14962, -1389,   661,
   13714,   631, 13714,   787,   850,   791, 12752,   -48,   670, -1389,
   14425, 12752, 15735, 15821,   673,    17, -1389,   675,    17,   680,
   15977, 15735, 15018,   683, 10374,   684,   687,   688,   605, -1389,
     429,   689, 10374,   691, 12752, -1389, -1389, -1389, -1389, -1389,
   -1389,   754,   679,   877, 14190,   748, -1389,  4855, 14190, -1389,
   -1389, -1389, 14425, 15735, -1389,   -37,   864,   824, 10005, -1389,
   -1389, -1389,   695, 12752,   711,   480, 14289, 13911,   699, 14061,
    6725,   435,   702, 12752,    23,   229, -1389,   736, -1389,   775,
   -1389, 13649,   874,   708, 14061, -1389, 14061, -1389,   709, -1389,
     783,   904,   716, -1389, -1389, -1389, 15118,   714,   909,  4416,
   15942, 14137, 14061, 15779, 13529, 14253, 14555, 16106,  3090,  4610,
    4610,  4610,  4610,  1205,  1205,  1205,  1205,   643,   643,   460,
     460,   460,   789,   789,   789,   789, 15735, 13482, 15735, -1389,
   15735, -1389,   721, -1389, -1389, -1389, 15821, -1389,   825, 15145,
     451, -1389, 14289, -1389, -1389, 13122,   720, -1389,   969, -1389,
      54, 12752, -1389, -1389, -1389, 12752, -1389, 12752, 12752, -1389,
   -1389, -1389,   151,   910, 14061, -1389,  3279,   725, 10374,   480,
   15735,   798,   727, -1389,   729,    17, 12752, 10374,   731, -1389,
   -1389, -1389,   737,   739, -1389, 10374,   740, -1389, 14190, -1389,
   14190, -1389,   742, -1389,   805,   744,   925, -1389,   480,   905,
   -1389,   745, -1389, -1389,   749,   751,    96, -1389, -1389, 15821,
     747,   752, -1389,  3627, -1389, -1389, -1389, -1389, -1389, 13714,
   -1389, 13714, -1389, 15821, 15160, -1389, 14061,  4855, -1389, -1389,
   14061, -1389, 14061, -1389, 14041, 14061, 12752,   753,  6930, 13714,
   -1389,   298, 13714, 15145, -1389, 15164,   784, 14550, -1389, -1389,
   -1389,   571, 13505,    71,   487,    97, -1389, -1389,   790,  3405,
    3500, 15735, 15735,   865,   935,   876, 14061, 15821,   755, 10374,
     757,   847,   798,  1023,   798,   759, 15735,   760, -1389,  1331,
    1352, -1389,    17,   762, -1389, -1389,   836, -1389, 14190, -1389,
    4855, -1389, -1389,  8775, -1389, -1389, -1389, -1389,  9595, -1389,
   -1389, -1389,  8775, -1389,   763, 14061, 15821,   837, 15821, 15216,
   14041, 13438, -1389, -1389, -1389, 15145, 15145,   952,    63, -1389,
   -1389, -1389, -1389,    72,   766,    73, -1389, 13121, -1389, -1389,
      76, -1389, -1389,  2827, -1389,   769, -1389,   891,   426, -1389,
   13714, -1389,   571, -1389, -1389, -1389, -1389, -1389,   957, 14061,
   -1389, 15821, 10374,   776, -1389,   779,   773,   -56, -1389,   847,
     798, -1389, -1389, -1389,  1510,   777, -1389, 14190, -1389,   851,
    8775,  9800,  9595, -1389, -1389, -1389,  8775, -1389, 15821, 14061,
   14061, 12752,  7135,   780,   781, 14061, 15145, -1389, -1389,   302,
   15164, -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389,
   -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389,
   -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389,
   -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389,
   -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389,
   -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389,
   -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389,
   -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389, -1389,
   -1389, -1389,   476, -1389,   784, -1389, -1389, -1389, -1389, -1389,
      67,   363, -1389,   964,    78, 15480,   891,   968,   426, 14061,
   15821,   786, -1389,    83, -1389, -1389, -1389, -1389,   782,   -56,
   -1389,   798, -1389, 14190, -1389, -1389, -1389, -1389,  7340, 15821,
   15821, 13394, -1389, -1389, -1389, 15821, -1389,  2230,    47, -1389,
   -1389, 14061, 13121, 13121,   931, -1389,  2827,  2827,   424, -1389,
   -1389, -1389, 14061,   908, -1389,   795,    79, 14061, 15480, 15821,
   -1389,   915, -1389,   979,  7545,  7750, -1389, -1389,   -56, -1389,
    7955,   796,   916,   881, -1389,   901,   855, -1389, -1389,   907,
     302, -1389, 15821, -1389, -1389,   853, -1389,   973, -1389, -1389,
   -1389, -1389, 15821,   995, -1389, -1389, 15821,   817, -1389,   113,
     819, -1389, -1389,  8160, -1389,   816, -1389, -1389,   823,   857,
   15480,   487, -1389, -1389, 14061,    18, -1389,   941, -1389, -1389,
   -1389, -1389, 15145,   611, -1389,   861, 15480,     9, 15821,   827,
    1015,   446,    18, -1389,   949, -1389, 15145,   829, -1389,   798,
      38, -1389, -1389, -1389, -1389, 13714, -1389,   831,   833,    86,
   -1389,    33,   446,   286,   798,   832, -1389, -1389, -1389, -1389,
   13714,   955,  1025,   958,    33, -1389,  8365,   287,  1027, 14061,
   -1389, -1389,  8570, -1389,   963,  1029,   966, 14061, 15821, -1389,
    1034, 14061, -1389, 15821, 14061, 15821, 15821
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1389, -1389, -1389,  -497, -1389, -1389, -1389,    -4, -1389, -1389,
   -1389,   552,   307,   309,    36,  1020,  3599, -1389,  2264, -1389,
    -427, -1389,     3, -1389, -1389, -1389, -1389, -1389, -1389, -1389,
   -1389, -1389, -1389, -1389,  -552, -1389, -1389,  -168,    31,     2,
   -1389, -1389, -1389,    12, -1389, -1389, -1389, -1389,    13, -1389,
   -1389,   674,   682,   678,   898,   222,  -780,   226,   283,  -551,
       0,  -843, -1389,  -302, -1389, -1389, -1389, -1389,  -648,  -140,
   -1389, -1389, -1389, -1389,  -537, -1389,  -775, -1389,  -360, -1389,
   -1389,   575, -1389,  -922, -1389, -1389, -1389, -1389, -1389, -1389,
   -1389, -1389, -1389, -1389,  -314, -1389, -1389, -1389, -1389, -1389,
    -396, -1389,  -159, -1388, -1389, -1348,  -555, -1389,  -158,    20,
    -131,  -541, -1389,  -394, -1389,   -76,   -24,  1052,  -656,  -355,
   -1389, -1389,   -33, -1389, -1389,  3600,   -62,  -265, -1389, -1389,
   -1389, -1389, -1389, -1389,    98,  -760, -1389, -1389, -1389, -1389,
   -1389, -1389, -1389, -1389, -1389, -1389, -1389,   715, -1389, -1389,
     139, -1389,   627, -1389, -1389, -1389, -1389, -1389, -1389, -1389,
     146, -1389,   635, -1389, -1389,   370, -1389,   118, -1389, -1389,
   -1389, -1389, -1389, -1389, -1389, -1389, -1054, -1389,  1763,  2167,
    -339, -1389, -1389,    75,  3410,  3794, -1389, -1389,   191,  -198,
    -575, -1389, -1389,   256,  -644,    66, -1389, -1389, -1389, -1389,
   -1389,   247, -1389, -1389, -1389,  -269,  -779,  -188,  -171,  -133,
   -1389, -1389,    48, -1389, -1389, -1389, -1389,    29,  -156, -1389,
     104, -1389, -1389, -1389,  -387,   830, -1389, -1389, -1389, -1389,
   -1389,   809, -1389,   461,   232, -1389, -1389,   839,  -291, -1389,
   -1389,  -323,   -85,  -194,  -224,   410, -1389, -1139, -1389,   216,
   -1389, -1389, -1389,  -214,  -977
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -854
static const yytype_int16 yytable[] =
{
     120,   380,   337,   303,   817,   581,   129,   127,   232,   308,
     309,   409,   678,   407,   547,   863,   130,   131,   426,   237,
     261,   655,  1002,   241,   133,  1162,   429,   631,   994,   402,
     612,   882,  1271,   456,   457,   128,   311,   778,   434,   461,
    1064,   557,  1017,   652,  1014,   244,   896,   334,   254,   337,
     313,    13,   161,  1214,   693,   435,  1560,  1027,   897,   225,
     737,   328,    13,    13,    13,   286,   691,   976,  1148,   443,
     274,   673,  1386,   205,   211,   212,  1526,   500,   505,   508,
    1340,  -272,  1390,    13,   286,  1474,  1003,  1533,  1533,   205,
     286,   286,   273,   755,   436,  1386,   404,   397,   755,   767,
    1527,   767,   767,  -744,  1258,   767,   767,  1073,  1074,   397,
     523,   575,   783,   397,   300,  1154,   917,   301,  1541,   280,
     325,    58,    59,    60,   173,   174,   335,   400,   286,   291,
    1004,   419,   291,  1357,   317,  1359,   488,   320,  1567,  1568,
     400,   324,     3,   427,  1495,  1496,   400,   184,  1597,   546,
    -853,  1547,   876,   877,   366,   367,   368,   228,   369,  1090,
     314,   231,  1542,  1155,   336,  -747,  -751,  -745,   585,   561,
     370,   524,  -746,  -782,  -748,   404,   293,  -598,  -783,   381,
    -785,  -749,  -750,  -784,   777,   671,  1263,  -597,   416,    97,
     535,   536,  1598,   489,   275,   294,   295,   406,   294,   295,
    1584,   805,  -678,  1055,  1056,  -678,   304,   433,   513,  1618,
    1076,   410,  1005,   115,   784,  -217,  -217,  -203,   898,   662,
     624,  1499,   514,  1215,  1207,   694,  -430,   977,   120,   785,
    1264,   405,   120,  1637,  1638,   440,   450,  -678,  -744,   445,
    1172,   624,   656,  1174,   696,  1071,   583,  1075,  1182,  1561,
     337,  1273,   329,   879,   463,   420,   692,   881,  1279,  1280,
     444,   422,   401,  1387,  1388,   624,  1528,   428,   501,   506,
     509,  1341,  -272,  1391,   624,   401,  1475,   624,  1534,  1575,
     161,   401,   988,   756,   161,   502,  1636,  1194,   757,   768,
     291,   850,  1141,   303,   334,  1297,  1343,  -753,  -754,   743,
    -747,  -751,  -745,   291,   494,   498,   499,  -746,  -782,  -748,
     405,   120,  1265,  -783,   291,  -785,  -749,  -750,  -784,   530,
     572,  1641,  1654,   504,   254,  1045,  -457,   286,  1073,  1074,
     510,   510,   515,   238,   533,   133,   291,   520,   797,   798,
     324,   530,   663,   529,   803,   789,   128,  -853,   291,   632,
     239,   747,   795,   292,   324,   525,   294,   295,   240,  1364,
     265,   748,  1548,   161,   964,  1642,  1655,   967,   325,   294,
     295,   806,   622,   286,   286,   286,    33,    34,    35,   981,
     294,   295,   369,   282,  1149,  1046,   278,   709,   807,  -853,
     279,   291,  1529,   648,   370,   283,   530,  1150,   225,   284,
    1275,  -853,   294,   295,  1197,   623,   324,  1204,   285,  1530,
     325,   628,  1531,   293,   294,   295,   324,   665,   289,   324,
     291,   832,   324,  1315,  1316,   530,   649,   304,   834,   675,
     323,  1217,  1151,   324,    72,    73,    74,    75,    76,   316,
    -853,   570,   120,  -853,   290,   711,   305,  1643,  1656,   685,
     623,    79,    80,  1569,   843,  1241,   520,   294,   295,   672,
     839,   330,   676,  1070,   571,    89,   420,  1307,  1563,  1564,
    1570,   834,   306,  1571,   307,  1025,   806,   616,   905,   908,
    1631,    96,   870,   327,  1033,   531,   294,   295,   575,   575,
    1521,   338,   339,   807,   161,  1644,    13,   547,   750,   871,
     824,  1030,   902,   340,   650,   341,  1522,  1365,   653,   945,
     946,   947,   375,   762,  1072,  1073,  1074,   342,   343,   772,
     774,   429,  -455,  1523,   526,   948,   374,   740,   532,   372,
    1369,   744,   557,  1211,  1073,  1074,   273,   872,   363,   364,
     365,   366,   367,   368,  1285,   369,  1286,   373,   376,   526,
     557,   532,   526,   532,   532,   403,  1242,   370,  1622,  1623,
    1624,  1243,    51,    58,    59,    60,   173,   174,   335,  1244,
      58,    59,    60,   173,   174,   335,  -752,   319,   321,   322,
    -456,   286,   408,  -597,   298,   999,    58,    59,    60,    61,
      62,   335,   413,   415,   370,   325,  1009,    68,   377,    58,
      59,    60,   173,   174,   335,  1245,  1246,   397,  1247,   421,
     424,   411,   383,   384,   385,   386,   387,   388,   389,   390,
     391,   392,   393,   394,   813,   425,  -596,   431,   547,  1143,
     430,    97,   546,   433,   378,   468,   469,   470,    97,   441,
     454,   458,   471,   472,  1368,   459,   473,   474,  1633,   624,
    -848,   462,   464,  1248,    97,   861,   507,   866,   516,   880,
     395,   396,   517,  1647,  1178,   465,  1049,    97,   467,   552,
     553,   559,  1186,   569,   120,    51,   560,   661,   841,  1206,
     -64,   683,   582,  1077,   659,  1078,   443,   889,   120,   942,
     943,   944,   945,   946,   947,   891,   664,   670,   133,   687,
     690,   702,   703,   723,   724,   726,  1238,   727,   948,   128,
    1083,   735,   741,   738,   867,   742,   868,  1087,   745,   746,
     754,   758,   759,  1502,   397,   761,   161,   764,   766,   775,
     769,   780,   782,   120,   787,   788,   790,   887,   791,   966,
     161,   793,   794,   969,   970,   796,   557,   800,   799,   557,
     801,  -458,   810,   812,  1029,  1255,   547,   133,   815,   816,
     818,  1137,   827,   546,   821,   828,   830,   831,   128,   847,
     835,   845,   848,  1293,   120,   844,   823,   883,   494,   873,
     129,   127,   498,   893,   899,   161,  1160,   995,   803,  1302,
     130,   131,   895,   900,   901,   903,   561,   916,   133,   918,
     921,  1007,   919,   973,   922,   950,   951,   952,  1270,   128,
     956,   959,   962,  1167,   520,   983,  1611,  1277,   972,   974,
     975,   980,   984,   985,   990,  1283,   161,   992,  1000,   998,
    1015,  1024,  1032,  1611,  1008,   286,  1034,  1048,  1044,  1067,
    1069,  1632,  1047,  1081,  1050,  1191,  1066,  1038,  1038,   861,
    1061,  1063,  1058,  1082,   948,  1086,   525,  1132,  1139,  1549,
    1142,  1159,  1085,  1157,  1164,  1370,  1163,  1009,  1168,   225,
    1165,  1254,  1171,   120,  1376,   120,  1173,   120,  1175,  1254,
    1059,  1177,  1188,  1179,  1189,  1382,  1190,  1185,  1180,  1181,
    1187,   546,  1193,  1228,  1200,  1203,  1079,  1201,  1208,   133,
    1233,   133,  1212,  1219,  1221,  1218,   557,  1222,  1225,  1353,
     128,  1226,   128,  1227,  1089,  1229,  1231,  1095,  1232,  1317,
    1237,  1239,  1256,  1269,  1266,   161,  1272,   161,  1274,   161,
    1278,   887,  1065,  1288,  1290,  1292,   547,  1282,  1281,  1284,
    1329,  1287,  1344,  1289,  1348,  1294,  1508,  1299,  1295,  1349,
    1296,  1144,  1300,  1352,  1312,  1350,  1354,  1355,  1360,  1361,
    1134,  1366,  1377,  1135,  1367,  1379,  1385,  1389,  1580,  1482,
    1483,  1489,  1138,  1241,  1494,  1492,  1501,  1493,  1532,  1503,
    1513,  1514,  1537,  1546,   120,  1540,  1565,  1573,  1579,  1588,
     129,   127,  1491,  1574,  1578,  1587,  1586,  -268,  1291,   547,
     130,   131,  1590,  1591,  1254,  1303,  1527,  1304,   133,  1594,
    1254,  1254,  1593,   557,    13,  1596,  1601,  1487,  1599,   128,
    1613,  1602,  1603,  1616,  1620,  1314,  1619,  1241,  1628,  1630,
    1634,  1199,  1635,  1645,  1648,  1621,   161,  1650,  1339,  1649,
     861,  1657,  1660,  1661,   861,  1662,   208,   208,  1664,   965,
     220,  1615,   749,   627,   120,   968,   626,   379,  1169,   625,
    1028,  1202,  1026,   989,  1205,  1629,   120,  1544,    13,  1545,
    1507,   546,   220,  1301,  1242,  1627,  1520,  1550,  1525,  1243,
     752,    58,    59,    60,   173,   174,   335,  1244,  1336,  1651,
     133,  1640,  1536,   234,  1498,  1254,  1342,   634,  1131,  1161,
    1198,   128,  1129,   733,   958,  1184,   161,  1088,  1040,  1152,
    1192,   734,   906,  1051,   520,   887,   551,  1080,   161,  1583,
     522,   337,   512,  1245,  1246,     0,  1247,     0,  1242,     0,
       0,     0,     0,  1243,   546,    58,    59,    60,   173,   174,
     335,  1244,     0,     0,     0,     0,     0,     0,     0,    97,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1252,     0,     0,     0,     0,     0,     0,     0,  1252,     0,
    1485,  1257,     0,     0,     0,     0,     0,  1245,  1246,     0,
    1247,     0,     0,     0,   861,     0,   861,     0,     0,     0,
     520,  -854,  -854,  -854,  -854,   361,   362,   363,   364,   365,
     366,   367,   368,    97,   369,     0,     0,     0,     0,     0,
       0,  1646,     0,     0,     0,     0,   370,     0,  1652,     0,
       0,     0,     0,     0,     0,  1358,     0,   208,     0,     0,
       0,     0,     0,   208,   120,     0,     0,     0,   254,   208,
       0,     0,     0,  1334,     0,  -854,  -854,  -854,  -854,   940,
     941,   942,   943,   944,   945,   946,   947,     0,   133,     0,
       0,     0,     0,     0,     0,     0,     0,   220,   220,   128,
     948,     0,     0,   220,   381,     0,     0,     0,     0,     0,
       0,     0,  1338,     0,   861,     0,   161,     0,     0,   120,
       0,     0,     0,  1252,   120,   208,     0,     0,   120,  1252,
    1252,  1374,   208,   208,     0,     0,     0,     0,     0,   208,
       0,     0,     0,   133,     0,   208,     0,     0,     0,     0,
    1538,     0,   133,  1471,   128,   220,   557,     0,     0,  1478,
       0,     0,     0,   128,     0,  1241,   254,     0,     0,     0,
       0,   161,     0,   557,     0,   220,   161,     0,   220,     0,
     161,   557,     0,     0,     0,     0,  1241,     0,     0,     0,
       0,     0,     0,   861,     0,     0,   120,   120,   120,     0,
       0,     0,   120,  1488,  1505,  1374,    13,     0,   120,     0,
       0,     0,     0,     0,  1252,     0,     0,     0,     0,   220,
     133,     0,     0,     0,     0,     0,   133,    13,     0,     0,
    1535,   128,   133,     0,     0,     0,     0,   128,     0,     0,
       0,   803,     0,   128,     0,     0,     0,     0,   161,   161,
     161,     0,     0,     0,   161,     0,   803,     0,   208,     0,
     161,     0,     0,  1606,     0,     0,  1242,     0,   208,     0,
       0,  1243,     0,    58,    59,    60,   173,   174,   335,  1244,
       0,     0,     0,  1577,     0,     0,     0,  1242,     0,     0,
       0,     0,  1243,     0,    58,    59,    60,   173,   174,   335,
    1244,     0,     0,     0,     0,     0,   337,     0,   220,   220,
       0,   286,   715,     0,     0,  1245,  1246,     0,  1247,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   861,
       0,     0,     0,     0,   120,     0,  1245,  1246,     0,  1247,
       0,    97,     0,  1555,  1241,     0,     0,     0,  1471,  1471,
       0,     0,  1478,  1478,     0,     0,     0,   715,   133,     0,
       0,     0,    97,  1362,   286,     0,     0,     0,     0,   128,
     120,   120,     0,     0,     0,     0,   120,     0,     0,     0,
       0,     0,     0,     0,  1363,    13,   161,     0,     0,     0,
       0,     0,     0,     0,   133,   133,     0,     0,     0,     0,
     133,     0,   220,   220,     0,   128,   128,     0,   220,   120,
       0,   128,     0,     0,     0,     0,  1605,     0,     0,     0,
       0,     0,   161,   161,     0,     0,   208,     0,   161,     0,
       0,     0,  1617,   133,     0,     0,     0,     0,     0,     0,
       0,  1607,     0,     0,   128,  1242,     0,     0,     0,     0,
    1243,     0,    58,    59,    60,   173,   174,   335,  1244,     0,
       0,   161,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   120,     0,     0,     0,     0,     0,   120,     0,
     208,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1245,  1246,   133,  1247,     0,     0,
       0,     0,   133,     0,     0,     0,     0,   128,     0,     0,
       0,     0,     0,   128,     0,     0,   208,     0,   208,     0,
      97,     0,     0,     0,   161,     0,     0,     0,     0,     0,
     161,     0,     0,     0,     0,     0,     0,     0,     0,   208,
     715,     0,  1500,     0,   344,   345,   346,     0,     0,     0,
       0,     0,   220,   220,   715,   715,   715,   715,   715,     0,
       0,     0,     0,   347,   715,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,     0,   369,   220,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   370,
       0,     0,     0,     0,     0,   208,     0,     0,     0,     0,
       0,     0,     0,     0,   220,     0,   208,   208,     0,   209,
     209,     0,     0,   221,     0,     0,     0,     0,     0,     0,
     220,     0,     0,     0,     0,     0,     0,     0,   220,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   220,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     220,     0,     0,     0,     0,     0,     0,     0,   715,     0,
       0,   220,   344,   345,   346,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     220,   347,     0,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,     0,   369,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   370,     0,     0,
       0,     0,     0,   208,   208,     0,     0,     0,     0,  1146,
       0,     0,     0,     0,     0,     0,     0,   220,     0,   220,
       0,   220,   411,   383,   384,   385,   386,   387,   388,   389,
     390,   391,   392,   393,   394,     0,   715,     0,     0,   220,
       0,     0,   715,   715,   715,   715,   715,   715,   715,   715,
     715,   715,   715,   715,   715,   715,   715,   715,   715,   715,
     715,   715,   715,   715,   715,   715,   715,   715,   715,     0,
       0,   395,   396,     0,     0,     0,   209,     0,   344,   345,
     346,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   715,     0,     0,   220,     0,   347,     0,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     220,   369,   220,     0,     0,     0,     0,     0,     0,     0,
     208,     0,     0,   370,   776,   397,     0,     0,   209,     0,
       0,     0,     0,     0,     0,   209,   209,     0,     0,     0,
       0,     0,   209,     0,     0,     0,     0,     0,   209,     0,
       0,     0,     0,     0,     0,     0,     0,   220,   209,     0,
       0,     0,   208,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   208,   208,     0,   715,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   220,     0,     0,   715,     0,   715,   760,     0,     0,
       0,     0,   344,   345,   346,     0,     0,     0,     0,     0,
       0,     0,   715,     0,     0,     0,     0,     0,     0,     0,
       0,   347,   221,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,     0,   369,     0,     0,     0,   220,
       0,     0,   208,     0,     0,     0,     0,   370,     0,     0,
     809,   209,     0,     0,     0,     0,     0,     0,     0,   344,
     345,   346,     0,     0,   715,     0,     0,     0,     0,     0,
       0,     0,     0,   210,   210,     0,     0,   222,   347,     0,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,     0,   369,     0,     0,   719,     0,     0,     0,     0,
      29,    30,     0,     0,   370,     0,     0,     0,     0,   220,
      36,   220,   205,     0,     0,     0,   715,   220,     0,     0,
     715,     0,   715,     0,     0,   715,     0,     0,     0,   220,
       0,     0,   220,   220,     0,   220,     0,     0,     0,     0,
     719,     0,   220,     0,     0,     0,     0,     0,     0,     0,
     206,     0,     0,     0,     0,     0,   715,     0,   382,   383,
     384,   385,   386,   387,   388,   389,   390,   391,   392,   393,
     394,     0,     0,     0,   846,     0,     0,    36,     0,     0,
     220,   177,     0,     0,    81,    82,   255,    83,    84,     0,
      85,    86,    87,     0,     0,   715,     0,     0,     0,    90,
       0,     0,     0,     0,     0,   220,   220,   395,   396,   209,
       0,     0,     0,     0,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     220,   418,     0,     0,     0,     0,   115,     0,     0,   715,
       0,   849,     0,     0,   210,     0,     0,     0,   177,     0,
     210,    81,     0,     0,    83,    84,   210,    85,    86,    87,
       0,     0,     0,   209,     0,     0,     0,     0,     0,   715,
     715,   397,     0,     0,     0,   715,   220,     0,     0,     0,
     220,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,     0,     0,   209,
       0,   209,  1554,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   210,     0,     0,     0,     0,     0,     0,   210,
     210,     0,   209,   719,     0,     0,   210,     0,     0,   344,
     345,   346,   210,     0,     0,     0,     0,   719,   719,   719,
     719,   719,   545,     0,     0,     0,     0,   719,   347,     0,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   961,   369,     0,     0,     0,     0,     0,     0,   715,
       0,   255,   255,     0,   370,     0,     0,   255,   209,     0,
       0,     0,     0,     0,     0,     0,     0,   979,     0,   209,
     209,     0,     0,     0,     0,     0,   222,   220,     0,     0,
       0,   715,     0,   979,     0,     0,     0,     0,     0,     0,
       0,   209,   715,     0,     0,     0,     0,   715,   411,   383,
     384,   385,   386,   387,   388,   389,   390,   391,   392,   393,
     394,     0,     0,     0,     0,   210,     0,     0,     0,     0,
       0,   719,     0,     0,  1016,   210,     0,     0,     0,   255,
       0,     0,   255,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   221,     0,     0,     0,   395,   396,     0,
       0,     0,     0,     0,   715,     0,     0,     0,     0,     0,
       0,     0,   220,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   220,     0,     0,     0,
       0,     0,     0,     0,     0,   220,   209,   209,     0,     0,
       0,   971,     0,     0,     0,     0,     0,     0,     0,     0,
     220,     0,     0,     0,     0,     0,     0,     0,     0,   715,
       0,   397,     0,     0,     0,     0,     0,   715,     0,   719,
       0,   715,   209,     0,   715,   719,   719,   719,   719,   719,
     719,   719,   719,   719,   719,   719,   719,   719,   719,   719,
     719,   719,   719,   719,   719,   719,   719,   719,   719,   719,
     719,   719,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   255,   698,     0,     0,   717,   344,   345,   346,
       0,     0,     0,     0,     0,   719,     0,     0,     0,     0,
       0,     0,     0,   210,     0,     0,   347,     0,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,     0,
     369,   717,     0,   209,     0,     0,     0,     0,     0,     0,
       0,     0,   370,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   210,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     209,     0,     0,     0,     0,   209,   255,   255,     0,     0,
       0,     0,   255,     0,     0,     0,     0,     0,     0,   209,
     209,     0,   719,   210,    36,   210,   205,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   719,     0,   719,
       0,   344,   345,   346,     0,     0,   210,     0,     0,     0,
       0,     0,     0,     0,     0,   719,     0,     0,     0,     0,
     347,     0,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,     0,   369,     0,     0,     0,     0,     0,
       0,     0,  1240,     0,    36,   209,   370,     0,     0,     0,
       0,    83,    84,     0,    85,    86,    87,     0,     0,  1023,
       0,     0,   210,     0,     0,     0,     0,   719,     0,     0,
       0,     0,     0,   210,   210,     0,     0,     0,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,   717,   545,     0,     0,   647,     0,
     115,     0,     0,     0,     0,     0,   255,   255,   717,   717,
     717,   717,   717,     0,     0,     0,     0,     0,   717,  1476,
       0,    83,    84,  1477,    85,    86,    87,     0,     0,   719,
     209,     0,     0,   719,     0,   719,     0,     0,   719,     0,
       0,     0,     0,     0,     0,     0,  1319,   222,  1328,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,     0,  1333,     0,     0,     0,   719,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1031,     0,     0,     0,     0,     0,     0,
     210,   210,     0,   209,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   255,     0,     0,     0,     0,   719,     0,
     344,   345,   346,     0,   255,     0,     0,     0,  1383,  1384,
       0,     0,   717,     0,     0,     0,   545,     0,     0,   347,
       0,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   719,   369,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   370,   932,   933,   934,   935,
     936,   937,   938,   939,   940,   941,   942,   943,   944,   945,
     946,   947,   719,   719,     0,     0,     0,     0,   719,  1516,
       0,     0,     0,  1328,     0,   948,     0,     0,     0,     0,
       0,   255,     0,   255,     0,   698,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   210,     0,     0,
     717,     0,     0,     0,     0,     0,   717,   717,   717,   717,
     717,   717,   717,   717,   717,   717,   717,   717,   717,   717,
     717,   717,   717,   717,   717,   717,   717,   717,   717,   717,
     717,   717,   717,     0,   545,     0,     0,     0,     0,   210,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   210,   210,     0,   717,     0,     0,   255,
       0,     0,     0,     0,     0,   247,     0,     0,     0,     0,
       0,     0,   719,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1054,     0,   255,     0,   255,     0,     0,     0,
       0,   248,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   719,     0,     0,     0,     0,   344,
     345,   346,     0,    36,     0,   719,     0,     0,     0,     0,
     719,     0,     0,     0,     0,     0,     0,     0,   347,   210,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,     0,   369,   717,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   370,   255,   249,   250,   717,     0,
     717,     0,     0,     0,     0,     0,     0,   719,     0,     0,
       0,     0,     0,     0,   177,  1614,   717,    81,   251,     0,
      83,    84,     0,    85,    86,    87,     0,   904,     0,  1319,
       0,     0,     0,     0,     0,     0,     0,     0,   252,     0,
       0,     0,     0,     0,   545,     0,     0,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,   719,     0,   253,   344,   345,   346,     0,     0,
     719,     0,     0,     0,   719,     0,     0,   719,   717,     0,
       0,     0,     0,     0,   347,     0,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   545,   369,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     370,     0,     0,     0,     0,     0,     0,    36,     0,   205,
       0,  1268,     0,   255,     0,   255,     0,     0,     0,     0,
     717,     0,     0,     0,   717,     0,   717,     0,     0,   717,
       0,     0,     0,   255,     0,     0,   255,     0,     0,     0,
     344,   345,   346,     0,     0,     0,   255,     0,     0,     0,
       0,     0,     0,   620,     0,     0,     0,     0,     0,   347,
     717,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,     0,   369,    83,    84,     0,    85,    86,    87,
       0,     0,     0,     0,     0,   370,     0,     0,     0,   717,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,     0,     0,     0,
       0,   621,     0,   115,   255,   181,   183,  1346,   185,   186,
     187,   189,   190,   717,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,     0,     0,   214,   217,
       0,     0,     0,     0,     0,     0,     0,   344,   345,   346,
       0,   235,     0,   717,   717,     0,     0,     0,   243,   717,
     246,     0,     0,   262,     0,   267,   347,  1214,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,     0,
     369,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   370,     0,     0,   344,   345,   346,     0,     0,
       0,     0,  1347,     0,     0,     0,     0,     0,     0,    36,
       0,   205,   312,     0,   347,     0,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,     0,   369,   344,
     345,   346,     0,     0,     0,     0,     0,     0,     0,     0,
     370,     0,     0,   717,     0,     0,     0,     0,   347,     0,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,  1556,   369,     0,   412,   717,    83,    84,     0,    85,
      86,    87,     0,     0,   370,     0,   717,     0,     0,     0,
       0,   717,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,  1215,   438,     0,
       0,   438,     0,   621,     0,   115,     0,     0,   235,   449,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   717,   369,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   370,   720,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   312,   371,     0,     0,     0,   255,
     214,     0,     0,     0,   528,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   255,     0,   548,   548,     0,     0,
       0,     0,     0,   717,     0,     0,   568,   720,     0,     0,
       0,   717,     0,     0,     0,   717,     0,   580,   717,   453,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   586,   587,   588,   590,   591,   592,
     593,   594,   595,   596,   597,   598,   599,   600,   601,   602,
     603,   604,   605,   606,   607,   608,   609,   610,   611,     0,
     613,     0,   614,   614,     0,   617,     0,     0,     0,     0,
       0,     0,   633,   635,   636,   637,   638,   639,   640,   641,
     642,   643,   644,   645,   646,     0,     0,     0,     0,     0,
     614,   651,     0,   580,   614,   654,     0,     0,     0,     0,
       0,   633,     0,     0,   658,     0,     0,     0,     0,     0,
       0,     0,     0,   667,     0,   669,     0,     0,     0,     0,
       0,   580,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   681,     0,   682,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   344,   345,   346,     0,
       0,   716,     0,     0,     0,     0,     0,     0,     0,   725,
       0,     0,   728,   731,   732,   347,     0,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,     0,   369,
     720,     0,     0,   751,     0,     0,   716,   344,   345,   346,
       0,   370,     0,     0,   720,   720,   720,   720,   720,     0,
       0,     0,     0,     0,   720,     0,   347,     0,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,     0,
     369,     0,     0,    36,     0,   205,     0,     0,     0,     0,
       0,     0,   370,     0,     0,     0,     0,     0,     0,   344,
     345,   346,     0,     0,     0,     0,     0,   814,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   347,   825,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,     0,   369,     0,     0,     0,     0,     0,     0,     0,
       0,   833,     0,     0,   370,     0,     0,     0,   720,   189,
      83,    84,     0,    85,    86,    87,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   455,   842,     0,     0,
       0,     0,     0,     0,     0,     0,   721,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,     0,     0,   874,     0,     0,   674,     0,   115,
       0,     0,     0,     0,     0,     0,     0,   235,     0,   716,
       0,     0,     0,     0,     0,     0,    36,   466,   205,     0,
       0,   753,     0,   716,   716,   716,   716,   716,     0,     0,
       0,     0,     0,   716,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   949,     0,     0,   720,     0,     0,     0,
       0,     0,   720,   720,   720,   720,   720,   720,   720,   720,
     720,   720,   720,   720,   720,   720,   720,   720,   720,   720,
     720,   720,   720,   720,   720,   720,   720,   720,   720,   490,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    83,    84,   986,    85,    86,    87,     0,
       0,     0,   720,     0,     0,     0,     0,   993,     0,     0,
       0,     0,   996,     0,   997,     0,     0,     0,     0,     0,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,  1012,   716,     0,     0,
    1006,     0,   115,     0,     0,     0,  1020,   924,   925,  1021,
       0,  1022,     0,     0,     0,   580,     0,     0,     0,     0,
       0,     0,     0,     0,   580,   926,     0,   927,   928,   929,
     930,   931,   932,   933,   934,   935,   936,   937,   938,   939,
     940,   941,   942,   943,   944,   945,   946,   947,     0,  1053,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   720,
       0,   948,     0,     0,   888,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   720,     0,   720,     0,   909,   910,
     911,   912,     0,     0,     0,     0,     0,     0,   920,     0,
       0,     0,   720,     0,     0,   716,     0,     0,     0,     0,
       0,   716,   716,   716,   716,   716,   716,   716,   716,   716,
     716,   716,   716,   716,   716,   716,   716,   716,   716,   716,
     716,   716,   716,   716,   716,   716,   716,   716,     0,     0,
    1126,  1127,  1128,     0,     0,     0,   728,  1130,     0,     0,
       0,     0,     0,     0,   344,   345,   346,     0,     0,     0,
       0,   716,     0,     0,   720,     0,     0,     0,     0,     0,
       0,     0,     0,   347,  1145,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,  1166,   369,     0,     0,
       0,  1170,  1013,     0,     0,     0,     0,     0,     0,   370,
       0,     0,     0,     0,   580,     0,     0,     0,     0,     0,
       0,     0,   580,     0,  1145,     0,   720,     0,     0,     0,
     720,     0,   720,     0,     0,   720,  -854,  -854,  -854,  -854,
     936,   937,   938,   939,   940,   941,   942,   943,   944,   945,
     946,   947,     0,   235,     0,     0,     0,     0,   716,     0,
       0,     0,     0,  1213,     0,   948,   720,     0,     0,     0,
       0,     0,     0,   716,     0,   716,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   716,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   720,  1096,  1099,  1100,  1101,
    1103,  1104,  1105,  1106,  1107,  1108,  1109,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,
    1123,  1124,  1125,     0,     0,     0,     0,     0,     0,     0,
       0,  1259,     0,   657,     0,  1260,     0,  1261,  1262,   720,
       0,     0,     0,   716,     0,     0,  1136,     0,   580,     0,
       0,     0,     0,     0,     0,     0,  1276,   580,     0,     0,
       0,     0,     0,     0,     0,   580,     0,     0,     0,   720,
     720,     0,     0,     0,     0,   720,     0,     0,     0,  1519,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   344,   345,   346,     0,
       0,     0,     0,     0,     0,   716,     0,     0,     0,   716,
       0,   716,     0,     0,   716,   347,  1311,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,     0,   369,
       0,     0,     0,  1209,     0,   716,     0,     0,     0,   580,
       0,   370,     0,     0,     0,     0,     0,     0,  1223,     0,
    1224,     0,    36,     0,   923,   924,   925,     0,     0,     0,
     697,     0,     0,     0,     0,     0,  1234,     0,     0,   720,
       0,     0,     0,   926,   716,   927,   928,   929,   930,   931,
     932,   933,   934,   935,   936,   937,   938,   939,   940,   941,
     942,   943,   944,   945,   946,   947,     0,     0,     0,     0,
       0,   720,    36,     0,   205,     0,     0,     0,     0,   948,
     540,     0,   720,     0,     0,     0,     0,   720,   716,     0,
       0,     0,   580,   177,     0,     0,    81,     0,  1267,    83,
      84,     0,    85,    86,    87,     0,     0,     0,     0,     0,
    1592,     0,   206,     0,     0,     0,     0,     0,   716,   716,
       0,  1511,     0,     0,   716,     0,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,     0,   177,   720,   677,    81,    82,     0,    83,
      84,     0,    85,    86,    87,     0,     0,     0,     0,     0,
    1306,     0,     0,     0,  1308,     0,  1309,     0,     0,  1310,
       0,     0,     0,     0,     0,     0,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     5,     6,     7,     8,     9,     0,     0,   541,   720,
    1351,    10,     0,     0,     0,     0,     0,   720,     0,     0,
       0,   720,     0,  1093,   720,   629,    12,     0,     0,     0,
       0,     0,     0,   630,     0,     0,     0,     0,   716,     0,
       0,     0,     0,     0,    14,    15,     0,     0,     0,  1378,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
     716,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,   716,     0,    40,     0,     0,   716,     0,     0,     0,
       0,     0,     0,  1490,     0,     0,     0,     0,     0,     0,
       0,     0,    51,     0,     0,     0,     0,     0,     0,     0,
      58,    59,    60,   173,   174,   175,     0,     0,    65,    66,
       0,     0,     0,  1509,  1510,     0,     0,   176,    71,  1515,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,   716,     0,     0,   177,    79,    80,    81,
      82,     0,    83,    84,     0,    85,    86,    87,     0,     0,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,    92,    93,     0,     0,     0,     0,    96,    97,   259,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,     0,   112,     0,   716,     0,
       0,   115,   116,     0,   117,   118,   716,     0,     0,     0,
     716,     0,     0,   716,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1539,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1562,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,  1572,     0,     0,     0,
       0,  1576,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,     0,     0,     0,    40,    41,    42,    43,     0,
      44,     0,    45,     0,    46,     0,     0,    47,  1608,     0,
       0,    48,    49,    50,    51,    52,    53,    54,     0,    55,
      56,    57,    58,    59,    60,    61,    62,    63,     0,    64,
      65,    66,    67,    68,    69,     0,     0,     0,     0,    70,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,    78,    79,
      80,    81,    82,  1658,    83,    84,     0,    85,    86,    87,
      88,  1663,     0,    89,     0,  1665,    90,     0,  1666,     0,
       0,     0,    91,    92,    93,    94,     0,    95,     0,    96,
      97,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,   112,     0,
     113,   114,   987,   115,   116,     0,   117,   118,     5,     6,
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
      52,    53,    54,     0,    55,    56,    57,    58,    59,    60,
      61,    62,    63,     0,    64,    65,    66,    67,    68,    69,
       0,     0,     0,     0,    70,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,    78,    79,    80,    81,    82,     0,    83,
      84,     0,    85,    86,    87,    88,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
      94,     0,    95,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,   113,   114,  1147,   115,   116,
       0,   117,   118,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,     0,     0,     0,    40,    41,    42,    43,     0,
      44,     0,    45,     0,    46,     0,     0,    47,     0,     0,
       0,    48,    49,    50,    51,    52,    53,    54,     0,    55,
      56,    57,    58,    59,    60,    61,    62,    63,     0,    64,
      65,    66,    67,    68,    69,     0,     0,     0,     0,    70,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,    78,    79,
      80,    81,    82,     0,    83,    84,     0,    85,    86,    87,
      88,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,    94,     0,    95,     0,    96,
      97,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,   112,     0,
     113,   114,     0,   115,   116,     0,   117,   118,     5,     6,
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
       0,     0,     0,   177,    79,    80,    81,    82,     0,    83,
      84,     0,    85,    86,    87,    88,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
       0,     0,     0,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,   113,   114,   562,   115,   116,
       0,   117,   118,     5,     6,     7,     8,     9,     0,     0,
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
       0,     0,     0,    77,     0,     0,     0,     0,   177,    79,
      80,    81,    82,     0,    83,    84,     0,    85,    86,    87,
      88,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,     0,     0,     0,     0,    96,
      97,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,   112,     0,
     113,   114,   960,   115,   116,     0,   117,   118,     5,     6,
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
       0,     0,     0,   177,    79,    80,    81,    82,     0,    83,
      84,     0,    85,    86,    87,    88,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
       0,     0,     0,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,   113,   114,  1060,   115,   116,
       0,   117,   118,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,     0,     0,     0,    40,    41,    42,    43,  1062,
      44,     0,    45,     0,    46,     0,     0,    47,     0,     0,
       0,    48,    49,    50,    51,     0,    53,    54,     0,    55,
       0,    57,    58,    59,    60,    61,    62,    63,     0,    64,
      65,    66,     0,    68,    69,     0,     0,     0,     0,    70,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   177,    79,
      80,    81,    82,     0,    83,    84,     0,    85,    86,    87,
      88,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,     0,     0,     0,     0,    96,
      97,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,   112,     0,
     113,   114,     0,   115,   116,     0,   117,   118,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,     0,     0,     0,
      40,    41,    42,    43,     0,    44,     0,    45,     0,    46,
    1210,     0,    47,     0,     0,     0,    48,    49,    50,    51,
       0,    53,    54,     0,    55,     0,    57,    58,    59,    60,
      61,    62,    63,     0,    64,    65,    66,     0,    68,    69,
       0,     0,     0,     0,    70,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   177,    79,    80,    81,    82,     0,    83,
      84,     0,    85,    86,    87,    88,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
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
       0,    39,     0,     0,     0,    40,    41,    42,    43,     0,
      44,     0,    45,     0,    46,     0,     0,    47,     0,     0,
       0,    48,    49,    50,    51,     0,    53,    54,     0,    55,
       0,    57,    58,    59,    60,    61,    62,    63,     0,    64,
      65,    66,     0,    68,    69,     0,     0,     0,     0,    70,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   177,    79,
      80,    81,    82,     0,    83,    84,     0,    85,    86,    87,
      88,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,     0,     0,     0,     0,    96,
      97,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,   112,     0,
     113,   114,  1313,   115,   116,     0,   117,   118,     5,     6,
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
       0,     0,     0,   177,    79,    80,    81,    82,     0,    83,
      84,     0,    85,    86,    87,    88,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
       0,     0,     0,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,   113,   114,  1512,   115,   116,
       0,   117,   118,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,     0,     0,     0,    40,    41,    42,    43,     0,
      44,     0,    45,  1551,    46,     0,     0,    47,     0,     0,
       0,    48,    49,    50,    51,     0,    53,    54,     0,    55,
       0,    57,    58,    59,    60,    61,    62,    63,     0,    64,
      65,    66,     0,    68,    69,     0,     0,     0,     0,    70,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   177,    79,
      80,    81,    82,     0,    83,    84,     0,    85,    86,    87,
      88,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,     0,     0,     0,     0,    96,
      97,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,   112,     0,
     113,   114,     0,   115,   116,     0,   117,   118,     5,     6,
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
       0,     0,     0,   177,    79,    80,    81,    82,     0,    83,
      84,     0,    85,    86,    87,    88,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
       0,     0,     0,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,   113,   114,  1581,   115,   116,
       0,   117,   118,     5,     6,     7,     8,     9,     0,     0,
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
       0,     0,     0,    77,     0,     0,     0,     0,   177,    79,
      80,    81,    82,     0,    83,    84,     0,    85,    86,    87,
      88,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,     0,     0,     0,     0,    96,
      97,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,   112,     0,
     113,   114,  1582,   115,   116,     0,   117,   118,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,     0,     0,     0,
      40,    41,    42,    43,     0,    44,  1585,    45,     0,    46,
       0,     0,    47,     0,     0,     0,    48,    49,    50,    51,
       0,    53,    54,     0,    55,     0,    57,    58,    59,    60,
      61,    62,    63,     0,    64,    65,    66,     0,    68,    69,
       0,     0,     0,     0,    70,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   177,    79,    80,    81,    82,     0,    83,
      84,     0,    85,    86,    87,    88,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
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
       0,    39,     0,     0,     0,    40,    41,    42,    43,     0,
      44,     0,    45,     0,    46,     0,     0,    47,     0,     0,
       0,    48,    49,    50,    51,     0,    53,    54,     0,    55,
       0,    57,    58,    59,    60,    61,    62,    63,     0,    64,
      65,    66,     0,    68,    69,     0,     0,     0,     0,    70,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   177,    79,
      80,    81,    82,     0,    83,    84,     0,    85,    86,    87,
      88,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,     0,     0,     0,     0,    96,
      97,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,   112,     0,
     113,   114,  1600,   115,   116,     0,   117,   118,     5,     6,
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
       0,     0,     0,   177,    79,    80,    81,    82,     0,    83,
      84,     0,    85,    86,    87,    88,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
       0,     0,     0,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,   113,   114,  1653,   115,   116,
       0,   117,   118,     5,     6,     7,     8,     9,     0,     0,
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
       0,     0,     0,    77,     0,     0,     0,     0,   177,    79,
      80,    81,    82,     0,    83,    84,     0,    85,    86,    87,
      88,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,     0,     0,     0,     0,    96,
      97,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,   112,     0,
     113,   114,  1659,   115,   116,     0,   117,   118,     5,     6,
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
       0,     0,     0,   177,    79,    80,    81,    82,     0,    83,
      84,     0,    85,    86,    87,    88,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
       0,     0,     0,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,   113,   114,     0,   115,   116,
       0,   117,   118,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
     439,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,     0,     0,     0,    40,    41,    42,    43,     0,
      44,     0,    45,     0,    46,     0,     0,    47,     0,     0,
       0,    48,    49,    50,    51,     0,    53,    54,     0,    55,
       0,    57,    58,    59,    60,   173,   174,    63,     0,    64,
      65,    66,     0,     0,     0,     0,     0,     0,     0,    70,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   177,    79,
      80,    81,    82,     0,    83,    84,     0,    85,    86,    87,
       0,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,     0,     0,     0,     0,    96,
      97,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,   112,     0,
     113,   114,     0,   115,   116,     0,   117,   118,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,   684,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,     0,     0,     0,
      40,    41,    42,    43,     0,    44,     0,    45,     0,    46,
       0,     0,    47,     0,     0,     0,    48,    49,    50,    51,
       0,    53,    54,     0,    55,     0,    57,    58,    59,    60,
     173,   174,    63,     0,    64,    65,    66,     0,     0,     0,
       0,     0,     0,     0,    70,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   177,    79,    80,    81,    82,     0,    83,
      84,     0,    85,    86,    87,     0,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
       0,     0,     0,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,   113,   114,     0,   115,   116,
       0,   117,   118,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
     890,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,     0,     0,     0,    40,    41,    42,    43,     0,
      44,     0,    45,     0,    46,     0,     0,    47,     0,     0,
       0,    48,    49,    50,    51,     0,    53,    54,     0,    55,
       0,    57,    58,    59,    60,   173,   174,    63,     0,    64,
      65,    66,     0,     0,     0,     0,     0,     0,     0,    70,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   177,    79,
      80,    81,    82,     0,    83,    84,     0,    85,    86,    87,
       0,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,     0,     0,     0,     0,    96,
      97,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,   112,     0,
     113,   114,     0,   115,   116,     0,   117,   118,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,  1373,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,     0,     0,     0,
      40,    41,    42,    43,     0,    44,     0,    45,     0,    46,
       0,     0,    47,     0,     0,     0,    48,    49,    50,    51,
       0,    53,    54,     0,    55,     0,    57,    58,    59,    60,
     173,   174,    63,     0,    64,    65,    66,     0,     0,     0,
       0,     0,     0,     0,    70,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   177,    79,    80,    81,    82,     0,    83,
      84,     0,    85,    86,    87,     0,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
       0,     0,     0,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,   113,   114,     0,   115,   116,
       0,   117,   118,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
    1504,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
      32,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,    39,     0,     0,     0,    40,    41,    42,    43,     0,
      44,     0,    45,     0,    46,     0,     0,    47,     0,     0,
       0,    48,    49,    50,    51,     0,    53,    54,     0,    55,
       0,    57,    58,    59,    60,   173,   174,    63,     0,    64,
      65,    66,     0,     0,     0,     0,     0,     0,     0,    70,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   177,    79,
      80,    81,    82,     0,    83,    84,     0,    85,    86,    87,
       0,     0,     0,    89,     0,     0,    90,     0,     0,     0,
       0,     0,    91,    92,    93,     0,     0,     0,     0,    96,
      97,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,   112,     0,
     113,   114,     0,   115,   116,     0,   117,   118,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,    15,     0,     0,     0,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,     0,     0,     0,    33,
      34,    35,    36,    37,    38,     0,    39,     0,     0,     0,
      40,    41,    42,    43,     0,    44,     0,    45,     0,    46,
       0,     0,    47,     0,     0,     0,    48,    49,    50,    51,
       0,    53,    54,     0,    55,     0,    57,    58,    59,    60,
     173,   174,    63,     0,    64,    65,    66,     0,     0,     0,
       0,     0,     0,     0,    70,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   177,    79,    80,    81,    82,     0,    83,
      84,     0,    85,    86,    87,     0,     0,     0,    89,     0,
       0,    90,     0,     0,     0,     0,     0,    91,    92,    93,
       0,     0,     0,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,   113,   114,     0,   115,   116,
       0,   117,   118,     5,     6,     7,     8,     9,     0,     0,
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
       0,     0,    58,    59,    60,   173,   174,   175,     0,     0,
      65,    66,     0,     0,     0,     0,     0,     0,     0,   176,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   177,    79,
      80,    81,    82,     0,    83,    84,     0,    85,    86,    87,
       0,     0,     0,    89,     0,     0,    90,     5,     6,     7,
       8,     9,    91,    92,    93,     0,     0,    10,     0,    96,
      97,   259,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,   112,   576,
     260,     0,     0,   115,   116,     0,   117,   118,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,    40,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    51,     0,
       0,     0,     0,     0,     0,     0,    58,    59,    60,   173,
     174,   175,     0,     0,    65,    66,     0,     0,     0,     0,
       0,     0,     0,   176,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   177,    79,    80,    81,    82,     0,    83,    84,
       0,    85,    86,    87,     0,   577,     0,    89,     0,     0,
      90,     5,     6,     7,     8,     9,    91,    92,    93,     0,
       0,    10,     0,    96,    97,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,     0,   112,  1010,     0,     0,     0,   115,   116,     0,
     117,   118,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,    40,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    51,     0,     0,     0,     0,     0,     0,     0,
      58,    59,    60,   173,   174,   175,     0,     0,    65,    66,
       0,     0,     0,     0,     0,     0,     0,   176,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   177,    79,    80,    81,
      82,     0,    83,    84,     0,    85,    86,    87,     0,  1011,
       0,    89,     0,     0,    90,     0,     0,     0,     0,     0,
      91,    92,    93,     0,     0,     0,     0,    96,    97,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,     0,   112,     0,     0,     0,
       0,   115,   116,     0,   117,   118,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     629,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,    40,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    51,     0,     0,
       0,     0,     0,     0,     0,    58,    59,    60,   173,   174,
     175,     0,     0,    65,    66,     0,     0,     0,     0,     0,
       0,     0,   176,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   177,    79,    80,    81,    82,     0,    83,    84,     0,
      85,    86,    87,     0,     0,     0,    89,     0,     0,    90,
       5,     6,     7,     8,     9,    91,    92,    93,     0,     0,
      10,     0,    96,    97,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,   112,     0,     0,     0,     0,   115,   116,     0,   117,
     118,     0,     0,    14,    15,     0,     0,     0,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,     0,     0,
       0,    33,    34,    35,    36,    37,    38,     0,     0,     0,
       0,     0,    40,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   188,     0,
       0,    51,     0,     0,     0,     0,     0,     0,     0,    58,
      59,    60,   173,   174,   175,     0,     0,    65,    66,     0,
       0,     0,     0,     0,     0,     0,   176,    71,     0,    72,
      73,    74,    75,    76,     0,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,   177,    79,    80,    81,    82,
       0,    83,    84,     0,    85,    86,    87,     0,     0,     0,
      89,     0,     0,    90,     5,     6,     7,     8,     9,    91,
      92,    93,     0,     0,    10,     0,    96,    97,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,     0,     0,   112,   213,     0,     0,     0,
     115,   116,     0,   117,   118,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,     0,     0,     0,     0,    40,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    51,     0,     0,     0,     0,
       0,     0,     0,    58,    59,    60,   173,   174,   175,     0,
       0,    65,    66,     0,     0,     0,     0,     0,     0,     0,
     176,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   177,
      79,    80,    81,    82,     0,    83,    84,     0,    85,    86,
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
     173,   174,   175,     0,     0,    65,    66,     0,     0,     0,
       0,     0,     0,     0,   176,    71,     0,    72,    73,    74,
      75,    76,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,   177,    79,    80,    81,    82,     0,    83,
      84,     0,    85,    86,    87,     0,     0,     0,    89,     0,
       0,    90,     5,     6,     7,     8,     9,    91,    92,    93,
       0,     0,    10,     0,    96,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,     0,     0,   112,     0,   242,     0,     0,   115,   116,
       0,   117,   118,     0,     0,    14,    15,     0,     0,     0,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
       0,     0,     0,    33,    34,    35,    36,    37,    38,     0,
       0,     0,     0,     0,    40,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    51,     0,     0,     0,     0,     0,     0,
       0,    58,    59,    60,   173,   174,   175,     0,     0,    65,
      66,     0,     0,     0,     0,     0,     0,     0,   176,    71,
       0,    72,    73,    74,    75,    76,     0,     0,     0,     0,
       0,     0,    77,     0,     0,     0,     0,   177,    79,    80,
      81,    82,     0,    83,    84,     0,    85,    86,    87,     0,
       0,     0,    89,     0,     0,    90,     5,     6,     7,     8,
       9,    91,    92,    93,     0,     0,    10,     0,    96,    97,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,   112,     0,   245,
       0,     0,   115,   116,     0,   117,   118,     0,     0,    14,
      15,     0,     0,     0,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,     0,     0,     0,    33,    34,    35,
      36,    37,    38,     0,     0,     0,     0,     0,    40,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    51,     0,     0,
       0,     0,     0,     0,     0,    58,    59,    60,   173,   174,
     175,     0,     0,    65,    66,     0,     0,     0,     0,     0,
       0,     0,   176,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   177,    79,    80,    81,    82,     0,    83,    84,     0,
      85,    86,    87,     0,     0,     0,    89,     0,     0,    90,
       0,     0,     0,     0,     0,    91,    92,    93,     0,     0,
       0,     0,    96,    97,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,   112,   437,     0,     0,     0,   115,   116,     0,   117,
     118,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   589,   369,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   370,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,    40,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    51,     0,     0,     0,     0,     0,     0,     0,
      58,    59,    60,   173,   174,   175,     0,     0,    65,    66,
       0,     0,     0,     0,     0,     0,     0,   176,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   177,    79,    80,    81,
      82,     0,    83,    84,     0,    85,    86,    87,     0,     0,
       0,    89,     0,     0,    90,     5,     6,     7,     8,     9,
      91,    92,    93,     0,     0,    10,     0,    96,    97,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,     0,     0,   112,   630,     0,     0,
       0,   115,   116,     0,   117,   118,     0,     0,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,     0,     0,     0,     0,    40,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    51,     0,     0,     0,
       0,     0,     0,     0,    58,    59,    60,   173,   174,   175,
       0,     0,    65,    66,     0,     0,     0,     0,     0,     0,
       0,   176,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     177,    79,    80,    81,    82,     0,    83,    84,     0,    85,
      86,    87,     0,     0,     0,    89,     0,     0,    90,     5,
       6,     7,     8,     9,    91,    92,    93,     0,     0,    10,
       0,    96,    97,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
     112,   666,     0,     0,     0,   115,   116,     0,   117,   118,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,     0,     0,     0,
       0,    40,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      51,     0,     0,     0,     0,     0,     0,     0,    58,    59,
      60,   173,   174,   175,     0,     0,    65,    66,     0,     0,
       0,     0,     0,     0,     0,   176,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   177,    79,    80,    81,    82,     0,
      83,    84,     0,    85,    86,    87,     0,     0,     0,    89,
       0,     0,    90,     5,     6,     7,     8,     9,    91,    92,
      93,     0,     0,    10,     0,    96,    97,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   112,   668,     0,     0,     0,   115,
     116,     0,   117,   118,     0,     0,    14,    15,     0,     0,
       0,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     0,    28,    29,    30,    31,
       0,     0,     0,     0,    33,    34,    35,    36,    37,    38,
       0,     0,     0,     0,     0,    40,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    51,     0,     0,     0,     0,     0,
       0,     0,    58,    59,    60,   173,   174,   175,     0,     0,
      65,    66,     0,     0,     0,     0,     0,     0,     0,   176,
      71,     0,    72,    73,    74,    75,    76,     0,     0,     0,
       0,     0,     0,    77,     0,     0,     0,     0,   177,    79,
      80,    81,    82,     0,    83,    84,     0,    85,    86,    87,
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
       0,     0,     0,     0,     0,     0,     0,     0,    51,     0,
       0,     0,     0,     0,     0,     0,    58,    59,    60,   173,
     174,   175,     0,     0,    65,    66,     0,     0,     0,     0,
       0,     0,     0,   176,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   177,    79,    80,    81,    82,     0,    83,    84,
       0,    85,    86,    87,     0,     0,     0,    89,     0,     0,
      90,     5,     6,     7,     8,     9,    91,    92,    93,     0,
       0,    10,     0,    96,    97,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,     0,   112,  1052,     0,   680,     0,   115,   116,     0,
     117,   118,     0,     0,    14,    15,     0,     0,     0,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,    28,    29,    30,    31,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,     0,     0,
       0,     0,     0,    40,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    51,     0,     0,     0,     0,     0,     0,     0,
      58,    59,    60,   173,   174,   175,     0,     0,    65,    66,
       0,     0,     0,     0,     0,     0,     0,   176,    71,     0,
      72,    73,    74,    75,    76,     0,     0,     0,     0,     0,
       0,    77,     0,     0,     0,     0,   177,    79,    80,    81,
      82,     0,    83,    84,     0,    85,    86,    87,     0,     0,
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
       0,     0,     0,     0,    58,    59,    60,   173,   174,   175,
       0,     0,    65,    66,     0,     0,     0,     0,     0,     0,
       0,   176,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     177,    79,    80,    81,    82,     0,    83,    84,     0,    85,
      86,    87,     0,     0,     0,    89,     0,     0,    90,     5,
       6,     7,     8,     9,    91,    92,    93,     0,     0,    10,
       0,    96,    97,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
     112,     0,     0,     0,     0,   115,   116,     0,   117,   118,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,     0,     0,     0,     0,
      33,    34,    35,    36,   527,    38,     0,     0,     0,     0,
       0,    40,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      51,     0,     0,     0,     0,     0,     0,     0,    58,    59,
      60,   173,   174,   175,     0,     0,    65,    66,     0,     0,
       0,     0,     0,     0,     0,   176,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   177,    79,    80,    81,    82,     0,
      83,    84,     0,    85,    86,    87,     0,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,     0,     0,     0,     0,    96,    97,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   112,     0,     0,     0,     0,   115,
     116,     0,   117,   118,  1392,  1393,  1394,  1395,  1396,     0,
       0,  1397,  1398,  1399,  1400,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1401,  1402,
       0,   347,     0,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,  1403,   369,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   370,  1404,  1405,
    1406,  1407,  1408,  1409,  1410,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,  1411,  1412,  1413,
    1414,  1415,  1416,  1417,  1418,  1419,  1420,  1421,  1422,  1423,
    1424,  1425,  1426,  1427,  1428,  1429,  1430,  1431,  1432,  1433,
    1434,  1435,  1436,  1437,  1438,  1439,  1440,  1441,  1442,  1443,
    1444,  1445,  1446,  1447,  1448,  1449,  1450,  1451,     0,     0,
    1452,  1453,     0,  1454,  1455,  1456,  1457,  1458,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1459,
    1460,  1461,     0,     0,     0,    83,    84,     0,    85,    86,
      87,  1462,     0,  1463,  1464,     0,  1465,     0,     0,     0,
       0,     0,     0,  1466,     0,     0,  1467,     0,  1468,     0,
    1469,  1470,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   344,   345,   346,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   347,     0,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,     0,   369,
     344,   345,   346,     0,     0,     0,     0,     0,     0,     0,
       0,   370,     0,     0,     0,     0,     0,     0,     0,   347,
       0,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,     0,   369,   344,   345,   346,     0,     0,     0,
       0,     0,     0,     0,     0,   370,     0,     0,     0,     0,
       0,     0,     0,   347,     0,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,     0,   369,   344,   345,
     346,     0,     0,     0,     0,     0,     0,     0,     0,   370,
       0,     0,     0,     0,     0,     0,     0,   347,     0,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
       0,   369,   344,   345,   346,     0,     0,     0,     0,     0,
     957,     0,     0,   370,     0,     0,     0,     0,     0,     0,
       0,   347,     0,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   247,   369,     0,     0,     0,     0,
     953,   954,     0,     0,     0,     0,     0,   370,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     248,   928,   929,   930,   931,   932,   933,   934,   935,   936,
     937,   938,   939,   940,   941,   942,   943,   944,   945,   946,
     947,  1552,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   948,     0,     0,     0,     0,   247,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  -310,
       0,     0,     0,     0,     0,     0,     0,    58,    59,    60,
     173,   174,   335,     0,  1381,   248,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   249,   250,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    36,     0,     0,
       0,     0,     0,   177,     0,     0,    81,   251,     0,    83,
      84,     0,    85,    86,    87,     0,     0,  1236,     0,     0,
       0,     0,     0,     0,   460,     0,     0,   252,   247,     0,
       0,     0,     0,     0,     0,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     249,   250,     0,   253,   248,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   177,     0,
       0,    81,   251,     0,    83,    84,    36,    85,    86,    87,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   252,   247,     0,     0,     0,     0,     0,     0,
       0,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,     0,   253,   248,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   249,
     250,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    36,     0,     0,     0,     0,     0,   177,     0,     0,
      81,   251,     0,    83,    84,     0,    85,    86,    87,     0,
    1220,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   252,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   249,   250,     0,   253,     0,     0,
      36,     0,   205,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   177,     0,     0,    81,   251,     0,    83,    84,
       0,    85,    86,    87,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   252,     0,     0,     0,
     206,     0,     0,  1102,     0,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   704,
     705,     0,   253,     0,     0,   706,     0,   707,     0,     0,
       0,   177,     0,     0,    81,    82,     0,    83,    84,   708,
      85,    86,    87,     0,     0,     0,     0,    33,    34,    35,
      36,     0,     0,     0,     0,     0,   884,     0,   709,     0,
       0,     0,     0,     0,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
       0,   207,     0,     0,   503,     0,   115,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,     0,
     205,     0,     0,   710,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,   711,     0,     0,     0,
       0,   177,    79,    80,    81,   712,     0,    83,    84,     0,
      85,    86,    87,     0,     0,     0,    89,     0,   206,     0,
       0,     0,     0,     0,     0,   713,     0,     0,     0,     0,
     885,     0,    96,    36,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,   177,
       0,   714,    81,    82,     0,    83,    84,     0,    85,    86,
      87,     0,   927,   928,   929,   930,   931,   932,   933,   934,
     935,   936,   937,   938,   939,   940,   941,   942,   943,   944,
     945,   946,   947,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   948,   704,   705,   207,
       0,     0,     0,   706,   115,   707,     0,   298,     0,     0,
      83,    84,     0,    85,    86,    87,     0,   708,     0,     0,
       0,     0,     0,     0,     0,    33,    34,    35,    36,     0,
       0,     0,     0,     0,     0,     0,   709,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,     0,     0,     0,     0,   926,   299,   927,   928,
     929,   930,   931,   932,   933,   934,   935,   936,   937,   938,
     939,   940,   941,   942,   943,   944,   945,   946,   947,     0,
       0,   710,     0,    72,    73,    74,    75,    76,     0,     0,
      36,     0,   948,     0,   711,     0,     0,     0,     0,   177,
      79,    80,    81,   712,     0,    83,    84,     0,    85,    86,
      87,     0,     0,     0,    89,     0,     0,     0,     0,     0,
       0,     0,     0,   713,     0,     0,   851,   852,     0,     0,
      96,     0,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   853,     0,     0,   714,
       0,     0,     0,     0,   854,   855,   856,    36,     0,     0,
       0,     0,     0,     0,   840,   857,     0,    83,    84,     0,
      85,    86,    87,    36,     0,   205,   929,   930,   931,   932,
     933,   934,   935,   936,   937,   938,   939,   940,   941,   942,
     943,   944,   945,   946,   947,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   948,     0,
     858,   582,     0,   206,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   859,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    83,    84,     0,    85,    86,    87,
    1035,  1036,  1037,    36,   177,     0,     0,    81,    82,     0,
      83,    84,   860,    85,    86,    87,    36,     0,   205,     0,
       0,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,     0,     0,   207,     0,   206,     0,     0,   115,
       0,     0,     0,     0,     0,     0,     0,     0,   519,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      83,    84,     0,    85,    86,    87,     0,   177,     0,     0,
      81,    82,     0,    83,    84,     0,    85,    86,    87,    36,
       0,   205,     0,     0,     0,     0,     0,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,     0,   207,     0,   206,
       0,     0,   115,     0,     0,     0,     0,     0,     0,     0,
       0,   982,    36,     0,   205,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     177,     0,     0,    81,    82,     0,    83,    84,     0,    85,
      86,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   206,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,    36,     0,     0,
     207,     0,     0,   177,     0,   115,    81,    82,     0,    83,
      84,     0,    85,    86,    87,    36,     0,   205,     0,   930,
     931,   932,   933,   934,   935,   936,   937,   938,   939,   940,
     941,   942,   943,   944,   945,   946,   947,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     948,     0,     0,   207,     0,   218,     0,    36,   115,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    83,    84,     0,    85,    86,    87,
       0,     0,     0,     0,     0,     0,   177,     0,     0,    81,
      82,     0,    83,    84,     0,    85,    86,    87,     0,     0,
       0,  1332,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,     0,   823,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,    83,    84,   219,    85,    86,    87,
       0,   115,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   344,   345,
     346,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,   347,  1333,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
       0,   369,   344,   345,   346,     0,     0,     0,     0,     0,
       0,     0,     0,   370,     0,     0,     0,     0,     0,     0,
       0,   347,     0,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,     0,   369,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   370,   344,   345,
     346,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   414,   347,     0,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
       0,   369,   344,   345,   346,     0,     0,     0,     0,     0,
       0,     0,     0,   370,     0,     0,     0,     0,     0,     0,
     423,   347,     0,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,     0,   369,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   370,   344,   345,
     346,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   792,   347,     0,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
       0,   369,   344,   345,   346,     0,     0,     0,     0,     0,
       0,     0,     0,   370,     0,     0,     0,     0,     0,     0,
     829,   347,     0,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,     0,   369,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   370,   344,   345,
     346,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   869,   347,     0,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
       0,   369,   923,   924,   925,     0,     0,     0,     0,     0,
       0,     0,     0,   370,     0,     0,     0,     0,     0,     0,
    1158,   926,     0,   927,   928,   929,   930,   931,   932,   933,
     934,   935,   936,   937,   938,   939,   940,   941,   942,   943,
     944,   945,   946,   947,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   948,   923,   924,
     925,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1176,   926,     0,   927,
     928,   929,   930,   931,   932,   933,   934,   935,   936,   937,
     938,   939,   940,   941,   942,   943,   944,   945,   946,   947,
     923,   924,   925,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   948,     0,     0,     0,     0,     0,   926,
    1084,   927,   928,   929,   930,   931,   932,   933,   934,   935,
     936,   937,   938,   939,   940,   941,   942,   943,   944,   945,
     946,   947,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    36,     0,     0,   948,   923,   924,   925,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    36,     0,     0,     0,   926,  1230,   927,   928,   929,
     930,   931,   932,   933,   934,   935,   936,   937,   938,   939,
     940,   941,   942,   943,   944,   945,   946,   947,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   948,  1320,     0,     0,     0,    36,     0,  1305,     0,
       0,     0,     0,   177,  1321,  1322,    81,    82,     0,    83,
      84,     0,    85,    86,    87,    36,     0,   770,   771,     0,
       0,     0,   177,   268,   269,    81,  1323,     0,    83,    84,
       0,    85,  1324,    87,     0,     0,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,     0,     0,  1380,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,    36,
     270,     0,     0,    83,    84,     0,    85,    86,    87,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    83,    84,     0,    85,    86,    87,     0,     0,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,     0,     0,     0,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,    36,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   332,     0,    83,    84,    36,    85,
      86,    87,     0,     0,     0,     0,   491,     0,     0,    83,
      84,     0,    85,    86,    87,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,     0,     0,     0,    36,     0,   495,     0,     0,
      83,    84,     0,    85,    86,    87,     0,     0,     0,     0,
      36,     0,   270,     0,     0,    83,    84,     0,    85,    86,
      87,     0,     0,     0,     0,     0,     0,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   620,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,  1094,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    83,    84,     0,    85,    86,    87,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    83,    84,     0,
      85,    86,    87,     0,     0,     0,     0,     0,     0,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
       0,     0,     0,     0,    83,    84,     0,    85,    86,    87,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   344,   345,   346,
       0,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   688,   347,     0,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,     0,
     369,   344,   345,   346,     0,     0,     0,     0,     0,     0,
       0,     0,   370,     0,     0,     0,     0,     0,     0,     0,
     347,   826,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   689,   369,   344,   345,   346,     0,     0,
       0,     0,     0,     0,     0,     0,   370,     0,     0,     0,
       0,     0,     0,     0,   347,     0,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,     0,   369,   923,
     924,   925,     0,     0,     0,     0,     0,     0,     0,     0,
     370,     0,     0,     0,     0,     0,     0,     0,   926,  1235,
     927,   928,   929,   930,   931,   932,   933,   934,   935,   936,
     937,   938,   939,   940,   941,   942,   943,   944,   945,   946,
     947,   923,   924,   925,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   948,     0,     0,     0,     0,     0,
     926,     0,   927,   928,   929,   930,   931,   932,   933,   934,
     935,   936,   937,   938,   939,   940,   941,   942,   943,   944,
     945,   946,   947,   345,   346,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   948,     0,     0,     0,
       0,   347,     0,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   346,   369,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   370,     0,     0,
       0,   347,     0,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   925,   369,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   370,     0,     0,
       0,   926,     0,   927,   928,   929,   930,   931,   932,   933,
     934,   935,   936,   937,   938,   939,   940,   941,   942,   943,
     944,   945,   946,   947,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   948,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,     0,
     369,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   370,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,     0,   369,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   370,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,     0,   369,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   370,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,     0,   369,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     370,   931,   932,   933,   934,   935,   936,   937,   938,   939,
     940,   941,   942,   943,   944,   945,   946,   947,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   948,  -854,  -854,  -854,  -854,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,     0,   369,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   370
};

static const yytype_int16 yycheck[] =
{
       4,   159,   133,    88,   579,   328,     4,     4,    32,    94,
      95,   179,   439,   169,   305,   659,     4,     4,   216,    43,
      53,   408,   801,    47,     4,  1002,   220,   382,   788,   162,
     369,   687,  1171,   247,   248,     4,   112,   534,   226,   253,
     883,   310,   822,   403,   819,    49,   694,   132,    52,   180,
     112,    45,     4,    30,    30,   226,     9,   837,     9,    30,
     487,     9,    45,    45,    45,    69,     9,     9,   990,     9,
      79,   431,     9,    79,    26,    27,     9,     9,     9,     9,
       9,     9,     9,    45,    88,     9,    35,     9,     9,    79,
      94,    95,    56,     9,   227,     9,    66,   127,     9,     9,
      33,     9,     9,    66,    50,     9,     9,    99,   100,   127,
      66,   325,    50,   127,    78,   153,    84,    81,    35,    79,
     168,   112,   113,   114,   115,   116,   117,    66,   132,    79,
      79,   207,    79,  1272,    84,  1274,   108,    84,  1526,  1527,
      66,   151,     0,   219,   200,   201,    66,   198,    35,   305,
     198,  1499,    71,    72,    49,    50,    51,   198,    53,   919,
     112,   198,    79,   201,   133,    66,    66,    66,   336,   199,
      65,   127,    66,    66,    66,    66,   144,   147,    66,   159,
      66,    66,    66,    66,   202,   199,    35,   147,   192,   180,
     200,   201,    79,   165,   203,   145,   146,   203,   145,   146,
    1548,   127,   196,    71,    72,   199,   152,   127,   284,   200,
     202,   180,   161,   203,   152,   196,   199,   199,   169,   417,
     376,  1360,   284,   200,  1067,   201,     8,   169,   232,   167,
      79,   201,   236,   200,   201,   232,   240,   199,   201,   236,
    1015,   397,   410,  1018,   458,   893,   331,   895,  1028,   202,
     381,  1173,   200,   680,   258,   207,   199,   684,  1180,  1181,
     200,   213,   201,   200,   201,   421,   199,   219,   200,   200,
     200,   200,   200,   200,   430,   201,   200,   433,   200,   200,
     232,   201,   779,   199,   236,    96,   200,  1047,   199,   199,
      79,   199,   199,   378,   379,   199,   199,   198,   198,    96,
     201,   201,   201,    79,   268,   269,   270,   201,   201,   201,
     201,   315,   161,   201,    79,   201,   201,   201,   201,    84,
     324,    35,    35,   275,   328,    84,    66,   331,    99,   100,
     282,   283,   284,   198,   298,   315,    79,   289,   552,   553,
     151,    84,   418,   295,   558,   543,   315,   147,    79,   382,
     198,    96,   550,    84,   151,   144,   145,   146,   198,  1281,
     201,    96,  1501,   315,    96,    79,    79,    96,   168,   145,
     146,   559,   376,   377,   378,   379,    74,    75,    76,   766,
     145,   146,    53,   198,   153,   144,   123,    85,   559,   147,
     123,    79,    29,   397,    65,   198,    84,   166,   369,   198,
    1175,   201,   145,   146,  1048,   376,   151,  1063,   198,    46,
     168,   380,    49,   144,   145,   146,   151,   421,   198,   151,
      79,   619,   151,   125,   126,    84,   397,   152,   622,   433,
      30,   202,   201,   151,   132,   133,   134,   135,   136,   201,
     198,   206,   446,   201,   198,   143,   198,   161,   161,   446,
     421,   149,   150,    29,   648,     4,   408,   145,   146,   430,
     628,    35,   433,   890,   207,   163,   418,  1227,  1522,  1523,
      46,   665,   198,    49,   198,   835,   664,   373,   702,   703,
    1619,   179,   670,   198,   844,   144,   145,   146,   702,   703,
      14,   200,   200,   664,   446,  1634,    45,   788,   502,   670,
     585,   840,   700,   200,   400,   200,    30,  1282,   404,    49,
      50,    51,   201,   517,    98,    99,   100,   200,   200,   523,
     524,   715,    66,    47,   292,    65,   200,   491,   296,    66,
    1290,   495,   801,    98,    99,   100,   500,   670,    46,    47,
      48,    49,    50,    51,  1188,    53,  1190,    66,   147,   317,
     819,   319,   320,   321,   322,   198,   105,    65,   112,   113,
     114,   110,   104,   112,   113,   114,   115,   116,   117,   118,
     112,   113,   114,   115,   116,   117,   198,   116,   117,   118,
      66,   585,   198,   147,   151,   799,   112,   113,   114,   115,
     116,   117,   200,    44,    65,   168,   810,   123,   124,   112,
     113,   114,   115,   116,   117,   154,   155,   127,   157,   147,
     205,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,   576,     9,   147,   198,   919,   984,
     147,   180,   788,   127,   160,   181,   182,   183,   180,     8,
     200,   168,   188,   189,  1288,   198,   192,   193,  1625,   805,
      14,    14,    79,   202,   180,   659,    14,   661,   199,   683,
      63,    64,   168,  1640,  1024,   200,   864,   180,   200,    14,
      96,   199,  1032,   204,   678,   104,   199,     9,   630,  1066,
     198,    88,   198,   897,   198,   899,     9,   691,   692,    46,
      47,    48,    49,    50,    51,   692,   199,   199,   678,   200,
      14,   198,     9,   184,    79,    79,  1133,    79,    65,   678,
     908,   187,   200,   198,   666,     9,   668,   915,   200,     9,
      79,   199,   199,  1367,   127,   200,   678,   125,   198,    66,
     199,   126,   167,   737,   128,     9,   199,   689,   147,   743,
     692,   199,     9,   747,   748,   199,  1015,   196,    14,  1018,
       9,    66,     9,   169,   839,  1142,  1047,   737,   199,     9,
      14,   975,   205,   919,   125,   205,   202,     9,   737,   199,
     198,   205,   205,  1200,   778,   198,   198,    96,   742,   199,
     778,   778,   746,   200,   128,   737,  1000,   791,  1002,  1216,
     778,   778,   200,   147,     9,   199,   199,   198,   778,   147,
     147,   805,   198,   755,   201,   184,   184,    14,  1168,   778,
       9,    79,   201,  1007,   766,   767,  1595,  1177,    14,   200,
      96,   201,    14,   205,   201,  1185,   778,    14,    30,   200,
     198,   198,   198,  1612,   805,   839,    14,     9,   198,   128,
      14,  1620,   198,     9,   199,  1043,   198,   851,   852,   853,
     200,   200,   876,   199,    65,     9,   144,    79,     9,  1503,
     198,   200,   205,   128,    14,  1292,    79,  1081,   198,   840,
      79,  1140,   199,   877,  1301,   879,   201,   881,   198,  1148,
     877,   198,   128,   199,   205,  1312,     9,   198,   201,   201,
     199,  1047,   144,  1091,    30,   200,   900,    73,   199,   879,
    1098,   881,   200,   128,    30,   169,  1175,   199,   199,  1269,
     879,   128,   881,     9,   918,   199,   202,   921,     9,  1242,
     199,    96,   202,   198,    14,   877,   199,   879,   199,   881,
     199,   883,   884,   128,     9,    30,  1227,   198,   201,   199,
     156,   199,   152,   199,    79,   200,  1373,   200,   199,    14,
     199,   984,   200,   198,   201,    79,   199,   110,   199,   199,
     964,   199,   199,   967,   128,   128,    14,   201,  1543,   200,
      79,    14,   976,     4,   201,   199,   199,   198,    14,   128,
     200,   200,    14,   201,   988,   199,    55,    79,     9,   108,
     988,   988,  1352,   198,    79,    79,   200,    96,  1196,  1290,
     988,   988,   147,    96,  1273,  1219,    33,  1221,   988,    14,
    1279,  1280,   159,  1282,    45,   198,   200,  1340,   199,   988,
      79,   198,   165,   162,     9,  1239,   199,     4,    79,   200,
     199,  1055,   199,   201,    79,  1610,   988,    79,  1252,    14,
    1044,    14,    79,    14,  1048,    79,    26,    27,    14,   742,
      30,  1603,   500,   379,  1058,   746,   378,   159,  1010,   377,
     838,  1058,   836,   780,  1064,  1616,  1070,  1494,    45,  1496,
    1372,  1227,    52,  1213,   105,  1612,  1390,  1504,  1474,   110,
     505,   112,   113,   114,   115,   116,   117,   118,  1247,  1644,
    1070,  1632,  1486,    41,  1359,  1364,  1254,   382,   959,  1001,
    1052,  1070,   956,   476,   734,  1030,  1058,   916,   852,   991,
    1044,   476,   702,   866,  1066,  1067,   307,   901,  1070,  1546,
     290,  1252,   283,   154,   155,    -1,   157,    -1,   105,    -1,
      -1,    -1,    -1,   110,  1290,   112,   113,   114,   115,   116,
     117,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   180,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1140,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1148,    -1,
    1338,   202,    -1,    -1,    -1,    -1,    -1,   154,   155,    -1,
     157,    -1,    -1,    -1,  1188,    -1,  1190,    -1,    -1,    -1,
    1142,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,   180,    53,    -1,    -1,    -1,    -1,    -1,
      -1,  1638,    -1,    -1,    -1,    -1,    65,    -1,  1645,    -1,
      -1,    -1,    -1,    -1,    -1,   202,    -1,   207,    -1,    -1,
      -1,    -1,    -1,   213,  1238,    -1,    -1,    -1,  1242,   219,
      -1,    -1,    -1,  1247,    -1,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,  1238,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   247,   248,  1238,
      65,    -1,    -1,   253,  1254,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1251,    -1,  1288,    -1,  1238,    -1,    -1,  1293,
      -1,    -1,    -1,  1273,  1298,   275,    -1,    -1,  1302,  1279,
    1280,  1298,   282,   283,    -1,    -1,    -1,    -1,    -1,   289,
      -1,    -1,    -1,  1293,    -1,   295,    -1,    -1,    -1,    -1,
    1488,    -1,  1302,  1327,  1293,   305,  1595,    -1,    -1,  1333,
      -1,    -1,    -1,  1302,    -1,     4,  1340,    -1,    -1,    -1,
      -1,  1293,    -1,  1612,    -1,   325,  1298,    -1,   328,    -1,
    1302,  1620,    -1,    -1,    -1,    -1,     4,    -1,    -1,    -1,
      -1,    -1,    -1,  1367,    -1,    -1,  1370,  1371,  1372,    -1,
      -1,    -1,  1376,  1342,  1371,  1372,    45,    -1,  1382,    -1,
      -1,    -1,    -1,    -1,  1364,    -1,    -1,    -1,    -1,   369,
    1370,    -1,    -1,    -1,    -1,    -1,  1376,    45,    -1,    -1,
    1485,  1370,  1382,    -1,    -1,    -1,    -1,  1376,    -1,    -1,
      -1,  1625,    -1,  1382,    -1,    -1,    -1,    -1,  1370,  1371,
    1372,    -1,    -1,    -1,  1376,    -1,  1640,    -1,   408,    -1,
    1382,    -1,    -1,  1591,    -1,    -1,   105,    -1,   418,    -1,
      -1,   110,    -1,   112,   113,   114,   115,   116,   117,   118,
      -1,    -1,    -1,  1538,    -1,    -1,    -1,   105,    -1,    -1,
      -1,    -1,   110,    -1,   112,   113,   114,   115,   116,   117,
     118,    -1,    -1,    -1,    -1,    -1,  1607,    -1,   458,   459,
      -1,  1485,   462,    -1,    -1,   154,   155,    -1,   157,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1503,
      -1,    -1,    -1,    -1,  1508,    -1,   154,   155,    -1,   157,
      -1,   180,    -1,  1517,     4,    -1,    -1,    -1,  1522,  1523,
      -1,    -1,  1526,  1527,    -1,    -1,    -1,   507,  1508,    -1,
      -1,    -1,   180,   202,  1538,    -1,    -1,    -1,    -1,  1508,
    1544,  1545,    -1,    -1,    -1,    -1,  1550,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   202,    45,  1508,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1544,  1545,    -1,    -1,    -1,    -1,
    1550,    -1,   552,   553,    -1,  1544,  1545,    -1,   558,  1583,
      -1,  1550,    -1,    -1,    -1,    -1,  1590,    -1,    -1,    -1,
      -1,    -1,  1544,  1545,    -1,    -1,   576,    -1,  1550,    -1,
      -1,    -1,  1606,  1583,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1591,    -1,    -1,  1583,   105,    -1,    -1,    -1,    -1,
     110,    -1,   112,   113,   114,   115,   116,   117,   118,    -1,
      -1,  1583,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1646,    -1,    -1,    -1,    -1,    -1,  1652,    -1,
     630,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   154,   155,  1646,   157,    -1,    -1,
      -1,    -1,  1652,    -1,    -1,    -1,    -1,  1646,    -1,    -1,
      -1,    -1,    -1,  1652,    -1,    -1,   666,    -1,   668,    -1,
     180,    -1,    -1,    -1,  1646,    -1,    -1,    -1,    -1,    -1,
    1652,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   689,
     690,    -1,   202,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,   702,   703,   704,   705,   706,   707,   708,    -1,
      -1,    -1,    -1,    29,   714,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,   738,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,   755,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   764,    -1,   766,   767,    -1,    26,
      27,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,
     780,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   788,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   799,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     810,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   818,    -1,
      -1,   821,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     840,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,   883,   884,    -1,    -1,    -1,    -1,   205,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   897,    -1,   899,
      -1,   901,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,   916,    -1,    -1,   919,
      -1,    -1,   922,   923,   924,   925,   926,   927,   928,   929,
     930,   931,   932,   933,   934,   935,   936,   937,   938,   939,
     940,   941,   942,   943,   944,   945,   946,   947,   948,    -1,
      -1,    63,    64,    -1,    -1,    -1,   213,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   972,    -1,    -1,   975,    -1,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
    1000,    53,  1002,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1010,    -1,    -1,    65,   202,   127,    -1,    -1,   275,    -1,
      -1,    -1,    -1,    -1,    -1,   282,   283,    -1,    -1,    -1,
      -1,    -1,   289,    -1,    -1,    -1,    -1,    -1,   295,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1047,   305,    -1,
      -1,    -1,  1052,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1066,  1067,    -1,  1069,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1081,    -1,    -1,  1084,    -1,  1086,   199,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,   369,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,  1139,
      -1,    -1,  1142,    -1,    -1,    -1,    -1,    65,    -1,    -1,
     202,   408,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,  1164,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,    27,    -1,    -1,    30,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,   462,    -1,    -1,    -1,    -1,
      67,    68,    -1,    -1,    65,    -1,    -1,    -1,    -1,  1219,
      77,  1221,    79,    -1,    -1,    -1,  1226,  1227,    -1,    -1,
    1230,    -1,  1232,    -1,    -1,  1235,    -1,    -1,    -1,  1239,
      -1,    -1,  1242,  1243,    -1,  1245,    -1,    -1,    -1,    -1,
     507,    -1,  1252,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     117,    -1,    -1,    -1,    -1,    -1,  1266,    -1,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,    -1,   202,    -1,    -1,    77,    -1,    -1,
    1290,   148,    -1,    -1,   151,   152,    52,   154,   155,    -1,
     157,   158,   159,    -1,    -1,  1305,    -1,    -1,    -1,   166,
      -1,    -1,    -1,    -1,    -1,  1315,  1316,    63,    64,   576,
      -1,    -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,    -1,
    1340,   198,    -1,    -1,    -1,    -1,   203,    -1,    -1,  1349,
      -1,   202,    -1,    -1,   207,    -1,    -1,    -1,   148,    -1,
     213,   151,    -1,    -1,   154,   155,   219,   157,   158,   159,
      -1,    -1,    -1,   630,    -1,    -1,    -1,    -1,    -1,  1379,
    1380,   127,    -1,    -1,    -1,  1385,  1386,    -1,    -1,    -1,
    1390,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    -1,    -1,    -1,    -1,   666,
      -1,   668,   202,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   275,    -1,    -1,    -1,    -1,    -1,    -1,   282,
     283,    -1,   689,   690,    -1,    -1,   289,    -1,    -1,    10,
      11,    12,   295,    -1,    -1,    -1,    -1,   704,   705,   706,
     707,   708,   305,    -1,    -1,    -1,    -1,   714,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,   738,    53,    -1,    -1,    -1,    -1,    -1,    -1,  1489,
      -1,   247,   248,    -1,    65,    -1,    -1,   253,   755,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   764,    -1,   766,
     767,    -1,    -1,    -1,    -1,    -1,   369,  1517,    -1,    -1,
      -1,  1521,    -1,   780,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   788,  1532,    -1,    -1,    -1,    -1,  1537,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    -1,    -1,    -1,    -1,   408,    -1,    -1,    -1,    -1,
      -1,   818,    -1,    -1,   821,   418,    -1,    -1,    -1,   325,
      -1,    -1,   328,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   840,    -1,    -1,    -1,    63,    64,    -1,
      -1,    -1,    -1,    -1,  1594,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1602,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1616,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1625,   883,   884,    -1,    -1,
      -1,   202,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1640,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1649,
      -1,   127,    -1,    -1,    -1,    -1,    -1,  1657,    -1,   916,
      -1,  1661,   919,    -1,  1664,   922,   923,   924,   925,   926,
     927,   928,   929,   930,   931,   932,   933,   934,   935,   936,
     937,   938,   939,   940,   941,   942,   943,   944,   945,   946,
     947,   948,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   458,   459,    -1,    -1,   462,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,   972,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   576,    -1,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,   507,    -1,  1010,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   630,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1047,    -1,    -1,    -1,    -1,  1052,   552,   553,    -1,    -1,
      -1,    -1,   558,    -1,    -1,    -1,    -1,    -1,    -1,  1066,
    1067,    -1,  1069,   666,    77,   668,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1084,    -1,  1086,
      -1,    10,    11,    12,    -1,    -1,   689,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1102,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1139,    -1,    77,  1142,    65,    -1,    -1,    -1,
      -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,   202,
      -1,    -1,   755,    -1,    -1,    -1,    -1,  1164,    -1,    -1,
      -1,    -1,    -1,   766,   767,    -1,    -1,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,    -1,   690,   788,    -1,    -1,   201,    -1,
     203,    -1,    -1,    -1,    -1,    -1,   702,   703,   704,   705,
     706,   707,   708,    -1,    -1,    -1,    -1,    -1,   714,   152,
      -1,   154,   155,   156,   157,   158,   159,    -1,    -1,  1226,
    1227,    -1,    -1,  1230,    -1,  1232,    -1,    -1,  1235,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1243,   840,  1245,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,    -1,    -1,   198,    -1,    -1,    -1,  1266,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   202,    -1,    -1,    -1,    -1,    -1,    -1,
     883,   884,    -1,  1290,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   799,    -1,    -1,    -1,    -1,  1305,    -1,
      10,    11,    12,    -1,   810,    -1,    -1,    -1,  1315,  1316,
      -1,    -1,   818,    -1,    -1,    -1,   919,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,  1349,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,  1379,  1380,    -1,    -1,    -1,    -1,  1385,  1386,
      -1,    -1,    -1,  1390,    -1,    65,    -1,    -1,    -1,    -1,
      -1,   897,    -1,   899,    -1,   901,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1010,    -1,    -1,
     916,    -1,    -1,    -1,    -1,    -1,   922,   923,   924,   925,
     926,   927,   928,   929,   930,   931,   932,   933,   934,   935,
     936,   937,   938,   939,   940,   941,   942,   943,   944,   945,
     946,   947,   948,    -1,  1047,    -1,    -1,    -1,    -1,  1052,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1066,  1067,    -1,   972,    -1,    -1,   975,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    -1,    -1,    -1,
      -1,    -1,  1489,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   202,    -1,  1000,    -1,  1002,    -1,    -1,    -1,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1521,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    77,    -1,  1532,    -1,    -1,    -1,    -1,
    1537,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,  1142,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,  1069,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,  1081,   130,   131,  1084,    -1,
    1086,    -1,    -1,    -1,    -1,    -1,    -1,  1594,    -1,    -1,
      -1,    -1,    -1,    -1,   148,  1602,  1102,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,    -1,   161,    -1,  1616,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   172,    -1,
      -1,    -1,    -1,    -1,  1227,    -1,    -1,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,    -1,  1649,    -1,   198,    10,    11,    12,    -1,    -1,
    1657,    -1,    -1,    -1,  1661,    -1,    -1,  1664,  1164,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,  1290,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    79,
      -1,   202,    -1,  1219,    -1,  1221,    -1,    -1,    -1,    -1,
    1226,    -1,    -1,    -1,  1230,    -1,  1232,    -1,    -1,  1235,
      -1,    -1,    -1,  1239,    -1,    -1,  1242,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,  1252,    -1,    -1,    -1,
      -1,    -1,    -1,   123,    -1,    -1,    -1,    -1,    -1,    29,
    1266,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    53,   154,   155,    -1,   157,   158,   159,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,  1305,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    -1,    -1,    -1,    -1,    -1,
      -1,   201,    -1,   203,  1340,     5,     6,   202,     8,     9,
      10,    11,    12,  1349,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    -1,    -1,    28,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    41,    -1,  1379,  1380,    -1,    -1,    -1,    48,  1385,
      50,    -1,    -1,    53,    -1,    55,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,   202,    -1,    -1,    -1,    -1,    -1,    -1,    77,
      -1,    79,   112,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,  1489,    -1,    -1,    -1,    -1,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,  1517,    53,    -1,   184,  1521,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    65,    -1,  1532,    -1,    -1,    -1,
      -1,  1537,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   200,   228,    -1,
      -1,   231,    -1,   201,    -1,   203,    -1,    -1,   238,   239,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,  1594,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,   462,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   284,   200,    -1,    -1,    -1,  1625,
     290,    -1,    -1,    -1,   294,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1640,    -1,   306,   307,    -1,    -1,
      -1,    -1,    -1,  1649,    -1,    -1,   316,   507,    -1,    -1,
      -1,  1657,    -1,    -1,    -1,  1661,    -1,   327,  1664,   200,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,    -1,
     370,    -1,   372,   373,    -1,   375,    -1,    -1,    -1,    -1,
      -1,    -1,   382,   383,   384,   385,   386,   387,   388,   389,
     390,   391,   392,   393,   394,    -1,    -1,    -1,    -1,    -1,
     400,   401,    -1,   403,   404,   405,    -1,    -1,    -1,    -1,
      -1,   411,    -1,    -1,   414,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   423,    -1,   425,    -1,    -1,    -1,    -1,
      -1,   431,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   441,    -1,   443,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,   462,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   469,
      -1,    -1,   472,   473,   474,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
     690,    -1,    -1,   503,    -1,    -1,   507,    10,    11,    12,
      -1,    65,    -1,    -1,   704,   705,   706,   707,   708,    -1,
      -1,    -1,    -1,    -1,   714,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    77,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,   577,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,   589,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   621,    -1,    -1,    65,    -1,    -1,    -1,   818,   629,
     154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   200,   647,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   462,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,    -1,    -1,    -1,   674,    -1,    -1,   201,    -1,   203,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   687,    -1,   690,
      -1,    -1,    -1,    -1,    -1,    -1,    77,   200,    79,    -1,
      -1,   507,    -1,   704,   705,   706,   707,   708,    -1,    -1,
      -1,    -1,    -1,   714,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   723,    -1,    -1,   916,    -1,    -1,    -1,
      -1,    -1,   922,   923,   924,   925,   926,   927,   928,   929,
     930,   931,   932,   933,   934,   935,   936,   937,   938,   939,
     940,   941,   942,   943,   944,   945,   946,   947,   948,   200,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   154,   155,   775,   157,   158,   159,    -1,
      -1,    -1,   972,    -1,    -1,    -1,    -1,   787,    -1,    -1,
      -1,    -1,   792,    -1,   794,    -1,    -1,    -1,    -1,    -1,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    -1,    -1,   816,   818,    -1,    -1,
     201,    -1,   203,    -1,    -1,    -1,   826,    11,    12,   829,
      -1,   831,    -1,    -1,    -1,   835,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   844,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,   869,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1069,
      -1,    65,    -1,    -1,   690,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1084,    -1,  1086,    -1,   704,   705,
     706,   707,    -1,    -1,    -1,    -1,    -1,    -1,   714,    -1,
      -1,    -1,  1102,    -1,    -1,   916,    -1,    -1,    -1,    -1,
      -1,   922,   923,   924,   925,   926,   927,   928,   929,   930,
     931,   932,   933,   934,   935,   936,   937,   938,   939,   940,
     941,   942,   943,   944,   945,   946,   947,   948,    -1,    -1,
     950,   951,   952,    -1,    -1,    -1,   956,   957,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,   972,    -1,    -1,  1164,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   984,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,  1006,    53,    -1,    -1,
      -1,  1011,   818,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,  1024,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1032,    -1,  1034,    -1,  1226,    -1,    -1,    -1,
    1230,    -1,  1232,    -1,    -1,  1235,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,  1063,    -1,    -1,    -1,    -1,  1069,    -1,
      -1,    -1,    -1,  1073,    -1,    65,  1266,    -1,    -1,    -1,
      -1,    -1,    -1,  1084,    -1,  1086,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1305,   922,   923,   924,   925,
     926,   927,   928,   929,   930,   931,   932,   933,   934,   935,
     936,   937,   938,   939,   940,   941,   942,   943,   944,   945,
     946,   947,   948,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1151,    -1,   199,    -1,  1155,    -1,  1157,  1158,  1349,
      -1,    -1,    -1,  1164,    -1,    -1,   972,    -1,  1168,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1176,  1177,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1185,    -1,    -1,    -1,  1379,
    1380,    -1,    -1,    -1,    -1,  1385,    -1,    -1,    -1,  1389,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,  1226,    -1,    -1,    -1,  1230,
      -1,  1232,    -1,    -1,  1235,    29,  1236,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,  1069,    -1,  1266,    -1,    -1,    -1,  1269,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,  1084,    -1,
    1086,    -1,    77,    -1,    10,    11,    12,    -1,    -1,    -1,
      85,    -1,    -1,    -1,    -1,    -1,  1102,    -1,    -1,  1489,
      -1,    -1,    -1,    29,  1305,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,
      -1,  1521,    77,    -1,    79,    -1,    -1,    -1,    -1,    65,
      85,    -1,  1532,    -1,    -1,    -1,    -1,  1537,  1349,    -1,
      -1,    -1,  1352,   148,    -1,    -1,   151,    -1,  1164,   154,
     155,    -1,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,
    1560,    -1,   117,    -1,    -1,    -1,    -1,    -1,  1379,  1380,
      -1,  1381,    -1,    -1,  1385,    -1,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      -1,    -1,    -1,   148,  1594,   199,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,
    1226,    -1,    -1,    -1,  1230,    -1,  1232,    -1,    -1,  1235,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      -1,     3,     4,     5,     6,     7,    -1,    -1,   203,  1649,
    1266,    13,    -1,    -1,    -1,    -1,    -1,  1657,    -1,    -1,
      -1,  1661,    -1,   199,  1664,    27,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,  1489,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,  1305,
      52,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    69,    -1,    -1,
    1521,    -1,    74,    75,    76,    77,    78,    79,    -1,    -1,
      -1,  1532,    -1,    85,    -1,    -1,  1537,    -1,    -1,    -1,
      -1,    -1,    -1,  1349,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     112,   113,   114,   115,   116,   117,    -1,    -1,   120,   121,
      -1,    -1,    -1,  1379,  1380,    -1,    -1,   129,   130,  1385,
     132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,
      -1,   143,    -1,  1594,    -1,    -1,   148,   149,   150,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,
     172,   173,   174,    -1,    -1,    -1,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,  1649,    -1,
      -1,   203,   204,    -1,   206,   207,  1657,    -1,    -1,    -1,
    1661,    -1,    -1,  1664,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1489,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1521,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1532,    -1,    -1,    -1,
      -1,  1537,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    -1,    -1,    -1,    85,    86,    87,    88,    -1,
      90,    -1,    92,    -1,    94,    -1,    -1,    97,  1594,    -1,
      -1,   101,   102,   103,   104,   105,   106,   107,    -1,   109,
     110,   111,   112,   113,   114,   115,   116,   117,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,    -1,    -1,   129,
     130,    -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,
      -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,   152,  1649,   154,   155,    -1,   157,   158,   159,
     160,  1657,    -1,   163,    -1,  1661,   166,    -1,  1664,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,    -1,   179,
     180,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,   198,    -1,
     200,   201,   202,   203,   204,    -1,   206,   207,     3,     4,
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
     105,   106,   107,    -1,   109,   110,   111,   112,   113,   114,
     115,   116,   117,    -1,   119,   120,   121,   122,   123,   124,
      -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,
     135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,
      -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,   160,    -1,    -1,   163,    -1,
      -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,   174,
     175,    -1,   177,    -1,   179,   180,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,   198,    -1,   200,   201,   202,   203,   204,
      -1,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    -1,    -1,    -1,    85,    86,    87,    88,    -1,
      90,    -1,    92,    -1,    94,    -1,    -1,    97,    -1,    -1,
      -1,   101,   102,   103,   104,   105,   106,   107,    -1,   109,
     110,   111,   112,   113,   114,   115,   116,   117,    -1,   119,
     120,   121,   122,   123,   124,    -1,    -1,    -1,    -1,   129,
     130,    -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,
      -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,   152,    -1,   154,   155,    -1,   157,   158,   159,
     160,    -1,    -1,   163,    -1,    -1,   166,    -1,    -1,    -1,
      -1,    -1,   172,   173,   174,   175,    -1,   177,    -1,   179,
     180,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,   198,    -1,
     200,   201,    -1,   203,   204,    -1,   206,   207,     3,     4,
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
      -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,   198,    -1,   200,   201,   202,   203,   204,
      -1,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
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
      -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,   179,
     180,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,   198,    -1,
     200,   201,   202,   203,   204,    -1,   206,   207,     3,     4,
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
      -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,   198,    -1,   200,   201,   202,   203,   204,
      -1,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    -1,    -1,    -1,    85,    86,    87,    88,    89,
      90,    -1,    92,    -1,    94,    -1,    -1,    97,    -1,    -1,
      -1,   101,   102,   103,   104,    -1,   106,   107,    -1,   109,
      -1,   111,   112,   113,   114,   115,   116,   117,    -1,   119,
     120,   121,    -1,   123,   124,    -1,    -1,    -1,    -1,   129,
     130,    -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,
      -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,   152,    -1,   154,   155,    -1,   157,   158,   159,
     160,    -1,    -1,   163,    -1,    -1,   166,    -1,    -1,    -1,
      -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,   179,
     180,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,   198,    -1,
     200,   201,    -1,   203,   204,    -1,   206,   207,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,
      85,    86,    87,    88,    -1,    90,    -1,    92,    -1,    94,
      95,    -1,    97,    -1,    -1,    -1,   101,   102,   103,   104,
      -1,   106,   107,    -1,   109,    -1,   111,   112,   113,   114,
     115,   116,   117,    -1,   119,   120,   121,    -1,   123,   124,
      -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,
     135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,
      -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,   160,    -1,    -1,   163,    -1,
      -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,   174,
      -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,   198,    -1,   200,   201,    -1,   203,   204,
      -1,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
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
      -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,   179,
     180,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,   198,    -1,
     200,   201,   202,   203,   204,    -1,   206,   207,     3,     4,
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
      -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,   198,    -1,   200,   201,   202,   203,   204,
      -1,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,    -1,
      -1,    -1,    52,    -1,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    69,
      70,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      -1,    81,    -1,    -1,    -1,    85,    86,    87,    88,    -1,
      90,    -1,    92,    93,    94,    -1,    -1,    97,    -1,    -1,
      -1,   101,   102,   103,   104,    -1,   106,   107,    -1,   109,
      -1,   111,   112,   113,   114,   115,   116,   117,    -1,   119,
     120,   121,    -1,   123,   124,    -1,    -1,    -1,    -1,   129,
     130,    -1,   132,   133,   134,   135,   136,    -1,    -1,    -1,
      -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,
     150,   151,   152,    -1,   154,   155,    -1,   157,   158,   159,
     160,    -1,    -1,   163,    -1,    -1,   166,    -1,    -1,    -1,
      -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,   179,
     180,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,   198,    -1,
     200,   201,    -1,   203,   204,    -1,   206,   207,     3,     4,
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
      -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,   198,    -1,   200,   201,   202,   203,   204,
      -1,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
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
      -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,   179,
     180,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,   198,    -1,
     200,   201,   202,   203,   204,    -1,   206,   207,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    74,
      75,    76,    77,    78,    79,    -1,    81,    -1,    -1,    -1,
      85,    86,    87,    88,    -1,    90,    91,    92,    -1,    94,
      -1,    -1,    97,    -1,    -1,    -1,   101,   102,   103,   104,
      -1,   106,   107,    -1,   109,    -1,   111,   112,   113,   114,
     115,   116,   117,    -1,   119,   120,   121,    -1,   123,   124,
      -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,
     135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,
      -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,   160,    -1,    -1,   163,    -1,
      -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,   174,
      -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,   198,    -1,   200,   201,    -1,   203,   204,
      -1,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
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
      -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,   179,
     180,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,   198,    -1,
     200,   201,   202,   203,   204,    -1,   206,   207,     3,     4,
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
      -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,   198,    -1,   200,   201,   202,   203,   204,
      -1,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
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
      -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,   179,
     180,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,   198,    -1,
     200,   201,   202,   203,   204,    -1,   206,   207,     3,     4,
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
      -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,   198,    -1,   200,   201,    -1,   203,   204,
      -1,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
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
      -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,   179,
     180,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,   198,    -1,
     200,   201,    -1,   203,   204,    -1,   206,   207,     3,     4,
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
      -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,   198,    -1,   200,   201,    -1,   203,   204,
      -1,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
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
      -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,   179,
     180,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,   198,    -1,
     200,   201,    -1,   203,   204,    -1,   206,   207,     3,     4,
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
      -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,   198,    -1,   200,   201,    -1,   203,   204,
      -1,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
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
      -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,   179,
     180,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,   198,    -1,
     200,   201,    -1,   203,   204,    -1,   206,   207,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,    -1,    -1,   198,    -1,   200,   201,    -1,   203,   204,
      -1,   206,   207,     3,     4,     5,     6,     7,    -1,    -1,
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
      -1,    -1,    -1,   163,    -1,    -1,   166,     3,     4,     5,
       6,     7,   172,   173,   174,    -1,    -1,    13,    -1,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,   198,    35,
     200,    -1,    -1,   203,   204,    -1,   206,   207,    -1,    -1,
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
      -1,   157,   158,   159,    -1,   161,    -1,   163,    -1,    -1,
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
     152,    -1,   154,   155,    -1,   157,   158,   159,    -1,   161,
      -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,
     172,   173,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,    -1,    -1,   198,    -1,    -1,    -1,
      -1,   203,   204,    -1,   206,   207,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
       3,     4,     5,     6,     7,   172,   173,   174,    -1,    -1,
      13,    -1,   179,   180,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,    -1,
      -1,   198,    -1,    -1,    -1,    -1,   203,   204,    -1,   206,
     207,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,    -1,    -1,    -1,
      -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   101,    -1,
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
     195,    -1,    -1,   198,    -1,   200,    -1,    -1,   203,   204,
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
     191,   192,   193,   194,   195,    -1,    -1,   198,    -1,   200,
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
      -1,   198,   199,    -1,    -1,    -1,   203,   204,    -1,   206,
     207,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    30,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
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
     192,   193,   194,   195,    -1,    -1,   198,    35,    -1,    -1,
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
     198,    35,    -1,    -1,    -1,   203,   204,    -1,   206,   207,
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
     194,   195,    -1,    -1,   198,    35,    -1,    -1,    -1,   203,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   112,   113,   114,   115,
     116,   117,    -1,    -1,   120,   121,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   129,   130,    -1,   132,   133,   134,   135,
     136,    -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,
      -1,    -1,   148,   149,   150,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,    -1,    -1,    -1,   163,    -1,    -1,
     166,     3,     4,     5,     6,     7,   172,   173,   174,    -1,
      -1,    13,    -1,   179,   180,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      -1,    -1,   198,    35,    -1,   201,    -1,   203,   204,    -1,
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
     198,    -1,    -1,    -1,    -1,   203,   204,    -1,   206,   207,
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
      -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,
     174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,   198,    -1,    -1,    -1,    -1,   203,
     204,    -1,   206,   207,     3,     4,     5,     6,     7,    -1,
      -1,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    53,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    67,    68,
      69,    70,    71,    72,    73,    -1,    -1,    -1,    77,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    -1,    -1,
     129,   130,    -1,   132,   133,   134,   135,   136,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   148,
     149,   150,    -1,    -1,    -1,   154,   155,    -1,   157,   158,
     159,   160,    -1,   162,   163,    -1,   165,    -1,    -1,    -1,
      -1,    -1,    -1,   172,    -1,    -1,   175,    -1,   177,    -1,
     179,   180,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
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
     194,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    29,    53,    -1,    -1,    -1,    -1,
     190,   191,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      55,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,   187,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,   113,   114,
     115,   116,   117,    -1,   186,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   130,   131,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    -1,   148,    -1,    -1,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,    -1,    -1,   185,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,   172,    29,    -1,
      -1,    -1,    -1,    -1,    -1,   180,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     130,   131,    -1,   198,    55,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,
      -1,   151,   152,    -1,   154,   155,    77,   157,   158,   159,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   172,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    -1,    -1,    -1,   198,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,
     131,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,
     151,   152,    -1,   154,   155,    -1,   157,   158,   159,    -1,
     161,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   172,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   130,   131,    -1,   198,    -1,    -1,
      77,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   148,    -1,    -1,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   172,    -1,    -1,    -1,
     117,    -1,    -1,    30,    -1,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,    46,
      47,    -1,   198,    -1,    -1,    52,    -1,    54,    -1,    -1,
      -1,   148,    -1,    -1,   151,   152,    -1,   154,   155,    66,
     157,   158,   159,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    -1,    -1,    -1,    -1,    -1,    35,    -1,    85,    -1,
      -1,    -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,    -1,
      -1,   198,    -1,    -1,   201,    -1,   203,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,
      79,    -1,    -1,   130,    -1,   132,   133,   134,   135,   136,
      -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,
      -1,   148,   149,   150,   151,   152,    -1,   154,   155,    -1,
     157,   158,   159,    -1,    -1,    -1,   163,    -1,   117,    -1,
      -1,    -1,    -1,    -1,    -1,   172,    -1,    -1,    -1,    -1,
     129,    -1,   179,    77,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,   148,
      -1,   198,   151,   152,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,    65,    46,    47,   198,
      -1,    -1,    -1,    52,   203,    54,    -1,   151,    -1,    -1,
     154,   155,    -1,   157,   158,   159,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    85,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,    -1,    -1,    -1,    -1,    -1,    29,   201,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,   130,    -1,   132,   133,   134,   135,   136,    -1,    -1,
      77,    -1,    65,    -1,   143,    -1,    -1,    -1,    -1,   148,
     149,   150,   151,   152,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   172,    -1,    -1,    46,    47,    -1,    -1,
     179,    -1,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,    66,    -1,    -1,   198,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    85,    -1,   154,   155,    -1,
     157,   158,   159,    77,    -1,    79,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    65,    -1,
     130,   198,    -1,   117,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   154,   155,    -1,   157,   158,   159,
      74,    75,    76,    77,   148,    -1,    -1,   151,   152,    -1,
     154,   155,   172,   157,   158,   159,    77,    -1,    79,    -1,
      -1,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    -1,    -1,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,    -1,    -1,    -1,   198,    -1,   117,    -1,    -1,   203,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     154,   155,    -1,   157,   158,   159,    -1,   148,    -1,    -1,
     151,   152,    -1,   154,   155,    -1,   157,   158,   159,    77,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    -1,    -1,    -1,   198,    -1,   117,
      -1,    -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   129,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     148,    -1,    -1,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,    77,    -1,    -1,
     198,    -1,    -1,   148,    -1,   203,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,    77,    -1,    79,    -1,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      65,    -1,    -1,   198,    -1,   117,    -1,    77,   203,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   154,   155,    -1,   157,   158,   159,
      -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,   121,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    -1,    -1,    -1,   198,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    -1,   154,   155,   198,   157,   158,   159,
      -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    -1,    -1,    29,   198,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   128,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
     128,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   128,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
     128,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   128,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
     128,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   128,    29,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    29,
     128,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77,    -1,    -1,    65,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    29,   128,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,   118,    -1,    -1,    -1,    77,    -1,   128,    -1,
      -1,    -1,    -1,   148,   130,   131,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,    77,    -1,    79,    80,    -1,
      -1,    -1,   148,   104,   105,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,    -1,    -1,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      -1,    -1,    -1,    -1,   128,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,    77,
     151,    -1,    -1,   154,   155,    -1,   157,   158,   159,    -1,
      -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    -1,    -1,    -1,    -1,    -1,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    77,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   152,    -1,   154,   155,    77,   157,
     158,   159,    -1,    -1,    -1,    -1,   151,    -1,    -1,   154,
     155,    -1,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      -1,    -1,    -1,    -1,    -1,    77,    -1,   151,    -1,    -1,
     154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,    -1,
      77,    -1,   151,    -1,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   123,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   123,    77,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,   155,    -1,
     157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    -1,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,    -1,
      -1,    -1,    -1,    -1,   154,   155,    -1,   157,   158,   159,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    28,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    96,    53,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    12,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    12,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65
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
     251,   256,   262,   317,   318,   323,   327,   328,   329,   330,
     331,   332,   333,   334,   336,   339,   348,   349,   350,   351,
     353,   354,   356,   375,   385,   386,   387,   392,   395,   413,
     418,   420,   421,   422,   423,   424,   425,   426,   427,   429,
     444,   446,   448,   115,   116,   117,   129,   148,   215,   246,
     317,   333,   420,   333,   198,   333,   333,   333,   101,   333,
     333,   411,   412,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,    79,   117,   198,   223,   386,
     387,   420,   420,    35,   333,   433,   434,   333,   117,   198,
     223,   386,   387,   388,   419,   425,   430,   431,   198,   324,
     389,   198,   324,   340,   325,   333,   232,   324,   198,   198,
     198,   324,   200,   333,   215,   200,   333,    29,    55,   130,
     131,   152,   172,   198,   215,   226,   449,   460,   461,   181,
     200,   330,   333,   355,   357,   201,   239,   333,   104,   105,
     151,   216,   219,   222,    79,   203,   288,   289,   123,   123,
      79,   290,   198,   198,   198,   198,   215,   260,   450,   198,
     198,    79,    84,   144,   145,   146,   441,   442,   151,   201,
     222,   222,   261,   450,   152,   198,   198,   198,   450,   450,
     341,   323,   333,   334,   420,   228,   201,    84,   390,   441,
      84,   441,   441,    30,   151,   168,   451,   198,     9,   200,
      35,   245,   152,   259,   450,   117,   246,   318,   200,   200,
     200,   200,   200,   200,    10,    11,    12,    29,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    53,
      65,   200,    66,    66,   200,   201,   147,   124,   160,   262,
     316,   317,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    63,    64,   127,   415,   416,
      66,   201,   417,   198,    66,   201,   203,   426,   198,   245,
     246,    14,   333,   200,   128,    44,   215,   410,   198,   323,
     420,   147,   420,   128,   205,     9,   397,   323,   420,   451,
     147,   198,   391,   127,   415,   416,   417,   199,   333,    30,
     230,     8,   342,     9,   200,   230,   231,   325,   326,   333,
     215,   274,   234,   200,   200,   200,   461,   461,   168,   198,
     104,   461,    14,   215,    79,   200,   200,   200,   181,   182,
     183,   188,   189,   192,   193,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   370,   371,   372,   240,   108,   165,
     200,   151,   217,   220,   222,   151,   218,   221,   222,   222,
       9,   200,    96,   201,   420,     9,   200,    14,     9,   200,
     420,   445,   445,   323,   334,   420,   199,   168,   254,   129,
     420,   432,   433,    66,   127,   144,   442,    78,   333,   420,
      84,   144,   442,   222,   214,   200,   201,   257,   376,   378,
      85,   203,   343,   344,   346,   387,   426,   446,   333,   439,
     440,   439,    14,    96,   447,   284,   285,   413,   414,   199,
     199,   199,   202,   229,   230,   247,   251,   256,   333,   204,
     206,   207,   215,   452,   453,   461,    35,   161,   286,   287,
     333,   449,   198,   450,   252,   245,   333,   333,   333,    30,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   333,   388,   333,   333,   428,   428,   333,   435,   436,
     123,   201,   215,   425,   426,   260,   261,   259,   246,    27,
      35,   327,   330,   333,   355,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   201,   215,   425,
     428,   333,   286,   428,   333,   432,   245,   199,   333,   198,
     409,     9,   397,   323,   199,   215,    35,   333,    35,   333,
     199,   199,   425,   286,   201,   215,   425,   199,   228,   278,
     201,   333,   333,    88,    30,   230,   272,   200,    28,    96,
      14,     9,   199,    30,   201,   275,   461,    85,   226,   457,
     458,   459,   198,     9,    46,    47,    52,    54,    66,    85,
     130,   143,   152,   172,   198,   223,   224,   226,   352,   386,
     392,   393,   394,   184,    79,   333,    79,    79,   333,   367,
     368,   333,   333,   360,   370,   187,   373,   228,   198,   238,
     222,   200,     9,    96,   222,   200,     9,    96,    96,   219,
     215,   333,   289,   393,    79,     9,   199,   199,   199,   199,
     199,   200,   215,   456,   125,   265,   198,     9,   199,   199,
      79,    80,   215,   443,   215,    66,   202,   202,   211,   213,
     126,   264,   167,    50,   152,   167,   380,   128,     9,   397,
     199,   147,   128,   199,     9,   397,   199,   461,   461,    14,
     196,     9,   398,   461,   462,   127,   415,   416,   417,   202,
       9,   397,   169,   420,   333,   199,     9,   398,    14,   337,
     248,   125,   263,   198,   450,   333,    30,   205,   205,   128,
     202,     9,   397,   333,   451,   198,   255,   258,   253,   245,
      68,   420,   333,   451,   198,   205,   202,   199,   205,   202,
     199,    46,    47,    66,    74,    75,    76,    85,   130,   143,
     172,   215,   400,   402,   405,   408,   215,   420,   420,   128,
     415,   416,   417,   199,   333,   279,    71,    72,   280,   228,
     324,   228,   326,    96,    35,   129,   269,   420,   393,   215,
      30,   230,   273,   200,   276,   200,   276,     9,   169,   128,
     147,     9,   397,   199,   161,   452,   453,   454,   452,   393,
     393,   393,   393,   393,   396,   399,   198,    84,   147,   198,
     393,   147,   201,    10,    11,    12,    29,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    65,   333,
     184,   184,    14,   190,   191,   369,     9,   194,   373,    79,
     202,   386,   201,   242,    96,   220,   215,    96,   221,   215,
     215,   202,    14,   420,   200,    96,     9,   169,   266,   386,
     201,   432,   129,   420,    14,   205,   333,   202,   211,   266,
     201,   379,    14,   333,   343,   215,   333,   333,   200,   461,
      30,   455,   414,    35,    79,   161,   201,   215,   425,   461,
      35,   161,   333,   393,   284,   198,   386,   264,   338,   249,
     333,   333,   333,   202,   198,   286,   265,   264,   263,   450,
     388,   202,   198,   286,    14,    74,    75,    76,   215,   401,
     401,   402,   403,   404,   198,    84,   144,   198,     9,   397,
     199,   409,    35,   333,   202,    71,    72,   281,   324,   230,
     202,   200,    89,   200,   269,   420,   198,   128,   268,    14,
     228,   276,    98,    99,   100,   276,   202,   461,   461,   215,
     457,     9,   199,   397,   128,   205,     9,   397,   396,   215,
     343,   345,   347,   199,   123,   215,   393,   437,   438,   393,
     393,   393,    30,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   333,   333,   333,   368,
     333,   358,    79,   243,   215,   215,   393,   461,   215,     9,
     291,   199,   198,   327,   330,   333,   205,   202,   291,   153,
     166,   201,   375,   382,   153,   201,   381,   128,   128,   200,
     461,   342,   462,    79,    14,    79,   333,   451,   198,   420,
     333,   199,   284,   201,   284,   198,   128,   198,   286,   199,
     201,   201,   264,   250,   391,   198,   286,   199,   128,   205,
       9,   397,   403,   144,   343,   406,   407,   402,   420,   324,
      30,    73,   230,   200,   326,   268,   432,   269,   199,   393,
      95,    98,   200,   333,    30,   200,   277,   202,   169,   128,
     161,    30,   199,   393,   393,   199,   128,     9,   397,   199,
     128,   202,     9,   397,   393,    30,   185,   199,   228,    96,
     386,     4,   105,   110,   118,   154,   155,   157,   202,   292,
     315,   316,   317,   322,   413,   432,   202,   202,    50,   333,
     333,   333,   333,    35,    79,   161,    14,   393,   202,   198,
     286,   455,   199,   291,   199,   284,   333,   286,   199,   291,
     291,   201,   198,   286,   199,   402,   402,   199,   128,   199,
       9,   397,    30,   228,   200,   199,   199,   199,   235,   200,
     200,   277,   228,   461,   461,   128,   393,   343,   393,   393,
     393,   333,   201,   202,   461,   125,   126,   449,   267,   386,
     118,   130,   131,   152,   158,   301,   302,   303,   386,   156,
     307,   308,   121,   198,   215,   309,   310,   293,   246,   461,
       9,   200,   316,   199,   152,   377,   202,   202,    79,    14,
      79,   393,   198,   286,   199,   110,   335,   455,   202,   455,
     199,   199,   202,   202,   291,   284,   199,   128,   402,   343,
     228,   233,   236,    30,   230,   271,   228,   199,   393,   128,
     128,   186,   228,   386,   386,    14,     9,   200,   201,   201,
       9,   200,     3,     4,     5,     6,     7,    10,    11,    12,
      13,    27,    28,    53,    67,    68,    69,    70,    71,    72,
      73,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   129,   130,   132,   133,   134,   135,   136,   148,
     149,   150,   160,   162,   163,   165,   172,   175,   177,   179,
     180,   215,   383,   384,     9,   200,   152,   156,   215,   310,
     311,   312,   200,    79,   321,   245,   294,   449,   246,    14,
     393,   286,   199,   198,   201,   200,   201,   313,   335,   455,
     202,   199,   402,   128,    30,   230,   270,   271,   228,   393,
     393,   333,   202,   200,   200,   393,   386,   297,   304,   392,
     302,    14,    30,    47,   305,   308,     9,    33,   199,    29,
      46,    49,    14,     9,   200,   450,   321,    14,   245,   393,
     199,    35,    79,   374,   228,   228,   201,   313,   455,   402,
     228,    93,   187,   241,   202,   215,   226,   298,   299,   300,
       9,   202,   393,   384,   384,    55,   306,   311,   311,    29,
      46,    49,   393,    79,   198,   200,   393,   450,    79,     9,
     398,   202,   202,   228,   313,    91,   200,    79,   108,   237,
     147,    96,   392,   159,    14,   295,   198,    35,    79,   199,
     202,   200,   198,   165,   244,   215,   316,   317,   393,   282,
     283,   414,   296,    79,   386,   242,   162,   215,   200,   199,
       9,   398,   112,   113,   114,   319,   320,   282,    79,   267,
     200,   455,   414,   462,   199,   199,   200,   200,   201,   314,
     319,    35,    79,   161,   455,   201,   228,   462,    79,    14,
      79,   314,   228,   202,    35,    79,   161,    14,   393,   202,
      79,    14,    79,   393,    14,   393,   393
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
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 751 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 754 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 756 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 757 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 758 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 759 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 760 "hphp.y"
    { _p->nns(); (yyval).reset();;}
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
#line 765 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
#line 793 "hphp.y"
    { ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 794 "hphp.y"
    { ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 799 "hphp.y"
    { ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 800 "hphp.y"
    { ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 805 "hphp.y"
    { ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 806 "hphp.y"
    { ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 810 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 811 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 812 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 814 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 818 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 819 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 820 "hphp.y"
    { _p->onUseFunction((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 822 "hphp.y"
    { _p->onUseFunction((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 826 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 827 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 828 "hphp.y"
    { _p->onUseConst((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 830 "hphp.y"
    { _p->onUseConst((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 841 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 842 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 845 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 852 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 859 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 867 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         _p->onConst((yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 870 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         _p->onConst((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 876 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 877 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 880 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
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
#line 886 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 890 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 895 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 896 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 898 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 902 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 905 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 909 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 911 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
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
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 919 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 920 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 921 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 922 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 923 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 924 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 925 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 926 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 928 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 929 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 930 "hphp.y"
    { (yyval).reset(); (yyval) = ';';;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 931 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 934 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 936 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]), false);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 941 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 943 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]), true);
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 947 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 954 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 955 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 958 "hphp.y"
    { _p->onCompleteLabelScope(false);;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 959 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 960 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 961 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval));;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 965 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
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
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 972 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 973 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 974 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 983 "hphp.y"
    { _p->onNewLabelScope(false);;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 984 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 993 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 994 "hphp.y"
    { (yyval).reset();;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 998 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1000 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1006 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { (yyval).reset();;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1011 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { (yyval).reset();;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1016 "hphp.y"
    { _p->pushFuncLocation(); ;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1021 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1027 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1033 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1039 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1045 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1051 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1059 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1062 "hphp.y"
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

  case 139:

/* Line 1455 of yacc.c  */
#line 1077 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1080 "hphp.y"
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

  case 141:

/* Line 1455 of yacc.c  */
#line 1094 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1097 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1102 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1105 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1112 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1115 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1123 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1126 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1134 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1135 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1139 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1142 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1145 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1146 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1147 "hphp.y"
    { (yyval) = T_FINAL;;}
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
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1160 "hphp.y"
    { (yyval).reset();;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1163 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1165 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1168 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1170 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1174 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1175 "hphp.y"
    { (yyval).reset();;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1178 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1179 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1180 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1184 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1186 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1189 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1191 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1194 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1196 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1199 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1201 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1211 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1212 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1213 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1214 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1219 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1221 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1222 "hphp.y"
    { (yyval).reset();;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1225 "hphp.y"
    { (yyval).reset();;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1226 "hphp.y"
    { (yyval).reset();;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1231 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1232 "hphp.y"
    { (yyval).reset();;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1237 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1238 "hphp.y"
    { (yyval).reset();;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1241 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1242 "hphp.y"
    { (yyval).reset();;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1245 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1246 "hphp.y"
    { (yyval).reset();;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1254 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),false,
                                                            &(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)])); ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1260 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]), &(yyvsp[(4) - (6)]));
                                        (yyval) = (yyvsp[(1) - (6)]); ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1268 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),false,
                                                            &(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)])); ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), &(yyvsp[(2) - (4)]));
                                        (yyval).reset(); ;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1276 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1282 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1286 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1291 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1296 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1301 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1312 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1318 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1326 "hphp.y"
    { _p->onVariadicParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),
                                        false,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1331 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), NULL);
                                        (yyval) = (yyvsp[(1) - (5)]); ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1335 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1338 "hphp.y"
    { _p->onVariadicParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),
                                                            false,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1342 "hphp.y"
    { validate_hh_variadic_variant(
                                          _p, (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), NULL);
                                        (yyval).reset(); ;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1345 "hphp.y"
    { (yyval).reset();;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1350 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),false,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL); ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1353 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),true,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL); ;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1357 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),true,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL); ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1361 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),false,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL); ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1365 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),false,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL); ;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1369 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),true,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL); ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1374 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),true,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL); ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1379 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),false,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL); ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1385 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1386 "hphp.y"
    { (yyval).reset();;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1389 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),false,false);;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1390 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),true,false);;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1391 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),false,true);;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1393 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),false, false);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1395 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),false,true);;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1397 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),true, false);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1401 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1402 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1405 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1406 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1407 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1411 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1413 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1414 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1415 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1420 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1421 "hphp.y"
    { (yyval).reset();;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1424 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1425 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1428 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1429 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1431 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1435 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1441 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1448 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1454 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1459 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1461 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1463 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1465 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), true); ;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1467 "hphp.y"
    { _p->onClassRequire((yyval), (yyvsp[(3) - (4)]), false); ;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1468 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1471 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1474 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1475 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1476 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1482 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1486 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1489 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1497 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1502 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1505 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1512 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1514 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1518 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1519 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1525 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1527 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1528 "hphp.y"
    { (yyval) = 9; ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1534 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1538 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1539 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1543 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1556 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1568 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1575 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1578 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1581 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1587 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1590 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1591 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1595 "hphp.y"
    { (yyval).reset();;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1596 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1600 "hphp.y"
    { (yyval).reset();;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1601 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1604 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1605 "hphp.y"
    { (yyval).reset();;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1608 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { (yyval).reset();;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1617 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1618 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1619 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1620 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1621 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1622 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1623 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1627 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1628 "hphp.y"
    { (yyval).reset();;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1631 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1632 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1633 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1637 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1639 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1640 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1641 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1645 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1647 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1651 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1654 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1655 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1659 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1663 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1664 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1668 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1669 "hphp.y"
    { (yyval).reset();;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1673 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1678 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1683 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1687 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1696 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1700 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1701 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1702 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1708 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1713 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1714 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1716 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1717 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1718 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1719 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1721 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1722 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1723 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW_EQUAL);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1724 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1725 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1726 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1727 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1728 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1729 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1730 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1731 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1732 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1735 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1736 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1738 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1739 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1744 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1745 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1748 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1749 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1751 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1752 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1761 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { (yyval).reset();;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); ;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { Token v; Token w;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); ;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1872 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1875 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1890 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1891 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1897 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { _p->onMapArray((yyval),(yyvsp[(3) - (4)]),T_MIARRAY);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { _p->onMapArray((yyval),(yyvsp[(3) - (4)]),T_MSARRAY);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1944 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1967 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1969 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1974 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1975 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1977 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1978 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1982 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1986 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1990 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1995 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 2000 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 2004 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 2008 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2009 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2013 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2014 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2018 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2019 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2023 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2024 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2028 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2032 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2036 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2040 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2041 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2042 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2043 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2050 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2053 "hphp.y"
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

  case 497:

/* Line 1455 of yacc.c  */
#line 2064 "hphp.y"
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

  case 498:

/* Line 1455 of yacc.c  */
#line 2075 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2076 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2081 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2082 "hphp.y"
    { (yyval).reset();;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2085 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2086 "hphp.y"
    { (yyval).reset();;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2089 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2093 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2096 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2099 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2106 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2107 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2111 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2115 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2118 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2120 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2121 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2123 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2124 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2125 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2126 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2127 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2128 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2129 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2132 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2133 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2135 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2136 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2137 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2139 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2140 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2141 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2142 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2146 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2147 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2148 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2149 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2150 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2151 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2153 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2154 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2155 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2156 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2159 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2160 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2161 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2162 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2163 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2164 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2165 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2166 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2167 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2168 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2169 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2170 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2171 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2173 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2174 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2175 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2176 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2177 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2178 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2179 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2181 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2182 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2183 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2184 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2185 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2186 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2187 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2190 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2191 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2193 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2194 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2195 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2207 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2210 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2211 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2216 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2222 "hphp.y"
    { (yyval).reset();;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { (yyval).reset();;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { (yyval).reset();;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { (yyval).reset();;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2337 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2338 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2345 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { (yyval).reset();;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { (yyval).reset();;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { (yyval).reset();;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2355 "hphp.y"
    { (yyval).reset();;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { (yyval).reset();;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { (yyval).reset();;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    { (yyval).reset();;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2466 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2473 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2489 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2491 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2496 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2497 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2498 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2499 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2505 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2508 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2512 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2513 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2514 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2515 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2517 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2519 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2521 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2522 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2526 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2527 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2528 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2534 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2537 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2540 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2544 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2552 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2559 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2563 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2567 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2573 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2578 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2579 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2580 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2583 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2584 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2587 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2588 "hphp.y"
    { (yyval).reset();;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2592 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2593 "hphp.y"
    { (yyval)++;;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2597 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2598 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2599 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2601 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2604 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2609 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2611 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2613 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2618 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2619 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2621 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 2623 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2624 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2629 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2630 "hphp.y"
    { (yyval).reset();;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2634 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2635 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2636 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2637 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2640 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2642 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2643 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2644 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2649 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2650 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2654 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2655 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2656 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2657 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2662 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2663 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2672 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2673 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2678 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2679 "hphp.y"
    { _p->onEmptyMapArray((yyval));;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2683 "hphp.y"
    { _p->onMapArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2684 "hphp.y"
    { _p->onMapArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2688 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2690 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2691 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2693 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2698 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2700 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2702 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2704 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2706 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2707 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2710 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2711 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2712 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2716 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2717 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2718 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2719 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2720 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2721 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2722 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2723 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2724 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2728 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2729 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2750 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2760 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2761 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2767 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2771 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2777 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 2778 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 2782 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2785 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2796 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2798 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2799 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2803 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2804 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2809 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); ;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2810 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); ;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2812 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); ;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2813 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2819 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2824 "hphp.y"
    { ;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    {;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2842 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2849 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2852 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2856 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 878:

/* Line 1455 of yacc.c  */
#line 2859 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 879:

/* Line 1455 of yacc.c  */
#line 2862 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2864 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2867 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2870 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2876 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2882 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2890 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2891 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13116 "hphp.tab.cpp"
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
#line 2894 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

