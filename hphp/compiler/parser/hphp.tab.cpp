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
   * The basic builtin types "bool", "int", "double", and "string" all map to
   * T_STRING in the parser, and the parser always uses type code 5 for
   * T_STRING. However, XHP uses different type codes for these basic builtin
   * types, so we need to fix up the type code here to make XHP happy.
   */
  if (type.num() == 5 && type.text().size() >= 3 && type.text().size() <= 7) {
    switch (type.text()[0]) {
      case 'b':
        if ((type.text().size() == 4 &&
             strcasecmp(type.text().c_str(), "bool") == 0) ||
            (type.text().size() == 7 &&
             strcasecmp(type.text().c_str(), "boolean") == 0)) {
          type.reset();
          type.setNum(2);
        }
        break;
      case 'd':
        if (type.text().size() == 6 &&
            strcasecmp(type.text().c_str(), "double") == 0) {
          type.reset();
          type.setNum(8);
        }
        break;
      case 'f':
        if (type.text().size() == 5 &&
            strcasecmp(type.text().c_str(), "float") == 0) {
          type.reset();
          type.setNum(8);
        }
        break;
      case 'i':
        if ((type.text().size() == 3 &&
             strcasecmp(type.text().c_str(), "int") == 0) ||
            (type.text().size() == 7 &&
             strcasecmp(type.text().c_str(), "integer") == 0)) {
          type.reset();
          type.setNum(3);
        }
        break;
      case 'm':
        if ((type.text().size() == 5 &&
             strcasecmp(type.text().c_str(), "mixed") == 0)) {
          type.reset();
          type.setNum(6);
        }
        break;
      case 'r':
        if (type.text().size() == 4 &&
            strcasecmp(type.text().c_str(), "real") == 0) {
          type.reset();
          type.setNum(8);
        }
        break;
      case 's':
        if (type.text().size() == 6 &&
            strcasecmp(type.text().c_str(), "string") == 0) {
          type.reset();
          type.setNum(1);
        }
        break;
      default:
        break;
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
#line 639 "hphp.tab.cpp"

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
#line 869 "hphp.tab.cpp"

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
#define YYLAST   14776

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  205
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  246
/* YYNRULES -- Number of rules.  */
#define YYNRULES  825
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1534

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
    1556,  1558,  1560,  1562,  1564,  1566,  1571,  1576,  1579,  1582,
    1587,  1596,  1607,  1610,  1612,  1616,  1618,  1621,  1623,  1625,
    1627,  1629,  1632,  1637,  1641,  1645,  1650,  1652,  1655,  1660,
    1663,  1670,  1671,  1673,  1678,  1679,  1682,  1683,  1685,  1687,
    1691,  1693,  1697,  1699,  1701,  1705,  1709,  1711,  1713,  1715,
    1717,  1719,  1721,  1723,  1725,  1727,  1729,  1731,  1733,  1735,
    1737,  1739,  1741,  1743,  1745,  1747,  1749,  1751,  1753,  1755,
    1757,  1759,  1761,  1763,  1765,  1767,  1769,  1771,  1773,  1775,
    1777,  1779,  1781,  1783,  1785,  1787,  1789,  1791,  1793,  1795,
    1797,  1799,  1801,  1803,  1805,  1807,  1809,  1811,  1813,  1815,
    1817,  1819,  1821,  1823,  1825,  1827,  1829,  1831,  1833,  1835,
    1837,  1839,  1841,  1843,  1845,  1847,  1849,  1851,  1853,  1855,
    1857,  1859,  1861,  1863,  1865,  1867,  1869,  1874,  1876,  1878,
    1880,  1882,  1884,  1886,  1888,  1890,  1893,  1895,  1896,  1897,
    1899,  1901,  1905,  1906,  1908,  1910,  1912,  1914,  1916,  1918,
    1920,  1922,  1924,  1926,  1928,  1930,  1932,  1936,  1939,  1941,
    1943,  1946,  1949,  1954,  1959,  1963,  1968,  1970,  1972,  1976,
    1980,  1984,  1986,  1988,  1990,  1992,  1996,  2000,  2004,  2007,
    2008,  2010,  2011,  2013,  2014,  2020,  2024,  2028,  2030,  2032,
    2034,  2036,  2038,  2042,  2045,  2047,  2049,  2051,  2053,  2055,
    2057,  2060,  2063,  2068,  2073,  2077,  2082,  2085,  2086,  2092,
    2096,  2100,  2102,  2106,  2108,  2111,  2112,  2118,  2122,  2125,
    2126,  2130,  2131,  2136,  2139,  2140,  2144,  2148,  2150,  2151,
    2153,  2156,  2159,  2164,  2168,  2172,  2175,  2180,  2183,  2188,
    2190,  2192,  2194,  2196,  2198,  2201,  2206,  2210,  2215,  2219,
    2221,  2223,  2225,  2227,  2230,  2235,  2240,  2244,  2246,  2248,
    2252,  2260,  2267,  2276,  2286,  2295,  2306,  2314,  2321,  2330,
    2332,  2335,  2340,  2345,  2347,  2349,  2354,  2356,  2357,  2359,
    2362,  2364,  2366,  2369,  2374,  2378,  2382,  2383,  2385,  2388,
    2393,  2397,  2400,  2404,  2411,  2412,  2414,  2419,  2422,  2423,
    2429,  2433,  2437,  2439,  2446,  2451,  2456,  2459,  2462,  2463,
    2469,  2473,  2477,  2479,  2482,  2483,  2489,  2493,  2497,  2499,
    2502,  2505,  2507,  2510,  2512,  2517,  2521,  2525,  2532,  2536,
    2538,  2540,  2542,  2547,  2552,  2557,  2562,  2565,  2568,  2573,
    2576,  2579,  2581,  2585,  2589,  2593,  2594,  2597,  2603,  2610,
    2612,  2615,  2617,  2622,  2626,  2627,  2629,  2633,  2637,  2639,
    2641,  2642,  2643,  2646,  2650,  2652,  2658,  2662,  2666,  2670,
    2672,  2675,  2676,  2681,  2684,  2687,  2689,  2691,  2693,  2695,
    2700,  2707,  2709,  2718,  2724,  2726
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
     325,    -1,   180,    75,   181,   325,    -1,   180,    75,   181,
     325,   182,   325,   183,   325,    -1,   180,    75,   181,   325,
     182,   325,   183,   325,   184,    75,    -1,   186,   358,    -1,
     359,    -1,   358,     9,   359,    -1,   325,    -1,   325,   360,
      -1,   187,    -1,   188,    -1,   362,    -1,   363,    -1,   189,
     325,    -1,   190,   325,   191,   325,    -1,   184,    75,   349,
      -1,   365,     9,    75,    -1,   365,     9,    32,    75,    -1,
      75,    -1,    32,    75,    -1,   163,   149,   367,   164,    -1,
     369,    47,    -1,   369,   164,   370,   163,    47,   368,    -1,
      -1,   149,    -1,   369,   371,    14,   372,    -1,    -1,   370,
     373,    -1,    -1,   149,    -1,   150,    -1,   198,   325,   199,
      -1,   150,    -1,   198,   325,   199,    -1,   366,    -1,   375,
      -1,   374,    27,   375,    -1,   374,    44,   375,    -1,   212,
      -1,    65,    -1,   100,    -1,   101,    -1,   102,    -1,   148,
      -1,   175,    -1,   103,    -1,   104,    -1,   162,    -1,   105,
      -1,    66,    -1,    67,    -1,    69,    -1,    68,    -1,    84,
      -1,    85,    -1,    83,    -1,    86,    -1,    87,    -1,    88,
      -1,    89,    -1,    90,    -1,    91,    -1,    50,    -1,    92,
      -1,    93,    -1,    94,    -1,    95,    -1,    96,    -1,    97,
      -1,    99,    -1,    98,    -1,    82,    -1,    13,    -1,   119,
      -1,   120,    -1,   121,    -1,   122,    -1,    64,    -1,    63,
      -1,   114,    -1,     5,    -1,     7,    -1,     6,    -1,     4,
      -1,     3,    -1,   144,    -1,   106,    -1,   107,    -1,   116,
      -1,   117,    -1,   118,    -1,   113,    -1,   112,    -1,   111,
      -1,   110,    -1,   109,    -1,   108,    -1,   176,    -1,   115,
      -1,   125,    -1,   126,    -1,    10,    -1,    12,    -1,    11,
      -1,   128,    -1,   130,    -1,   129,    -1,   131,    -1,   132,
      -1,   146,    -1,   145,    -1,   174,    -1,   157,    -1,   159,
      -1,   158,    -1,   170,    -1,   172,    -1,   169,    -1,   218,
     195,   278,   196,    -1,   219,    -1,   149,    -1,   377,    -1,
     113,    -1,   417,    -1,   377,    -1,   113,    -1,   421,    -1,
     195,   196,    -1,   316,    -1,    -1,    -1,    80,    -1,   430,
      -1,   195,   278,   196,    -1,    -1,    70,    -1,    71,    -1,
      72,    -1,    81,    -1,   131,    -1,   132,    -1,   146,    -1,
     128,    -1,   159,    -1,   129,    -1,   130,    -1,   145,    -1,
     174,    -1,   139,    80,   140,    -1,   139,   140,    -1,   383,
      -1,   217,    -1,    43,   384,    -1,    44,   384,    -1,   126,
     195,   387,   196,    -1,   177,   195,   387,   196,    -1,    62,
     387,   202,    -1,   169,   195,   339,   196,    -1,   385,    -1,
     343,    -1,   219,   143,   212,    -1,   149,   143,   212,    -1,
     219,   143,   119,    -1,   217,    -1,    74,    -1,   435,    -1,
     383,    -1,   203,   430,   203,    -1,   204,   430,   204,    -1,
     139,   430,   140,    -1,   390,   388,    -1,    -1,     9,    -1,
      -1,     9,    -1,    -1,   390,     9,   384,   124,   384,    -1,
     390,     9,   384,    -1,   384,   124,   384,    -1,   384,    -1,
      70,    -1,    71,    -1,    72,    -1,    81,    -1,   139,    80,
     140,    -1,   139,   140,    -1,    70,    -1,    71,    -1,    72,
      -1,   212,    -1,   391,    -1,   212,    -1,    43,   392,    -1,
      44,   392,    -1,   126,   195,   394,   196,    -1,   177,   195,
     394,   196,    -1,    62,   394,   202,    -1,   169,   195,   397,
     196,    -1,   395,   388,    -1,    -1,   395,     9,   393,   124,
     393,    -1,   395,     9,   393,    -1,   393,   124,   393,    -1,
     393,    -1,   396,     9,   393,    -1,   393,    -1,   398,   388,
      -1,    -1,   398,     9,   335,   124,   393,    -1,   335,   124,
     393,    -1,   396,   388,    -1,    -1,   195,   399,   196,    -1,
      -1,   401,     9,   212,   400,    -1,   212,   400,    -1,    -1,
     403,   401,   388,    -1,    42,   402,    41,    -1,   404,    -1,
      -1,   407,    -1,   123,   416,    -1,   123,   212,    -1,   123,
     198,   325,   199,    -1,    62,   419,   202,    -1,   198,   325,
     199,    -1,   412,   408,    -1,   195,   315,   196,   408,    -1,
     422,   408,    -1,   195,   315,   196,   408,    -1,   416,    -1,
     376,    -1,   414,    -1,   415,    -1,   409,    -1,   411,   406,
      -1,   195,   315,   196,   406,    -1,   378,   143,   416,    -1,
     413,   195,   278,   196,    -1,   195,   411,   196,    -1,   376,
      -1,   414,    -1,   415,    -1,   409,    -1,   411,   407,    -1,
     195,   315,   196,   407,    -1,   413,   195,   278,   196,    -1,
     195,   411,   196,    -1,   416,    -1,   409,    -1,   195,   411,
     196,    -1,   411,   123,   212,   440,   195,   278,   196,    -1,
     411,   123,   416,   195,   278,   196,    -1,   411,   123,   198,
     325,   199,   195,   278,   196,    -1,   195,   315,   196,   123,
     212,   440,   195,   278,   196,    -1,   195,   315,   196,   123,
     416,   195,   278,   196,    -1,   195,   315,   196,   123,   198,
     325,   199,   195,   278,   196,    -1,   378,   143,   212,   440,
     195,   278,   196,    -1,   378,   143,   416,   195,   278,   196,
      -1,   378,   143,   198,   325,   199,   195,   278,   196,    -1,
     417,    -1,   420,   417,    -1,   417,    62,   419,   202,    -1,
     417,   198,   325,   199,    -1,   418,    -1,    75,    -1,   200,
     198,   325,   199,    -1,   325,    -1,    -1,   200,    -1,   420,
     200,    -1,   416,    -1,   410,    -1,   421,   406,    -1,   195,
     315,   196,   406,    -1,   378,   143,   416,    -1,   195,   411,
     196,    -1,    -1,   410,    -1,   421,   407,    -1,   195,   315,
     196,   407,    -1,   195,   411,   196,    -1,   423,     9,    -1,
     423,     9,   411,    -1,   423,     9,   125,   195,   423,   196,
      -1,    -1,   411,    -1,   125,   195,   423,   196,    -1,   425,
     388,    -1,    -1,   425,     9,   325,   124,   325,    -1,   425,
       9,   325,    -1,   325,   124,   325,    -1,   325,    -1,   425,
       9,   325,   124,    32,   411,    -1,   425,     9,    32,   411,
      -1,   325,   124,    32,   411,    -1,    32,   411,    -1,   427,
     388,    -1,    -1,   427,     9,   325,   124,   325,    -1,   427,
       9,   325,    -1,   325,   124,   325,    -1,   325,    -1,   429,
     388,    -1,    -1,   429,     9,   384,   124,   384,    -1,   429,
       9,   384,    -1,   384,   124,   384,    -1,   384,    -1,   430,
     431,    -1,   430,    80,    -1,   431,    -1,    80,   431,    -1,
      75,    -1,    75,    62,   432,   202,    -1,    75,   123,   212,
      -1,   141,   325,   199,    -1,   141,    74,    62,   325,   202,
     199,    -1,   142,   411,   199,    -1,   212,    -1,    76,    -1,
      75,    -1,   116,   195,   434,   196,    -1,   117,   195,   411,
     196,    -1,   117,   195,   326,   196,    -1,   117,   195,   315,
     196,    -1,     7,   325,    -1,     6,   325,    -1,     5,   195,
     325,   196,    -1,     4,   325,    -1,     3,   325,    -1,   411,
      -1,   434,     9,   411,    -1,   378,   143,   212,    -1,   378,
     143,   119,    -1,    -1,    92,   449,    -1,   170,   439,    14,
     449,   197,    -1,   172,   439,   436,    14,   449,   197,    -1,
     212,    -1,   449,   212,    -1,   212,    -1,   212,   165,   444,
     166,    -1,   165,   441,   166,    -1,    -1,   449,    -1,   441,
       9,   449,    -1,   441,     9,   160,    -1,   441,    -1,   160,
      -1,    -1,    -1,    27,   449,    -1,   444,     9,   212,    -1,
     212,    -1,   444,     9,   212,    92,   449,    -1,   212,    92,
     449,    -1,    81,   124,   449,    -1,   446,     9,   445,    -1,
     445,    -1,   446,   388,    -1,    -1,   169,   195,   447,   196,
      -1,    26,   449,    -1,    52,   449,    -1,   219,    -1,   126,
      -1,   127,    -1,   448,    -1,   126,   165,   449,   166,    -1,
     126,   165,   449,     9,   449,   166,    -1,   149,    -1,   195,
     100,   195,   442,   196,    27,   449,   196,    -1,   195,   441,
       9,   449,   196,    -1,   449,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   727,   727,   727,   736,   738,   741,   742,   743,   744,
     745,   746,   749,   751,   751,   753,   753,   755,   756,   761,
     762,   763,   764,   765,   766,   767,   768,   769,   770,   771,
     772,   773,   774,   775,   776,   777,   778,   779,   780,   784,
     786,   789,   790,   791,   792,   797,   798,   802,   803,   805,
     808,   814,   821,   828,   832,   838,   840,   843,   844,   845,
     846,   849,   850,   854,   859,   859,   865,   865,   872,   871,
     877,   877,   882,   883,   884,   885,   886,   887,   888,   889,
     890,   891,   892,   893,   894,   897,   895,   902,   910,   904,
     914,   912,   916,   917,   921,   922,   923,   924,   925,   926,
     927,   928,   929,   930,   931,   939,   939,   944,   950,   954,
     954,   962,   963,   967,   968,   972,   977,   976,   989,   987,
    1001,   999,  1015,  1014,  1033,  1031,  1050,  1049,  1058,  1056,
    1068,  1067,  1079,  1077,  1090,  1091,  1095,  1098,  1101,  1102,
    1103,  1106,  1108,  1111,  1112,  1115,  1116,  1119,  1120,  1124,
    1125,  1130,  1131,  1134,  1135,  1136,  1140,  1141,  1145,  1146,
    1150,  1151,  1155,  1156,  1161,  1162,  1167,  1168,  1169,  1170,
    1173,  1176,  1178,  1181,  1182,  1186,  1188,  1191,  1194,  1197,
    1198,  1201,  1202,  1206,  1208,  1210,  1211,  1215,  1219,  1223,
    1228,  1233,  1238,  1243,  1249,  1258,  1260,  1262,  1263,  1267,
    1270,  1273,  1277,  1281,  1285,  1289,  1294,  1302,  1304,  1307,
    1308,  1309,  1311,  1316,  1317,  1320,  1321,  1322,  1326,  1327,
    1329,  1330,  1334,  1336,  1339,  1339,  1343,  1342,  1346,  1350,
    1348,  1363,  1360,  1373,  1375,  1377,  1379,  1381,  1383,  1385,
    1389,  1390,  1391,  1394,  1400,  1403,  1409,  1412,  1417,  1419,
    1424,  1429,  1433,  1434,  1440,  1441,  1446,  1447,  1452,  1453,
    1457,  1458,  1462,  1464,  1470,  1475,  1476,  1478,  1482,  1483,
    1484,  1485,  1489,  1490,  1491,  1492,  1493,  1494,  1496,  1501,
    1504,  1505,  1509,  1510,  1514,  1515,  1518,  1519,  1522,  1523,
    1526,  1527,  1531,  1532,  1533,  1534,  1535,  1536,  1537,  1541,
    1542,  1545,  1546,  1547,  1550,  1552,  1554,  1555,  1558,  1560,
    1564,  1565,  1567,  1568,  1569,  1572,  1576,  1577,  1581,  1582,
    1586,  1587,  1591,  1595,  1600,  1604,  1608,  1613,  1614,  1615,
    1618,  1620,  1621,  1622,  1625,  1626,  1627,  1628,  1629,  1630,
    1631,  1632,  1633,  1634,  1635,  1636,  1637,  1638,  1639,  1640,
    1641,  1642,  1643,  1644,  1645,  1646,  1647,  1648,  1649,  1650,
    1651,  1652,  1653,  1654,  1655,  1656,  1657,  1658,  1659,  1660,
    1661,  1662,  1663,  1664,  1665,  1667,  1668,  1670,  1672,  1673,
    1674,  1675,  1676,  1677,  1678,  1679,  1680,  1681,  1682,  1683,
    1684,  1685,  1686,  1687,  1688,  1689,  1690,  1691,  1692,  1696,
    1700,  1705,  1704,  1719,  1717,  1734,  1734,  1749,  1749,  1767,
    1768,  1773,  1778,  1782,  1788,  1792,  1798,  1800,  1804,  1806,
    1810,  1814,  1815,  1819,  1826,  1833,  1835,  1840,  1841,  1842,
    1846,  1850,  1854,  1858,  1860,  1862,  1864,  1869,  1870,  1875,
    1876,  1877,  1878,  1879,  1880,  1884,  1888,  1892,  1896,  1898,
    1900,  1905,  1910,  1914,  1915,  1919,  1920,  1924,  1925,  1929,
    1930,  1934,  1938,  1942,  1946,  1947,  1948,  1949,  1953,  1959,
    1968,  1981,  1982,  1985,  1988,  1991,  1992,  1995,  1999,  2002,
    2005,  2012,  2013,  2017,  2018,  2020,  2024,  2025,  2026,  2027,
    2028,  2029,  2030,  2031,  2032,  2033,  2034,  2035,  2036,  2037,
    2038,  2039,  2040,  2041,  2042,  2043,  2044,  2045,  2046,  2047,
    2048,  2049,  2050,  2051,  2052,  2053,  2054,  2055,  2056,  2057,
    2058,  2059,  2060,  2061,  2062,  2063,  2064,  2065,  2066,  2067,
    2068,  2069,  2070,  2071,  2072,  2073,  2074,  2075,  2076,  2077,
    2078,  2079,  2080,  2081,  2082,  2083,  2084,  2085,  2086,  2087,
    2088,  2089,  2090,  2091,  2092,  2093,  2094,  2095,  2096,  2097,
    2098,  2099,  2100,  2101,  2102,  2103,  2107,  2112,  2113,  2116,
    2117,  2118,  2122,  2123,  2124,  2128,  2129,  2130,  2134,  2135,
    2136,  2139,  2141,  2145,  2146,  2147,  2148,  2150,  2151,  2152,
    2153,  2154,  2155,  2156,  2157,  2158,  2159,  2162,  2167,  2168,
    2169,  2170,  2171,  2173,  2175,  2176,  2178,  2179,  2183,  2186,
    2189,  2195,  2196,  2197,  2198,  2199,  2200,  2201,  2206,  2208,
    2212,  2213,  2216,  2217,  2221,  2224,  2226,  2228,  2232,  2233,
    2234,  2235,  2237,  2240,  2244,  2245,  2246,  2247,  2250,  2251,
    2252,  2253,  2254,  2256,  2258,  2259,  2264,  2266,  2269,  2272,
    2274,  2276,  2279,  2281,  2285,  2287,  2290,  2293,  2299,  2301,
    2304,  2305,  2310,  2313,  2317,  2317,  2322,  2325,  2326,  2330,
    2331,  2336,  2337,  2341,  2342,  2346,  2347,  2352,  2354,  2359,
    2360,  2361,  2362,  2363,  2364,  2365,  2367,  2370,  2372,  2376,
    2377,  2378,  2379,  2380,  2382,  2384,  2386,  2390,  2391,  2392,
    2396,  2399,  2402,  2405,  2409,  2413,  2420,  2424,  2428,  2435,
    2436,  2441,  2443,  2444,  2447,  2448,  2451,  2452,  2456,  2457,
    2461,  2462,  2463,  2464,  2466,  2469,  2472,  2473,  2474,  2476,
    2478,  2482,  2483,  2484,  2486,  2487,  2488,  2492,  2494,  2497,
    2499,  2500,  2501,  2502,  2505,  2507,  2508,  2512,  2514,  2517,
    2519,  2520,  2521,  2525,  2527,  2530,  2533,  2535,  2537,  2541,
    2542,  2544,  2545,  2551,  2552,  2554,  2556,  2558,  2560,  2563,
    2564,  2565,  2569,  2570,  2571,  2572,  2573,  2574,  2575,  2576,
    2577,  2581,  2582,  2586,  2588,  2596,  2598,  2602,  2606,  2613,
    2614,  2620,  2621,  2628,  2631,  2635,  2638,  2643,  2644,  2645,
    2646,  2650,  2651,  2655,  2657,  2658,  2660,  2664,  2670,  2672,
    2676,  2679,  2682,  2690,  2693,  2696,  2697,  2700,  2703,  2704,
    2707,  2711,  2715,  2721,  2729,  2730
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
     351,   351,   351,   351,   351,   352,   353,   354,   355,   355,
     355,   356,   357,   358,   358,   359,   359,   360,   360,   361,
     361,   362,   363,   364,   365,   365,   365,   365,   366,   367,
     367,   368,   368,   369,   369,   370,   370,   371,   372,   372,
     373,   373,   373,   374,   374,   374,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   375,   375,   375,   375,
     375,   375,   375,   375,   375,   375,   376,   377,   377,   378,
     378,   378,   379,   379,   379,   380,   380,   380,   381,   381,
     381,   382,   382,   383,   383,   383,   383,   383,   383,   383,
     383,   383,   383,   383,   383,   383,   383,   383,   384,   384,
     384,   384,   384,   384,   384,   384,   384,   384,   385,   385,
     385,   386,   386,   386,   386,   386,   386,   386,   387,   387,
     388,   388,   389,   389,   390,   390,   390,   390,   391,   391,
     391,   391,   391,   391,   392,   392,   392,   392,   393,   393,
     393,   393,   393,   393,   393,   393,   394,   394,   395,   395,
     395,   395,   396,   396,   397,   397,   398,   398,   399,   399,
     400,   400,   401,   401,   403,   402,   404,   405,   405,   406,
     406,   407,   407,   408,   408,   409,   409,   410,   410,   411,
     411,   411,   411,   411,   411,   411,   411,   411,   411,   412,
     412,   412,   412,   412,   412,   412,   412,   413,   413,   413,
     414,   414,   414,   414,   414,   414,   415,   415,   415,   416,
     416,   417,   417,   417,   418,   418,   419,   419,   420,   420,
     421,   421,   421,   421,   421,   421,   422,   422,   422,   422,
     422,   423,   423,   423,   423,   423,   423,   424,   424,   425,
     425,   425,   425,   425,   425,   425,   425,   426,   426,   427,
     427,   427,   427,   428,   428,   429,   429,   429,   429,   430,
     430,   430,   430,   431,   431,   431,   431,   431,   431,   432,
     432,   432,   433,   433,   433,   433,   433,   433,   433,   433,
     433,   434,   434,   435,   435,   436,   436,   437,   437,   438,
     438,   439,   439,   440,   440,   441,   441,   442,   442,   442,
     442,   443,   443,   444,   444,   444,   444,   445,   446,   446,
     447,   447,   448,   449,   449,   449,   449,   449,   449,   449,
     449,   449,   449,   449,   450,   450
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
       1,     1,     1,     1,     1,     4,     4,     2,     2,     4,
       8,    10,     2,     1,     3,     1,     2,     1,     1,     1,
       1,     2,     4,     3,     3,     4,     1,     2,     4,     2,
       6,     0,     1,     4,     0,     2,     0,     1,     1,     3,
       1,     3,     1,     1,     3,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     4,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     1,     0,     0,     1,
       1,     3,     0,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     2,     1,     1,
       2,     2,     4,     4,     3,     4,     1,     1,     3,     3,
       3,     1,     1,     1,     1,     3,     3,     3,     2,     0,
       1,     0,     1,     0,     5,     3,     3,     1,     1,     1,
       1,     1,     3,     2,     1,     1,     1,     1,     1,     1,
       2,     2,     4,     4,     3,     4,     2,     0,     5,     3,
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
       2,     1,     4,     3,     0,     1,     3,     3,     1,     1,
       0,     0,     2,     3,     1,     5,     3,     3,     3,     1,
       2,     0,     4,     2,     2,     1,     1,     1,     1,     4,
       6,     1,     8,     5,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   664,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   738,     0,   726,   577,
       0,   583,   584,   585,    19,   612,   714,    84,   586,     0,
      66,     0,     0,     0,     0,     0,     0,     0,     0,   115,
       0,     0,     0,     0,     0,     0,   292,   293,   294,   297,
     296,   295,     0,     0,     0,     0,   138,     0,     0,     0,
     590,   592,   593,   587,   588,     0,     0,   594,   589,     0,
       0,   568,    20,    21,    22,    24,    23,     0,   591,     0,
       0,     0,     0,   595,     0,   298,    25,    26,    27,    29,
      28,    30,    31,    32,    33,    34,    35,    36,    37,    38,
     407,     0,    83,    56,   718,   578,     0,     0,     4,    45,
      47,    50,   611,     0,   567,     0,     6,   114,     7,     8,
       9,     0,     0,   290,   329,     0,     0,     0,     0,     0,
       0,     0,   327,   396,   397,   393,   392,   314,   398,     0,
       0,   313,   680,   569,     0,   614,   391,   289,   683,   328,
       0,     0,   681,   682,   679,   709,   713,     0,   381,   613,
      10,   297,   296,   295,     0,     0,    45,   114,     0,   780,
     328,   779,     0,   777,   776,   395,     0,     0,   365,   366,
     367,   368,   390,   388,   387,   386,   385,   384,   383,   382,
     714,   570,     0,   794,   569,     0,   348,   346,     0,   742,
       0,   621,   312,   573,     0,   794,   572,     0,   582,   721,
     720,   574,     0,     0,   576,   389,     0,     0,     0,     0,
     317,     0,    64,   319,     0,     0,    70,    72,     0,     0,
      74,     0,     0,     0,   816,   817,   821,     0,     0,    45,
     815,     0,   818,     0,     0,    76,     0,     0,     0,     0,
     105,     0,     0,     0,     0,    40,    41,   215,     0,     0,
     214,   140,   139,   220,     0,     0,     0,     0,     0,   791,
     126,   136,   734,   738,   763,     0,   597,     0,     0,     0,
     761,     0,    15,     0,    49,     0,   320,   130,   137,   474,
     417,     0,   785,   324,   668,   329,     0,   327,   328,     0,
       0,   579,     0,   580,     0,     0,     0,   104,     0,     0,
      52,   208,     0,    18,   113,     0,   135,   122,   134,   295,
     114,   291,    95,    96,    97,    98,    99,   101,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   726,    94,   717,   717,   102,   748,     0,
       0,     0,     0,     0,   288,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   347,   345,     0,
     684,   669,   717,     0,   675,   208,   717,     0,   719,   710,
     734,     0,   114,     0,     0,   666,   661,   621,     0,     0,
       0,     0,   746,     0,   422,   620,   737,     0,     0,    52,
       0,   208,   311,     0,   722,   669,   677,   575,     0,    56,
     176,     0,   406,     0,    81,     0,     0,   318,     0,     0,
       0,     0,     0,    73,    93,    75,   813,   814,     0,   811,
       0,     0,   795,     0,   790,     0,   100,    77,   103,     0,
       0,     0,     0,     0,     0,     0,   430,     0,   437,   439,
     440,   441,   442,   443,   444,   435,   459,   460,    56,     0,
      90,    92,    42,     0,    17,     0,     0,   216,     0,    79,
       0,     0,    80,   781,     0,     0,   329,   327,   328,     0,
       0,   146,     0,   735,     0,     0,     0,     0,   596,   762,
     612,     0,     0,   760,   617,   759,    48,     5,    12,    13,
      78,     0,   144,     0,     0,   411,     0,   621,     0,     0,
       0,     0,   197,     0,   623,   667,   825,   310,   378,   688,
      61,    55,    57,    58,    59,    60,     0,   394,   615,   616,
      46,     0,     0,     0,   623,   209,     0,   401,   116,   142,
       0,   351,   353,   352,     0,     0,   349,   350,   354,   356,
     355,   370,   369,   372,   371,   373,   375,   376,   374,   364,
     363,   358,   359,   357,   360,   361,   362,   377,   716,     0,
       0,   752,     0,   621,   784,     0,   783,   686,   709,   128,
     132,   124,   114,     0,     0,   322,   325,   331,   431,   344,
     343,   342,   341,   340,   339,   338,   337,   336,   335,   334,
       0,   671,   670,     0,     0,     0,     0,     0,     0,     0,
     778,   659,   663,   620,   665,     0,     0,   794,     0,   741,
       0,   740,     0,   725,   724,     0,     0,   671,   670,   315,
     178,   180,    56,   409,   316,     0,    56,   160,    65,   319,
       0,     0,     0,     0,   172,   172,    71,     0,     0,   809,
     621,     0,   800,     0,     0,     0,   619,     0,     0,   568,
       0,    25,    50,   599,   567,   607,     0,   598,    54,   606,
       0,     0,   447,   714,   448,     0,   455,   452,   453,   461,
       0,   438,   433,     0,   436,     0,     0,     0,     0,    39,
      43,     0,   213,   221,   218,     0,     0,   772,   775,   774,
     773,    11,   804,     0,     0,     0,   734,   731,     0,   421,
     771,   770,   769,     0,   765,     0,   766,   768,     0,     5,
     321,     0,     0,   468,   469,   477,   476,     0,     0,   620,
     416,   420,     0,   786,     0,   801,   668,   196,   824,     0,
       0,   685,   669,   676,   715,     0,   793,   210,   566,   622,
     207,     0,   668,     0,     0,   144,   403,   118,   380,     0,
     425,   426,     0,   423,   620,   747,     0,     0,   208,   146,
     144,   142,     0,   726,   332,     0,     0,   208,   673,   674,
     687,   711,   712,     0,     0,     0,   647,   628,   629,   630,
     631,     0,     0,     0,    25,   639,   638,   653,   621,     0,
     661,   745,   744,     0,   723,   669,   678,   581,     0,   182,
       0,     0,    62,     0,     0,     0,     0,     0,     0,   152,
     153,   164,     0,    56,   162,    87,   172,     0,   172,     0,
       0,   819,     0,   620,   810,   812,   799,   798,     0,   796,
     600,   601,   627,     0,   621,   619,     0,     0,   419,   619,
       0,   754,   432,     0,     0,     0,   457,   458,   456,     0,
       0,   434,     0,   106,     0,   109,    91,    44,   217,     0,
     782,    82,     0,     0,   792,   145,   147,   223,     0,     0,
     732,     0,   764,     0,    16,     0,   143,   223,     0,     0,
     413,     0,   787,     0,     0,     0,   195,   825,     0,   199,
       0,   671,   670,   796,     0,   211,    53,     0,   668,   141,
       0,   668,     0,   379,   751,   750,     0,   208,     0,     0,
       0,   144,   120,   582,   672,   208,     0,     0,   634,   635,
     636,   637,   640,   641,   651,     0,   621,   647,     0,   633,
     655,   647,   620,   658,   660,   662,     0,   739,   672,     0,
       0,     0,     0,   179,   410,    67,     0,   319,   154,   734,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   166,
       0,   807,   808,     0,     0,   823,     0,   604,   620,   618,
       0,   609,     0,   621,     0,     0,   610,   608,   758,     0,
     621,   445,   449,   446,   454,   462,   463,     0,    56,   219,
     806,   803,     0,   289,   736,   734,   323,   326,   330,     0,
      14,   289,   480,     0,     0,   482,   475,   478,     0,   473,
       0,   788,   802,   408,     0,   200,     0,     0,     0,   208,
     212,   801,     0,   223,     0,   668,     0,   208,     0,   707,
     223,   223,     0,     0,   333,   208,     0,   701,     0,   644,
     620,   646,     0,   632,     0,     0,   621,     0,   652,   743,
       0,    56,     0,   175,   161,     0,     0,   151,    85,   165,
       0,     0,   168,     0,   173,   174,    56,   167,   820,   797,
       0,   626,   625,   602,     0,   620,   418,   605,   603,     0,
     424,   620,   753,     0,     0,     0,     0,   148,     0,     0,
       0,   287,     0,     0,     0,   127,   222,   224,     0,   286,
       0,   289,     0,   767,   131,   471,     0,     0,   412,     0,
     203,     0,   202,   672,   208,     0,   400,   801,   289,   801,
       0,   749,     0,   706,   289,   289,   223,   668,     0,   700,
     650,   649,   642,     0,   645,   620,   654,   643,    56,   181,
      63,    68,   155,     0,   163,   169,    56,   171,     0,     0,
     415,     0,   757,   756,     0,    56,   110,   805,     0,     0,
       0,     0,   149,   254,   252,   568,    24,     0,   248,     0,
     253,   264,     0,   262,   267,     0,   266,     0,   265,     0,
     114,   226,     0,   228,     0,   733,   472,   470,   481,   479,
     204,     0,   201,   208,     0,   704,     0,     0,     0,   123,
     400,   801,   708,   129,   133,   289,     0,   702,     0,   657,
       0,   177,     0,    56,   158,    86,   170,   822,   624,     0,
       0,     0,     0,     0,     0,     0,     0,   238,   242,     0,
       0,   233,   532,   531,   528,   530,   529,   549,   551,   550,
     520,   510,   526,   525,   487,   497,   498,   500,   499,   519,
     503,   501,   502,   504,   505,   506,   507,   508,   509,   511,
     512,   513,   514,   515,   516,   518,   517,   488,   489,   490,
     493,   494,   496,   534,   535,   544,   543,   542,   541,   540,
     539,   527,   546,   536,   537,   538,   521,   522,   523,   524,
     547,   548,   552,   554,   553,   555,   556,   533,   558,   557,
     491,   560,   562,   561,   495,   565,   563,   564,   559,   492,
     545,   486,   259,   483,     0,   234,   280,   281,   279,   272,
       0,   273,   235,   306,     0,     0,     0,     0,   114,     0,
     206,     0,   703,     0,    56,   282,    56,   117,     0,     0,
     125,   801,   648,     0,    56,   156,    69,     0,   414,   755,
     450,   108,   236,   237,   309,   150,     0,     0,   256,   249,
       0,     0,     0,   261,   263,     0,     0,   268,   275,   276,
     274,     0,     0,   225,     0,     0,     0,     0,   205,   705,
       0,   466,   623,     0,     0,    56,   119,     0,   656,     0,
       0,     0,    88,   239,    45,     0,   240,   241,     0,     0,
     255,   258,   484,   485,     0,   250,   277,   278,   270,   271,
     269,   307,   304,   229,   227,   308,     0,   467,   622,     0,
     402,   283,     0,   121,     0,   159,   451,     0,   112,     0,
     289,   257,   260,     0,   668,   231,     0,   464,   399,   404,
     157,     0,     0,    89,   246,     0,   288,   305,   185,     0,
     623,   300,   668,   465,     0,   111,     0,     0,   245,   801,
     668,   184,   301,   302,   303,   825,   299,     0,     0,     0,
     244,     0,   183,   300,     0,   801,     0,   243,   284,    56,
     230,   825,     0,   187,     0,    56,     0,     0,   188,     0,
     232,     0,   285,     0,   191,     0,   190,   107,   192,     0,
     189,     0,   194,   193
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   118,   739,   517,   176,   264,   265,
     120,   121,   122,   123,   124,   125,   309,   541,   542,   436,
     231,  1242,   442,  1173,  1458,   707,   261,   478,  1422,   886,
    1018,  1473,   325,   177,   543,   773,   932,  1063,   544,   559,
     791,   501,   789,   545,   522,   790,   327,   280,   297,   131,
     775,   742,   725,   895,  1191,   981,   839,  1376,  1245,   658,
     845,   441,   666,   847,  1096,   651,   829,   832,   971,  1479,
    1480,   533,   534,   553,   554,   269,   270,   274,  1023,  1126,
    1209,  1356,  1464,  1482,  1386,  1426,  1427,  1428,  1197,  1198,
    1199,  1387,  1393,  1435,  1202,  1203,  1207,  1349,  1350,  1351,
    1367,  1510,  1127,  1128,   178,   133,  1495,  1496,  1354,  1130,
     134,   224,   437,   438,   135,   136,   137,   138,   139,   140,
     141,   142,  1227,   143,   772,   931,   144,   228,   304,   432,
     526,   527,  1003,   528,  1004,   145,   146,   147,   685,   148,
     149,   258,   150,   259,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   697,   698,   878,   475,   476,   477,   704,
    1412,   151,   523,  1217,   524,   908,   747,  1039,  1036,  1342,
    1343,   152,   153,   154,   218,   225,   312,   422,   155,   862,
     689,   156,   863,   416,   757,   864,   816,   952,   954,   955,
     956,   818,  1075,  1076,   819,   632,   407,   186,   187,   157,
     536,   390,   391,   763,   158,   219,   180,   160,   161,   162,
     163,   164,   165,   166,   589,   167,   221,   222,   504,   210,
     211,   592,   593,  1009,  1010,   289,   290,   733,   168,   494,
     169,   531,   170,   251,   281,   320,   451,   858,   915,   723,
     669,   670,   671,   252,   253,   759
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1016
static const yytype_int16 yypact[] =
{
   -1016,   129, -1016, -1016,  3966, 11642, 11642,   -58, 11642, 11642,
   11642, -1016, 11642, 11642, 11642, 11642, 11642, 11642, 11642, 11642,
   11642, 11642, 11642, 11642,  3408,  3408,  9016, 11642, 13795,   -53,
     -46, -1016, -1016, -1016, -1016, -1016,   152, -1016, -1016, 11642,
   -1016,   -46,    -9,    -1,    26,   -46,  9218,  9664,  9420, -1016,
    2552,  8612,   -36, 11642, 14097,   -18, -1016, -1016, -1016,    99,
     231,    16,   151,   167,   189,   209, -1016,  9664,   218,   220,
   -1016, -1016, -1016, -1016, -1016,   365,  1717, -1016, -1016,  9664,
    9622, -1016, -1016, -1016, -1016, -1016, -1016,  9664, -1016,   269,
     238,  9664,  9664, -1016, 11642, -1016, -1016, -1016, -1016, -1016,
   -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016,
   -1016, 11642, -1016, -1016,   154,   424,   448,   448, -1016,   425,
     325,   266, -1016,   313, -1016,    35, -1016,   444, -1016, -1016,
   -1016, 14161,   442, -1016, -1016,   333,   336,   346,   348,   366,
     370, 12745, -1016, -1016, -1016, -1016,   529, -1016,   531,   538,
     429, -1016,    88,   439,   493, -1016, -1016,   462,   125,  1623,
     118,   449,   119,   136,   461,    79, -1016,   131, -1016,   587,
   -1016, -1016, -1016,   514,   465,   515, -1016,   444,   442, 14686,
    1676, 14686, 11642, 14686, 14686,  3952,   620,  9664, -1016, -1016,
     614, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016,
   -1016, -1016, 13605,   504, -1016,   525,   551,   551,  3408, 14343,
     473,   667, -1016,   514, 13605,   504,   535,   536,   488,   139,
   -1016,   561,   118,  9824, -1016, -1016, 11642,  7198,   678,    51,
   14686,  8208, -1016, 11642, 11642,  9664, -1016, -1016, 12786,   491,
   -1016, 12827,  2552,  2552,   527, -1016, -1016,   496, 13150,   680,
   -1016,   682, -1016,  9664,   622, -1016,   505, 12868,   506,   520,
   -1016,    50, 12915,  9664,    55, -1016,   234, -1016, 13657,    60,
   -1016, -1016, -1016,   693,    64,  3408,  3408, 11642,   512,   547,
   -1016, -1016,  3099,  9016,    32,   454, -1016, 11844,  3408,   407,
   -1016,  9664, -1016,   -22,   325,   516, 14384, -1016, -1016, -1016,
     633,   701,   625, 14686,    24,   523, 14686,   528,   497,  4168,
   11642,   305,   532,   530,   305,   264,   219, -1016,  9664,  2552,
     533, 10026,  2552, -1016, -1016,  8452, -1016, -1016, -1016, -1016,
     444, -1016, -1016, -1016, -1016, -1016, -1016, -1016, 11642, 11642,
   11642, 10228, 11642, 11642, 11642, 11642, 11642, 11642, 11642, 11642,
   11642, 11642, 11642, 11642, 11642, 11642, 11642, 11642, 11642, 11642,
   11642, 11642, 11642, 13795, -1016, 11642, 11642, -1016, 11642,  1588,
    9664,  9664, 14161,   621,   475,  8410, 11642, 11642, 11642, 11642,
   11642, 11642, 11642, 11642, 11642, 11642, 11642, -1016, -1016,  1803,
   -1016,   145, 11642, 11642, -1016, 10026, 11642, 11642,   154,   157,
    3099,   534,   444, 10430, 12956, -1016,   540,   717, 13605,   541,
     -48, 13029,   551, 10632, -1016, 10834, -1016,   548,   183, -1016,
     138, 10026, -1016, 13837, -1016,   185, -1016, -1016, 12997, -1016,
   -1016, 11036, -1016, 11642, -1016,   646,  7400,   727,   546, 14578,
     731,    83,    38, -1016, -1016, -1016, -1016, -1016,  2552,   666,
     555,   743, -1016, 13466, -1016,   574, -1016, -1016, -1016,   684,
   11642, 12046,   685, 11642, 11642, 11642, -1016,   520, -1016, -1016,
   -1016, -1016, -1016, -1016, -1016,   577, -1016, -1016, -1016,   567,
   -1016, -1016,   255, 14097, -1016,  9664, 11642,   551,   -18, -1016,
   13466,   688, -1016,   551,    84,    86,   568,   573,   966,   576,
    9664,   649,   590,   551,    87,   600, 14112,  9664, -1016, -1016,
     724,    74,   -26, -1016, -1016, -1016,   325, -1016, -1016, -1016,
   -1016, 11642,   675,   634,   285, -1016,   679,   795,   611,  2552,
    2552,   796, -1016,   616,   802, -1016,  2552,   102,   751,   135,
   -1016, -1016, -1016, -1016, -1016, -1016,  2093, -1016, -1016, -1016,
   -1016,    49,  3408,   623,   807, 14686,   803, -1016, -1016,   697,
    8856, 14726,  3752,  3952, 11642, 14645,  4355,  4556,  2872,  4756,
    2456,  4957,  4957,  4957,  4957,  1678,  1678,  1678,  1678,   733,
     733,   438,   438,   438,   614,   614,   614, -1016, 14686,   618,
     619, 14440,   624,   813, -1016, 11642,   300,   629,   157, -1016,
   -1016, -1016,   444,  2088, 11642, -1016, -1016,  3952, -1016,  3952,
    3952,  3952,  3952,  3952,  3952,  3952,  3952,  3952,  3952,  3952,
   11642,   300,   630,   628,  2972,   640,   635,  3030,    89,   631,
   -1016, 13557, -1016,  9664, -1016,   523,   102,   504,  3408, 14686,
    3408, 14481,   127,   203, -1016,   642, 11642, -1016, -1016, -1016,
    6996,   433, -1016, 14686, 14686,   -46, -1016, -1016, -1016, 11642,
   13378, 13466,  9664,  7602,   643,   644, -1016,    58,   715, -1016,
     834,   648, 13217,  2552, 13466, 13466, 13466,   650,   247,   706,
     655,   656,   176, -1016,   709, -1016,   658, -1016, -1016, -1016,
   11642,   689, 14686,   120, 14686,   845, 13038,   852, -1016, 14686,
    3494, -1016,   577,   794, -1016,  4370, 14042,   677,  9664, -1016,
   -1016,  3098, -1016, -1016,   859,  3408,   687, -1016, -1016, -1016,
   -1016, -1016,   785,   121, 14042,   683,  3099, 13741,   864, -1016,
   -1016, -1016, -1016,   686, -1016, 11642, -1016, -1016,  3562, -1016,
   14686, 14042,   692, -1016, -1016, -1016, -1016,   868, 11642,   633,
   -1016, -1016,   690, -1016,  2552,   858,   115, -1016, -1016,   103,
   13887, -1016,   210, -1016, -1016,  2552, -1016,   551, -1016, 11238,
   -1016, 13466,    30,   691, 14042,   675, -1016, -1016,  4154, 11642,
   -1016, -1016, 11642, -1016, 11642, -1016, 12232,   696, 10026,   649,
     675,   697,  9664, 13795,   551, 12273,   698, 10026, -1016, -1016,
     227, -1016, -1016,   878, 13169, 13169, 13557, -1016, -1016, -1016,
   -1016,   702,   260,   704,   705, -1016, -1016, -1016,   887,   707,
     540,   551,   551, 11440, -1016,   262, -1016, -1016, 12314,   458,
     -46,  8208, -1016,  4572,   710,  4774,   712,  3408,   719,   778,
     551, -1016,   890, -1016, -1016, -1016, -1016,   503, -1016,   263,
    2552, -1016,  2552,   666, -1016, -1016, -1016,   901,   716,   720,
   -1016, -1016,   791,   722,   908, 13466,   781,  9664,   633, 13466,
    9058, 13466, 14686, 11642, 11642, 11642, -1016, -1016, -1016, 11642,
   11642, -1016,   520, -1016,   847, -1016, -1016, -1016, -1016, 13466,
     551, -1016,  2552,  9664, -1016,   917, -1016, -1016,    92,   732,
     551,  8814, -1016,  2843, -1016,  3764,   917, -1016,   201,     2,
   14686,   804, -1016,   734,  2552,   678, -1016,  2552,   854,   916,
   11642,   300,   738, -1016,  3408, 14686, -1016,   740,    30, -1016,
     736,    30,   742,  4154, 14686, 14537,   744, 10026,   745,   746,
     748,   675, -1016,   488,   747, 10026,   752, 11642, -1016, -1016,
   -1016, -1016, -1016, -1016,   814,   741,   931, 13557,   810, -1016,
     633, 13557, 13557, -1016, -1016, -1016,  3408, 14686, -1016,   -46,
     920,   891,  8208, -1016, -1016, -1016,   759, 11642,   551,  3099,
   13378,   763, 13466,  4976,   517,   765, 11642,    41,   276, -1016,
     798, -1016, -1016, 13290,   939, -1016, 13466, -1016, 13466, -1016,
     771, -1016,   846,   960,   775,   776, -1016, -1016,   850,   779,
     968, 14686, 13127, 14686, -1016, 14686, -1016,   799, -1016, -1016,
   -1016,   900, 14042,    57, -1016,  3099, -1016, -1016,  3952,   801,
   -1016,   426, -1016,    23, 11642, -1016, -1016, -1016, 11642, -1016,
   11642, -1016, -1016, -1016,   104,   980, 13466, 12355,   806, 10026,
     551,   858,   808, -1016,   815,    30, 11642, 10026,   816, -1016,
   -1016, -1016,   811,   827, -1016, 10026,   828, -1016, 13557, -1016,
   13557, -1016,   829, -1016,   873,   832,   993,   833, -1016,   551,
     996, -1016,   835, -1016, -1016,   837,   114, -1016, -1016, -1016,
     838,   841, -1016, 12489, -1016, -1016, -1016, -1016, -1016, -1016,
    2552, -1016,   906, -1016, 13466,   633, -1016, -1016, -1016, 13466,
   -1016, 13466, -1016, 11642,   836,  5178,  2552, -1016,   482,  2552,
   14042, -1016, 14027,   884, 13978, -1016, -1016, -1016,   621,  3248,
      67,   475,   117, -1016, -1016,   882, 12404, 12445, 14686,   964,
    1026, 13466, -1016,   849, 10026,   851,   935,   858,   681,   858,
     853, 14686,   857, -1016,   907,  1201, -1016,    30,   860, -1016,
   -1016,   918, -1016, 13557, -1016,   633, -1016, -1016, -1016,  6996,
   -1016, -1016, -1016,  7804, -1016, -1016, -1016,  6996,   861, 13466,
   -1016,   921, -1016,   922, 13085, -1016, -1016, -1016, 14042, 14042,
    1034,    62, -1016, -1016, -1016,    69,   856,    70, -1016, 12563,
   -1016, -1016,    71, -1016, -1016,  2160, -1016,   863, -1016,   975,
     444, -1016,  2552, -1016,   621, -1016, -1016, -1016, -1016, -1016,
    1041, 13466, -1016, 10026,   867, -1016,   870,   869,   362, -1016,
     935,   858, -1016, -1016, -1016,  1477,   872, -1016, 13557, -1016,
     945,  6996,  8006, -1016, -1016, -1016,  6996, -1016, -1016, 13466,
   13466, 11642,  5380,   874,   875, 13466, 14042, -1016, -1016,  1379,
   14027, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016,
   -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016,
   -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016,
   -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016,
   -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016,
   -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016,
   -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016,
   -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016,
   -1016, -1016,   466, -1016,   884, -1016, -1016, -1016, -1016, -1016,
      44,   451, -1016,  1056,    72,  9664,   975,  1059,   444, 13466,
   -1016,   879, -1016,   303, -1016, -1016, -1016, -1016,   876,   362,
   -1016,   858, -1016, 13557, -1016, -1016, -1016,  5582, -1016, -1016,
   12552, -1016, -1016, -1016, -1016, -1016, 13937,    46, -1016, -1016,
   13466, 12563, 12563,  1024, -1016,  2160,  2160,   498, -1016, -1016,
   -1016, 13466,  1002, -1016,   885,    73, 13466,  9664, -1016, -1016,
    1004, -1016,  1072,  5784,  5986, -1016, -1016,   362, -1016,  6188,
     888,  1007,   982, -1016,   992,   944, -1016, -1016,   998,  1379,
   -1016, -1016, -1016, -1016,   932, -1016,  1063, -1016, -1016, -1016,
   -1016, -1016,  1080, -1016, -1016, -1016,   905, -1016,   321,   899,
   -1016, -1016,  6390, -1016,   904, -1016, -1016,   909,   934,  9664,
     475, -1016, -1016, 13466,   111, -1016,  1030, -1016, -1016, -1016,
   -1016, 14042,   677, -1016,   949,  9664,   281, -1016, -1016,   914,
    1103,   513,   111, -1016,  1038, -1016, 14042,   919, -1016,   858,
     132, -1016, -1016, -1016, -1016,  2552, -1016,   923,   925,    81,
   -1016,   409, -1016,   513,   341,   858,   924, -1016, -1016, -1016,
   -1016,  2552,  1042,  1104,   409, -1016,  6592,   345,  1109, 13466,
   -1016,  6794, -1016,  1049,  1111, 13466, -1016, -1016,  1112, 13466,
   -1016, 13466, -1016, -1016
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1016, -1016, -1016,  -476, -1016, -1016, -1016,    -4, -1016,   647,
     -17,   895,   840, -1016,  1439, -1016,  -401, -1016,     3, -1016,
   -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016,  -345,
   -1016, -1016,  -174,    13,     0, -1016, -1016, -1016,     1, -1016,
   -1016, -1016, -1016,     8, -1016, -1016,   757,   761,   762,   979,
     349,  -644,   343,   398,  -342, -1016,   165, -1016, -1016, -1016,
   -1016, -1016, -1016,  -609,    53, -1016, -1016, -1016, -1016,  -333,
   -1016,  -743, -1016,  -376, -1016, -1016,   662, -1016,  -858, -1016,
   -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016,  -108,
   -1016, -1016, -1016, -1016, -1016,  -191, -1016,    31,  -781, -1016,
    -966,  -360, -1016,  -155,    20,  -131,  -347, -1016,  -198, -1016,
     -63,   -15,  1118,  -620,  -355, -1016, -1016,   -33, -1016, -1016,
    2435,   -57,   -70, -1016, -1016, -1016, -1016, -1016, -1016,   246,
    -707, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016,
   -1016,   789, -1016, -1016,   284, -1016,   700, -1016, -1016, -1016,
   -1016, -1016, -1016, -1016,   293, -1016,   711, -1016, -1016,   477,
   -1016,   272, -1016, -1016, -1016, -1016, -1016, -1016, -1016, -1016,
    -760, -1016,  1074,   168,  -338, -1016, -1016,   242,  1978,  2052,
   -1016, -1016,  -725,  -394,  -545, -1016, -1016,   381,  -599,  -814,
   -1016, -1016, -1016, -1016, -1016,   367, -1016, -1016, -1016,  -283,
    -733,  -188,  -183,  -133, -1016, -1016,    27, -1016, -1016, -1016,
   -1016,     6,  -127, -1016,    45, -1016, -1016, -1016,  -378,   911,
   -1016, -1016, -1016, -1016, -1016,   526,   339, -1016, -1016,   912,
   -1016, -1016, -1016,  -314,   -81,  -199,  -284, -1016, -1015, -1016,
     338, -1016, -1016, -1016,   -38,  -903
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -795
static const yytype_int16 yytable[] =
{
     119,   331,   373,   401,   128,   129,   298,   126,   556,   770,
     301,   302,   130,   634,  1044,   227,   419,   127,   256,   625,
     605,   535,   628,   917,   132,   587,   232,   394,   650,   927,
     236,   159,   817,   424,   220,   551,  1146,   266,   425,   836,
     399,   738,   911,   239,   322,   645,   249,   331,   305,  1031,
     328,   206,   207,  1395,   307,  1429,   849,   267,   765,   293,
     433,  1118,   294,   279,   483,   664,    11,   850,  1094,   488,
    1135,  1256,    11,   491,  1396,   389,  1212,   705,  -251,  1260,
    1344,  1402,  1402,   279,   338,   339,   340,   279,   279,   426,
    1256,   273,   662,   715,   506,   715,   727,   389,   727,    11,
     341,   727,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   727,   363,   318,   727,   279,  -405,     3,
     893,   930,  1228,   750,  1230,   918,  1139,   182,   308,   409,
    1000,   396,   223,  1072,  1005,   330,   940,  1077,   539,   226,
    -689,   417,  1037,    11,   479,   507,   560,    11,  1119,  -570,
    -405,  1002,   260,  1120,   392,    56,    57,    58,   171,   172,
     329,  1121,   299,   737,    11,   518,   519,   374,   919,  1140,
     392,  -690,   268,   406,   532,  1052,   233,  -692,  1054,   392,
     532,   402,   205,   205,   234,  1148,   217,  -696,  -691,   785,
    1038,  -727,  1154,  1155,   446,   447,   200,  -693,  1122,  1123,
     452,  1124,   480,   200,   496,   766,  1369,  -198,   271,   396,
     497,   235,  -571,   119,   851,   760,  -198,   119,   629,   410,
     430,   440,   323,    95,   435,   412,   665,   984,  1095,   988,
    1397,   418,   598,   331,   558,  1430,   482,  -728,   434,   454,
     423,   833,   484,  1074,   159,   835,  1125,   489,   159,  1257,
    1258,   492,   598,   905,  1213,  -730,  -251,  1261,  1345,  1403,
    1444,  1478,  -694,   736,   516,   916,   854,   397,  1507,   663,
     716,   452,   717,   728,   598,   803,  -689,   894,  1024,  -695,
     298,   328,  1502,   598,   284,   487,   598,  1062,  1235,   513,
     393,   874,   493,   493,   498,   119,   389,  -186,  -622,   503,
    1172,  -622,  1150,  1215,   550,   512,   393,  -690,   249,  -794,
    -698,   279,   127,  -692,  -729,   393,   485,   866,  -622,   132,
    -699,   398,   744,  -696,  -691,  1410,   159,  -727,   114,   284,
     958,   319,   606,  -693,   513,   635,   275,   708,   898,   761,
     272,  1032,   310,  1466,   762,   397,  1417,  1085,   986,   987,
     287,   288,   276,  1078,  1033,   596,   279,   279,   279,   220,
     205,   986,   987,  1512,  -794,   597,   205,  1523,  1411,   643,
     284,   318,   205,  -728,   277,   621,   602,   286,   857,    56,
      57,    58,   171,   172,   329,   622,  1467,   787,  1181,  1034,
     959,  -730,   318,  1416,   278,   287,   288,   637,  -694,  -794,
     667,   590,   938,   282,  1236,   283,  1513,   597,   299,   647,
    1524,   946,   796,   549,   963,  -695,   644,   503,   792,   648,
    1118,   319,   119,   300,   745,   410,   205,   623,   787,   657,
     284,   626,   983,   205,   205,   285,   287,   288,   761,   746,
     205,  1453,   317,   762,   824,   943,   205,    95,  1240,   825,
    -729,  -794,   989,   159,  -794,   319,   266,   548,    11,  1160,
     999,  1161,   318,   535,  1501,  1097,   324,  1398,  1488,   777,
    1390,   710,   284,   419,   360,   361,   362,   513,   363,   535,
    1514,   752,   753,  1391,  1399,  -794,   722,  1400,   758,   284,
     830,   831,   732,   734,   311,   286,   287,   288,   321,   826,
    1392,   403,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   284,  1438,   969,   970,  1119,   314,   284,
     332,   217,  1120,   333,    56,    57,    58,   171,   172,   329,
    1121,  1439,    49,   334,  1440,   335,  1026,   514,   287,   288,
      56,    57,    58,   171,   172,   329,   279,   387,   388,  1365,
    1366,  1058,  1071,   336,  1239,   287,   288,   337,   205,  1066,
      56,    57,    58,    59,    60,   329,   205,  1122,  1123,   767,
    1124,    66,   370,    56,    57,    58,   171,   172,   329,   287,
     288,  -427,  1504,   365,   508,   287,   288,   985,   986,   987,
     366,  1086,    95,  1188,  1189,   284,  1508,  1509,  1517,  1106,
     513,  1091,   986,   987,  1436,  1437,  1112,  1115,    95,   371,
     389,  1492,  1493,  1494,   509,  1134,   367,   815,   515,   820,
     794,  1432,  1433,   598,   452,   859,   369,   368,    95,  1372,
     834,   313,   315,   316,   395,   535,   119,  1132,   535,  -428,
     509,    95,   515,   509,   515,   515,  -697,  -570,   842,   119,
     400,   405,   291,   127,   363,   821,   844,   822,   411,   319,
     132,   287,   288,  1145,   389,   414,   415,   159,  -569,   420,
    1169,  1152,  1166,   421,   423,  1118,   431,   840,   444,  1158,
     159,   449,   448,   539,  -789,  1177,   453,   455,   459,   460,
     461,   119,   456,   458,   887,   462,   463,   490,   499,   464,
     465,   942,   500,   520,   525,   529,   913,   530,   127,   537,
     205,    49,  1048,    11,   538,   132,   633,   923,   -51,   557,
     655,  1481,   159,   547,   119,   631,   433,   636,   128,   129,
    1131,   126,   890,   659,   642,   661,   130,   668,  1131,  1481,
     672,   127,   673,   503,   900,   690,   921,  1503,   132,   691,
     695,   703,   706,   714,   718,   159,   922,  1241,  1224,   719,
     724,   205,   535,   721,  1418,  1246,   357,   358,   359,   360,
     361,   362,  1119,   363,  1252,   726,   735,  1120,   279,    56,
      57,    58,   171,   172,   329,  1121,   729,   741,   743,   220,
     951,   951,   815,   748,   749,  1190,   205,   751,   205,   755,
     754,   756,   990,  -429,   991,   972,   769,   771,   774,   768,
     780,   781,   784,   783,   788,   797,   776,   119,   205,   119,
     798,   119,  1122,  1123,   973,  1124,   800,   801,   827,   852,
     846,   848,  1377,   853,   855,   865,   127,  1361,   127,   867,
     868,   869,   870,   132,  1020,   132,   871,    95,   159,   875,
     159,   879,   159,  1001,   978,  1131,  1007,  1449,  1027,   882,
     873,  1131,  1131,   889,   535,   885,  1042,   892,   901,   758,
    1229,   897,   909,   205,   891,   914,   928,   912,   902,  1021,
     907,   937,   947,   945,   205,   205,   962,   957,  1357,   960,
     961,   119,   980,   964,   982,   128,   129,   975,   126,   977,
     993,  1118,   994,   130,   979,   996,   995,   998,   127,   203,
     203,   508,  1017,   215,   997,   132,  1022,  1025,  1040,  1045,
    1046,  1041,   159,  1049,  1053,  1491,  1051,  1055,  1068,  1057,
    1070,  1059,  1065,  1069,  1060,   215,  1061,  1081,  1067,    11,
    1073,  1050,  1131,   815,  1080,   923,  1084,   815,   815,  1088,
    1082,   217,  1092,  1413,  1098,  1414,  1100,  1103,   119,  1105,
    1104,  1107,  1108,  1419,  1109,  1083,  1214,  1111,  1110,   119,
     403,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,  1116,  1079,  1141,  1114,   127,  1163,   331,   159,
    1133,  1144,  1165,   132,  1147,   205,   503,   840,  1119,  1156,
     159,  1149,  1153,  1120,  1452,    56,    57,    58,   171,   172,
     329,  1121,  1157,  1168,  1159,  1162,   387,   388,  1164,  1167,
    1179,  1216,  1170,  1171,  1185,  1174,  1355,  1201,  1175,  1220,
    1221,  1226,  1238,  1129,  1223,  1249,  1250,  1225,  1255,  1231,
    1353,  1129,   503,  1232,  1259,  1359,  1237,  1247,  1122,  1123,
    1352,  1124,  1178,  1362,   815,  1363,   815,  1364,  1371,  1373,
    1401,  1382,  1383,  1406,  1415,  1409,  1434,  1442,  1187,  1447,
    1443,  1448,  1456,    95,  -247,  1455,  1457,  1459,  1462,   389,
    1460,  1211,   205,  1396,  1463,  1468,  1472,   203,   204,   204,
    1465,  1470,   216,   203,  1471,  1483,  1233,  1486,  1516,   203,
    1489,   119,  1490,  1498,  1521,   249,  1500,  1518,  1519,  1505,
    1206,  1506,  1515,  1525,  1528,  1529,  1531,  1485,   127,   601,
     709,   599,   939,   600,   205,   132,   372,   215,   215,   906,
     941,  1210,   159,   215,  1499,  1087,  1176,   205,   205,  1497,
     712,   374,  1389,  1394,  1520,  1208,  1511,   229,  1405,   815,
    1368,  1043,   720,   203,   608,   119,  1016,   701,  1129,   119,
     203,   203,  1014,   119,  1129,  1129,  1244,   203,   702,   881,
    1035,   535,   127,   203,  1407,  1064,   953,   965,   495,   132,
     127,   992,     0,   205,   505,  1341,   159,   132,     0,   535,
     159,  1348,     0,     0,   159,  1118,     0,   535,   249,     0,
       0,     0,     0,     0,   215,     0,     0,   215,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1358,     0,     0,
       0,     0,     0,     0,   815,     0,     0,   119,   119,     0,
       0,     0,   119,    11,     0,  1375,     0,     0,   119,     0,
       0,     0,     0,     0,   127,  1129,     0,     0,   215,   127,
       0,   132,     0,     0,     0,   127,   132,     0,   159,   159,
       0,     0,   132,   159,  1404,     0,     0,     0,     0,   159,
       0,     0,   204,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   683,     0,   203,     0,     0,     0,     0,
       0,     0,  1119,   203,     0,  1475,     0,  1120,     0,    56,
      57,    58,   171,   172,   329,  1121,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1446,     0,     0,     0,
     683,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   204,   215,     0,   331,     0,     0,   682,   204,
     204,   279,  1122,  1123,     0,  1124,   204,     0,     0,     0,
       0,     0,   204,     0,     0,     0,     0,     0,     0,   815,
       0,     0,     0,   119,     0,     0,     0,    95,     0,     0,
       0,     0,  1424,     0,     0,   682,     0,  1341,  1341,     0,
     127,  1348,  1348,     0,     0,     0,     0,   132,     0,     0,
    1234,     0,     0,   279,   159,     0,     0,     0,     0,   119,
     119,     0,     0,     0,     0,   119,     0,     0,     0,     0,
       0,     0,     0,     0,   215,   215,   127,   127,     0,     0,
       0,   215,   127,   132,   132,     0,     0,   216,     0,   132,
     159,   159,     0,     0,     0,     0,   159,   203,   119,    31,
      32,    33,     0,     0,     0,  1474,     0,   758,     0,     0,
      38,     0,     0,     0,     0,   127,     0,     0,     0,     0,
       0,  1487,   132,   758,   204,     0,     0,     0,     0,   159,
    1476,  1118,     0,     0,     0,     0,     0,     0,     0,   250,
       0,     0,     0,     0,     0,     0,     0,     0,   203,     0,
       0,   683,     0,     0,     0,     0,     0,    70,    71,    72,
      73,    74,   119,     0,   683,   683,   683,   119,   678,    11,
       0,     0,     0,     0,    77,    78,     0,   686,     0,   127,
       0,     0,     0,   203,   127,   203,   132,     0,    88,     0,
       0,   132,     0,   159,     0,     0,     0,     0,   159,     0,
       0,     0,     0,    93,     0,   203,   682,     0,     0,     0,
       0,     0,     0,     0,   686,     0,     0,   215,   215,   682,
     682,   682,     0,     0,     0,     0,     0,     0,  1119,     0,
       0,     0,     0,  1120,     0,    56,    57,    58,   171,   172,
     329,  1121,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   215,     0,     0,     0,     0,     0,     0,     0,     0,
     203,   683,     0,     0,     0,     0,     0,     0,     0,   215,
       0,   203,   203,     0,     0,     0,   204,     0,  1122,  1123,
       0,  1124,     0,     0,     0,     0,   215,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,   386,   215,
       0,     0,     0,    95,     0,     0,     0,     0,     0,     0,
     215,    34,     0,   200,     0,     0,   682,     0,     0,   215,
       0,     0,     0,     0,     0,     0,  1370,   204,     0,     0,
       0,   250,   250,   387,   388,     0,     0,   250,   215,     0,
     403,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,     0,     0,     0,   683,     0,   594,     0,   683,
       0,   683,   204,     0,   204,  -795,  -795,  -795,  -795,   355,
     356,   357,   358,   359,   360,   361,   362,     0,   363,   683,
       0,     0,   203,     0,   204,   686,   387,   388,     0,    82,
      83,     0,    84,    85,    86,   215,   389,   215,   686,   686,
     686,     0,     0,     0,     0,     0,     0,     0,   250,     0,
     682,   250,     0,     0,   682,    96,   682,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     884,     0,     0,     0,   682,     0,   595,   215,   114,   204,
      34,     0,     0,     0,     0,     0,     0,     0,   896,   389,
     204,   204,     0,     0,     0,     0,     0,     0,     0,   215,
       0,     0,   215,     0,     0,   896,     0,     0,     0,   203,
       0,     0,   683,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   683,     0,   683,     0,
       0,     0,     0,     0,     0,   686,     0,     0,   929,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   203,     0,     0,   291,     0,     0,   216,    82,    83,
       0,    84,    85,    86,   203,   203,    34,   682,   200,     0,
       0,     0,     0,     0,     0,     0,   683,   250,   215,     0,
       0,   682,   684,   682,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,   204,     0,     0,     0,   292,     0,   215,     0,     0,
     203,     0,     0,     0,     0,     0,     0,     0,     0,   684,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   686,
       0,   682,     0,   686,   683,   686,     0,     0,     0,   683,
       0,   683,     0,     0,    82,    83,     0,    84,    85,    86,
       0,     0,     0,   686,     0,     0,     0,     0,   250,   250,
       0,     0,     0,     0,     0,   250,     0,     0,     0,     0,
      96,   683,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   215,     0,     0,   204,   682,
       0,   620,     0,   114,   682,     0,   682,     0,     0,     0,
       0,   215,     0,     0,   215,   215,     0,   215,     0,   683,
       0,     0,     0,     0,   215,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   682,     0,     0,     0,
     204,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   204,   204,     0,   686,     0,     0,     0,
       0,   683,     0,     0,     0,     0,     0,     0,     0,     0,
     686,     0,   686,     0,   682,     0,     0,     0,     0,     0,
       0,     0,     0,   215,   215,     0,     0,     0,     0,   683,
     683,     0,     0,     0,     0,   683,  1117,     0,     0,   204,
     684,     0,     0,   338,   339,   340,     0,   215,     0,     0,
       0,   250,   250,   684,   684,   684,   682,     0,     0,   341,
     686,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,     0,   363,   682,   682,     0,     0,     0,     0,
     682,   215,   793,     0,     0,   215,     0,     0,     0,     0,
       0,    34,     0,   200,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   686,     0,
       0,     0,     0,   686,     0,   686,     0,     0,     0,     0,
       0,     0,     0,   250,  1192,     0,  1200,     0,     0,   683,
       0,   201,     0,     0,   250,     0,     0,     0,     0,     0,
     684,     0,     0,     0,     0,   686,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     683,     0,   175,    34,     0,    79,     0,    81,     0,    82,
      83,   683,    84,    85,    86,     0,   683,     0,     0,     0,
       0,     0,     0,   686,   682,     0,     0,     0,     0,     0,
       0,     0,  1253,  1254,     0,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,   215,     0,   202,     0,   682,     0,     0,   114,   250,
       0,   250,   764,     0,     0,   686,   682,     0,     0,     0,
       0,   682,     0,   683,   684,     0,     0,     0,   684,  1346,
     684,    82,    83,  1347,    84,    85,    86,     0,     0,     0,
       0,     0,     0,   686,   686,     0,     0,     0,   684,   686,
    1385,   250,     0,     0,  1200,     0,     0,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,   250,     0,  1205,   250,     0,   682,   683,
       0,     0,     0,     0,     0,   683,   215,     0,     0,   683,
       0,   683,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   215,     0,     0,     0,     0,     0,     0,     0,     0,
     215,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   215,     0,     0,     0,
       0,     0,     0,     0,   682,     0,     0,     0,     0,     0,
     682,   684,     0,     0,   682,     0,   682,     0,     0,     0,
       0,   687,   250,   686,     0,   684,     0,   684,     0,     0,
     179,   181,     0,   183,   184,   185,     0,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,     0,
       0,   209,   212,     0,   686,     0,     0,     0,   687,     0,
       0,     0,     0,     0,   230,   686,     0,     0,     0,     0,
     686,   238,     0,   241,     0,   684,   257,     0,   262,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,   688,   363,     0,     0,     0,
       0,     0,     0,     0,     0,   296,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   303,
       0,     0,     0,     0,     0,     0,     0,   686,     0,   250,
       0,     0,   713,   684,     0,  1484,   306,     0,   684,     0,
     684,     0,     0,     0,     0,   250,     0,     0,   250,     0,
    1192,     0,     0,     0,     0,     0,     0,     0,   250,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   242,     0,
     684,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   686,     0,     0,     0,     0,     0,   686,
       0,     0,     0,   686,   243,   686,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   404,   684,     0,
       0,     0,     0,     0,     0,    34,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   687,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   250,   687,   687,   687,     0,     0,     0,   428,     0,
     684,   428,     0,     0,     0,     0,     0,     0,   230,   439,
       0,     0,     0,     0,     0,     0,     0,     0,   244,   245,
       0,     0,     0,     0,     0,     0,     0,     0,   684,   684,
       0,     0,     0,     0,   684,     0,   175,     0,     0,    79,
       0,   246,     0,    82,    83,     0,    84,    85,    86,     0,
       0,     0,   306,   841,     0,     0,     0,     0,   209,     0,
       0,   247,   511,     0,     0,     0,   860,   861,     0,    96,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,   546,     0,   248,     0,   687,
       0,     0,     0,     0,     0,     0,   555,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   561,   562,   563,   565,   566,   567,   568,
     569,   570,   571,   572,   573,   574,   575,   576,   577,   578,
     579,   580,   581,   582,   583,   584,   585,   586,   684,     0,
     588,   588,     0,   591,     0,     0,     0,     0,     0,     0,
     607,   609,   610,   611,   612,   613,   614,   615,   616,   617,
     618,   619,     0,   926,     0,  1425,     0,   588,   624,   684,
     555,   588,   627,     0,     0,     0,     0,     0,   607,     0,
     684,     0,     0,   687,     0,   684,     0,   687,   639,   687,
     641,     0,     0,   338,   339,   340,   555,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   653,   687,   654,   341,
       0,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   362,     0,   363,     0,   692,   694,     0,   696,   699,
     700,     0,   684,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,   711,   363,  1008,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   250,     0,     0,     0,     0,     0,
       0,  1019,     0,     0,     0,     0,     0,     0,     0,     0,
     250,     0,     0,     0,     0,     0,   740,     0,   684,     0,
     687,     0,     0,     0,   684,     0,     0,     0,   684,     0,
     684,     0,     0,     0,   687,     0,   687,     0,     0,     0,
       0,     0,   338,   339,   340,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   341,   778,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,     0,   363,     0,   687,     0,     0,     0,     0,     0,
     786,     0,     0,     0,  1089,     0,     0,     0,     0,   296,
     338,   339,   340,     0,     0,  1029,     0,     0,  1101,     0,
    1102,     0,     0,     0,     0,   795,   341,     0,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,     0,
     363,   828,   687,     0,     0,     0,     0,   687,     0,   687,
       0,     0,     0,     0,   230,     0,     0,     0,  1142,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   338,   339,
     340,     0,     0,     0,     0,     0,     0,     0,     0,   687,
       0,     0,     0,     0,   341,   872,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,     0,   363,     0,
       0,     0,     0,     0,     0,     0,  1180,   687,     0,     0,
       0,  1182,     0,  1183,     0,     0,     0,     0,     0,     0,
     903,   799,    34,     0,   200,     0,     0,     0,     0,     0,
       0,     0,     0,   910,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1222,     0,     0,     0,     0,     0,   687,
       0,     0,     0,     0,   925,     0,     0,     0,     0,     0,
       0,     0,   201,     0,   933,     0,     0,   934,     0,   935,
       0,     0,     0,   555,   502,     0,     0,   687,   687,   802,
       0,  1248,   555,   687,     0,     0,     0,  1388,     0,     0,
       0,     0,     0,   175,     0,     0,    79,     0,    81,     0,
      82,    83,     0,    84,    85,    86,     0,     0,   967,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1360,   242,     0,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,     0,   202,     0,     0,   888,     0,   114,
     243,  1378,  1379,     0,     0,     0,     0,  1384,  1011,  1012,
    1013,     0,     0,     0,   696,  1015,     0,     0,     0,     0,
       0,    34,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1028,   687,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  -288,     0,
       0,     0,     0,     0,     0,  1047,    56,    57,    58,   171,
     172,   329,     0,     0,     0,     0,     0,     0,   687,     0,
       0,     0,   555,     0,   244,   245,     0,     0,     0,   687,
     555,     0,  1028,     0,   687,     0,     0,     0,     0,     0,
       0,     0,   175,     0,     0,    79,     0,   246,     0,    82,
      83,     0,    84,    85,    86,     0,     0,  1461,     0,     0,
       0,  1408,   230,     0,     0,     0,     0,   247,     0,     0,
       0,  1093,     0,     0,    95,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,   687,  1431,   248,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1441,     0,     0,     0,     0,  1445,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1136,
       0,     0,     0,  1137,     0,  1138,     0,     0,     0,     0,
       0,    34,     0,   200,   555,     0,     0,     0,     0,     0,
       0,  1151,   555,     0,     0,     0,     0,   687,     0,     0,
     555,     0,     0,   687,   338,   339,   340,   687,     0,   687,
       0,     0,     0,     0,     0,  1477,     0,     0,     0,     0,
     341,   201,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,     0,   363,     0,     0,     0,  1184,     0,
       0,     0,   175,     0,     0,    79,     0,    81,     0,    82,
      83,     0,    84,    85,    86,     5,     6,     7,     8,     9,
       0,  1526,     0,     0,     0,    10,     0,  1530,     0,   555,
       0,  1532,     0,  1533,     0,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,     0,   202,    11,    12,    13,     0,   114,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,    36,     0,    37,
       0,     0,     0,    38,    39,    40,    41,     0,    42,     0,
      43,     0,    44,     0,     0,    45,     0,     0,   555,    46,
      47,    48,    49,    50,    51,    52,     0,    53,    54,    55,
      56,    57,    58,    59,    60,    61,     0,    62,    63,    64,
      65,    66,    67,     0,     0,   880,  1380,    68,    69,     0,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,    75,     0,     0,     0,     0,    76,    77,    78,    79,
      80,    81,     0,    82,    83,     0,    84,    85,    86,    87,
       0,    88,     0,     0,     0,    89,     0,     0,     0,     0,
       0,    90,    91,     0,    92,     0,    93,    94,    95,    96,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,   111,     0,   112,
     113,   904,   114,   115,   340,   116,   117,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   341,     0,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,     0,   363,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,    36,
       0,    37,     0,     0,     0,    38,    39,    40,    41,     0,
      42,     0,    43,     0,    44,     0,     0,    45,     0,     0,
       0,    46,    47,    48,    49,    50,    51,    52,     0,    53,
      54,    55,    56,    57,    58,    59,    60,    61,     0,    62,
      63,    64,    65,    66,    67,     0,     0,     0,     0,    68,
      69,     0,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,    75,     0,     0,     0,     0,    76,    77,
      78,    79,    80,    81,     0,    82,    83,     0,    84,    85,
      86,    87,     0,    88,     0,     0,     0,    89,     0,     0,
       0,     0,     0,    90,    91,     0,    92,     0,    93,    94,
      95,    96,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,     0,     0,   111,
       0,   112,   113,  1030,   114,   115,     0,   116,   117,     5,
       6,     7,     8,     9,     0,     0,     0,     0,   341,    10,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,     0,   363,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,    36,     0,    37,     0,     0,     0,    38,    39,    40,
      41,     0,    42,     0,    43,     0,    44,     0,     0,    45,
       0,     0,     0,    46,    47,    48,    49,    50,    51,    52,
       0,    53,    54,    55,    56,    57,    58,    59,    60,    61,
       0,    62,    63,    64,    65,    66,    67,     0,     0,     0,
       0,    68,    69,     0,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,    75,     0,     0,     0,     0,
      76,    77,    78,    79,    80,    81,     0,    82,    83,     0,
      84,    85,    86,    87,     0,    88,     0,     0,     0,    89,
       0,     0,     0,     0,     0,    90,    91,     0,    92,     0,
      93,    94,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,   112,   113,     0,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,     0,   363,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,    36,     0,    37,     0,     0,     0,    38,
      39,    40,    41,     0,    42,     0,    43,     0,    44,     0,
       0,    45,     0,     0,     0,    46,    47,    48,    49,     0,
      51,    52,     0,    53,     0,    55,    56,    57,    58,    59,
      60,    61,     0,    62,    63,    64,     0,    66,    67,     0,
       0,     0,     0,    68,    69,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,    75,     0,     0,
       0,     0,   175,    77,    78,    79,    80,    81,     0,    82,
      83,     0,    84,    85,    86,    87,     0,    88,     0,     0,
       0,    89,     0,     0,     0,     0,     0,    90,     0,     0,
       0,     0,    93,    94,    95,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,     0,   111,     0,   112,   113,   540,   114,   115,
       0,   116,   117,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,     0,   363,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,    36,     0,    37,     0,     0,
       0,    38,    39,    40,    41,     0,    42,     0,    43,     0,
      44,     0,     0,    45,     0,     0,     0,    46,    47,    48,
      49,     0,    51,    52,     0,    53,     0,    55,    56,    57,
      58,    59,    60,    61,     0,    62,    63,    64,     0,    66,
      67,     0,     0,     0,     0,    68,    69,     0,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,    75,
       0,     0,     0,     0,   175,    77,    78,    79,    80,    81,
       0,    82,    83,     0,    84,    85,    86,    87,     0,    88,
       0,     0,     0,    89,     0,     0,     0,     0,     0,    90,
       0,     0,     0,     0,    93,    94,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,   112,   113,   883,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,     0,   363,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,    36,     0,    37,
       0,     0,     0,    38,    39,    40,    41,     0,    42,     0,
      43,     0,    44,     0,     0,    45,     0,     0,     0,    46,
      47,    48,    49,     0,    51,    52,     0,    53,     0,    55,
      56,    57,    58,    59,    60,    61,     0,    62,    63,    64,
       0,    66,    67,     0,     0,     0,     0,    68,    69,     0,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,    75,     0,     0,     0,     0,   175,    77,    78,    79,
      80,    81,     0,    82,    83,     0,    84,    85,    86,    87,
       0,    88,     0,     0,     0,    89,     0,     0,     0,     0,
       0,    90,     0,     0,     0,     0,    93,    94,    95,    96,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,   111,     0,   112,
     113,   974,   114,   115,     0,   116,   117,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,     0,   363,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,    36,
       0,    37,     0,     0,     0,    38,    39,    40,    41,   976,
      42,     0,    43,     0,    44,     0,     0,    45,     0,     0,
       0,    46,    47,    48,    49,     0,    51,    52,     0,    53,
       0,    55,    56,    57,    58,    59,    60,    61,     0,    62,
      63,    64,     0,    66,    67,     0,     0,     0,     0,    68,
      69,     0,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,    75,     0,     0,     0,     0,   175,    77,
      78,    79,    80,    81,     0,    82,    83,     0,    84,    85,
      86,    87,     0,    88,     0,     0,     0,    89,     0,     0,
       0,     0,     0,    90,     0,     0,     0,     0,    93,    94,
      95,    96,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,     0,     0,   111,
       0,   112,   113,     0,   114,   115,     0,   116,   117,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
    -795,  -795,  -795,  -795,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,     0,   363,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,    36,     0,    37,     0,     0,     0,    38,    39,    40,
      41,     0,    42,     0,    43,     0,    44,  1090,     0,    45,
       0,     0,     0,    46,    47,    48,    49,     0,    51,    52,
       0,    53,     0,    55,    56,    57,    58,    59,    60,    61,
       0,    62,    63,    64,     0,    66,    67,     0,     0,     0,
       0,    68,    69,     0,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,    75,     0,     0,     0,     0,
     175,    77,    78,    79,    80,    81,     0,    82,    83,     0,
      84,    85,    86,    87,     0,    88,     0,     0,     0,    89,
       0,     0,     0,     0,     0,    90,     0,     0,     0,     0,
      93,    94,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,   112,   113,     0,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,    36,     0,    37,     0,     0,     0,    38,
      39,    40,    41,     0,    42,     0,    43,     0,    44,     0,
       0,    45,     0,     0,     0,    46,    47,    48,    49,     0,
      51,    52,     0,    53,     0,    55,    56,    57,    58,    59,
      60,    61,     0,    62,    63,    64,     0,    66,    67,     0,
       0,     0,     0,    68,    69,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,    75,     0,     0,
       0,     0,   175,    77,    78,    79,    80,    81,     0,    82,
      83,     0,    84,    85,    86,    87,     0,    88,     0,     0,
       0,    89,     0,     0,     0,     0,     0,    90,     0,     0,
       0,     0,    93,    94,    95,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,     0,   111,     0,   112,   113,  1186,   114,   115,
       0,   116,   117,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,    36,     0,    37,     0,     0,
       0,    38,    39,    40,    41,     0,    42,     0,    43,     0,
      44,     0,     0,    45,     0,     0,     0,    46,    47,    48,
      49,     0,    51,    52,     0,    53,     0,    55,    56,    57,
      58,    59,    60,    61,     0,    62,    63,    64,     0,    66,
      67,     0,     0,     0,     0,    68,    69,     0,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,    75,
       0,     0,     0,     0,   175,    77,    78,    79,    80,    81,
       0,    82,    83,     0,    84,    85,    86,    87,     0,    88,
       0,     0,     0,    89,     0,     0,     0,     0,     0,    90,
       0,     0,     0,     0,    93,    94,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,   112,   113,  1381,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,    36,     0,    37,
       0,     0,     0,    38,    39,    40,    41,     0,    42,     0,
      43,  1420,    44,     0,     0,    45,     0,     0,     0,    46,
      47,    48,    49,     0,    51,    52,     0,    53,     0,    55,
      56,    57,    58,    59,    60,    61,     0,    62,    63,    64,
       0,    66,    67,     0,     0,     0,     0,    68,    69,     0,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,    75,     0,     0,     0,     0,   175,    77,    78,    79,
      80,    81,     0,    82,    83,     0,    84,    85,    86,    87,
       0,    88,     0,     0,     0,    89,     0,     0,     0,     0,
       0,    90,     0,     0,     0,     0,    93,    94,    95,    96,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,   111,     0,   112,
     113,     0,   114,   115,     0,   116,   117,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,    36,
       0,    37,     0,     0,     0,    38,    39,    40,    41,     0,
      42,     0,    43,     0,    44,     0,     0,    45,     0,     0,
       0,    46,    47,    48,    49,     0,    51,    52,     0,    53,
       0,    55,    56,    57,    58,    59,    60,    61,     0,    62,
      63,    64,     0,    66,    67,     0,     0,     0,     0,    68,
      69,     0,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,    75,     0,     0,     0,     0,   175,    77,
      78,    79,    80,    81,     0,    82,    83,     0,    84,    85,
      86,    87,     0,    88,     0,     0,     0,    89,     0,     0,
       0,     0,     0,    90,     0,     0,     0,     0,    93,    94,
      95,    96,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,     0,     0,   111,
       0,   112,   113,  1450,   114,   115,     0,   116,   117,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,    36,     0,    37,     0,     0,     0,    38,    39,    40,
      41,     0,    42,     0,    43,     0,    44,     0,     0,    45,
       0,     0,     0,    46,    47,    48,    49,     0,    51,    52,
       0,    53,     0,    55,    56,    57,    58,    59,    60,    61,
       0,    62,    63,    64,     0,    66,    67,     0,     0,     0,
       0,    68,    69,     0,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,    75,     0,     0,     0,     0,
     175,    77,    78,    79,    80,    81,     0,    82,    83,     0,
      84,    85,    86,    87,     0,    88,     0,     0,     0,    89,
       0,     0,     0,     0,     0,    90,     0,     0,     0,     0,
      93,    94,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,   112,   113,  1451,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,    36,     0,    37,     0,     0,     0,    38,
      39,    40,    41,     0,    42,  1454,    43,     0,    44,     0,
       0,    45,     0,     0,     0,    46,    47,    48,    49,     0,
      51,    52,     0,    53,     0,    55,    56,    57,    58,    59,
      60,    61,     0,    62,    63,    64,     0,    66,    67,     0,
       0,     0,     0,    68,    69,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,    75,     0,     0,
       0,     0,   175,    77,    78,    79,    80,    81,     0,    82,
      83,     0,    84,    85,    86,    87,     0,    88,     0,     0,
       0,    89,     0,     0,     0,     0,     0,    90,     0,     0,
       0,     0,    93,    94,    95,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,     0,   111,     0,   112,   113,     0,   114,   115,
       0,   116,   117,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,    36,     0,    37,     0,     0,
       0,    38,    39,    40,    41,     0,    42,     0,    43,     0,
      44,     0,     0,    45,     0,     0,     0,    46,    47,    48,
      49,     0,    51,    52,     0,    53,     0,    55,    56,    57,
      58,    59,    60,    61,     0,    62,    63,    64,     0,    66,
      67,     0,     0,     0,     0,    68,    69,     0,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,    75,
       0,     0,     0,     0,   175,    77,    78,    79,    80,    81,
       0,    82,    83,     0,    84,    85,    86,    87,     0,    88,
       0,     0,     0,    89,     0,     0,     0,     0,     0,    90,
       0,     0,     0,     0,    93,    94,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,   112,   113,  1469,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,    36,     0,    37,
       0,     0,     0,    38,    39,    40,    41,     0,    42,     0,
      43,     0,    44,     0,     0,    45,     0,     0,     0,    46,
      47,    48,    49,     0,    51,    52,     0,    53,     0,    55,
      56,    57,    58,    59,    60,    61,     0,    62,    63,    64,
       0,    66,    67,     0,     0,     0,     0,    68,    69,     0,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,    75,     0,     0,     0,     0,   175,    77,    78,    79,
      80,    81,     0,    82,    83,     0,    84,    85,    86,    87,
       0,    88,     0,     0,     0,    89,     0,     0,     0,     0,
       0,    90,     0,     0,     0,     0,    93,    94,    95,    96,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,   111,     0,   112,
     113,  1522,   114,   115,     0,   116,   117,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,    36,
       0,    37,     0,     0,     0,    38,    39,    40,    41,     0,
      42,     0,    43,     0,    44,     0,     0,    45,     0,     0,
       0,    46,    47,    48,    49,     0,    51,    52,     0,    53,
       0,    55,    56,    57,    58,    59,    60,    61,     0,    62,
      63,    64,     0,    66,    67,     0,     0,     0,     0,    68,
      69,     0,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,    75,     0,     0,     0,     0,   175,    77,
      78,    79,    80,    81,     0,    82,    83,     0,    84,    85,
      86,    87,     0,    88,     0,     0,     0,    89,     0,     0,
       0,     0,     0,    90,     0,     0,     0,     0,    93,    94,
      95,    96,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,     0,     0,   111,
       0,   112,   113,  1527,   114,   115,     0,   116,   117,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,    36,     0,    37,     0,     0,     0,    38,    39,    40,
      41,     0,    42,     0,    43,     0,    44,     0,     0,    45,
       0,     0,     0,    46,    47,    48,    49,     0,    51,    52,
       0,    53,     0,    55,    56,    57,    58,    59,    60,    61,
       0,    62,    63,    64,     0,    66,    67,     0,     0,     0,
       0,    68,    69,     0,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,    75,     0,     0,     0,     0,
     175,    77,    78,    79,    80,    81,     0,    82,    83,     0,
      84,    85,    86,    87,     0,    88,     0,     0,     0,    89,
       0,     0,     0,     0,     0,    90,     0,     0,     0,     0,
      93,    94,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,   112,   113,     0,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   429,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,    36,     0,    37,     0,     0,     0,    38,
      39,    40,    41,     0,    42,     0,    43,     0,    44,     0,
       0,    45,     0,     0,     0,    46,    47,    48,    49,     0,
      51,    52,     0,    53,     0,    55,    56,    57,    58,   171,
     172,    61,     0,    62,    63,    64,     0,     0,     0,     0,
       0,     0,     0,    68,    69,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,    75,     0,     0,
       0,     0,   175,    77,    78,    79,    80,    81,     0,    82,
      83,     0,    84,    85,    86,     0,     0,    88,     0,     0,
       0,    89,     0,     0,     0,     0,     0,    90,     0,     0,
       0,     0,    93,    94,    95,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,     0,   111,     0,   112,   113,     0,   114,   115,
       0,   116,   117,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   656,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,    36,     0,    37,     0,     0,
       0,    38,    39,    40,    41,     0,    42,     0,    43,     0,
      44,     0,     0,    45,     0,     0,     0,    46,    47,    48,
      49,     0,    51,    52,     0,    53,     0,    55,    56,    57,
      58,   171,   172,    61,     0,    62,    63,    64,     0,     0,
       0,     0,     0,     0,     0,    68,    69,     0,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,    75,
       0,     0,     0,     0,   175,    77,    78,    79,    80,    81,
       0,    82,    83,     0,    84,    85,    86,     0,     0,    88,
       0,     0,     0,    89,     0,     0,     0,     0,     0,    90,
       0,     0,     0,     0,    93,    94,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,   112,   113,     0,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   843,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,    36,     0,    37,
       0,     0,     0,    38,    39,    40,    41,     0,    42,     0,
      43,     0,    44,     0,     0,    45,     0,     0,     0,    46,
      47,    48,    49,     0,    51,    52,     0,    53,     0,    55,
      56,    57,    58,   171,   172,    61,     0,    62,    63,    64,
       0,     0,     0,     0,     0,     0,     0,    68,    69,     0,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,    75,     0,     0,     0,     0,   175,    77,    78,    79,
      80,    81,     0,    82,    83,     0,    84,    85,    86,     0,
       0,    88,     0,     0,     0,    89,     0,     0,     0,     0,
       0,    90,     0,     0,     0,     0,    93,    94,    95,    96,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,   111,     0,   112,
     113,     0,   114,   115,     0,   116,   117,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1243,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,    36,
       0,    37,     0,     0,     0,    38,    39,    40,    41,     0,
      42,     0,    43,     0,    44,     0,     0,    45,     0,     0,
       0,    46,    47,    48,    49,     0,    51,    52,     0,    53,
       0,    55,    56,    57,    58,   171,   172,    61,     0,    62,
      63,    64,     0,     0,     0,     0,     0,     0,     0,    68,
      69,     0,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,    75,     0,     0,     0,     0,   175,    77,
      78,    79,    80,    81,     0,    82,    83,     0,    84,    85,
      86,     0,     0,    88,     0,     0,     0,    89,     0,     0,
       0,     0,     0,    90,     0,     0,     0,     0,    93,    94,
      95,    96,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,     0,     0,   111,
       0,   112,   113,     0,   114,   115,     0,   116,   117,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1374,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,    36,     0,    37,     0,     0,     0,    38,    39,    40,
      41,     0,    42,     0,    43,     0,    44,     0,     0,    45,
       0,     0,     0,    46,    47,    48,    49,     0,    51,    52,
       0,    53,     0,    55,    56,    57,    58,   171,   172,    61,
       0,    62,    63,    64,     0,     0,     0,     0,     0,     0,
       0,    68,    69,     0,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,    75,     0,     0,     0,     0,
     175,    77,    78,    79,    80,    81,     0,    82,    83,     0,
      84,    85,    86,     0,     0,    88,     0,     0,     0,    89,
       0,     0,     0,     0,     0,    90,     0,     0,     0,     0,
      93,    94,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,   112,   113,     0,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,    36,     0,    37,     0,     0,     0,    38,
      39,    40,    41,     0,    42,     0,    43,     0,    44,     0,
       0,    45,     0,     0,     0,    46,    47,    48,    49,     0,
      51,    52,     0,    53,     0,    55,    56,    57,    58,   171,
     172,    61,     0,    62,    63,    64,     0,     0,     0,     0,
       0,     0,     0,    68,    69,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,    75,     0,     0,
       0,     0,   175,    77,    78,    79,    80,    81,     0,    82,
      83,     0,    84,    85,    86,     0,     0,    88,     0,     0,
       0,    89,     0,     0,     0,     0,     0,    90,     0,     0,
       0,     0,    93,    94,    95,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,     0,   111,     0,   112,   113,     0,   114,   115,
       0,   116,   117,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   603,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,    36,     0,     0,     0,     0,
       0,    38,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,    56,    57,
      58,   171,   172,   173,     0,    34,    63,    64,     0,     0,
       0,     0,     0,     0,     0,   174,    69,     0,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,    75,
       0,     0,     0,     0,   175,    77,    78,    79,   604,    81,
       0,    82,    83,     0,    84,    85,    86,     0,     0,    88,
       0,     0,     0,    89,     0,     0,     0,     0,     0,    90,
       0,     0,     0,     0,    93,    94,    95,    96,   254,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,    82,    83,   111,    84,    85,    86,     0,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,    96,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,     0,   557,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,    34,    35,    36,     0,     0,
       0,     0,     0,    38,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    49,     0,     0,     0,     0,     0,     0,     0,
      56,    57,    58,   171,   172,   173,     0,     0,    63,    64,
       0,     0,     0,     0,     0,     0,     0,   174,    69,     0,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,    75,     0,     0,     0,     0,   175,    77,    78,    79,
       0,    81,     0,    82,    83,     0,    84,    85,    86,     0,
       0,    88,     0,     0,     0,    89,     0,     0,     0,     0,
       0,    90,     0,     0,     0,     0,    93,    94,    95,    96,
     254,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,   111,     0,   255,
       0,     0,   114,   115,     0,   116,   117,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,    35,    36,
       0,     0,     0,     0,     0,    38,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,    56,    57,    58,   171,   172,   173,     0,    34,
      63,    64,     0,     0,     0,     0,     0,     0,     0,   174,
      69,     0,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,    75,     0,     0,     0,     0,   175,    77,
      78,    79,   604,    81,     0,    82,    83,     0,    84,    85,
      86,     0,     0,    88,     0,     0,     0,    89,     0,     0,
       0,     0,     0,    90,     0,     0,     0,     0,    93,    94,
      95,    96,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,    82,    83,   111,
      84,    85,    86,     0,   114,   115,     0,   116,   117,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   208,     0,
       0,   776,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,    36,     0,     0,     0,     0,     0,    38,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    49,     0,     0,     0,
       0,     0,     0,     0,    56,    57,    58,   171,   172,   173,
       0,    34,    63,    64,     0,     0,     0,     0,     0,     0,
       0,   174,    69,     0,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,    75,     0,     0,     0,     0,
     175,    77,    78,    79,     0,    81,     0,    82,    83,     0,
      84,    85,    86,     0,     0,    88,     0,  1006,     0,    89,
       0,     0,     0,     0,     0,    90,     0,     0,     0,     0,
      93,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,    82,
      83,   111,    84,    85,    86,     0,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,    36,     0,     0,     0,     0,     0,    38,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    49,     0,
       0,     0,     0,     0,     0,     0,    56,    57,    58,   171,
     172,   173,     0,     0,    63,    64,     0,     0,     0,     0,
       0,     0,     0,   174,    69,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,    75,     0,     0,
       0,     0,   175,    77,    78,    79,     0,    81,     0,    82,
      83,     0,    84,    85,    86,     0,     0,    88,     0,     0,
       0,    89,     0,     0,     0,     0,     0,    90,     0,     0,
       0,     0,    93,     0,    95,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,     0,   111,     0,   237,     0,     0,   114,   115,
       0,   116,   117,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,    36,     0,     0,     0,     0,
       0,    38,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,    56,    57,
      58,   171,   172,   173,     0,     0,    63,    64,     0,     0,
       0,     0,     0,     0,     0,   174,    69,     0,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,    75,
       0,     0,     0,     0,   175,    77,    78,    79,     0,    81,
       0,    82,    83,     0,    84,    85,    86,     0,     0,    88,
       0,     0,     0,    89,     0,     0,     0,     0,     0,    90,
       0,     0,     0,     0,    93,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,   240,     0,     0,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,    34,    35,    36,     0,     0,
       0,     0,     0,    38,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   295,
       0,     0,    49,     0,     0,     0,     0,     0,     0,     0,
      56,    57,    58,   171,   172,   173,     0,    34,    63,    64,
       0,     0,     0,     0,     0,     0,     0,   174,    69,     0,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,    75,     0,     0,     0,     0,   175,    77,    78,    79,
       0,    81,     0,    82,    83,     0,    84,    85,    86,     0,
       0,    88,     0,     0,     0,    89,     0,     0,     0,     0,
       0,    90,     0,     0,     0,     0,    93,     0,    95,    96,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,    82,    83,   111,    84,    85,
      86,     0,   114,   115,     0,   116,   117,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,    96,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,    35,    36,
       0,     0,     0,     0,     0,    38,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,    56,    57,    58,   171,   172,   173,     0,     0,
      63,    64,     0,     0,     0,     0,     0,     0,     0,   174,
      69,     0,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,    75,     0,     0,     0,     0,   175,    77,
      78,    79,     0,    81,     0,    82,    83,     0,    84,    85,
      86,     0,     0,    88,     0,     0,     0,    89,     0,     0,
       0,     0,     0,    90,     0,     0,     0,     0,    93,     0,
      95,    96,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,     0,     0,   111,
     427,     0,     0,     0,   114,   115,     0,   116,   117,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   552,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,    36,     0,     0,     0,     0,     0,    38,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    49,     0,     0,     0,
       0,     0,     0,     0,    56,    57,    58,   171,   172,   173,
       0,     0,    63,    64,     0,     0,     0,     0,     0,     0,
       0,   174,    69,     0,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,    75,     0,     0,     0,     0,
     175,    77,    78,    79,     0,    81,     0,    82,    83,     0,
      84,    85,    86,     0,     0,    88,     0,     0,     0,    89,
       0,     0,     0,     0,     0,    90,     0,     0,     0,     0,
      93,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,     0,     0,     0,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   564,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,    36,     0,     0,     0,     0,     0,    38,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    49,     0,
       0,     0,     0,     0,     0,     0,    56,    57,    58,   171,
     172,   173,     0,     0,    63,    64,     0,     0,     0,     0,
       0,     0,     0,   174,    69,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,    75,     0,     0,
       0,     0,   175,    77,    78,    79,     0,    81,     0,    82,
      83,     0,    84,    85,    86,     0,     0,    88,     0,     0,
       0,    89,     0,     0,     0,     0,     0,    90,     0,     0,
       0,     0,    93,     0,    95,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,     0,   111,     0,     0,     0,     0,   114,   115,
       0,   116,   117,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   603,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,    36,     0,     0,     0,     0,
       0,    38,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,    56,    57,
      58,   171,   172,   173,     0,     0,    63,    64,     0,     0,
       0,     0,     0,     0,     0,   174,    69,     0,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,    75,
       0,     0,     0,     0,   175,    77,    78,    79,     0,    81,
       0,    82,    83,     0,    84,    85,    86,     0,     0,    88,
       0,     0,     0,    89,     0,     0,     0,     0,     0,    90,
       0,     0,     0,     0,    93,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,     0,     0,     0,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   638,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,    34,    35,    36,     0,     0,
       0,     0,     0,    38,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    49,     0,     0,     0,     0,     0,     0,     0,
      56,    57,    58,   171,   172,   173,     0,     0,    63,    64,
       0,     0,     0,     0,     0,     0,     0,   174,    69,     0,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,    75,     0,     0,     0,     0,   175,    77,    78,    79,
       0,    81,     0,    82,    83,     0,    84,    85,    86,     0,
       0,    88,     0,     0,     0,    89,     0,     0,     0,     0,
       0,    90,     0,     0,     0,     0,    93,     0,    95,    96,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,   111,     0,     0,
       0,     0,   114,   115,     0,   116,   117,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   640,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,    35,    36,
       0,     0,     0,     0,     0,    38,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,    56,    57,    58,   171,   172,   173,     0,     0,
      63,    64,     0,     0,     0,     0,     0,     0,     0,   174,
      69,     0,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,    75,     0,     0,     0,     0,   175,    77,
      78,    79,     0,    81,     0,    82,    83,     0,    84,    85,
      86,     0,     0,    88,     0,     0,     0,    89,     0,     0,
       0,     0,     0,    90,     0,     0,     0,     0,    93,     0,
      95,    96,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,     0,     0,   111,
       0,     0,     0,     0,   114,   115,     0,   116,   117,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,    36,     0,     0,     0,     0,     0,    38,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    49,     0,     0,     0,
       0,     0,     0,     0,    56,    57,    58,   171,   172,   173,
       0,     0,    63,    64,     0,     0,     0,     0,     0,     0,
       0,   174,    69,     0,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,    75,     0,     0,     0,     0,
     175,    77,    78,    79,     0,    81,     0,    82,    83,     0,
      84,    85,    86,     0,     0,    88,     0,     0,     0,    89,
       0,     0,     0,     0,     0,    90,     0,     0,     0,     0,
      93,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,     0,     0,   652,     0,   114,   115,     0,   116,
     117,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     924,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,    36,     0,     0,     0,     0,     0,    38,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    49,     0,
       0,     0,     0,     0,     0,     0,    56,    57,    58,   171,
     172,   173,     0,     0,    63,    64,     0,     0,     0,     0,
       0,     0,     0,   174,    69,     0,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,    75,     0,     0,
       0,     0,   175,    77,    78,    79,     0,    81,     0,    82,
      83,     0,    84,    85,    86,     0,     0,    88,     0,     0,
       0,    89,     0,     0,     0,     0,     0,    90,     0,     0,
       0,     0,    93,     0,    95,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,     0,   111,     0,     0,     0,     0,   114,   115,
       0,   116,   117,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   966,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,    36,     0,     0,     0,     0,
       0,    38,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,     0,    56,    57,
      58,   171,   172,   173,     0,     0,    63,    64,     0,     0,
       0,     0,     0,     0,     0,   174,    69,     0,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,    75,
       0,     0,     0,     0,   175,    77,    78,    79,     0,    81,
       0,    82,    83,     0,    84,    85,    86,     0,     0,    88,
       0,     0,     0,    89,     0,     0,     0,     0,     0,    90,
       0,     0,     0,     0,    93,     0,    95,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,     0,   111,     0,     0,     0,     0,
     114,   115,     0,   116,   117,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,    34,    35,    36,     0,     0,
       0,     0,     0,    38,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    49,     0,     0,     0,     0,     0,     0,     0,
      56,    57,    58,   171,   172,   173,     0,     0,    63,    64,
       0,     0,     0,     0,     0,     0,     0,   174,    69,     0,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,    75,     0,     0,     0,     0,   175,    77,    78,    79,
       0,    81,     0,    82,    83,     0,    84,    85,    86,     0,
       0,    88,     0,     0,     0,    89,     0,     0,     0,     0,
       0,    90,     0,     0,     0,     0,    93,     0,    95,    96,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,     0,   111,     0,     0,
       0,     0,   114,   115,     0,   116,   117,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,   510,    36,
       0,     0,     0,     0,     0,    38,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    49,     0,     0,     0,     0,     0,
       0,     0,    56,    57,    58,   171,   172,   173,     0,     0,
      63,    64,     0,     0,     0,     0,     0,     0,     0,   174,
      69,     0,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,    75,     0,     0,     0,     0,   175,    77,
      78,    79,     0,    81,     0,    82,    83,     0,    84,    85,
      86,     0,     0,    88,     0,     0,     0,    89,     0,     0,
       0,     0,     0,    90,     0,     0,     0,     0,    93,     0,
      95,    96,     0,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,     0,     0,   111,
       0,     0,     0,     0,   114,   115,     0,   116,   117,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,   693,     0,     0,     0,     0,     0,    38,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    49,     0,     0,     0,
       0,     0,     0,     0,    56,    57,    58,   171,   172,   173,
       0,     0,    63,    64,     0,     0,     0,     0,     0,     0,
       0,   174,    69,     0,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,    75,     0,     0,     0,     0,
     175,    77,    78,    79,     0,    81,     0,    82,    83,     0,
      84,    85,    86,     0,     0,    88,     0,     0,     0,    89,
       0,     0,     0,     0,     0,    90,     0,     0,     0,     0,
      93,     0,    95,    96,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
       0,   111,   338,   339,   340,     0,   114,   115,     0,   116,
     117,     0,     0,     0,     0,     0,     0,     0,   341,     0,
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
     360,   361,   362,     0,   363,   338,   339,   340,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   341,     0,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,     0,   363,     0,     0,     0,     0,
       0,     0,     0,     0,   338,   339,   340,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     341,   936,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,     0,   363,   338,   339,   340,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   341,   944,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,     0,   363,     0,     0,     0,   338,
     339,   340,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   968,     0,   341,  1094,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,     0,   363,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1143,     0,     0,     0,     0,     0,
       0,     0,   338,   339,   340,     0,  1262,  1263,  1264,  1265,
    1266,     0,     0,  1267,  1268,  1269,  1270,     0,   341,     0,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
     362,     0,   363,  1218,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1271,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1272,  1273,  1274,  1275,
    1276,  1277,  1278,     0,     0,     0,    34,     0,     0,     0,
       0,     0,     0,     0,  1219,  1279,  1280,  1281,  1282,  1283,
    1284,  1285,  1286,  1287,  1288,  1289,  1290,  1291,  1292,  1293,
    1294,  1295,  1296,  1297,  1298,  1299,  1300,  1301,  1302,  1303,
    1304,  1305,  1306,  1307,  1308,  1309,  1310,  1311,  1312,  1313,
    1314,  1315,  1316,  1317,  1318,  1319,  1095,     0,  1320,  1321,
       0,  1322,  1323,  1324,  1325,  1326,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1327,  1328,  1329,
       0,  1330,     0,     0,    82,    83,     0,    84,    85,    86,
    1331,  1332,  1333,     0,     0,  1334,     0,     0,     0,     0,
       0,     0,  1335,  1336,     0,  1337,  1421,  1338,  1339,  1340,
      96,     0,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   338,   339,   340,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,   338,   339,   340,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   341,   364,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,     0,   363,   338,   339,   340,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   341,   443,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   362,     0,   363,   338,   339,   340,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   341,   445,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,     0,   363,   338,   339,
     340,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   341,   457,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,     0,   363,     0,
       0,     0,     0,     0,     0,   338,   339,   340,     0,     0,
       0,     0,    34,     0,   200,     0,     0,     0,     0,     0,
       0,   341,   481,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,     0,   363,     0,   338,   339,   340,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   630,   341,     0,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,   242,   363,     0,     0,
      82,    83,     0,    84,    85,    86,     0,     0,     0,     0,
       0,     0,     0,   649,     0,     0,     0,     0,     0,     0,
       0,     0,   243,     0,     0,     0,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,    34,     0,   876,   877,   595,     0,   114,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   948,
     949,   950,    34,   242,     0,     0,     0,     0,     0,     0,
     450,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1251,   243,
       0,     0,     0,     0,     0,     0,   244,   245,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      34,     0,     0,     0,   175,     0,     0,    79,     0,   246,
       0,    82,    83,     0,    84,    85,    86,     0,     0,  1113,
       0,     0,     0,     0,     0,     0,   242,     0,     0,   247,
      82,    83,     0,    84,    85,    86,     0,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   243,   244,   245,   248,    96,     0,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   175,     0,    34,    79,     0,   246,     0,    82,    83,
       0,    84,    85,    86,     0,     0,     0,   856,     0,     0,
       0,     0,     0,     0,     0,     0,   247,     0,     0,     0,
       0,     0,     0,     0,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
     837,     0,   248,     0,     0,     0,   244,   245,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   175,     0,     0,    79,     0,   246,
       0,    82,    83,     0,    84,    85,    86,     0,     0,     0,
    1099,    34,     0,   200,     0,     0,     0,     0,     0,   247,
       0,     0,     0,     0,     0,     0,     0,    96,     0,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,     0,   248,     0,     0,     0,     0,
       0,   201,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   838,     0,     0,     0,     0,     0,   674,
     675,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   175,     0,     0,    79,     0,    81,   676,    82,
      83,     0,    84,    85,    86,     0,    31,    32,    33,    34,
       0,     0,     0,     0,     0,     0,     0,    38,     0,     0,
       0,     0,     0,     0,     0,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,     0,   202,     0,     0,     0,     0,   114,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   677,     0,    70,    71,    72,    73,    74,     0,
     804,   805,     0,     0,     0,   678,     0,     0,     0,     0,
     175,    77,    78,    79,     0,   679,     0,    82,    83,   806,
      84,    85,    86,     0,     0,    88,     0,   807,   808,   809,
      34,     0,     0,     0,     0,   680,     0,     0,   810,     0,
      93,     0,     0,   681,     0,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    27,    28,
       0,     0,     0,     0,     0,     0,     0,     0,    34,     0,
     200,     0,     0,   811,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   812,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    82,    83,
       0,    84,    85,    86,     0,     0,     0,     0,   201,     0,
       0,     0,     0,     0,     0,     0,   813,     0,     0,     0,
      34,     0,   200,     0,   814,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   175,
       0,     0,    79,     0,    81,     0,    82,    83,     0,    84,
      85,    86,     0,     0,     0,     0,     0,     0,    89,     0,
     201,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,     0,
     408,   175,     0,     0,    79,   114,    81,     0,    82,    83,
       0,    84,    85,    86,    34,     0,   200,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,     0,   202,     0,   201,   486,     0,   114,     0,     0,
       0,     0,     0,     0,     0,     0,   899,     0,    34,     0,
     200,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   175,     0,     0,    79,     0,
      81,     0,    82,    83,     0,    84,    85,    86,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   213,     0,
      34,     0,   200,     0,     0,     0,     0,     0,    96,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,     0,   202,     0,     0,   175,
       0,   114,    79,     0,    81,     0,    82,    83,     0,    84,
      85,    86,     0,     0,     0,     0,     0,     0,     0,     0,
      34,     0,   200,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    96,     0,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,    82,    83,
     214,    84,    85,    86,     0,   114,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      34,     0,     0,     0,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,     0,     0,     0,     0,   646,     0,   114,    82,    83,
       0,    84,    85,    86,     0,     0,     0,     0,     0,     0,
       0,    34,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,   175,     0,     0,    79,   920,     0,   114,    82,    83,
       0,    84,    85,    86,     0,  1204,     0,     0,     0,     0,
      34,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    96,    34,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,    82,
      83,     0,    84,    85,    86,     0,  1423,     0,     0,     0,
       0,  1193,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1194,     0,    96,     0,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
      34,   175,     0,  1205,    79,     0,  1195,     0,    82,    83,
       0,    84,  1196,    86,     0,    34,   175,   730,   731,    79,
       0,    81,     0,    82,    83,     0,    84,    85,    86,     0,
       0,     0,     0,     0,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,    96,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,    34,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   263,     0,     0,     0,    82,    83,
       0,    84,    85,    86,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    82,    83,     0,    84,    85,    86,     0,
       0,     0,     0,     0,    96,     0,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,    96,
       0,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,     0,     0,     0,     0,
     326,     0,    82,    83,     0,    84,    85,    86,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    96,     0,
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
       0,     0,     0,   341,   782,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   362,     0,   363,   338,   339,
     340,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   341,   823,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,     0,   363,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   338,   339,   340,     0,     0,
       0,  1056,     0,     0,     0,     0,     0,     0,     0,     0,
     660,   341,   779,   342,   343,   344,   345,   346,   347,   348,
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
  ((yystate) == (-1016))

#define yytable_value_is_error(yytable_value) \
  ((yytable_value) == (-795))

static const yytype_int16 yycheck[] =
{
       4,   132,   157,   177,     4,     4,    87,     4,   322,   554,
      91,    92,     4,   407,   917,    30,   215,     4,    51,   395,
     375,   304,   400,   756,     4,   363,    41,   160,   429,   772,
      45,     4,   631,   221,    28,   319,  1051,    54,   221,   659,
     167,   517,   749,    47,     9,   421,    50,   178,   111,   907,
     131,    24,    25,     9,   111,     9,   665,    75,     9,    76,
       9,     4,    79,    67,     9,    27,    42,     9,    27,     9,
      47,     9,    42,     9,    30,   123,     9,   478,     9,     9,
       9,     9,     9,    87,    10,    11,    12,    91,    92,   222,
       9,    75,     9,     9,    62,     9,     9,   123,     9,    42,
      26,     9,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,     9,    50,   147,     9,   131,     8,     0,
       9,   775,  1147,   527,  1149,    32,    32,   195,   111,   202,
     865,    62,   195,   957,   869,   132,   790,   961,   196,   195,
      62,   214,   150,    42,   104,   123,   330,    42,   101,   143,
       8,   868,   198,   106,    62,   108,   109,   110,   111,   112,
     113,   114,   149,   199,    42,   197,   198,   157,    75,    75,
      62,    62,   200,   187,   160,   928,   195,    62,   931,    62,
     160,   178,    24,    25,   195,  1053,    28,    62,    62,   593,
     198,    62,  1060,  1061,   242,   243,    75,    62,   151,   152,
     248,   154,   162,    75,   277,   166,  1231,   193,   119,    62,
     277,   195,   143,   227,   166,   123,   196,   231,   402,   202,
     227,   235,   197,   176,   231,   208,   198,   846,   197,   848,
     196,   214,   369,   374,   325,   199,   263,    62,   197,   253,
     123,   652,   197,   960,   227,   656,   199,   197,   231,   197,
     198,   197,   389,   739,   197,    62,   197,   197,   197,   197,
     197,   160,    62,   199,   291,   160,   670,   198,   197,   196,
     196,   319,   196,   196,   411,   196,   198,   166,   196,    62,
     371,   372,   160,   420,    75,   268,   423,   941,  1156,    80,
     198,   181,   275,   276,   277,   309,   123,   196,   193,   282,
     196,   196,  1055,   196,   318,   288,   198,   198,   322,   143,
     195,   325,   309,   198,    62,   198,    92,    80,   196,   309,
     195,   200,    47,   198,   198,    32,   309,   198,   200,    75,
      80,   165,   375,   198,    80,   408,   195,    92,   726,   537,
     119,   150,   198,    32,   537,   198,  1371,   977,    95,    96,
     141,   142,   195,   962,   163,   369,   370,   371,   372,   363,
     202,    95,    96,    32,   198,   369,   208,    32,    75,   196,
      75,   147,   214,   198,   195,   389,   373,   140,   672,   108,
     109,   110,   111,   112,   113,   389,    75,   596,  1105,   198,
     140,   198,   147,  1369,   195,   141,   142,   411,   198,   143,
     448,   366,   788,   195,  1157,   195,    75,   411,   149,   423,
      75,   797,   621,   204,   818,   198,   420,   400,   602,   423,
       4,   165,   436,   195,   149,   408,   268,   392,   637,   436,
      75,   396,   843,   275,   276,    80,   141,   142,   636,   164,
     282,  1417,    27,   636,   642,   793,   288,   176,  1165,   642,
     198,   195,   199,   436,   198,   165,   483,   203,    42,  1068,
     864,  1070,   147,   756,  1489,   199,    32,    26,   197,   560,
      14,   485,    75,   682,    46,    47,    48,    80,    50,   772,
    1505,   529,   530,    27,    43,   195,   500,    46,   536,    75,
      67,    68,   506,   507,    80,   140,   141,   142,   195,   642,
      44,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    75,    26,    67,    68,   101,    80,    75,
     197,   363,   106,   197,   108,   109,   110,   111,   112,   113,
     114,    43,   100,   197,    46,   197,   901,   140,   141,   142,
     108,   109,   110,   111,   112,   113,   560,    60,    61,   197,
     198,   937,   956,   197,  1163,   141,   142,   197,   400,   945,
     108,   109,   110,   111,   112,   113,   408,   151,   152,   552,
     154,   119,   120,   108,   109,   110,   111,   112,   113,   141,
     142,    62,  1495,    62,   140,   141,   142,    94,    95,    96,
      62,   979,   176,   121,   122,    75,   197,   198,  1511,  1003,
      80,    94,    95,    96,  1395,  1396,  1010,  1018,   176,   157,
     123,   108,   109,   110,   285,   199,   197,   631,   289,   633,
     603,  1391,  1392,   760,   672,   673,   143,   198,   176,  1238,
     655,   115,   116,   117,   195,   928,   650,  1025,   931,    62,
     311,   176,   313,   314,   315,   316,   195,   143,   662,   663,
     195,    41,   147,   650,    50,   638,   663,   640,   143,   165,
     650,   141,   142,  1049,   123,   202,     9,   650,   143,   143,
    1081,  1057,  1076,   195,   123,     4,     8,   660,   197,  1065,
     663,   195,   165,   196,    14,  1096,    14,    75,   178,   179,
     180,   705,   197,   197,   708,   185,   186,    14,   196,   189,
     190,   792,   165,   197,    81,    14,   754,    92,   705,   196,
     552,   100,   921,    42,   196,   705,     9,   765,   195,   195,
      84,  1464,   705,   201,   738,   195,     9,   196,   738,   738,
    1023,   738,   715,   197,   196,    14,   738,    81,  1031,  1482,
     195,   738,     9,   726,   727,   181,   760,  1490,   738,    75,
      75,   184,   195,    75,   196,   738,   760,  1168,  1144,   196,
     121,   603,  1055,   197,  1373,  1176,    43,    44,    45,    46,
      47,    48,   101,    50,  1185,   195,    62,   106,   792,   108,
     109,   110,   111,   112,   113,   114,   196,   122,   164,   793,
     804,   805,   806,   124,     9,  1119,   638,   196,   640,   193,
      14,     9,   850,    62,   852,   830,     9,    14,   121,   196,
     202,   202,     9,   199,   195,   195,   195,   831,   660,   833,
     202,   835,   151,   152,   831,   154,   196,   202,   196,   124,
     197,   197,  1243,     9,   196,   195,   833,  1223,   835,   143,
     195,   195,   143,   833,   892,   835,   198,   176,   831,    14,
     833,     9,   835,   867,   837,  1148,   870,  1412,   901,    75,
     181,  1154,  1155,    14,  1157,   198,   914,    92,    14,   917,
     199,   198,    14,   715,   197,    27,   195,   197,   202,   893,
     198,   195,    14,   195,   726,   727,     9,   195,  1212,   195,
     195,   905,   124,   196,    14,   905,   905,   197,   905,   197,
       9,     4,   196,   905,   195,   124,   196,     9,   905,    24,
      25,   140,    75,    28,   202,   905,     9,   195,   124,    75,
      14,   197,   905,   195,   198,  1480,   196,   195,   124,   195,
       9,   196,   195,   202,   198,    50,   198,    27,   196,    42,
     140,   924,  1235,   957,   969,   993,   197,   961,   962,   196,
      69,   793,   197,  1364,   166,  1366,    27,   196,   972,     9,
     124,   196,   196,  1374,   124,   972,  1131,     9,   199,   983,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    92,   966,    14,   196,   983,   124,  1129,   972,
     199,   195,     9,   983,   196,   837,   979,   980,   101,   198,
     983,   196,   196,   106,  1415,   108,   109,   110,   111,   112,
     113,   114,   195,    27,   196,   196,    60,    61,   196,   196,
     124,   149,   197,   196,   198,   197,  1210,   153,   197,    75,
      14,   106,   124,  1023,   195,   124,   124,   196,    14,   196,
      75,  1031,  1025,   196,   198,    14,   196,   196,   151,   152,
     197,   154,  1100,   196,  1068,   195,  1070,   198,   196,   124,
      14,   197,   197,    14,   198,   196,    52,    75,  1116,    75,
     195,     9,    75,   176,    92,   197,   104,   143,   156,   123,
      92,  1129,   924,    30,    14,   196,   162,   202,    24,    25,
     195,   197,    28,   208,   195,    75,   199,   158,  1509,   214,
     196,  1115,     9,    75,  1515,  1119,   197,    75,    14,   196,
    1124,   196,   198,    14,    75,    14,    14,  1472,  1115,   372,
     483,   370,   789,   371,   966,  1115,   157,   242,   243,   741,
     791,  1128,  1115,   248,  1486,   980,  1093,   979,   980,  1482,
     488,  1131,  1260,  1344,  1514,  1124,  1503,    39,  1356,  1163,
    1230,   915,   196,   268,   375,  1169,   882,   467,  1148,  1173,
     275,   276,   879,  1177,  1154,  1155,  1173,   282,   467,   702,
     908,  1464,  1169,   288,  1358,   943,   805,   820,   276,  1169,
    1177,   853,    -1,  1025,   283,  1199,  1169,  1177,    -1,  1482,
    1173,  1205,    -1,    -1,  1177,     4,    -1,  1490,  1212,    -1,
      -1,    -1,    -1,    -1,   319,    -1,    -1,   322,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1214,    -1,    -1,
      -1,    -1,    -1,    -1,  1238,    -1,    -1,  1241,  1242,    -1,
      -1,    -1,  1246,    42,    -1,  1242,    -1,    -1,  1252,    -1,
      -1,    -1,    -1,    -1,  1241,  1235,    -1,    -1,   363,  1246,
      -1,  1241,    -1,    -1,    -1,  1252,  1246,    -1,  1241,  1242,
      -1,    -1,  1252,  1246,  1355,    -1,    -1,    -1,    -1,  1252,
      -1,    -1,   208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   453,    -1,   400,    -1,    -1,    -1,    -1,
      -1,    -1,   101,   408,    -1,  1460,    -1,   106,    -1,   108,
     109,   110,   111,   112,   113,   114,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1407,    -1,    -1,    -1,
     490,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   268,   448,    -1,  1476,    -1,    -1,   453,   275,
     276,  1355,   151,   152,    -1,   154,   282,    -1,    -1,    -1,
      -1,    -1,   288,    -1,    -1,    -1,    -1,    -1,    -1,  1373,
      -1,    -1,    -1,  1377,    -1,    -1,    -1,   176,    -1,    -1,
      -1,    -1,  1386,    -1,    -1,   490,    -1,  1391,  1392,    -1,
    1377,  1395,  1396,    -1,    -1,    -1,    -1,  1377,    -1,    -1,
     199,    -1,    -1,  1407,  1377,    -1,    -1,    -1,    -1,  1413,
    1414,    -1,    -1,    -1,    -1,  1419,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   529,   530,  1413,  1414,    -1,    -1,
      -1,   536,  1419,  1413,  1414,    -1,    -1,   363,    -1,  1419,
    1413,  1414,    -1,    -1,    -1,    -1,  1419,   552,  1452,    70,
      71,    72,    -1,    -1,    -1,  1459,    -1,  1495,    -1,    -1,
      81,    -1,    -1,    -1,    -1,  1452,    -1,    -1,    -1,    -1,
      -1,  1475,  1452,  1511,   400,    -1,    -1,    -1,    -1,  1452,
    1460,     4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   603,    -1,
      -1,   661,    -1,    -1,    -1,    -1,    -1,   128,   129,   130,
     131,   132,  1516,    -1,   674,   675,   676,  1521,   139,    42,
      -1,    -1,    -1,    -1,   145,   146,    -1,   453,    -1,  1516,
      -1,    -1,    -1,   638,  1521,   640,  1516,    -1,   159,    -1,
      -1,  1521,    -1,  1516,    -1,    -1,    -1,    -1,  1521,    -1,
      -1,    -1,    -1,   174,    -1,   660,   661,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   490,    -1,    -1,   672,   673,   674,
     675,   676,    -1,    -1,    -1,    -1,    -1,    -1,   101,    -1,
      -1,    -1,    -1,   106,    -1,   108,   109,   110,   111,   112,
     113,   114,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   706,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     715,   771,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   724,
      -1,   726,   727,    -1,    -1,    -1,   552,    -1,   151,   152,
      -1,   154,    -1,    -1,    -1,    -1,   741,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,   754,
      -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,
     765,    73,    -1,    75,    -1,    -1,   771,    -1,    -1,   774,
      -1,    -1,    -1,    -1,    -1,    -1,   199,   603,    -1,    -1,
      -1,   242,   243,    60,    61,    -1,    -1,   248,   793,    -1,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    -1,    -1,    -1,   865,    -1,   119,    -1,   869,
      -1,   871,   638,    -1,   640,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,   889,
      -1,    -1,   837,    -1,   660,   661,    60,    61,    -1,   151,
     152,    -1,   154,   155,   156,   850,   123,   852,   674,   675,
     676,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   319,    -1,
     865,   322,    -1,    -1,   869,   177,   871,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     706,    -1,    -1,    -1,   889,    -1,   198,   892,   200,   715,
      73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   724,   123,
     726,   727,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   914,
      -1,    -1,   917,    -1,    -1,   741,    -1,    -1,    -1,   924,
      -1,    -1,   982,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   996,    -1,   998,    -1,
      -1,    -1,    -1,    -1,    -1,   771,    -1,    -1,   774,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   966,    -1,    -1,   147,    -1,    -1,   793,   151,   152,
      -1,   154,   155,   156,   979,   980,    73,   982,    75,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1046,   448,   993,    -1,
      -1,   996,   453,   998,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,    -1,
      -1,   837,    -1,    -1,    -1,   198,    -1,  1022,    -1,    -1,
    1025,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   490,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   865,
      -1,  1046,    -1,   869,  1104,   871,    -1,    -1,    -1,  1109,
      -1,  1111,    -1,    -1,   151,   152,    -1,   154,   155,   156,
      -1,    -1,    -1,   889,    -1,    -1,    -1,    -1,   529,   530,
      -1,    -1,    -1,    -1,    -1,   536,    -1,    -1,    -1,    -1,
     177,  1141,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,  1100,    -1,    -1,   924,  1104,
      -1,   198,    -1,   200,  1109,    -1,  1111,    -1,    -1,    -1,
      -1,  1116,    -1,    -1,  1119,  1120,    -1,  1122,    -1,  1179,
      -1,    -1,    -1,    -1,  1129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1141,    -1,    -1,    -1,
     966,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   979,   980,    -1,   982,    -1,    -1,    -1,
      -1,  1221,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     996,    -1,   998,    -1,  1179,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1188,  1189,    -1,    -1,    -1,    -1,  1249,
    1250,    -1,    -1,    -1,    -1,  1255,  1022,    -1,    -1,  1025,
     661,    -1,    -1,    10,    11,    12,    -1,  1212,    -1,    -1,
      -1,   672,   673,   674,   675,   676,  1221,    -1,    -1,    26,
    1046,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    -1,    50,  1249,  1250,    -1,    -1,    -1,    -1,
    1255,  1256,    64,    -1,    -1,  1260,    -1,    -1,    -1,    -1,
      -1,    73,    -1,    75,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1104,    -1,
      -1,    -1,    -1,  1109,    -1,  1111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   754,  1120,    -1,  1122,    -1,    -1,  1359,
      -1,   113,    -1,    -1,   765,    -1,    -1,    -1,    -1,    -1,
     771,    -1,    -1,    -1,    -1,  1141,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1390,    -1,   144,    73,    -1,   147,    -1,   149,    -1,   151,
     152,  1401,   154,   155,   156,    -1,  1406,    -1,    -1,    -1,
      -1,    -1,    -1,  1179,  1359,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1188,  1189,    -1,   177,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
      -1,  1386,    -1,   195,    -1,  1390,    -1,    -1,   200,   850,
      -1,   852,   199,    -1,    -1,  1221,  1401,    -1,    -1,    -1,
      -1,  1406,    -1,  1463,   865,    -1,    -1,    -1,   869,   149,
     871,   151,   152,   153,   154,   155,   156,    -1,    -1,    -1,
      -1,    -1,    -1,  1249,  1250,    -1,    -1,    -1,   889,  1255,
    1256,   892,    -1,    -1,  1260,    -1,    -1,   177,    -1,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,    -1,   914,    -1,   195,   917,    -1,  1463,  1519,
      -1,    -1,    -1,    -1,    -1,  1525,  1471,    -1,    -1,  1529,
      -1,  1531,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1486,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1495,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1511,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1519,    -1,    -1,    -1,    -1,    -1,
    1525,   982,    -1,    -1,  1529,    -1,  1531,    -1,    -1,    -1,
      -1,   453,   993,  1359,    -1,   996,    -1,   998,    -1,    -1,
       5,     6,    -1,     8,     9,    10,    -1,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    -1,
      -1,    26,    27,    -1,  1390,    -1,    -1,    -1,   490,    -1,
      -1,    -1,    -1,    -1,    39,  1401,    -1,    -1,    -1,    -1,
    1406,    46,    -1,    48,    -1,  1046,    51,    -1,    53,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,   453,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1463,    -1,  1100,
      -1,    -1,   490,  1104,    -1,  1471,   111,    -1,  1109,    -1,
    1111,    -1,    -1,    -1,    -1,  1116,    -1,    -1,  1119,    -1,
    1486,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1129,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,
    1141,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1519,    -1,    -1,    -1,    -1,    -1,  1525,
      -1,    -1,    -1,  1529,    52,  1531,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   182,  1179,    -1,
      -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   661,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1212,   674,   675,   676,    -1,    -1,    -1,   223,    -1,
    1221,   226,    -1,    -1,    -1,    -1,    -1,    -1,   233,   234,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   126,   127,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1249,  1250,
      -1,    -1,    -1,    -1,  1255,    -1,   144,    -1,    -1,   147,
      -1,   149,    -1,   151,   152,    -1,   154,   155,   156,    -1,
      -1,    -1,   277,   661,    -1,    -1,    -1,    -1,   283,    -1,
      -1,   169,   287,    -1,    -1,    -1,   674,   675,    -1,   177,
      -1,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,    -1,   310,    -1,   195,    -1,   771,
      -1,    -1,    -1,    -1,    -1,    -1,   321,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,  1359,    -1,
     365,   366,    -1,   368,    -1,    -1,    -1,    -1,    -1,    -1,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,    -1,   771,    -1,  1386,    -1,   392,   393,  1390,
     395,   396,   397,    -1,    -1,    -1,    -1,    -1,   403,    -1,
    1401,    -1,    -1,   865,    -1,  1406,    -1,   869,   413,   871,
     415,    -1,    -1,    10,    11,    12,   421,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   431,   889,   433,    26,
      -1,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    -1,    50,    -1,   460,   461,    -1,   463,   464,
     465,    -1,  1463,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,   486,    50,   871,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1495,    -1,    -1,    -1,    -1,    -1,
      -1,   889,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1511,    -1,    -1,    -1,    -1,    -1,   521,    -1,  1519,    -1,
     982,    -1,    -1,    -1,  1525,    -1,    -1,    -1,  1529,    -1,
    1531,    -1,    -1,    -1,   996,    -1,   998,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,   564,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    50,    -1,  1046,    -1,    -1,    -1,    -1,    -1,
     595,    -1,    -1,    -1,   982,    -1,    -1,    -1,    -1,   604,
      10,    11,    12,    -1,    -1,   202,    -1,    -1,   996,    -1,
     998,    -1,    -1,    -1,    -1,   620,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,   646,  1104,    -1,    -1,    -1,    -1,  1109,    -1,  1111,
      -1,    -1,    -1,    -1,   659,    -1,    -1,    -1,  1046,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1141,
      -1,    -1,    -1,    -1,    26,   690,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1104,  1179,    -1,    -1,
      -1,  1109,    -1,  1111,    -1,    -1,    -1,    -1,    -1,    -1,
     735,   199,    73,    -1,    75,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   748,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1141,    -1,    -1,    -1,    -1,    -1,  1221,
      -1,    -1,    -1,    -1,   769,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   113,    -1,   779,    -1,    -1,   782,    -1,   784,
      -1,    -1,    -1,   788,   125,    -1,    -1,  1249,  1250,   199,
      -1,  1179,   797,  1255,    -1,    -1,    -1,  1259,    -1,    -1,
      -1,    -1,    -1,   144,    -1,    -1,   147,    -1,   149,    -1,
     151,   152,    -1,   154,   155,   156,    -1,    -1,   823,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1221,    26,    -1,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,    -1,    -1,    -1,   195,    -1,    -1,   199,    -1,   200,
      52,  1249,  1250,    -1,    -1,    -1,    -1,  1255,   873,   874,
     875,    -1,    -1,    -1,   879,   880,    -1,    -1,    -1,    -1,
      -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   901,  1359,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,   920,   108,   109,   110,   111,
     112,   113,    -1,    -1,    -1,    -1,    -1,    -1,  1390,    -1,
      -1,    -1,   937,    -1,   126,   127,    -1,    -1,    -1,  1401,
     945,    -1,   947,    -1,  1406,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   144,    -1,    -1,   147,    -1,   149,    -1,   151,
     152,    -1,   154,   155,   156,    -1,    -1,  1429,    -1,    -1,
      -1,  1359,   977,    -1,    -1,    -1,    -1,   169,    -1,    -1,
      -1,   986,    -1,    -1,   176,   177,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
      -1,  1463,  1390,   195,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1401,    -1,    -1,    -1,    -1,  1406,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1034,
      -1,    -1,    -1,  1038,    -1,  1040,    -1,    -1,    -1,    -1,
      -1,    73,    -1,    75,  1049,    -1,    -1,    -1,    -1,    -1,
      -1,  1056,  1057,    -1,    -1,    -1,    -1,  1519,    -1,    -1,
    1065,    -1,    -1,  1525,    10,    11,    12,  1529,    -1,  1531,
      -1,    -1,    -1,    -1,    -1,  1463,    -1,    -1,    -1,    -1,
      26,   113,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    -1,    -1,    -1,  1113,    -1,
      -1,    -1,   144,    -1,    -1,   147,    -1,   149,    -1,   151,
     152,    -1,   154,   155,   156,     3,     4,     5,     6,     7,
      -1,  1519,    -1,    -1,    -1,    13,    -1,  1525,    -1,  1144,
      -1,  1529,    -1,  1531,    -1,   177,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
      -1,    -1,    -1,   195,    42,    43,    44,    -1,   200,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    75,    -1,    77,
      -1,    -1,    -1,    81,    82,    83,    84,    -1,    86,    -1,
      88,    -1,    90,    -1,    -1,    93,    -1,    -1,  1223,    97,
      98,    99,   100,   101,   102,   103,    -1,   105,   106,   107,
     108,   109,   110,   111,   112,   113,    -1,   115,   116,   117,
     118,   119,   120,    -1,    -1,   191,  1251,   125,   126,    -1,
     128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,
      -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,
     148,   149,    -1,   151,   152,    -1,   154,   155,   156,   157,
      -1,   159,    -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,
      -1,   169,   170,    -1,   172,    -1,   174,   175,   176,   177,
      -1,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,    -1,    -1,   195,    -1,   197,
     198,   199,   200,   201,    12,   203,   204,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    26,    -1,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    50,    -1,    -1,    -1,    42,    43,    44,    -1,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      -1,    77,    -1,    -1,    -1,    81,    82,    83,    84,    -1,
      86,    -1,    88,    -1,    90,    -1,    -1,    93,    -1,    -1,
      -1,    97,    98,    99,   100,   101,   102,   103,    -1,   105,
     106,   107,   108,   109,   110,   111,   112,   113,    -1,   115,
     116,   117,   118,   119,   120,    -1,    -1,    -1,    -1,   125,
     126,    -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,
      -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,
     146,   147,   148,   149,    -1,   151,   152,    -1,   154,   155,
     156,   157,    -1,   159,    -1,    -1,    -1,   163,    -1,    -1,
      -1,    -1,    -1,   169,   170,    -1,   172,    -1,   174,   175,
     176,   177,    -1,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,    -1,    -1,   195,
      -1,   197,   198,   199,   200,   201,    -1,   203,   204,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    26,    13,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    50,    -1,    -1,    -1,    -1,    -1,    42,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    75,    -1,    77,    -1,    -1,    -1,    81,    82,    83,
      84,    -1,    86,    -1,    88,    -1,    90,    -1,    -1,    93,
      -1,    -1,    -1,    97,    98,    99,   100,   101,   102,   103,
      -1,   105,   106,   107,   108,   109,   110,   111,   112,   113,
      -1,   115,   116,   117,   118,   119,   120,    -1,    -1,    -1,
      -1,   125,   126,    -1,   128,   129,   130,   131,   132,    -1,
      -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,
     144,   145,   146,   147,   148,   149,    -1,   151,   152,    -1,
     154,   155,   156,   157,    -1,   159,    -1,    -1,    -1,   163,
      -1,    -1,    -1,    -1,    -1,   169,   170,    -1,   172,    -1,
     174,   175,   176,   177,    -1,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,    -1,
      -1,   195,    -1,   197,   198,    -1,   200,   201,    -1,   203,
     204,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,    -1,
      42,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    75,    -1,    77,    -1,    -1,    -1,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    90,    -1,
      -1,    93,    -1,    -1,    -1,    97,    98,    99,   100,    -1,
     102,   103,    -1,   105,    -1,   107,   108,   109,   110,   111,
     112,   113,    -1,   115,   116,   117,    -1,   119,   120,    -1,
      -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,   131,
     132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,
      -1,    -1,   144,   145,   146,   147,   148,   149,    -1,   151,
     152,    -1,   154,   155,   156,   157,    -1,   159,    -1,    -1,
      -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,
      -1,    -1,   174,   175,   176,   177,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,    -1,    -1,   195,    -1,   197,   198,   199,   200,   201,
      -1,   203,   204,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    75,    -1,    77,    -1,    -1,
      -1,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      90,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,    99,
     100,    -1,   102,   103,    -1,   105,    -1,   107,   108,   109,
     110,   111,   112,   113,    -1,   115,   116,   117,    -1,   119,
     120,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,
     130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,
      -1,    -1,    -1,    -1,   144,   145,   146,   147,   148,   149,
      -1,   151,   152,    -1,   154,   155,   156,   157,    -1,   159,
      -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,
      -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,    -1,    -1,   195,    -1,   197,   198,   199,
     200,   201,    -1,   203,   204,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    75,    -1,    77,
      -1,    -1,    -1,    81,    82,    83,    84,    -1,    86,    -1,
      88,    -1,    90,    -1,    -1,    93,    -1,    -1,    -1,    97,
      98,    99,   100,    -1,   102,   103,    -1,   105,    -1,   107,
     108,   109,   110,   111,   112,   113,    -1,   115,   116,   117,
      -1,   119,   120,    -1,    -1,    -1,    -1,   125,   126,    -1,
     128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,
      -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,
     148,   149,    -1,   151,   152,    -1,   154,   155,   156,   157,
      -1,   159,    -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,
      -1,   169,    -1,    -1,    -1,    -1,   174,   175,   176,   177,
      -1,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,    -1,    -1,   195,    -1,   197,
     198,   199,   200,   201,    -1,   203,   204,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,    -1,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      -1,    77,    -1,    -1,    -1,    81,    82,    83,    84,    85,
      86,    -1,    88,    -1,    90,    -1,    -1,    93,    -1,    -1,
      -1,    97,    98,    99,   100,    -1,   102,   103,    -1,   105,
      -1,   107,   108,   109,   110,   111,   112,   113,    -1,   115,
     116,   117,    -1,   119,   120,    -1,    -1,    -1,    -1,   125,
     126,    -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,
      -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,
     146,   147,   148,   149,    -1,   151,   152,    -1,   154,   155,
     156,   157,    -1,   159,    -1,    -1,    -1,   163,    -1,    -1,
      -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,   175,
     176,   177,    -1,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,    -1,    -1,   195,
      -1,   197,   198,    -1,   200,   201,    -1,   203,   204,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    75,    -1,    77,    -1,    -1,    -1,    81,    82,    83,
      84,    -1,    86,    -1,    88,    -1,    90,    91,    -1,    93,
      -1,    -1,    -1,    97,    98,    99,   100,    -1,   102,   103,
      -1,   105,    -1,   107,   108,   109,   110,   111,   112,   113,
      -1,   115,   116,   117,    -1,   119,   120,    -1,    -1,    -1,
      -1,   125,   126,    -1,   128,   129,   130,   131,   132,    -1,
      -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,
     144,   145,   146,   147,   148,   149,    -1,   151,   152,    -1,
     154,   155,   156,   157,    -1,   159,    -1,    -1,    -1,   163,
      -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,
     174,   175,   176,   177,    -1,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,    -1,
      -1,   195,    -1,   197,   198,    -1,   200,   201,    -1,   203,
     204,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    75,    -1,    77,    -1,    -1,    -1,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    90,    -1,
      -1,    93,    -1,    -1,    -1,    97,    98,    99,   100,    -1,
     102,   103,    -1,   105,    -1,   107,   108,   109,   110,   111,
     112,   113,    -1,   115,   116,   117,    -1,   119,   120,    -1,
      -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,   131,
     132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,
      -1,    -1,   144,   145,   146,   147,   148,   149,    -1,   151,
     152,    -1,   154,   155,   156,   157,    -1,   159,    -1,    -1,
      -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,
      -1,    -1,   174,   175,   176,   177,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,    -1,    -1,   195,    -1,   197,   198,   199,   200,   201,
      -1,   203,   204,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    75,    -1,    77,    -1,    -1,
      -1,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      90,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,    99,
     100,    -1,   102,   103,    -1,   105,    -1,   107,   108,   109,
     110,   111,   112,   113,    -1,   115,   116,   117,    -1,   119,
     120,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,
     130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,
      -1,    -1,    -1,    -1,   144,   145,   146,   147,   148,   149,
      -1,   151,   152,    -1,   154,   155,   156,   157,    -1,   159,
      -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,
      -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,    -1,    -1,   195,    -1,   197,   198,   199,
     200,   201,    -1,   203,   204,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    75,    -1,    77,
      -1,    -1,    -1,    81,    82,    83,    84,    -1,    86,    -1,
      88,    89,    90,    -1,    -1,    93,    -1,    -1,    -1,    97,
      98,    99,   100,    -1,   102,   103,    -1,   105,    -1,   107,
     108,   109,   110,   111,   112,   113,    -1,   115,   116,   117,
      -1,   119,   120,    -1,    -1,    -1,    -1,   125,   126,    -1,
     128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,
      -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,
     148,   149,    -1,   151,   152,    -1,   154,   155,   156,   157,
      -1,   159,    -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,
      -1,   169,    -1,    -1,    -1,    -1,   174,   175,   176,   177,
      -1,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,    -1,    -1,   195,    -1,   197,
     198,    -1,   200,   201,    -1,   203,   204,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,    -1,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      -1,    77,    -1,    -1,    -1,    81,    82,    83,    84,    -1,
      86,    -1,    88,    -1,    90,    -1,    -1,    93,    -1,    -1,
      -1,    97,    98,    99,   100,    -1,   102,   103,    -1,   105,
      -1,   107,   108,   109,   110,   111,   112,   113,    -1,   115,
     116,   117,    -1,   119,   120,    -1,    -1,    -1,    -1,   125,
     126,    -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,
      -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,
     146,   147,   148,   149,    -1,   151,   152,    -1,   154,   155,
     156,   157,    -1,   159,    -1,    -1,    -1,   163,    -1,    -1,
      -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,   175,
     176,   177,    -1,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,    -1,    -1,   195,
      -1,   197,   198,   199,   200,   201,    -1,   203,   204,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    75,    -1,    77,    -1,    -1,    -1,    81,    82,    83,
      84,    -1,    86,    -1,    88,    -1,    90,    -1,    -1,    93,
      -1,    -1,    -1,    97,    98,    99,   100,    -1,   102,   103,
      -1,   105,    -1,   107,   108,   109,   110,   111,   112,   113,
      -1,   115,   116,   117,    -1,   119,   120,    -1,    -1,    -1,
      -1,   125,   126,    -1,   128,   129,   130,   131,   132,    -1,
      -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,
     144,   145,   146,   147,   148,   149,    -1,   151,   152,    -1,
     154,   155,   156,   157,    -1,   159,    -1,    -1,    -1,   163,
      -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,
     174,   175,   176,   177,    -1,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,    -1,
      -1,   195,    -1,   197,   198,   199,   200,   201,    -1,   203,
     204,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    75,    -1,    77,    -1,    -1,    -1,    81,
      82,    83,    84,    -1,    86,    87,    88,    -1,    90,    -1,
      -1,    93,    -1,    -1,    -1,    97,    98,    99,   100,    -1,
     102,   103,    -1,   105,    -1,   107,   108,   109,   110,   111,
     112,   113,    -1,   115,   116,   117,    -1,   119,   120,    -1,
      -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,   131,
     132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,
      -1,    -1,   144,   145,   146,   147,   148,   149,    -1,   151,
     152,    -1,   154,   155,   156,   157,    -1,   159,    -1,    -1,
      -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,
      -1,    -1,   174,   175,   176,   177,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,    -1,    -1,   195,    -1,   197,   198,    -1,   200,   201,
      -1,   203,   204,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    75,    -1,    77,    -1,    -1,
      -1,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      90,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,    99,
     100,    -1,   102,   103,    -1,   105,    -1,   107,   108,   109,
     110,   111,   112,   113,    -1,   115,   116,   117,    -1,   119,
     120,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,
     130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,
      -1,    -1,    -1,    -1,   144,   145,   146,   147,   148,   149,
      -1,   151,   152,    -1,   154,   155,   156,   157,    -1,   159,
      -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,
      -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,    -1,    -1,   195,    -1,   197,   198,   199,
     200,   201,    -1,   203,   204,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    75,    -1,    77,
      -1,    -1,    -1,    81,    82,    83,    84,    -1,    86,    -1,
      88,    -1,    90,    -1,    -1,    93,    -1,    -1,    -1,    97,
      98,    99,   100,    -1,   102,   103,    -1,   105,    -1,   107,
     108,   109,   110,   111,   112,   113,    -1,   115,   116,   117,
      -1,   119,   120,    -1,    -1,    -1,    -1,   125,   126,    -1,
     128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,
      -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,
     148,   149,    -1,   151,   152,    -1,   154,   155,   156,   157,
      -1,   159,    -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,
      -1,   169,    -1,    -1,    -1,    -1,   174,   175,   176,   177,
      -1,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,    -1,    -1,   195,    -1,   197,
     198,   199,   200,   201,    -1,   203,   204,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,    -1,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      -1,    77,    -1,    -1,    -1,    81,    82,    83,    84,    -1,
      86,    -1,    88,    -1,    90,    -1,    -1,    93,    -1,    -1,
      -1,    97,    98,    99,   100,    -1,   102,   103,    -1,   105,
      -1,   107,   108,   109,   110,   111,   112,   113,    -1,   115,
     116,   117,    -1,   119,   120,    -1,    -1,    -1,    -1,   125,
     126,    -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,
      -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,
     146,   147,   148,   149,    -1,   151,   152,    -1,   154,   155,
     156,   157,    -1,   159,    -1,    -1,    -1,   163,    -1,    -1,
      -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,   175,
     176,   177,    -1,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,    -1,    -1,   195,
      -1,   197,   198,   199,   200,   201,    -1,   203,   204,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    75,    -1,    77,    -1,    -1,    -1,    81,    82,    83,
      84,    -1,    86,    -1,    88,    -1,    90,    -1,    -1,    93,
      -1,    -1,    -1,    97,    98,    99,   100,    -1,   102,   103,
      -1,   105,    -1,   107,   108,   109,   110,   111,   112,   113,
      -1,   115,   116,   117,    -1,   119,   120,    -1,    -1,    -1,
      -1,   125,   126,    -1,   128,   129,   130,   131,   132,    -1,
      -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,
     144,   145,   146,   147,   148,   149,    -1,   151,   152,    -1,
     154,   155,   156,   157,    -1,   159,    -1,    -1,    -1,   163,
      -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,
     174,   175,   176,   177,    -1,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,    -1,
      -1,   195,    -1,   197,   198,    -1,   200,   201,    -1,   203,
     204,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    75,    -1,    77,    -1,    -1,    -1,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    90,    -1,
      -1,    93,    -1,    -1,    -1,    97,    98,    99,   100,    -1,
     102,   103,    -1,   105,    -1,   107,   108,   109,   110,   111,
     112,   113,    -1,   115,   116,   117,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,   131,
     132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,
      -1,    -1,   144,   145,   146,   147,   148,   149,    -1,   151,
     152,    -1,   154,   155,   156,    -1,    -1,   159,    -1,    -1,
      -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,
      -1,    -1,   174,   175,   176,   177,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,    -1,    -1,   195,    -1,   197,   198,    -1,   200,   201,
      -1,   203,   204,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    75,    -1,    77,    -1,    -1,
      -1,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      90,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,    99,
     100,    -1,   102,   103,    -1,   105,    -1,   107,   108,   109,
     110,   111,   112,   113,    -1,   115,   116,   117,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,
     130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,
      -1,    -1,    -1,    -1,   144,   145,   146,   147,   148,   149,
      -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,   159,
      -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,
      -1,    -1,    -1,    -1,   174,   175,   176,   177,    -1,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,    -1,    -1,   195,    -1,   197,   198,    -1,
     200,   201,    -1,   203,   204,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    75,    -1,    77,
      -1,    -1,    -1,    81,    82,    83,    84,    -1,    86,    -1,
      88,    -1,    90,    -1,    -1,    93,    -1,    -1,    -1,    97,
      98,    99,   100,    -1,   102,   103,    -1,   105,    -1,   107,
     108,   109,   110,   111,   112,   113,    -1,   115,   116,   117,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,   126,    -1,
     128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,
      -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,
     148,   149,    -1,   151,   152,    -1,   154,   155,   156,    -1,
      -1,   159,    -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,
      -1,   169,    -1,    -1,    -1,    -1,   174,   175,   176,   177,
      -1,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,    -1,    -1,   195,    -1,   197,
     198,    -1,   200,   201,    -1,   203,   204,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      -1,    77,    -1,    -1,    -1,    81,    82,    83,    84,    -1,
      86,    -1,    88,    -1,    90,    -1,    -1,    93,    -1,    -1,
      -1,    97,    98,    99,   100,    -1,   102,   103,    -1,   105,
      -1,   107,   108,   109,   110,   111,   112,   113,    -1,   115,
     116,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,
     126,    -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,
      -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,
     146,   147,   148,   149,    -1,   151,   152,    -1,   154,   155,
     156,    -1,    -1,   159,    -1,    -1,    -1,   163,    -1,    -1,
      -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,   175,
     176,   177,    -1,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,    -1,    -1,   195,
      -1,   197,   198,    -1,   200,   201,    -1,   203,   204,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    75,    -1,    77,    -1,    -1,    -1,    81,    82,    83,
      84,    -1,    86,    -1,    88,    -1,    90,    -1,    -1,    93,
      -1,    -1,    -1,    97,    98,    99,   100,    -1,   102,   103,
      -1,   105,    -1,   107,   108,   109,   110,   111,   112,   113,
      -1,   115,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   125,   126,    -1,   128,   129,   130,   131,   132,    -1,
      -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,
     144,   145,   146,   147,   148,   149,    -1,   151,   152,    -1,
     154,   155,   156,    -1,    -1,   159,    -1,    -1,    -1,   163,
      -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,
     174,   175,   176,   177,    -1,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,    -1,
      -1,   195,    -1,   197,   198,    -1,   200,   201,    -1,   203,
     204,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    75,    -1,    77,    -1,    -1,    -1,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    90,    -1,
      -1,    93,    -1,    -1,    -1,    97,    98,    99,   100,    -1,
     102,   103,    -1,   105,    -1,   107,   108,   109,   110,   111,
     112,   113,    -1,   115,   116,   117,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,   131,
     132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,
      -1,    -1,   144,   145,   146,   147,   148,   149,    -1,   151,
     152,    -1,   154,   155,   156,    -1,    -1,   159,    -1,    -1,
      -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,
      -1,    -1,   174,   175,   176,   177,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,    -1,    -1,   195,    -1,   197,   198,    -1,   200,   201,
      -1,   203,   204,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    75,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
     110,   111,   112,   113,    -1,    73,   116,   117,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,
     130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,
      -1,    -1,    -1,    -1,   144,   145,   146,   147,   148,   149,
      -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,   159,
      -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,
      -1,    -1,    -1,    -1,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   151,   152,   195,   154,   155,   156,    -1,
     200,   201,    -1,   203,   204,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,   177,
      -1,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,    -1,    -1,    -1,   195,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    75,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,   109,   110,   111,   112,   113,    -1,    -1,   116,   117,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,   126,    -1,
     128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,
      -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,
      -1,   149,    -1,   151,   152,    -1,   154,   155,   156,    -1,
      -1,   159,    -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,
      -1,   169,    -1,    -1,    -1,    -1,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,    -1,    -1,   195,    -1,   197,
      -1,    -1,   200,   201,    -1,   203,   204,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,   109,   110,   111,   112,   113,    -1,    73,
     116,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,
     126,    -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,
      -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,
     146,   147,   148,   149,    -1,   151,   152,    -1,   154,   155,
     156,    -1,    -1,   159,    -1,    -1,    -1,   163,    -1,    -1,
      -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,   175,
     176,   177,    -1,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   151,   152,   195,
     154,   155,   156,    -1,   200,   201,    -1,   203,   204,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,   177,    -1,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,    32,    -1,
      -1,   195,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    75,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,   109,   110,   111,   112,   113,
      -1,    73,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   125,   126,    -1,   128,   129,   130,   131,   132,    -1,
      -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,
     144,   145,   146,   147,    -1,   149,    -1,   151,   152,    -1,
     154,   155,   156,    -1,    -1,   159,    -1,   119,    -1,   163,
      -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,
     174,    -1,   176,   177,    -1,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   151,
     152,   195,   154,   155,   156,    -1,   200,   201,    -1,   203,
     204,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,   177,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,
     112,   113,    -1,    -1,   116,   117,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,   131,
     132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,
      -1,    -1,   144,   145,   146,   147,    -1,   149,    -1,   151,
     152,    -1,   154,   155,   156,    -1,    -1,   159,    -1,    -1,
      -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,
      -1,    -1,   174,    -1,   176,   177,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,    -1,    -1,   195,    -1,   197,    -1,    -1,   200,   201,
      -1,   203,   204,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    75,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
     110,   111,   112,   113,    -1,    -1,   116,   117,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,
     130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,
      -1,    -1,    -1,    -1,   144,   145,   146,   147,    -1,   149,
      -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,   159,
      -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,
      -1,    -1,    -1,    -1,   174,    -1,   176,   177,    -1,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,    -1,    -1,   195,    -1,   197,    -1,    -1,
     200,   201,    -1,   203,   204,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    75,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    97,
      -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,   109,   110,   111,   112,   113,    -1,    73,   116,   117,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,   126,    -1,
     128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,
      -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,
      -1,   149,    -1,   151,   152,    -1,   154,   155,   156,    -1,
      -1,   159,    -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,
      -1,   169,    -1,    -1,    -1,    -1,   174,    -1,   176,   177,
      -1,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   151,   152,   195,   154,   155,
     156,    -1,   200,   201,    -1,   203,   204,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,   177,    -1,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,   109,   110,   111,   112,   113,    -1,    -1,
     116,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,
     126,    -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,
      -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,
     146,   147,    -1,   149,    -1,   151,   152,    -1,   154,   155,
     156,    -1,    -1,   159,    -1,    -1,    -1,   163,    -1,    -1,
      -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,    -1,
     176,   177,    -1,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,    -1,    -1,   195,
     196,    -1,    -1,    -1,   200,   201,    -1,   203,   204,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    75,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,   109,   110,   111,   112,   113,
      -1,    -1,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   125,   126,    -1,   128,   129,   130,   131,   132,    -1,
      -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,
     144,   145,   146,   147,    -1,   149,    -1,   151,   152,    -1,
     154,   155,   156,    -1,    -1,   159,    -1,    -1,    -1,   163,
      -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,
     174,    -1,   176,   177,    -1,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,    -1,
      -1,   195,    -1,    -1,    -1,    -1,   200,   201,    -1,   203,
     204,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,
     112,   113,    -1,    -1,   116,   117,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,   131,
     132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,
      -1,    -1,   144,   145,   146,   147,    -1,   149,    -1,   151,
     152,    -1,   154,   155,   156,    -1,    -1,   159,    -1,    -1,
      -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,
      -1,    -1,   174,    -1,   176,   177,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,    -1,    -1,   195,    -1,    -1,    -1,    -1,   200,   201,
      -1,   203,   204,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    75,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
     110,   111,   112,   113,    -1,    -1,   116,   117,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,
     130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,
      -1,    -1,    -1,    -1,   144,   145,   146,   147,    -1,   149,
      -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,   159,
      -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,
      -1,    -1,    -1,    -1,   174,    -1,   176,   177,    -1,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,    -1,    -1,   195,    -1,    -1,    -1,    -1,
     200,   201,    -1,   203,   204,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    75,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,   109,   110,   111,   112,   113,    -1,    -1,   116,   117,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,   126,    -1,
     128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,
      -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,
      -1,   149,    -1,   151,   152,    -1,   154,   155,   156,    -1,
      -1,   159,    -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,
      -1,   169,    -1,    -1,    -1,    -1,   174,    -1,   176,   177,
      -1,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,    -1,    -1,   195,    -1,    -1,
      -1,    -1,   200,   201,    -1,   203,   204,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,   109,   110,   111,   112,   113,    -1,    -1,
     116,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,
     126,    -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,
      -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,
     146,   147,    -1,   149,    -1,   151,   152,    -1,   154,   155,
     156,    -1,    -1,   159,    -1,    -1,    -1,   163,    -1,    -1,
      -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,    -1,
     176,   177,    -1,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,    -1,    -1,   195,
      -1,    -1,    -1,    -1,   200,   201,    -1,   203,   204,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    75,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,   109,   110,   111,   112,   113,
      -1,    -1,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   125,   126,    -1,   128,   129,   130,   131,   132,    -1,
      -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,
     144,   145,   146,   147,    -1,   149,    -1,   151,   152,    -1,
     154,   155,   156,    -1,    -1,   159,    -1,    -1,    -1,   163,
      -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,
     174,    -1,   176,   177,    -1,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,    -1,
      -1,   195,    -1,    -1,   198,    -1,   200,   201,    -1,   203,
     204,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    75,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,
     112,   113,    -1,    -1,   116,   117,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   125,   126,    -1,   128,   129,   130,   131,
     132,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,
      -1,    -1,   144,   145,   146,   147,    -1,   149,    -1,   151,
     152,    -1,   154,   155,   156,    -1,    -1,   159,    -1,    -1,
      -1,   163,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,
      -1,    -1,   174,    -1,   176,   177,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,    -1,    -1,   195,    -1,    -1,    -1,    -1,   200,   201,
      -1,   203,   204,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    75,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
     110,   111,   112,   113,    -1,    -1,   116,   117,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   125,   126,    -1,   128,   129,
     130,   131,   132,    -1,    -1,    -1,    -1,    -1,    -1,   139,
      -1,    -1,    -1,    -1,   144,   145,   146,   147,    -1,   149,
      -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,   159,
      -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,   169,
      -1,    -1,    -1,    -1,   174,    -1,   176,   177,    -1,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,    -1,    -1,   195,    -1,    -1,    -1,    -1,
     200,   201,    -1,   203,   204,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    75,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,   109,   110,   111,   112,   113,    -1,    -1,   116,   117,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,   126,    -1,
     128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,    -1,
      -1,   139,    -1,    -1,    -1,    -1,   144,   145,   146,   147,
      -1,   149,    -1,   151,   152,    -1,   154,   155,   156,    -1,
      -1,   159,    -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,
      -1,   169,    -1,    -1,    -1,    -1,   174,    -1,   176,   177,
      -1,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,    -1,    -1,   195,    -1,    -1,
      -1,    -1,   200,   201,    -1,   203,   204,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,    75,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,   109,   110,   111,   112,   113,    -1,    -1,
     116,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,
     126,    -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,
      -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,   144,   145,
     146,   147,    -1,   149,    -1,   151,   152,    -1,   154,   155,
     156,    -1,    -1,   159,    -1,    -1,    -1,   163,    -1,    -1,
      -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,    -1,
     176,   177,    -1,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,    -1,    -1,   195,
      -1,    -1,    -1,    -1,   200,   201,    -1,   203,   204,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    75,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,   109,   110,   111,   112,   113,
      -1,    -1,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   125,   126,    -1,   128,   129,   130,   131,   132,    -1,
      -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,
     144,   145,   146,   147,    -1,   149,    -1,   151,   152,    -1,
     154,   155,   156,    -1,    -1,   159,    -1,    -1,    -1,   163,
      -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,
     174,    -1,   176,   177,    -1,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,    -1,
      -1,   195,    10,    11,    12,    -1,   200,   201,    -1,   203,
     204,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,
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
      46,    47,    48,    -1,    50,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,   199,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,   199,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   199,    -1,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   199,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    12,    -1,     3,     4,     5,     6,
       7,    -1,    -1,    10,    11,    12,    13,    -1,    26,    -1,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    50,   199,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    63,    64,    65,    66,
      67,    68,    69,    -1,    -1,    -1,    73,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   199,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   197,    -1,   125,   126,
      -1,   128,   129,   130,   131,   132,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   144,   145,   146,
      -1,   148,    -1,    -1,   151,   152,    -1,   154,   155,   156,
     157,   158,   159,    -1,    -1,   162,    -1,    -1,    -1,    -1,
      -1,    -1,   169,   170,    -1,   172,   184,   174,   175,   176,
     177,    -1,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,   197,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,   197,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,   197,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    26,   197,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    73,    -1,    75,    -1,    -1,    -1,    -1,    -1,
      -1,    26,   197,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   196,    26,    -1,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    26,    50,    -1,    -1,
     151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   196,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    52,    -1,    -1,    -1,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,    -1,    -1,    73,    -1,   187,   188,   198,    -1,   200,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,
      71,    72,    73,    26,    -1,    -1,    -1,    -1,    -1,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   183,    52,
      -1,    -1,    -1,    -1,    -1,    -1,   126,   127,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      73,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,   149,
      -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,   182,
      -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,   169,
     151,   152,    -1,   154,   155,   156,    -1,   177,    -1,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,    52,   126,   127,   195,   177,    -1,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   144,    -1,    73,   147,    -1,   149,    -1,   151,   152,
      -1,   154,   155,   156,    -1,    -1,    -1,   160,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,    -1,
      32,    -1,   195,    -1,    -1,    -1,   126,   127,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,   149,
      -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,    -1,
     160,    73,    -1,    75,    -1,    -1,    -1,    -1,    -1,   169,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   177,    -1,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,    -1,    -1,    -1,   195,    -1,    -1,    -1,    -1,
      -1,   113,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   125,    -1,    -1,    -1,    -1,    -1,    43,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   144,    -1,    -1,   147,    -1,   149,    62,   151,
     152,    -1,   154,   155,   156,    -1,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   177,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
      -1,    -1,    -1,   195,    -1,    -1,    -1,    -1,   200,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   126,    -1,   128,   129,   130,   131,   132,    -1,
      43,    44,    -1,    -1,    -1,   139,    -1,    -1,    -1,    -1,
     144,   145,   146,   147,    -1,   149,    -1,   151,   152,    62,
     154,   155,   156,    -1,    -1,   159,    -1,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,   169,    -1,    -1,    81,    -1,
     174,    -1,    -1,   177,    -1,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    64,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,
      75,    -1,    -1,   126,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   151,   152,
      -1,   154,   155,   156,    -1,    -1,    -1,    -1,   113,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,
      73,    -1,    75,    -1,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   144,
      -1,    -1,   147,    -1,   149,    -1,   151,   152,    -1,   154,
     155,   156,    -1,    -1,    -1,    -1,    -1,    -1,   163,    -1,
     113,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,    -1,    -1,    -1,
     195,   144,    -1,    -1,   147,   200,   149,    -1,   151,   152,
      -1,   154,   155,   156,    73,    -1,    75,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,    -1,
      -1,    -1,   195,    -1,   113,   198,    -1,   200,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   125,    -1,    73,    -1,
      75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,
     149,    -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,    -1,
      73,    -1,    75,    -1,    -1,    -1,    -1,    -1,   177,    -1,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,    -1,    -1,    -1,   195,    -1,    -1,   144,
      -1,   200,   147,    -1,   149,    -1,   151,   152,    -1,   154,
     155,   156,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      73,    -1,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   177,    -1,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,    -1,   151,   152,
     195,   154,   155,   156,    -1,   200,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      73,    -1,    -1,    -1,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,    -1,
      -1,    -1,    -1,    -1,    -1,   198,    -1,   200,   151,   152,
      -1,   154,   155,   156,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,    -1,
      -1,   144,    -1,    -1,   147,   198,    -1,   200,   151,   152,
      -1,   154,   155,   156,    -1,   117,    -1,    -1,    -1,    -1,
      73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   177,    73,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   151,
     152,    -1,   154,   155,   156,    -1,   199,    -1,    -1,    -1,
      -1,   114,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   126,    -1,   177,    -1,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
      73,   144,    -1,   195,   147,    -1,   149,    -1,   151,   152,
      -1,   154,   155,   156,    -1,    73,   144,    75,    76,   147,
      -1,   149,    -1,   151,   152,    -1,   154,   155,   156,    -1,
      -1,    -1,    -1,    -1,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   177,
      -1,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,    73,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   147,    -1,    -1,    -1,   151,   152,
      -1,   154,   155,   156,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   151,   152,    -1,   154,   155,   156,    -1,
      -1,    -1,    -1,    -1,   177,    -1,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   177,
      -1,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,    -1,    -1,    -1,    -1,    -1,    -1,
     149,    -1,   151,   152,    -1,   154,   155,   156,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   177,    -1,
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
     181,    75,   325,    75,   325,    75,   325,   358,   359,   325,
     325,   351,   361,   184,   364,   221,   195,   230,    92,   214,
     212,   325,   281,   384,    75,     9,   196,   196,   196,   196,
     196,   197,   212,   444,   121,   257,   195,     9,   196,   196,
      75,    76,   212,   432,   212,    62,   199,   199,   208,   210,
     325,   122,   256,   164,    47,   149,   164,   371,   124,     9,
     388,   196,   449,   449,    14,   193,     9,   389,   449,   450,
     123,   406,   407,   408,   199,     9,   166,   411,   196,     9,
     389,    14,   329,   240,   121,   255,   195,   439,   325,    27,
     202,   202,   124,   199,     9,   388,   325,   440,   195,   247,
     250,   245,   237,    64,   411,   325,   440,   195,   202,   199,
     196,   202,   199,   196,    43,    44,    62,    70,    71,    72,
      81,   126,   139,   169,   177,   212,   391,   393,   396,   399,
     212,   411,   411,   124,   406,   407,   408,   196,   325,   271,
      67,    68,   272,   221,   316,   221,   318,    32,   125,   261,
     411,   384,   212,    27,   223,   265,   197,   268,   197,   268,
       9,   166,   124,     9,   388,   196,   160,   441,   442,   449,
     384,   384,   384,   387,   390,   195,    80,   143,   195,   195,
     143,   198,   325,   181,   181,    14,   187,   188,   360,     9,
     191,   364,    75,   199,   377,   198,   234,   212,   199,    14,
     411,   197,    92,     9,   166,   258,   377,   198,   423,   125,
     411,    14,   202,   325,   199,   208,   258,   198,   370,    14,
     325,   335,   197,   449,    27,   443,   160,   405,    32,    75,
     198,   212,   416,   449,    32,   325,   384,   276,   195,   377,
     256,   330,   241,   325,   325,   325,   199,   195,   278,   257,
     256,   255,   439,   379,   199,   195,   278,    14,    70,    71,
      72,   212,   392,   392,   393,   394,   395,   195,    80,   140,
     195,   195,     9,   388,   196,   400,    32,   325,   199,    67,
      68,   273,   316,   223,   199,   197,    85,   197,   411,   195,
     124,   260,    14,   221,   268,    94,    95,    96,   268,   199,
     449,   449,   445,     9,   196,   196,   124,   202,     9,   388,
     387,   212,   335,   337,   339,   387,   119,   212,   384,   428,
     429,   325,   325,   325,   359,   325,   349,    75,   235,   384,
     449,   212,     9,   283,   196,   195,   319,   322,   325,   202,
     199,   283,   150,   163,   198,   366,   373,   150,   198,   372,
     124,   197,   449,   334,   450,    75,    14,   325,   440,   195,
     411,   196,   276,   198,   276,   195,   124,   195,   278,   196,
     198,   198,   256,   242,   382,   195,   278,   196,   124,   202,
       9,   388,   394,   140,   335,   397,   398,   394,   393,   411,
     316,    27,    69,   223,   197,   318,   423,   261,   196,   384,
      91,    94,   197,   325,    27,   197,   269,   199,   166,   160,
      27,   384,   384,   196,   124,     9,   388,   196,   196,   124,
     199,     9,   388,   182,   196,   221,    92,   377,     4,   101,
     106,   114,   151,   152,   154,   199,   284,   307,   308,   309,
     314,   404,   423,   199,   199,    47,   325,   325,   325,    32,
      75,    14,   384,   199,   195,   278,   443,   196,   283,   196,
     276,   325,   278,   196,   283,   283,   198,   195,   278,   196,
     393,   393,   196,   124,   196,     9,   388,   196,    27,   221,
     197,   196,   196,   228,   197,   197,   269,   221,   449,   124,
     384,   335,   384,   384,   325,   198,   199,   449,   121,   122,
     438,   259,   377,   114,   126,   149,   155,   293,   294,   295,
     377,   153,   299,   300,   117,   195,   212,   301,   302,   285,
     238,   449,     9,   197,   308,   196,   149,   368,   199,   199,
      75,    14,   384,   195,   278,   196,   106,   327,   443,   199,
     443,   196,   196,   199,   199,   283,   276,   196,   124,   393,
     335,   221,   226,    27,   223,   263,   221,   196,   384,   124,
     124,   183,   221,   377,   377,    14,     9,   197,   198,   198,
       9,   197,     3,     4,     5,     6,     7,    10,    11,    12,
      13,    50,    63,    64,    65,    66,    67,    68,    69,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     125,   126,   128,   129,   130,   131,   132,   144,   145,   146,
     148,   157,   158,   159,   162,   169,   170,   172,   174,   175,
     176,   212,   374,   375,     9,   197,   149,   153,   212,   302,
     303,   304,   197,    75,   313,   237,   286,   438,   238,    14,
     384,   278,   196,   195,   198,   197,   198,   305,   327,   443,
     199,   196,   393,   124,    27,   223,   262,   221,   384,   384,
     325,   199,   197,   197,   384,   377,   289,   296,   383,   294,
      14,    27,    44,   297,   300,     9,    30,   196,    26,    43,
      46,    14,     9,   197,   439,   313,    14,   237,   384,   196,
      32,    75,   365,   221,   221,   198,   305,   443,   393,   221,
      89,   184,   233,   199,   212,   219,   290,   291,   292,     9,
     199,   384,   375,   375,    52,   298,   303,   303,    26,    43,
      46,   384,    75,   195,   197,   384,   439,    75,     9,   389,
     199,   199,   221,   305,    87,   197,    75,   104,   229,   143,
      92,   383,   156,    14,   287,   195,    32,    75,   196,   199,
     197,   195,   162,   236,   212,   308,   309,   384,   160,   274,
     275,   405,   288,    75,   377,   234,   158,   212,   197,   196,
       9,   389,   108,   109,   110,   311,   312,   274,    75,   259,
     197,   443,   160,   405,   450,   196,   196,   197,   197,   198,
     306,   311,    32,    75,   443,   198,   221,   450,    75,    14,
     306,   221,   199,    32,    75,    14,   384,   199,    75,    14,
     384,    14,   384,   384
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
#line 727 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 730 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 737 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));}
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 738 "hphp.y"
    { }
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 741 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 742 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 743 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 744 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 745 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 746 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 749 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();}
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 751 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());}
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 752 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);}
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 753 "hphp.y"
    { _p->onNamespaceStart("");}
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 754 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);}
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 755 "hphp.y"
    { _p->nns(); (yyval).reset();}
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 756 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;}
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 761 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 762 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 763 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 764 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 765 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 766 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 769 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 771 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 778 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 780 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 785 "hphp.y"
    { }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 786 "hphp.y"
    { }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 789 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");}
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 790 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");}
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 791 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());}
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 793 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());}
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 799 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;}
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 802 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;}
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 804 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));}
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 805 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;}
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 808 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 815 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 822 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 830 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 833 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 839 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 840 "hphp.y"
    { _p->onStatementListStart((yyval));}
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 843 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 845 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 846 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 849 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 853 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 858 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));}
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 859 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 861 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 865 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 868 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 872 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 874 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 877 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 879 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 882 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);}
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 883 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));}
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 884 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);}
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 885 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));}
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 886 "hphp.y"
    { _p->onReturn((yyval), NULL);}
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 887 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));}
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 888 "hphp.y"
    { _p->onYieldBreak((yyval));}
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 889 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 890 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 891 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);}
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 892 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));}
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 893 "hphp.y"
    { (yyval).reset(); (yyval) = ';';}
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 894 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);}
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 897 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 899 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 903 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;}
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 910 "hphp.y"
    { _p->onCompleteLabelScope(false);}
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 911 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));}
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 914 "hphp.y"
    { _p->onCompleteLabelScope(false);}
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 915 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));}
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 916 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 917 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval));}
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 921 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 922 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 923 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 924 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 925 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 926 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 927 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); }
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 928 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 929 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 930 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 931 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 939 "hphp.y"
    { _p->onNewLabelScope(false);}
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 940 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 949 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));}
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 950 "hphp.y"
    { (yyval).reset();}
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 954 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 956 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 962 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 963 "hphp.y"
    { (yyval).reset();}
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 967 "hphp.y"
    { (yyval) = 1;}
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 968 "hphp.y"
    { (yyval).reset();}
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 972 "hphp.y"
    { _p->pushFuncLocation(); }
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 977 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();}
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 983 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 989 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();}
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 995 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 1001 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();}
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 1007 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 1015 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));}
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 1018 "hphp.y"
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
#line 1033 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));}
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 1036 "hphp.y"
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
#line 1050 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));}
    break;

  case 127:

/* Line 1806 of yacc.c  */
#line 1053 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 128:

/* Line 1806 of yacc.c  */
#line 1058 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));}
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 1061 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 130:

/* Line 1806 of yacc.c  */
#line 1068 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));}
    break;

  case 131:

/* Line 1806 of yacc.c  */
#line 1071 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0);
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 1079 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));}
    break;

  case 133:

/* Line 1806 of yacc.c  */
#line 1082 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 1090 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 1091 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 136:

/* Line 1806 of yacc.c  */
#line 1095 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 1098 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 1101 "hphp.y"
    { (yyval) = T_CLASS;}
    break;

  case 139:

/* Line 1806 of yacc.c  */
#line 1102 "hphp.y"
    { (yyval) = T_ABSTRACT;}
    break;

  case 140:

/* Line 1806 of yacc.c  */
#line 1103 "hphp.y"
    { (yyval) = T_FINAL;}
    break;

  case 141:

/* Line 1806 of yacc.c  */
#line 1107 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 142:

/* Line 1806 of yacc.c  */
#line 1108 "hphp.y"
    { (yyval).reset();}
    break;

  case 143:

/* Line 1806 of yacc.c  */
#line 1111 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 1112 "hphp.y"
    { (yyval).reset();}
    break;

  case 145:

