/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.5"

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

/* Line 268 of yacc.c  */
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
    if (YYID (N)) {                                                     \
      (Current).first(YYRHSLOC (Rhs, 1));                               \
      (Current).last (YYRHSLOC (Rhs, N));                               \
    } else {                                                            \
      (Current).line0 = (Current).line1 = YYRHSLOC (Rhs, 0).line1;      \
      (Current).char0 = (Current).char1 = YYRHSLOC (Rhs, 0).char1;      \
    }                                                                   \
  while (YYID (0));                                                     \
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
  while (YYID (0))

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
  while (YYID (0))

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
  while (YYID (0))

# define YYSTACK_RELOCATE_RESET(Stack_alloc, Stack)                     \
  do                                                                    \
    {                                                                   \
      YYSIZE_T yynewbytes;                                              \
      YYCOPY_RESET (&yyptr->Stack_alloc, Stack, yysize);                \
      Stack = &yyptr->Stack_alloc;                                      \
      yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
      yyptr += yynewbytes / sizeof (*yyptr);                            \
    }                                                                   \
  while (YYID (0))

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
// converting constant declaration to "define(name, value);"
// TODO: get rid of this, or pass in more info, task 3491019.

static void on_constant(Parser *_p, Token &out, Token &name, Token &value) {
  Token sname;   _p->onScalar(sname, T_CONSTANT_ENCAPSED_STRING, name);

  Token fname;   fname.setText("define");
  Token params1; _p->onCallParam(params1, NULL, sname, 0);
  Token params2; _p->onCallParam(params2, &params1, value, 0);
  Token call;    _p->onCall(call, 0, fname, params2, 0);

  _p->onExpStatement(out, call);
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


/* Line 268 of yacc.c  */
#line 650 "hphp.tab.cpp"

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
     T_SR_EQUAL = 268,
     T_SL_EQUAL = 269,
     T_XOR_EQUAL = 270,
     T_OR_EQUAL = 271,
     T_AND_EQUAL = 272,
     T_MOD_EQUAL = 273,
     T_CONCAT_EQUAL = 274,
     T_DIV_EQUAL = 275,
     T_MUL_EQUAL = 276,
     T_MINUS_EQUAL = 277,
     T_PLUS_EQUAL = 278,
     T_BOOLEAN_OR = 279,
     T_BOOLEAN_AND = 280,
     T_IS_NOT_IDENTICAL = 281,
     T_IS_IDENTICAL = 282,
     T_IS_NOT_EQUAL = 283,
     T_IS_EQUAL = 284,
     T_IS_GREATER_OR_EQUAL = 285,
     T_IS_SMALLER_OR_EQUAL = 286,
     T_SR = 287,
     T_SL = 288,
     T_INSTANCEOF = 289,
     T_UNSET_CAST = 290,
     T_BOOL_CAST = 291,
     T_OBJECT_CAST = 292,
     T_ARRAY_CAST = 293,
     T_STRING_CAST = 294,
     T_DOUBLE_CAST = 295,
     T_INT_CAST = 296,
     T_DEC = 297,
     T_INC = 298,
     T_CLONE = 299,
     T_NEW = 300,
     T_EXIT = 301,
     T_IF = 302,
     T_ELSEIF = 303,
     T_ELSE = 304,
     T_ENDIF = 305,
     T_LNUMBER = 306,
     T_DNUMBER = 307,
     T_ONUMBER = 308,
     T_STRING = 309,
     T_STRING_VARNAME = 310,
     T_VARIABLE = 311,
     T_NUM_STRING = 312,
     T_INLINE_HTML = 313,
     T_CHARACTER = 314,
     T_BAD_CHARACTER = 315,
     T_ENCAPSED_AND_WHITESPACE = 316,
     T_CONSTANT_ENCAPSED_STRING = 317,
     T_ECHO = 318,
     T_DO = 319,
     T_WHILE = 320,
     T_ENDWHILE = 321,
     T_FOR = 322,
     T_ENDFOR = 323,
     T_FOREACH = 324,
     T_ENDFOREACH = 325,
     T_DECLARE = 326,
     T_ENDDECLARE = 327,
     T_AS = 328,
     T_SWITCH = 329,
     T_ENDSWITCH = 330,
     T_CASE = 331,
     T_DEFAULT = 332,
     T_BREAK = 333,
     T_GOTO = 334,
     T_CONTINUE = 335,
     T_FUNCTION = 336,
     T_CONST = 337,
     T_RETURN = 338,
     T_TRY = 339,
     T_CATCH = 340,
     T_THROW = 341,
     T_USE = 342,
     T_GLOBAL = 343,
     T_PUBLIC = 344,
     T_PROTECTED = 345,
     T_PRIVATE = 346,
     T_FINAL = 347,
     T_ABSTRACT = 348,
     T_STATIC = 349,
     T_VAR = 350,
     T_UNSET = 351,
     T_ISSET = 352,
     T_EMPTY = 353,
     T_HALT_COMPILER = 354,
     T_CLASS = 355,
     T_INTERFACE = 356,
     T_EXTENDS = 357,
     T_IMPLEMENTS = 358,
     T_OBJECT_OPERATOR = 359,
     T_DOUBLE_ARROW = 360,
     T_LIST = 361,
     T_ARRAY = 362,
     T_CALLABLE = 363,
     T_CLASS_C = 364,
     T_METHOD_C = 365,
     T_FUNC_C = 366,
     T_LINE = 367,
     T_FILE = 368,
     T_COMMENT = 369,
     T_DOC_COMMENT = 370,
     T_OPEN_TAG = 371,
     T_OPEN_TAG_WITH_ECHO = 372,
     T_CLOSE_TAG = 373,
     T_WHITESPACE = 374,
     T_START_HEREDOC = 375,
     T_END_HEREDOC = 376,
     T_DOLLAR_OPEN_CURLY_BRACES = 377,
     T_CURLY_OPEN = 378,
     T_DOUBLE_COLON = 379,
     T_NAMESPACE = 380,
     T_NS_C = 381,
     T_DIR = 382,
     T_NS_SEPARATOR = 383,
     T_YIELD = 384,
     T_XHP_LABEL = 385,
     T_XHP_TEXT = 386,
     T_XHP_ATTRIBUTE = 387,
     T_XHP_CATEGORY = 388,
     T_XHP_CATEGORY_LABEL = 389,
     T_XHP_CHILDREN = 390,
     T_XHP_ENUM = 391,
     T_XHP_REQUIRED = 392,
     T_TRAIT = 393,
     T_INSTEADOF = 394,
     T_TRAIT_C = 395,
     T_VARARG = 396,
     T_HH_ERROR = 397,
     T_FINALLY = 398,
     T_XHP_TAG_LT = 399,
     T_XHP_TAG_GT = 400,
     T_TYPELIST_LT = 401,
     T_TYPELIST_GT = 402,
     T_UNRESOLVED_LT = 403,
     T_COLLECTION = 404,
     T_SHAPE = 405,
     T_TYPE = 406,
     T_UNRESOLVED_TYPE = 407,
     T_NEWTYPE = 408,
     T_UNRESOLVED_NEWTYPE = 409,
     T_COMPILER_HALT_OFFSET = 410,
     T_AWAIT = 411,
     T_ASYNC = 412,
     T_TUPLE = 413,
     T_FROM = 414,
     T_WHERE = 415,
     T_JOIN = 416,
     T_IN = 417,
     T_ON = 418,
     T_EQUALS = 419,
     T_INTO = 420,
     T_LET = 421,
     T_ORDERBY = 422,
     T_ASCENDING = 423,
     T_DESCENDING = 424,
     T_SELECT = 425,
     T_GROUP = 426,
     T_BY = 427,
     T_LAMBDA_OP = 428,
     T_LAMBDA_CP = 429,
     T_UNRESOLVED_OP = 430
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


/* Line 343 of yacc.c  */
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
# if defined YYENABLE_NLS && YYENABLE_NLS
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
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

# define YYCOPY_NEEDED 1

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

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
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
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   14646

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  205
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  246
/* YYNRULES -- Number of rules.  */
#define YYNRULES  823
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1533

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   430

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    49,   203,     2,   200,    48,    32,   204,
     195,   196,    46,    43,     9,    44,    45,    47,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    27,   197,
      37,    14,    38,    26,    52,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    62,     2,   202,    31,     2,   201,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   198,    30,   199,    51,     2,     2,     2,
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
      17,    18,    19,    20,    21,    22,    23,    24,    25,    28,
      29,    33,    34,    35,    36,    39,    40,    41,    42,    50,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
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
     194
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    11,    13,    15,    17,
      19,    21,    26,    30,    31,    38,    39,    45,    49,    52,
      54,    56,    58,    60,    62,    64,    66,    68,    70,    72,
      74,    76,    78,    80,    82,    84,    86,    88,    90,    92,
      96,    98,   100,   103,   107,   112,   114,   118,   120,   124,
     127,   129,   132,   135,   141,   146,   149,   150,   152,   154,
     156,   158,   162,   168,   177,   178,   183,   184,   191,   192,
     203,   204,   209,   212,   216,   219,   223,   226,   230,   234,
     238,   242,   246,   252,   254,   256,   257,   267,   273,   274,
     288,   289,   295,   299,   303,   306,   309,   312,   315,   318,
     321,   325,   328,   331,   335,   338,   339,   344,   354,   355,
     356,   361,   364,   365,   367,   368,   370,   371,   381,   382,
     393,   394,   406,   407,   416,   417,   427,   428,   436,   437,
     446,   447,   455,   456,   465,   467,   469,   471,   473,   475,
     478,   481,   484,   485,   488,   489,   492,   493,   495,   499,
     501,   505,   508,   509,   511,   514,   519,   521,   526,   528,
     533,   535,   540,   542,   547,   551,   557,   561,   566,   571,
     577,   583,   588,   589,   591,   593,   598,   599,   605,   606,
     609,   610,   614,   615,   619,   622,   624,   625,   630,   636,
     644,   651,   658,   666,   676,   685,   689,   692,   694,   695,
     699,   704,   711,   717,   723,   730,   739,   747,   750,   751,
     753,   756,   760,   765,   769,   771,   773,   776,   781,   785,
     791,   793,   797,   800,   801,   802,   807,   808,   814,   817,
     818,   829,   830,   842,   846,   850,   854,   859,   864,   868,
     874,   877,   880,   881,   888,   894,   899,   903,   905,   907,
     911,   916,   918,   920,   922,   924,   929,   931,   935,   938,
     939,   942,   943,   945,   949,   951,   953,   955,   957,   961,
     966,   971,   976,   978,   980,   983,   986,   989,   993,   997,
     999,  1001,  1003,  1005,  1009,  1011,  1015,  1017,  1019,  1021,
    1022,  1024,  1027,  1029,  1031,  1033,  1035,  1037,  1039,  1041,
    1043,  1044,  1046,  1048,  1050,  1054,  1060,  1062,  1066,  1072,
    1077,  1081,  1085,  1088,  1090,  1092,  1096,  1100,  1102,  1104,
    1105,  1108,  1113,  1117,  1124,  1127,  1131,  1138,  1140,  1142,
    1144,  1151,  1155,  1160,  1167,  1171,  1175,  1179,  1183,  1187,
    1191,  1195,  1199,  1203,  1207,  1211,  1214,  1217,  1220,  1223,
    1227,  1231,  1235,  1239,  1243,  1247,  1251,  1255,  1259,  1263,
    1267,  1271,  1275,  1279,  1283,  1287,  1290,  1293,  1296,  1299,
    1303,  1307,  1311,  1315,  1319,  1323,  1327,  1331,  1335,  1339,
    1345,  1350,  1352,  1355,  1358,  1361,  1364,  1367,  1370,  1373,
    1376,  1379,  1381,  1383,  1385,  1389,  1392,  1394,  1396,  1398,
    1404,  1405,  1406,  1418,  1419,  1432,  1433,  1437,  1438,  1445,
    1448,  1453,  1455,  1461,  1465,  1471,  1475,  1478,  1479,  1482,
    1483,  1488,  1493,  1497,  1502,  1507,  1512,  1517,  1519,  1521,
    1525,  1528,  1532,  1537,  1540,  1544,  1546,  1549,  1551,  1554,
    1556,  1558,  1560,  1562,  1564,  1566,  1571,  1576,  1579,  1588,
    1599,  1602,  1604,  1608,  1610,  1613,  1615,  1617,  1619,  1621,
    1624,  1629,  1633,  1637,  1642,  1644,  1647,  1652,  1655,  1662,
    1663,  1665,  1670,  1671,  1674,  1675,  1677,  1679,  1683,  1685,
    1689,  1691,  1693,  1697,  1701,  1703,  1705,  1707,  1709,  1711,
    1713,  1715,  1717,  1719,  1721,  1723,  1725,  1727,  1729,  1731,
    1733,  1735,  1737,  1739,  1741,  1743,  1745,  1747,  1749,  1751,
    1753,  1755,  1757,  1759,  1761,  1763,  1765,  1767,  1769,  1771,
    1773,  1775,  1777,  1779,  1781,  1783,  1785,  1787,  1789,  1791,
    1793,  1795,  1797,  1799,  1801,  1803,  1805,  1807,  1809,  1811,
    1813,  1815,  1817,  1819,  1821,  1823,  1825,  1827,  1829,  1831,
    1833,  1835,  1837,  1839,  1841,  1843,  1845,  1847,  1849,  1851,
    1853,  1855,  1857,  1859,  1861,  1866,  1868,  1870,  1872,  1874,
    1876,  1878,  1880,  1882,  1885,  1887,  1888,  1889,  1891,  1893,
    1897,  1898,  1900,  1902,  1904,  1906,  1908,  1910,  1912,  1914,
    1916,  1918,  1920,  1922,  1924,  1928,  1931,  1933,  1935,  1938,
    1941,  1946,  1951,  1955,  1960,  1962,  1964,  1968,  1972,  1976,
    1978,  1980,  1982,  1984,  1988,  1992,  1996,  1999,  2000,  2002,
    2003,  2005,  2006,  2012,  2016,  2020,  2022,  2024,  2026,  2028,
    2030,  2034,  2037,  2039,  2041,  2043,  2045,  2047,  2049,  2052,
    2055,  2060,  2065,  2069,  2074,  2077,  2078,  2084,  2088,  2092,
    2094,  2098,  2100,  2103,  2104,  2110,  2114,  2117,  2118,  2122,
    2123,  2128,  2131,  2132,  2136,  2140,  2142,  2143,  2145,  2148,
    2151,  2156,  2160,  2164,  2167,  2172,  2175,  2180,  2182,  2184,
    2186,  2188,  2190,  2193,  2198,  2202,  2207,  2211,  2213,  2215,
    2217,  2219,  2222,  2227,  2232,  2236,  2238,  2240,  2244,  2252,
    2259,  2268,  2278,  2287,  2298,  2306,  2313,  2322,  2324,  2327,
    2332,  2337,  2339,  2341,  2346,  2348,  2349,  2351,  2354,  2356,
    2358,  2361,  2366,  2370,  2374,  2375,  2377,  2380,  2385,  2389,
    2392,  2396,  2403,  2404,  2406,  2411,  2414,  2415,  2421,  2425,
    2429,  2431,  2438,  2443,  2448,  2451,  2454,  2455,  2461,  2465,
    2469,  2471,  2474,  2475,  2481,  2485,  2489,  2491,  2494,  2497,
    2499,  2502,  2504,  2509,  2513,  2517,  2524,  2528,  2530,  2532,
    2534,  2539,  2544,  2549,  2554,  2557,  2560,  2565,  2568,  2571,
    2573,  2577,  2581,  2585,  2586,  2589,  2595,  2602,  2604,  2607,
    2609,  2614,  2618,  2619,  2621,  2625,  2629,  2631,  2633,  2634,
    2635,  2638,  2642,  2644,  2650,  2654,  2658,  2662,  2664,  2667,
    2668,  2673,  2676,  2679,  2681,  2683,  2685,  2687,  2692,  2699,
    2701,  2710,  2716,  2718
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     206,     0,    -1,    -1,   207,   208,    -1,   208,   209,    -1,
      -1,   223,    -1,   239,    -1,   243,    -1,   248,    -1,   437,
      -1,   118,   195,   196,   197,    -1,   144,   215,   197,    -1,
      -1,   144,   215,   198,   210,   208,   199,    -1,    -1,   144,
     198,   211,   208,   199,    -1,   106,   213,   197,    -1,   220,
     197,    -1,    73,    -1,   151,    -1,   152,    -1,   154,    -1,
     156,    -1,   155,    -1,   177,    -1,   179,    -1,   180,    -1,
     182,    -1,   181,    -1,   183,    -1,   184,    -1,   185,    -1,
     186,    -1,   187,    -1,   188,    -1,   189,    -1,   190,    -1,
     191,    -1,   213,     9,   214,    -1,   214,    -1,   215,    -1,
     147,   215,    -1,   215,    92,   212,    -1,   147,   215,    92,
     212,    -1,   212,    -1,   215,   147,   212,    -1,   215,    -1,
     144,   147,   215,    -1,   147,   215,    -1,   216,    -1,   216,
     440,    -1,   216,   440,    -1,   220,     9,   438,    14,   384,
      -1,   101,   438,    14,   384,    -1,   221,   222,    -1,    -1,
     223,    -1,   239,    -1,   243,    -1,   248,    -1,   198,   221,
     199,    -1,    66,   316,   223,   270,   272,    -1,    66,   316,
      27,   221,   271,   273,    69,   197,    -1,    -1,    84,   316,
     224,   264,    -1,    -1,    83,   225,   223,    84,   316,   197,
      -1,    -1,    86,   195,   318,   197,   318,   197,   318,   196,
     226,   262,    -1,    -1,    93,   316,   227,   267,    -1,    97,
     197,    -1,    97,   325,   197,    -1,    99,   197,    -1,    99,
     325,   197,    -1,   102,   197,    -1,   102,   325,   197,    -1,
     148,    97,   197,    -1,   107,   280,   197,    -1,   113,   282,
     197,    -1,    82,   317,   197,    -1,   115,   195,   434,   196,
     197,    -1,   197,    -1,    77,    -1,    -1,    88,   195,   325,
      92,   261,   260,   196,   228,   263,    -1,    90,   195,   266,
     196,   265,    -1,    -1,   103,   231,   104,   195,   377,    75,
     196,   198,   221,   199,   233,   229,   236,    -1,    -1,   103,
     231,   162,   230,   234,    -1,   105,   325,   197,    -1,    98,
     212,   197,    -1,   325,   197,    -1,   319,   197,    -1,   320,
     197,    -1,   321,   197,    -1,   322,   197,    -1,   323,   197,
      -1,   102,   322,   197,    -1,   324,   197,    -1,   347,   197,
      -1,   102,   346,   197,    -1,   212,    27,    -1,    -1,   198,
     232,   221,   199,    -1,   233,   104,   195,   377,    75,   196,
     198,   221,   199,    -1,    -1,    -1,   198,   235,   221,   199,
      -1,   162,   234,    -1,    -1,    32,    -1,    -1,   100,    -1,
      -1,   238,   237,   439,   240,   195,   276,   196,   443,   305,
      -1,    -1,   309,   238,   237,   439,   241,   195,   276,   196,
     443,   305,    -1,    -1,   404,   308,   238,   237,   439,   242,
     195,   276,   196,   443,   305,    -1,    -1,   254,   251,   244,
     255,   256,   198,   283,   199,    -1,    -1,   404,   254,   251,
     245,   255,   256,   198,   283,   199,    -1,    -1,   120,   252,
     246,   257,   198,   283,   199,    -1,    -1,   404,   120,   252,
     247,   257,   198,   283,   199,    -1,    -1,   157,   253,   249,
     256,   198,   283,   199,    -1,    -1,   404,   157,   253,   250,
     256,   198,   283,   199,    -1,   439,    -1,   149,    -1,   439,
      -1,   439,    -1,   119,    -1,   112,   119,    -1,   111,   119,
      -1,   121,   377,    -1,    -1,   122,   258,    -1,    -1,   121,
     258,    -1,    -1,   377,    -1,   258,     9,   377,    -1,   377,
      -1,   259,     9,   377,    -1,   124,   261,    -1,    -1,   411,
      -1,    32,   411,    -1,   125,   195,   423,   196,    -1,   223,
      -1,    27,   221,    87,   197,    -1,   223,    -1,    27,   221,
      89,   197,    -1,   223,    -1,    27,   221,    85,   197,    -1,
     223,    -1,    27,   221,    91,   197,    -1,   212,    14,   384,
      -1,   266,     9,   212,    14,   384,    -1,   198,   268,   199,
      -1,   198,   197,   268,   199,    -1,    27,   268,    94,   197,
      -1,    27,   197,   268,    94,   197,    -1,   268,    95,   325,
     269,   221,    -1,   268,    96,   269,   221,    -1,    -1,    27,
      -1,   197,    -1,   270,    67,   316,   223,    -1,    -1,   271,
      67,   316,    27,   221,    -1,    -1,    68,   223,    -1,    -1,
      68,    27,   221,    -1,    -1,   275,     9,   160,    -1,   275,
     389,    -1,   160,    -1,    -1,   405,   311,   450,    75,    -1,
     405,   311,   450,    32,    75,    -1,   405,   311,   450,    32,
      75,    14,   384,    -1,   405,   311,   450,    75,    14,   384,
      -1,   275,     9,   405,   311,   450,    75,    -1,   275,     9,
     405,   311,   450,    32,    75,    -1,   275,     9,   405,   311,
     450,    32,    75,    14,   384,    -1,   275,     9,   405,   311,
     450,    75,    14,   384,    -1,   277,     9,   160,    -1,   277,
     389,    -1,   160,    -1,    -1,   405,   450,    75,    -1,   405,
     450,    32,    75,    -1,   405,   450,    32,    75,    14,   384,
      -1,   405,   450,    75,    14,   384,    -1,   277,     9,   405,
     450,    75,    -1,   277,     9,   405,   450,    32,    75,    -1,
     277,     9,   405,   450,    32,    75,    14,   384,    -1,   277,
       9,   405,   450,    75,    14,   384,    -1,   279,   389,    -1,
      -1,   325,    -1,    32,   411,    -1,   279,     9,   325,    -1,
     279,     9,    32,   411,    -1,   280,     9,   281,    -1,   281,
      -1,    75,    -1,   200,   411,    -1,   200,   198,   325,   199,
      -1,   282,     9,    75,    -1,   282,     9,    75,    14,   384,
      -1,    75,    -1,    75,    14,   384,    -1,   283,   284,    -1,
      -1,    -1,   307,   285,   313,   197,    -1,    -1,   309,   449,
     286,   313,   197,    -1,   314,   197,    -1,    -1,   308,   238,
     237,   439,   195,   287,   274,   196,   443,   306,    -1,    -1,
     404,   308,   238,   237,   439,   195,   288,   274,   196,   443,
     306,    -1,   151,   293,   197,    -1,   152,   299,   197,    -1,
     154,   301,   197,    -1,     4,   121,   377,   197,    -1,     4,
     122,   377,   197,    -1,   106,   259,   197,    -1,   106,   259,
     198,   289,   199,    -1,   289,   290,    -1,   289,   291,    -1,
      -1,   219,   143,   212,   158,   259,   197,    -1,   292,    92,
     308,   212,   197,    -1,   292,    92,   309,   197,    -1,   219,
     143,   212,    -1,   212,    -1,   294,    -1,   293,     9,   294,
      -1,   295,   374,   297,   298,    -1,   149,    -1,   126,    -1,
     377,    -1,   114,    -1,   155,   198,   296,   199,    -1,   383,
      -1,   296,     9,   383,    -1,    14,   384,    -1,    -1,    52,
     156,    -1,    -1,   300,    -1,   299,     9,   300,    -1,   153,
      -1,   302,    -1,   212,    -1,   117,    -1,   195,   303,   196,
      -1,   195,   303,   196,    46,    -1,   195,   303,   196,    26,
      -1,   195,   303,   196,    43,    -1,   302,    -1,   304,    -1,
     304,    46,    -1,   304,    26,    -1,   304,    43,    -1,   303,
       9,   303,    -1,   303,    30,   303,    -1,   212,    -1,   149,
      -1,   153,    -1,   197,    -1,   198,   221,   199,    -1,   197,
      -1,   198,   221,   199,    -1,   309,    -1,   114,    -1,   309,
      -1,    -1,   310,    -1,   309,   310,    -1,   108,    -1,   109,
      -1,   110,    -1,   113,    -1,   112,    -1,   111,    -1,   176,
      -1,   312,    -1,    -1,   108,    -1,   109,    -1,   110,    -1,
     313,     9,    75,    -1,   313,     9,    75,    14,   384,    -1,
      75,    -1,    75,    14,   384,    -1,   314,     9,   438,    14,
     384,    -1,   101,   438,    14,   384,    -1,   195,   315,   196,
      -1,    64,   379,   382,    -1,    63,   325,    -1,   366,    -1,
     342,    -1,   195,   325,   196,    -1,   317,     9,   325,    -1,
     325,    -1,   317,    -1,    -1,   148,   325,    -1,   148,   325,
     124,   325,    -1,   411,    14,   319,    -1,   125,   195,   423,
     196,    14,   319,    -1,   175,   325,    -1,   411,    14,   322,
      -1,   125,   195,   423,   196,    14,   322,    -1,   326,    -1,
     411,    -1,   315,    -1,   125,   195,   423,   196,    14,   325,
      -1,   411,    14,   325,    -1,   411,    14,    32,   411,    -1,
     411,    14,    32,    64,   379,   382,    -1,   411,    25,   325,
      -1,   411,    24,   325,    -1,   411,    23,   325,    -1,   411,
      22,   325,    -1,   411,    21,   325,    -1,   411,    20,   325,
      -1,   411,    19,   325,    -1,   411,    18,   325,    -1,   411,
      17,   325,    -1,   411,    16,   325,    -1,   411,    15,   325,
      -1,   411,    61,    -1,    61,   411,    -1,   411,    60,    -1,
      60,   411,    -1,   325,    28,   325,    -1,   325,    29,   325,
      -1,   325,    10,   325,    -1,   325,    12,   325,    -1,   325,
      11,   325,    -1,   325,    30,   325,    -1,   325,    32,   325,
      -1,   325,    31,   325,    -1,   325,    45,   325,    -1,   325,
      43,   325,    -1,   325,    44,   325,    -1,   325,    46,   325,
      -1,   325,    47,   325,    -1,   325,    48,   325,    -1,   325,
      42,   325,    -1,   325,    41,   325,    -1,    43,   325,    -1,
      44,   325,    -1,    49,   325,    -1,    51,   325,    -1,   325,
      34,   325,    -1,   325,    33,   325,    -1,   325,    36,   325,
      -1,   325,    35,   325,    -1,   325,    37,   325,    -1,   325,
      40,   325,    -1,   325,    38,   325,    -1,   325,    39,   325,
      -1,   325,    50,   379,    -1,   195,   326,   196,    -1,   325,
      26,   325,    27,   325,    -1,   325,    26,    27,   325,    -1,
     433,    -1,    59,   325,    -1,    58,   325,    -1,    57,   325,
      -1,    56,   325,    -1,    55,   325,    -1,    54,   325,    -1,
      53,   325,    -1,    65,   380,    -1,    52,   325,    -1,   386,
      -1,   341,    -1,   340,    -1,   201,   381,   201,    -1,    13,
     325,    -1,   328,    -1,   331,    -1,   344,    -1,   106,   195,
     365,   389,   196,    -1,    -1,    -1,   238,   237,   195,   329,
     276,   196,   443,   327,   198,   221,   199,    -1,    -1,   309,
     238,   237,   195,   330,   276,   196,   443,   327,   198,   221,
     199,    -1,    -1,    75,   332,   334,    -1,    -1,   192,   333,
     276,   193,   443,   334,    -1,     8,   325,    -1,     8,   198,
     221,   199,    -1,    81,    -1,   336,     9,   335,   124,   325,
      -1,   335,   124,   325,    -1,   337,     9,   335,   124,   384,
      -1,   335,   124,   384,    -1,   336,   388,    -1,    -1,   337,
     388,    -1,    -1,   169,   195,   338,   196,    -1,   126,   195,
     424,   196,    -1,    62,   424,   202,    -1,   377,   198,   426,
     199,    -1,   377,   198,   428,   199,    -1,   344,    62,   419,
     202,    -1,   345,    62,   419,   202,    -1,   341,    -1,   435,
      -1,   195,   326,   196,    -1,   348,   349,    -1,   411,    14,
     346,    -1,   178,    75,   181,   325,    -1,   350,   361,    -1,
     350,   361,   364,    -1,   361,    -1,   361,   364,    -1,   351,
      -1,   350,   351,    -1,   352,    -1,   353,    -1,   354,    -1,
     355,    -1,   356,    -1,   357,    -1,   178,    75,   181,   325,
      -1,   185,    75,    14,   325,    -1,   179,   325,    -1,   180,
      75,   181,   325,   182,   325,   183,   325,    -1,   180,    75,
     181,   325,   182,   325,   183,   325,   184,    75,    -1,   186,
     358,    -1,   359,    -1,   358,     9,   359,    -1,   325,    -1,
     325,   360,    -1,   187,    -1,   188,    -1,   362,    -1,   363,
      -1,   189,   325,    -1,   190,   325,   191,   325,    -1,   184,
      75,   349,    -1,   365,     9,    75,    -1,   365,     9,    32,
      75,    -1,    75,    -1,    32,    75,    -1,   163,   149,   367,
     164,    -1,   369,    47,    -1,   369,   164,   370,   163,    47,
     368,    -1,    -1,   149,    -1,   369,   371,    14,   372,    -1,
      -1,   370,   373,    -1,    -1,   149,    -1,   150,    -1,   198,
     325,   199,    -1,   150,    -1,   198,   325,   199,    -1,   366,
      -1,   375,    -1,   374,    27,   375,    -1,   374,    44,   375,
      -1,   212,    -1,    65,    -1,   100,    -1,   101,    -1,   102,
      -1,   148,    -1,   175,    -1,   103,    -1,   104,    -1,   162,
      -1,   105,    -1,    66,    -1,    67,    -1,    69,    -1,    68,
      -1,    84,    -1,    85,    -1,    83,    -1,    86,    -1,    87,
      -1,    88,    -1,    89,    -1,    90,    -1,    91,    -1,    50,
      -1,    92,    -1,    93,    -1,    94,    -1,    95,    -1,    96,
      -1,    97,    -1,    99,    -1,    98,    -1,    82,    -1,    13,
      -1,   119,    -1,   120,    -1,   121,    -1,   122,    -1,    64,
      -1,    63,    -1,   114,    -1,     5,    -1,     7,    -1,     6,
      -1,     4,    -1,     3,    -1,   144,    -1,   106,    -1,   107,
      -1,   116,    -1,   117,    -1,   118,    -1,   113,    -1,   112,
      -1,   111,    -1,   110,    -1,   109,    -1,   108,    -1,   176,
      -1,   115,    -1,   125,    -1,   126,    -1,    10,    -1,    12,
      -1,    11,    -1,   128,    -1,   130,    -1,   129,    -1,   131,
      -1,   132,    -1,   146,    -1,   145,    -1,   174,    -1,   157,
      -1,   159,    -1,   158,    -1,   170,    -1,   172,    -1,   169,
      -1,   218,   195,   278,   196,    -1,   219,    -1,   149,    -1,
     377,    -1,   113,    -1,   417,    -1,   377,    -1,   113,    -1,
     421,    -1,   195,   196,    -1,   316,    -1,    -1,    -1,    80,
      -1,   430,    -1,   195,   278,   196,    -1,    -1,    70,    -1,
      71,    -1,    72,    -1,    81,    -1,   131,    -1,   132,    -1,
     146,    -1,   128,    -1,   159,    -1,   129,    -1,   130,    -1,
     145,    -1,   174,    -1,   139,    80,   140,    -1,   139,   140,
      -1,   383,    -1,   217,    -1,    43,   384,    -1,    44,   384,
      -1,   126,   195,   387,   196,    -1,   177,   195,   387,   196,
      -1,    62,   387,   202,    -1,   169,   195,   339,   196,    -1,
     385,    -1,   343,    -1,   219,   143,   212,    -1,   149,   143,
     212,    -1,   219,   143,   119,    -1,   217,    -1,    74,    -1,
     435,    -1,   383,    -1,   203,   430,   203,    -1,   204,   430,
     204,    -1,   139,   430,   140,    -1,   390,   388,    -1,    -1,
       9,    -1,    -1,     9,    -1,    -1,   390,     9,   384,   124,
     384,    -1,   390,     9,   384,    -1,   384,   124,   384,    -1,
     384,    -1,    70,    -1,    71,    -1,    72,    -1,    81,    -1,
     139,    80,   140,    -1,   139,   140,    -1,    70,    -1,    71,
      -1,    72,    -1,   212,    -1,   391,    -1,   212,    -1,    43,
     392,    -1,    44,   392,    -1,   126,   195,   394,   196,    -1,
     177,   195,   394,   196,    -1,    62,   394,   202,    -1,   169,
     195,   397,   196,    -1,   395,   388,    -1,    -1,   395,     9,
     393,   124,   393,    -1,   395,     9,   393,    -1,   393,   124,
     393,    -1,   393,    -1,   396,     9,   393,    -1,   393,    -1,
     398,   388,    -1,    -1,   398,     9,   335,   124,   393,    -1,
     335,   124,   393,    -1,   396,   388,    -1,    -1,   195,   399,
     196,    -1,    -1,   401,     9,   212,   400,    -1,   212,   400,
      -1,    -1,   403,   401,   388,    -1,    42,   402,    41,    -1,
     404,    -1,    -1,   407,    -1,   123,   416,    -1,   123,   212,
      -1,   123,   198,   325,   199,    -1,    62,   419,   202,    -1,
     198,   325,   199,    -1,   412,   408,    -1,   195,   315,   196,
     408,    -1,   422,   408,    -1,   195,   315,   196,   408,    -1,
     416,    -1,   376,    -1,   414,    -1,   415,    -1,   409,    -1,
     411,   406,    -1,   195,   315,   196,   406,    -1,   378,   143,
     416,    -1,   413,   195,   278,   196,    -1,   195,   411,   196,
      -1,   376,    -1,   414,    -1,   415,    -1,   409,    -1,   411,
     407,    -1,   195,   315,   196,   407,    -1,   413,   195,   278,
     196,    -1,   195,   411,   196,    -1,   416,    -1,   409,    -1,
     195,   411,   196,    -1,   411,   123,   212,   440,   195,   278,
     196,    -1,   411,   123,   416,   195,   278,   196,    -1,   411,
     123,   198,   325,   199,   195,   278,   196,    -1,   195,   315,
     196,   123,   212,   440,   195,   278,   196,    -1,   195,   315,
     196,   123,   416,   195,   278,   196,    -1,   195,   315,   196,
     123,   198,   325,   199,   195,   278,   196,    -1,   378,   143,
     212,   440,   195,   278,   196,    -1,   378,   143,   416,   195,
     278,   196,    -1,   378,   143,   198,   325,   199,   195,   278,
     196,    -1,   417,    -1,   420,   417,    -1,   417,    62,   419,
     202,    -1,   417,   198,   325,   199,    -1,   418,    -1,    75,
      -1,   200,   198,   325,   199,    -1,   325,    -1,    -1,   200,
      -1,   420,   200,    -1,   416,    -1,   410,    -1,   421,   406,
      -1,   195,   315,   196,   406,    -1,   378,   143,   416,    -1,
     195,   411,   196,    -1,    -1,   410,    -1,   421,   407,    -1,
     195,   315,   196,   407,    -1,   195,   411,   196,    -1,   423,
       9,    -1,   423,     9,   411,    -1,   423,     9,   125,   195,
     423,   196,    -1,    -1,   411,    -1,   125,   195,   423,   196,
      -1,   425,   388,    -1,    -1,   425,     9,   325,   124,   325,
      -1,   425,     9,   325,    -1,   325,   124,   325,    -1,   325,
      -1,   425,     9,   325,   124,    32,   411,    -1,   425,     9,
      32,   411,    -1,   325,   124,    32,   411,    -1,    32,   411,
      -1,   427,   388,    -1,    -1,   427,     9,   325,   124,   325,
      -1,   427,     9,   325,    -1,   325,   124,   325,    -1,   325,
      -1,   429,   388,    -1,    -1,   429,     9,   384,   124,   384,
      -1,   429,     9,   384,    -1,   384,   124,   384,    -1,   384,
      -1,   430,   431,    -1,   430,    80,    -1,   431,    -1,    80,
     431,    -1,    75,    -1,    75,    62,   432,   202,    -1,    75,
     123,   212,    -1,   141,   325,   199,    -1,   141,    74,    62,
     325,   202,   199,    -1,   142,   411,   199,    -1,   212,    -1,
      76,    -1,    75,    -1,   116,   195,   434,   196,    -1,   117,
     195,   411,   196,    -1,   117,   195,   326,   196,    -1,   117,
     195,   315,   196,    -1,     7,   325,    -1,     6,   325,    -1,
       5,   195,   325,   196,    -1,     4,   325,    -1,     3,   325,
      -1,   411,    -1,   434,     9,   411,    -1,   378,   143,   212,
      -1,   378,   143,   119,    -1,    -1,    92,   449,    -1,   170,
     439,    14,   449,   197,    -1,   172,   439,   436,    14,   449,
     197,    -1,   212,    -1,   449,   212,    -1,   212,    -1,   212,
     165,   444,   166,    -1,   165,   441,   166,    -1,    -1,   449,
      -1,   441,     9,   449,    -1,   441,     9,   160,    -1,   441,
      -1,   160,    -1,    -1,    -1,    27,   449,    -1,   444,     9,
     212,    -1,   212,    -1,   444,     9,   212,    92,   449,    -1,
     212,    92,   449,    -1,    81,   124,   449,    -1,   446,     9,
     445,    -1,   445,    -1,   446,   388,    -1,    -1,   169,   195,
     447,   196,    -1,    26,   449,    -1,    52,   449,    -1,   219,
      -1,   126,    -1,   127,    -1,   448,    -1,   126,   165,   449,
     166,    -1,   126,   165,   449,     9,   449,   166,    -1,   149,
      -1,   195,   100,   195,   442,   196,    27,   449,   196,    -1,
     195,   441,     9,   449,   196,    -1,   449,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   738,   738,   738,   747,   749,   752,   753,   754,   755,
     756,   757,   760,   762,   762,   764,   764,   766,   767,   772,
     773,   774,   775,   776,   777,   778,   779,   780,   781,   782,
     783,   784,   785,   786,   787,   788,   789,   790,   791,   795,
     797,   800,   801,   802,   803,   808,   809,   813,   814,   816,
     819,   825,   832,   839,   843,   849,   851,   854,   855,   856,
     857,   860,   861,   865,   870,   870,   876,   876,   883,   882,
     888,   888,   893,   894,   895,   896,   897,   898,   899,   900,
     901,   902,   903,   904,   905,   908,   906,   913,   921,   915,
     925,   923,   927,   928,   932,   933,   934,   935,   936,   937,
     938,   939,   940,   941,   942,   950,   950,   955,   961,   965,
     965,   973,   974,   978,   979,   983,   988,   987,  1000,   998,
    1012,  1010,  1026,  1025,  1044,  1042,  1061,  1060,  1069,  1067,
    1079,  1078,  1090,  1088,  1101,  1102,  1106,  1109,  1112,  1113,
    1114,  1117,  1119,  1122,  1123,  1126,  1127,  1130,  1131,  1135,
    1136,  1141,  1142,  1145,  1146,  1147,  1151,  1152,  1156,  1157,
    1161,  1162,  1166,  1167,  1172,  1173,  1178,  1179,  1180,  1181,
    1184,  1187,  1189,  1192,  1193,  1197,  1199,  1202,  1205,  1208,
    1209,  1212,  1213,  1217,  1219,  1221,  1222,  1226,  1230,  1234,
    1239,  1244,  1249,  1254,  1260,  1269,  1271,  1273,  1274,  1278,
    1281,  1284,  1288,  1292,  1296,  1300,  1305,  1313,  1315,  1318,
    1319,  1320,  1322,  1327,  1328,  1331,  1332,  1333,  1337,  1338,
    1340,  1341,  1345,  1347,  1350,  1350,  1354,  1353,  1357,  1361,
    1359,  1374,  1371,  1384,  1386,  1388,  1390,  1392,  1394,  1396,
    1400,  1401,  1402,  1405,  1411,  1414,  1420,  1423,  1428,  1430,
    1435,  1440,  1444,  1445,  1451,  1452,  1457,  1458,  1463,  1464,
    1468,  1469,  1473,  1475,  1481,  1486,  1487,  1489,  1493,  1494,
    1495,  1496,  1500,  1501,  1502,  1503,  1504,  1505,  1507,  1512,
    1515,  1516,  1520,  1521,  1525,  1526,  1529,  1530,  1533,  1534,
    1537,  1538,  1542,  1543,  1544,  1545,  1546,  1547,  1548,  1552,
    1553,  1556,  1557,  1558,  1561,  1563,  1565,  1566,  1569,  1571,
    1575,  1576,  1578,  1579,  1580,  1583,  1587,  1588,  1592,  1593,
    1597,  1598,  1602,  1606,  1611,  1615,  1619,  1624,  1625,  1626,
    1629,  1631,  1632,  1633,  1636,  1637,  1638,  1639,  1640,  1641,
    1642,  1643,  1644,  1645,  1646,  1647,  1648,  1649,  1650,  1651,
    1652,  1653,  1654,  1655,  1656,  1657,  1658,  1659,  1660,  1661,
    1662,  1663,  1664,  1665,  1666,  1667,  1668,  1669,  1670,  1671,
    1672,  1673,  1674,  1675,  1676,  1678,  1679,  1681,  1683,  1684,
    1685,  1686,  1687,  1688,  1689,  1690,  1691,  1692,  1693,  1694,
    1695,  1696,  1697,  1698,  1699,  1700,  1701,  1702,  1703,  1707,
    1711,  1716,  1715,  1730,  1728,  1745,  1745,  1760,  1760,  1778,
    1779,  1784,  1789,  1793,  1799,  1803,  1809,  1811,  1815,  1817,
    1821,  1825,  1826,  1830,  1837,  1844,  1846,  1851,  1852,  1853,
    1857,  1861,  1865,  1869,  1871,  1873,  1875,  1880,  1881,  1886,
    1887,  1888,  1889,  1890,  1891,  1895,  1899,  1903,  1907,  1912,
    1917,  1921,  1922,  1926,  1927,  1931,  1932,  1936,  1937,  1941,
    1945,  1949,  1953,  1954,  1955,  1956,  1960,  1966,  1975,  1988,
    1989,  1992,  1995,  1998,  1999,  2002,  2006,  2009,  2012,  2019,
    2020,  2024,  2025,  2027,  2031,  2032,  2033,  2034,  2035,  2036,
    2037,  2038,  2039,  2040,  2041,  2042,  2043,  2044,  2045,  2046,
    2047,  2048,  2049,  2050,  2051,  2052,  2053,  2054,  2055,  2056,
    2057,  2058,  2059,  2060,  2061,  2062,  2063,  2064,  2065,  2066,
    2067,  2068,  2069,  2070,  2071,  2072,  2073,  2074,  2075,  2076,
    2077,  2078,  2079,  2080,  2081,  2082,  2083,  2084,  2085,  2086,
    2087,  2088,  2089,  2090,  2091,  2092,  2093,  2094,  2095,  2096,
    2097,  2098,  2099,  2100,  2101,  2102,  2103,  2104,  2105,  2106,
    2107,  2108,  2109,  2110,  2114,  2119,  2120,  2123,  2124,  2125,
    2129,  2130,  2131,  2135,  2136,  2137,  2141,  2142,  2143,  2146,
    2148,  2152,  2153,  2154,  2155,  2157,  2158,  2159,  2160,  2161,
    2162,  2163,  2164,  2165,  2166,  2169,  2174,  2175,  2176,  2177,
    2178,  2180,  2182,  2183,  2185,  2186,  2190,  2193,  2196,  2202,
    2203,  2204,  2205,  2206,  2207,  2208,  2213,  2215,  2219,  2220,
    2223,  2224,  2228,  2231,  2233,  2235,  2239,  2240,  2241,  2242,
    2244,  2247,  2251,  2252,  2253,  2254,  2257,  2258,  2259,  2260,
    2261,  2263,  2265,  2266,  2271,  2273,  2276,  2279,  2281,  2283,
    2286,  2288,  2292,  2294,  2297,  2300,  2306,  2308,  2311,  2312,
    2317,  2320,  2324,  2324,  2329,  2332,  2333,  2337,  2338,  2343,
    2344,  2348,  2349,  2353,  2354,  2359,  2361,  2366,  2367,  2368,
    2369,  2370,  2371,  2372,  2374,  2377,  2379,  2383,  2384,  2385,
    2386,  2387,  2389,  2391,  2393,  2397,  2398,  2399,  2403,  2406,
    2409,  2412,  2416,  2420,  2427,  2431,  2435,  2442,  2443,  2448,
    2450,  2451,  2454,  2455,  2458,  2459,  2463,  2464,  2468,  2469,
    2470,  2471,  2473,  2476,  2479,  2480,  2481,  2483,  2485,  2489,
    2490,  2491,  2493,  2494,  2495,  2499,  2501,  2504,  2506,  2507,
    2508,  2509,  2512,  2514,  2515,  2519,  2521,  2524,  2526,  2527,
    2528,  2532,  2534,  2537,  2540,  2542,  2544,  2548,  2549,  2551,
    2552,  2558,  2559,  2561,  2563,  2565,  2567,  2570,  2571,  2572,
    2576,  2577,  2578,  2579,  2580,  2581,  2582,  2583,  2584,  2588,
    2589,  2593,  2595,  2603,  2605,  2609,  2613,  2620,  2621,  2627,
    2628,  2635,  2638,  2642,  2645,  2650,  2651,  2652,  2653,  2657,
    2658,  2662,  2664,  2665,  2667,  2671,  2677,  2679,  2683,  2686,
    2689,  2697,  2700,  2703,  2704,  2707,  2710,  2711,  2714,  2718,
    2722,  2728,  2736,  2737
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_REQUIRE_ONCE", "T_REQUIRE", "T_EVAL",
  "T_INCLUDE_ONCE", "T_INCLUDE", "T_LAMBDA_ARROW", "','", "T_LOGICAL_OR",
  "T_LOGICAL_XOR", "T_LOGICAL_AND", "T_PRINT", "'='", "T_SR_EQUAL",
  "T_SL_EQUAL", "T_XOR_EQUAL", "T_OR_EQUAL", "T_AND_EQUAL", "T_MOD_EQUAL",
  "T_CONCAT_EQUAL", "T_DIV_EQUAL", "T_MUL_EQUAL", "T_MINUS_EQUAL",
  "T_PLUS_EQUAL", "'?'", "':'", "T_BOOLEAN_OR", "T_BOOLEAN_AND", "'|'",
  "'^'", "'&'", "T_IS_NOT_IDENTICAL", "T_IS_IDENTICAL", "T_IS_NOT_EQUAL",
  "T_IS_EQUAL", "'<'", "'>'", "T_IS_GREATER_OR_EQUAL",
  "T_IS_SMALLER_OR_EQUAL", "T_SR", "T_SL", "'+'", "'-'", "'.'", "'*'",
  "'/'", "'%'", "'!'", "T_INSTANCEOF", "'~'", "'@'", "T_UNSET_CAST",
  "T_BOOL_CAST", "T_OBJECT_CAST", "T_ARRAY_CAST", "T_STRING_CAST",
  "T_DOUBLE_CAST", "T_INT_CAST", "T_DEC", "T_INC", "'['", "T_CLONE",
  "T_NEW", "T_EXIT", "T_IF", "T_ELSEIF", "T_ELSE", "T_ENDIF", "T_LNUMBER",
  "T_DNUMBER", "T_ONUMBER", "T_STRING", "T_STRING_VARNAME", "T_VARIABLE",
  "T_NUM_STRING", "T_INLINE_HTML", "T_CHARACTER", "T_BAD_CHARACTER",
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
  "T_NAMESPACE", "T_NS_C", "T_DIR", "T_NS_SEPARATOR", "T_YIELD",
  "T_XHP_LABEL", "T_XHP_TEXT", "T_XHP_ATTRIBUTE", "T_XHP_CATEGORY",
  "T_XHP_CATEGORY_LABEL", "T_XHP_CHILDREN", "T_XHP_ENUM", "T_XHP_REQUIRED",
  "T_TRAIT", "T_INSTEADOF", "T_TRAIT_C", "T_VARARG", "T_HH_ERROR",
  "T_FINALLY", "T_XHP_TAG_LT", "T_XHP_TAG_GT", "T_TYPELIST_LT",
  "T_TYPELIST_GT", "T_UNRESOLVED_LT", "T_COLLECTION", "T_SHAPE", "T_TYPE",
  "T_UNRESOLVED_TYPE", "T_NEWTYPE", "T_UNRESOLVED_NEWTYPE",
  "T_COMPILER_HALT_OFFSET", "T_AWAIT", "T_ASYNC", "T_TUPLE", "T_FROM",
  "T_WHERE", "T_JOIN", "T_IN", "T_ON", "T_EQUALS", "T_INTO", "T_LET",
  "T_ORDERBY", "T_ASCENDING", "T_DESCENDING", "T_SELECT", "T_GROUP",
  "T_BY", "T_LAMBDA_OP", "T_LAMBDA_CP", "T_UNRESOLVED_OP", "'('", "')'",
  "';'", "'{'", "'}'", "'$'", "'`'", "']'", "'\"'", "'\\''", "$accept",
  "start", "$@1", "top_statement_list", "top_statement", "$@2", "$@3",
  "ident", "use_declarations", "use_declaration", "namespace_name",
  "namespace_string_base", "namespace_string", "namespace_string_typeargs",
  "class_namespace_string_typeargs", "constant_declaration",
  "inner_statement_list", "inner_statement", "statement", "$@4", "$@5",
  "$@6", "$@7", "$@8", "$@9", "$@10", "try_statement_list", "$@11",
  "additional_catches", "finally_statement_list", "$@12",
  "optional_finally", "is_reference", "function_loc",
  "function_declaration_statement", "$@13", "$@14", "$@15",
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
  "static_class_constant", "scalar", "static_array_pair_list",
  "possible_comma", "hh_possible_comma",
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
     273,   274,   275,   276,   277,   278,    63,    58,   279,   280,
     124,    94,    38,   281,   282,   283,   284,    60,    62,   285,
     286,   287,   288,    43,    45,    46,    42,    47,    37,    33,
     289,   126,    64,   290,   291,   292,   293,   294,   295,   296,
     297,   298,    91,   299,   300,   301,   302,   303,   304,   305,
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
     426,   427,   428,   429,   430,    40,    41,    59,   123,   125,
      36,    96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   205,   207,   206,   208,   208,   209,   209,   209,   209,
     209,   209,   209,   210,   209,   211,   209,   209,   209,   212,
     212,   212,   212,   212,   212,   212,   212,   212,   212,   212,
     212,   212,   212,   212,   212,   212,   212,   212,   212,   213,
     213,   214,   214,   214,   214,   215,   215,   216,   216,   216,
     217,   218,   219,   220,   220,   221,   221,   222,   222,   222,
     222,   223,   223,   223,   224,   223,   225,   223,   226,   223,
     227,   223,   223,   223,   223,   223,   223,   223,   223,   223,
     223,   223,   223,   223,   223,   228,   223,   223,   229,   223,
     230,   223,   223,   223,   223,   223,   223,   223,   223,   223,
     223,   223,   223,   223,   223,   232,   231,   233,   233,   235,
     234,   236,   236,   237,   237,   238,   240,   239,   241,   239,
     242,   239,   244,   243,   245,   243,   246,   243,   247,   243,
     249,   248,   250,   248,   251,   251,   252,   253,   254,   254,
     254,   255,   255,   256,   256,   257,   257,   258,   258,   259,
     259,   260,   260,   261,   261,   261,   262,   262,   263,   263,
     264,   264,   265,   265,   266,   266,   267,   267,   267,   267,
     268,   268,   268,   269,   269,   270,   270,   271,   271,   272,
     272,   273,   273,   274,   274,   274,   274,   275,   275,   275,
     275,   275,   275,   275,   275,   276,   276,   276,   276,   277,
     277,   277,   277,   277,   277,   277,   277,   278,   278,   279,
     279,   279,   279,   280,   280,   281,   281,   281,   282,   282,
     282,   282,   283,   283,   285,   284,   286,   284,   284,   287,
     284,   288,   284,   284,   284,   284,   284,   284,   284,   284,
     289,   289,   289,   290,   291,   291,   292,   292,   293,   293,
     294,   294,   295,   295,   295,   295,   296,   296,   297,   297,
     298,   298,   299,   299,   300,   301,   301,   301,   302,   302,
     302,   302,   303,   303,   303,   303,   303,   303,   303,   304,
     304,   304,   305,   305,   306,   306,   307,   307,   308,   308,
     309,   309,   310,   310,   310,   310,   310,   310,   310,   311,
     311,   312,   312,   312,   313,   313,   313,   313,   314,   314,
     315,   315,   315,   315,   315,   316,   317,   317,   318,   318,
     319,   319,   320,   321,   322,   323,   324,   325,   325,   325,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   327,
     327,   329,   328,   330,   328,   332,   331,   333,   331,   334,
     334,   335,   336,   336,   337,   337,   338,   338,   339,   339,
     340,   341,   341,   342,   343,   344,   344,   345,   345,   345,
     346,   347,   348,   349,   349,   349,   349,   350,   350,   351,
     351,   351,   351,   351,   351,   352,   353,   354,   355,   356,
     357,   358,   358,   359,   359,   360,   360,   361,   361,   362,
     363,   364,   365,   365,   365,   365,   366,   367,   367,   368,
     368,   369,   369,   370,   370,   371,   372,   372,   373,   373,
     373,   374,   374,   374,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   376,   377,   377,   378,   378,   378,
     379,   379,   379,   380,   380,   380,   381,   381,   381,   382,
     382,   383,   383,   383,   383,   383,   383,   383,   383,   383,
     383,   383,   383,   383,   383,   383,   384,   384,   384,   384,
     384,   384,   384,   384,   384,   384,   385,   385,   385,   386,
     386,   386,   386,   386,   386,   386,   387,   387,   388,   388,
     389,   389,   390,   390,   390,   390,   391,   391,   391,   391,
     391,   391,   392,   392,   392,   392,   393,   393,   393,   393,
     393,   393,   393,   393,   394,   394,   395,   395,   395,   395,
     396,   396,   397,   397,   398,   398,   399,   399,   400,   400,
     401,   401,   403,   402,   404,   405,   405,   406,   406,   407,
     407,   408,   408,   409,   409,   410,   410,   411,   411,   411,
     411,   411,   411,   411,   411,   411,   411,   412,   412,   412,
     412,   412,   412,   412,   412,   413,   413,   413,   414,   414,
     414,   414,   414,   414,   415,   415,   415,   416,   416,   417,
     417,   417,   418,   418,   419,   419,   420,   420,   421,   421,
     421,   421,   421,   421,   422,   422,   422,   422,   422,   423,
     423,   423,   423,   423,   423,   424,   424,   425,   425,   425,
     425,   425,   425,   425,   425,   426,   426,   427,   427,   427,
     427,   428,   428,   429,   429,   429,   429,   430,   430,   430,
     430,   431,   431,   431,   431,   431,   431,   432,   432,   432,
     433,   433,   433,   433,   433,   433,   433,   433,   433,   434,
     434,   435,   435,   436,   436,   437,   437,   438,   438,   439,
     439,   440,   440,   441,   441,   442,   442,   442,   442,   443,
     443,   444,   444,   444,   444,   445,   446,   446,   447,   447,
     448,   449,   449,   449,   449,   449,   449,   449,   449,   449,
     449,   449,   450,   450
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     4,     3,     0,     6,     0,     5,     3,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     1,     2,     3,     4,     1,     3,     1,     3,     2,
       1,     2,     2,     5,     4,     2,     0,     1,     1,     1,
       1,     3,     5,     8,     0,     4,     0,     6,     0,    10,
       0,     4,     2,     3,     2,     3,     2,     3,     3,     3,
       3,     3,     5,     1,     1,     0,     9,     5,     0,    13,
       0,     5,     3,     3,     2,     2,     2,     2,     2,     2,
       3,     2,     2,     3,     2,     0,     4,     9,     0,     0,
       4,     2,     0,     1,     0,     1,     0,     9,     0,    10,
       0,    11,     0,     8,     0,     9,     0,     7,     0,     8,
       0,     7,     0,     8,     1,     1,     1,     1,     1,     2,
       2,     2,     0,     2,     0,     2,     0,     1,     3,     1,
       3,     2,     0,     1,     2,     4,     1,     4,     1,     4,
       1,     4,     1,     4,     3,     5,     3,     4,     4,     5,
       5,     4,     0,     1,     1,     4,     0,     5,     0,     2,
       0,     3,     0,     3,     2,     1,     0,     4,     5,     7,
       6,     6,     7,     9,     8,     3,     2,     1,     0,     3,
       4,     6,     5,     5,     6,     8,     7,     2,     0,     1,
       2,     3,     4,     3,     1,     1,     2,     4,     3,     5,
       1,     3,     2,     0,     0,     4,     0,     5,     2,     0,
      10,     0,    11,     3,     3,     3,     4,     4,     3,     5,
       2,     2,     0,     6,     5,     4,     3,     1,     1,     3,
       4,     1,     1,     1,     1,     4,     1,     3,     2,     0,
       2,     0,     1,     3,     1,     1,     1,     1,     3,     4,
       4,     4,     1,     1,     2,     2,     2,     3,     3,     1,
       1,     1,     1,     3,     1,     3,     1,     1,     1,     0,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     1,     1,     1,     3,     5,     1,     3,     5,     4,
       3,     3,     2,     1,     1,     3,     3,     1,     1,     0,
       2,     4,     3,     6,     2,     3,     6,     1,     1,     1,
       6,     3,     4,     6,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     5,
       4,     1,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     1,     1,     1,     3,     2,     1,     1,     1,     5,
       0,     0,    11,     0,    12,     0,     3,     0,     6,     2,
       4,     1,     5,     3,     5,     3,     2,     0,     2,     0,
       4,     4,     3,     4,     4,     4,     4,     1,     1,     3,
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
       1,     1,     1,     1,     3,     2,     1,     1,     2,     2,
       4,     4,     3,     4,     1,     1,     3,     3,     3,     1,
       1,     1,     1,     3,     3,     3,     2,     0,     1,     0,
       1,     0,     5,     3,     3,     1,     1,     1,     1,     1,
       3,     2,     1,     1,     1,     1,     1,     1,     2,     2,
       4,     4,     3,     4,     2,     0,     5,     3,     3,     1,
       3,     1,     2,     0,     5,     3,     2,     0,     3,     0,
       4,     2,     0,     3,     3,     1,     0,     1,     2,     2,
       4,     3,     3,     2,     4,     2,     4,     1,     1,     1,
       1,     1,     2,     4,     3,     4,     3,     1,     1,     1,
       1,     2,     4,     4,     3,     1,     1,     3,     7,     6,
       8,     9,     8,    10,     7,     6,     8,     1,     2,     4,
       4,     1,     1,     4,     1,     0,     1,     2,     1,     1,
       2,     4,     3,     3,     0,     1,     2,     4,     3,     2,
       3,     6,     0,     1,     4,     2,     0,     5,     3,     3,
       1,     6,     4,     4,     2,     2,     0,     5,     3,     3,
       1,     2,     0,     5,     3,     3,     1,     2,     2,     1,
       2,     1,     4,     3,     3,     6,     3,     1,     1,     1,
       4,     4,     4,     4,     2,     2,     4,     2,     2,     1,
       3,     3,     3,     0,     2,     5,     6,     1,     2,     1,
       4,     3,     0,     1,     3,     3,     1,     1,     0,     0,
       2,     3,     1,     5,     3,     3,     3,     1,     2,     0,
       4,     2,     2,     1,     1,     1,     1,     4,     6,     1,
       8,     5,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   662,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   736,     0,   724,   575,
       0,   581,   582,   583,    19,   610,   712,    84,   584,     0,
      66,     0,     0,     0,     0,     0,     0,     0,     0,   115,
       0,     0,     0,     0,     0,     0,   292,   293,   294,   297,
     296,   295,     0,     0,     0,     0,   138,     0,     0,     0,
     588,   590,   591,   585,   586,     0,     0,   592,   587,     0,
       0,   566,    20,    21,    22,    24,    23,     0,   589,     0,
       0,     0,     0,   593,     0,   298,    25,    26,    27,    29,
      28,    30,    31,    32,    33,    34,    35,    36,    37,    38,
     407,     0,    83,    56,   716,   576,     0,     0,     4,    45,
      47,    50,   609,     0,   565,     0,     6,   114,     7,     8,
       9,     0,     0,   290,   329,     0,     0,     0,     0,     0,
       0,     0,   327,   396,   397,   393,   392,   314,   398,     0,
       0,   313,   678,   567,     0,   612,   391,   289,   681,   328,
       0,     0,   679,   680,   677,   707,   711,     0,   381,   611,
      10,   297,   296,   295,     0,     0,    45,   114,     0,   778,
     328,   777,     0,   775,   774,   395,     0,     0,   365,   366,
     367,   368,   390,   388,   387,   386,   385,   384,   383,   382,
     712,   568,     0,   792,   567,     0,   348,   346,     0,   740,
       0,   619,   312,   571,     0,   792,   570,     0,   580,   719,
     718,   572,     0,     0,   574,   389,     0,     0,     0,     0,
     317,     0,    64,   319,     0,     0,    70,    72,     0,     0,
      74,     0,     0,     0,   814,   815,   819,     0,     0,    45,
     813,     0,   816,     0,     0,    76,     0,     0,     0,     0,
     105,     0,     0,     0,     0,    40,    41,   215,     0,     0,
     214,   140,   139,   220,     0,     0,     0,     0,     0,   789,
     126,   136,   732,   736,   761,     0,   595,     0,     0,     0,
     759,     0,    15,     0,    49,     0,   320,   130,   137,   472,
     417,     0,   783,   324,   666,   329,     0,   327,   328,     0,
       0,   577,     0,   578,     0,     0,     0,   104,     0,     0,
      52,   208,     0,    18,   113,     0,   135,   122,   134,   295,
     114,   291,    95,    96,    97,    98,    99,   101,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   724,    94,   715,   715,   102,   746,     0,
       0,     0,     0,     0,   288,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   347,   345,     0,
     682,   667,   715,     0,   673,   208,   715,     0,   717,   708,
     732,     0,   114,     0,     0,   664,   659,   619,     0,     0,
       0,     0,   744,     0,   422,   618,   735,     0,     0,    52,
       0,   208,   311,     0,   720,   667,   675,   573,     0,    56,
     176,     0,   406,     0,    81,     0,     0,   318,     0,     0,
       0,     0,     0,    73,    93,    75,   811,   812,     0,   809,
       0,     0,   793,     0,   788,     0,   100,    77,   103,     0,
       0,     0,     0,     0,     0,     0,   430,     0,   437,   439,
     440,   441,   442,   443,   444,   435,   457,   458,    56,     0,
      90,    92,    42,     0,    17,     0,     0,   216,     0,    79,
       0,     0,    80,   779,     0,     0,   329,   327,   328,     0,
       0,   146,     0,   733,     0,     0,     0,     0,   594,   760,
     610,     0,     0,   758,   615,   757,    48,     5,    12,    13,
      78,     0,   144,     0,     0,   411,     0,   619,     0,     0,
       0,     0,   197,     0,   621,   665,   823,   310,   378,   686,
      61,    55,    57,    58,    59,    60,     0,   394,   613,   614,
      46,     0,     0,     0,   621,   209,     0,   401,   116,   142,
       0,   351,   353,   352,     0,     0,   349,   350,   354,   356,
     355,   370,   369,   372,   371,   373,   375,   376,   374,   364,
     363,   358,   359,   357,   360,   361,   362,   377,   714,     0,
       0,   750,     0,   619,   782,     0,   781,   684,   707,   128,
     132,   124,   114,     0,     0,   322,   325,   331,   431,   344,
     343,   342,   341,   340,   339,   338,   337,   336,   335,   334,
       0,   669,   668,     0,     0,     0,     0,     0,     0,     0,
     776,   657,   661,   618,   663,     0,     0,   792,     0,   739,
       0,   738,     0,   723,   722,     0,     0,   669,   668,   315,
     178,   180,    56,   409,   316,     0,    56,   160,    65,   319,
       0,     0,     0,     0,   172,   172,    71,     0,     0,   807,
     619,     0,   798,     0,     0,     0,   617,     0,     0,   566,
       0,    25,    50,   597,   565,   605,     0,   596,    54,   604,
       0,     0,   447,     0,     0,   453,   450,   451,   459,     0,
     438,   433,     0,   436,     0,     0,     0,     0,    39,    43,
       0,   213,   221,   218,     0,     0,   770,   773,   772,   771,
      11,   802,     0,     0,     0,   732,   729,     0,   421,   769,
     768,   767,     0,   763,     0,   764,   766,     0,     5,   321,
       0,     0,   466,   467,   475,   474,     0,     0,   618,   416,
     420,     0,   784,     0,   799,   666,   196,   822,     0,     0,
     683,   667,   674,   713,     0,   791,   210,   564,   620,   207,
       0,   666,     0,     0,   144,   403,   118,   380,     0,   425,
     426,     0,   423,   618,   745,     0,     0,   208,   146,   144,
     142,     0,   724,   332,     0,     0,   208,   671,   672,   685,
     709,   710,     0,     0,     0,   645,   626,   627,   628,   629,
       0,     0,     0,    25,   637,   636,   651,   619,     0,   659,
     743,   742,     0,   721,   667,   676,   579,     0,   182,     0,
       0,    62,     0,     0,     0,     0,     0,     0,   152,   153,
     164,     0,    56,   162,    87,   172,     0,   172,     0,     0,
     817,     0,   618,   808,   810,   797,   796,     0,   794,   598,
     599,   625,     0,   619,   617,     0,     0,   419,   617,     0,
     752,   432,     0,     0,     0,   455,   456,   454,     0,     0,
     434,     0,   106,     0,   109,    91,    44,   217,     0,   780,
      82,     0,     0,   790,   145,   147,   223,     0,     0,   730,
       0,   762,     0,    16,     0,   143,   223,     0,     0,   413,
       0,   785,     0,     0,     0,   195,   823,     0,   199,     0,
     669,   668,   794,     0,   211,    53,     0,   666,   141,     0,
     666,     0,   379,   749,   748,     0,   208,     0,     0,     0,
     144,   120,   580,   670,   208,     0,     0,   632,   633,   634,
     635,   638,   639,   649,     0,   619,   645,     0,   631,   653,
     645,   618,   656,   658,   660,     0,   737,   670,     0,     0,
       0,     0,   179,   410,    67,     0,   319,   154,   732,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   166,     0,
     805,   806,     0,     0,   821,     0,   602,   618,   616,     0,
     607,     0,   619,     0,     0,   608,   606,   756,     0,   619,
     445,     0,   446,   452,   460,   461,     0,    56,   219,   804,
     801,     0,   289,   734,   732,   323,   326,   330,     0,    14,
     289,   478,     0,     0,   480,   473,   476,     0,   471,     0,
     786,   800,   408,     0,   200,     0,     0,     0,   208,   212,
     799,     0,   223,     0,   666,     0,   208,     0,   705,   223,
     223,     0,     0,   333,   208,     0,   699,     0,   642,   618,
     644,     0,   630,     0,     0,   619,     0,   650,   741,     0,
      56,     0,   175,   161,     0,     0,   151,    85,   165,     0,
       0,   168,     0,   173,   174,    56,   167,   818,   795,     0,
     624,   623,   600,     0,   618,   418,   603,   601,     0,   424,
     618,   751,     0,     0,     0,     0,   148,     0,     0,     0,
     287,     0,     0,     0,   127,   222,   224,     0,   286,     0,
     289,     0,   765,   131,   469,     0,     0,   412,     0,   203,
       0,   202,   670,   208,     0,   400,   799,   289,   799,     0,
     747,     0,   704,   289,   289,   223,   666,     0,   698,   648,
     647,   640,     0,   643,   618,   652,   641,    56,   181,    63,
      68,   155,     0,   163,   169,    56,   171,     0,     0,   415,
       0,   755,   754,     0,    56,   110,   803,     0,     0,     0,
       0,   149,   254,   252,   566,    24,     0,   248,     0,   253,
     264,     0,   262,   267,     0,   266,     0,   265,     0,   114,
     226,     0,   228,     0,   731,   470,   468,   479,   477,   204,
       0,   201,   208,     0,   702,     0,     0,     0,   123,   400,
     799,   706,   129,   133,   289,     0,   700,     0,   655,     0,
     177,     0,    56,   158,    86,   170,   820,   622,     0,     0,
       0,     0,     0,     0,     0,     0,   238,   242,     0,     0,
     233,   530,   529,   526,   528,   527,   547,   549,   548,   518,
     508,   524,   523,   485,   495,   496,   498,   497,   517,   501,
     499,   500,   502,   503,   504,   505,   506,   507,   509,   510,
     511,   512,   513,   514,   516,   515,   486,   487,   488,   491,
     492,   494,   532,   533,   542,   541,   540,   539,   538,   537,
     525,   544,   534,   535,   536,   519,   520,   521,   522,   545,
     546,   550,   552,   551,   553,   554,   531,   556,   555,   489,
     558,   560,   559,   493,   563,   561,   562,   557,   490,   543,
     484,   259,   481,     0,   234,   280,   281,   279,   272,     0,
     273,   235,   306,     0,     0,     0,     0,   114,     0,   206,
       0,   701,     0,    56,   282,    56,   117,     0,     0,   125,
     799,   646,     0,    56,   156,    69,     0,   414,   753,   448,
     108,   236,   237,   309,   150,     0,     0,   256,   249,     0,
       0,     0,   261,   263,     0,     0,   268,   275,   276,   274,
       0,     0,   225,     0,     0,     0,     0,   205,   703,     0,
     464,   621,     0,     0,    56,   119,     0,   654,     0,     0,
       0,    88,   239,    45,     0,   240,   241,     0,     0,   255,
     258,   482,   483,     0,   250,   277,   278,   270,   271,   269,
     307,   304,   229,   227,   308,     0,   465,   620,     0,   402,
     283,     0,   121,     0,   159,   449,     0,   112,     0,   289,
     257,   260,     0,   666,   231,     0,   462,   399,   404,   157,
       0,     0,    89,   246,     0,   288,   305,   185,     0,   621,
     300,   666,   463,     0,   111,     0,     0,   245,   799,   666,
     184,   301,   302,   303,   823,   299,     0,     0,     0,   244,
       0,   183,   300,     0,   799,     0,   243,   284,    56,   230,
     823,     0,   187,     0,    56,     0,     0,   188,     0,   232,
       0,   285,     0,   191,     0,   190,   107,   192,     0,   189,
       0,   194,   193
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   118,   738,   517,   176,   264,   265,
     120,   121,   122,   123,   124,   125,   309,   541,   542,   436,
     231,  1241,   442,  1172,  1457,   706,   261,   478,  1421,   885,
    1017,  1472,   325,   177,   543,   772,   931,  1062,   544,   559,
     790,   501,   788,   545,   522,   789,   327,   280,   297,   131,
     774,   741,   724,   894,  1190,   980,   838,  1375,  1244,   658,
     844,   441,   666,   846,  1095,   651,   828,   831,   970,  1478,
    1479,   533,   534,   553,   554,   269,   270,   274,  1022,  1125,
    1208,  1355,  1463,  1481,  1385,  1425,  1426,  1427,  1196,  1197,
    1198,  1386,  1392,  1434,  1201,  1202,  1206,  1348,  1349,  1350,
    1366,  1509,  1126,  1127,   178,   133,  1494,  1495,  1353,  1129,
     134,   224,   437,   438,   135,   136,   137,   138,   139,   140,
     141,   142,  1226,   143,   771,   930,   144,   228,   304,   432,
     526,   527,  1002,   528,  1003,   145,   146,   147,   685,   148,
     149,   258,   150,   259,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   696,   697,   877,   475,   476,   477,   703,
    1411,   151,   523,  1216,   524,   907,   746,  1038,  1035,  1341,
    1342,   152,   153,   154,   218,   225,   312,   422,   155,   861,
     689,   156,   862,   416,   756,   863,   815,   951,   953,   954,
     955,   817,  1074,  1075,   818,   632,   407,   186,   187,   157,
     536,   390,   391,   762,   158,   219,   180,   160,   161,   162,
     163,   164,   165,   166,   589,   167,   221,   222,   504,   210,
     211,   592,   593,  1008,  1009,   289,   290,   732,   168,   494,
     169,   531,   170,   251,   281,   320,   451,   857,   914,   722,
     669,   670,   671,   252,   253,   758
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1223
static const yytype_int16 yypact[] =
{
   -1223,   107, -1223, -1223,  3901, 11577, 11577,   -70, 11577, 11577,
   11577, -1223, 11577, 11577, 11577, 11577, 11577, 11577, 11577, 11577,
   11577, 11577, 11577, 11577, 13644, 13644,  8951, 11577, 13709,   -51,
     -46, -1223, -1223, -1223, -1223, -1223,   149, -1223, -1223, 11577,
   -1223,   -46,   -35,   -33,   -20,   -46,  9153,  9599,  9355, -1223,
   13090,  8547,   109, 11577, 13109,   -18, -1223, -1223, -1223,   212,
     221,    21,   155,   189,   192,   200, -1223,  9599,   213,   220,
   -1223, -1223, -1223, -1223, -1223,   412, 13882, -1223, -1223,  9599,
    9557, -1223, -1223, -1223, -1223, -1223, -1223,  9599, -1223,    64,
     229,  9599,  9599, -1223, 11577, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, 11577, -1223, -1223,   173,   252,   431,   431, -1223,   399,
     287,   205, -1223,   245, -1223,    36, -1223,   439, -1223, -1223,
   -1223, 14031,   667, -1223, -1223,   293,   302,   304,   308,   318,
     334, 12432, -1223, -1223, -1223, -1223,   495, -1223,   512,   514,
     350, -1223,   126,   284,   437, -1223, -1223,   485,   122,  1055,
     128,   407,   154,   156,   416,    24, -1223,   -13, -1223,   526,
   -1223, -1223, -1223,   475,   436,   487, -1223,   439,   667, 14556,
    1621, 14556, 11577, 14556, 14556,  2411,   591,  9599, -1223, -1223,
     586, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, 13320,   480, -1223,   500,   525,   525, 13644, 14213,
     447,   642, -1223,   475, 13320,   480,   513,   517,   462,   158,
   -1223,   546,   128,  9759, -1223, -1223, 11577,  7133,   660,    45,
   14556,  8143, -1223, 11577, 11577,  9599, -1223, -1223, 12473,   476,
   -1223, 12514, 13090, 13090,   507, -1223, -1223,   479,  3159,   662,
   -1223,   664, -1223,  9599,   605, -1223,   489, 12561,   491,   696,
   -1223,    35, 12602,  9599,    56, -1223,   -21, -1223, 13456,    63,
   -1223, -1223, -1223,   678,    65, 13644, 13644, 11577,   497,   530,
   -1223, -1223, 13506,  8951,     8,   401, -1223, 11779, 13644,   420,
   -1223,  9599, -1223,   347,   287,   499, 14254, -1223, -1223, -1223,
     616,   684,   607, 14556,    16,   506, 14556,   509,    98,  4103,
   11577,   241,   505,   540,   241,   336,   289, -1223,  9599, 13090,
     523,  9961, 13090, -1223, -1223,  8387, -1223, -1223, -1223, -1223,
     439, -1223, -1223, -1223, -1223, -1223, -1223, -1223, 11577, 11577,
   11577, 10163, 11577, 11577, 11577, 11577, 11577, 11577, 11577, 11577,
   11577, 11577, 11577, 11577, 11577, 11577, 11577, 11577, 11577, 11577,
   11577, 11577, 11577, 13709, -1223, 11577, 11577, -1223, 11577,  1586,
    9599,  9599, 14031,   608,   425,  8345, 11577, 11577, 11577, 11577,
   11577, 11577, 11577, 11577, 11577, 11577, 11577, -1223, -1223,  1801,
   -1223,   176, 11577, 11577, -1223,  9961, 11577, 11577,   173,   246,
   13506,   524,   439, 10365, 12643, -1223,   528,   698, 13320,   516,
     -48,  2594,   525, 10567, -1223, 10769, -1223,   529,    14, -1223,
      -2,  9961, -1223, 12755, -1223,   248, -1223, -1223, 12684, -1223,
   -1223, 10971, -1223, 11577, -1223,   625,  7335,   711,   531, 14448,
     713,    44,    39, -1223, -1223, -1223, -1223, -1223, 13090,   649,
     537,   725, -1223, 13181, -1223,   554, -1223, -1223, -1223,   661,
   11577,   668,   669, 11577, 11577, 11577, -1223,   696, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223,   561, -1223, -1223, -1223,   553,
   -1223, -1223,    18, 13109, -1223,  9599, 11577,   525,   -18, -1223,
   13181,   676, -1223,   525,    83,    84,   558,   562,   965,   564,
    9599,   638,   567,   525,    86,   569, 14014,  9599, -1223, -1223,
     701,  2091,   -56, -1223, -1223, -1223,   287, -1223, -1223, -1223,
   -1223, 11577,   647,   609,   224, -1223,   648,   775,   589, 13090,
   13090,   781, -1223,   603,   788, -1223, 13090,   238,   736,   130,
   -1223, -1223, -1223, -1223, -1223, -1223,  2985, -1223, -1223, -1223,
   -1223,    73, 13644,   606,   794, 14556,   790, -1223, -1223,   685,
    8791, 14596,  3889,  2411, 11577, 14515,  4290,  4491,  3069,   777,
    1654,  2693,  2693,  2693,  2693,  1678,  1678,  1678,  1678,   792,
     792,   470,   470,   470,   586,   586,   586, -1223, 14556,   627,
     644, 14310,   645,   841, -1223, 11577,   124,   653,   246, -1223,
   -1223, -1223,   439, 13372, 11577, -1223, -1223,  2411, -1223,  2411,
    2411,  2411,  2411,  2411,  2411,  2411,  2411,  2411,  2411,  2411,
   11577,   124,   656,   651,  3117,   659,   658,  3160,    88,   663,
   -1223, 13272, -1223,  9599, -1223,   506,   238,   480, 13644, 14556,
   13644, 14351,   239,   250, -1223,   671, 11577, -1223, -1223, -1223,
    6931,    74, -1223, 14556, 14556,   -46, -1223, -1223, -1223, 11577,
    1304, 13181,  9599,  7537,   675,   681, -1223,    99,   744, -1223,
     868,   687, 12944, 13090, 13181, 13181, 13181,   689,    23,   737,
     692,   694,   198, -1223,   747, -1223,   693, -1223, -1223, -1223,
   11577,   712, 14556,   714,   878, 12772,   885, -1223, 14556, 12731,
   -1223,   561,   821, -1223,  4305, 13947,   699,  9599, -1223, -1223,
    3400, -1223, -1223,   884, 13644,   702, -1223, -1223, -1223, -1223,
   -1223,   810,   119, 13947,   705, 13506, 13590,   892, -1223, -1223,
   -1223, -1223,   706, -1223, 11577, -1223, -1223,  3495, -1223, 14556,
   13947,   716, -1223, -1223, -1223, -1223,   895, 11577,   616, -1223,
   -1223,   718, -1223, 13090,   889,   137, -1223, -1223,   343, 13763,
   -1223,   251, -1223, -1223, 13090, -1223,   525, -1223, 11173, -1223,
   13181,   138,   726, 13947,   647, -1223, -1223,  4089, 11577, -1223,
   -1223, 11577, -1223, 11577, -1223,  3446,   728,  9961,   638,   647,
     685,  9599, 13709,   525, 11965,   730,  9961, -1223, -1223,   275,
   -1223, -1223,   897, 13691, 13691, 13272, -1223, -1223, -1223, -1223,
     731,    49,   732,   734, -1223, -1223, -1223,   911,   739,   528,
     525,   525, 11375, -1223,   276, -1223, -1223, 12006,   337,   -46,
    8143, -1223,  4507,   733,  4709,   735, 13644,   738,   812,   525,
   -1223,   923, -1223, -1223, -1223, -1223,   434, -1223,   281, 13090,
   -1223, 13090,   649, -1223, -1223, -1223,   929,   743,   746, -1223,
   -1223,   816,   741,   936, 13181,   807,  9599,   616, 13181,  8993,
   13181, 14556, 11577, 11577, 11577, -1223, -1223, -1223, 11577, 11577,
   -1223,   696, -1223,   874, -1223, -1223, -1223, -1223, 13181,   525,
   -1223, 13090,  9599, -1223,   944, -1223, -1223,    91,   759,   525,
    8749, -1223,  1548, -1223,  3699,   944, -1223,   249,   222, 14556,
     831, -1223,   761, 13090,   660, -1223, 13090,   886,   945, 11577,
     124,   765, -1223, 13644, 14556, -1223,   767,   138, -1223,   768,
     138,   770,  4089, 14556, 14407,   773,  9961,   774,   771,   778,
     647, -1223,   462,   776,  9961,   795, 11577, -1223, -1223, -1223,
   -1223, -1223, -1223,   849,   791,   968, 13272,   854, -1223,   616,
   13272, 13272, -1223, -1223, -1223, 13644, 14556, -1223,   -46,   969,
     930,  8143, -1223, -1223, -1223,   803, 11577,   525, 13506,  1304,
     805, 13181,  4911,   545,   806, 11577,    79,   285, -1223,   838,
   -1223, -1223, 13024,   981, -1223, 13181, -1223, 13181, -1223,   814,
   -1223,   887,  1012,   826,   827, -1223, -1223,   900,   828,  1019,
   14556, 12855, 14556, -1223, 14556, -1223,   833, -1223, -1223, -1223,
     938, 13947,    60, -1223, 13506, -1223, -1223,  2411,   832, -1223,
      94, -1223,    52, 11577, -1223, -1223, -1223, 11577, -1223, 11577,
   -1223, -1223, -1223,   346,  1018, 13181, 12047,   842,  9961,   525,
     889,   837, -1223,   840,   138, 11577,  9961,   843, -1223, -1223,
   -1223,   836,   845, -1223,  9961,   847, -1223, 13272, -1223, 13272,
   -1223,   848, -1223,   914,   850,  1032,   851, -1223,   525,  1021,
   -1223,   852, -1223, -1223,   856,    92, -1223, -1223, -1223,   857,
     858, -1223, 12391, -1223, -1223, -1223, -1223, -1223, -1223, 13090,
   -1223,   921, -1223, 13181,   616, -1223, -1223, -1223, 13181, -1223,
   13181, -1223, 11577,   855,  5113, 13090, -1223,   391, 13090, 13947,
   -1223, 13930,   903,  1743, -1223, -1223, -1223,   608, 12878,    67,
     425,    96, -1223, -1223,   910, 12088, 12137, 14556,   986,  1048,
   13181, -1223,   869,  9961,   870,   961,   889,   356,   889,   872,
   14556,   888, -1223,   413,   680, -1223,   138,   890, -1223, -1223,
     957, -1223, 13272, -1223,   616, -1223, -1223, -1223,  6931, -1223,
   -1223, -1223,  7739, -1223, -1223, -1223,  6931,   891, 13181, -1223,
     959, -1223,   966, 12197, -1223, -1223, -1223, 13947, 13947,  1071,
      59, -1223, -1223, -1223,    69,   893,    70, -1223, 12209, -1223,
   -1223,    71, -1223, -1223,  1131, -1223,   896, -1223,  1014,   439,
   -1223, 13090, -1223,   608, -1223, -1223, -1223, -1223, -1223,  1078,
   13181, -1223,  9961,   898, -1223,   904,   902,   393, -1223,   961,
     889, -1223, -1223, -1223,   906,   899, -1223, 13272, -1223,   979,
    6931,  7941, -1223, -1223, -1223,  6931, -1223, -1223, 13181, 13181,
   11577,  5315,   907,   909, 13181, 13947, -1223, -1223,   478, 13930,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223,   415, -1223,   903, -1223, -1223, -1223, -1223, -1223,    81,
     363, -1223,  1095,    72,  9599,  1014,  1097,   439, 13181, -1223,
     916, -1223,   358, -1223, -1223, -1223, -1223,   919,   393, -1223,
     889, -1223, 13272, -1223, -1223, -1223,  5517, -1223, -1223, 12814,
   -1223, -1223, -1223, -1223, -1223, 13828,    51, -1223, -1223, 13181,
   12209, 12209,  1066, -1223,  1131,  1131,   532, -1223, -1223, -1223,
   13181,  1045, -1223,   926,    75, 13181,  9599, -1223, -1223,  1047,
   -1223,  1114,  5719,  5921, -1223, -1223,   393, -1223,  6123,   927,
    1050,  1022, -1223,  1036,   987, -1223, -1223,  1037,   478, -1223,
   -1223, -1223, -1223,   975, -1223,  1102, -1223, -1223, -1223, -1223,
   -1223,  1119, -1223, -1223, -1223,   940, -1223,   370,   942, -1223,
   -1223,  6325, -1223,   946, -1223, -1223,   949,   977,  9599,   425,
   -1223, -1223, 13181,   139, -1223,  1070, -1223, -1223, -1223, -1223,
   13947,   699, -1223,   988,  9599,   474, -1223, -1223,   951,  1139,
     460,   139, -1223,  1074, -1223, 13947,   954, -1223,   889,   151,
   -1223, -1223, -1223, -1223, 13090, -1223,   956,   958,    76, -1223,
     402, -1223,   460,   429,   889,   955, -1223, -1223, -1223, -1223,
   13090,  1080,  1142,   402, -1223,  6527,   465,  1143, 13181, -1223,
    6729, -1223,  1084,  1146, 13181, -1223, -1223,  1149, 13181, -1223,
   13181, -1223, -1223
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1223, -1223, -1223,  -475, -1223, -1223, -1223,    -4, -1223,   682,
      15,   894,   839, -1223,  1547, -1223,  -401, -1223,     3, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,  -305,
   -1223, -1223,  -174,    13,     0, -1223, -1223, -1223,     1, -1223,
   -1223, -1223, -1223,     9, -1223, -1223,   799,   809,   813,  1020,
     395,  -665,   398,   450,  -298, -1223,   214, -1223, -1223, -1223,
   -1223, -1223, -1223,  -621,   100, -1223, -1223, -1223, -1223,  -290,
   -1223,  -731, -1223,  -373, -1223, -1223,   709, -1223,  -874, -1223,
   -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,   -58,
   -1223, -1223, -1223, -1223, -1223,  -141, -1223,    82,  -781, -1223,
   -1222,  -304, -1223,  -155,    20,  -131,  -294, -1223,  -145, -1223,
     -62,   -22,  1172,  -618,  -349, -1223, -1223,   -31, -1223, -1223,
    2479,   -52,   -17, -1223, -1223, -1223, -1223, -1223, -1223,   300,
    -712, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
   -1223,   844, -1223, -1223,   339, -1223,   748, -1223, -1223, -1223,
   -1223, -1223, -1223, -1223,   340, -1223,   750, -1223, -1223,   520,
   -1223,   315, -1223, -1223, -1223, -1223, -1223, -1223, -1223, -1223,
    -765, -1223,  1073,  2347,  -334, -1223, -1223,   282,  1165,  1872,
   -1223, -1223,  -717,  -374,  -545, -1223, -1223,   419,  -616,  -806,
   -1223, -1223, -1223, -1223, -1223,   406, -1223, -1223, -1223,  -283,
    -721,  -186,  -182,  -133, -1223, -1223,    27, -1223, -1223, -1223,
   -1223,   -10,  -137, -1223,  -262, -1223, -1223, -1223,  -386,   947,
   -1223, -1223, -1223, -1223, -1223,   538,   400, -1223, -1223,   952,
   -1223, -1223, -1223,  -310,   -81,  -199,  -281, -1223, -1013, -1223,
     375, -1223, -1223, -1223,  -187,  -891
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -793
static const yytype_int16 yytable[] =
{
     119,   331,   373,   401,   128,   129,   298,   126,   227,   769,
     301,   302,   556,   130,   628,   816,   419,   127,   220,   232,
     256,   535,   625,   236,   132,  1043,   605,   394,   650,   587,
     399,   159,  1030,   634,   916,   424,   910,  1145,   551,   425,
     926,   835,   737,   239,   848,   322,   249,   331,   645,   305,
     328,   206,   207,   662,   433,   446,   447,   267,    11,   307,
    1428,   452,   200,   279,  1117,   483,   664,   389,  1255,   266,
     506,   485,   488,   200,   491,   389,  1211,   704,  -251,  1259,
    1343,  1401,   764,   279,  1401,  1255,   396,   279,   279,   426,
    1394,   293,   714,   714,   294,   726,   273,   726,  1117,  1134,
     726,   726,    11,   865,   590,   726,  1093,     3,   849,   929,
     707,  1395,   403,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   939,   182,   318,   279,   892,   957,
     623,   507,   452,  1227,   626,  1229,    11,   389,   308,   479,
     409,   829,   830,   736,   223,   330,  1415,   999,   539,   226,
    1071,  1004,   417,   749,  1076,  1001,   560,  -405,   387,   388,
     233,  1118,   234,   286,  -568,   318,  1119,  -569,    56,    57,
      58,   171,   172,   329,  1120,   235,   532,   374,  1147,    11,
      11,    11,   268,   406,  -690,  1153,  1154,   398,  -687,   958,
     392,   402,  -694,    11,  1452,  1118,  1051,   480,   114,  1053,
    1119,   299,    56,    57,    58,   171,   172,   329,  1120,  -198,
     643,  1121,  1122,   299,  1123,   496,  -688,  1368,  -689,   784,
    -725,   389,   397,   119,   983,   497,   987,   119,   629,   410,
     430,   440,   598,   323,   435,   412,    95,   665,  -691,   765,
     663,   418,   434,   331,   558,  1121,  1122,  1073,  1123,   454,
    1429,   832,   598,   484,   159,   834,  1256,  1257,   159,  1124,
     489,   667,   492,   904,  1212,   850,  -251,  1260,  1344,  1402,
      95,   743,  1443,  1506,   598,  1061,  1094,  1396,   482,   715,
     716,  1234,   727,   598,   802,   893,   598,  1023,  1171,   319,
     298,   328,  1214,  1133,   539,   487,   853,   915,   532,  1477,
     392,   392,   493,   493,   498,   119,   516,   260,   396,   503,
    -726,  1501,  -728,  -692,   550,   512,   284,  -696,   249,  -792,
    -690,   279,   127,  1149,  -687,  -697,   393,   284,  -694,   132,
    -620,   271,   311,  -620,  -198,  -186,   159,  -693,  -727,   897,
     272,  -792,   751,   752,   606,  1077,   635,  -620,  -792,   757,
     275,   760,  -688,   220,  -689,   761,  -725,  1416,  1084,   597,
    1117,   759,   423,   319,   284,   596,   279,   279,   279,   513,
     319,   310,  1036,   744,  -691,   917,   985,   986,  1138,   622,
     985,   986,   287,   288,   276,   621,   602,   277,   745,  1397,
    1409,   856,  1180,   287,   288,   278,  -792,   786,    11,  1031,
    -792,   597,  1465,  -792,   968,   969,  1398,   637,   282,  1399,
     644,   284,  1032,   648,   937,   283,   513,  1117,   918,   647,
    1037,  1139,   795,   945,   300,  1235,   317,   503,   791,  1389,
     287,   288,   119,  1410,   318,   410,   393,   393,   786,   657,
     321,   982,  1390,   962,   397,  1466,  -726,  1033,  -728,  -692,
     760,  1159,  1239,  1160,   761,    11,   823,  1118,   942,  1391,
     824,  1511,  1119,   159,    56,    57,    58,   171,   172,   329,
    1120,   324,   535,  -693,  -727,  1500,   284,   287,   288,   776,
     988,   709,   368,   419,  1096,   452,   858,   284,   535,   998,
     332,  1513,   285,   549,   318,   284,   721,  1522,   266,   333,
     513,   334,   731,   733,  1512,   335,   284,  1121,  1122,   825,
    1123,   314,  1187,  1188,  1118,   336,   360,   361,   362,  1119,
     363,    56,    57,    58,   171,   172,   329,  1120,   984,   985,
     986,   337,    95,    56,    57,    58,   171,   172,   329,   548,
    1523,   508,   287,   288,   518,   519,  1238,   367,    31,    32,
      33,  1025,   286,   287,   288,  1228,   279,  -427,  1437,    38,
     514,   287,   288,  1057,  1121,  1122,   912,  1123,  1491,  1492,
    1493,  1065,   287,   288,   365,  1438,   366,   922,  1439,   766,
     369,  1070,    56,    57,    58,   171,   172,   329,  -428,    95,
    1364,  1365,  1085,    56,    57,    58,    59,    60,   329,  1507,
    1508,    95,   395,  1503,    66,   370,    70,    71,    72,    73,
      74,  -695,  1232,  1435,  1436,   284,  1114,   678,  -568,  1516,
     513,  1371,   598,    77,    78,  1431,  1432,   814,  1105,   819,
     793,   400,   405,   833,   291,  1111,   363,    88,  1131,  1090,
     985,   986,   371,   411,   535,   319,   119,   535,   389,   414,
      95,   415,    93,   313,   315,   316,  -567,   421,   841,   119,
     420,    95,   989,   127,   990,   820,   843,   821,   431,   423,
     132,  1487,   448,   444,   449,  1144,  -787,   159,   453,  1168,
     455,   287,   288,  1151,  1117,   509,   456,   839,   458,   515,
     159,  1157,   490,   499,  1176,   500,   520,   525,   529,   530,
     119,  1165,   537,   886,  1019,   538,   547,   633,    49,   655,
     941,   509,   636,   515,   509,   515,   515,   127,   -51,   557,
     433,  1047,    11,   631,   132,   642,  1041,   661,   659,   757,
     668,   159,   672,   119,   673,   690,   691,   128,   129,  1130,
     126,   889,  1480,   693,   694,   702,   130,  1130,   705,   921,
     127,   713,   503,   899,   717,   920,  1417,   132,   718,   723,
    1480,   720,   725,   734,   159,   728,  1240,    49,  1502,   740,
    1223,   535,   747,   742,  1245,    56,    57,    58,   171,   172,
     329,  1118,   220,  1251,   748,   750,  1119,   279,    56,    57,
      58,   171,   172,   329,  1120,   753,   754,   755,  -429,   950,
     950,   814,   767,   768,   770,   922,   773,   971,  1189,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   119,   363,   119,   779,
     119,  1121,  1122,   972,  1123,   357,   358,   359,   360,   361,
     362,  1376,   363,    95,   782,   127,   780,   127,   787,  1360,
     783,   796,   132,   797,   132,   799,    95,   159,   775,   159,
     800,   159,  1000,   977,  1130,  1006,  1448,   826,   851,  1026,
    1130,  1130,   845,   535,   459,   460,   461,   852,   847,  1233,
     866,   462,   463,   854,   864,   464,   465,   867,  1020,   868,
     869,   870,   874,   872,   878,   873,   881,   884,   888,   890,
     119,  1356,   891,   896,   128,   129,   900,   126,   901,   908,
    1117,   946,  1177,   130,   906,   911,   913,   127,   203,   203,
     961,   927,   215,   936,   132,   944,   956,   959,  1186,   960,
     974,   159,   976,   978,  1490,   963,   979,   981,   992,   993,
     995,  1210,   994,   996,   215,   997,  1079,   508,    11,  1016,
    1049,  1130,   814,  1021,  1024,  1039,   814,   814,  1040,  1045,
    1048,  1044,  1412,  1050,  1413,  1054,  1052,   119,  1056,  1059,
    1058,  1064,  1418,  1067,  1082,  1213,  1060,  1069,   119,   403,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,  1066,  1078,  1068,  1072,   127,  1080,   331,   159,  1081,
    1083,  1087,   132,  1091,  1097,   503,   839,  1118,  1099,   159,
    1102,  1103,  1119,  1451,    56,    57,    58,   171,   172,   329,
    1120,  1104,  1106,  1107,  1108,   387,   388,  1109,  1110,  1113,
    1115,  1132,  1140,  1146,  1155,  1354,  1148,  1143,  1162,  1152,
    1156,  1164,  1128,  1158,  1161,  1178,  1163,  1166,  1167,  1169,
    1128,   503,  1170,  1184,  1173,  1174,  1200,  1121,  1122,  1215,
    1123,  1219,  1220,   814,  1222,   814,  1224,  1225,  1230,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,  1237,    95,  1248,  1231,  1254,  1236,  1246,   389,  1352,
    1249,  1258,  1358,  1351,  1361,  1370,   203,   204,   204,  1362,
    1363,   216,   203,  1372,  1381,  1369,  1382,  1515,   203,  1400,
     119,  1405,  1408,  1520,   249,   387,   388,  1414,  1433,  1205,
    1441,  1442,  1446,  1447,  1454,  1455,  1456,   127,  -247,  1459,
    1458,  1461,  1395,  1462,   132,  1464,   215,   215,  1467,  1471,
    1209,   159,   215,  1469,  1470,  1482,  1485,  1488,  1489,  1497,
     374,  1499,  1504,  1514,  1505,  1517,  1518,  1524,   814,  1527,
    1528,   719,   203,  1530,   119,   708,  1484,  1128,   119,   203,
     203,   601,   119,  1128,  1128,  1243,   203,   372,   389,   599,
     535,   127,   203,  1406,   600,   940,   938,  1498,   132,   127,
     905,  1496,  1175,  1086,  1340,   159,   132,   711,   535,   159,
    1347,  1388,  1393,   159,    34,  1207,   535,   249,  1510,  1519,
    1404,   229,  1367,   215,  1042,   700,   215,   701,  1013,   608,
    1015,   880,  1034,   952,  1063,   964,  1357,   991,   495,     0,
     505,     0,     0,   814,     0,     0,   119,   119,     0,     0,
       0,   119,     0,     0,  1374,     0,     0,   119,     0,     0,
       0,     0,     0,   127,  1128,     0,     0,   215,   127,     0,
     132,     0,     0,     0,   127,   132,     0,   159,   159,     0,
       0,   132,   159,  1403,     0,     0,     0,     0,   159,     0,
    1345,   204,    82,    83,  1346,    84,    85,    86,     0,     0,
       0,     0,   683,     0,   203,     0,     0,     0,     0,     0,
       0,     0,   203,     0,  1474,     0,     0,   757,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   757,     0,  1445,  1204,     0,     0,   683,
       0,     0,     0,     0,     0,     0,   836,     0,     0,     0,
       0,   204,   215,     0,   331,     0,     0,   682,   204,   204,
     279,     0,     0,     0,     0,   204,     0,     0,     0,     0,
       0,   204,     0,     0,     0,     0,     0,     0,   814,     0,
       0,     0,   119,     0,     0,     0,     0,    34,     0,   200,
       0,  1423,     0,     0,   682,     0,  1340,  1340,     0,   127,
    1347,  1347,     0,     0,     0,     0,   132,     0,     0,     0,
       0,     0,   279,   159,     0,     0,     0,     0,   119,   119,
       0,     0,     0,     0,   119,     0,     0,   201,     0,     0,
       0,     0,     0,   215,   215,   127,   127,     0,     0,   837,
     215,   127,   132,   132,     0,     0,   216,     0,   132,   159,
     159,     0,     0,     0,     0,   159,   203,   119,   175,     0,
       0,    79,     0,    81,  1473,    82,    83,     0,    84,    85,
      86,     0,     0,     0,   127,     0,     0,     0,     0,     0,
    1486,   132,     0,   204,     0,     0,     0,     0,   159,  1475,
       0,    96,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,   203,     0,   202,
     683,     0,     0,     0,   114,     0,     0,     0,     0,     0,
       0,   119,     0,   683,   683,   683,   119,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   686,     0,   127,     0,
       0,     0,   203,   127,   203,   132,     0,     0,     0,     0,
     132,     0,   159,     0,     0,     0,     0,   159,     0,     0,
       0,     0,     0,     0,   203,   682,     0,     0,   338,   339,
     340,     0,     0,   686,     0,     0,   215,   215,   682,   682,
     682,     0,     0,     0,   341,     0,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   250,   363,   215,
       0,     0,     0,     0,     0,     0,     0,     0,   203,   683,
       0,     0,     0,     0,     0,     0,     0,   215,   687,   203,
     203,     0,     0,     0,     0,   204,     0,     0,     0,     0,
       0,     0,     0,     0,   215,   403,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,   386,   215,     0,     0,
       0,     0,     0,     0,     0,   687,     0,     0,   215,    34,
       0,   200,     0,     0,   682,     0,     0,   215,     0,     0,
       0,     0,     0,     0,     0,     0,   204,     0,     0,     0,
       0,   387,   388,     0,     0,     0,   215,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   683,   363,   594,     0,   683,     0,   683,
       0,   204,     0,   204,     0,  -793,  -793,  -793,  -793,   355,
     356,   357,   358,   359,   360,   361,   362,   683,   363,     0,
     203,     0,     0,   204,   686,     0,     0,    82,    83,     0,
      84,    85,    86,   215,   389,   215,     0,   686,   686,   686,
    1028,     0,     0,     0,     0,     0,     0,     0,   682,     0,
       0,     0,   682,    96,   682,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   883,     0,
       0,     0,   682,     0,   595,   215,   114,   204,     0,   250,
     250,     0,     0,     0,     0,   250,   895,     0,   204,   204,
       0,     0,     0,     0,     0,     0,     0,   215,     0,     0,
     215,     0,     0,   895,     0,     0,    34,   203,     0,     0,
     683,     0,     0,     0,     0,     0,   687,     0,     0,     0,
       0,     0,     0,     0,   683,     0,   683,     0,     0,   687,
     687,   687,     0,   686,     0,     0,   928,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   203,
    1203,     0,     0,     0,     0,   216,   250,     0,     0,   250,
       0,     0,   203,   203,    34,   682,   200,     0,     0,     0,
       0,     0,     0,     0,   683,     0,   215,     0,     0,   682,
       0,   682,     0,     0,    82,    83,     0,    84,    85,    86,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   204,
       0,     0,     0,     0,     0,   215,     0,     0,   203,     0,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   687,     0,   686,  1204,   682,
       0,   686,   683,   686,     0,     0,     0,   683,     0,   683,
       0,     0,    82,    83,     0,    84,    85,    86,     0,     0,
       0,   686,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    96,   683,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   215,     0,   250,   204,   682,     0,   620,
     684,   114,   682,     0,   682,     0,     0,     0,     0,   215,
       0,     0,   215,   215,     0,   215,     0,   683,     0,     0,
       0,     0,   215,     0,     0,     0,     0,     0,     0,   687,
       0,     0,     0,   687,   682,   687,     0,   684,   204,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   204,   204,   687,   686,     0,     0,     0,     0,   683,
       0,     0,     0,     0,     0,     0,     0,     0,   686,     0,
     686,     0,   682,     0,     0,     0,   250,   250,     0,     0,
       0,   215,   215,   250,     0,     0,     0,   683,   683,     0,
       0,     0,     0,   683,  1116,     0,     0,   204,     0,     0,
       0,   338,   339,   340,     0,   215,     0,     0,     0,     0,
       0,     0,     0,     0,   682,     0,     0,   341,   686,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
       0,   363,   682,   682,     0,     0,   687,     0,   682,   215,
       0,     0,     0,   215,     0,     0,     0,     0,     0,     0,
     687,     0,   687,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   686,     0,     0,     0,
       0,   686,     0,   686,     0,     0,     0,     0,     0,     0,
       0,     0,  1191,     0,  1199,     0,     0,   683,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   684,     0,
     687,     0,     0,   686,     0,     0,     0,     0,     0,   250,
     250,   684,   684,   684,     0,     0,     0,     0,   683,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   683,
       0,     0,     0,     0,   683,     0,     0,     0,     0,     0,
       0,   686,   682,     0,     0,     0,     0,     0,     0,     0,
    1252,  1253,     0,     0,     0,     0,     0,     0,   687,     0,
       0,     0,     0,   687,     0,   687,     0,     0,     0,   215,
       0,     0,     0,   682,     0,     0,     0,     0,     0,     0,
     735,     0,     0,   686,   682,     0,     0,     0,     0,   682,
     250,   683,     0,     0,     0,   687,     0,     0,     0,     0,
       0,   250,     0,     0,     0,     0,     0,   684,     0,     0,
       0,   686,   686,     0,     0,   688,     0,   686,  1384,     0,
       0,     0,  1199,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   687,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   682,   683,     0,     0,
       0,     0,   712,   683,   215,     0,     0,   683,     0,   683,
       0,   205,   205,     0,     0,   217,     0,     0,     0,   215,
       0,     0,     0,     0,     0,   687,     0,     0,   215,     0,
       0,     0,     0,     0,     0,     0,   250,     0,   250,     0,
       0,     0,     0,     0,   215,     0,     0,     0,     0,     0,
       0,   684,   682,   687,   687,   684,     0,   684,   682,   687,
       0,     0,   682,  1387,   682,     0,     0,     0,     0,     0,
       0,   686,     0,     0,     0,   684,     0,   341,   250,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     250,   363,   686,   250,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   686,     0,     0,     0,     0,   686,     0,
       0,     0,     0,     0,   179,   181,     0,   183,   184,   185,
       0,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,   199,     0,     0,   209,   212,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   230,     0,
       0,     0,     0,   687,     0,   238,     0,   241,   684,     0,
     257,     0,   262,   840,     0,   686,     0,     0,     0,   250,
       0,     0,   684,  1483,   684,     0,   859,   860,     0,   205,
       0,     0,     0,     0,   687,   205,     0,     0,  1191,   296,
       0,   205,     0,     0,     0,   687,     0,     0,     0,     0,
     687,     0,     0,   303,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     306,   686,   684,  1460,     0,     0,     0,   686,     0,     0,
       0,   686,     0,   686,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   205,     0,     0,     0,     0,
       0,     0,   205,   205,     0,     0,     0,   687,     0,   205,
       0,     0,     0,     0,     0,   205,     0,     0,     0,     0,
       0,     0,   925,     0,     0,     0,   250,     0,     0,     0,
     684,     0,     0,     0,     0,   684,     0,   684,     0,     0,
       0,   404,   250,     0,     0,   250,     0,    34,     0,   200,
       0,     0,     0,     0,     0,   250,     0,     0,     0,     0,
       0,     0,     0,   687,     0,     0,     0,   684,     0,   687,
       0,     0,     0,   687,     0,   687,     0,     0,     0,     0,
       0,     0,   428,     0,     0,   428,     0,     0,     0,     0,
     217,     0,   230,   439,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   684,  -793,  -793,  -793,  -793,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,  1007,   363,     0,    82,    83,   205,    84,    85,
      86,     0,     0,     0,     0,   205,   306,     0,   250,     0,
    1018,     0,   209,     0,     0,     0,   511,   684,     0,     0,
       0,    96,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,     0,   546,
       0,     0,   595,     0,   114,   684,   684,     0,     0,     0,
     555,   684,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   561,   562,   563,
     565,   566,   567,   568,   569,   570,   571,   572,   573,   574,
     575,   576,   577,   578,   579,   580,   581,   582,   583,   584,
     585,   586,     0,     0,   588,   588,     0,   591,     0,     0,
       0,     0,     0,  1088,   607,   609,   610,   611,   612,   613,
     614,   615,   616,   617,   618,   619,     0,  1100,     0,  1101,
       0,   588,   624,     0,   555,   588,   627,     0,     0,     0,
       0,     0,   607,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   639,     0,   641,     0,     0,     0,     0,   205,
     555,     0,     0,     0,     0,   684,     0,     0,     0,     0,
     653,     0,   654,     0,     0,     0,     0,  1141,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1424,     0,     0,     0,   684,     0,     0,   692,
       0,     0,   695,   698,   699,     0,     0,   684,     0,     0,
     205,     0,   684,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   710,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1179,     0,     0,     0,     0,
    1181,     0,  1182,     0,     0,   205,     0,   205,     0,     0,
       0,     0,     0,     0,     0,   338,   339,   340,     0,     0,
     739,     0,     0,     0,     0,     0,     0,   205,     0,   684,
       0,   341,  1221,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,     0,   363,     0,     0,     0,     0,
       0,   250,     0,   777,     0,     0,     0,     0,     0,     0,
    1247,     0,     0,     0,     0,     0,     0,   250,     0,     0,
       0,   205,     0,     0,     0,   684,     0,     0,     0,     0,
       0,   684,   205,   205,   785,   684,     0,   684,     0,     0,
       0,     0,     0,   296,     0,     0,     0,     0,     0,     0,
       0,     0,  1359,     0,     0,     0,     0,     0,     0,   794,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,     0,   363,
    1377,  1378,     0,     0,     0,   827,  1383,   338,   339,   340,
       0,     0,     0,     0,     0,     0,     0,     0,   230,   217,
       0,     0,     0,   341,     0,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,     0,   363,     0,   871,
     338,   339,   340,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   205,   763,   242,   341,     0,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,     0,
     363,   243,     0,   902,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   909,     0,     0,     0,
    1407,     0,    34,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   924,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   932,     0,   450,
     933,  1430,   934,     0,     0,     0,   555,     0,     0,     0,
     205,     0,  1440,     0,     0,   555,     0,  1444,     0,     0,
       0,     0,     0,     0,     0,   244,   245,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   966,     0,   175,     0,     0,    79,     0,   246,     0,
      82,    83,   205,    84,    85,    86,   798,     0,     0,     0,
       0,     0,     0,     0,     0,   205,   205,     0,   247,     0,
       0,     0,     0,     0,  1476,     0,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,  1010,  1011,  1012,   248,     0,     0,   695,  1014,   801,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   205,     0,     0,     0,     0,     0,     0,     0,  1027,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1525,     0,     0,     0,     0,     0,  1529,     0,  1046,     0,
    1531,     0,  1532,     0,     0,     0,     0,     0,     0,     0,
     338,   339,   340,     0,     0,   555,     0,     0,     0,     0,
       0,     0,     0,   555,     0,  1027,   341,     0,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,     0,
     363,     0,     0,     0,     0,   230,   338,   339,   340,     0,
       0,     0,     0,     0,  1092,     0,     0,     0,     0,     0,
       0,     0,   341,     0,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,     0,   363,     0,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,  1135,     0,     0,     0,  1136,     0,  1137,     0,
       0,     0,     0,     0,     0,     0,     0,   555,     0,     0,
       0,     0,     0,     0,  1150,   555,     0,    11,    12,    13,
       0,     0,     0,   555,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,    37,     0,     0,     0,    38,    39,    40,    41,
       0,    42,     0,    43,     0,    44,     0,     0,    45,     0,
       0,  1183,    46,    47,    48,    49,    50,    51,    52,   887,
      53,    54,    55,    56,    57,    58,    59,    60,    61,     0,
      62,    63,    64,    65,    66,    67,     0,     0,     0,     0,
      68,    69,   555,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,    75,     0,     0,     0,     0,    76,
      77,    78,    79,    80,    81,   935,    82,    83,     0,    84,
      85,    86,    87,     0,    88,     0,     0,     0,    89,     0,
       0,     0,     0,     0,    90,    91,     0,    92,     0,    93,
      94,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,   903,   114,   115,     0,   116,   117,
       0,   555,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1379,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,    36,     0,    37,     0,     0,     0,
      38,    39,    40,    41,     0,    42,     0,    43,     0,    44,
       0,     0,    45,     0,     0,     0,    46,    47,    48,    49,
      50,    51,    52,     0,    53,    54,    55,    56,    57,    58,
      59,    60,    61,     0,    62,    63,    64,    65,    66,    67,
       0,     0,     0,     0,    68,    69,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,    76,    77,    78,    79,    80,    81,     0,
      82,    83,     0,    84,    85,    86,    87,     0,    88,     0,
       0,     0,    89,     0,     0,     0,     0,     0,    90,    91,
       0,    92,     0,    93,    94,    95,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,   111,     0,   112,   113,  1029,   114,
     115,   340,   116,   117,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   341,     0,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,     0,   363,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,    37,     0,
       0,     0,    38,    39,    40,    41,     0,    42,     0,    43,
       0,    44,     0,     0,    45,     0,     0,     0,    46,    47,
      48,    49,    50,    51,    52,     0,    53,    54,    55,    56,
      57,    58,    59,    60,    61,     0,    62,    63,    64,    65,
      66,    67,     0,     0,     0,     0,    68,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,    76,    77,    78,    79,    80,
      81,     0,    82,    83,     0,    84,    85,    86,    87,     0,
      88,     0,     0,     0,    89,     0,     0,     0,     0,     0,
      90,    91,     0,    92,     0,    93,    94,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,     0,   363,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,    36,     0,
      37,     0,     0,     0,    38,    39,    40,    41,     0,    42,
       0,    43,     0,    44,     0,     0,    45,     0,     0,     0,
      46,    47,    48,    49,     0,    51,    52,     0,    53,     0,
      55,    56,    57,    58,    59,    60,    61,     0,    62,    63,
      64,     0,    66,    67,     0,     0,     0,     0,    68,    69,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,     0,   175,    77,    78,
      79,    80,    81,     0,    82,    83,     0,    84,    85,    86,
      87,     0,    88,     0,     0,     0,    89,     0,     0,     0,
       0,     0,    90,     0,     0,     0,     0,    93,    94,    95,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,     0,   111,     0,
     112,   113,   540,   114,   115,     0,   116,   117,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,     0,
     363,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,    37,     0,     0,     0,    38,    39,    40,    41,
       0,    42,     0,    43,     0,    44,     0,     0,    45,     0,
       0,     0,    46,    47,    48,    49,     0,    51,    52,     0,
      53,     0,    55,    56,    57,    58,    59,    60,    61,     0,
      62,    63,    64,     0,    66,    67,     0,     0,     0,     0,
      68,    69,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,    75,     0,     0,     0,     0,   175,
      77,    78,    79,    80,    81,     0,    82,    83,     0,    84,
      85,    86,    87,     0,    88,     0,     0,     0,    89,     0,
       0,     0,     0,     0,    90,     0,     0,     0,     0,    93,
      94,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,   882,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
       0,   363,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,    36,     0,    37,     0,     0,     0,    38,    39,
      40,    41,     0,    42,     0,    43,     0,    44,     0,     0,
      45,     0,     0,     0,    46,    47,    48,    49,     0,    51,
      52,     0,    53,     0,    55,    56,    57,    58,    59,    60,
      61,     0,    62,    63,    64,     0,    66,    67,     0,     0,
       0,     0,    68,    69,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
       0,   175,    77,    78,    79,    80,    81,     0,    82,    83,
       0,    84,    85,    86,    87,     0,    88,     0,     0,     0,
      89,     0,     0,     0,     0,     0,    90,     0,     0,     0,
       0,    93,    94,    95,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,   111,     0,   112,   113,   973,   114,   115,     0,
     116,   117,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,    36,     0,    37,     0,     0,     0,
      38,    39,    40,    41,   975,    42,     0,    43,     0,    44,
       0,     0,    45,     0,     0,     0,    46,    47,    48,    49,
       0,    51,    52,     0,    53,     0,    55,    56,    57,    58,
      59,    60,    61,     0,    62,    63,    64,     0,    66,    67,
       0,     0,     0,     0,    68,    69,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,   175,    77,    78,    79,    80,    81,     0,
      82,    83,     0,    84,    85,    86,    87,     0,    88,     0,
       0,     0,    89,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,    93,    94,    95,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,   111,     0,   112,   113,     0,   114,
     115,     0,   116,   117,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,    37,     0,
       0,     0,    38,    39,    40,    41,     0,    42,     0,    43,
       0,    44,  1089,     0,    45,     0,     0,     0,    46,    47,
      48,    49,     0,    51,    52,     0,    53,     0,    55,    56,
      57,    58,    59,    60,    61,     0,    62,    63,    64,     0,
      66,    67,     0,     0,     0,     0,    68,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,   175,    77,    78,    79,    80,
      81,     0,    82,    83,     0,    84,    85,    86,    87,     0,
      88,     0,     0,     0,    89,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,    93,    94,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,    36,     0,
      37,     0,     0,     0,    38,    39,    40,    41,     0,    42,
       0,    43,     0,    44,     0,     0,    45,     0,     0,     0,
      46,    47,    48,    49,     0,    51,    52,     0,    53,     0,
      55,    56,    57,    58,    59,    60,    61,     0,    62,    63,
      64,     0,    66,    67,     0,     0,     0,     0,    68,    69,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,     0,   175,    77,    78,
      79,    80,    81,     0,    82,    83,     0,    84,    85,    86,
      87,     0,    88,     0,     0,     0,    89,     0,     0,     0,
       0,     0,    90,     0,     0,     0,     0,    93,    94,    95,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,     0,   111,     0,
     112,   113,  1185,   114,   115,     0,   116,   117,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,    37,     0,     0,     0,    38,    39,    40,    41,
       0,    42,     0,    43,     0,    44,     0,     0,    45,     0,
       0,     0,    46,    47,    48,    49,     0,    51,    52,     0,
      53,     0,    55,    56,    57,    58,    59,    60,    61,     0,
      62,    63,    64,     0,    66,    67,     0,     0,     0,     0,
      68,    69,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,    75,     0,     0,     0,     0,   175,
      77,    78,    79,    80,    81,     0,    82,    83,     0,    84,
      85,    86,    87,     0,    88,     0,     0,     0,    89,     0,
       0,     0,     0,     0,    90,     0,     0,     0,     0,    93,
      94,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,  1380,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,    36,     0,    37,     0,     0,     0,    38,    39,
      40,    41,     0,    42,     0,    43,  1419,    44,     0,     0,
      45,     0,     0,     0,    46,    47,    48,    49,     0,    51,
      52,     0,    53,     0,    55,    56,    57,    58,    59,    60,
      61,     0,    62,    63,    64,     0,    66,    67,     0,     0,
       0,     0,    68,    69,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
       0,   175,    77,    78,    79,    80,    81,     0,    82,    83,
       0,    84,    85,    86,    87,     0,    88,     0,     0,     0,
      89,     0,     0,     0,     0,     0,    90,     0,     0,     0,
       0,    93,    94,    95,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,   111,     0,   112,   113,     0,   114,   115,     0,
     116,   117,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,    36,     0,    37,     0,     0,     0,
      38,    39,    40,    41,     0,    42,     0,    43,     0,    44,
       0,     0,    45,     0,     0,     0,    46,    47,    48,    49,
       0,    51,    52,     0,    53,     0,    55,    56,    57,    58,
      59,    60,    61,     0,    62,    63,    64,     0,    66,    67,
       0,     0,     0,     0,    68,    69,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,   175,    77,    78,    79,    80,    81,     0,
      82,    83,     0,    84,    85,    86,    87,     0,    88,     0,
       0,     0,    89,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,    93,    94,    95,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,   111,     0,   112,   113,  1449,   114,
     115,     0,   116,   117,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,    37,     0,
       0,     0,    38,    39,    40,    41,     0,    42,     0,    43,
       0,    44,     0,     0,    45,     0,     0,     0,    46,    47,
      48,    49,     0,    51,    52,     0,    53,     0,    55,    56,
      57,    58,    59,    60,    61,     0,    62,    63,    64,     0,
      66,    67,     0,     0,     0,     0,    68,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,   175,    77,    78,    79,    80,
      81,     0,    82,    83,     0,    84,    85,    86,    87,     0,
      88,     0,     0,     0,    89,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,    93,    94,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
    1450,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,    36,     0,
      37,     0,     0,     0,    38,    39,    40,    41,     0,    42,
    1453,    43,     0,    44,     0,     0,    45,     0,     0,     0,
      46,    47,    48,    49,     0,    51,    52,     0,    53,     0,
      55,    56,    57,    58,    59,    60,    61,     0,    62,    63,
      64,     0,    66,    67,     0,     0,     0,     0,    68,    69,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,     0,   175,    77,    78,
      79,    80,    81,     0,    82,    83,     0,    84,    85,    86,
      87,     0,    88,     0,     0,     0,    89,     0,     0,     0,
       0,     0,    90,     0,     0,     0,     0,    93,    94,    95,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,     0,   111,     0,
     112,   113,     0,   114,   115,     0,   116,   117,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,    37,     0,     0,     0,    38,    39,    40,    41,
       0,    42,     0,    43,     0,    44,     0,     0,    45,     0,
       0,     0,    46,    47,    48,    49,     0,    51,    52,     0,
      53,     0,    55,    56,    57,    58,    59,    60,    61,     0,
      62,    63,    64,     0,    66,    67,     0,     0,     0,     0,
      68,    69,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,    75,     0,     0,     0,     0,   175,
      77,    78,    79,    80,    81,     0,    82,    83,     0,    84,
      85,    86,    87,     0,    88,     0,     0,     0,    89,     0,
       0,     0,     0,     0,    90,     0,     0,     0,     0,    93,
      94,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,  1468,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,    36,     0,    37,     0,     0,     0,    38,    39,
      40,    41,     0,    42,     0,    43,     0,    44,     0,     0,
      45,     0,     0,     0,    46,    47,    48,    49,     0,    51,
      52,     0,    53,     0,    55,    56,    57,    58,    59,    60,
      61,     0,    62,    63,    64,     0,    66,    67,     0,     0,
       0,     0,    68,    69,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
       0,   175,    77,    78,    79,    80,    81,     0,    82,    83,
       0,    84,    85,    86,    87,     0,    88,     0,     0,     0,
      89,     0,     0,     0,     0,     0,    90,     0,     0,     0,
       0,    93,    94,    95,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,   111,     0,   112,   113,  1521,   114,   115,     0,
     116,   117,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,    36,     0,    37,     0,     0,     0,
      38,    39,    40,    41,     0,    42,     0,    43,     0,    44,
       0,     0,    45,     0,     0,     0,    46,    47,    48,    49,
       0,    51,    52,     0,    53,     0,    55,    56,    57,    58,
      59,    60,    61,     0,    62,    63,    64,     0,    66,    67,
       0,     0,     0,     0,    68,    69,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,   175,    77,    78,    79,    80,    81,     0,
      82,    83,     0,    84,    85,    86,    87,     0,    88,     0,
       0,     0,    89,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,    93,    94,    95,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,   111,     0,   112,   113,  1526,   114,
     115,     0,   116,   117,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,    37,     0,
       0,     0,    38,    39,    40,    41,     0,    42,     0,    43,
       0,    44,     0,     0,    45,     0,     0,     0,    46,    47,
      48,    49,     0,    51,    52,     0,    53,     0,    55,    56,
      57,    58,    59,    60,    61,     0,    62,    63,    64,     0,
      66,    67,     0,     0,     0,     0,    68,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,   175,    77,    78,    79,    80,
      81,     0,    82,    83,     0,    84,    85,    86,    87,     0,
      88,     0,     0,     0,    89,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,    93,    94,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     429,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,    36,     0,
      37,     0,     0,     0,    38,    39,    40,    41,     0,    42,
       0,    43,     0,    44,     0,     0,    45,     0,     0,     0,
      46,    47,    48,    49,     0,    51,    52,     0,    53,     0,
      55,    56,    57,    58,   171,   172,    61,     0,    62,    63,
      64,     0,     0,     0,     0,     0,     0,     0,    68,    69,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,     0,   175,    77,    78,
      79,    80,    81,     0,    82,    83,     0,    84,    85,    86,
       0,     0,    88,     0,     0,     0,    89,     0,     0,     0,
       0,     0,    90,     0,     0,     0,     0,    93,    94,    95,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,     0,   111,     0,
     112,   113,     0,   114,   115,     0,   116,   117,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   656,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,    37,     0,     0,     0,    38,    39,    40,    41,
       0,    42,     0,    43,     0,    44,     0,     0,    45,     0,
       0,     0,    46,    47,    48,    49,     0,    51,    52,     0,
      53,     0,    55,    56,    57,    58,   171,   172,    61,     0,
      62,    63,    64,     0,     0,     0,     0,     0,     0,     0,
      68,    69,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,    75,     0,     0,     0,     0,   175,
      77,    78,    79,    80,    81,     0,    82,    83,     0,    84,
      85,    86,     0,     0,    88,     0,     0,     0,    89,     0,
       0,     0,     0,     0,    90,     0,     0,     0,     0,    93,
      94,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   112,   113,     0,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   842,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,    36,     0,    37,     0,     0,     0,    38,    39,
      40,    41,     0,    42,     0,    43,     0,    44,     0,     0,
      45,     0,     0,     0,    46,    47,    48,    49,     0,    51,
      52,     0,    53,     0,    55,    56,    57,    58,   171,   172,
      61,     0,    62,    63,    64,     0,     0,     0,     0,     0,
       0,     0,    68,    69,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
       0,   175,    77,    78,    79,    80,    81,     0,    82,    83,
       0,    84,    85,    86,     0,     0,    88,     0,     0,     0,
      89,     0,     0,     0,     0,     0,    90,     0,     0,     0,
       0,    93,    94,    95,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,   111,     0,   112,   113,     0,   114,   115,     0,
     116,   117,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1242,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,    36,     0,    37,     0,     0,     0,
      38,    39,    40,    41,     0,    42,     0,    43,     0,    44,
       0,     0,    45,     0,     0,     0,    46,    47,    48,    49,
       0,    51,    52,     0,    53,     0,    55,    56,    57,    58,
     171,   172,    61,     0,    62,    63,    64,     0,     0,     0,
       0,     0,     0,     0,    68,    69,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,   175,    77,    78,    79,    80,    81,     0,
      82,    83,     0,    84,    85,    86,     0,     0,    88,     0,
       0,     0,    89,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,    93,    94,    95,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,   111,     0,   112,   113,     0,   114,
     115,     0,   116,   117,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1373,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,    37,     0,
       0,     0,    38,    39,    40,    41,     0,    42,     0,    43,
       0,    44,     0,     0,    45,     0,     0,     0,    46,    47,
      48,    49,     0,    51,    52,     0,    53,     0,    55,    56,
      57,    58,   171,   172,    61,     0,    62,    63,    64,     0,
       0,     0,     0,     0,     0,     0,    68,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,   175,    77,    78,    79,    80,
      81,     0,    82,    83,     0,    84,    85,    86,     0,     0,
      88,     0,     0,     0,    89,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,    93,    94,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,   112,   113,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,    36,     0,
      37,     0,     0,     0,    38,    39,    40,    41,     0,    42,
       0,    43,     0,    44,     0,     0,    45,     0,     0,     0,
      46,    47,    48,    49,     0,    51,    52,     0,    53,     0,
      55,    56,    57,    58,   171,   172,    61,     0,    62,    63,
      64,     0,     0,     0,     0,     0,     0,     0,    68,    69,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,     0,   175,    77,    78,
      79,    80,    81,     0,    82,    83,     0,    84,    85,    86,
       0,     0,    88,     0,     0,     0,    89,     0,     0,     0,
       0,     0,    90,     0,     0,     0,     0,    93,    94,    95,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,     0,   111,     0,
     112,   113,     0,   114,   115,     0,   116,   117,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   603,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,     0,     0,     0,     0,    38,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,     0,
       0,     0,     0,    56,    57,    58,   171,   172,   173,     0,
      34,    63,    64,     0,     0,     0,     0,     0,     0,     0,
     174,    69,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,    75,     0,     0,     0,     0,   175,
      77,    78,    79,   604,    81,     0,    82,    83,     0,    84,
      85,    86,     0,     0,    88,     0,     0,     0,    89,     0,
       0,     0,     0,     0,    90,     0,     0,     0,     0,    93,
      94,    95,    96,   254,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,    82,    83,
     111,    84,    85,    86,     0,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,     0,   557,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,    35,    36,     0,     0,     0,     0,     0,    38,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    49,     0,     0,
       0,     0,     0,     0,     0,    56,    57,    58,   171,   172,
     173,     0,     0,    63,    64,     0,     0,     0,     0,     0,
       0,     0,   174,    69,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
       0,   175,    77,    78,    79,     0,    81,     0,    82,    83,
       0,    84,    85,    86,     0,     0,    88,     0,     0,     0,
      89,     0,     0,     0,     0,     0,    90,     0,     0,     0,
       0,    93,    94,    95,    96,   254,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,   111,     0,   255,     0,     0,   114,   115,     0,
     116,   117,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,    35,    36,     0,     0,     0,     0,     0,
      38,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    56,    57,    58,
     171,   172,   173,     0,    34,    63,    64,     0,     0,     0,
       0,     0,     0,     0,   174,    69,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,   175,    77,    78,    79,   604,    81,     0,
      82,    83,     0,    84,    85,    86,     0,     0,    88,     0,
       0,     0,    89,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,    93,    94,    95,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,    82,    83,   111,    84,    85,    86,     0,   114,
     115,     0,   116,   117,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   208,     0,     0,   775,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,     0,     0,
       0,     0,    38,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    49,     0,     0,     0,     0,     0,     0,     0,    56,
      57,    58,   171,   172,   173,     0,    34,    63,    64,     0,
       0,     0,     0,     0,     0,     0,   174,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,   175,    77,    78,    79,     0,
      81,     0,    82,    83,     0,    84,    85,    86,     0,     0,
      88,     0,  1005,     0,    89,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,    93,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,    82,    83,   111,    84,    85,    86,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,    33,    34,    35,    36,     0,
       0,     0,     0,     0,    38,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    49,     0,     0,     0,     0,     0,     0,
       0,    56,    57,    58,   171,   172,   173,     0,     0,    63,
      64,     0,     0,     0,     0,     0,     0,     0,   174,    69,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,     0,   175,    77,    78,
      79,     0,    81,     0,    82,    83,     0,    84,    85,    86,
       0,     0,    88,     0,     0,     0,    89,     0,     0,     0,
       0,     0,    90,     0,     0,     0,     0,    93,     0,    95,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,     0,   111,     0,
     237,     0,     0,   114,   115,     0,   116,   117,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,     0,     0,     0,     0,    38,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,     0,
       0,     0,     0,    56,    57,    58,   171,   172,   173,     0,
       0,    63,    64,     0,     0,     0,     0,     0,     0,     0,
     174,    69,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,    75,     0,     0,     0,     0,   175,
      77,    78,    79,     0,    81,     0,    82,    83,     0,    84,
      85,    86,     0,     0,    88,     0,     0,     0,    89,     0,
       0,     0,     0,     0,    90,     0,     0,     0,     0,    93,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,   240,     0,     0,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,    35,    36,     0,     0,     0,     0,     0,    38,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   295,     0,     0,    49,     0,     0,
       0,     0,     0,     0,     0,    56,    57,    58,   171,   172,
     173,     0,    34,    63,    64,     0,     0,     0,     0,     0,
       0,     0,   174,    69,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
       0,   175,    77,    78,    79,     0,    81,     0,    82,    83,
       0,    84,    85,    86,     0,     0,    88,     0,     0,     0,
      89,     0,     0,     0,     0,     0,    90,     0,     0,     0,
       0,    93,     0,    95,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
      82,    83,   111,    84,    85,    86,     0,   114,   115,     0,
     116,   117,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,    35,    36,     0,     0,     0,     0,     0,
      38,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    56,    57,    58,
     171,   172,   173,     0,     0,    63,    64,     0,     0,     0,
       0,     0,     0,     0,   174,    69,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,   175,    77,    78,    79,     0,    81,     0,
      82,    83,     0,    84,    85,    86,     0,     0,    88,     0,
       0,     0,    89,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,    93,     0,    95,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,   111,   427,     0,     0,     0,   114,
     115,     0,   116,   117,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   552,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,     0,     0,
       0,     0,    38,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    49,     0,     0,     0,     0,     0,     0,     0,    56,
      57,    58,   171,   172,   173,     0,     0,    63,    64,     0,
       0,     0,     0,     0,     0,     0,   174,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,   175,    77,    78,    79,     0,
      81,     0,    82,    83,     0,    84,    85,    86,     0,     0,
      88,     0,     0,     0,    89,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,    93,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,     0,     0,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     564,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,    33,    34,    35,    36,     0,
       0,     0,     0,     0,    38,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    49,     0,     0,     0,     0,     0,     0,
       0,    56,    57,    58,   171,   172,   173,     0,     0,    63,
      64,     0,     0,     0,     0,     0,     0,     0,   174,    69,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,     0,   175,    77,    78,
      79,     0,    81,     0,    82,    83,     0,    84,    85,    86,
       0,     0,    88,     0,     0,     0,    89,     0,     0,     0,
       0,     0,    90,     0,     0,     0,     0,    93,     0,    95,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,     0,   111,     0,
       0,     0,     0,   114,   115,     0,   116,   117,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   603,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,     0,     0,     0,     0,    38,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,     0,
       0,     0,     0,    56,    57,    58,   171,   172,   173,     0,
       0,    63,    64,     0,     0,     0,     0,     0,     0,     0,
     174,    69,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,    75,     0,     0,     0,     0,   175,
      77,    78,    79,     0,    81,     0,    82,    83,     0,    84,
      85,    86,     0,     0,    88,     0,     0,     0,    89,     0,
       0,     0,     0,     0,    90,     0,     0,     0,     0,    93,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,     0,     0,     0,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   638,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,    35,    36,     0,     0,     0,     0,     0,    38,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    49,     0,     0,
       0,     0,     0,     0,     0,    56,    57,    58,   171,   172,
     173,     0,     0,    63,    64,     0,     0,     0,     0,     0,
       0,     0,   174,    69,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
       0,   175,    77,    78,    79,     0,    81,     0,    82,    83,
       0,    84,    85,    86,     0,     0,    88,     0,     0,     0,
      89,     0,     0,     0,     0,     0,    90,     0,     0,     0,
       0,    93,     0,    95,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,   111,     0,     0,     0,     0,   114,   115,     0,
     116,   117,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   640,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,    35,    36,     0,     0,     0,     0,     0,
      38,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    56,    57,    58,
     171,   172,   173,     0,     0,    63,    64,     0,     0,     0,
       0,     0,     0,     0,   174,    69,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,   175,    77,    78,    79,     0,    81,     0,
      82,    83,     0,    84,    85,    86,     0,     0,    88,     0,
       0,     0,    89,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,    93,     0,    95,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,   111,     0,     0,     0,     0,   114,
     115,     0,   116,   117,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,    35,    36,     0,     0,     0,
       0,     0,    38,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    49,     0,     0,     0,     0,     0,     0,     0,    56,
      57,    58,   171,   172,   173,     0,     0,    63,    64,     0,
       0,     0,     0,     0,     0,     0,   174,    69,     0,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
      75,     0,     0,     0,     0,   175,    77,    78,    79,     0,
      81,     0,    82,    83,     0,    84,    85,    86,     0,     0,
      88,     0,     0,     0,    89,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,    93,     0,    95,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,     0,   111,     0,     0,   652,
       0,   114,   115,     0,   116,   117,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   923,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,    33,    34,    35,    36,     0,
       0,     0,     0,     0,    38,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    49,     0,     0,     0,     0,     0,     0,
       0,    56,    57,    58,   171,   172,   173,     0,     0,    63,
      64,     0,     0,     0,     0,     0,     0,     0,   174,    69,
       0,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,    75,     0,     0,     0,     0,   175,    77,    78,
      79,     0,    81,     0,    82,    83,     0,    84,    85,    86,
       0,     0,    88,     0,     0,     0,    89,     0,     0,     0,
       0,     0,    90,     0,     0,     0,     0,    93,     0,    95,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,     0,   111,     0,
       0,     0,     0,   114,   115,     0,   116,   117,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   965,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
      36,     0,     0,     0,     0,     0,    38,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    49,     0,     0,     0,     0,
       0,     0,     0,    56,    57,    58,   171,   172,   173,     0,
       0,    63,    64,     0,     0,     0,     0,     0,     0,     0,
     174,    69,     0,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,    75,     0,     0,     0,     0,   175,
      77,    78,    79,     0,    81,     0,    82,    83,     0,    84,
      85,    86,     0,     0,    88,     0,     0,     0,    89,     0,
       0,     0,     0,     0,    90,     0,     0,     0,     0,    93,
       0,    95,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,     0,
     111,     0,     0,     0,     0,   114,   115,     0,   116,   117,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,    35,    36,     0,     0,     0,     0,     0,    38,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    49,     0,     0,
       0,     0,     0,     0,     0,    56,    57,    58,   171,   172,
     173,     0,     0,    63,    64,     0,     0,     0,     0,     0,
       0,     0,   174,    69,     0,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,    75,     0,     0,     0,
       0,   175,    77,    78,    79,     0,    81,     0,    82,    83,
       0,    84,    85,    86,     0,     0,    88,     0,     0,     0,
      89,     0,     0,     0,     0,     0,    90,     0,     0,     0,
       0,    93,     0,    95,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,     0,   111,     0,     0,     0,     0,   114,   115,     0,
     116,   117,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,   510,    36,     0,     0,     0,     0,     0,
      38,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    49,
       0,     0,     0,     0,     0,     0,     0,    56,    57,    58,
     171,   172,   173,     0,     0,    63,    64,     0,     0,     0,
       0,     0,     0,     0,   174,    69,     0,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,    75,     0,
       0,     0,     0,   175,    77,    78,    79,     0,    81,     0,
      82,    83,     0,    84,    85,    86,     0,     0,    88,     0,
       0,     0,    89,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,    93,     0,    95,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,     0,   111,   338,   339,   340,     0,   114,
     115,     0,   116,   117,     0,     0,     0,     0,     0,     0,
       0,   341,     0,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,     0,   363,   338,   339,   340,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   341,     0,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,     0,   363,   338,   339,   340,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   341,     0,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,     0,   363,   338,   339,
     340,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   341,     0,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,     0,   363,     0,
       0,     0,     0,     0,     0,     0,     0,   338,   339,   340,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   341,   943,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,     0,   363,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   967,     0,   338,   339,   340,
       0,     0,  1261,  1262,  1263,  1264,  1265,     0,     0,  1266,
    1267,  1268,  1269,   341,     0,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,  1142,   363,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1270,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1271,  1272,  1273,  1274,  1275,  1276,  1277,     0,
       0,     0,    34,     0,     0,     0,     0,  1217,     0,     0,
       0,  1278,  1279,  1280,  1281,  1282,  1283,  1284,  1285,  1286,
    1287,  1288,  1289,  1290,  1291,  1292,  1293,  1294,  1295,  1296,
    1297,  1298,  1299,  1300,  1301,  1302,  1303,  1304,  1305,  1306,
    1307,  1308,  1309,  1310,  1311,  1312,  1313,  1314,  1315,  1316,
    1317,  1318,     0,     0,  1319,  1320,  1218,  1321,  1322,  1323,
    1324,  1325,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1326,  1327,  1328,     0,  1329,     0,     0,
      82,    83,     0,    84,    85,    86,  1330,  1331,  1332,     0,
       0,  1333,     0,     0,     0,     0,     0,     0,  1334,  1335,
    1250,  1336,     0,  1337,  1338,  1339,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   338,   339,   340,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   341,  1093,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
       0,   363,   338,   339,   340,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   341,     0,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,     0,   363,   338,   339,   340,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   341,
       0,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,     0,   363,   338,   339,   340,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     341,     0,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,     0,   363,     0,     0,     0,     0,     0,
       0,   338,   339,   340,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   341,  1094,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
       0,   363,   338,   339,   340,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   341,   364,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,     0,   363,   338,   339,   340,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   341,
     443,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,     0,   363,   338,   339,   340,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     341,   445,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,     0,   363,     0,     0,     0,     0,     0,
       0,   338,   339,   340,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   341,   457,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
       0,   363,   338,   339,   340,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   341,   481,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,     0,   363,     0,   338,   339,   340,     0,    34,     0,
     200,     0,     0,     0,     0,     0,     0,     0,     0,   630,
     341,     0,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,     0,   363,   338,   339,   340,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     649,   341,     0,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   242,   363,    82,    83,     0,    84,
      85,    86,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   879,     0,     0,     0,     0,     0,     0,     0,
     243,     0,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,     0,
       0,    34,     0,   646,     0,   114,     0,     0,     0,   875,
     876,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     242,     0,     0,     0,     0,     0,     0,     0,  -288,     0,
       0,     0,     0,     0,     0,     0,    56,    57,    58,   171,
     172,   329,     0,     0,     0,     0,   243,     0,  1420,     0,
       0,     0,     0,     0,   244,   245,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    34,     0,     0,
       0,     0,   175,     0,     0,    79,     0,   246,     0,    82,
      83,     0,    84,    85,    86,     0,     0,  1112,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   247,     0,     0,
     242,     0,     0,     0,    95,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     244,   245,     0,   248,     0,     0,   243,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   175,     0,
       0,    79,     0,   246,     0,    82,    83,    34,    84,    85,
      86,     0,     0,     0,   855,     0,     0,     0,     0,     0,
       0,     0,     0,   247,     0,     0,   242,     0,     0,     0,
       0,    96,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,     0,   248,
       0,     0,   243,     0,     0,     0,     0,     0,     0,     0,
     244,   245,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    34,     0,     0,     0,     0,   175,     0,
       0,    79,     0,   246,     0,    82,    83,     0,    84,    85,
      86,     0,    34,     0,  1098,     0,     0,     0,     0,     0,
       0,     0,     0,   247,     0,     0,     0,     0,     0,     0,
       0,    96,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   244,   245,     0,   248,
       0,     0,     0,     0,   674,   675,     0,     0,     0,     0,
       0,     0,     0,     0,   175,     0,     0,    79,     0,   246,
       0,    82,    83,   676,    84,    85,    86,     0,     0,     0,
       0,    31,    32,    33,    34,     0,   263,     0,     0,   247,
      82,    83,    38,    84,    85,    86,     0,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,     0,   248,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,     0,     0,     0,     0,   677,     0,    70,
      71,    72,    73,    74,     0,   803,   804,     0,     0,     0,
     678,     0,     0,     0,     0,   175,    77,    78,    79,     0,
     679,     0,    82,    83,   805,    84,    85,    86,     0,     0,
      88,     0,   806,   807,   808,    34,     0,     0,     0,     0,
     680,     0,     0,   809,     0,    93,     0,     0,   681,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    27,    28,     0,     0,     0,     0,     0,
       0,     0,     0,    34,     0,   200,     0,     0,   810,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   811,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    82,    83,     0,    84,    85,    86,     0,
       0,     0,     0,   201,     0,     0,   792,     0,     0,     0,
       0,   812,     0,     0,     0,    34,     0,   200,     0,   813,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   175,     0,     0,    79,     0,    81,
       0,    82,    83,     0,    84,    85,    86,     0,     0,     0,
       0,     0,     0,    89,     0,   201,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,     0,   408,   175,     0,     0,    79,
     114,    81,     0,    82,    83,     0,    84,    85,    86,    34,
       0,   200,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    96,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,     0,   202,     0,   201,
       0,     0,   114,     0,     0,     0,     0,     0,     0,    34,
       0,   200,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     175,     0,     0,    79,     0,    81,     0,    82,    83,     0,
      84,    85,    86,     0,     0,     0,     0,     0,     0,   201,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   502,     0,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
     175,   202,     0,    79,   486,    81,   114,    82,    83,     0,
      84,    85,    86,    34,     0,   200,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
       0,   202,     0,   201,     0,     0,   114,     0,     0,     0,
       0,     0,     0,     0,     0,   898,     0,    34,     0,   200,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   175,     0,     0,    79,     0,    81,
       0,    82,    83,     0,    84,    85,    86,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   201,     0,     0,
       0,   947,   948,   949,    34,     0,     0,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,    34,     0,   200,   202,     0,     0,   175,     0,
     114,    79,     0,    81,     0,    82,    83,     0,    84,    85,
      86,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    96,   213,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,    34,     0,   200,   202,
       0,     0,    82,    83,   114,    84,    85,    86,     0,     0,
       0,     0,     0,   175,     0,     0,    79,     0,    81,     0,
      82,    83,     0,    84,    85,    86,     0,     0,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,     0,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,    34,     0,     0,   214,     0,     0,     0,     0,   114,
       0,     0,     0,     0,    82,    83,     0,    84,    85,    86,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,    34,     0,     0,     0,     0,
       0,   919,     0,   114,     0,     0,     0,     0,     0,     0,
       0,     0,   175,     0,     0,    79,     0,     0,     0,    82,
      83,     0,    84,    85,    86,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    34,     0,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
      34,     0,     0,     0,     0,     0,     0,  1422,     0,   291,
       0,     0,     0,    82,    83,     0,    84,    85,    86,     0,
       0,     0,     0,     0,  1192,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1193,     0,     0,    96,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   175,     0,     0,    79,     0,  1194,
     292,    82,    83,     0,    84,  1195,    86,    34,     0,   729,
     730,   175,     0,     0,    79,     0,    81,     0,    82,    83,
       0,    84,    85,    86,    34,     0,     0,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    82,    83,     0,    84,    85,
      86,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     326,     0,    82,    83,     0,    84,    85,    86,     0,     0,
       0,    96,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   338,   339,   340,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   341,
       0,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,     0,   363,   338,   339,   340,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     341,     0,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,     0,   363,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     338,   339,   340,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   341,   413,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,     0,
     363,   338,   339,   340,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   341,   521,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
       0,   363,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   338,   339,   340,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   341,   781,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,     0,   363,   338,   339,
     340,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   341,   822,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,     0,   363,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   338,   339,   340,     0,     0,
       0,  1055,     0,     0,     0,     0,     0,     0,     0,     0,
     660,   341,   778,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,     0,   363,   338,   339,   340,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   341,     0,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,     0,   363,   339,   340,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   341,     0,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,     0,   363
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-1223))

#define yytable_value_is_error(yytable_value) \
  ((yytable_value) == (-793))

static const yytype_int16 yycheck[] =
{
       4,   132,   157,   177,     4,     4,    87,     4,    30,   554,
      91,    92,   322,     4,   400,   631,   215,     4,    28,    41,
      51,   304,   395,    45,     4,   916,   375,   160,   429,   363,
     167,     4,   906,   407,   755,   221,   748,  1050,   319,   221,
     771,   659,   517,    47,   665,     9,    50,   178,   421,   111,
     131,    24,    25,     9,     9,   242,   243,    75,    42,   111,
       9,   248,    75,    67,     4,     9,    27,   123,     9,    54,
      62,    92,     9,    75,     9,   123,     9,   478,     9,     9,
       9,     9,     9,    87,     9,     9,    62,    91,    92,   222,
       9,    76,     9,     9,    79,     9,    75,     9,     4,    47,
       9,     9,    42,    80,   366,     9,    27,     0,     9,   774,
      92,    30,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,   789,   195,   147,   131,     9,    80,
     392,   123,   319,  1146,   396,  1148,    42,   123,   111,   104,
     202,    67,    68,   199,   195,   132,  1368,   864,   196,   195,
     956,   868,   214,   527,   960,   867,   330,     8,    60,    61,
     195,   101,   195,   140,   143,   147,   106,   143,   108,   109,
     110,   111,   112,   113,   114,   195,   160,   157,  1052,    42,
      42,    42,   200,   187,    62,  1059,  1060,   200,    62,   140,
      62,   178,    62,    42,  1416,   101,   927,   162,   200,   930,
     106,   149,   108,   109,   110,   111,   112,   113,   114,   193,
     196,   151,   152,   149,   154,   277,    62,  1230,    62,   593,
      62,   123,   198,   227,   845,   277,   847,   231,   402,   202,
     227,   235,   369,   197,   231,   208,   176,   198,    62,   166,
     196,   214,   197,   374,   325,   151,   152,   959,   154,   253,
     199,   652,   389,   197,   227,   656,   197,   198,   231,   199,
     197,   448,   197,   738,   197,   166,   197,   197,   197,   197,
     176,    47,   197,   197,   411,   940,   197,   196,   263,   196,
     196,  1155,   196,   420,   196,   166,   423,   196,   196,   165,
     371,   372,   196,   199,   196,   268,   670,   160,   160,   160,
      62,    62,   275,   276,   277,   309,   291,   198,    62,   282,
      62,   160,    62,    62,   318,   288,    75,   195,   322,   195,
     198,   325,   309,  1054,   198,   195,   198,    75,   198,   309,
     193,   119,    80,   196,   196,   196,   309,    62,    62,   725,
     119,   143,   529,   530,   375,   961,   408,   196,   143,   536,
     195,   537,   198,   363,   198,   537,   198,  1370,   976,   369,
       4,   123,   123,   165,    75,   369,   370,   371,   372,    80,
     165,   198,   150,   149,   198,    32,    95,    96,    32,   389,
      95,    96,   141,   142,   195,   389,   373,   195,   164,    26,
      32,   672,  1104,   141,   142,   195,   198,   596,    42,   150,
     195,   411,    32,   198,    67,    68,    43,   411,   195,    46,
     420,    75,   163,   423,   787,   195,    80,     4,    75,   423,
     198,    75,   621,   796,   195,  1156,    27,   400,   602,    14,
     141,   142,   436,    75,   147,   408,   198,   198,   637,   436,
     195,   842,    27,   817,   198,    75,   198,   198,   198,   198,
     636,  1067,  1164,  1069,   636,    42,   642,   101,   792,    44,
     642,    32,   106,   436,   108,   109,   110,   111,   112,   113,
     114,    32,   755,   198,   198,  1488,    75,   141,   142,   560,
     199,   485,   198,   682,   199,   672,   673,    75,   771,   863,
     197,  1504,    80,   204,   147,    75,   500,    32,   483,   197,
      80,   197,   506,   507,    75,   197,    75,   151,   152,   642,
     154,    80,   121,   122,   101,   197,    46,    47,    48,   106,
      50,   108,   109,   110,   111,   112,   113,   114,    94,    95,
      96,   197,   176,   108,   109,   110,   111,   112,   113,   203,
      75,   140,   141,   142,   197,   198,  1162,   197,    70,    71,
      72,   900,   140,   141,   142,   199,   560,    62,    26,    81,
     140,   141,   142,   936,   151,   152,   753,   154,   108,   109,
     110,   944,   141,   142,    62,    43,    62,   764,    46,   552,
     143,   955,   108,   109,   110,   111,   112,   113,    62,   176,
     197,   198,   978,   108,   109,   110,   111,   112,   113,   197,
     198,   176,   195,  1494,   119,   120,   128,   129,   130,   131,
     132,   195,   199,  1394,  1395,    75,  1017,   139,   143,  1510,
      80,  1237,   759,   145,   146,  1390,  1391,   631,  1002,   633,
     603,   195,    41,   655,   147,  1009,    50,   159,  1024,    94,
      95,    96,   157,   143,   927,   165,   650,   930,   123,   202,
     176,     9,   174,   115,   116,   117,   143,   195,   662,   663,
     143,   176,   849,   650,   851,   638,   663,   640,     8,   123,
     650,   197,   165,   197,   195,  1048,    14,   650,    14,  1080,
      75,   141,   142,  1056,     4,   285,   197,   660,   197,   289,
     663,  1064,    14,   196,  1095,   165,   197,    81,    14,    92,
     704,  1075,   196,   707,   891,   196,   201,     9,   100,    84,
     791,   311,   196,   313,   314,   315,   316,   704,   195,   195,
       9,   920,    42,   195,   704,   196,   913,    14,   197,   916,
      81,   704,   195,   737,     9,   181,    75,   737,   737,  1022,
     737,   714,  1463,    75,    75,   184,   737,  1030,   195,   759,
     737,    75,   725,   726,   196,   759,  1372,   737,   196,   121,
    1481,   197,   195,    62,   737,   196,  1167,   100,  1489,   122,
    1143,  1054,   124,   164,  1175,   108,   109,   110,   111,   112,
     113,   101,   792,  1184,     9,   196,   106,   791,   108,   109,
     110,   111,   112,   113,   114,    14,   193,     9,    62,   803,
     804,   805,   196,     9,    14,   992,   121,   829,  1118,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,   830,    50,   832,   202,
     834,   151,   152,   830,   154,    43,    44,    45,    46,    47,
      48,  1242,    50,   176,   199,   832,   202,   834,   195,  1222,
       9,   195,   832,   202,   834,   196,   176,   830,   195,   832,
     202,   834,   866,   836,  1147,   869,  1411,   196,   124,   900,
    1153,  1154,   197,  1156,   178,   179,   180,     9,   197,   199,
     143,   185,   186,   196,   195,   189,   190,   195,   892,   195,
     143,   198,    14,   181,     9,   181,    75,   198,    14,   197,
     904,  1211,    92,   198,   904,   904,    14,   904,   202,    14,
       4,    14,  1099,   904,   198,   197,    27,   904,    24,    25,
       9,   195,    28,   195,   904,   195,   195,   195,  1115,   195,
     197,   904,   197,   195,  1479,   196,   124,    14,     9,   196,
     124,  1128,   196,   202,    50,     9,   968,   140,    42,    75,
     923,  1234,   956,     9,   195,   124,   960,   961,   197,    14,
     195,    75,  1363,   196,  1365,   195,   198,   971,   195,   198,
     196,   195,  1373,   124,   971,  1130,   198,     9,   982,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,   196,   965,   202,   140,   982,    27,  1128,   971,    69,
     197,   196,   982,   197,   166,   978,   979,   101,    27,   982,
     196,   124,   106,  1414,   108,   109,   110,   111,   112,   113,
     114,     9,   196,   196,   124,    60,    61,   199,     9,   196,
      92,   199,    14,   196,   198,  1209,   196,   195,   124,   196,
     195,     9,  1022,   196,   196,   124,   196,   196,    27,   197,
    1030,  1024,   196,   198,   197,   197,   153,   151,   152,   149,
     154,    75,    14,  1067,   195,  1069,   196,   106,   196,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,   124,   176,   124,   196,    14,   196,   196,   123,    75,
     124,   198,    14,   197,   196,   196,   202,    24,    25,   195,
     198,    28,   208,   124,   197,   199,   197,  1508,   214,    14,
    1114,    14,   196,  1514,  1118,    60,    61,   198,    52,  1123,
      75,   195,    75,     9,   197,    75,   104,  1114,    92,    92,
     143,   156,    30,    14,  1114,   195,   242,   243,   196,   162,
    1127,  1114,   248,   197,   195,    75,   158,   196,     9,    75,
    1130,   197,   196,   198,   196,    75,    14,    14,  1162,    75,
      14,   196,   268,    14,  1168,   483,  1471,  1147,  1172,   275,
     276,   372,  1176,  1153,  1154,  1172,   282,   157,   123,   370,
    1463,  1168,   288,  1357,   371,   790,   788,  1485,  1168,  1176,
     740,  1481,  1092,   979,  1198,  1168,  1176,   488,  1481,  1172,
    1204,  1259,  1343,  1176,    73,  1123,  1489,  1211,  1502,  1513,
    1355,    39,  1229,   319,   914,   467,   322,   467,   878,   375,
     881,   701,   907,   804,   942,   819,  1213,   852,   276,    -1,
     283,    -1,    -1,  1237,    -1,    -1,  1240,  1241,    -1,    -1,
      -1,  1245,    -1,    -1,  1241,    -1,    -1,  1251,    -1,    -1,
      -1,    -1,    -1,  1240,  1234,    -1,    -1,   363,  1245,    -1,
    1240,    -1,    -1,    -1,  1251,  1245,    -1,  1240,  1241,    -1,
      -1,  1251,  1245,  1354,    -1,    -1,    -1,    -1,  1251,    -1,
     149,   208,   151,   152,   153,   154,   155,   156,    -1,    -1,
      -1,    -1,   453,    -1,   400,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   408,    -1,  1459,    -1,    -1,  1494,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,  1510,    -1,  1406,   195,    -1,    -1,   490,
      -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,
      -1,   268,   448,    -1,  1475,    -1,    -1,   453,   275,   276,
    1354,    -1,    -1,    -1,    -1,   282,    -1,    -1,    -1,    -1,
      -1,   288,    -1,    -1,    -1,    -1,    -1,    -1,  1372,    -1,
      -1,    -1,  1376,    -1,    -1,    -1,    -1,    73,    -1,    75,
      -1,  1385,    -1,    -1,   490,    -1,  1390,  1391,    -1,  1376,
    1394,  1395,    -1,    -1,    -1,    -1,  1376,    -1,    -1,    -1,
      -1,    -1,  1406,  1376,    -1,    -1,    -1,    -1,  1412,  1413,
      -1,    -1,    -1,    -1,  1418,    -1,    -1,   113,    -1,    -1,
      -1,    -1,    -1,   529,   530,  1412,  1413,    -1,    -1,   125,
     536,  1418,  1412,  1413,    -1,    -1,   363,    -1,  1418,  1412,
    1413,    -1,    -1,    -1,    -1,  1418,   552,  1451,   144,    -1,
      -1,   147,    -1,   149,  1458,   151,   152,    -1,   154,   155,
     156,    -1,    -1,    -1,  1451,    -1,    -1,    -1,    -1,    -1,
    1474,  1451,    -1,   400,    -1,    -1,    -1,    -1,  1451,  1459,
      -1,   177,    -1,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,    -1,   603,    -1,   195,
     661,    -1,    -1,    -1,   200,    -1,    -1,    -1,    -1,    -1,
      -1,  1515,    -1,   674,   675,   676,  1520,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   453,    -1,  1515,    -1,
      -1,    -1,   638,  1520,   640,  1515,    -1,    -1,    -1,    -1,
    1520,    -1,  1515,    -1,    -1,    -1,    -1,  1520,    -1,    -1,
      -1,    -1,    -1,    -1,   660,   661,    -1,    -1,    10,    11,
      12,    -1,    -1,   490,    -1,    -1,   672,   673,   674,   675,
     676,    -1,    -1,    -1,    26,    -1,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    50,    50,   705,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   714,   770,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   723,   453,   725,
     726,    -1,    -1,    -1,    -1,   552,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   740,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,   753,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   490,    -1,    -1,   764,    73,
      -1,    75,    -1,    -1,   770,    -1,    -1,   773,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   603,    -1,    -1,    -1,
      -1,    60,    61,    -1,    -1,    -1,   792,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,   864,    50,   119,    -1,   868,    -1,   870,
      -1,   638,    -1,   640,    -1,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,   888,    50,    -1,
     836,    -1,    -1,   660,   661,    -1,    -1,   151,   152,    -1,
     154,   155,   156,   849,   123,   851,    -1,   674,   675,   676,
     202,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   864,    -1,
      -1,    -1,   868,   177,   870,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   705,    -1,
      -1,    -1,   888,    -1,   198,   891,   200,   714,    -1,   242,
     243,    -1,    -1,    -1,    -1,   248,   723,    -1,   725,   726,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   913,    -1,    -1,
     916,    -1,    -1,   740,    -1,    -1,    73,   923,    -1,    -1,
     981,    -1,    -1,    -1,    -1,    -1,   661,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   995,    -1,   997,    -1,    -1,   674,
     675,   676,    -1,   770,    -1,    -1,   773,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   965,
     117,    -1,    -1,    -1,    -1,   792,   319,    -1,    -1,   322,
      -1,    -1,   978,   979,    73,   981,    75,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1045,    -1,   992,    -1,    -1,   995,
      -1,   997,    -1,    -1,   151,   152,    -1,   154,   155,   156,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   836,
      -1,    -1,    -1,    -1,    -1,  1021,    -1,    -1,  1024,    -1,
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   770,    -1,   864,   195,  1045,
      -1,   868,  1103,   870,    -1,    -1,    -1,  1108,    -1,  1110,
      -1,    -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,
      -1,   888,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   177,  1140,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,  1099,    -1,   448,   923,  1103,    -1,   198,
     453,   200,  1108,    -1,  1110,    -1,    -1,    -1,    -1,  1115,
      -1,    -1,  1118,  1119,    -1,  1121,    -1,  1178,    -1,    -1,
      -1,    -1,  1128,    -1,    -1,    -1,    -1,    -1,    -1,   864,
      -1,    -1,    -1,   868,  1140,   870,    -1,   490,   965,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   978,   979,   888,   981,    -1,    -1,    -1,    -1,  1220,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   995,    -1,
     997,    -1,  1178,    -1,    -1,    -1,   529,   530,    -1,    -1,
      -1,  1187,  1188,   536,    -1,    -1,    -1,  1248,  1249,    -1,
      -1,    -1,    -1,  1254,  1021,    -1,    -1,  1024,    -1,    -1,
      -1,    10,    11,    12,    -1,  1211,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1220,    -1,    -1,    26,  1045,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,  1248,  1249,    -1,    -1,   981,    -1,  1254,  1255,
      -1,    -1,    -1,  1259,    -1,    -1,    -1,    -1,    -1,    -1,
     995,    -1,   997,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1103,    -1,    -1,    -1,
      -1,  1108,    -1,  1110,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1119,    -1,  1121,    -1,    -1,  1358,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   661,    -1,
    1045,    -1,    -1,  1140,    -1,    -1,    -1,    -1,    -1,   672,
     673,   674,   675,   676,    -1,    -1,    -1,    -1,  1389,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1400,
      -1,    -1,    -1,    -1,  1405,    -1,    -1,    -1,    -1,    -1,
      -1,  1178,  1358,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1187,  1188,    -1,    -1,    -1,    -1,    -1,    -1,  1103,    -1,
      -1,    -1,    -1,  1108,    -1,  1110,    -1,    -1,    -1,  1385,
      -1,    -1,    -1,  1389,    -1,    -1,    -1,    -1,    -1,    -1,
     199,    -1,    -1,  1220,  1400,    -1,    -1,    -1,    -1,  1405,
     753,  1462,    -1,    -1,    -1,  1140,    -1,    -1,    -1,    -1,
      -1,   764,    -1,    -1,    -1,    -1,    -1,   770,    -1,    -1,
      -1,  1248,  1249,    -1,    -1,   453,    -1,  1254,  1255,    -1,
      -1,    -1,  1259,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1178,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1462,  1518,    -1,    -1,
      -1,    -1,   490,  1524,  1470,    -1,    -1,  1528,    -1,  1530,
      -1,    24,    25,    -1,    -1,    28,    -1,    -1,    -1,  1485,
      -1,    -1,    -1,    -1,    -1,  1220,    -1,    -1,  1494,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   849,    -1,   851,    -1,
      -1,    -1,    -1,    -1,  1510,    -1,    -1,    -1,    -1,    -1,
      -1,   864,  1518,  1248,  1249,   868,    -1,   870,  1524,  1254,
      -1,    -1,  1528,  1258,  1530,    -1,    -1,    -1,    -1,    -1,
      -1,  1358,    -1,    -1,    -1,   888,    -1,    26,   891,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
     913,    50,  1389,   916,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1400,    -1,    -1,    -1,    -1,  1405,    -1,
      -1,    -1,    -1,    -1,     5,     6,    -1,     8,     9,    10,
      -1,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    -1,    -1,    26,    27,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,
      -1,    -1,    -1,  1358,    -1,    46,    -1,    48,   981,    -1,
      51,    -1,    53,   661,    -1,  1462,    -1,    -1,    -1,   992,
      -1,    -1,   995,  1470,   997,    -1,   674,   675,    -1,   202,
      -1,    -1,    -1,    -1,  1389,   208,    -1,    -1,  1485,    80,
      -1,   214,    -1,    -1,    -1,  1400,    -1,    -1,    -1,    -1,
    1405,    -1,    -1,    94,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,  1518,  1045,  1428,    -1,    -1,    -1,  1524,    -1,    -1,
      -1,  1528,    -1,  1530,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   268,    -1,    -1,    -1,    -1,
      -1,    -1,   275,   276,    -1,    -1,    -1,  1462,    -1,   282,
      -1,    -1,    -1,    -1,    -1,   288,    -1,    -1,    -1,    -1,
      -1,    -1,   770,    -1,    -1,    -1,  1099,    -1,    -1,    -1,
    1103,    -1,    -1,    -1,    -1,  1108,    -1,  1110,    -1,    -1,
      -1,   182,  1115,    -1,    -1,  1118,    -1,    73,    -1,    75,
      -1,    -1,    -1,    -1,    -1,  1128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1518,    -1,    -1,    -1,  1140,    -1,  1524,
      -1,    -1,    -1,  1528,    -1,  1530,    -1,    -1,    -1,    -1,
      -1,    -1,   223,    -1,    -1,   226,    -1,    -1,    -1,    -1,
     363,    -1,   233,   234,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1178,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,   870,    50,    -1,   151,   152,   400,   154,   155,
     156,    -1,    -1,    -1,    -1,   408,   277,    -1,  1211,    -1,
     888,    -1,   283,    -1,    -1,    -1,   287,  1220,    -1,    -1,
      -1,   177,    -1,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,    -1,    -1,    -1,   310,
      -1,    -1,   198,    -1,   200,  1248,  1249,    -1,    -1,    -1,
     321,  1254,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   338,   339,   340,
     341,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,    -1,    -1,   365,   366,    -1,   368,    -1,    -1,
      -1,    -1,    -1,   981,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,    -1,   995,    -1,   997,
      -1,   392,   393,    -1,   395,   396,   397,    -1,    -1,    -1,
      -1,    -1,   403,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   413,    -1,   415,    -1,    -1,    -1,    -1,   552,
     421,    -1,    -1,    -1,    -1,  1358,    -1,    -1,    -1,    -1,
     431,    -1,   433,    -1,    -1,    -1,    -1,  1045,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1385,    -1,    -1,    -1,  1389,    -1,    -1,   460,
      -1,    -1,   463,   464,   465,    -1,    -1,  1400,    -1,    -1,
     603,    -1,  1405,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   486,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1103,    -1,    -1,    -1,    -1,
    1108,    -1,  1110,    -1,    -1,   638,    -1,   640,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
     521,    -1,    -1,    -1,    -1,    -1,    -1,   660,    -1,  1462,
      -1,    26,  1140,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,
      -1,  1494,    -1,   564,    -1,    -1,    -1,    -1,    -1,    -1,
    1178,    -1,    -1,    -1,    -1,    -1,    -1,  1510,    -1,    -1,
      -1,   714,    -1,    -1,    -1,  1518,    -1,    -1,    -1,    -1,
      -1,  1524,   725,   726,   595,  1528,    -1,  1530,    -1,    -1,
      -1,    -1,    -1,   604,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1220,    -1,    -1,    -1,    -1,    -1,    -1,   620,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
    1248,  1249,    -1,    -1,    -1,   646,  1254,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   659,   792,
      -1,    -1,    -1,    26,    -1,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    -1,   690,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   836,   199,    26,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    52,    -1,   734,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   747,    -1,    -1,    -1,
    1358,    -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   768,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   778,    -1,   100,
     781,  1389,   783,    -1,    -1,    -1,   787,    -1,    -1,    -1,
     923,    -1,  1400,    -1,    -1,   796,    -1,  1405,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   126,   127,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   822,    -1,   144,    -1,    -1,   147,    -1,   149,    -1,
     151,   152,   965,   154,   155,   156,   199,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   978,   979,    -1,   169,    -1,
      -1,    -1,    -1,    -1,  1462,    -1,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   872,   873,   874,   195,    -1,    -1,   878,   879,   199,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1024,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   900,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1518,    -1,    -1,    -1,    -1,    -1,  1524,    -1,   919,    -1,
    1528,    -1,  1530,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,   936,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   944,    -1,   946,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    -1,    -1,    -1,    -1,   976,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,   985,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,    -1,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    -1,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,  1033,    -1,    -1,    -1,  1037,    -1,  1039,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1048,    -1,    -1,
      -1,    -1,    -1,    -1,  1055,  1056,    -1,    42,    43,    44,
      -1,    -1,    -1,  1064,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      75,    -1,    77,    -1,    -1,    -1,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    90,    -1,    -1,    93,    -1,
      -1,  1112,    97,    98,    99,   100,   101,   102,   103,   199,
     105,   106,   107,   108,   109,   110,   111,   112,   113,    -1,
     115,   116,   117,   118,   119,   120,    -1,    -1,    -1,    -1,
     125,   126,  1143,   128,   129,   130,   131,   132,    -1,    -1,
      -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,
     145,   146,   147,   148,   149,   199,   151,   152,    -1,   154,
     155,   156,   157,    -1,   159,    -1,    -1,    -1,   163,    -1,
      -1,    -1,    -1,    -1,   169,   170,    -1,   172,    -1,   174,
     175,   176,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    -1,    -1,
     195,    -1,   197,   198,   199,   200,   201,    -1,   203,   204,
      -1,  1222,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1250,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    75,    -1,    77,    -1,    -1,    -1,
      81,    82,    83,    84,    -1,    86,    -1,    88,    -1,    90,
      -1,    -1,    93,    -1,    -1,    -1,    97,    98,    99,   100,
     101,   102,   103,    -1,   105,   106,   107,   108,   109,   110,
     111,   112,   113,    -1,   115,   116,   117,   118,   119,   120,
      -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,
     131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,
      -1,    -1,    -1,   144,   145,   146,   147,   148,   149,    -1,
     151,   152,    -1,   154,   155,   156,   157,    -1,   159,    -1,
      -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,   170,
      -1,   172,    -1,   174,   175,   176,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,    -1,    -1,   195,    -1,   197,   198,   199,   200,
     201,    12,   203,   204,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    26,    -1,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      -1,    -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    75,    -1,    77,    -1,
      -1,    -1,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    90,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,
      99,   100,   101,   102,   103,    -1,   105,   106,   107,   108,
     109,   110,   111,   112,   113,    -1,   115,   116,   117,   118,
     119,   120,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,
     129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,   148,
     149,    -1,   151,   152,    -1,   154,   155,   156,   157,    -1,
     159,    -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,
     169,   170,    -1,   172,    -1,   174,   175,   176,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,    -1,    -1,   195,    -1,   197,   198,
      -1,   200,   201,    -1,   203,   204,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    42,    43,    44,    -1,    -1,
      -1,    -1,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    75,    -1,
      77,    -1,    -1,    -1,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    90,    -1,    -1,    93,    -1,    -1,    -1,
      97,    98,    99,   100,    -1,   102,   103,    -1,   105,    -1,
     107,   108,   109,   110,   111,   112,   113,    -1,   115,   116,
     117,    -1,   119,   120,    -1,    -1,    -1,    -1,   125,   126,
      -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,
      -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,
     147,   148,   149,    -1,   151,   152,    -1,   154,   155,   156,
     157,    -1,   159,    -1,    -1,    -1,   163,    -1,    -1,    -1,
      -1,    -1,   169,    -1,    -1,    -1,    -1,   174,   175,   176,
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,    -1,    -1,   195,    -1,
     197,   198,   199,   200,   201,    -1,   203,   204,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      75,    -1,    77,    -1,    -1,    -1,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    90,    -1,    -1,    93,    -1,
      -1,    -1,    97,    98,    99,   100,    -1,   102,   103,    -1,
     105,    -1,   107,   108,   109,   110,   111,   112,   113,    -1,
     115,   116,   117,    -1,   119,   120,    -1,    -1,    -1,    -1,
     125,   126,    -1,   128,   129,   130,   131,   132,    -1,    -1,
      -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,
     145,   146,   147,   148,   149,    -1,   151,   152,    -1,   154,
     155,   156,   157,    -1,   159,    -1,    -1,    -1,   163,    -1,
      -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,
     175,   176,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    -1,    -1,
     195,    -1,   197,   198,   199,   200,   201,    -1,   203,   204,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    75,    -1,    77,    -1,    -1,    -1,    81,    82,
      83,    84,    -1,    86,    -1,    88,    -1,    90,    -1,    -1,
      93,    -1,    -1,    -1,    97,    98,    99,   100,    -1,   102,
     103,    -1,   105,    -1,   107,   108,   109,   110,   111,   112,
     113,    -1,   115,   116,   117,    -1,   119,   120,    -1,    -1,
      -1,    -1,   125,   126,    -1,   128,   129,   130,   131,   132,
      -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,
      -1,   144,   145,   146,   147,   148,   149,    -1,   151,   152,
      -1,   154,   155,   156,   157,    -1,   159,    -1,    -1,    -1,
     163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,
      -1,   174,   175,   176,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
      -1,    -1,   195,    -1,   197,   198,   199,   200,   201,    -1,
     203,   204,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    75,    -1,    77,    -1,    -1,    -1,
      81,    82,    83,    84,    85,    86,    -1,    88,    -1,    90,
      -1,    -1,    93,    -1,    -1,    -1,    97,    98,    99,   100,
      -1,   102,   103,    -1,   105,    -1,   107,   108,   109,   110,
     111,   112,   113,    -1,   115,   116,   117,    -1,   119,   120,
      -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,
     131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,
      -1,    -1,    -1,   144,   145,   146,   147,   148,   149,    -1,
     151,   152,    -1,   154,   155,   156,   157,    -1,   159,    -1,
      -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,
      -1,    -1,    -1,   174,   175,   176,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,    -1,    -1,   195,    -1,   197,   198,    -1,   200,
     201,    -1,   203,   204,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    75,    -1,    77,    -1,
      -1,    -1,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    90,    91,    -1,    93,    -1,    -1,    -1,    97,    98,
      99,   100,    -1,   102,   103,    -1,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,    -1,   115,   116,   117,    -1,
     119,   120,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,
     129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,   148,
     149,    -1,   151,   152,    -1,   154,   155,   156,   157,    -1,
     159,    -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,
     169,    -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,    -1,    -1,   195,    -1,   197,   198,
      -1,   200,   201,    -1,   203,   204,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,    44,    -1,    -1,
      -1,    -1,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    75,    -1,
      77,    -1,    -1,    -1,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    90,    -1,    -1,    93,    -1,    -1,    -1,
      97,    98,    99,   100,    -1,   102,   103,    -1,   105,    -1,
     107,   108,   109,   110,   111,   112,   113,    -1,   115,   116,
     117,    -1,   119,   120,    -1,    -1,    -1,    -1,   125,   126,
      -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,
      -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,
     147,   148,   149,    -1,   151,   152,    -1,   154,   155,   156,
     157,    -1,   159,    -1,    -1,    -1,   163,    -1,    -1,    -1,
      -1,    -1,   169,    -1,    -1,    -1,    -1,   174,   175,   176,
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,    -1,    -1,   195,    -1,
     197,   198,   199,   200,   201,    -1,   203,   204,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      75,    -1,    77,    -1,    -1,    -1,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    90,    -1,    -1,    93,    -1,
      -1,    -1,    97,    98,    99,   100,    -1,   102,   103,    -1,
     105,    -1,   107,   108,   109,   110,   111,   112,   113,    -1,
     115,   116,   117,    -1,   119,   120,    -1,    -1,    -1,    -1,
     125,   126,    -1,   128,   129,   130,   131,   132,    -1,    -1,
      -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,
     145,   146,   147,   148,   149,    -1,   151,   152,    -1,   154,
     155,   156,   157,    -1,   159,    -1,    -1,    -1,   163,    -1,
      -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,
     175,   176,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    -1,    -1,
     195,    -1,   197,   198,   199,   200,   201,    -1,   203,   204,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    75,    -1,    77,    -1,    -1,    -1,    81,    82,
      83,    84,    -1,    86,    -1,    88,    89,    90,    -1,    -1,
      93,    -1,    -1,    -1,    97,    98,    99,   100,    -1,   102,
     103,    -1,   105,    -1,   107,   108,   109,   110,   111,   112,
     113,    -1,   115,   116,   117,    -1,   119,   120,    -1,    -1,
      -1,    -1,   125,   126,    -1,   128,   129,   130,   131,   132,
      -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,
      -1,   144,   145,   146,   147,   148,   149,    -1,   151,   152,
      -1,   154,   155,   156,   157,    -1,   159,    -1,    -1,    -1,
     163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,
      -1,   174,   175,   176,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
      -1,    -1,   195,    -1,   197,   198,    -1,   200,   201,    -1,
     203,   204,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    75,    -1,    77,    -1,    -1,    -1,
      81,    82,    83,    84,    -1,    86,    -1,    88,    -1,    90,
      -1,    -1,    93,    -1,    -1,    -1,    97,    98,    99,   100,
      -1,   102,   103,    -1,   105,    -1,   107,   108,   109,   110,
     111,   112,   113,    -1,   115,   116,   117,    -1,   119,   120,
      -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,
     131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,
      -1,    -1,    -1,   144,   145,   146,   147,   148,   149,    -1,
     151,   152,    -1,   154,   155,   156,   157,    -1,   159,    -1,
      -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,
      -1,    -1,    -1,   174,   175,   176,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,    -1,    -1,   195,    -1,   197,   198,   199,   200,
     201,    -1,   203,   204,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    75,    -1,    77,    -1,
      -1,    -1,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    90,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,
      99,   100,    -1,   102,   103,    -1,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,    -1,   115,   116,   117,    -1,
     119,   120,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,
     129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,   148,
     149,    -1,   151,   152,    -1,   154,   155,   156,   157,    -1,
     159,    -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,
     169,    -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,    -1,    -1,   195,    -1,   197,   198,
     199,   200,   201,    -1,   203,   204,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,    44,    -1,    -1,
      -1,    -1,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    75,    -1,
      77,    -1,    -1,    -1,    81,    82,    83,    84,    -1,    86,
      87,    88,    -1,    90,    -1,    -1,    93,    -1,    -1,    -1,
      97,    98,    99,   100,    -1,   102,   103,    -1,   105,    -1,
     107,   108,   109,   110,   111,   112,   113,    -1,   115,   116,
     117,    -1,   119,   120,    -1,    -1,    -1,    -1,   125,   126,
      -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,
      -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,
     147,   148,   149,    -1,   151,   152,    -1,   154,   155,   156,
     157,    -1,   159,    -1,    -1,    -1,   163,    -1,    -1,    -1,
      -1,    -1,   169,    -1,    -1,    -1,    -1,   174,   175,   176,
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,    -1,    -1,   195,    -1,
     197,   198,    -1,   200,   201,    -1,   203,   204,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      75,    -1,    77,    -1,    -1,    -1,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    90,    -1,    -1,    93,    -1,
      -1,    -1,    97,    98,    99,   100,    -1,   102,   103,    -1,
     105,    -1,   107,   108,   109,   110,   111,   112,   113,    -1,
     115,   116,   117,    -1,   119,   120,    -1,    -1,    -1,    -1,
     125,   126,    -1,   128,   129,   130,   131,   132,    -1,    -1,
      -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,
     145,   146,   147,   148,   149,    -1,   151,   152,    -1,   154,
     155,   156,   157,    -1,   159,    -1,    -1,    -1,   163,    -1,
      -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,
     175,   176,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    -1,    -1,
     195,    -1,   197,   198,   199,   200,   201,    -1,   203,   204,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    75,    -1,    77,    -1,    -1,    -1,    81,    82,
      83,    84,    -1,    86,    -1,    88,    -1,    90,    -1,    -1,
      93,    -1,    -1,    -1,    97,    98,    99,   100,    -1,   102,
     103,    -1,   105,    -1,   107,   108,   109,   110,   111,   112,
     113,    -1,   115,   116,   117,    -1,   119,   120,    -1,    -1,
      -1,    -1,   125,   126,    -1,   128,   129,   130,   131,   132,
      -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,
      -1,   144,   145,   146,   147,   148,   149,    -1,   151,   152,
      -1,   154,   155,   156,   157,    -1,   159,    -1,    -1,    -1,
     163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,
      -1,   174,   175,   176,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
      -1,    -1,   195,    -1,   197,   198,   199,   200,   201,    -1,
     203,   204,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    75,    -1,    77,    -1,    -1,    -1,
      81,    82,    83,    84,    -1,    86,    -1,    88,    -1,    90,
      -1,    -1,    93,    -1,    -1,    -1,    97,    98,    99,   100,
      -1,   102,   103,    -1,   105,    -1,   107,   108,   109,   110,
     111,   112,   113,    -1,   115,   116,   117,    -1,   119,   120,
      -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,
     131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,
      -1,    -1,    -1,   144,   145,   146,   147,   148,   149,    -1,
     151,   152,    -1,   154,   155,   156,   157,    -1,   159,    -1,
      -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,
      -1,    -1,    -1,   174,   175,   176,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,    -1,    -1,   195,    -1,   197,   198,   199,   200,
     201,    -1,   203,   204,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    75,    -1,    77,    -1,
      -1,    -1,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    90,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,
      99,   100,    -1,   102,   103,    -1,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,    -1,   115,   116,   117,    -1,
     119,   120,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,
     129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,   148,
     149,    -1,   151,   152,    -1,   154,   155,   156,   157,    -1,
     159,    -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,
     169,    -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,    -1,    -1,   195,    -1,   197,   198,
      -1,   200,   201,    -1,   203,   204,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,
      -1,    -1,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    75,    -1,
      77,    -1,    -1,    -1,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    90,    -1,    -1,    93,    -1,    -1,    -1,
      97,    98,    99,   100,    -1,   102,   103,    -1,   105,    -1,
     107,   108,   109,   110,   111,   112,   113,    -1,   115,   116,
     117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,   126,
      -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,
      -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,
     147,   148,   149,    -1,   151,   152,    -1,   154,   155,   156,
      -1,    -1,   159,    -1,    -1,    -1,   163,    -1,    -1,    -1,
      -1,    -1,   169,    -1,    -1,    -1,    -1,   174,   175,   176,
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,    -1,    -1,   195,    -1,
     197,   198,    -1,   200,   201,    -1,   203,   204,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      75,    -1,    77,    -1,    -1,    -1,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    90,    -1,    -1,    93,    -1,
      -1,    -1,    97,    98,    99,   100,    -1,   102,   103,    -1,
     105,    -1,   107,   108,   109,   110,   111,   112,   113,    -1,
     115,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     125,   126,    -1,   128,   129,   130,   131,   132,    -1,    -1,
      -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,
     145,   146,   147,   148,   149,    -1,   151,   152,    -1,   154,
     155,   156,    -1,    -1,   159,    -1,    -1,    -1,   163,    -1,
      -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,
     175,   176,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    -1,    -1,
     195,    -1,   197,   198,    -1,   200,   201,    -1,   203,   204,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    75,    -1,    77,    -1,    -1,    -1,    81,    82,
      83,    84,    -1,    86,    -1,    88,    -1,    90,    -1,    -1,
      93,    -1,    -1,    -1,    97,    98,    99,   100,    -1,   102,
     103,    -1,   105,    -1,   107,   108,   109,   110,   111,   112,
     113,    -1,   115,   116,   117,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   125,   126,    -1,   128,   129,   130,   131,   132,
      -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,
      -1,   144,   145,   146,   147,   148,   149,    -1,   151,   152,
      -1,   154,   155,   156,    -1,    -1,   159,    -1,    -1,    -1,
     163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,
      -1,   174,   175,   176,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
      -1,    -1,   195,    -1,   197,   198,    -1,   200,   201,    -1,
     203,   204,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    75,    -1,    77,    -1,    -1,    -1,
      81,    82,    83,    84,    -1,    86,    -1,    88,    -1,    90,
      -1,    -1,    93,    -1,    -1,    -1,    97,    98,    99,   100,
      -1,   102,   103,    -1,   105,    -1,   107,   108,   109,   110,
     111,   112,   113,    -1,   115,   116,   117,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,
     131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,
      -1,    -1,    -1,   144,   145,   146,   147,   148,   149,    -1,
     151,   152,    -1,   154,   155,   156,    -1,    -1,   159,    -1,
      -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,
      -1,    -1,    -1,   174,   175,   176,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,    -1,    -1,   195,    -1,   197,   198,    -1,   200,
     201,    -1,   203,   204,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    75,    -1,    77,    -1,
      -1,    -1,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    90,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,
      99,   100,    -1,   102,   103,    -1,   105,    -1,   107,   108,
     109,   110,   111,   112,   113,    -1,   115,   116,   117,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,
     129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,   148,
     149,    -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,
     159,    -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,
     169,    -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,    -1,    -1,   195,    -1,   197,   198,
      -1,   200,   201,    -1,   203,   204,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,
      -1,    -1,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    75,    -1,
      77,    -1,    -1,    -1,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    90,    -1,    -1,    93,    -1,    -1,    -1,
      97,    98,    99,   100,    -1,   102,   103,    -1,   105,    -1,
     107,   108,   109,   110,   111,   112,   113,    -1,   115,   116,
     117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,   126,
      -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,
      -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,
     147,   148,   149,    -1,   151,   152,    -1,   154,   155,   156,
      -1,    -1,   159,    -1,    -1,    -1,   163,    -1,    -1,    -1,
      -1,    -1,   169,    -1,    -1,    -1,    -1,   174,   175,   176,
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,    -1,    -1,   195,    -1,
     197,   198,    -1,   200,   201,    -1,   203,   204,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      75,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,   110,   111,   112,   113,    -1,
      73,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     125,   126,    -1,   128,   129,   130,   131,   132,    -1,    -1,
      -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,
     145,   146,   147,   148,   149,    -1,   151,   152,    -1,   154,
     155,   156,    -1,    -1,   159,    -1,    -1,    -1,   163,    -1,
      -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   151,   152,
     195,   154,   155,   156,    -1,   200,   201,    -1,   203,   204,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,    -1,
      -1,    -1,   195,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    75,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,   112,
     113,    -1,    -1,   116,   117,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   125,   126,    -1,   128,   129,   130,   131,   132,
      -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,
      -1,   144,   145,   146,   147,    -1,   149,    -1,   151,   152,
      -1,   154,   155,   156,    -1,    -1,   159,    -1,    -1,    -1,
     163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,
      -1,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
      -1,    -1,   195,    -1,   197,    -1,    -1,   200,   201,    -1,
     203,   204,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,
     111,   112,   113,    -1,    73,   116,   117,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,
     131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,
      -1,    -1,    -1,   144,   145,   146,   147,   148,   149,    -1,
     151,   152,    -1,   154,   155,   156,    -1,    -1,   159,    -1,
      -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,
      -1,    -1,    -1,   174,   175,   176,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   151,   152,   195,   154,   155,   156,    -1,   200,
     201,    -1,   203,   204,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,    32,    -1,    -1,   195,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    75,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,   110,   111,   112,   113,    -1,    73,   116,   117,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,
     129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,    -1,
     149,    -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,
     159,    -1,   119,    -1,   163,    -1,    -1,    -1,    -1,    -1,
     169,    -1,    -1,    -1,    -1,   174,    -1,   176,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   151,   152,   195,   154,   155,   156,
      -1,   200,   201,    -1,   203,   204,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,
      -1,    -1,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    75,    -1,
      -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,   110,   111,   112,   113,    -1,    -1,   116,
     117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,   126,
      -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,
      -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,
     147,    -1,   149,    -1,   151,   152,    -1,   154,   155,   156,
      -1,    -1,   159,    -1,    -1,    -1,   163,    -1,    -1,    -1,
      -1,    -1,   169,    -1,    -1,    -1,    -1,   174,    -1,   176,
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,    -1,    -1,   195,    -1,
     197,    -1,    -1,   200,   201,    -1,   203,   204,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      75,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,   110,   111,   112,   113,    -1,
      -1,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     125,   126,    -1,   128,   129,   130,   131,   132,    -1,    -1,
      -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,
     145,   146,   147,    -1,   149,    -1,   151,   152,    -1,   154,
     155,   156,    -1,    -1,   159,    -1,    -1,    -1,   163,    -1,
      -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,
      -1,   176,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    -1,    -1,
     195,    -1,   197,    -1,    -1,   200,   201,    -1,   203,   204,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    75,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    97,    -1,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,   112,
     113,    -1,    73,   116,   117,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   125,   126,    -1,   128,   129,   130,   131,   132,
      -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,
      -1,   144,   145,   146,   147,    -1,   149,    -1,   151,   152,
      -1,   154,   155,   156,    -1,    -1,   159,    -1,    -1,    -1,
     163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,
      -1,   174,    -1,   176,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     151,   152,   195,   154,   155,   156,    -1,   200,   201,    -1,
     203,   204,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,
     111,   112,   113,    -1,    -1,   116,   117,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,
     131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,
      -1,    -1,    -1,   144,   145,   146,   147,    -1,   149,    -1,
     151,   152,    -1,   154,   155,   156,    -1,    -1,   159,    -1,
      -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,
      -1,    -1,    -1,   174,    -1,   176,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,    -1,    -1,   195,   196,    -1,    -1,    -1,   200,
     201,    -1,   203,   204,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    75,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,   110,   111,   112,   113,    -1,    -1,   116,   117,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,
     129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,    -1,
     149,    -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,
     159,    -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,
     169,    -1,    -1,    -1,    -1,   174,    -1,   176,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,    -1,    -1,   195,    -1,    -1,    -1,
      -1,   200,   201,    -1,   203,   204,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,
      -1,    -1,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    75,    -1,
      -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,   110,   111,   112,   113,    -1,    -1,   116,
     117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,   126,
      -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,
      -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,
     147,    -1,   149,    -1,   151,   152,    -1,   154,   155,   156,
      -1,    -1,   159,    -1,    -1,    -1,   163,    -1,    -1,    -1,
      -1,    -1,   169,    -1,    -1,    -1,    -1,   174,    -1,   176,
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,    -1,    -1,   195,    -1,
      -1,    -1,    -1,   200,   201,    -1,   203,   204,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      75,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,   110,   111,   112,   113,    -1,
      -1,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     125,   126,    -1,   128,   129,   130,   131,   132,    -1,    -1,
      -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,
     145,   146,   147,    -1,   149,    -1,   151,   152,    -1,   154,
     155,   156,    -1,    -1,   159,    -1,    -1,    -1,   163,    -1,
      -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,
      -1,   176,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    -1,    -1,
     195,    -1,    -1,    -1,    -1,   200,   201,    -1,   203,   204,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    75,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,   112,
     113,    -1,    -1,   116,   117,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   125,   126,    -1,   128,   129,   130,   131,   132,
      -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,
      -1,   144,   145,   146,   147,    -1,   149,    -1,   151,   152,
      -1,   154,   155,   156,    -1,    -1,   159,    -1,    -1,    -1,
     163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,
      -1,   174,    -1,   176,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
      -1,    -1,   195,    -1,    -1,    -1,    -1,   200,   201,    -1,
     203,   204,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,
     111,   112,   113,    -1,    -1,   116,   117,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,
     131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,
      -1,    -1,    -1,   144,   145,   146,   147,    -1,   149,    -1,
     151,   152,    -1,   154,   155,   156,    -1,    -1,   159,    -1,
      -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,
      -1,    -1,    -1,   174,    -1,   176,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,    -1,    -1,   195,    -1,    -1,    -1,    -1,   200,
     201,    -1,   203,   204,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    75,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,   110,   111,   112,   113,    -1,    -1,   116,   117,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,
     129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,    -1,
     149,    -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,
     159,    -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,
     169,    -1,    -1,    -1,    -1,   174,    -1,   176,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,    -1,    -1,   195,    -1,    -1,   198,
      -1,   200,   201,    -1,   203,   204,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,
      -1,    -1,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    75,    -1,
      -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,   110,   111,   112,   113,    -1,    -1,   116,
     117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,   126,
      -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,
      -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,
     147,    -1,   149,    -1,   151,   152,    -1,   154,   155,   156,
      -1,    -1,   159,    -1,    -1,    -1,   163,    -1,    -1,    -1,
      -1,    -1,   169,    -1,    -1,    -1,    -1,   174,    -1,   176,
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,    -1,    -1,   195,    -1,
      -1,    -1,    -1,   200,   201,    -1,   203,   204,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      75,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,   110,   111,   112,   113,    -1,
      -1,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     125,   126,    -1,   128,   129,   130,   131,   132,    -1,    -1,
      -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,
     145,   146,   147,    -1,   149,    -1,   151,   152,    -1,   154,
     155,   156,    -1,    -1,   159,    -1,    -1,    -1,   163,    -1,
      -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,
      -1,   176,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,    -1,    -1,
     195,    -1,    -1,    -1,    -1,   200,   201,    -1,   203,   204,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    75,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,   112,
     113,    -1,    -1,   116,   117,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   125,   126,    -1,   128,   129,   130,   131,   132,
      -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,
      -1,   144,   145,   146,   147,    -1,   149,    -1,   151,   152,
      -1,   154,   155,   156,    -1,    -1,   159,    -1,    -1,    -1,
     163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,
      -1,   174,    -1,   176,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
      -1,    -1,   195,    -1,    -1,    -1,    -1,   200,   201,    -1,
     203,   204,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,
     111,   112,   113,    -1,    -1,   116,   117,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,
     131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,
      -1,    -1,    -1,   144,   145,   146,   147,    -1,   149,    -1,
     151,   152,    -1,   154,   155,   156,    -1,    -1,   159,    -1,
      -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,
      -1,    -1,    -1,   174,    -1,   176,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,    -1,    -1,   195,    10,    11,    12,    -1,   200,
     201,    -1,   203,   204,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,    -1,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,    -1,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    26,    -1,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,   199,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   199,    -1,    10,    11,    12,
      -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,    10,
      11,    12,    13,    26,    -1,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,   199,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    63,    64,    65,    66,    67,    68,    69,    -1,
      -1,    -1,    73,    -1,    -1,    -1,    -1,   199,    -1,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,    -1,    -1,   125,   126,   199,   128,   129,   130,
     131,   132,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   144,   145,   146,    -1,   148,    -1,    -1,
     151,   152,    -1,   154,   155,   156,   157,   158,   159,    -1,
      -1,   162,    -1,    -1,    -1,    -1,    -1,    -1,   169,   170,
     183,   172,    -1,   174,   175,   176,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    50,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
      -1,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    -1,    50,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,   197,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,   197,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    50,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
     197,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    -1,    50,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,   197,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,   197,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,   197,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    50,    -1,    10,    11,    12,    -1,    73,    -1,
      75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   196,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     196,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    26,    50,   151,   152,    -1,   154,
     155,   156,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   191,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      52,    -1,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,    -1,    -1,    -1,
      -1,    73,    -1,   198,    -1,   200,    -1,    -1,    -1,   187,
     188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,
     112,   113,    -1,    -1,    -1,    -1,    52,    -1,   184,    -1,
      -1,    -1,    -1,    -1,   126,   127,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,
      -1,    -1,   144,    -1,    -1,   147,    -1,   149,    -1,   151,
     152,    -1,   154,   155,   156,    -1,    -1,   182,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,
      26,    -1,    -1,    -1,   176,   177,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     126,   127,    -1,   195,    -1,    -1,    52,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   144,    -1,
      -1,   147,    -1,   149,    -1,   151,   152,    73,   154,   155,
     156,    -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   169,    -1,    -1,    26,    -1,    -1,    -1,
      -1,   177,    -1,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,    -1,    -1,    -1,   195,
      -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     126,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,   144,    -1,
      -1,   147,    -1,   149,    -1,   151,   152,    -1,   154,   155,
     156,    -1,    73,    -1,   160,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   177,    -1,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   126,   127,    -1,   195,
      -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,   149,
      -1,   151,   152,    62,   154,   155,   156,    -1,    -1,    -1,
      -1,    70,    71,    72,    73,    -1,   147,    -1,    -1,   169,
     151,   152,    81,   154,   155,   156,    -1,   177,    -1,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,    -1,    -1,    -1,   195,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,    -1,    -1,    -1,    -1,    -1,    -1,   126,    -1,   128,
     129,   130,   131,   132,    -1,    43,    44,    -1,    -1,    -1,
     139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,    -1,
     149,    -1,   151,   152,    62,   154,   155,   156,    -1,    -1,
     159,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
     169,    -1,    -1,    81,    -1,   174,    -1,    -1,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    63,    64,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    73,    -1,    75,    -1,    -1,   126,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   151,   152,    -1,   154,   155,   156,    -1,
      -1,    -1,    -1,   113,    -1,    -1,    64,    -1,    -1,    -1,
      -1,   169,    -1,    -1,    -1,    73,    -1,    75,    -1,   177,
      -1,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   144,    -1,    -1,   147,    -1,   149,
      -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,
      -1,    -1,    -1,   163,    -1,   113,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   177,    -1,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,    -1,    -1,    -1,   195,   144,    -1,    -1,   147,
     200,   149,    -1,   151,   152,    -1,   154,   155,   156,    73,
      -1,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   177,
      -1,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,    -1,    -1,    -1,   195,    -1,   113,
      -1,    -1,   200,    -1,    -1,    -1,    -1,    -1,    -1,    73,
      -1,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     144,    -1,    -1,   147,    -1,   149,    -1,   151,   152,    -1,
     154,   155,   156,    -1,    -1,    -1,    -1,    -1,    -1,   113,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   125,    -1,   177,    -1,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,    -1,    -1,
     144,   195,    -1,   147,   198,   149,   200,   151,   152,    -1,
     154,   155,   156,    73,    -1,    75,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   177,    -1,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,    -1,    -1,
      -1,   195,    -1,   113,    -1,    -1,   200,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   125,    -1,    73,    -1,    75,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,   149,
      -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,
      -1,    70,    71,    72,    73,    -1,    -1,   177,    -1,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,    73,    -1,    75,   195,    -1,    -1,   144,    -1,
     200,   147,    -1,   149,    -1,   151,   152,    -1,   154,   155,
     156,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   177,   113,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,    73,    -1,    75,   195,
      -1,    -1,   151,   152,   200,   154,   155,   156,    -1,    -1,
      -1,    -1,    -1,   144,    -1,    -1,   147,    -1,   149,    -1,
     151,   152,    -1,   154,   155,   156,    -1,    -1,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,    -1,    -1,    -1,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,    73,    -1,    -1,   195,    -1,    -1,    -1,    -1,   200,
      -1,    -1,    -1,    -1,   151,   152,    -1,   154,   155,   156,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,    73,    -1,    -1,    -1,    -1,
      -1,   198,    -1,   200,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   144,    -1,    -1,   147,    -1,    -1,    -1,   151,
     152,    -1,   154,   155,   156,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    73,    -1,   177,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
      73,    -1,    -1,    -1,    -1,    -1,    -1,   199,    -1,   147,
      -1,    -1,    -1,   151,   152,    -1,   154,   155,   156,    -1,
      -1,    -1,    -1,    -1,   114,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   126,    -1,    -1,   177,
      -1,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   144,    -1,    -1,   147,    -1,   149,
     198,   151,   152,    -1,   154,   155,   156,    73,    -1,    75,
      76,   144,    -1,    -1,   147,    -1,   149,    -1,   151,   152,
      -1,   154,   155,   156,    73,    -1,    -1,   177,    -1,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,    -1,    -1,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   151,   152,    -1,   154,   155,
     156,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     149,    -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,
      -1,   177,    -1,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,    -1,    -1,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
      -1,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    -1,    50,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    26,   124,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,   124,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,   124,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    26,   124,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      92,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,    -1,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,    -1,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   206,   207,     0,   208,     3,     4,     5,     6,     7,
      13,    42,    43,    44,    49,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    70,    71,    72,    73,    74,    75,    77,    81,    82,
      83,    84,    86,    88,    90,    93,    97,    98,    99,   100,
     101,   102,   103,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   115,   116,   117,   118,   119,   120,   125,   126,
     128,   129,   130,   131,   132,   139,   144,   145,   146,   147,
     148,   149,   151,   152,   154,   155,   156,   157,   159,   163,
     169,   170,   172,   174,   175,   176,   177,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   195,   197,   198,   200,   201,   203,   204,   209,   212,
     215,   216,   217,   218,   219,   220,   223,   238,   239,   243,
     248,   254,   309,   310,   315,   319,   320,   321,   322,   323,
     324,   325,   326,   328,   331,   340,   341,   342,   344,   345,
     347,   366,   376,   377,   378,   383,   386,   404,   409,   411,
     412,   413,   414,   415,   416,   417,   418,   420,   433,   435,
     437,   111,   112,   113,   125,   144,   212,   238,   309,   325,
     411,   325,   195,   325,   325,   325,   402,   403,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
      75,   113,   195,   216,   377,   378,   411,   411,    32,   325,
     424,   425,   325,   113,   195,   216,   377,   378,   379,   410,
     416,   421,   422,   195,   316,   380,   195,   316,   332,   317,
     325,   225,   316,   195,   195,   195,   316,   197,   325,   212,
     197,   325,    26,    52,   126,   127,   149,   169,   195,   212,
     219,   438,   448,   449,   178,   197,   322,   325,   346,   348,
     198,   231,   325,   147,   213,   214,   215,    75,   200,   280,
     281,   119,   119,    75,   282,   195,   195,   195,   195,   212,
     252,   439,   195,   195,    75,    80,   140,   141,   142,   430,
     431,   147,   198,   215,   215,    97,   325,   253,   439,   149,
     195,   439,   439,   325,   333,   315,   325,   326,   411,   221,
     198,    80,   381,   430,    80,   430,   430,    27,   147,   165,
     440,   195,     9,   197,    32,   237,   149,   251,   439,   113,
     238,   310,   197,   197,   197,   197,   197,   197,    10,    11,
      12,    26,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    50,   197,    62,    62,   197,   198,   143,
     120,   157,   254,   308,   309,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    60,    61,   123,
     406,   407,    62,   198,   408,   195,    62,   198,   200,   417,
     195,   237,   238,    14,   325,    41,   212,   401,   195,   315,
     411,   143,   411,   124,   202,     9,   388,   315,   411,   440,
     143,   195,   382,   123,   406,   407,   408,   196,   325,    27,
     223,     8,   334,     9,   197,   223,   224,   317,   318,   325,
     212,   266,   227,   197,   197,   197,   449,   449,   165,   195,
     100,   441,   449,    14,   212,    75,   197,   197,   197,   178,
     179,   180,   185,   186,   189,   190,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   361,   362,   363,   232,   104,
     162,   197,   215,     9,   197,    92,   198,   411,     9,   197,
      14,     9,   197,   411,   434,   434,   315,   326,   411,   196,
     165,   246,   125,   411,   423,   424,    62,   123,   140,   431,
      74,   325,   411,    80,   140,   431,   215,   211,   197,   198,
     197,   124,   249,   367,   369,    81,   335,   336,   338,    14,
      92,   436,   160,   276,   277,   404,   405,   196,   196,   196,
     199,   222,   223,   239,   243,   248,   325,   201,   203,   204,
     212,   441,    32,   278,   279,   325,   438,   195,   439,   244,
     237,   325,   325,   325,    27,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   379,   325,   419,
     419,   325,   426,   427,   119,   198,   212,   416,   417,   252,
     253,   251,   238,    32,   148,   319,   322,   325,   346,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     198,   212,   416,   419,   325,   278,   419,   325,   423,   237,
     196,   195,   400,     9,   388,   315,   196,   212,    32,   325,
      32,   325,   196,   196,   416,   278,   198,   212,   416,   196,
     221,   270,   198,   325,   325,    84,    27,   223,   264,   197,
      92,    14,     9,   196,    27,   198,   267,   449,    81,   445,
     446,   447,   195,     9,    43,    44,    62,   126,   139,   149,
     169,   177,   216,   217,   219,   343,   377,   383,   384,   385,
     181,    75,   325,    75,    75,   325,   358,   359,   325,   325,
     351,   361,   184,   364,   221,   195,   230,    92,   214,   212,
     325,   281,   384,    75,     9,   196,   196,   196,   196,   196,
     197,   212,   444,   121,   257,   195,     9,   196,   196,    75,
      76,   212,   432,   212,    62,   199,   199,   208,   210,   325,
     122,   256,   164,    47,   149,   164,   371,   124,     9,   388,
     196,   449,   449,    14,   193,     9,   389,   449,   450,   123,
     406,   407,   408,   199,     9,   166,   411,   196,     9,   389,
      14,   329,   240,   121,   255,   195,   439,   325,    27,   202,
     202,   124,   199,     9,   388,   325,   440,   195,   247,   250,
     245,   237,    64,   411,   325,   440,   195,   202,   199,   196,
     202,   199,   196,    43,    44,    62,    70,    71,    72,    81,
     126,   139,   169,   177,   212,   391,   393,   396,   399,   212,
     411,   411,   124,   406,   407,   408,   196,   325,   271,    67,
      68,   272,   221,   316,   221,   318,    32,   125,   261,   411,
     384,   212,    27,   223,   265,   197,   268,   197,   268,     9,
     166,   124,     9,   388,   196,   160,   441,   442,   449,   384,
     384,   384,   387,   390,   195,    80,   143,   195,   195,   143,
     198,   325,   181,   181,    14,   187,   188,   360,     9,   191,
     364,    75,   199,   377,   198,   234,   212,   199,    14,   411,
     197,    92,     9,   166,   258,   377,   198,   423,   125,   411,
      14,   202,   325,   199,   208,   258,   198,   370,    14,   325,
     335,   197,   449,    27,   443,   160,   405,    32,    75,   198,
     212,   416,   449,    32,   325,   384,   276,   195,   377,   256,
     330,   241,   325,   325,   325,   199,   195,   278,   257,   256,
     255,   439,   379,   199,   195,   278,    14,    70,    71,    72,
     212,   392,   392,   393,   394,   395,   195,    80,   140,   195,
     195,     9,   388,   196,   400,    32,   325,   199,    67,    68,
     273,   316,   223,   199,   197,    85,   197,   411,   195,   124,
     260,    14,   221,   268,    94,    95,    96,   268,   199,   449,
     449,   445,     9,   196,   196,   124,   202,     9,   388,   387,
     212,   335,   337,   339,   387,   119,   212,   384,   428,   429,
     325,   325,   325,   359,   325,   349,    75,   235,   384,   449,
     212,     9,   283,   196,   195,   319,   322,   325,   202,   199,
     283,   150,   163,   198,   366,   373,   150,   198,   372,   124,
     197,   449,   334,   450,    75,    14,   325,   440,   195,   411,
     196,   276,   198,   276,   195,   124,   195,   278,   196,   198,
     198,   256,   242,   382,   195,   278,   196,   124,   202,     9,
     388,   394,   140,   335,   397,   398,   394,   393,   411,   316,
      27,    69,   223,   197,   318,   423,   261,   196,   384,    91,
      94,   197,   325,    27,   197,   269,   199,   166,   160,    27,
     384,   384,   196,   124,     9,   388,   196,   196,   124,   199,
       9,   388,   182,   196,   221,    92,   377,     4,   101,   106,
     114,   151,   152,   154,   199,   284,   307,   308,   309,   314,
     404,   423,   199,   199,    47,   325,   325,   325,    32,    75,
      14,   384,   199,   195,   278,   443,   196,   283,   196,   276,
     325,   278,   196,   283,   283,   198,   195,   278,   196,   393,
     393,   196,   124,   196,     9,   388,   196,    27,   221,   197,
     196,   196,   228,   197,   197,   269,   221,   449,   124,   384,
     335,   384,   384,   325,   198,   199,   449,   121,   122,   438,
     259,   377,   114,   126,   149,   155,   293,   294,   295,   377,
     153,   299,   300,   117,   195,   212,   301,   302,   285,   238,
     449,     9,   197,   308,   196,   149,   368,   199,   199,    75,
      14,   384,   195,   278,   196,   106,   327,   443,   199,   443,
     196,   196,   199,   199,   283,   276,   196,   124,   393,   335,
     221,   226,    27,   223,   263,   221,   196,   384,   124,   124,
     183,   221,   377,   377,    14,     9,   197,   198,   198,     9,
     197,     3,     4,     5,     6,     7,    10,    11,    12,    13,
      50,    63,    64,    65,    66,    67,    68,    69,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   125,
     126,   128,   129,   130,   131,   132,   144,   145,   146,   148,
     157,   158,   159,   162,   169,   170,   172,   174,   175,   176,
     212,   374,   375,     9,   197,   149,   153,   212,   302,   303,
     304,   197,    75,   313,   237,   286,   438,   238,    14,   384,
     278,   196,   195,   198,   197,   198,   305,   327,   443,   199,
     196,   393,   124,    27,   223,   262,   221,   384,   384,   325,
     199,   197,   197,   384,   377,   289,   296,   383,   294,    14,
      27,    44,   297,   300,     9,    30,   196,    26,    43,    46,
      14,     9,   197,   439,   313,    14,   237,   384,   196,    32,
      75,   365,   221,   221,   198,   305,   443,   393,   221,    89,
     184,   233,   199,   212,   219,   290,   291,   292,     9,   199,
     384,   375,   375,    52,   298,   303,   303,    26,    43,    46,
     384,    75,   195,   197,   384,   439,    75,     9,   389,   199,
     199,   221,   305,    87,   197,    75,   104,   229,   143,    92,
     383,   156,    14,   287,   195,    32,    75,   196,   199,   197,
     195,   162,   236,   212,   308,   309,   384,   160,   274,   275,
     405,   288,    75,   377,   234,   158,   212,   197,   196,     9,
     389,   108,   109,   110,   311,   312,   274,    75,   259,   197,
     443,   160,   405,   450,   196,   196,   197,   197,   198,   306,
     311,    32,    75,   443,   198,   221,   450,    75,    14,   306,
     221,   199,    32,    75,    14,   384,   199,    75,    14,   384,
      14,   384,   384
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
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
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
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
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
# define YYLEX yylex (&yylval, &yylloc)
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

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
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


/*----------.
| yyparse.  |
`----------*/

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
    YYLTYPE yyerror_range[3];

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

#if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
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
  if (yypact_value_is_default (yyn))
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
      if (yytable_value_is_error (yyn))
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

/* Line 1806 of yacc.c  */
#line 738 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 741 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 748 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));}
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 749 "hphp.y"
    { }
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 752 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 753 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 754 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 755 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 756 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 757 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 760 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();}
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 762 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());}
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 763 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);}
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 764 "hphp.y"
    { _p->onNamespaceStart("");}
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 765 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);}
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 766 "hphp.y"
    { _p->nns(); (yyval).reset();}
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 767 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;}
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 778 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 780 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 781 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 782 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 783 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 796 "hphp.y"
    { }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 797 "hphp.y"
    { }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 800 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");}
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 801 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");}
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 802 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());}
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 804 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());}
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;}
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 813 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;}
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 815 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));}
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 816 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;}
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 819 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 826 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 833 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 841 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 844 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 850 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 851 "hphp.y"
    { _p->onStatementListStart((yyval));}
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 854 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 856 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 857 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 860 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 864 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 869 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));}
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 870 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 872 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 876 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 879 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 883 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 885 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 888 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 890 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 893 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);}
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 894 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));}
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 895 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);}
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 896 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));}
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 897 "hphp.y"
    { _p->onReturn((yyval), NULL);}
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 898 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));}
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 899 "hphp.y"
    { _p->onYieldBreak((yyval));}
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 900 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 901 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 902 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);}
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 903 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));}
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 904 "hphp.y"
    { (yyval).reset(); (yyval) = ';';}
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 905 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);}
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 908 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 910 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 914 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;}
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 921 "hphp.y"
    { _p->onCompleteLabelScope(false);}
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 922 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));}
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 925 "hphp.y"
    { _p->onCompleteLabelScope(false);}
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 926 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));}
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 927 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 928 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval));}
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 932 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 933 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 934 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 935 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 936 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 937 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 938 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); }
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 939 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 940 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 941 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 942 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 950 "hphp.y"
    { _p->onNewLabelScope(false);}
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 951 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 960 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));}
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 961 "hphp.y"
    { (yyval).reset();}
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 965 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 967 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 973 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 974 "hphp.y"
    { (yyval).reset();}
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 978 "hphp.y"
    { (yyval) = 1;}
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 979 "hphp.y"
    { (yyval).reset();}
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 983 "hphp.y"
    { _p->pushFuncLocation(); }
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 988 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();}
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 994 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 1000 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();}
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 1006 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 1012 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();}
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 1018 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 1026 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));}
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 1029 "hphp.y"
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
                                         _p->popTypeScope();}
    break;

  case 124:

