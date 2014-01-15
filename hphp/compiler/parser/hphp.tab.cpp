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
#include "hphp/util/util.h"
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
// converting constant declartion to "define(name, value);"

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
  HPHP::Util::split(':', attributes.text().c_str(), classes, true);
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
#line 638 "hphp.tab.cpp"

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
     T_STRING = 308,
     T_STRING_VARNAME = 309,
     T_VARIABLE = 310,
     T_NUM_STRING = 311,
     T_INLINE_HTML = 312,
     T_CHARACTER = 313,
     T_BAD_CHARACTER = 314,
     T_ENCAPSED_AND_WHITESPACE = 315,
     T_CONSTANT_ENCAPSED_STRING = 316,
     T_ECHO = 317,
     T_DO = 318,
     T_WHILE = 319,
     T_ENDWHILE = 320,
     T_FOR = 321,
     T_ENDFOR = 322,
     T_FOREACH = 323,
     T_ENDFOREACH = 324,
     T_DECLARE = 325,
     T_ENDDECLARE = 326,
     T_AS = 327,
     T_SWITCH = 328,
     T_ENDSWITCH = 329,
     T_CASE = 330,
     T_DEFAULT = 331,
     T_BREAK = 332,
     T_GOTO = 333,
     T_CONTINUE = 334,
     T_FUNCTION = 335,
     T_CONST = 336,
     T_RETURN = 337,
     T_TRY = 338,
     T_CATCH = 339,
     T_THROW = 340,
     T_USE = 341,
     T_GLOBAL = 342,
     T_PUBLIC = 343,
     T_PROTECTED = 344,
     T_PRIVATE = 345,
     T_FINAL = 346,
     T_ABSTRACT = 347,
     T_STATIC = 348,
     T_VAR = 349,
     T_UNSET = 350,
     T_ISSET = 351,
     T_EMPTY = 352,
     T_HALT_COMPILER = 353,
     T_CLASS = 354,
     T_INTERFACE = 355,
     T_EXTENDS = 356,
     T_IMPLEMENTS = 357,
     T_OBJECT_OPERATOR = 358,
     T_DOUBLE_ARROW = 359,
     T_LIST = 360,
     T_ARRAY = 361,
     T_CALLABLE = 362,
     T_CLASS_C = 363,
     T_METHOD_C = 364,
     T_FUNC_C = 365,
     T_LINE = 366,
     T_FILE = 367,
     T_COMMENT = 368,
     T_DOC_COMMENT = 369,
     T_OPEN_TAG = 370,
     T_OPEN_TAG_WITH_ECHO = 371,
     T_CLOSE_TAG = 372,
     T_WHITESPACE = 373,
     T_START_HEREDOC = 374,
     T_END_HEREDOC = 375,
     T_DOLLAR_OPEN_CURLY_BRACES = 376,
     T_CURLY_OPEN = 377,
     T_DOUBLE_COLON = 378,
     T_NAMESPACE = 379,
     T_NS_C = 380,
     T_DIR = 381,
     T_NS_SEPARATOR = 382,
     T_YIELD = 383,
     T_XHP_LABEL = 384,
     T_XHP_TEXT = 385,
     T_XHP_ATTRIBUTE = 386,
     T_XHP_CATEGORY = 387,
     T_XHP_CATEGORY_LABEL = 388,
     T_XHP_CHILDREN = 389,
     T_XHP_ENUM = 390,
     T_XHP_REQUIRED = 391,
     T_TRAIT = 392,
     T_INSTEADOF = 393,
     T_TRAIT_C = 394,
     T_VARARG = 395,
     T_HH_ERROR = 396,
     T_FINALLY = 397,
     T_XHP_TAG_LT = 398,
     T_XHP_TAG_GT = 399,
     T_TYPELIST_LT = 400,
     T_TYPELIST_GT = 401,
     T_UNRESOLVED_LT = 402,
     T_COLLECTION = 403,
     T_SHAPE = 404,
     T_TYPE = 405,
     T_UNRESOLVED_TYPE = 406,
     T_NEWTYPE = 407,
     T_UNRESOLVED_NEWTYPE = 408,
     T_COMPILER_HALT_OFFSET = 409,
     T_AWAIT = 410,
     T_ASYNC = 411,
     T_TUPLE = 412,
     T_FROM = 413,
     T_WHERE = 414,
     T_JOIN = 415,
     T_IN = 416,
     T_ON = 417,
     T_EQUALS = 418,
     T_INTO = 419,
     T_LET = 420,
     T_ORDERBY = 421,
     T_ASCENDING = 422,
     T_DESCENDING = 423,
     T_SELECT = 424,
     T_GROUP = 425,
     T_BY = 426,
     T_LAMBDA_OP = 427,
     T_LAMBDA_CP = 428,
     T_UNRESOLVED_OP = 429
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
#line 867 "hphp.tab.cpp"

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
#define YYLAST   14589

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  204
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  246
/* YYNRULES -- Number of rules.  */
#define YYNRULES  820
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1530

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   429

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    49,   202,     2,   199,    48,    32,   203,
     194,   195,    46,    43,     9,    44,    45,    47,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    27,   196,
      37,    14,    38,    26,    52,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    62,     2,   201,    31,     2,   200,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   197,    30,   198,    51,     2,     2,     2,
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
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193
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
    1916,  1918,  1920,  1922,  1926,  1929,  1931,  1933,  1936,  1939,
    1944,  1949,  1953,  1958,  1960,  1962,  1966,  1970,  1974,  1976,
    1978,  1980,  1982,  1986,  1990,  1994,  1997,  1998,  2000,  2001,
    2003,  2004,  2010,  2014,  2018,  2020,  2022,  2024,  2026,  2030,
    2033,  2035,  2037,  2039,  2041,  2043,  2046,  2049,  2054,  2059,
    2063,  2068,  2071,  2072,  2078,  2082,  2086,  2088,  2092,  2094,
    2097,  2098,  2104,  2108,  2111,  2112,  2116,  2117,  2122,  2125,
    2126,  2130,  2134,  2136,  2137,  2139,  2142,  2145,  2150,  2154,
    2158,  2161,  2166,  2169,  2174,  2176,  2178,  2180,  2182,  2184,
    2187,  2192,  2196,  2201,  2205,  2207,  2209,  2211,  2213,  2216,
    2221,  2226,  2230,  2232,  2234,  2238,  2246,  2253,  2262,  2272,
    2281,  2292,  2300,  2307,  2316,  2318,  2321,  2326,  2331,  2333,
    2335,  2340,  2342,  2343,  2345,  2348,  2350,  2352,  2355,  2360,
    2364,  2368,  2369,  2371,  2374,  2379,  2383,  2386,  2390,  2397,
    2398,  2400,  2405,  2408,  2409,  2415,  2419,  2423,  2425,  2432,
    2437,  2442,  2445,  2448,  2449,  2455,  2459,  2463,  2465,  2468,
    2469,  2475,  2479,  2483,  2485,  2488,  2491,  2493,  2496,  2498,
    2503,  2507,  2511,  2518,  2522,  2524,  2526,  2528,  2533,  2538,
    2543,  2548,  2551,  2554,  2559,  2562,  2565,  2567,  2571,  2575,
    2579,  2580,  2583,  2589,  2596,  2598,  2601,  2603,  2608,  2612,
    2613,  2615,  2619,  2623,  2625,  2627,  2628,  2629,  2632,  2636,
    2638,  2644,  2648,  2652,  2656,  2658,  2661,  2662,  2667,  2670,
    2673,  2675,  2677,  2679,  2681,  2686,  2693,  2695,  2704,  2710,
    2712
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     205,     0,    -1,    -1,   206,   207,    -1,   207,   208,    -1,
      -1,   222,    -1,   238,    -1,   242,    -1,   247,    -1,   436,
      -1,   117,   194,   195,   196,    -1,   143,   214,   196,    -1,
      -1,   143,   214,   197,   209,   207,   198,    -1,    -1,   143,
     197,   210,   207,   198,    -1,   105,   212,   196,    -1,   219,
     196,    -1,    72,    -1,   150,    -1,   151,    -1,   153,    -1,
     155,    -1,   154,    -1,   176,    -1,   178,    -1,   179,    -1,
     181,    -1,   180,    -1,   182,    -1,   183,    -1,   184,    -1,
     185,    -1,   186,    -1,   187,    -1,   188,    -1,   189,    -1,
     190,    -1,   212,     9,   213,    -1,   213,    -1,   214,    -1,
     146,   214,    -1,   214,    91,   211,    -1,   146,   214,    91,
     211,    -1,   211,    -1,   214,   146,   211,    -1,   214,    -1,
     143,   146,   214,    -1,   146,   214,    -1,   215,    -1,   215,
     439,    -1,   215,   439,    -1,   219,     9,   437,    14,   383,
      -1,   100,   437,    14,   383,    -1,   220,   221,    -1,    -1,
     222,    -1,   238,    -1,   242,    -1,   247,    -1,   197,   220,
     198,    -1,    66,   315,   222,   269,   271,    -1,    66,   315,
      27,   220,   270,   272,    69,   196,    -1,    -1,    83,   315,
     223,   263,    -1,    -1,    82,   224,   222,    83,   315,   196,
      -1,    -1,    85,   194,   317,   196,   317,   196,   317,   195,
     225,   261,    -1,    -1,    92,   315,   226,   266,    -1,    96,
     196,    -1,    96,   324,   196,    -1,    98,   196,    -1,    98,
     324,   196,    -1,   101,   196,    -1,   101,   324,   196,    -1,
     147,    96,   196,    -1,   106,   279,   196,    -1,   112,   281,
     196,    -1,    81,   316,   196,    -1,   114,   194,   433,   195,
     196,    -1,   196,    -1,    76,    -1,    -1,    87,   194,   324,
      91,   260,   259,   195,   227,   262,    -1,    89,   194,   265,
     195,   264,    -1,    -1,   102,   230,   103,   194,   376,    74,
     195,   197,   220,   198,   232,   228,   235,    -1,    -1,   102,
     230,   161,   229,   233,    -1,   104,   324,   196,    -1,    97,
     211,   196,    -1,   324,   196,    -1,   318,   196,    -1,   319,
     196,    -1,   320,   196,    -1,   321,   196,    -1,   322,   196,
      -1,   101,   321,   196,    -1,   323,   196,    -1,   346,   196,
      -1,   101,   345,   196,    -1,   211,    27,    -1,    -1,   197,
     231,   220,   198,    -1,   232,   103,   194,   376,    74,   195,
     197,   220,   198,    -1,    -1,    -1,   197,   234,   220,   198,
      -1,   161,   233,    -1,    -1,    32,    -1,    -1,    99,    -1,
      -1,   237,   236,   438,   239,   194,   275,   195,   442,   304,
      -1,    -1,   308,   237,   236,   438,   240,   194,   275,   195,
     442,   304,    -1,    -1,   403,   307,   237,   236,   438,   241,
     194,   275,   195,   442,   304,    -1,    -1,   253,   250,   243,
     254,   255,   197,   282,   198,    -1,    -1,   403,   253,   250,
     244,   254,   255,   197,   282,   198,    -1,    -1,   119,   251,
     245,   256,   197,   282,   198,    -1,    -1,   403,   119,   251,
     246,   256,   197,   282,   198,    -1,    -1,   156,   252,   248,
     255,   197,   282,   198,    -1,    -1,   403,   156,   252,   249,
     255,   197,   282,   198,    -1,   438,    -1,   148,    -1,   438,
      -1,   438,    -1,   118,    -1,   111,   118,    -1,   110,   118,
      -1,   120,   376,    -1,    -1,   121,   257,    -1,    -1,   120,
     257,    -1,    -1,   376,    -1,   257,     9,   376,    -1,   376,
      -1,   258,     9,   376,    -1,   123,   260,    -1,    -1,   410,
      -1,    32,   410,    -1,   124,   194,   422,   195,    -1,   222,
      -1,    27,   220,    86,   196,    -1,   222,    -1,    27,   220,
      88,   196,    -1,   222,    -1,    27,   220,    84,   196,    -1,
     222,    -1,    27,   220,    90,   196,    -1,   211,    14,   383,
      -1,   265,     9,   211,    14,   383,    -1,   197,   267,   198,
      -1,   197,   196,   267,   198,    -1,    27,   267,    93,   196,
      -1,    27,   196,   267,    93,   196,    -1,   267,    94,   324,
     268,   220,    -1,   267,    95,   268,   220,    -1,    -1,    27,
      -1,   196,    -1,   269,    67,   315,   222,    -1,    -1,   270,
      67,   315,    27,   220,    -1,    -1,    68,   222,    -1,    -1,
      68,    27,   220,    -1,    -1,   274,     9,   159,    -1,   274,
     388,    -1,   159,    -1,    -1,   404,   310,   449,    74,    -1,
     404,   310,   449,    32,    74,    -1,   404,   310,   449,    32,
      74,    14,   383,    -1,   404,   310,   449,    74,    14,   383,
      -1,   274,     9,   404,   310,   449,    74,    -1,   274,     9,
     404,   310,   449,    32,    74,    -1,   274,     9,   404,   310,
     449,    32,    74,    14,   383,    -1,   274,     9,   404,   310,
     449,    74,    14,   383,    -1,   276,     9,   159,    -1,   276,
     388,    -1,   159,    -1,    -1,   404,   449,    74,    -1,   404,
     449,    32,    74,    -1,   404,   449,    32,    74,    14,   383,
      -1,   404,   449,    74,    14,   383,    -1,   276,     9,   404,
     449,    74,    -1,   276,     9,   404,   449,    32,    74,    -1,
     276,     9,   404,   449,    32,    74,    14,   383,    -1,   276,
       9,   404,   449,    74,    14,   383,    -1,   278,   388,    -1,
      -1,   324,    -1,    32,   410,    -1,   278,     9,   324,    -1,
     278,     9,    32,   410,    -1,   279,     9,   280,    -1,   280,
      -1,    74,    -1,   199,   410,    -1,   199,   197,   324,   198,
      -1,   281,     9,    74,    -1,   281,     9,    74,    14,   383,
      -1,    74,    -1,    74,    14,   383,    -1,   282,   283,    -1,
      -1,    -1,   306,   284,   312,   196,    -1,    -1,   308,   448,
     285,   312,   196,    -1,   313,   196,    -1,    -1,   307,   237,
     236,   438,   194,   286,   273,   195,   442,   305,    -1,    -1,
     403,   307,   237,   236,   438,   194,   287,   273,   195,   442,
     305,    -1,   150,   292,   196,    -1,   151,   298,   196,    -1,
     153,   300,   196,    -1,     4,   120,   376,   196,    -1,     4,
     121,   376,   196,    -1,   105,   258,   196,    -1,   105,   258,
     197,   288,   198,    -1,   288,   289,    -1,   288,   290,    -1,
      -1,   218,   142,   211,   157,   258,   196,    -1,   291,    91,
     307,   211,   196,    -1,   291,    91,   308,   196,    -1,   218,
     142,   211,    -1,   211,    -1,   293,    -1,   292,     9,   293,
      -1,   294,   373,   296,   297,    -1,   148,    -1,   125,    -1,
     376,    -1,   113,    -1,   154,   197,   295,   198,    -1,   382,
      -1,   295,     9,   382,    -1,    14,   383,    -1,    -1,    52,
     155,    -1,    -1,   299,    -1,   298,     9,   299,    -1,   152,
      -1,   301,    -1,   211,    -1,   116,    -1,   194,   302,   195,
      -1,   194,   302,   195,    46,    -1,   194,   302,   195,    26,
      -1,   194,   302,   195,    43,    -1,   301,    -1,   303,    -1,
     303,    46,    -1,   303,    26,    -1,   303,    43,    -1,   302,
       9,   302,    -1,   302,    30,   302,    -1,   211,    -1,   148,
      -1,   152,    -1,   196,    -1,   197,   220,   198,    -1,   196,
      -1,   197,   220,   198,    -1,   308,    -1,   113,    -1,   308,
      -1,    -1,   309,    -1,   308,   309,    -1,   107,    -1,   108,
      -1,   109,    -1,   112,    -1,   111,    -1,   110,    -1,   175,
      -1,   311,    -1,    -1,   107,    -1,   108,    -1,   109,    -1,
     312,     9,    74,    -1,   312,     9,    74,    14,   383,    -1,
      74,    -1,    74,    14,   383,    -1,   313,     9,   437,    14,
     383,    -1,   100,   437,    14,   383,    -1,   194,   314,   195,
      -1,    64,   378,   381,    -1,    63,   324,    -1,   365,    -1,
     341,    -1,   194,   324,   195,    -1,   316,     9,   324,    -1,
     324,    -1,   316,    -1,    -1,   147,   324,    -1,   147,   324,
     123,   324,    -1,   410,    14,   318,    -1,   124,   194,   422,
     195,    14,   318,    -1,   174,   324,    -1,   410,    14,   321,
      -1,   124,   194,   422,   195,    14,   321,    -1,   325,    -1,
     410,    -1,   314,    -1,   124,   194,   422,   195,    14,   324,
      -1,   410,    14,   324,    -1,   410,    14,    32,   410,    -1,
     410,    14,    32,    64,   378,   381,    -1,   410,    25,   324,
      -1,   410,    24,   324,    -1,   410,    23,   324,    -1,   410,
      22,   324,    -1,   410,    21,   324,    -1,   410,    20,   324,
      -1,   410,    19,   324,    -1,   410,    18,   324,    -1,   410,
      17,   324,    -1,   410,    16,   324,    -1,   410,    15,   324,
      -1,   410,    61,    -1,    61,   410,    -1,   410,    60,    -1,
      60,   410,    -1,   324,    28,   324,    -1,   324,    29,   324,
      -1,   324,    10,   324,    -1,   324,    12,   324,    -1,   324,
      11,   324,    -1,   324,    30,   324,    -1,   324,    32,   324,
      -1,   324,    31,   324,    -1,   324,    45,   324,    -1,   324,
      43,   324,    -1,   324,    44,   324,    -1,   324,    46,   324,
      -1,   324,    47,   324,    -1,   324,    48,   324,    -1,   324,
      42,   324,    -1,   324,    41,   324,    -1,    43,   324,    -1,
      44,   324,    -1,    49,   324,    -1,    51,   324,    -1,   324,
      34,   324,    -1,   324,    33,   324,    -1,   324,    36,   324,
      -1,   324,    35,   324,    -1,   324,    37,   324,    -1,   324,
      40,   324,    -1,   324,    38,   324,    -1,   324,    39,   324,
      -1,   324,    50,   378,    -1,   194,   325,   195,    -1,   324,
      26,   324,    27,   324,    -1,   324,    26,    27,   324,    -1,
     432,    -1,    59,   324,    -1,    58,   324,    -1,    57,   324,
      -1,    56,   324,    -1,    55,   324,    -1,    54,   324,    -1,
      53,   324,    -1,    65,   379,    -1,    52,   324,    -1,   385,
      -1,   340,    -1,   339,    -1,   200,   380,   200,    -1,    13,
     324,    -1,   327,    -1,   330,    -1,   343,    -1,   105,   194,
     364,   388,   195,    -1,    -1,    -1,   237,   236,   194,   328,
     275,   195,   442,   326,   197,   220,   198,    -1,    -1,   308,
     237,   236,   194,   329,   275,   195,   442,   326,   197,   220,
     198,    -1,    -1,    74,   331,   333,    -1,    -1,   191,   332,
     275,   192,   442,   333,    -1,     8,   324,    -1,     8,   197,
     220,   198,    -1,    80,    -1,   335,     9,   334,   123,   324,
      -1,   334,   123,   324,    -1,   336,     9,   334,   123,   383,
      -1,   334,   123,   383,    -1,   335,   387,    -1,    -1,   336,
     387,    -1,    -1,   168,   194,   337,   195,    -1,   125,   194,
     423,   195,    -1,    62,   423,   201,    -1,   376,   197,   425,
     198,    -1,   376,   197,   427,   198,    -1,   343,    62,   418,
     201,    -1,   344,    62,   418,   201,    -1,   340,    -1,   434,
      -1,   194,   325,   195,    -1,   347,   348,    -1,   410,    14,
     345,    -1,   177,    74,   180,   324,    -1,   349,   360,    -1,
     349,   360,   363,    -1,   360,    -1,   360,   363,    -1,   350,
      -1,   349,   350,    -1,   351,    -1,   352,    -1,   353,    -1,
     354,    -1,   355,    -1,   356,    -1,   177,    74,   180,   324,
      -1,   184,    74,    14,   324,    -1,   178,   324,    -1,   179,
      74,   180,   324,   181,   324,   182,   324,    -1,   179,    74,
     180,   324,   181,   324,   182,   324,   183,    74,    -1,   185,
     357,    -1,   358,    -1,   357,     9,   358,    -1,   324,    -1,
     324,   359,    -1,   186,    -1,   187,    -1,   361,    -1,   362,
      -1,   188,   324,    -1,   189,   324,   190,   324,    -1,   183,
      74,   348,    -1,   364,     9,    74,    -1,   364,     9,    32,
      74,    -1,    74,    -1,    32,    74,    -1,   162,   148,   366,
     163,    -1,   368,    47,    -1,   368,   163,   369,   162,    47,
     367,    -1,    -1,   148,    -1,   368,   370,    14,   371,    -1,
      -1,   369,   372,    -1,    -1,   148,    -1,   149,    -1,   197,
     324,   198,    -1,   149,    -1,   197,   324,   198,    -1,   365,
      -1,   374,    -1,   373,    27,   374,    -1,   373,    44,   374,
      -1,   211,    -1,    65,    -1,    99,    -1,   100,    -1,   101,
      -1,   147,    -1,   174,    -1,   102,    -1,   103,    -1,   161,
      -1,   104,    -1,    66,    -1,    67,    -1,    69,    -1,    68,
      -1,    83,    -1,    84,    -1,    82,    -1,    85,    -1,    86,
      -1,    87,    -1,    88,    -1,    89,    -1,    90,    -1,    50,
      -1,    91,    -1,    92,    -1,    93,    -1,    94,    -1,    95,
      -1,    96,    -1,    98,    -1,    97,    -1,    81,    -1,    13,
      -1,   118,    -1,   119,    -1,   120,    -1,   121,    -1,    64,
      -1,    63,    -1,   113,    -1,     5,    -1,     7,    -1,     6,
      -1,     4,    -1,     3,    -1,   143,    -1,   105,    -1,   106,
      -1,   115,    -1,   116,    -1,   117,    -1,   112,    -1,   111,
      -1,   110,    -1,   109,    -1,   108,    -1,   107,    -1,   175,
      -1,   114,    -1,   124,    -1,   125,    -1,    10,    -1,    12,
      -1,    11,    -1,   127,    -1,   129,    -1,   128,    -1,   130,
      -1,   131,    -1,   145,    -1,   144,    -1,   173,    -1,   156,
      -1,   158,    -1,   157,    -1,   169,    -1,   171,    -1,   168,
      -1,   217,   194,   277,   195,    -1,   218,    -1,   148,    -1,
     376,    -1,   112,    -1,   416,    -1,   376,    -1,   112,    -1,
     420,    -1,   194,   195,    -1,   315,    -1,    -1,    -1,    79,
      -1,   429,    -1,   194,   277,   195,    -1,    -1,    70,    -1,
      71,    -1,    80,    -1,   130,    -1,   131,    -1,   145,    -1,
     127,    -1,   158,    -1,   128,    -1,   129,    -1,   144,    -1,
     173,    -1,   138,    79,   139,    -1,   138,   139,    -1,   382,
      -1,   216,    -1,    43,   383,    -1,    44,   383,    -1,   125,
     194,   386,   195,    -1,   176,   194,   386,   195,    -1,    62,
     386,   201,    -1,   168,   194,   338,   195,    -1,   384,    -1,
     342,    -1,   218,   142,   211,    -1,   148,   142,   211,    -1,
     218,   142,   118,    -1,   216,    -1,    73,    -1,   434,    -1,
     382,    -1,   202,   429,   202,    -1,   203,   429,   203,    -1,
     138,   429,   139,    -1,   389,   387,    -1,    -1,     9,    -1,
      -1,     9,    -1,    -1,   389,     9,   383,   123,   383,    -1,
     389,     9,   383,    -1,   383,   123,   383,    -1,   383,    -1,
      70,    -1,    71,    -1,    80,    -1,   138,    79,   139,    -1,
     138,   139,    -1,    70,    -1,    71,    -1,   211,    -1,   390,
      -1,   211,    -1,    43,   391,    -1,    44,   391,    -1,   125,
     194,   393,   195,    -1,   176,   194,   393,   195,    -1,    62,
     393,   201,    -1,   168,   194,   396,   195,    -1,   394,   387,
      -1,    -1,   394,     9,   392,   123,   392,    -1,   394,     9,
     392,    -1,   392,   123,   392,    -1,   392,    -1,   395,     9,
     392,    -1,   392,    -1,   397,   387,    -1,    -1,   397,     9,
     334,   123,   392,    -1,   334,   123,   392,    -1,   395,   387,
      -1,    -1,   194,   398,   195,    -1,    -1,   400,     9,   211,
     399,    -1,   211,   399,    -1,    -1,   402,   400,   387,    -1,
      42,   401,    41,    -1,   403,    -1,    -1,   406,    -1,   122,
     415,    -1,   122,   211,    -1,   122,   197,   324,   198,    -1,
      62,   418,   201,    -1,   197,   324,   198,    -1,   411,   407,
      -1,   194,   314,   195,   407,    -1,   421,   407,    -1,   194,
     314,   195,   407,    -1,   415,    -1,   375,    -1,   413,    -1,
     414,    -1,   408,    -1,   410,   405,    -1,   194,   314,   195,
     405,    -1,   377,   142,   415,    -1,   412,   194,   277,   195,
      -1,   194,   410,   195,    -1,   375,    -1,   413,    -1,   414,
      -1,   408,    -1,   410,   406,    -1,   194,   314,   195,   406,
      -1,   412,   194,   277,   195,    -1,   194,   410,   195,    -1,
     415,    -1,   408,    -1,   194,   410,   195,    -1,   410,   122,
     211,   439,   194,   277,   195,    -1,   410,   122,   415,   194,
     277,   195,    -1,   410,   122,   197,   324,   198,   194,   277,
     195,    -1,   194,   314,   195,   122,   211,   439,   194,   277,
     195,    -1,   194,   314,   195,   122,   415,   194,   277,   195,
      -1,   194,   314,   195,   122,   197,   324,   198,   194,   277,
     195,    -1,   377,   142,   211,   439,   194,   277,   195,    -1,
     377,   142,   415,   194,   277,   195,    -1,   377,   142,   197,
     324,   198,   194,   277,   195,    -1,   416,    -1,   419,   416,
      -1,   416,    62,   418,   201,    -1,   416,   197,   324,   198,
      -1,   417,    -1,    74,    -1,   199,   197,   324,   198,    -1,
     324,    -1,    -1,   199,    -1,   419,   199,    -1,   415,    -1,
     409,    -1,   420,   405,    -1,   194,   314,   195,   405,    -1,
     377,   142,   415,    -1,   194,   410,   195,    -1,    -1,   409,
      -1,   420,   406,    -1,   194,   314,   195,   406,    -1,   194,
     410,   195,    -1,   422,     9,    -1,   422,     9,   410,    -1,
     422,     9,   124,   194,   422,   195,    -1,    -1,   410,    -1,
     124,   194,   422,   195,    -1,   424,   387,    -1,    -1,   424,
       9,   324,   123,   324,    -1,   424,     9,   324,    -1,   324,
     123,   324,    -1,   324,    -1,   424,     9,   324,   123,    32,
     410,    -1,   424,     9,    32,   410,    -1,   324,   123,    32,
     410,    -1,    32,   410,    -1,   426,   387,    -1,    -1,   426,
       9,   324,   123,   324,    -1,   426,     9,   324,    -1,   324,
     123,   324,    -1,   324,    -1,   428,   387,    -1,    -1,   428,
       9,   383,   123,   383,    -1,   428,     9,   383,    -1,   383,
     123,   383,    -1,   383,    -1,   429,   430,    -1,   429,    79,
      -1,   430,    -1,    79,   430,    -1,    74,    -1,    74,    62,
     431,   201,    -1,    74,   122,   211,    -1,   140,   324,   198,
      -1,   140,    73,    62,   324,   201,   198,    -1,   141,   410,
     198,    -1,   211,    -1,    75,    -1,    74,    -1,   115,   194,
     433,   195,    -1,   116,   194,   410,   195,    -1,   116,   194,
     325,   195,    -1,   116,   194,   314,   195,    -1,     7,   324,
      -1,     6,   324,    -1,     5,   194,   324,   195,    -1,     4,
     324,    -1,     3,   324,    -1,   410,    -1,   433,     9,   410,
      -1,   377,   142,   211,    -1,   377,   142,   118,    -1,    -1,
      91,   448,    -1,   169,   438,    14,   448,   196,    -1,   171,
     438,   435,    14,   448,   196,    -1,   211,    -1,   448,   211,
      -1,   211,    -1,   211,   164,   443,   165,    -1,   164,   440,
     165,    -1,    -1,   448,    -1,   440,     9,   448,    -1,   440,
       9,   159,    -1,   440,    -1,   159,    -1,    -1,    -1,    27,
     448,    -1,   443,     9,   211,    -1,   211,    -1,   443,     9,
     211,    91,   448,    -1,   211,    91,   448,    -1,    80,   123,
     448,    -1,   445,     9,   444,    -1,   444,    -1,   445,   387,
      -1,    -1,   168,   194,   446,   195,    -1,    26,   448,    -1,
      52,   448,    -1,   218,    -1,   125,    -1,   126,    -1,   447,
      -1,   125,   164,   448,   165,    -1,   125,   164,   448,     9,
     448,   165,    -1,   148,    -1,   194,    99,   194,   441,   195,
      27,   448,   195,    -1,   194,   440,     9,   448,   195,    -1,
     448,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   725,   725,   725,   734,   736,   739,   740,   741,   742,
     743,   744,   747,   749,   749,   751,   751,   753,   754,   759,
     760,   761,   762,   763,   764,   765,   766,   767,   768,   769,
     770,   771,   772,   773,   774,   775,   776,   777,   778,   782,
     784,   787,   788,   789,   790,   795,   796,   800,   801,   803,
     806,   812,   819,   826,   830,   836,   838,   841,   842,   843,
     844,   847,   848,   852,   857,   857,   863,   863,   870,   869,
     875,   875,   880,   881,   882,   883,   884,   885,   886,   887,
     888,   889,   890,   891,   892,   895,   893,   900,   908,   902,
     912,   910,   914,   915,   919,   920,   921,   922,   923,   924,
     925,   926,   927,   928,   929,   937,   937,   942,   948,   952,
     952,   960,   961,   965,   966,   970,   975,   974,   987,   985,
     999,   997,  1013,  1012,  1031,  1029,  1048,  1047,  1056,  1054,
    1066,  1065,  1077,  1075,  1088,  1089,  1093,  1096,  1099,  1100,
    1101,  1104,  1106,  1109,  1110,  1113,  1114,  1117,  1118,  1122,
    1123,  1128,  1129,  1132,  1133,  1134,  1138,  1139,  1143,  1144,
    1148,  1149,  1153,  1154,  1159,  1160,  1165,  1166,  1167,  1168,
    1171,  1174,  1176,  1179,  1180,  1184,  1186,  1189,  1192,  1195,
    1196,  1199,  1200,  1204,  1206,  1208,  1209,  1213,  1217,  1221,
    1226,  1231,  1236,  1241,  1247,  1256,  1258,  1260,  1261,  1265,
    1268,  1271,  1275,  1279,  1283,  1287,  1292,  1300,  1302,  1305,
    1306,  1307,  1309,  1314,  1315,  1318,  1319,  1320,  1324,  1325,
    1327,  1328,  1332,  1334,  1337,  1337,  1341,  1340,  1344,  1348,
    1346,  1361,  1358,  1371,  1373,  1375,  1377,  1379,  1381,  1383,
    1387,  1388,  1389,  1392,  1398,  1401,  1407,  1410,  1415,  1417,
    1422,  1427,  1431,  1432,  1438,  1439,  1444,  1445,  1450,  1451,
    1455,  1456,  1460,  1462,  1468,  1473,  1474,  1476,  1480,  1481,
    1482,  1483,  1487,  1488,  1489,  1490,  1491,  1492,  1494,  1499,
    1502,  1503,  1507,  1508,  1512,  1513,  1516,  1517,  1520,  1521,
    1524,  1525,  1529,  1530,  1531,  1532,  1533,  1534,  1535,  1539,
    1540,  1543,  1544,  1545,  1548,  1550,  1552,  1553,  1556,  1558,
    1562,  1563,  1565,  1566,  1567,  1570,  1574,  1575,  1579,  1580,
    1584,  1585,  1589,  1593,  1598,  1602,  1606,  1611,  1612,  1613,
    1616,  1618,  1619,  1620,  1623,  1624,  1625,  1626,  1627,  1628,
    1629,  1630,  1631,  1632,  1633,  1634,  1635,  1636,  1637,  1638,
    1639,  1640,  1641,  1642,  1643,  1644,  1645,  1646,  1647,  1648,
    1649,  1650,  1651,  1652,  1653,  1654,  1655,  1656,  1657,  1658,
    1659,  1660,  1661,  1662,  1663,  1665,  1666,  1668,  1670,  1671,
    1672,  1673,  1674,  1675,  1676,  1677,  1678,  1679,  1680,  1681,
    1682,  1683,  1684,  1685,  1686,  1687,  1688,  1689,  1690,  1694,
    1698,  1703,  1702,  1717,  1715,  1732,  1732,  1747,  1747,  1765,
    1766,  1771,  1776,  1780,  1786,  1790,  1796,  1798,  1802,  1804,
    1808,  1812,  1813,  1817,  1824,  1831,  1833,  1838,  1839,  1840,
    1844,  1848,  1852,  1856,  1858,  1860,  1862,  1867,  1868,  1873,
    1874,  1875,  1876,  1877,  1878,  1882,  1886,  1890,  1894,  1899,
    1904,  1908,  1909,  1913,  1914,  1918,  1919,  1923,  1924,  1928,
    1932,  1936,  1940,  1941,  1942,  1943,  1947,  1953,  1962,  1975,
    1976,  1979,  1982,  1985,  1986,  1989,  1993,  1996,  1999,  2006,
    2007,  2011,  2012,  2014,  2018,  2019,  2020,  2021,  2022,  2023,
    2024,  2025,  2026,  2027,  2028,  2029,  2030,  2031,  2032,  2033,
    2034,  2035,  2036,  2037,  2038,  2039,  2040,  2041,  2042,  2043,
    2044,  2045,  2046,  2047,  2048,  2049,  2050,  2051,  2052,  2053,
    2054,  2055,  2056,  2057,  2058,  2059,  2060,  2061,  2062,  2063,
    2064,  2065,  2066,  2067,  2068,  2069,  2070,  2071,  2072,  2073,
    2074,  2075,  2076,  2077,  2078,  2079,  2080,  2081,  2082,  2083,
    2084,  2085,  2086,  2087,  2088,  2089,  2090,  2091,  2092,  2093,
    2094,  2095,  2096,  2097,  2101,  2106,  2107,  2110,  2111,  2112,
    2116,  2117,  2118,  2122,  2123,  2124,  2128,  2129,  2130,  2133,
    2135,  2139,  2140,  2141,  2143,  2144,  2145,  2146,  2147,  2148,
    2149,  2150,  2151,  2152,  2155,  2160,  2161,  2162,  2163,  2164,
    2166,  2168,  2169,  2171,  2172,  2176,  2179,  2182,  2188,  2189,
    2190,  2191,  2192,  2193,  2194,  2199,  2201,  2205,  2206,  2209,
    2210,  2214,  2217,  2219,  2221,  2225,  2226,  2227,  2229,  2232,
    2236,  2237,  2238,  2241,  2242,  2243,  2244,  2245,  2247,  2249,
    2250,  2255,  2257,  2260,  2263,  2265,  2267,  2270,  2272,  2276,
    2278,  2281,  2284,  2290,  2292,  2295,  2296,  2301,  2304,  2308,
    2308,  2313,  2316,  2317,  2321,  2322,  2327,  2328,  2332,  2333,
    2337,  2338,  2343,  2345,  2350,  2351,  2352,  2353,  2354,  2355,
    2356,  2358,  2361,  2363,  2367,  2368,  2369,  2370,  2371,  2373,
    2375,  2377,  2381,  2382,  2383,  2387,  2390,  2393,  2396,  2400,
    2404,  2411,  2415,  2419,  2426,  2427,  2432,  2434,  2435,  2438,
    2439,  2442,  2443,  2447,  2448,  2452,  2453,  2454,  2455,  2457,
    2460,  2463,  2464,  2465,  2467,  2469,  2473,  2474,  2475,  2477,
    2478,  2479,  2483,  2485,  2488,  2490,  2491,  2492,  2493,  2496,
    2498,  2499,  2503,  2505,  2508,  2510,  2511,  2512,  2516,  2518,
    2521,  2524,  2526,  2528,  2532,  2533,  2535,  2536,  2542,  2543,
    2545,  2547,  2549,  2551,  2554,  2555,  2556,  2560,  2561,  2562,
    2563,  2564,  2565,  2566,  2567,  2568,  2572,  2573,  2577,  2579,
    2587,  2589,  2593,  2597,  2604,  2605,  2611,  2612,  2619,  2622,
    2626,  2629,  2634,  2635,  2636,  2637,  2641,  2642,  2646,  2648,
    2649,  2651,  2655,  2661,  2663,  2667,  2670,  2673,  2681,  2684,
    2687,  2688,  2691,  2694,  2695,  2698,  2702,  2706,  2712,  2720,
    2721
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
  "T_DNUMBER", "T_STRING", "T_STRING_VARNAME", "T_VARIABLE",
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
     426,   427,   428,   429,    40,    41,    59,   123,   125,    36,
      96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   204,   206,   205,   207,   207,   208,   208,   208,   208,
     208,   208,   208,   209,   208,   210,   208,   208,   208,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   212,
     212,   213,   213,   213,   213,   214,   214,   215,   215,   215,
     216,   217,   218,   219,   219,   220,   220,   221,   221,   221,
     221,   222,   222,   222,   223,   222,   224,   222,   225,   222,
     226,   222,   222,   222,   222,   222,   222,   222,   222,   222,
     222,   222,   222,   222,   222,   227,   222,   222,   228,   222,
     229,   222,   222,   222,   222,   222,   222,   222,   222,   222,
     222,   222,   222,   222,   222,   231,   230,   232,   232,   234,
     233,   235,   235,   236,   236,   237,   239,   238,   240,   238,
     241,   238,   243,   242,   244,   242,   245,   242,   246,   242,
     248,   247,   249,   247,   250,   250,   251,   252,   253,   253,
     253,   254,   254,   255,   255,   256,   256,   257,   257,   258,
     258,   259,   259,   260,   260,   260,   261,   261,   262,   262,
     263,   263,   264,   264,   265,   265,   266,   266,   266,   266,
     267,   267,   267,   268,   268,   269,   269,   270,   270,   271,
     271,   272,   272,   273,   273,   273,   273,   274,   274,   274,
     274,   274,   274,   274,   274,   275,   275,   275,   275,   276,
     276,   276,   276,   276,   276,   276,   276,   277,   277,   278,
     278,   278,   278,   279,   279,   280,   280,   280,   281,   281,
     281,   281,   282,   282,   284,   283,   285,   283,   283,   286,
     283,   287,   283,   283,   283,   283,   283,   283,   283,   283,
     288,   288,   288,   289,   290,   290,   291,   291,   292,   292,
     293,   293,   294,   294,   294,   294,   295,   295,   296,   296,
     297,   297,   298,   298,   299,   300,   300,   300,   301,   301,
     301,   301,   302,   302,   302,   302,   302,   302,   302,   303,
     303,   303,   304,   304,   305,   305,   306,   306,   307,   307,
     308,   308,   309,   309,   309,   309,   309,   309,   309,   310,
     310,   311,   311,   311,   312,   312,   312,   312,   313,   313,
     314,   314,   314,   314,   314,   315,   316,   316,   317,   317,
     318,   318,   319,   320,   321,   322,   323,   324,   324,   324,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   326,
     326,   328,   327,   329,   327,   331,   330,   332,   330,   333,
     333,   334,   335,   335,   336,   336,   337,   337,   338,   338,
     339,   340,   340,   341,   342,   343,   343,   344,   344,   344,
     345,   346,   347,   348,   348,   348,   348,   349,   349,   350,
     350,   350,   350,   350,   350,   351,   352,   353,   354,   355,
     356,   357,   357,   358,   358,   359,   359,   360,   360,   361,
     362,   363,   364,   364,   364,   364,   365,   366,   366,   367,
     367,   368,   368,   369,   369,   370,   371,   371,   372,   372,
     372,   373,   373,   373,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   375,   376,   376,   377,   377,   377,
     378,   378,   378,   379,   379,   379,   380,   380,   380,   381,
     381,   382,   382,   382,   382,   382,   382,   382,   382,   382,
     382,   382,   382,   382,   382,   383,   383,   383,   383,   383,
     383,   383,   383,   383,   383,   384,   384,   384,   385,   385,
     385,   385,   385,   385,   385,   386,   386,   387,   387,   388,
     388,   389,   389,   389,   389,   390,   390,   390,   390,   390,
     391,   391,   391,   392,   392,   392,   392,   392,   392,   392,
     392,   393,   393,   394,   394,   394,   394,   395,   395,   396,
     396,   397,   397,   398,   398,   399,   399,   400,   400,   402,
     401,   403,   404,   404,   405,   405,   406,   406,   407,   407,
     408,   408,   409,   409,   410,   410,   410,   410,   410,   410,
     410,   410,   410,   410,   411,   411,   411,   411,   411,   411,
     411,   411,   412,   412,   412,   413,   413,   413,   413,   413,
     413,   414,   414,   414,   415,   415,   416,   416,   416,   417,
     417,   418,   418,   419,   419,   420,   420,   420,   420,   420,
     420,   421,   421,   421,   421,   421,   422,   422,   422,   422,
     422,   422,   423,   423,   424,   424,   424,   424,   424,   424,
     424,   424,   425,   425,   426,   426,   426,   426,   427,   427,
     428,   428,   428,   428,   429,   429,   429,   429,   430,   430,
     430,   430,   430,   430,   431,   431,   431,   432,   432,   432,
     432,   432,   432,   432,   432,   432,   433,   433,   434,   434,
     435,   435,   436,   436,   437,   437,   438,   438,   439,   439,
     440,   440,   441,   441,   441,   441,   442,   442,   443,   443,
     443,   443,   444,   445,   445,   446,   446,   447,   448,   448,
     448,   448,   448,   448,   448,   448,   448,   448,   448,   449,
     449
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
       1,     1,     1,     3,     2,     1,     1,     2,     2,     4,
       4,     3,     4,     1,     1,     3,     3,     3,     1,     1,
       1,     1,     3,     3,     3,     2,     0,     1,     0,     1,
       0,     5,     3,     3,     1,     1,     1,     1,     3,     2,
       1,     1,     1,     1,     1,     2,     2,     4,     4,     3,
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
       5,     3,     3,     1,     2,     2,     1,     2,     1,     4,
       3,     3,     6,     3,     1,     1,     1,     4,     4,     4,
       4,     2,     2,     4,     2,     2,     1,     3,     3,     3,
       0,     2,     5,     6,     1,     2,     1,     4,     3,     0,
       1,     3,     3,     1,     1,     0,     0,     2,     3,     1,
       5,     3,     3,     3,     1,     2,     0,     4,     2,     2,
       1,     1,     1,     1,     4,     6,     1,     8,     5,     1,
       0
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   659,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   733,     0,   721,   575,
       0,   581,   582,    19,   609,   709,    84,   583,     0,    66,
       0,     0,     0,     0,     0,     0,     0,     0,   115,     0,
       0,     0,     0,     0,     0,   292,   293,   294,   297,   296,
     295,     0,     0,     0,     0,   138,     0,     0,     0,   587,
     589,   590,   584,   585,     0,     0,   591,   586,     0,     0,
     566,    20,    21,    22,    24,    23,     0,   588,     0,     0,
       0,     0,   592,     0,   298,    25,    26,    27,    29,    28,
      30,    31,    32,    33,    34,    35,    36,    37,    38,   407,
       0,    83,    56,   713,   576,     0,     0,     4,    45,    47,
      50,   608,     0,   565,     0,     6,   114,     7,     8,     9,
       0,     0,   290,   329,     0,     0,     0,     0,     0,     0,
       0,   327,   396,   397,   393,   392,   314,   398,     0,     0,
     313,   675,   567,     0,   611,   391,   289,   678,   328,     0,
       0,   676,   677,   674,   704,   708,     0,   381,   610,    10,
     297,   296,   295,     0,     0,    45,   114,     0,   775,   328,
     774,     0,   772,   771,   395,     0,     0,   365,   366,   367,
     368,   390,   388,   387,   386,   385,   384,   383,   382,   709,
     568,     0,   789,   567,     0,   348,   346,     0,   737,     0,
     618,   312,   571,     0,   789,   570,     0,   580,   716,   715,
     572,     0,     0,   574,   389,     0,     0,     0,     0,   317,
       0,    64,   319,     0,     0,    70,    72,     0,     0,    74,
       0,     0,     0,   811,   812,   816,     0,     0,    45,   810,
       0,   813,     0,     0,    76,     0,     0,     0,     0,   105,
       0,     0,     0,     0,    40,    41,   215,     0,     0,   214,
     140,   139,   220,     0,     0,     0,     0,     0,   786,   126,
     136,   729,   733,   758,     0,   594,     0,     0,     0,   756,
       0,    15,     0,    49,     0,   320,   130,   137,   472,   417,
       0,   780,   324,   663,   329,     0,   327,   328,     0,     0,
     577,     0,   578,     0,     0,     0,   104,     0,     0,    52,
     208,     0,    18,   113,     0,   135,   122,   134,   295,   114,
     291,    95,    96,    97,    98,    99,   101,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   721,    94,   712,   712,   102,   743,     0,     0,
       0,     0,     0,   288,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   347,   345,     0,   679,
     664,   712,     0,   670,   208,   712,     0,   714,   705,   729,
       0,   114,     0,     0,   661,   656,   618,     0,     0,     0,
       0,   741,     0,   422,   617,   732,     0,     0,    52,     0,
     208,   311,     0,   717,   664,   672,   573,     0,    56,   176,
       0,   406,     0,    81,     0,     0,   318,     0,     0,     0,
       0,     0,    73,    93,    75,   808,   809,     0,   806,     0,
       0,   790,     0,   785,     0,   100,    77,   103,     0,     0,
       0,     0,     0,     0,     0,   430,     0,   437,   439,   440,
     441,   442,   443,   444,   435,   457,   458,    56,     0,    90,
      92,    42,     0,    17,     0,     0,   216,     0,    79,     0,
       0,    80,   776,     0,     0,   329,   327,   328,     0,     0,
     146,     0,   730,     0,     0,     0,     0,   593,   757,   609,
       0,     0,   755,   614,   754,    48,     5,    12,    13,    78,
       0,   144,     0,     0,   411,     0,   618,     0,     0,     0,
       0,   197,     0,   620,   662,   820,   310,   378,   683,    61,
      55,    57,    58,    59,    60,     0,   394,   612,   613,    46,
       0,     0,     0,   620,   209,     0,   401,   116,   142,     0,
     351,   353,   352,     0,     0,   349,   350,   354,   356,   355,
     370,   369,   372,   371,   373,   375,   376,   374,   364,   363,
     358,   359,   357,   360,   361,   362,   377,   711,     0,     0,
     747,     0,   618,   779,     0,   778,   681,   704,   128,   132,
     124,   114,     0,     0,   322,   325,   331,   431,   344,   343,
     342,   341,   340,   339,   338,   337,   336,   335,   334,     0,
     666,   665,     0,     0,     0,     0,     0,     0,     0,   773,
     654,   658,   617,   660,     0,     0,   789,     0,   736,     0,
     735,     0,   720,   719,     0,     0,   666,   665,   315,   178,
     180,    56,   409,   316,     0,    56,   160,    65,   319,     0,
       0,     0,     0,   172,   172,    71,     0,     0,   804,   618,
       0,   795,     0,     0,     0,   616,     0,     0,   566,     0,
      25,    50,   596,   565,   604,     0,   595,    54,   603,     0,
       0,   447,     0,     0,   453,   450,   451,   459,     0,   438,
     433,     0,   436,     0,     0,     0,     0,    39,    43,     0,
     213,   221,   218,     0,     0,   767,   770,   769,   768,    11,
     799,     0,     0,     0,   729,   726,     0,   421,   766,   765,
     764,     0,   760,     0,   761,   763,     0,     5,   321,     0,
       0,   466,   467,   475,   474,     0,     0,   617,   416,   420,
       0,   781,     0,   796,   663,   196,   819,     0,     0,   680,
     664,   671,   710,     0,   788,   210,   564,   619,   207,     0,
     663,     0,     0,   144,   403,   118,   380,     0,   425,   426,
       0,   423,   617,   742,     0,     0,   208,   146,   144,   142,
       0,   721,   332,     0,     0,   208,   668,   669,   682,   706,
     707,     0,     0,     0,   642,   625,   626,   627,     0,     0,
       0,    25,   634,   633,   648,   618,     0,   656,   740,   739,
       0,   718,   664,   673,   579,     0,   182,     0,     0,    62,
       0,     0,     0,     0,     0,     0,   152,   153,   164,     0,
      56,   162,    87,   172,     0,   172,     0,     0,   814,     0,
     617,   805,   807,   794,   793,     0,   791,   597,   598,   624,
       0,   618,   616,     0,     0,   419,   616,     0,   749,   432,
       0,     0,     0,   455,   456,   454,     0,     0,   434,     0,
     106,     0,   109,    91,    44,   217,     0,   777,    82,     0,
       0,   787,   145,   147,   223,     0,     0,   727,     0,   759,
       0,    16,     0,   143,   223,     0,     0,   413,     0,   782,
       0,     0,     0,   195,   820,     0,   199,     0,   666,   665,
     791,     0,   211,    53,     0,   663,   141,     0,   663,     0,
     379,   746,   745,     0,   208,     0,     0,     0,   144,   120,
     580,   667,   208,     0,     0,   630,   631,   632,   635,   636,
     646,     0,   618,   642,     0,   629,   650,   642,   617,   653,
     655,   657,     0,   734,   667,     0,     0,     0,     0,   179,
     410,    67,     0,   319,   154,   729,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   166,     0,   802,   803,     0,
       0,   818,     0,   601,   617,   615,     0,   606,     0,   618,
       0,     0,   607,   605,   753,     0,   618,   445,     0,   446,
     452,   460,   461,     0,    56,   219,   801,   798,     0,   289,
     731,   729,   323,   326,   330,     0,    14,   289,   478,     0,
       0,   480,   473,   476,     0,   471,     0,   783,   797,   408,
       0,   200,     0,     0,     0,   208,   212,   796,     0,   223,
       0,   663,     0,   208,     0,   702,   223,   223,     0,     0,
     333,   208,     0,   696,     0,   639,   617,   641,     0,   628,
       0,     0,   618,     0,   647,   738,     0,    56,     0,   175,
     161,     0,     0,   151,    85,   165,     0,     0,   168,     0,
     173,   174,    56,   167,   815,   792,     0,   623,   622,   599,
       0,   617,   418,   602,   600,     0,   424,   617,   748,     0,
       0,     0,     0,   148,     0,     0,     0,   287,     0,     0,
       0,   127,   222,   224,     0,   286,     0,   289,     0,   762,
     131,   469,     0,     0,   412,     0,   203,     0,   202,   667,
     208,     0,   400,   796,   289,   796,     0,   744,     0,   701,
     289,   289,   223,   663,     0,   695,   645,   644,   637,     0,
     640,   617,   649,   638,    56,   181,    63,    68,   155,     0,
     163,   169,    56,   171,     0,     0,   415,     0,   752,   751,
       0,    56,   110,   800,     0,     0,     0,     0,   149,   254,
     252,   566,    24,     0,   248,     0,   253,   264,     0,   262,
     267,     0,   266,     0,   265,     0,   114,   226,     0,   228,
       0,   728,   470,   468,   479,   477,   204,     0,   201,   208,
       0,   699,     0,     0,     0,   123,   400,   796,   703,   129,
     133,   289,     0,   697,     0,   652,     0,   177,     0,    56,
     158,    86,   170,   817,   621,     0,     0,     0,     0,     0,
       0,     0,     0,   238,   242,     0,     0,   233,   530,   529,
     526,   528,   527,   547,   549,   548,   518,   508,   524,   523,
     485,   495,   496,   498,   497,   517,   501,   499,   500,   502,
     503,   504,   505,   506,   507,   509,   510,   511,   512,   513,
     514,   516,   515,   486,   487,   488,   491,   492,   494,   532,
     533,   542,   541,   540,   539,   538,   537,   525,   544,   534,
     535,   536,   519,   520,   521,   522,   545,   546,   550,   552,
     551,   553,   554,   531,   556,   555,   489,   558,   560,   559,
     493,   563,   561,   562,   557,   490,   543,   484,   259,   481,
       0,   234,   280,   281,   279,   272,     0,   273,   235,   306,
       0,     0,     0,     0,   114,     0,   206,     0,   698,     0,
      56,   282,    56,   117,     0,     0,   125,   796,   643,     0,
      56,   156,    69,     0,   414,   750,   448,   108,   236,   237,
     309,   150,     0,     0,   256,   249,     0,     0,     0,   261,
     263,     0,     0,   268,   275,   276,   274,     0,     0,   225,
       0,     0,     0,     0,   205,   700,     0,   464,   620,     0,
       0,    56,   119,     0,   651,     0,     0,     0,    88,   239,
      45,     0,   240,   241,     0,     0,   255,   258,   482,   483,
       0,   250,   277,   278,   270,   271,   269,   307,   304,   229,
     227,   308,     0,   465,   619,     0,   402,   283,     0,   121,
       0,   159,   449,     0,   112,     0,   289,   257,   260,     0,
     663,   231,     0,   462,   399,   404,   157,     0,     0,    89,
     246,     0,   288,   305,   185,     0,   620,   300,   663,   463,
       0,   111,     0,     0,   245,   796,   663,   184,   301,   302,
     303,   820,   299,     0,     0,     0,   244,     0,   183,   300,
       0,   796,     0,   243,   284,    56,   230,   820,     0,   187,
       0,    56,     0,     0,   188,     0,   232,     0,   285,     0,
     191,     0,   190,   107,   192,     0,   189,     0,   194,   193
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   117,   737,   516,   175,   263,   264,
     119,   120,   121,   122,   123,   124,   308,   540,   541,   435,
     230,  1238,   441,  1169,  1454,   705,   260,   477,  1418,   883,
    1014,  1469,   324,   176,   542,   771,   929,  1059,   543,   558,
     789,   500,   787,   544,   521,   788,   326,   279,   296,   130,
     773,   740,   723,   892,  1187,   977,   836,  1372,  1241,   657,
     842,   440,   665,   844,  1092,   650,   826,   829,   967,  1475,
    1476,   532,   533,   552,   553,   268,   269,   273,  1019,  1122,
    1205,  1352,  1460,  1478,  1382,  1422,  1423,  1424,  1193,  1194,
    1195,  1383,  1389,  1431,  1198,  1199,  1203,  1345,  1346,  1347,
    1363,  1506,  1123,  1124,   177,   132,  1491,  1492,  1350,  1126,
     133,   223,   436,   437,   134,   135,   136,   137,   138,   139,
     140,   141,  1223,   142,   770,   928,   143,   227,   303,   431,
     525,   526,   999,   527,  1000,   144,   145,   146,   684,   147,
     148,   257,   149,   258,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   695,   696,   875,   474,   475,   476,   702,
    1408,   150,   522,  1213,   523,   905,   745,  1035,  1032,  1338,
    1339,   151,   152,   153,   217,   224,   311,   421,   154,   859,
     688,   155,   860,   415,   755,   861,   813,   948,   950,   951,
     952,   815,  1071,  1072,   816,   631,   406,   185,   186,   156,
     535,   389,   390,   761,   157,   218,   179,   159,   160,   161,
     162,   163,   164,   165,   588,   166,   220,   221,   503,   209,
     210,   591,   592,  1005,  1006,   288,   289,   731,   167,   493,
     168,   530,   169,   250,   280,   319,   450,   855,   912,   721,
     668,   669,   670,   251,   252,   757
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1015
static const yytype_int16 yypact[] =
{
   -1015,   134, -1015, -1015,  4260, 11898, 11898,   -46, 11898, 11898,
   11898, -1015, 11898, 11898, 11898, 11898, 11898, 11898, 11898, 11898,
   11898, 11898, 11898, 11898, 13593, 13593,  9285, 11898, 13658,   -39,
     -24, -1015, -1015, -1015, -1015,   175, -1015, -1015, 11898, -1015,
     -24,    78,   152,   154,   -24,  9486,  9930,  9687, -1015,  2535,
    8883,    52, 11898,  2612,   119, -1015, -1015, -1015,   241,   253,
      43,   183,   193,   201,   272, -1015,  9930,   274,   281, -1015,
   -1015, -1015, -1015, -1015,   384,  1487, -1015, -1015,  9930,  9888,
   -1015, -1015, -1015, -1015, -1015, -1015,  9930, -1015,   296,   297,
    9930,  9930, -1015, 11898, -1015, -1015, -1015, -1015, -1015, -1015,
   -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015,
   11898, -1015, -1015,   192,   353,   400,   400, -1015,   456,   352,
     309, -1015,   302, -1015,    51, -1015,   472, -1015, -1015, -1015,
   13978,   666, -1015, -1015,   312,   314,   349,   355,   363,   380,
   12369, -1015, -1015, -1015, -1015,   487, -1015,   518,   519,   387,
   -1015,    89,   398,   405, -1015, -1015,   967,    90,   514,    91,
     390,   101,   135,   406,    83, -1015,   158, -1015,   539, -1015,
   -1015, -1015,   461,   410,   473, -1015,   472,   666, 14499,  1800,
   14499, 11898, 14499, 14499,  4246,   579,  9930, -1015, -1015,   574,
   -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015,
   -1015, 13319,   463, -1015,   488,   503,   503, 13593, 14159,   430,
     624, -1015,   461, 13319,   463,   492,   493,   444,   139, -1015,
     517,    91, 10089, -1015, -1015, 11898,  7476,   632,    56, 14499,
    8481, -1015, 11898, 11898,  9930, -1015, -1015, 12624,   447, -1015,
   12665,  2535,  2535,   484, -1015, -1015,   453,  2047,   636, -1015,
     637, -1015,  9930,   581, -1015,   460, 12706,   471,   -17, -1015,
      46, 12747,  9930,    58, -1015,    68, -1015, 13371,    59, -1015,
   -1015, -1015,   645,    60, 13593, 13593, 11898,   475,   509, -1015,
   -1015, 13455,  9285,    77,   403, -1015, 12099, 13593,   567, -1015,
    9930, -1015,   223,   352,   478, 14200, -1015, -1015, -1015,   597,
     664,   588, 14499,   133,   486, 14499,   495,   592,  4461, 11898,
      54,   482,   570,    54,     1,   289, -1015,  9930,  2535,   489,
   10290,  2535, -1015, -1015,  8724, -1015, -1015, -1015, -1015,   472,
   -1015, -1015, -1015, -1015, -1015, -1015, -1015, 11898, 11898, 11898,
   10491, 11898, 11898, 11898, 11898, 11898, 11898, 11898, 11898, 11898,
   11898, 11898, 11898, 11898, 11898, 11898, 11898, 11898, 11898, 11898,
   11898, 11898, 13658, -1015, 11898, 11898, -1015, 11898,  3132,  9930,
    9930, 13978,   593,   709,  8682, 11898, 11898, 11898, 11898, 11898,
   11898, 11898, 11898, 11898, 11898, 11898, -1015, -1015,  1303, -1015,
     140, 11898, 11898, -1015, 10290, 11898, 11898,   192,   143, 13455,
     499,   472, 10692, 12793, -1015,   500,   678, 13319,   501,    -7,
    1981,   503, 10893, -1015, 11094, -1015,   520,     5, -1015,   195,
   10290, -1015, 13712, -1015,   147, -1015, -1015, 12834, -1015, -1015,
   11295, -1015, 11898, -1015,   612,  7677,   689,   505, 14392,   690,
      55,    45, -1015, -1015, -1015, -1015, -1015,  2535,   633,   523,
     703, -1015, 13181, -1015,   538, -1015, -1015, -1015,   646, 11898,
     647,   648, 11898, 11898, 11898, -1015,   -17, -1015, -1015, -1015,
   -1015, -1015, -1015, -1015,   541, -1015, -1015, -1015,   532, -1015,
   -1015,   242,  2612, -1015,  9930, 11898,   503,   119, -1015, 13181,
     653, -1015,   503,    81,    84,   536,   542,   858,   537,  9930,
     614,   547,   503,   111,   548, 13935,  9930, -1015, -1015,   680,
    1544,   -65, -1015, -1015, -1015,   352, -1015, -1015, -1015, -1015,
   11898,   623,   582,   313, -1015,   625,   741,   558,  2535,  2535,
     745, -1015,   568,   752, -1015,  2535,    96,   702,   103, -1015,
   -1015, -1015, -1015, -1015, -1015,  1675, -1015, -1015, -1015, -1015,
     109, 13593,   571,   759, 14499,   755, -1015, -1015,   651,  9126,
   14539,  4047,  4246, 11898, 14458,  3452,  4646,  4846,  2292,  2607,
    5045,  5045,  5045,  5045,  1582,  1582,  1582,  1582,   789,   789,
     506,   506,   506,   574,   574,   574, -1015, 14499,   583,   587,
   14255,   584,   774, -1015, 11898,   215,   596,   143, -1015, -1015,
   -1015,   472,  2742, 11898, -1015, -1015,  4246, -1015,  4246,  4246,
    4246,  4246,  4246,  4246,  4246,  4246,  4246,  4246,  4246, 11898,
     215,   598,   590,  1754,   600,   595,  2496,   112,   599, -1015,
   13271, -1015,  9930, -1015,   486,    96,   463, 13593, 14499, 13593,
   14296,   142,   155, -1015,   602, 11898, -1015, -1015, -1015,  7275,
     348, -1015, 14499, 14499,   -24, -1015, -1015, -1015, 11898,  1775,
   13181,  9930,  7878,   605,   606, -1015,   113,   681, -1015,   799,
     618,  3512,  2535, 13181, 13181, 13181,   620,    57,   673,   628,
     629,    15, -1015,   683, -1015,   630, -1015, -1015, -1015, 11898,
     649, 14499,   650,   824, 12916,   831, -1015, 14499, 12875, -1015,
     541,   768, -1015,  4662, 13438,   654,  9930, -1015, -1015,  3141,
   -1015, -1015,   830, 13593,   652, -1015, -1015, -1015, -1015, -1015,
     762,   116, 13438,   657, 13455, 13539,   842, -1015, -1015, -1015,
   -1015,   663, -1015, 11898, -1015, -1015,  3858, -1015, 14499, 13438,
     661, -1015, -1015, -1015, -1015,   851, 11898,   597, -1015, -1015,
     670, -1015,  2535,   840,   132, -1015, -1015,   338, 13753, -1015,
     176, -1015, -1015,  2535, -1015,   503, -1015, 11496, -1015, 13181,
      28,   675, 13438,   623, -1015, -1015,  4447, 11898, -1015, -1015,
   11898, -1015, 11898, -1015,  3231,   676, 10290,   614,   623,   651,
    9930, 13658,   503,  3365,   677, 10290, -1015, -1015,   177, -1015,
   -1015,   871,  2880,  2880, 13271, -1015, -1015, -1015,   694,    67,
     695,   696, -1015, -1015, -1015,   882,   697,   500,   503,   503,
   11697, -1015,   250, -1015, -1015,  3414,   421,   -24,  8481, -1015,
    4863,   699,  5064,   701, 13593,   712,   780,   503, -1015,   893,
   -1015, -1015, -1015, -1015,   346, -1015,   235,  2535, -1015,  2535,
     633, -1015, -1015, -1015,   899,   716,   717, -1015, -1015,   790,
     713,   912, 13181,   784,  9930,   597, 13181,  9327, 13181, 14499,
   11898, 11898, 11898, -1015, -1015, -1015, 11898, 11898, -1015,   -17,
   -1015,   853, -1015, -1015, -1015, -1015, 13181,   503, -1015,  2535,
    9930, -1015,   916, -1015, -1015,   114,   734,   503,  9084, -1015,
      66, -1015,  4059,   916, -1015,    -6,   -11, 14499,   807, -1015,
     735,  2535,   632, -1015,  2535,   859,   918, 11898,   215,   740,
   -1015, 13593, 14499, -1015,   743,    28, -1015,   747,    28,   746,
    4447, 14499, 14351,   748, 10290,   750,   749,   753,   623, -1015,
     444,   757, 10290,   760, 11898, -1015, -1015, -1015, -1015, -1015,
     829,   758,   947, 13271,   819, -1015,   597, 13271, 13271, -1015,
   -1015, -1015, 13593, 14499, -1015,   -24,   933,   894,  8481, -1015,
   -1015, -1015,   765, 11898,   503, 13455,  1775,   770, 13181,  5265,
     469,   771, 11898,    39,   267, -1015,   801, -1015, -1015, 13091,
     942, -1015, 13181, -1015, 13181, -1015,   778, -1015,   854,   970,
     781,   786, -1015, -1015,   860,   787,   973, 14499, 13002, 14499,
   -1015, 14499, -1015,   791, -1015, -1015, -1015,   896, 13438,   292,
   -1015, 13455, -1015, -1015,  4246,   792, -1015,   407, -1015,   151,
   11898, -1015, -1015, -1015, 11898, -1015, 11898, -1015, -1015, -1015,
     340,   974, 13181,  3773,   797, 10290,   503,   840,   798, -1015,
     802,    28, 11898, 10290,   803, -1015, -1015, -1015,   804,   800,
   -1015, 10290,   805, -1015, 13271, -1015, 13271, -1015,   809, -1015,
     861,   812,   999,   814, -1015,   503,   994, -1015,   828, -1015,
   -1015,   839,   115, -1015, -1015, -1015,   841,   844, -1015,  3709,
   -1015, -1015, -1015, -1015, -1015, -1015,  2535, -1015,   903, -1015,
   13181,   597, -1015, -1015, -1015, 13181, -1015, 13181, -1015, 11898,
     838,  5466,  2535, -1015,   260,  2535, 13438, -1015, 13893,   884,
   13844, -1015, -1015, -1015,   593, 13025,    62,   709,   121, -1015,
   -1015,   890, 12284, 12325, 14499,   968,  1029, 13181, -1015,   850,
   10290,   855,   940,   840,   905,   840,   856, 14499,   857, -1015,
     920,  1176, -1015,    28,   862, -1015, -1015,   923, -1015, 13271,
   -1015,   597, -1015, -1015, -1015,  7275, -1015, -1015, -1015,  8079,
   -1015, -1015, -1015,  7275,   864, 13181, -1015,   926, -1015,   931,
   12431, -1015, -1015, -1015, 13438, 13438,  1047,    49, -1015, -1015,
   -1015,    64,   866,    70, -1015, 12443, -1015, -1015,    72, -1015,
   -1015, 12905, -1015,   868, -1015,   991,   472, -1015,  2535, -1015,
     593, -1015, -1015, -1015, -1015, -1015,  1052, 13181, -1015, 10290,
     872, -1015,   874,   875,   325, -1015,   940,   840, -1015, -1015,
   -1015,  1204,   886, -1015, 13271, -1015,   946,  7275,  8280, -1015,
   -1015, -1015,  7275, -1015, -1015, 13181, 13181, 11898,  5667,   887,
     888, 13181, 13438, -1015, -1015,  1389, 13893, -1015, -1015, -1015,
   -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015,
   -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015,
   -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015,
   -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015,
   -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015,
   -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015,
   -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015,
   -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015,   364, -1015,
     884, -1015, -1015, -1015, -1015, -1015,   110,   426, -1015,  1068,
      74,  9930,   991,  1073,   472, 13181, -1015,   895, -1015,   350,
   -1015, -1015, -1015, -1015,   891,   325, -1015,   840, -1015, 13271,
   -1015, -1015, -1015,  5868, -1015, -1015, 12961, -1015, -1015, -1015,
   -1015, -1015, 13803,    29, -1015, -1015, 13181, 12443, 12443,  1039,
   -1015, 12905, 12905,   438, -1015, -1015, -1015, 13181,  1018, -1015,
     900,    75, 13181,  9930, -1015, -1015,  1022, -1015,  1088,  6069,
    6270, -1015, -1015,   325, -1015,  6471,   902,  1026,   998, -1015,
    1011,   962, -1015, -1015,  1015,  1389, -1015, -1015, -1015, -1015,
     953, -1015,  1079, -1015, -1015, -1015, -1015, -1015,  1096, -1015,
   -1015, -1015,   921, -1015,   351,   917, -1015, -1015,  6672, -1015,
     924, -1015, -1015,   927,   956,  9930,   709, -1015, -1015, 13181,
     124, -1015,  1040, -1015, -1015, -1015, -1015, 13438,   654, -1015,
     965,  9930,   479, -1015, -1015,   930,  1117,   485,   124, -1015,
    1053, -1015, 13438,   932, -1015,   840,   136, -1015, -1015, -1015,
   -1015,  2535, -1015,   934,   935,    79, -1015,   330, -1015,   485,
     361,   840,   938, -1015, -1015, -1015, -1015,  2535,  1062,  1126,
     330, -1015,  6873,   396,  1127, 13181, -1015,  7074, -1015,  1069,
    1130, 13181, -1015, -1015,  1131, 13181, -1015, 13181, -1015, -1015
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1015, -1015, -1015,  -475, -1015, -1015, -1015,    -4, -1015,   667,
     -25,   892,  2372, -1015,  1455, -1015,  -392, -1015,     3, -1015,
   -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015,  -322,
   -1015, -1015,  -165,    13,    -1, -1015, -1015, -1015,     0, -1015,
   -1015, -1015, -1015,     2, -1015, -1015,   777,   782,   783,   996,
     365,  -714,   369,   411,  -325, -1015,   182, -1015, -1015, -1015,
   -1015, -1015, -1015,  -624,    71, -1015, -1015, -1015, -1015,  -316,
   -1015,  -736, -1015,  -373, -1015, -1015,   687, -1015,  -841, -1015,
   -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015,   -93,
   -1015, -1015, -1015, -1015, -1015,  -172, -1015,    61,  -824, -1015,
    -990,  -335, -1015,  -155,    20,  -129,  -323, -1015,  -175, -1015,
     -66,   -22,  1144,  -615,  -348, -1015, -1015,   -36, -1015, -1015,
    2734,   -56,   -43, -1015, -1015, -1015, -1015, -1015, -1015,   275,
    -715, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015,
   -1015,   810, -1015, -1015,   311, -1015,   722, -1015, -1015, -1015,
   -1015, -1015, -1015, -1015,   318, -1015,   729, -1015, -1015,   498,
   -1015,   294, -1015, -1015, -1015, -1015, -1015, -1015, -1015, -1015,
    -817, -1015,  1481,   298,  -332, -1015, -1015,   261,  1383,  2331,
   -1015, -1015,  -693,  -315,  -540, -1015, -1015,   399,  -611,  -776,
   -1015, -1015, -1015, -1015, -1015,   388, -1015, -1015, -1015,  -257,
    -731,  -185,  -181,  -132, -1015, -1015,    27, -1015, -1015, -1015,
   -1015,   -12,  -137, -1015,  -211, -1015, -1015, -1015,  -379,   925,
   -1015, -1015, -1015, -1015, -1015,   483,   497, -1015, -1015,   936,
   -1015, -1015, -1015,  -309,   -81,  -199,  -281, -1015, -1014, -1015,
     356, -1015, -1015, -1015,  -186,  -889
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -790
static const yytype_int16 yytable[] =
{
     118,   372,   330,   127,   128,   297,   129,   125,   226,   300,
     301,   400,   555,   768,   255,   418,   219,   126,   231,   814,
     627,   624,   235,   914,   131,  1040,   604,   393,   265,   398,
     586,   158,   908,  1142,   924,   423,   649,   550,  1425,   424,
     846,   736,   238,   833,   304,   248,   534,   644,   330,   327,
     292,   205,   206,   293,   306,   445,   446,   388,  1252,   927,
     321,   451,   278,  1027,   661,   432,  1090,   482,   487,   490,
      11,  1208,   663,  -251,   937,   283,   337,   338,   339,  1256,
     512,  1340,   278,  1398,  1398,   703,   278,   278,  1252,   425,
     713,   633,   340,   713,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   388,   362,   272,   763,  1391,
     725,   725,   847,   725,   725,   890,   278,   388,   283,  1224,
     725,  1226,   451,   735,     3,   408,   863,   307,  1033,   505,
    1392,   286,   287,  1028,   329,   395,   954,   416,   181,   478,
     998,  -684,  -687,   391,   589,   222,  1029,  -789,   391,   484,
     458,   459,   460,  -685,   559,  -691,    11,   461,   462,   996,
     225,   463,   464,  1001,    11,    11,   373,  1068,    11,   318,
     622,  1073,   405,  -405,   625,  -568,  1034,   531,   538,  1048,
     401,  1030,  1050,   266,   286,   287,   285,  -686,  1131,   506,
     642,  -722,  -688,   547,   391,   395,   955,   479,  1144,  -723,
     495,   748,  -789,  1365,   317,  1150,  1151,  -725,   758,   980,
     496,   984,   118,  -198,  1058,  -569,   118,  1426,   409,   429,
     439,   597,   199,   434,   411,  1091,   628,   481,  -689,  -690,
     417,  1070,   664,   557,   330,  1253,  1254,   322,   453,   259,
     662,   597,   433,   158,   483,   488,   491,   158,  1209,   830,
    -251,   666,   902,   832,   422,   515,  1257,  1025,  1341,   199,
    1399,  1440,   232,   597,   764,  1503,   714,   783,   848,   715,
     396,   891,   597,  1474,  -693,   597,  -684,  -687,   392,   297,
     327,   913,   531,   392,   486,  1498,  1114,  -694,  -685,   298,
    -691,   492,   492,   497,   118,  1393,   726,   801,   502,  1020,
    1168,  1231,  -724,   549,   511,  1146,  1211,   248,   267,  -186,
     278,   126,   204,   204,  -619,  -198,   216,  -619,   131,   982,
     983,  -619,  -686,   706,    11,   158,  -722,  -688,   605,   392,
     396,   634,   750,   751,  -723,   895,   233,  1074,   234,   756,
     219,   759,  -725,  1413,   851,   760,   596,   397,  1081,   270,
     742,   982,   983,   283,   595,   278,   278,   278,   512,   317,
     915,   271,  1135,  -689,  -690,  1412,   621,   274,  1386,   318,
    1184,  1185,  1406,  1462,   620,   601,  1177,   275,   317,   309,
     854,  1387,  1115,  1508,   113,   276,   785,  1116,   596,    55,
      56,    57,   170,   171,   328,  1117,   636,   643,  1388,  -789,
     647,  1114,   916,   935,  1136,   827,   828,  1232,   646,   517,
     518,   794,   943,  1449,  1407,  1463,   502,   283,  1519,   286,
     287,   118,   310,   985,   409,  1509,   790,   785,   656,   981,
     982,   983,  1118,  1119,   298,  1120,  1236,  -724,   979,    11,
     759,  -789,  1394,  1156,   760,  1157,   821,   265,   283,   940,
     822,   743,   158,   284,  1434,  1093,   277,    94,   281,  1395,
    1520,  1497,  1396,   318,   283,   282,   744,   283,   775,   313,
     708,  1435,   418,   316,  1436,   451,   856,  1510,   965,   966,
    1121,   299,   548,   286,   287,   720,   320,   534,   317,   204,
     959,   730,   732,  -789,   323,   204,  -789,  1115,   331,   823,
     332,   204,  1116,   534,    55,    56,    57,   170,   171,   328,
    1117,  1361,  1362,   285,   286,   287,  1504,  1505,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     286,   287,   507,   286,   287,   333,   995,   368,  1235,  -427,
    1022,   334,   359,   360,   361,   278,   362,  1118,  1119,   335,
    1120,  1054,  1087,   982,   983,   204,   910,  1432,  1433,  1062,
    1428,  1429,   204,   204,   386,   387,   336,   920,   765,   204,
     364,   365,    94,   366,   394,   204,    55,    56,    57,   170,
     171,   328,  1488,  1489,  1490,   367,  1082,   312,   314,   315,
    -692,  -428,  1500,  -568,   399,  1130,   402,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,  1513,   290,
     404,   597,  1111,  1368,   362,   388,   812,   318,   817,   792,
     410,   413,   831,   414,  -567,   419,   388,  1067,   420,   422,
     430,   283,  1128,   443,   283,   118,   512,   448,   447,   512,
    -784,   452,   386,   387,    94,   454,   455,   839,   118,   489,
     216,   986,   126,   987,   818,   841,   819,   457,   534,   131,
     498,   534,  1141,   499,   519,  1484,   158,   524,   528,   529,
    1148,   536,   546,   -51,  1102,  1165,   837,   632,  1154,   158,
     537,  1108,    48,   556,   630,   654,   635,   204,   432,   118,
    1173,   658,   884,  1016,   660,   204,   513,   286,   287,   939,
     286,   287,   672,   667,   388,   641,   126,   671,   689,  1044,
     690,   692,   693,   131,   701,  1038,   704,   712,   756,  1477,
     158,   716,   118,   719,   722,   127,   128,   717,   129,   125,
     887,   724,   733,   727,   739,   741,   919,  1477,   746,   126,
     747,   502,   897,   749,   918,  1499,   131,  1162,  1414,   752,
     753,   754,  1127,   158,  -429,    48,   766,  1220,   767,   769,
    1127,   772,  1237,    55,    56,    57,   170,   171,   328,   219,
    1242,   508,   781,   782,   778,   514,   278,   538,   779,  1248,
     786,   796,   795,   774,   534,   798,   799,   824,   947,   947,
     812,   843,   845,   920,   849,   968,  1186,   508,   850,   514,
     508,   514,   514,   852,   862,   864,    55,    56,    57,   170,
     171,   328,   865,   866,   118,   867,   118,   868,   118,   870,
     871,   969,   356,   357,   358,   359,   360,   361,   872,   362,
     876,    94,   879,   126,   886,   126,  1357,  1373,   888,   204,
     131,   882,   131,   889,   894,   158,   898,   158,   904,   158,
     997,   974,  1023,  1003,   899,   906,   909,   911,  1445,   925,
     934,   942,   402,   375,   376,   377,   378,   379,   380,   381,
     382,   383,   384,   385,    94,   944,  1017,  1127,   953,   956,
     957,   958,   960,  1127,  1127,   971,   534,   973,   118,  1353,
     204,   127,   128,   976,   129,   125,   975,   978,   989,  1114,
    1174,   990,   991,   992,   993,   126,   202,   202,   386,   387,
     214,   994,   131,   507,  1114,  1018,  1183,  1013,  1021,   158,
    1036,  1037,  1042,  1041,  1045,   204,  1487,   204,  1047,  1207,
    1051,   214,  1053,  1076,  1049,  1055,  1056,    11,  1046,   812,
    1057,  1061,  1064,   812,   812,  1063,  1066,   204,  1069,  1065,
    1077,  1080,    11,  1078,   118,  1084,  1094,  1088,  1409,  1096,
    1410,  1079,  1210,  1099,  1127,   118,  1103,  1100,  1415,  1101,
     388,  1104,  1107,  1105,  1159,  1106,  1110,  1112,  1137,  1075,
    1129,  1140,   126,  1143,  1153,   158,   330,  1145,  1149,   131,
    1155,  1152,   502,   837,  1158,  1115,   158,  1160,  1161,  1163,
    1116,   204,    55,    56,    57,   170,   171,   328,  1117,  1448,
    1115,  1164,   204,   204,  1166,  1116,  1175,    55,    56,    57,
     170,   171,   328,  1117,  1167,  1181,  1197,  1170,  1212,  1125,
    1171,  1351,  1216,  1217,  1219,  1222,  1234,  1125,   502,  1245,
    1221,  1227,  1228,   718,  1246,  1118,  1119,  1233,  1120,  1243,
     812,  1251,   812,  1255,  1348,  1349,  1355,  1358,  1359,  1369,
    1118,  1119,  1360,  1120,    55,    56,    57,    58,    59,   328,
      94,  1367,  1397,  1378,  1379,    65,   369,  1402,  1411,   216,
    1405,  1430,  1438,   202,  1439,    94,  1443,  1444,  1451,   202,
    1452,  1453,  -247,  1225,  1455,   202,  1456,   118,  1458,  1392,
    1459,   248,  1464,  1512,  1479,  1461,  1202,  1468,  1229,  1517,
    1466,  1467,  1482,   370,   126,  1485,  1486,  1494,  1496,  1501,
    1502,   131,   204,   214,   214,  1511,  1514,  1206,   158,   214,
    1515,  1521,    94,  1524,  1525,  1527,  1481,   373,   600,   707,
     903,   598,   371,   599,   938,   812,   936,  1495,  1083,   202,
    1172,   118,  1493,  1385,  1125,   118,   202,   202,  1390,   118,
    1125,  1125,  1240,   202,   710,  1516,  1507,  1401,   126,   202,
    1114,  1204,   228,  1364,   607,   131,   126,  1039,   699,  1403,
    1012,  1337,   158,   131,  1010,   700,   158,  1344,   878,  1031,
     158,  1060,   949,   534,   248,   961,   988,   504,  1114,     0,
     214,   494,     0,   214,     0,     0,     0,     0,    11,   204,
       0,   534,     0,  1354,     0,     0,     0,     0,     0,   534,
     812,     0,     0,   118,   118,     0,     0,     0,   118,     0,
       0,  1371,     0,     0,   118,     0,    11,     0,     0,     0,
     126,  1125,     0,     0,   214,   126,     0,   131,     0,     0,
     204,   126,   131,     0,   158,   158,     0,     0,   131,   158,
    1400,     0,     0,   204,   204,   158,  1115,     0,     0,     0,
       0,  1116,     0,    55,    56,    57,   170,   171,   328,  1117,
       0,   202,     0,     0,     0,     0,     0,     0,     0,   202,
       0,  1471,     0,     0,  1115,   756,     0,     0,     0,  1116,
       0,    55,    56,    57,   170,   171,   328,  1117,     0,   204,
       0,   756,  1442,     0,     0,     0,  1118,  1119,     0,  1120,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   214,
       0,     0,     0,   330,   681,     0,     0,   278,     0,     0,
       0,    94,     0,     0,  1118,  1119,     0,  1120,     0,     0,
       0,     0,     0,     0,     0,   812,     0,     0,     0,   118,
       0,     0,     0,     0,  1230,    33,     0,   199,  1420,    94,
       0,   681,     0,  1337,  1337,     0,   126,  1344,  1344,     0,
       0,     0,     0,   131,     0,     0,     0,     0,     0,   278,
     158,     0,  1366,     0,     0,   118,   118,     0,     0,     0,
       0,   118,     0,     0,     0,     0,     0,     0,     0,     0,
     214,   214,   126,   126,     0,     0,     0,   214,   126,   131,
     131,     0,     0,     0,     0,   131,   158,   158,     0,     0,
       0,     0,   158,   202,   118,     0,     0,     0,     0,     0,
       0,  1470,     0,    81,    82,     0,    83,    84,    85,    31,
      32,   126,     0,     0,     0,     0,     0,  1483,   131,    37,
       0,     0,     0,     0,     0,   158,  1472,     0,     0,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   202,     0,     0,     0,     0,     0,
     619,     0,   113,     0,   249,   203,   203,     0,   118,   215,
       0,     0,     0,   118,     0,     0,    69,    70,    71,    72,
      73,     0,     0,     0,     0,   126,     0,   677,     0,   202,
     126,   202,   131,    76,    77,     0,     0,   131,     0,   158,
       0,     0,     0,     0,   158,     0,     0,    87,     0,     0,
       0,   202,   681,     0,   337,   338,   339,     0,     0,    33,
       0,     0,    92,   214,   214,   681,   681,   681,     0,     0,
     340,     0,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,     0,   362,     0,   214,     0,     0,     0,
       0,     0,     0,     0,     0,   202,     0,     0,     0,     0,
       0,     0,     0,     0,   214,     0,   202,   202,     0,  -790,
    -790,  -790,  -790,   354,   355,   356,   357,   358,   359,   360,
     361,   214,   362,   290,     0,     0,     0,    81,    82,     0,
      83,    84,    85,     0,   214,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   214,     0,     0,     0,     0,
       0,   681,     0,    95,   214,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,     0,     0,
       0,     0,     0,   214,   291,   337,   338,   339,   203,     0,
       0,     0,     0,     0,     0,     0,   249,   249,     0,     0,
       0,   340,   249,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,     0,   362,   202,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   214,
       0,   214,   734,     0,     0,     0,     0,     0,   203,     0,
       0,     0,     0,     0,   681,   203,   203,     0,   681,     0,
     681,     0,   203,     0,   337,   338,   339,     0,   203,     0,
       0,     0,     0,   249,     0,     0,   249,     0,   681,     0,
     340,   214,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   214,   362,     0,   214,   834,     0,     0,
       0,     0,     0,   202,   402,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   686,     0,     0,     0,     0,
       0,     0,     0,   215,     0,     0,     0,    33,     0,   199,
       0,     0,     0,     0,   202,     0,     0,     0,     0,     0,
     386,   387,     0,     0,     0,     0,     0,   202,   202,     0,
     681,     0,   686,   762,     0,     0,     0,     0,     0,     0,
     203,   214,     0,     0,   681,     0,   681,   200,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   835,
       0,     0,   249,     0,     0,     0,     0,   683,     0,     0,
     214,     0,     0,   202,     0,     0,     0,     0,   174,     0,
       0,    78,   388,    80,     0,    81,    82,     0,    83,    84,
      85,     0,     0,   685,   681,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   683,     0,     0,     0,     0,     0,
       0,    95,   797,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,     0,     0,     0,   201,
     685,     0,     0,     0,   113,     0,     0,     0,     0,     0,
       0,     0,     0,   249,   249,     0,     0,     0,   214,     0,
     249,     0,   681,     0,     0,     0,     0,   681,     0,   681,
       0,     0,     0,     0,   214,     0,     0,   214,   214,     0,
     214,     0,     0,     0,     0,     0,     0,   214,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   681,
       0,     0,   203,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   686,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    33,     0,   199,   686,   686,   686,     0,
       0,     0,     0,     0,     0,     0,     0,   681,     0,     0,
       0,     0,     0,   241,     0,     0,   214,   214,     0,     0,
       0,     0,     0,   203,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   242,
     214,     0,     0,     0,     0,     0,     0,     0,     0,   681,
       0,     0,     0,     0,     0,   683,     0,     0,   203,    33,
     203,     0,     0,     0,     0,     0,   249,   249,   683,   683,
     683,    81,    82,     0,    83,    84,    85,   681,   681,     0,
     203,   685,     0,   681,   214,     0,   449,     0,   214,     0,
       0,     0,   686,     0,   685,   685,   685,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   243,   244,     0,     0,     0,     0,   594,     0,
     113,     0,     0,     0,     0,   881,     0,     0,     0,     0,
     174,     0,     0,    78,   203,   245,     0,    81,    82,     0,
      83,    84,    85,   893,     0,   203,   203,   249,     0,     0,
       0,     0,     0,     0,     0,   246,     0,     0,   249,     0,
     893,     0,     0,    95,   683,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,     0,     0,
       0,   247,     0,     0,     0,   686,     0,   681,     0,   686,
     685,   686,     0,   926,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   686,
       0,     0,   215,     0,   214,     0,     0,     0,   681,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   681,
       0,     0,     0,     0,   681,     0,     0,     0,     0,     0,
       0,     0,   249,     0,   249,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   203,     0,   683,     0,     0,
       0,   683,     0,   683,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   683,   362,   685,   249,     0,     0,   685,     0,   685,
       0,   681,     0,     0,     0,     0,     0,     0,     0,   214,
       0,   686,     0,     0,     0,     0,   249,   685,     0,   249,
       0,     0,     0,     0,   214,   686,     0,   686,     0,     0,
       0,     0,     0,   214,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   214,
       0,     0,   203,     0,     0,     0,     0,   681,     0,     0,
       0,     0,     0,   681,     0,     0,     0,   681,     0,   681,
       0,     0,     0,     0,     0,   686,     0,     0,     0,     0,
       0,     0,     0,   683,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   203,   249,     0,     0,   683,     0,   683,
       0,     0,     0,     0,     0,     0,   203,   203,     0,   685,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   685,     0,   685,     0,     0,     0,     0,
       0,     0,     0,   686,     0,     0,     0,     0,   686,     0,
     686,     0,     0,     0,     0,     0,     0,   683,     0,  1113,
       0,     0,   203,     0,     0,     0,   337,   338,   339,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     686,     0,   340,   685,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,     0,   362,     0,     0,     0,
       0,   249,     0,     0,     0,   683,     0,     0,   686,     0,
     683,   241,   683,     0,     0,     0,     0,   249,     0,     0,
     249,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     249,   685,     0,     0,     0,     0,   685,   242,   685,     0,
       0,     0,   683,     0,     0,     0,     0,  1188,     0,  1196,
     686,     0,     0,     0,     0,     0,     0,    33,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   685,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   686,   686,
     683,     0,     0,     0,   686,     0,     0,     0,  1384,     0,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   685,   362,     0,     0,
     243,   244,     0,   249,     0,  1249,  1250,     0,     0,     0,
       0,     0,   683,     0,     0,     0,     0,     0,   174,     0,
       0,    78,     0,   245,    33,    81,    82,     0,    83,    84,
      85,     0,     0,     0,   800,     0,     0,     0,   685,     0,
     683,   683,     0,   246,     0,     0,   683,     0,     0,     0,
       0,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   685,   685,     0,   247,
       0,     0,   685,  1381,     0,     0,     0,  1196,   686,   178,
     180,     0,   182,   183,   184,     0,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   262,     0,
     208,   211,    81,    82,     0,    83,    84,    85,     0,   686,
       0,     0,   229,     0,     0,     0,     0,     0,     0,   237,
     686,   240,     0,   687,   256,   686,   261,     0,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,     0,     0,     0,   791,     0,  1457,     0,
     683,     0,     0,   295,    33,     0,   199,     0,     0,     0,
     711,     0,     0,     0,   682,     0,     0,   302,     0,     0,
       0,     0,     0,     0,     0,     0,   685,  1421,     0,     0,
       0,   683,   686,     0,   305,     0,     0,     0,     0,     0,
       0,     0,   683,     0,   200,     0,     0,   683,     0,     0,
       0,   682,     0,     0,     0,     0,     0,   685,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   685,     0,
       0,     0,     0,   685,     0,   174,     0,     0,    78,     0,
      80,     0,    81,    82,     0,    83,    84,    85,   686,     0,
       0,     0,     0,     0,   686,     0,     0,     0,   686,     0,
     686,     0,     0,     0,   683,   403,     0,     0,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,     0,     0,     0,   201,     0,     0,     0,
     685,   113,     0,     0,     0,     0,   249,     0,  1480,     0,
     945,   946,    33,     0,     0,     0,   427,     0,     0,   427,
       0,     0,   249,  1188,     0,     0,   229,   438,     0,     0,
     683,     0,     0,     0,     0,     0,   683,     0,     0,     0,
     683,     0,   683,     0,     0,     0,     0,     0,     0,     0,
       0,   838,     0,     0,     0,     0,   685,     0,     0,     0,
       0,     0,   685,     0,   857,   858,   685,     0,   685,     0,
     305,     0,     0,     0,     0,     0,   208,     0,     0,     0,
     510,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      81,    82,   682,    83,    84,    85,     0,     0,     0,     0,
       0,     0,     0,   545,     0,   682,   682,   682,     0,     0,
       0,     0,     0,     0,   554,     0,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   560,   561,   562,   564,   565,   566,   567,   568,   569,
     570,   571,   572,   573,   574,   575,   576,   577,   578,   579,
     580,   581,   582,   583,   584,   585,     0,     0,   587,   587,
     923,   590,     0,     0,     0,     0,     0,     0,   606,   608,
     609,   610,   611,   612,   613,   614,   615,   616,   617,   618,
       0,     0,     0,     0,     0,   587,   623,     0,   554,   587,
     626,     0,     0,     0,     0,     0,   606,     0,     0,     0,
       0,   682,     0,     0,     0,     0,   638,     0,   640,     0,
       0,   337,   338,   339,   554,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   652,     0,   653,   340,     0,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
       0,   362,     0,   691,     0,     0,   694,   697,   698,  1004,
       0,     0,     0,     0,    33,     0,   199,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1015,     0,   709,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   682,     0,     0,     0,   682,     0,
     682,   337,   338,   339,     0,     0,     0,     0,     0,     0,
     593,     0,     0,     0,   738,     0,     0,   340,   682,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
       0,   362,    81,    82,     0,    83,    84,    85,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   776,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    95,  1085,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,  1097,     0,  1098,     0,     0,   784,   594,
       0,   113,     0,     0,     0,     0,     0,   295,     0,   885,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     682,     0,     0,   793,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   682,     0,   682,     0,     0,     0,
       0,     0,     0,  1138,     0,   337,   338,   339,     0,   825,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   340,   229,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,   682,   362,     0,     0,     0,     0,
       0,     0,     0,   869,   337,   338,   339,     0,     0,   933,
       0,  1176,     0,     0,     0,     0,  1178,     0,  1179,     0,
     340,     0,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,     0,   362,     0,     0,   900,  1218,     0,
       0,     0,   682,     0,     0,     0,     0,   682,     0,   682,
     907,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   922,   362,     0,     0,     0,  1244,     0,     0,   682,
       0,   930,     0,     0,   931,     0,   932,     0,     0,     0,
     554,     0,     0,     0,     0,     0,     0,     0,     0,   554,
       0,     0,     0,     0,     0,     0,     0,     0,   241,     0,
       0,     0,     0,     0,     0,     0,     0,   682,  1356,     0,
       0,     0,     0,     0,   963,     0,     0,     0,     0,     0,
       0,     0,     0,   941,   242,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1374,  1375,     0,     0,
       0,     0,  1380,     0,    33,     0,     0,     0,     0,   682,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1007,  1008,  1009,     0,     0,     0,
     694,  1011,   964,     0,     0,     0,     0,   682,   682,     0,
       0,     0,     0,   682,     0,     0,     0,     0,     0,     0,
       0,     0,  1024,     0,     0,     0,     0,   243,   244,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1043,     0,     0,     0,   174,     0,     0,    78,     0,
     245,     0,    81,    82,     0,    83,    84,    85,   554,     0,
       0,   853,     0,     0,     0,     0,   554,     0,  1024,     0,
     246,     0,     0,     0,     0,     0,  1404,     0,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,     0,     0,     0,   247,   229,     0,     0,
       0,     0,     0,     0,     0,     0,  1089,  1427,     0,   337,
     338,   339,     0,     0,     0,     0,     0,   682,  1437,     0,
       0,     0,     0,  1441,     0,   340,  1090,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   682,   362,
       0,     0,     0,     0,  1132,     0,     0,     0,  1133,   682,
    1134,     0,     0,     0,   682,     0,     0,     0,     0,   554,
       0,     0,     0,   337,   338,   339,  1147,   554,     0,     0,
    1473,     0,     0,     0,     0,   554,     0,     0,     0,   340,
       0,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,     0,   362,     0,     0,     0,     0,     0,     0,
       0,   682,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1180,     0,     0,  1522,     0,     0,     0,
       0,     0,  1526,     0,     0,     0,  1528,     0,  1529,     0,
       0,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,   554,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   682,     0,     0,
       0,     0,     0,   682,     0,     0,     0,   682,     0,   682,
      11,    12,    13,     0,     0,  1091,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,   554,    45,    46,    47,    48,    49,    50,
      51,     0,    52,    53,    54,    55,    56,    57,    58,    59,
      60,  1139,    61,    62,    63,    64,    65,    66,     0,     0,
       0,  1376,    67,    68,     0,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,    75,    76,    77,    78,    79,    80,     0,    81,    82,
       0,    83,    84,    85,    86,     0,    87,     0,     0,     0,
      88,     0,     0,     0,     0,     0,    89,    90,     0,    91,
       0,    92,    93,    94,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,   110,     0,   111,   112,   901,   113,   114,   339,
     115,   116,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,   340,     0,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,     0,   362,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,     0,    41,     0,    42,     0,    43,     0,
       0,    44,     0,     0,     0,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,    55,    56,    57,    58,
      59,    60,     0,    61,    62,    63,    64,    65,    66,     0,
       0,     0,     0,    67,    68,     0,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,    75,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,    90,     0,
      91,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,  1026,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,   340,    10,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,     0,   362,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
      49,    50,    51,     0,    52,    53,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,    64,    65,    66,
       0,     0,     0,     0,    67,    68,     0,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,    74,     0,
       0,     0,     0,    75,    76,    77,    78,    79,    80,     0,
      81,    82,     0,    83,    84,    85,    86,     0,    87,     0,
       0,     0,    88,     0,     0,     0,     0,     0,    89,    90,
       0,    91,     0,    92,    93,    94,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,   110,     0,   111,   112,     0,   113,
     114,     0,   115,   116,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,     0,   362,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,     0,    41,     0,    42,     0,
      43,     0,     0,    44,     0,     0,     0,    45,    46,    47,
      48,     0,    50,    51,     0,    52,     0,    54,    55,    56,
      57,    58,    59,    60,     0,    61,    62,    63,     0,    65,
      66,     0,     0,     0,     0,    67,    68,     0,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   174,    76,    77,    78,    79,    80,
       0,    81,    82,     0,    83,    84,    85,    86,     0,    87,
       0,     0,     0,    88,     0,     0,     0,     0,     0,    89,
       0,     0,     0,     0,    92,    93,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,   110,     0,   111,   112,   539,
     113,   114,     0,   115,   116,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,     0,   362,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,     0,    50,    51,     0,    52,     0,    54,    55,
      56,    57,    58,    59,    60,     0,    61,    62,    63,     0,
      65,    66,     0,     0,     0,     0,    67,    68,     0,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
      74,     0,     0,     0,     0,   174,    76,    77,    78,    79,
      80,     0,    81,    82,     0,    83,    84,    85,    86,     0,
      87,     0,     0,     0,    88,     0,     0,     0,     0,     0,
      89,     0,     0,     0,     0,    92,    93,    94,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,   110,     0,   111,   112,
     880,   113,   114,     0,   115,   116,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,     0,   362,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,     0,    36,
       0,     0,     0,    37,    38,    39,    40,     0,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,     0,    50,    51,     0,    52,     0,    54,
      55,    56,    57,    58,    59,    60,     0,    61,    62,    63,
       0,    65,    66,     0,     0,     0,     0,    67,    68,     0,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   174,    76,    77,    78,
      79,    80,     0,    81,    82,     0,    83,    84,    85,    86,
       0,    87,     0,     0,     0,    88,     0,     0,     0,     0,
       0,    89,     0,     0,     0,     0,    92,    93,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,   110,     0,   111,
     112,   970,   113,   114,     0,   115,   116,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,  -790,  -790,
    -790,  -790,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,     0,   362,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,     0,     0,     0,    37,    38,    39,    40,   972,    41,
       0,    42,     0,    43,     0,     0,    44,     0,     0,     0,
      45,    46,    47,    48,     0,    50,    51,     0,    52,     0,
      54,    55,    56,    57,    58,    59,    60,     0,    61,    62,
      63,     0,    65,    66,     0,     0,     0,     0,    67,    68,
       0,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,    74,     0,     0,     0,     0,   174,    76,    77,
      78,    79,    80,     0,    81,    82,     0,    83,    84,    85,
      86,     0,    87,     0,     0,     0,    88,     0,     0,     0,
       0,     0,    89,     0,     0,     0,     0,    92,    93,    94,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,   110,     0,
     111,   112,     0,   113,   114,     0,   115,   116,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
       0,    36,     0,     0,     0,    37,    38,    39,    40,     0,
      41,     0,    42,     0,    43,  1086,     0,    44,     0,     0,
       0,    45,    46,    47,    48,     0,    50,    51,     0,    52,
       0,    54,    55,    56,    57,    58,    59,    60,     0,    61,
      62,    63,     0,    65,    66,     0,     0,     0,     0,    67,
      68,     0,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   174,    76,
      77,    78,    79,    80,     0,    81,    82,     0,    83,    84,
      85,    86,     0,    87,     0,     0,     0,    88,     0,     0,
       0,     0,     0,    89,     0,     0,     0,     0,    92,    93,
      94,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,   110,
       0,   111,   112,     0,   113,   114,     0,   115,   116,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,     0,    43,     0,     0,    44,     0,
       0,     0,    45,    46,    47,    48,     0,    50,    51,     0,
      52,     0,    54,    55,    56,    57,    58,    59,    60,     0,
      61,    62,    63,     0,    65,    66,     0,     0,     0,     0,
      67,    68,     0,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,    74,     0,     0,     0,     0,   174,
      76,    77,    78,    79,    80,     0,    81,    82,     0,    83,
      84,    85,    86,     0,    87,     0,     0,     0,    88,     0,
       0,     0,     0,     0,    89,     0,     0,     0,     0,    92,
      93,    94,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
     110,     0,   111,   112,  1182,   113,   114,     0,   115,   116,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,     0,    41,     0,    42,     0,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,     0,    50,    51,
       0,    52,     0,    54,    55,    56,    57,    58,    59,    60,
       0,    61,    62,    63,     0,    65,    66,     0,     0,     0,
       0,    67,    68,     0,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     174,    76,    77,    78,    79,    80,     0,    81,    82,     0,
      83,    84,    85,    86,     0,    87,     0,     0,     0,    88,
       0,     0,     0,     0,     0,    89,     0,     0,     0,     0,
      92,    93,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,   110,     0,   111,   112,  1377,   113,   114,     0,   115,
     116,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,  1416,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,     0,    50,
      51,     0,    52,     0,    54,    55,    56,    57,    58,    59,
      60,     0,    61,    62,    63,     0,    65,    66,     0,     0,
       0,     0,    67,    68,     0,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,   174,    76,    77,    78,    79,    80,     0,    81,    82,
       0,    83,    84,    85,    86,     0,    87,     0,     0,     0,
      88,     0,     0,     0,     0,     0,    89,     0,     0,     0,
       0,    92,    93,    94,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,   110,     0,   111,   112,     0,   113,   114,     0,
     115,   116,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,     0,    41,     0,    42,     0,    43,     0,
       0,    44,     0,     0,     0,    45,    46,    47,    48,     0,
      50,    51,     0,    52,     0,    54,    55,    56,    57,    58,
      59,    60,     0,    61,    62,    63,     0,    65,    66,     0,
       0,     0,     0,    67,    68,     0,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,  1446,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,     0,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,    74,     0,
       0,     0,     0,   174,    76,    77,    78,    79,    80,     0,
      81,    82,     0,    83,    84,    85,    86,     0,    87,     0,
       0,     0,    88,     0,     0,     0,     0,     0,    89,     0,
       0,     0,     0,    92,    93,    94,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,   110,     0,   111,   112,  1447,   113,
     114,     0,   115,   116,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,     0,    41,  1450,    42,     0,
      43,     0,     0,    44,     0,     0,     0,    45,    46,    47,
      48,     0,    50,    51,     0,    52,     0,    54,    55,    56,
      57,    58,    59,    60,     0,    61,    62,    63,     0,    65,
      66,     0,     0,     0,     0,    67,    68,     0,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   174,    76,    77,    78,    79,    80,
       0,    81,    82,     0,    83,    84,    85,    86,     0,    87,
       0,     0,     0,    88,     0,     0,     0,     0,     0,    89,
       0,     0,     0,     0,    92,    93,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,   110,     0,   111,   112,     0,
     113,   114,     0,   115,   116,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,     0,    50,    51,     0,    52,     0,    54,    55,
      56,    57,    58,    59,    60,     0,    61,    62,    63,     0,
      65,    66,     0,     0,     0,     0,    67,    68,     0,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
      74,     0,     0,     0,     0,   174,    76,    77,    78,    79,
      80,     0,    81,    82,     0,    83,    84,    85,    86,     0,
      87,     0,     0,     0,    88,     0,     0,     0,     0,     0,
      89,     0,     0,     0,     0,    92,    93,    94,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,   110,     0,   111,   112,
    1465,   113,   114,     0,   115,   116,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,     0,    36,
       0,     0,     0,    37,    38,    39,    40,     0,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,     0,    50,    51,     0,    52,     0,    54,
      55,    56,    57,    58,    59,    60,     0,    61,    62,    63,
       0,    65,    66,     0,     0,     0,     0,    67,    68,     0,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   174,    76,    77,    78,
      79,    80,     0,    81,    82,     0,    83,    84,    85,    86,
       0,    87,     0,     0,     0,    88,     0,     0,     0,     0,
       0,    89,     0,     0,     0,     0,    92,    93,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,   110,     0,   111,
     112,  1518,   113,   114,     0,   115,   116,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,     0,     0,     0,    37,    38,    39,    40,     0,    41,
       0,    42,     0,    43,     0,     0,    44,     0,     0,     0,
      45,    46,    47,    48,     0,    50,    51,     0,    52,     0,
      54,    55,    56,    57,    58,    59,    60,     0,    61,    62,
      63,     0,    65,    66,     0,     0,     0,     0,    67,    68,
       0,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,    74,     0,     0,     0,     0,   174,    76,    77,
      78,    79,    80,     0,    81,    82,     0,    83,    84,    85,
      86,     0,    87,     0,     0,     0,    88,     0,     0,     0,
       0,     0,    89,     0,     0,     0,     0,    92,    93,    94,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,   110,     0,
     111,   112,  1523,   113,   114,     0,   115,   116,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
       0,    36,     0,     0,     0,    37,    38,    39,    40,     0,
      41,     0,    42,     0,    43,     0,     0,    44,     0,     0,
       0,    45,    46,    47,    48,     0,    50,    51,     0,    52,
       0,    54,    55,    56,    57,    58,    59,    60,     0,    61,
      62,    63,     0,    65,    66,     0,     0,     0,     0,    67,
      68,     0,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   174,    76,
      77,    78,    79,    80,     0,    81,    82,     0,    83,    84,
      85,    86,     0,    87,     0,     0,     0,    88,     0,     0,
       0,     0,     0,    89,     0,     0,     0,     0,    92,    93,
      94,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,   110,
       0,   111,   112,     0,   113,   114,     0,   115,   116,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   428,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,     0,    43,     0,     0,    44,     0,
       0,     0,    45,    46,    47,    48,     0,    50,    51,     0,
      52,     0,    54,    55,    56,    57,   170,   171,    60,     0,
      61,    62,    63,     0,     0,     0,     0,     0,     0,     0,
      67,    68,     0,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,    74,     0,     0,     0,     0,   174,
      76,    77,    78,    79,    80,     0,    81,    82,     0,    83,
      84,    85,     0,     0,    87,     0,     0,     0,    88,     0,
       0,     0,     0,     0,    89,     0,     0,     0,     0,    92,
      93,    94,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
     110,     0,   111,   112,     0,   113,   114,     0,   115,   116,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   655,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,     0,    41,     0,    42,     0,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,     0,    50,    51,
       0,    52,     0,    54,    55,    56,    57,   170,   171,    60,
       0,    61,    62,    63,     0,     0,     0,     0,     0,     0,
       0,    67,    68,     0,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     174,    76,    77,    78,    79,    80,     0,    81,    82,     0,
      83,    84,    85,     0,     0,    87,     0,     0,     0,    88,
       0,     0,     0,     0,     0,    89,     0,     0,     0,     0,
      92,    93,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,   110,     0,   111,   112,     0,   113,   114,     0,   115,
     116,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   840,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,     0,    50,
      51,     0,    52,     0,    54,    55,    56,    57,   170,   171,
      60,     0,    61,    62,    63,     0,     0,     0,     0,     0,
       0,     0,    67,    68,     0,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,   174,    76,    77,    78,    79,    80,     0,    81,    82,
       0,    83,    84,    85,     0,     0,    87,     0,     0,     0,
      88,     0,     0,     0,     0,     0,    89,     0,     0,     0,
       0,    92,    93,    94,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,   110,     0,   111,   112,     0,   113,   114,     0,
     115,   116,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1239,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,     0,    41,     0,    42,     0,    43,     0,
       0,    44,     0,     0,     0,    45,    46,    47,    48,     0,
      50,    51,     0,    52,     0,    54,    55,    56,    57,   170,
     171,    60,     0,    61,    62,    63,     0,     0,     0,     0,
       0,     0,     0,    67,    68,     0,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,     0,   111,   112,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1370,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
     170,   171,    60,     0,    61,    62,    63,     0,     0,     0,
       0,     0,     0,     0,    67,    68,     0,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,    74,     0,
       0,     0,     0,   174,    76,    77,    78,    79,    80,     0,
      81,    82,     0,    83,    84,    85,     0,     0,    87,     0,
       0,     0,    88,     0,     0,     0,     0,     0,    89,     0,
       0,     0,     0,    92,    93,    94,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,   110,     0,   111,   112,     0,   113,
     114,     0,   115,   116,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,     0,    41,     0,    42,     0,
      43,     0,     0,    44,     0,     0,     0,    45,    46,    47,
      48,     0,    50,    51,     0,    52,     0,    54,    55,    56,
      57,   170,   171,    60,     0,    61,    62,    63,     0,     0,
       0,     0,     0,     0,     0,    67,    68,     0,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   174,    76,    77,    78,    79,    80,
       0,    81,    82,     0,    83,    84,    85,     0,     0,    87,
       0,     0,     0,    88,     0,     0,     0,     0,     0,    89,
       0,     0,     0,     0,    92,    93,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,   110,     0,   111,   112,     0,
     113,   114,     0,   115,   116,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   602,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,    34,    35,     0,     0,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    55,
      56,    57,   170,   171,   172,     0,    33,    62,    63,     0,
       0,     0,     0,     0,     0,     0,   173,    68,     0,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
      74,     0,     0,     0,     0,   174,    76,    77,    78,   603,
      80,     0,    81,    82,     0,    83,    84,    85,     0,     0,
      87,     0,     0,     0,    88,     0,     0,     0,     0,     0,
      89,     0,     0,     0,     0,    92,    93,    94,    95,   253,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,    81,    82,   110,    83,    84,    85,
       0,   113,   114,     0,   115,   116,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,     0,     0,     0,   556,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,    33,    34,    35,     0,     0,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      55,    56,    57,   170,   171,   172,     0,     0,    62,    63,
       0,     0,     0,     0,     0,     0,     0,   173,    68,     0,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   174,    76,    77,    78,
       0,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,    87,     0,     0,     0,    88,     0,     0,     0,     0,
       0,    89,     0,     0,     0,     0,    92,    93,    94,    95,
     253,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,   110,     0,   254,
       0,     0,   113,   114,     0,   115,   116,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,    35,     0,
       0,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    55,    56,    57,   170,   171,   172,     0,    33,    62,
      63,     0,     0,     0,     0,     0,     0,     0,   173,    68,
       0,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,    74,     0,     0,     0,     0,   174,    76,    77,
      78,   603,    80,     0,    81,    82,     0,    83,    84,    85,
       0,     0,    87,     0,     0,     0,    88,     0,     0,     0,
       0,     0,    89,     0,     0,     0,     0,    92,    93,    94,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,    81,    82,   110,    83,
      84,    85,     0,   113,   114,     0,   115,   116,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   207,     0,     0,
     774,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
       0,     0,     0,     0,     0,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    55,    56,    57,   170,   171,   172,     0,    33,
      62,    63,     0,     0,     0,     0,     0,     0,     0,   173,
      68,     0,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   174,    76,
      77,    78,     0,    80,     0,    81,    82,     0,    83,    84,
      85,     0,     0,    87,     0,  1002,     0,    88,     0,     0,
       0,     0,     0,    89,     0,     0,     0,     0,    92,     0,
      94,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,    81,    82,   110,
      83,    84,    85,     0,   113,   114,     0,   115,   116,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,     0,     0,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    55,    56,    57,   170,   171,   172,     0,
       0,    62,    63,     0,     0,     0,     0,     0,     0,     0,
     173,    68,     0,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,    74,     0,     0,     0,     0,   174,
      76,    77,    78,     0,    80,     0,    81,    82,     0,    83,
      84,    85,     0,     0,    87,     0,     0,     0,    88,     0,
       0,     0,     0,     0,    89,     0,     0,     0,     0,    92,
       0,    94,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
     110,     0,   236,     0,     0,   113,   114,     0,   115,   116,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,    35,     0,     0,     0,     0,     0,    37,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    55,    56,    57,   170,   171,   172,
       0,     0,    62,    63,     0,     0,     0,     0,     0,     0,
       0,   173,    68,     0,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     174,    76,    77,    78,     0,    80,     0,    81,    82,     0,
      83,    84,    85,     0,     0,    87,     0,     0,     0,    88,
       0,     0,     0,     0,     0,    89,     0,     0,     0,     0,
      92,     0,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,   110,     0,   239,     0,     0,   113,   114,     0,   115,
     116,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   294,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    55,    56,    57,   170,   171,
     172,     0,    33,    62,    63,     0,     0,     0,     0,     0,
       0,     0,   173,    68,     0,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,   174,    76,    77,    78,     0,    80,     0,    81,    82,
       0,    83,    84,    85,     0,     0,    87,     0,     0,     0,
      88,     0,     0,     0,     0,     0,    89,     0,     0,     0,
       0,    92,     0,    94,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
      81,    82,   110,    83,    84,    85,     0,   113,   114,     0,
     115,   116,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,    35,     0,     0,     0,     0,     0,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    55,    56,    57,   170,
     171,   172,     0,     0,    62,    63,     0,     0,     0,     0,
       0,     0,     0,   173,    68,     0,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,   426,     0,     0,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   551,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     170,   171,   172,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   173,    68,     0,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,    74,     0,
       0,     0,     0,   174,    76,    77,    78,     0,    80,     0,
      81,    82,     0,    83,    84,    85,     0,     0,    87,     0,
       0,     0,    88,     0,     0,     0,     0,     0,    89,     0,
       0,     0,     0,    92,     0,    94,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,   110,     0,     0,     0,     0,   113,
     114,     0,   115,   116,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   563,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,    35,     0,     0,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    55,    56,
      57,   170,   171,   172,     0,     0,    62,    63,     0,     0,
       0,     0,     0,     0,     0,   173,    68,     0,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   174,    76,    77,    78,     0,    80,
       0,    81,    82,     0,    83,    84,    85,     0,     0,    87,
       0,     0,     0,    88,     0,     0,     0,     0,     0,    89,
       0,     0,     0,     0,    92,     0,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,   110,     0,     0,     0,     0,
     113,   114,     0,   115,   116,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   602,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,    34,    35,     0,     0,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    55,
      56,    57,   170,   171,   172,     0,     0,    62,    63,     0,
       0,     0,     0,     0,     0,     0,   173,    68,     0,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
      74,     0,     0,     0,     0,   174,    76,    77,    78,     0,
      80,     0,    81,    82,     0,    83,    84,    85,     0,     0,
      87,     0,     0,     0,    88,     0,     0,     0,     0,     0,
      89,     0,     0,     0,     0,    92,     0,    94,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,   110,     0,     0,     0,
       0,   113,   114,     0,   115,   116,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   637,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,    33,    34,    35,     0,     0,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      55,    56,    57,   170,   171,   172,     0,     0,    62,    63,
       0,     0,     0,     0,     0,     0,     0,   173,    68,     0,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   174,    76,    77,    78,
       0,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,    87,     0,     0,     0,    88,     0,     0,     0,     0,
       0,    89,     0,     0,     0,     0,    92,     0,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,   110,     0,     0,
       0,     0,   113,   114,     0,   115,   116,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   639,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,    35,     0,
       0,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    55,    56,    57,   170,   171,   172,     0,     0,    62,
      63,     0,     0,     0,     0,     0,     0,     0,   173,    68,
       0,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,    74,     0,     0,     0,     0,   174,    76,    77,
      78,     0,    80,     0,    81,    82,     0,    83,    84,    85,
       0,     0,    87,     0,     0,     0,    88,     0,     0,     0,
       0,     0,    89,     0,     0,     0,     0,    92,     0,    94,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,   110,     0,
       0,     0,     0,   113,   114,     0,   115,   116,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
       0,     0,     0,     0,     0,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    55,    56,    57,   170,   171,   172,     0,     0,
      62,    63,     0,     0,     0,     0,     0,     0,     0,   173,
      68,     0,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   174,    76,
      77,    78,     0,    80,     0,    81,    82,     0,    83,    84,
      85,     0,     0,    87,     0,     0,     0,    88,     0,     0,
       0,     0,     0,    89,     0,     0,     0,     0,    92,     0,
      94,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,   110,
       0,     0,   651,     0,   113,   114,     0,   115,   116,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   921,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,     0,     0,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    55,    56,    57,   170,   171,   172,     0,
       0,    62,    63,     0,     0,     0,     0,     0,     0,     0,
     173,    68,     0,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,    74,     0,     0,     0,     0,   174,
      76,    77,    78,     0,    80,     0,    81,    82,     0,    83,
      84,    85,     0,     0,    87,     0,     0,     0,    88,     0,
       0,     0,     0,     0,    89,     0,     0,     0,     0,    92,
       0,    94,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
     110,     0,     0,     0,     0,   113,   114,     0,   115,   116,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   962,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,    35,     0,     0,     0,     0,     0,    37,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    55,    56,    57,   170,   171,   172,
       0,     0,    62,    63,     0,     0,     0,     0,     0,     0,
       0,   173,    68,     0,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     174,    76,    77,    78,     0,    80,     0,    81,    82,     0,
      83,    84,    85,     0,     0,    87,     0,     0,     0,    88,
       0,     0,     0,     0,     0,    89,     0,     0,     0,     0,
      92,     0,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,   110,     0,     0,     0,     0,   113,   114,     0,   115,
     116,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    55,    56,    57,   170,   171,
     172,     0,     0,    62,    63,     0,     0,     0,     0,     0,
       0,     0,   173,    68,     0,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,   174,    76,    77,    78,     0,    80,     0,    81,    82,
       0,    83,    84,    85,     0,     0,    87,     0,     0,     0,
      88,     0,     0,     0,     0,     0,    89,     0,     0,     0,
       0,    92,     0,    94,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,   110,     0,     0,     0,     0,   113,   114,     0,
     115,   116,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,   509,    35,     0,     0,     0,     0,     0,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    55,    56,    57,   170,
     171,   172,     0,     0,    62,    63,     0,     0,     0,     0,
       0,     0,     0,   173,    68,     0,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,   110,   337,   338,   339,     0,   113,   114,
       0,   115,   116,     0,     0,     0,     0,     0,     0,     0,
     340,     0,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,     0,   362,   337,   338,   339,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   340,     0,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,     0,   362,     0,     0,     0,   337,
     338,   339,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   340,     0,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,     0,   362,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   337,   338,   339,     0,     0,  1258,  1259,  1260,  1261,
    1262,     0,     0,  1263,  1264,  1265,  1266,   340,     0,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
       0,   362,  1214,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1267,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1268,  1269,  1270,  1271,
    1272,  1273,  1274,     0,     0,    33,     0,     0,     0,     0,
       0,     0,     0,  1215,  1275,  1276,  1277,  1278,  1279,  1280,
    1281,  1282,  1283,  1284,  1285,  1286,  1287,  1288,  1289,  1290,
    1291,  1292,  1293,  1294,  1295,  1296,  1297,  1298,  1299,  1300,
    1301,  1302,  1303,  1304,  1305,  1306,  1307,  1308,  1309,  1310,
    1311,  1312,  1313,  1314,  1315,   363,     0,  1316,  1317,     0,
    1318,  1319,  1320,  1321,  1322,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1323,  1324,  1325,     0,
    1326,     0,     0,    81,    82,     0,    83,    84,    85,  1327,
    1328,  1329,     0,     0,  1330,     0,     0,     0,     0,     0,
       0,  1331,  1332,  1247,  1333,     0,  1334,  1335,  1336,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   337,   338,   339,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     340,     0,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,     0,   362,   337,   338,   339,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   340,     0,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,     0,   362,   337,   338,   339,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   340,     0,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,     0,   362,   337,   338,   339,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   340,     0,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,     0,   362,     0,     0,
       0,     0,     0,   337,   338,   339,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   340,
     442,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,     0,   362,   337,   338,   339,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     340,   444,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,     0,   362,   337,   338,   339,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   340,   456,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,     0,   362,   337,   338,   339,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   340,   480,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,     0,   362,     0,     0,     0,
       0,   337,   338,   339,     0,     0,     0,    33,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   340,   629,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
       0,   362,   337,   338,   339,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   340,   648,
     341,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,   241,   362,  1342,     0,    81,    82,  1343,    83,    84,
      85,     0,     0,     0,     0,   877,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   242,     0,     0,
       0,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,     0,    33,     0,  1201,
       0,     0,   873,   874,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   241,     0,     0,
       0,     0,     0,     0,  -288,     0,     0,     0,     0,     0,
       0,     0,    55,    56,    57,   170,   171,   328,     0,     0,
       0,     0,     0,   242,  1417,     0,     0,     0,     0,     0,
     243,   244,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    33,     0,     0,     0,     0,   174,     0,
       0,    78,     0,   245,     0,    81,    82,     0,    83,    84,
      85,     0,     0,  1109,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   246,     0,     0,     0,     0,     0,     0,
      94,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   243,   244,     0,   247,
       0,     0,     0,     0,   673,   674,     0,     0,     0,     0,
       0,     0,     0,     0,   174,     0,     0,    78,     0,   245,
       0,    81,    82,   675,    83,    84,    85,     0,     0,     0,
    1095,    31,    32,    33,     0,     0,     0,     0,     0,   246,
       0,    37,     0,     0,     0,     0,     0,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,     0,     0,     0,   247,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   676,     0,    69,    70,
      71,    72,    73,     0,   802,   803,     0,     0,     0,   677,
       0,     0,     0,     0,   174,    76,    77,    78,     0,   678,
       0,    81,    82,   804,    83,    84,    85,     0,     0,    87,
       0,   805,   806,    33,     0,     0,     0,     0,     0,   679,
       0,   807,     0,     0,    92,     0,     0,   680,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    27,    28,     0,     0,     0,     0,     0,     0,
       0,    33,     0,   199,     0,     0,   808,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   809,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    81,    82,     0,    83,    84,    85,     0,     0,     0,
       0,   200,     0,     0,     0,     0,     0,     0,     0,   810,
       0,     0,     0,    33,     0,   199,     0,   811,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   174,     0,     0,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,     0,     0,     0,
       0,    88,     0,   200,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
      33,     0,     0,   407,   174,     0,     0,    78,   113,    80,
       0,    81,    82,     0,    83,    84,    85,    33,     0,   199,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,     0,     0,     0,   201,     0,   200,   485,     0,
     113,     0,     0,     0,     0,     0,     0,     0,     0,   501,
       0,   174,     0,     0,    78,     0,    80,     0,    81,    82,
       0,    83,    84,    85,     0,     0,     0,     0,   174,     0,
       0,    78,     0,    80,     0,    81,    82,     0,    83,    84,
      85,    33,     0,   199,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,     0,
       0,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,     0,     0,     0,   201,
       0,   200,     0,     0,   113,     0,     0,     0,     0,     0,
       0,     0,     0,   896,     0,    33,     0,   199,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   174,     0,     0,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   200,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
      33,     0,   199,   201,     0,     0,   174,     0,   113,    78,
       0,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    95,
     212,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,    33,     0,   199,   201,     0,     0,
       0,     0,   113,     0,     0,     0,     0,     0,     0,     0,
       0,   174,     0,     0,    78,     0,    80,     0,    81,    82,
       0,    83,    84,    85,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    33,     0,   199,     0,     0,
       0,     0,     0,     0,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,     0,
       0,     0,   213,     0,     0,     0,     0,   113,     0,     0,
       0,     0,    81,    82,     0,    83,    84,    85,     0,     0,
       0,     0,     0,     0,     0,    33,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,    81,    82,     0,    83,    84,    85,   645,
       0,   113,     0,     0,     0,     0,    33,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,     0,     0,   174,     0,     0,    78,
     917,     0,   113,    81,    82,     0,    83,    84,    85,     0,
    1200,     0,     0,     0,     0,    33,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,    81,    82,     0,    83,    84,    85,
       0,  1419,     0,     0,     0,     0,  1189,    33,     0,   728,
     729,     0,     0,     0,     0,     0,     0,     0,  1190,     0,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,     0,   174,     0,  1201,    78,
       0,  1191,     0,    81,    82,     0,    83,  1192,    85,     0,
      33,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,     0,    81,    82,     0,    83,    84,
      85,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   325,     0,    81,    82,
       0,    83,    84,    85,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   337,
     338,   339,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   340,     0,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,     0,   362,
     337,   338,   339,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   340,     0,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,     0,
     362,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   337,   338,   339,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   340,   412,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,     0,   362,   337,   338,   339,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   340,   520,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,     0,   362,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   337,   338,   339,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   340,   780,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
       0,   362,   337,   338,   339,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   340,   820,
     341,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,     0,   362,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   337,   338,
     339,     0,     0,     0,  1052,     0,     0,     0,     0,     0,
       0,     0,     0,   659,   340,   777,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,     0,   362,   337,
     338,   339,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   340,     0,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,     0,   362,
     338,   339,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   340,     0,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,     0,   362
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-1015))