/* Line 1806 of yacc.c  */
#line 1115 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 146:

/* Line 1806 of yacc.c  */
#line 1116 "hphp.y"
    { (yyval).reset();}
    break;

  case 147:

/* Line 1806 of yacc.c  */
#line 1119 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 148:

/* Line 1806 of yacc.c  */
#line 1121 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 149:

/* Line 1806 of yacc.c  */
#line 1124 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 150:

/* Line 1806 of yacc.c  */
#line 1126 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 151:

/* Line 1806 of yacc.c  */
#line 1130 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 152:

/* Line 1806 of yacc.c  */
#line 1131 "hphp.y"
    { (yyval).reset();}
    break;

  case 153:

/* Line 1806 of yacc.c  */
#line 1134 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 154:

/* Line 1806 of yacc.c  */
#line 1135 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;}
    break;

  case 155:

/* Line 1806 of yacc.c  */
#line 1136 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);}
    break;

  case 156:

/* Line 1806 of yacc.c  */
#line 1140 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 157:

/* Line 1806 of yacc.c  */
#line 1142 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 158:

/* Line 1806 of yacc.c  */
#line 1145 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 159:

/* Line 1806 of yacc.c  */
#line 1147 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 160:

/* Line 1806 of yacc.c  */
#line 1150 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 161:

/* Line 1806 of yacc.c  */
#line 1152 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 162:

/* Line 1806 of yacc.c  */
#line 1155 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 163:

/* Line 1806 of yacc.c  */
#line 1157 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 166:

/* Line 1806 of yacc.c  */
#line 1167 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 167:

/* Line 1806 of yacc.c  */
#line 1168 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 168:

/* Line 1806 of yacc.c  */
#line 1169 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 169:

/* Line 1806 of yacc.c  */
#line 1170 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);}
    break;

  case 170:

/* Line 1806 of yacc.c  */
#line 1175 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 171:

/* Line 1806 of yacc.c  */
#line 1177 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));}
    break;

  case 172:

/* Line 1806 of yacc.c  */
#line 1178 "hphp.y"
    { (yyval).reset();}
    break;

  case 173:

/* Line 1806 of yacc.c  */
#line 1181 "hphp.y"
    { (yyval).reset();}
    break;

  case 174:

/* Line 1806 of yacc.c  */
#line 1182 "hphp.y"
    { (yyval).reset();}
    break;

  case 175:

/* Line 1806 of yacc.c  */
#line 1187 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 176:

/* Line 1806 of yacc.c  */
#line 1188 "hphp.y"
    { (yyval).reset();}
    break;

  case 177:

/* Line 1806 of yacc.c  */
#line 1193 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 178:

/* Line 1806 of yacc.c  */
#line 1194 "hphp.y"
    { (yyval).reset();}
    break;

  case 179:

/* Line 1806 of yacc.c  */
#line 1197 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 180:

/* Line 1806 of yacc.c  */
#line 1198 "hphp.y"
    { (yyval).reset();}
    break;

  case 181:

/* Line 1806 of yacc.c  */
#line 1201 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);}
    break;

  case 182:

/* Line 1806 of yacc.c  */
#line 1202 "hphp.y"
    { (yyval).reset();}
    break;

  case 183:

/* Line 1806 of yacc.c  */
#line 1207 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]);}
    break;

  case 184:

/* Line 1806 of yacc.c  */
#line 1209 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 185:

/* Line 1806 of yacc.c  */
#line 1210 "hphp.y"
    { (yyval).reset();}
    break;

  case 186:

/* Line 1806 of yacc.c  */
#line 1211 "hphp.y"
    { (yyval).reset();}
    break;

  case 187:

/* Line 1806 of yacc.c  */
#line 1217 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));}
    break;

  case 188:

/* Line 1806 of yacc.c  */
#line 1221 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));}
    break;

  case 189:

/* Line 1806 of yacc.c  */
#line 1226 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));}
    break;

  case 190:

/* Line 1806 of yacc.c  */
#line 1231 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));}
    break;

  case 191:

/* Line 1806 of yacc.c  */
#line 1236 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));}
    break;

  case 192:

/* Line 1806 of yacc.c  */
#line 1241 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));}
    break;

  case 193:

/* Line 1806 of yacc.c  */
#line 1247 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));}
    break;

  case 194:

/* Line 1806 of yacc.c  */
#line 1253 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));}
    break;

  case 195:

/* Line 1806 of yacc.c  */
#line 1259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]);}
    break;

  case 196:

/* Line 1806 of yacc.c  */
#line 1261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 197:

/* Line 1806 of yacc.c  */
#line 1262 "hphp.y"
    { (yyval).reset();}
    break;

  case 198:

/* Line 1806 of yacc.c  */
#line 1263 "hphp.y"
    { (yyval).reset();}
    break;

  case 199:

/* Line 1806 of yacc.c  */
#line 1268 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),0,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL);}
    break;

  case 200:

/* Line 1806 of yacc.c  */
#line 1271 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),1,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL);}
    break;

  case 201:

/* Line 1806 of yacc.c  */
#line 1275 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),1,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL);}
    break;

  case 202:

/* Line 1806 of yacc.c  */
#line 1279 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),0,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL);}
    break;

  case 203:

/* Line 1806 of yacc.c  */
#line 1283 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),0,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL);}
    break;

  case 204:

/* Line 1806 of yacc.c  */
#line 1287 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),1,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL);}
    break;

  case 205:

/* Line 1806 of yacc.c  */
#line 1292 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),1,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL);}
    break;

  case 206:

/* Line 1806 of yacc.c  */
#line 1297 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),0,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL);}
    break;

  case 207:

/* Line 1806 of yacc.c  */
#line 1303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 208:

/* Line 1806 of yacc.c  */
#line 1304 "hphp.y"
    { (yyval).reset();}
    break;

  case 209:

/* Line 1806 of yacc.c  */
#line 1307 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),0);}
    break;

  case 210:

/* Line 1806 of yacc.c  */
#line 1308 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),1);}
    break;

  case 211:

/* Line 1806 of yacc.c  */
#line 1310 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 212:

/* Line 1806 of yacc.c  */
#line 1312 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 213:

/* Line 1806 of yacc.c  */
#line 1316 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 214:

/* Line 1806 of yacc.c  */
#line 1317 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 215:

/* Line 1806 of yacc.c  */
#line 1320 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 216:

/* Line 1806 of yacc.c  */
#line 1321 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;}
    break;

  case 217:

/* Line 1806 of yacc.c  */
#line 1322 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;}
    break;

  case 218:

/* Line 1806 of yacc.c  */
#line 1326 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 219:

/* Line 1806 of yacc.c  */
#line 1328 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));}
    break;

  case 220:

/* Line 1806 of yacc.c  */
#line 1329 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);}
    break;

  case 221:

/* Line 1806 of yacc.c  */
#line 1330 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 222:

/* Line 1806 of yacc.c  */
#line 1335 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 223:

/* Line 1806 of yacc.c  */
#line 1336 "hphp.y"
    { (yyval).reset();}
    break;

  case 224:

/* Line 1806 of yacc.c  */
#line 1339 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));}
    break;

  case 225:

/* Line 1806 of yacc.c  */
#line 1340 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 226:

/* Line 1806 of yacc.c  */
#line 1343 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));}
    break;

  case 227:

/* Line 1806 of yacc.c  */
#line 1344 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));}
    break;

  case 228:

/* Line 1806 of yacc.c  */
#line 1346 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);}
    break;

  case 229:

/* Line 1806 of yacc.c  */
#line 1350 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();}
    break;

  case 230:

/* Line 1806 of yacc.c  */
#line 1356 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 231:

/* Line 1806 of yacc.c  */
#line 1363 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();}
    break;

  case 232:

/* Line 1806 of yacc.c  */
#line 1369 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 233:

/* Line 1806 of yacc.c  */
#line 1374 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));}
    break;

  case 234:

/* Line 1806 of yacc.c  */
#line 1376 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));}
    break;

  case 235:

/* Line 1806 of yacc.c  */
#line 1378 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));}
    break;

  case 236:

/* Line 1806 of yacc.c  */
#line 1380 "hphp.y"
    { _p->onTraitRequire((yyval), (yyvsp[(3) - (4)]), true); }
    break;

  case 237:

/* Line 1806 of yacc.c  */
#line 1382 "hphp.y"
    { _p->onTraitRequire((yyval), (yyvsp[(3) - (4)]), false); }
    break;

  case 238:

/* Line 1806 of yacc.c  */
#line 1383 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); }
    break;

  case 239:

/* Line 1806 of yacc.c  */
#line 1386 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); }
    break;

  case 240:

/* Line 1806 of yacc.c  */
#line 1389 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); }
    break;

  case 241:

/* Line 1806 of yacc.c  */
#line 1390 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); }
    break;

  case 242:

/* Line 1806 of yacc.c  */
#line 1391 "hphp.y"
    { (yyval).reset(); }
    break;

  case 243:

/* Line 1806 of yacc.c  */
#line 1397 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 244:

/* Line 1806 of yacc.c  */
#line 1401 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));}
    break;

  case 245:

/* Line 1806 of yacc.c  */
#line 1404 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);}
    break;

  case 246:

/* Line 1806 of yacc.c  */
#line 1411 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 247:

/* Line 1806 of yacc.c  */
#line 1412 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));}
    break;

  case 248:

/* Line 1806 of yacc.c  */
#line 1417 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));}
    break;

  case 249:

/* Line 1806 of yacc.c  */
#line 1420 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 250:

/* Line 1806 of yacc.c  */
#line 1427 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;}
    break;

  case 251:

/* Line 1806 of yacc.c  */
#line 1429 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;}
    break;

  case 252:

/* Line 1806 of yacc.c  */
#line 1433 "hphp.y"
    { (yyval) = 4;}
    break;

  case 253:

/* Line 1806 of yacc.c  */
#line 1434 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));}
    break;

  case 254:

/* Line 1806 of yacc.c  */
#line 1440 "hphp.y"
    { (yyval) = 6;}
    break;

  case 255:

/* Line 1806 of yacc.c  */
#line 1442 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;}
    break;

  case 256:

/* Line 1806 of yacc.c  */
#line 1446 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);}
    break;

  case 257:

/* Line 1806 of yacc.c  */
#line 1448 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);}
    break;

  case 258:

/* Line 1806 of yacc.c  */
#line 1452 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 259:

/* Line 1806 of yacc.c  */
#line 1453 "hphp.y"
    { scalar_null(_p, (yyval));}
    break;

  case 260:

/* Line 1806 of yacc.c  */
#line 1457 "hphp.y"
    { scalar_num(_p, (yyval), "1");}
    break;

  case 261:

/* Line 1806 of yacc.c  */
#line 1458 "hphp.y"
    { scalar_num(_p, (yyval), "0");}
    break;

  case 262:

/* Line 1806 of yacc.c  */
#line 1462 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);}
    break;

  case 263:

/* Line 1806 of yacc.c  */
#line 1465 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);}
    break;

  case 264:

/* Line 1806 of yacc.c  */
#line 1470 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 265:

/* Line 1806 of yacc.c  */
#line 1475 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;}
    break;

  case 266:

/* Line 1806 of yacc.c  */
#line 1476 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;}
    break;

  case 267:

/* Line 1806 of yacc.c  */
#line 1478 "hphp.y"
    { (yyval) = 0;}
    break;

  case 268:

/* Line 1806 of yacc.c  */
#line 1482 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);}
    break;

  case 269:

/* Line 1806 of yacc.c  */
#line 1483 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);}
    break;

  case 270:

/* Line 1806 of yacc.c  */
#line 1484 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);}
    break;

  case 271:

/* Line 1806 of yacc.c  */
#line 1485 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);}
    break;

  case 272:

/* Line 1806 of yacc.c  */
#line 1489 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 273:

/* Line 1806 of yacc.c  */
#line 1490 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);}
    break;

  case 274:

/* Line 1806 of yacc.c  */
#line 1491 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);}
    break;

  case 275:

/* Line 1806 of yacc.c  */
#line 1492 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);}
    break;

  case 276:

/* Line 1806 of yacc.c  */
#line 1493 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);}
    break;

  case 277:

/* Line 1806 of yacc.c  */
#line 1495 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));}
    break;

  case 278:

/* Line 1806 of yacc.c  */
#line 1497 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));}
    break;

  case 279:

/* Line 1806 of yacc.c  */
#line 1501 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;}
    break;

  case 280:

/* Line 1806 of yacc.c  */
#line 1504 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;}
    break;

  case 281:

/* Line 1806 of yacc.c  */
#line 1505 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;}
    break;

  case 282:

/* Line 1806 of yacc.c  */
#line 1509 "hphp.y"
    { (yyval).reset();}
    break;

  case 283:

/* Line 1806 of yacc.c  */
#line 1510 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;}
    break;

  case 284:

/* Line 1806 of yacc.c  */
#line 1514 "hphp.y"
    { (yyval).reset();}
    break;

  case 285:

/* Line 1806 of yacc.c  */
#line 1515 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;}
    break;

  case 286:

/* Line 1806 of yacc.c  */
#line 1518 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 287:

/* Line 1806 of yacc.c  */
#line 1519 "hphp.y"
    { (yyval).reset();}
    break;

  case 288:

/* Line 1806 of yacc.c  */
#line 1522 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 289:

/* Line 1806 of yacc.c  */
#line 1523 "hphp.y"
    { (yyval).reset();}
    break;

  case 290:

/* Line 1806 of yacc.c  */
#line 1526 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));}
    break;

  case 291:

/* Line 1806 of yacc.c  */
#line 1528 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 292:

/* Line 1806 of yacc.c  */
#line 1531 "hphp.y"
    { (yyval) = T_PUBLIC;}
    break;

  case 293:

/* Line 1806 of yacc.c  */
#line 1532 "hphp.y"
    { (yyval) = T_PROTECTED;}
    break;

  case 294:

/* Line 1806 of yacc.c  */
#line 1533 "hphp.y"
    { (yyval) = T_PRIVATE;}
    break;

  case 295:

/* Line 1806 of yacc.c  */
#line 1534 "hphp.y"
    { (yyval) = T_STATIC;}
    break;

  case 296:

/* Line 1806 of yacc.c  */
#line 1535 "hphp.y"
    { (yyval) = T_ABSTRACT;}
    break;

  case 297:

/* Line 1806 of yacc.c  */
#line 1536 "hphp.y"
    { (yyval) = T_FINAL;}
    break;

  case 298:

/* Line 1806 of yacc.c  */
#line 1537 "hphp.y"
    { (yyval) = T_ASYNC;}
    break;

  case 299:

/* Line 1806 of yacc.c  */
#line 1541 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 300:

/* Line 1806 of yacc.c  */
#line 1542 "hphp.y"
    { (yyval).reset();}
    break;

  case 301:

/* Line 1806 of yacc.c  */
#line 1545 "hphp.y"
    { (yyval) = T_PUBLIC;}
    break;

  case 302:

/* Line 1806 of yacc.c  */
#line 1546 "hphp.y"
    { (yyval) = T_PROTECTED;}
    break;

  case 303:

/* Line 1806 of yacc.c  */
#line 1547 "hphp.y"
    { (yyval) = T_PRIVATE;}
    break;

  case 304:

/* Line 1806 of yacc.c  */
#line 1551 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 305:

/* Line 1806 of yacc.c  */
#line 1553 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));}
    break;

  case 306:

/* Line 1806 of yacc.c  */
#line 1554 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);}
    break;

  case 307:

/* Line 1806 of yacc.c  */
#line 1555 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 308:

/* Line 1806 of yacc.c  */
#line 1559 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 309:

/* Line 1806 of yacc.c  */
#line 1560 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 310:

/* Line 1806 of yacc.c  */
#line 1564 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 311:

/* Line 1806 of yacc.c  */
#line 1566 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 312:

/* Line 1806 of yacc.c  */
#line 1567 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);}
    break;

  case 313:

/* Line 1806 of yacc.c  */
#line 1568 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 314:

/* Line 1806 of yacc.c  */
#line 1569 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 315:

/* Line 1806 of yacc.c  */
#line 1572 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 316:

/* Line 1806 of yacc.c  */
#line 1576 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 317:

/* Line 1806 of yacc.c  */
#line 1577 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 318:

/* Line 1806 of yacc.c  */
#line 1581 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 319:

/* Line 1806 of yacc.c  */
#line 1582 "hphp.y"
    { (yyval).reset();}
    break;

  case 320:

/* Line 1806 of yacc.c  */
#line 1586 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));}
    break;

  case 321:

/* Line 1806 of yacc.c  */
#line 1587 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 322:

/* Line 1806 of yacc.c  */
#line 1591 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 323:

/* Line 1806 of yacc.c  */
#line 1596 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);}
    break;

  case 324:

/* Line 1806 of yacc.c  */
#line 1600 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 325:

/* Line 1806 of yacc.c  */
#line 1604 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 326:

/* Line 1806 of yacc.c  */
#line 1609 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);}
    break;

  case 327:

/* Line 1806 of yacc.c  */
#line 1613 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 328:

/* Line 1806 of yacc.c  */
#line 1614 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 329:

/* Line 1806 of yacc.c  */
#line 1615 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 330:

/* Line 1806 of yacc.c  */
#line 1619 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));}
    break;

  case 331:

/* Line 1806 of yacc.c  */
#line 1620 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 332:

/* Line 1806 of yacc.c  */
#line 1621 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);}
    break;

  case 333:

/* Line 1806 of yacc.c  */
#line 1624 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));}
    break;

  case 334:

/* Line 1806 of yacc.c  */
#line 1625 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);}
    break;

  case 335:

/* Line 1806 of yacc.c  */
#line 1626 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);}
    break;

  case 336:

/* Line 1806 of yacc.c  */
#line 1627 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);}
    break;

  case 337:

/* Line 1806 of yacc.c  */
#line 1628 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);}
    break;

  case 338:

/* Line 1806 of yacc.c  */
#line 1629 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);}
    break;

  case 339:

/* Line 1806 of yacc.c  */
#line 1630 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);}
    break;

  case 340:

/* Line 1806 of yacc.c  */
#line 1631 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);}
    break;

  case 341:

/* Line 1806 of yacc.c  */
#line 1632 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);}
    break;

  case 342:

/* Line 1806 of yacc.c  */
#line 1633 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);}
    break;

  case 343:

/* Line 1806 of yacc.c  */
#line 1634 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);}
    break;

  case 344:

/* Line 1806 of yacc.c  */
#line 1635 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);}
    break;

  case 345:

/* Line 1806 of yacc.c  */
#line 1636 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);}
    break;

  case 346:

/* Line 1806 of yacc.c  */
#line 1637 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);}
    break;

  case 347:

/* Line 1806 of yacc.c  */
#line 1638 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);}
    break;

  case 348:

/* Line 1806 of yacc.c  */
#line 1639 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);}
    break;

  case 349:

/* Line 1806 of yacc.c  */
#line 1640 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);}
    break;

  case 350:

/* Line 1806 of yacc.c  */
#line 1641 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);}
    break;

  case 351:

/* Line 1806 of yacc.c  */
#line 1642 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);}
    break;

  case 352:

/* Line 1806 of yacc.c  */
#line 1643 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);}
    break;

  case 353:

/* Line 1806 of yacc.c  */
#line 1644 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);}
    break;

  case 354:

/* Line 1806 of yacc.c  */
#line 1645 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');}
    break;

  case 355:

/* Line 1806 of yacc.c  */
#line 1646 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');}
    break;

  case 356:

/* Line 1806 of yacc.c  */
#line 1647 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');}
    break;

  case 357:

/* Line 1806 of yacc.c  */
#line 1648 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');}
    break;

  case 358:

/* Line 1806 of yacc.c  */
#line 1649 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');}
    break;

  case 359:

/* Line 1806 of yacc.c  */
#line 1650 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');}
    break;

  case 360:

/* Line 1806 of yacc.c  */
#line 1651 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');}
    break;

  case 361:

/* Line 1806 of yacc.c  */
#line 1652 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');}
    break;

  case 362:

/* Line 1806 of yacc.c  */
#line 1653 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');}
    break;

  case 363:

/* Line 1806 of yacc.c  */
#line 1654 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);}
    break;

  case 364:

/* Line 1806 of yacc.c  */
#line 1655 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);}
    break;

  case 365:

/* Line 1806 of yacc.c  */
#line 1656 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 366:

/* Line 1806 of yacc.c  */
#line 1657 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 367:

/* Line 1806 of yacc.c  */
#line 1658 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);}
    break;

  case 368:

/* Line 1806 of yacc.c  */
#line 1659 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);}
    break;

  case 369:

/* Line 1806 of yacc.c  */
#line 1660 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);}
    break;

  case 370:

/* Line 1806 of yacc.c  */
#line 1661 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);}
    break;

  case 371:

/* Line 1806 of yacc.c  */
#line 1662 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);}
    break;

  case 372:

/* Line 1806 of yacc.c  */
#line 1663 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);}
    break;

  case 373:

/* Line 1806 of yacc.c  */
#line 1664 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');}
    break;

  case 374:

/* Line 1806 of yacc.c  */
#line 1665 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);}
    break;

  case 375:

/* Line 1806 of yacc.c  */
#line 1667 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');}
    break;

  case 376:

/* Line 1806 of yacc.c  */
#line 1668 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);}
    break;

  case 377:

/* Line 1806 of yacc.c  */
#line 1671 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);}
    break;

  case 378:

/* Line 1806 of yacc.c  */
#line 1672 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 379:

/* Line 1806 of yacc.c  */
#line 1673 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));}
    break;

  case 380:

/* Line 1806 of yacc.c  */
#line 1674 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));}
    break;

  case 381:

/* Line 1806 of yacc.c  */
#line 1675 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 382:

/* Line 1806 of yacc.c  */
#line 1676 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);}
    break;

  case 383:

/* Line 1806 of yacc.c  */
#line 1677 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);}
    break;

  case 384:

/* Line 1806 of yacc.c  */
#line 1678 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);}
    break;

  case 385:

/* Line 1806 of yacc.c  */
#line 1679 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);}
    break;

  case 386:

/* Line 1806 of yacc.c  */
#line 1680 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);}
    break;

  case 387:

/* Line 1806 of yacc.c  */
#line 1681 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);}
    break;

  case 388:

/* Line 1806 of yacc.c  */
#line 1682 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);}
    break;

  case 389:

/* Line 1806 of yacc.c  */
#line 1683 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);}
    break;

  case 390:

/* Line 1806 of yacc.c  */
#line 1684 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);}
    break;

  case 391:

/* Line 1806 of yacc.c  */
#line 1685 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 392:

/* Line 1806 of yacc.c  */
#line 1686 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 393:

/* Line 1806 of yacc.c  */
#line 1687 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 394:

/* Line 1806 of yacc.c  */
#line 1688 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));}
    break;

  case 395:

/* Line 1806 of yacc.c  */
#line 1689 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);}
    break;

  case 396:

/* Line 1806 of yacc.c  */
#line 1690 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 397:

/* Line 1806 of yacc.c  */
#line 1691 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 398:

/* Line 1806 of yacc.c  */
#line 1692 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 399:

/* Line 1806 of yacc.c  */
#line 1699 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);}
    break;

  case 400:

/* Line 1806 of yacc.c  */
#line 1700 "hphp.y"
    { (yyval).reset();}
    break;

  case 401:

/* Line 1806 of yacc.c  */
#line 1705 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
    break;

  case 402:

/* Line 1806 of yacc.c  */
#line 1711 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 403:

/* Line 1806 of yacc.c  */
#line 1719 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
    break;

  case 404:

/* Line 1806 of yacc.c  */
#line 1725 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 405:

/* Line 1806 of yacc.c  */
#line 1734 "hphp.y"
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
#line 1742 "hphp.y"
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
#line 1749 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
    break;

  case 408:

/* Line 1806 of yacc.c  */
#line 1757 "hphp.y"
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
#line 1767 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));}
    break;

  case 410:

/* Line 1806 of yacc.c  */
#line 1769 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 411:

/* Line 1806 of yacc.c  */
#line 1773 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); }
    break;

  case 412:

/* Line 1806 of yacc.c  */
#line 1781 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 413:

/* Line 1806 of yacc.c  */
#line 1784 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 414:

/* Line 1806 of yacc.c  */
#line 1791 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 415:

/* Line 1806 of yacc.c  */
#line 1794 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 416:

/* Line 1806 of yacc.c  */
#line 1799 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 417:

/* Line 1806 of yacc.c  */
#line 1800 "hphp.y"
    { (yyval).reset(); }
    break;

  case 418:

/* Line 1806 of yacc.c  */
#line 1805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 419:

/* Line 1806 of yacc.c  */
#line 1806 "hphp.y"
    { (yyval).reset(); }
    break;

  case 420:

/* Line 1806 of yacc.c  */
#line 1810 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);}
    break;

  case 421:

/* Line 1806 of yacc.c  */
#line 1814 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 422:

/* Line 1806 of yacc.c  */
#line 1815 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 423:

/* Line 1806 of yacc.c  */
#line 1820 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);}
    break;

  case 424:

/* Line 1806 of yacc.c  */
#line 1827 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);}
    break;

  case 425:

/* Line 1806 of yacc.c  */
#line 1834 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 426:

/* Line 1806 of yacc.c  */
#line 1836 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 427:

/* Line 1806 of yacc.c  */
#line 1840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 428:

/* Line 1806 of yacc.c  */
#line 1841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 429:

/* Line 1806 of yacc.c  */
#line 1842 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 430:

/* Line 1806 of yacc.c  */
#line 1846 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); }
    break;

  case 431:

/* Line 1806 of yacc.c  */
#line 1850 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 432:

/* Line 1806 of yacc.c  */
#line 1854 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 433:

/* Line 1806 of yacc.c  */
#line 1859 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); }
    break;

  case 434:

/* Line 1806 of yacc.c  */
#line 1861 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); }
    break;

  case 435:

/* Line 1806 of yacc.c  */
#line 1863 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); }
    break;

  case 436:

/* Line 1806 of yacc.c  */
#line 1865 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); }
    break;

  case 437:

/* Line 1806 of yacc.c  */
#line 1869 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); }
    break;

  case 438:

/* Line 1806 of yacc.c  */
#line 1871 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); }
    break;

  case 439:

/* Line 1806 of yacc.c  */
#line 1875 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 440:

/* Line 1806 of yacc.c  */
#line 1876 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 441:

/* Line 1806 of yacc.c  */
#line 1877 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 442:

/* Line 1806 of yacc.c  */
#line 1878 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 443:

/* Line 1806 of yacc.c  */
#line 1879 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 444:

/* Line 1806 of yacc.c  */
#line 1880 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 445:

/* Line 1806 of yacc.c  */
#line 1884 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 446:

/* Line 1806 of yacc.c  */
#line 1888 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 447:

/* Line 1806 of yacc.c  */
#line 1892 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 448:

/* Line 1806 of yacc.c  */
#line 1897 "hphp.y"
    { _p->onJoinClause((yyval), NULL, (yyvsp[(2) - (2)]), NULL, NULL); }
    break;

  case 449:

/* Line 1806 of yacc.c  */
#line 1899 "hphp.y"
    { _p->onJoinClause((yyval), &(yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]), NULL, NULL); }
    break;

  case 450:

/* Line 1806 of yacc.c  */
#line 1901 "hphp.y"
    { _p->onJoinClause((yyval), &(yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), &(yyvsp[(6) - (8)]), &(yyvsp[(8) - (8)])); }
    break;

  case 451:

/* Line 1806 of yacc.c  */
#line 1906 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); }
    break;

  case 452:

/* Line 1806 of yacc.c  */
#line 1910 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 453:

/* Line 1806 of yacc.c  */
#line 1914 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); }
    break;

  case 454:

/* Line 1806 of yacc.c  */
#line 1915 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); }
    break;

  case 455:

/* Line 1806 of yacc.c  */
#line 1919 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); }
    break;

  case 456:

/* Line 1806 of yacc.c  */
#line 1920 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); }
    break;

  case 457:

/* Line 1806 of yacc.c  */
#line 1924 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 458:

/* Line 1806 of yacc.c  */
#line 1925 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 459:

/* Line 1806 of yacc.c  */
#line 1929 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 460:

/* Line 1806 of yacc.c  */
#line 1930 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 461:

/* Line 1806 of yacc.c  */
#line 1934 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 462:

/* Line 1806 of yacc.c  */
#line 1938 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 463:

/* Line 1806 of yacc.c  */
#line 1942 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); }
    break;

  case 464:

/* Line 1806 of yacc.c  */
#line 1946 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 465:

/* Line 1806 of yacc.c  */
#line 1947 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 466:

/* Line 1806 of yacc.c  */
#line 1948 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 467:

/* Line 1806 of yacc.c  */
#line 1949 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);}
    break;

  case 468:

/* Line 1806 of yacc.c  */
#line 1956 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));}
    break;

  case 469:

/* Line 1806 of yacc.c  */
#line 1959 "hphp.y"
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

  case 470:

/* Line 1806 of yacc.c  */
#line 1970 "hphp.y"
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

  case 471:

/* Line 1806 of yacc.c  */
#line 1981 "hphp.y"
    { (yyval).reset(); (yyval).setText("");}
    break;

  case 472:

/* Line 1806 of yacc.c  */
#line 1982 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));}
    break;

  case 473:

/* Line 1806 of yacc.c  */
#line 1987 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);}
    break;

  case 474:

/* Line 1806 of yacc.c  */
#line 1988 "hphp.y"
    { (yyval).reset();}
    break;

  case 475:

/* Line 1806 of yacc.c  */
#line 1991 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);}
    break;

  case 476:

/* Line 1806 of yacc.c  */
#line 1992 "hphp.y"
    { (yyval).reset();}
    break;

  case 477:

/* Line 1806 of yacc.c  */
#line 1995 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 478:

/* Line 1806 of yacc.c  */
#line 1999 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 479:

/* Line 1806 of yacc.c  */
#line 2002 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 480:

/* Line 1806 of yacc.c  */
#line 2005 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       }
    break;

  case 481:

/* Line 1806 of yacc.c  */
#line 2012 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 482:

/* Line 1806 of yacc.c  */
#line 2013 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 483:

/* Line 1806 of yacc.c  */
#line 2017 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 484:

/* Line 1806 of yacc.c  */
#line 2019 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);}
    break;

  case 485:

/* Line 1806 of yacc.c  */
#line 2021 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);}
    break;

  case 486:

/* Line 1806 of yacc.c  */
#line 2024 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 487:

/* Line 1806 of yacc.c  */
#line 2025 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 488:

/* Line 1806 of yacc.c  */
#line 2026 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 489:

/* Line 1806 of yacc.c  */
#line 2027 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 490:

/* Line 1806 of yacc.c  */
#line 2028 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 491:

/* Line 1806 of yacc.c  */
#line 2029 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 492:

/* Line 1806 of yacc.c  */
#line 2030 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 493:

/* Line 1806 of yacc.c  */
#line 2031 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 494:

/* Line 1806 of yacc.c  */
#line 2032 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 495:

/* Line 1806 of yacc.c  */
#line 2033 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 496:

/* Line 1806 of yacc.c  */
#line 2034 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 497:

/* Line 1806 of yacc.c  */
#line 2035 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 498:

/* Line 1806 of yacc.c  */
#line 2036 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 499:

/* Line 1806 of yacc.c  */
#line 2037 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 500:

/* Line 1806 of yacc.c  */
#line 2038 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 501:

/* Line 1806 of yacc.c  */
#line 2039 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 502:

/* Line 1806 of yacc.c  */
#line 2040 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 503:

/* Line 1806 of yacc.c  */
#line 2041 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 504:

/* Line 1806 of yacc.c  */
#line 2042 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 505:

/* Line 1806 of yacc.c  */
#line 2043 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 506:

/* Line 1806 of yacc.c  */
#line 2044 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 507:

/* Line 1806 of yacc.c  */
#line 2045 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 508:

/* Line 1806 of yacc.c  */
#line 2046 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 509:

/* Line 1806 of yacc.c  */
#line 2047 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 510:

/* Line 1806 of yacc.c  */
#line 2048 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 511:

/* Line 1806 of yacc.c  */
#line 2049 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 512:

/* Line 1806 of yacc.c  */
#line 2050 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 513:

/* Line 1806 of yacc.c  */
#line 2051 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 514:

/* Line 1806 of yacc.c  */
#line 2052 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 515:

/* Line 1806 of yacc.c  */
#line 2053 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 516:

/* Line 1806 of yacc.c  */
#line 2054 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 517:

/* Line 1806 of yacc.c  */
#line 2055 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 518:

/* Line 1806 of yacc.c  */
#line 2056 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 519:

/* Line 1806 of yacc.c  */
#line 2057 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 520:

/* Line 1806 of yacc.c  */
#line 2058 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 521:

/* Line 1806 of yacc.c  */
#line 2059 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 522:

/* Line 1806 of yacc.c  */
#line 2060 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 523:

/* Line 1806 of yacc.c  */
#line 2061 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 524:

/* Line 1806 of yacc.c  */
#line 2062 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 525:

/* Line 1806 of yacc.c  */
#line 2063 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 526:

/* Line 1806 of yacc.c  */
#line 2064 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 527:

/* Line 1806 of yacc.c  */
#line 2065 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 528:

/* Line 1806 of yacc.c  */
#line 2066 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 529:

/* Line 1806 of yacc.c  */
#line 2067 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 530:

/* Line 1806 of yacc.c  */
#line 2068 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 531:

/* Line 1806 of yacc.c  */
#line 2069 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 532:

/* Line 1806 of yacc.c  */
#line 2070 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 533:

/* Line 1806 of yacc.c  */
#line 2071 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 534:

/* Line 1806 of yacc.c  */
#line 2072 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 535:

/* Line 1806 of yacc.c  */
#line 2073 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 536:

/* Line 1806 of yacc.c  */
#line 2074 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 537:

/* Line 1806 of yacc.c  */
#line 2075 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 538:

/* Line 1806 of yacc.c  */
#line 2076 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 539:

/* Line 1806 of yacc.c  */
#line 2077 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 540:

/* Line 1806 of yacc.c  */
#line 2078 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 541:

/* Line 1806 of yacc.c  */
#line 2079 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 542:

/* Line 1806 of yacc.c  */
#line 2080 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 543:

/* Line 1806 of yacc.c  */
#line 2081 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 544:

/* Line 1806 of yacc.c  */
#line 2082 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 545:

/* Line 1806 of yacc.c  */
#line 2083 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 546:

/* Line 1806 of yacc.c  */
#line 2084 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 547:

/* Line 1806 of yacc.c  */
#line 2085 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 548:

/* Line 1806 of yacc.c  */
#line 2086 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 549:

/* Line 1806 of yacc.c  */
#line 2087 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 550:

/* Line 1806 of yacc.c  */
#line 2088 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 551:

/* Line 1806 of yacc.c  */
#line 2089 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 552:

/* Line 1806 of yacc.c  */
#line 2090 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 553:

/* Line 1806 of yacc.c  */
#line 2091 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 554:

/* Line 1806 of yacc.c  */
#line 2092 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 555:

/* Line 1806 of yacc.c  */
#line 2093 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 556:

/* Line 1806 of yacc.c  */
#line 2094 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 557:

/* Line 1806 of yacc.c  */
#line 2095 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 558:

/* Line 1806 of yacc.c  */
#line 2096 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 559:

/* Line 1806 of yacc.c  */
#line 2097 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 560:

/* Line 1806 of yacc.c  */
#line 2098 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 561:

/* Line 1806 of yacc.c  */
#line 2099 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 562:

/* Line 1806 of yacc.c  */
#line 2100 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 563:

/* Line 1806 of yacc.c  */
#line 2101 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 564:

/* Line 1806 of yacc.c  */
#line 2102 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 565:

/* Line 1806 of yacc.c  */
#line 2103 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 566:

/* Line 1806 of yacc.c  */
#line 2108 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 567:

/* Line 1806 of yacc.c  */
#line 2112 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 568:

/* Line 1806 of yacc.c  */
#line 2113 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 569:

/* Line 1806 of yacc.c  */
#line 2116 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);}
    break;

  case 570:

/* Line 1806 of yacc.c  */
#line 2117 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);}
    break;

  case 571:

/* Line 1806 of yacc.c  */
#line 2118 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);}
    break;

  case 572:

/* Line 1806 of yacc.c  */
#line 2122 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);}
    break;

  case 573:

/* Line 1806 of yacc.c  */
#line 2123 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);}
    break;

  case 574:

/* Line 1806 of yacc.c  */
#line 2124 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);}
    break;

  case 575:

/* Line 1806 of yacc.c  */
#line 2128 "hphp.y"
    { (yyval).reset();}
    break;

  case 576:

/* Line 1806 of yacc.c  */
#line 2129 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 577:

/* Line 1806 of yacc.c  */
#line 2130 "hphp.y"
    { (yyval).reset();}
    break;

  case 578:

/* Line 1806 of yacc.c  */
#line 2134 "hphp.y"
    { (yyval).reset();}
    break;

  case 579:

/* Line 1806 of yacc.c  */
#line 2135 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);}
    break;

  case 580:

/* Line 1806 of yacc.c  */
#line 2136 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 581:

/* Line 1806 of yacc.c  */
#line 2140 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 582:

/* Line 1806 of yacc.c  */
#line 2141 "hphp.y"
    { (yyval).reset();}
    break;

  case 583:

/* Line 1806 of yacc.c  */
#line 2145 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 584:

/* Line 1806 of yacc.c  */
#line 2146 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 585:

/* Line 1806 of yacc.c  */
#line 2147 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 586:

/* Line 1806 of yacc.c  */
#line 2148 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));}
    break;

  case 587:

/* Line 1806 of yacc.c  */
#line 2150 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));}
    break;

  case 588:

/* Line 1806 of yacc.c  */
#line 2151 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));}
    break;

  case 589:

/* Line 1806 of yacc.c  */
#line 2152 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));}
    break;

  case 590:

/* Line 1806 of yacc.c  */
#line 2153 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));}
    break;

  case 591:

/* Line 1806 of yacc.c  */
#line 2154 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));}
    break;

  case 592:

/* Line 1806 of yacc.c  */
#line 2155 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));}
    break;

  case 593:

/* Line 1806 of yacc.c  */
#line 2156 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));}
    break;

  case 594:

/* Line 1806 of yacc.c  */
#line 2157 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));}
    break;

  case 595:

/* Line 1806 of yacc.c  */
#line 2158 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));}
    break;

  case 596:

/* Line 1806 of yacc.c  */
#line 2161 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));}
    break;

  case 597:

/* Line 1806 of yacc.c  */
#line 2163 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
    break;

  case 598:

/* Line 1806 of yacc.c  */
#line 2167 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 599:

/* Line 1806 of yacc.c  */
#line 2168 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 600:

/* Line 1806 of yacc.c  */
#line 2169 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 601:

/* Line 1806 of yacc.c  */
#line 2170 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 602:

/* Line 1806 of yacc.c  */
#line 2172 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 603:

/* Line 1806 of yacc.c  */
#line 2174 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 604:

/* Line 1806 of yacc.c  */
#line 2175 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); }
    break;

  case 605:

/* Line 1806 of yacc.c  */
#line 2177 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 606:

/* Line 1806 of yacc.c  */
#line 2178 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 607:

/* Line 1806 of yacc.c  */
#line 2179 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 608:

/* Line 1806 of yacc.c  */
#line 2185 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 609:

/* Line 1806 of yacc.c  */
#line 2187 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 610:

/* Line 1806 of yacc.c  */
#line 2191 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 611:

/* Line 1806 of yacc.c  */
#line 2195 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 612:

/* Line 1806 of yacc.c  */
#line 2196 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 613:

/* Line 1806 of yacc.c  */
#line 2197 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 614:

/* Line 1806 of yacc.c  */
#line 2198 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 615:

/* Line 1806 of yacc.c  */
#line 2199 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));}
    break;

  case 616:

/* Line 1806 of yacc.c  */
#line 2200 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));}
    break;

  case 617:

/* Line 1806 of yacc.c  */
#line 2202 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));}
    break;

  case 618:

/* Line 1806 of yacc.c  */
#line 2207 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 619:

/* Line 1806 of yacc.c  */
#line 2208 "hphp.y"
    { (yyval).reset();}
    break;

  case 620:

/* Line 1806 of yacc.c  */
#line 2212 "hphp.y"
    { (yyval).reset();}
    break;

  case 621:

/* Line 1806 of yacc.c  */
#line 2213 "hphp.y"
    { (yyval).reset();}
    break;

  case 622:

/* Line 1806 of yacc.c  */
#line 2216 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();}
    break;

  case 623:

/* Line 1806 of yacc.c  */
#line 2217 "hphp.y"
    { (yyval).reset();}
    break;

  case 624:

/* Line 1806 of yacc.c  */
#line 2223 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 625:

/* Line 1806 of yacc.c  */
#line 2225 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 626:

/* Line 1806 of yacc.c  */
#line 2227 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 627:

/* Line 1806 of yacc.c  */
#line 2228 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 628:

/* Line 1806 of yacc.c  */
#line 2232 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 629:

/* Line 1806 of yacc.c  */
#line 2233 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 630:

/* Line 1806 of yacc.c  */
#line 2234 "hphp.y"
    { _p->onScalar((yyval), T_ONUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 631:

/* Line 1806 of yacc.c  */
#line 2235 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));}
    break;

  case 632:

/* Line 1806 of yacc.c  */
#line 2239 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));}
    break;

  case 633:

/* Line 1806 of yacc.c  */
#line 2241 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
    break;

  case 634:

/* Line 1806 of yacc.c  */
#line 2244 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));}
    break;

  case 635:

/* Line 1806 of yacc.c  */
#line 2245 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));}
    break;

  case 636:

/* Line 1806 of yacc.c  */
#line 2246 "hphp.y"
    { _p->onScalar((yyval),T_ONUMBER,(yyvsp[(1) - (1)]));}
    break;

  case 637:

/* Line 1806 of yacc.c  */
#line 2247 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));}
    break;

  case 638:

/* Line 1806 of yacc.c  */
#line 2250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 639:

/* Line 1806 of yacc.c  */
#line 2251 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));}
    break;

  case 640:

/* Line 1806 of yacc.c  */
#line 2252 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 641:

/* Line 1806 of yacc.c  */
#line 2253 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 642:

/* Line 1806 of yacc.c  */
#line 2255 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 643:

/* Line 1806 of yacc.c  */
#line 2257 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 644:

/* Line 1806 of yacc.c  */
#line 2258 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 645:

/* Line 1806 of yacc.c  */
#line 2260 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 646:

/* Line 1806 of yacc.c  */
#line 2265 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 647:

/* Line 1806 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval).reset();}
    break;

  case 648:

/* Line 1806 of yacc.c  */
#line 2271 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 649:

/* Line 1806 of yacc.c  */
#line 2273 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 650:

/* Line 1806 of yacc.c  */
#line 2275 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 651:

/* Line 1806 of yacc.c  */
#line 2276 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 652:

/* Line 1806 of yacc.c  */
#line 2280 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 653:

/* Line 1806 of yacc.c  */
#line 2281 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 654:

/* Line 1806 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 655:

/* Line 1806 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval).reset(); }
    break;

  case 656:

/* Line 1806 of yacc.c  */
#line 2292 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 657:

/* Line 1806 of yacc.c  */
#line 2295 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 658:

/* Line 1806 of yacc.c  */
#line 2300 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 659:

/* Line 1806 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval).reset();}
    break;

  case 660:

/* Line 1806 of yacc.c  */
#line 2304 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 661:

/* Line 1806 of yacc.c  */
#line 2305 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
    break;

  case 662:

/* Line 1806 of yacc.c  */
#line 2312 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 663:

/* Line 1806 of yacc.c  */
#line 2314 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 664:

/* Line 1806 of yacc.c  */
#line 2317 "hphp.y"
    { only_in_hh_syntax(_p);}
    break;

  case 665:

/* Line 1806 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 666:

/* Line 1806 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 667:

/* Line 1806 of yacc.c  */
#line 2325 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 668:

/* Line 1806 of yacc.c  */
#line 2326 "hphp.y"
    { (yyval).reset();}
    break;

  case 669:

/* Line 1806 of yacc.c  */
#line 2330 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 670:

/* Line 1806 of yacc.c  */
#line 2332 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 671:

/* Line 1806 of yacc.c  */
#line 2336 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 672:

/* Line 1806 of yacc.c  */
#line 2337 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 673:

/* Line 1806 of yacc.c  */
#line 2341 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 674:

/* Line 1806 of yacc.c  */
#line 2342 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 675:

/* Line 1806 of yacc.c  */
#line 2346 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 676:

/* Line 1806 of yacc.c  */
#line 2348 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 677:

/* Line 1806 of yacc.c  */
#line 2353 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 678:

/* Line 1806 of yacc.c  */
#line 2355 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 679:

/* Line 1806 of yacc.c  */
#line 2359 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 680:

/* Line 1806 of yacc.c  */
#line 2360 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 681:

/* Line 1806 of yacc.c  */
#line 2361 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 682:

/* Line 1806 of yacc.c  */
#line 2362 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 683:

/* Line 1806 of yacc.c  */
#line 2363 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 684:

/* Line 1806 of yacc.c  */
#line 2364 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 685:

/* Line 1806 of yacc.c  */
#line 2366 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 686:

/* Line 1806 of yacc.c  */
#line 2369 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 687:

/* Line 1806 of yacc.c  */
#line 2371 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 688:

/* Line 1806 of yacc.c  */
#line 2372 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 689:

/* Line 1806 of yacc.c  */
#line 2376 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 690:

/* Line 1806 of yacc.c  */
#line 2377 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 691:

/* Line 1806 of yacc.c  */
#line 2378 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 692:

/* Line 1806 of yacc.c  */
#line 2379 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 693:

/* Line 1806 of yacc.c  */
#line 2381 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 694:

/* Line 1806 of yacc.c  */
#line 2383 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 695:

/* Line 1806 of yacc.c  */
#line 2385 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 696:

/* Line 1806 of yacc.c  */
#line 2386 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 697:

/* Line 1806 of yacc.c  */
#line 2390 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 698:

/* Line 1806 of yacc.c  */
#line 2391 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 699:

/* Line 1806 of yacc.c  */
#line 2392 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 700:

/* Line 1806 of yacc.c  */
#line 2398 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));}
    break;

  case 701:

/* Line 1806 of yacc.c  */
#line 2401 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 702:

/* Line 1806 of yacc.c  */
#line 2404 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));}
    break;

  case 703:

/* Line 1806 of yacc.c  */
#line 2408 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));}
    break;

  case 704:

/* Line 1806 of yacc.c  */
#line 2412 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));}
    break;

  case 705:

/* Line 1806 of yacc.c  */
#line 2416 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));}
    break;

  case 706:

/* Line 1806 of yacc.c  */
#line 2423 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));}
    break;

  case 707:

/* Line 1806 of yacc.c  */
#line 2427 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));}
    break;

  case 708:

/* Line 1806 of yacc.c  */
#line 2431 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));}
    break;

  case 709:

/* Line 1806 of yacc.c  */
#line 2435 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 710:

/* Line 1806 of yacc.c  */
#line 2437 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 711:

/* Line 1806 of yacc.c  */
#line 2442 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 712:

/* Line 1806 of yacc.c  */
#line 2443 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 713:

/* Line 1806 of yacc.c  */
#line 2444 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 714:

/* Line 1806 of yacc.c  */
#line 2447 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 715:

/* Line 1806 of yacc.c  */
#line 2448 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);}
    break;

  case 716:

/* Line 1806 of yacc.c  */
#line 2451 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 717:

/* Line 1806 of yacc.c  */
#line 2452 "hphp.y"
    { (yyval).reset();}
    break;

  case 718:

/* Line 1806 of yacc.c  */
#line 2456 "hphp.y"
    { (yyval) = 1;}
    break;

  case 719:

/* Line 1806 of yacc.c  */
#line 2457 "hphp.y"
    { (yyval)++;}
    break;

  case 720:

/* Line 1806 of yacc.c  */
#line 2461 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 721:

/* Line 1806 of yacc.c  */
#line 2462 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 722:

/* Line 1806 of yacc.c  */
#line 2463 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 723:

/* Line 1806 of yacc.c  */
#line 2465 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 724:

/* Line 1806 of yacc.c  */
#line 2468 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 725:

/* Line 1806 of yacc.c  */
#line 2469 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 727:

/* Line 1806 of yacc.c  */
#line 2473 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 728:

/* Line 1806 of yacc.c  */
#line 2475 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 729:

/* Line 1806 of yacc.c  */
#line 2477 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 730:

/* Line 1806 of yacc.c  */
#line 2478 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 731:

/* Line 1806 of yacc.c  */
#line 2482 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);}
    break;

  case 732:

/* Line 1806 of yacc.c  */
#line 2483 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 733:

/* Line 1806 of yacc.c  */
#line 2485 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 734:

/* Line 1806 of yacc.c  */
#line 2486 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);}
    break;

  case 735:

/* Line 1806 of yacc.c  */
#line 2487 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));}
    break;

  case 736:

/* Line 1806 of yacc.c  */
#line 2488 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));}
    break;

  case 737:

/* Line 1806 of yacc.c  */
#line 2493 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 738:

/* Line 1806 of yacc.c  */
#line 2494 "hphp.y"
    { (yyval).reset();}
    break;

  case 739:

/* Line 1806 of yacc.c  */
#line 2498 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 740:

/* Line 1806 of yacc.c  */
#line 2499 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 741:

/* Line 1806 of yacc.c  */
#line 2500 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 742:

/* Line 1806 of yacc.c  */
#line 2501 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 743:

/* Line 1806 of yacc.c  */
#line 2504 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);}
    break;

  case 744:

/* Line 1806 of yacc.c  */
#line 2506 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);}
    break;

  case 745:

/* Line 1806 of yacc.c  */
#line 2507 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 746:

/* Line 1806 of yacc.c  */
#line 2508 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);}
    break;

  case 747:

/* Line 1806 of yacc.c  */
#line 2513 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 748:

/* Line 1806 of yacc.c  */
#line 2514 "hphp.y"
    { _p->onEmptyCollection((yyval));}
    break;

  case 749:

/* Line 1806 of yacc.c  */
#line 2518 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 750:

/* Line 1806 of yacc.c  */
#line 2519 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));}
    break;

  case 751:

/* Line 1806 of yacc.c  */
#line 2520 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 752:

/* Line 1806 of yacc.c  */
#line 2521 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));}
    break;

  case 753:

/* Line 1806 of yacc.c  */
#line 2526 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 754:

/* Line 1806 of yacc.c  */
#line 2527 "hphp.y"
    { _p->onEmptyCollection((yyval));}
    break;

  case 755:

/* Line 1806 of yacc.c  */
#line 2532 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 756:

/* Line 1806 of yacc.c  */
#line 2534 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));}
    break;

  case 757:

/* Line 1806 of yacc.c  */
#line 2536 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 758:

/* Line 1806 of yacc.c  */
#line 2537 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));}
    break;

  case 759:

/* Line 1806 of yacc.c  */
#line 2541 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);}
    break;

  case 760:

/* Line 1806 of yacc.c  */
#line 2543 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);}
    break;

  case 761:

/* Line 1806 of yacc.c  */
#line 2544 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);}
    break;

  case 762:

/* Line 1806 of yacc.c  */
#line 2546 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); }
    break;

  case 763:

/* Line 1806 of yacc.c  */
#line 2551 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 764:

/* Line 1806 of yacc.c  */
#line 2553 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 765:

/* Line 1806 of yacc.c  */
#line 2555 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 766:

/* Line 1806 of yacc.c  */
#line 2557 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);}
    break;

  case 767:

/* Line 1806 of yacc.c  */
#line 2559 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));}
    break;

  case 768:

/* Line 1806 of yacc.c  */
#line 2560 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 769:

/* Line 1806 of yacc.c  */
#line 2563 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;}
    break;

  case 770:

/* Line 1806 of yacc.c  */
#line 2564 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;}
    break;

  case 771:

/* Line 1806 of yacc.c  */
#line 2565 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;}
    break;

  case 772:

/* Line 1806 of yacc.c  */
#line 2569 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);}
    break;

  case 773:

/* Line 1806 of yacc.c  */
#line 2570 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);}
    break;

  case 774:

/* Line 1806 of yacc.c  */
#line 2571 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);}
    break;

  case 775:

/* Line 1806 of yacc.c  */
#line 2572 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);}
    break;

  case 776:

/* Line 1806 of yacc.c  */
#line 2573 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);}
    break;

  case 777:

/* Line 1806 of yacc.c  */
#line 2574 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);}
    break;

  case 778:

/* Line 1806 of yacc.c  */
#line 2575 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);}
    break;

  case 779:

/* Line 1806 of yacc.c  */
#line 2576 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);}
    break;

  case 780:

/* Line 1806 of yacc.c  */
#line 2577 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);}
    break;

  case 781:

/* Line 1806 of yacc.c  */
#line 2581 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 782:

/* Line 1806 of yacc.c  */
#line 2582 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 783:

/* Line 1806 of yacc.c  */
#line 2587 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 784:

/* Line 1806 of yacc.c  */
#line 2589 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 787:

/* Line 1806 of yacc.c  */
#line 2603 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); }
    break;

  case 788:

/* Line 1806 of yacc.c  */
#line 2607 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); }
    break;

  case 789:

/* Line 1806 of yacc.c  */
#line 2613 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 790:

/* Line 1806 of yacc.c  */
#line 2614 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 791:

/* Line 1806 of yacc.c  */
#line 2620 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 792:

/* Line 1806 of yacc.c  */
#line 2624 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); }
    break;

  case 793:

/* Line 1806 of yacc.c  */
#line 2630 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 794:

/* Line 1806 of yacc.c  */
#line 2631 "hphp.y"
    { (yyval).reset(); }
    break;

  case 795:

/* Line 1806 of yacc.c  */
#line 2635 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 796:

/* Line 1806 of yacc.c  */
#line 2638 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); }
    break;

  case 797:

/* Line 1806 of yacc.c  */
#line 2643 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); }
    break;

  case 798:

/* Line 1806 of yacc.c  */
#line 2644 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 799:

/* Line 1806 of yacc.c  */
#line 2645 "hphp.y"
    { (yyval).reset(); }
    break;

  case 800:

/* Line 1806 of yacc.c  */
#line 2646 "hphp.y"
    { (yyval).reset(); }
    break;

  case 801:

/* Line 1806 of yacc.c  */
#line 2650 "hphp.y"
    { (yyval).reset(); }
    break;

  case 802:

/* Line 1806 of yacc.c  */
#line 2651 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 803:

/* Line 1806 of yacc.c  */
#line 2656 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); }
    break;

  case 804:

/* Line 1806 of yacc.c  */
#line 2657 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); }
    break;

  case 805:

/* Line 1806 of yacc.c  */
#line 2659 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); }
    break;

  case 806:

/* Line 1806 of yacc.c  */
#line 2660 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); }
    break;

  case 807:

/* Line 1806 of yacc.c  */
#line 2666 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); }
    break;

  case 810:

/* Line 1806 of yacc.c  */
#line 2677 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 811:

/* Line 1806 of yacc.c  */
#line 2679 "hphp.y"
    {}
    break;

  case 812:

/* Line 1806 of yacc.c  */
#line 2683 "hphp.y"
    { (yyval).setText("array"); }
    break;

  case 813:

/* Line 1806 of yacc.c  */
#line 2690 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 814:

/* Line 1806 of yacc.c  */
#line 2693 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 815:

/* Line 1806 of yacc.c  */
#line 2696 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 816:

/* Line 1806 of yacc.c  */
#line 2697 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); }
    break;

  case 817:

/* Line 1806 of yacc.c  */
#line 2700 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); }
    break;

  case 818:

/* Line 1806 of yacc.c  */
#line 2703 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 819:

/* Line 1806 of yacc.c  */
#line 2705 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); }
    break;

  case 820:

/* Line 1806 of yacc.c  */
#line 2708 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); }
    break;

  case 821:

/* Line 1806 of yacc.c  */
#line 2711 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
    break;

  case 822:

/* Line 1806 of yacc.c  */
#line 2717 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
    break;

  case 823:

/* Line 1806 of yacc.c  */
#line 2721 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (5)]));
                                        _p->onTypeSpecialization((yyval), 't'); }
    break;

  case 824:

/* Line 1806 of yacc.c  */
#line 2729 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 825:

/* Line 1806 of yacc.c  */
#line 2730 "hphp.y"
    { (yyval).reset(); }
    break;



/* Line 1806 of yacc.c  */
#line 12323 "hphp.tab.cpp"
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
#line 2733 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