/* Line 1806 of yacc.c  */
#line 1044 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));}
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 1047 "hphp.y"
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
                                         _p->popTypeScope();}
    break;

  case 126:

/* Line 1806 of yacc.c  */
#line 1061 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));}
    break;

  case 127:

/* Line 1806 of yacc.c  */
#line 1064 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 128:

/* Line 1806 of yacc.c  */
#line 1069 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));}
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 1072 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 130:

/* Line 1806 of yacc.c  */
#line 1079 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));}
    break;

  case 131:

/* Line 1806 of yacc.c  */
#line 1082 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0);
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 1090 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));}
    break;

  case 133:

/* Line 1806 of yacc.c  */
#line 1093 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 1101 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 1102 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 136:

/* Line 1806 of yacc.c  */
#line 1106 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 1109 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 1112 "hphp.y"
    { (yyval) = T_CLASS;}
    break;

  case 139:

/* Line 1806 of yacc.c  */
#line 1113 "hphp.y"
    { (yyval) = T_ABSTRACT;}
    break;

  case 140:

/* Line 1806 of yacc.c  */
#line 1114 "hphp.y"
    { (yyval) = T_FINAL;}
    break;

  case 141:

/* Line 1806 of yacc.c  */
#line 1118 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 142:

/* Line 1806 of yacc.c  */
#line 1119 "hphp.y"
    { (yyval).reset();}
    break;

  case 143:

/* Line 1806 of yacc.c  */
#line 1122 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 1123 "hphp.y"
    { (yyval).reset();}
    break;

  case 145:

/* Line 1806 of yacc.c  */
#line 1126 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 146:

/* Line 1806 of yacc.c  */
#line 1127 "hphp.y"
    { (yyval).reset();}
    break;

  case 147:

/* Line 1806 of yacc.c  */
#line 1130 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 148:

/* Line 1806 of yacc.c  */
#line 1132 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 149:

/* Line 1806 of yacc.c  */
#line 1135 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 150:

/* Line 1806 of yacc.c  */
#line 1137 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 151:

/* Line 1806 of yacc.c  */
#line 1141 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 152:

/* Line 1806 of yacc.c  */
#line 1142 "hphp.y"
    { (yyval).reset();}
    break;

  case 153:

/* Line 1806 of yacc.c  */
#line 1145 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 154:

/* Line 1806 of yacc.c  */
#line 1146 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;}
    break;

  case 155:

/* Line 1806 of yacc.c  */
#line 1147 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);}
    break;

  case 156:

/* Line 1806 of yacc.c  */
#line 1151 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 157:

/* Line 1806 of yacc.c  */
#line 1153 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 158:

/* Line 1806 of yacc.c  */
#line 1156 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 159:

/* Line 1806 of yacc.c  */
#line 1158 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 160:

/* Line 1806 of yacc.c  */
#line 1161 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 161:

/* Line 1806 of yacc.c  */
#line 1163 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 162:

/* Line 1806 of yacc.c  */
#line 1166 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 163:

/* Line 1806 of yacc.c  */
#line 1168 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 166:

/* Line 1806 of yacc.c  */
#line 1178 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 167:

/* Line 1806 of yacc.c  */
#line 1179 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 168:

/* Line 1806 of yacc.c  */
#line 1180 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 169:

/* Line 1806 of yacc.c  */
#line 1181 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);}
    break;

  case 170:

/* Line 1806 of yacc.c  */
#line 1186 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 171:

/* Line 1806 of yacc.c  */
#line 1188 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));}
    break;

  case 172:

/* Line 1806 of yacc.c  */
#line 1189 "hphp.y"
    { (yyval).reset();}
    break;

  case 173:

/* Line 1806 of yacc.c  */
#line 1192 "hphp.y"
    { (yyval).reset();}
    break;

  case 174:

/* Line 1806 of yacc.c  */
#line 1193 "hphp.y"
    { (yyval).reset();}
    break;

  case 175:

/* Line 1806 of yacc.c  */
#line 1198 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 176:

/* Line 1806 of yacc.c  */
#line 1199 "hphp.y"
    { (yyval).reset();}
    break;

  case 177:

/* Line 1806 of yacc.c  */
#line 1204 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 178:

/* Line 1806 of yacc.c  */
#line 1205 "hphp.y"
    { (yyval).reset();}
    break;

  case 179:

/* Line 1806 of yacc.c  */
#line 1208 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 180:

/* Line 1806 of yacc.c  */
#line 1209 "hphp.y"
    { (yyval).reset();}
    break;

  case 181:

/* Line 1806 of yacc.c  */
#line 1212 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);}
    break;

  case 182:

/* Line 1806 of yacc.c  */
#line 1213 "hphp.y"
    { (yyval).reset();}
    break;

  case 183:

/* Line 1806 of yacc.c  */
#line 1218 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]);}
    break;

  case 184:

/* Line 1806 of yacc.c  */
#line 1220 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 185:

/* Line 1806 of yacc.c  */
#line 1221 "hphp.y"
    { (yyval).reset();}
    break;

  case 186:

/* Line 1806 of yacc.c  */
#line 1222 "hphp.y"
    { (yyval).reset();}
    break;

  case 187:

/* Line 1806 of yacc.c  */
#line 1228 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));}
    break;

  case 188:

/* Line 1806 of yacc.c  */
#line 1232 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));}
    break;

  case 189:

/* Line 1806 of yacc.c  */
#line 1237 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));}
    break;

  case 190:

/* Line 1806 of yacc.c  */
#line 1242 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));}
    break;

  case 191:

/* Line 1806 of yacc.c  */
#line 1247 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));}
    break;

  case 192:

/* Line 1806 of yacc.c  */
#line 1252 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));}
    break;

  case 193:

/* Line 1806 of yacc.c  */
#line 1258 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));}
    break;

  case 194:

/* Line 1806 of yacc.c  */
#line 1264 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));}
    break;

  case 195:

/* Line 1806 of yacc.c  */
#line 1270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]);}
    break;

  case 196:

/* Line 1806 of yacc.c  */
#line 1272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 197:

/* Line 1806 of yacc.c  */
#line 1273 "hphp.y"
    { (yyval).reset();}
    break;

  case 198:

/* Line 1806 of yacc.c  */
#line 1274 "hphp.y"
    { (yyval).reset();}
    break;

  case 199:

/* Line 1806 of yacc.c  */
#line 1279 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),0,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL);}
    break;

  case 200:

/* Line 1806 of yacc.c  */
#line 1282 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),1,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL);}
    break;

  case 201:

/* Line 1806 of yacc.c  */
#line 1286 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),1,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL);}
    break;

  case 202:

/* Line 1806 of yacc.c  */
#line 1290 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),0,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL);}
    break;

  case 203:

/* Line 1806 of yacc.c  */
#line 1294 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),0,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL);}
    break;

  case 204:

/* Line 1806 of yacc.c  */
#line 1298 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),1,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL);}
    break;

  case 205:

/* Line 1806 of yacc.c  */
#line 1303 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),1,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL);}
    break;

  case 206:

/* Line 1806 of yacc.c  */
#line 1308 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),0,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL);}
    break;

  case 207:

/* Line 1806 of yacc.c  */
#line 1314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 208:

/* Line 1806 of yacc.c  */
#line 1315 "hphp.y"
    { (yyval).reset();}
    break;

  case 209:

/* Line 1806 of yacc.c  */
#line 1318 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),0);}
    break;

  case 210:

/* Line 1806 of yacc.c  */
#line 1319 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),1);}
    break;

  case 211:

/* Line 1806 of yacc.c  */
#line 1321 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 212:

/* Line 1806 of yacc.c  */
#line 1323 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 213:

/* Line 1806 of yacc.c  */
#line 1327 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 214:

/* Line 1806 of yacc.c  */
#line 1328 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 215:

/* Line 1806 of yacc.c  */
#line 1331 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 216:

/* Line 1806 of yacc.c  */
#line 1332 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;}
    break;

  case 217:

/* Line 1806 of yacc.c  */
#line 1333 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;}
    break;

  case 218:

/* Line 1806 of yacc.c  */
#line 1337 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 219:

/* Line 1806 of yacc.c  */
#line 1339 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));}
    break;

  case 220:

/* Line 1806 of yacc.c  */
#line 1340 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);}
    break;

  case 221:

/* Line 1806 of yacc.c  */
#line 1341 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 222:

/* Line 1806 of yacc.c  */
#line 1346 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 223:

/* Line 1806 of yacc.c  */
#line 1347 "hphp.y"
    { (yyval).reset();}
    break;

  case 224:

/* Line 1806 of yacc.c  */
#line 1350 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));}
    break;

  case 225:

/* Line 1806 of yacc.c  */
#line 1351 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 226:

/* Line 1806 of yacc.c  */
#line 1354 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));}
    break;

  case 227:

/* Line 1806 of yacc.c  */
#line 1355 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));}
    break;

  case 228:

/* Line 1806 of yacc.c  */
#line 1357 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);}
    break;

  case 229:

/* Line 1806 of yacc.c  */
#line 1361 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();}
    break;

  case 230:

/* Line 1806 of yacc.c  */
#line 1367 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 231:

/* Line 1806 of yacc.c  */
#line 1374 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();}
    break;

  case 232:

/* Line 1806 of yacc.c  */
#line 1380 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 233:

/* Line 1806 of yacc.c  */
#line 1385 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));}
    break;

  case 234:

/* Line 1806 of yacc.c  */
#line 1387 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));}
    break;

  case 235:

/* Line 1806 of yacc.c  */
#line 1389 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));}
    break;

  case 236:

/* Line 1806 of yacc.c  */
#line 1391 "hphp.y"
    { _p->onTraitRequire((yyval), (yyvsp[(3) - (4)]), true); }
    break;

  case 237:

/* Line 1806 of yacc.c  */
#line 1393 "hphp.y"
    { _p->onTraitRequire((yyval), (yyvsp[(3) - (4)]), false); }
    break;

  case 238:

/* Line 1806 of yacc.c  */
#line 1394 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); }
    break;

  case 239:

/* Line 1806 of yacc.c  */
#line 1397 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); }
    break;

  case 240:

/* Line 1806 of yacc.c  */
#line 1400 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); }
    break;

  case 241:

/* Line 1806 of yacc.c  */
#line 1401 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); }
    break;

  case 242:

/* Line 1806 of yacc.c  */
#line 1402 "hphp.y"
    { (yyval).reset(); }
    break;

  case 243:

/* Line 1806 of yacc.c  */
#line 1408 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 244:

/* Line 1806 of yacc.c  */
#line 1412 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));}
    break;

  case 245:

/* Line 1806 of yacc.c  */
#line 1415 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);}
    break;

  case 246:

/* Line 1806 of yacc.c  */
#line 1422 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 247:

/* Line 1806 of yacc.c  */
#line 1423 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));}
    break;

  case 248:

/* Line 1806 of yacc.c  */
#line 1428 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));}
    break;

  case 249:

/* Line 1806 of yacc.c  */
#line 1431 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 250:

/* Line 1806 of yacc.c  */
#line 1438 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;}
    break;

  case 251:

/* Line 1806 of yacc.c  */
#line 1440 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;}
    break;

  case 252:

/* Line 1806 of yacc.c  */
#line 1444 "hphp.y"
    { (yyval) = 4;}
    break;

  case 253:

/* Line 1806 of yacc.c  */
#line 1445 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));}
    break;

  case 254:

/* Line 1806 of yacc.c  */
#line 1451 "hphp.y"
    { (yyval) = 6;}
    break;

  case 255:

/* Line 1806 of yacc.c  */
#line 1453 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;}
    break;

  case 256:

/* Line 1806 of yacc.c  */
#line 1457 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);}
    break;

  case 257:

/* Line 1806 of yacc.c  */
#line 1459 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);}
    break;

  case 258:

/* Line 1806 of yacc.c  */
#line 1463 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 259:

/* Line 1806 of yacc.c  */
#line 1464 "hphp.y"
    { scalar_null(_p, (yyval));}
    break;

  case 260:

/* Line 1806 of yacc.c  */
#line 1468 "hphp.y"
    { scalar_num(_p, (yyval), "1");}
    break;

  case 261:

/* Line 1806 of yacc.c  */
#line 1469 "hphp.y"
    { scalar_num(_p, (yyval), "0");}
    break;

  case 262:

/* Line 1806 of yacc.c  */
#line 1473 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);}
    break;

  case 263:

/* Line 1806 of yacc.c  */
#line 1476 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);}
    break;

  case 264:

/* Line 1806 of yacc.c  */
#line 1481 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 265:

/* Line 1806 of yacc.c  */
#line 1486 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;}
    break;

  case 266:

/* Line 1806 of yacc.c  */
#line 1487 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;}
    break;

  case 267:

/* Line 1806 of yacc.c  */
#line 1489 "hphp.y"
    { (yyval) = 0;}
    break;

  case 268:

/* Line 1806 of yacc.c  */
#line 1493 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);}
    break;

  case 269:

/* Line 1806 of yacc.c  */
#line 1494 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);}
    break;

  case 270:

/* Line 1806 of yacc.c  */
#line 1495 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);}
    break;

  case 271:

/* Line 1806 of yacc.c  */
#line 1496 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);}
    break;

  case 272:

/* Line 1806 of yacc.c  */
#line 1500 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 273:

/* Line 1806 of yacc.c  */
#line 1501 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);}
    break;

  case 274:

/* Line 1806 of yacc.c  */
#line 1502 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);}
    break;

  case 275:

/* Line 1806 of yacc.c  */
#line 1503 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);}
    break;

  case 276:

/* Line 1806 of yacc.c  */
#line 1504 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);}
    break;

  case 277:

/* Line 1806 of yacc.c  */
#line 1506 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));}
    break;

  case 278:

/* Line 1806 of yacc.c  */
#line 1508 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));}
    break;

  case 279:

/* Line 1806 of yacc.c  */
#line 1512 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;}
    break;

  case 280:

/* Line 1806 of yacc.c  */
#line 1515 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;}
    break;

  case 281:

/* Line 1806 of yacc.c  */
#line 1516 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;}
    break;

  case 282:

/* Line 1806 of yacc.c  */
#line 1520 "hphp.y"
    { (yyval).reset();}
    break;

  case 283:

/* Line 1806 of yacc.c  */
#line 1521 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;}
    break;

  case 284:

/* Line 1806 of yacc.c  */
#line 1525 "hphp.y"
    { (yyval).reset();}
    break;

  case 285:

/* Line 1806 of yacc.c  */
#line 1526 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;}
    break;

  case 286:

/* Line 1806 of yacc.c  */
#line 1529 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 287:

/* Line 1806 of yacc.c  */
#line 1530 "hphp.y"
    { (yyval).reset();}
    break;

  case 288:

/* Line 1806 of yacc.c  */
#line 1533 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 289:

/* Line 1806 of yacc.c  */
#line 1534 "hphp.y"
    { (yyval).reset();}
    break;

  case 290:

/* Line 1806 of yacc.c  */
#line 1537 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));}
    break;

  case 291:

/* Line 1806 of yacc.c  */
#line 1539 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 292:

/* Line 1806 of yacc.c  */
#line 1542 "hphp.y"
    { (yyval) = T_PUBLIC;}
    break;

  case 293:

/* Line 1806 of yacc.c  */
#line 1543 "hphp.y"
    { (yyval) = T_PROTECTED;}
    break;

  case 294:

/* Line 1806 of yacc.c  */
#line 1544 "hphp.y"
    { (yyval) = T_PRIVATE;}
    break;

  case 295:

/* Line 1806 of yacc.c  */
#line 1545 "hphp.y"
    { (yyval) = T_STATIC;}
    break;

  case 296:

/* Line 1806 of yacc.c  */
#line 1546 "hphp.y"
    { (yyval) = T_ABSTRACT;}
    break;

  case 297:

/* Line 1806 of yacc.c  */
#line 1547 "hphp.y"
    { (yyval) = T_FINAL;}
    break;

  case 298:

/* Line 1806 of yacc.c  */
#line 1548 "hphp.y"
    { (yyval) = T_ASYNC;}
    break;

  case 299:

/* Line 1806 of yacc.c  */
#line 1552 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 300:

/* Line 1806 of yacc.c  */
#line 1553 "hphp.y"
    { (yyval).reset();}
    break;

  case 301:

/* Line 1806 of yacc.c  */
#line 1556 "hphp.y"
    { (yyval) = T_PUBLIC;}
    break;

  case 302:

/* Line 1806 of yacc.c  */
#line 1557 "hphp.y"
    { (yyval) = T_PROTECTED;}
    break;

  case 303:

/* Line 1806 of yacc.c  */
#line 1558 "hphp.y"
    { (yyval) = T_PRIVATE;}
    break;

  case 304:

/* Line 1806 of yacc.c  */
#line 1562 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 305:

/* Line 1806 of yacc.c  */
#line 1564 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));}
    break;

  case 306:

/* Line 1806 of yacc.c  */
#line 1565 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);}
    break;

  case 307:

/* Line 1806 of yacc.c  */
#line 1566 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 308:

/* Line 1806 of yacc.c  */
#line 1570 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 309:

/* Line 1806 of yacc.c  */
#line 1571 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 310:

/* Line 1806 of yacc.c  */
#line 1575 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 311:

/* Line 1806 of yacc.c  */
#line 1577 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 312:

/* Line 1806 of yacc.c  */
#line 1578 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);}
    break;

  case 313:

/* Line 1806 of yacc.c  */
#line 1579 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 314:

/* Line 1806 of yacc.c  */
#line 1580 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 315:

/* Line 1806 of yacc.c  */
#line 1583 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 316:

/* Line 1806 of yacc.c  */
#line 1587 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 317:

/* Line 1806 of yacc.c  */
#line 1588 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 318:

/* Line 1806 of yacc.c  */
#line 1592 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 319:

/* Line 1806 of yacc.c  */
#line 1593 "hphp.y"
    { (yyval).reset();}
    break;

  case 320:

/* Line 1806 of yacc.c  */
#line 1597 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));}
    break;

  case 321:

/* Line 1806 of yacc.c  */
#line 1598 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 322:

/* Line 1806 of yacc.c  */
#line 1602 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 323:

/* Line 1806 of yacc.c  */
#line 1607 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);}
    break;

  case 324:

/* Line 1806 of yacc.c  */
#line 1611 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 325:

/* Line 1806 of yacc.c  */
#line 1615 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 326:

/* Line 1806 of yacc.c  */
#line 1620 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);}
    break;

  case 327:

/* Line 1806 of yacc.c  */
#line 1624 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 328:

/* Line 1806 of yacc.c  */
#line 1625 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 329:

/* Line 1806 of yacc.c  */
#line 1626 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 330:

/* Line 1806 of yacc.c  */
#line 1630 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));}
    break;

  case 331:

/* Line 1806 of yacc.c  */
#line 1631 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 332:

/* Line 1806 of yacc.c  */
#line 1632 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);}
    break;

  case 333:

/* Line 1806 of yacc.c  */
#line 1635 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));}
    break;

  case 334:

/* Line 1806 of yacc.c  */
#line 1636 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);}
    break;

  case 335:

/* Line 1806 of yacc.c  */
#line 1637 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);}
    break;

  case 336:

/* Line 1806 of yacc.c  */
#line 1638 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);}
    break;

  case 337:

/* Line 1806 of yacc.c  */
#line 1639 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);}
    break;

  case 338:

/* Line 1806 of yacc.c  */
#line 1640 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);}
    break;

  case 339:

/* Line 1806 of yacc.c  */
#line 1641 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);}
    break;

  case 340:

/* Line 1806 of yacc.c  */
#line 1642 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);}
    break;

  case 341:

/* Line 1806 of yacc.c  */
#line 1643 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);}
    break;

  case 342:

/* Line 1806 of yacc.c  */
#line 1644 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);}
    break;

  case 343:

/* Line 1806 of yacc.c  */
#line 1645 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);}
    break;

  case 344:

/* Line 1806 of yacc.c  */
#line 1646 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);}
    break;

  case 345:

/* Line 1806 of yacc.c  */
#line 1647 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);}
    break;

  case 346:

/* Line 1806 of yacc.c  */
#line 1648 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);}
    break;

  case 347:

/* Line 1806 of yacc.c  */
#line 1649 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);}
    break;

  case 348:

/* Line 1806 of yacc.c  */
#line 1650 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);}
    break;

  case 349:

/* Line 1806 of yacc.c  */
#line 1651 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);}
    break;

  case 350:

/* Line 1806 of yacc.c  */
#line 1652 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);}
    break;

  case 351:

/* Line 1806 of yacc.c  */
#line 1653 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);}
    break;

  case 352:

/* Line 1806 of yacc.c  */
#line 1654 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);}
    break;

  case 353:

/* Line 1806 of yacc.c  */
#line 1655 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);}
    break;

  case 354:

/* Line 1806 of yacc.c  */
#line 1656 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');}
    break;

  case 355:

/* Line 1806 of yacc.c  */
#line 1657 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');}
    break;

  case 356:

/* Line 1806 of yacc.c  */
#line 1658 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');}
    break;

  case 357:

/* Line 1806 of yacc.c  */
#line 1659 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');}
    break;

  case 358:

/* Line 1806 of yacc.c  */
#line 1660 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');}
    break;

  case 359:

/* Line 1806 of yacc.c  */
#line 1661 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');}
    break;

  case 360:

/* Line 1806 of yacc.c  */
#line 1662 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');}
    break;

  case 361:

/* Line 1806 of yacc.c  */
#line 1663 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');}
    break;

  case 362:

/* Line 1806 of yacc.c  */
#line 1664 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');}
    break;

  case 363:

/* Line 1806 of yacc.c  */
#line 1665 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);}
    break;

  case 364:

/* Line 1806 of yacc.c  */
#line 1666 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);}
    break;

  case 365:

/* Line 1806 of yacc.c  */
#line 1667 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 366:

/* Line 1806 of yacc.c  */
#line 1668 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 367:

/* Line 1806 of yacc.c  */
#line 1669 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);}
    break;

  case 368:

/* Line 1806 of yacc.c  */
#line 1670 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);}
    break;

  case 369:

/* Line 1806 of yacc.c  */
#line 1671 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);}
    break;

  case 370:

/* Line 1806 of yacc.c  */
#line 1672 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);}
    break;

  case 371:

/* Line 1806 of yacc.c  */
#line 1673 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);}
    break;

  case 372:

/* Line 1806 of yacc.c  */
#line 1674 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);}
    break;

  case 373:

/* Line 1806 of yacc.c  */
#line 1675 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');}
    break;

  case 374:

/* Line 1806 of yacc.c  */
#line 1676 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);}
    break;

  case 375:

/* Line 1806 of yacc.c  */
#line 1678 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');}
    break;

  case 376:

/* Line 1806 of yacc.c  */
#line 1679 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);}
    break;

  case 377:

/* Line 1806 of yacc.c  */
#line 1682 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);}
    break;

  case 378:

/* Line 1806 of yacc.c  */
#line 1683 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 379:

/* Line 1806 of yacc.c  */
#line 1684 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));}
    break;

  case 380:

/* Line 1806 of yacc.c  */
#line 1685 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));}
    break;

  case 381:

/* Line 1806 of yacc.c  */
#line 1686 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 382:

/* Line 1806 of yacc.c  */
#line 1687 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);}
    break;

  case 383:

/* Line 1806 of yacc.c  */
#line 1688 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);}
    break;

  case 384:

/* Line 1806 of yacc.c  */
#line 1689 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);}
    break;

  case 385:

/* Line 1806 of yacc.c  */
#line 1690 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);}
    break;

  case 386:

/* Line 1806 of yacc.c  */
#line 1691 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);}
    break;

  case 387:

/* Line 1806 of yacc.c  */
#line 1692 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);}
    break;

  case 388:

/* Line 1806 of yacc.c  */
#line 1693 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);}
    break;

  case 389:

/* Line 1806 of yacc.c  */
#line 1694 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);}
    break;

  case 390:

/* Line 1806 of yacc.c  */
#line 1695 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);}
    break;

  case 391:

/* Line 1806 of yacc.c  */
#line 1696 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 392:

/* Line 1806 of yacc.c  */
#line 1697 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 393:

/* Line 1806 of yacc.c  */
#line 1698 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 394:

/* Line 1806 of yacc.c  */
#line 1699 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));}
    break;

  case 395:

/* Line 1806 of yacc.c  */
#line 1700 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);}
    break;

  case 396:

/* Line 1806 of yacc.c  */
#line 1701 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 397:

/* Line 1806 of yacc.c  */
#line 1702 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 398:

/* Line 1806 of yacc.c  */
#line 1703 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 399:

/* Line 1806 of yacc.c  */
#line 1710 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);}
    break;

  case 400:

/* Line 1806 of yacc.c  */
#line 1711 "hphp.y"
    { (yyval).reset();}
    break;

  case 401:

/* Line 1806 of yacc.c  */
#line 1716 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
    break;

  case 402:

/* Line 1806 of yacc.c  */
#line 1722 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 403:

/* Line 1806 of yacc.c  */
#line 1730 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
    break;

  case 404:

/* Line 1806 of yacc.c  */
#line 1736 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 405:

/* Line 1806 of yacc.c  */
#line 1745 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam((yyvsp[(1) - (1)]),NULL,u,(yyvsp[(1) - (1)]),0,
                                                     NULL,NULL,NULL);}
    break;

  case 406:

/* Line 1806 of yacc.c  */
#line 1753 "hphp.y"
    { Token v; Token w;
                                         _p->finishStatement((yyvsp[(3) - (3)]), (yyvsp[(3) - (3)])); (yyvsp[(3) - (3)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,(yyvsp[(1) - (3)]),w,(yyvsp[(3) - (3)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 407:

/* Line 1806 of yacc.c  */
#line 1760 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
    break;

  case 408:

/* Line 1806 of yacc.c  */
#line 1768 "hphp.y"
    { Token u; Token v;
                                         _p->finishStatement((yyvsp[(6) - (6)]), (yyvsp[(6) - (6)])); (yyvsp[(6) - (6)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,(yyvsp[(3) - (6)]),v,(yyvsp[(6) - (6)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 409:

/* Line 1806 of yacc.c  */
#line 1778 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));}
    break;

  case 410:

/* Line 1806 of yacc.c  */
#line 1780 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 411:

/* Line 1806 of yacc.c  */
#line 1784 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); }
    break;

  case 412:

/* Line 1806 of yacc.c  */
#line 1792 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 413:

/* Line 1806 of yacc.c  */
#line 1795 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 414:

/* Line 1806 of yacc.c  */
#line 1802 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 415:

/* Line 1806 of yacc.c  */
#line 1805 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 416:

/* Line 1806 of yacc.c  */
#line 1810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 417:

/* Line 1806 of yacc.c  */
#line 1811 "hphp.y"
    { (yyval).reset(); }
    break;

  case 418:

/* Line 1806 of yacc.c  */
#line 1816 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 419:

/* Line 1806 of yacc.c  */
#line 1817 "hphp.y"
    { (yyval).reset(); }
    break;

  case 420:

/* Line 1806 of yacc.c  */
#line 1821 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);}
    break;

  case 421:

/* Line 1806 of yacc.c  */
#line 1825 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 422:

/* Line 1806 of yacc.c  */
#line 1826 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 423:

/* Line 1806 of yacc.c  */
#line 1831 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);}
    break;

  case 424:

/* Line 1806 of yacc.c  */
#line 1838 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);}
    break;

  case 425:

/* Line 1806 of yacc.c  */
#line 1845 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 426:

/* Line 1806 of yacc.c  */
#line 1847 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 427:

/* Line 1806 of yacc.c  */
#line 1851 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 428:

/* Line 1806 of yacc.c  */
#line 1852 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 429:

/* Line 1806 of yacc.c  */
#line 1853 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 430:

/* Line 1806 of yacc.c  */
#line 1857 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); }
    break;

  case 431:

/* Line 1806 of yacc.c  */
#line 1861 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 432:

/* Line 1806 of yacc.c  */
#line 1865 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 433:

/* Line 1806 of yacc.c  */
#line 1870 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); }
    break;

  case 434:

/* Line 1806 of yacc.c  */
#line 1872 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); }
    break;

  case 435:

/* Line 1806 of yacc.c  */
#line 1874 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); }
    break;

  case 436:

/* Line 1806 of yacc.c  */
#line 1876 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); }
    break;

  case 437:

/* Line 1806 of yacc.c  */
#line 1880 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); }
    break;

  case 438:

/* Line 1806 of yacc.c  */
#line 1882 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); }
    break;

  case 439:

/* Line 1806 of yacc.c  */
#line 1886 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 440:

/* Line 1806 of yacc.c  */
#line 1887 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 441:

/* Line 1806 of yacc.c  */
#line 1888 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 442:

/* Line 1806 of yacc.c  */
#line 1889 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 443:

/* Line 1806 of yacc.c  */
#line 1890 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 444:

/* Line 1806 of yacc.c  */
#line 1891 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 445:

/* Line 1806 of yacc.c  */
#line 1895 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 446:

/* Line 1806 of yacc.c  */
#line 1899 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 447:

/* Line 1806 of yacc.c  */
#line 1903 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 448:

/* Line 1806 of yacc.c  */
#line 1908 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); }
    break;

  case 449:

/* Line 1806 of yacc.c  */
#line 1913 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); }
    break;

  case 450:

/* Line 1806 of yacc.c  */
#line 1917 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 451:

/* Line 1806 of yacc.c  */
#line 1921 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); }
    break;

  case 452:

/* Line 1806 of yacc.c  */
#line 1922 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); }
    break;

  case 453:

/* Line 1806 of yacc.c  */
#line 1926 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); }
    break;

  case 454:

/* Line 1806 of yacc.c  */
#line 1927 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); }
    break;

  case 455:

/* Line 1806 of yacc.c  */
#line 1931 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 456:

/* Line 1806 of yacc.c  */
#line 1932 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 457:

/* Line 1806 of yacc.c  */
#line 1936 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 458:

/* Line 1806 of yacc.c  */
#line 1937 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 459:

/* Line 1806 of yacc.c  */
#line 1941 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 460:

/* Line 1806 of yacc.c  */
#line 1945 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 461:

/* Line 1806 of yacc.c  */
#line 1949 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); }
    break;

  case 462:

/* Line 1806 of yacc.c  */
#line 1953 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 463:

/* Line 1806 of yacc.c  */
#line 1954 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 464:

/* Line 1806 of yacc.c  */
#line 1955 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 465:

/* Line 1806 of yacc.c  */
#line 1956 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);}
    break;

  case 466:

/* Line 1806 of yacc.c  */
#line 1963 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));}
    break;

  case 467:

/* Line 1806 of yacc.c  */
#line 1966 "hphp.y"
    { Token t1; _p->onArray(t1,(yyvsp[(1) - (2)]));
                                         Token t2; _p->onArray(t2,(yyvsp[(2) - (2)]));
                                         Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onCallParam((yyvsp[(1) - (2)]),NULL,t1,0);
                                         _p->onCallParam((yyval), &(yyvsp[(1) - (2)]),t2,0);
                                         _p->onCallParam((yyvsp[(1) - (2)]), &(yyvsp[(1) - (2)]),file,0);
                                         _p->onCallParam((yyvsp[(1) - (2)]), &(yyvsp[(1) - (2)]),line,0);
                                         (yyval).setText("");}
    break;

  case 468:

/* Line 1806 of yacc.c  */
#line 1977 "hphp.y"
    { Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onArray((yyvsp[(4) - (6)]),(yyvsp[(1) - (6)]));
                                         _p->onArray((yyvsp[(5) - (6)]),(yyvsp[(3) - (6)]));
                                         _p->onCallParam((yyvsp[(2) - (6)]),NULL,(yyvsp[(4) - (6)]),0);
                                         _p->onCallParam((yyval), &(yyvsp[(2) - (6)]),(yyvsp[(5) - (6)]),0);
                                         _p->onCallParam((yyvsp[(2) - (6)]), &(yyvsp[(2) - (6)]),file,0);
                                         _p->onCallParam((yyvsp[(2) - (6)]), &(yyvsp[(2) - (6)]),line,0);
                                         (yyval).setText((yyvsp[(6) - (6)]).text());}
    break;

  case 469:

/* Line 1806 of yacc.c  */
#line 1988 "hphp.y"
    { (yyval).reset(); (yyval).setText("");}
    break;

  case 470:

/* Line 1806 of yacc.c  */
#line 1989 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));}
    break;

  case 471:

/* Line 1806 of yacc.c  */
#line 1994 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);}
    break;

  case 472:

/* Line 1806 of yacc.c  */
#line 1995 "hphp.y"
    { (yyval).reset();}
    break;

  case 473:

/* Line 1806 of yacc.c  */
#line 1998 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);}
    break;

  case 474:

/* Line 1806 of yacc.c  */
#line 1999 "hphp.y"
    { (yyval).reset();}
    break;

  case 475:

/* Line 1806 of yacc.c  */
#line 2002 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 476:

/* Line 1806 of yacc.c  */
#line 2006 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 477:

/* Line 1806 of yacc.c  */
#line 2009 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 478:

/* Line 1806 of yacc.c  */
#line 2012 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       }
    break;

  case 479:

/* Line 1806 of yacc.c  */
#line 2019 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 480:

/* Line 1806 of yacc.c  */
#line 2020 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 481:

/* Line 1806 of yacc.c  */
#line 2024 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 482:

/* Line 1806 of yacc.c  */
#line 2026 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);}
    break;

  case 483:

/* Line 1806 of yacc.c  */
#line 2028 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);}
    break;

  case 484:

/* Line 1806 of yacc.c  */
#line 2031 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 485:

/* Line 1806 of yacc.c  */
#line 2032 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 486:

/* Line 1806 of yacc.c  */
#line 2033 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 487:

/* Line 1806 of yacc.c  */
#line 2034 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 488:

/* Line 1806 of yacc.c  */
#line 2035 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 489:

/* Line 1806 of yacc.c  */
#line 2036 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 490:

/* Line 1806 of yacc.c  */
#line 2037 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 491:

/* Line 1806 of yacc.c  */
#line 2038 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 492:

/* Line 1806 of yacc.c  */
#line 2039 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 493:

/* Line 1806 of yacc.c  */
#line 2040 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 494:

/* Line 1806 of yacc.c  */
#line 2041 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 495:

/* Line 1806 of yacc.c  */
#line 2042 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 496:

/* Line 1806 of yacc.c  */
#line 2043 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 497:

/* Line 1806 of yacc.c  */
#line 2044 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 498:

/* Line 1806 of yacc.c  */
#line 2045 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 499:

/* Line 1806 of yacc.c  */
#line 2046 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 500:

/* Line 1806 of yacc.c  */
#line 2047 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 501:

/* Line 1806 of yacc.c  */
#line 2048 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 502:

/* Line 1806 of yacc.c  */
#line 2049 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 503:

/* Line 1806 of yacc.c  */
#line 2050 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 504:

/* Line 1806 of yacc.c  */
#line 2051 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 505:

/* Line 1806 of yacc.c  */
#line 2052 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 506:

/* Line 1806 of yacc.c  */
#line 2053 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 507:

/* Line 1806 of yacc.c  */
#line 2054 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 508:

/* Line 1806 of yacc.c  */
#line 2055 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 509:

/* Line 1806 of yacc.c  */
#line 2056 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 510:

/* Line 1806 of yacc.c  */
#line 2057 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 511:

/* Line 1806 of yacc.c  */
#line 2058 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 512:

/* Line 1806 of yacc.c  */
#line 2059 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 513:

/* Line 1806 of yacc.c  */
#line 2060 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 514:

/* Line 1806 of yacc.c  */
#line 2061 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 515:

/* Line 1806 of yacc.c  */
#line 2062 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 516:

/* Line 1806 of yacc.c  */
#line 2063 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 517:

/* Line 1806 of yacc.c  */
#line 2064 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 518:

/* Line 1806 of yacc.c  */
#line 2065 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 519:

/* Line 1806 of yacc.c  */
#line 2066 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 520:

/* Line 1806 of yacc.c  */
#line 2067 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 521:

/* Line 1806 of yacc.c  */
#line 2068 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 522:

/* Line 1806 of yacc.c  */
#line 2069 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 523:

/* Line 1806 of yacc.c  */
#line 2070 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 524:

/* Line 1806 of yacc.c  */
#line 2071 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 525:

/* Line 1806 of yacc.c  */
#line 2072 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 526:

/* Line 1806 of yacc.c  */
#line 2073 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 527:

/* Line 1806 of yacc.c  */
#line 2074 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 528:

/* Line 1806 of yacc.c  */
#line 2075 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 529:

/* Line 1806 of yacc.c  */
#line 2076 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 530:

/* Line 1806 of yacc.c  */
#line 2077 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 531:

/* Line 1806 of yacc.c  */
#line 2078 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 532:

/* Line 1806 of yacc.c  */
#line 2079 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 533:

/* Line 1806 of yacc.c  */
#line 2080 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 534:

/* Line 1806 of yacc.c  */
#line 2081 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 535:

/* Line 1806 of yacc.c  */
#line 2082 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 536:

/* Line 1806 of yacc.c  */
#line 2083 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 537:

/* Line 1806 of yacc.c  */
#line 2084 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 538:

/* Line 1806 of yacc.c  */
#line 2085 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 539:

/* Line 1806 of yacc.c  */
#line 2086 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 540:

/* Line 1806 of yacc.c  */
#line 2087 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 541:

/* Line 1806 of yacc.c  */
#line 2088 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 542:

/* Line 1806 of yacc.c  */
#line 2089 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 543:

/* Line 1806 of yacc.c  */
#line 2090 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 544:

/* Line 1806 of yacc.c  */
#line 2091 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 545:

/* Line 1806 of yacc.c  */
#line 2092 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 546:

/* Line 1806 of yacc.c  */
#line 2093 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 547:

/* Line 1806 of yacc.c  */
#line 2094 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 548:

/* Line 1806 of yacc.c  */
#line 2095 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 549:

/* Line 1806 of yacc.c  */
#line 2096 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 550:

/* Line 1806 of yacc.c  */
#line 2097 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 551:

/* Line 1806 of yacc.c  */
#line 2098 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 552:

/* Line 1806 of yacc.c  */
#line 2099 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 553:

/* Line 1806 of yacc.c  */
#line 2100 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 554:

/* Line 1806 of yacc.c  */
#line 2101 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 555:

/* Line 1806 of yacc.c  */
#line 2102 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 556:

/* Line 1806 of yacc.c  */
#line 2103 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 557:

/* Line 1806 of yacc.c  */
#line 2104 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 558:

/* Line 1806 of yacc.c  */
#line 2105 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 559:

/* Line 1806 of yacc.c  */
#line 2106 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 560:

/* Line 1806 of yacc.c  */
#line 2107 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 561:

/* Line 1806 of yacc.c  */
#line 2108 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 562:

/* Line 1806 of yacc.c  */
#line 2109 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 563:

/* Line 1806 of yacc.c  */
#line 2110 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 564:

/* Line 1806 of yacc.c  */
#line 2115 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 565:

/* Line 1806 of yacc.c  */
#line 2119 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 566:

/* Line 1806 of yacc.c  */
#line 2120 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 567:

/* Line 1806 of yacc.c  */
#line 2123 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);}
    break;

  case 568:

/* Line 1806 of yacc.c  */
#line 2124 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);}
    break;

  case 569:

/* Line 1806 of yacc.c  */
#line 2125 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);}
    break;

  case 570:

/* Line 1806 of yacc.c  */
#line 2129 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);}
    break;

  case 571:

/* Line 1806 of yacc.c  */
#line 2130 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);}
    break;

  case 572:

/* Line 1806 of yacc.c  */
#line 2131 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);}
    break;

  case 573:

/* Line 1806 of yacc.c  */
#line 2135 "hphp.y"
    { (yyval).reset();}
    break;

  case 574:

/* Line 1806 of yacc.c  */
#line 2136 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 575:

/* Line 1806 of yacc.c  */
#line 2137 "hphp.y"
    { (yyval).reset();}
    break;

  case 576:

/* Line 1806 of yacc.c  */
#line 2141 "hphp.y"
    { (yyval).reset();}
    break;

  case 577:

/* Line 1806 of yacc.c  */
#line 2142 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);}
    break;

  case 578:

/* Line 1806 of yacc.c  */
#line 2143 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 579:

/* Line 1806 of yacc.c  */
#line 2147 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 580:

/* Line 1806 of yacc.c  */
#line 2148 "hphp.y"
    { (yyval).reset();}
    break;

  case 581:

/* Line 1806 of yacc.c  */
#line 2152 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 582:

/* Line 1806 of yacc.c  */
#line 2153 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 583:

/* Line 1806 of yacc.c  */
#line 2154 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 584:

/* Line 1806 of yacc.c  */
#line 2155 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));}
    break;

  case 585:

/* Line 1806 of yacc.c  */
#line 2157 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));}
    break;

  case 586:

/* Line 1806 of yacc.c  */
#line 2158 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));}
    break;

  case 587:

/* Line 1806 of yacc.c  */
#line 2159 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));}
    break;

  case 588:

/* Line 1806 of yacc.c  */
#line 2160 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));}
    break;

  case 589:

/* Line 1806 of yacc.c  */
#line 2161 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));}
    break;

  case 590:

/* Line 1806 of yacc.c  */
#line 2162 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));}
    break;

  case 591:

/* Line 1806 of yacc.c  */
#line 2163 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));}
    break;

  case 592:

/* Line 1806 of yacc.c  */
#line 2164 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));}
    break;

  case 593:

/* Line 1806 of yacc.c  */
#line 2165 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));}
    break;

  case 594:

/* Line 1806 of yacc.c  */
#line 2168 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));}
    break;

  case 595:

/* Line 1806 of yacc.c  */
#line 2170 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
    break;

  case 596:

/* Line 1806 of yacc.c  */
#line 2174 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 597:

/* Line 1806 of yacc.c  */
#line 2175 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 598:

/* Line 1806 of yacc.c  */
#line 2176 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 599:

/* Line 1806 of yacc.c  */
#line 2177 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 600:

/* Line 1806 of yacc.c  */
#line 2179 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 601:

/* Line 1806 of yacc.c  */
#line 2181 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 602:

/* Line 1806 of yacc.c  */
#line 2182 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); }
    break;

  case 603:

/* Line 1806 of yacc.c  */
#line 2184 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 604:

/* Line 1806 of yacc.c  */
#line 2185 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 605:

/* Line 1806 of yacc.c  */
#line 2186 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 606:

/* Line 1806 of yacc.c  */
#line 2192 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 607:

/* Line 1806 of yacc.c  */
#line 2194 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 608:

/* Line 1806 of yacc.c  */
#line 2198 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 609:

/* Line 1806 of yacc.c  */
#line 2202 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 610:

/* Line 1806 of yacc.c  */
#line 2203 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 611:

/* Line 1806 of yacc.c  */
#line 2204 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 612:

/* Line 1806 of yacc.c  */
#line 2205 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 613:

/* Line 1806 of yacc.c  */
#line 2206 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));}
    break;

  case 614:

/* Line 1806 of yacc.c  */
#line 2207 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));}
    break;

  case 615:

/* Line 1806 of yacc.c  */
#line 2209 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));}
    break;

  case 616:

/* Line 1806 of yacc.c  */
#line 2214 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 617:

/* Line 1806 of yacc.c  */
#line 2215 "hphp.y"
    { (yyval).reset();}
    break;

  case 618:

/* Line 1806 of yacc.c  */
#line 2219 "hphp.y"
    { (yyval).reset();}
    break;

  case 619:

/* Line 1806 of yacc.c  */
#line 2220 "hphp.y"
    { (yyval).reset();}
    break;

  case 620:

/* Line 1806 of yacc.c  */
#line 2223 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();}
    break;

  case 621:

/* Line 1806 of yacc.c  */
#line 2224 "hphp.y"
    { (yyval).reset();}
    break;

  case 622:

/* Line 1806 of yacc.c  */
#line 2230 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 623:

/* Line 1806 of yacc.c  */
#line 2232 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 624:

/* Line 1806 of yacc.c  */
#line 2234 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 625:

/* Line 1806 of yacc.c  */
#line 2235 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 626:

/* Line 1806 of yacc.c  */
#line 2239 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 627:

/* Line 1806 of yacc.c  */
#line 2240 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 628:

/* Line 1806 of yacc.c  */
#line 2241 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 629:

/* Line 1806 of yacc.c  */
#line 2242 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));}
    break;

  case 630:

/* Line 1806 of yacc.c  */
#line 2246 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));}
    break;

  case 631:

/* Line 1806 of yacc.c  */
#line 2248 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
    break;

  case 632:

/* Line 1806 of yacc.c  */
#line 2251 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));}
    break;

  case 633:

/* Line 1806 of yacc.c  */
#line 2252 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));}
    break;

  case 634:

/* Line 1806 of yacc.c  */
#line 2253 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));}
    break;

  case 635:

/* Line 1806 of yacc.c  */
#line 2254 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));}
    break;

  case 636:

/* Line 1806 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 637:

/* Line 1806 of yacc.c  */
#line 2258 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));}
    break;

  case 638:

/* Line 1806 of yacc.c  */
#line 2259 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 639:

/* Line 1806 of yacc.c  */
#line 2260 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 640:

/* Line 1806 of yacc.c  */
#line 2262 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 641:

/* Line 1806 of yacc.c  */
#line 2264 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 642:

/* Line 1806 of yacc.c  */
#line 2265 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 643:

/* Line 1806 of yacc.c  */
#line 2267 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 644:

/* Line 1806 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 645:

/* Line 1806 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval).reset();}
    break;

  case 646:

/* Line 1806 of yacc.c  */
#line 2278 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 647:

/* Line 1806 of yacc.c  */
#line 2280 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 648:

/* Line 1806 of yacc.c  */
#line 2282 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 649:

/* Line 1806 of yacc.c  */
#line 2283 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 650:

/* Line 1806 of yacc.c  */
#line 2287 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 651:

/* Line 1806 of yacc.c  */
#line 2288 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 652:

/* Line 1806 of yacc.c  */
#line 2293 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 653:

/* Line 1806 of yacc.c  */
#line 2294 "hphp.y"
    { (yyval).reset(); }
    break;

  case 654:

/* Line 1806 of yacc.c  */
#line 2299 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 655:

/* Line 1806 of yacc.c  */
#line 2302 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 656:

/* Line 1806 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 657:

/* Line 1806 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval).reset();}
    break;

  case 658:

/* Line 1806 of yacc.c  */
#line 2311 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 659:

/* Line 1806 of yacc.c  */
#line 2312 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
    break;

  case 660:

/* Line 1806 of yacc.c  */
#line 2319 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 661:

/* Line 1806 of yacc.c  */
#line 2321 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 662:

/* Line 1806 of yacc.c  */
#line 2324 "hphp.y"
    { only_in_hh_syntax(_p);}
    break;

  case 663:

/* Line 1806 of yacc.c  */
#line 2326 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 664:

/* Line 1806 of yacc.c  */
#line 2329 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 665:

/* Line 1806 of yacc.c  */
#line 2332 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 666:

/* Line 1806 of yacc.c  */
#line 2333 "hphp.y"
    { (yyval).reset();}
    break;

  case 667:

/* Line 1806 of yacc.c  */
#line 2337 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 668:

/* Line 1806 of yacc.c  */
#line 2339 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 669:

/* Line 1806 of yacc.c  */
#line 2343 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 670:

/* Line 1806 of yacc.c  */
#line 2344 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 671:

/* Line 1806 of yacc.c  */
#line 2348 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 672:

/* Line 1806 of yacc.c  */
#line 2349 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 673:

/* Line 1806 of yacc.c  */
#line 2353 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 674:

/* Line 1806 of yacc.c  */
#line 2355 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 675:

/* Line 1806 of yacc.c  */
#line 2360 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 676:

/* Line 1806 of yacc.c  */
#line 2362 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 677:

/* Line 1806 of yacc.c  */
#line 2366 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 678:

/* Line 1806 of yacc.c  */
#line 2367 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 679:

/* Line 1806 of yacc.c  */
#line 2368 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 680:

/* Line 1806 of yacc.c  */
#line 2369 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 681:

/* Line 1806 of yacc.c  */
#line 2370 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 682:

/* Line 1806 of yacc.c  */
#line 2371 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 683:

/* Line 1806 of yacc.c  */
#line 2373 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 684:

/* Line 1806 of yacc.c  */
#line 2376 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 685:

/* Line 1806 of yacc.c  */
#line 2378 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 686:

/* Line 1806 of yacc.c  */
#line 2379 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 687:

/* Line 1806 of yacc.c  */
#line 2383 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 688:

/* Line 1806 of yacc.c  */
#line 2384 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 689:

/* Line 1806 of yacc.c  */
#line 2385 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 690:

/* Line 1806 of yacc.c  */
#line 2386 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 691:

/* Line 1806 of yacc.c  */
#line 2388 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 692:

/* Line 1806 of yacc.c  */
#line 2390 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 693:

/* Line 1806 of yacc.c  */
#line 2392 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 694:

/* Line 1806 of yacc.c  */
#line 2393 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 695:

/* Line 1806 of yacc.c  */
#line 2397 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 696:

/* Line 1806 of yacc.c  */
#line 2398 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 697:

/* Line 1806 of yacc.c  */
#line 2399 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 698:

/* Line 1806 of yacc.c  */
#line 2405 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));}
    break;

  case 699:

/* Line 1806 of yacc.c  */
#line 2408 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 700:

/* Line 1806 of yacc.c  */
#line 2411 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));}
    break;

  case 701:

/* Line 1806 of yacc.c  */
#line 2415 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));}
    break;

  case 702:

/* Line 1806 of yacc.c  */
#line 2419 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));}
    break;

  case 703:

/* Line 1806 of yacc.c  */
#line 2423 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));}
    break;

  case 704:

/* Line 1806 of yacc.c  */
#line 2430 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));}
    break;

  case 705:

/* Line 1806 of yacc.c  */
#line 2434 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));}
    break;

  case 706:

/* Line 1806 of yacc.c  */
#line 2438 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));}
    break;

  case 707:

/* Line 1806 of yacc.c  */
#line 2442 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 708:

/* Line 1806 of yacc.c  */
#line 2444 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 709:

/* Line 1806 of yacc.c  */
#line 2449 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 710:

/* Line 1806 of yacc.c  */
#line 2450 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 711:

/* Line 1806 of yacc.c  */
#line 2451 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 712:

/* Line 1806 of yacc.c  */
#line 2454 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 713:

/* Line 1806 of yacc.c  */
#line 2455 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);}
    break;

  case 714:

/* Line 1806 of yacc.c  */
#line 2458 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 715:

/* Line 1806 of yacc.c  */
#line 2459 "hphp.y"
    { (yyval).reset();}
    break;

  case 716:

/* Line 1806 of yacc.c  */
#line 2463 "hphp.y"
    { (yyval) = 1;}
    break;

  case 717:

/* Line 1806 of yacc.c  */
#line 2464 "hphp.y"
    { (yyval)++;}
    break;

  case 718:

/* Line 1806 of yacc.c  */
#line 2468 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 719:

/* Line 1806 of yacc.c  */
#line 2469 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 720:

/* Line 1806 of yacc.c  */
#line 2470 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 721:

/* Line 1806 of yacc.c  */
#line 2472 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 722:

/* Line 1806 of yacc.c  */
#line 2475 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 723:

/* Line 1806 of yacc.c  */
#line 2476 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 725:

/* Line 1806 of yacc.c  */
#line 2480 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 726:

/* Line 1806 of yacc.c  */
#line 2482 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 727:

/* Line 1806 of yacc.c  */
#line 2484 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 728:

/* Line 1806 of yacc.c  */
#line 2485 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 729:

/* Line 1806 of yacc.c  */
#line 2489 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);}
    break;

  case 730:

/* Line 1806 of yacc.c  */
#line 2490 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 731:

/* Line 1806 of yacc.c  */
#line 2492 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 732:

/* Line 1806 of yacc.c  */
#line 2493 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);}
    break;

  case 733:

/* Line 1806 of yacc.c  */
#line 2494 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));}
    break;

  case 734:

/* Line 1806 of yacc.c  */
#line 2495 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));}
    break;

  case 735:

/* Line 1806 of yacc.c  */
#line 2500 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 736:

/* Line 1806 of yacc.c  */
#line 2501 "hphp.y"
    { (yyval).reset();}
    break;

  case 737:

/* Line 1806 of yacc.c  */
#line 2505 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 738:

/* Line 1806 of yacc.c  */
#line 2506 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 739:

/* Line 1806 of yacc.c  */
#line 2507 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 740:

/* Line 1806 of yacc.c  */
#line 2508 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 741:

/* Line 1806 of yacc.c  */
#line 2511 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);}
    break;

  case 742:

/* Line 1806 of yacc.c  */
#line 2513 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);}
    break;

  case 743:

/* Line 1806 of yacc.c  */
#line 2514 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 744:

/* Line 1806 of yacc.c  */
#line 2515 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);}
    break;

  case 745:

/* Line 1806 of yacc.c  */
#line 2520 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 746:

/* Line 1806 of yacc.c  */
#line 2521 "hphp.y"
    { _p->onEmptyCollection((yyval));}
    break;

  case 747:

/* Line 1806 of yacc.c  */
#line 2525 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 748:

/* Line 1806 of yacc.c  */
#line 2526 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));}
    break;

  case 749:

/* Line 1806 of yacc.c  */
#line 2527 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 750:

/* Line 1806 of yacc.c  */
#line 2528 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));}
    break;

  case 751:

/* Line 1806 of yacc.c  */
#line 2533 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 752:

/* Line 1806 of yacc.c  */
#line 2534 "hphp.y"
    { _p->onEmptyCollection((yyval));}
    break;

  case 753:

/* Line 1806 of yacc.c  */
#line 2539 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 754:

/* Line 1806 of yacc.c  */
#line 2541 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));}
    break;

  case 755:

/* Line 1806 of yacc.c  */
#line 2543 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 756:

/* Line 1806 of yacc.c  */
#line 2544 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));}
    break;

  case 757:

/* Line 1806 of yacc.c  */
#line 2548 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);}
    break;

  case 758:

/* Line 1806 of yacc.c  */
#line 2550 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);}
    break;

  case 759:

/* Line 1806 of yacc.c  */
#line 2551 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);}
    break;

  case 760:

/* Line 1806 of yacc.c  */
#line 2553 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); }
    break;

  case 761:

/* Line 1806 of yacc.c  */
#line 2558 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 762:

/* Line 1806 of yacc.c  */
#line 2560 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 763:

/* Line 1806 of yacc.c  */
#line 2562 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 764:

/* Line 1806 of yacc.c  */
#line 2564 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);}
    break;

  case 765:

/* Line 1806 of yacc.c  */
#line 2566 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));}
    break;

  case 766:

/* Line 1806 of yacc.c  */
#line 2567 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 767:

/* Line 1806 of yacc.c  */
#line 2570 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;}
    break;

  case 768:

/* Line 1806 of yacc.c  */
#line 2571 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;}
    break;

  case 769:

/* Line 1806 of yacc.c  */
#line 2572 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;}
    break;

  case 770:

/* Line 1806 of yacc.c  */
#line 2576 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);}
    break;

  case 771:

/* Line 1806 of yacc.c  */
#line 2577 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);}
    break;

  case 772:

/* Line 1806 of yacc.c  */
#line 2578 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);}
    break;

  case 773:

/* Line 1806 of yacc.c  */
#line 2579 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);}
    break;

  case 774:

/* Line 1806 of yacc.c  */
#line 2580 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);}
    break;

  case 775:

/* Line 1806 of yacc.c  */
#line 2581 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);}
    break;

  case 776:

/* Line 1806 of yacc.c  */
#line 2582 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);}
    break;

  case 777:

/* Line 1806 of yacc.c  */
#line 2583 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);}
    break;

  case 778:

/* Line 1806 of yacc.c  */
#line 2584 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);}
    break;

  case 779:

/* Line 1806 of yacc.c  */
#line 2588 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 780:

/* Line 1806 of yacc.c  */
#line 2589 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 781:

/* Line 1806 of yacc.c  */
#line 2594 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 782:

/* Line 1806 of yacc.c  */
#line 2596 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 785:

/* Line 1806 of yacc.c  */
#line 2610 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); }
    break;

  case 786:

/* Line 1806 of yacc.c  */
#line 2614 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); }
    break;

  case 787:

/* Line 1806 of yacc.c  */
#line 2620 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 788:

/* Line 1806 of yacc.c  */
#line 2621 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 789:

/* Line 1806 of yacc.c  */
#line 2627 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 790:

/* Line 1806 of yacc.c  */
#line 2631 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); }
    break;

  case 791:

/* Line 1806 of yacc.c  */
#line 2637 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 792:

/* Line 1806 of yacc.c  */
#line 2638 "hphp.y"
    { (yyval).reset(); }
    break;

  case 793:

/* Line 1806 of yacc.c  */
#line 2642 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 794:

/* Line 1806 of yacc.c  */
#line 2645 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); }
    break;

  case 795:

/* Line 1806 of yacc.c  */
#line 2650 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); }
    break;

  case 796:

/* Line 1806 of yacc.c  */
#line 2651 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 797:

/* Line 1806 of yacc.c  */
#line 2652 "hphp.y"
    { (yyval).reset(); }
    break;

  case 798:

/* Line 1806 of yacc.c  */
#line 2653 "hphp.y"
    { (yyval).reset(); }
    break;

  case 799:

/* Line 1806 of yacc.c  */
#line 2657 "hphp.y"
    { (yyval).reset(); }
    break;

  case 800:

/* Line 1806 of yacc.c  */
#line 2658 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 801:

/* Line 1806 of yacc.c  */
#line 2663 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); }
    break;

  case 802:

/* Line 1806 of yacc.c  */
#line 2664 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); }
    break;

  case 803:

/* Line 1806 of yacc.c  */
#line 2666 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); }
    break;

  case 804:

/* Line 1806 of yacc.c  */
#line 2667 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); }
    break;

  case 805:

/* Line 1806 of yacc.c  */
#line 2673 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); }
    break;

  case 808:

/* Line 1806 of yacc.c  */
#line 2684 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 809:

/* Line 1806 of yacc.c  */
#line 2686 "hphp.y"
    {}
    break;

  case 810:

/* Line 1806 of yacc.c  */
#line 2690 "hphp.y"
    { (yyval).setText("array"); }
    break;

  case 811:

/* Line 1806 of yacc.c  */
#line 2697 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 812:

/* Line 1806 of yacc.c  */
#line 2700 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 813:

/* Line 1806 of yacc.c  */
#line 2703 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 814:

/* Line 1806 of yacc.c  */
#line 2704 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); }
    break;

  case 815:

/* Line 1806 of yacc.c  */
#line 2707 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); }
    break;

  case 816:

/* Line 1806 of yacc.c  */
#line 2710 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 817:

/* Line 1806 of yacc.c  */
#line 2712 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); }
    break;

  case 818:

/* Line 1806 of yacc.c  */
#line 2715 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); }
    break;

  case 819:

/* Line 1806 of yacc.c  */
#line 2718 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
    break;

  case 820:

/* Line 1806 of yacc.c  */
#line 2724 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
    break;

  case 821:

/* Line 1806 of yacc.c  */
#line 2728 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (5)]));
                                        _p->onTypeSpecialization((yyval), 't'); }
    break;

  case 822:

/* Line 1806 of yacc.c  */
#line 2736 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 823:

/* Line 1806 of yacc.c  */
#line 2737 "hphp.y"
    { (yyval).reset(); }
    break;



/* Line 1806 of yacc.c  */
#line 12293 "hphp.tab.cpp"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
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
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, _p, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (&yylloc, _p, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

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

  yyerror_range[1] = yylsp[1-yylen];
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
      if (!yypact_value_is_default (yyn))
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

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp, _p);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
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
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, _p);
    }
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



/* Line 2067 of yacc.c  */
#line 2740 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

