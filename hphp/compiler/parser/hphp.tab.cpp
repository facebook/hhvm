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
#line 868 "hphp.tab.cpp"

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
#define YYLAST   15014

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  204
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  246
/* YYNRULES -- Number of rules.  */
#define YYNRULES  822
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1531

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
    1920,  1922,  1924,  1926,  1928,  1930,  1934,  1937,  1939,  1941,
    1944,  1947,  1952,  1957,  1961,  1966,  1968,  1970,  1974,  1978,
    1982,  1984,  1986,  1988,  1990,  1994,  1998,  2002,  2005,  2006,
    2008,  2009,  2011,  2012,  2018,  2022,  2026,  2028,  2030,  2032,
    2034,  2038,  2041,  2043,  2045,  2047,  2049,  2051,  2054,  2057,
    2062,  2067,  2071,  2076,  2079,  2080,  2086,  2090,  2094,  2096,
    2100,  2102,  2105,  2106,  2112,  2116,  2119,  2120,  2124,  2125,
    2130,  2133,  2134,  2138,  2142,  2144,  2145,  2147,  2150,  2153,
    2158,  2162,  2166,  2169,  2174,  2177,  2182,  2184,  2186,  2188,
    2190,  2192,  2195,  2200,  2204,  2209,  2213,  2215,  2217,  2219,
    2221,  2224,  2229,  2234,  2238,  2240,  2242,  2246,  2254,  2261,
    2270,  2280,  2289,  2300,  2308,  2315,  2324,  2326,  2329,  2334,
    2339,  2341,  2343,  2348,  2350,  2351,  2353,  2356,  2358,  2360,
    2363,  2368,  2372,  2376,  2377,  2379,  2382,  2387,  2391,  2394,
    2398,  2405,  2406,  2408,  2413,  2416,  2417,  2423,  2427,  2431,
    2433,  2440,  2445,  2450,  2453,  2456,  2457,  2463,  2467,  2471,
    2473,  2476,  2477,  2483,  2487,  2491,  2493,  2496,  2499,  2501,
    2504,  2506,  2511,  2515,  2519,  2526,  2530,  2532,  2534,  2536,
    2541,  2546,  2551,  2556,  2559,  2562,  2567,  2570,  2573,  2575,
    2579,  2583,  2587,  2588,  2591,  2597,  2604,  2606,  2609,  2611,
    2616,  2620,  2621,  2623,  2627,  2631,  2633,  2635,  2636,  2637,
    2640,  2644,  2646,  2652,  2656,  2660,  2664,  2666,  2669,  2670,
    2675,  2678,  2681,  2683,  2685,  2687,  2689,  2694,  2701,  2703,
    2712,  2718,  2720
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
     324,    -1,   179,    74,   180,   324,    -1,   179,    74,   180,
     324,   181,   324,   182,   324,    -1,   179,    74,   180,   324,
     181,   324,   182,   324,   183,    74,    -1,   185,   357,    -1,
     358,    -1,   357,     9,   358,    -1,   324,    -1,   324,   359,
      -1,   186,    -1,   187,    -1,   361,    -1,   362,    -1,   188,
     324,    -1,   189,   324,   190,   324,    -1,   183,    74,   348,
      -1,   364,     9,    74,    -1,   364,     9,    32,    74,    -1,
      74,    -1,    32,    74,    -1,   162,   148,   366,   163,    -1,
     368,    47,    -1,   368,   163,   369,   162,    47,   367,    -1,
      -1,   148,    -1,   368,   370,    14,   371,    -1,    -1,   369,
     372,    -1,    -1,   148,    -1,   149,    -1,   197,   324,   198,
      -1,   149,    -1,   197,   324,   198,    -1,   365,    -1,   374,
      -1,   373,    27,   374,    -1,   373,    44,   374,    -1,   211,
      -1,    65,    -1,    99,    -1,   100,    -1,   101,    -1,   147,
      -1,   174,    -1,   102,    -1,   103,    -1,   161,    -1,   104,
      -1,    66,    -1,    67,    -1,    69,    -1,    68,    -1,    83,
      -1,    84,    -1,    82,    -1,    85,    -1,    86,    -1,    87,
      -1,    88,    -1,    89,    -1,    90,    -1,    50,    -1,    91,
      -1,    92,    -1,    93,    -1,    94,    -1,    95,    -1,    96,
      -1,    98,    -1,    97,    -1,    81,    -1,    13,    -1,   118,
      -1,   119,    -1,   120,    -1,   121,    -1,    64,    -1,    63,
      -1,   113,    -1,     5,    -1,     7,    -1,     6,    -1,     4,
      -1,     3,    -1,   143,    -1,   105,    -1,   106,    -1,   115,
      -1,   116,    -1,   117,    -1,   112,    -1,   111,    -1,   110,
      -1,   109,    -1,   108,    -1,   107,    -1,   175,    -1,   114,
      -1,   124,    -1,   125,    -1,    10,    -1,    12,    -1,    11,
      -1,   127,    -1,   129,    -1,   128,    -1,   130,    -1,   131,
      -1,   145,    -1,   144,    -1,   173,    -1,   156,    -1,   158,
      -1,   157,    -1,   169,    -1,   171,    -1,   168,    -1,   217,
     194,   277,   195,    -1,   218,    -1,   148,    -1,   376,    -1,
     112,    -1,   416,    -1,   376,    -1,   112,    -1,   420,    -1,
     194,   195,    -1,   315,    -1,    -1,    -1,    79,    -1,   429,
      -1,   194,   277,   195,    -1,    -1,    70,    -1,    71,    -1,
      80,    -1,   130,    -1,   131,    -1,   145,    -1,   127,    -1,
     158,    -1,   128,    -1,   129,    -1,   144,    -1,   173,    -1,
     138,    79,   139,    -1,   138,   139,    -1,   382,    -1,   216,
      -1,    43,   383,    -1,    44,   383,    -1,   125,   194,   386,
     195,    -1,   176,   194,   386,   195,    -1,    62,   386,   201,
      -1,   168,   194,   338,   195,    -1,   384,    -1,   342,    -1,
     218,   142,   211,    -1,   148,   142,   211,    -1,   218,   142,
     118,    -1,   216,    -1,    73,    -1,   434,    -1,   382,    -1,
     202,   429,   202,    -1,   203,   429,   203,    -1,   138,   429,
     139,    -1,   389,   387,    -1,    -1,     9,    -1,    -1,     9,
      -1,    -1,   389,     9,   383,   123,   383,    -1,   389,     9,
     383,    -1,   383,   123,   383,    -1,   383,    -1,    70,    -1,
      71,    -1,    80,    -1,   138,    79,   139,    -1,   138,   139,
      -1,    70,    -1,    71,    -1,   211,    -1,   390,    -1,   211,
      -1,    43,   391,    -1,    44,   391,    -1,   125,   194,   393,
     195,    -1,   176,   194,   393,   195,    -1,    62,   393,   201,
      -1,   168,   194,   396,   195,    -1,   394,   387,    -1,    -1,
     394,     9,   392,   123,   392,    -1,   394,     9,   392,    -1,
     392,   123,   392,    -1,   392,    -1,   395,     9,   392,    -1,
     392,    -1,   397,   387,    -1,    -1,   397,     9,   334,   123,
     392,    -1,   334,   123,   392,    -1,   395,   387,    -1,    -1,
     194,   398,   195,    -1,    -1,   400,     9,   211,   399,    -1,
     211,   399,    -1,    -1,   402,   400,   387,    -1,    42,   401,
      41,    -1,   403,    -1,    -1,   406,    -1,   122,   415,    -1,
     122,   211,    -1,   122,   197,   324,   198,    -1,    62,   418,
     201,    -1,   197,   324,   198,    -1,   411,   407,    -1,   194,
     314,   195,   407,    -1,   421,   407,    -1,   194,   314,   195,
     407,    -1,   415,    -1,   375,    -1,   413,    -1,   414,    -1,
     408,    -1,   410,   405,    -1,   194,   314,   195,   405,    -1,
     377,   142,   415,    -1,   412,   194,   277,   195,    -1,   194,
     410,   195,    -1,   375,    -1,   413,    -1,   414,    -1,   408,
      -1,   410,   406,    -1,   194,   314,   195,   406,    -1,   412,
     194,   277,   195,    -1,   194,   410,   195,    -1,   415,    -1,
     408,    -1,   194,   410,   195,    -1,   410,   122,   211,   439,
     194,   277,   195,    -1,   410,   122,   415,   194,   277,   195,
      -1,   410,   122,   197,   324,   198,   194,   277,   195,    -1,
     194,   314,   195,   122,   211,   439,   194,   277,   195,    -1,
     194,   314,   195,   122,   415,   194,   277,   195,    -1,   194,
     314,   195,   122,   197,   324,   198,   194,   277,   195,    -1,
     377,   142,   211,   439,   194,   277,   195,    -1,   377,   142,
     415,   194,   277,   195,    -1,   377,   142,   197,   324,   198,
     194,   277,   195,    -1,   416,    -1,   419,   416,    -1,   416,
      62,   418,   201,    -1,   416,   197,   324,   198,    -1,   417,
      -1,    74,    -1,   199,   197,   324,   198,    -1,   324,    -1,
      -1,   199,    -1,   419,   199,    -1,   415,    -1,   409,    -1,
     420,   405,    -1,   194,   314,   195,   405,    -1,   377,   142,
     415,    -1,   194,   410,   195,    -1,    -1,   409,    -1,   420,
     406,    -1,   194,   314,   195,   406,    -1,   194,   410,   195,
      -1,   422,     9,    -1,   422,     9,   410,    -1,   422,     9,
     124,   194,   422,   195,    -1,    -1,   410,    -1,   124,   194,
     422,   195,    -1,   424,   387,    -1,    -1,   424,     9,   324,
     123,   324,    -1,   424,     9,   324,    -1,   324,   123,   324,
      -1,   324,    -1,   424,     9,   324,   123,    32,   410,    -1,
     424,     9,    32,   410,    -1,   324,   123,    32,   410,    -1,
      32,   410,    -1,   426,   387,    -1,    -1,   426,     9,   324,
     123,   324,    -1,   426,     9,   324,    -1,   324,   123,   324,
      -1,   324,    -1,   428,   387,    -1,    -1,   428,     9,   383,
     123,   383,    -1,   428,     9,   383,    -1,   383,   123,   383,
      -1,   383,    -1,   429,   430,    -1,   429,    79,    -1,   430,
      -1,    79,   430,    -1,    74,    -1,    74,    62,   431,   201,
      -1,    74,   122,   211,    -1,   140,   324,   198,    -1,   140,
      73,    62,   324,   201,   198,    -1,   141,   410,   198,    -1,
     211,    -1,    75,    -1,    74,    -1,   115,   194,   433,   195,
      -1,   116,   194,   410,   195,    -1,   116,   194,   325,   195,
      -1,   116,   194,   314,   195,    -1,     7,   324,    -1,     6,
     324,    -1,     5,   194,   324,   195,    -1,     4,   324,    -1,
       3,   324,    -1,   410,    -1,   433,     9,   410,    -1,   377,
     142,   211,    -1,   377,   142,   118,    -1,    -1,    91,   448,
      -1,   169,   438,    14,   448,   196,    -1,   171,   438,   435,
      14,   448,   196,    -1,   211,    -1,   448,   211,    -1,   211,
      -1,   211,   164,   443,   165,    -1,   164,   440,   165,    -1,
      -1,   448,    -1,   440,     9,   448,    -1,   440,     9,   159,
      -1,   440,    -1,   159,    -1,    -1,    -1,    27,   448,    -1,
     443,     9,   211,    -1,   211,    -1,   443,     9,   211,    91,
     448,    -1,   211,    91,   448,    -1,    80,   123,   448,    -1,
     445,     9,   444,    -1,   444,    -1,   445,   387,    -1,    -1,
     168,   194,   446,   195,    -1,    26,   448,    -1,    52,   448,
      -1,   218,    -1,   125,    -1,   126,    -1,   447,    -1,   125,
     164,   448,   165,    -1,   125,   164,   448,     9,   448,   165,
      -1,   148,    -1,   194,    99,   194,   441,   195,    27,   448,
     195,    -1,   194,   440,     9,   448,   195,    -1,   448,    -1,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   726,   726,   726,   735,   737,   740,   741,   742,   743,
     744,   745,   748,   750,   750,   752,   752,   754,   755,   760,
     761,   762,   763,   764,   765,   766,   767,   768,   769,   770,
     771,   772,   773,   774,   775,   776,   777,   778,   779,   783,
     785,   788,   789,   790,   791,   796,   797,   801,   802,   804,
     807,   813,   820,   827,   831,   837,   839,   842,   843,   844,
     845,   848,   849,   853,   858,   858,   864,   864,   871,   870,
     876,   876,   881,   882,   883,   884,   885,   886,   887,   888,
     889,   890,   891,   892,   893,   896,   894,   901,   909,   903,
     913,   911,   915,   916,   920,   921,   922,   923,   924,   925,
     926,   927,   928,   929,   930,   938,   938,   943,   949,   953,
     953,   961,   962,   966,   967,   971,   976,   975,   988,   986,
    1000,   998,  1014,  1013,  1032,  1030,  1049,  1048,  1057,  1055,
    1067,  1066,  1078,  1076,  1089,  1090,  1094,  1097,  1100,  1101,
    1102,  1105,  1107,  1110,  1111,  1114,  1115,  1118,  1119,  1123,
    1124,  1129,  1130,  1133,  1134,  1135,  1139,  1140,  1144,  1145,
    1149,  1150,  1154,  1155,  1160,  1161,  1166,  1167,  1168,  1169,
    1172,  1175,  1177,  1180,  1181,  1185,  1187,  1190,  1193,  1196,
    1197,  1200,  1201,  1205,  1207,  1209,  1210,  1214,  1218,  1222,
    1227,  1232,  1237,  1242,  1248,  1257,  1259,  1261,  1262,  1266,
    1269,  1272,  1276,  1280,  1284,  1288,  1293,  1301,  1303,  1306,
    1307,  1308,  1310,  1315,  1316,  1319,  1320,  1321,  1325,  1326,
    1328,  1329,  1333,  1335,  1338,  1338,  1342,  1341,  1345,  1349,
    1347,  1362,  1359,  1372,  1374,  1376,  1378,  1380,  1382,  1384,
    1388,  1389,  1390,  1393,  1399,  1402,  1408,  1411,  1416,  1418,
    1423,  1428,  1432,  1433,  1439,  1440,  1445,  1446,  1451,  1452,
    1456,  1457,  1461,  1463,  1469,  1474,  1475,  1477,  1481,  1482,
    1483,  1484,  1488,  1489,  1490,  1491,  1492,  1493,  1495,  1500,
    1503,  1504,  1508,  1509,  1513,  1514,  1517,  1518,  1521,  1522,
    1525,  1526,  1530,  1531,  1532,  1533,  1534,  1535,  1536,  1540,
    1541,  1544,  1545,  1546,  1549,  1551,  1553,  1554,  1557,  1559,
    1563,  1564,  1566,  1567,  1568,  1571,  1575,  1576,  1580,  1581,
    1585,  1586,  1590,  1594,  1599,  1603,  1607,  1612,  1613,  1614,
    1617,  1619,  1620,  1621,  1624,  1625,  1626,  1627,  1628,  1629,
    1630,  1631,  1632,  1633,  1634,  1635,  1636,  1637,  1638,  1639,
    1640,  1641,  1642,  1643,  1644,  1645,  1646,  1647,  1648,  1649,
    1650,  1651,  1652,  1653,  1654,  1655,  1656,  1657,  1658,  1659,
    1660,  1661,  1662,  1663,  1664,  1666,  1667,  1669,  1671,  1672,
    1673,  1674,  1675,  1676,  1677,  1678,  1679,  1680,  1681,  1682,
    1683,  1684,  1685,  1686,  1687,  1688,  1689,  1690,  1691,  1695,
    1699,  1704,  1703,  1718,  1716,  1733,  1733,  1748,  1748,  1766,
    1767,  1772,  1777,  1781,  1787,  1791,  1797,  1799,  1803,  1805,
    1809,  1813,  1814,  1818,  1825,  1832,  1834,  1839,  1840,  1841,
    1845,  1849,  1853,  1857,  1859,  1861,  1863,  1868,  1869,  1874,
    1875,  1876,  1877,  1878,  1879,  1883,  1887,  1891,  1895,  1897,
    1899,  1904,  1909,  1913,  1914,  1918,  1919,  1923,  1924,  1928,
    1929,  1933,  1937,  1941,  1945,  1946,  1947,  1948,  1952,  1958,
    1967,  1980,  1981,  1984,  1987,  1990,  1991,  1994,  1998,  2001,
    2004,  2011,  2012,  2016,  2017,  2019,  2023,  2024,  2025,  2026,
    2027,  2028,  2029,  2030,  2031,  2032,  2033,  2034,  2035,  2036,
    2037,  2038,  2039,  2040,  2041,  2042,  2043,  2044,  2045,  2046,
    2047,  2048,  2049,  2050,  2051,  2052,  2053,  2054,  2055,  2056,
    2057,  2058,  2059,  2060,  2061,  2062,  2063,  2064,  2065,  2066,
    2067,  2068,  2069,  2070,  2071,  2072,  2073,  2074,  2075,  2076,
    2077,  2078,  2079,  2080,  2081,  2082,  2083,  2084,  2085,  2086,
    2087,  2088,  2089,  2090,  2091,  2092,  2093,  2094,  2095,  2096,
    2097,  2098,  2099,  2100,  2101,  2102,  2106,  2111,  2112,  2115,
    2116,  2117,  2121,  2122,  2123,  2127,  2128,  2129,  2133,  2134,
    2135,  2138,  2140,  2144,  2145,  2146,  2148,  2149,  2150,  2151,
    2152,  2153,  2154,  2155,  2156,  2157,  2160,  2165,  2166,  2167,
    2168,  2169,  2171,  2173,  2174,  2176,  2177,  2181,  2184,  2187,
    2193,  2194,  2195,  2196,  2197,  2198,  2199,  2204,  2206,  2210,
    2211,  2214,  2215,  2219,  2222,  2224,  2226,  2230,  2231,  2232,
    2234,  2237,  2241,  2242,  2243,  2246,  2247,  2248,  2249,  2250,
    2252,  2254,  2255,  2260,  2262,  2265,  2268,  2270,  2272,  2275,
    2277,  2281,  2283,  2286,  2289,  2295,  2297,  2300,  2301,  2306,
    2309,  2313,  2313,  2318,  2321,  2322,  2326,  2327,  2332,  2333,
    2337,  2338,  2342,  2343,  2348,  2350,  2355,  2356,  2357,  2358,
    2359,  2360,  2361,  2363,  2366,  2368,  2372,  2373,  2374,  2375,
    2376,  2378,  2380,  2382,  2386,  2387,  2388,  2392,  2395,  2398,
    2401,  2405,  2409,  2416,  2420,  2424,  2431,  2432,  2437,  2439,
    2440,  2443,  2444,  2447,  2448,  2452,  2453,  2457,  2458,  2459,
    2460,  2462,  2465,  2468,  2469,  2470,  2472,  2474,  2478,  2479,
    2480,  2482,  2483,  2484,  2488,  2490,  2493,  2495,  2496,  2497,
    2498,  2501,  2503,  2504,  2508,  2510,  2513,  2515,  2516,  2517,
    2521,  2523,  2526,  2529,  2531,  2533,  2537,  2538,  2540,  2541,
    2547,  2548,  2550,  2552,  2554,  2556,  2559,  2560,  2561,  2565,
    2566,  2567,  2568,  2569,  2570,  2571,  2572,  2573,  2577,  2578,
    2582,  2584,  2592,  2594,  2598,  2602,  2609,  2610,  2616,  2617,
    2624,  2627,  2631,  2634,  2639,  2640,  2641,  2642,  2646,  2647,
    2651,  2653,  2654,  2656,  2660,  2666,  2668,  2672,  2675,  2678,
    2686,  2689,  2692,  2693,  2696,  2699,  2700,  2703,  2707,  2711,
    2717,  2725,  2726
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
     350,   350,   350,   350,   350,   351,   352,   353,   354,   354,
     354,   355,   356,   357,   357,   358,   358,   359,   359,   360,
     360,   361,   362,   363,   364,   364,   364,   364,   365,   366,
     366,   367,   367,   368,   368,   369,   369,   370,   371,   371,
     372,   372,   372,   373,   373,   373,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   374,   374,   374,   374,
     374,   374,   374,   374,   374,   374,   375,   376,   376,   377,
     377,   377,   378,   378,   378,   379,   379,   379,   380,   380,
     380,   381,   381,   382,   382,   382,   382,   382,   382,   382,
     382,   382,   382,   382,   382,   382,   382,   383,   383,   383,
     383,   383,   383,   383,   383,   383,   383,   384,   384,   384,
     385,   385,   385,   385,   385,   385,   385,   386,   386,   387,
     387,   388,   388,   389,   389,   389,   389,   390,   390,   390,
     390,   390,   391,   391,   391,   392,   392,   392,   392,   392,
     392,   392,   392,   393,   393,   394,   394,   394,   394,   395,
     395,   396,   396,   397,   397,   398,   398,   399,   399,   400,
     400,   402,   401,   403,   404,   404,   405,   405,   406,   406,
     407,   407,   408,   408,   409,   409,   410,   410,   410,   410,
     410,   410,   410,   410,   410,   410,   411,   411,   411,   411,
     411,   411,   411,   411,   412,   412,   412,   413,   413,   413,
     413,   413,   413,   414,   414,   414,   415,   415,   416,   416,
     416,   417,   417,   418,   418,   419,   419,   420,   420,   420,
     420,   420,   420,   421,   421,   421,   421,   421,   422,   422,
     422,   422,   422,   422,   423,   423,   424,   424,   424,   424,
     424,   424,   424,   424,   425,   425,   426,   426,   426,   426,
     427,   427,   428,   428,   428,   428,   429,   429,   429,   429,
     430,   430,   430,   430,   430,   430,   431,   431,   431,   432,
     432,   432,   432,   432,   432,   432,   432,   432,   433,   433,
     434,   434,   435,   435,   436,   436,   437,   437,   438,   438,
     439,   439,   440,   440,   441,   441,   441,   441,   442,   442,
     443,   443,   443,   443,   444,   445,   445,   446,   446,   447,
     448,   448,   448,   448,   448,   448,   448,   448,   448,   448,
     448,   449,   449
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
       1,     1,     1,     1,     1,     3,     2,     1,     1,     2,
       2,     4,     4,     3,     4,     1,     1,     3,     3,     3,
       1,     1,     1,     1,     3,     3,     3,     2,     0,     1,
       0,     1,     0,     5,     3,     3,     1,     1,     1,     1,
       3,     2,     1,     1,     1,     1,     1,     2,     2,     4,
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
       2,     0,     5,     3,     3,     1,     2,     2,     1,     2,
       1,     4,     3,     3,     6,     3,     1,     1,     1,     4,
       4,     4,     4,     2,     2,     4,     2,     2,     1,     3,
       3,     3,     0,     2,     5,     6,     1,     2,     1,     4,
       3,     0,     1,     3,     3,     1,     1,     0,     0,     2,
       3,     1,     5,     3,     3,     3,     1,     2,     0,     4,
       2,     2,     1,     1,     1,     1,     4,     6,     1,     8,
       5,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   661,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   735,     0,   723,   577,
       0,   583,   584,    19,   611,   711,    84,   585,     0,    66,
       0,     0,     0,     0,     0,     0,     0,     0,   115,     0,
       0,     0,     0,     0,     0,   292,   293,   294,   297,   296,
     295,     0,     0,     0,     0,   138,     0,     0,     0,   589,
     591,   592,   586,   587,     0,     0,   593,   588,     0,     0,
     568,    20,    21,    22,    24,    23,     0,   590,     0,     0,
       0,     0,   594,     0,   298,    25,    26,    27,    29,    28,
      30,    31,    32,    33,    34,    35,    36,    37,    38,   407,
       0,    83,    56,   715,   578,     0,     0,     4,    45,    47,
      50,   610,     0,   567,     0,     6,   114,     7,     8,     9,
       0,     0,   290,   329,     0,     0,     0,     0,     0,     0,
       0,   327,   396,   397,   393,   392,   314,   398,     0,     0,
     313,   677,   569,     0,   613,   391,   289,   680,   328,     0,
       0,   678,   679,   676,   706,   710,     0,   381,   612,    10,
     297,   296,   295,     0,     0,    45,   114,     0,   777,   328,
     776,     0,   774,   773,   395,     0,     0,   365,   366,   367,
     368,   390,   388,   387,   386,   385,   384,   383,   382,   711,
     570,     0,   791,   569,     0,   348,   346,     0,   739,     0,
     620,   312,   573,     0,   791,   572,     0,   582,   718,   717,
     574,     0,     0,   576,   389,     0,     0,     0,     0,   317,
       0,    64,   319,     0,     0,    70,    72,     0,     0,    74,
       0,     0,     0,   813,   814,   818,     0,     0,    45,   812,
       0,   815,     0,     0,    76,     0,     0,     0,     0,   105,
       0,     0,     0,     0,    40,    41,   215,     0,     0,   214,
     140,   139,   220,     0,     0,     0,     0,     0,   788,   126,
     136,   731,   735,   760,     0,   596,     0,     0,     0,   758,
       0,    15,     0,    49,     0,   320,   130,   137,   474,   417,
       0,   782,   324,   665,   329,     0,   327,   328,     0,     0,
     579,     0,   580,     0,     0,     0,   104,     0,     0,    52,
     208,     0,    18,   113,     0,   135,   122,   134,   295,   114,
     291,    95,    96,    97,    98,    99,   101,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   723,    94,   714,   714,   102,   745,     0,     0,
       0,     0,     0,   288,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   347,   345,     0,   681,
     666,   714,     0,   672,   208,   714,     0,   716,   707,   731,
       0,   114,     0,     0,   663,   658,   620,     0,     0,     0,
       0,   743,     0,   422,   619,   734,     0,     0,    52,     0,
     208,   311,     0,   719,   666,   674,   575,     0,    56,   176,
       0,   406,     0,    81,     0,     0,   318,     0,     0,     0,
       0,     0,    73,    93,    75,   810,   811,     0,   808,     0,
       0,   792,     0,   787,     0,   100,    77,   103,     0,     0,
       0,     0,     0,     0,     0,   430,     0,   437,   439,   440,
     441,   442,   443,   444,   435,   459,   460,    56,     0,    90,
      92,    42,     0,    17,     0,     0,   216,     0,    79,     0,
       0,    80,   778,     0,     0,   329,   327,   328,     0,     0,
     146,     0,   732,     0,     0,     0,     0,   595,   759,   611,
       0,     0,   757,   616,   756,    48,     5,    12,    13,    78,
       0,   144,     0,     0,   411,     0,   620,     0,     0,     0,
       0,   197,     0,   622,   664,   822,   310,   378,   685,    61,
      55,    57,    58,    59,    60,     0,   394,   614,   615,    46,
       0,     0,     0,   622,   209,     0,   401,   116,   142,     0,
     351,   353,   352,     0,     0,   349,   350,   354,   356,   355,
     370,   369,   372,   371,   373,   375,   376,   374,   364,   363,
     358,   359,   357,   360,   361,   362,   377,   713,     0,     0,
     749,     0,   620,   781,     0,   780,   683,   706,   128,   132,
     124,   114,     0,     0,   322,   325,   331,   431,   344,   343,
     342,   341,   340,   339,   338,   337,   336,   335,   334,     0,
     668,   667,     0,     0,     0,     0,     0,     0,     0,   775,
     656,   660,   619,   662,     0,     0,   791,     0,   738,     0,
     737,     0,   722,   721,     0,     0,   668,   667,   315,   178,
     180,    56,   409,   316,     0,    56,   160,    65,   319,     0,
       0,     0,     0,   172,   172,    71,     0,     0,   806,   620,
       0,   797,     0,     0,     0,   618,     0,     0,   568,     0,
      25,    50,   598,   567,   606,     0,   597,    54,   605,     0,
       0,   447,   711,   448,     0,   455,   452,   453,   461,     0,
     438,   433,     0,   436,     0,     0,     0,     0,    39,    43,
       0,   213,   221,   218,     0,     0,   769,   772,   771,   770,
      11,   801,     0,     0,     0,   731,   728,     0,   421,   768,
     767,   766,     0,   762,     0,   763,   765,     0,     5,   321,
       0,     0,   468,   469,   477,   476,     0,     0,   619,   416,
     420,     0,   783,     0,   798,   665,   196,   821,     0,     0,
     682,   666,   673,   712,     0,   790,   210,   566,   621,   207,
       0,   665,     0,     0,   144,   403,   118,   380,     0,   425,
     426,     0,   423,   619,   744,     0,     0,   208,   146,   144,
     142,     0,   723,   332,     0,     0,   208,   670,   671,   684,
     708,   709,     0,     0,     0,   644,   627,   628,   629,     0,
       0,     0,    25,   636,   635,   650,   620,     0,   658,   742,
     741,     0,   720,   666,   675,   581,     0,   182,     0,     0,
      62,     0,     0,     0,     0,     0,     0,   152,   153,   164,
       0,    56,   162,    87,   172,     0,   172,     0,     0,   816,
       0,   619,   807,   809,   796,   795,     0,   793,   599,   600,
     626,     0,   620,   618,     0,     0,   419,   618,     0,   751,
     432,     0,     0,     0,   457,   458,   456,     0,     0,   434,
       0,   106,     0,   109,    91,    44,   217,     0,   779,    82,
       0,     0,   789,   145,   147,   223,     0,     0,   729,     0,
     761,     0,    16,     0,   143,   223,     0,     0,   413,     0,
     784,     0,     0,     0,   195,   822,     0,   199,     0,   668,
     667,   793,     0,   211,    53,     0,   665,   141,     0,   665,
       0,   379,   748,   747,     0,   208,     0,     0,     0,   144,
     120,   582,   669,   208,     0,     0,   632,   633,   634,   637,
     638,   648,     0,   620,   644,     0,   631,   652,   644,   619,
     655,   657,   659,     0,   736,   669,     0,     0,     0,     0,
     179,   410,    67,     0,   319,   154,   731,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   166,     0,   804,   805,
       0,     0,   820,     0,   603,   619,   617,     0,   608,     0,
     620,     0,     0,   609,   607,   755,     0,   620,   445,   449,
     446,   454,   462,   463,     0,    56,   219,   803,   800,     0,
     289,   733,   731,   323,   326,   330,     0,    14,   289,   480,
       0,     0,   482,   475,   478,     0,   473,     0,   785,   799,
     408,     0,   200,     0,     0,     0,   208,   212,   798,     0,
     223,     0,   665,     0,   208,     0,   704,   223,   223,     0,
       0,   333,   208,     0,   698,     0,   641,   619,   643,     0,
     630,     0,     0,   620,     0,   649,   740,     0,    56,     0,
     175,   161,     0,     0,   151,    85,   165,     0,     0,   168,
       0,   173,   174,    56,   167,   817,   794,     0,   625,   624,
     601,     0,   619,   418,   604,   602,     0,   424,   619,   750,
       0,     0,     0,     0,   148,     0,     0,     0,   287,     0,
       0,     0,   127,   222,   224,     0,   286,     0,   289,     0,
     764,   131,   471,     0,     0,   412,     0,   203,     0,   202,
     669,   208,     0,   400,   798,   289,   798,     0,   746,     0,
     703,   289,   289,   223,   665,     0,   697,   647,   646,   639,
       0,   642,   619,   651,   640,    56,   181,    63,    68,   155,
       0,   163,   169,    56,   171,     0,     0,   415,     0,   754,
     753,     0,    56,   110,   802,     0,     0,     0,     0,   149,
     254,   252,   568,    24,     0,   248,     0,   253,   264,     0,
     262,   267,     0,   266,     0,   265,     0,   114,   226,     0,
     228,     0,   730,   472,   470,   481,   479,   204,     0,   201,
     208,     0,   701,     0,     0,     0,   123,   400,   798,   705,
     129,   133,   289,     0,   699,     0,   654,     0,   177,     0,
      56,   158,    86,   170,   819,   623,     0,     0,     0,     0,
       0,     0,     0,     0,   238,   242,     0,     0,   233,   532,
     531,   528,   530,   529,   549,   551,   550,   520,   510,   526,
     525,   487,   497,   498,   500,   499,   519,   503,   501,   502,
     504,   505,   506,   507,   508,   509,   511,   512,   513,   514,
     515,   516,   518,   517,   488,   489,   490,   493,   494,   496,
     534,   535,   544,   543,   542,   541,   540,   539,   527,   546,
     536,   537,   538,   521,   522,   523,   524,   547,   548,   552,
     554,   553,   555,   556,   533,   558,   557,   491,   560,   562,
     561,   495,   565,   563,   564,   559,   492,   545,   486,   259,
     483,     0,   234,   280,   281,   279,   272,     0,   273,   235,
     306,     0,     0,     0,     0,   114,     0,   206,     0,   700,
       0,    56,   282,    56,   117,     0,     0,   125,   798,   645,
       0,    56,   156,    69,     0,   414,   752,   450,   108,   236,
     237,   309,   150,     0,     0,   256,   249,     0,     0,     0,
     261,   263,     0,     0,   268,   275,   276,   274,     0,     0,
     225,     0,     0,     0,     0,   205,   702,     0,   466,   622,
       0,     0,    56,   119,     0,   653,     0,     0,     0,    88,
     239,    45,     0,   240,   241,     0,     0,   255,   258,   484,
     485,     0,   250,   277,   278,   270,   271,   269,   307,   304,
     229,   227,   308,     0,   467,   621,     0,   402,   283,     0,
     121,     0,   159,   451,     0,   112,     0,   289,   257,   260,
       0,   665,   231,     0,   464,   399,   404,   157,     0,     0,
      89,   246,     0,   288,   305,   185,     0,   622,   300,   665,
     465,     0,   111,     0,     0,   245,   798,   665,   184,   301,
     302,   303,   822,   299,     0,     0,     0,   244,     0,   183,
     300,     0,   798,     0,   243,   284,    56,   230,   822,     0,
     187,     0,    56,     0,     0,   188,     0,   232,     0,   285,
       0,   191,     0,   190,   107,   192,     0,   189,     0,   194,
     193
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   117,   738,   516,   175,   263,   264,
     119,   120,   121,   122,   123,   124,   308,   540,   541,   435,
     230,  1239,   441,  1170,  1455,   706,   260,   477,  1419,   884,
    1015,  1470,   324,   176,   542,   772,   930,  1060,   543,   558,
     790,   500,   788,   544,   521,   789,   326,   279,   296,   130,
     774,   741,   724,   893,  1188,   978,   837,  1373,  1242,   657,
     843,   440,   665,   845,  1093,   650,   827,   830,   968,  1476,
    1477,   532,   533,   552,   553,   268,   269,   273,  1020,  1123,
    1206,  1353,  1461,  1479,  1383,  1423,  1424,  1425,  1194,  1195,
    1196,  1384,  1390,  1432,  1199,  1200,  1204,  1346,  1347,  1348,
    1364,  1507,  1124,  1125,   177,   132,  1492,  1493,  1351,  1127,
     133,   223,   436,   437,   134,   135,   136,   137,   138,   139,
     140,   141,  1224,   142,   771,   929,   143,   227,   303,   431,
     525,   526,  1000,   527,  1001,   144,   145,   146,   684,   147,
     148,   257,   149,   258,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   696,   697,   876,   474,   475,   476,   703,
    1409,   150,   522,  1214,   523,   906,   746,  1036,  1033,  1339,
    1340,   151,   152,   153,   217,   224,   311,   421,   154,   860,
     688,   155,   861,   415,   756,   862,   814,   949,   951,   952,
     953,   816,  1072,  1073,   817,   631,   406,   185,   186,   156,
     535,   389,   390,   762,   157,   218,   179,   159,   160,   161,
     162,   163,   164,   165,   588,   166,   220,   221,   503,   209,
     210,   591,   592,  1006,  1007,   288,   289,   732,   167,   493,
     168,   530,   169,   250,   280,   319,   450,   856,   913,   722,
     668,   669,   670,   251,   252,   758
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1198
static const yytype_int16 yypact[] =
{
   -1198,   147, -1198, -1198,  4001, 11639, 11639,  -113, 11639, 11639,
   11639, -1198, 11639, 11639, 11639, 11639, 11639, 11639, 11639, 11639,
   11639, 11639, 11639, 11639, 14001, 14001,  9026, 11639, 14066,   -64,
     -52, -1198, -1198, -1198, -1198,   152, -1198, -1198, 11639, -1198,
     -52,   -29,   -26,   -15,   -52,  9227,  9671,  9428, -1198, 13452,
    8624,   135, 11639, 14360,    80, -1198, -1198, -1198,   102,   155,
      42,    -1,   192,   202,   234, -1198,  9671,   243,   246, -1198,
   -1198, -1198, -1198, -1198,   316,  2072, -1198, -1198,  9671,  9629,
   -1198, -1198, -1198, -1198, -1198, -1198,  9671, -1198,   161,   252,
    9671,  9671, -1198, 11639, -1198, -1198, -1198, -1198, -1198, -1198,
   -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198,
   11639, -1198, -1198,   172,   279,   302,   302, -1198,   408,   331,
     309, -1198,   285, -1198,    46, -1198,   417, -1198, -1198, -1198,
   14403,   512, -1198, -1198,   308,   314,   321,   374,   375,   376,
   12819, -1198, -1198, -1198, -1198,   445, -1198,   468,   511,   383,
   -1198,    84,   377,   357, -1198, -1198,   662,     4,   853,    99,
     386,   101,   121,   389,    15, -1198,   126, -1198,   522, -1198,
   -1198, -1198,   443,   394,   447, -1198,   417,   512, 14924,  1337,
   14924, 11639, 14924, 14924,  2290,   548,  9671, -1198, -1198,   540,
   -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198,
   -1198, 13678,   430, -1198,   453,   474,   474, 14001, 14584,   396,
     590, -1198,   443, 13678,   430,   459,   460,   416,   129, -1198,
     503,    99,  9830, -1198, -1198, 11639,  7217,   619,    49, 14924,
    8222, -1198, 11639, 11639,  9671, -1198, -1198, 12860,   434, -1198,
   12901, 13452, 13452,   467, -1198, -1198,   439, 13263,   620, -1198,
     621, -1198,  9671,   562, -1198,   441, 12942,   446,   334, -1198,
      25, 12988,  9671,    50, -1198,   253, -1198, 13813,    58, -1198,
   -1198, -1198,   625,    62, 14001, 14001, 11639,   449,   476, -1198,
   -1198, 13863,  9026,    89,   335, -1198, 11840, 14001,   329, -1198,
    9671, -1198,   -19,   331,   463, 14625, -1198, -1198, -1198,   561,
     632,   565, 14924,    43,   466, 14924,   472,   521,  4202, 11639,
     342,   473,   326,   342,   282,   257, -1198,  9671, 13452,   480,
   10031, 13452, -1198, -1198,  8465, -1198, -1198, -1198, -1198,   417,
   -1198, -1198, -1198, -1198, -1198, -1198, -1198, 11639, 11639, 11639,
   10232, 11639, 11639, 11639, 11639, 11639, 11639, 11639, 11639, 11639,
   11639, 11639, 11639, 11639, 11639, 11639, 11639, 11639, 11639, 11639,
   11639, 11639, 14066, -1198, 11639, 11639, -1198, 11639, 13142,  9671,
    9671, 14403,   569,   682,  8423, 11639, 11639, 11639, 11639, 11639,
   11639, 11639, 11639, 11639, 11639, 11639, -1198, -1198,  1644, -1198,
     130, 11639, 11639, -1198, 10031, 11639, 11639,   172,   132, 13863,
     481,   417, 10433, 13029, -1198,   483,   669, 13678,   484,    19,
    3152,   474, 10634, -1198, 10835, -1198,   486,    23, -1198,   175,
   10031, -1198, 14120, -1198,   133, -1198, -1198, 13070, -1198, -1198,
   11036, -1198, 11639, -1198,   599,  7418,   674,   488, 14817,   671,
      32,    34, -1198, -1198, -1198, -1198, -1198, 13452,   610,   497,
     684, -1198, 13628, -1198,   514, -1198, -1198, -1198,   623, 11639,
   12041,   624, 11639, 11639, 11639, -1198,   334, -1198, -1198, -1198,
   -1198, -1198, -1198, -1198,   516, -1198, -1198, -1198,   501, -1198,
   -1198,   255, 14360, -1198,  9671, 11639,   474,    80, -1198, 13628,
     627, -1198,   474,    61,    67,   507,   509,  1730,   513,  9671,
     585,   517,   474,   110,   518, 14344,  9671, -1198, -1198,   645,
    3040,   -50, -1198, -1198, -1198,   331, -1198, -1198, -1198, -1198,
   11639,   587,   551,   214, -1198,   592,   709,   524, 13452, 13452,
     707, -1198,   530,   714, -1198, 13452,    87,   663,   125, -1198,
   -1198, -1198, -1198, -1198, -1198,  3244, -1198, -1198, -1198, -1198,
      51, 14001,   531,   718, 14924,   715, -1198, -1198,   612,  8867,
   14964,  3989,  2290, 11639, 14883,  4388,  4588,  3451,  4787,  2136,
    3781,  3781,  3781,  3781,  1028,  1028,  1028,  1028,   605,   605,
     450,   450,   450,   540,   540,   540, -1198, 14924,   529,   534,
   14680,   538,   729, -1198, 11639,    11,   549,   132, -1198, -1198,
   -1198,   417, 13763, 11639, -1198, -1198,  2290, -1198,  2290,  2290,
    2290,  2290,  2290,  2290,  2290,  2290,  2290,  2290,  2290, 11639,
      11,   553,   547,  3538,   554,   555, 12226,   111,   564, -1198,
    2374, -1198,  9671, -1198,   466,    87,   430, 14001, 14924, 14001,
   14721,    88,   137, -1198,   566, 11639, -1198, -1198, -1198,  7016,
      85, -1198, 14924, 14924,   -52, -1198, -1198, -1198, 11639, 13541,
   13628,  9671,  7619,   563,   567, -1198,   109,   642, -1198,   757,
     572, 13312, 13452, 13628, 13628, 13628,   582,    76,   635,   584,
     588,    16, -1198,   637, -1198,   586, -1198, -1198, -1198, 11639,
     608, 14924,    39, 14924,   771, 13157,   786, -1198, 14924, 13111,
   -1198,   516,   722, -1198,  4403, 14280,   600,  9671, -1198, -1198,
   12267, -1198, -1198,   784, 14001,   606, -1198, -1198, -1198, -1198,
   -1198,   712,   113, 14280,   607, 13863, 13947,   791, -1198, -1198,
   -1198, -1198,   609, -1198, 11639, -1198, -1198,  3597, -1198, 14924,
   14280,   611, -1198, -1198, -1198, -1198,   793, 11639,   561, -1198,
   -1198,   615, -1198, 13452,   787,    91, -1198, -1198,   107, 14161,
   -1198,   141, -1198, -1198, 13452, -1198,   474, -1198, 11237, -1198,
   13628,    92,   622, 14280,   587, -1198, -1198,  4188, 11639, -1198,
   -1198, 11639, -1198, 11639, -1198, 12308,   626, 10031,   585,   587,
     612,  9671, 14066,   474, 12349,   628, 10031, -1198, -1198,   142,
   -1198, -1198,   799,  3033,  3033,  2374, -1198, -1198, -1198,   629,
      93,   630,   634, -1198, -1198, -1198,   806,   631,   483,   474,
     474, 11438, -1198,   146, -1198, -1198, 12397,   311,   -52,  8222,
   -1198,  4604,   638,  4805,   639, 14001,   636,   694,   474, -1198,
     805, -1198, -1198, -1198, -1198,   392, -1198,   165, 13452, -1198,
   13452,   610, -1198, -1198, -1198,   812,   641,   643, -1198, -1198,
     708,   640,   824, 13628,   700,  9671,   561, 13628,  9068, 13628,
   14924, 11639, 11639, 11639, -1198, -1198, -1198, 11639, 11639, -1198,
     334, -1198,   766, -1198, -1198, -1198, -1198, 13628,   474, -1198,
   13452,  9671, -1198,   833, -1198, -1198,   112,   649,   474,  8825,
   -1198,    63, -1198,  3800,   833, -1198,   210,   196, 14924,   724,
   -1198,   652, 13452,   619, -1198, 13452,   775,   831, 11639,    11,
     656, -1198, 14001, 14924, -1198,   657,    92, -1198,   658,    92,
     672,  4188, 14924, 14776,   687, 10031,   668,   685,   686,   587,
   -1198,   416,   690, 10031,   695, 11639, -1198, -1198, -1198, -1198,
   -1198,   768,   688,   883,  2374,   754, -1198,   561,  2374,  2374,
   -1198, -1198, -1198, 14001, 14924, -1198,   -52,   867,   826,  8222,
   -1198, -1198, -1198,   701, 11639,   474, 13863, 13541,   703, 13628,
    5006,   432,   704, 11639,    29,   229, -1198,   731, -1198, -1198,
   13382,   874, -1198, 13628, -1198, 13628, -1198,   713, -1198,   780,
     895,   720,   725, -1198, -1198,   801,   727,   898, 14924, 13240,
   14924, -1198, 14924, -1198,   733, -1198, -1198, -1198,   819, 14280,
     905, -1198, 13863, -1198, -1198,  2290,   728, -1198,   977, -1198,
     241, 11639, -1198, -1198, -1198, 11639, -1198, 11639, -1198, -1198,
   -1198,   320,   908, 13628, 12438,   735, 10031,   474,   787,   736,
   -1198,   737,    92, 11639, 10031,   738, -1198, -1198, -1198,   739,
     740, -1198, 10031,   742, -1198,  2374, -1198,  2374, -1198,   743,
   -1198,   816,   745,   932,   750, -1198,   474,   919, -1198,   755,
   -1198, -1198,   758,   115, -1198, -1198, -1198,   756,   760, -1198,
   12574, -1198, -1198, -1198, -1198, -1198, -1198, 13452, -1198,   834,
   -1198, 13628,   561, -1198, -1198, -1198, 13628, -1198, 13628, -1198,
   11639,   762,  5207, 13452, -1198,   262, 13452, 14280, -1198, 14260,
     808,  3245, -1198, -1198, -1198,   569,  2643,    69,   682,   120,
   -1198, -1198,   813, 12479, 12520, 14924,   888,   949, 13628, -1198,
     770, 10031,   772,   863,   787,  1174,   787,   774, 14924,   776,
   -1198,  1374,  1517, -1198,    92,   779, -1198, -1198,   847, -1198,
    2374, -1198,   561, -1198, -1198, -1198,  7016, -1198, -1198, -1198,
    7820, -1198, -1198, -1198,  7016,   782, 13628, -1198,   855, -1198,
     856, 12628, -1198, -1198, -1198, 14280, 14280,   966,    41, -1198,
   -1198, -1198,    70,   785,    71, -1198, 12638, -1198, -1198,    74,
   -1198, -1198, 14211, -1198,   788, -1198,   909,   417, -1198, 13452,
   -1198,   569, -1198, -1198, -1198, -1198, -1198,   971, 13628, -1198,
   10031,   792, -1198,   794,   789,   228, -1198,   863,   787, -1198,
   -1198, -1198,  1589,   796, -1198,  2374, -1198,   866,  7016,  8021,
   -1198, -1198, -1198,  7016, -1198, -1198, 13628, 13628, 11639,  5408,
     798,   802, 13628, 14280, -1198, -1198,  1362, 14260, -1198, -1198,
   -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198,
   -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198,
   -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198,
   -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198,
   -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198,
   -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198,
   -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198,
   -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198,   444,
   -1198,   808, -1198, -1198, -1198, -1198, -1198,   105,   345, -1198,
     978,    75,  9671,   909,   981,   417, 13628, -1198,   804, -1198,
     336, -1198, -1198, -1198, -1198,   809,   228, -1198,   787, -1198,
    2374, -1198, -1198, -1198,  5609, -1198, -1198, 13199, -1198, -1198,
   -1198, -1198, -1198,   378,    26, -1198, -1198, 13628, 12638, 12638,
     950, -1198, 14211, 14211,   418, -1198, -1198, -1198, 13628,   927,
   -1198,   814,    81, 13628,  9671, -1198, -1198,   935, -1198,  1002,
    5810,  6011, -1198, -1198,   228, -1198,  6212,   825,   946,   920,
   -1198,   931,   882, -1198, -1198,   934,  1362, -1198, -1198, -1198,
   -1198,   871, -1198,   997, -1198, -1198, -1198, -1198, -1198,  1014,
   -1198, -1198, -1198,   835, -1198,   338,   836, -1198, -1198,  6413,
   -1198,   837, -1198, -1198,   838,   873,  9671,   682, -1198, -1198,
   13628,    96, -1198,   961, -1198, -1198, -1198, -1198, 14280,   600,
   -1198,   879,  9671,   496, -1198, -1198,   844,  1032,   440,    96,
   -1198,   963, -1198, 14280,   848, -1198,   787,   117, -1198, -1198,
   -1198, -1198, 13452, -1198,   850,   851,   103, -1198,   236, -1198,
     440,   341,   787,   846, -1198, -1198, -1198, -1198, 13452,   976,
    1037,   236, -1198,  6614,   343,  1038, 13628, -1198,  6815, -1198,
     979,  1040, 13628, -1198, -1198,  1043, 13628, -1198, 13628, -1198,
   -1198
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1198, -1198, -1198,  -470, -1198, -1198, -1198,    -4, -1198,   577,
     -10,   893,  1990, -1198,  1633, -1198,  -135, -1198,     3, -1198,
   -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198,  -409,
   -1198, -1198,  -165,    13,     2, -1198, -1198, -1198,     8, -1198,
   -1198, -1198, -1198,     9, -1198, -1198,   691,   710,   711,   936,
     274,  -657,   295,   351,  -390, -1198,   118, -1198, -1198, -1198,
   -1198, -1198, -1198,  -549,     6, -1198, -1198, -1198, -1198,  -382,
   -1198,  -741, -1198,  -366, -1198, -1198,   614, -1198,  -861, -1198,
   -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198,  -159,
   -1198, -1198, -1198, -1198, -1198,  -242, -1198,   -17,  -901, -1198,
   -1197,  -404, -1198,  -155,    20,  -129,  -398, -1198,  -244, -1198,
     -70,   -22,  1072,  -619,  -349, -1198, -1198,   -34, -1198, -1198,
    2582,   -53,  -116, -1198, -1198, -1198, -1198, -1198, -1198,   200,
    -710, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198,
   -1198,   741, -1198, -1198,   238, -1198,   648, -1198, -1198, -1198,
   -1198, -1198, -1198, -1198,   239, -1198,   653, -1198, -1198,   419,
   -1198,   215, -1198, -1198, -1198, -1198, -1198, -1198, -1198, -1198,
    -874, -1198,  1322,  1179,  -333, -1198, -1198,   181,   842,  2014,
   -1198, -1198,  -696,  -353,  -550, -1198, -1198,   319,  -626,  -784,
   -1198, -1198, -1198, -1198, -1198,   306, -1198, -1198, -1198,  -266,
    -719,  -188,  -187,  -133, -1198, -1198,    27, -1198, -1198, -1198,
   -1198,    -8,  -147, -1198,  -229, -1198, -1198, -1198,  -384,   849,
   -1198, -1198, -1198, -1198, -1198,   437,   303, -1198, -1198,   854,
   -1198, -1198, -1198,  -307,   -81,  -191,  -284, -1198, -1021, -1198,
     275, -1198, -1198, -1198,  -178,  -894
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -792
static const yytype_int16 yytable[] =
{
     118,   372,   330,   769,   815,   297,   127,   125,   226,   300,
     301,   400,   128,   129,   555,   627,   255,   126,   231,   398,
     219,  1041,   235,   418,   131,   604,   393,  1143,   624,   586,
     925,   158,   423,   424,   550,  1426,   915,   534,   909,   834,
     304,   661,   238,   265,  1028,   248,   737,  -405,   330,   327,
    1253,   205,   206,   633,   644,   321,  1091,   306,   432,   482,
     764,   663,   278,   445,   446,   292,  -689,   487,   293,   451,
     714,   490,   388,   337,   338,   339,   714,   395,  1209,  -251,
    1257,   181,   278,  1341,  1399,    11,   278,   278,   425,   340,
    1399,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,  1253,   362,  1392,   847,   272,   928,   848,   726,
     726,   726,   891,  1225,   726,  1227,   278,   317,   478,   726,
     222,   408,   938,    11,    11,  1393,   589,   307,    11,   916,
     451,   388,   225,   416,   329,   388,  -686,     3,   736,   391,
     391,   505,   828,   829,   266,   864,   999,  -571,  -791,    11,
    -405,   391,   622,  -687,   559,   232,   625,   997,   233,  1413,
    1069,  1002,   955,   749,  1074,   318,   373,   517,   518,   234,
     318,   917,   405,  -688,  -570,  1049,   479,  -693,  1051,  1145,
     401,  -724,  -690,   274,   395,  -725,  1151,  1152,  -695,  -727,
     199,  -689,   531,  -691,  -692,  -791,   495,  1366,  -726,   759,
     422,   506,   396,  -791,   538,   285,   765,  1450,   642,   872,
     270,   597,   118,   496,  1427,  1092,   118,   662,   409,   429,
     439,   664,   956,   434,   411,  -198,   628,  1254,  1255,   784,
     417,   597,   322,   557,   330,   433,   483,  1071,   453,   199,
     914,   531,   481,   158,   488,  1475,   715,   158,   491,   983,
     984,   743,   716,   597,  1026,  1210,  -251,  1258,   903,   666,
    1342,  1400,   597,   271,   849,   597,  1499,  1441,   892,   267,
     515,  -686,  1059,  -621,   392,   392,  -621,  -198,  1132,   297,
     327,  -186,  1232,   649,   486,   981,   392,   985,  -687,  1504,
    1394,   492,   492,   497,   118,   727,   802,  1021,   502,   298,
    1169,  1147,  -621,   549,   511,  1212,   852,   248,  -688,  -696,
     278,   126,  -693,   983,   984,   397,  -724,  -690,   131,   396,
    -725,   283,   259,  1075,  -727,   158,   512,   634,  -691,  -692,
     605,   896,   704,  -726,   484,  1034,   707,  1414,   760,   761,
     751,   752,  1136,   283,   219,  1082,   283,   757,   310,  1029,
     596,   512,   744,   986,   595,   278,   278,   278,  1407,   309,
    1463,  1395,  1030,  1509,   113,  1520,   283,   745,   966,   967,
     621,   313,  1185,  1186,   620,   601,   275,   855,  1396,   298,
     283,  1397,  1178,  1035,  1137,   284,   276,   286,   287,   317,
     283,   317,   596,   283,   786,   512,   636,  1031,   512,   283,
    1408,   643,  1464,  1233,   647,  1510,   283,  1521,   646,   286,
     287,   936,   286,   287,  1362,  1363,   502,  1094,   277,   795,
     944,   118,  1505,  1506,   409,   316,   791,   281,   656,  1157,
     282,  1158,   286,   287,  1435,   786,   299,   760,   761,   323,
      33,  -791,  1237,   822,   823,   285,   286,   287,  1387,   941,
     548,  1436,   158,   960,  1437,  1498,   286,   287,   513,   286,
     287,  1388,   265,   318,   507,   286,   287,   317,   776,   320,
     709,  1511,   286,   287,   547,   982,   983,   984,  1389,   534,
     418,  1433,  1434,   451,   857,   721,   359,   360,   361,   368,
     362,   731,   733,  -791,   331,   534,  -791,  -427,   824,   996,
     332,   458,   459,   460,  1429,  1430,   831,   333,   461,   462,
     833,   174,   463,   464,    78,  1088,   983,   984,    81,    82,
     364,    83,    84,    85,  1236,   402,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,  1489,  1490,  1491,
    1023,   312,   314,   315,    95,   278,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,  1055,
     334,   335,   336,   365,   367,   911,  1420,  1063,   766,   366,
     394,   386,   387,  -694,  -428,  -570,   921,   508,   399,   404,
     362,   514,  1083,   290,   318,   410,   388,   413,  1501,   414,
    1068,  -569,   419,    55,    56,    57,   170,   171,   328,  1369,
     420,    48,   597,   508,  1514,   514,   508,   514,   514,    55,
      56,    57,   170,   171,   328,   422,   813,   430,   818,   793,
     443,   447,   832,   448,  -786,   452,   454,   455,  1129,   489,
     499,   524,   457,   388,   498,   118,   528,  1103,   356,   357,
     358,   359,   360,   361,  1109,   362,   529,   840,   118,   519,
     534,   536,   126,   534,   819,   842,   820,   537,    48,   131,
     987,    94,   988,   546,   -51,   556,   158,   630,   632,   635,
    1142,   641,   654,   432,   658,   660,   838,    94,  1149,   158,
     667,   671,  1485,   672,   689,   705,  1155,   690,   694,   702,
     118,   713,   717,   885,   718,   723,   980,   734,   740,   720,
     940,   725,  1017,   728,   742,   747,   538,   126,   748,   750,
    1163,   753,   754,   755,   131,  -429,   767,   768,  1045,   770,
     779,   158,   773,   118,  1039,   780,   782,   757,   783,   127,
     125,   888,  1478,   787,  1415,   128,   129,   796,   797,   799,
     126,   920,   502,   898,  1128,   919,   800,   131,   775,   844,
    1478,   825,  1128,   846,   158,   850,   851,   853,  1500,    55,
      56,    57,    58,    59,   328,  1221,   863,   865,   866,   868,
      65,   369,   867,   869,   219,   873,   534,   278,   871,    55,
      56,    57,   170,   171,   328,   877,   880,   883,   887,   948,
     948,   813,   889,   890,   895,   899,   969,   907,   905,  1187,
     900,   910,   921,   945,   912,   959,   926,   977,   370,   979,
     935,   990,   943,   954,   957,   118,   961,   118,   958,   118,
     976,   993,   970,   995,   972,   974,   991,    94,   992,   507,
    1014,   994,  1019,  1022,   126,  1043,   126,  1037,  1038,  1042,
    1046,   131,  1048,   131,  1358,  1050,   158,    94,   158,  1446,
     158,   998,   975,  1056,  1004,  1024,  1052,   374,   375,   376,
     377,   378,   379,   380,   381,   382,   383,   384,   385,  1128,
    1112,  1054,  1057,  1058,  1062,  1128,  1128,  1018,   534,  1066,
    1064,  1065,  1067,  1070,  1078,  1079,  1095,  1081,  1085,   118,
    1089,  1097,  1354,  1101,  1102,   127,   125,  1108,  1100,  1115,
    1113,   128,   129,   386,   387,  1104,   126,   202,   202,  1175,
    1105,   214,  1138,   131,  1106,  1107,  1130,  1488,  1111,  1141,
     158,  1144,  1146,  1150,  1154,  1184,  1153,  1156,  1159,  1160,
    1161,  1162,   214,  1166,  1077,  1164,  1165,    11,  1208,  1047,
     813,  1167,  1171,  1168,   813,   813,  1172,  1176,  1174,  1182,
    1198,  1213,  1217,  1218,  1220,   118,  1128,  1222,  1223,  1228,
    1235,  1229,  1080,  1211,  1234,   388,   118,  1244,  1246,  1247,
    1252,  1115,  1256,  1350,  1349,  1356,  1361,  1359,  1360,  1370,
    1076,  1368,  1398,   126,  1379,  1403,   158,   330,  1380,  1406,
     131,  1439,  1431,   502,   838,  1116,  1412,   158,  1440,  1444,
    1117,  1445,    55,    56,    57,   170,   171,   328,  1118,    11,
    1453,  1452,  -247,  1454,  1456,  1457,  1459,  1393,  1460,  1462,
    1238,  1465,  1468,  1467,  1469,  1480,  1483,  1495,  1243,  1486,
    1126,  1487,  1352,  1512,  1497,  1502,  1503,  1249,  1126,   502,
    1515,  1516,  1522,  1525,  1526,  1119,  1120,  1528,  1121,   708,
    1482,   813,   600,   813,   939,  -792,  -792,  -792,  -792,   354,
     355,   356,   357,   358,   359,   360,   361,  1116,   362,   598,
      94,   599,  1117,   937,    55,    56,    57,   170,   171,   328,
    1118,   904,   371,  1496,   202,  1084,  1173,  1494,  1386,  1391,
     202,   711,  1508,  1122,  1205,  1374,   202,  1517,   118,  1402,
     228,  1365,   248,  1040,   700,   607,  1011,  1203,  1013,   701,
     879,  1032,  1061,   950,   962,   126,   989,  1119,  1120,   494,
    1121,   504,   131,     0,   214,   214,     0,     0,  1207,   158,
     214,     0,     0,     0,     0,     0,     0,     0,   373,     0,
       0,     0,    94,     0,     0,     0,   813,     0,     0,     0,
     202,     0,   118,     0,     0,  1126,   118,   202,   202,     0,
     118,  1126,  1126,  1241,   202,  1131,     0,     0,  1115,   126,
     202,     0,     0,     0,     0,     0,   131,   126,     0,     0,
    1404,     0,  1338,   158,   131,   534,     0,   158,  1345,     0,
       0,   158,     0,   204,   204,   248,     0,   216,     0,     0,
       0,   214,     0,   534,   214,     0,    11,     0,     0,     0,
       0,   534,     0,     0,  1355,     0,  1410,     0,  1411,     0,
       0,   813,     0,     0,   118,   118,  1416,     0,     0,   118,
       0,     0,  1372,     0,     0,   118,     0,     0,     0,     0,
       0,   126,  1126,     0,     0,   214,   126,     0,   131,     0,
       0,     0,   126,   131,     0,   158,   158,     0,     0,   131,
     158,  1401,     0,     0,  1116,     0,   158,  1449,     0,  1117,
       0,    55,    56,    57,   170,   171,   328,  1118,     0,     0,
       0,     0,   202,     0,   686,     0,     0,     0,     0,     0,
     202,     0,  1472,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   757,     0,     0,     0,     0,     0,
       0,     0,     0,  1443,  1119,  1120,     0,  1121,     0,     0,
     757,   686,     0,     0,     0,     0,     0,     0,     0,     0,
     214,     0,     0,     0,   330,   681,   203,   203,   278,    94,
     215,   402,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,     0,     0,     0,   813,     0,     0,     0,
     118,  1513,  1226,     0,     0,     0,     0,  1518,  1115,  1421,
     204,     0,   681,     0,  1338,  1338,   204,   126,  1345,  1345,
       0,     0,   204,     0,   131,     0,     0,   386,   387,     0,
     278,   158,     0,     0,     0,     0,   118,   118,     0,     0,
       0,     0,   118,     0,     0,     0,    11,     0,     0,     0,
       0,   214,   214,   126,   126,     0,     0,     0,   214,   126,
     131,   131,    31,    32,     0,     0,   131,   158,   158,     0,
       0,     0,    37,   158,   202,   118,   204,     0,     0,     0,
       0,     0,  1471,   204,   204,     0,     0,     0,     0,   388,
     204,     0,   126,     0,     0,     0,   204,     0,  1484,   131,
       0,     0,     0,     0,  1116,     0,   158,  1473,     0,  1117,
       0,    55,    56,    57,   170,   171,   328,  1118,     0,    69,
      70,    71,    72,    73,     0,   202,     0,     0,     0,     0,
     677,     0,   686,     0,     0,     0,    76,    77,     0,   118,
       0,     0,     0,     0,   118,   686,   686,   686,     0,     0,
      87,  1115,     0,     0,  1119,  1120,   126,  1121,     0,   203,
     202,   126,   202,   131,     0,    92,     0,     0,   131,     0,
     158,   216,     0,     0,     0,   158,     0,     0,     0,    94,
       0,     0,   202,   681,     0,     0,     0,     0,     0,    11,
       0,     0,     0,     0,   214,   214,   681,   681,   681,     0,
       0,     0,  1230,     0,     0,     0,     0,     0,   204,     0,
       0,     0,     0,     0,     0,     0,   204,     0,     0,   203,
       0,     0,     0,  1115,     0,     0,   203,   203,   214,     0,
       0,     0,     0,   203,     0,     0,     0,   202,     0,   203,
       0,     0,   686,     0,     0,     0,   214,  1116,   202,   202,
       0,     0,  1117,     0,    55,    56,    57,   170,   171,   328,
    1118,    11,     0,   214,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   214,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   214,     0,     0,
       0,     0,     0,   681,     0,     0,   214,  1119,  1120,     0,
    1121,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   249,     0,   215,   214,     0,     0,     0,  1116,
       0,     0,    94,     0,  1117,     0,    55,    56,    57,   170,
     171,   328,  1118,     0,     0,   686,     0,     0,     0,   686,
       0,   686,     0,     0,     0,  1231,    33,     0,   199,     0,
       0,   203,     0,     0,     0,     0,     0,     0,   202,   686,
     204,     0,     0,     0,     0,     0,     0,     0,     0,  1119,
    1120,   214,  1121,   214,   402,   375,   376,   377,   378,   379,
     380,   381,   382,   383,   384,   385,   681,     0,     0,     0,
     681,     0,   681,     0,    94,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   685,     0,     0,     0,     0,     0,
     681,   204,     0,   214,     0,     0,     0,  1367,     0,     0,
     386,   387,     0,     0,    81,    82,     0,    83,    84,    85,
       0,     0,     0,     0,     0,   214,     0,     0,   214,     0,
       0,   685,     0,     0,     0,   202,   204,     0,   204,     0,
      95,   686,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   686,     0,   686,   204,     0,
       0,   619,     0,   113,     0,     0,     0,     0,     0,     0,
       0,     0,   388,     0,     0,     0,   202,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   202,
     202,     0,   681,   203,   249,   249,     0,     0,     0,     0,
     249,     0,     0,   214,     0,   686,   681,     0,   681,     0,
       0,     0,     0,   204,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   204,   204,     0,     0,     0,     0,
       0,     0,   214,     0,     0,   202,     0,     0,     0,     0,
       0,     0,     0,     0,   203,   719,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   681,     0,     0,     0,
       0,     0,     0,   686,     0,     0,     0,     0,   686,     0,
     686,   249,     0,     0,   249,     0,     0,     0,     0,   203,
       0,   203,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   216,     0,     0,     0,     0,     0,     0,     0,     0,
     686,   203,   685,     0,     0,     0,     0,     0,     0,     0,
     214,     0,     0,     0,   681,   685,   685,   685,     0,   681,
       0,   681,     0,     0,     0,     0,   214,     0,     0,   214,
     214,     0,   214,     0,   204,     0,     0,     0,   686,   214,
       0,     0,     0,     0,     0,     0,     0,   882,     0,     0,
       0,   681,     0,     0,     0,     0,   203,     0,     0,     0,
       0,     0,     0,     0,     0,   894,     0,   203,   203,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     686,     0,   894,     0,     0,     0,     0,     0,     0,   681,
       0,     0,     0,     0,     0,     0,     0,     0,   214,   214,
     249,     0,     0,     0,     0,   683,     0,     0,   686,   686,
       0,     0,   685,     0,   686,   927,     0,     0,  1385,     0,
       0,   204,   214,     0,     0,     0,     0,     0,     0,     0,
       0,   681,     0,     0,   215,     0,     0,     0,     0,     0,
       0,     0,   683,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   681,
     681,     0,   204,     0,    33,   681,   214,     0,     0,     0,
     214,     0,     0,     0,     0,   204,   204,   203,     0,     0,
       0,   249,   249,     0,     0,     0,     0,     0,   249,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,   685,   362,     0,     0,   685,
       0,   685,     0,     0,     0,     0,     0,     0,   686,     0,
       0,   204,     0,     0,     0,     0,     0,     0,     0,   685,
       0,     0,     0,     0,     0,     0,     0,     0,   290,     0,
       0,     0,    81,    82,     0,    83,    84,    85,     0,   686,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     686,     0,     0,     0,   203,   686,     0,     0,    95,   681,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,     0,     0,     0,     0,     0,  1458,   291,
       0,     0,     0,     0,     0,     0,   214,     0,     0,     0,
     681,     0,     0,     0,     0,   203,     0,     0,     0,     0,
       0,   681,     0,   683,     0,     0,   681,     0,   203,   203,
       0,   685,   686,     0,   249,   249,   683,   683,   683,     0,
       0,     0,     0,     0,     0,   685,   340,   685,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,     0,
     362,  1114,     0,     0,   203,     0,     0,     0,     0,     0,
       0,     0,     0,   681,     0,     0,     0,     0,   686,     0,
       0,   214,     0,     0,   686,   685,     0,     0,   686,     0,
     686,     0,     0,     0,     0,     0,   214,     0,     0,     0,
       0,     0,     0,     0,     0,   214,   249,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   249,     0,     0,
       0,   214,     0,   683,     0,     0,     0,     0,     0,   681,
       0,     0,     0,     0,     0,   681,     0,   803,   804,   681,
       0,   681,     0,   685,     0,     0,     0,     0,   685,     0,
     685,     0,     0,     0,     0,     0,   805,     0,     0,  1189,
       0,  1197,   682,     0,   806,   807,    33,     0,     0,     0,
       0,     0,     0,     0,   808,     0,     0,     0,     0,     0,
     685,     0,     0,     0,     0,     0,   687,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   682,
       0,   249,     0,   249,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   683,     0,   685,   809,
     683,     0,   683,   712,     0,     0,     0,  1250,  1251,     0,
       0,     0,   810,     0,     0,     0,     0,     0,     0,     0,
     683,     0,     0,   249,    81,    82,     0,    83,    84,    85,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     685,     0,   811,     0,     0,   249,     0,     0,   249,     0,
     812,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,     0,     0,     0,   685,   685,
       0,     0,     0,     0,   685,  1382,     0,     0,     0,  1197,
       0,     0,     0,     0,     0,     0,     0,   178,   180,     0,
     182,   183,   184,     0,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,     0,     0,   208,   211,
       0,     0,   683,     0,     0,     0,     0,     0,     0,     0,
     229,     0,     0,   249,     0,     0,   683,   237,   683,   240,
       0,     0,   256,     0,   261,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     682,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   295,     0,   682,   682,   682,     0,     0,     0,   241,
       0,     0,     0,     0,   839,   302,   683,     0,   685,     0,
       0,     0,     0,     0,     0,     0,     0,   858,   859,     0,
       0,     0,   305,     0,     0,   242,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   685,
       0,     0,     0,     0,     0,    33,     0,     0,     0,     0,
     685,     0,     0,     0,     0,   685,     0,     0,     0,     0,
     249,     0,     0,     0,   683,     0,     0,     0,     0,   683,
       0,   683,  -288,     0,     0,     0,   249,     0,     0,   249,
      55,    56,    57,   170,   171,   328,     0,     0,     0,   249,
     682,     0,     0,   403,     0,     0,     0,     0,   243,   244,
       0,   683,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   685,     0,   924,     0,   174,     0,     0,    78,
    1481,   245,     0,    81,    82,     0,    83,    84,    85,     0,
       0,     0,     0,     0,   427,  1189,     0,   427,     0,   683,
       0,   246,     0,     0,   229,   438,     0,     0,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,     0,     0,     0,   247,   685,     0,
       0,     0,   249,     0,   685,     0,     0,     0,   685,     0,
     685,   683,     0,   682,     0,     0,     0,   682,   305,   682,
       0,     0,     0,     0,   208,     0,     0,     0,   510,     0,
       0,     0,     0,     0,     0,     0,     0,   682,     0,   683,
     683,     0,     0,  1005,     0,   683,     0,     0,     0,     0,
       0,   545,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1016,   554,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   560,
     561,   562,   564,   565,   566,   567,   568,   569,   570,   571,
     572,   573,   574,   575,   576,   577,   578,   579,   580,   581,
     582,   583,   584,   585,     0,     0,   587,   587,     0,   590,
       0,     0,     0,     0,     0,     0,   606,   608,   609,   610,
     611,   612,   613,   614,   615,   616,   617,   618,     0,   682,
       0,     0,     0,   587,   623,     0,   554,   587,   626,     0,
       0,     0,     0,   682,   606,   682,     0,     0,     0,   683,
       0,     0,     0,  1086,   638,     0,   640,     0,     0,     0,
       0,     0,   554,     0,     0,     0,     0,  1098,     0,  1099,
       0,     0,   652,     0,   653,     0,  1422,     0,     0,     0,
     683,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   683,     0,   682,     0,     0,   683,     0,     0,     0,
       0,   691,   693,     0,   695,   698,   699,     0,     0,     0,
     337,   338,   339,     0,     0,     0,     0,  1139,     0,     0,
       0,     0,     0,     0,     0,     0,   340,   710,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,     0,
     362,   682,     0,   683,     0,     0,   682,     0,   682,     0,
       0,     0,   739,   946,   947,    33,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1177,     0,     0,     0,     0,
    1179,     0,  1180,     0,     0,   249,     0,     0,   682,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   249,     0,     0,     0,   777,     0,     0,     0,   683,
       0,     0,  1219,     0,     0,   683,     0,     0,     0,   683,
       0,   683,     0,     0,     0,     0,   682,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   785,     0,     0,     0,
       0,     0,     0,    81,    82,   295,    83,    84,    85,     0,
    1245,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   794,     0,     0,     0,     0,     0,     0,   682,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,    33,     0,   199,   826,     0,     0,
       0,     0,  1357,     0,     0,     0,   682,   682,   735,     0,
     229,     0,   682,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   337,   338,   339,     0,     0,     0,
    1375,  1376,     0,     0,     0,     0,  1381,     0,     0,     0,
     340,   870,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,     0,   362,     0,     0,     0,     0,     0,
       0,     0,    81,    82,     0,    83,    84,    85,     0,     0,
       0,     0,     0,     0,     0,     0,   901,    33,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    95,   908,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,     0,     0,     0,   682,     0,     0,   594,
     923,   113,     0,     0,     0,     0,     0,     0,     0,     0,
     931,  1201,     0,   932,     0,   933,     0,     0,     0,   554,
    1405,     0,     0,     0,     0,     0,     0,   682,   554,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   682,     0,
       0,     0,     0,   682,     0,    81,    82,     0,    83,    84,
      85,  1428,     0,   964,     0,     0,     0,     0,     0,     0,
       0,     0,  1438,     0,     0,     0,     0,  1442,     0,     0,
       0,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,     0,     0,     0,  1202,
       0,     0,   763,     0,     0,     0,     0,     0,     0,     0,
     682,     0,     0,  1008,  1009,  1010,     0,     0,     0,   695,
    1012,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1474,     0,     0,     0,     0,     0,
       0,  1025,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
    1044,   362,     0,     0,     0,     0,   682,     0,     0,     0,
       0,     0,   682,     0,     0,     0,   682,   554,   682,     0,
       0,     0,     0,     0,     0,   554,     0,  1025,     0,     0,
    1523,     0,     0,     0,     0,     0,  1527,     0,     0,     0,
    1529,     0,  1530,     0,     0,     0,     0,     0,   337,   338,
     339,     0,     0,     0,     0,     0,   229,     0,     0,     0,
       0,     0,     0,     0,   340,  1090,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,     0,   362,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,  1133,     0,     0,     0,  1134,     0,  1135,
       0,     0,     0,     0,     0,     0,     0,     0,   554,     0,
       0,     0,     0,     0,     0,  1148,   554,     0,     0,    11,
      12,    13,     0,     0,   554,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,     0,    41,     0,    42,     0,    43,     0,     0,    44,
       0,     0,  1181,    45,    46,    47,    48,    49,    50,    51,
       0,    52,    53,    54,    55,    56,    57,    58,    59,    60,
       0,    61,    62,    63,    64,    65,    66,     0,     0,     0,
       0,    67,    68,   554,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,   798,     0,     0,     0,
      75,    76,    77,    78,    79,    80,     0,    81,    82,     0,
      83,    84,    85,    86,     0,    87,     0,     0,     0,    88,
       0,     0,     0,     0,     0,    89,    90,     0,    91,     0,
      92,    93,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,   110,     0,   111,   112,   902,   113,   114,     0,   115,
     116,     0,   554,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,  -792,  -792,  -792,  -792,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
    1377,   362,     0,     0,     0,     0,     0,     0,     0,     0,
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
     108,   109,     0,     0,   110,     0,   111,   112,  1027,   113,
     114,   339,   115,   116,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,   340,     0,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,     0,   362,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,     0,    41,     0,    42,     0,
      43,     0,     0,    44,     0,     0,     0,    45,    46,    47,
      48,    49,    50,    51,     0,    52,    53,    54,    55,    56,
      57,    58,    59,    60,     0,    61,    62,    63,    64,    65,
      66,     0,     0,     0,     0,    67,    68,     0,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,    75,    76,    77,    78,    79,    80,
       0,    81,    82,     0,    83,    84,    85,    86,     0,    87,
       0,     0,     0,    88,     0,     0,     0,     0,     0,    89,
      90,     0,    91,     0,    92,    93,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,   110,     0,   111,   112,     0,
     113,   114,     0,   115,   116,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,     0,   362,     0,
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
     539,   113,   114,     0,   115,   116,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,     0,   362,     0,
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
     112,   881,   113,   114,     0,   115,   116,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,     0,   362,     0,
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
     111,   112,   971,   113,   114,     0,   115,   116,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,     0,   362,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
       0,    36,     0,     0,     0,    37,    38,    39,    40,   973,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,     0,    43,  1087,     0,    44,     0,
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
     110,     0,   111,   112,     0,   113,   114,     0,   115,   116,
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
       0,   110,     0,   111,   112,  1183,   113,   114,     0,   115,
     116,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
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
       0,     0,   110,     0,   111,   112,  1378,   113,   114,     0,
     115,   116,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,     0,    41,     0,    42,  1417,    43,     0,
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
     109,     0,     0,   110,     0,   111,   112,     0,   113,   114,
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
     107,   108,   109,     0,     0,   110,     0,   111,   112,  1448,
     113,   114,     0,   115,   116,     5,     6,     7,     8,     9,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,     0,    41,  1451,    42,
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
       0,   113,   114,     0,   115,   116,     5,     6,     7,     8,
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
     112,  1466,   113,   114,     0,   115,   116,     5,     6,     7,
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
     111,   112,  1519,   113,   114,     0,   115,   116,     5,     6,
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
       0,   111,   112,  1524,   113,   114,     0,   115,   116,     5,
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
     110,     0,   111,   112,     0,   113,   114,     0,   115,   116,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   428,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,   655,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,   841,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,  1240,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,  1371,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,     0,    50,    51,     0,    52,     0,    54,    55,
      56,    57,   170,   171,    60,     0,    61,    62,    63,     0,
       0,     0,     0,     0,     0,     0,    67,    68,     0,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
      74,     0,     0,     0,     0,   174,    76,    77,    78,    79,
      80,     0,    81,    82,     0,    83,    84,    85,     0,     0,
      87,     0,     0,     0,    88,     0,     0,     0,     0,     0,
      89,     0,     0,     0,     0,    92,    93,    94,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,   110,     0,   111,   112,
       0,   113,   114,     0,   115,   116,     5,     6,     7,     8,
       9,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   602,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,    33,    34,    35,     0,     0,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      55,    56,    57,   170,   171,   172,     0,    33,    62,    63,
       0,     0,     0,     0,     0,     0,     0,   173,    68,     0,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   174,    76,    77,    78,
     603,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,    87,     0,     0,     0,    88,     0,     0,     0,     0,
       0,    89,     0,     0,     0,     0,    92,    93,    94,    95,
     253,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,    81,    82,   110,    83,    84,
      85,     0,   113,   114,     0,   115,   116,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,     0,     0,     0,   556,
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
       0,     0,    89,     0,     0,     0,     0,    92,    93,    94,
      95,   253,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,   110,     0,
     254,     0,     0,   113,   114,     0,   115,   116,     5,     6,
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
       0,     0,    55,    56,    57,   170,   171,   172,     0,    33,
      62,    63,     0,     0,     0,     0,     0,     0,     0,   173,
      68,     0,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   174,    76,
      77,    78,   603,    80,     0,    81,    82,     0,    83,    84,
      85,     0,     0,    87,     0,     0,     0,    88,     0,     0,
       0,     0,     0,    89,     0,     0,     0,     0,    92,    93,
      94,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,    81,    82,   110,
      83,    84,    85,     0,   113,   114,     0,   115,   116,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   207,     0,
       0,   775,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,     0,     0,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    55,    56,    57,   170,   171,   172,     0,
      33,    62,    63,     0,     0,     0,     0,     0,     0,     0,
     173,    68,     0,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,    74,     0,     0,     0,     0,   174,
      76,    77,    78,     0,    80,     0,    81,    82,     0,    83,
      84,    85,     0,     0,    87,     0,  1003,     0,    88,     0,
       0,     0,     0,     0,    89,     0,     0,     0,     0,    92,
       0,    94,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,    81,    82,
     110,    83,    84,    85,     0,   113,   114,     0,   115,   116,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,     0,
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
       0,   110,     0,   236,     0,     0,   113,   114,     0,   115,
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
       0,     0,   110,     0,   239,     0,     0,   113,   114,     0,
     115,   116,     5,     6,     7,     8,     9,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,    35,     0,     0,     0,     0,     0,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   294,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    55,    56,    57,   170,
     171,   172,     0,    33,    62,    63,     0,     0,     0,     0,
       0,     0,     0,   173,    68,     0,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   174,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,    81,    82,   110,    83,    84,    85,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,     0,     0,     0,     0,     0,     0,     0,     0,
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
     108,   109,     0,     0,   110,   426,     0,     0,     0,   113,
     114,     0,   115,   116,     5,     6,     7,     8,     9,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   551,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,   563,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,   602,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,   637,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,   639,     0,     0,
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
       0,     0,     0,     0,   113,   114,     0,   115,   116,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
     110,     0,     0,   651,     0,   113,   114,     0,   115,   116,
       5,     6,     7,     8,     9,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   922,
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
     963,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
     109,     0,     0,   110,     0,     0,     0,     0,   113,   114,
       0,   115,   116,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,   509,    35,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,   692,     0,     0,     0,     0,
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
     107,   108,   109,     0,     0,   110,   337,   338,   339,     0,
     113,   114,     0,   115,   116,     0,     0,     0,     0,     0,
       0,     0,   340,     0,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,     0,   362,   337,   338,   339,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   340,     0,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,     0,   362,   337,   338,
     339,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   340,     0,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,     0,   362,   337,
     338,   339,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   340,     0,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,     0,   362,
       0,     0,     0,     0,     0,     0,     0,   337,   338,   339,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   340,   801,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,     0,   362,   337,   338,
     339,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   340,   886,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,     0,   362,   337,
     338,   339,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   340,   934,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,     0,   362,
     337,   338,   339,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   340,   942,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,     0,
     362,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   337,   338,   339,     0,     0,     0,
       0,     0,     0,     0,     0,   965,     0,     0,     0,     0,
     340,  1091,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,     0,   362,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1140,     0,   337,   338,
     339,  1259,  1260,  1261,  1262,  1263,     0,     0,  1264,  1265,
    1266,  1267,     0,     0,   340,     0,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,  1215,   362,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1268,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1269,  1270,  1271,  1272,  1273,  1274,  1275,     0,     0,
      33,     0,     0,     0,     0,     0,     0,     0,  1216,  1276,
    1277,  1278,  1279,  1280,  1281,  1282,  1283,  1284,  1285,  1286,
    1287,  1288,  1289,  1290,  1291,  1292,  1293,  1294,  1295,  1296,
    1297,  1298,  1299,  1300,  1301,  1302,  1303,  1304,  1305,  1306,
    1307,  1308,  1309,  1310,  1311,  1312,  1313,  1314,  1315,  1316,
       0,     0,  1317,  1318,     0,  1319,  1320,  1321,  1322,  1323,
    1092,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1324,  1325,  1326,     0,  1327,     0,     0,    81,    82,
       0,    83,    84,    85,  1328,  1329,  1330,     0,     0,  1331,
       0,     0,     0,     0,     0,     0,  1332,  1333,     0,  1334,
    1248,  1335,  1336,  1337,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   337,
     338,   339,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   340,     0,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,     0,   362,
     337,   338,   339,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   340,     0,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,     0,
     362,   337,   338,   339,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   340,     0,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
       0,   362,   337,   338,   339,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   340,     0,
     341,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
     361,     0,   362,     0,     0,     0,     0,     0,   337,   338,
     339,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   340,   363,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,     0,   362,   337,
     338,   339,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   340,   442,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,     0,   362,
     337,   338,   339,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   340,   444,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,     0,
     362,   337,   338,   339,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   340,   456,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
       0,   362,     0,     0,     0,     0,     0,   337,   338,   339,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   340,   480,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,     0,   362,     0,   337,
     338,   339,     0,     0,    33,     0,   199,     0,     0,     0,
       0,     0,     0,     0,   629,   340,     0,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,     0,   362,
     337,   338,   339,     0,     0,     0,     0,     0,     0,     0,
     593,     0,     0,     0,     0,   648,   340,     0,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   241,
     362,     0,    81,    82,     0,    83,    84,    85,     0,     0,
       0,   878,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   242,     0,     0,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,     0,     0,    33,     0,     0,   241,   594,
       0,   113,     0,   874,   875,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   449,     0,   242,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1418,     0,    33,     0,     0,     0,   243,   244,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   174,     0,   241,    78,
       0,   245,     0,    81,    82,     0,    83,    84,    85,     0,
       0,  1110,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   246,     0,     0,   242,     0,     0,   243,   244,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,    33,   174,     0,   247,    78,     0,
     245,     0,    81,    82,     0,    83,    84,    85,     0,     0,
       0,   854,     0,     0,     0,     0,     0,     0,   241,     0,
     246,     0,     0,     0,     0,     0,     0,     0,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,     0,   242,     0,   247,   243,   244,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    33,   174,     0,     0,    78,     0,
     245,     0,    81,    82,     0,    83,    84,    85,     0,     0,
       0,  1096,     0,     0,     0,     0,     0,     0,     0,     0,
     246,     0,     0,     0,     0,     0,     0,     0,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   835,     0,     0,   247,   243,   244,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   174,     0,     0,    78,     0,
     245,     0,    81,    82,     0,    83,    84,    85,     0,     0,
       0,     0,     0,    33,     0,   199,     0,     0,     0,     0,
     246,     0,     0,     0,     0,     0,     0,     0,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,     0,     0,     0,   247,     0,     0,     0,
       0,     0,     0,   200,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   836,     0,     0,     0,     0,
       0,   673,   674,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   174,     0,     0,    78,     0,    80,
     675,    81,    82,     0,    83,    84,    85,     0,    31,    32,
      33,     0,     0,     0,     0,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,     0,     0,     0,   201,     0,     0,     0,     0,
     113,    27,    28,     0,     0,     0,     0,     0,     0,     0,
      33,     0,   199,   676,     0,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,   677,     0,     0,     0,
       0,   174,    76,    77,    78,     0,   678,     0,    81,    82,
       0,    83,    84,    85,     0,     0,    87,     0,     0,     0,
     200,     0,     0,     0,     0,     0,   679,     0,     0,     0,
       0,    92,     0,     0,   680,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,     0,
       0,   174,     0,     0,    78,     0,    80,   792,    81,    82,
       0,    83,    84,    85,     0,    33,     0,   199,     0,     0,
      88,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,     0,
       0,     0,   407,     0,     0,   200,     0,   113,     0,     0,
       0,     0,     0,     0,     0,    33,     0,   199,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   174,     0,     0,    78,
       0,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   200,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    33,     0,   199,     0,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,     0,     0,   174,   201,     0,    78,
       0,    80,   113,    81,    82,     0,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   200,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   501,     0,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,     0,     0,   174,   201,     0,    78,
     485,    80,   113,    81,    82,     0,    83,    84,    85,    33,
       0,   199,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,     0,     0,     0,   201,     0,   200,
       0,     0,   113,     0,     0,     0,     0,     0,     0,     0,
       0,   897,     0,    33,     0,   199,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     174,     0,     0,    78,     0,    80,     0,    81,    82,     0,
      83,    84,    85,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   200,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,    33,     0,
     199,   201,     0,     0,   174,     0,   113,    78,     0,    80,
       0,    81,    82,     0,    83,    84,    85,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    95,   212,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,    33,     0,   199,   201,     0,     0,     0,     0,
     113,     0,     0,     0,     0,     0,     0,     0,     0,   174,
       0,     0,    78,     0,    80,     0,    81,    82,     0,    83,
      84,    85,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    33,     0,   199,     0,     0,     0,     0,
       0,     0,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,     0,     0,     0,
     213,     0,     0,     0,     0,   113,     0,     0,     0,     0,
      81,    82,     0,    83,    84,    85,     0,     0,     0,     0,
       0,     0,     0,    33,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,    81,    82,     0,    83,    84,    85,   645,     0,   113,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    33,     0,     0,     0,     0,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,    33,     0,     0,     0,     0,     0,   918,  1343,
     113,    81,    82,  1344,    83,    84,    85,     0,     0,     0,
       0,     0,     0,  1190,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1191,     0,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,     0,   174,     0,  1202,    78,     0,  1192,     0,
      81,    82,     0,    83,  1193,    85,    33,     0,   729,   730,
       0,     0,     0,   174,     0,     0,    78,     0,    80,     0,
      81,    82,    33,    83,    84,    85,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,     0,     0,     0,     0,     0,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,     0,     0,     0,     0,    33,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    81,    82,     0,    83,    84,    85,
       0,     0,     0,     0,     0,     0,   262,     0,     0,     0,
      81,    82,     0,    83,    84,    85,     0,     0,     0,     0,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,     0,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   325,     0,    81,    82,     0,    83,    84,    85,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   337,   338,   339,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     340,     0,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,     0,   362,   337,   338,   339,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   340,     0,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,     0,   362,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     337,   338,   339,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   340,   412,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,     0,
     362,   337,   338,   339,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   340,   520,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,   361,
       0,   362,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   337,   338,   339,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   340,   781,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,   361,     0,   362,   337,   338,   339,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   340,   821,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,     0,   362,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   337,   338,   339,     0,     0,     0,  1053,
       0,     0,     0,     0,     0,     0,     0,     0,   659,   340,
     778,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,     0,   362,   337,   338,   339,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     340,     0,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,     0,   362,   338,   339,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     340,     0,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,     0,   362
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-1198))