#define yytable_value_is_error(yytable_value) \
  ((yytable_value) == (-790))

static const yytype_int16 yycheck[] =
{
       4,   156,   131,     4,     4,    86,     4,     4,    30,    90,
      91,   176,   321,   553,    50,   214,    28,     4,    40,   630,
     399,   394,    44,   754,     4,   914,   374,   159,    53,   166,
     362,     4,   747,  1047,   770,   220,   428,   318,     9,   220,
     664,   516,    46,   658,   110,    49,   303,   420,   177,   130,
      75,    24,    25,    78,   110,   241,   242,   122,     9,   773,
       9,   247,    66,   904,     9,     9,    27,     9,     9,     9,
      42,     9,    27,     9,   788,    74,    10,    11,    12,     9,
      79,     9,    86,     9,     9,   477,    90,    91,     9,   221,
       9,   406,    26,     9,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,   122,    50,    74,     9,     9,
       9,     9,     9,     9,     9,     9,   130,   122,    74,  1143,
       9,  1145,   318,   198,     0,   201,    79,   110,   149,    62,
      30,   140,   141,   149,   131,    62,    79,   213,   194,   103,
     865,    62,    62,    62,   365,   194,   162,   142,    62,    91,
     177,   178,   179,    62,   329,    62,    42,   184,   185,   862,
     194,   188,   189,   866,    42,    42,   156,   953,    42,   164,
     391,   957,   186,     8,   395,   142,   197,   159,   195,   925,
     177,   197,   928,    74,   140,   141,   139,    62,    47,   122,
     195,    62,    62,   202,    62,    62,   139,   161,  1049,    62,
     276,   526,   197,  1227,   146,  1056,  1057,    62,   122,   843,
     276,   845,   226,   195,   938,   142,   230,   198,   201,   226,
     234,   368,    74,   230,   207,   196,   401,   262,    62,    62,
     213,   956,   197,   324,   373,   196,   197,   196,   252,   197,
     195,   388,   196,   226,   196,   196,   196,   230,   196,   651,
     196,   447,   737,   655,   122,   290,   196,   201,   196,    74,
     196,   196,   194,   410,   165,   196,   195,   592,   165,   195,
     197,   165,   419,   159,   194,   422,   197,   197,   197,   370,
     371,   159,   159,   197,   267,   159,     4,   194,   197,   148,
     197,   274,   275,   276,   308,   195,   195,   195,   281,   195,
     195,  1152,    62,   317,   287,  1051,   195,   321,   199,   195,
     324,   308,    24,    25,   192,   192,    28,   195,   308,    94,
      95,   195,   197,    91,    42,   308,   197,   197,   374,   197,
     197,   407,   528,   529,   197,   724,   194,   958,   194,   535,
     362,   536,   197,  1367,   669,   536,   368,   199,   973,   118,
      47,    94,    95,    74,   368,   369,   370,   371,    79,   146,
      32,   118,    32,   197,   197,  1365,   388,   194,    14,   164,
     120,   121,    32,    32,   388,   372,  1101,   194,   146,   197,
     671,    27,   100,    32,   199,   194,   595,   105,   410,   107,
     108,   109,   110,   111,   112,   113,   410,   419,    44,   194,
     422,     4,    74,   786,    74,    67,    68,  1153,   422,   196,
     197,   620,   795,  1413,    74,    74,   399,    74,    32,   140,
     141,   435,    79,   198,   407,    74,   601,   636,   435,    93,
      94,    95,   150,   151,   148,   153,  1161,   197,   840,    42,
     635,   142,    26,  1064,   635,  1066,   641,   482,    74,   791,
     641,   148,   435,    79,    26,   198,   194,   175,   194,    43,
      74,  1485,    46,   164,    74,   194,   163,    74,   559,    79,
     484,    43,   681,    27,    46,   671,   672,  1501,    67,    68,
     198,   194,   203,   140,   141,   499,   194,   754,   146,   201,
     815,   505,   506,   194,    32,   207,   197,   100,   196,   641,
     196,   213,   105,   770,   107,   108,   109,   110,   111,   112,
     113,   196,   197,   139,   140,   141,   196,   197,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
     140,   141,   139,   140,   141,   196,   861,   142,  1159,    62,
     898,   196,    46,    47,    48,   559,    50,   150,   151,   196,
     153,   934,    93,    94,    95,   267,   752,  1391,  1392,   942,
    1387,  1388,   274,   275,    60,    61,   196,   763,   551,   281,
      62,    62,   175,   196,   194,   287,   107,   108,   109,   110,
     111,   112,   107,   108,   109,   197,   975,   114,   115,   116,
     194,    62,  1491,   142,   194,   198,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,  1507,   146,
      41,   758,  1014,  1234,    50,   122,   630,   164,   632,   602,
     142,   201,   654,     9,   142,   142,   122,   952,   194,   122,
       8,    74,  1021,   196,    74,   649,    79,   194,   164,    79,
      14,    14,    60,    61,   175,    74,   196,   661,   662,    14,
     362,   847,   649,   849,   637,   662,   639,   196,   925,   649,
     195,   928,  1045,   164,   196,   196,   649,    80,    14,    91,
    1053,   195,   200,   194,   999,  1077,   659,     9,  1061,   662,
     195,  1006,    99,   194,   194,    83,   195,   399,     9,   703,
    1092,   196,   706,   889,    14,   407,   139,   140,   141,   790,
     140,   141,     9,    80,   122,   195,   703,   194,   180,   918,
      74,    74,    74,   703,   183,   911,   194,    74,   914,  1460,
     703,   195,   736,   196,   120,   736,   736,   195,   736,   736,
     713,   194,    62,   195,   121,   163,   758,  1478,   123,   736,
       9,   724,   725,   195,   758,  1486,   736,  1072,  1369,    14,
     192,     9,  1019,   736,    62,    99,   195,  1140,     9,    14,
    1027,   120,  1164,   107,   108,   109,   110,   111,   112,   791,
    1172,   284,   198,     9,   201,   288,   790,   195,   201,  1181,
     194,   201,   194,   194,  1051,   195,   201,   195,   802,   803,
     804,   196,   196,   989,   123,   827,  1115,   310,     9,   312,
     313,   314,   315,   195,   194,   142,   107,   108,   109,   110,
     111,   112,   194,   194,   828,   142,   830,   197,   832,   180,
     180,   828,    43,    44,    45,    46,    47,    48,    14,    50,
       9,   175,    74,   830,    14,   832,  1219,  1239,   196,   551,
     830,   197,   832,    91,   197,   828,    14,   830,   197,   832,
     864,   834,   898,   867,   201,    14,   196,    27,  1408,   194,
     194,   194,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,   175,    14,   890,  1144,   194,   194,
     194,     9,   195,  1150,  1151,   196,  1153,   196,   902,  1208,
     602,   902,   902,   123,   902,   902,   194,    14,     9,     4,
    1096,   195,   195,   123,   201,   902,    24,    25,    60,    61,
      28,     9,   902,   139,     4,     9,  1112,    74,   194,   902,
     123,   196,    14,    74,   194,   637,  1476,   639,   195,  1125,
     194,    49,   194,   965,   197,   195,   197,    42,   921,   953,
     197,   194,   123,   957,   958,   195,     9,   659,   139,   201,
      27,   196,    42,    69,   968,   195,   165,   196,  1360,    27,
    1362,   968,  1127,   195,  1231,   979,   195,   123,  1370,     9,
     122,   195,     9,   123,   123,   198,   195,    91,    14,   962,
     198,   194,   979,   195,   194,   968,  1125,   195,   195,   979,
     195,   197,   975,   976,   195,   100,   979,   195,     9,   195,
     105,   713,   107,   108,   109,   110,   111,   112,   113,  1411,
     100,    27,   724,   725,   196,   105,   123,   107,   108,   109,
     110,   111,   112,   113,   195,   197,   152,   196,   148,  1019,
     196,  1206,    74,    14,   194,   105,   123,  1027,  1021,   123,
     195,   195,   195,   195,   123,   150,   151,   195,   153,   195,
    1064,    14,  1066,   197,   196,    74,    14,   195,   194,   123,
     150,   151,   197,   153,   107,   108,   109,   110,   111,   112,
     175,   195,    14,   196,   196,   118,   119,    14,   197,   791,
     195,    52,    74,   201,   194,   175,    74,     9,   196,   207,
      74,   103,    91,   198,   142,   213,    91,  1111,   155,    30,
      14,  1115,   195,  1505,    74,   194,  1120,   161,   198,  1511,
     196,   194,   157,   156,  1111,   195,     9,    74,   196,   195,
     195,  1111,   834,   241,   242,   197,    74,  1124,  1111,   247,
      14,    14,   175,    74,    14,    14,  1468,  1127,   371,   482,
     739,   369,   156,   370,   789,  1159,   787,  1482,   976,   267,
    1089,  1165,  1478,  1256,  1144,  1169,   274,   275,  1340,  1173,
    1150,  1151,  1169,   281,   487,  1510,  1499,  1352,  1165,   287,
       4,  1120,    38,  1226,   374,  1165,  1173,   912,   466,  1354,
     879,  1195,  1165,  1173,   876,   466,  1169,  1201,   700,   905,
    1173,   940,   803,  1460,  1208,   817,   850,   282,     4,    -1,
     318,   275,    -1,   321,    -1,    -1,    -1,    -1,    42,   921,
      -1,  1478,    -1,  1210,    -1,    -1,    -1,    -1,    -1,  1486,
    1234,    -1,    -1,  1237,  1238,    -1,    -1,    -1,  1242,    -1,
      -1,  1238,    -1,    -1,  1248,    -1,    42,    -1,    -1,    -1,
    1237,  1231,    -1,    -1,   362,  1242,    -1,  1237,    -1,    -1,
     962,  1248,  1242,    -1,  1237,  1238,    -1,    -1,  1248,  1242,
    1351,    -1,    -1,   975,   976,  1248,   100,    -1,    -1,    -1,
      -1,   105,    -1,   107,   108,   109,   110,   111,   112,   113,
      -1,   399,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   407,
      -1,  1456,    -1,    -1,   100,  1491,    -1,    -1,    -1,   105,
      -1,   107,   108,   109,   110,   111,   112,   113,    -1,  1021,
      -1,  1507,  1403,    -1,    -1,    -1,   150,   151,    -1,   153,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   447,
      -1,    -1,    -1,  1472,   452,    -1,    -1,  1351,    -1,    -1,
      -1,   175,    -1,    -1,   150,   151,    -1,   153,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1369,    -1,    -1,    -1,  1373,
      -1,    -1,    -1,    -1,   198,    72,    -1,    74,  1382,   175,
      -1,   489,    -1,  1387,  1388,    -1,  1373,  1391,  1392,    -1,
      -1,    -1,    -1,  1373,    -1,    -1,    -1,    -1,    -1,  1403,
    1373,    -1,   198,    -1,    -1,  1409,  1410,    -1,    -1,    -1,
      -1,  1415,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     528,   529,  1409,  1410,    -1,    -1,    -1,   535,  1415,  1409,
    1410,    -1,    -1,    -1,    -1,  1415,  1409,  1410,    -1,    -1,
      -1,    -1,  1415,   551,  1448,    -1,    -1,    -1,    -1,    -1,
      -1,  1455,    -1,   150,   151,    -1,   153,   154,   155,    70,
      71,  1448,    -1,    -1,    -1,    -1,    -1,  1471,  1448,    80,
      -1,    -1,    -1,    -1,    -1,  1448,  1456,    -1,    -1,   176,
      -1,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   602,    -1,    -1,    -1,    -1,    -1,
     197,    -1,   199,    -1,    49,    24,    25,    -1,  1512,    28,
      -1,    -1,    -1,  1517,    -1,    -1,   127,   128,   129,   130,
     131,    -1,    -1,    -1,    -1,  1512,    -1,   138,    -1,   637,
    1517,   639,  1512,   144,   145,    -1,    -1,  1517,    -1,  1512,
      -1,    -1,    -1,    -1,  1517,    -1,    -1,   158,    -1,    -1,
      -1,   659,   660,    -1,    10,    11,    12,    -1,    -1,    72,
      -1,    -1,   173,   671,   672,   673,   674,   675,    -1,    -1,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    -1,   704,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   713,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   722,    -1,   724,   725,    -1,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,   739,    50,   146,    -1,    -1,    -1,   150,   151,    -1,
     153,   154,   155,    -1,   752,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   763,    -1,    -1,    -1,    -1,
      -1,   769,    -1,   176,   772,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,    -1,    -1,
      -1,    -1,    -1,   791,   197,    10,    11,    12,   207,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   241,   242,    -1,    -1,
      -1,    26,   247,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,   834,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   847,
      -1,   849,   198,    -1,    -1,    -1,    -1,    -1,   267,    -1,
      -1,    -1,    -1,    -1,   862,   274,   275,    -1,   866,    -1,
     868,    -1,   281,    -1,    10,    11,    12,    -1,   287,    -1,
      -1,    -1,    -1,   318,    -1,    -1,   321,    -1,   886,    -1,
      26,   889,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,   911,    50,    -1,   914,    32,    -1,    -1,
      -1,    -1,    -1,   921,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   452,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   362,    -1,    -1,    -1,    72,    -1,    74,
      -1,    -1,    -1,    -1,   962,    -1,    -1,    -1,    -1,    -1,
      60,    61,    -1,    -1,    -1,    -1,    -1,   975,   976,    -1,
     978,    -1,   489,   198,    -1,    -1,    -1,    -1,    -1,    -1,
     399,   989,    -1,    -1,   992,    -1,   994,   112,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,
      -1,    -1,   447,    -1,    -1,    -1,    -1,   452,    -1,    -1,
    1018,    -1,    -1,  1021,    -1,    -1,    -1,    -1,   143,    -1,
      -1,   146,   122,   148,    -1,   150,   151,    -1,   153,   154,
     155,    -1,    -1,   452,  1042,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   489,    -1,    -1,    -1,    -1,    -1,
      -1,   176,   198,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,    -1,    -1,    -1,   194,
     489,    -1,    -1,    -1,   199,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   528,   529,    -1,    -1,    -1,  1096,    -1,
     535,    -1,  1100,    -1,    -1,    -1,    -1,  1105,    -1,  1107,
      -1,    -1,    -1,    -1,  1112,    -1,    -1,  1115,  1116,    -1,
    1118,    -1,    -1,    -1,    -1,    -1,    -1,  1125,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1137,
      -1,    -1,   551,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   660,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    72,    -1,    74,   673,   674,   675,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1175,    -1,    -1,
      -1,    -1,    -1,    26,    -1,    -1,  1184,  1185,    -1,    -1,
      -1,    -1,    -1,   602,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    52,
    1208,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1217,
      -1,    -1,    -1,    -1,    -1,   660,    -1,    -1,   637,    72,
     639,    -1,    -1,    -1,    -1,    -1,   671,   672,   673,   674,
     675,   150,   151,    -1,   153,   154,   155,  1245,  1246,    -1,
     659,   660,    -1,  1251,  1252,    -1,    99,    -1,  1256,    -1,
      -1,    -1,   769,    -1,   673,   674,   675,   176,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   125,   126,    -1,    -1,    -1,    -1,   197,    -1,
     199,    -1,    -1,    -1,    -1,   704,    -1,    -1,    -1,    -1,
     143,    -1,    -1,   146,   713,   148,    -1,   150,   151,    -1,
     153,   154,   155,   722,    -1,   724,   725,   752,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   168,    -1,    -1,   763,    -1,
     739,    -1,    -1,   176,   769,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,    -1,    -1,
      -1,   194,    -1,    -1,    -1,   862,    -1,  1355,    -1,   866,
     769,   868,    -1,   772,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   886,
      -1,    -1,   791,    -1,  1382,    -1,    -1,    -1,  1386,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1397,
      -1,    -1,    -1,    -1,  1402,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   847,    -1,   849,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   834,    -1,   862,    -1,    -1,
      -1,   866,    -1,   868,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,   886,    50,   862,   889,    -1,    -1,   866,    -1,   868,
      -1,  1459,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1467,
      -1,   978,    -1,    -1,    -1,    -1,   911,   886,    -1,   914,
      -1,    -1,    -1,    -1,  1482,   992,    -1,   994,    -1,    -1,
      -1,    -1,    -1,  1491,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1507,
      -1,    -1,   921,    -1,    -1,    -1,    -1,  1515,    -1,    -1,
      -1,    -1,    -1,  1521,    -1,    -1,    -1,  1525,    -1,  1527,
      -1,    -1,    -1,    -1,    -1,  1042,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   978,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   962,   989,    -1,    -1,   992,    -1,   994,
      -1,    -1,    -1,    -1,    -1,    -1,   975,   976,    -1,   978,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   992,    -1,   994,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1100,    -1,    -1,    -1,    -1,  1105,    -1,
    1107,    -1,    -1,    -1,    -1,    -1,    -1,  1042,    -1,  1018,
      -1,    -1,  1021,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1137,    -1,    26,  1042,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,
      -1,  1096,    -1,    -1,    -1,  1100,    -1,    -1,  1175,    -1,
    1105,    26,  1107,    -1,    -1,    -1,    -1,  1112,    -1,    -1,
    1115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1125,  1100,    -1,    -1,    -1,    -1,  1105,    52,  1107,    -1,
      -1,    -1,  1137,    -1,    -1,    -1,    -1,  1116,    -1,  1118,
    1217,    -1,    -1,    -1,    -1,    -1,    -1,    72,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1137,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1245,  1246,
    1175,    -1,    -1,    -1,  1251,    -1,    -1,    -1,  1255,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,  1175,    50,    -1,    -1,
     125,   126,    -1,  1208,    -1,  1184,  1185,    -1,    -1,    -1,
      -1,    -1,  1217,    -1,    -1,    -1,    -1,    -1,   143,    -1,
      -1,   146,    -1,   148,    72,   150,   151,    -1,   153,   154,
     155,    -1,    -1,    -1,   198,    -1,    -1,    -1,  1217,    -1,
    1245,  1246,    -1,   168,    -1,    -1,  1251,    -1,    -1,    -1,
      -1,   176,    -1,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,  1245,  1246,    -1,   194,
      -1,    -1,  1251,  1252,    -1,    -1,    -1,  1256,  1355,     5,
       6,    -1,     8,     9,    10,    -1,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,   146,    -1,
      26,    27,   150,   151,    -1,   153,   154,   155,    -1,  1386,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,
    1397,    47,    -1,   452,    50,  1402,    52,    -1,   176,    -1,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,    -1,    -1,    -1,    64,    -1,  1425,    -1,
    1355,    -1,    -1,    79,    72,    -1,    74,    -1,    -1,    -1,
     489,    -1,    -1,    -1,   452,    -1,    -1,    93,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1355,  1382,    -1,    -1,
      -1,  1386,  1459,    -1,   110,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1397,    -1,   112,    -1,    -1,  1402,    -1,    -1,
      -1,   489,    -1,    -1,    -1,    -1,    -1,  1386,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1397,    -1,
      -1,    -1,    -1,  1402,    -1,   143,    -1,    -1,   146,    -1,
     148,    -1,   150,   151,    -1,   153,   154,   155,  1515,    -1,
      -1,    -1,    -1,    -1,  1521,    -1,    -1,    -1,  1525,    -1,
    1527,    -1,    -1,    -1,  1459,   181,    -1,    -1,   176,    -1,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,    -1,    -1,    -1,   194,    -1,    -1,    -1,
    1459,   199,    -1,    -1,    -1,    -1,  1491,    -1,  1467,    -1,
      70,    71,    72,    -1,    -1,    -1,   222,    -1,    -1,   225,
      -1,    -1,  1507,  1482,    -1,    -1,   232,   233,    -1,    -1,
    1515,    -1,    -1,    -1,    -1,    -1,  1521,    -1,    -1,    -1,
    1525,    -1,  1527,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   660,    -1,    -1,    -1,    -1,  1515,    -1,    -1,    -1,
      -1,    -1,  1521,    -1,   673,   674,  1525,    -1,  1527,    -1,
     276,    -1,    -1,    -1,    -1,    -1,   282,    -1,    -1,    -1,
     286,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     150,   151,   660,   153,   154,   155,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   309,    -1,   673,   674,   675,    -1,    -1,
      -1,    -1,    -1,    -1,   320,    -1,   176,    -1,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,    -1,    -1,   364,   365,
     769,   367,    -1,    -1,    -1,    -1,    -1,    -1,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
      -1,    -1,    -1,    -1,    -1,   391,   392,    -1,   394,   395,
     396,    -1,    -1,    -1,    -1,    -1,   402,    -1,    -1,    -1,
      -1,   769,    -1,    -1,    -1,    -1,   412,    -1,   414,    -1,
      -1,    10,    11,    12,   420,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   430,    -1,   432,    26,    -1,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,    -1,   459,    -1,    -1,   462,   463,   464,   868,
      -1,    -1,    -1,    -1,    72,    -1,    74,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   886,    -1,   485,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   862,    -1,    -1,    -1,   866,    -1,
     868,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
     118,    -1,    -1,    -1,   520,    -1,    -1,    26,   886,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,   150,   151,    -1,   153,   154,   155,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   563,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,   978,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   992,    -1,   994,    -1,    -1,   594,   197,
      -1,   199,    -1,    -1,    -1,    -1,    -1,   603,    -1,   198,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     978,    -1,    -1,   619,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   992,    -1,   994,    -1,    -1,    -1,
      -1,    -1,    -1,  1042,    -1,    10,    11,    12,    -1,   645,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,   658,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,  1042,    50,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   689,    10,    11,    12,    -1,    -1,   198,
      -1,  1100,    -1,    -1,    -1,    -1,  1105,    -1,  1107,    -1,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    -1,    -1,   733,  1137,    -1,
      -1,    -1,  1100,    -1,    -1,    -1,    -1,  1105,    -1,  1107,
     746,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,   767,    50,    -1,    -1,    -1,  1175,    -1,    -1,  1137,
      -1,   777,    -1,    -1,   780,    -1,   782,    -1,    -1,    -1,
     786,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   795,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1175,  1217,    -1,
      -1,    -1,    -1,    -1,   820,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   198,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1245,  1246,    -1,    -1,
      -1,    -1,  1251,    -1,    72,    -1,    -1,    -1,    -1,  1217,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   870,   871,   872,    -1,    -1,    -1,
     876,   877,   198,    -1,    -1,    -1,    -1,  1245,  1246,    -1,
      -1,    -1,    -1,  1251,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   898,    -1,    -1,    -1,    -1,   125,   126,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   917,    -1,    -1,    -1,   143,    -1,    -1,   146,    -1,
     148,    -1,   150,   151,    -1,   153,   154,   155,   934,    -1,
      -1,   159,    -1,    -1,    -1,    -1,   942,    -1,   944,    -1,
     168,    -1,    -1,    -1,    -1,    -1,  1355,    -1,   176,    -1,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,    -1,    -1,    -1,   194,   973,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   982,  1386,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,  1355,  1397,    -1,
      -1,    -1,    -1,  1402,    -1,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,  1386,    50,
      -1,    -1,    -1,    -1,  1030,    -1,    -1,    -1,  1034,  1397,
    1036,    -1,    -1,    -1,  1402,    -1,    -1,    -1,    -1,  1045,
      -1,    -1,    -1,    10,    11,    12,  1052,  1053,    -1,    -1,
    1459,    -1,    -1,    -1,    -1,  1061,    -1,    -1,    -1,    26,
      -1,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1459,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1109,    -1,    -1,  1515,    -1,    -1,    -1,
      -1,    -1,  1521,    -1,    -1,    -1,  1525,    -1,  1527,    -1,
      -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,  1140,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1515,    -1,    -1,
      -1,    -1,    -1,  1521,    -1,    -1,    -1,  1525,    -1,  1527,
      42,    43,    44,    -1,    -1,   196,    -1,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    -1,    76,    -1,    -1,    -1,    80,    81,
      82,    83,    -1,    85,    -1,    87,    -1,    89,    -1,    -1,
      92,    -1,    -1,  1219,    96,    97,    98,    99,   100,   101,
     102,    -1,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   198,   114,   115,   116,   117,   118,   119,    -1,    -1,
      -1,  1247,   124,   125,    -1,   127,   128,   129,   130,   131,
      -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,
      -1,   143,   144,   145,   146,   147,   148,    -1,   150,   151,
      -1,   153,   154,   155,   156,    -1,   158,    -1,    -1,    -1,
     162,    -1,    -1,    -1,    -1,    -1,   168,   169,    -1,   171,
      -1,   173,   174,   175,   176,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
      -1,    -1,   194,    -1,   196,   197,   198,   199,   200,    12,
     202,   203,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    26,    -1,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    -1,    -1,
      -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,    80,
      81,    82,    83,    -1,    85,    -1,    87,    -1,    89,    -1,
      -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,   100,
     101,   102,    -1,   104,   105,   106,   107,   108,   109,   110,
     111,   112,    -1,   114,   115,   116,   117,   118,   119,    -1,
      -1,    -1,    -1,   124,   125,    -1,   127,   128,   129,   130,
     131,    -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,
      -1,    -1,   143,   144,   145,   146,   147,   148,    -1,   150,
     151,    -1,   153,   154,   155,   156,    -1,   158,    -1,    -1,
      -1,   162,    -1,    -1,    -1,    -1,    -1,   168,   169,    -1,
     171,    -1,   173,   174,   175,   176,    -1,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,    -1,    -1,   194,    -1,   196,   197,   198,   199,   200,
      -1,   202,   203,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    26,    13,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
     100,   101,   102,    -1,   104,   105,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,   117,   118,   119,
      -1,    -1,    -1,    -1,   124,   125,    -1,   127,   128,   129,
     130,   131,    -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,
      -1,    -1,    -1,   143,   144,   145,   146,   147,   148,    -1,
     150,   151,    -1,   153,   154,   155,   156,    -1,   158,    -1,
      -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,   168,   169,
      -1,   171,    -1,   173,   174,   175,   176,    -1,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,    -1,    -1,   194,    -1,   196,   197,    -1,   199,
     200,    -1,   202,   203,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    -1,    76,    -1,    -1,
      -1,    80,    81,    82,    83,    -1,    85,    -1,    87,    -1,
      89,    -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,
      99,    -1,   101,   102,    -1,   104,    -1,   106,   107,   108,
     109,   110,   111,   112,    -1,   114,   115,   116,    -1,   118,
     119,    -1,    -1,    -1,    -1,   124,   125,    -1,   127,   128,
     129,   130,   131,    -1,    -1,    -1,    -1,    -1,    -1,   138,
      -1,    -1,    -1,    -1,   143,   144,   145,   146,   147,   148,
      -1,   150,   151,    -1,   153,   154,   155,   156,    -1,   158,
      -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,   168,
      -1,    -1,    -1,    -1,   173,   174,   175,   176,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,    -1,    -1,   194,    -1,   196,   197,   198,
     199,   200,    -1,   202,   203,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    -1,    76,    -1,
      -1,    -1,    80,    81,    82,    83,    -1,    85,    -1,    87,
      -1,    89,    -1,    -1,    92,    -1,    -1,    -1,    96,    97,
      98,    99,    -1,   101,   102,    -1,   104,    -1,   106,   107,
     108,   109,   110,   111,   112,    -1,   114,   115,   116,    -1,
     118,   119,    -1,    -1,    -1,    -1,   124,   125,    -1,   127,
     128,   129,   130,   131,    -1,    -1,    -1,    -1,    -1,    -1,
     138,    -1,    -1,    -1,    -1,   143,   144,   145,   146,   147,
     148,    -1,   150,   151,    -1,   153,   154,   155,   156,    -1,
     158,    -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,
     168,    -1,    -1,    -1,    -1,   173,   174,   175,   176,    -1,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,    -1,    -1,   194,    -1,   196,   197,
     198,   199,   200,    -1,   202,   203,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,    44,    -1,    -1,
      -1,    -1,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    -1,    76,
      -1,    -1,    -1,    80,    81,    82,    83,    -1,    85,    -1,
      87,    -1,    89,    -1,    -1,    92,    -1,    -1,    -1,    96,
      97,    98,    99,    -1,   101,   102,    -1,   104,    -1,   106,
     107,   108,   109,   110,   111,   112,    -1,   114,   115,   116,
      -1,   118,   119,    -1,    -1,    -1,    -1,   124,   125,    -1,
     127,   128,   129,   130,   131,    -1,    -1,    -1,    -1,    -1,
      -1,   138,    -1,    -1,    -1,    -1,   143,   144,   145,   146,
     147,   148,    -1,   150,   151,    -1,   153,   154,   155,   156,
      -1,   158,    -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,
      -1,   168,    -1,    -1,    -1,    -1,   173,   174,   175,   176,
      -1,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,    -1,    -1,   194,    -1,   196,
     197,   198,   199,   200,    -1,   202,   203,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,    -1,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    -1,    -1,    70,    71,    72,    73,    74,    -1,
      76,    -1,    -1,    -1,    80,    81,    82,    83,    84,    85,
      -1,    87,    -1,    89,    -1,    -1,    92,    -1,    -1,    -1,
      96,    97,    98,    99,    -1,   101,   102,    -1,   104,    -1,
     106,   107,   108,   109,   110,   111,   112,    -1,   114,   115,
     116,    -1,   118,   119,    -1,    -1,    -1,    -1,   124,   125,
      -1,   127,   128,   129,   130,   131,    -1,    -1,    -1,    -1,
      -1,    -1,   138,    -1,    -1,    -1,    -1,   143,   144,   145,
     146,   147,   148,    -1,   150,   151,    -1,   153,   154,   155,
     156,    -1,   158,    -1,    -1,    -1,   162,    -1,    -1,    -1,
      -1,    -1,   168,    -1,    -1,    -1,    -1,   173,   174,   175,
     176,    -1,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,    -1,    -1,   194,    -1,
     196,   197,    -1,   199,   200,    -1,   202,   203,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      -1,    76,    -1,    -1,    -1,    80,    81,    82,    83,    -1,
      85,    -1,    87,    -1,    89,    90,    -1,    92,    -1,    -1,
      -1,    96,    97,    98,    99,    -1,   101,   102,    -1,   104,
      -1,   106,   107,   108,   109,   110,   111,   112,    -1,   114,
     115,   116,    -1,   118,   119,    -1,    -1,    -1,    -1,   124,
     125,    -1,   127,   128,   129,   130,   131,    -1,    -1,    -1,
      -1,    -1,    -1,   138,    -1,    -1,    -1,    -1,   143,   144,
     145,   146,   147,   148,    -1,   150,   151,    -1,   153,   154,
     155,   156,    -1,   158,    -1,    -1,    -1,   162,    -1,    -1,
      -1,    -1,    -1,   168,    -1,    -1,    -1,    -1,   173,   174,
     175,   176,    -1,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,    -1,    -1,   194,
      -1,   196,   197,    -1,   199,   200,    -1,   202,   203,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    -1,    76,    -1,    -1,    -1,    80,    81,    82,    83,
      -1,    85,    -1,    87,    -1,    89,    -1,    -1,    92,    -1,
      -1,    -1,    96,    97,    98,    99,    -1,   101,   102,    -1,
     104,    -1,   106,   107,   108,   109,   110,   111,   112,    -1,
     114,   115,   116,    -1,   118,   119,    -1,    -1,    -1,    -1,
     124,   125,    -1,   127,   128,   129,   130,   131,    -1,    -1,
      -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,    -1,   143,
     144,   145,   146,   147,   148,    -1,   150,   151,    -1,   153,
     154,   155,   156,    -1,   158,    -1,    -1,    -1,   162,    -1,
      -1,    -1,    -1,    -1,   168,    -1,    -1,    -1,    -1,   173,
     174,   175,   176,    -1,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,    -1,    -1,
     194,    -1,   196,   197,   198,   199,   200,    -1,   202,   203,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    -1,    76,    -1,    -1,    -1,    80,    81,    82,
      83,    -1,    85,    -1,    87,    -1,    89,    -1,    -1,    92,
      -1,    -1,    -1,    96,    97,    98,    99,    -1,   101,   102,
      -1,   104,    -1,   106,   107,   108,   109,   110,   111,   112,
      -1,   114,   115,   116,    -1,   118,   119,    -1,    -1,    -1,
      -1,   124,   125,    -1,   127,   128,   129,   130,   131,    -1,
      -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,    -1,
     143,   144,   145,   146,   147,   148,    -1,   150,   151,    -1,
     153,   154,   155,   156,    -1,   158,    -1,    -1,    -1,   162,
      -1,    -1,    -1,    -1,    -1,   168,    -1,    -1,    -1,    -1,
     173,   174,   175,   176,    -1,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,    -1,
      -1,   194,    -1,   196,   197,   198,   199,   200,    -1,   202,
     203,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    -1,    76,    -1,    -1,    -1,    80,    81,
      82,    83,    -1,    85,    -1,    87,    88,    89,    -1,    -1,
      92,    -1,    -1,    -1,    96,    97,    98,    99,    -1,   101,
     102,    -1,   104,    -1,   106,   107,   108,   109,   110,   111,
     112,    -1,   114,   115,   116,    -1,   118,   119,    -1,    -1,
      -1,    -1,   124,   125,    -1,   127,   128,   129,   130,   131,
      -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,
      -1,   143,   144,   145,   146,   147,   148,    -1,   150,   151,
      -1,   153,   154,   155,   156,    -1,   158,    -1,    -1,    -1,
     162,    -1,    -1,    -1,    -1,    -1,   168,    -1,    -1,    -1,
      -1,   173,   174,   175,   176,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
      -1,    -1,   194,    -1,   196,   197,    -1,   199,   200,    -1,
     202,   203,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,    80,
      81,    82,    83,    -1,    85,    -1,    87,    -1,    89,    -1,
      -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,    -1,
     101,   102,    -1,   104,    -1,   106,   107,   108,   109,   110,
     111,   112,    -1,   114,   115,   116,    -1,   118,   119,    -1,
      -1,    -1,    -1,   124,   125,    -1,   127,   128,   129,   130,
     131,    -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,
      -1,    -1,   143,   144,   145,   146,   147,   148,    -1,   150,
     151,    -1,   153,   154,   155,   156,    -1,   158,    -1,    -1,
      -1,   162,    -1,    -1,    -1,    -1,    -1,   168,    -1,    -1,
      -1,    -1,   173,   174,   175,   176,    -1,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,    -1,    -1,   194,    -1,   196,   197,   198,   199,   200,
      -1,   202,   203,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,   118,   119,
      -1,    -1,    -1,    -1,   124,   125,    -1,   127,   128,   129,
     130,   131,    -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,
      -1,    -1,    -1,   143,   144,   145,   146,   147,   148,    -1,
     150,   151,    -1,   153,   154,   155,   156,    -1,   158,    -1,
      -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,   168,    -1,
      -1,    -1,    -1,   173,   174,   175,   176,    -1,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,    -1,    -1,   194,    -1,   196,   197,   198,   199,
     200,    -1,   202,   203,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    -1,    76,    -1,    -1,
      -1,    80,    81,    82,    83,    -1,    85,    86,    87,    -1,
      89,    -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,
      99,    -1,   101,   102,    -1,   104,    -1,   106,   107,   108,
     109,   110,   111,   112,    -1,   114,   115,   116,    -1,   118,
     119,    -1,    -1,    -1,    -1,   124,   125,    -1,   127,   128,
     129,   130,   131,    -1,    -1,    -1,    -1,    -1,    -1,   138,
      -1,    -1,    -1,    -1,   143,   144,   145,   146,   147,   148,
      -1,   150,   151,    -1,   153,   154,   155,   156,    -1,   158,
      -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,   168,
      -1,    -1,    -1,    -1,   173,   174,   175,   176,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,    -1,    -1,   194,    -1,   196,   197,    -1,
     199,   200,    -1,   202,   203,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    -1,    76,    -1,
      -1,    -1,    80,    81,    82,    83,    -1,    85,    -1,    87,
      -1,    89,    -1,    -1,    92,    -1,    -1,    -1,    96,    97,
      98,    99,    -1,   101,   102,    -1,   104,    -1,   106,   107,
     108,   109,   110,   111,   112,    -1,   114,   115,   116,    -1,
     118,   119,    -1,    -1,    -1,    -1,   124,   125,    -1,   127,
     128,   129,   130,   131,    -1,    -1,    -1,    -1,    -1,    -1,
     138,    -1,    -1,    -1,    -1,   143,   144,   145,   146,   147,
     148,    -1,   150,   151,    -1,   153,   154,   155,   156,    -1,
     158,    -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,
     168,    -1,    -1,    -1,    -1,   173,   174,   175,   176,    -1,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,    -1,    -1,   194,    -1,   196,   197,
     198,   199,   200,    -1,   202,   203,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,    44,    -1,    -1,
      -1,    -1,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    -1,    76,
      -1,    -1,    -1,    80,    81,    82,    83,    -1,    85,    -1,
      87,    -1,    89,    -1,    -1,    92,    -1,    -1,    -1,    96,
      97,    98,    99,    -1,   101,   102,    -1,   104,    -1,   106,
     107,   108,   109,   110,   111,   112,    -1,   114,   115,   116,
      -1,   118,   119,    -1,    -1,    -1,    -1,   124,   125,    -1,
     127,   128,   129,   130,   131,    -1,    -1,    -1,    -1,    -1,
      -1,   138,    -1,    -1,    -1,    -1,   143,   144,   145,   146,
     147,   148,    -1,   150,   151,    -1,   153,   154,   155,   156,
      -1,   158,    -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,
      -1,   168,    -1,    -1,    -1,    -1,   173,   174,   175,   176,
      -1,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,    -1,    -1,   194,    -1,   196,
     197,   198,   199,   200,    -1,   202,   203,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,    -1,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    -1,    -1,    -1,    70,    71,    72,    73,    74,    -1,
      76,    -1,    -1,    -1,    80,    81,    82,    83,    -1,    85,
      -1,    87,    -1,    89,    -1,    -1,    92,    -1,    -1,    -1,
      96,    97,    98,    99,    -1,   101,   102,    -1,   104,    -1,
     106,   107,   108,   109,   110,   111,   112,    -1,   114,   115,
     116,    -1,   118,   119,    -1,    -1,    -1,    -1,   124,   125,
      -1,   127,   128,   129,   130,   131,    -1,    -1,    -1,    -1,
      -1,    -1,   138,    -1,    -1,    -1,    -1,   143,   144,   145,
     146,   147,   148,    -1,   150,   151,    -1,   153,   154,   155,
     156,    -1,   158,    -1,    -1,    -1,   162,    -1,    -1,    -1,
      -1,    -1,   168,    -1,    -1,    -1,    -1,   173,   174,   175,
     176,    -1,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,    -1,    -1,   194,    -1,
     196,   197,   198,   199,   200,    -1,   202,   203,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      -1,    76,    -1,    -1,    -1,    80,    81,    82,    83,    -1,
      85,    -1,    87,    -1,    89,    -1,    -1,    92,    -1,    -1,
      -1,    96,    97,    98,    99,    -1,   101,   102,    -1,   104,
      -1,   106,   107,   108,   109,   110,   111,   112,    -1,   114,
     115,   116,    -1,   118,   119,    -1,    -1,    -1,    -1,   124,
     125,    -1,   127,   128,   129,   130,   131,    -1,    -1,    -1,
      -1,    -1,    -1,   138,    -1,    -1,    -1,    -1,   143,   144,
     145,   146,   147,   148,    -1,   150,   151,    -1,   153,   154,
     155,   156,    -1,   158,    -1,    -1,    -1,   162,    -1,    -1,
      -1,    -1,    -1,   168,    -1,    -1,    -1,    -1,   173,   174,
     175,   176,    -1,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,    -1,    -1,   194,
      -1,   196,   197,    -1,   199,   200,    -1,   202,   203,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    -1,    76,    -1,    -1,    -1,    80,    81,    82,    83,
      -1,    85,    -1,    87,    -1,    89,    -1,    -1,    92,    -1,
      -1,    -1,    96,    97,    98,    99,    -1,   101,   102,    -1,
     104,    -1,   106,   107,   108,   109,   110,   111,   112,    -1,
     114,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     124,   125,    -1,   127,   128,   129,   130,   131,    -1,    -1,
      -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,    -1,   143,
     144,   145,   146,   147,   148,    -1,   150,   151,    -1,   153,
     154,   155,    -1,    -1,   158,    -1,    -1,    -1,   162,    -1,
      -1,    -1,    -1,    -1,   168,    -1,    -1,    -1,    -1,   173,
     174,   175,   176,    -1,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,    -1,    -1,
     194,    -1,   196,   197,    -1,   199,   200,    -1,   202,   203,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    -1,    76,    -1,    -1,    -1,    80,    81,    82,
      83,    -1,    85,    -1,    87,    -1,    89,    -1,    -1,    92,
      -1,    -1,    -1,    96,    97,    98,    99,    -1,   101,   102,
      -1,   104,    -1,   106,   107,   108,   109,   110,   111,   112,
      -1,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,   125,    -1,   127,   128,   129,   130,   131,    -1,
      -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,    -1,
     143,   144,   145,   146,   147,   148,    -1,   150,   151,    -1,
     153,   154,   155,    -1,    -1,   158,    -1,    -1,    -1,   162,
      -1,    -1,    -1,    -1,    -1,   168,    -1,    -1,    -1,    -1,
     173,   174,   175,   176,    -1,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,    -1,
      -1,   194,    -1,   196,   197,    -1,   199,   200,    -1,   202,
     203,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    -1,    76,    -1,    -1,    -1,    80,    81,
      82,    83,    -1,    85,    -1,    87,    -1,    89,    -1,    -1,
      92,    -1,    -1,    -1,    96,    97,    98,    99,    -1,   101,
     102,    -1,   104,    -1,   106,   107,   108,   109,   110,   111,
     112,    -1,   114,   115,   116,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   124,   125,    -1,   127,   128,   129,   130,   131,
      -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,
      -1,   143,   144,   145,   146,   147,   148,    -1,   150,   151,
      -1,   153,   154,   155,    -1,    -1,   158,    -1,    -1,    -1,
     162,    -1,    -1,    -1,    -1,    -1,   168,    -1,    -1,    -1,
      -1,   173,   174,   175,   176,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
      -1,    -1,   194,    -1,   196,   197,    -1,   199,   200,    -1,
     202,   203,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,    80,
      81,    82,    83,    -1,    85,    -1,    87,    -1,    89,    -1,
      -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,    -1,
     101,   102,    -1,   104,    -1,   106,   107,   108,   109,   110,
     111,   112,    -1,   114,   115,   116,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   124,   125,    -1,   127,   128,   129,   130,
     131,    -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,
      -1,    -1,   143,   144,   145,   146,   147,   148,    -1,   150,
     151,    -1,   153,   154,   155,    -1,    -1,   158,    -1,    -1,
      -1,   162,    -1,    -1,    -1,    -1,    -1,   168,    -1,    -1,
      -1,    -1,   173,   174,   175,   176,    -1,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,    -1,    -1,   194,    -1,   196,   197,    -1,   199,   200,
      -1,   202,   203,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,
      80,    81,    82,    83,    -1,    85,    -1,    87,    -1,    89,
      -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,    99,
      -1,   101,   102,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   114,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,    -1,   127,   128,   129,
     130,   131,    -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,
      -1,    -1,    -1,   143,   144,   145,   146,   147,   148,    -1,
     150,   151,    -1,   153,   154,   155,    -1,    -1,   158,    -1,
      -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,   168,    -1,
      -1,    -1,    -1,   173,   174,   175,   176,    -1,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,    -1,    -1,   194,    -1,   196,   197,    -1,   199,
     200,    -1,   202,   203,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    -1,    76,    -1,    -1,
      -1,    80,    81,    82,    83,    -1,    85,    -1,    87,    -1,
      89,    -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,
      99,    -1,   101,   102,    -1,   104,    -1,   106,   107,   108,
     109,   110,   111,   112,    -1,   114,   115,   116,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   124,   125,    -1,   127,   128,
     129,   130,   131,    -1,    -1,    -1,    -1,    -1,    -1,   138,
      -1,    -1,    -1,    -1,   143,   144,   145,   146,   147,   148,
      -1,   150,   151,    -1,   153,   154,   155,    -1,    -1,   158,
      -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,   168,
      -1,    -1,    -1,    -1,   173,   174,   175,   176,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,    -1,    -1,   194,    -1,   196,   197,    -1,
     199,   200,    -1,   202,   203,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,
     108,   109,   110,   111,   112,    -1,    72,   115,   116,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,   125,    -1,   127,
     128,   129,   130,   131,    -1,    -1,    -1,    -1,    -1,    -1,
     138,    -1,    -1,    -1,    -1,   143,   144,   145,   146,   147,
     148,    -1,   150,   151,    -1,   153,   154,   155,    -1,    -1,
     158,    -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,
     168,    -1,    -1,    -1,    -1,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   150,   151,   194,   153,   154,   155,
      -1,   199,   200,    -1,   202,   203,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
     176,    -1,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,    -1,    -1,    -1,   194,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,
      -1,    -1,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     107,   108,   109,   110,   111,   112,    -1,    -1,   115,   116,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,   125,    -1,
     127,   128,   129,   130,   131,    -1,    -1,    -1,    -1,    -1,
      -1,   138,    -1,    -1,    -1,    -1,   143,   144,   145,   146,
      -1,   148,    -1,   150,   151,    -1,   153,   154,   155,    -1,
      -1,   158,    -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,
      -1,   168,    -1,    -1,    -1,    -1,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,    -1,    -1,   194,    -1,   196,
      -1,    -1,   199,   200,    -1,   202,   203,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   107,   108,   109,   110,   111,   112,    -1,    72,   115,
     116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,   125,
      -1,   127,   128,   129,   130,   131,    -1,    -1,    -1,    -1,
      -1,    -1,   138,    -1,    -1,    -1,    -1,   143,   144,   145,
     146,   147,   148,    -1,   150,   151,    -1,   153,   154,   155,
      -1,    -1,   158,    -1,    -1,    -1,   162,    -1,    -1,    -1,
      -1,    -1,   168,    -1,    -1,    -1,    -1,   173,   174,   175,
     176,    -1,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   150,   151,   194,   153,
     154,   155,    -1,   199,   200,    -1,   202,   203,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,   176,    -1,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,    32,    -1,    -1,
     194,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   107,   108,   109,   110,   111,   112,    -1,    72,
     115,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,
     125,    -1,   127,   128,   129,   130,   131,    -1,    -1,    -1,
      -1,    -1,    -1,   138,    -1,    -1,    -1,    -1,   143,   144,
     145,   146,    -1,   148,    -1,   150,   151,    -1,   153,   154,
     155,    -1,    -1,   158,    -1,   118,    -1,   162,    -1,    -1,
      -1,    -1,    -1,   168,    -1,    -1,    -1,    -1,   173,    -1,
     175,   176,    -1,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   150,   151,   194,
     153,   154,   155,    -1,   199,   200,    -1,   202,   203,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,   176,    -1,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   107,   108,   109,   110,   111,   112,    -1,
      -1,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     124,   125,    -1,   127,   128,   129,   130,   131,    -1,    -1,
      -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,    -1,   143,
     144,   145,   146,    -1,   148,    -1,   150,   151,    -1,   153,
     154,   155,    -1,    -1,   158,    -1,    -1,    -1,   162,    -1,
      -1,    -1,    -1,    -1,   168,    -1,    -1,    -1,    -1,   173,
      -1,   175,   176,    -1,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,    -1,    -1,
     194,    -1,   196,    -1,    -1,   199,   200,    -1,   202,   203,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   107,   108,   109,   110,   111,   112,
      -1,    -1,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,   125,    -1,   127,   128,   129,   130,   131,    -1,
      -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,    -1,
     143,   144,   145,   146,    -1,   148,    -1,   150,   151,    -1,
     153,   154,   155,    -1,    -1,   158,    -1,    -1,    -1,   162,
      -1,    -1,    -1,    -1,    -1,   168,    -1,    -1,    -1,    -1,
     173,    -1,   175,   176,    -1,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,    -1,
      -1,   194,    -1,   196,    -1,    -1,   199,   200,    -1,   202,
     203,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    -1,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    96,    -1,    -1,    99,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,   108,   109,   110,   111,
     112,    -1,    72,   115,   116,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   124,   125,    -1,   127,   128,   129,   130,   131,
      -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,
      -1,   143,   144,   145,   146,    -1,   148,    -1,   150,   151,
      -1,   153,   154,   155,    -1,    -1,   158,    -1,    -1,    -1,
     162,    -1,    -1,    -1,    -1,    -1,   168,    -1,    -1,    -1,
      -1,   173,    -1,   175,   176,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     150,   151,   194,   153,   154,   155,    -1,   199,   200,    -1,
     202,   203,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,   176,    -1,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,   110,
     111,   112,    -1,    -1,   115,   116,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   124,   125,    -1,   127,   128,   129,   130,
     131,    -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,
      -1,    -1,   143,   144,   145,   146,    -1,   148,    -1,   150,
     151,    -1,   153,   154,   155,    -1,    -1,   158,    -1,    -1,
      -1,   162,    -1,    -1,    -1,    -1,    -1,   168,    -1,    -1,
      -1,    -1,   173,    -1,   175,   176,    -1,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,    -1,    -1,   194,   195,    -1,    -1,    -1,   199,   200,
      -1,   202,   203,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,
      70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,
      80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,    -1,    -1,   115,   116,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,    -1,   127,   128,   129,
     130,   131,    -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,
      -1,    -1,    -1,   143,   144,   145,   146,    -1,   148,    -1,
     150,   151,    -1,   153,   154,   155,    -1,    -1,   158,    -1,
      -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,   168,    -1,
      -1,    -1,    -1,   173,    -1,   175,   176,    -1,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,    -1,    -1,   194,    -1,    -1,    -1,    -1,   199,
     200,    -1,   202,   203,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,    -1,
      -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,
     109,   110,   111,   112,    -1,    -1,   115,   116,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   124,   125,    -1,   127,   128,
     129,   130,   131,    -1,    -1,    -1,    -1,    -1,    -1,   138,
      -1,    -1,    -1,    -1,   143,   144,   145,   146,    -1,   148,
      -1,   150,   151,    -1,   153,   154,   155,    -1,    -1,   158,
      -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,   168,
      -1,    -1,    -1,    -1,   173,    -1,   175,   176,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,    -1,    -1,   194,    -1,    -1,    -1,    -1,
     199,   200,    -1,   202,   203,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    -1,    -1,    -1,
      -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,
     108,   109,   110,   111,   112,    -1,    -1,   115,   116,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,   125,    -1,   127,
     128,   129,   130,   131,    -1,    -1,    -1,    -1,    -1,    -1,
     138,    -1,    -1,    -1,    -1,   143,   144,   145,   146,    -1,
     148,    -1,   150,   151,    -1,   153,   154,   155,    -1,    -1,
     158,    -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,
     168,    -1,    -1,    -1,    -1,   173,    -1,   175,   176,    -1,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,    -1,    -1,   194,    -1,    -1,    -1,
      -1,   199,   200,    -1,   202,   203,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,
      -1,    -1,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    -1,    70,    71,    72,    73,    74,    -1,    -1,
      -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     107,   108,   109,   110,   111,   112,    -1,    -1,   115,   116,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,   125,    -1,
     127,   128,   129,   130,   131,    -1,    -1,    -1,    -1,    -1,
      -1,   138,    -1,    -1,    -1,    -1,   143,   144,   145,   146,
      -1,   148,    -1,   150,   151,    -1,   153,   154,   155,    -1,
      -1,   158,    -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,
      -1,   168,    -1,    -1,    -1,    -1,   173,    -1,   175,   176,
      -1,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,    -1,    -1,   194,    -1,    -1,
      -1,    -1,   199,   200,    -1,   202,   203,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,    -1,
      -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   107,   108,   109,   110,   111,   112,    -1,    -1,   115,
     116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,   125,
      -1,   127,   128,   129,   130,   131,    -1,    -1,    -1,    -1,
      -1,    -1,   138,    -1,    -1,    -1,    -1,   143,   144,   145,
     146,    -1,   148,    -1,   150,   151,    -1,   153,   154,   155,
      -1,    -1,   158,    -1,    -1,    -1,   162,    -1,    -1,    -1,
      -1,    -1,   168,    -1,    -1,    -1,    -1,   173,    -1,   175,
     176,    -1,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,    -1,    -1,   194,    -1,
      -1,    -1,    -1,   199,   200,    -1,   202,   203,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   107,   108,   109,   110,   111,   112,    -1,    -1,
     115,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,
     125,    -1,   127,   128,   129,   130,   131,    -1,    -1,    -1,
      -1,    -1,    -1,   138,    -1,    -1,    -1,    -1,   143,   144,
     145,   146,    -1,   148,    -1,   150,   151,    -1,   153,   154,
     155,    -1,    -1,   158,    -1,    -1,    -1,   162,    -1,    -1,
      -1,    -1,    -1,   168,    -1,    -1,    -1,    -1,   173,    -1,
     175,   176,    -1,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,    -1,    -1,   194,
      -1,    -1,   197,    -1,   199,   200,    -1,   202,   203,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   107,   108,   109,   110,   111,   112,    -1,
      -1,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     124,   125,    -1,   127,   128,   129,   130,   131,    -1,    -1,
      -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,    -1,   143,
     144,   145,   146,    -1,   148,    -1,   150,   151,    -1,   153,
     154,   155,    -1,    -1,   158,    -1,    -1,    -1,   162,    -1,
      -1,    -1,    -1,    -1,   168,    -1,    -1,    -1,    -1,   173,
      -1,   175,   176,    -1,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,    -1,    -1,
     194,    -1,    -1,    -1,    -1,   199,   200,    -1,   202,   203,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   107,   108,   109,   110,   111,   112,
      -1,    -1,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,   125,    -1,   127,   128,   129,   130,   131,    -1,
      -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,    -1,
     143,   144,   145,   146,    -1,   148,    -1,   150,   151,    -1,
     153,   154,   155,    -1,    -1,   158,    -1,    -1,    -1,   162,
      -1,    -1,    -1,    -1,    -1,   168,    -1,    -1,    -1,    -1,
     173,    -1,   175,   176,    -1,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,    -1,
      -1,   194,    -1,    -1,    -1,    -1,   199,   200,    -1,   202,
     203,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    -1,    70,    71,
      72,    73,    74,    -1,    -1,    -1,    -1,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   107,   108,   109,   110,   111,
     112,    -1,    -1,   115,   116,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   124,   125,    -1,   127,   128,   129,   130,   131,
      -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,
      -1,   143,   144,   145,   146,    -1,   148,    -1,   150,   151,
      -1,   153,   154,   155,    -1,    -1,   158,    -1,    -1,    -1,
     162,    -1,    -1,    -1,    -1,    -1,   168,    -1,    -1,    -1,
      -1,   173,    -1,   175,   176,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
      -1,    -1,   194,    -1,    -1,    -1,    -1,   199,   200,    -1,
     202,   203,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,   110,
     111,   112,    -1,    -1,   115,   116,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   124,   125,    -1,   127,   128,   129,   130,
     131,    -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,
      -1,    -1,   143,   144,   145,   146,    -1,   148,    -1,   150,
     151,    -1,   153,   154,   155,    -1,    -1,   158,    -1,    -1,
      -1,   162,    -1,    -1,    -1,    -1,    -1,   168,    -1,    -1,
      -1,    -1,   173,    -1,   175,   176,    -1,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,    -1,    -1,   194,    10,    11,    12,    -1,   199,   200,
      -1,   202,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,     3,     4,     5,     6,
       7,    -1,    -1,    10,    11,    12,    13,    26,    -1,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,   198,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    63,    64,    65,    66,
      67,    68,    69,    -1,    -1,    72,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   198,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   196,    -1,   124,   125,    -1,
     127,   128,   129,   130,   131,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   143,   144,   145,    -1,
     147,    -1,    -1,   150,   151,    -1,   153,   154,   155,   156,
     157,   158,    -1,    -1,   161,    -1,    -1,    -1,    -1,    -1,
      -1,   168,   169,   182,   171,    -1,   173,   174,   175,   176,
      -1,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    10,    11,    12,    -1,    -1,
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
      43,    44,    45,    46,    47,    48,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
     196,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    -1,    50,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,   196,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,   196,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,   196,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    72,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,   195,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,   195,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    26,    50,   148,    -1,   150,   151,   152,   153,   154,
     155,    -1,    -1,    -1,    -1,   190,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,
      -1,   176,    -1,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,    -1,    72,    -1,   194,
      -1,    -1,   186,   187,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,
      -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   107,   108,   109,   110,   111,   112,    -1,    -1,
      -1,    -1,    -1,    52,   183,    -1,    -1,    -1,    -1,    -1,
     125,   126,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,   143,    -1,
      -1,   146,    -1,   148,    -1,   150,   151,    -1,   153,   154,
     155,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,    -1,
     175,   176,    -1,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   125,   126,    -1,   194,
      -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   143,    -1,    -1,   146,    -1,   148,
      -1,   150,   151,    62,   153,   154,   155,    -1,    -1,    -1,
     159,    70,    71,    72,    -1,    -1,    -1,    -1,    -1,   168,
      -1,    80,    -1,    -1,    -1,    -1,    -1,   176,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,    -1,    -1,    -1,   194,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   125,    -1,   127,   128,
     129,   130,   131,    -1,    43,    44,    -1,    -1,    -1,   138,
      -1,    -1,    -1,    -1,   143,   144,   145,   146,    -1,   148,
      -1,   150,   151,    62,   153,   154,   155,    -1,    -1,   158,
      -1,    70,    71,    72,    -1,    -1,    -1,    -1,    -1,   168,
      -1,    80,    -1,    -1,   173,    -1,    -1,   176,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    63,    64,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    72,    -1,    74,    -1,    -1,   125,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   150,   151,    -1,   153,   154,   155,    -1,    -1,    -1,
      -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   168,
      -1,    -1,    -1,    72,    -1,    74,    -1,   176,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   143,    -1,    -1,   146,    -1,   148,    -1,   150,
     151,    -1,   153,   154,   155,    -1,    -1,    -1,    -1,    -1,
      -1,   162,    -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   176,    -1,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
      72,    -1,    -1,   194,   143,    -1,    -1,   146,   199,   148,
      -1,   150,   151,    -1,   153,   154,   155,    72,    -1,    74,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,    -1,    -1,    -1,   194,    -1,   112,   197,    -1,
     199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,
      -1,   143,    -1,    -1,   146,    -1,   148,    -1,   150,   151,
      -1,   153,   154,   155,    -1,    -1,    -1,    -1,   143,    -1,
      -1,   146,    -1,   148,    -1,   150,   151,    -1,   153,   154,
     155,    72,    -1,    74,   176,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,    -1,
      -1,   176,    -1,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,    -1,    -1,    -1,   194,
      -1,   112,    -1,    -1,   199,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   124,    -1,    72,    -1,    74,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   143,    -1,    -1,   146,    -1,   148,    -1,   150,
     151,    -1,   153,   154,   155,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   176,    -1,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
      72,    -1,    74,   194,    -1,    -1,   143,    -1,   199,   146,
      -1,   148,    -1,   150,   151,    -1,   153,   154,   155,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,
     112,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,    72,    -1,    74,   194,    -1,    -1,
      -1,    -1,   199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   143,    -1,    -1,   146,    -1,   148,    -1,   150,   151,
      -1,   153,   154,   155,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    72,    -1,    74,    -1,    -1,
      -1,    -1,    -1,    -1,   176,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,    -1,
      -1,    -1,   194,    -1,    -1,    -1,    -1,   199,    -1,    -1,
      -1,    -1,   150,   151,    -1,   153,   154,   155,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   150,   151,    -1,   153,   154,   155,   197,
      -1,   199,    -1,    -1,    -1,    -1,    72,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,
      -1,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,    -1,    -1,   143,    -1,    -1,   146,
     197,    -1,   199,   150,   151,    -1,   153,   154,   155,    -1,
     116,    -1,    -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,
      -1,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   150,   151,    -1,   153,   154,   155,
      -1,   198,    -1,    -1,    -1,    -1,   113,    72,    -1,    74,
      75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,    -1,
     176,    -1,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,    -1,   143,    -1,   194,   146,
      -1,   148,    -1,   150,   151,    -1,   153,   154,   155,    -1,
      72,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,
      -1,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,    -1,   150,   151,    -1,   153,   154,
     155,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   176,    -1,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   148,    -1,   150,   151,
      -1,   153,   154,   155,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   176,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,   123,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,   123,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,   123,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,   123,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,   123,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    91,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   205,   206,     0,   207,     3,     4,     5,     6,     7,
      13,    42,    43,    44,    49,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    70,    71,    72,    73,    74,    76,    80,    81,    82,
      83,    85,    87,    89,    92,    96,    97,    98,    99,   100,
     101,   102,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   114,   115,   116,   117,   118,   119,   124,   125,   127,
     128,   129,   130,   131,   138,   143,   144,   145,   146,   147,
     148,   150,   151,   153,   154,   155,   156,   158,   162,   168,
     169,   171,   173,   174,   175,   176,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     194,   196,   197,   199,   200,   202,   203,   208,   211,   214,
     215,   216,   217,   218,   219,   222,   237,   238,   242,   247,
     253,   308,   309,   314,   318,   319,   320,   321,   322,   323,
     324,   325,   327,   330,   339,   340,   341,   343,   344,   346,
     365,   375,   376,   377,   382,   385,   403,   408,   410,   411,
     412,   413,   414,   415,   416,   417,   419,   432,   434,   436,
     110,   111,   112,   124,   143,   211,   237,   308,   324,   410,
     324,   194,   324,   324,   324,   401,   402,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,    74,
     112,   194,   215,   376,   377,   410,   410,    32,   324,   423,
     424,   324,   112,   194,   215,   376,   377,   378,   409,   415,
     420,   421,   194,   315,   379,   194,   315,   331,   316,   324,
     224,   315,   194,   194,   194,   315,   196,   324,   211,   196,
     324,    26,    52,   125,   126,   148,   168,   194,   211,   218,
     437,   447,   448,   177,   196,   321,   324,   345,   347,   197,
     230,   324,   146,   212,   213,   214,    74,   199,   279,   280,
     118,   118,    74,   281,   194,   194,   194,   194,   211,   251,
     438,   194,   194,    74,    79,   139,   140,   141,   429,   430,
     146,   197,   214,   214,    96,   324,   252,   438,   148,   194,
     438,   438,   324,   332,   314,   324,   325,   410,   220,   197,
      79,   380,   429,    79,   429,   429,    27,   146,   164,   439,
     194,     9,   196,    32,   236,   148,   250,   438,   112,   237,
     309,   196,   196,   196,   196,   196,   196,    10,    11,    12,
      26,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    50,   196,    62,    62,   196,   197,   142,   119,
     156,   253,   307,   308,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    60,    61,   122,   405,
     406,    62,   197,   407,   194,    62,   197,   199,   416,   194,
     236,   237,    14,   324,    41,   211,   400,   194,   314,   410,
     142,   410,   123,   201,     9,   387,   314,   410,   439,   142,
     194,   381,   122,   405,   406,   407,   195,   324,    27,   222,
       8,   333,     9,   196,   222,   223,   316,   317,   324,   211,
     265,   226,   196,   196,   196,   448,   448,   164,   194,    99,
     440,   448,    14,   211,    74,   196,   196,   196,   177,   178,
     179,   184,   185,   188,   189,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   360,   361,   362,   231,   103,   161,
     196,   214,     9,   196,    91,   197,   410,     9,   196,    14,
       9,   196,   410,   433,   433,   314,   325,   410,   195,   164,
     245,   124,   410,   422,   423,    62,   122,   139,   430,    73,
     324,   410,    79,   139,   430,   214,   210,   196,   197,   196,
     123,   248,   366,   368,    80,   334,   335,   337,    14,    91,
     435,   159,   275,   276,   403,   404,   195,   195,   195,   198,
     221,   222,   238,   242,   247,   324,   200,   202,   203,   211,
     440,    32,   277,   278,   324,   437,   194,   438,   243,   236,
     324,   324,   324,    27,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   324,
     324,   324,   324,   324,   324,   324,   378,   324,   418,   418,
     324,   425,   426,   118,   197,   211,   415,   416,   251,   252,
     250,   237,    32,   147,   318,   321,   324,   345,   324,   324,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   197,
     211,   415,   418,   324,   277,   418,   324,   422,   236,   195,
     194,   399,     9,   387,   314,   195,   211,    32,   324,    32,
     324,   195,   195,   415,   277,   197,   211,   415,   195,   220,
     269,   197,   324,   324,    83,    27,   222,   263,   196,    91,
      14,     9,   195,    27,   197,   266,   448,    80,   444,   445,
     446,   194,     9,    43,    44,    62,   125,   138,   148,   168,
     176,   215,   216,   218,   342,   376,   382,   383,   384,   180,
      74,   324,    74,    74,   324,   357,   358,   324,   324,   350,
     360,   183,   363,   220,   194,   229,    91,   213,   211,   324,
     280,   383,    74,     9,   195,   195,   195,   195,   195,   196,
     211,   443,   120,   256,   194,     9,   195,   195,    74,    75,
     211,   431,   211,    62,   198,   198,   207,   209,   324,   121,
     255,   163,    47,   148,   163,   370,   123,     9,   387,   195,
     448,   448,    14,   192,     9,   388,   448,   449,   122,   405,
     406,   407,   198,     9,   165,   410,   195,     9,   388,    14,
     328,   239,   120,   254,   194,   438,   324,    27,   201,   201,
     123,   198,     9,   387,   324,   439,   194,   246,   249,   244,
     236,    64,   410,   324,   439,   194,   201,   198,   195,   201,
     198,   195,    43,    44,    62,    70,    71,    80,   125,   138,
     168,   176,   211,   390,   392,   395,   398,   211,   410,   410,
     123,   405,   406,   407,   195,   324,   270,    67,    68,   271,
     220,   315,   220,   317,    32,   124,   260,   410,   383,   211,
      27,   222,   264,   196,   267,   196,   267,     9,   165,   123,
       9,   387,   195,   159,   440,   441,   448,   383,   383,   383,
     386,   389,   194,    79,   142,   194,   194,   142,   197,   324,
     180,   180,    14,   186,   187,   359,     9,   190,   363,    74,
     198,   376,   197,   233,   211,   198,    14,   410,   196,    91,
       9,   165,   257,   376,   197,   422,   124,   410,    14,   201,
     324,   198,   207,   257,   197,   369,    14,   324,   334,   196,
     448,    27,   442,   159,   404,    32,    74,   197,   211,   415,
     448,    32,   324,   383,   275,   194,   376,   255,   329,   240,
     324,   324,   324,   198,   194,   277,   256,   255,   254,   438,
     378,   198,   194,   277,    14,    70,    71,   211,   391,   391,
     392,   393,   394,   194,    79,   139,   194,   194,     9,   387,
     195,   399,    32,   324,   198,    67,    68,   272,   315,   222,
     198,   196,    84,   196,   410,   194,   123,   259,    14,   220,
     267,    93,    94,    95,   267,   198,   448,   448,   444,     9,
     195,   195,   123,   201,     9,   387,   386,   211,   334,   336,
     338,   386,   118,   211,   383,   427,   428,   324,   324,   324,
     358,   324,   348,    74,   234,   383,   448,   211,     9,   282,
     195,   194,   318,   321,   324,   201,   198,   282,   149,   162,
     197,   365,   372,   149,   197,   371,   123,   196,   448,   333,
     449,    74,    14,   324,   439,   194,   410,   195,   275,   197,
     275,   194,   123,   194,   277,   195,   197,   197,   255,   241,
     381,   194,   277,   195,   123,   201,     9,   387,   393,   139,
     334,   396,   397,   393,   392,   410,   315,    27,    69,   222,
     196,   317,   422,   260,   195,   383,    90,    93,   196,   324,
      27,   196,   268,   198,   165,   159,    27,   383,   383,   195,
     123,     9,   387,   195,   195,   123,   198,     9,   387,   181,
     195,   220,    91,   376,     4,   100,   105,   113,   150,   151,
     153,   198,   283,   306,   307,   308,   313,   403,   422,   198,
     198,    47,   324,   324,   324,    32,    74,    14,   383,   198,
     194,   277,   442,   195,   282,   195,   275,   324,   277,   195,
     282,   282,   197,   194,   277,   195,   392,   392,   195,   123,
     195,     9,   387,   195,    27,   220,   196,   195,   195,   227,
     196,   196,   268,   220,   448,   123,   383,   334,   383,   383,
     324,   197,   198,   448,   120,   121,   437,   258,   376,   113,
     125,   148,   154,   292,   293,   294,   376,   152,   298,   299,
     116,   194,   211,   300,   301,   284,   237,   448,     9,   196,
     307,   195,   148,   367,   198,   198,    74,    14,   383,   194,
     277,   195,   105,   326,   442,   198,   442,   195,   195,   198,
     198,   282,   275,   195,   123,   392,   334,   220,   225,    27,
     222,   262,   220,   195,   383,   123,   123,   182,   220,   376,
     376,    14,     9,   196,   197,   197,     9,   196,     3,     4,
       5,     6,     7,    10,    11,    12,    13,    50,    63,    64,
      65,    66,    67,    68,    69,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   124,   125,   127,   128,
     129,   130,   131,   143,   144,   145,   147,   156,   157,   158,
     161,   168,   169,   171,   173,   174,   175,   211,   373,   374,
       9,   196,   148,   152,   211,   301,   302,   303,   196,    74,
     312,   236,   285,   437,   237,    14,   383,   277,   195,   194,
     197,   196,   197,   304,   326,   442,   198,   195,   392,   123,
      27,   222,   261,   220,   383,   383,   324,   198,   196,   196,
     383,   376,   288,   295,   382,   293,    14,    27,    44,   296,
     299,     9,    30,   195,    26,    43,    46,    14,     9,   196,
     438,   312,    14,   236,   383,   195,    32,    74,   364,   220,
     220,   197,   304,   442,   392,   220,    88,   183,   232,   198,
     211,   218,   289,   290,   291,     9,   198,   383,   374,   374,
      52,   297,   302,   302,    26,    43,    46,   383,    74,   194,
     196,   383,   438,    74,     9,   388,   198,   198,   220,   304,
      86,   196,    74,   103,   228,   142,    91,   382,   155,    14,
     286,   194,    32,    74,   195,   198,   196,   194,   161,   235,
     211,   307,   308,   383,   159,   273,   274,   404,   287,    74,
     376,   233,   157,   211,   196,   195,     9,   388,   107,   108,
     109,   310,   311,   273,    74,   258,   196,   442,   159,   404,
     449,   195,   195,   196,   196,   197,   305,   310,    32,    74,
     442,   197,   220,   449,    74,    14,   305,   220,   198,    32,
      74,    14,   383,   198,    74,    14,   383,    14,   383,   383
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
#line 725 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 728 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 735 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));}
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 736 "hphp.y"
    { }
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 739 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num()); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 740 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 741 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 742 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 743 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 744 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 747 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();}
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 749 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());}
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 750 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);}
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 751 "hphp.y"
    { _p->onNamespaceStart("");}
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 752 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);}
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 753 "hphp.y"
    { _p->nns(); (yyval).reset();}
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 754 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;}
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 759 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 760 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 761 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 762 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 763 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 764 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 765 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 766 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 769 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 771 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 778 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 783 "hphp.y"
    { }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 784 "hphp.y"
    { }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 787 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");}
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 788 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");}
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 789 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());}
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 791 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());}
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;}
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 800 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;}
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 802 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));}
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 803 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;}
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 806 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 813 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 820 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 828 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 831 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 837 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 838 "hphp.y"
    { _p->onStatementListStart((yyval));}
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 842 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 843 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 847 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 851 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 856 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));}
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 857 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 859 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 863 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 866 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 870 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 872 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 875 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 877 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 880 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);}
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 881 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));}
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 882 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);}
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 883 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));}
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 884 "hphp.y"
    { _p->onReturn((yyval), NULL);}
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 885 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));}
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 886 "hphp.y"
    { _p->onYieldBreak((yyval));}
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 887 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 888 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 889 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);}
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 890 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));}
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 891 "hphp.y"
    { (yyval).reset(); (yyval) = ';';}
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 892 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);}
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 895 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 897 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 901 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;}
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 908 "hphp.y"
    { _p->onCompleteLabelScope(false);}
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 909 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));}
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 912 "hphp.y"
    { _p->onCompleteLabelScope(false);}
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 913 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));}
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 914 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 915 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval));}
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 919 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 920 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 921 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 922 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 923 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 924 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 925 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); }
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 926 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 927 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 928 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 929 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 937 "hphp.y"
    { _p->onNewLabelScope(false);}
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 938 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 947 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));}
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 948 "hphp.y"
    { (yyval).reset();}
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 952 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 954 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 960 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 961 "hphp.y"
    { (yyval).reset();}
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 965 "hphp.y"
    { (yyval) = 1;}
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 966 "hphp.y"
    { (yyval).reset();}
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 970 "hphp.y"
    { _p->pushFuncLocation(); }
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 975 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();}
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 981 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 987 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();}
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 993 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 999 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();}
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 1005 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 1013 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));}
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 1016 "hphp.y"
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
#line 1031 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));}
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 1034 "hphp.y"
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
#line 1048 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));}
    break;

  case 127:

/* Line 1806 of yacc.c  */
#line 1051 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 128:

/* Line 1806 of yacc.c  */
#line 1056 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));}
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 1059 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 130:

/* Line 1806 of yacc.c  */
#line 1066 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));}
    break;

  case 131:

/* Line 1806 of yacc.c  */
#line 1069 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0);
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 1077 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));}
    break;

  case 133:

/* Line 1806 of yacc.c  */
#line 1080 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 1088 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 1089 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 136:

/* Line 1806 of yacc.c  */
#line 1093 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 1096 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 1099 "hphp.y"
    { (yyval) = T_CLASS;}
    break;

  case 139:

/* Line 1806 of yacc.c  */
#line 1100 "hphp.y"
    { (yyval) = T_ABSTRACT;}
    break;

  case 140:

/* Line 1806 of yacc.c  */
#line 1101 "hphp.y"
    { (yyval) = T_FINAL;}
    break;

  case 141:

/* Line 1806 of yacc.c  */
#line 1105 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 142:

/* Line 1806 of yacc.c  */
#line 1106 "hphp.y"
    { (yyval).reset();}
    break;

  case 143:

/* Line 1806 of yacc.c  */
#line 1109 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 1110 "hphp.y"
    { (yyval).reset();}
    break;

  case 145:

/* Line 1806 of yacc.c  */
#line 1113 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 146:

/* Line 1806 of yacc.c  */
#line 1114 "hphp.y"
    { (yyval).reset();}
    break;

  case 147:

/* Line 1806 of yacc.c  */
#line 1117 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 148:

/* Line 1806 of yacc.c  */
#line 1119 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 149:

/* Line 1806 of yacc.c  */
#line 1122 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 150:

/* Line 1806 of yacc.c  */
#line 1124 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 151:

/* Line 1806 of yacc.c  */
#line 1128 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 152:

/* Line 1806 of yacc.c  */
#line 1129 "hphp.y"
    { (yyval).reset();}
    break;

  case 153:

/* Line 1806 of yacc.c  */
#line 1132 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 154:

/* Line 1806 of yacc.c  */
#line 1133 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;}
    break;

  case 155:

/* Line 1806 of yacc.c  */
#line 1134 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);}
    break;

  case 156:

/* Line 1806 of yacc.c  */
#line 1138 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 157:

/* Line 1806 of yacc.c  */
#line 1140 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 158:

/* Line 1806 of yacc.c  */
#line 1143 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 159:

/* Line 1806 of yacc.c  */
#line 1145 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 160:

/* Line 1806 of yacc.c  */
#line 1148 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 161:

/* Line 1806 of yacc.c  */
#line 1150 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 162:

/* Line 1806 of yacc.c  */
#line 1153 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 163:

/* Line 1806 of yacc.c  */
#line 1155 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 166:

/* Line 1806 of yacc.c  */
#line 1165 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 167:

/* Line 1806 of yacc.c  */
#line 1166 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 168:

/* Line 1806 of yacc.c  */
#line 1167 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 169:

/* Line 1806 of yacc.c  */
#line 1168 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);}
    break;

  case 170:

/* Line 1806 of yacc.c  */
#line 1173 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 171:

/* Line 1806 of yacc.c  */
#line 1175 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));}
    break;

  case 172:

/* Line 1806 of yacc.c  */
#line 1176 "hphp.y"
    { (yyval).reset();}
    break;

  case 173:

/* Line 1806 of yacc.c  */
#line 1179 "hphp.y"
    { (yyval).reset();}
    break;

  case 174:

/* Line 1806 of yacc.c  */
#line 1180 "hphp.y"
    { (yyval).reset();}
    break;

  case 175:

/* Line 1806 of yacc.c  */
#line 1185 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 176:

/* Line 1806 of yacc.c  */
#line 1186 "hphp.y"
    { (yyval).reset();}
    break;

  case 177:

/* Line 1806 of yacc.c  */
#line 1191 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 178:

/* Line 1806 of yacc.c  */
#line 1192 "hphp.y"
    { (yyval).reset();}
    break;

  case 179:

/* Line 1806 of yacc.c  */
#line 1195 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 180:

/* Line 1806 of yacc.c  */
#line 1196 "hphp.y"
    { (yyval).reset();}
    break;

  case 181:

/* Line 1806 of yacc.c  */
#line 1199 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);}
    break;

  case 182:

/* Line 1806 of yacc.c  */
#line 1200 "hphp.y"
    { (yyval).reset();}
    break;

  case 183:

/* Line 1806 of yacc.c  */
#line 1205 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]);}
    break;

  case 184:

/* Line 1806 of yacc.c  */
#line 1207 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 185:

/* Line 1806 of yacc.c  */
#line 1208 "hphp.y"
    { (yyval).reset();}
    break;

  case 186:

/* Line 1806 of yacc.c  */
#line 1209 "hphp.y"
    { (yyval).reset();}
    break;

  case 187:

/* Line 1806 of yacc.c  */
#line 1215 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));}
    break;

  case 188:

/* Line 1806 of yacc.c  */
#line 1219 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));}
    break;

  case 189:

/* Line 1806 of yacc.c  */
#line 1224 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));}
    break;

  case 190:

/* Line 1806 of yacc.c  */
#line 1229 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));}
    break;

  case 191:

/* Line 1806 of yacc.c  */
#line 1234 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));}
    break;

  case 192:

/* Line 1806 of yacc.c  */
#line 1239 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));}
    break;

  case 193:

/* Line 1806 of yacc.c  */
#line 1245 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));}
    break;

  case 194:

/* Line 1806 of yacc.c  */
#line 1251 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));}
    break;

  case 195:

/* Line 1806 of yacc.c  */
#line 1257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]);}
    break;

  case 196:

/* Line 1806 of yacc.c  */
#line 1259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 197:

/* Line 1806 of yacc.c  */
#line 1260 "hphp.y"
    { (yyval).reset();}
    break;

  case 198:

/* Line 1806 of yacc.c  */
#line 1261 "hphp.y"
    { (yyval).reset();}
    break;

  case 199:

/* Line 1806 of yacc.c  */
#line 1266 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),0,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL);}
    break;

  case 200:

/* Line 1806 of yacc.c  */
#line 1269 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),1,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL);}
    break;

  case 201:

/* Line 1806 of yacc.c  */
#line 1273 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),1,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL);}
    break;

  case 202:

/* Line 1806 of yacc.c  */
#line 1277 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),0,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL);}
    break;

  case 203:

/* Line 1806 of yacc.c  */
#line 1281 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),0,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL);}
    break;

  case 204:

/* Line 1806 of yacc.c  */
#line 1285 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),1,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL);}
    break;

  case 205:

/* Line 1806 of yacc.c  */
#line 1290 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),1,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL);}
    break;

  case 206:

/* Line 1806 of yacc.c  */
#line 1295 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),0,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL);}
    break;

  case 207:

/* Line 1806 of yacc.c  */
#line 1301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 208:

/* Line 1806 of yacc.c  */
#line 1302 "hphp.y"
    { (yyval).reset();}
    break;

  case 209:

/* Line 1806 of yacc.c  */
#line 1305 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),0);}
    break;

  case 210:

/* Line 1806 of yacc.c  */
#line 1306 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),1);}
    break;

  case 211:

/* Line 1806 of yacc.c  */
#line 1308 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 212:

/* Line 1806 of yacc.c  */
#line 1310 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 213:

/* Line 1806 of yacc.c  */
#line 1314 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 214:

/* Line 1806 of yacc.c  */
#line 1315 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 215:

/* Line 1806 of yacc.c  */
#line 1318 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 216:

/* Line 1806 of yacc.c  */
#line 1319 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;}
    break;

  case 217:

/* Line 1806 of yacc.c  */
#line 1320 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;}
    break;

  case 218:

/* Line 1806 of yacc.c  */
#line 1324 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 219:

/* Line 1806 of yacc.c  */
#line 1326 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));}
    break;

  case 220:

/* Line 1806 of yacc.c  */
#line 1327 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);}
    break;

  case 221:

/* Line 1806 of yacc.c  */
#line 1328 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 222:

/* Line 1806 of yacc.c  */
#line 1333 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 223:

/* Line 1806 of yacc.c  */
#line 1334 "hphp.y"
    { (yyval).reset();}
    break;

  case 224:

/* Line 1806 of yacc.c  */
#line 1337 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));}
    break;

  case 225:

/* Line 1806 of yacc.c  */
#line 1338 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 226:

/* Line 1806 of yacc.c  */
#line 1341 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));}
    break;

  case 227:

/* Line 1806 of yacc.c  */
#line 1342 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));}
    break;

  case 228:

/* Line 1806 of yacc.c  */
#line 1344 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);}
    break;

  case 229:

/* Line 1806 of yacc.c  */
#line 1348 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();}
    break;

  case 230:

/* Line 1806 of yacc.c  */
#line 1354 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 231:

/* Line 1806 of yacc.c  */
#line 1361 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();}
    break;

  case 232:

/* Line 1806 of yacc.c  */
#line 1367 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 233:

/* Line 1806 of yacc.c  */
#line 1372 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));}
    break;

  case 234:

/* Line 1806 of yacc.c  */
#line 1374 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));}
    break;

  case 235:

/* Line 1806 of yacc.c  */
#line 1376 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));}
    break;

  case 236:

/* Line 1806 of yacc.c  */
#line 1378 "hphp.y"
    { _p->onTraitRequire((yyval), (yyvsp[(3) - (4)]), true); }
    break;

  case 237:

/* Line 1806 of yacc.c  */
#line 1380 "hphp.y"
    { _p->onTraitRequire((yyval), (yyvsp[(3) - (4)]), false); }
    break;

  case 238:

/* Line 1806 of yacc.c  */
#line 1381 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); }
    break;

  case 239:

/* Line 1806 of yacc.c  */
#line 1384 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); }
    break;

  case 240:

/* Line 1806 of yacc.c  */
#line 1387 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); }
    break;

  case 241:

/* Line 1806 of yacc.c  */
#line 1388 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); }
    break;

  case 242:

/* Line 1806 of yacc.c  */
#line 1389 "hphp.y"
    { (yyval).reset(); }
    break;

  case 243:

/* Line 1806 of yacc.c  */
#line 1395 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 244:

/* Line 1806 of yacc.c  */
#line 1399 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));}
    break;

  case 245:

/* Line 1806 of yacc.c  */
#line 1402 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);}
    break;

  case 246:

/* Line 1806 of yacc.c  */
#line 1409 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 247:

/* Line 1806 of yacc.c  */
#line 1410 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));}
    break;

  case 248:

/* Line 1806 of yacc.c  */
#line 1415 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));}
    break;

  case 249:

/* Line 1806 of yacc.c  */
#line 1418 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 250:

/* Line 1806 of yacc.c  */
#line 1425 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;}
    break;

  case 251:

/* Line 1806 of yacc.c  */
#line 1427 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;}
    break;

  case 252:

/* Line 1806 of yacc.c  */
#line 1431 "hphp.y"
    { (yyval) = 4;}
    break;

  case 253:

/* Line 1806 of yacc.c  */
#line 1432 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));}
    break;

  case 254:

/* Line 1806 of yacc.c  */
#line 1438 "hphp.y"
    { (yyval) = 6;}
    break;

  case 255:

/* Line 1806 of yacc.c  */
#line 1440 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;}
    break;

  case 256:

/* Line 1806 of yacc.c  */
#line 1444 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);}
    break;

  case 257:

/* Line 1806 of yacc.c  */
#line 1446 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);}
    break;

  case 258:

/* Line 1806 of yacc.c  */
#line 1450 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 259:

/* Line 1806 of yacc.c  */
#line 1451 "hphp.y"
    { scalar_null(_p, (yyval));}
    break;

  case 260:

/* Line 1806 of yacc.c  */
#line 1455 "hphp.y"
    { scalar_num(_p, (yyval), "1");}
    break;

  case 261:

/* Line 1806 of yacc.c  */
#line 1456 "hphp.y"
    { scalar_num(_p, (yyval), "0");}
    break;

  case 262:

/* Line 1806 of yacc.c  */
#line 1460 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);}
    break;

  case 263:

/* Line 1806 of yacc.c  */
#line 1463 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);}
    break;

  case 264:

/* Line 1806 of yacc.c  */
#line 1468 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 265:

/* Line 1806 of yacc.c  */
#line 1473 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;}
    break;

  case 266:

/* Line 1806 of yacc.c  */
#line 1474 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;}
    break;

  case 267:

/* Line 1806 of yacc.c  */
#line 1476 "hphp.y"
    { (yyval) = 0;}
    break;

  case 268:

/* Line 1806 of yacc.c  */
#line 1480 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);}
    break;

  case 269:

/* Line 1806 of yacc.c  */
#line 1481 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);}
    break;

  case 270:

/* Line 1806 of yacc.c  */
#line 1482 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);}
    break;

  case 271:

/* Line 1806 of yacc.c  */
#line 1483 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);}
    break;

  case 272:

/* Line 1806 of yacc.c  */
#line 1487 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 273:

/* Line 1806 of yacc.c  */
#line 1488 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);}
    break;

  case 274:

/* Line 1806 of yacc.c  */
#line 1489 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);}
    break;

  case 275:

/* Line 1806 of yacc.c  */
#line 1490 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);}
    break;

  case 276:

/* Line 1806 of yacc.c  */
#line 1491 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);}
    break;

  case 277:

/* Line 1806 of yacc.c  */
#line 1493 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));}
    break;

  case 278:

/* Line 1806 of yacc.c  */
#line 1495 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));}
    break;

  case 279:

/* Line 1806 of yacc.c  */
#line 1499 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;}
    break;

  case 280:

/* Line 1806 of yacc.c  */
#line 1502 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;}
    break;

  case 281:

/* Line 1806 of yacc.c  */
#line 1503 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;}
    break;

  case 282:

/* Line 1806 of yacc.c  */
#line 1507 "hphp.y"
    { (yyval).reset();}
    break;

  case 283:

/* Line 1806 of yacc.c  */
#line 1508 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;}
    break;

  case 284:

/* Line 1806 of yacc.c  */
#line 1512 "hphp.y"
    { (yyval).reset();}
    break;

  case 285:

/* Line 1806 of yacc.c  */
#line 1513 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;}
    break;

  case 286:

/* Line 1806 of yacc.c  */
#line 1516 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 287:

/* Line 1806 of yacc.c  */
#line 1517 "hphp.y"
    { (yyval).reset();}
    break;

  case 288:

/* Line 1806 of yacc.c  */
#line 1520 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 289:

/* Line 1806 of yacc.c  */
#line 1521 "hphp.y"
    { (yyval).reset();}
    break;

  case 290:

/* Line 1806 of yacc.c  */
#line 1524 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));}
    break;

  case 291:

/* Line 1806 of yacc.c  */
#line 1526 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 292:

/* Line 1806 of yacc.c  */
#line 1529 "hphp.y"
    { (yyval) = T_PUBLIC;}
    break;

  case 293:

/* Line 1806 of yacc.c  */
#line 1530 "hphp.y"
    { (yyval) = T_PROTECTED;}
    break;

  case 294:

/* Line 1806 of yacc.c  */
#line 1531 "hphp.y"
    { (yyval) = T_PRIVATE;}
    break;

  case 295:

/* Line 1806 of yacc.c  */
#line 1532 "hphp.y"
    { (yyval) = T_STATIC;}
    break;

  case 296:

/* Line 1806 of yacc.c  */
#line 1533 "hphp.y"
    { (yyval) = T_ABSTRACT;}
    break;

  case 297:

/* Line 1806 of yacc.c  */
#line 1534 "hphp.y"
    { (yyval) = T_FINAL;}
    break;

  case 298:

/* Line 1806 of yacc.c  */
#line 1535 "hphp.y"
    { (yyval) = T_ASYNC;}
    break;

  case 299:

/* Line 1806 of yacc.c  */
#line 1539 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 300:

/* Line 1806 of yacc.c  */
#line 1540 "hphp.y"
    { (yyval).reset();}
    break;

  case 301:

/* Line 1806 of yacc.c  */
#line 1543 "hphp.y"
    { (yyval) = T_PUBLIC;}
    break;

  case 302:

/* Line 1806 of yacc.c  */
#line 1544 "hphp.y"
    { (yyval) = T_PROTECTED;}
    break;

  case 303:

/* Line 1806 of yacc.c  */
#line 1545 "hphp.y"
    { (yyval) = T_PRIVATE;}
    break;

  case 304:

/* Line 1806 of yacc.c  */
#line 1549 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 305:

/* Line 1806 of yacc.c  */
#line 1551 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));}
    break;

  case 306:

/* Line 1806 of yacc.c  */
#line 1552 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);}
    break;

  case 307:

/* Line 1806 of yacc.c  */
#line 1553 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 308:

/* Line 1806 of yacc.c  */
#line 1557 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 309:

/* Line 1806 of yacc.c  */
#line 1558 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 310:

/* Line 1806 of yacc.c  */
#line 1562 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 311:

/* Line 1806 of yacc.c  */
#line 1564 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 312:

/* Line 1806 of yacc.c  */
#line 1565 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);}
    break;

  case 313:

/* Line 1806 of yacc.c  */
#line 1566 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 314:

/* Line 1806 of yacc.c  */
#line 1567 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 315:

/* Line 1806 of yacc.c  */
#line 1570 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 316:

/* Line 1806 of yacc.c  */
#line 1574 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 317:

/* Line 1806 of yacc.c  */
#line 1575 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 318:

/* Line 1806 of yacc.c  */
#line 1579 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 319:

/* Line 1806 of yacc.c  */
#line 1580 "hphp.y"
    { (yyval).reset();}
    break;

  case 320:

/* Line 1806 of yacc.c  */
#line 1584 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));}
    break;

  case 321:

/* Line 1806 of yacc.c  */
#line 1585 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 322:

/* Line 1806 of yacc.c  */
#line 1589 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 323:

/* Line 1806 of yacc.c  */
#line 1594 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);}
    break;

  case 324:

/* Line 1806 of yacc.c  */
#line 1598 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 325:

/* Line 1806 of yacc.c  */
#line 1602 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 326:

/* Line 1806 of yacc.c  */
#line 1607 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);}
    break;

  case 327:

/* Line 1806 of yacc.c  */
#line 1611 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 328:

/* Line 1806 of yacc.c  */
#line 1612 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 329:

/* Line 1806 of yacc.c  */
#line 1613 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 330:

/* Line 1806 of yacc.c  */
#line 1617 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));}
    break;

  case 331:

/* Line 1806 of yacc.c  */
#line 1618 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 332:

/* Line 1806 of yacc.c  */
#line 1619 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);}
    break;

  case 333:

/* Line 1806 of yacc.c  */
#line 1622 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));}
    break;

  case 334:

/* Line 1806 of yacc.c  */
#line 1623 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);}
    break;

  case 335:

/* Line 1806 of yacc.c  */
#line 1624 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);}
    break;

  case 336:

/* Line 1806 of yacc.c  */
#line 1625 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);}
    break;

  case 337:

/* Line 1806 of yacc.c  */
#line 1626 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);}
    break;

  case 338:

/* Line 1806 of yacc.c  */
#line 1627 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);}
    break;

  case 339:

/* Line 1806 of yacc.c  */
#line 1628 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);}
    break;

  case 340:

/* Line 1806 of yacc.c  */
#line 1629 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);}
    break;

  case 341:

/* Line 1806 of yacc.c  */
#line 1630 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);}
    break;

  case 342:

/* Line 1806 of yacc.c  */
#line 1631 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);}
    break;

  case 343:

/* Line 1806 of yacc.c  */
#line 1632 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);}
    break;

  case 344:

/* Line 1806 of yacc.c  */
#line 1633 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);}
    break;

  case 345:

/* Line 1806 of yacc.c  */
#line 1634 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);}
    break;

  case 346:

/* Line 1806 of yacc.c  */
#line 1635 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);}
    break;

  case 347:

/* Line 1806 of yacc.c  */
#line 1636 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);}
    break;

  case 348:

/* Line 1806 of yacc.c  */
#line 1637 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);}
    break;

  case 349:

/* Line 1806 of yacc.c  */
#line 1638 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);}
    break;

  case 350:

/* Line 1806 of yacc.c  */
#line 1639 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);}
    break;

  case 351:

/* Line 1806 of yacc.c  */
#line 1640 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);}
    break;

  case 352:

/* Line 1806 of yacc.c  */
#line 1641 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);}
    break;

  case 353:

/* Line 1806 of yacc.c  */
#line 1642 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);}
    break;

  case 354:

/* Line 1806 of yacc.c  */
#line 1643 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');}
    break;

  case 355:

/* Line 1806 of yacc.c  */
#line 1644 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');}
    break;

  case 356:

/* Line 1806 of yacc.c  */
#line 1645 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');}
    break;

  case 357:

/* Line 1806 of yacc.c  */
#line 1646 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');}
    break;

  case 358:

/* Line 1806 of yacc.c  */
#line 1647 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');}
    break;

  case 359:

/* Line 1806 of yacc.c  */
#line 1648 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');}
    break;

  case 360:

/* Line 1806 of yacc.c  */
#line 1649 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');}
    break;

  case 361:

/* Line 1806 of yacc.c  */
#line 1650 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');}
    break;

  case 362:

/* Line 1806 of yacc.c  */
#line 1651 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');}
    break;

  case 363:

/* Line 1806 of yacc.c  */
#line 1652 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);}
    break;

  case 364:

/* Line 1806 of yacc.c  */
#line 1653 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);}
    break;

  case 365:

/* Line 1806 of yacc.c  */
#line 1654 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 366:

/* Line 1806 of yacc.c  */
#line 1655 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 367:

/* Line 1806 of yacc.c  */
#line 1656 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);}
    break;

  case 368:

/* Line 1806 of yacc.c  */
#line 1657 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);}
    break;

  case 369:

/* Line 1806 of yacc.c  */
#line 1658 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);}
    break;

  case 370:

/* Line 1806 of yacc.c  */
#line 1659 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);}
    break;

  case 371:

/* Line 1806 of yacc.c  */
#line 1660 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);}
    break;

  case 372:

/* Line 1806 of yacc.c  */
#line 1661 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);}
    break;

  case 373:

/* Line 1806 of yacc.c  */
#line 1662 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');}
    break;

  case 374:

/* Line 1806 of yacc.c  */
#line 1663 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);}
    break;

  case 375:

/* Line 1806 of yacc.c  */
#line 1665 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');}
    break;

  case 376:

/* Line 1806 of yacc.c  */
#line 1666 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);}
    break;

  case 377:

/* Line 1806 of yacc.c  */
#line 1669 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);}
    break;

  case 378:

/* Line 1806 of yacc.c  */
#line 1670 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 379:

/* Line 1806 of yacc.c  */
#line 1671 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));}
    break;

  case 380:

/* Line 1806 of yacc.c  */
#line 1672 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));}
    break;

  case 381:

/* Line 1806 of yacc.c  */
#line 1673 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 382:

/* Line 1806 of yacc.c  */
#line 1674 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);}
    break;

  case 383:

/* Line 1806 of yacc.c  */
#line 1675 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);}
    break;

  case 384:

/* Line 1806 of yacc.c  */
#line 1676 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);}
    break;

  case 385:

/* Line 1806 of yacc.c  */
#line 1677 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);}
    break;

  case 386:

/* Line 1806 of yacc.c  */
#line 1678 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);}
    break;

  case 387:

/* Line 1806 of yacc.c  */
#line 1679 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);}
    break;

  case 388:

/* Line 1806 of yacc.c  */
#line 1680 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);}
    break;

  case 389:

/* Line 1806 of yacc.c  */
#line 1681 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);}
    break;

  case 390:

/* Line 1806 of yacc.c  */
#line 1682 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);}
    break;

  case 391:

/* Line 1806 of yacc.c  */
#line 1683 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 392:

/* Line 1806 of yacc.c  */
#line 1684 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 393:

/* Line 1806 of yacc.c  */
#line 1685 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 394:

/* Line 1806 of yacc.c  */
#line 1686 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));}
    break;

  case 395:

/* Line 1806 of yacc.c  */
#line 1687 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);}
    break;

  case 396:

/* Line 1806 of yacc.c  */
#line 1688 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 397:

/* Line 1806 of yacc.c  */
#line 1689 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 398:

/* Line 1806 of yacc.c  */
#line 1690 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 399:

/* Line 1806 of yacc.c  */
#line 1697 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);}
    break;

  case 400:

/* Line 1806 of yacc.c  */
#line 1698 "hphp.y"
    { (yyval).reset();}
    break;

  case 401:

/* Line 1806 of yacc.c  */
#line 1703 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
    break;

  case 402:

/* Line 1806 of yacc.c  */
#line 1709 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 403:

/* Line 1806 of yacc.c  */
#line 1717 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
    break;

  case 404:

/* Line 1806 of yacc.c  */
#line 1723 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 405:

/* Line 1806 of yacc.c  */
#line 1732 "hphp.y"
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
#line 1740 "hphp.y"
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
#line 1747 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
    break;

  case 408:

/* Line 1806 of yacc.c  */
#line 1755 "hphp.y"
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
#line 1765 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));}
    break;

  case 410:

/* Line 1806 of yacc.c  */
#line 1767 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 411:

/* Line 1806 of yacc.c  */
#line 1771 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); }
    break;

  case 412:

/* Line 1806 of yacc.c  */
#line 1779 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 413:

/* Line 1806 of yacc.c  */
#line 1782 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 414:

/* Line 1806 of yacc.c  */
#line 1789 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 415:

/* Line 1806 of yacc.c  */
#line 1792 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 416:

/* Line 1806 of yacc.c  */
#line 1797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 417:

/* Line 1806 of yacc.c  */
#line 1798 "hphp.y"
    { (yyval).reset(); }
    break;

  case 418:

/* Line 1806 of yacc.c  */
#line 1803 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 419:

/* Line 1806 of yacc.c  */
#line 1804 "hphp.y"
    { (yyval).reset(); }
    break;

  case 420:

/* Line 1806 of yacc.c  */
#line 1808 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);}
    break;

  case 421:

/* Line 1806 of yacc.c  */
#line 1812 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 422:

/* Line 1806 of yacc.c  */
#line 1813 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 423:

/* Line 1806 of yacc.c  */
#line 1818 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);}
    break;

  case 424:

/* Line 1806 of yacc.c  */
#line 1825 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);}
    break;

  case 425:

/* Line 1806 of yacc.c  */
#line 1832 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 426:

/* Line 1806 of yacc.c  */
#line 1834 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 427:

/* Line 1806 of yacc.c  */
#line 1838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 428:

/* Line 1806 of yacc.c  */
#line 1839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 429:

/* Line 1806 of yacc.c  */
#line 1840 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 430:

/* Line 1806 of yacc.c  */
#line 1844 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); }
    break;

  case 431:

/* Line 1806 of yacc.c  */
#line 1848 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 432:

/* Line 1806 of yacc.c  */
#line 1852 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 433:

/* Line 1806 of yacc.c  */
#line 1857 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); }
    break;

  case 434:

/* Line 1806 of yacc.c  */
#line 1859 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); }
    break;

  case 435:

/* Line 1806 of yacc.c  */
#line 1861 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); }
    break;

  case 436:

/* Line 1806 of yacc.c  */
#line 1863 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); }
    break;

  case 437:

/* Line 1806 of yacc.c  */
#line 1867 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); }
    break;

  case 438:

/* Line 1806 of yacc.c  */
#line 1869 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); }
    break;

  case 439:

/* Line 1806 of yacc.c  */
#line 1873 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 440:

/* Line 1806 of yacc.c  */
#line 1874 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 441:

/* Line 1806 of yacc.c  */
#line 1875 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 442:

/* Line 1806 of yacc.c  */
#line 1876 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 443:

/* Line 1806 of yacc.c  */
#line 1877 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 444:

/* Line 1806 of yacc.c  */
#line 1878 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 445:

/* Line 1806 of yacc.c  */
#line 1882 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 446:

/* Line 1806 of yacc.c  */
#line 1886 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 447:

/* Line 1806 of yacc.c  */
#line 1890 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 448:

/* Line 1806 of yacc.c  */
#line 1895 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); }
    break;

  case 449:

/* Line 1806 of yacc.c  */
#line 1900 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); }
    break;

  case 450:

/* Line 1806 of yacc.c  */
#line 1904 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 451:

/* Line 1806 of yacc.c  */
#line 1908 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); }
    break;

  case 452:

/* Line 1806 of yacc.c  */
#line 1909 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); }
    break;

  case 453:

/* Line 1806 of yacc.c  */
#line 1913 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); }
    break;

  case 454:

/* Line 1806 of yacc.c  */
#line 1914 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); }
    break;

  case 455:

/* Line 1806 of yacc.c  */
#line 1918 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 456:

/* Line 1806 of yacc.c  */
#line 1919 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 457:

/* Line 1806 of yacc.c  */
#line 1923 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 458:

/* Line 1806 of yacc.c  */
#line 1924 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 459:

/* Line 1806 of yacc.c  */
#line 1928 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 460:

/* Line 1806 of yacc.c  */
#line 1932 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 461:

/* Line 1806 of yacc.c  */
#line 1936 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); }
    break;

  case 462:

/* Line 1806 of yacc.c  */
#line 1940 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 463:

/* Line 1806 of yacc.c  */
#line 1941 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 464:

/* Line 1806 of yacc.c  */
#line 1942 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 465:

/* Line 1806 of yacc.c  */
#line 1943 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);}
    break;

  case 466:

/* Line 1806 of yacc.c  */
#line 1950 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));}
    break;

  case 467:

/* Line 1806 of yacc.c  */
#line 1953 "hphp.y"
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
#line 1964 "hphp.y"
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
#line 1975 "hphp.y"
    { (yyval).reset(); (yyval).setText("");}
    break;

  case 470:

/* Line 1806 of yacc.c  */
#line 1976 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));}
    break;

  case 471:

/* Line 1806 of yacc.c  */
#line 1981 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);}
    break;

  case 472:

/* Line 1806 of yacc.c  */
#line 1982 "hphp.y"
    { (yyval).reset();}
    break;

  case 473:

/* Line 1806 of yacc.c  */
#line 1985 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);}
    break;

  case 474:

/* Line 1806 of yacc.c  */
#line 1986 "hphp.y"
    { (yyval).reset();}
    break;

  case 475:

/* Line 1806 of yacc.c  */
#line 1989 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 476:

/* Line 1806 of yacc.c  */
#line 1993 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 477:

/* Line 1806 of yacc.c  */
#line 1996 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 478:

/* Line 1806 of yacc.c  */
#line 1999 "hphp.y"
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
#line 2006 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 480:

/* Line 1806 of yacc.c  */
#line 2007 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 481:

/* Line 1806 of yacc.c  */
#line 2011 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 482:

/* Line 1806 of yacc.c  */
#line 2013 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);}
    break;

  case 483:

/* Line 1806 of yacc.c  */
#line 2015 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);}
    break;

  case 484:

/* Line 1806 of yacc.c  */
#line 2018 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 485:

/* Line 1806 of yacc.c  */
#line 2019 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 486:

/* Line 1806 of yacc.c  */
#line 2020 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 487:

/* Line 1806 of yacc.c  */
#line 2021 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 488:

/* Line 1806 of yacc.c  */
#line 2022 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 489:

/* Line 1806 of yacc.c  */
#line 2023 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 490:

/* Line 1806 of yacc.c  */
#line 2024 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 491:

/* Line 1806 of yacc.c  */
#line 2025 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 492:

/* Line 1806 of yacc.c  */
#line 2026 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 493:

/* Line 1806 of yacc.c  */
#line 2027 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 494:

/* Line 1806 of yacc.c  */
#line 2028 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 495:

/* Line 1806 of yacc.c  */
#line 2029 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 496:

/* Line 1806 of yacc.c  */
#line 2030 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 497:

/* Line 1806 of yacc.c  */
#line 2031 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 498:

/* Line 1806 of yacc.c  */
#line 2032 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 499:

/* Line 1806 of yacc.c  */
#line 2033 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 500:

/* Line 1806 of yacc.c  */
#line 2034 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 501:

/* Line 1806 of yacc.c  */
#line 2035 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 502:

/* Line 1806 of yacc.c  */
#line 2036 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 503:

/* Line 1806 of yacc.c  */
#line 2037 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 504:

/* Line 1806 of yacc.c  */
#line 2038 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 505:

/* Line 1806 of yacc.c  */
#line 2039 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 506:

/* Line 1806 of yacc.c  */
#line 2040 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 507:

/* Line 1806 of yacc.c  */
#line 2041 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 508:

/* Line 1806 of yacc.c  */
#line 2042 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 509:

/* Line 1806 of yacc.c  */
#line 2043 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 510:

/* Line 1806 of yacc.c  */
#line 2044 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 511:

/* Line 1806 of yacc.c  */
#line 2045 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 512:

/* Line 1806 of yacc.c  */
#line 2046 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 513:

/* Line 1806 of yacc.c  */
#line 2047 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 514:

/* Line 1806 of yacc.c  */
#line 2048 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 515:

/* Line 1806 of yacc.c  */
#line 2049 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 516:

/* Line 1806 of yacc.c  */
#line 2050 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 517:

/* Line 1806 of yacc.c  */
#line 2051 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 518:

/* Line 1806 of yacc.c  */
#line 2052 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 519:

/* Line 1806 of yacc.c  */
#line 2053 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 520:

/* Line 1806 of yacc.c  */
#line 2054 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 521:

/* Line 1806 of yacc.c  */
#line 2055 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 522:

/* Line 1806 of yacc.c  */
#line 2056 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 523:

/* Line 1806 of yacc.c  */
#line 2057 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 524:

/* Line 1806 of yacc.c  */
#line 2058 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 525:

/* Line 1806 of yacc.c  */
#line 2059 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 526:

/* Line 1806 of yacc.c  */
#line 2060 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 527:

/* Line 1806 of yacc.c  */
#line 2061 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 528:

/* Line 1806 of yacc.c  */
#line 2062 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 529:

/* Line 1806 of yacc.c  */
#line 2063 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 530:

/* Line 1806 of yacc.c  */
#line 2064 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 531:

/* Line 1806 of yacc.c  */
#line 2065 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 532:

/* Line 1806 of yacc.c  */
#line 2066 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 533:

/* Line 1806 of yacc.c  */
#line 2067 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 534:

/* Line 1806 of yacc.c  */
#line 2068 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 535:

/* Line 1806 of yacc.c  */
#line 2069 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 536:

/* Line 1806 of yacc.c  */
#line 2070 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 537:

/* Line 1806 of yacc.c  */
#line 2071 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 538:

/* Line 1806 of yacc.c  */
#line 2072 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 539:

/* Line 1806 of yacc.c  */
#line 2073 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 540:

/* Line 1806 of yacc.c  */
#line 2074 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 541:

/* Line 1806 of yacc.c  */
#line 2075 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 542:

/* Line 1806 of yacc.c  */
#line 2076 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 543:

/* Line 1806 of yacc.c  */
#line 2077 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 544:

/* Line 1806 of yacc.c  */
#line 2078 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 545:

/* Line 1806 of yacc.c  */
#line 2079 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 546:

/* Line 1806 of yacc.c  */
#line 2080 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 547:

/* Line 1806 of yacc.c  */
#line 2081 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 548:

/* Line 1806 of yacc.c  */
#line 2082 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 549:

/* Line 1806 of yacc.c  */
#line 2083 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 550:

/* Line 1806 of yacc.c  */
#line 2084 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 551:

/* Line 1806 of yacc.c  */
#line 2085 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 552:

/* Line 1806 of yacc.c  */
#line 2086 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 553:

/* Line 1806 of yacc.c  */
#line 2087 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 554:

/* Line 1806 of yacc.c  */
#line 2088 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 555:

/* Line 1806 of yacc.c  */
#line 2089 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 556:

/* Line 1806 of yacc.c  */
#line 2090 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 557:

/* Line 1806 of yacc.c  */
#line 2091 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 558:

/* Line 1806 of yacc.c  */
#line 2092 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 559:

/* Line 1806 of yacc.c  */
#line 2093 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 560:

/* Line 1806 of yacc.c  */
#line 2094 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 561:

/* Line 1806 of yacc.c  */
#line 2095 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 562:

/* Line 1806 of yacc.c  */
#line 2096 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 563:

/* Line 1806 of yacc.c  */
#line 2097 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 564:

/* Line 1806 of yacc.c  */
#line 2102 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 565:

/* Line 1806 of yacc.c  */
#line 2106 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 566:

/* Line 1806 of yacc.c  */
#line 2107 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 567:

/* Line 1806 of yacc.c  */
#line 2110 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);}
    break;

  case 568:

/* Line 1806 of yacc.c  */
#line 2111 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);}
    break;

  case 569:

/* Line 1806 of yacc.c  */
#line 2112 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);}
    break;

  case 570:

/* Line 1806 of yacc.c  */
#line 2116 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);}
    break;

  case 571:

/* Line 1806 of yacc.c  */
#line 2117 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);}
    break;

  case 572:

/* Line 1806 of yacc.c  */
#line 2118 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);}
    break;

  case 573:

/* Line 1806 of yacc.c  */
#line 2122 "hphp.y"
    { (yyval).reset();}
    break;

  case 574:

/* Line 1806 of yacc.c  */
#line 2123 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 575:

/* Line 1806 of yacc.c  */
#line 2124 "hphp.y"
    { (yyval).reset();}
    break;

  case 576:

/* Line 1806 of yacc.c  */
#line 2128 "hphp.y"
    { (yyval).reset();}
    break;

  case 577:

/* Line 1806 of yacc.c  */
#line 2129 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);}
    break;

  case 578:

/* Line 1806 of yacc.c  */
#line 2130 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 579:

/* Line 1806 of yacc.c  */
#line 2134 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 580:

/* Line 1806 of yacc.c  */
#line 2135 "hphp.y"
    { (yyval).reset();}
    break;

  case 581:

/* Line 1806 of yacc.c  */
#line 2139 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 582:

/* Line 1806 of yacc.c  */
#line 2140 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 583:

/* Line 1806 of yacc.c  */
#line 2141 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));}
    break;

  case 584:

/* Line 1806 of yacc.c  */
#line 2143 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));}
    break;

  case 585:

/* Line 1806 of yacc.c  */
#line 2144 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));}
    break;

  case 586:

/* Line 1806 of yacc.c  */
#line 2145 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));}
    break;

  case 587:

/* Line 1806 of yacc.c  */
#line 2146 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));}
    break;

  case 588:

/* Line 1806 of yacc.c  */
#line 2147 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));}
    break;

  case 589:

/* Line 1806 of yacc.c  */
#line 2148 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));}
    break;

  case 590:

/* Line 1806 of yacc.c  */
#line 2149 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));}
    break;

  case 591:

/* Line 1806 of yacc.c  */
#line 2150 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));}
    break;

  case 592:

/* Line 1806 of yacc.c  */
#line 2151 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));}
    break;

  case 593:

/* Line 1806 of yacc.c  */
#line 2154 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));}
    break;

  case 594:

/* Line 1806 of yacc.c  */
#line 2156 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
    break;

  case 595:

/* Line 1806 of yacc.c  */
#line 2160 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 596:

/* Line 1806 of yacc.c  */
#line 2161 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 597:

/* Line 1806 of yacc.c  */
#line 2162 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 598:

/* Line 1806 of yacc.c  */
#line 2163 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 599:

/* Line 1806 of yacc.c  */
#line 2165 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 600:

/* Line 1806 of yacc.c  */
#line 2167 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 601:

/* Line 1806 of yacc.c  */
#line 2168 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); }
    break;

  case 602:

/* Line 1806 of yacc.c  */
#line 2170 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 603:

/* Line 1806 of yacc.c  */
#line 2171 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 604:

/* Line 1806 of yacc.c  */
#line 2172 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 605:

/* Line 1806 of yacc.c  */
#line 2178 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 606:

/* Line 1806 of yacc.c  */
#line 2180 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 607:

/* Line 1806 of yacc.c  */
#line 2184 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 608:

/* Line 1806 of yacc.c  */
#line 2188 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 609:

/* Line 1806 of yacc.c  */
#line 2189 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 610:

/* Line 1806 of yacc.c  */
#line 2190 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 611:

/* Line 1806 of yacc.c  */
#line 2191 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 612:

/* Line 1806 of yacc.c  */
#line 2192 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));}
    break;

  case 613:

/* Line 1806 of yacc.c  */
#line 2193 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));}
    break;

  case 614:

/* Line 1806 of yacc.c  */
#line 2195 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));}
    break;

  case 615:

/* Line 1806 of yacc.c  */
#line 2200 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 616:

/* Line 1806 of yacc.c  */
#line 2201 "hphp.y"
    { (yyval).reset();}
    break;

  case 617:

/* Line 1806 of yacc.c  */
#line 2205 "hphp.y"
    { (yyval).reset();}
    break;

  case 618:

/* Line 1806 of yacc.c  */
#line 2206 "hphp.y"
    { (yyval).reset();}
    break;

  case 619:

/* Line 1806 of yacc.c  */
#line 2209 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();}
    break;

  case 620:

/* Line 1806 of yacc.c  */
#line 2210 "hphp.y"
    { (yyval).reset();}
    break;

  case 621:

/* Line 1806 of yacc.c  */
#line 2216 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 622:

/* Line 1806 of yacc.c  */
#line 2218 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 623:

/* Line 1806 of yacc.c  */
#line 2220 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 624:

/* Line 1806 of yacc.c  */
#line 2221 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 625:

/* Line 1806 of yacc.c  */
#line 2225 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 626:

/* Line 1806 of yacc.c  */
#line 2226 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 627:

/* Line 1806 of yacc.c  */
#line 2227 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));}
    break;

  case 628:

/* Line 1806 of yacc.c  */
#line 2231 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));}
    break;

  case 629:

/* Line 1806 of yacc.c  */
#line 2233 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
    break;

  case 630:

/* Line 1806 of yacc.c  */
#line 2236 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));}
    break;

  case 631:

/* Line 1806 of yacc.c  */
#line 2237 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));}
    break;

  case 632:

/* Line 1806 of yacc.c  */
#line 2238 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));}
    break;

  case 633:

/* Line 1806 of yacc.c  */
#line 2241 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 634:

/* Line 1806 of yacc.c  */
#line 2242 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));}
    break;

  case 635:

/* Line 1806 of yacc.c  */
#line 2243 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 636:

/* Line 1806 of yacc.c  */
#line 2244 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 637:

/* Line 1806 of yacc.c  */
#line 2246 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 638:

/* Line 1806 of yacc.c  */
#line 2248 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 639:

/* Line 1806 of yacc.c  */
#line 2249 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 640:

/* Line 1806 of yacc.c  */
#line 2251 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 641:

/* Line 1806 of yacc.c  */
#line 2256 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 642:

/* Line 1806 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval).reset();}
    break;

  case 643:

/* Line 1806 of yacc.c  */
#line 2262 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 644:

/* Line 1806 of yacc.c  */
#line 2264 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 645:

/* Line 1806 of yacc.c  */
#line 2266 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 646:

/* Line 1806 of yacc.c  */
#line 2267 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 647:

/* Line 1806 of yacc.c  */
#line 2271 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 648:

/* Line 1806 of yacc.c  */
#line 2272 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 649:

/* Line 1806 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 650:

/* Line 1806 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval).reset(); }
    break;

  case 651:

/* Line 1806 of yacc.c  */
#line 2283 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 652:

/* Line 1806 of yacc.c  */
#line 2286 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 653:

/* Line 1806 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 654:

/* Line 1806 of yacc.c  */
#line 2292 "hphp.y"
    { (yyval).reset();}
    break;

  case 655:

/* Line 1806 of yacc.c  */
#line 2295 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 656:

/* Line 1806 of yacc.c  */
#line 2296 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
    break;

  case 657:

/* Line 1806 of yacc.c  */
#line 2303 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 658:

/* Line 1806 of yacc.c  */
#line 2305 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 659:

/* Line 1806 of yacc.c  */
#line 2308 "hphp.y"
    { only_in_hh_syntax(_p);}
    break;

  case 660:

/* Line 1806 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 661:

/* Line 1806 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 662:

/* Line 1806 of yacc.c  */
#line 2316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 663:

/* Line 1806 of yacc.c  */
#line 2317 "hphp.y"
    { (yyval).reset();}
    break;

  case 664:

/* Line 1806 of yacc.c  */
#line 2321 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 665:

/* Line 1806 of yacc.c  */
#line 2323 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 666:

/* Line 1806 of yacc.c  */
#line 2327 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 667:

/* Line 1806 of yacc.c  */
#line 2328 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 668:

/* Line 1806 of yacc.c  */
#line 2332 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 669:

/* Line 1806 of yacc.c  */
#line 2333 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 670:

/* Line 1806 of yacc.c  */
#line 2337 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 671:

/* Line 1806 of yacc.c  */
#line 2339 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 672:

/* Line 1806 of yacc.c  */
#line 2344 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 673:

/* Line 1806 of yacc.c  */
#line 2346 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 674:

/* Line 1806 of yacc.c  */
#line 2350 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 675:

/* Line 1806 of yacc.c  */
#line 2351 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 676:

/* Line 1806 of yacc.c  */
#line 2352 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 677:

/* Line 1806 of yacc.c  */
#line 2353 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 678:

/* Line 1806 of yacc.c  */
#line 2354 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 679:

/* Line 1806 of yacc.c  */
#line 2355 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 680:

/* Line 1806 of yacc.c  */
#line 2357 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 681:

/* Line 1806 of yacc.c  */
#line 2360 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 682:

/* Line 1806 of yacc.c  */
#line 2362 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 683:

/* Line 1806 of yacc.c  */
#line 2363 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 684:

/* Line 1806 of yacc.c  */
#line 2367 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 685:

/* Line 1806 of yacc.c  */
#line 2368 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 686:

/* Line 1806 of yacc.c  */
#line 2369 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 687:

/* Line 1806 of yacc.c  */
#line 2370 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 688:

/* Line 1806 of yacc.c  */
#line 2372 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 689:

/* Line 1806 of yacc.c  */
#line 2374 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 690:

/* Line 1806 of yacc.c  */
#line 2376 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 691:

/* Line 1806 of yacc.c  */
#line 2377 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 692:

/* Line 1806 of yacc.c  */
#line 2381 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 693:

/* Line 1806 of yacc.c  */
#line 2382 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 694:

/* Line 1806 of yacc.c  */
#line 2383 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 695:

/* Line 1806 of yacc.c  */
#line 2389 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));}
    break;

  case 696:

/* Line 1806 of yacc.c  */
#line 2392 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 697:

/* Line 1806 of yacc.c  */
#line 2395 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));}
    break;

  case 698:

/* Line 1806 of yacc.c  */
#line 2399 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));}
    break;

  case 699:

/* Line 1806 of yacc.c  */
#line 2403 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));}
    break;

  case 700:

/* Line 1806 of yacc.c  */
#line 2407 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));}
    break;

  case 701:

/* Line 1806 of yacc.c  */
#line 2414 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));}
    break;

  case 702:

/* Line 1806 of yacc.c  */
#line 2418 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));}
    break;

  case 703:

/* Line 1806 of yacc.c  */
#line 2422 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));}
    break;

  case 704:

/* Line 1806 of yacc.c  */
#line 2426 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 705:

/* Line 1806 of yacc.c  */
#line 2428 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 706:

/* Line 1806 of yacc.c  */
#line 2433 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 707:

/* Line 1806 of yacc.c  */
#line 2434 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 708:

/* Line 1806 of yacc.c  */
#line 2435 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 709:

/* Line 1806 of yacc.c  */
#line 2438 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 710:

/* Line 1806 of yacc.c  */
#line 2439 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);}
    break;

  case 711:

/* Line 1806 of yacc.c  */
#line 2442 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 712:

/* Line 1806 of yacc.c  */
#line 2443 "hphp.y"
    { (yyval).reset();}
    break;

  case 713:

/* Line 1806 of yacc.c  */
#line 2447 "hphp.y"
    { (yyval) = 1;}
    break;

  case 714:

/* Line 1806 of yacc.c  */
#line 2448 "hphp.y"
    { (yyval)++;}
    break;

  case 715:

/* Line 1806 of yacc.c  */
#line 2452 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 716:

/* Line 1806 of yacc.c  */
#line 2453 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 717:

/* Line 1806 of yacc.c  */
#line 2454 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 718:

/* Line 1806 of yacc.c  */
#line 2456 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 719:

/* Line 1806 of yacc.c  */
#line 2459 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 720:

/* Line 1806 of yacc.c  */
#line 2460 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 722:

/* Line 1806 of yacc.c  */
#line 2464 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 723:

/* Line 1806 of yacc.c  */
#line 2466 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 724:

/* Line 1806 of yacc.c  */
#line 2468 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 725:

/* Line 1806 of yacc.c  */
#line 2469 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 726:

/* Line 1806 of yacc.c  */
#line 2473 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);}
    break;

  case 727:

/* Line 1806 of yacc.c  */
#line 2474 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 728:

/* Line 1806 of yacc.c  */
#line 2476 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 729:

/* Line 1806 of yacc.c  */
#line 2477 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);}
    break;

  case 730:

/* Line 1806 of yacc.c  */
#line 2478 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));}
    break;

  case 731:

/* Line 1806 of yacc.c  */
#line 2479 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));}
    break;

  case 732:

/* Line 1806 of yacc.c  */
#line 2484 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 733:

/* Line 1806 of yacc.c  */
#line 2485 "hphp.y"
    { (yyval).reset();}
    break;

  case 734:

/* Line 1806 of yacc.c  */
#line 2489 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 735:

/* Line 1806 of yacc.c  */
#line 2490 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 736:

/* Line 1806 of yacc.c  */
#line 2491 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 737:

/* Line 1806 of yacc.c  */
#line 2492 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 738:

/* Line 1806 of yacc.c  */
#line 2495 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);}
    break;

  case 739:

/* Line 1806 of yacc.c  */
#line 2497 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);}
    break;

  case 740:

/* Line 1806 of yacc.c  */
#line 2498 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 741:

/* Line 1806 of yacc.c  */
#line 2499 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);}
    break;

  case 742:

/* Line 1806 of yacc.c  */
#line 2504 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 743:

/* Line 1806 of yacc.c  */
#line 2505 "hphp.y"
    { _p->onEmptyCollection((yyval));}
    break;

  case 744:

/* Line 1806 of yacc.c  */
#line 2509 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 745:

/* Line 1806 of yacc.c  */
#line 2510 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));}
    break;

  case 746:

/* Line 1806 of yacc.c  */
#line 2511 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 747:

/* Line 1806 of yacc.c  */
#line 2512 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));}
    break;

  case 748:

/* Line 1806 of yacc.c  */
#line 2517 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 749:

/* Line 1806 of yacc.c  */
#line 2518 "hphp.y"
    { _p->onEmptyCollection((yyval));}
    break;

  case 750:

/* Line 1806 of yacc.c  */
#line 2523 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 751:

/* Line 1806 of yacc.c  */
#line 2525 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));}
    break;

  case 752:

/* Line 1806 of yacc.c  */
#line 2527 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 753:

/* Line 1806 of yacc.c  */
#line 2528 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));}
    break;

  case 754:

/* Line 1806 of yacc.c  */
#line 2532 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);}
    break;

  case 755:

/* Line 1806 of yacc.c  */
#line 2534 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);}
    break;

  case 756:

/* Line 1806 of yacc.c  */
#line 2535 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);}
    break;

  case 757:

/* Line 1806 of yacc.c  */
#line 2537 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); }
    break;

  case 758:

/* Line 1806 of yacc.c  */
#line 2542 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 759:

/* Line 1806 of yacc.c  */
#line 2544 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 760:

/* Line 1806 of yacc.c  */
#line 2546 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 761:

/* Line 1806 of yacc.c  */
#line 2548 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);}
    break;

  case 762:

/* Line 1806 of yacc.c  */
#line 2550 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));}
    break;

  case 763:

/* Line 1806 of yacc.c  */
#line 2551 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 764:

/* Line 1806 of yacc.c  */
#line 2554 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;}
    break;

  case 765:

/* Line 1806 of yacc.c  */
#line 2555 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;}
    break;

  case 766:

/* Line 1806 of yacc.c  */
#line 2556 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;}
    break;

  case 767:

/* Line 1806 of yacc.c  */
#line 2560 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);}
    break;

  case 768:

/* Line 1806 of yacc.c  */
#line 2561 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);}
    break;

  case 769:

/* Line 1806 of yacc.c  */
#line 2562 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);}
    break;

  case 770:

/* Line 1806 of yacc.c  */
#line 2563 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);}
    break;

  case 771:

/* Line 1806 of yacc.c  */
#line 2564 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);}
    break;

  case 772:

/* Line 1806 of yacc.c  */
#line 2565 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);}
    break;

  case 773:

/* Line 1806 of yacc.c  */
#line 2566 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);}
    break;

  case 774:

/* Line 1806 of yacc.c  */
#line 2567 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);}
    break;

  case 775:

/* Line 1806 of yacc.c  */
#line 2568 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);}
    break;

  case 776:

/* Line 1806 of yacc.c  */
#line 2572 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 777:

/* Line 1806 of yacc.c  */
#line 2573 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 778:

/* Line 1806 of yacc.c  */
#line 2578 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 779:

/* Line 1806 of yacc.c  */
#line 2580 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 782:

/* Line 1806 of yacc.c  */
#line 2594 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); }
    break;

  case 783:

/* Line 1806 of yacc.c  */
#line 2598 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); }
    break;

  case 784:

/* Line 1806 of yacc.c  */
#line 2604 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 785:

/* Line 1806 of yacc.c  */
#line 2605 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 786:

/* Line 1806 of yacc.c  */
#line 2611 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 787:

/* Line 1806 of yacc.c  */
#line 2615 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); }
    break;

  case 788:

/* Line 1806 of yacc.c  */
#line 2621 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 789:

/* Line 1806 of yacc.c  */
#line 2622 "hphp.y"
    { (yyval).reset(); }
    break;

  case 790:

/* Line 1806 of yacc.c  */
#line 2626 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 791:

/* Line 1806 of yacc.c  */
#line 2629 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); }
    break;

  case 792:

/* Line 1806 of yacc.c  */
#line 2634 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); }
    break;

  case 793:

/* Line 1806 of yacc.c  */
#line 2635 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 794:

/* Line 1806 of yacc.c  */
#line 2636 "hphp.y"
    { (yyval).reset(); }
    break;

  case 795:

/* Line 1806 of yacc.c  */
#line 2637 "hphp.y"
    { (yyval).reset(); }
    break;

  case 796:

/* Line 1806 of yacc.c  */
#line 2641 "hphp.y"
    { (yyval).reset(); }
    break;

  case 797:

/* Line 1806 of yacc.c  */
#line 2642 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 798:

/* Line 1806 of yacc.c  */
#line 2647 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); }
    break;

  case 799:

/* Line 1806 of yacc.c  */
#line 2648 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); }
    break;

  case 800:

/* Line 1806 of yacc.c  */
#line 2650 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); }
    break;

  case 801:

/* Line 1806 of yacc.c  */
#line 2651 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); }
    break;

  case 802:

/* Line 1806 of yacc.c  */
#line 2657 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); }
    break;

  case 805:

/* Line 1806 of yacc.c  */
#line 2668 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 806:

/* Line 1806 of yacc.c  */
#line 2670 "hphp.y"
    {}
    break;

  case 807:

/* Line 1806 of yacc.c  */
#line 2674 "hphp.y"
    { (yyval).setText("array"); }
    break;

  case 808:

/* Line 1806 of yacc.c  */
#line 2681 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 809:

/* Line 1806 of yacc.c  */
#line 2684 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 810:

/* Line 1806 of yacc.c  */
#line 2687 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 811:

/* Line 1806 of yacc.c  */
#line 2688 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); }
    break;

  case 812:

/* Line 1806 of yacc.c  */
#line 2691 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); }
    break;

  case 813:

/* Line 1806 of yacc.c  */
#line 2694 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 814:

/* Line 1806 of yacc.c  */
#line 2696 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); }
    break;

  case 815:

/* Line 1806 of yacc.c  */
#line 2699 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); }
    break;

  case 816:

/* Line 1806 of yacc.c  */
#line 2702 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
    break;

  case 817:

/* Line 1806 of yacc.c  */
#line 2708 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
    break;

  case 818:

/* Line 1806 of yacc.c  */
#line 2712 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (5)]));
                                        _p->onTypeSpecialization((yyval), 't'); }
    break;

  case 819:

/* Line 1806 of yacc.c  */
#line 2720 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 820:

/* Line 1806 of yacc.c  */
#line 2721 "hphp.y"
    { (yyval).reset(); }
    break;



/* Line 1806 of yacc.c  */
#line 12243 "hphp.tab.cpp"
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
#line 2724 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

