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
#define YYLAST   16293

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  208
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  261
/* YYNRULES -- Number of rules.  */
#define YYNRULES  895
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1693

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
    1661,  1666,  1671,  1676,  1681,  1683,  1685,  1687,  1691,  1694,
    1698,  1703,  1706,  1710,  1712,  1715,  1717,  1720,  1722,  1724,
    1726,  1728,  1730,  1732,  1737,  1742,  1745,  1754,  1765,  1768,
    1770,  1774,  1776,  1779,  1781,  1783,  1785,  1787,  1790,  1795,
    1799,  1803,  1808,  1810,  1813,  1818,  1821,  1828,  1829,  1831,
    1836,  1837,  1840,  1841,  1843,  1845,  1849,  1851,  1855,  1857,
    1859,  1863,  1867,  1869,  1871,  1873,  1875,  1877,  1879,  1881,
    1883,  1885,  1887,  1889,  1891,  1893,  1895,  1897,  1899,  1901,
    1903,  1905,  1907,  1909,  1911,  1913,  1915,  1917,  1919,  1921,
    1923,  1925,  1927,  1929,  1931,  1933,  1935,  1937,  1939,  1941,
    1943,  1945,  1947,  1949,  1951,  1953,  1955,  1957,  1959,  1961,
    1963,  1965,  1967,  1969,  1971,  1973,  1975,  1977,  1979,  1981,
    1983,  1985,  1987,  1989,  1991,  1993,  1995,  1997,  1999,  2001,
    2003,  2005,  2007,  2009,  2011,  2013,  2015,  2017,  2019,  2021,
    2023,  2025,  2027,  2032,  2034,  2036,  2038,  2040,  2042,  2044,
    2046,  2048,  2051,  2053,  2054,  2055,  2057,  2059,  2063,  2064,
    2066,  2068,  2070,  2072,  2074,  2076,  2078,  2080,  2082,  2084,
    2086,  2088,  2090,  2094,  2097,  2099,  2101,  2106,  2110,  2115,
    2117,  2119,  2123,  2127,  2131,  2135,  2139,  2143,  2147,  2151,
    2155,  2159,  2163,  2167,  2171,  2175,  2179,  2183,  2187,  2191,
    2194,  2197,  2200,  2203,  2207,  2211,  2215,  2219,  2223,  2227,
    2231,  2235,  2241,  2246,  2250,  2254,  2258,  2260,  2262,  2264,
    2266,  2270,  2274,  2278,  2281,  2282,  2284,  2285,  2287,  2288,
    2294,  2298,  2302,  2304,  2306,  2308,  2310,  2312,  2316,  2319,
    2321,  2323,  2325,  2327,  2329,  2331,  2334,  2337,  2342,  2346,
    2351,  2354,  2355,  2361,  2365,  2369,  2371,  2375,  2377,  2380,
    2381,  2387,  2391,  2394,  2395,  2399,  2400,  2405,  2408,  2409,
    2413,  2417,  2419,  2420,  2422,  2425,  2428,  2433,  2437,  2441,
    2444,  2449,  2452,  2457,  2459,  2461,  2463,  2465,  2467,  2470,
    2475,  2479,  2484,  2488,  2490,  2492,  2494,  2496,  2499,  2504,
    2509,  2513,  2515,  2517,  2521,  2529,  2536,  2545,  2555,  2564,
    2575,  2583,  2590,  2599,  2601,  2604,  2609,  2614,  2616,  2618,
    2623,  2625,  2626,  2628,  2631,  2633,  2635,  2638,  2643,  2647,
    2651,  2652,  2654,  2657,  2662,  2666,  2669,  2673,  2680,  2681,
    2683,  2688,  2691,  2692,  2698,  2702,  2706,  2708,  2715,  2720,
    2725,  2728,  2731,  2732,  2738,  2742,  2746,  2748,  2751,  2752,
    2758,  2762,  2766,  2768,  2771,  2772,  2778,  2782,  2785,  2788,
    2790,  2793,  2795,  2800,  2804,  2808,  2815,  2819,  2821,  2823,
    2825,  2830,  2835,  2840,  2845,  2848,  2851,  2856,  2859,  2862,
    2864,  2868,  2872,  2876,  2877,  2880,  2886,  2893,  2895,  2898,
    2900,  2905,  2909,  2910,  2912,  2916,  2919,  2923,  2925,  2927,
    2928,  2929,  2932,  2936,  2938,  2944,  2948,  2952,  2958,  2962,
    2964,  2967,  2968,  2973,  2976,  2979,  2981,  2983,  2985,  2987,
    2992,  2999,  3001,  3010,  3017,  3019
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     209,     0,    -1,    -1,   210,   211,    -1,   211,   212,    -1,
      -1,   230,    -1,   247,    -1,   254,    -1,   251,    -1,   259,
      -1,   454,    -1,   122,   198,   199,   200,    -1,   148,   222,
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
     457,    -1,   223,   457,    -1,   227,     9,   455,    14,   399,
      -1,   105,   455,    14,   399,    -1,   228,   229,    -1,    -1,
     230,    -1,   247,    -1,   254,    -1,   259,    -1,   201,   228,
     202,    -1,    70,   330,   230,   281,   283,    -1,    70,   330,
      30,   228,   282,   284,    73,   200,    -1,    -1,    88,   330,
     231,   275,    -1,    -1,    87,   232,   230,    88,   330,   200,
      -1,    -1,    90,   198,   332,   200,   332,   200,   332,   199,
     233,   273,    -1,    -1,    97,   330,   234,   278,    -1,   101,
     200,    -1,   101,   339,   200,    -1,   103,   200,    -1,   103,
     339,   200,    -1,   106,   200,    -1,   106,   339,   200,    -1,
      27,   101,   200,    -1,   111,   291,   200,    -1,   117,   293,
     200,    -1,    86,   331,   200,    -1,   119,   198,   451,   199,
     200,    -1,   200,    -1,    81,    -1,    -1,    92,   198,   339,
      96,   272,   271,   199,   235,   274,    -1,    -1,    92,   198,
     339,    28,    96,   272,   271,   199,   236,   274,    -1,    94,
     198,   277,   199,   276,    -1,    -1,   107,   239,   108,   198,
     392,    79,   199,   201,   228,   202,   241,   237,   244,    -1,
      -1,   107,   239,   165,   238,   242,    -1,   109,   339,   200,
      -1,   102,   215,   200,    -1,   339,   200,    -1,   333,   200,
      -1,   334,   200,    -1,   335,   200,    -1,   336,   200,    -1,
     337,   200,    -1,   106,   336,   200,    -1,   338,   200,    -1,
     362,   200,    -1,   106,   361,   200,    -1,   215,    30,    -1,
      -1,   201,   240,   228,   202,    -1,   241,   108,   198,   392,
      79,   199,   201,   228,   202,    -1,    -1,    -1,   201,   243,
     228,   202,    -1,   165,   242,    -1,    -1,    35,    -1,    -1,
     104,    -1,    -1,   246,   245,   456,   248,   198,   287,   199,
     461,   319,    -1,    -1,   323,   246,   245,   456,   249,   198,
     287,   199,   461,   319,    -1,    -1,   419,   322,   246,   245,
     456,   250,   198,   287,   199,   461,   319,    -1,    -1,   158,
     215,   252,    30,   467,   453,   201,   294,   202,    -1,    -1,
     419,   158,   215,   253,    30,   467,   453,   201,   294,   202,
      -1,    -1,   265,   262,   255,   266,   267,   201,   297,   202,
      -1,    -1,   419,   265,   262,   256,   266,   267,   201,   297,
     202,    -1,    -1,   124,   263,   257,   268,   201,   297,   202,
      -1,    -1,   419,   124,   263,   258,   268,   201,   297,   202,
      -1,    -1,   160,   264,   260,   267,   201,   297,   202,    -1,
      -1,   419,   160,   264,   261,   267,   201,   297,   202,    -1,
     456,    -1,   152,    -1,   456,    -1,   456,    -1,   123,    -1,
     116,   123,    -1,   115,   123,    -1,   125,   392,    -1,    -1,
     126,   269,    -1,    -1,   125,   269,    -1,    -1,   392,    -1,
     269,     9,   392,    -1,   392,    -1,   270,     9,   392,    -1,
     128,   272,    -1,    -1,   426,    -1,    35,   426,    -1,   129,
     198,   438,   199,    -1,   230,    -1,    30,   228,    91,   200,
      -1,   230,    -1,    30,   228,    93,   200,    -1,   230,    -1,
      30,   228,    89,   200,    -1,   230,    -1,    30,   228,    95,
     200,    -1,   215,    14,   399,    -1,   277,     9,   215,    14,
     399,    -1,   201,   279,   202,    -1,   201,   200,   279,   202,
      -1,    30,   279,    98,   200,    -1,    30,   200,   279,    98,
     200,    -1,   279,    99,   339,   280,   228,    -1,   279,   100,
     280,   228,    -1,    -1,    30,    -1,   200,    -1,   281,    71,
     330,   230,    -1,    -1,   282,    71,   330,    30,   228,    -1,
      -1,    72,   230,    -1,    -1,    72,    30,   228,    -1,    -1,
     286,     9,   420,   325,   468,   161,    79,    -1,   286,     9,
     420,   325,   468,   161,    -1,   286,   404,    -1,   420,   325,
     468,   161,    79,    -1,   420,   325,   468,   161,    -1,    -1,
     420,   325,   468,    79,    -1,   420,   325,   468,    35,    79,
      -1,   420,   325,   468,    35,    79,    14,   399,    -1,   420,
     325,   468,    79,    14,   399,    -1,   286,     9,   420,   325,
     468,    79,    -1,   286,     9,   420,   325,   468,    35,    79,
      -1,   286,     9,   420,   325,   468,    35,    79,    14,   399,
      -1,   286,     9,   420,   325,   468,    79,    14,   399,    -1,
     288,     9,   420,   468,   161,    79,    -1,   288,     9,   420,
     468,   161,    -1,   288,   404,    -1,   420,   468,   161,    79,
      -1,   420,   468,   161,    -1,    -1,   420,   468,    79,    -1,
     420,   468,    35,    79,    -1,   420,   468,    35,    79,    14,
     399,    -1,   420,   468,    79,    14,   399,    -1,   288,     9,
     420,   468,    79,    -1,   288,     9,   420,   468,    35,    79,
      -1,   288,     9,   420,   468,    35,    79,    14,   399,    -1,
     288,     9,   420,   468,    79,    14,   399,    -1,   290,   404,
      -1,    -1,   339,    -1,    35,   426,    -1,   161,   339,    -1,
     290,     9,   339,    -1,   290,     9,   161,   339,    -1,   290,
       9,    35,   426,    -1,   291,     9,   292,    -1,   292,    -1,
      79,    -1,   203,   426,    -1,   203,   201,   339,   202,    -1,
     293,     9,    79,    -1,   293,     9,    79,    14,   399,    -1,
      79,    -1,    79,    14,   399,    -1,   294,   295,    -1,    -1,
     296,   200,    -1,   455,    14,   399,    -1,   297,   298,    -1,
      -1,    -1,   321,   299,   327,   200,    -1,    -1,   323,   467,
     300,   327,   200,    -1,   328,   200,    -1,    -1,   322,   246,
     245,   456,   198,   301,   285,   199,   461,   320,    -1,    -1,
     419,   322,   246,   245,   456,   198,   302,   285,   199,   461,
     320,    -1,   154,   307,   200,    -1,   155,   313,   200,    -1,
     157,   315,   200,    -1,     4,   125,   392,   200,    -1,     4,
     126,   392,   200,    -1,   110,   270,   200,    -1,   110,   270,
     201,   303,   202,    -1,   303,   304,    -1,   303,   305,    -1,
      -1,   226,   147,   215,   162,   270,   200,    -1,   306,    96,
     322,   215,   200,    -1,   306,    96,   323,   200,    -1,   226,
     147,   215,    -1,   215,    -1,   308,    -1,   307,     9,   308,
      -1,   309,   389,   311,   312,    -1,   152,    -1,   130,    -1,
     392,    -1,   118,    -1,   158,   201,   310,   202,    -1,   131,
      -1,   398,    -1,   310,     9,   398,    -1,    14,   399,    -1,
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
     399,    -1,    79,    -1,    79,    14,   399,    -1,   328,     9,
     455,    14,   399,    -1,   105,   455,    14,   399,    -1,   198,
     329,   199,    -1,    68,   394,   397,    -1,    67,   339,    -1,
     381,    -1,   356,    -1,   198,   339,   199,    -1,   331,     9,
     339,    -1,   339,    -1,   331,    -1,    -1,    27,   339,    -1,
      27,   339,   128,   339,    -1,   426,    14,   333,    -1,   129,
     198,   438,   199,    14,   333,    -1,    28,   339,    -1,   426,
      14,   336,    -1,   129,   198,   438,   199,    14,   336,    -1,
     340,    -1,   426,    -1,   329,    -1,   129,   198,   438,   199,
      14,   339,    -1,   426,    14,   339,    -1,   426,    14,    35,
     426,    -1,   426,    14,    35,    68,   394,   397,    -1,   426,
      26,   339,    -1,   426,    25,   339,    -1,   426,    24,   339,
      -1,   426,    23,   339,    -1,   426,    22,   339,    -1,   426,
      21,   339,    -1,   426,    20,   339,    -1,   426,    19,   339,
      -1,   426,    18,   339,    -1,   426,    17,   339,    -1,   426,
      16,   339,    -1,   426,    15,   339,    -1,   426,    64,    -1,
      64,   426,    -1,   426,    63,    -1,    63,   426,    -1,   339,
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
      -1,   339,    53,   394,    -1,   198,   340,   199,    -1,   339,
      29,   339,    30,   339,    -1,   339,    29,    30,   339,    -1,
     450,    -1,    62,   339,    -1,    61,   339,    -1,    60,   339,
      -1,    59,   339,    -1,    58,   339,    -1,    57,   339,    -1,
      56,   339,    -1,    69,   395,    -1,    55,   339,    -1,   401,
      -1,   355,    -1,   354,    -1,   357,    -1,   204,   396,   204,
      -1,    13,   339,    -1,   342,    -1,   345,    -1,   359,    -1,
     110,   198,   380,   404,   199,    -1,    -1,    -1,   246,   245,
     198,   343,   287,   199,   461,   341,   201,   228,   202,    -1,
      -1,   323,   246,   245,   198,   344,   287,   199,   461,   341,
     201,   228,   202,    -1,    -1,    79,   346,   348,    -1,    -1,
     195,   347,   287,   196,   461,   348,    -1,     8,   339,    -1,
       8,   201,   228,   202,    -1,    85,    -1,   452,    -1,   350,
       9,   349,   128,   339,    -1,   349,   128,   339,    -1,   351,
       9,   349,   128,   399,    -1,   349,   128,   399,    -1,   350,
     403,    -1,    -1,   351,   403,    -1,    -1,   172,   198,   352,
     199,    -1,   130,   198,   439,   199,    -1,    66,   439,   205,
      -1,   392,   201,   441,   202,    -1,   173,   198,   445,   199,
      -1,   174,   198,   445,   199,    -1,   392,   201,   443,   202,
      -1,   359,    66,   434,   205,    -1,   360,    66,   434,   205,
      -1,   355,    -1,   452,    -1,    85,    -1,   198,   340,   199,
      -1,   363,   364,    -1,   426,    14,   361,    -1,   181,    79,
     184,   339,    -1,   365,   376,    -1,   365,   376,   379,    -1,
     376,    -1,   376,   379,    -1,   366,    -1,   365,   366,    -1,
     367,    -1,   368,    -1,   369,    -1,   370,    -1,   371,    -1,
     372,    -1,   181,    79,   184,   339,    -1,   188,    79,    14,
     339,    -1,   182,   339,    -1,   183,    79,   184,   339,   185,
     339,   186,   339,    -1,   183,    79,   184,   339,   185,   339,
     186,   339,   187,    79,    -1,   189,   373,    -1,   374,    -1,
     373,     9,   374,    -1,   339,    -1,   339,   375,    -1,   190,
      -1,   191,    -1,   377,    -1,   378,    -1,   192,   339,    -1,
     193,   339,   194,   339,    -1,   187,    79,   364,    -1,   380,
       9,    79,    -1,   380,     9,    35,    79,    -1,    79,    -1,
      35,    79,    -1,   166,   152,   382,   167,    -1,   384,    50,
      -1,   384,   167,   385,   166,    50,   383,    -1,    -1,   152,
      -1,   384,   386,    14,   387,    -1,    -1,   385,   388,    -1,
      -1,   152,    -1,   153,    -1,   201,   339,   202,    -1,   153,
      -1,   201,   339,   202,    -1,   381,    -1,   390,    -1,   389,
      30,   390,    -1,   389,    47,   390,    -1,   215,    -1,    69,
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
     199,    -1,   226,    -1,   152,    -1,   392,    -1,   117,    -1,
     432,    -1,   392,    -1,   117,    -1,   436,    -1,   198,   199,
      -1,   330,    -1,    -1,    -1,    84,    -1,   447,    -1,   198,
     289,   199,    -1,    -1,    74,    -1,    75,    -1,    76,    -1,
      85,    -1,   135,    -1,   136,    -1,   150,    -1,   132,    -1,
     163,    -1,   133,    -1,   134,    -1,   149,    -1,   179,    -1,
     143,    84,   144,    -1,   143,   144,    -1,   398,    -1,   224,
      -1,   130,   198,   402,   199,    -1,    66,   402,   205,    -1,
     172,   198,   353,   199,    -1,   400,    -1,   358,    -1,   198,
     399,   199,    -1,   399,    31,   399,    -1,   399,    32,   399,
      -1,   399,    10,   399,    -1,   399,    12,   399,    -1,   399,
      11,   399,    -1,   399,    33,   399,    -1,   399,    35,   399,
      -1,   399,    34,   399,    -1,   399,    48,   399,    -1,   399,
      46,   399,    -1,   399,    47,   399,    -1,   399,    49,   399,
      -1,   399,    50,   399,    -1,   399,    51,   399,    -1,   399,
      45,   399,    -1,   399,    44,   399,    -1,   399,    65,   399,
      -1,    52,   399,    -1,    54,   399,    -1,    46,   399,    -1,
      47,   399,    -1,   399,    37,   399,    -1,   399,    36,   399,
      -1,   399,    39,   399,    -1,   399,    38,   399,    -1,   399,
      40,   399,    -1,   399,    43,   399,    -1,   399,    41,   399,
      -1,   399,    42,   399,    -1,   399,    29,   399,    30,   399,
      -1,   399,    29,    30,   399,    -1,   226,   147,   215,    -1,
     152,   147,   215,    -1,   226,   147,   123,    -1,   224,    -1,
      78,    -1,   452,    -1,   398,    -1,   206,   447,   206,    -1,
     207,   447,   207,    -1,   143,   447,   144,    -1,   405,   403,
      -1,    -1,     9,    -1,    -1,     9,    -1,    -1,   405,     9,
     399,   128,   399,    -1,   405,     9,   399,    -1,   399,   128,
     399,    -1,   399,    -1,    74,    -1,    75,    -1,    76,    -1,
      85,    -1,   143,    84,   144,    -1,   143,   144,    -1,    74,
      -1,    75,    -1,    76,    -1,   215,    -1,   406,    -1,   215,
      -1,    46,   407,    -1,    47,   407,    -1,   130,   198,   409,
     199,    -1,    66,   409,   205,    -1,   172,   198,   412,   199,
      -1,   410,   403,    -1,    -1,   410,     9,   408,   128,   408,
      -1,   410,     9,   408,    -1,   408,   128,   408,    -1,   408,
      -1,   411,     9,   408,    -1,   408,    -1,   413,   403,    -1,
      -1,   413,     9,   349,   128,   408,    -1,   349,   128,   408,
      -1,   411,   403,    -1,    -1,   198,   414,   199,    -1,    -1,
     416,     9,   215,   415,    -1,   215,   415,    -1,    -1,   418,
     416,   403,    -1,    45,   417,    44,    -1,   419,    -1,    -1,
     422,    -1,   127,   431,    -1,   127,   215,    -1,   127,   201,
     339,   202,    -1,    66,   434,   205,    -1,   201,   339,   202,
      -1,   427,   423,    -1,   198,   329,   199,   423,    -1,   437,
     423,    -1,   198,   329,   199,   423,    -1,   431,    -1,   391,
      -1,   429,    -1,   430,    -1,   424,    -1,   426,   421,    -1,
     198,   329,   199,   421,    -1,   393,   147,   431,    -1,   428,
     198,   289,   199,    -1,   198,   426,   199,    -1,   391,    -1,
     429,    -1,   430,    -1,   424,    -1,   426,   422,    -1,   198,
     329,   199,   422,    -1,   428,   198,   289,   199,    -1,   198,
     426,   199,    -1,   431,    -1,   424,    -1,   198,   426,   199,
      -1,   426,   127,   215,   457,   198,   289,   199,    -1,   426,
     127,   431,   198,   289,   199,    -1,   426,   127,   201,   339,
     202,   198,   289,   199,    -1,   198,   329,   199,   127,   215,
     457,   198,   289,   199,    -1,   198,   329,   199,   127,   431,
     198,   289,   199,    -1,   198,   329,   199,   127,   201,   339,
     202,   198,   289,   199,    -1,   393,   147,   215,   457,   198,
     289,   199,    -1,   393,   147,   431,   198,   289,   199,    -1,
     393,   147,   201,   339,   202,   198,   289,   199,    -1,   432,
      -1,   435,   432,    -1,   432,    66,   434,   205,    -1,   432,
     201,   339,   202,    -1,   433,    -1,    79,    -1,   203,   201,
     339,   202,    -1,   339,    -1,    -1,   203,    -1,   435,   203,
      -1,   431,    -1,   425,    -1,   436,   421,    -1,   198,   329,
     199,   421,    -1,   393,   147,   431,    -1,   198,   426,   199,
      -1,    -1,   425,    -1,   436,   422,    -1,   198,   329,   199,
     422,    -1,   198,   426,   199,    -1,   438,     9,    -1,   438,
       9,   426,    -1,   438,     9,   129,   198,   438,   199,    -1,
      -1,   426,    -1,   129,   198,   438,   199,    -1,   440,   403,
      -1,    -1,   440,     9,   339,   128,   339,    -1,   440,     9,
     339,    -1,   339,   128,   339,    -1,   339,    -1,   440,     9,
     339,   128,    35,   426,    -1,   440,     9,    35,   426,    -1,
     339,   128,    35,   426,    -1,    35,   426,    -1,   442,   403,
      -1,    -1,   442,     9,   339,   128,   339,    -1,   442,     9,
     339,    -1,   339,   128,   339,    -1,   339,    -1,   444,   403,
      -1,    -1,   444,     9,   399,   128,   399,    -1,   444,     9,
     399,    -1,   399,   128,   399,    -1,   399,    -1,   446,   403,
      -1,    -1,   446,     9,   339,   128,   339,    -1,   339,   128,
     339,    -1,   447,   448,    -1,   447,    84,    -1,   448,    -1,
      84,   448,    -1,    79,    -1,    79,    66,   449,   205,    -1,
      79,   127,   215,    -1,   145,   339,   202,    -1,   145,    78,
      66,   339,   205,   202,    -1,   146,   426,   202,    -1,   215,
      -1,    80,    -1,    79,    -1,   120,   198,   451,   199,    -1,
     121,   198,   426,   199,    -1,   121,   198,   340,   199,    -1,
     121,   198,   329,   199,    -1,     7,   339,    -1,     6,   339,
      -1,     5,   198,   339,   199,    -1,     4,   339,    -1,     3,
     339,    -1,   426,    -1,   451,     9,   426,    -1,   393,   147,
     215,    -1,   393,   147,   123,    -1,    -1,    96,   467,    -1,
     175,   456,    14,   467,   200,    -1,   177,   456,   453,    14,
     467,   200,    -1,   215,    -1,   467,   215,    -1,   215,    -1,
     215,   168,   462,   169,    -1,   168,   459,   169,    -1,    -1,
     467,    -1,   458,     9,   467,    -1,   458,   403,    -1,   458,
       9,   161,    -1,   459,    -1,   161,    -1,    -1,    -1,    30,
     467,    -1,   462,     9,   215,    -1,   215,    -1,   462,     9,
     215,    96,   467,    -1,   215,    96,   467,    -1,    85,   128,
     467,    -1,   226,   147,   215,   128,   467,    -1,   464,     9,
     463,    -1,   463,    -1,   464,   403,    -1,    -1,   172,   198,
     465,   199,    -1,    29,   467,    -1,    55,   467,    -1,   226,
      -1,   130,    -1,   131,    -1,   466,    -1,   130,   168,   467,
     169,    -1,   130,   168,   467,     9,   467,   169,    -1,   152,
      -1,   198,   104,   198,   460,   199,    30,   467,   199,    -1,
     198,   467,     9,   458,   403,   199,    -1,   467,    -1,    -1
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
    1952,  1956,  1963,  1965,  1970,  1971,  1972,  1974,  1978,  1982,
    1986,  1990,  1992,  1994,  1996,  2001,  2002,  2007,  2008,  2009,
    2010,  2011,  2012,  2016,  2020,  2024,  2028,  2033,  2038,  2042,
    2043,  2047,  2048,  2052,  2053,  2057,  2058,  2062,  2066,  2070,
    2074,  2075,  2076,  2077,  2081,  2087,  2096,  2109,  2110,  2113,
    2116,  2119,  2120,  2123,  2127,  2130,  2133,  2140,  2141,  2145,
    2146,  2148,  2152,  2153,  2154,  2155,  2156,  2157,  2158,  2159,
    2160,  2161,  2162,  2163,  2164,  2165,  2166,  2167,  2168,  2169,
    2170,  2171,  2172,  2173,  2174,  2175,  2176,  2177,  2178,  2179,
    2180,  2181,  2182,  2183,  2184,  2185,  2186,  2187,  2188,  2189,
    2190,  2191,  2192,  2193,  2194,  2195,  2196,  2197,  2198,  2199,
    2200,  2201,  2202,  2203,  2204,  2205,  2206,  2207,  2208,  2209,
    2210,  2211,  2212,  2213,  2214,  2215,  2216,  2217,  2218,  2219,
    2220,  2221,  2222,  2223,  2224,  2225,  2226,  2227,  2228,  2229,
    2230,  2231,  2235,  2240,  2241,  2244,  2245,  2246,  2250,  2251,
    2252,  2256,  2257,  2258,  2262,  2263,  2264,  2267,  2269,  2273,
    2274,  2275,  2276,  2278,  2279,  2280,  2281,  2282,  2283,  2284,
    2285,  2286,  2287,  2290,  2295,  2296,  2297,  2299,  2300,  2302,
    2303,  2305,  2306,  2308,  2310,  2312,  2314,  2316,  2317,  2318,
    2319,  2320,  2321,  2322,  2323,  2324,  2325,  2326,  2327,  2328,
    2329,  2330,  2331,  2332,  2334,  2336,  2338,  2340,  2341,  2344,
    2345,  2349,  2351,  2355,  2358,  2361,  2367,  2368,  2369,  2370,
    2371,  2372,  2373,  2378,  2380,  2384,  2385,  2388,  2389,  2393,
    2396,  2398,  2400,  2404,  2405,  2406,  2407,  2409,  2412,  2416,
    2417,  2418,  2419,  2422,  2423,  2424,  2425,  2426,  2428,  2429,
    2434,  2436,  2439,  2442,  2444,  2446,  2449,  2451,  2455,  2457,
    2460,  2463,  2469,  2471,  2474,  2475,  2480,  2483,  2487,  2487,
    2492,  2495,  2496,  2500,  2501,  2506,  2507,  2511,  2512,  2516,
    2517,  2522,  2524,  2529,  2530,  2531,  2532,  2533,  2534,  2535,
    2537,  2540,  2542,  2546,  2547,  2548,  2549,  2550,  2552,  2554,
    2556,  2560,  2561,  2562,  2566,  2569,  2572,  2575,  2579,  2583,
    2590,  2594,  2598,  2605,  2606,  2611,  2613,  2614,  2617,  2618,
    2621,  2622,  2626,  2627,  2631,  2632,  2633,  2634,  2636,  2639,
    2642,  2643,  2644,  2646,  2648,  2652,  2653,  2654,  2656,  2657,
    2658,  2662,  2664,  2667,  2669,  2670,  2671,  2672,  2675,  2677,
    2678,  2682,  2684,  2687,  2689,  2690,  2691,  2695,  2697,  2700,
    2703,  2705,  2707,  2711,  2713,  2716,  2718,  2722,  2723,  2725,
    2726,  2732,  2733,  2735,  2737,  2739,  2741,  2744,  2745,  2746,
    2750,  2751,  2752,  2753,  2754,  2755,  2756,  2757,  2758,  2762,
    2763,  2767,  2769,  2777,  2779,  2783,  2787,  2794,  2795,  2801,
    2802,  2809,  2812,  2816,  2819,  2824,  2829,  2831,  2832,  2833,
    2837,  2838,  2842,  2844,  2845,  2847,  2851,  2854,  2863,  2865,
    2869,  2872,  2875,  2883,  2886,  2889,  2890,  2893,  2896,  2897,
    2900,  2904,  2908,  2914,  2924,  2925
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
     357,   358,   359,   359,   360,   360,   360,   360,   361,   362,
     363,   364,   364,   364,   364,   365,   365,   366,   366,   366,
     366,   366,   366,   367,   368,   369,   370,   371,   372,   373,
     373,   374,   374,   375,   375,   376,   376,   377,   378,   379,
     380,   380,   380,   380,   381,   382,   382,   383,   383,   384,
     384,   385,   385,   386,   387,   387,   388,   388,   388,   389,
     389,   389,   390,   390,   390,   390,   390,   390,   390,   390,
     390,   390,   390,   390,   390,   390,   390,   390,   390,   390,
     390,   390,   390,   390,   390,   390,   390,   390,   390,   390,
     390,   390,   390,   390,   390,   390,   390,   390,   390,   390,
     390,   390,   390,   390,   390,   390,   390,   390,   390,   390,
     390,   390,   390,   390,   390,   390,   390,   390,   390,   390,
     390,   390,   390,   390,   390,   390,   390,   390,   390,   390,
     390,   390,   390,   390,   390,   390,   390,   390,   390,   390,
     390,   390,   391,   392,   392,   393,   393,   393,   394,   394,
     394,   395,   395,   395,   396,   396,   396,   397,   397,   398,
     398,   398,   398,   398,   398,   398,   398,   398,   398,   398,
     398,   398,   398,   398,   399,   399,   399,   399,   399,   399,
     399,   399,   399,   399,   399,   399,   399,   399,   399,   399,
     399,   399,   399,   399,   399,   399,   399,   399,   399,   399,
     399,   399,   399,   399,   399,   399,   399,   399,   399,   399,
     399,   399,   399,   400,   400,   400,   401,   401,   401,   401,
     401,   401,   401,   402,   402,   403,   403,   404,   404,   405,
     405,   405,   405,   406,   406,   406,   406,   406,   406,   407,
     407,   407,   407,   408,   408,   408,   408,   408,   408,   408,
     409,   409,   410,   410,   410,   410,   411,   411,   412,   412,
     413,   413,   414,   414,   415,   415,   416,   416,   418,   417,
     419,   420,   420,   421,   421,   422,   422,   423,   423,   424,
     424,   425,   425,   426,   426,   426,   426,   426,   426,   426,
     426,   426,   426,   427,   427,   427,   427,   427,   427,   427,
     427,   428,   428,   428,   429,   429,   429,   429,   429,   429,
     430,   430,   430,   431,   431,   432,   432,   432,   433,   433,
     434,   434,   435,   435,   436,   436,   436,   436,   436,   436,
     437,   437,   437,   437,   437,   438,   438,   438,   438,   438,
     438,   439,   439,   440,   440,   440,   440,   440,   440,   440,
     440,   441,   441,   442,   442,   442,   442,   443,   443,   444,
     444,   444,   444,   445,   445,   446,   446,   447,   447,   447,
     447,   448,   448,   448,   448,   448,   448,   449,   449,   449,
     450,   450,   450,   450,   450,   450,   450,   450,   450,   451,
     451,   452,   452,   453,   453,   454,   454,   455,   455,   456,
     456,   457,   457,   458,   458,   459,   460,   460,   460,   460,
     461,   461,   462,   462,   462,   462,   463,   463,   464,   464,
     465,   465,   466,   467,   467,   467,   467,   467,   467,   467,
     467,   467,   467,   467,   468,   468
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
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     5,     4,     3,     3,     3,     1,     1,     1,     1,
       3,     3,     3,     2,     0,     1,     0,     1,     0,     5,
       3,     3,     1,     1,     1,     1,     1,     3,     2,     1,
       1,     1,     1,     1,     1,     2,     2,     4,     3,     4,
       2,     0,     5,     3,     3,     1,     3,     1,     2,     0,
       5,     3,     2,     0,     3,     0,     4,     2,     0,     3,
       3,     1,     0,     1,     2,     2,     4,     3,     3,     2,
       4,     2,     4,     1,     1,     1,     1,     1,     2,     4,
       3,     4,     3,     1,     1,     1,     1,     2,     4,     4,
       3,     1,     1,     3,     7,     6,     8,     9,     8,    10,
       7,     6,     8,     1,     2,     4,     4,     1,     1,     4,
       1,     0,     1,     2,     1,     1,     2,     4,     3,     3,
       0,     1,     2,     4,     3,     2,     3,     6,     0,     1,
       4,     2,     0,     5,     3,     3,     1,     6,     4,     4,
       2,     2,     0,     5,     3,     3,     1,     2,     0,     5,
       3,     3,     1,     2,     0,     5,     3,     2,     2,     1,
       2,     1,     4,     3,     3,     6,     3,     1,     1,     1,
       4,     4,     4,     4,     2,     2,     4,     2,     2,     1,
       3,     3,     3,     0,     2,     5,     6,     1,     2,     1,
       4,     3,     0,     1,     3,     2,     3,     1,     1,     0,
       0,     2,     3,     1,     5,     3,     3,     5,     3,     1,
       2,     0,     4,     2,     2,     1,     1,     1,     1,     4,
       6,     1,     8,     6,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,     0,     0,   728,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   802,     0,
     790,   613,     0,   619,   620,   621,    22,   677,   778,    98,
     622,     0,    80,     0,     0,     0,     0,     0,     0,     0,
       0,   131,     0,     0,     0,     0,     0,     0,   323,   324,
     325,   328,   327,   326,     0,     0,     0,     0,   158,     0,
       0,     0,   626,   628,   629,   623,   624,     0,     0,   630,
     625,     0,   604,    23,    24,    25,    27,    26,     0,   627,
       0,     0,     0,     0,     0,     0,   631,   329,    28,    29,
      31,    30,    32,    33,    34,    35,    36,    37,    38,    39,
      40,   441,     0,    97,    70,   782,   614,     0,     0,     4,
      59,    61,    64,   676,     0,   603,     0,     6,   130,     7,
       9,     8,    10,     0,     0,   321,   360,     0,     0,     0,
       0,     0,     0,     0,   358,   430,   431,   426,   425,   345,
     427,   432,     0,     0,   344,   744,   605,     0,   679,   424,
     320,   747,   359,     0,     0,   745,   746,   743,   773,   777,
       0,   414,   678,    11,   328,   327,   326,     0,     0,    27,
      59,   130,     0,   848,   359,   847,     0,   845,   844,   429,
       0,   351,   355,     0,     0,   398,   399,   400,   401,   423,
     421,   420,   419,   418,   417,   416,   415,   778,   606,     0,
     862,   605,     0,   380,   378,     0,   806,     0,   686,   343,
     609,     0,   862,   608,     0,   618,   785,   784,   610,     0,
       0,   612,   422,     0,     0,     0,     0,   348,     0,    78,
     350,     0,     0,    84,    86,     0,     0,    88,     0,     0,
       0,   886,   887,   891,     0,     0,    59,   885,     0,   888,
       0,     0,    90,     0,     0,     0,     0,   121,     0,     0,
       0,     0,     0,     0,    42,    47,   241,     0,     0,   240,
     160,   159,   246,     0,     0,     0,     0,     0,   859,   146,
     156,   798,   802,   831,     0,   633,     0,     0,     0,   829,
       0,    16,     0,    63,   138,   150,   157,   510,   452,   824,
     824,     0,   853,   732,   360,     0,   358,   359,     0,     0,
     615,     0,   616,     0,     0,     0,   120,     0,     0,    66,
     232,     0,    21,   129,     0,   155,   142,   154,   326,   130,
     322,   111,   112,   113,   114,   115,   117,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   790,     0,   110,   781,   781,   118,   812,     0,
       0,     0,     0,     0,     0,   319,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   379,
     377,     0,   748,   733,   781,     0,   739,   232,   781,     0,
     783,   774,   798,     0,   130,     0,     0,    92,     0,   730,
     725,   686,     0,     0,     0,     0,   810,     0,   457,   685,
     801,     0,     0,    66,     0,   232,   342,     0,   786,   733,
     741,   611,     0,    70,   196,     0,   440,     0,    95,     0,
       0,   349,     0,     0,     0,     0,     0,    87,   109,    89,
     883,   884,     0,   881,     0,     0,     0,   858,     0,   116,
      91,   119,     0,     0,     0,     0,     0,     0,     0,   468,
       0,   475,   477,   478,   479,   480,   481,   482,   473,   495,
     496,    70,     0,   106,   108,     0,     0,    44,    51,     0,
       0,    46,    55,    48,     0,    18,     0,     0,   242,     0,
      93,     0,     0,    94,   849,     0,     0,   360,   358,   359,
       0,     0,   166,     0,   799,     0,     0,     0,     0,   632,
     830,   677,     0,     0,   828,   682,   827,    62,     5,    13,
      14,     0,   164,     0,     0,   445,     0,     0,   686,     0,
       0,   607,   446,     0,     0,   686,     0,     0,     0,     0,
       0,   688,   731,   895,   341,   411,   752,    75,    69,    71,
      72,    73,    74,   320,     0,   428,   680,   681,    60,   686,
       0,   863,     0,     0,     0,   688,   233,     0,   435,   132,
     162,     0,   383,   385,   384,     0,     0,   381,   382,   386,
     388,   387,   403,   402,   405,   404,   406,   408,   409,   407,
     397,   396,   390,   391,   389,   392,   393,   395,   410,   394,
     780,     0,     0,   816,     0,   686,   852,     0,   851,   750,
     773,   148,   140,   152,   144,   130,     0,     0,   353,   356,
     362,   469,   376,   375,   374,   373,   372,   371,   370,   369,
     368,   367,   366,   365,     0,   735,   734,     0,     0,     0,
       0,     0,     0,     0,   846,   352,   723,   727,   685,   729,
       0,     0,   862,     0,   805,     0,   804,     0,   789,   788,
       0,     0,   735,   734,   346,   198,   200,    70,   443,   347,
       0,    70,   180,    79,   350,     0,     0,     0,     0,     0,
     192,   192,    85,     0,     0,     0,   879,   686,     0,   869,
       0,     0,     0,     0,     0,   684,   622,     0,     0,   604,
       0,     0,    64,   635,   603,   640,     0,   634,    68,   639,
       0,     0,   485,     0,     0,   491,   488,   489,   497,     0,
     476,   471,     0,   474,     0,     0,     0,    52,    19,     0,
       0,    56,    20,     0,     0,     0,    41,    49,     0,   239,
     247,   244,     0,     0,   840,   843,   842,   841,    12,   873,
       0,     0,     0,   798,   795,     0,   456,   839,   838,   837,
       0,   833,     0,   834,   836,     0,     5,     0,     0,     0,
     504,   505,   513,   512,     0,     0,   685,   451,   455,     0,
       0,   459,   685,   823,   460,     0,   854,     0,   870,   732,
     219,   894,     0,     0,   749,   733,   740,   779,   685,   865,
     861,   234,   235,   602,   687,   231,     0,   732,     0,     0,
     164,   437,   134,   413,     0,   462,   463,     0,   458,   685,
     811,     0,     0,   232,   166,     0,   164,   162,     0,   790,
     363,     0,     0,   232,   737,   738,   751,   775,   776,     0,
       0,     0,   711,   693,   694,   695,   696,     0,     0,     0,
     704,   703,   717,   686,     0,   725,   809,   808,     0,   787,
     733,   742,   617,     0,   202,     0,     0,    76,     0,     0,
       0,     0,     0,     0,     0,   172,   173,   184,     0,    70,
     182,   103,   192,     0,   192,     0,     0,   889,     0,     0,
     685,   880,   882,   868,   686,   867,     0,   686,   661,   662,
     659,   660,   692,     0,   686,   684,     0,     0,   454,     0,
       0,   818,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   470,     0,
       0,     0,   493,   494,   492,     0,     0,   472,     0,   122,
       0,   125,   107,     0,    43,    53,     0,    45,    57,    50,
     243,     0,   850,    96,     0,     0,   860,   165,   167,   253,
       0,     0,   796,     0,   832,     0,    17,     0,   853,   163,
     253,     0,     0,   448,     0,   851,   826,     0,   855,     0,
       0,     0,   895,     0,   223,   221,     0,   735,   734,   864,
       0,     0,   236,    67,     0,   732,   161,     0,   732,     0,
     412,   815,   814,     0,   232,     0,     0,     0,     0,   164,
     136,   618,   736,   232,     0,     0,   699,   700,   701,   702,
     705,   706,   715,     0,   686,   711,     0,   698,   719,   685,
     722,   724,   726,     0,   803,   736,     0,     0,     0,     0,
     199,   444,    81,     0,   350,   172,   174,   798,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   186,     0,   876,
       0,   878,   685,     0,     0,     0,   637,   685,   683,     0,
     674,     0,   686,     0,   641,   675,   673,   822,     0,   686,
     644,   646,   645,     0,     0,   642,   643,   647,   649,   648,
     664,   663,   666,   665,   667,   669,   670,   668,   657,   656,
     651,   652,   650,   653,   654,   655,   658,   483,     0,   484,
     490,   498,   499,     0,    70,    54,    58,   245,   875,   872,
       0,   320,   800,   798,   354,   357,   361,     0,    15,     0,
     320,   516,     0,     0,   518,   511,   514,     0,   509,     0,
       0,   856,   871,   442,     0,   224,     0,   220,     0,     0,
     232,   238,   237,   870,     0,   253,     0,   732,     0,   232,
       0,   771,   253,   853,   253,     0,     0,   364,   232,     0,
     765,     0,   708,   685,   710,     0,   697,     0,     0,   686,
     716,   807,     0,    70,     0,   195,   181,     0,     0,     0,
     171,    99,   185,     0,     0,   188,     0,   193,   194,    70,
     187,   890,     0,   866,     0,   893,   691,   690,   636,     0,
     685,   453,   638,     0,   461,   685,   817,   672,     0,     0,
       0,     0,     0,   168,     0,     0,     0,   318,     0,     0,
       0,   147,   252,   254,     0,   317,     0,   320,     0,   835,
     249,   151,   507,     0,     0,   447,   825,     0,   227,   218,
       0,   226,   736,   232,     0,   434,   870,   320,   870,     0,
     813,     0,   770,   320,     0,   320,   253,   732,     0,   764,
     714,   713,   707,     0,   709,   685,   718,    70,   201,    77,
      82,   101,   175,     0,   183,   189,    70,   191,   877,     0,
       0,   450,     0,   821,   820,   671,     0,    70,   126,   874,
       0,     0,     0,     0,   169,   284,   282,   286,   604,    27,
       0,   278,     0,   283,   295,     0,   293,   298,     0,   297,
       0,   296,     0,   130,   256,     0,   258,     0,   797,     0,
     508,   506,   517,   515,   228,     0,   217,   225,   232,     0,
     768,     0,     0,     0,   143,   434,   870,   772,   149,   249,
     153,   320,     0,   766,     0,   721,     0,   197,     0,     0,
      70,   178,   100,   190,   892,   689,     0,     0,     0,     0,
       0,     0,     0,     0,   268,   272,     0,     0,   263,   568,
     567,   564,   566,   565,   585,   587,   586,   556,   527,   528,
     546,   562,   561,   523,   533,   534,   536,   535,   555,   539,
     537,   538,   540,   541,   542,   543,   544,   545,   547,   548,
     549,   550,   551,   552,   554,   553,   524,   525,   526,   529,
     530,   532,   570,   571,   580,   579,   578,   577,   576,   575,
     563,   582,   572,   573,   574,   557,   558,   559,   560,   583,
     584,   588,   590,   589,   591,   592,   569,   594,   593,   596,
     598,   597,   531,   601,   599,   600,   595,   581,   522,   290,
     519,     0,   264,   311,   312,   310,   303,     0,   304,   265,
     337,     0,     0,     0,     0,   130,   139,   248,     0,     0,
       0,   230,     0,   767,     0,    70,   313,    70,   133,     0,
       0,     0,   145,   870,   712,     0,    70,   176,    83,   102,
       0,   449,   819,   486,   124,   266,   267,   340,   170,     0,
       0,   287,   279,     0,     0,     0,   292,   294,     0,     0,
     299,   306,   307,   305,     0,     0,   255,     0,     0,     0,
       0,   250,     0,   229,   769,     0,   502,   688,     0,     0,
      70,   135,   141,     0,   720,     0,     0,     0,   104,   269,
      59,     0,   270,   271,     0,     0,   285,   289,   520,   521,
       0,   280,   308,   309,   301,   302,   300,   338,   335,   259,
     257,   339,     0,   251,   503,   687,     0,   436,   314,     0,
     137,     0,   179,   487,     0,   128,     0,   320,   288,   291,
       0,   732,   261,     0,   500,   433,   438,   177,     0,     0,
     105,   276,     0,   319,   336,     0,   688,   331,   732,   501,
       0,   127,     0,     0,   275,   870,   732,   205,   332,   333,
     334,   895,   330,     0,     0,     0,   274,     0,   331,     0,
     870,     0,   273,   315,    70,   260,   895,     0,   209,   207,
       0,    70,     0,     0,   210,     0,   206,   262,     0,   316,
       0,   213,   204,     0,   212,   123,   214,     0,   203,   211,
       0,   216,   215
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   119,   786,   538,   180,   273,   496,
     500,   274,   497,   501,   121,   122,   123,   124,   125,   126,
     318,   568,   569,   450,   238,  1388,   456,  1313,  1389,  1615,
     746,   268,   491,  1578,   972,  1144,  1630,   334,   181,   570,
     828,  1029,  1196,   130,   541,   845,   571,   590,   847,   522,
     844,   572,   542,   846,   336,   289,   305,   133,   830,   789,
     772,   987,  1333,  1079,   895,  1528,  1392,   693,   901,   455,
     702,   903,  1229,   686,   884,   887,  1068,  1635,  1636,   560,
     561,   584,   585,   278,   279,   283,  1359,  1507,  1508,  1151,
    1262,  1352,  1503,  1621,  1638,  1539,  1582,  1583,  1584,  1340,
    1341,  1342,  1540,  1546,  1591,  1345,  1346,  1350,  1496,  1497,
    1498,  1518,  1665,  1263,  1264,   182,   135,  1651,  1652,  1501,
    1266,   136,   231,   451,   452,   137,   138,   139,   140,   141,
     142,   143,   144,  1372,   145,   827,  1028,   146,   235,   313,
     446,   547,   548,  1102,   549,  1103,   147,   148,   149,   150,
     725,   151,   152,   265,   153,   266,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   736,   737,   964,   488,   489,
     490,   743,  1567,   154,   543,  1361,   544,  1001,   794,  1168,
    1165,  1489,  1490,   155,   156,   157,   225,   232,   321,   436,
     158,   922,   729,   159,   923,   819,   810,   924,   871,  1050,
    1052,  1053,  1054,   873,  1208,  1209,   874,   667,   421,   193,
     194,   573,   563,   402,   403,   816,   161,   226,   184,   163,
     164,   165,   166,   167,   168,   169,   621,   170,   228,   229,
     525,   217,   218,   624,   625,  1108,  1109,   554,   555,   298,
     299,   780,   171,   515,   172,   559,   173,  1509,   290,   329,
     579,   580,   916,  1011,   770,   706,   707,   708,   259,   260,
     812
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1175
static const yytype_int16 yypact[] =
{
   -1175,   126, -1175, -1175,  5931, 13188, 13188,   -75, 13188, 13188,
   13188, 11343, 13188, -1175, 13188, 13188, 13188, 13188, 13188, 13188,
   13188, 13188, 13188, 13188, 13188, 13188, 14402, 14402, 11507, 13188,
   14485,   -66,   -57, -1175, -1175, -1175, -1175, -1175,   156, -1175,
      92, 13188, -1175,   -57,   -32,     1,     4,   -57, 11671, 15559,
   11835, -1175,  5020, 10646,   171, 13188, 15288,    18, -1175, -1175,
   -1175,    82,   231,    56,   187,   195,   205,   216, -1175, 15559,
     218,   220, -1175, -1175, -1175, -1175, -1175,   290,  3718, -1175,
   -1175, 15559, -1175, -1175, -1175, -1175, 15559, -1175, 15559, -1175,
     278,   247,   250,   265, 15559, 15559, -1175, -1175, -1175, -1175,
   -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175,
   -1175, -1175, 13188, -1175, -1175,   211,   336,   409,   409, -1175,
     435,   323,   360, -1175,   285, -1175,    32, -1175,   450, -1175,
   -1175, -1175, -1175, 15361,   249, -1175, -1175,   304,   306,   335,
     351,   357,   359,  2426, -1175, -1175, -1175, -1175,   460, -1175,
   -1175,   498,   501,   369, -1175,    80,   384,   447, -1175, -1175,
     594,    -6,  2528,    81,   421,   114,   117,   451,    -5, -1175,
     136, -1175,   585, -1175, -1175, -1175,   511,   461,   512, -1175,
   -1175,   450,   249, 15814,  2662, 15814, 13188, 15814, 15814,  4251,
     466, 14797, 15814,   623, 15559,   603,   603,    83,   603,   603,
     603,   603,   603,   603,   603,   603,   603, -1175, -1175,  4801,
     503, -1175,   522,   545,   545, 14402, 14841,   468,   666, -1175,
     511,  4801,   503,   530,   532,   482,   121, -1175,   555,    81,
   11999, -1175, -1175, 13188,  9211,   675,    60, 15814, 10236, -1175,
   13188, 13188, 15559, -1175, -1175,  3767,   486, -1175,  3817,  5020,
    5020,   519, -1175, -1175,   490, 13740,   676, -1175,   678, -1175,
   15559,   614, -1175,   497,  4185,   499,   634, -1175,    -2,  4297,
   15374, 15425, 15559,    61, -1175,    -7, -1175, 13799,    62, -1175,
   -1175, -1175,   684,    68, 14402, 14402, 13188,   514,   546, -1175,
   -1175, 14266, 11507,    59,   280, -1175, 13352, 14402,   385, -1175,
   15559, -1175,   -44,   323, -1175, -1175, -1175, -1175, 14538, 13188,
   13188,   702,   629,    13,   528, 15814,   535,    96,  6136, 13188,
     326,   527,   481,   326,   264,   261, -1175, 15559,  5020,   541,
   10810,  5020, -1175, -1175,  5257, -1175, -1175, -1175, -1175,   450,
   -1175, -1175, -1175, -1175, -1175, -1175, -1175, 13188, 13188, 13188,
   12204, 13188, 13188, 13188, 13188, 13188, 13188, 13188, 13188, 13188,
   13188, 13188, 13188, 13188, 13188, 13188, 13188, 13188, 13188, 13188,
   13188, 13188, 14485, 13188, -1175, 13188, 13188, -1175, 13188, 14579,
   15559, 15559, 15559, 15361,   631,   338, 10441, 13188, 13188, 13188,
   13188, 13188, 13188, 13188, 13188, 13188, 13188, 13188, 13188, -1175,
   -1175,  2200, -1175,   123, 13188, 13188, -1175, 10810, 13188, 13188,
     211,   127, 14266,   543,   450, 12368,  3910, -1175, 13188, -1175,
     544,   735,  4801,   548,     7,  3998,   545, 12532, -1175, 12696,
   -1175,   549,   214, -1175,   139, 10810, -1175,  5159, -1175,   128,
   -1175, -1175,  4410, -1175, -1175, 12860, -1175, 13188, -1175,   657,
    9416,   740,   553, 15726,   741,    84,    27, -1175, -1175, -1175,
   -1175, -1175,  5020, 14251,   559,   749, 14038, -1175,   575, -1175,
   -1175, -1175,   682, 13188,   685,   686, 13188, 13188, 13188, -1175,
     634, -1175, -1175, -1175, -1175, -1175, -1175, -1175,   580, -1175,
   -1175, -1175,   568, -1175, -1175, 15559,   570,   759,   229, 15559,
     571,   764,   260,   288, 15440, -1175, 15559, 13188,   545,    18,
   -1175, 14038,   696, -1175,   545,    86,    89,   577,   578,   153,
     583, 15559,   654,   582,   545,    90,   586, 15307, 15559, -1175,
   -1175,   720,  1912,   -40, -1175, -1175, -1175,   323, -1175, -1175,
   -1175,   757,   662,   624,   164, -1175,   211,   664,   784,   605,
     658,   127, -1175, 14897,   608,   801,   612,  5020,  5020,   799,
     622,   810, -1175,  5020,    10,   755,     6, -1175, -1175, -1175,
   -1175, -1175, -1175,   476,  2252, -1175, -1175, -1175, -1175,   819,
     661, -1175, 14402, 13188,   632,   823, 15814,   824, -1175, -1175,
     708, 14100, 15941, 16024,  4251, 13188, 15770, 16133,  3010,  5016,
   16198, 12182, 13717, 13717, 13717, 13717,  1059,  1059,  1059,  1059,
     673,   673,   579,   579,   579,    83,    83,    83, -1175,   603,
   15814,   635,   637, 14941,   641,   830, -1175, 13188,   -59,   647,
     127, -1175, -1175, -1175, -1175,   450, 13188, 14183, -1175, -1175,
    4251, -1175,  4251,  4251,  4251,  4251,  4251,  4251,  4251,  4251,
    4251,  4251,  4251,  4251, 13188,   -59,   665,   650,  2473,   660,
     655,  2819,    91,   670, -1175, 15814, 14167, -1175, 15559, -1175,
     528,    10,   503, 14402, 15814, 14402, 14997,   125,   131, -1175,
     663, 13188, -1175, -1175, -1175,  9006,   370, -1175, 15814, 15814,
     -57, -1175, -1175, -1175, 13188,   768,  2945, 14038, 15559,  9621,
     669,   672, -1175,    55,   742,   726, -1175,   866,   677,  3209,
    5020, 14038, 14038, 14038, 14038, 14038, -1175,   680,    40,   732,
     683, 14038,   342, -1175,   733, -1175,   689, -1175, 15900, -1175,
   13188,   699, 15814,   701,   877,  5076,   883, -1175, 15814,  4802,
   -1175,   580,   814, -1175,  6341, 15224,   693,   292, -1175, 15374,
   15559,   293, -1175, 15425, 15559, 15559, -1175, -1175,  2863, -1175,
   15900,   881, 14402,   698, -1175, -1175, -1175, -1175, -1175,   800,
      57, 15224,   700, 14266, 14349,   886, -1175, -1175, -1175, -1175,
     705, -1175, 13188, -1175, -1175,  5502, -1175,  5020, 15224,   706,
   -1175, -1175, -1175, -1175,   889, 13188, 14538, -1175, -1175, 15497,
   13188, -1175, 13188, -1175, -1175,   711, -1175,  5020,   882,    23,
   -1175, -1175,    66, 13951, -1175,   132, -1175, -1175,  5020, -1175,
   -1175,   545, 15814, -1175, 10974, -1175, 14038,   105,   715, 15224,
     662, -1175, -1175, 13555, 13188, -1175, -1175, 13188, -1175, 13188,
   -1175,  2955,   719, 10810,   654,   890,   662,   708, 15559, 14485,
     545,  3172,   724, 10810, -1175, -1175,   134, -1175, -1175,   910,
    4786,  4786, 14167, -1175, -1175, -1175, -1175,   727,   129,   729,
   -1175, -1175, -1175,   920,   731,   544,   545,   545, 13024, -1175,
     135, -1175, -1175,  3402,   405,   -57, 10236, -1175,  6546,   734,
    6751,   736,  2945, 14402,   737,   803,   545, 15900,   918, -1175,
   -1175, -1175, -1175,   415, -1175,   238,  5020, -1175,  5020, 15559,
   14251, -1175, -1175, -1175,   924, -1175,   738,   819,   590,   590,
     876,   876, 15141,   739,   929, 14038,   805, 15559, 14538,  1782,
   15512, 14038, 14038, 14038, 14038, 13889, 14038, 14038, 14038, 14038,
   14038, 14038, 14038, 14038, 14038, 14038, 14038, 14038, 14038, 14038,
   14038, 14038, 14038, 14038, 14038, 14038, 14038, 14038, 15814, 13188,
   13188, 13188, -1175, -1175, -1175, 13188, 13188, -1175,   634, -1175,
     864, -1175, -1175, 15559, -1175, -1175, 15559, -1175, -1175, -1175,
   -1175, 14038,   545, -1175,  5020, 15559, -1175,   937, -1175, -1175,
      93,   752,   545, 11179, -1175,  1705, -1175,  5726,   629,   937,
   -1175,   442,   -13, 15814,   834, -1175, 15814, 15041, -1175,   747,
    5020,   675,  5020,   872,   938,   884, 13188,   -59,   766, -1175,
   14402, 13188, 15814, 15900,   767,   105, -1175,   769,   105,   770,
   13555, 15814, 15097,   773, 10810,   774,   775,  5020,   776,   662,
   -1175,   482,   777, 10810,   780, 13188, -1175, -1175, -1175, -1175,
   -1175, -1175,   837,   762,   965, 14167,   836, -1175, 14538, 14167,
   -1175, -1175, -1175, 14402, 15814, -1175,   -57,   954,   922, 10236,
   -1175, -1175, -1175,   804, 13188,   803,   545, 14266,  2945,   806,
   14038,  6956,   422,   807, 13188,    64,   258, -1175,   832, -1175,
     871, -1175,  4523,   973,   809, 14038, -1175, 14038, -1175,   815,
   -1175,   878,  1006,   817, -1175, -1175, -1175, 15197,   816,  1008,
   15984, 16064,  3210, 14038, 15858, 16167,  4696,  5676, 14230, 14532,
   16228, 16228, 16228, 16228,  1436,  1436,  1436,  1436,   802,   802,
     590,   590,   590,   876,   876,   876,   876, 15814,  5425, 15814,
   -1175, 15814, -1175,   820, -1175, -1175, -1175, 15900, -1175,   928,
   15224,   382, -1175, 14266, -1175, -1175,  4251,   818, -1175,   828,
     458, -1175,   160, 13188, -1175, -1175, -1175, 13188, -1175, 13188,
   13188, -1175, -1175, -1175,   150,  1017, 14038, -1175,  3478,   835,
   10810,   545, 15814,   882,   833, -1175,   838,   105, 13188, 10810,
     839, -1175, -1175,   629, -1175,   841,   842, -1175, 10810,   840,
   -1175, 14167, -1175, 14167, -1175,   844, -1175,   906,   845,  1026,
   -1175,   545,  1018, -1175,   847, -1175, -1175,   851,   853,    95,
   -1175, -1175, 15900,   854,   857, -1175,  3671, -1175, -1175, -1175,
   -1175, -1175,  5020, -1175,  5020, -1175, 15900, 15239, -1175, 14038,
   14538, -1175, -1175, 14038, -1175, 14038, -1175, 16099, 14038, 13188,
     858,  7161,  5020, -1175,   365,  5020, 15224, -1175, 15243,   904,
   14629, -1175, -1175, -1175,   631, 13675,    69,   338,    99, -1175,
   -1175, -1175,   911,  3526,  3624, 15814, 15814,   983,  1050,   987,
   14038, 15900,   869, 10810,   870,   958,   882,   843,   882,   874,
   15814,   875, -1175,   873,   885,  1037, -1175,   105,   879, -1175,
   -1175,   942, -1175, 14167, -1175, 14538, -1175, -1175,  9006, -1175,
   -1175, -1175, -1175,  9826, -1175, -1175, -1175,  9006, -1175,   888,
   14038, 15900,   943, 15900, 15295, 16099,  5345, -1175, -1175, -1175,
   15224, 15224,  1058,    58, -1175, -1175, -1175, -1175,    71,   887,
      72, -1175, 13557, -1175, -1175,    74, -1175, -1175,  3822, -1175,
     891, -1175,  1004,   450, -1175,  5020, -1175,   631, -1175,  2063,
   -1175, -1175, -1175, -1175,  1070, 14038, -1175, 15900, 10810,   893,
   -1175,   892,   894,   341, -1175,   958,   882, -1175, -1175, -1175,
   -1175,  1115,   897, -1175, 14167, -1175,   985,  9006, 10031,  9826,
   -1175, -1175, -1175,  9006, -1175, 15900, 14038, 14038, 13188,  7366,
     914,   915, 14038, 15224, -1175, -1175,   471, 15243, -1175, -1175,
   -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175,
   -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175,
   -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175,
   -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175,
   -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175,
   -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175,
   -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175,
   -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175,   502,
   -1175,   904, -1175, -1175, -1175, -1175, -1175,    42,   373, -1175,
    1102,    76, 15559,  1004,  1106,   450, -1175, -1175,   923,  1108,
   14038, 15900,   926, -1175,   103, -1175, -1175, -1175, -1175,   925,
     341,  4353, -1175,   882, -1175, 14167, -1175, -1175, -1175, -1175,
    7571, 15900, 15900,  5239, -1175, -1175, -1175, 15900, -1175, 13973,
      44, -1175, -1175, 14038, 13557, 13557,  1073, -1175,  3822,  3822,
     537, -1175, -1175, -1175, 14038,  1051, -1175,   933,    77, 14038,
   15559, -1175, 14038, 15900, -1175,  1053, -1175,  1125,  7776,  7981,
   -1175, -1175, -1175,   341, -1175,  8186,   935,  1057,  1031, -1175,
    1044,   994, -1175, -1175,  1048,   471, -1175, 15900, -1175, -1175,
     986, -1175,  1123, -1175, -1175, -1175, -1175, 15900,  1144, -1175,
   -1175, 15900,   961, 15900, -1175,   316,   962, -1175, -1175,  8391,
   -1175,   963, -1175, -1175,   964,   999, 15559,   338, -1175, -1175,
   14038,   106, -1175,  1086, -1175, -1175, -1175, -1175, 15224,   693,
   -1175,  1005, 15559,   465, 15900,   967,  1159,   504,   106, -1175,
    1092, -1175, 15224,   970, -1175,   882,   109, -1175, -1175, -1175,
   -1175,  5020, -1175,   974,   975,    79, -1175,   396,   504,   230,
     882,   971, -1175, -1175, -1175, -1175,  5020,  1096,  1162,  1098,
     396, -1175,  8596,   271,  1164, 14038, -1175, -1175,  8801, -1175,
    1101,  1167,  1104, 14038, 15900, -1175,  1171, 14038, -1175, 15900,
   14038, 15900, 15900
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1175, -1175, -1175,  -499, -1175, -1175, -1175,    -4, -1175, -1175,
   -1175,   691,   437,   434,    49,  1091,  3683, -1175,  2374, -1175,
    -388, -1175,    29, -1175, -1175, -1175, -1175, -1175, -1175, -1175,
   -1175, -1175, -1175, -1175,  -441, -1175, -1175,  -174,    -1,    12,
   -1175, -1175, -1175, -1175, -1175, -1175,    14, -1175, -1175, -1175,
   -1175,    15, -1175, -1175,   813,   821,   808,  -124,   346,  -784,
     345,   410,  -442,   130,  -829, -1175,  -187, -1175, -1175, -1175,
   -1175,  -657,   -22, -1175, -1175, -1175, -1175,  -432, -1175,  -789,
   -1175,  -386, -1175, -1175,   704, -1175,  -172, -1175, -1175,  -941,
   -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175,
    -198, -1175, -1175, -1175, -1175, -1175,  -281, -1175,   -49,  -947,
   -1175, -1174,  -456, -1175,  -156,    52,  -128,  -443, -1175,  -287,
   -1175,   -78,   -20,  1177,  -670,  -358, -1175, -1175,   -39, -1175,
   -1175,  3732,   -16,  -154, -1175, -1175, -1175, -1175, -1175, -1175,
     208,  -767, -1175, -1175, -1175, -1175, -1175, -1175, -1175, -1175,
   -1175, -1175, -1175,   848, -1175, -1175,   254, -1175,   743, -1175,
   -1175, -1175, -1175, -1175, -1175, -1175,   259, -1175,   746, -1175,
   -1175,   494, -1175,   235, -1175, -1175, -1175, -1175, -1175, -1175,
   -1175, -1175,  -935, -1175,  1866,  1423,  -340, -1175, -1175,   196,
    3322,  4087, -1175, -1175,   313,  -188,  -575, -1175, -1175,   379,
    -651,   186, -1175, -1175, -1175, -1175, -1175,   367, -1175, -1175,
   -1175,    -3,  -792,  -193,  -191,  -137, -1175, -1175,    16, -1175,
   -1175, -1175, -1175,    22,  -159, -1175,    -8, -1175, -1175, -1175,
    -381,   951, -1175, -1175, -1175, -1175, -1175,   934, -1175,   506,
     380, -1175, -1175,   960,  -295,  -958, -1175,   -47,   -86,  -197,
     -63,   539, -1175, -1133, -1175,   339, -1175, -1175, -1175,  -176,
    -990
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -863
static const yytype_int16 yytable[] =
{
     120,   160,   306,   128,   384,   258,   340,   413,   311,   312,
     825,   411,   234,   552,   263,   872,   129,  1012,   131,   132,
     162,   659,  1174,   239,   891,   433,   406,   243,   638,  1004,
     430,   662,   618,   127,   314,   438,   383,   439,  1024,   785,
    1159,   331,   213,   214,   905,   246,  1027,   337,   256,   680,
    1285,  1548,   227,  1585,   340,   685,   134,   700,    13,  1160,
    -756,   408,  1038,  1075,   906,   288,   985,  1403,    13,   447,
     504,   509,  -760,   460,   461,  1549,   404,   512,  1355,   465,
    -281,  1407,   304,  1491,   288,  1555,  1555,   401,  1403,   506,
     288,   288,   440,   698,  1227,   762,   316,   276,   762,   774,
     774,  1013,   774,   744,   774,   275,   492,   327,   774,   328,
     415,   387,   388,   389,   390,   391,   392,   393,   394,   395,
     396,   397,   398,   186,   926,   527,     3,   302,   317,   288,
     303,   423,   230,   339,   401,   282,   372,   813,  1565,  -862,
    1166,   233,  -607,   431,   327,  1014,  -753,   404,   373,   551,
      13,    13,   581,  1373,    13,  1375,   539,   540,  -466,   399,
     400,  1101,   784,   493,  -439,   591,   240,   415,   387,   388,
     389,   390,   391,   392,   393,   394,   395,   396,   397,   398,
    -754,   414,  1566,  -755,   295,  1277,   528,  -791,  1167,  -757,
     420,   404,  -762,   408,  -792,  -756,   409,  -794,  -758,   241,
    -759,  -793,   242,  -606,  -763,   280,   566,  -760,   517,  -222,
    1272,   405,   385,  1056,   791,   207,   399,   400,   207,  -687,
     630,   277,  -687,   401,   907,   424,   986,  1015,   701,  1278,
     120,   426,   332,   669,   120,  1294,  1184,   432,   454,  1186,
     663,  1550,   630,  1520,  1287,  1082,  1586,  1086,   589,  1220,
     162,  1293,   437,  1295,   162,  1195,   467,   340,  1404,  1405,
     448,   505,   510,   444,  1228,  1667,   630,   449,   513,  1356,
     518,  -281,  1408,  1057,  1492,   630,  1556,  1600,   630,  1662,
     401,  -753,   405,   699,   587,   763,   703,   997,   764,   775,
     859,  1207,  1152,   508,  1312,   566,   306,   337,  1358,   888,
     514,   514,   519,   890,  -222,  -208,  1680,   524,  -687,  1668,
     562,  1279,   307,   533,   120,  -754,   792,   128,  -755,   498,
     502,   503,  -791,   578,  -757,   750,   405,   256,   409,  -792,
     288,   793,  -794,  -758,   162,  -759,  -793,  1084,  1085,   410,
     293,   401,   115,   293,   670,   534,  1571,   639,   534,   537,
    1681,  1623,   767,    51,   281,  1381,   754,  1084,  1085,   293,
     797,    58,    59,    60,   174,   175,   338,   803,   622,   293,
     134,   814,   267,   815,   294,   628,   288,   632,   288,   288,
     327,   805,   806,   635,   755,   284,  1254,   811,   973,   976,
    1573,  1669,   990,   285,   227,  1624,   657,   655,  1289,  1610,
     660,   629,  1551,   286,  1217,   293,   296,   297,  1210,   296,
     297,   327,   319,   678,   287,   293,   291,   384,   292,  1552,
     320,   672,  1553,   656,   529,   296,   297,    13,   524,    97,
     307,   842,  1682,   682,   295,   296,   297,   840,   424,   327,
    1087,   885,   886,   327,   327,   308,   120,   629,   309,   383,
      58,    59,    60,   174,   175,   338,   679,  1035,   852,   683,
    1230,   848,  1254,   310,   293,   326,   162,  1044,   577,   534,
     576,   296,   297,  1322,   327,   842,  1066,  1067,   814,   692,
     815,   296,   297,   330,   879,   333,   880,  1255,   293,  -862,
    1330,  1331,  1256,   323,    58,    59,    60,   174,   175,   338,
    1257,   552,   757,    13,   341,   832,   342,  -862,  1382,  1041,
     328,  1081,  1657,  1083,  1084,  1085,  1543,   769,    97,   911,
    1224,  1084,  1085,   779,   781,   433,  -464,  1670,   328,   535,
     296,   297,  1544,   581,   581,   343,  1258,  1259,  1386,  1260,
     881,  1516,  1517,  -862,   747,    33,    34,    35,   751,  1545,
    1300,   344,  1301,   275,   296,   297,   716,   345,  -862,   346,
     293,  -862,    97,  1255,   375,   534,  1594,   376,  1256,   377,
      58,    59,    60,   174,   175,   338,  1257,    58,    59,    60,
     174,   175,   338,  1595,  1261,   378,  1596,   288,    58,    59,
      60,    61,    62,   338,   379,  1161,  1663,  1664,   821,    68,
     380,  1592,  1593,    72,    73,    74,    75,    76,  1162,  1588,
    1589,   998,  1258,  1259,   718,  1260,  1648,  1649,  1650,   407,
      79,    80,   322,   324,   325,   385,   296,   297,   369,   370,
     371,  1009,   372,   552,    89,  1154,   382,   551,    97,   954,
     955,   956,  1019,  1163,   373,    97,   914,   917,  1190,  -761,
      96,  -465,  1385,   850,   630,   957,    97,  1199,  -606,   412,
    1271,  1659,   870,   300,   875,  1644,   417,   419,   373,   425,
     889,   328,   401,   428,   530,   429,  1673,  -605,   536,   434,
     435,   120,   437,   445,   128,  1060,   458,   462,   463,   876,
    -857,   877,   466,   468,   898,   120,  1219,   469,   511,   471,
     530,   162,   536,   530,   536,   536,    58,    59,    60,    61,
      62,   338,   896,   520,   521,   162,   557,    68,   380,   366,
     367,   368,   369,   370,   371,   558,   372,   564,   900,  1094,
    1088,   575,  1089,  1524,   565,    51,  1098,   134,   373,   -65,
     120,   588,   666,   128,   668,   690,   975,   671,   677,   447,
     978,   979,   381,   694,   382,   697,  1251,   709,   710,   730,
     162,   731,  1040,   552,   733,   734,   745,   742,   749,   551,
     748,   752,  1268,   753,    97,   761,   765,   766,   982,   771,
     773,   120,   160,   768,   128,   776,   782,   787,   788,   524,
     992,   790,   795,   796,  1284,  1005,   134,   129,   498,   131,
     132,   162,   502,  1291,   798,   799,   562,   801,  1148,  1017,
     802,   804,  1298,   807,   127,   472,   473,   474,   808,   809,
    1179,  -467,   475,   476,   562,  1308,   477,   478,   818,  1637,
     820,   823,   824,   829,  1172,  1018,   811,   134,   826,   839,
     835,  1317,   836,   838,   288,   843,  1637,  1254,   951,   952,
     953,   954,   955,   956,  1658,   854,  1049,  1049,   870,   856,
     857,  1193,   882,   853,   892,  1069,  1204,   957,   831,   902,
     908,   227,   904,   909,  1574,   910,   912,  1254,   925,   927,
     930,   928,   120,   959,   120,   960,   120,   128,    13,   128,
     931,   961,   965,   968,   971,   981,   984,  1369,   983,   551,
     993,   989,   162,  1002,   162,  1090,   162,  1000,   896,  1076,
     994,  1008,  1010,  1025,  1241,  1070,  1019,  1034,    13,  1387,
    1037,  1246,  1043,  1100,  1045,  1055,  1106,  1058,  1393,  1059,
    1061,  1078,  1080,  1092,  1072,  1077,  1074,  1093,  1097,  1399,
     134,   957,   134,  1143,  1096,   552,  1150,  1171,  1255,   529,
    1153,  1175,  1176,  1256,  1155,    58,    59,    60,   174,   175,
     338,  1257,  1169,  1177,  1180,  1201,  1183,  1202,  1187,  1145,
    1185,  1189,  1146,  1191,  1203,  1198,  1192,  1194,  1255,  1200,
    1206,  1149,  1512,  1256,  1213,    58,    59,    60,   174,   175,
     338,  1257,  1606,   120,   160,  1214,   128,  1258,  1259,  1232,
    1260,  1231,  1530,  1234,  1216,  1221,  1239,  1225,  1235,   129,
     552,   131,   132,   162,  1238,  1240,  1242,  1245,  1244,  1250,
    1269,  1306,   562,    97,  1252,   562,   127,  1258,  1259,  1270,
    1260,  1280,  1286,  1283,  1303,  1305,  1181,  1288,  1292,  1299,
    1297,  1254,  1296,  1302,  1304,  1374,  1212,  1309,  1307,   134,
    1310,   870,  1311,    97,  1314,   870,  1318,  1315,  1319,  1327,
    1344,  1647,  1364,  1360,  1365,   120,  1366,  1368,  1371,  1370,
    1384,  1396,  1402,  1376,  1377,  1378,  1329,   120,  1383,  1211,
     128,   551,    13,  1500,  1510,   162,  1379,  1394,  1406,  1354,
    1514,  1499,  1513,   524,   896,  1515,  1523,   162,  1215,  -863,
    -863,  -863,  -863,   364,   365,   366,   367,   368,   369,   370,
     371,  1357,   372,  1525,  1535,  1536,  1554,   210,   210,  1254,
    1559,   222,  1562,  1561,   373,  1564,  1570,  1568,  1590,  1569,
    1598,  1599,  1604,   134,  1605,  1612,  1613,   340,  1575,  1614,
    -277,  1616,  1255,   222,  1617,  1619,   551,  1256,  1267,    58,
      59,    60,   174,   175,   338,  1257,  1549,  1267,  1620,  1622,
      13,  1625,  1628,  1627,  1629,  1639,  1645,  1642,  1646,   524,
    1656,  1654,  1671,  1660,  1661,  1674,  1675,  1676,  1683,  1502,
    1686,  1687,  1609,  1688,   562,  1690,   974,   977,  1641,  1036,
     633,  1258,  1259,  1039,  1260,   756,   634,   870,   999,   870,
    1655,   631,  1529,  1265,  1316,  1218,  1653,  1521,  1332,  1542,
    1547,  1351,  1265,   759,  1677,  1666,  1558,    97,   236,  1173,
    1255,  1519,  1142,   740,  1140,  1256,   741,    58,    59,    60,
     174,   175,   338,  1257,   641,   967,  1164,  1197,  1099,  1380,
    1051,  1205,  1062,   526,   556,   516,     0,   120,   915,  1091,
     128,   256,     0,     0,     0,     0,  1349,     0,     0,     0,
       0,     0,     0,  1353,     0,     0,     0,   162,     0,  1258,
    1259,     0,  1260,     0,     0,     0,  1672,     0,     0,     0,
       0,     0,     0,  1678,  1267,     0,     0,     0,     0,     0,
    1267,     0,  1267,     0,   562,    97,     0,     0,     0,   870,
     210,     0,     0,   134,   120,     0,   210,   128,  1504,   120,
       0,     0,   210,   120,     0,     0,   128,  1522,     0,   385,
       0,     0,     0,     0,   162,     0,     0,     0,     0,   162,
       0,  1560,     0,   162,     0,     0,     0,     0,  1488,  1265,
     222,   222,  1391,     0,  1495,  1265,   222,  1265,     0,     0,
       0,   256,     0,     0,     0,   256,  1505,     0,     0,     0,
     134,     0,     0,     0,     0,     0,     0,     0,   210,   134,
       0,     0,     0,     0,     0,   210,   210,     0,  1267,     0,
     870,     0,   210,   120,   120,   120,   128,     0,   210,   120,
       0,     0,   128,     0,     0,   120,     0,     0,   128,   222,
       0,     0,     0,   162,   162,   162,     0,     0,     0,   162,
       0,     0,     0,     0,     0,   162,  1557,  1527,  1391,   222,
       0,     0,   222,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1265,     0,     0,     0,     0,     0,   134,
       0,     0,     0,     0,     0,   134,     0,     0,     0,   212,
     212,   134,     0,   224,     0,     0,     0,     0,     0,     0,
       0,  1632,     0,   222,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1602,   811,  -863,  -863,  -863,  -863,
     949,   950,   951,   952,   953,   954,   955,   956,     0,     0,
     811,     0,     0,     0,     0,     0,     0,     0,   288,     0,
       0,   957,     0,   210,     0,   340,     0,     0,     0,     0,
       0,     0,     0,   210,     0,     0,     0,   256,     0,     0,
       0,   870,     0,     0,     0,     0,   120,     0,     0,   128,
       0,     0,     0,     0,     0,  1580,     0,     0,     0,     0,
    1488,  1488,     0,     0,  1495,  1495,   162,     0,     0,     0,
       0,     0,     0,   222,   222,     0,   288,   722,     0,     0,
       0,     0,     0,     0,   120,   120,     0,   128,   128,     0,
       0,   120,     0,     0,   128,     0,     0,     0,     0,     0,
       0,     0,   134,     0,   162,   162,     0,     0,     0,     0,
       0,   162,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   722,     0,     0,   120,     0,     0,   128,     0,
       0,     0,  1631,     0,     0,     0,     0,     0,   562,     0,
     134,   134,     0,     0,     0,   162,     0,   134,  1643,     0,
       0,     0,   212,     0,     0,   562,     0,     0,   212,     0,
       0,     0,     0,   562,   212,     0,     0,     0,   222,   222,
       0,     0,     0,     0,   222,     0,     0,     0,     0,     0,
       0,   134,     0,     0,     0,     0,     0,     0,   120,  1633,
       0,   128,     0,   210,   120,     0,     0,   128,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   162,     0,
       0,     0,     0,     0,   162,     0,     0,     0,     0,     0,
     212,     0,     0,     0,     0,     0,     0,   212,   212,     0,
       0,     0,     0,     0,   212,   347,   348,   349,     0,     0,
     212,     0,     0,     0,   134,     0,     0,     0,   210,     0,
     134,   550,     0,     0,   350,     0,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,     0,   372,     0,
       0,     0,     0,     0,   210,     0,   210,     0,     0,     0,
     373,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   210,   722,     0,
       0,     0,   932,   933,   934,   224,     0,     0,     0,     0,
     222,   222,   722,   722,   722,   722,   722,     0,     0,     0,
       0,   935,   722,   936,   937,   938,   939,   940,   941,   942,
     943,   944,   945,   946,   947,   948,   949,   950,   951,   952,
     953,   954,   955,   956,     0,   212,   222,     0,     0,     0,
       0,     0,     0,     0,     0,   212,     0,   957,     0,     0,
       0,     0,     0,   210,     0,     0,     0,     0,     0,     0,
       0,     0,   222,     0,   210,   210,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   222,   222,
       0,     0,     0,     0,     0,     0,     0,   222,     0,     0,
       0,     0,   211,   211,     0,     0,   223,     0,   222,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   222,
    1157,     0,     0,     0,     0,     0,     0,   722,     0,     0,
     222,     0,   347,   348,   349,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     222,   350,     0,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,     0,   372,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   373,     0,     0,
       0,  1104,     0,   210,   210,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   222,     0,   222,
       0,   222,     0,     0,     0,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   722,     0,     0,   222,
       0,     0,   722,   722,   722,   722,   722,   722,   722,   722,
     722,   722,   722,   722,   722,   722,   722,   722,   722,   722,
     722,   722,   722,   722,   722,   722,   722,   722,   722,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     212,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   722,     0,     0,   222,     0,     0,     0,     0,
       0,   211,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   249,     0,     0,     0,   212,     0,   212,     0,
       0,   222,     0,   222,     0,     0,     0,     0,     0,     0,
       0,   210,     0,     0,   783,     0,     0,     0,   250,   212,
       0,     0,     0,     0,     0,     0,     0,     0,   222,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      36,     0,     0,   211,     0,     0,     0,     0,     0,   222,
     211,   211,     0,     0,   210,     0,     0,   211,     0,     0,
       0,     0,     0,   211,     0,     0,     0,     0,   210,   210,
       0,   722,     0,     0,   211,     0,     0,     0,     0,     0,
       0,     0,     0,   222,     0,   212,   722,     0,   722,     0,
       0,     0,     0,   251,   252,     0,   212,   212,     0,     0,
       0,     0,     0,     0,   722,     0,     0,     0,     0,     0,
       0,   178,     0,     0,    81,   253,     0,    83,    84,   550,
      85,   179,    87,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   254,     0,     0,   223,     0,
       0,   222,     0,     0,   210,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
       0,   255,   347,   348,   349,  1506,     0,   722,     0,     0,
       0,     0,   224,     0,     0,     0,     0,    36,   211,   207,
       0,   350,     0,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,     0,   372,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   212,   212,   373,     0,     0,
       0,     0,     0,   222,     0,   222,     0,     0,     0,     0,
     722,   222,   726,     0,   722,     0,   722,     0,     0,   722,
       0,     0,     0,   222,     0,     0,   222,   222,     0,   222,
       0,   550,     0,     0,    83,    84,   222,    85,   179,    87,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   722,     0,     0,     0,     0,     0,   726,     0,     0,
       0,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,   222,     0,     0,     0,
       0,   654,     0,   115,     0,     0,     0,     0,     0,     0,
       0,   722,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   222,   222,     0,     0,     0,   257,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   347,   348,   349,     0,
       0,     0,     0,   212,     0,     0,   222,     0,   211,     0,
     222,     0,     0,     0,   817,   350,   722,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,     0,   372,
       0,   550,     0,   347,   348,   349,   212,   722,   722,     0,
       0,   373,     0,   722,   222,     0,     0,     0,   222,     0,
     212,   212,   350,   211,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,     0,   372,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   373,   211,
       0,   211,   386,   387,   388,   389,   390,   391,   392,   393,
     394,   395,   396,   397,   398,     0,     0,     0,     0,     0,
       0,     0,   211,   726,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   212,   726,   726,   726,
     726,   726,     0,     0,     0,     0,     0,   726,     0,     0,
       0,   399,   400,     0,     0,     0,     0,     0,     0,     0,
       0,   722,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   970,   222,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   257,   257,     0,   374,     0,   211,   257,
     222,     0,     0,     0,   722,     0,     0,   988,     0,   211,
     211,     0,     0,     0,     0,   722,     0,     0,     0,     0,
     722,     0,     0,   722,   988,   401,     0,     0,     0,     0,
       0,     0,   211,   550,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   855,   415,   387,   388,   389,
     390,   391,   392,   393,   394,   395,   396,   397,   398,     0,
       0,     0,   726,     0,     0,  1026,     0,     0,     0,     0,
       0,     0,   257,     0,     0,   257,     0,     0,     0,     0,
       0,   722,     0,     0,     0,   223,     0,     0,     0,   222,
       0,     0,     0,     0,     0,   399,   400,     0,   550,     0,
       0,     0,     0,   222,     0,     0,     0,     0,     0,     0,
       0,     0,   222,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   222,   211,   211,
       0,     0,     0,     0,     0,     0,   722,     0,     0,     0,
       0,     0,     0,     0,   722,     0,     0,     0,   722,     0,
       0,   722,     0,     0,     0,     0,     0,     0,     0,   401,
       0,   726,     0,     0,   211,     0,     0,   726,   726,   726,
     726,   726,   726,   726,   726,   726,   726,   726,   726,   726,
     726,   726,   726,   726,   726,   726,   726,   726,   726,   726,
     726,   726,   726,   726,     0,     0,     0,     0,     0,   347,
     348,   349,     0,     0,     0,     0,   257,   705,     0,     0,
     724,     0,     0,     0,     0,     0,     0,   726,   350,     0,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,     0,   372,   347,   348,   349,     0,     0,     0,     0,
       0,     0,     0,     0,   373,   724,   211,     0,     0,     0,
       0,     0,   350,     0,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,     0,   372,     0,     0,     0,
       0,     0,     0,     0,   211,     0,     0,     0,   373,   211,
       0,   257,   257,     0,     0,     0,     0,   257,     0,     0,
       0,     0,     0,   211,   211,     0,   726,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   726,     0,   726,     0,   347,   348,   349,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   726,
     893,     0,     0,     0,   350,     0,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,     0,   372,     0,
       0,     0,     0,     0,     0,     0,  1253,     0,     0,   211,
     373,   858,    36,     0,   207,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   726,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   208,   372,     0,   980,     0,     0,     0,     0,
       0,   724,     0,     0,   894,   373,     0,     0,     0,     0,
       0,     0,     0,   257,   257,   724,   724,   724,   724,   724,
       0,     0,     0,   178,     0,   724,    81,    82,     0,    83,
      84,     0,    85,   179,    87,   726,   211,     0,     0,   726,
       0,   726,     0,     0,   726,     0,     0,     0,     0,     0,
       0,     0,  1334,     0,  1343,     0,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,     0,   209,     0,     0,   726,     0,   115,     0,
       0,     0,     0,     0,     0,     0,     0,  1033,     0,     0,
       0,   257,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   211,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   257,   347,   348,   349,     0,   726,     0,     0,     0,
       0,     0,   257,     0,     0,     0,  1400,  1401,     0,     0,
     724,   350,     0,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,     0,   372,     0,     0,     0,     0,
       0,   726,     0,     0,     0,     0,     0,   373,   249,   935,
       0,   936,   937,   938,   939,   940,   941,   942,   943,   944,
     945,   946,   947,   948,   949,   950,   951,   952,   953,   954,
     955,   956,   726,   726,   250,     0,     0,     0,   726,  1538,
       0,     0,     0,  1343,     0,   957,     0,     0,     0,     0,
     257,     0,   257,     0,   705,     0,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   724,
       0,     0,     0,     0,     0,   724,   724,   724,   724,   724,
     724,   724,   724,   724,   724,   724,   724,   724,   724,   724,
     724,   724,   724,   724,   724,   724,   724,   724,   724,   724,
     724,   724,     0,     0,     0,     0,     0,     0,     0,   251,
     252,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   724,     0,   178,   257,     0,
      81,   253,     0,    83,    84,     0,    85,   179,    87,     0,
     913,     0,     0,     0,  1042,     0,   726,     0,     0,     0,
       0,   254,     0,     0,   257,     0,   257,     0,     0,     0,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,     0,   255,     0,   726,
       0,   257,   347,   348,   349,     0,     0,     0,     0,     0,
     726,     0,     0,     0,     0,   726,     0,     0,   726,     0,
       0,   350,     0,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   724,   372,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   257,   373,     0,   724,
       0,   724,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   726,   724,   347,   348,
     349,     0,     0,     0,  1640,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   350,  1334,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
       0,   372,     0,     0,     0,     0,   347,   348,   349,     0,
       0,   726,     0,   373,     0,     0,     0,     0,     0,   726,
     724,     0,     0,   726,     0,   350,   726,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,     0,   372,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   373,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1065,     0,   257,     0,   257,     0,
       0,     0,     0,   724,     0,     0,     0,   724,     0,   724,
       0,     0,   724,     0,     0,     0,   257,     0,     0,   257,
       0,     0,     0,     0,   347,   348,   349,     0,     0,   257,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   350,   724,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,     0,   372,     0,     0,
    1282,   347,   348,   349,     0,     0,     0,     0,     0,   373,
       0,     0,     0,     0,   724,     0,     0,     0,     0,     0,
     350,  1227,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,     0,   372,     0,     0,     0,  1362,   257,
       0,     0,     0,   257,     0,     0,   373,   183,   185,   724,
     187,   188,   189,   191,   192,     0,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,     0,     0,
     216,   219,     0,     0,     0,     0,     0,     0,     0,     0,
     724,   724,     0,   237,     0,     0,   724,   347,   348,   349,
     245,     0,   248,     0,     0,   264,     0,   269,   727,     0,
       0,     0,     0,     0,     0,    36,   350,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,     0,
     372,     0,     0,     0,     0,     0,  1363,   347,   348,   349,
       0,     0,   373,   727,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   315,     0,   350,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   300,
     372,  1228,    83,    84,     0,    85,   179,    87,     0,     0,
       0,     0,   373,     0,   724,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   257,     0,     0,     0,    36,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,  1581,     0,     0,     0,   724,   416,   301,
     347,   348,   349,     0,     0,     0,     0,     0,   724,     0,
       0,     0,     0,   724,     0,     0,   724,     0,     0,   350,
       0,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   442,   372,     0,   442,     0,   457,     0,     0,
       0,     0,   237,   453,  1493,   373,    83,    84,  1494,    85,
     179,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   724,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   459,   315,   727,
    1348,     0,     0,     0,   216,   257,     0,     0,   532,     0,
       0,     0,     0,   727,   727,   727,   727,   727,     0,     0,
     257,   553,   553,   727,     0,     0,     0,     0,     0,   724,
       0,   574,     0,     0,     0,     0,     0,   724,     0,     0,
       0,   724,   586,     0,   724,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    36,     0,   207,     0,   592,
     593,   594,   596,   597,   598,   599,   600,   601,   602,   603,
     604,   605,   606,   607,   608,   609,   610,   611,   612,   613,
     614,   615,   616,   617,     0,   619,     0,   620,   620,   664,
     623,     0,     0,     0,     0,     0,     0,     0,   640,   642,
     643,   644,   645,   646,   647,   648,   649,   650,   651,   652,
     653,     0,     0,     0,     0,     0,   620,   658,     0,   586,
     620,   661,     0,     0,     0,     0,     0,   640,   727,   723,
     665,     0,    83,    84,     0,    85,   179,    87,     0,   674,
       0,   676,     0,     0,     0,     0,     0,   586,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   688,     0,   689,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,   723,   347,   348,   349,     0,   627,
       0,   115,     0,     0,     0,   732,     0,     0,   735,   738,
     739,     0,     0,     0,   350,     0,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,     0,   372,   758,
       0,     0,     0,     0,     0,     0,     0,   727,     0,     0,
     373,     0,     0,   727,   727,   727,   727,   727,   727,   727,
     727,   727,   727,   727,   727,   727,   727,   727,   727,   727,
     727,   727,   727,   727,   727,   727,   727,   727,   727,   727,
     350,     0,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   727,   372,     0,     0,   347,   348,   349,
       0,     0,     0,     0,     0,   822,   373,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   350,   833,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,     0,
     372,     0,     0,     0,     0,     0,     0,     0,     0,   841,
       0,     0,   373,     0,     0,     0,     0,     0,   191,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     723,     0,   249,     0,     0,   470,   851,     0,     0,     0,
       0,     0,     0,     0,   723,   723,   723,   723,   723,     0,
       0,     0,   727,     0,   723,     0,     0,     0,   250,     0,
       0,     0,     0,   883,     0,     0,     0,   727,     0,   727,
     347,   348,   349,     0,     0,     0,   237,     0,     0,     0,
      36,     0,     0,     0,     0,   727,     0,     0,     0,   350,
       0,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   958,   372,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   373,     0,     0,     0,     0,
       0,     0,     0,   251,   252,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   494,   727,     0,
       0,   178,     0,     0,    81,   253,     0,    83,    84,   723,
      85,   179,    87,     0,   995,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   254,     0,  1003,     0,     0,
       0,     0,  1006,     0,  1007,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
       0,   255,   249,   728,     0,  1572,  1022,     0,     0,     0,
       0,   727,     0,     0,     0,   727,  1030,   727,     0,  1031,
     727,  1032,     0,     0,     0,   586,     0,     0,   250,     0,
       0,     0,     0,     0,     0,   586,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   760,     0,
      36,     0,   727,     0,     0,     0,     0,     0,   723,   684,
    1064,     0,     0,     0,   723,   723,   723,   723,   723,   723,
     723,   723,   723,   723,   723,   723,   723,   723,   723,   723,
     723,   723,   723,   723,   723,   723,   723,   723,   723,   723,
     723,     0,   727,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   251,   252,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   723,     0,     0,     0,     0,     0,
       0,   178,     0,     0,    81,   253,     0,    83,    84,     0,
      85,   179,    87,     0,  1233,     0,     0,   727,     0,     0,
       0,  1137,  1138,  1139,     0,   254,     0,   735,  1141,     0,
       0,     0,     0,     0,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   727,   727,
       0,   255,     0,     0,   727,  1156,     0,     0,  1541,   938,
     939,   940,   941,   942,   943,   944,   945,   946,   947,   948,
     949,   950,   951,   952,   953,   954,   955,   956,  1178,     0,
       0,     0,     0,  1182,     0,     0,     0,     0,     0,     0,
       0,   957,     0,   723,     0,     0,   586,     0,     0,     0,
       0,     0,     0,     0,     0,   586,     0,  1156,   723,     0,
     723,     0,     0,     0,   897,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   723,     0,   918,   919,
     920,   921,     0,     0,     0,     0,   237,     0,   929,     0,
       0,     0,   347,   348,   349,     0,  1226,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   350,   727,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,     0,   372,     0,     0,     0,   723,
    1046,  1047,  1048,    36,     0,   727,     0,   373,    29,    30,
       0,     0,     0,     0,     0,     0,   727,     0,    36,     0,
     207,   727,     0,     0,   727,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1273,     0,     0,     0,  1274,
       0,  1275,  1276,     0,     0,     0,     0,  1618,     0,     0,
       0,     0,   586,  1023,     0,     0,     0,     0,   208,     0,
    1290,   586,   723,     0,     0,     0,   723,     0,   723,     0,
     586,   723,     0,     0,     0,     0,     0,     0,     0,     0,
      83,    84,   727,    85,   179,    87,     0,     0,     0,   178,
       0,     0,    81,    82,     0,    83,    84,     0,    85,   179,
      87,     0,     0,   723,     0,     0,     0,    90,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,  1326,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   966,   727,     0,   422,
       0,     0,     0,   723,   115,   727,     0,     0,     0,   727,
       0,     0,   727,     0,     0,   586,     0,     0,  1107,  1110,
    1111,  1112,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,
    1122,  1123,  1124,  1125,  1126,  1127,  1128,  1129,  1130,  1131,
    1132,  1133,  1134,  1135,  1136,     0,     0,     0,   723,   249,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,  1147,   372,
       0,     0,     0,     0,     0,   250,     0,     0,     0,   723,
     723,   373,     0,     0,     0,   723,   347,   348,   349,     0,
       0,     0,     0,     0,     0,     0,     0,    36,     0,     0,
     586,     0,     0,     0,     0,   350,     0,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,     0,   372,
    1533,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   373,     0,     0,     0,     0,     0,     0,     0,     0,
     251,   252,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1222,   178,     0,
       0,    81,   253,     0,    83,    84,     0,    85,   179,    87,
       0,     0,  1236,     0,  1237,     0,     0,     0,     0,     0,
       0,     0,   254,   723,     0,     0,     0,     0,     0,     0,
    1247,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,     0,   255,     0,
       0,     0,     0,     0,     0,     0,   723,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,   723,   207,     0,
       0,     0,   723,     0,     0,   723,     0,     0,     0,   347,
     348,   349,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1281,     0,     0,   962,   963,   350,     0,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,     0,   372,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   723,   373,     0,     0,     0,     0,     0,
       0,     0,     0,    83,    84,     0,    85,   179,    87,     0,
       0,     0,     0,     0,     0,     0,  1321,     0,     0,     0,
    1323,     0,  1324,     0,    36,  1325,     0,     0,     0,     0,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,   347,   348,   349,   723,     0,
     681,     0,   115,     0,     0,     0,   723,  1367,     0,     0,
     723,     0,     0,   723,   350,     0,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,     0,   372,     0,
       0,     0,     0,     0,     0,     0,     0,  1395,     0,     0,
     373,    83,    84,     0,    85,   179,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1577,     0,     0,     0,
       0,     0,     0,     0,     0,   347,   348,   349,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,  1511,     0,   350,   588,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,     0,   372,     0,
       0,     0,     0,  1531,  1532,     0,     0,     0,     0,  1537,
     373,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,  1398,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,     0,     0,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,    32,     0,     0,     0,    33,    34,    35,    36,
      37,    38,     0,    39,     0,     0,     0,    40,    41,    42,
      43,     0,    44,     0,    45,     0,    46,  1563,     0,    47,
       0,     0,     0,    48,    49,    50,    51,    52,    53,    54,
    1249,    55,    56,    57,    58,    59,    60,    61,    62,    63,
       0,    64,    65,    66,    67,    68,    69,     0,     0,     0,
    1587,    70,    71,     0,    72,    73,    74,    75,    76,     0,
       0,  1597,     0,     0,     0,    77,  1601,     0,     0,  1603,
      78,    79,    80,    81,    82,     0,    83,    84,     0,    85,
      86,    87,    88,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,    94,     0,    95,
       0,    96,    97,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
     112,     0,   113,   114,   996,   115,   116,  1634,   117,   118,
     939,   940,   941,   942,   943,   944,   945,   946,   947,   948,
     949,   950,   951,   952,   953,   954,   955,   956,     0,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,   957,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,  1684,     0,     0,     0,     0,     0,     0,     0,
    1689,    13,    14,    15,  1691,     0,     0,  1692,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,     0,     0,
       0,    40,    41,    42,    43,     0,    44,     0,    45,     0,
      46,     0,     0,    47,     0,     0,     0,    48,    49,    50,
      51,    52,    53,    54,     0,    55,    56,    57,    58,    59,
      60,    61,    62,    63,     0,    64,    65,    66,    67,    68,
      69,     0,     0,     0,     0,    70,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,    78,    79,    80,    81,    82,     0,
      83,    84,     0,    85,    86,    87,    88,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,    94,     0,    95,     0,    96,    97,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   112,     0,   113,   114,  1158,   115,
     116,     0,   117,   118,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
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
       0,     0,     0,    91,    92,    93,    94,     0,    95,     0,
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
      33,    34,    35,    36,    37,    38,     0,    39,     0,     0,
       0,    40,    41,    42,    43,     0,    44,     0,    45,     0,
      46,     0,     0,    47,     0,     0,     0,    48,    49,    50,
      51,     0,    53,    54,     0,    55,     0,    57,    58,    59,
      60,    61,    62,    63,     0,    64,    65,    66,     0,    68,
      69,     0,     0,     0,     0,    70,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   178,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   179,    87,    88,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,     0,     0,     0,     0,    96,    97,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   112,     0,   113,   114,   567,   115,
     116,     0,   117,   118,     5,     6,     7,     8,     9,     0,
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
       0,     0,     0,     0,    77,     0,     0,     0,     0,   178,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   179,
      87,    88,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,     0,     0,     0,     0,
      96,    97,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,   112,
       0,   113,   114,   969,   115,   116,     0,   117,   118,     5,
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
       0,     0,     0,     0,   178,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   179,    87,    88,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,     0,     0,     0,     0,    96,    97,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   112,     0,   113,   114,  1071,   115,
     116,     0,   117,   118,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,     0,     0,     0,    40,    41,    42,    43,
    1073,    44,     0,    45,     0,    46,     0,     0,    47,     0,
       0,     0,    48,    49,    50,    51,     0,    53,    54,     0,
      55,     0,    57,    58,    59,    60,    61,    62,    63,     0,
      64,    65,    66,     0,    68,    69,     0,     0,     0,     0,
      70,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   178,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   179,
      87,    88,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,     0,     0,     0,     0,
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
      33,    34,    35,    36,    37,    38,     0,    39,     0,     0,
       0,    40,    41,    42,    43,     0,    44,     0,    45,     0,
      46,  1223,     0,    47,     0,     0,     0,    48,    49,    50,
      51,     0,    53,    54,     0,    55,     0,    57,    58,    59,
      60,    61,    62,    63,     0,    64,    65,    66,     0,    68,
      69,     0,     0,     0,     0,    70,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   178,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   179,    87,    88,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,     0,     0,     0,     0,    96,    97,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   112,     0,   113,   114,     0,   115,
     116,     0,   117,   118,     5,     6,     7,     8,     9,     0,
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
       0,     0,     0,     0,    77,     0,     0,     0,     0,   178,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   179,
      87,    88,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,     0,     0,     0,     0,
      96,    97,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,   112,
       0,   113,   114,  1328,   115,   116,     0,   117,   118,     5,
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
       0,     0,     0,     0,   178,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   179,    87,    88,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,     0,     0,     0,     0,    96,    97,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   112,     0,   113,   114,  1534,   115,
     116,     0,   117,   118,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,     0,     0,     0,    40,    41,    42,    43,
       0,    44,     0,    45,  1576,    46,     0,     0,    47,     0,
       0,     0,    48,    49,    50,    51,     0,    53,    54,     0,
      55,     0,    57,    58,    59,    60,    61,    62,    63,     0,
      64,    65,    66,     0,    68,    69,     0,     0,     0,     0,
      70,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   178,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   179,
      87,    88,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,     0,     0,     0,     0,
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
      33,    34,    35,    36,    37,    38,     0,    39,     0,     0,
       0,    40,    41,    42,    43,     0,    44,     0,    45,     0,
      46,     0,     0,    47,     0,     0,     0,    48,    49,    50,
      51,     0,    53,    54,     0,    55,     0,    57,    58,    59,
      60,    61,    62,    63,     0,    64,    65,    66,     0,    68,
      69,     0,     0,     0,     0,    70,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   178,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   179,    87,    88,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,     0,     0,     0,     0,    96,    97,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   112,     0,   113,   114,  1607,   115,
     116,     0,   117,   118,     5,     6,     7,     8,     9,     0,
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
       0,     0,     0,     0,    77,     0,     0,     0,     0,   178,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   179,
      87,    88,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,     0,     0,     0,     0,
      96,    97,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,   112,
       0,   113,   114,  1608,   115,   116,     0,   117,   118,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,     0,     0,
       0,    40,    41,    42,    43,     0,    44,  1611,    45,     0,
      46,     0,     0,    47,     0,     0,     0,    48,    49,    50,
      51,     0,    53,    54,     0,    55,     0,    57,    58,    59,
      60,    61,    62,    63,     0,    64,    65,    66,     0,    68,
      69,     0,     0,     0,     0,    70,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   178,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   179,    87,    88,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,     0,     0,     0,     0,    96,    97,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   112,     0,   113,   114,     0,   115,
     116,     0,   117,   118,     5,     6,     7,     8,     9,     0,
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
       0,     0,     0,     0,    77,     0,     0,     0,     0,   178,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   179,
      87,    88,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,     0,     0,     0,     0,
      96,    97,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,   112,
       0,   113,   114,  1626,   115,   116,     0,   117,   118,     5,
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
       0,     0,     0,     0,   178,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   179,    87,    88,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,     0,     0,     0,     0,    96,    97,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   112,     0,   113,   114,  1679,   115,
     116,     0,   117,   118,     5,     6,     7,     8,     9,     0,
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
       0,     0,     0,     0,    77,     0,     0,     0,     0,   178,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   179,
      87,    88,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,     0,     0,     0,     0,
      96,    97,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,   112,
       0,   113,   114,  1685,   115,   116,     0,   117,   118,     5,
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
       0,     0,     0,     0,   178,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   179,    87,    88,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,     0,     0,     0,     0,    96,    97,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   112,     0,   113,   114,     0,   115,
     116,     0,   117,   118,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,   443,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,     0,     0,     0,    40,    41,    42,    43,
       0,    44,     0,    45,     0,    46,     0,     0,    47,     0,
       0,     0,    48,    49,    50,    51,     0,    53,    54,     0,
      55,     0,    57,    58,    59,    60,   174,   175,    63,     0,
      64,    65,    66,     0,     0,     0,     0,     0,     0,     0,
      70,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   178,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   179,
      87,     0,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,     0,     0,     0,     0,
      96,    97,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,   112,
       0,   113,   114,     0,   115,   116,     0,   117,   118,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,   691,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,     0,     0,
       0,    40,    41,    42,    43,     0,    44,     0,    45,     0,
      46,     0,     0,    47,     0,     0,     0,    48,    49,    50,
      51,     0,    53,    54,     0,    55,     0,    57,    58,    59,
      60,   174,   175,    63,     0,    64,    65,    66,     0,     0,
       0,     0,     0,     0,     0,    70,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   178,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   179,    87,     0,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,     0,     0,     0,     0,    96,    97,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   112,     0,   113,   114,     0,   115,
     116,     0,   117,   118,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,   899,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,     0,     0,     0,    40,    41,    42,    43,
       0,    44,     0,    45,     0,    46,     0,     0,    47,     0,
       0,     0,    48,    49,    50,    51,     0,    53,    54,     0,
      55,     0,    57,    58,    59,    60,   174,   175,    63,     0,
      64,    65,    66,     0,     0,     0,     0,     0,     0,     0,
      70,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   178,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   179,
      87,     0,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,     0,     0,     0,     0,
      96,    97,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,   112,
       0,   113,   114,     0,   115,   116,     0,   117,   118,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,  1390,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    15,     0,     0,     0,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     0,    28,    29,    30,    31,    32,     0,     0,     0,
      33,    34,    35,    36,    37,    38,     0,    39,     0,     0,
       0,    40,    41,    42,    43,     0,    44,     0,    45,     0,
      46,     0,     0,    47,     0,     0,     0,    48,    49,    50,
      51,     0,    53,    54,     0,    55,     0,    57,    58,    59,
      60,   174,   175,    63,     0,    64,    65,    66,     0,     0,
       0,     0,     0,     0,     0,    70,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   178,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   179,    87,     0,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,     0,     0,     0,     0,    96,    97,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   112,     0,   113,   114,     0,   115,
     116,     0,   117,   118,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
       0,  1526,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,    32,     0,     0,     0,    33,    34,    35,    36,    37,
      38,     0,    39,     0,     0,     0,    40,    41,    42,    43,
       0,    44,     0,    45,     0,    46,     0,     0,    47,     0,
       0,     0,    48,    49,    50,    51,     0,    53,    54,     0,
      55,     0,    57,    58,    59,    60,   174,   175,    63,     0,
      64,    65,    66,     0,     0,     0,     0,     0,     0,     0,
      70,    71,     0,    72,    73,    74,    75,    76,     0,     0,
       0,     0,     0,     0,    77,     0,     0,     0,     0,   178,
      79,    80,    81,    82,     0,    83,    84,     0,    85,   179,
      87,     0,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,     0,     0,     0,     0,
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
      33,    34,    35,    36,    37,    38,     0,    39,     0,     0,
       0,    40,    41,    42,    43,     0,    44,     0,    45,     0,
      46,     0,     0,    47,     0,     0,     0,    48,    49,    50,
      51,     0,    53,    54,     0,    55,     0,    57,    58,    59,
      60,   174,   175,    63,     0,    64,    65,    66,     0,     0,
       0,     0,     0,     0,     0,    70,    71,     0,    72,    73,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,   178,    79,    80,    81,    82,     0,
      83,    84,     0,    85,   179,    87,     0,     0,     0,    89,
       0,     0,    90,     0,     0,     0,     0,     0,    91,    92,
      93,     0,     0,     0,     0,    96,    97,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   112,     0,   113,   114,     0,   115,
     116,     0,   117,   118,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   636,    12,
       0,     0,     0,     0,     0,     0,   637,     0,     0,     0,
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
      87,     0,     0,     0,    89,     0,     0,    90,     0,     0,
       0,     0,     0,    91,    92,    93,     0,     0,     0,     0,
      96,    97,   261,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,     0,     0,   112,
       0,     0,     0,     0,   115,   116,     0,   117,   118,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
      93,     0,     0,    10,     0,    96,    97,   261,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,     0,     0,   112,   582,   262,     0,     0,   115,
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
       0,   583,     0,    89,     0,     0,    90,     5,     6,     7,
       8,     9,    91,    92,    93,     0,     0,    10,     0,    96,
      97,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,   112,  1020,
       0,     0,     0,   115,   116,     0,   117,   118,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,    40,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    51,     0,
       0,     0,     0,     0,     0,     0,    58,    59,    60,   174,
     175,   176,     0,     0,    65,    66,     0,     0,     0,     0,
       0,     0,     0,   177,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   178,    79,    80,    81,    82,     0,    83,    84,
       0,    85,   179,    87,     0,  1021,     0,    89,     0,     0,
      90,     0,     0,     0,     0,     0,    91,    92,    93,     0,
       0,     0,     0,    96,    97,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,     0,   112,     0,     0,     0,     0,   115,   116,     0,
     117,   118,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   636,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    14,    15,     0,     0,     0,
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
      36,    37,    38,     0,     0,     0,     0,     0,    40,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   190,     0,     0,    51,     0,     0,
       0,     0,     0,     0,     0,    58,    59,    60,   174,   175,
     176,     0,     0,    65,    66,     0,     0,     0,     0,     0,
       0,     0,   177,    71,     0,    72,    73,    74,    75,    76,
       0,     0,     0,     0,     0,     0,    77,     0,     0,     0,
       0,   178,    79,    80,    81,    82,     0,    83,    84,     0,
      85,   179,    87,     0,     0,     0,    89,     0,     0,    90,
       5,     6,     7,     8,     9,    91,    92,    93,     0,     0,
      10,     0,    96,    97,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,     0,
       0,   112,   215,     0,     0,     0,   115,   116,     0,   117,
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
       0,   244,     0,     0,   115,   116,     0,   117,   118,     0,
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
     111,     0,     0,   112,     0,   247,     0,     0,   115,   116,
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
       0,     0,    89,     0,     0,    90,     0,     0,     0,     0,
       0,    91,    92,    93,     0,     0,     0,     0,    96,    97,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,     0,     0,   112,   441,     0,
       0,     0,   115,   116,     0,   117,   118,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   595,   372,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   373,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,    40,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    51,     0,
       0,     0,     0,     0,     0,     0,    58,    59,    60,   174,
     175,   176,     0,     0,    65,    66,     0,     0,     0,     0,
       0,     0,     0,   177,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   178,    79,    80,    81,    82,     0,    83,    84,
       0,    85,   179,    87,     0,     0,     0,    89,     0,     0,
      90,     5,     6,     7,     8,     9,    91,    92,    93,     0,
       0,    10,     0,    96,    97,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,     0,   112,   637,     0,     0,     0,   115,   116,     0,
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
     108,   109,   110,   111,     0,     0,   112,   673,     0,     0,
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
     112,   675,     0,     0,     0,   115,   116,     0,   117,   118,
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
     110,   111,     0,     0,   112,     0,     0,     0,     0,   115,
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
       0,     0,     0,    89,     0,     0,    90,     5,     6,     7,
       8,     9,    91,    92,    93,     0,     0,    10,     0,    96,
      97,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,     0,     0,   112,  1063,
       0,   687,     0,   115,   116,     0,   117,   118,     0,     0,
      14,    15,     0,     0,     0,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     0,
      28,    29,    30,    31,     0,     0,     0,     0,    33,    34,
      35,    36,    37,    38,     0,     0,     0,     0,     0,    40,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    51,     0,
       0,     0,     0,     0,     0,     0,    58,    59,    60,   174,
     175,   176,     0,     0,    65,    66,     0,     0,     0,     0,
       0,     0,     0,   177,    71,     0,    72,    73,    74,    75,
      76,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,   178,    79,    80,    81,    82,     0,    83,    84,
       0,    85,   179,    87,     0,     0,     0,    89,     0,     0,
      90,     5,     6,     7,     8,     9,    91,    92,    93,     0,
       0,    10,     0,    96,    97,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
       0,     0,   112,     0,     0,     0,     0,   115,   116,     0,
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
     531,    38,     0,     0,     0,     0,     0,    40,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    51,     0,     0,     0,
       0,     0,     0,     0,    58,    59,    60,   174,   175,   176,
       0,     0,    65,    66,     0,     0,     0,     0,     0,     0,
       0,   177,    71,     0,    72,    73,    74,    75,    76,     0,
       0,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     178,    79,    80,    81,    82,     0,    83,    84,     0,    85,
     179,    87,     0,     0,     0,    89,     0,     0,    90,     0,
       0,     0,     0,     0,    91,    92,    93,     0,     0,     0,
       0,    96,    97,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,     0,     0,
     112,     0,     0,     0,     0,   115,   116,     0,   117,   118,
    1409,  1410,  1411,  1412,  1413,     0,     0,  1414,  1415,  1416,
    1417,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1418,  1419,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,     0,   372,     0,
    1420,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     373,     0,     0,     0,  1421,  1422,  1423,  1424,  1425,  1426,
    1427,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,  1428,  1429,  1430,  1431,  1432,  1433,  1434,
    1435,  1436,  1437,  1438,  1439,  1440,  1441,  1442,  1443,  1444,
    1445,  1446,  1447,  1448,  1449,  1450,  1451,  1452,  1453,  1454,
    1455,  1456,  1457,  1458,  1459,  1460,  1461,  1462,  1463,  1464,
    1465,  1466,  1467,  1468,     0,     0,  1469,  1470,     0,  1471,
    1472,  1473,  1474,  1475,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   249,  1476,  1477,  1478,     0,     0,
       0,    83,    84,     0,    85,   179,    87,  1479,     0,  1480,
    1481,     0,  1482,     0,     0,     0,     0,     0,     0,  1483,
     250,     0,  1484,     0,  1485,     0,  1486,  1487,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,    36,  -863,  -863,  -863,  -863,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   249,
     372,     0,     0,     0,     0,     0,     0,     0,     0,  -319,
       0,     0,   373,     0,     0,     0,     0,    58,    59,    60,
     174,   175,   338,     0,     0,   250,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   251,   252,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    36,     0,     0,
       0,     0,     0,   178,     0,     0,    81,   253,     0,    83,
      84,     0,    85,   179,    87,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   464,     0,     0,   254,     0,     0,
       0,     0,     0,     0,     0,    97,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     251,   252,     0,   255,     0,     0,    36,     0,   207,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   178,     0,
       0,    81,   253,     0,    83,    84,     0,    85,   179,    87,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   254,     0,     0,     0,   208,     0,     0,  1113,
       0,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   711,   712,     0,   255,     0,
       0,   713,     0,   714,     0,     0,     0,   178,     0,     0,
      81,    82,     0,    83,    84,   715,    85,   179,    87,     0,
       0,     0,     0,    33,    34,    35,    36,     0,     0,     0,
       0,     0,     0,     0,   716,     0,     0,     0,     0,     0,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,     0,   209,     0,     0,
     507,     0,   115,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   717,
       0,    72,    73,    74,    75,    76,     0,     0,    36,     0,
     207,     0,   718,     0,     0,     0,     0,   178,    79,    80,
      81,   719,     0,    83,    84,     0,    85,   179,    87,     0,
      36,     0,    89,     0,     0,     0,     0,     0,     0,     0,
       0,   720,     0,     0,     0,     0,     0,     0,    96,     0,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   711,   712,     0,   721,     0,     0,
     713,     0,   714,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   715,    83,    84,     0,    85,   179,
      87,     0,    33,    34,    35,    36,     0,     0,     0,     0,
       0,   178,     0,   716,    81,     0,     0,    83,    84,     0,
      85,   179,    87,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,     0,     0,     0,
       0,     0,  1016,     0,   115,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   717,     0,
      72,    73,    74,    75,    76,  1579,     0,    36,     0,     0,
       0,   718,     0,     0,     0,     0,   178,    79,    80,    81,
     719,     0,    83,    84,     0,    85,   179,    87,     0,     0,
       0,    89,     0,     0,     0,     0,     0,     0,     0,     0,
     720,     0,     0,   860,   861,     0,     0,    96,     0,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   862,     0,     0,   721,     0,     0,     0,
       0,   863,   864,   865,    36,     0,     0,     0,     0,     0,
       0,   849,   866,     0,    83,    84,     0,    85,   179,    87,
      36,     0,   207,     0,     0,   940,   941,   942,   943,   944,
     945,   946,   947,   948,   949,   950,   951,   952,   953,   954,
     955,   956,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   957,     0,   867,   831,     0,
     208,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     868,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    83,    84,     0,    85,   179,    87,     0,    36,     0,
       0,   178,     0,     0,    81,    82,   704,    83,    84,   869,
      85,   179,    87,    36,     0,   207,     0,     0,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
       0,   209,     0,   208,     0,     0,   115,     0,     0,     0,
       0,     0,     0,     0,     0,   523,     0,     0,     0,   178,
       0,     0,    81,     0,     0,    83,    84,     0,    85,   179,
      87,     0,     0,     0,   178,     0,     0,    81,    82,     0,
      83,    84,     0,    85,   179,    87,    36,     0,   207,     0,
       0,     0,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,     0,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,     0,     0,   209,     0,   208,     0,     0,   115,
       0,     0,     0,     0,     0,     0,     0,     0,   991,    36,
       0,   207,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   178,     0,     0,
      81,    82,     0,    83,    84,     0,    85,   179,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   208,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,     0,   209,     0,     0,
     178,     0,   115,    81,    82,     0,    83,    84,     0,    85,
     179,    87,    36,     0,   207,     0,     0,     0,   941,   942,
     943,   944,   945,   946,   947,   948,   949,   950,   951,   952,
     953,   954,   955,   956,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   957,     0,     0,
     209,     0,   220,     0,     0,   115,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    36,     0,   207,     0,     0,
       0,     0,     0,   545,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   178,     0,     0,    81,    82,     0,    83,
      84,     0,    85,   179,    87,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   208,    36,     0,   207,     0,
       0,     0,     0,     0,     0,     0,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,     0,   221,     0,     0,   178,     0,   115,    81,
      82,     0,    83,    84,     0,    85,   179,    87,     0,     0,
       0,     0,   626,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,    83,    84,     0,    85,   179,    87,     0,
       0,   546,     0,     0,     0,     0,     0,     0,     0,     0,
    1347,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,     0,     0,     0,     0,
     627,     0,   115,    83,    84,     0,    85,   179,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   347,   348,   349,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   350,  1348,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,     0,
     372,   347,   348,   349,     0,     0,     0,     0,     0,     0,
       0,     0,   373,     0,     0,     0,     0,     0,     0,     0,
     350,     0,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,     0,   372,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   373,   347,   348,   349,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   418,   350,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,     0,
     372,   347,   348,   349,     0,     0,     0,     0,     0,     0,
       0,     0,   373,     0,     0,     0,     0,     0,     0,   427,
     350,     0,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,     0,   372,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   373,   347,   348,   349,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   800,   350,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,     0,
     372,   347,   348,   349,     0,     0,     0,     0,     0,     0,
       0,     0,   373,     0,     0,     0,     0,     0,     0,   837,
     350,     0,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,     0,   372,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   373,   347,   348,   349,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   878,   350,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,     0,
     372,   932,   933,   934,     0,     0,     0,     0,     0,     0,
       0,     0,   373,     0,     0,     0,     0,     0,     0,  1170,
     935,     0,   936,   937,   938,   939,   940,   941,   942,   943,
     944,   945,   946,   947,   948,   949,   950,   951,   952,   953,
     954,   955,   956,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   957,   932,   933,   934,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1188,   935,     0,   936,   937,
     938,   939,   940,   941,   942,   943,   944,   945,   946,   947,
     948,   949,   950,   951,   952,   953,   954,   955,   956,   932,
     933,   934,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   957,     0,     0,     0,     0,     0,   935,  1095,
     936,   937,   938,   939,   940,   941,   942,   943,   944,   945,
     946,   947,   948,   949,   950,   951,   952,   953,   954,   955,
     956,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    36,     0,     0,   957,   932,   933,   934,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      36,     0,     0,     0,   935,  1243,   936,   937,   938,   939,
     940,   941,   942,   943,   944,   945,   946,   947,   948,   949,
     950,   951,   952,   953,   954,   955,   956,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     957,  1335,     0,     0,     0,    36,     0,  1320,     0,     0,
       0,     0,   178,  1336,  1337,    81,    82,     0,    83,    84,
       0,    85,   179,    87,    36,     0,   777,   778,     0,     0,
       0,   178,   270,   271,    81,  1338,     0,    83,    84,     0,
      85,  1339,    87,     0,     0,     0,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,     0,     0,  1397,     0,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,    36,   272,
       0,     0,    83,    84,     0,    85,   179,    87,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    83,    84,     0,    85,   179,    87,     0,     0,     0,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,     0,     0,     0,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   335,     0,    83,    84,    36,    85,   179,
      87,     0,     0,     0,     0,   495,     0,     0,    83,    84,
       0,    85,   179,    87,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,     0,     0,     0,    36,     0,   499,     0,     0,    83,
      84,     0,    85,   179,    87,     0,     0,     0,     0,    36,
       0,   272,     0,     0,    83,    84,     0,    85,   179,    87,
       0,     0,     0,     0,     0,     0,     0,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     626,     0,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,  1105,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    83,    84,     0,    85,   179,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    83,    84,     0,    85,
     179,    87,     0,     0,     0,     0,     0,     0,     0,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,     0,     0,     0,
       0,     0,     0,    83,    84,     0,    85,   179,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   347,   348,   349,     0,
       0,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   695,   350,     0,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,     0,   372,
     347,   348,   349,     0,     0,     0,     0,     0,     0,     0,
       0,   373,     0,     0,     0,     0,     0,     0,     0,   350,
     834,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   696,   372,   347,   348,   349,     0,     0,     0,
       0,     0,     0,     0,     0,   373,     0,     0,     0,     0,
       0,     0,     0,   350,     0,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,     0,   372,   932,   933,
     934,     0,     0,     0,     0,     0,     0,     0,     0,   373,
       0,     0,     0,     0,     0,     0,     0,   935,  1248,   936,
     937,   938,   939,   940,   941,   942,   943,   944,   945,   946,
     947,   948,   949,   950,   951,   952,   953,   954,   955,   956,
     932,   933,   934,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   957,     0,     0,     0,     0,     0,   935,
       0,   936,   937,   938,   939,   940,   941,   942,   943,   944,
     945,   946,   947,   948,   949,   950,   951,   952,   953,   954,
     955,   956,   348,   349,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   957,     0,     0,     0,     0,
     350,     0,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,     0,   372,   933,   934,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   373,     0,     0,     0,
       0,     0,     0,   935,     0,   936,   937,   938,   939,   940,
     941,   942,   943,   944,   945,   946,   947,   948,   949,   950,
     951,   952,   953,   954,   955,   956,   349,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   957,
       0,     0,     0,   350,     0,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   934,   372,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   373,
       0,     0,     0,   935,     0,   936,   937,   938,   939,   940,
     941,   942,   943,   944,   945,   946,   947,   948,   949,   950,
     951,   952,   953,   954,   955,   956,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   957,
     936,   937,   938,   939,   940,   941,   942,   943,   944,   945,
     946,   947,   948,   949,   950,   951,   952,   953,   954,   955,
     956,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   957,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   368,   369,   370,   371,     0,   372,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   373,   937,
     938,   939,   940,   941,   942,   943,   944,   945,   946,   947,
     948,   949,   950,   951,   952,   953,   954,   955,   956,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   957,   355,   356,   357,   358,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
       0,   372,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   373,  -863,  -863,  -863,  -863,   945,   946,
     947,   948,   949,   950,   951,   952,   953,   954,   955,   956,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   957
};

static const yytype_int16 yycheck[] =
{
       4,     4,    88,     4,   160,    52,   134,   181,    94,    95,
     585,   170,    32,   308,    53,   666,     4,   809,     4,     4,
       4,   407,  1012,    43,   694,   222,   163,    47,   386,   796,
     218,   412,   372,     4,   112,   228,   160,   228,   827,   538,
     998,     9,    26,    27,   701,    49,   830,   133,    52,   435,
    1183,     9,    30,     9,   182,   443,     4,    30,    45,  1000,
      66,    66,   846,   892,     9,    69,     9,     9,    45,     9,
       9,     9,    66,   249,   250,    33,    66,     9,     9,   255,
       9,     9,    86,     9,    88,     9,     9,   127,     9,    96,
      94,    95,   229,     9,    30,     9,   112,    79,     9,     9,
       9,    35,     9,   491,     9,    56,   108,   151,     9,   168,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,   198,    84,    66,     0,    78,   112,   133,
      81,   209,   198,   134,   127,    79,    53,   127,    35,   198,
     153,   198,   147,   221,   151,    79,    66,    66,    65,   308,
      45,    45,   328,  1286,    45,  1288,   200,   201,    66,    63,
      64,   928,   202,   165,     8,   339,   198,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      66,   182,    79,    66,   144,    35,   127,    66,   201,    66,
     194,    66,   198,    66,    66,   201,   201,    66,    66,   198,
      66,    66,   198,   147,   198,   123,   199,   201,   286,   196,
      50,   201,   160,    84,    50,    79,    63,    64,    79,   196,
     379,   203,   199,   127,   169,   209,   169,   161,   201,    79,
     234,   215,   200,   421,   238,  1193,  1025,   221,   242,  1028,
     414,   199,   401,  1376,  1185,   902,   202,   904,   334,  1078,
     234,  1192,   127,  1194,   238,  1039,   260,   385,   200,   201,
     200,   200,   200,   234,   200,    35,   425,   238,   200,   200,
     286,   200,   200,   144,   200,   434,   200,   200,   437,   200,
     127,   201,   201,   199,   331,   199,   462,   786,   199,   199,
     199,  1058,   199,   277,   199,   199,   382,   383,   199,   687,
     284,   285,   286,   691,   199,   199,    35,   291,   199,    79,
     313,   161,   152,   297,   318,   201,   152,   318,   201,   270,
     271,   272,   201,   327,   201,    96,   201,   331,   201,   201,
     334,   167,   201,   201,   318,   201,   201,    99,   100,   203,
      79,   127,   203,    79,   422,    84,  1520,   386,    84,   300,
      79,    35,   199,   104,   123,  1296,    96,    99,   100,    79,
     548,   112,   113,   114,   115,   116,   117,   555,   376,    79,
     318,   564,   201,   564,    84,   379,   380,   381,   382,   383,
     151,   557,   558,   384,    96,   198,     4,   563,    96,    96,
    1523,   161,   773,   198,   372,    79,   404,   401,  1187,  1573,
     408,   379,    29,   198,  1074,    79,   145,   146,  1059,   145,
     146,   151,   201,   199,   198,    79,   198,   573,   198,    46,
      84,   425,    49,   401,   144,   145,   146,    45,   412,   180,
     152,   628,   161,   437,   144,   145,   146,   625,   422,   151,
     202,    71,    72,   151,   151,   198,   450,   425,   198,   573,
     112,   113,   114,   115,   116,   117,   434,   843,   655,   437,
     202,   635,     4,   198,    79,    30,   450,   853,   207,    84,
     206,   145,   146,  1240,   151,   672,    71,    72,   671,   450,
     671,   145,   146,   198,   677,    35,   677,   105,    79,   147,
     125,   126,   110,    84,   112,   113,   114,   115,   116,   117,
     118,   796,   506,    45,   200,   591,   200,   147,  1297,   849,
     168,   899,  1645,    98,    99,   100,    14,   521,   180,   707,
      98,    99,   100,   527,   528,   722,    66,  1660,   168,   144,
     145,   146,    30,   709,   710,   200,   154,   155,  1305,   157,
     677,   200,   201,   201,   495,    74,    75,    76,   499,    47,
    1201,   200,  1203,   504,   145,   146,    85,   200,   198,   200,
      79,   201,   180,   105,    66,    84,    29,    66,   110,   200,
     112,   113,   114,   115,   116,   117,   118,   112,   113,   114,
     115,   116,   117,    46,   202,   201,    49,   591,   112,   113,
     114,   115,   116,   117,   147,   153,   200,   201,   582,   123,
     124,  1548,  1549,   132,   133,   134,   135,   136,   166,  1544,
    1545,   787,   154,   155,   143,   157,   112,   113,   114,   198,
     149,   150,   116,   117,   118,   573,   145,   146,    49,    50,
      51,   807,    53,   928,   163,   993,   160,   796,   180,    49,
      50,    51,   818,   201,    65,   180,   709,   710,  1034,   198,
     179,    66,  1303,   637,   813,    65,   180,  1043,   147,   198,
     202,  1651,   666,   151,   668,   200,   200,    44,    65,   147,
     690,   168,   127,   205,   294,     9,  1666,   147,   298,   147,
     198,   685,   127,     8,   685,   873,   200,   168,   198,   673,
      14,   675,    14,    79,   698,   699,  1077,   200,    14,   200,
     320,   685,   322,   323,   324,   325,   112,   113,   114,   115,
     116,   117,   696,   199,   168,   699,    14,   123,   124,    46,
      47,    48,    49,    50,    51,    96,    53,   199,   699,   917,
     906,   204,   908,  1384,   199,   104,   924,   685,    65,   198,
     744,   198,   198,   744,     9,    88,   750,   199,   199,     9,
     754,   755,   158,   200,   160,    14,  1144,   198,     9,   184,
     744,    79,   848,  1058,    79,    79,   198,   187,     9,   928,
     200,   200,  1153,     9,   180,    79,   199,   199,   762,   125,
     198,   785,   785,   200,   785,   199,    66,    30,   126,   773,
     774,   167,   128,     9,  1180,   799,   744,   785,   749,   785,
     785,   785,   753,  1189,   199,   147,   809,   199,   984,   813,
       9,   199,  1198,    14,   785,   181,   182,   183,   196,     9,
    1017,    66,   188,   189,   827,  1213,   192,   193,     9,  1621,
     169,   199,     9,   125,  1010,   813,  1012,   785,    14,     9,
     205,  1229,   205,   202,   848,   198,  1638,     4,    46,    47,
      48,    49,    50,    51,  1646,   205,   860,   861,   862,   199,
     205,  1037,   199,   198,    96,   885,  1054,    65,   198,   200,
     128,   849,   200,   147,  1525,     9,   199,     4,   198,   147,
     147,   198,   886,   184,   888,   184,   890,   888,    45,   890,
     201,    14,     9,    79,   201,    14,    96,  1283,   200,  1058,
      14,   201,   886,    14,   888,   909,   890,   201,   892,   893,
     205,   200,    30,   198,  1102,   886,  1092,   198,    45,  1307,
      30,  1109,   198,   927,    14,   198,   930,   198,  1316,     9,
     199,   128,    14,     9,   200,   198,   200,   199,     9,  1327,
     888,    65,   890,    79,   205,  1240,     9,   200,   105,   144,
     198,    79,    14,   110,   993,   112,   113,   114,   115,   116,
     117,   118,   128,    79,   198,   128,   199,   205,   198,   973,
     201,   198,   976,   199,     9,   198,   201,   201,   105,   199,
     144,   985,  1368,   110,    30,   112,   113,   114,   115,   116,
     117,   118,  1567,   997,   997,    73,   997,   154,   155,   128,
     157,   169,  1390,    30,   200,   199,   128,   200,   199,   997,
    1305,   997,   997,   997,   199,     9,   199,     9,   202,   199,
     202,  1209,  1025,   180,    96,  1028,   997,   154,   155,   201,
     157,    14,   199,   198,   128,     9,  1020,   199,   199,   199,
     198,     4,   201,   199,   199,   202,  1066,   200,    30,   997,
     199,  1055,   199,   180,   200,  1059,  1232,   200,  1234,   201,
     156,  1636,    79,   152,    14,  1069,    79,   198,   110,   199,
     128,   128,    14,   199,   199,   202,  1252,  1081,   199,  1063,
    1081,  1240,    45,    79,    14,  1069,   201,   199,   201,  1265,
     198,   200,   199,  1077,  1078,   201,   199,  1081,  1069,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,  1267,    53,   128,   200,   200,    14,    26,    27,     4,
      14,    30,    14,   200,    65,   199,   201,  1515,    55,  1517,
      79,   198,    79,  1081,     9,   200,    79,  1265,  1526,   108,
      96,   147,   105,    52,    96,   159,  1305,   110,  1151,   112,
     113,   114,   115,   116,   117,   118,    33,  1160,    14,   198,
      45,   199,   198,   200,   165,    79,   199,   162,     9,  1153,
     200,    79,   201,   199,   199,    79,    14,    79,    14,  1353,
      79,    14,  1570,    79,  1187,    14,   749,   753,  1629,   844,
     382,   154,   155,   847,   157,   504,   383,  1201,   788,  1203,
    1642,   380,  1389,  1151,  1226,  1075,  1638,  1379,  1255,  1407,
    1491,  1260,  1160,   509,  1670,  1658,  1503,   180,    41,  1011,
     105,  1375,   968,   480,   965,   110,   480,   112,   113,   114,
     115,   116,   117,   118,   386,   741,  1001,  1041,   925,   202,
     861,  1055,   875,   292,   310,   285,    -1,  1251,   709,   910,
    1251,  1255,    -1,    -1,    -1,    -1,  1260,    -1,    -1,    -1,
      -1,    -1,    -1,  1264,    -1,    -1,    -1,  1251,    -1,   154,
     155,    -1,   157,    -1,    -1,    -1,  1664,    -1,    -1,    -1,
      -1,    -1,    -1,  1671,  1287,    -1,    -1,    -1,    -1,    -1,
    1293,    -1,  1295,    -1,  1297,   180,    -1,    -1,    -1,  1303,
     209,    -1,    -1,  1251,  1308,    -1,   215,  1308,  1355,  1313,
      -1,    -1,   221,  1317,    -1,    -1,  1317,   202,    -1,  1267,
      -1,    -1,    -1,    -1,  1308,    -1,    -1,    -1,    -1,  1313,
      -1,  1505,    -1,  1317,    -1,    -1,    -1,    -1,  1342,  1287,
     249,   250,  1313,    -1,  1348,  1293,   255,  1295,    -1,    -1,
      -1,  1355,    -1,    -1,    -1,  1359,  1357,    -1,    -1,    -1,
    1308,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   277,  1317,
      -1,    -1,    -1,    -1,    -1,   284,   285,    -1,  1381,    -1,
    1384,    -1,   291,  1387,  1388,  1389,  1387,    -1,   297,  1393,
      -1,    -1,  1393,    -1,    -1,  1399,    -1,    -1,  1399,   308,
      -1,    -1,    -1,  1387,  1388,  1389,    -1,    -1,    -1,  1393,
      -1,    -1,    -1,    -1,    -1,  1399,  1502,  1388,  1389,   328,
      -1,    -1,   331,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1381,    -1,    -1,    -1,    -1,    -1,  1387,
      -1,    -1,    -1,    -1,    -1,  1393,    -1,    -1,    -1,    26,
      27,  1399,    -1,    30,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1617,    -1,   372,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1560,  1651,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    -1,
    1666,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1502,    -1,
      -1,    65,    -1,   412,    -1,  1633,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   422,    -1,    -1,    -1,  1521,    -1,    -1,
      -1,  1525,    -1,    -1,    -1,    -1,  1530,    -1,    -1,  1530,
      -1,    -1,    -1,    -1,    -1,  1539,    -1,    -1,    -1,    -1,
    1544,  1545,    -1,    -1,  1548,  1549,  1530,    -1,    -1,    -1,
      -1,    -1,    -1,   462,   463,    -1,  1560,   466,    -1,    -1,
      -1,    -1,    -1,    -1,  1568,  1569,    -1,  1568,  1569,    -1,
      -1,  1575,    -1,    -1,  1575,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1530,    -1,  1568,  1569,    -1,    -1,    -1,    -1,
      -1,  1575,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   511,    -1,    -1,  1609,    -1,    -1,  1609,    -1,
      -1,    -1,  1616,    -1,    -1,    -1,    -1,    -1,  1621,    -1,
    1568,  1569,    -1,    -1,    -1,  1609,    -1,  1575,  1632,    -1,
      -1,    -1,   209,    -1,    -1,  1638,    -1,    -1,   215,    -1,
      -1,    -1,    -1,  1646,   221,    -1,    -1,    -1,   557,   558,
      -1,    -1,    -1,    -1,   563,    -1,    -1,    -1,    -1,    -1,
      -1,  1609,    -1,    -1,    -1,    -1,    -1,    -1,  1672,  1617,
      -1,  1672,    -1,   582,  1678,    -1,    -1,  1678,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1672,    -1,
      -1,    -1,    -1,    -1,  1678,    -1,    -1,    -1,    -1,    -1,
     277,    -1,    -1,    -1,    -1,    -1,    -1,   284,   285,    -1,
      -1,    -1,    -1,    -1,   291,    10,    11,    12,    -1,    -1,
     297,    -1,    -1,    -1,  1672,    -1,    -1,    -1,   637,    -1,
    1678,   308,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,    -1,   673,    -1,   675,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   696,   697,    -1,
      -1,    -1,    10,    11,    12,   372,    -1,    -1,    -1,    -1,
     709,   710,   711,   712,   713,   714,   715,    -1,    -1,    -1,
      -1,    29,   721,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,   412,   745,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   422,    -1,    65,    -1,    -1,
      -1,    -1,    -1,   762,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   771,    -1,   773,   774,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   787,   788,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   796,    -1,    -1,
      -1,    -1,    26,    27,    -1,    -1,    30,    -1,   807,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   818,
     205,    -1,    -1,    -1,    -1,    -1,    -1,   826,    -1,    -1,
     829,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     849,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,   199,    -1,   892,   893,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   906,    -1,   908,
      -1,   910,    -1,    -1,    -1,   582,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   925,    -1,    -1,   928,
      -1,    -1,   931,   932,   933,   934,   935,   936,   937,   938,
     939,   940,   941,   942,   943,   944,   945,   946,   947,   948,
     949,   950,   951,   952,   953,   954,   955,   956,   957,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     637,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   981,    -1,    -1,   984,    -1,    -1,    -1,    -1,
      -1,   215,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    -1,    -1,   673,    -1,   675,    -1,
      -1,  1010,    -1,  1012,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1020,    -1,    -1,   202,    -1,    -1,    -1,    55,   696,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1037,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      77,    -1,    -1,   277,    -1,    -1,    -1,    -1,    -1,  1058,
     284,   285,    -1,    -1,  1063,    -1,    -1,   291,    -1,    -1,
      -1,    -1,    -1,   297,    -1,    -1,    -1,    -1,  1077,  1078,
      -1,  1080,    -1,    -1,   308,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1092,    -1,   762,  1095,    -1,  1097,    -1,
      -1,    -1,    -1,   130,   131,    -1,   773,   774,    -1,    -1,
      -1,    -1,    -1,    -1,  1113,    -1,    -1,    -1,    -1,    -1,
      -1,   148,    -1,    -1,   151,   152,    -1,   154,   155,   796,
     157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   172,    -1,    -1,   372,    -1,
      -1,  1150,    -1,    -1,  1153,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,    -1,
      -1,   198,    10,    11,    12,   202,    -1,  1176,    -1,    -1,
      -1,    -1,   849,    -1,    -1,    -1,    -1,    77,   412,    79,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   892,   893,    65,    -1,    -1,
      -1,    -1,    -1,  1232,    -1,  1234,    -1,    -1,    -1,    -1,
    1239,  1240,   466,    -1,  1243,    -1,  1245,    -1,    -1,  1248,
      -1,    -1,    -1,  1252,    -1,    -1,  1255,  1256,    -1,  1258,
      -1,   928,    -1,    -1,   154,   155,  1265,   157,   158,   159,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1280,    -1,    -1,    -1,    -1,    -1,   511,    -1,    -1,
      -1,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    -1,  1305,    -1,    -1,    -1,
      -1,   201,    -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1320,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1330,  1331,    -1,    -1,    -1,    52,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,  1020,    -1,    -1,  1355,    -1,   582,    -1,
    1359,    -1,    -1,    -1,   202,    29,  1365,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,  1058,    -1,    10,    11,    12,  1063,  1396,  1397,    -1,
      -1,    65,    -1,  1402,  1403,    -1,    -1,    -1,  1407,    -1,
    1077,  1078,    29,   637,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,   673,
      -1,   675,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   696,   697,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1153,   711,   712,   713,
     714,   715,    -1,    -1,    -1,    -1,    -1,   721,    -1,    -1,
      -1,    63,    64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1510,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   745,  1521,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   249,   250,    -1,   200,    -1,   762,   255,
    1539,    -1,    -1,    -1,  1543,    -1,    -1,   771,    -1,   773,
     774,    -1,    -1,    -1,    -1,  1554,    -1,    -1,    -1,    -1,
    1559,    -1,    -1,  1562,   788,   127,    -1,    -1,    -1,    -1,
      -1,    -1,   796,  1240,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   202,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      -1,    -1,   826,    -1,    -1,   829,    -1,    -1,    -1,    -1,
      -1,    -1,   328,    -1,    -1,   331,    -1,    -1,    -1,    -1,
      -1,  1620,    -1,    -1,    -1,   849,    -1,    -1,    -1,  1628,
      -1,    -1,    -1,    -1,    -1,    63,    64,    -1,  1305,    -1,
      -1,    -1,    -1,  1642,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1651,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1666,   892,   893,
      -1,    -1,    -1,    -1,    -1,    -1,  1675,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1683,    -1,    -1,    -1,  1687,    -1,
      -1,  1690,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   127,
      -1,   925,    -1,    -1,   928,    -1,    -1,   931,   932,   933,
     934,   935,   936,   937,   938,   939,   940,   941,   942,   943,
     944,   945,   946,   947,   948,   949,   950,   951,   952,   953,
     954,   955,   956,   957,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,   462,   463,    -1,    -1,
     466,    -1,    -1,    -1,    -1,    -1,    -1,   981,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,   511,  1020,    -1,    -1,    -1,
      -1,    -1,    29,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1058,    -1,    -1,    -1,    65,  1063,
      -1,   557,   558,    -1,    -1,    -1,    -1,   563,    -1,    -1,
      -1,    -1,    -1,  1077,  1078,    -1,  1080,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1095,    -1,  1097,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1113,
      35,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1150,    -1,    -1,  1153,
      65,   202,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1176,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,   117,    53,    -1,   202,    -1,    -1,    -1,    -1,
      -1,   697,    -1,    -1,   129,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   709,   710,   711,   712,   713,   714,   715,
      -1,    -1,    -1,   148,    -1,   721,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,  1239,  1240,    -1,    -1,  1243,
      -1,  1245,    -1,    -1,  1248,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1256,    -1,  1258,    -1,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      -1,    -1,    -1,   198,    -1,    -1,  1280,    -1,   203,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   202,    -1,    -1,
      -1,   787,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1305,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   807,    10,    11,    12,    -1,  1320,    -1,    -1,    -1,
      -1,    -1,   818,    -1,    -1,    -1,  1330,  1331,    -1,    -1,
     826,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,    -1,
      -1,  1365,    -1,    -1,    -1,    -1,    -1,    65,    29,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,  1396,  1397,    55,    -1,    -1,    -1,  1402,  1403,
      -1,    -1,    -1,  1407,    -1,    65,    -1,    -1,    -1,    -1,
     906,    -1,   908,    -1,   910,    -1,    77,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   925,
      -1,    -1,    -1,    -1,    -1,   931,   932,   933,   934,   935,
     936,   937,   938,   939,   940,   941,   942,   943,   944,   945,
     946,   947,   948,   949,   950,   951,   952,   953,   954,   955,
     956,   957,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,
     131,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   981,    -1,   148,   984,    -1,
     151,   152,    -1,   154,   155,    -1,   157,   158,   159,    -1,
     161,    -1,    -1,    -1,   202,    -1,  1510,    -1,    -1,    -1,
      -1,   172,    -1,    -1,  1010,    -1,  1012,    -1,    -1,    -1,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    -1,    -1,    -1,   198,    -1,  1543,
      -1,  1037,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
    1554,    -1,    -1,    -1,    -1,  1559,    -1,    -1,  1562,    -1,
      -1,    29,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,  1080,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1092,    65,    -1,  1095,
      -1,  1097,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1620,  1113,    10,    11,
      12,    -1,    -1,    -1,  1628,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,  1642,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,  1675,    -1,    65,    -1,    -1,    -1,    -1,    -1,  1683,
    1176,    -1,    -1,  1687,    -1,    29,  1690,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   202,    -1,  1232,    -1,  1234,    -1,
      -1,    -1,    -1,  1239,    -1,    -1,    -1,  1243,    -1,  1245,
      -1,    -1,  1248,    -1,    -1,    -1,  1252,    -1,    -1,  1255,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,  1265,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,  1280,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    -1,    -1,
     202,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,  1320,    -1,    -1,    -1,    -1,    -1,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    -1,    -1,    -1,   202,  1355,
      -1,    -1,    -1,  1359,    -1,    -1,    65,     5,     6,  1365,
       8,     9,    10,    11,    12,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    -1,    -1,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1396,  1397,    -1,    41,    -1,    -1,  1402,    10,    11,    12,
      48,    -1,    50,    -1,    -1,    53,    -1,    55,   466,    -1,
      -1,    -1,    -1,    -1,    -1,    77,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,   202,    10,    11,    12,
      -1,    -1,    65,   511,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   112,    -1,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,   151,
      53,   200,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,    -1,    65,    -1,  1510,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1521,    -1,    -1,    -1,    77,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,  1539,    -1,    -1,    -1,  1543,   186,   201,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,  1554,    -1,
      -1,    -1,    -1,  1559,    -1,    -1,  1562,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,   230,    53,    -1,   233,    -1,   200,    -1,    -1,
      -1,    -1,   240,   241,   152,    65,   154,   155,   156,   157,
     158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1620,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   200,   286,   697,
     198,    -1,    -1,    -1,   292,  1651,    -1,    -1,   296,    -1,
      -1,    -1,    -1,   711,   712,   713,   714,   715,    -1,    -1,
    1666,   309,   310,   721,    -1,    -1,    -1,    -1,    -1,  1675,
      -1,   319,    -1,    -1,    -1,    -1,    -1,  1683,    -1,    -1,
      -1,  1687,   330,    -1,  1690,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    77,    -1,    79,    -1,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,    -1,   373,    -1,   375,   376,   199,
     378,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   386,   387,
     388,   389,   390,   391,   392,   393,   394,   395,   396,   397,
     398,    -1,    -1,    -1,    -1,    -1,   404,   405,    -1,   407,
     408,   409,    -1,    -1,    -1,    -1,    -1,   415,   826,   466,
     418,    -1,   154,   155,    -1,   157,   158,   159,    -1,   427,
      -1,   429,    -1,    -1,    -1,    -1,    -1,   435,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   445,    -1,   447,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    -1,   511,    10,    11,    12,    -1,   201,
      -1,   203,    -1,    -1,    -1,   473,    -1,    -1,   476,   477,
     478,    -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,   507,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   925,    -1,    -1,
      65,    -1,    -1,   931,   932,   933,   934,   935,   936,   937,
     938,   939,   940,   941,   942,   943,   944,   945,   946,   947,
     948,   949,   950,   951,   952,   953,   954,   955,   956,   957,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,   981,    53,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,   583,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,   595,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   627,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,   636,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     697,    -1,    29,    -1,    -1,   200,   654,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   711,   712,   713,   714,   715,    -1,
      -1,    -1,  1080,    -1,   721,    -1,    -1,    -1,    55,    -1,
      -1,    -1,    -1,   681,    -1,    -1,    -1,  1095,    -1,  1097,
      10,    11,    12,    -1,    -1,    -1,   694,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    -1,  1113,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,   730,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   200,  1176,    -1,
      -1,   148,    -1,    -1,   151,   152,    -1,   154,   155,   826,
     157,   158,   159,    -1,   782,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   172,    -1,   795,    -1,    -1,
      -1,    -1,   800,    -1,   802,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,    -1,
      -1,   198,    29,   466,    -1,   202,   824,    -1,    -1,    -1,
      -1,  1239,    -1,    -1,    -1,  1243,   834,  1245,    -1,   837,
    1248,   839,    -1,    -1,    -1,   843,    -1,    -1,    55,    -1,
      -1,    -1,    -1,    -1,    -1,   853,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   511,    -1,
      77,    -1,  1280,    -1,    -1,    -1,    -1,    -1,   925,   199,
     878,    -1,    -1,    -1,   931,   932,   933,   934,   935,   936,
     937,   938,   939,   940,   941,   942,   943,   944,   945,   946,
     947,   948,   949,   950,   951,   952,   953,   954,   955,   956,
     957,    -1,  1320,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   130,   131,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   981,    -1,    -1,    -1,    -1,    -1,
      -1,   148,    -1,    -1,   151,   152,    -1,   154,   155,    -1,
     157,   158,   159,    -1,   161,    -1,    -1,  1365,    -1,    -1,
      -1,   959,   960,   961,    -1,   172,    -1,   965,   966,    -1,
      -1,    -1,    -1,    -1,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,  1396,  1397,
      -1,   198,    -1,    -1,  1402,   993,    -1,    -1,  1406,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,  1016,    -1,
      -1,    -1,    -1,  1021,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,  1080,    -1,    -1,  1034,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1043,    -1,  1045,  1095,    -1,
    1097,    -1,    -1,    -1,   697,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1113,    -1,   711,   712,
     713,   714,    -1,    -1,    -1,    -1,  1074,    -1,   721,    -1,
      -1,    -1,    10,    11,    12,    -1,  1084,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    29,  1510,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,  1176,
      74,    75,    76,    77,    -1,  1543,    -1,    65,    67,    68,
      -1,    -1,    -1,    -1,    -1,    -1,  1554,    -1,    77,    -1,
      79,  1559,    -1,    -1,  1562,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1163,    -1,    -1,    -1,  1167,
      -1,  1169,  1170,    -1,    -1,    -1,    -1,  1585,    -1,    -1,
      -1,    -1,  1180,   826,    -1,    -1,    -1,    -1,   117,    -1,
    1188,  1189,  1239,    -1,    -1,    -1,  1243,    -1,  1245,    -1,
    1198,  1248,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     154,   155,  1620,   157,   158,   159,    -1,    -1,    -1,   148,
      -1,    -1,   151,   152,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    -1,  1280,    -1,    -1,    -1,   166,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,  1249,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   194,  1675,    -1,   198,
      -1,    -1,    -1,  1320,   203,  1683,    -1,    -1,    -1,  1687,
      -1,    -1,  1690,    -1,    -1,  1283,    -1,    -1,   931,   932,
     933,   934,   935,   936,   937,   938,   939,   940,   941,   942,
     943,   944,   945,   946,   947,   948,   949,   950,   951,   952,
     953,   954,   955,   956,   957,    -1,    -1,    -1,  1365,    29,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,   981,    53,
      -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,  1396,
    1397,    65,    -1,    -1,    -1,  1402,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,
    1368,    -1,    -1,    -1,    -1,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
    1398,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     130,   131,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1080,   148,    -1,
      -1,   151,   152,    -1,   154,   155,    -1,   157,   158,   159,
      -1,    -1,  1095,    -1,  1097,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   172,  1510,    -1,    -1,    -1,    -1,    -1,    -1,
    1113,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    -1,    -1,    -1,   198,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1543,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    77,  1554,    79,    -1,
      -1,    -1,  1559,    -1,    -1,  1562,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1176,    -1,    -1,   190,   191,    29,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1620,    65,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   154,   155,    -1,   157,   158,   159,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1239,    -1,    -1,    -1,
    1243,    -1,  1245,    -1,    77,  1248,    -1,    -1,    -1,    -1,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    -1,    10,    11,    12,  1675,    -1,
     201,    -1,   203,    -1,    -1,    -1,  1683,  1280,    -1,    -1,
    1687,    -1,    -1,  1690,    29,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1320,    -1,    -1,
      65,   154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   187,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,  1365,    -1,    29,   198,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      -1,    -1,    -1,  1396,  1397,    -1,    -1,    -1,    -1,  1402,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      28,   186,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,
      -1,    -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    69,    70,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,
      88,    -1,    90,    -1,    92,    -1,    94,  1510,    -1,    97,
      -1,    -1,    -1,   101,   102,   103,   104,   105,   106,   107,
     185,   109,   110,   111,   112,   113,   114,   115,   116,   117,
      -1,   119,   120,   121,   122,   123,   124,    -1,    -1,    -1,
    1543,   129,   130,    -1,   132,   133,   134,   135,   136,    -1,
      -1,  1554,    -1,    -1,    -1,   143,  1559,    -1,    -1,  1562,
     148,   149,   150,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,   160,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,   175,    -1,   177,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,   200,   201,   202,   203,   204,  1620,   206,   207,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1675,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1683,    45,    46,    47,  1687,    -1,    -1,  1690,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    -1,    -1,
      -1,    85,    86,    87,    88,    -1,    90,    -1,    92,    -1,
      94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,   103,
     104,   105,   106,   107,    -1,   109,   110,   111,   112,   113,
     114,   115,   116,   117,    -1,   119,   120,   121,   122,   123,
     124,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,
     134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,   160,    -1,    -1,   163,
      -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,
     174,   175,    -1,   177,    -1,   179,   180,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,   198,    -1,   200,   201,   202,   203,
     204,    -1,   206,   207,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,
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
      -1,    -1,    -1,   172,   173,   174,   175,    -1,   177,    -1,
     179,   180,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,   198,
      -1,   200,   201,    -1,   203,   204,    -1,   206,   207,     3,
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
     174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,   198,    -1,   200,   201,   202,   203,
     204,    -1,   206,   207,     3,     4,     5,     6,     7,    -1,
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
      -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,
     179,   180,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,   198,
      -1,   200,   201,   202,   203,   204,    -1,   206,   207,     3,
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
     174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,   198,    -1,   200,   201,   202,   203,
     204,    -1,   206,   207,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,
      -1,    -1,    -1,    52,    -1,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,    -1,    81,    -1,    -1,    -1,    85,    86,    87,    88,
      89,    90,    -1,    92,    -1,    94,    -1,    -1,    97,    -1,
      -1,    -1,   101,   102,   103,   104,    -1,   106,   107,    -1,
     109,    -1,   111,   112,   113,   114,   115,   116,   117,    -1,
     119,   120,   121,    -1,   123,   124,    -1,    -1,    -1,    -1,
     129,   130,    -1,   132,   133,   134,   135,   136,    -1,    -1,
      -1,    -1,    -1,    -1,   143,    -1,    -1,    -1,    -1,   148,
     149,   150,   151,   152,    -1,   154,   155,    -1,   157,   158,
     159,   160,    -1,    -1,   163,    -1,    -1,   166,    -1,    -1,
      -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,
     179,   180,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,   198,
      -1,   200,   201,    -1,   203,   204,    -1,   206,   207,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    -1,    -1,
      -1,    85,    86,    87,    88,    -1,    90,    -1,    92,    -1,
      94,    95,    -1,    97,    -1,    -1,    -1,   101,   102,   103,
     104,    -1,   106,   107,    -1,   109,    -1,   111,   112,   113,
     114,   115,   116,   117,    -1,   119,   120,   121,    -1,   123,
     124,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,
     134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,   160,    -1,    -1,   163,
      -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,
     174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,   198,    -1,   200,   201,    -1,   203,
     204,    -1,   206,   207,     3,     4,     5,     6,     7,    -1,
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
      -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,
     179,   180,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,   198,
      -1,   200,   201,   202,   203,   204,    -1,   206,   207,     3,
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
     174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,   198,    -1,   200,   201,   202,   203,
     204,    -1,   206,   207,     3,     4,     5,     6,     7,    -1,
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
      -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,
     179,   180,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,   198,
      -1,   200,   201,    -1,   203,   204,    -1,   206,   207,     3,
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
     174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,   198,    -1,   200,   201,   202,   203,
     204,    -1,   206,   207,     3,     4,     5,     6,     7,    -1,
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
      -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,
     179,   180,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,   198,
      -1,   200,   201,   202,   203,   204,    -1,   206,   207,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,
      74,    75,    76,    77,    78,    79,    -1,    81,    -1,    -1,
      -1,    85,    86,    87,    88,    -1,    90,    91,    92,    -1,
      94,    -1,    -1,    97,    -1,    -1,    -1,   101,   102,   103,
     104,    -1,   106,   107,    -1,   109,    -1,   111,   112,   113,
     114,   115,   116,   117,    -1,   119,   120,   121,    -1,   123,
     124,    -1,    -1,    -1,    -1,   129,   130,    -1,   132,   133,
     134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,   143,
      -1,    -1,    -1,    -1,   148,   149,   150,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,   160,    -1,    -1,   163,
      -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,   172,   173,
     174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,   198,    -1,   200,   201,    -1,   203,
     204,    -1,   206,   207,     3,     4,     5,     6,     7,    -1,
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
      -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,
     179,   180,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,   198,
      -1,   200,   201,   202,   203,   204,    -1,   206,   207,     3,
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
     174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,   198,    -1,   200,   201,   202,   203,
     204,    -1,   206,   207,     3,     4,     5,     6,     7,    -1,
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
      -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,
     179,   180,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,   198,
      -1,   200,   201,   202,   203,   204,    -1,   206,   207,     3,
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
     174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,   198,    -1,   200,   201,    -1,   203,
     204,    -1,   206,   207,     3,     4,     5,     6,     7,    -1,
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
      -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,
     179,   180,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,   198,
      -1,   200,   201,    -1,   203,   204,    -1,   206,   207,     3,
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
     174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,   198,    -1,   200,   201,    -1,   203,
     204,    -1,   206,   207,     3,     4,     5,     6,     7,    -1,
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
      -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,
     179,   180,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,   198,
      -1,   200,   201,    -1,   203,   204,    -1,   206,   207,     3,
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
     174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,   198,    -1,   200,   201,    -1,   203,
     204,    -1,   206,   207,     3,     4,     5,     6,     7,    -1,
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
      -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,
     179,   180,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,   198,
      -1,   200,   201,    -1,   203,   204,    -1,   206,   207,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,
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
     174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,   198,    -1,   200,   201,    -1,   203,
     204,    -1,   206,   207,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,
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
     159,    -1,    -1,    -1,   163,    -1,    -1,   166,    -1,    -1,
      -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,    -1,    -1,   198,
      -1,    -1,    -1,    -1,   203,   204,    -1,   206,   207,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     174,    -1,    -1,    13,    -1,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,    -1,    -1,   198,    35,   200,    -1,    -1,   203,
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
      -1,   161,    -1,   163,    -1,    -1,   166,     3,     4,     5,
       6,     7,   172,   173,   174,    -1,    -1,    13,    -1,   179,
     180,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,    -1,    -1,   198,    35,
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
      -1,   157,   158,   159,    -1,   161,    -1,   163,    -1,    -1,
     166,    -1,    -1,    -1,    -1,    -1,   172,   173,   174,    -1,
      -1,    -1,    -1,   179,   180,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
      -1,    -1,   198,    -1,    -1,    -1,    -1,   203,   204,    -1,
     206,   207,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
       7,   172,   173,   174,    -1,    -1,    13,    -1,   179,   180,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,    -1,   198,    -1,    -1,
      -1,    -1,   203,   204,    -1,   206,   207,    -1,    -1,    46,
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
      -1,   200,    -1,    -1,   203,   204,    -1,   206,   207,    -1,
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
      -1,    -1,   163,    -1,    -1,   166,    -1,    -1,    -1,    -1,
      -1,   172,   173,   174,    -1,    -1,    -1,    -1,   179,   180,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,    -1,    -1,   198,   199,    -1,
      -1,    -1,   203,   204,    -1,   206,   207,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    30,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
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
     194,   195,    -1,    -1,   198,    -1,    -1,    -1,    -1,   203,
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
     190,   191,   192,   193,   194,   195,    -1,    -1,   198,    35,
      -1,   201,    -1,   203,   204,    -1,   206,   207,    -1,    -1,
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
      -1,    -1,   198,    -1,    -1,    -1,    -1,   203,   204,    -1,
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
     158,   159,    -1,    -1,    -1,   163,    -1,    -1,   166,    -1,
      -1,    -1,    -1,    -1,   172,   173,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,    -1,    -1,
     198,    -1,    -1,    -1,    -1,   203,   204,    -1,   206,   207,
       3,     4,     5,     6,     7,    -1,    -1,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    28,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    67,    68,    69,    70,    71,    72,
      73,    -1,    -1,    -1,    77,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    -1,    -1,   129,   130,    -1,   132,
     133,   134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    29,   148,   149,   150,    -1,    -1,
      -1,   154,   155,    -1,   157,   158,   159,   160,    -1,   162,
     163,    -1,   165,    -1,    -1,    -1,    -1,    -1,    -1,   172,
      55,    -1,   175,    -1,   177,    -1,   179,   180,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    77,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    29,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
      -1,    -1,    65,    -1,    -1,    -1,    -1,   112,   113,   114,
     115,   116,   117,    -1,    -1,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   130,   131,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,
      -1,    -1,    -1,   148,    -1,    -1,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   104,    -1,    -1,   172,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   180,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     130,   131,    -1,   198,    -1,    -1,    77,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,
      -1,   151,   152,    -1,   154,   155,    -1,   157,   158,   159,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   172,    -1,    -1,    -1,   117,    -1,    -1,    30,
      -1,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    46,    47,    -1,   198,    -1,
      -1,    52,    -1,    54,    -1,    -1,    -1,   148,    -1,    -1,
     151,   152,    -1,   154,   155,    66,   157,   158,   159,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    -1,    -1,    -1,   198,    -1,    -1,
     201,    -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,
      -1,   132,   133,   134,   135,   136,    -1,    -1,    77,    -1,
      79,    -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,
     151,   152,    -1,   154,   155,    -1,   157,   158,   159,    -1,
      77,    -1,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   172,    -1,    -1,    -1,    -1,    -1,    -1,   179,    -1,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    46,    47,    -1,   198,    -1,    -1,
      52,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    66,   154,   155,    -1,   157,   158,
     159,    -1,    74,    75,    76,    77,    -1,    -1,    -1,    -1,
      -1,   148,    -1,    85,   151,    -1,    -1,   154,   155,    -1,
     157,   158,   159,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,    -1,    -1,    -1,    -1,
      -1,    -1,   201,    -1,   203,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   130,    -1,
     132,   133,   134,   135,   136,   202,    -1,    77,    -1,    -1,
      -1,   143,    -1,    -1,    -1,    -1,   148,   149,   150,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     172,    -1,    -1,    46,    47,    -1,    -1,   179,    -1,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    66,    -1,    -1,   198,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    85,    -1,   154,   155,    -1,   157,   158,   159,
      77,    -1,    79,    -1,    -1,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,    65,    -1,   130,   198,    -1,
     117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   154,   155,    -1,   157,   158,   159,    -1,    77,    -1,
      -1,   148,    -1,    -1,   151,   152,    85,   154,   155,   172,
     157,   158,   159,    77,    -1,    79,    -1,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,    -1,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    -1,    -1,
      -1,   198,    -1,   117,    -1,    -1,   203,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   129,    -1,    -1,    -1,   148,
      -1,    -1,   151,    -1,    -1,   154,   155,    -1,   157,   158,
     159,    -1,    -1,    -1,   148,    -1,    -1,   151,   152,    -1,
     154,   155,    -1,   157,   158,   159,    77,    -1,    79,    -1,
      -1,    -1,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,    -1,    -1,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,    -1,    -1,    -1,   198,    -1,   117,    -1,    -1,   203,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,    77,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   148,    -1,    -1,
     151,   152,    -1,   154,   155,    -1,   157,   158,   159,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   117,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    -1,    -1,    -1,   198,    -1,    -1,
     148,    -1,   203,   151,   152,    -1,   154,   155,    -1,   157,
     158,   159,    77,    -1,    79,    -1,    -1,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,    65,    -1,    -1,
     198,    -1,   117,    -1,    -1,   203,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    77,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   148,    -1,    -1,   151,   152,    -1,   154,
     155,    -1,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   117,    77,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
      -1,    -1,    -1,   198,    -1,    -1,   148,    -1,   203,   151,
     152,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,    -1,   123,    -1,    -1,    -1,    77,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   154,   155,    -1,   157,   158,   159,    -1,
      -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    -1,    -1,    -1,    -1,    -1,    -1,
     201,    -1,   203,   154,   155,    -1,   157,   158,   159,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    -1,    -1,    29,   198,    31,    32,
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
      -1,    -1,    -1,    -1,    -1,    -1,    65,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   128,    29,    -1,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    29,   128,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    77,    -1,    -1,    65,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      77,    -1,    -1,    -1,    29,   128,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,   118,    -1,    -1,    -1,    77,    -1,   128,    -1,    -1,
      -1,    -1,   148,   130,   131,   151,   152,    -1,   154,   155,
      -1,   157,   158,   159,    77,    -1,    79,    80,    -1,    -1,
      -1,   148,   104,   105,   151,   152,    -1,   154,   155,    -1,
     157,   158,   159,    -1,    -1,    -1,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,    -1,
      -1,    -1,    -1,   128,    -1,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,    77,   151,
      -1,    -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,
      -1,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,    -1,    -1,    -1,    -1,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    77,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   152,    -1,   154,   155,    77,   157,   158,
     159,    -1,    -1,    -1,    -1,   151,    -1,    -1,   154,   155,
      -1,   157,   158,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,    -1,
      -1,    -1,    -1,    -1,    77,    -1,   151,    -1,    -1,   154,
     155,    -1,   157,   158,   159,    -1,    -1,    -1,    -1,    77,
      -1,   151,    -1,    -1,   154,   155,    -1,   157,   158,   159,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     123,    -1,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   123,    77,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   154,   155,    -1,   157,   158,   159,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   154,   155,    -1,   157,
     158,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,    -1,    -1,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,    -1,    -1,    -1,
      -1,    -1,    -1,   154,   155,    -1,   157,   158,   159,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,    28,    29,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    53,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    96,    53,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    53,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    29,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      29,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    -1,    53,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    12,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      -1,    -1,    -1,    29,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    65,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65
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
     357,   359,   360,   362,   381,   391,   392,   393,   398,   401,
     419,   424,   426,   427,   428,   429,   430,   431,   432,   433,
     435,   450,   452,   454,   115,   116,   117,   129,   148,   158,
     215,   246,   323,   339,   426,   339,   198,   339,   339,   339,
     101,   339,   339,   417,   418,   339,   339,   339,   339,   339,
     339,   339,   339,   339,   339,   339,   339,    79,   117,   198,
     223,   392,   393,   426,   426,    35,   339,   439,   440,   339,
     117,   198,   223,   392,   393,   394,   425,   431,   436,   437,
     198,   330,   395,   198,   330,   346,   331,   339,   232,   330,
     198,   198,   198,   330,   200,   339,   215,   200,   339,    29,
      55,   130,   131,   152,   172,   198,   215,   226,   455,   466,
     467,   181,   200,   336,   339,   361,   363,   201,   239,   339,
     104,   105,   151,   216,   219,   222,    79,   203,   291,   292,
     123,   123,    79,   293,   198,   198,   198,   198,   215,   263,
     456,   198,   198,    79,    84,   144,   145,   146,   447,   448,
     151,   201,   222,   222,   215,   264,   456,   152,   198,   198,
     198,   456,   456,   347,   329,   339,   340,   426,   228,   201,
      84,   396,   447,    84,   447,   447,    30,   151,   168,   457,
     198,     9,   200,    35,   245,   152,   262,   456,   117,   246,
     324,   200,   200,   200,   200,   200,   200,    10,    11,    12,
      29,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    53,    65,   200,    66,    66,   200,   201,   147,
     124,   158,   160,   265,   322,   323,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    63,
      64,   127,   421,   422,    66,   201,   423,   198,    66,   201,
     203,   432,   198,   245,   246,    14,   339,   200,   128,    44,
     215,   416,   198,   329,   426,   147,   426,   128,   205,     9,
     403,   329,   426,   457,   147,   198,   397,   127,   421,   422,
     423,   199,   339,    30,   230,     8,   348,     9,   200,   230,
     231,   331,   332,   339,   215,   277,   234,   200,   200,   200,
     467,   467,   168,   198,   104,   467,    14,   215,    79,   200,
     200,   200,   181,   182,   183,   188,   189,   192,   193,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   376,   377,
     378,   240,   108,   165,   200,   151,   217,   220,   222,   151,
     218,   221,   222,   222,     9,   200,    96,   201,   426,     9,
     200,    14,     9,   200,   426,   451,   451,   329,   340,   426,
     199,   168,   257,   129,   426,   438,   439,    66,   127,   144,
     448,    78,   339,   426,    84,   144,   448,   222,   214,   200,
     201,   252,   260,   382,   384,    85,   203,   349,   350,   352,
     393,   432,   452,   339,   445,   446,   445,    14,    96,   453,
     287,   288,   419,   420,   199,   199,   199,   202,   229,   230,
     247,   254,   259,   419,   339,   204,   206,   207,   215,   458,
     459,   467,    35,   161,   289,   290,   339,   455,   198,   456,
     255,   245,   339,   339,   339,    30,   339,   339,   339,   339,
     339,   339,   339,   339,   339,   339,   339,   339,   339,   339,
     339,   339,   339,   339,   339,   339,   339,   339,   394,   339,
     339,   434,   434,   339,   441,   442,   123,   201,   215,   431,
     432,   263,   215,   264,   262,   246,    27,    35,   333,   336,
     339,   361,   339,   339,   339,   339,   339,   339,   339,   339,
     339,   339,   339,   339,   201,   215,   431,   434,   339,   289,
     434,   339,   438,   245,   199,   339,   198,   415,     9,   403,
     329,   199,   215,    35,   339,    35,   339,   199,   199,   431,
     289,   201,   215,   431,   199,   228,   281,   201,   339,   339,
      88,    30,   230,   275,   200,    28,    96,    14,     9,   199,
      30,   201,   278,   467,    85,   226,   463,   464,   465,   198,
       9,    46,    47,    52,    54,    66,    85,   130,   143,   152,
     172,   198,   223,   224,   226,   358,   392,   398,   399,   400,
     184,    79,   339,    79,    79,   339,   373,   374,   339,   339,
     366,   376,   187,   379,   228,   198,   238,   222,   200,     9,
      96,   222,   200,     9,    96,    96,   219,   215,   339,   292,
     399,    79,     9,   199,   199,   199,   199,   199,   200,   215,
     462,   125,   268,   198,     9,   199,   199,    79,    80,   215,
     449,   215,    66,   202,   202,   211,   213,    30,   126,   267,
     167,    50,   152,   167,   386,   128,     9,   403,   199,   147,
     128,   199,     9,   403,   199,   467,   467,    14,   196,     9,
     404,   467,   468,   127,   421,   422,   423,   202,     9,   403,
     169,   426,   339,   199,     9,   404,    14,   343,   248,   125,
     266,   198,   456,   339,    30,   205,   205,   128,   202,     9,
     403,   339,   457,   198,   258,   253,   261,   256,   245,    68,
     426,   339,   457,   198,   205,   202,   199,   205,   202,   199,
      46,    47,    66,    74,    75,    76,    85,   130,   143,   172,
     215,   406,   408,   411,   414,   215,   426,   426,   128,   421,
     422,   423,   199,   339,   282,    71,    72,   283,   228,   330,
     228,   332,    96,    35,   129,   272,   426,   399,   215,    30,
     230,   276,   200,   279,   200,   279,     9,   169,   128,   147,
       9,   403,   199,   161,   458,   459,   460,   458,   399,   399,
     399,   399,   399,   402,   405,   198,    84,   147,   198,   399,
     147,   201,    10,    11,    12,    29,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    65,   339,   184,
     184,    14,   190,   191,   375,     9,   194,   379,    79,   202,
     392,   201,   242,    96,   220,   215,    96,   221,   215,   215,
     202,    14,   426,   200,    96,     9,   169,   269,   392,   201,
     438,   129,   426,    14,   205,   339,   202,   211,   467,   269,
     201,   385,    14,   339,   349,   215,   339,   339,   200,   467,
      30,   461,   420,    35,    79,   161,   201,   215,   431,   467,
      35,   161,   339,   399,   287,   198,   392,   267,   344,   249,
     339,   339,   339,   202,   198,   289,   268,    30,   267,   266,
     456,   394,   202,   198,   289,    14,    74,    75,    76,   215,
     407,   407,   408,   409,   410,   198,    84,   144,   198,     9,
     403,   199,   415,    35,   339,   202,    71,    72,   284,   330,
     230,   202,   200,    89,   200,   272,   426,   198,   128,   271,
      14,   228,   279,    98,    99,   100,   279,   202,   467,   467,
     215,   463,     9,   199,   403,   128,   205,     9,   403,   402,
     215,   349,   351,   353,   199,   123,   215,   399,   443,   444,
     399,   399,   399,    30,   399,   399,   399,   399,   399,   399,
     399,   399,   399,   399,   399,   399,   399,   399,   399,   399,
     399,   399,   399,   399,   399,   399,   399,   339,   339,   339,
     374,   339,   364,    79,   243,   215,   215,   399,   467,   215,
       9,   297,   199,   198,   333,   336,   339,   205,   202,   453,
     297,   153,   166,   201,   381,   388,   153,   201,   387,   128,
     128,   200,   467,   348,   468,    79,    14,    79,   339,   457,
     198,   426,   339,   199,   287,   201,   287,   198,   128,   198,
     289,   199,   201,   467,   201,   267,   250,   397,   198,   289,
     199,   128,   205,     9,   403,   409,   144,   349,   412,   413,
     408,   426,   330,    30,    73,   230,   200,   332,   271,   438,
     272,   199,   399,    95,    98,   200,   339,    30,   200,   280,
     202,   169,   128,   161,    30,   199,   399,   399,   199,   128,
       9,   403,   199,   128,   202,     9,   403,   399,    30,   185,
     199,   228,    96,   392,     4,   105,   110,   118,   154,   155,
     157,   202,   298,   321,   322,   323,   328,   419,   438,   202,
     201,   202,    50,   339,   339,   339,   339,    35,    79,   161,
      14,   399,   202,   198,   289,   461,   199,   297,   199,   287,
     339,   289,   199,   297,   453,   297,   201,   198,   289,   199,
     408,   408,   199,   128,   199,     9,   403,    30,   228,   200,
     199,   199,   199,   235,   200,   200,   280,   228,   467,   467,
     128,   399,   349,   399,   399,   399,   339,   201,   202,   467,
     125,   126,   455,   270,   392,   118,   130,   131,   152,   158,
     307,   308,   309,   392,   156,   313,   314,   121,   198,   215,
     315,   316,   299,   246,   467,     9,   200,   322,   199,   294,
     152,   383,   202,   202,    79,    14,    79,   399,   198,   289,
     199,   110,   341,   461,   202,   461,   199,   199,   202,   201,
     202,   297,   287,   199,   128,   408,   349,   228,   233,   236,
      30,   230,   274,   228,   199,   399,   128,   128,   186,   228,
     392,   392,    14,     9,   200,   201,   201,     9,   200,     3,
       4,     5,     6,     7,    10,    11,    12,    13,    27,    28,
      53,    67,    68,    69,    70,    71,    72,    73,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   129,
     130,   132,   133,   134,   135,   136,   148,   149,   150,   160,
     162,   163,   165,   172,   175,   177,   179,   180,   215,   389,
     390,     9,   200,   152,   156,   215,   316,   317,   318,   200,
      79,   327,   245,   300,   455,   246,   202,   295,   296,   455,
      14,   399,   289,   199,   198,   201,   200,   201,   319,   341,
     461,   294,   202,   199,   408,   128,    30,   230,   273,   274,
     228,   399,   399,   339,   202,   200,   200,   399,   392,   303,
     310,   398,   308,    14,    30,    47,   311,   314,     9,    33,
     199,    29,    46,    49,    14,     9,   200,   456,   327,    14,
     245,   200,    14,   399,   199,    35,    79,   380,   228,   228,
     201,   319,   202,   461,   408,   228,    93,   187,   241,   202,
     215,   226,   304,   305,   306,     9,   202,   399,   390,   390,
      55,   312,   317,   317,    29,    46,    49,   399,    79,   198,
     200,   399,   456,   399,    79,     9,   404,   202,   202,   228,
     319,    91,   200,    79,   108,   237,   147,    96,   398,   159,
      14,   301,   198,    35,    79,   199,   202,   200,   198,   165,
     244,   215,   322,   323,   399,   285,   286,   420,   302,    79,
     392,   242,   162,   215,   200,   199,     9,   404,   112,   113,
     114,   325,   326,   285,    79,   270,   200,   461,   420,   468,
     199,   199,   200,   200,   201,   320,   325,    35,    79,   161,
     461,   201,   228,   468,    79,    14,    79,   320,   228,   202,
      35,    79,   161,    14,   399,   202,    79,    14,    79,   399,
      14,   399,   399
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
#line 1957 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1966 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1970 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1971 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1974 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1978 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1982 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1986 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1991 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); ;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1993 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); ;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1995 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1997 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 2001 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 2003 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 2007 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 2008 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 2009 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 2010 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2011 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2012 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2016 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2020 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2024 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2029 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); ;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2034 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); ;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2038 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2042 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); ;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2043 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2047 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); ;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2048 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); ;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2052 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2053 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2057 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2058 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2062 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2066 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2070 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2074 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2075 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2076 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2077 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2084 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2087 "hphp.y"
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

  case 506:

/* Line 1455 of yacc.c  */
#line 2098 "hphp.y"
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

  case 507:

/* Line 1455 of yacc.c  */
#line 2109 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2110 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2115 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2116 "hphp.y"
    { (yyval).reset();;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2120 "hphp.y"
    { (yyval).reset();;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2123 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2127 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2133 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2140 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2141 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2147 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2149 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2153 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2154 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2155 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2156 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2159 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2160 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2161 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2162 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2163 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2164 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2165 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2166 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2167 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2168 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2169 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2170 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2171 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2173 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2174 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2175 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2176 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2177 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2178 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2179 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2181 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2182 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2183 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2184 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2185 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2186 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2187 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2190 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2191 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2193 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2194 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2195 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2199 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2200 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2201 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2204 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2205 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2207 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2210 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2211 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2213 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2214 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2215 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2216 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2219 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2221 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2222 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2227 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2236 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { (yyval).reset();;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval).reset();;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval).reset();;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { (yyval).reset();;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_POW);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2337 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2344 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { (yyval).reset();;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { (yyval).reset();;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { (yyval).reset();;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { (yyval).reset();;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2424 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { (yyval).reset();;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2465 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2471 "hphp.y"
    { (yyval).reset();;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2475 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2487 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2489 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2492 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2495 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2496 "hphp.y"
    { (yyval).reset();;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2500 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2502 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2506 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2507 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2511 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2512 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2516 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2518 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2523 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2529 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2531 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2532 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2533 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2534 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2536 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2539 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2541 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2542 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2546 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2547 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 2549 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 2551 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 2553 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2555 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2560 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2561 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 2562 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 2568 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2571 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 2574 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 2578 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 2582 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 2586 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 2593 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 2597 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 2601 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 2605 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 2607 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 2612 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 2613 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 2614 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 2617 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 2618 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 2621 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 2622 "hphp.y"
    { (yyval).reset();;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 2626 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 2627 "hphp.y"
    { (yyval)++;;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 2631 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 2632 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 2633 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 2635 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 2638 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 2639 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 2643 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 2645 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 2647 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 2648 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 795:

/* Line 1455 of yacc.c  */
#line 2652 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 2653 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 2655 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 2656 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 2657 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 2658 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 2663 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 2664 "hphp.y"
    { (yyval).reset();;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 2668 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 2669 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 2670 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 2671 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 807:

/* Line 1455 of yacc.c  */
#line 2674 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 2676 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 2677 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 2678 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 2683 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 2684 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 2688 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 2689 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 2690 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 2691 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 2696 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 2697 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 2702 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 2704 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 2706 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 2707 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 823:

/* Line 1455 of yacc.c  */
#line 2712 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 824:

/* Line 1455 of yacc.c  */
#line 2713 "hphp.y"
    { _p->onEmptyMapArray((yyval));;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 2717 "hphp.y"
    { _p->onMapArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 2718 "hphp.y"
    { _p->onMapArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 2722 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 828:

/* Line 1455 of yacc.c  */
#line 2724 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 2725 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 2727 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 831:

/* Line 1455 of yacc.c  */
#line 2732 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 832:

/* Line 1455 of yacc.c  */
#line 2734 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 2736 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 2738 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 2740 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 2741 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 2744 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 2745 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 2746 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 840:

/* Line 1455 of yacc.c  */
#line 2750 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 841:

/* Line 1455 of yacc.c  */
#line 2751 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 2752 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 2753 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 2754 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 2755 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 2756 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 2757 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 848:

/* Line 1455 of yacc.c  */
#line 2758 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 849:

/* Line 1455 of yacc.c  */
#line 2762 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 2763 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 2768 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 2770 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 2784 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 2788 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 2794 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 2795 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 2801 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 2805 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 861:

/* Line 1455 of yacc.c  */
#line 2811 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 862:

/* Line 1455 of yacc.c  */
#line 2812 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 863:

/* Line 1455 of yacc.c  */
#line 2816 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 864:

/* Line 1455 of yacc.c  */
#line 2819 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 865:

/* Line 1455 of yacc.c  */
#line 2825 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 866:

/* Line 1455 of yacc.c  */
#line 2830 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 867:

/* Line 1455 of yacc.c  */
#line 2831 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 868:

/* Line 1455 of yacc.c  */
#line 2832 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 869:

/* Line 1455 of yacc.c  */
#line 2833 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 870:

/* Line 1455 of yacc.c  */
#line 2837 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 871:

/* Line 1455 of yacc.c  */
#line 2838 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 872:

/* Line 1455 of yacc.c  */
#line 2843 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); ;}
    break;

  case 873:

/* Line 1455 of yacc.c  */
#line 2844 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); ;}
    break;

  case 874:

/* Line 1455 of yacc.c  */
#line 2846 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); ;}
    break;

  case 875:

/* Line 1455 of yacc.c  */
#line 2847 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 876:

/* Line 1455 of yacc.c  */
#line 2853 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 877:

/* Line 1455 of yacc.c  */
#line 2858 "hphp.y"
    { ;}
    break;

  case 880:

/* Line 1455 of yacc.c  */
#line 2870 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 881:

/* Line 1455 of yacc.c  */
#line 2872 "hphp.y"
    {;}
    break;

  case 882:

/* Line 1455 of yacc.c  */
#line 2876 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 883:

/* Line 1455 of yacc.c  */
#line 2883 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 884:

/* Line 1455 of yacc.c  */
#line 2886 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 885:

/* Line 1455 of yacc.c  */
#line 2889 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 886:

/* Line 1455 of yacc.c  */
#line 2890 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 887:

/* Line 1455 of yacc.c  */
#line 2893 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 888:

/* Line 1455 of yacc.c  */
#line 2896 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 889:

/* Line 1455 of yacc.c  */
#line 2898 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 890:

/* Line 1455 of yacc.c  */
#line 2901 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 891:

/* Line 1455 of yacc.c  */
#line 2904 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 892:

/* Line 1455 of yacc.c  */
#line 2910 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 893:

/* Line 1455 of yacc.c  */
#line 2916 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (6)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 894:

/* Line 1455 of yacc.c  */
#line 2924 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 895:

/* Line 1455 of yacc.c  */
#line 2925 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 13220 "hphp.tab.cpp"
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
#line 2928 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