#define yytable_value_is_error(yytable_value) \
  ((yytable_value) == (-792))

static const yytype_int16 yycheck[] =
{
       4,   156,   131,   553,   630,    86,     4,     4,    30,    90,
      91,   176,     4,     4,   321,   399,    50,     4,    40,   166,
      28,   915,    44,   214,     4,   374,   159,  1048,   394,   362,
     771,     4,   220,   220,   318,     9,   755,   303,   748,   658,
     110,     9,    46,    53,   905,    49,   516,     8,   177,   130,
       9,    24,    25,   406,   420,     9,    27,   110,     9,     9,
       9,    27,    66,   241,   242,    75,    62,     9,    78,   247,
       9,     9,   122,    10,    11,    12,     9,    62,     9,     9,
       9,   194,    86,     9,     9,    42,    90,    91,   221,    26,
       9,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,     9,    50,     9,   664,    74,   774,     9,     9,
       9,     9,     9,  1144,     9,  1146,   130,   146,   103,     9,
     194,   201,   789,    42,    42,    30,   365,   110,    42,    32,
     318,   122,   194,   213,   131,   122,    62,     0,   198,    62,
      62,    62,    67,    68,    74,    79,   866,   142,   142,    42,
       8,    62,   391,    62,   329,   194,   395,   863,   194,  1366,
     954,   867,    79,   526,   958,   164,   156,   196,   197,   194,
     164,    74,   186,    62,   142,   926,   161,    62,   929,  1050,
     177,    62,    62,   194,    62,    62,  1057,  1058,   194,    62,
      74,   197,   159,    62,    62,   194,   276,  1228,    62,   122,
     122,   122,   197,   197,   195,   139,   165,  1414,   195,   180,
     118,   368,   226,   276,   198,   196,   230,   195,   201,   226,
     234,   197,   139,   230,   207,   192,   401,   196,   197,   592,
     213,   388,   196,   324,   373,   196,   196,   957,   252,    74,
     159,   159,   262,   226,   196,   159,   195,   230,   196,    94,
      95,    47,   195,   410,   201,   196,   196,   196,   738,   447,
     196,   196,   419,   118,   165,   422,   159,   196,   165,   199,
     290,   197,   939,   192,   197,   197,   195,   195,    47,   370,
     371,   195,  1153,   428,   267,   844,   197,   846,   197,   196,
     195,   274,   275,   276,   308,   195,   195,   195,   281,   148,
     195,  1052,   195,   317,   287,   195,   669,   321,   197,   194,
     324,   308,   197,    94,    95,   199,   197,   197,   308,   197,
     197,    74,   197,   959,   197,   308,    79,   407,   197,   197,
     374,   725,   477,   197,    91,   149,    91,  1368,   536,   536,
     528,   529,    32,    74,   362,   974,    74,   535,    79,   149,
     368,    79,   148,   198,   368,   369,   370,   371,    32,   197,
      32,    26,   162,    32,   199,    32,    74,   163,    67,    68,
     388,    79,   120,   121,   388,   372,   194,   671,    43,   148,
      74,    46,  1102,   197,    74,    79,   194,   140,   141,   146,
      74,   146,   410,    74,   595,    79,   410,   197,    79,    74,
      74,   419,    74,  1154,   422,    74,    74,    74,   422,   140,
     141,   787,   140,   141,   196,   197,   399,   198,   194,   620,
     796,   435,   196,   197,   407,    27,   601,   194,   435,  1065,
     194,  1067,   140,   141,    26,   636,   194,   635,   635,    32,
      72,   142,  1162,   641,   641,   139,   140,   141,    14,   792,
     203,    43,   435,   816,    46,  1486,   140,   141,   139,   140,
     141,    27,   482,   164,   139,   140,   141,   146,   559,   194,
     484,  1502,   140,   141,   202,    93,    94,    95,    44,   755,
     681,  1392,  1393,   671,   672,   499,    46,    47,    48,   142,
      50,   505,   506,   194,   196,   771,   197,    62,   641,   862,
     196,   177,   178,   179,  1388,  1389,   651,   196,   184,   185,
     655,   143,   188,   189,   146,    93,    94,    95,   150,   151,
      62,   153,   154,   155,  1160,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,   107,   108,   109,
     899,   114,   115,   116,   176,   559,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   935,
     196,   196,   196,    62,   197,   753,   198,   943,   551,   196,
     194,    60,    61,   194,    62,   142,   764,   284,   194,    41,
      50,   288,   976,   146,   164,   142,   122,   201,  1492,     9,
     953,   142,   142,   107,   108,   109,   110,   111,   112,  1235,
     194,    99,   759,   310,  1508,   312,   313,   314,   315,   107,
     108,   109,   110,   111,   112,   122,   630,     8,   632,   602,
     196,   164,   654,   194,    14,    14,    74,   196,  1022,    14,
     164,    80,   196,   122,   195,   649,    14,  1000,    43,    44,
      45,    46,    47,    48,  1007,    50,    91,   661,   662,   196,
     926,   195,   649,   929,   637,   662,   639,   195,    99,   649,
     848,   175,   850,   200,   194,   194,   649,   194,     9,   195,
    1046,   195,    83,     9,   196,    14,   659,   175,  1054,   662,
      80,   194,   196,     9,   180,   194,  1062,    74,    74,   183,
     704,    74,   195,   707,   195,   120,   841,    62,   121,   196,
     791,   194,   890,   195,   163,   123,   195,   704,     9,   195,
    1073,    14,   192,     9,   704,    62,   195,     9,   919,    14,
     201,   704,   120,   737,   912,   201,   198,   915,     9,   737,
     737,   714,  1461,   194,  1370,   737,   737,   194,   201,   195,
     737,   759,   725,   726,  1020,   759,   201,   737,   194,   196,
    1479,   195,  1028,   196,   737,   123,     9,   195,  1487,   107,
     108,   109,   110,   111,   112,  1141,   194,   142,   194,   142,
     118,   119,   194,   197,   792,    14,  1052,   791,   180,   107,
     108,   109,   110,   111,   112,     9,    74,   197,    14,   803,
     804,   805,   196,    91,   197,    14,   828,    14,   197,  1116,
     201,   196,   990,    14,    27,     9,   194,   123,   156,    14,
     194,     9,   194,   194,   194,   829,   195,   831,   194,   833,
     194,   123,   829,     9,   196,   196,   195,   175,   195,   139,
      74,   201,     9,   194,   831,    14,   833,   123,   196,    74,
     194,   831,   195,   833,  1220,   197,   829,   175,   831,  1409,
     833,   865,   835,   195,   868,   899,   194,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,  1145,
    1015,   194,   197,   197,   194,  1151,  1152,   891,  1154,   201,
     195,   123,     9,   139,    27,    69,   165,   196,   195,   903,
     196,    27,  1209,   123,     9,   903,   903,     9,   195,     4,
      91,   903,   903,    60,    61,   195,   903,    24,    25,  1097,
     195,    28,    14,   903,   123,   198,   198,  1477,   195,   194,
     903,   195,   195,   195,   194,  1113,   197,   195,   195,   123,
     195,     9,    49,  1078,   966,   195,    27,    42,  1126,   922,
     954,   196,   196,   195,   958,   959,   196,   123,  1093,   197,
     152,   148,    74,    14,   194,   969,  1232,   195,   105,   195,
     123,   195,   969,  1128,   195,   122,   980,   195,   123,   123,
      14,     4,   197,    74,   196,    14,   197,   195,   194,   123,
     963,   195,    14,   980,   196,    14,   969,  1126,   196,   195,
     980,    74,    52,   976,   977,   100,   197,   980,   194,    74,
     105,     9,   107,   108,   109,   110,   111,   112,   113,    42,
      74,   196,    91,   103,   142,    91,   155,    30,    14,   194,
    1165,   195,   194,   196,   161,    74,   157,    74,  1173,   195,
    1020,     9,  1207,   197,   196,   195,   195,  1182,  1028,  1022,
      74,    14,    14,    74,    14,   150,   151,    14,   153,   482,
    1469,  1065,   371,  1067,   790,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,   100,    50,   369,
     175,   370,   105,   788,   107,   108,   109,   110,   111,   112,
     113,   740,   156,  1483,   201,   977,  1090,  1479,  1257,  1341,
     207,   487,  1500,   198,  1121,  1240,   213,  1511,  1112,  1353,
      38,  1227,  1116,   913,   466,   374,   877,  1121,   880,   466,
     701,   906,   941,   804,   818,  1112,   851,   150,   151,   275,
     153,   282,  1112,    -1,   241,   242,    -1,    -1,  1125,  1112,
     247,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1128,    -1,
      -1,    -1,   175,    -1,    -1,    -1,  1160,    -1,    -1,    -1,
     267,    -1,  1166,    -1,    -1,  1145,  1170,   274,   275,    -1,
    1174,  1151,  1152,  1170,   281,   198,    -1,    -1,     4,  1166,
     287,    -1,    -1,    -1,    -1,    -1,  1166,  1174,    -1,    -1,
    1355,    -1,  1196,  1166,  1174,  1461,    -1,  1170,  1202,    -1,
      -1,  1174,    -1,    24,    25,  1209,    -1,    28,    -1,    -1,
      -1,   318,    -1,  1479,   321,    -1,    42,    -1,    -1,    -1,
      -1,  1487,    -1,    -1,  1211,    -1,  1361,    -1,  1363,    -1,
      -1,  1235,    -1,    -1,  1238,  1239,  1371,    -1,    -1,  1243,
      -1,    -1,  1239,    -1,    -1,  1249,    -1,    -1,    -1,    -1,
      -1,  1238,  1232,    -1,    -1,   362,  1243,    -1,  1238,    -1,
      -1,    -1,  1249,  1243,    -1,  1238,  1239,    -1,    -1,  1249,
    1243,  1352,    -1,    -1,   100,    -1,  1249,  1412,    -1,   105,
      -1,   107,   108,   109,   110,   111,   112,   113,    -1,    -1,
      -1,    -1,   399,    -1,   452,    -1,    -1,    -1,    -1,    -1,
     407,    -1,  1457,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1492,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1404,   150,   151,    -1,   153,    -1,    -1,
    1508,   489,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     447,    -1,    -1,    -1,  1473,   452,    24,    25,  1352,   175,
      28,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    -1,    -1,    -1,  1370,    -1,    -1,    -1,
    1374,  1506,   198,    -1,    -1,    -1,    -1,  1512,     4,  1383,
     201,    -1,   489,    -1,  1388,  1389,   207,  1374,  1392,  1393,
      -1,    -1,   213,    -1,  1374,    -1,    -1,    60,    61,    -1,
    1404,  1374,    -1,    -1,    -1,    -1,  1410,  1411,    -1,    -1,
      -1,    -1,  1416,    -1,    -1,    -1,    42,    -1,    -1,    -1,
      -1,   528,   529,  1410,  1411,    -1,    -1,    -1,   535,  1416,
    1410,  1411,    70,    71,    -1,    -1,  1416,  1410,  1411,    -1,
      -1,    -1,    80,  1416,   551,  1449,   267,    -1,    -1,    -1,
      -1,    -1,  1456,   274,   275,    -1,    -1,    -1,    -1,   122,
     281,    -1,  1449,    -1,    -1,    -1,   287,    -1,  1472,  1449,
      -1,    -1,    -1,    -1,   100,    -1,  1449,  1457,    -1,   105,
      -1,   107,   108,   109,   110,   111,   112,   113,    -1,   127,
     128,   129,   130,   131,    -1,   602,    -1,    -1,    -1,    -1,
     138,    -1,   660,    -1,    -1,    -1,   144,   145,    -1,  1513,
      -1,    -1,    -1,    -1,  1518,   673,   674,   675,    -1,    -1,
     158,     4,    -1,    -1,   150,   151,  1513,   153,    -1,   207,
     637,  1518,   639,  1513,    -1,   173,    -1,    -1,  1518,    -1,
    1513,   362,    -1,    -1,    -1,  1518,    -1,    -1,    -1,   175,
      -1,    -1,   659,   660,    -1,    -1,    -1,    -1,    -1,    42,
      -1,    -1,    -1,    -1,   671,   672,   673,   674,   675,    -1,
      -1,    -1,   198,    -1,    -1,    -1,    -1,    -1,   399,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   407,    -1,    -1,   267,
      -1,    -1,    -1,     4,    -1,    -1,   274,   275,   705,    -1,
      -1,    -1,    -1,   281,    -1,    -1,    -1,   714,    -1,   287,
      -1,    -1,   770,    -1,    -1,    -1,   723,   100,   725,   726,
      -1,    -1,   105,    -1,   107,   108,   109,   110,   111,   112,
     113,    42,    -1,   740,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   753,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   764,    -1,    -1,
      -1,    -1,    -1,   770,    -1,    -1,   773,   150,   151,    -1,
     153,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    -1,   362,   792,    -1,    -1,    -1,   100,
      -1,    -1,   175,    -1,   105,    -1,   107,   108,   109,   110,
     111,   112,   113,    -1,    -1,   863,    -1,    -1,    -1,   867,
      -1,   869,    -1,    -1,    -1,   198,    72,    -1,    74,    -1,
      -1,   399,    -1,    -1,    -1,    -1,    -1,    -1,   835,   887,
     551,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   150,
     151,   848,   153,   850,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,   863,    -1,    -1,    -1,
     867,    -1,   869,    -1,   175,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   452,    -1,    -1,    -1,    -1,    -1,
     887,   602,    -1,   890,    -1,    -1,    -1,   198,    -1,    -1,
      60,    61,    -1,    -1,   150,   151,    -1,   153,   154,   155,
      -1,    -1,    -1,    -1,    -1,   912,    -1,    -1,   915,    -1,
      -1,   489,    -1,    -1,    -1,   922,   637,    -1,   639,    -1,
     176,   979,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   993,    -1,   995,   659,    -1,
      -1,   197,    -1,   199,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   122,    -1,    -1,    -1,   963,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   976,
     977,    -1,   979,   551,   241,   242,    -1,    -1,    -1,    -1,
     247,    -1,    -1,   990,    -1,  1043,   993,    -1,   995,    -1,
      -1,    -1,    -1,   714,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   725,   726,    -1,    -1,    -1,    -1,
      -1,    -1,  1019,    -1,    -1,  1022,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   602,   195,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1043,    -1,    -1,    -1,
      -1,    -1,    -1,  1101,    -1,    -1,    -1,    -1,  1106,    -1,
    1108,   318,    -1,    -1,   321,    -1,    -1,    -1,    -1,   637,
      -1,   639,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   792,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1138,   659,   660,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1097,    -1,    -1,    -1,  1101,   673,   674,   675,    -1,  1106,
      -1,  1108,    -1,    -1,    -1,    -1,  1113,    -1,    -1,  1116,
    1117,    -1,  1119,    -1,   835,    -1,    -1,    -1,  1176,  1126,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   705,    -1,    -1,
      -1,  1138,    -1,    -1,    -1,    -1,   714,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   723,    -1,   725,   726,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1218,    -1,   740,    -1,    -1,    -1,    -1,    -1,    -1,  1176,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1185,  1186,
     447,    -1,    -1,    -1,    -1,   452,    -1,    -1,  1246,  1247,
      -1,    -1,   770,    -1,  1252,   773,    -1,    -1,  1256,    -1,
      -1,   922,  1209,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1218,    -1,    -1,   792,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   489,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1246,
    1247,    -1,   963,    -1,    72,  1252,  1253,    -1,    -1,    -1,
    1257,    -1,    -1,    -1,    -1,   976,   977,   835,    -1,    -1,
      -1,   528,   529,    -1,    -1,    -1,    -1,    -1,   535,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,   863,    50,    -1,    -1,   867,
      -1,   869,    -1,    -1,    -1,    -1,    -1,    -1,  1356,    -1,
      -1,  1022,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   887,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,
      -1,    -1,   150,   151,    -1,   153,   154,   155,    -1,  1387,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1398,    -1,    -1,    -1,   922,  1403,    -1,    -1,   176,  1356,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,    -1,    -1,    -1,    -1,    -1,  1426,   197,
      -1,    -1,    -1,    -1,    -1,    -1,  1383,    -1,    -1,    -1,
    1387,    -1,    -1,    -1,    -1,   963,    -1,    -1,    -1,    -1,
      -1,  1398,    -1,   660,    -1,    -1,  1403,    -1,   976,   977,
      -1,   979,  1460,    -1,   671,   672,   673,   674,   675,    -1,
      -1,    -1,    -1,    -1,    -1,   993,    26,   995,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,  1019,    -1,    -1,  1022,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1460,    -1,    -1,    -1,    -1,  1516,    -1,
      -1,  1468,    -1,    -1,  1522,  1043,    -1,    -1,  1526,    -1,
    1528,    -1,    -1,    -1,    -1,    -1,  1483,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1492,   753,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   764,    -1,    -1,
      -1,  1508,    -1,   770,    -1,    -1,    -1,    -1,    -1,  1516,
      -1,    -1,    -1,    -1,    -1,  1522,    -1,    43,    44,  1526,
      -1,  1528,    -1,  1101,    -1,    -1,    -1,    -1,  1106,    -1,
    1108,    -1,    -1,    -1,    -1,    -1,    62,    -1,    -1,  1117,
      -1,  1119,   452,    -1,    70,    71,    72,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,
    1138,    -1,    -1,    -1,    -1,    -1,   452,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   489,
      -1,   848,    -1,   850,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   863,    -1,  1176,   125,
     867,    -1,   869,   489,    -1,    -1,    -1,  1185,  1186,    -1,
      -1,    -1,   138,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     887,    -1,    -1,   890,   150,   151,    -1,   153,   154,   155,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1218,    -1,   168,    -1,    -1,   912,    -1,    -1,   915,    -1,
     176,    -1,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,    -1,    -1,    -1,  1246,  1247,
      -1,    -1,    -1,    -1,  1252,  1253,    -1,    -1,    -1,  1257,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     5,     6,    -1,
       8,     9,    10,    -1,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    -1,    -1,    26,    27,
      -1,    -1,   979,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      38,    -1,    -1,   990,    -1,    -1,   993,    45,   995,    47,
      -1,    -1,    50,    -1,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     660,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    79,    -1,   673,   674,   675,    -1,    -1,    -1,    26,
      -1,    -1,    -1,    -1,   660,    93,  1043,    -1,  1356,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   673,   674,    -1,
      -1,    -1,   110,    -1,    -1,    52,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1387,
      -1,    -1,    -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,
    1398,    -1,    -1,    -1,    -1,  1403,    -1,    -1,    -1,    -1,
    1097,    -1,    -1,    -1,  1101,    -1,    -1,    -1,    -1,  1106,
      -1,  1108,    99,    -1,    -1,    -1,  1113,    -1,    -1,  1116,
     107,   108,   109,   110,   111,   112,    -1,    -1,    -1,  1126,
     770,    -1,    -1,   181,    -1,    -1,    -1,    -1,   125,   126,
      -1,  1138,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1460,    -1,   770,    -1,   143,    -1,    -1,   146,
    1468,   148,    -1,   150,   151,    -1,   153,   154,   155,    -1,
      -1,    -1,    -1,    -1,   222,  1483,    -1,   225,    -1,  1176,
      -1,   168,    -1,    -1,   232,   233,    -1,    -1,   175,   176,
      -1,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,    -1,    -1,    -1,   194,  1516,    -1,
      -1,    -1,  1209,    -1,  1522,    -1,    -1,    -1,  1526,    -1,
    1528,  1218,    -1,   863,    -1,    -1,    -1,   867,   276,   869,
      -1,    -1,    -1,    -1,   282,    -1,    -1,    -1,   286,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   887,    -1,  1246,
    1247,    -1,    -1,   869,    -1,  1252,    -1,    -1,    -1,    -1,
      -1,   309,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   887,   320,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   361,    -1,    -1,   364,   365,    -1,   367,
      -1,    -1,    -1,    -1,    -1,    -1,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,    -1,   979,
      -1,    -1,    -1,   391,   392,    -1,   394,   395,   396,    -1,
      -1,    -1,    -1,   993,   402,   995,    -1,    -1,    -1,  1356,
      -1,    -1,    -1,   979,   412,    -1,   414,    -1,    -1,    -1,
      -1,    -1,   420,    -1,    -1,    -1,    -1,   993,    -1,   995,
      -1,    -1,   430,    -1,   432,    -1,  1383,    -1,    -1,    -1,
    1387,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1398,    -1,  1043,    -1,    -1,  1403,    -1,    -1,    -1,
      -1,   459,   460,    -1,   462,   463,   464,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,  1043,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    26,   485,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,  1101,    -1,  1460,    -1,    -1,  1106,    -1,  1108,    -1,
      -1,    -1,   520,    70,    71,    72,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1101,    -1,    -1,    -1,    -1,
    1106,    -1,  1108,    -1,    -1,  1492,    -1,    -1,  1138,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1508,    -1,    -1,    -1,   563,    -1,    -1,    -1,  1516,
      -1,    -1,  1138,    -1,    -1,  1522,    -1,    -1,    -1,  1526,
      -1,  1528,    -1,    -1,    -1,    -1,  1176,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   594,    -1,    -1,    -1,
      -1,    -1,    -1,   150,   151,   603,   153,   154,   155,    -1,
    1176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   619,    -1,    -1,    -1,    -1,    -1,    -1,  1218,   176,
      -1,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,    72,    -1,    74,   645,    -1,    -1,
      -1,    -1,  1218,    -1,    -1,    -1,  1246,  1247,   198,    -1,
     658,    -1,  1252,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
    1246,  1247,    -1,    -1,    -1,    -1,  1252,    -1,    -1,    -1,
      26,   689,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   150,   151,    -1,   153,   154,   155,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   734,    72,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,   747,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,    -1,    -1,    -1,  1356,    -1,    -1,   197,
     768,   199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     778,   116,    -1,   781,    -1,   783,    -1,    -1,    -1,   787,
    1356,    -1,    -1,    -1,    -1,    -1,    -1,  1387,   796,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1398,    -1,
      -1,    -1,    -1,  1403,    -1,   150,   151,    -1,   153,   154,
     155,  1387,    -1,   821,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1398,    -1,    -1,    -1,    -1,  1403,    -1,    -1,
      -1,   176,    -1,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,    -1,    -1,    -1,   194,
      -1,    -1,   198,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1460,    -1,    -1,   871,   872,   873,    -1,    -1,    -1,   877,
     878,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1460,    -1,    -1,    -1,    -1,    -1,
      -1,   899,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
     918,    50,    -1,    -1,    -1,    -1,  1516,    -1,    -1,    -1,
      -1,    -1,  1522,    -1,    -1,    -1,  1526,   935,  1528,    -1,
      -1,    -1,    -1,    -1,    -1,   943,    -1,   945,    -1,    -1,
    1516,    -1,    -1,    -1,    -1,    -1,  1522,    -1,    -1,    -1,
    1526,    -1,  1528,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,   974,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    26,   983,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,  1031,    -1,    -1,    -1,  1035,    -1,  1037,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1046,    -1,
      -1,    -1,    -1,    -1,    -1,  1053,  1054,    -1,    -1,    42,
      43,    44,    -1,    -1,  1062,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    -1,    -1,    70,    71,    72,
      73,    74,    -1,    76,    -1,    -1,    -1,    80,    81,    82,
      83,    -1,    85,    -1,    87,    -1,    89,    -1,    -1,    92,
      -1,    -1,  1110,    96,    97,    98,    99,   100,   101,   102,
      -1,   104,   105,   106,   107,   108,   109,   110,   111,   112,
      -1,   114,   115,   116,   117,   118,   119,    -1,    -1,    -1,
      -1,   124,   125,  1141,   127,   128,   129,   130,   131,    -1,
      -1,    -1,    -1,    -1,    -1,   138,   198,    -1,    -1,    -1,
     143,   144,   145,   146,   147,   148,    -1,   150,   151,    -1,
     153,   154,   155,   156,    -1,   158,    -1,    -1,    -1,   162,
      -1,    -1,    -1,    -1,    -1,   168,   169,    -1,   171,    -1,
     173,   174,   175,   176,    -1,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,    -1,
      -1,   194,    -1,   196,   197,   198,   199,   200,    -1,   202,
     203,    -1,  1220,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
    1248,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     190,   191,    -1,    -1,   194,    -1,   196,   197,   198,   199,
     200,    12,   202,   203,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    26,    -1,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      -1,    -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    -1,    -1,
      -1,    70,    71,    72,    73,    74,    -1,    76,    -1,    -1,
      -1,    80,    81,    82,    83,    -1,    85,    -1,    87,    -1,
      89,    -1,    -1,    92,    -1,    -1,    -1,    96,    97,    98,
      99,   100,   101,   102,    -1,   104,   105,   106,   107,   108,
     109,   110,   111,   112,    -1,   114,   115,   116,   117,   118,
     119,    -1,    -1,    -1,    -1,   124,   125,    -1,   127,   128,
     129,   130,   131,    -1,    -1,    -1,    -1,    -1,    -1,   138,
      -1,    -1,    -1,    -1,   143,   144,   145,   146,   147,   148,
      -1,   150,   151,    -1,   153,   154,   155,   156,    -1,   158,
      -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,   168,
     169,    -1,   171,    -1,   173,   174,   175,   176,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,    -1,    -1,   194,    -1,   196,   197,    -1,
     199,   200,    -1,   202,   203,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    -1,    13,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,    -1,
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
       7,    -1,    -1,    -1,    -1,    -1,    13,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,    -1,
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
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,    -1,
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
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,
      -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    -1,    -1,    70,    71,    72,    73,    74,
      -1,    76,    -1,    -1,    -1,    80,    81,    82,    83,    84,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    -1,    76,    -1,    -1,    -1,    80,    81,    82,    83,
      -1,    85,    -1,    87,    -1,    89,    90,    -1,    92,    -1,
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
     194,    -1,   196,   197,    -1,   199,   200,    -1,   202,   203,
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
      82,    83,    -1,    85,    -1,    87,    -1,    89,    -1,    -1,
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
      -1,    -1,   194,    -1,   196,   197,   198,   199,   200,    -1,
     202,   203,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    -1,    76,    -1,    -1,    -1,    80,
      81,    82,    83,    -1,    85,    -1,    87,    88,    89,    -1,
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
     191,    -1,    -1,   194,    -1,   196,   197,    -1,   199,   200,
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
      -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    -1,    76,    -1,
      -1,    -1,    80,    81,    82,    83,    -1,    85,    86,    87,
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
      -1,   199,   200,    -1,   202,   203,     3,     4,     5,     6,
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
      -1,   196,   197,   198,   199,   200,    -1,   202,   203,     3,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      -1,    -1,    70,    71,    72,    73,    74,    -1,    76,    -1,
      -1,    -1,    80,    81,    82,    83,    -1,    85,    -1,    87,
      -1,    89,    -1,    -1,    92,    -1,    -1,    -1,    96,    97,
      98,    99,    -1,   101,   102,    -1,   104,    -1,   106,   107,
     108,   109,   110,   111,   112,    -1,   114,   115,   116,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,   125,    -1,   127,
     128,   129,   130,   131,    -1,    -1,    -1,    -1,    -1,    -1,
     138,    -1,    -1,    -1,    -1,   143,   144,   145,   146,   147,
     148,    -1,   150,   151,    -1,   153,   154,   155,    -1,    -1,
     158,    -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,
     168,    -1,    -1,    -1,    -1,   173,   174,   175,   176,    -1,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,    -1,    -1,   194,    -1,   196,   197,
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
     107,   108,   109,   110,   111,   112,    -1,    72,   115,   116,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,   125,    -1,
     127,   128,   129,   130,   131,    -1,    -1,    -1,    -1,    -1,
      -1,   138,    -1,    -1,    -1,    -1,   143,   144,   145,   146,
     147,   148,    -1,   150,   151,    -1,   153,   154,   155,    -1,
      -1,   158,    -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,
      -1,   168,    -1,    -1,    -1,    -1,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   150,   151,   194,   153,   154,
     155,    -1,   199,   200,    -1,   202,   203,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    -1,    13,    -1,    -1,
      -1,   176,    -1,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,    -1,    -1,    -1,   194,
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
      -1,    -1,   168,    -1,    -1,    -1,    -1,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,    -1,    -1,   194,    -1,
     196,    -1,    -1,   199,   200,    -1,   202,   203,     3,     4,
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
      -1,    -1,   107,   108,   109,   110,   111,   112,    -1,    72,
     115,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,
     125,    -1,   127,   128,   129,   130,   131,    -1,    -1,    -1,
      -1,    -1,    -1,   138,    -1,    -1,    -1,    -1,   143,   144,
     145,   146,   147,   148,    -1,   150,   151,    -1,   153,   154,
     155,    -1,    -1,   158,    -1,    -1,    -1,   162,    -1,    -1,
      -1,    -1,    -1,   168,    -1,    -1,    -1,    -1,   173,   174,
     175,   176,    -1,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   150,   151,   194,
     153,   154,   155,    -1,   199,   200,    -1,   202,   203,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,   176,    -1,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,    32,    -1,
      -1,   194,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      44,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    -1,    70,    71,    72,    73,
      74,    -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   107,   108,   109,   110,   111,   112,    -1,
      72,   115,   116,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     124,   125,    -1,   127,   128,   129,   130,   131,    -1,    -1,
      -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,    -1,   143,
     144,   145,   146,    -1,   148,    -1,   150,   151,    -1,   153,
     154,   155,    -1,    -1,   158,    -1,   118,    -1,   162,    -1,
      -1,    -1,    -1,    -1,   168,    -1,    -1,    -1,    -1,   173,
      -1,   175,   176,    -1,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   150,   151,
     194,   153,   154,   155,    -1,   199,   200,    -1,   202,   203,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    -1,    -1,   176,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,    -1,
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
      -1,    -1,   194,    -1,   196,    -1,    -1,   199,   200,    -1,
     202,   203,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    -1,    70,
      71,    72,    73,    74,    -1,    -1,    -1,    -1,    -1,    80,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    96,    -1,    -1,    99,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,   110,
     111,   112,    -1,    72,   115,   116,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   124,   125,    -1,   127,   128,   129,   130,
     131,    -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,
      -1,    -1,   143,   144,   145,   146,    -1,   148,    -1,   150,
     151,    -1,   153,   154,   155,    -1,    -1,   158,    -1,    -1,
      -1,   162,    -1,    -1,    -1,    -1,    -1,   168,    -1,    -1,
      -1,    -1,   173,    -1,   175,   176,    -1,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   150,   151,   194,   153,   154,   155,    -1,   199,   200,
      -1,   202,   203,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,   176,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     190,   191,    -1,    -1,   194,   195,    -1,    -1,    -1,   199,
     200,    -1,   202,   203,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,
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
      -1,    -1,    -1,    -1,   199,   200,    -1,   202,   203,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     194,    -1,    -1,   197,    -1,   199,   200,    -1,   202,   203,
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
      32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     191,    -1,    -1,   194,    -1,    -1,    -1,    -1,   199,   200,
      -1,   202,   203,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     189,   190,   191,    -1,    -1,   194,    10,    11,    12,    -1,
     199,   200,    -1,   202,   203,    -1,    -1,    -1,    -1,    -1,
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
      42,    43,    44,    45,    46,    47,    48,    -1,    50,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,   198,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    26,   198,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,   198,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    26,   198,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   198,    -1,    -1,    -1,    -1,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   198,    -1,    10,    11,
      12,     3,     4,     5,     6,     7,    -1,    -1,    10,    11,
      12,    13,    -1,    -1,    26,    -1,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,   198,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    63,    64,    65,    66,    67,    68,    69,    -1,    -1,
      72,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   198,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
      -1,    -1,   124,   125,    -1,   127,   128,   129,   130,   131,
     196,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   143,   144,   145,    -1,   147,    -1,    -1,   150,   151,
      -1,   153,   154,   155,   156,   157,   158,    -1,    -1,   161,
      -1,    -1,    -1,    -1,    -1,    -1,   168,   169,    -1,   171,
     182,   173,   174,   175,   176,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    -1,    50,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    26,   196,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    -1,    50,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,   196,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    26,   196,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,   196,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,   196,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    -1,    10,
      11,    12,    -1,    -1,    72,    -1,    74,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   195,    26,    -1,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    -1,    50,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     118,    -1,    -1,    -1,    -1,   195,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    26,
      50,    -1,   150,   151,    -1,   153,   154,   155,    -1,    -1,
      -1,   190,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,   176,    -1,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,    -1,    -1,    72,    -1,    -1,    26,   197,
      -1,   199,    -1,   186,   187,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    99,    -1,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   183,    -1,    72,    -1,    -1,    -1,   125,   126,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    26,   146,
      -1,   148,    -1,   150,   151,    -1,   153,   154,   155,    -1,
      -1,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   168,    -1,    -1,    52,    -1,    -1,   125,   126,   176,
      -1,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,    72,   143,    -1,   194,   146,    -1,
     148,    -1,   150,   151,    -1,   153,   154,   155,    -1,    -1,
      -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,
     168,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,    -1,    52,    -1,   194,   125,   126,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    72,   143,    -1,    -1,   146,    -1,
     148,    -1,   150,   151,    -1,   153,   154,   155,    -1,    -1,
      -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,    32,    -1,    -1,   194,   125,   126,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,   146,    -1,
     148,    -1,   150,   151,    -1,   153,   154,   155,    -1,    -1,
      -1,    -1,    -1,    72,    -1,    74,    -1,    -1,    -1,    -1,
     168,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,    -1,    -1,    -1,   194,    -1,    -1,    -1,
      -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   143,    -1,    -1,   146,    -1,   148,
      62,   150,   151,    -1,   153,   154,   155,    -1,    70,    71,
      72,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    80,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,    -1,    -1,    -1,   194,    -1,    -1,    -1,    -1,
     199,    63,    64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      72,    -1,    74,   125,    -1,   127,   128,   129,   130,   131,
      -1,    -1,    -1,    -1,    -1,    -1,   138,    -1,    -1,    -1,
      -1,   143,   144,   145,   146,    -1,   148,    -1,   150,   151,
      -1,   153,   154,   155,    -1,    -1,   158,    -1,    -1,    -1,
     112,    -1,    -1,    -1,    -1,    -1,   168,    -1,    -1,    -1,
      -1,   173,    -1,    -1,   176,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,    -1,
      -1,   143,    -1,    -1,   146,    -1,   148,    64,   150,   151,
      -1,   153,   154,   155,    -1,    72,    -1,    74,    -1,    -1,
     162,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   176,    -1,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,    -1,
      -1,    -1,   194,    -1,    -1,   112,    -1,   199,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    72,    -1,    74,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   143,    -1,    -1,   146,
      -1,   148,    -1,   150,   151,    -1,   153,   154,   155,    -1,
      -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    72,    -1,    74,    -1,   176,
      -1,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,    -1,    -1,   143,   194,    -1,   146,
      -1,   148,   199,   150,   151,    -1,   153,   154,   155,    -1,
      -1,    -1,    -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,   176,
      -1,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,    -1,    -1,   143,   194,    -1,   146,
     197,   148,   199,   150,   151,    -1,   153,   154,   155,    72,
      -1,    74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,
      -1,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,    -1,    -1,    -1,   194,    -1,   112,
      -1,    -1,   199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,    -1,    72,    -1,    74,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     143,    -1,    -1,   146,    -1,   148,    -1,   150,   151,    -1,
     153,   154,   155,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   176,    -1,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,    72,    -1,
      74,   194,    -1,    -1,   143,    -1,   199,   146,    -1,   148,
      -1,   150,   151,    -1,   153,   154,   155,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,   112,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,    72,    -1,    74,   194,    -1,    -1,    -1,    -1,
     199,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   143,
      -1,    -1,   146,    -1,   148,    -1,   150,   151,    -1,   153,
     154,   155,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    72,    -1,    74,    -1,    -1,    -1,    -1,
      -1,    -1,   176,    -1,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,    -1,    -1,    -1,
     194,    -1,    -1,    -1,    -1,   199,    -1,    -1,    -1,    -1,
     150,   151,    -1,   153,   154,   155,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   150,   151,    -1,   153,   154,   155,   197,    -1,   199,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    72,    -1,    -1,    -1,    -1,   176,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,    72,    -1,    -1,    -1,    -1,    -1,   197,   148,
     199,   150,   151,   152,   153,   154,   155,    -1,    -1,    -1,
      -1,    -1,    -1,   113,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   125,    -1,   176,    -1,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,    -1,   143,    -1,   194,   146,    -1,   148,    -1,
     150,   151,    -1,   153,   154,   155,    72,    -1,    74,    75,
      -1,    -1,    -1,   143,    -1,    -1,   146,    -1,   148,    -1,
     150,   151,    72,   153,   154,   155,   176,    -1,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,    -1,    -1,    -1,   176,    -1,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    -1,    -1,    -1,    -1,    72,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   150,   151,    -1,   153,   154,   155,
      -1,    -1,    -1,    -1,    -1,    -1,   146,    -1,    -1,    -1,
     150,   151,    -1,   153,   154,   155,    -1,    -1,    -1,    -1,
     176,    -1,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,    -1,   176,    -1,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   148,    -1,   150,   151,    -1,   153,   154,   155,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,
      -1,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    50,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    26,   123,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    -1,
      50,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,   123,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,   123,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,   123,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    12,    -1,    -1,    -1,   123,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    91,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    -1,    50,    10,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50,    11,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,    -1,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    -1,    50
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
      74,   324,    74,   324,    74,   324,   357,   358,   324,   324,
     350,   360,   183,   363,   220,   194,   229,    91,   213,   211,
     324,   280,   383,    74,     9,   195,   195,   195,   195,   195,
     196,   211,   443,   120,   256,   194,     9,   195,   195,    74,
      75,   211,   431,   211,    62,   198,   198,   207,   209,   324,
     121,   255,   163,    47,   148,   163,   370,   123,     9,   387,
     195,   448,   448,    14,   192,     9,   388,   448,   449,   122,
     405,   406,   407,   198,     9,   165,   410,   195,     9,   388,
      14,   328,   239,   120,   254,   194,   438,   324,    27,   201,
     201,   123,   198,     9,   387,   324,   439,   194,   246,   249,
     244,   236,    64,   410,   324,   439,   194,   201,   198,   195,
     201,   198,   195,    43,    44,    62,    70,    71,    80,   125,
     138,   168,   176,   211,   390,   392,   395,   398,   211,   410,
     410,   123,   405,   406,   407,   195,   324,   270,    67,    68,
     271,   220,   315,   220,   317,    32,   124,   260,   410,   383,
     211,    27,   222,   264,   196,   267,   196,   267,     9,   165,
     123,     9,   387,   195,   159,   440,   441,   448,   383,   383,
     383,   386,   389,   194,    79,   142,   194,   194,   142,   197,
     324,   180,   180,    14,   186,   187,   359,     9,   190,   363,
      74,   198,   376,   197,   233,   211,   198,    14,   410,   196,
      91,     9,   165,   257,   376,   197,   422,   124,   410,    14,
     201,   324,   198,   207,   257,   197,   369,    14,   324,   334,
     196,   448,    27,   442,   159,   404,    32,    74,   197,   211,
     415,   448,    32,   324,   383,   275,   194,   376,   255,   329,
     240,   324,   324,   324,   198,   194,   277,   256,   255,   254,
     438,   378,   198,   194,   277,    14,    70,    71,   211,   391,
     391,   392,   393,   394,   194,    79,   139,   194,   194,     9,
     387,   195,   399,    32,   324,   198,    67,    68,   272,   315,
     222,   198,   196,    84,   196,   410,   194,   123,   259,    14,
     220,   267,    93,    94,    95,   267,   198,   448,   448,   444,
       9,   195,   195,   123,   201,     9,   387,   386,   211,   334,
     336,   338,   386,   118,   211,   383,   427,   428,   324,   324,
     324,   358,   324,   348,    74,   234,   383,   448,   211,     9,
     282,   195,   194,   318,   321,   324,   201,   198,   282,   149,
     162,   197,   365,   372,   149,   197,   371,   123,   196,   448,
     333,   449,    74,    14,   324,   439,   194,   410,   195,   275,
     197,   275,   194,   123,   194,   277,   195,   197,   197,   255,
     241,   381,   194,   277,   195,   123,   201,     9,   387,   393,
     139,   334,   396,   397,   393,   392,   410,   315,    27,    69,
     222,   196,   317,   422,   260,   195,   383,    90,    93,   196,
     324,    27,   196,   268,   198,   165,   159,    27,   383,   383,
     195,   123,     9,   387,   195,   195,   123,   198,     9,   387,
     181,   195,   220,    91,   376,     4,   100,   105,   113,   150,
     151,   153,   198,   283,   306,   307,   308,   313,   403,   422,
     198,   198,    47,   324,   324,   324,    32,    74,    14,   383,
     198,   194,   277,   442,   195,   282,   195,   275,   324,   277,
     195,   282,   282,   197,   194,   277,   195,   392,   392,   195,
     123,   195,     9,   387,   195,    27,   220,   196,   195,   195,
     227,   196,   196,   268,   220,   448,   123,   383,   334,   383,
     383,   324,   197,   198,   448,   120,   121,   437,   258,   376,
     113,   125,   148,   154,   292,   293,   294,   376,   152,   298,
     299,   116,   194,   211,   300,   301,   284,   237,   448,     9,
     196,   307,   195,   148,   367,   198,   198,    74,    14,   383,
     194,   277,   195,   105,   326,   442,   198,   442,   195,   195,
     198,   198,   282,   275,   195,   123,   392,   334,   220,   225,
      27,   222,   262,   220,   195,   383,   123,   123,   182,   220,
     376,   376,    14,     9,   196,   197,   197,     9,   196,     3,
       4,     5,     6,     7,    10,    11,    12,    13,    50,    63,
      64,    65,    66,    67,    68,    69,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   124,   125,   127,
     128,   129,   130,   131,   143,   144,   145,   147,   156,   157,
     158,   161,   168,   169,   171,   173,   174,   175,   211,   373,
     374,     9,   196,   148,   152,   211,   301,   302,   303,   196,
      74,   312,   236,   285,   437,   237,    14,   383,   277,   195,
     194,   197,   196,   197,   304,   326,   442,   198,   195,   392,
     123,    27,   222,   261,   220,   383,   383,   324,   198,   196,
     196,   383,   376,   288,   295,   382,   293,    14,    27,    44,
     296,   299,     9,    30,   195,    26,    43,    46,    14,     9,
     196,   438,   312,    14,   236,   383,   195,    32,    74,   364,
     220,   220,   197,   304,   442,   392,   220,    88,   183,   232,
     198,   211,   218,   289,   290,   291,     9,   198,   383,   374,
     374,    52,   297,   302,   302,    26,    43,    46,   383,    74,
     194,   196,   383,   438,    74,     9,   388,   198,   198,   220,
     304,    86,   196,    74,   103,   228,   142,    91,   382,   155,
      14,   286,   194,    32,    74,   195,   198,   196,   194,   161,
     235,   211,   307,   308,   383,   159,   273,   274,   404,   287,
      74,   376,   233,   157,   211,   196,   195,     9,   388,   107,
     108,   109,   310,   311,   273,    74,   258,   196,   442,   159,
     404,   449,   195,   195,   196,   196,   197,   305,   310,    32,
      74,   442,   197,   220,   449,    74,    14,   305,   220,   198,
      32,    74,    14,   383,   198,    74,    14,   383,    14,   383,
     383
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
#line 726 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 729 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 736 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));}
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 737 "hphp.y"
    { }
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 740 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num(), (yyvsp[(1) - (1)]).text()); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 741 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 742 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 743 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 744 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 745 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 748 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();}
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 750 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());}
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 751 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);}
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 752 "hphp.y"
    { _p->onNamespaceStart("");}
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 753 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);}
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 754 "hphp.y"
    { _p->nns(); (yyval).reset();}
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 755 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;}
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 760 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 761 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 762 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 763 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 764 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 765 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 766 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 769 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 771 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 778 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 784 "hphp.y"
    { }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 785 "hphp.y"
    { }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 788 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");}
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 789 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");}
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 790 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());}
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 792 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());}
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 796 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;}
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 801 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;}
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 803 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));}
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 804 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;}
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 807 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 814 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 821 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 829 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 832 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 838 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 839 "hphp.y"
    { _p->onStatementListStart((yyval));}
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 842 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 843 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 845 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 848 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 852 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 857 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));}
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 858 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 860 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 864 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 867 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 871 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 873 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 876 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 878 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 881 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);}
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 882 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));}
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 883 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);}
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 884 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));}
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 885 "hphp.y"
    { _p->onReturn((yyval), NULL);}
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 886 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));}
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 887 "hphp.y"
    { _p->onYieldBreak((yyval));}
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 888 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 889 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 890 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);}
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 891 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));}
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 892 "hphp.y"
    { (yyval).reset(); (yyval) = ';';}
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 893 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);}
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 896 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 898 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 902 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;}
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 909 "hphp.y"
    { _p->onCompleteLabelScope(false);}
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 910 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));}
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 913 "hphp.y"
    { _p->onCompleteLabelScope(false);}
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 914 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));}
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 915 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 916 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval));}
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 920 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 921 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 922 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 923 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 924 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 925 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 926 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); }
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 927 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 928 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 929 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 930 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 938 "hphp.y"
    { _p->onNewLabelScope(false);}
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 939 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 948 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));}
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 949 "hphp.y"
    { (yyval).reset();}
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 953 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 955 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 961 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 962 "hphp.y"
    { (yyval).reset();}
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 966 "hphp.y"
    { (yyval) = 1;}
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 967 "hphp.y"
    { (yyval).reset();}
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 971 "hphp.y"
    { _p->pushFuncLocation(); }
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 976 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();}
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 982 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 988 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();}
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 994 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 1000 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();}
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 1006 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 1014 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));}
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 1017 "hphp.y"
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
#line 1032 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));}
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 1035 "hphp.y"
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
#line 1049 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));}
    break;

  case 127:

/* Line 1806 of yacc.c  */
#line 1052 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 128:

/* Line 1806 of yacc.c  */
#line 1057 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));}
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 1060 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 130:

/* Line 1806 of yacc.c  */
#line 1067 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));}
    break;

  case 131:

/* Line 1806 of yacc.c  */
#line 1070 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0);
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 1078 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));}
    break;

  case 133:

/* Line 1806 of yacc.c  */
#line 1081 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 1089 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 1090 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 136:

/* Line 1806 of yacc.c  */
#line 1094 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 1097 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 1100 "hphp.y"
    { (yyval) = T_CLASS;}
    break;

  case 139:

/* Line 1806 of yacc.c  */
#line 1101 "hphp.y"
    { (yyval) = T_ABSTRACT;}
    break;

  case 140:

/* Line 1806 of yacc.c  */
#line 1102 "hphp.y"
    { (yyval) = T_FINAL;}
    break;

  case 141:

/* Line 1806 of yacc.c  */
#line 1106 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 142:

/* Line 1806 of yacc.c  */
#line 1107 "hphp.y"
    { (yyval).reset();}
    break;

  case 143:

/* Line 1806 of yacc.c  */
#line 1110 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 1111 "hphp.y"
    { (yyval).reset();}
    break;

  case 145:

/* Line 1806 of yacc.c  */
#line 1114 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 146:

/* Line 1806 of yacc.c  */
#line 1115 "hphp.y"
    { (yyval).reset();}
    break;

  case 147:

/* Line 1806 of yacc.c  */
#line 1118 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 148:

/* Line 1806 of yacc.c  */
#line 1120 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 149:

/* Line 1806 of yacc.c  */
#line 1123 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 150:

/* Line 1806 of yacc.c  */
#line 1125 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 151:

/* Line 1806 of yacc.c  */
#line 1129 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 152:

/* Line 1806 of yacc.c  */
#line 1130 "hphp.y"
    { (yyval).reset();}
    break;

  case 153:

/* Line 1806 of yacc.c  */
#line 1133 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 154:

/* Line 1806 of yacc.c  */
#line 1134 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;}
    break;

  case 155:

/* Line 1806 of yacc.c  */
#line 1135 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);}
    break;

  case 156:

/* Line 1806 of yacc.c  */
#line 1139 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 157:

/* Line 1806 of yacc.c  */
#line 1141 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 158:

/* Line 1806 of yacc.c  */
#line 1144 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 159:

/* Line 1806 of yacc.c  */
#line 1146 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 160:

/* Line 1806 of yacc.c  */
#line 1149 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 161:

/* Line 1806 of yacc.c  */
#line 1151 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 162:

/* Line 1806 of yacc.c  */
#line 1154 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 163:

/* Line 1806 of yacc.c  */
#line 1156 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 166:

/* Line 1806 of yacc.c  */
#line 1166 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 167:

/* Line 1806 of yacc.c  */
#line 1167 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 168:

/* Line 1806 of yacc.c  */
#line 1168 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 169:

/* Line 1806 of yacc.c  */
#line 1169 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);}
    break;

  case 170:

/* Line 1806 of yacc.c  */
#line 1174 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 171:

/* Line 1806 of yacc.c  */
#line 1176 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));}
    break;

  case 172:

/* Line 1806 of yacc.c  */
#line 1177 "hphp.y"
    { (yyval).reset();}
    break;

  case 173:

/* Line 1806 of yacc.c  */
#line 1180 "hphp.y"
    { (yyval).reset();}
    break;

  case 174:

/* Line 1806 of yacc.c  */
#line 1181 "hphp.y"
    { (yyval).reset();}
    break;

  case 175:

/* Line 1806 of yacc.c  */
#line 1186 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 176:

/* Line 1806 of yacc.c  */
#line 1187 "hphp.y"
    { (yyval).reset();}
    break;

  case 177:

/* Line 1806 of yacc.c  */
#line 1192 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 178:

/* Line 1806 of yacc.c  */
#line 1193 "hphp.y"
    { (yyval).reset();}
    break;

  case 179:

/* Line 1806 of yacc.c  */
#line 1196 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 180:

/* Line 1806 of yacc.c  */
#line 1197 "hphp.y"
    { (yyval).reset();}
    break;

  case 181:

/* Line 1806 of yacc.c  */
#line 1200 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);}
    break;

  case 182:

/* Line 1806 of yacc.c  */
#line 1201 "hphp.y"
    { (yyval).reset();}
    break;

  case 183:

/* Line 1806 of yacc.c  */
#line 1206 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]);}
    break;

  case 184:

/* Line 1806 of yacc.c  */
#line 1208 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 185:

/* Line 1806 of yacc.c  */
#line 1209 "hphp.y"
    { (yyval).reset();}
    break;

  case 186:

/* Line 1806 of yacc.c  */
#line 1210 "hphp.y"
    { (yyval).reset();}
    break;

  case 187:

/* Line 1806 of yacc.c  */
#line 1216 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));}
    break;

  case 188:

/* Line 1806 of yacc.c  */
#line 1220 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));}
    break;

  case 189:

/* Line 1806 of yacc.c  */
#line 1225 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));}
    break;

  case 190:

/* Line 1806 of yacc.c  */
#line 1230 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));}
    break;

  case 191:

/* Line 1806 of yacc.c  */
#line 1235 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));}
    break;

  case 192:

/* Line 1806 of yacc.c  */
#line 1240 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));}
    break;

  case 193:

/* Line 1806 of yacc.c  */
#line 1246 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));}
    break;

  case 194:

/* Line 1806 of yacc.c  */
#line 1252 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));}
    break;

  case 195:

/* Line 1806 of yacc.c  */
#line 1258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]);}
    break;

  case 196:

/* Line 1806 of yacc.c  */
#line 1260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 197:

/* Line 1806 of yacc.c  */
#line 1261 "hphp.y"
    { (yyval).reset();}
    break;

  case 198:

/* Line 1806 of yacc.c  */
#line 1262 "hphp.y"
    { (yyval).reset();}
    break;

  case 199:

/* Line 1806 of yacc.c  */
#line 1267 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),0,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL);}
    break;

  case 200:

/* Line 1806 of yacc.c  */
#line 1270 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),1,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL);}
    break;

  case 201:

/* Line 1806 of yacc.c  */
#line 1274 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),1,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL);}
    break;

  case 202:

/* Line 1806 of yacc.c  */
#line 1278 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),0,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL);}
    break;

  case 203:

/* Line 1806 of yacc.c  */
#line 1282 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),0,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL);}
    break;

  case 204:

/* Line 1806 of yacc.c  */
#line 1286 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),1,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL);}
    break;

  case 205:

/* Line 1806 of yacc.c  */
#line 1291 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),1,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL);}
    break;

  case 206:

/* Line 1806 of yacc.c  */
#line 1296 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),0,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL);}
    break;

  case 207:

/* Line 1806 of yacc.c  */
#line 1302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 208:

/* Line 1806 of yacc.c  */
#line 1303 "hphp.y"
    { (yyval).reset();}
    break;

  case 209:

/* Line 1806 of yacc.c  */
#line 1306 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),0);}
    break;

  case 210:

/* Line 1806 of yacc.c  */
#line 1307 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),1);}
    break;

  case 211:

/* Line 1806 of yacc.c  */
#line 1309 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 212:

/* Line 1806 of yacc.c  */
#line 1311 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 213:

/* Line 1806 of yacc.c  */
#line 1315 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 214:

/* Line 1806 of yacc.c  */
#line 1316 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 215:

/* Line 1806 of yacc.c  */
#line 1319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 216:

/* Line 1806 of yacc.c  */
#line 1320 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;}
    break;

  case 217:

/* Line 1806 of yacc.c  */
#line 1321 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;}
    break;

  case 218:

/* Line 1806 of yacc.c  */
#line 1325 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 219:

/* Line 1806 of yacc.c  */
#line 1327 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));}
    break;

  case 220:

/* Line 1806 of yacc.c  */
#line 1328 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);}
    break;

  case 221:

/* Line 1806 of yacc.c  */
#line 1329 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 222:

/* Line 1806 of yacc.c  */
#line 1334 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 223:

/* Line 1806 of yacc.c  */
#line 1335 "hphp.y"
    { (yyval).reset();}
    break;

  case 224:

/* Line 1806 of yacc.c  */
#line 1338 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));}
    break;

  case 225:

/* Line 1806 of yacc.c  */
#line 1339 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 226:

/* Line 1806 of yacc.c  */
#line 1342 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));}
    break;

  case 227:

/* Line 1806 of yacc.c  */
#line 1343 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));}
    break;

  case 228:

/* Line 1806 of yacc.c  */
#line 1345 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);}
    break;

  case 229:

/* Line 1806 of yacc.c  */
#line 1349 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();}
    break;

  case 230:

/* Line 1806 of yacc.c  */
#line 1355 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 231:

/* Line 1806 of yacc.c  */
#line 1362 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();}
    break;

  case 232:

/* Line 1806 of yacc.c  */
#line 1368 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 233:

/* Line 1806 of yacc.c  */
#line 1373 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));}
    break;

  case 234:

/* Line 1806 of yacc.c  */
#line 1375 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));}
    break;

  case 235:

/* Line 1806 of yacc.c  */
#line 1377 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));}
    break;

  case 236:

/* Line 1806 of yacc.c  */
#line 1379 "hphp.y"
    { _p->onTraitRequire((yyval), (yyvsp[(3) - (4)]), true); }
    break;

  case 237:

/* Line 1806 of yacc.c  */
#line 1381 "hphp.y"
    { _p->onTraitRequire((yyval), (yyvsp[(3) - (4)]), false); }
    break;

  case 238:

/* Line 1806 of yacc.c  */
#line 1382 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); }
    break;

  case 239:

/* Line 1806 of yacc.c  */
#line 1385 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); }
    break;

  case 240:

/* Line 1806 of yacc.c  */
#line 1388 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); }
    break;

  case 241:

/* Line 1806 of yacc.c  */
#line 1389 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); }
    break;

  case 242:

/* Line 1806 of yacc.c  */
#line 1390 "hphp.y"
    { (yyval).reset(); }
    break;

  case 243:

/* Line 1806 of yacc.c  */
#line 1396 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 244:

/* Line 1806 of yacc.c  */
#line 1400 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));}
    break;

  case 245:

/* Line 1806 of yacc.c  */
#line 1403 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);}
    break;

  case 246:

/* Line 1806 of yacc.c  */
#line 1410 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 247:

/* Line 1806 of yacc.c  */
#line 1411 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));}
    break;

  case 248:

/* Line 1806 of yacc.c  */
#line 1416 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));}
    break;

  case 249:

/* Line 1806 of yacc.c  */
#line 1419 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 250:

/* Line 1806 of yacc.c  */
#line 1426 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;}
    break;

  case 251:

/* Line 1806 of yacc.c  */
#line 1428 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;}
    break;

  case 252:

/* Line 1806 of yacc.c  */
#line 1432 "hphp.y"
    { (yyval) = 4;}
    break;

  case 253:

/* Line 1806 of yacc.c  */
#line 1433 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));}
    break;

  case 254:

/* Line 1806 of yacc.c  */
#line 1439 "hphp.y"
    { (yyval) = 6;}
    break;

  case 255:

/* Line 1806 of yacc.c  */
#line 1441 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;}
    break;

  case 256:

/* Line 1806 of yacc.c  */
#line 1445 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);}
    break;

  case 257:

/* Line 1806 of yacc.c  */
#line 1447 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);}
    break;

  case 258:

/* Line 1806 of yacc.c  */
#line 1451 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 259:

/* Line 1806 of yacc.c  */
#line 1452 "hphp.y"
    { scalar_null(_p, (yyval));}
    break;

  case 260:

/* Line 1806 of yacc.c  */
#line 1456 "hphp.y"
    { scalar_num(_p, (yyval), "1");}
    break;

  case 261:

/* Line 1806 of yacc.c  */
#line 1457 "hphp.y"
    { scalar_num(_p, (yyval), "0");}
    break;

  case 262:

/* Line 1806 of yacc.c  */
#line 1461 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);}
    break;

  case 263:

/* Line 1806 of yacc.c  */
#line 1464 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);}
    break;

  case 264:

/* Line 1806 of yacc.c  */
#line 1469 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 265:

/* Line 1806 of yacc.c  */
#line 1474 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;}
    break;

  case 266:

/* Line 1806 of yacc.c  */
#line 1475 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;}
    break;

  case 267:

/* Line 1806 of yacc.c  */
#line 1477 "hphp.y"
    { (yyval) = 0;}
    break;

  case 268:

/* Line 1806 of yacc.c  */
#line 1481 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);}
    break;

  case 269:

/* Line 1806 of yacc.c  */
#line 1482 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);}
    break;

  case 270:

/* Line 1806 of yacc.c  */
#line 1483 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);}
    break;

  case 271:

/* Line 1806 of yacc.c  */
#line 1484 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);}
    break;

  case 272:

/* Line 1806 of yacc.c  */
#line 1488 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 273:

/* Line 1806 of yacc.c  */
#line 1489 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);}
    break;

  case 274:

/* Line 1806 of yacc.c  */
#line 1490 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);}
    break;

  case 275:

/* Line 1806 of yacc.c  */
#line 1491 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);}
    break;

  case 276:

/* Line 1806 of yacc.c  */
#line 1492 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);}
    break;

  case 277:

/* Line 1806 of yacc.c  */
#line 1494 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));}
    break;

  case 278:

/* Line 1806 of yacc.c  */
#line 1496 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));}
    break;

  case 279:

/* Line 1806 of yacc.c  */
#line 1500 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;}
    break;

  case 280:

/* Line 1806 of yacc.c  */
#line 1503 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;}
    break;

  case 281:

/* Line 1806 of yacc.c  */
#line 1504 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;}
    break;

  case 282:

/* Line 1806 of yacc.c  */
#line 1508 "hphp.y"
    { (yyval).reset();}
    break;

  case 283:

/* Line 1806 of yacc.c  */
#line 1509 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;}
    break;

  case 284:

/* Line 1806 of yacc.c  */
#line 1513 "hphp.y"
    { (yyval).reset();}
    break;

  case 285:

/* Line 1806 of yacc.c  */
#line 1514 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;}
    break;

  case 286:

/* Line 1806 of yacc.c  */
#line 1517 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 287:

/* Line 1806 of yacc.c  */
#line 1518 "hphp.y"
    { (yyval).reset();}
    break;

  case 288:

/* Line 1806 of yacc.c  */
#line 1521 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 289:

/* Line 1806 of yacc.c  */
#line 1522 "hphp.y"
    { (yyval).reset();}
    break;

  case 290:

/* Line 1806 of yacc.c  */
#line 1525 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));}
    break;

  case 291:

/* Line 1806 of yacc.c  */
#line 1527 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 292:

/* Line 1806 of yacc.c  */
#line 1530 "hphp.y"
    { (yyval) = T_PUBLIC;}
    break;

  case 293:

/* Line 1806 of yacc.c  */
#line 1531 "hphp.y"
    { (yyval) = T_PROTECTED;}
    break;

  case 294:

/* Line 1806 of yacc.c  */
#line 1532 "hphp.y"
    { (yyval) = T_PRIVATE;}
    break;

  case 295:

/* Line 1806 of yacc.c  */
#line 1533 "hphp.y"
    { (yyval) = T_STATIC;}
    break;

  case 296:

/* Line 1806 of yacc.c  */
#line 1534 "hphp.y"
    { (yyval) = T_ABSTRACT;}
    break;

  case 297:

/* Line 1806 of yacc.c  */
#line 1535 "hphp.y"
    { (yyval) = T_FINAL;}
    break;

  case 298:

/* Line 1806 of yacc.c  */
#line 1536 "hphp.y"
    { (yyval) = T_ASYNC;}
    break;

  case 299:

/* Line 1806 of yacc.c  */
#line 1540 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 300:

/* Line 1806 of yacc.c  */
#line 1541 "hphp.y"
    { (yyval).reset();}
    break;

  case 301:

/* Line 1806 of yacc.c  */
#line 1544 "hphp.y"
    { (yyval) = T_PUBLIC;}
    break;

  case 302:

/* Line 1806 of yacc.c  */
#line 1545 "hphp.y"
    { (yyval) = T_PROTECTED;}
    break;

  case 303:

/* Line 1806 of yacc.c  */
#line 1546 "hphp.y"
    { (yyval) = T_PRIVATE;}
    break;

  case 304:

/* Line 1806 of yacc.c  */
#line 1550 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 305:

/* Line 1806 of yacc.c  */
#line 1552 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));}
    break;

  case 306:

/* Line 1806 of yacc.c  */
#line 1553 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);}
    break;

  case 307:

/* Line 1806 of yacc.c  */
#line 1554 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 308:

/* Line 1806 of yacc.c  */
#line 1558 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 309:

/* Line 1806 of yacc.c  */
#line 1559 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 310:

/* Line 1806 of yacc.c  */
#line 1563 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 311:

/* Line 1806 of yacc.c  */
#line 1565 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 312:

/* Line 1806 of yacc.c  */
#line 1566 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);}
    break;

  case 313:

/* Line 1806 of yacc.c  */
#line 1567 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 314:

/* Line 1806 of yacc.c  */
#line 1568 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 315:

/* Line 1806 of yacc.c  */
#line 1571 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 316:

/* Line 1806 of yacc.c  */
#line 1575 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 317:

/* Line 1806 of yacc.c  */
#line 1576 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 318:

/* Line 1806 of yacc.c  */
#line 1580 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 319:

/* Line 1806 of yacc.c  */
#line 1581 "hphp.y"
    { (yyval).reset();}
    break;

  case 320:

/* Line 1806 of yacc.c  */
#line 1585 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));}
    break;

  case 321:

/* Line 1806 of yacc.c  */
#line 1586 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 322:

/* Line 1806 of yacc.c  */
#line 1590 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 323:

/* Line 1806 of yacc.c  */
#line 1595 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);}
    break;

  case 324:

/* Line 1806 of yacc.c  */
#line 1599 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 325:

/* Line 1806 of yacc.c  */
#line 1603 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 326:

/* Line 1806 of yacc.c  */
#line 1608 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);}
    break;

  case 327:

/* Line 1806 of yacc.c  */
#line 1612 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 328:

/* Line 1806 of yacc.c  */
#line 1613 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 329:

/* Line 1806 of yacc.c  */
#line 1614 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 330:

/* Line 1806 of yacc.c  */
#line 1618 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));}
    break;

  case 331:

/* Line 1806 of yacc.c  */
#line 1619 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 332:

/* Line 1806 of yacc.c  */
#line 1620 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);}
    break;

  case 333:

/* Line 1806 of yacc.c  */
#line 1623 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));}
    break;

  case 334:

/* Line 1806 of yacc.c  */
#line 1624 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);}
    break;

  case 335:

/* Line 1806 of yacc.c  */
#line 1625 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);}
    break;

  case 336:

/* Line 1806 of yacc.c  */
#line 1626 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);}
    break;

  case 337:

/* Line 1806 of yacc.c  */
#line 1627 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);}
    break;

  case 338:

/* Line 1806 of yacc.c  */
#line 1628 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);}
    break;

  case 339:

/* Line 1806 of yacc.c  */
#line 1629 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);}
    break;

  case 340:

/* Line 1806 of yacc.c  */
#line 1630 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);}
    break;

  case 341:

/* Line 1806 of yacc.c  */
#line 1631 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);}
    break;

  case 342:

/* Line 1806 of yacc.c  */
#line 1632 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);}
    break;

  case 343:

/* Line 1806 of yacc.c  */
#line 1633 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);}
    break;

  case 344:

/* Line 1806 of yacc.c  */
#line 1634 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);}
    break;

  case 345:

/* Line 1806 of yacc.c  */
#line 1635 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);}
    break;

  case 346:

/* Line 1806 of yacc.c  */
#line 1636 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);}
    break;

  case 347:

/* Line 1806 of yacc.c  */
#line 1637 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);}
    break;

  case 348:

/* Line 1806 of yacc.c  */
#line 1638 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);}
    break;

  case 349:

/* Line 1806 of yacc.c  */
#line 1639 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);}
    break;

  case 350:

/* Line 1806 of yacc.c  */
#line 1640 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);}
    break;

  case 351:

/* Line 1806 of yacc.c  */
#line 1641 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);}
    break;

  case 352:

/* Line 1806 of yacc.c  */
#line 1642 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);}
    break;

  case 353:

/* Line 1806 of yacc.c  */
#line 1643 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);}
    break;

  case 354:

/* Line 1806 of yacc.c  */
#line 1644 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');}
    break;

  case 355:

/* Line 1806 of yacc.c  */
#line 1645 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');}
    break;

  case 356:

/* Line 1806 of yacc.c  */
#line 1646 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');}
    break;

  case 357:

/* Line 1806 of yacc.c  */
#line 1647 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');}
    break;

  case 358:

/* Line 1806 of yacc.c  */
#line 1648 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');}
    break;

  case 359:

/* Line 1806 of yacc.c  */
#line 1649 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');}
    break;

  case 360:

/* Line 1806 of yacc.c  */
#line 1650 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');}
    break;

  case 361:

/* Line 1806 of yacc.c  */
#line 1651 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');}
    break;

  case 362:

/* Line 1806 of yacc.c  */
#line 1652 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');}
    break;

  case 363:

/* Line 1806 of yacc.c  */
#line 1653 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);}
    break;

  case 364:

/* Line 1806 of yacc.c  */
#line 1654 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);}
    break;

  case 365:

/* Line 1806 of yacc.c  */
#line 1655 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 366:

/* Line 1806 of yacc.c  */
#line 1656 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 367:

/* Line 1806 of yacc.c  */
#line 1657 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);}
    break;

  case 368:

/* Line 1806 of yacc.c  */
#line 1658 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);}
    break;

  case 369:

/* Line 1806 of yacc.c  */
#line 1659 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);}
    break;

  case 370:

/* Line 1806 of yacc.c  */
#line 1660 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);}
    break;

  case 371:

/* Line 1806 of yacc.c  */
#line 1661 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);}
    break;

  case 372:

/* Line 1806 of yacc.c  */
#line 1662 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);}
    break;

  case 373:

/* Line 1806 of yacc.c  */
#line 1663 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');}
    break;

  case 374:

/* Line 1806 of yacc.c  */
#line 1664 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);}
    break;

  case 375:

/* Line 1806 of yacc.c  */
#line 1666 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');}
    break;

  case 376:

/* Line 1806 of yacc.c  */
#line 1667 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);}
    break;

  case 377:

/* Line 1806 of yacc.c  */
#line 1670 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);}
    break;

  case 378:

/* Line 1806 of yacc.c  */
#line 1671 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 379:

/* Line 1806 of yacc.c  */
#line 1672 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));}
    break;

  case 380:

/* Line 1806 of yacc.c  */
#line 1673 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));}
    break;

  case 381:

/* Line 1806 of yacc.c  */
#line 1674 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 382:

/* Line 1806 of yacc.c  */
#line 1675 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);}
    break;

  case 383:

/* Line 1806 of yacc.c  */
#line 1676 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);}
    break;

  case 384:

/* Line 1806 of yacc.c  */
#line 1677 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);}
    break;

  case 385:

/* Line 1806 of yacc.c  */
#line 1678 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);}
    break;

  case 386:

/* Line 1806 of yacc.c  */
#line 1679 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);}
    break;

  case 387:

/* Line 1806 of yacc.c  */
#line 1680 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);}
    break;

  case 388:

/* Line 1806 of yacc.c  */
#line 1681 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);}
    break;

  case 389:

/* Line 1806 of yacc.c  */
#line 1682 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);}
    break;

  case 390:

/* Line 1806 of yacc.c  */
#line 1683 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);}
    break;

  case 391:

/* Line 1806 of yacc.c  */
#line 1684 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 392:

/* Line 1806 of yacc.c  */
#line 1685 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 393:

/* Line 1806 of yacc.c  */
#line 1686 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 394:

/* Line 1806 of yacc.c  */
#line 1687 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));}
    break;

  case 395:

/* Line 1806 of yacc.c  */
#line 1688 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);}
    break;

  case 396:

/* Line 1806 of yacc.c  */
#line 1689 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 397:

/* Line 1806 of yacc.c  */
#line 1690 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 398:

/* Line 1806 of yacc.c  */
#line 1691 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 399:

/* Line 1806 of yacc.c  */
#line 1698 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);}
    break;

  case 400:

/* Line 1806 of yacc.c  */
#line 1699 "hphp.y"
    { (yyval).reset();}
    break;

  case 401:

/* Line 1806 of yacc.c  */
#line 1704 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
    break;

  case 402:

/* Line 1806 of yacc.c  */
#line 1710 "hphp.y"
    { _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            (yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 403:

/* Line 1806 of yacc.c  */
#line 1718 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
    break;

  case 404:

/* Line 1806 of yacc.c  */
#line 1724 "hphp.y"
    { _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         (yyval) = _p->onClosure(ClosureType::Long,
                                                            &(yyvsp[(1) - (12)]),
                                                            (yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 405:

/* Line 1806 of yacc.c  */
#line 1733 "hphp.y"
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
#line 1741 "hphp.y"
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
#line 1748 "hphp.y"
    { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
    break;

  case 408:

/* Line 1806 of yacc.c  */
#line 1756 "hphp.y"
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
#line 1766 "hphp.y"
    { (yyval) = _p->onExprForLambda((yyvsp[(2) - (2)]));}
    break;

  case 410:

/* Line 1806 of yacc.c  */
#line 1768 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 411:

/* Line 1806 of yacc.c  */
#line 1772 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); }
    break;

  case 412:

/* Line 1806 of yacc.c  */
#line 1780 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 413:

/* Line 1806 of yacc.c  */
#line 1783 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 414:

/* Line 1806 of yacc.c  */
#line 1790 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 415:

/* Line 1806 of yacc.c  */
#line 1793 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 416:

/* Line 1806 of yacc.c  */
#line 1798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 417:

/* Line 1806 of yacc.c  */
#line 1799 "hphp.y"
    { (yyval).reset(); }
    break;

  case 418:

/* Line 1806 of yacc.c  */
#line 1804 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 419:

/* Line 1806 of yacc.c  */
#line 1805 "hphp.y"
    { (yyval).reset(); }
    break;

  case 420:

/* Line 1806 of yacc.c  */
#line 1809 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);}
    break;

  case 421:

/* Line 1806 of yacc.c  */
#line 1813 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 422:

/* Line 1806 of yacc.c  */
#line 1814 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 423:

/* Line 1806 of yacc.c  */
#line 1819 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);}
    break;

  case 424:

/* Line 1806 of yacc.c  */
#line 1826 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);}
    break;

  case 425:

/* Line 1806 of yacc.c  */
#line 1833 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 426:

/* Line 1806 of yacc.c  */
#line 1835 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 427:

/* Line 1806 of yacc.c  */
#line 1839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 428:

/* Line 1806 of yacc.c  */
#line 1840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 429:

/* Line 1806 of yacc.c  */
#line 1841 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 430:

/* Line 1806 of yacc.c  */
#line 1845 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); }
    break;

  case 431:

/* Line 1806 of yacc.c  */
#line 1849 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 432:

/* Line 1806 of yacc.c  */
#line 1853 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 433:

/* Line 1806 of yacc.c  */
#line 1858 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); }
    break;

  case 434:

/* Line 1806 of yacc.c  */
#line 1860 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); }
    break;

  case 435:

/* Line 1806 of yacc.c  */
#line 1862 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); }
    break;

  case 436:

/* Line 1806 of yacc.c  */
#line 1864 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); }
    break;

  case 437:

/* Line 1806 of yacc.c  */
#line 1868 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); }
    break;

  case 438:

/* Line 1806 of yacc.c  */
#line 1870 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); }
    break;

  case 439:

/* Line 1806 of yacc.c  */
#line 1874 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 440:

/* Line 1806 of yacc.c  */
#line 1875 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 441:

/* Line 1806 of yacc.c  */
#line 1876 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 442:

/* Line 1806 of yacc.c  */
#line 1877 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 443:

/* Line 1806 of yacc.c  */
#line 1878 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 444:

/* Line 1806 of yacc.c  */
#line 1879 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 445:

/* Line 1806 of yacc.c  */
#line 1883 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 446:

/* Line 1806 of yacc.c  */
#line 1887 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 447:

/* Line 1806 of yacc.c  */
#line 1891 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 448:

/* Line 1806 of yacc.c  */
#line 1896 "hphp.y"
    { _p->onJoinClause((yyval), NULL, (yyvsp[(2) - (2)]), NULL, NULL); }
    break;

  case 449:

/* Line 1806 of yacc.c  */
#line 1898 "hphp.y"
    { _p->onJoinClause((yyval), &(yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]), NULL, NULL); }
    break;

  case 450:

/* Line 1806 of yacc.c  */
#line 1900 "hphp.y"
    { _p->onJoinClause((yyval), &(yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), &(yyvsp[(6) - (8)]), &(yyvsp[(8) - (8)])); }
    break;

  case 451:

/* Line 1806 of yacc.c  */
#line 1905 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); }
    break;

  case 452:

/* Line 1806 of yacc.c  */
#line 1909 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 453:

/* Line 1806 of yacc.c  */
#line 1913 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); }
    break;

  case 454:

/* Line 1806 of yacc.c  */
#line 1914 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); }
    break;

  case 455:

/* Line 1806 of yacc.c  */
#line 1918 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); }
    break;

  case 456:

/* Line 1806 of yacc.c  */
#line 1919 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); }
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
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 460:

/* Line 1806 of yacc.c  */
#line 1929 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 461:

/* Line 1806 of yacc.c  */
#line 1933 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 462:

/* Line 1806 of yacc.c  */
#line 1937 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 463:

/* Line 1806 of yacc.c  */
#line 1941 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); }
    break;

  case 464:

/* Line 1806 of yacc.c  */
#line 1945 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 465:

/* Line 1806 of yacc.c  */
#line 1946 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 466:

/* Line 1806 of yacc.c  */
#line 1947 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 467:

/* Line 1806 of yacc.c  */
#line 1948 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);}
    break;

  case 468:

/* Line 1806 of yacc.c  */
#line 1955 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));}
    break;

  case 469:

/* Line 1806 of yacc.c  */
#line 1958 "hphp.y"
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
#line 1969 "hphp.y"
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
#line 1980 "hphp.y"
    { (yyval).reset(); (yyval).setText("");}
    break;

  case 472:

/* Line 1806 of yacc.c  */
#line 1981 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));}
    break;

  case 473:

/* Line 1806 of yacc.c  */
#line 1986 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);}
    break;

  case 474:

/* Line 1806 of yacc.c  */
#line 1987 "hphp.y"
    { (yyval).reset();}
    break;

  case 475:

/* Line 1806 of yacc.c  */
#line 1990 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);}
    break;

  case 476:

/* Line 1806 of yacc.c  */
#line 1991 "hphp.y"
    { (yyval).reset();}
    break;

  case 477:

/* Line 1806 of yacc.c  */
#line 1994 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 478:

/* Line 1806 of yacc.c  */
#line 1998 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 479:

/* Line 1806 of yacc.c  */
#line 2001 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 480:

/* Line 1806 of yacc.c  */
#line 2004 "hphp.y"
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
#line 2011 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 482:

/* Line 1806 of yacc.c  */
#line 2012 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 483:

/* Line 1806 of yacc.c  */
#line 2016 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 484:

/* Line 1806 of yacc.c  */
#line 2018 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);}
    break;

  case 485:

/* Line 1806 of yacc.c  */
#line 2020 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);}
    break;

  case 486:

/* Line 1806 of yacc.c  */
#line 2023 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 487:

/* Line 1806 of yacc.c  */
#line 2024 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 488:

/* Line 1806 of yacc.c  */
#line 2025 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 489:

/* Line 1806 of yacc.c  */
#line 2026 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 490:

/* Line 1806 of yacc.c  */
#line 2027 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 491:

/* Line 1806 of yacc.c  */
#line 2028 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 492:

/* Line 1806 of yacc.c  */
#line 2029 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 493:

/* Line 1806 of yacc.c  */
#line 2030 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 494:

/* Line 1806 of yacc.c  */
#line 2031 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 495:

/* Line 1806 of yacc.c  */
#line 2032 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 496:

/* Line 1806 of yacc.c  */
#line 2033 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 497:

/* Line 1806 of yacc.c  */
#line 2034 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 498:

/* Line 1806 of yacc.c  */
#line 2035 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 499:

/* Line 1806 of yacc.c  */
#line 2036 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 500:

/* Line 1806 of yacc.c  */
#line 2037 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 501:

/* Line 1806 of yacc.c  */
#line 2038 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 502:

/* Line 1806 of yacc.c  */
#line 2039 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 503:

/* Line 1806 of yacc.c  */
#line 2040 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 504:

/* Line 1806 of yacc.c  */
#line 2041 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 505:

/* Line 1806 of yacc.c  */
#line 2042 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 506:

/* Line 1806 of yacc.c  */
#line 2043 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 507:

/* Line 1806 of yacc.c  */
#line 2044 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 508:

/* Line 1806 of yacc.c  */
#line 2045 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 509:

/* Line 1806 of yacc.c  */
#line 2046 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 510:

/* Line 1806 of yacc.c  */
#line 2047 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 511:

/* Line 1806 of yacc.c  */
#line 2048 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 512:

/* Line 1806 of yacc.c  */
#line 2049 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 513:

/* Line 1806 of yacc.c  */
#line 2050 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 514:

/* Line 1806 of yacc.c  */
#line 2051 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 515:

/* Line 1806 of yacc.c  */
#line 2052 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 516:

/* Line 1806 of yacc.c  */
#line 2053 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 517:

/* Line 1806 of yacc.c  */
#line 2054 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 518:

/* Line 1806 of yacc.c  */
#line 2055 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 519:

/* Line 1806 of yacc.c  */
#line 2056 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 520:

/* Line 1806 of yacc.c  */
#line 2057 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 521:

/* Line 1806 of yacc.c  */
#line 2058 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 522:

/* Line 1806 of yacc.c  */
#line 2059 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 523:

/* Line 1806 of yacc.c  */
#line 2060 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 524:

/* Line 1806 of yacc.c  */
#line 2061 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 525:

/* Line 1806 of yacc.c  */
#line 2062 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 526:

/* Line 1806 of yacc.c  */
#line 2063 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 527:

/* Line 1806 of yacc.c  */
#line 2064 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 528:

/* Line 1806 of yacc.c  */
#line 2065 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 529:

/* Line 1806 of yacc.c  */
#line 2066 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 530:

/* Line 1806 of yacc.c  */
#line 2067 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 531:

/* Line 1806 of yacc.c  */
#line 2068 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 532:

/* Line 1806 of yacc.c  */
#line 2069 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 533:

/* Line 1806 of yacc.c  */
#line 2070 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 534:

/* Line 1806 of yacc.c  */
#line 2071 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 535:

/* Line 1806 of yacc.c  */
#line 2072 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 536:

/* Line 1806 of yacc.c  */
#line 2073 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 537:

/* Line 1806 of yacc.c  */
#line 2074 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 538:

/* Line 1806 of yacc.c  */
#line 2075 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 539:

/* Line 1806 of yacc.c  */
#line 2076 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 540:

/* Line 1806 of yacc.c  */
#line 2077 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 541:

/* Line 1806 of yacc.c  */
#line 2078 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 542:

/* Line 1806 of yacc.c  */
#line 2079 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 543:

/* Line 1806 of yacc.c  */
#line 2080 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 544:

/* Line 1806 of yacc.c  */
#line 2081 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 545:

/* Line 1806 of yacc.c  */
#line 2082 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 546:

/* Line 1806 of yacc.c  */
#line 2083 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 547:

/* Line 1806 of yacc.c  */
#line 2084 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 548:

/* Line 1806 of yacc.c  */
#line 2085 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 549:

/* Line 1806 of yacc.c  */
#line 2086 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 550:

/* Line 1806 of yacc.c  */
#line 2087 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 551:

/* Line 1806 of yacc.c  */
#line 2088 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 552:

/* Line 1806 of yacc.c  */
#line 2089 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 553:

/* Line 1806 of yacc.c  */
#line 2090 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 554:

/* Line 1806 of yacc.c  */
#line 2091 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 555:

/* Line 1806 of yacc.c  */
#line 2092 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 556:

/* Line 1806 of yacc.c  */
#line 2093 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 557:

/* Line 1806 of yacc.c  */
#line 2094 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 558:

/* Line 1806 of yacc.c  */
#line 2095 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 559:

/* Line 1806 of yacc.c  */
#line 2096 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 560:

/* Line 1806 of yacc.c  */
#line 2097 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 561:

/* Line 1806 of yacc.c  */
#line 2098 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 562:

/* Line 1806 of yacc.c  */
#line 2099 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 563:

/* Line 1806 of yacc.c  */
#line 2100 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 564:

/* Line 1806 of yacc.c  */
#line 2101 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 565:

/* Line 1806 of yacc.c  */
#line 2102 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 566:

/* Line 1806 of yacc.c  */
#line 2107 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 567:

/* Line 1806 of yacc.c  */
#line 2111 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 568:

/* Line 1806 of yacc.c  */
#line 2112 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 569:

/* Line 1806 of yacc.c  */
#line 2115 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);}
    break;

  case 570:

/* Line 1806 of yacc.c  */
#line 2116 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);}
    break;

  case 571:

/* Line 1806 of yacc.c  */
#line 2117 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);}
    break;

  case 572:

/* Line 1806 of yacc.c  */
#line 2121 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);}
    break;

  case 573:

/* Line 1806 of yacc.c  */
#line 2122 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);}
    break;

  case 574:

/* Line 1806 of yacc.c  */
#line 2123 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);}
    break;

  case 575:

/* Line 1806 of yacc.c  */
#line 2127 "hphp.y"
    { (yyval).reset();}
    break;

  case 576:

/* Line 1806 of yacc.c  */
#line 2128 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 577:

/* Line 1806 of yacc.c  */
#line 2129 "hphp.y"
    { (yyval).reset();}
    break;

  case 578:

/* Line 1806 of yacc.c  */
#line 2133 "hphp.y"
    { (yyval).reset();}
    break;

  case 579:

/* Line 1806 of yacc.c  */
#line 2134 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);}
    break;

  case 580:

/* Line 1806 of yacc.c  */
#line 2135 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 581:

/* Line 1806 of yacc.c  */
#line 2139 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 582:

/* Line 1806 of yacc.c  */
#line 2140 "hphp.y"
    { (yyval).reset();}
    break;

  case 583:

/* Line 1806 of yacc.c  */
#line 2144 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 584:

/* Line 1806 of yacc.c  */
#line 2145 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 585:

/* Line 1806 of yacc.c  */
#line 2146 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));}
    break;

  case 586:

/* Line 1806 of yacc.c  */
#line 2148 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));}
    break;

  case 587:

/* Line 1806 of yacc.c  */
#line 2149 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));}
    break;

  case 588:

/* Line 1806 of yacc.c  */
#line 2150 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));}
    break;

  case 589:

/* Line 1806 of yacc.c  */
#line 2151 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));}
    break;

  case 590:

/* Line 1806 of yacc.c  */
#line 2152 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));}
    break;

  case 591:

/* Line 1806 of yacc.c  */
#line 2153 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));}
    break;

  case 592:

/* Line 1806 of yacc.c  */
#line 2154 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));}
    break;

  case 593:

/* Line 1806 of yacc.c  */
#line 2155 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));}
    break;

  case 594:

/* Line 1806 of yacc.c  */
#line 2156 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));}
    break;

  case 595:

/* Line 1806 of yacc.c  */
#line 2159 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));}
    break;

  case 596:

/* Line 1806 of yacc.c  */
#line 2161 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
    break;

  case 597:

/* Line 1806 of yacc.c  */
#line 2165 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 598:

/* Line 1806 of yacc.c  */
#line 2166 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 599:

/* Line 1806 of yacc.c  */
#line 2167 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 600:

/* Line 1806 of yacc.c  */
#line 2168 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 601:

/* Line 1806 of yacc.c  */
#line 2170 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 602:

/* Line 1806 of yacc.c  */
#line 2172 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 603:

/* Line 1806 of yacc.c  */
#line 2173 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); }
    break;

  case 604:

/* Line 1806 of yacc.c  */
#line 2175 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 605:

/* Line 1806 of yacc.c  */
#line 2176 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 606:

/* Line 1806 of yacc.c  */
#line 2177 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 607:

/* Line 1806 of yacc.c  */
#line 2183 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 608:

/* Line 1806 of yacc.c  */
#line 2185 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 609:

/* Line 1806 of yacc.c  */
#line 2189 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 610:

/* Line 1806 of yacc.c  */
#line 2193 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 611:

/* Line 1806 of yacc.c  */
#line 2194 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 612:

/* Line 1806 of yacc.c  */
#line 2195 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 613:

/* Line 1806 of yacc.c  */
#line 2196 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 614:

/* Line 1806 of yacc.c  */
#line 2197 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));}
    break;

  case 615:

/* Line 1806 of yacc.c  */
#line 2198 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));}
    break;

  case 616:

/* Line 1806 of yacc.c  */
#line 2200 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));}
    break;

  case 617:

/* Line 1806 of yacc.c  */
#line 2205 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 618:

/* Line 1806 of yacc.c  */
#line 2206 "hphp.y"
    { (yyval).reset();}
    break;

  case 619:

/* Line 1806 of yacc.c  */
#line 2210 "hphp.y"
    { (yyval).reset();}
    break;

  case 620:

/* Line 1806 of yacc.c  */
#line 2211 "hphp.y"
    { (yyval).reset();}
    break;

  case 621:

/* Line 1806 of yacc.c  */
#line 2214 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();}
    break;

  case 622:

/* Line 1806 of yacc.c  */
#line 2215 "hphp.y"
    { (yyval).reset();}
    break;

  case 623:

/* Line 1806 of yacc.c  */
#line 2221 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 624:

/* Line 1806 of yacc.c  */
#line 2223 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 625:

/* Line 1806 of yacc.c  */
#line 2225 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 626:

/* Line 1806 of yacc.c  */
#line 2226 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 627:

/* Line 1806 of yacc.c  */
#line 2230 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 628:

/* Line 1806 of yacc.c  */
#line 2231 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 629:

/* Line 1806 of yacc.c  */
#line 2232 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));}
    break;

  case 630:

/* Line 1806 of yacc.c  */
#line 2236 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));}
    break;

  case 631:

/* Line 1806 of yacc.c  */
#line 2238 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
    break;

  case 632:

/* Line 1806 of yacc.c  */
#line 2241 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));}
    break;

  case 633:

/* Line 1806 of yacc.c  */
#line 2242 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));}
    break;

  case 634:

/* Line 1806 of yacc.c  */
#line 2243 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));}
    break;

  case 635:

/* Line 1806 of yacc.c  */
#line 2246 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 636:

/* Line 1806 of yacc.c  */
#line 2247 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));}
    break;

  case 637:

/* Line 1806 of yacc.c  */
#line 2248 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 638:

/* Line 1806 of yacc.c  */
#line 2249 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 639:

/* Line 1806 of yacc.c  */
#line 2251 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 640:

/* Line 1806 of yacc.c  */
#line 2253 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 641:

/* Line 1806 of yacc.c  */
#line 2254 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 642:

/* Line 1806 of yacc.c  */
#line 2256 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 643:

/* Line 1806 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 644:

/* Line 1806 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval).reset();}
    break;

  case 645:

/* Line 1806 of yacc.c  */
#line 2267 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 646:

/* Line 1806 of yacc.c  */
#line 2269 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 647:

/* Line 1806 of yacc.c  */
#line 2271 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 648:

/* Line 1806 of yacc.c  */
#line 2272 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 649:

/* Line 1806 of yacc.c  */
#line 2276 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 650:

/* Line 1806 of yacc.c  */
#line 2277 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 651:

/* Line 1806 of yacc.c  */
#line 2282 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 652:

/* Line 1806 of yacc.c  */
#line 2283 "hphp.y"
    { (yyval).reset(); }
    break;

  case 653:

/* Line 1806 of yacc.c  */
#line 2288 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 654:

/* Line 1806 of yacc.c  */
#line 2291 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 655:

/* Line 1806 of yacc.c  */
#line 2296 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 656:

/* Line 1806 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval).reset();}
    break;

  case 657:

/* Line 1806 of yacc.c  */
#line 2300 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 658:

/* Line 1806 of yacc.c  */
#line 2301 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
    break;

  case 659:

/* Line 1806 of yacc.c  */
#line 2308 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 660:

/* Line 1806 of yacc.c  */
#line 2310 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 661:

/* Line 1806 of yacc.c  */
#line 2313 "hphp.y"
    { only_in_hh_syntax(_p);}
    break;

  case 662:

/* Line 1806 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 663:

/* Line 1806 of yacc.c  */
#line 2318 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 664:

/* Line 1806 of yacc.c  */
#line 2321 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 665:

/* Line 1806 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval).reset();}
    break;

  case 666:

/* Line 1806 of yacc.c  */
#line 2326 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 667:

/* Line 1806 of yacc.c  */
#line 2328 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 668:

/* Line 1806 of yacc.c  */
#line 2332 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 669:

/* Line 1806 of yacc.c  */
#line 2333 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 670:

/* Line 1806 of yacc.c  */
#line 2337 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 671:

/* Line 1806 of yacc.c  */
#line 2338 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 672:

/* Line 1806 of yacc.c  */
#line 2342 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 673:

/* Line 1806 of yacc.c  */
#line 2344 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 674:

/* Line 1806 of yacc.c  */
#line 2349 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 675:

/* Line 1806 of yacc.c  */
#line 2351 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 676:

/* Line 1806 of yacc.c  */
#line 2355 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 677:

/* Line 1806 of yacc.c  */
#line 2356 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 678:

/* Line 1806 of yacc.c  */
#line 2357 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 679:

/* Line 1806 of yacc.c  */
#line 2358 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 680:

/* Line 1806 of yacc.c  */
#line 2359 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 681:

/* Line 1806 of yacc.c  */
#line 2360 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 682:

/* Line 1806 of yacc.c  */
#line 2362 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 683:

/* Line 1806 of yacc.c  */
#line 2365 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 684:

/* Line 1806 of yacc.c  */
#line 2367 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 685:

/* Line 1806 of yacc.c  */
#line 2368 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 686:

/* Line 1806 of yacc.c  */
#line 2372 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 687:

/* Line 1806 of yacc.c  */
#line 2373 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 688:

/* Line 1806 of yacc.c  */
#line 2374 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 689:

/* Line 1806 of yacc.c  */
#line 2375 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 690:

/* Line 1806 of yacc.c  */
#line 2377 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 691:

/* Line 1806 of yacc.c  */
#line 2379 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 692:

/* Line 1806 of yacc.c  */
#line 2381 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 693:

/* Line 1806 of yacc.c  */
#line 2382 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 694:

/* Line 1806 of yacc.c  */
#line 2386 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 695:

/* Line 1806 of yacc.c  */
#line 2387 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 696:

/* Line 1806 of yacc.c  */
#line 2388 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 697:

/* Line 1806 of yacc.c  */
#line 2394 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));}
    break;

  case 698:

/* Line 1806 of yacc.c  */
#line 2397 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 699:

/* Line 1806 of yacc.c  */
#line 2400 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));}
    break;

  case 700:

/* Line 1806 of yacc.c  */
#line 2404 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));}
    break;

  case 701:

/* Line 1806 of yacc.c  */
#line 2408 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));}
    break;

  case 702:

/* Line 1806 of yacc.c  */
#line 2412 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));}
    break;

  case 703:

/* Line 1806 of yacc.c  */
#line 2419 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));}
    break;

  case 704:

/* Line 1806 of yacc.c  */
#line 2423 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));}
    break;

  case 705:

/* Line 1806 of yacc.c  */
#line 2427 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));}
    break;

  case 706:

/* Line 1806 of yacc.c  */
#line 2431 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 707:

/* Line 1806 of yacc.c  */
#line 2433 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 708:

/* Line 1806 of yacc.c  */
#line 2438 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 709:

/* Line 1806 of yacc.c  */
#line 2439 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 710:

/* Line 1806 of yacc.c  */
#line 2440 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 711:

/* Line 1806 of yacc.c  */
#line 2443 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 712:

/* Line 1806 of yacc.c  */
#line 2444 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);}
    break;

  case 713:

/* Line 1806 of yacc.c  */
#line 2447 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 714:

/* Line 1806 of yacc.c  */
#line 2448 "hphp.y"
    { (yyval).reset();}
    break;

  case 715:

/* Line 1806 of yacc.c  */
#line 2452 "hphp.y"
    { (yyval) = 1;}
    break;

  case 716:

/* Line 1806 of yacc.c  */
#line 2453 "hphp.y"
    { (yyval)++;}
    break;

  case 717:

/* Line 1806 of yacc.c  */
#line 2457 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 718:

/* Line 1806 of yacc.c  */
#line 2458 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 719:

/* Line 1806 of yacc.c  */
#line 2459 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 720:

/* Line 1806 of yacc.c  */
#line 2461 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 721:

/* Line 1806 of yacc.c  */
#line 2464 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 722:

/* Line 1806 of yacc.c  */
#line 2465 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 724:

/* Line 1806 of yacc.c  */
#line 2469 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 725:

/* Line 1806 of yacc.c  */
#line 2471 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 726:

/* Line 1806 of yacc.c  */
#line 2473 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 727:

/* Line 1806 of yacc.c  */
#line 2474 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 728:

/* Line 1806 of yacc.c  */
#line 2478 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);}
    break;

  case 729:

/* Line 1806 of yacc.c  */
#line 2479 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 730:

/* Line 1806 of yacc.c  */
#line 2481 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 731:

/* Line 1806 of yacc.c  */
#line 2482 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);}
    break;

  case 732:

/* Line 1806 of yacc.c  */
#line 2483 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));}
    break;

  case 733:

/* Line 1806 of yacc.c  */
#line 2484 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));}
    break;

  case 734:

/* Line 1806 of yacc.c  */
#line 2489 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 735:

/* Line 1806 of yacc.c  */
#line 2490 "hphp.y"
    { (yyval).reset();}
    break;

  case 736:

/* Line 1806 of yacc.c  */
#line 2494 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 737:

/* Line 1806 of yacc.c  */
#line 2495 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 738:

/* Line 1806 of yacc.c  */
#line 2496 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 739:

/* Line 1806 of yacc.c  */
#line 2497 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 740:

/* Line 1806 of yacc.c  */
#line 2500 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);}
    break;

  case 741:

/* Line 1806 of yacc.c  */
#line 2502 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);}
    break;

  case 742:

/* Line 1806 of yacc.c  */
#line 2503 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 743:

/* Line 1806 of yacc.c  */
#line 2504 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);}
    break;

  case 744:

/* Line 1806 of yacc.c  */
#line 2509 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 745:

/* Line 1806 of yacc.c  */
#line 2510 "hphp.y"
    { _p->onEmptyCollection((yyval));}
    break;

  case 746:

/* Line 1806 of yacc.c  */
#line 2514 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 747:

/* Line 1806 of yacc.c  */
#line 2515 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));}
    break;

  case 748:

/* Line 1806 of yacc.c  */
#line 2516 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 749:

/* Line 1806 of yacc.c  */
#line 2517 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));}
    break;

  case 750:

/* Line 1806 of yacc.c  */
#line 2522 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 751:

/* Line 1806 of yacc.c  */
#line 2523 "hphp.y"
    { _p->onEmptyCollection((yyval));}
    break;

  case 752:

/* Line 1806 of yacc.c  */
#line 2528 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 753:

/* Line 1806 of yacc.c  */
#line 2530 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));}
    break;

  case 754:

/* Line 1806 of yacc.c  */
#line 2532 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 755:

/* Line 1806 of yacc.c  */
#line 2533 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));}
    break;

  case 756:

/* Line 1806 of yacc.c  */
#line 2537 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);}
    break;

  case 757:

/* Line 1806 of yacc.c  */
#line 2539 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);}
    break;

  case 758:

/* Line 1806 of yacc.c  */
#line 2540 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);}
    break;

  case 759:

/* Line 1806 of yacc.c  */
#line 2542 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); }
    break;

  case 760:

/* Line 1806 of yacc.c  */
#line 2547 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 761:

/* Line 1806 of yacc.c  */
#line 2549 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 762:

/* Line 1806 of yacc.c  */
#line 2551 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 763:

/* Line 1806 of yacc.c  */
#line 2553 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);}
    break;

  case 764:

/* Line 1806 of yacc.c  */
#line 2555 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));}
    break;

  case 765:

/* Line 1806 of yacc.c  */
#line 2556 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 766:

/* Line 1806 of yacc.c  */
#line 2559 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;}
    break;

  case 767:

/* Line 1806 of yacc.c  */
#line 2560 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;}
    break;

  case 768:

/* Line 1806 of yacc.c  */
#line 2561 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;}
    break;

  case 769:

/* Line 1806 of yacc.c  */
#line 2565 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);}
    break;

  case 770:

/* Line 1806 of yacc.c  */
#line 2566 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);}
    break;

  case 771:

/* Line 1806 of yacc.c  */
#line 2567 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);}
    break;

  case 772:

/* Line 1806 of yacc.c  */
#line 2568 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);}
    break;

  case 773:

/* Line 1806 of yacc.c  */
#line 2569 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);}
    break;

  case 774:

/* Line 1806 of yacc.c  */
#line 2570 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);}
    break;

  case 775:

/* Line 1806 of yacc.c  */
#line 2571 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);}
    break;

  case 776:

/* Line 1806 of yacc.c  */
#line 2572 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);}
    break;

  case 777:

/* Line 1806 of yacc.c  */
#line 2573 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);}
    break;

  case 778:

/* Line 1806 of yacc.c  */
#line 2577 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 779:

/* Line 1806 of yacc.c  */
#line 2578 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 780:

/* Line 1806 of yacc.c  */
#line 2583 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 781:

/* Line 1806 of yacc.c  */
#line 2585 "hphp.y"
    { _p->onClassClass((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 784:

/* Line 1806 of yacc.c  */
#line 2599 "hphp.y"
    { (yyvsp[(2) - (5)]).setText(_p->nsDecl((yyvsp[(2) - (5)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); }
    break;

  case 785:

/* Line 1806 of yacc.c  */
#line 2603 "hphp.y"
    { (yyvsp[(2) - (6)]).setText(_p->nsDecl((yyvsp[(2) - (6)]).text()));
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); }
    break;

  case 786:

/* Line 1806 of yacc.c  */
#line 2609 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 787:

/* Line 1806 of yacc.c  */
#line 2610 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 788:

/* Line 1806 of yacc.c  */
#line 2616 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 789:

/* Line 1806 of yacc.c  */
#line 2620 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); }
    break;

  case 790:

/* Line 1806 of yacc.c  */
#line 2626 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 791:

/* Line 1806 of yacc.c  */
#line 2627 "hphp.y"
    { (yyval).reset(); }
    break;

  case 792:

/* Line 1806 of yacc.c  */
#line 2631 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 793:

/* Line 1806 of yacc.c  */
#line 2634 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); }
    break;

  case 794:

/* Line 1806 of yacc.c  */
#line 2639 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); }
    break;

  case 795:

/* Line 1806 of yacc.c  */
#line 2640 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 796:

/* Line 1806 of yacc.c  */
#line 2641 "hphp.y"
    { (yyval).reset(); }
    break;

  case 797:

/* Line 1806 of yacc.c  */
#line 2642 "hphp.y"
    { (yyval).reset(); }
    break;

  case 798:

/* Line 1806 of yacc.c  */
#line 2646 "hphp.y"
    { (yyval).reset(); }
    break;

  case 799:

/* Line 1806 of yacc.c  */
#line 2647 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 800:

/* Line 1806 of yacc.c  */
#line 2652 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); }
    break;

  case 801:

/* Line 1806 of yacc.c  */
#line 2653 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); }
    break;

  case 802:

/* Line 1806 of yacc.c  */
#line 2655 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); }
    break;

  case 803:

/* Line 1806 of yacc.c  */
#line 2656 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); }
    break;

  case 804:

/* Line 1806 of yacc.c  */
#line 2662 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); }
    break;

  case 807:

/* Line 1806 of yacc.c  */
#line 2673 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 808:

/* Line 1806 of yacc.c  */
#line 2675 "hphp.y"
    {}
    break;

  case 809:

/* Line 1806 of yacc.c  */
#line 2679 "hphp.y"
    { (yyval).setText("array"); }
    break;

  case 810:

/* Line 1806 of yacc.c  */
#line 2686 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 811:

/* Line 1806 of yacc.c  */
#line 2689 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 812:

/* Line 1806 of yacc.c  */
#line 2692 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 813:

/* Line 1806 of yacc.c  */
#line 2693 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); }
    break;

  case 814:

/* Line 1806 of yacc.c  */
#line 2696 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("callable");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); }
    break;

  case 815:

/* Line 1806 of yacc.c  */
#line 2699 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 816:

/* Line 1806 of yacc.c  */
#line 2701 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); }
    break;

  case 817:

/* Line 1806 of yacc.c  */
#line 2704 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); }
    break;

  case 818:

/* Line 1806 of yacc.c  */
#line 2707 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
    break;

  case 819:

/* Line 1806 of yacc.c  */
#line 2713 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
    break;

  case 820:

/* Line 1806 of yacc.c  */
#line 2717 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (5)]));
                                        _p->onTypeSpecialization((yyval), 't'); }
    break;

  case 821:

/* Line 1806 of yacc.c  */
#line 2725 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 822:

/* Line 1806 of yacc.c  */
#line 2726 "hphp.y"
    { (yyval).reset(); }
    break;



/* Line 1806 of yacc.c  */
#line 12348 "hphp.tab.cpp"
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
#line 2729 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

