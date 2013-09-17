
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

#ifdef XHPAST2_PARSER
#include "hphp/parser/xhpast2/parser.h"
#else
#include "hphp/compiler/parser/parser.h"
#endif
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
#define yyerror(loc,p,msg) p->fatal(loc,msg)

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

static void finally_statement(Parser *_p) {
  if (!_p->enableFinallyStatement()) {
    HPHP_PARSER_ERROR("Finally statement is not enabled", _p);
  }
}

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


/* Line 189 of yacc.c  */
#line 643 "hphp.tab.cpp"

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
     T_LOGICAL_OR = 263,
     T_LOGICAL_XOR = 264,
     T_LOGICAL_AND = 265,
     T_PRINT = 266,
     T_SR_EQUAL = 267,
     T_SL_EQUAL = 268,
     T_XOR_EQUAL = 269,
     T_OR_EQUAL = 270,
     T_AND_EQUAL = 271,
     T_MOD_EQUAL = 272,
     T_CONCAT_EQUAL = 273,
     T_DIV_EQUAL = 274,
     T_MUL_EQUAL = 275,
     T_MINUS_EQUAL = 276,
     T_PLUS_EQUAL = 277,
     T_BOOLEAN_OR = 278,
     T_BOOLEAN_AND = 279,
     T_IS_NOT_IDENTICAL = 280,
     T_IS_IDENTICAL = 281,
     T_IS_NOT_EQUAL = 282,
     T_IS_EQUAL = 283,
     T_IS_GREATER_OR_EQUAL = 284,
     T_IS_SMALLER_OR_EQUAL = 285,
     T_SR = 286,
     T_SL = 287,
     T_INSTANCEOF = 288,
     T_UNSET_CAST = 289,
     T_BOOL_CAST = 290,
     T_OBJECT_CAST = 291,
     T_ARRAY_CAST = 292,
     T_STRING_CAST = 293,
     T_DOUBLE_CAST = 294,
     T_INT_CAST = 295,
     T_DEC = 296,
     T_INC = 297,
     T_CLONE = 298,
     T_NEW = 299,
     T_EXIT = 300,
     T_IF = 301,
     T_ELSEIF = 302,
     T_ELSE = 303,
     T_ENDIF = 304,
     T_LNUMBER = 305,
     T_DNUMBER = 306,
     T_STRING = 307,
     T_STRING_VARNAME = 308,
     T_VARIABLE = 309,
     T_NUM_STRING = 310,
     T_INLINE_HTML = 311,
     T_CHARACTER = 312,
     T_BAD_CHARACTER = 313,
     T_ENCAPSED_AND_WHITESPACE = 314,
     T_CONSTANT_ENCAPSED_STRING = 315,
     T_ECHO = 316,
     T_DO = 317,
     T_WHILE = 318,
     T_ENDWHILE = 319,
     T_FOR = 320,
     T_ENDFOR = 321,
     T_FOREACH = 322,
     T_ENDFOREACH = 323,
     T_DECLARE = 324,
     T_ENDDECLARE = 325,
     T_AS = 326,
     T_SWITCH = 327,
     T_ENDSWITCH = 328,
     T_CASE = 329,
     T_DEFAULT = 330,
     T_BREAK = 331,
     T_GOTO = 332,
     T_CONTINUE = 333,
     T_FUNCTION = 334,
     T_CONST = 335,
     T_RETURN = 336,
     T_TRY = 337,
     T_CATCH = 338,
     T_THROW = 339,
     T_USE = 340,
     T_GLOBAL = 341,
     T_PUBLIC = 342,
     T_PROTECTED = 343,
     T_PRIVATE = 344,
     T_FINAL = 345,
     T_ABSTRACT = 346,
     T_STATIC = 347,
     T_VAR = 348,
     T_UNSET = 349,
     T_ISSET = 350,
     T_EMPTY = 351,
     T_HALT_COMPILER = 352,
     T_CLASS = 353,
     T_INTERFACE = 354,
     T_EXTENDS = 355,
     T_IMPLEMENTS = 356,
     T_OBJECT_OPERATOR = 357,
     T_DOUBLE_ARROW = 358,
     T_LIST = 359,
     T_ARRAY = 360,
     T_CLASS_C = 361,
     T_METHOD_C = 362,
     T_FUNC_C = 363,
     T_LINE = 364,
     T_FILE = 365,
     T_COMMENT = 366,
     T_DOC_COMMENT = 367,
     T_OPEN_TAG = 368,
     T_OPEN_TAG_WITH_ECHO = 369,
     T_CLOSE_TAG = 370,
     T_WHITESPACE = 371,
     T_START_HEREDOC = 372,
     T_END_HEREDOC = 373,
     T_DOLLAR_OPEN_CURLY_BRACES = 374,
     T_CURLY_OPEN = 375,
     T_PAAMAYIM_NEKUDOTAYIM = 376,
     T_NAMESPACE = 377,
     T_NS_C = 378,
     T_DIR = 379,
     T_NS_SEPARATOR = 380,
     T_YIELD = 381,
     T_XHP_LABEL = 382,
     T_XHP_TEXT = 383,
     T_XHP_ATTRIBUTE = 384,
     T_XHP_CATEGORY = 385,
     T_XHP_CATEGORY_LABEL = 386,
     T_XHP_CHILDREN = 387,
     T_XHP_ENUM = 388,
     T_XHP_REQUIRED = 389,
     T_TRAIT = 390,
     T_INSTEADOF = 391,
     T_TRAIT_C = 392,
     T_VARARG = 393,
     T_HH_ERROR = 394,
     T_FINALLY = 395,
     T_XHP_TAG_LT = 396,
     T_XHP_TAG_GT = 397,
     T_TYPELIST_LT = 398,
     T_TYPELIST_GT = 399,
     T_UNRESOLVED_LT = 400,
     T_COLLECTION = 401,
     T_SHAPE = 402,
     T_TYPE = 403,
     T_UNRESOLVED_TYPE = 404,
     T_NEWTYPE = 405,
     T_UNRESOLVED_NEWTYPE = 406,
     T_COMPILER_HALT_OFFSET = 407,
     T_AWAIT = 408,
     T_ASYNC = 409,
     T_TUPLE = 410
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
#line 853 "hphp.tab.cpp"

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
#define YYLAST   11029

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  185
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  218
/* YYNRULES -- Number of rules.  */
#define YYNRULES  753
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1412

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   410

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    48,   183,     2,   180,    47,    31,   184,
     175,   176,    45,    42,     8,    43,    44,    46,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    26,   177,
      36,    13,    37,    25,    51,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    61,     2,   182,    30,     2,   181,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   178,    29,   179,    50,     2,     2,     2,
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
       5,     6,     7,     9,    10,    11,    12,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    27,    28,
      32,    33,    34,    35,    38,    39,    40,    41,    49,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    62,    63,
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
     174
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    11,    13,    15,    17,
      19,    21,    26,    30,    31,    38,    39,    45,    49,    52,
      54,    56,    58,    60,    62,    64,    66,    70,    72,    74,
      77,    81,    86,    88,    92,    94,    98,   101,   103,   106,
     109,   115,   120,   123,   124,   126,   128,   130,   132,   136,
     142,   151,   152,   157,   158,   165,   166,   177,   178,   183,
     186,   190,   193,   197,   200,   204,   208,   212,   216,   220,
     226,   228,   230,   231,   241,   247,   262,   268,   272,   276,
     279,   282,   285,   288,   291,   294,   298,   301,   304,   314,
     315,   316,   322,   324,   325,   327,   328,   330,   331,   341,
     342,   353,   354,   366,   367,   376,   377,   387,   388,   396,
     397,   406,   407,   415,   416,   425,   427,   429,   431,   433,
     435,   438,   441,   444,   445,   448,   449,   452,   453,   455,
     459,   461,   465,   468,   469,   471,   474,   479,   481,   486,
     488,   493,   495,   500,   502,   507,   511,   517,   521,   526,
     531,   537,   543,   548,   549,   551,   553,   558,   559,   565,
     566,   569,   570,   574,   575,   579,   582,   584,   585,   590,
     596,   604,   611,   618,   626,   636,   645,   649,   652,   654,
     655,   659,   664,   671,   677,   683,   690,   699,   707,   710,
     711,   713,   716,   720,   725,   729,   731,   733,   736,   741,
     745,   751,   753,   757,   760,   761,   762,   767,   768,   774,
     777,   778,   789,   790,   802,   806,   810,   814,   818,   824,
     827,   830,   831,   838,   844,   849,   853,   855,   857,   861,
     866,   868,   870,   872,   874,   879,   881,   885,   888,   889,
     892,   893,   895,   899,   901,   903,   905,   907,   911,   916,
     921,   926,   928,   930,   933,   936,   939,   943,   947,   949,
     951,   953,   955,   959,   961,   965,   967,   969,   971,   972,
     974,   977,   979,   981,   983,   985,   987,   989,   991,   993,
     994,   996,   998,  1000,  1004,  1010,  1012,  1016,  1022,  1027,
    1031,  1035,  1038,  1040,  1042,  1046,  1050,  1052,  1054,  1055,
    1058,  1063,  1067,  1074,  1077,  1081,  1088,  1090,  1092,  1094,
    1101,  1105,  1110,  1117,  1121,  1125,  1129,  1133,  1137,  1141,
    1145,  1149,  1153,  1157,  1161,  1164,  1167,  1170,  1173,  1177,
    1181,  1185,  1189,  1193,  1197,  1201,  1205,  1209,  1213,  1217,
    1221,  1225,  1229,  1233,  1237,  1240,  1243,  1246,  1249,  1253,
    1257,  1261,  1265,  1269,  1273,  1277,  1281,  1285,  1289,  1295,
    1300,  1302,  1305,  1308,  1311,  1314,  1317,  1320,  1323,  1326,
    1329,  1331,  1333,  1335,  1339,  1342,  1343,  1355,  1356,  1369,
    1371,  1373,  1379,  1383,  1389,  1393,  1396,  1397,  1400,  1401,
    1406,  1411,  1415,  1420,  1425,  1430,  1435,  1437,  1439,  1443,
    1449,  1450,  1454,  1459,  1461,  1464,  1469,  1472,  1479,  1480,
    1482,  1487,  1488,  1491,  1492,  1494,  1496,  1500,  1502,  1506,
    1508,  1510,  1514,  1518,  1520,  1522,  1524,  1526,  1528,  1530,
    1532,  1534,  1536,  1538,  1540,  1542,  1544,  1546,  1548,  1550,
    1552,  1554,  1556,  1558,  1560,  1562,  1564,  1566,  1568,  1570,
    1572,  1574,  1576,  1578,  1580,  1582,  1584,  1586,  1588,  1590,
    1592,  1594,  1596,  1598,  1600,  1602,  1604,  1606,  1608,  1610,
    1612,  1614,  1616,  1618,  1620,  1622,  1624,  1626,  1628,  1630,
    1632,  1634,  1636,  1638,  1640,  1642,  1644,  1646,  1648,  1650,
    1652,  1654,  1656,  1658,  1660,  1662,  1664,  1666,  1668,  1670,
    1672,  1674,  1676,  1678,  1683,  1685,  1687,  1689,  1691,  1693,
    1695,  1697,  1699,  1702,  1704,  1705,  1706,  1708,  1710,  1714,
    1715,  1717,  1719,  1721,  1723,  1725,  1727,  1729,  1731,  1733,
    1735,  1737,  1739,  1743,  1746,  1748,  1750,  1753,  1756,  1761,
    1766,  1770,  1775,  1777,  1779,  1783,  1787,  1789,  1791,  1793,
    1795,  1799,  1803,  1807,  1810,  1811,  1813,  1814,  1816,  1817,
    1823,  1827,  1831,  1833,  1835,  1837,  1839,  1843,  1846,  1848,
    1850,  1852,  1854,  1856,  1859,  1862,  1867,  1872,  1876,  1881,
    1884,  1885,  1891,  1895,  1899,  1901,  1905,  1907,  1910,  1911,
    1917,  1921,  1924,  1925,  1929,  1930,  1935,  1938,  1939,  1943,
    1947,  1949,  1950,  1952,  1955,  1958,  1963,  1967,  1971,  1974,
    1979,  1982,  1987,  1989,  1991,  1993,  1995,  1997,  2000,  2005,
    2009,  2014,  2018,  2020,  2022,  2024,  2026,  2029,  2034,  2039,
    2043,  2045,  2047,  2051,  2059,  2066,  2075,  2085,  2094,  2105,
    2113,  2120,  2122,  2125,  2130,  2135,  2137,  2139,  2144,  2146,
    2147,  2149,  2152,  2154,  2156,  2159,  2164,  2168,  2172,  2173,
    2175,  2178,  2183,  2187,  2190,  2194,  2201,  2202,  2204,  2209,
    2212,  2213,  2219,  2223,  2227,  2229,  2236,  2241,  2246,  2249,
    2252,  2253,  2259,  2263,  2267,  2269,  2272,  2273,  2279,  2283,
    2287,  2289,  2292,  2295,  2297,  2300,  2302,  2307,  2311,  2315,
    2322,  2326,  2328,  2330,  2332,  2337,  2342,  2345,  2348,  2353,
    2356,  2359,  2361,  2365,  2369,  2370,  2373,  2379,  2386,  2388,
    2391,  2393,  2398,  2402,  2403,  2405,  2409,  2413,  2415,  2417,
    2418,  2419,  2422,  2426,  2428,  2434,  2438,  2442,  2446,  2448,
    2451,  2452,  2457,  2460,  2463,  2465,  2467,  2469,  2474,  2481,
    2483,  2492,  2498,  2500
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     186,     0,    -1,    -1,   187,   188,    -1,   188,   189,    -1,
      -1,   203,    -1,   215,    -1,   219,    -1,   224,    -1,   389,
      -1,   116,   175,   176,   177,    -1,   141,   195,   177,    -1,
      -1,   141,   195,   178,   190,   188,   179,    -1,    -1,   141,
     178,   191,   188,   179,    -1,   104,   193,   177,    -1,   200,
     177,    -1,    71,    -1,   148,    -1,   149,    -1,   151,    -1,
     153,    -1,   152,    -1,   174,    -1,   193,     8,   194,    -1,
     194,    -1,   195,    -1,   144,   195,    -1,   195,    90,   192,
      -1,   144,   195,    90,   192,    -1,   192,    -1,   195,   144,
     192,    -1,   195,    -1,   141,   144,   195,    -1,   144,   195,
      -1,   196,    -1,   196,   392,    -1,   196,   392,    -1,   200,
       8,   390,    13,   336,    -1,    99,   390,    13,   336,    -1,
     201,   202,    -1,    -1,   203,    -1,   215,    -1,   219,    -1,
     224,    -1,   178,   201,   179,    -1,    65,   292,   203,   246,
     248,    -1,    65,   292,    26,   201,   247,   249,    68,   177,
      -1,    -1,    82,   292,   204,   240,    -1,    -1,    81,   205,
     203,    82,   292,   177,    -1,    -1,    84,   175,   294,   177,
     294,   177,   294,   176,   206,   238,    -1,    -1,    91,   292,
     207,   243,    -1,    95,   177,    -1,    95,   301,   177,    -1,
      97,   177,    -1,    97,   301,   177,    -1,   100,   177,    -1,
     100,   301,   177,    -1,   145,    95,   177,    -1,   105,   256,
     177,    -1,   111,   258,   177,    -1,    80,   293,   177,    -1,
     113,   175,   386,   176,   177,    -1,   177,    -1,    75,    -1,
      -1,    86,   175,   301,    90,   237,   236,   176,   208,   239,
      -1,    88,   175,   242,   176,   241,    -1,   101,   178,   201,
     179,   102,   175,   329,    73,   176,   178,   201,   179,   209,
     212,    -1,   101,   178,   201,   179,   210,    -1,   103,   301,
     177,    -1,    96,   192,   177,    -1,   301,   177,    -1,   295,
     177,    -1,   296,   177,    -1,   297,   177,    -1,   298,   177,
      -1,   299,   177,    -1,   100,   298,   177,    -1,   300,   177,
      -1,   192,    26,    -1,   209,   102,   175,   329,    73,   176,
     178,   201,   179,    -1,    -1,    -1,   159,   178,   201,   179,
     211,    -1,   210,    -1,    -1,    31,    -1,    -1,    98,    -1,
      -1,   214,   213,   391,   216,   175,   252,   176,   395,   281,
      -1,    -1,   285,   214,   213,   391,   217,   175,   252,   176,
     395,   281,    -1,    -1,   356,   284,   214,   213,   391,   218,
     175,   252,   176,   395,   281,    -1,    -1,   230,   227,   220,
     231,   232,   178,   259,   179,    -1,    -1,   356,   230,   227,
     221,   231,   232,   178,   259,   179,    -1,    -1,   118,   228,
     222,   233,   178,   259,   179,    -1,    -1,   356,   118,   228,
     223,   233,   178,   259,   179,    -1,    -1,   154,   229,   225,
     232,   178,   259,   179,    -1,    -1,   356,   154,   229,   226,
     232,   178,   259,   179,    -1,   391,    -1,   146,    -1,   391,
      -1,   391,    -1,   117,    -1,   110,   117,    -1,   109,   117,
      -1,   119,   329,    -1,    -1,   120,   234,    -1,    -1,   119,
     234,    -1,    -1,   329,    -1,   234,     8,   329,    -1,   329,
      -1,   235,     8,   329,    -1,   122,   237,    -1,    -1,   363,
      -1,    31,   363,    -1,   123,   175,   375,   176,    -1,   203,
      -1,    26,   201,    85,   177,    -1,   203,    -1,    26,   201,
      87,   177,    -1,   203,    -1,    26,   201,    83,   177,    -1,
     203,    -1,    26,   201,    89,   177,    -1,   192,    13,   336,
      -1,   242,     8,   192,    13,   336,    -1,   178,   244,   179,
      -1,   178,   177,   244,   179,    -1,    26,   244,    92,   177,
      -1,    26,   177,   244,    92,   177,    -1,   244,    93,   301,
     245,   201,    -1,   244,    94,   245,   201,    -1,    -1,    26,
      -1,   177,    -1,   246,    66,   292,   203,    -1,    -1,   247,
      66,   292,    26,   201,    -1,    -1,    67,   203,    -1,    -1,
      67,    26,   201,    -1,    -1,   251,     8,   157,    -1,   251,
     341,    -1,   157,    -1,    -1,   357,   287,   402,    73,    -1,
     357,   287,   402,    31,    73,    -1,   357,   287,   402,    31,
      73,    13,   336,    -1,   357,   287,   402,    73,    13,   336,
      -1,   251,     8,   357,   287,   402,    73,    -1,   251,     8,
     357,   287,   402,    31,    73,    -1,   251,     8,   357,   287,
     402,    31,    73,    13,   336,    -1,   251,     8,   357,   287,
     402,    73,    13,   336,    -1,   253,     8,   157,    -1,   253,
     341,    -1,   157,    -1,    -1,   357,   402,    73,    -1,   357,
     402,    31,    73,    -1,   357,   402,    31,    73,    13,   336,
      -1,   357,   402,    73,    13,   336,    -1,   253,     8,   357,
     402,    73,    -1,   253,     8,   357,   402,    31,    73,    -1,
     253,     8,   357,   402,    31,    73,    13,   336,    -1,   253,
       8,   357,   402,    73,    13,   336,    -1,   255,   341,    -1,
      -1,   301,    -1,    31,   363,    -1,   255,     8,   301,    -1,
     255,     8,    31,   363,    -1,   256,     8,   257,    -1,   257,
      -1,    73,    -1,   180,   363,    -1,   180,   178,   301,   179,
      -1,   258,     8,    73,    -1,   258,     8,    73,    13,   336,
      -1,    73,    -1,    73,    13,   336,    -1,   259,   260,    -1,
      -1,    -1,   283,   261,   289,   177,    -1,    -1,   285,   401,
     262,   289,   177,    -1,   290,   177,    -1,    -1,   284,   214,
     213,   391,   175,   263,   250,   176,   395,   282,    -1,    -1,
     356,   284,   214,   213,   391,   175,   264,   250,   176,   395,
     282,    -1,   148,   269,   177,    -1,   149,   275,   177,    -1,
     151,   277,   177,    -1,   104,   235,   177,    -1,   104,   235,
     178,   265,   179,    -1,   265,   266,    -1,   265,   267,    -1,
      -1,   199,   140,   192,   155,   235,   177,    -1,   268,    90,
     284,   192,   177,    -1,   268,    90,   285,   177,    -1,   199,
     140,   192,    -1,   192,    -1,   270,    -1,   269,     8,   270,
      -1,   271,   326,   273,   274,    -1,   146,    -1,   124,    -1,
     329,    -1,   112,    -1,   152,   178,   272,   179,    -1,   335,
      -1,   272,     8,   335,    -1,    13,   336,    -1,    -1,    51,
     153,    -1,    -1,   276,    -1,   275,     8,   276,    -1,   150,
      -1,   278,    -1,   192,    -1,   115,    -1,   175,   279,   176,
      -1,   175,   279,   176,    45,    -1,   175,   279,   176,    25,
      -1,   175,   279,   176,    42,    -1,   278,    -1,   280,    -1,
     280,    45,    -1,   280,    25,    -1,   280,    42,    -1,   279,
       8,   279,    -1,   279,    29,   279,    -1,   192,    -1,   146,
      -1,   150,    -1,   177,    -1,   178,   201,   179,    -1,   177,
      -1,   178,   201,   179,    -1,   285,    -1,   112,    -1,   285,
      -1,    -1,   286,    -1,   285,   286,    -1,   106,    -1,   107,
      -1,   108,    -1,   111,    -1,   110,    -1,   109,    -1,   173,
      -1,   288,    -1,    -1,   106,    -1,   107,    -1,   108,    -1,
     289,     8,    73,    -1,   289,     8,    73,    13,   336,    -1,
      73,    -1,    73,    13,   336,    -1,   290,     8,   390,    13,
     336,    -1,    99,   390,    13,   336,    -1,   175,   291,   176,
      -1,    63,   331,   334,    -1,    62,   301,    -1,   318,    -1,
     312,    -1,   175,   301,   176,    -1,   293,     8,   301,    -1,
     301,    -1,   293,    -1,    -1,   145,   301,    -1,   145,   301,
     122,   301,    -1,   363,    13,   295,    -1,   123,   175,   375,
     176,    13,   295,    -1,   172,   301,    -1,   363,    13,   298,
      -1,   123,   175,   375,   176,    13,   298,    -1,   302,    -1,
     363,    -1,   291,    -1,   123,   175,   375,   176,    13,   301,
      -1,   363,    13,   301,    -1,   363,    13,    31,   363,    -1,
     363,    13,    31,    63,   331,   334,    -1,   363,    24,   301,
      -1,   363,    23,   301,    -1,   363,    22,   301,    -1,   363,
      21,   301,    -1,   363,    20,   301,    -1,   363,    19,   301,
      -1,   363,    18,   301,    -1,   363,    17,   301,    -1,   363,
      16,   301,    -1,   363,    15,   301,    -1,   363,    14,   301,
      -1,   363,    60,    -1,    60,   363,    -1,   363,    59,    -1,
      59,   363,    -1,   301,    27,   301,    -1,   301,    28,   301,
      -1,   301,     9,   301,    -1,   301,    11,   301,    -1,   301,
      10,   301,    -1,   301,    29,   301,    -1,   301,    31,   301,
      -1,   301,    30,   301,    -1,   301,    44,   301,    -1,   301,
      42,   301,    -1,   301,    43,   301,    -1,   301,    45,   301,
      -1,   301,    46,   301,    -1,   301,    47,   301,    -1,   301,
      41,   301,    -1,   301,    40,   301,    -1,    42,   301,    -1,
      43,   301,    -1,    48,   301,    -1,    50,   301,    -1,   301,
      33,   301,    -1,   301,    32,   301,    -1,   301,    35,   301,
      -1,   301,    34,   301,    -1,   301,    36,   301,    -1,   301,
      39,   301,    -1,   301,    37,   301,    -1,   301,    38,   301,
      -1,   301,    49,   331,    -1,   175,   302,   176,    -1,   301,
      25,   301,    26,   301,    -1,   301,    25,    26,   301,    -1,
     385,    -1,    58,   301,    -1,    57,   301,    -1,    56,   301,
      -1,    55,   301,    -1,    54,   301,    -1,    53,   301,    -1,
      52,   301,    -1,    64,   332,    -1,    51,   301,    -1,   338,
      -1,   311,    -1,   310,    -1,   181,   333,   181,    -1,    12,
     301,    -1,    -1,   214,   213,   175,   303,   252,   176,   395,
     316,   178,   201,   179,    -1,    -1,   285,   214,   213,   175,
     304,   252,   176,   395,   316,   178,   201,   179,    -1,   314,
      -1,    79,    -1,   306,     8,   305,   122,   301,    -1,   305,
     122,   301,    -1,   307,     8,   305,   122,   336,    -1,   305,
     122,   336,    -1,   306,   340,    -1,    -1,   307,   340,    -1,
      -1,   166,   175,   308,   176,    -1,   124,   175,   376,   176,
      -1,    61,   376,   182,    -1,   329,   178,   378,   179,    -1,
     329,   178,   380,   179,    -1,   314,    61,   371,   182,    -1,
     315,    61,   371,   182,    -1,   311,    -1,   387,    -1,   175,
     302,   176,    -1,   104,   175,   317,   341,   176,    -1,    -1,
     317,     8,    73,    -1,   317,     8,    31,    73,    -1,    73,
      -1,    31,    73,    -1,   160,   146,   319,   161,    -1,   321,
      46,    -1,   321,   161,   322,   160,    46,   320,    -1,    -1,
     146,    -1,   321,   323,    13,   324,    -1,    -1,   322,   325,
      -1,    -1,   146,    -1,   147,    -1,   178,   301,   179,    -1,
     147,    -1,   178,   301,   179,    -1,   318,    -1,   327,    -1,
     326,    26,   327,    -1,   326,    43,   327,    -1,   192,    -1,
      64,    -1,    98,    -1,    99,    -1,   100,    -1,   145,    -1,
     172,    -1,   101,    -1,   102,    -1,   159,    -1,   103,    -1,
      65,    -1,    66,    -1,    68,    -1,    67,    -1,    82,    -1,
      83,    -1,    81,    -1,    84,    -1,    85,    -1,    86,    -1,
      87,    -1,    88,    -1,    89,    -1,    49,    -1,    90,    -1,
      91,    -1,    92,    -1,    93,    -1,    94,    -1,    95,    -1,
      97,    -1,    96,    -1,    80,    -1,    12,    -1,   117,    -1,
     118,    -1,   119,    -1,   120,    -1,    63,    -1,    62,    -1,
     112,    -1,     5,    -1,     7,    -1,     6,    -1,     4,    -1,
       3,    -1,   141,    -1,   104,    -1,   105,    -1,   114,    -1,
     115,    -1,   116,    -1,   111,    -1,   110,    -1,   109,    -1,
     108,    -1,   107,    -1,   106,    -1,   173,    -1,   113,    -1,
     123,    -1,   124,    -1,     9,    -1,    11,    -1,    10,    -1,
     125,    -1,   127,    -1,   126,    -1,   128,    -1,   129,    -1,
     143,    -1,   142,    -1,   171,    -1,   154,    -1,   156,    -1,
     155,    -1,   167,    -1,   169,    -1,   166,    -1,   198,   175,
     254,   176,    -1,   199,    -1,   146,    -1,   329,    -1,   111,
      -1,   369,    -1,   329,    -1,   111,    -1,   373,    -1,   175,
     176,    -1,   292,    -1,    -1,    -1,    78,    -1,   382,    -1,
     175,   254,   176,    -1,    -1,    69,    -1,    70,    -1,    79,
      -1,   128,    -1,   129,    -1,   143,    -1,   125,    -1,   156,
      -1,   126,    -1,   127,    -1,   142,    -1,   171,    -1,   136,
      78,   137,    -1,   136,   137,    -1,   335,    -1,   197,    -1,
      42,   336,    -1,    43,   336,    -1,   124,   175,   339,   176,
      -1,   174,   175,   339,   176,    -1,    61,   339,   182,    -1,
     166,   175,   309,   176,    -1,   337,    -1,   313,    -1,   199,
     140,   192,    -1,   146,   140,   192,    -1,   197,    -1,    72,
      -1,   387,    -1,   335,    -1,   183,   382,   183,    -1,   184,
     382,   184,    -1,   136,   382,   137,    -1,   342,   340,    -1,
      -1,     8,    -1,    -1,     8,    -1,    -1,   342,     8,   336,
     122,   336,    -1,   342,     8,   336,    -1,   336,   122,   336,
      -1,   336,    -1,    69,    -1,    70,    -1,    79,    -1,   136,
      78,   137,    -1,   136,   137,    -1,    69,    -1,    70,    -1,
     192,    -1,   343,    -1,   192,    -1,    42,   344,    -1,    43,
     344,    -1,   124,   175,   346,   176,    -1,   174,   175,   346,
     176,    -1,    61,   346,   182,    -1,   166,   175,   349,   176,
      -1,   347,   340,    -1,    -1,   347,     8,   345,   122,   345,
      -1,   347,     8,   345,    -1,   345,   122,   345,    -1,   345,
      -1,   348,     8,   345,    -1,   345,    -1,   350,   340,    -1,
      -1,   350,     8,   305,   122,   345,    -1,   305,   122,   345,
      -1,   348,   340,    -1,    -1,   175,   351,   176,    -1,    -1,
     353,     8,   192,   352,    -1,   192,   352,    -1,    -1,   355,
     353,   340,    -1,    41,   354,    40,    -1,   356,    -1,    -1,
     359,    -1,   121,   368,    -1,   121,   192,    -1,   121,   178,
     301,   179,    -1,    61,   371,   182,    -1,   178,   301,   179,
      -1,   364,   360,    -1,   175,   291,   176,   360,    -1,   374,
     360,    -1,   175,   291,   176,   360,    -1,   368,    -1,   328,
      -1,   366,    -1,   367,    -1,   361,    -1,   363,   358,    -1,
     175,   291,   176,   358,    -1,   330,   140,   368,    -1,   365,
     175,   254,   176,    -1,   175,   363,   176,    -1,   328,    -1,
     366,    -1,   367,    -1,   361,    -1,   363,   359,    -1,   175,
     291,   176,   359,    -1,   365,   175,   254,   176,    -1,   175,
     363,   176,    -1,   368,    -1,   361,    -1,   175,   363,   176,
      -1,   363,   121,   192,   392,   175,   254,   176,    -1,   363,
     121,   368,   175,   254,   176,    -1,   363,   121,   178,   301,
     179,   175,   254,   176,    -1,   175,   291,   176,   121,   192,
     392,   175,   254,   176,    -1,   175,   291,   176,   121,   368,
     175,   254,   176,    -1,   175,   291,   176,   121,   178,   301,
     179,   175,   254,   176,    -1,   330,   140,   192,   392,   175,
     254,   176,    -1,   330,   140,   368,   175,   254,   176,    -1,
     369,    -1,   372,   369,    -1,   369,    61,   371,   182,    -1,
     369,   178,   301,   179,    -1,   370,    -1,    73,    -1,   180,
     178,   301,   179,    -1,   301,    -1,    -1,   180,    -1,   372,
     180,    -1,   368,    -1,   362,    -1,   373,   358,    -1,   175,
     291,   176,   358,    -1,   330,   140,   368,    -1,   175,   363,
     176,    -1,    -1,   362,    -1,   373,   359,    -1,   175,   291,
     176,   359,    -1,   175,   363,   176,    -1,   375,     8,    -1,
     375,     8,   363,    -1,   375,     8,   123,   175,   375,   176,
      -1,    -1,   363,    -1,   123,   175,   375,   176,    -1,   377,
     340,    -1,    -1,   377,     8,   301,   122,   301,    -1,   377,
       8,   301,    -1,   301,   122,   301,    -1,   301,    -1,   377,
       8,   301,   122,    31,   363,    -1,   377,     8,    31,   363,
      -1,   301,   122,    31,   363,    -1,    31,   363,    -1,   379,
     340,    -1,    -1,   379,     8,   301,   122,   301,    -1,   379,
       8,   301,    -1,   301,   122,   301,    -1,   301,    -1,   381,
     340,    -1,    -1,   381,     8,   336,   122,   336,    -1,   381,
       8,   336,    -1,   336,   122,   336,    -1,   336,    -1,   382,
     383,    -1,   382,    78,    -1,   383,    -1,    78,   383,    -1,
      73,    -1,    73,    61,   384,   182,    -1,    73,   121,   192,
      -1,   138,   301,   179,    -1,   138,    72,    61,   301,   182,
     179,    -1,   139,   363,   179,    -1,   192,    -1,    74,    -1,
      73,    -1,   114,   175,   386,   176,    -1,   115,   175,   363,
     176,    -1,     7,   301,    -1,     6,   301,    -1,     5,   175,
     301,   176,    -1,     4,   301,    -1,     3,   301,    -1,   363,
      -1,   386,     8,   363,    -1,   330,   140,   192,    -1,    -1,
      90,   401,    -1,   167,   391,    13,   401,   177,    -1,   169,
     391,   388,    13,   401,   177,    -1,   192,    -1,   401,   192,
      -1,   192,    -1,   192,   162,   396,   163,    -1,   162,   393,
     163,    -1,    -1,   401,    -1,   393,     8,   401,    -1,   393,
       8,   157,    -1,   393,    -1,   157,    -1,    -1,    -1,    26,
     401,    -1,   396,     8,   192,    -1,   192,    -1,   396,     8,
     192,    90,   401,    -1,   192,    90,   401,    -1,    79,   122,
     401,    -1,   398,     8,   397,    -1,   397,    -1,   398,   340,
      -1,    -1,   166,   175,   399,   176,    -1,    25,   401,    -1,
      51,   401,    -1,   199,    -1,   124,    -1,   400,    -1,   124,
     162,   401,   163,    -1,   124,   162,   401,     8,   401,   163,
      -1,   146,    -1,   175,    98,   175,   394,   176,    26,   401,
     176,    -1,   175,   393,     8,   401,   176,    -1,   401,    -1,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   707,   707,   707,   712,   714,   717,   718,   719,   720,
     721,   722,   725,   727,   727,   729,   729,   731,   732,   737,
     738,   739,   740,   741,   742,   743,   747,   749,   752,   753,
     754,   755,   760,   761,   765,   766,   768,   771,   777,   784,
     791,   795,   801,   803,   806,   807,   808,   809,   812,   813,
     817,   822,   822,   826,   826,   831,   830,   834,   834,   837,
     838,   839,   840,   841,   842,   843,   844,   845,   846,   847,
     848,   849,   852,   850,   855,   857,   865,   868,   869,   873,
     874,   875,   876,   877,   878,   879,   880,   881,   888,   894,
     899,   898,   904,   905,   909,   910,   914,   919,   918,   929,
     927,   939,   937,   951,   950,   969,   967,   986,   985,   994,
     992,  1004,  1003,  1015,  1013,  1026,  1027,  1031,  1034,  1037,
    1038,  1039,  1042,  1044,  1047,  1048,  1051,  1052,  1055,  1056,
    1060,  1061,  1066,  1067,  1070,  1071,  1072,  1076,  1077,  1081,
    1082,  1086,  1087,  1091,  1092,  1097,  1098,  1103,  1104,  1105,
    1106,  1109,  1112,  1114,  1117,  1118,  1122,  1124,  1127,  1130,
    1133,  1134,  1137,  1138,  1142,  1144,  1146,  1147,  1151,  1155,
    1159,  1164,  1169,  1174,  1179,  1185,  1194,  1196,  1198,  1199,
    1203,  1206,  1209,  1213,  1217,  1221,  1225,  1230,  1238,  1240,
    1243,  1244,  1245,  1247,  1252,  1253,  1256,  1257,  1258,  1262,
    1263,  1265,  1266,  1270,  1272,  1275,  1275,  1279,  1278,  1282,
    1286,  1284,  1297,  1294,  1305,  1307,  1309,  1311,  1313,  1317,
    1318,  1319,  1322,  1328,  1331,  1337,  1340,  1345,  1347,  1352,
    1357,  1361,  1362,  1368,  1369,  1374,  1375,  1380,  1381,  1385,
    1386,  1390,  1392,  1398,  1403,  1404,  1406,  1410,  1411,  1412,
    1413,  1417,  1418,  1419,  1420,  1421,  1422,  1424,  1429,  1432,
    1433,  1437,  1438,  1442,  1443,  1446,  1447,  1450,  1451,  1454,
    1455,  1459,  1460,  1461,  1462,  1463,  1464,  1465,  1469,  1470,
    1473,  1474,  1475,  1478,  1480,  1482,  1483,  1486,  1488,  1492,
    1493,  1495,  1496,  1497,  1500,  1504,  1505,  1509,  1510,  1514,
    1515,  1519,  1523,  1528,  1532,  1536,  1541,  1542,  1543,  1546,
    1548,  1549,  1550,  1553,  1554,  1555,  1556,  1557,  1558,  1559,
    1560,  1561,  1562,  1563,  1564,  1565,  1566,  1567,  1568,  1569,
    1570,  1571,  1572,  1573,  1574,  1575,  1576,  1577,  1578,  1579,
    1580,  1581,  1582,  1583,  1584,  1585,  1586,  1587,  1588,  1589,
    1590,  1591,  1592,  1593,  1595,  1596,  1598,  1600,  1601,  1602,
    1603,  1604,  1605,  1606,  1607,  1608,  1609,  1610,  1611,  1612,
    1613,  1614,  1615,  1616,  1617,  1619,  1618,  1628,  1627,  1636,
    1640,  1644,  1648,  1654,  1658,  1664,  1666,  1670,  1672,  1676,
    1680,  1681,  1685,  1692,  1699,  1701,  1706,  1707,  1708,  1712,
    1714,  1718,  1719,  1720,  1721,  1725,  1731,  1740,  1753,  1754,
    1757,  1760,  1763,  1764,  1767,  1771,  1774,  1777,  1784,  1785,
    1789,  1790,  1792,  1796,  1797,  1798,  1799,  1800,  1801,  1802,
    1803,  1804,  1805,  1806,  1807,  1808,  1809,  1810,  1811,  1812,
    1813,  1814,  1815,  1816,  1817,  1818,  1819,  1820,  1821,  1822,
    1823,  1824,  1825,  1826,  1827,  1828,  1829,  1830,  1831,  1832,
    1833,  1834,  1835,  1836,  1837,  1838,  1839,  1840,  1841,  1842,
    1843,  1844,  1845,  1846,  1847,  1848,  1849,  1850,  1851,  1852,
    1853,  1854,  1855,  1856,  1857,  1858,  1859,  1860,  1861,  1862,
    1863,  1864,  1865,  1866,  1867,  1868,  1869,  1870,  1871,  1872,
    1873,  1874,  1875,  1879,  1884,  1885,  1888,  1889,  1890,  1894,
    1895,  1896,  1900,  1901,  1902,  1906,  1907,  1908,  1911,  1913,
    1917,  1918,  1919,  1921,  1922,  1923,  1924,  1925,  1926,  1927,
    1928,  1929,  1930,  1933,  1938,  1939,  1940,  1941,  1942,  1944,
    1946,  1947,  1949,  1950,  1954,  1957,  1963,  1964,  1965,  1966,
    1967,  1968,  1969,  1974,  1976,  1980,  1981,  1984,  1985,  1989,
    1992,  1994,  1996,  2000,  2001,  2002,  2004,  2007,  2011,  2012,
    2013,  2016,  2017,  2018,  2019,  2020,  2022,  2024,  2025,  2030,
    2032,  2035,  2038,  2040,  2042,  2045,  2047,  2051,  2053,  2056,
    2059,  2065,  2067,  2070,  2071,  2076,  2079,  2083,  2083,  2088,
    2091,  2092,  2096,  2097,  2102,  2103,  2107,  2108,  2112,  2113,
    2118,  2120,  2125,  2126,  2127,  2128,  2129,  2130,  2131,  2133,
    2136,  2138,  2142,  2143,  2144,  2145,  2146,  2148,  2150,  2152,
    2156,  2157,  2158,  2162,  2165,  2168,  2171,  2175,  2179,  2186,
    2190,  2197,  2198,  2203,  2205,  2206,  2209,  2210,  2213,  2214,
    2218,  2219,  2223,  2224,  2225,  2226,  2228,  2231,  2234,  2235,
    2236,  2238,  2240,  2244,  2245,  2246,  2248,  2249,  2250,  2254,
    2256,  2259,  2261,  2262,  2263,  2264,  2267,  2269,  2270,  2274,
    2276,  2279,  2281,  2282,  2283,  2287,  2289,  2292,  2295,  2297,
    2299,  2303,  2304,  2306,  2307,  2313,  2314,  2316,  2318,  2320,
    2322,  2325,  2326,  2327,  2331,  2332,  2333,  2334,  2335,  2336,
    2337,  2341,  2342,  2346,  2354,  2356,  2360,  2363,  2369,  2370,
    2376,  2377,  2384,  2387,  2391,  2394,  2399,  2400,  2401,  2402,
    2406,  2407,  2411,  2413,  2414,  2416,  2420,  2426,  2428,  2432,
    2435,  2438,  2446,  2449,  2452,  2453,  2456,  2457,  2460,  2464,
    2468,  2474,  2482,  2483
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_REQUIRE_ONCE", "T_REQUIRE", "T_EVAL",
  "T_INCLUDE_ONCE", "T_INCLUDE", "','", "T_LOGICAL_OR", "T_LOGICAL_XOR",
  "T_LOGICAL_AND", "T_PRINT", "'='", "T_SR_EQUAL", "T_SL_EQUAL",
  "T_XOR_EQUAL", "T_OR_EQUAL", "T_AND_EQUAL", "T_MOD_EQUAL",
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
  "T_OBJECT_OPERATOR", "T_DOUBLE_ARROW", "T_LIST", "T_ARRAY", "T_CLASS_C",
  "T_METHOD_C", "T_FUNC_C", "T_LINE", "T_FILE", "T_COMMENT",
  "T_DOC_COMMENT", "T_OPEN_TAG", "T_OPEN_TAG_WITH_ECHO", "T_CLOSE_TAG",
  "T_WHITESPACE", "T_START_HEREDOC", "T_END_HEREDOC",
  "T_DOLLAR_OPEN_CURLY_BRACES", "T_CURLY_OPEN", "T_PAAMAYIM_NEKUDOTAYIM",
  "T_NAMESPACE", "T_NS_C", "T_DIR", "T_NS_SEPARATOR", "T_YIELD",
  "T_XHP_LABEL", "T_XHP_TEXT", "T_XHP_ATTRIBUTE", "T_XHP_CATEGORY",
  "T_XHP_CATEGORY_LABEL", "T_XHP_CHILDREN", "T_XHP_ENUM", "T_XHP_REQUIRED",
  "T_TRAIT", "T_INSTEADOF", "T_TRAIT_C", "T_VARARG", "T_HH_ERROR",
  "T_FINALLY", "T_XHP_TAG_LT", "T_XHP_TAG_GT", "T_TYPELIST_LT",
  "T_TYPELIST_GT", "T_UNRESOLVED_LT", "T_COLLECTION", "T_SHAPE", "T_TYPE",
  "T_UNRESOLVED_TYPE", "T_NEWTYPE", "T_UNRESOLVED_NEWTYPE",
  "T_COMPILER_HALT_OFFSET", "T_AWAIT", "T_ASYNC", "T_TUPLE", "'('", "')'",
  "';'", "'{'", "'}'", "'$'", "'`'", "']'", "'\"'", "'\\''", "$accept",
  "start", "$@1", "top_statement_list", "top_statement", "$@2", "$@3",
  "ident", "use_declarations", "use_declaration", "namespace_name",
  "namespace_string_base", "namespace_string", "namespace_string_typeargs",
  "class_namespace_string_typeargs", "constant_declaration",
  "inner_statement_list", "inner_statement", "statement", "$@4", "$@5",
  "$@6", "$@7", "$@8", "additional_catches", "finally", "@9",
  "optional_finally", "is_reference", "function_loc",
  "function_declaration_statement", "$@10", "$@11", "$@12",
  "class_declaration_statement", "$@13", "$@14", "$@15", "$@16",
  "trait_declaration_statement", "$@17", "$@18", "class_decl_name",
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
  "class_statement_list", "class_statement", "$@19", "$@20", "$@21",
  "$@22", "trait_rules", "trait_precedence_rule", "trait_alias_rule",
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
  "expr_no_variable", "$@23", "$@24", "shape_keyname",
  "non_empty_shape_pair_list", "non_empty_static_shape_pair_list",
  "shape_pair_list", "static_shape_pair_list", "shape_literal",
  "array_literal", "collection_literal", "static_collection_literal",
  "dim_expr", "dim_expr_base", "lexical_vars", "lexical_var_list",
  "xhp_tag", "xhp_tag_body", "xhp_opt_end_label", "xhp_attributes",
  "xhp_children", "xhp_attribute_name", "xhp_attribute_value", "xhp_child",
  "xhp_label_ws", "xhp_bareword", "simple_function_call",
  "fully_qualified_class_name", "static_class_name",
  "class_name_reference", "exit_expr", "backticks_expr", "ctor_arguments",
  "common_scalar", "static_scalar", "static_class_constant", "scalar",
  "static_array_pair_list", "possible_comma", "hh_possible_comma",
  "non_empty_static_array_pair_list", "common_scalar_ae",
  "static_numeric_scalar_ae", "static_scalar_ae",
  "static_array_pair_list_ae", "non_empty_static_array_pair_list_ae",
  "non_empty_static_scalar_list_ae", "static_shape_pair_list_ae",
  "non_empty_static_shape_pair_list_ae", "static_scalar_list_ae",
  "attribute_static_scalar_list", "non_empty_user_attribute_list",
  "user_attribute_list", "$@25", "non_empty_user_attributes",
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
       0,   256,   257,   258,   259,   260,   261,   262,    44,   263,
     264,   265,   266,    61,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,    63,    58,   278,   279,   124,
      94,    38,   280,   281,   282,   283,    60,    62,   284,   285,
     286,   287,    43,    45,    46,    42,    47,    37,    33,   288,
     126,    64,   289,   290,   291,   292,   293,   294,   295,   296,
     297,    91,   298,   299,   300,   301,   302,   303,   304,   305,
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
     406,   407,   408,   409,   410,    40,    41,    59,   123,   125,
      36,    96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   185,   187,   186,   188,   188,   189,   189,   189,   189,
     189,   189,   189,   190,   189,   191,   189,   189,   189,   192,
     192,   192,   192,   192,   192,   192,   193,   193,   194,   194,
     194,   194,   195,   195,   196,   196,   196,   197,   198,   199,
     200,   200,   201,   201,   202,   202,   202,   202,   203,   203,
     203,   204,   203,   205,   203,   206,   203,   207,   203,   203,
     203,   203,   203,   203,   203,   203,   203,   203,   203,   203,
     203,   203,   208,   203,   203,   203,   203,   203,   203,   203,
     203,   203,   203,   203,   203,   203,   203,   203,   209,   209,
     211,   210,   212,   212,   213,   213,   214,   216,   215,   217,
     215,   218,   215,   220,   219,   221,   219,   222,   219,   223,
     219,   225,   224,   226,   224,   227,   227,   228,   229,   230,
     230,   230,   231,   231,   232,   232,   233,   233,   234,   234,
     235,   235,   236,   236,   237,   237,   237,   238,   238,   239,
     239,   240,   240,   241,   241,   242,   242,   243,   243,   243,
     243,   244,   244,   244,   245,   245,   246,   246,   247,   247,
     248,   248,   249,   249,   250,   250,   250,   250,   251,   251,
     251,   251,   251,   251,   251,   251,   252,   252,   252,   252,
     253,   253,   253,   253,   253,   253,   253,   253,   254,   254,
     255,   255,   255,   255,   256,   256,   257,   257,   257,   258,
     258,   258,   258,   259,   259,   261,   260,   262,   260,   260,
     263,   260,   264,   260,   260,   260,   260,   260,   260,   265,
     265,   265,   266,   267,   267,   268,   268,   269,   269,   270,
     270,   271,   271,   271,   271,   272,   272,   273,   273,   274,
     274,   275,   275,   276,   277,   277,   277,   278,   278,   278,
     278,   279,   279,   279,   279,   279,   279,   279,   280,   280,
     280,   281,   281,   282,   282,   283,   283,   284,   284,   285,
     285,   286,   286,   286,   286,   286,   286,   286,   287,   287,
     288,   288,   288,   289,   289,   289,   289,   290,   290,   291,
     291,   291,   291,   291,   292,   293,   293,   294,   294,   295,
     295,   296,   297,   298,   299,   300,   301,   301,   301,   302,
     302,   302,   302,   302,   302,   302,   302,   302,   302,   302,
     302,   302,   302,   302,   302,   302,   302,   302,   302,   302,
     302,   302,   302,   302,   302,   302,   302,   302,   302,   302,
     302,   302,   302,   302,   302,   302,   302,   302,   302,   302,
     302,   302,   302,   302,   302,   302,   302,   302,   302,   302,
     302,   302,   302,   302,   302,   302,   302,   302,   302,   302,
     302,   302,   302,   302,   302,   303,   302,   304,   302,   302,
     305,   306,   306,   307,   307,   308,   308,   309,   309,   310,
     311,   311,   312,   313,   314,   314,   315,   315,   315,   316,
     316,   317,   317,   317,   317,   318,   319,   319,   320,   320,
     321,   321,   322,   322,   323,   324,   324,   325,   325,   325,
     326,   326,   326,   327,   327,   327,   327,   327,   327,   327,
     327,   327,   327,   327,   327,   327,   327,   327,   327,   327,
     327,   327,   327,   327,   327,   327,   327,   327,   327,   327,
     327,   327,   327,   327,   327,   327,   327,   327,   327,   327,
     327,   327,   327,   327,   327,   327,   327,   327,   327,   327,
     327,   327,   327,   327,   327,   327,   327,   327,   327,   327,
     327,   327,   327,   327,   327,   327,   327,   327,   327,   327,
     327,   327,   327,   327,   327,   327,   327,   327,   327,   327,
     327,   327,   327,   328,   329,   329,   330,   330,   330,   331,
     331,   331,   332,   332,   332,   333,   333,   333,   334,   334,
     335,   335,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   335,   335,   336,   336,   336,   336,   336,   336,
     336,   336,   336,   336,   337,   337,   338,   338,   338,   338,
     338,   338,   338,   339,   339,   340,   340,   341,   341,   342,
     342,   342,   342,   343,   343,   343,   343,   343,   344,   344,
     344,   345,   345,   345,   345,   345,   345,   345,   345,   346,
     346,   347,   347,   347,   347,   348,   348,   349,   349,   350,
     350,   351,   351,   352,   352,   353,   353,   355,   354,   356,
     357,   357,   358,   358,   359,   359,   360,   360,   361,   361,
     362,   362,   363,   363,   363,   363,   363,   363,   363,   363,
     363,   363,   364,   364,   364,   364,   364,   364,   364,   364,
     365,   365,   365,   366,   366,   366,   366,   366,   366,   367,
     367,   368,   368,   369,   369,   369,   370,   370,   371,   371,
     372,   372,   373,   373,   373,   373,   373,   373,   374,   374,
     374,   374,   374,   375,   375,   375,   375,   375,   375,   376,
     376,   377,   377,   377,   377,   377,   377,   377,   377,   378,
     378,   379,   379,   379,   379,   380,   380,   381,   381,   381,
     381,   382,   382,   382,   382,   383,   383,   383,   383,   383,
     383,   384,   384,   384,   385,   385,   385,   385,   385,   385,
     385,   386,   386,   387,   388,   388,   389,   389,   390,   390,
     391,   391,   392,   392,   393,   393,   394,   394,   394,   394,
     395,   395,   396,   396,   396,   396,   397,   398,   398,   399,
     399,   400,   401,   401,   401,   401,   401,   401,   401,   401,
     401,   401,   402,   402
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     4,     3,     0,     6,     0,     5,     3,     2,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     2,
       3,     4,     1,     3,     1,     3,     2,     1,     2,     2,
       5,     4,     2,     0,     1,     1,     1,     1,     3,     5,
       8,     0,     4,     0,     6,     0,    10,     0,     4,     2,
       3,     2,     3,     2,     3,     3,     3,     3,     3,     5,
       1,     1,     0,     9,     5,    14,     5,     3,     3,     2,
       2,     2,     2,     2,     2,     3,     2,     2,     9,     0,
       0,     5,     1,     0,     1,     0,     1,     0,     9,     0,
      10,     0,    11,     0,     8,     0,     9,     0,     7,     0,
       8,     0,     7,     0,     8,     1,     1,     1,     1,     1,
       2,     2,     2,     0,     2,     0,     2,     0,     1,     3,
       1,     3,     2,     0,     1,     2,     4,     1,     4,     1,
       4,     1,     4,     1,     4,     3,     5,     3,     4,     4,
       5,     5,     4,     0,     1,     1,     4,     0,     5,     0,
       2,     0,     3,     0,     3,     2,     1,     0,     4,     5,
       7,     6,     6,     7,     9,     8,     3,     2,     1,     0,
       3,     4,     6,     5,     5,     6,     8,     7,     2,     0,
       1,     2,     3,     4,     3,     1,     1,     2,     4,     3,
       5,     1,     3,     2,     0,     0,     4,     0,     5,     2,
       0,    10,     0,    11,     3,     3,     3,     3,     5,     2,
       2,     0,     6,     5,     4,     3,     1,     1,     3,     4,
       1,     1,     1,     1,     4,     1,     3,     2,     0,     2,
       0,     1,     3,     1,     1,     1,     1,     3,     4,     4,
       4,     1,     1,     2,     2,     2,     3,     3,     1,     1,
       1,     1,     3,     1,     3,     1,     1,     1,     0,     1,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       1,     1,     1,     3,     5,     1,     3,     5,     4,     3,
       3,     2,     1,     1,     3,     3,     1,     1,     0,     2,
       4,     3,     6,     2,     3,     6,     1,     1,     1,     6,
       3,     4,     6,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     5,     4,
       1,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       1,     1,     1,     3,     2,     0,    11,     0,    12,     1,
       1,     5,     3,     5,     3,     2,     0,     2,     0,     4,
       4,     3,     4,     4,     4,     4,     1,     1,     3,     5,
       0,     3,     4,     1,     2,     4,     2,     6,     0,     1,
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
       1,     1,     3,     2,     1,     1,     2,     2,     4,     4,
       3,     4,     1,     1,     3,     3,     1,     1,     1,     1,
       3,     3,     3,     2,     0,     1,     0,     1,     0,     5,
       3,     3,     1,     1,     1,     1,     3,     2,     1,     1,
       1,     1,     1,     2,     2,     4,     4,     3,     4,     2,
       0,     5,     3,     3,     1,     3,     1,     2,     0,     5,
       3,     2,     0,     3,     0,     4,     2,     0,     3,     3,
       1,     0,     1,     2,     2,     4,     3,     3,     2,     4,
       2,     4,     1,     1,     1,     1,     1,     2,     4,     3,
       4,     3,     1,     1,     1,     1,     2,     4,     4,     3,
       1,     1,     3,     7,     6,     8,     9,     8,    10,     7,
       6,     1,     2,     4,     4,     1,     1,     4,     1,     0,
       1,     2,     1,     1,     2,     4,     3,     3,     0,     1,
       2,     4,     3,     2,     3,     6,     0,     1,     4,     2,
       0,     5,     3,     3,     1,     6,     4,     4,     2,     2,
       0,     5,     3,     3,     1,     2,     0,     5,     3,     3,
       1,     2,     2,     1,     2,     1,     4,     3,     3,     6,
       3,     1,     1,     1,     4,     4,     2,     2,     4,     2,
       2,     1,     3,     3,     0,     2,     5,     6,     1,     2,
       1,     4,     3,     0,     1,     3,     3,     1,     1,     0,
       0,     2,     3,     1,     5,     3,     3,     3,     1,     2,
       0,     4,     2,     2,     1,     1,     1,     4,     6,     1,
       8,     5,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   597,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   670,     0,   658,   514,
       0,   520,   521,    19,   547,   646,    71,   522,     0,    53,
       0,     0,     0,     0,     0,     0,     0,     0,    96,     0,
       0,     0,     0,     0,     0,   271,   272,   273,   276,   275,
     274,     0,     0,     0,     0,   119,     0,     0,     0,   526,
     528,   529,   523,   524,     0,     0,   530,   525,     0,     0,
     505,    20,    21,    22,    24,    23,     0,   527,     0,     0,
       0,     0,   531,     0,   277,    25,     0,    70,    43,   650,
     515,     0,     0,     4,    32,    34,    37,   546,     0,   504,
       0,     6,    95,     7,     8,     9,     0,     0,   269,   308,
       0,     0,     0,     0,     0,     0,     0,   306,   372,   371,
     293,   379,     0,   292,   613,   506,     0,   549,   370,   268,
     616,   307,     0,     0,   614,   615,   612,   641,   645,     0,
     360,   548,    10,   276,   275,   274,     0,     0,    32,    95,
       0,   710,   307,   709,     0,   707,   706,   374,     0,     0,
     344,   345,   346,   347,   369,   367,   366,   365,   364,   363,
     362,   361,   507,     0,   723,   506,     0,   327,   325,     0,
     674,     0,   556,   291,   510,     0,   723,   509,     0,   519,
     653,   652,   511,     0,     0,   513,   368,     0,     0,     0,
     296,     0,    51,   298,     0,     0,    57,    59,     0,     0,
      61,     0,     0,     0,   745,   749,     0,     0,    32,   744,
       0,   746,     0,    63,     0,     0,    43,     0,     0,     0,
      27,    28,   196,     0,     0,   195,   121,   120,   201,     0,
       0,     0,     0,     0,   720,   107,   117,   666,   670,   695,
       0,   533,     0,     0,     0,   693,     0,    15,     0,    36,
       0,   299,   111,   118,   411,   386,     0,   714,   303,   308,
       0,   306,   307,     0,     0,   516,     0,   517,     0,     0,
       0,    87,     0,     0,    39,   189,     0,    18,    94,     0,
     116,   103,   115,   274,    95,   270,    80,    81,    82,    83,
      84,    86,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   658,    79,   649,
     649,   680,     0,     0,     0,     0,     0,   267,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     326,   324,     0,   617,   602,   649,     0,   608,   189,   649,
       0,   651,   642,   666,     0,    95,     0,     0,   599,   594,
     556,     0,     0,     0,     0,   678,     0,   391,   555,   669,
       0,     0,    39,     0,   189,   290,     0,   654,   602,   610,
     512,     0,    43,   157,     0,    68,     0,     0,   297,     0,
       0,     0,     0,     0,    60,    78,    62,   742,   743,     0,
     740,     0,     0,   724,     0,   719,    85,    64,     0,    77,
      29,     0,    17,     0,     0,   197,     0,    66,     0,     0,
      67,   711,     0,     0,     0,     0,     0,   127,     0,   667,
       0,     0,     0,     0,   532,   694,   547,     0,     0,   692,
     552,   691,    35,     5,    12,    13,    65,     0,   125,     0,
       0,   380,     0,   556,     0,     0,     0,     0,   289,   357,
     621,    48,    42,    44,    45,    46,    47,     0,   373,   550,
     551,    33,     0,     0,     0,   558,   190,     0,   375,    97,
     123,     0,   330,   332,   331,     0,     0,   328,   329,   333,
     335,   334,   349,   348,   351,   350,   352,   354,   355,   353,
     343,   342,   337,   338,   336,   339,   340,   341,   356,   648,
       0,     0,   684,     0,   556,   713,   619,   641,   109,   113,
     105,    95,     0,     0,   301,   304,   310,   323,   322,   321,
     320,   319,   318,   317,   316,   315,   314,   313,     0,   604,
     603,     0,     0,     0,     0,     0,     0,     0,   708,   592,
     596,   555,   598,     0,     0,   723,     0,   673,     0,   672,
       0,   657,   656,     0,     0,   604,   603,   294,   159,   161,
     295,     0,    43,   141,    52,   298,     0,     0,     0,     0,
     153,   153,    58,     0,     0,   738,   556,     0,   729,     0,
       0,     0,   554,     0,     0,   505,     0,    25,    37,   535,
     504,   543,     0,   534,    41,   542,     0,     0,    26,    30,
       0,   194,   202,   199,     0,     0,   704,   705,    11,   733,
       0,     0,     0,   666,   663,     0,   390,   703,   702,   701,
       0,   697,     0,   698,   700,     0,     5,   300,     0,     0,
     405,   406,   414,   413,     0,     0,   555,   385,   389,     0,
     715,     0,     0,   618,   602,   609,   647,     0,   722,   191,
     503,   557,   188,     0,   601,     0,     0,   125,   377,    99,
     359,     0,   394,   395,     0,   392,   555,   679,     0,   189,
     127,   125,   123,     0,   658,   311,     0,     0,   189,   606,
     607,   620,   643,   644,     0,     0,     0,   580,   563,   564,
     565,     0,     0,     0,    25,   572,   571,   586,   556,     0,
     594,   677,   676,     0,   655,   602,   611,   518,     0,   163,
       0,     0,    49,     0,     0,     0,     0,     0,   133,   134,
     145,     0,    43,   143,    74,   153,     0,   153,     0,     0,
     747,     0,   555,   739,   741,   728,   727,     0,   725,   536,
     537,   562,     0,   556,   554,     0,     0,   388,   554,     0,
     686,     0,     0,    76,    31,   198,     0,   712,    69,     0,
       0,   721,   126,   128,   204,     0,     0,   664,     0,   696,
       0,    16,     0,   124,   204,     0,     0,   382,     0,   716,
       0,     0,   604,   603,   725,     0,   192,    40,   178,     0,
     558,   600,   753,   601,   122,     0,   601,     0,   358,   683,
     682,   189,     0,     0,     0,   125,   101,   519,   605,   189,
       0,     0,   568,   569,   570,   573,   574,   584,     0,   556,
     580,     0,   567,   588,   580,   555,   591,   593,   595,     0,
     671,   605,     0,     0,     0,     0,   160,    54,     0,   298,
     135,   666,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   147,     0,   736,   737,     0,     0,   751,     0,   540,
     555,   553,     0,   545,     0,   556,     0,     0,   544,   690,
       0,   556,     0,    43,   200,   735,   732,     0,   268,   668,
     666,   302,   305,   309,     0,    14,   268,   417,     0,     0,
     419,   412,   415,     0,   410,     0,   717,     0,     0,   189,
     193,   730,   601,   177,   752,     0,     0,   204,     0,   601,
       0,     0,   640,   204,   204,     0,     0,   312,   189,     0,
     634,     0,   577,   555,   579,     0,   566,     0,     0,   556,
       0,   585,   675,     0,    43,     0,   156,   142,     0,     0,
     132,    72,   146,     0,     0,   149,     0,   154,   155,    43,
     148,   748,   726,     0,   561,   560,   538,     0,   555,   387,
     541,   539,     0,   393,   555,   685,     0,     0,     0,   129,
       0,     0,   266,     0,     0,     0,   108,   203,   205,     0,
     265,     0,   268,     0,   699,   112,   408,     0,     0,   381,
     605,   189,     0,     0,   400,   176,   753,     0,   180,   730,
     268,   730,     0,   681,   639,   268,   268,   204,   601,     0,
     633,   583,   582,   575,     0,   578,   555,   587,   576,    43,
     162,    50,    55,   136,     0,   144,   150,    43,   152,     0,
       0,   384,     0,   689,   688,     0,    90,   734,     0,     0,
     130,   233,   231,   505,    24,     0,   227,     0,   232,   243,
       0,   241,   246,     0,   245,     0,   244,     0,    95,   207,
       0,   209,     0,   665,   409,   407,   418,   416,   189,     0,
     637,   731,     0,     0,     0,   181,     0,     0,   104,   400,
     730,   110,   114,   268,     0,   635,     0,   590,     0,   158,
       0,    43,   139,    73,   151,   750,   559,     0,     0,     0,
      91,     0,     0,   217,   221,     0,     0,   214,   469,   468,
     465,   467,   466,   486,   488,   487,   457,   447,   463,   462,
     424,   434,   435,   437,   436,   456,   440,   438,   439,   441,
     442,   443,   444,   445,   446,   448,   449,   450,   451,   452,
     453,   455,   454,   425,   426,   427,   430,   431,   433,   471,
     472,   481,   480,   479,   478,   477,   476,   464,   483,   473,
     474,   475,   458,   459,   460,   461,   484,   485,   489,   491,
     490,   492,   493,   470,   495,   494,   428,   497,   499,   498,
     432,   502,   500,   501,   496,   429,   482,   423,   238,   420,
       0,   215,   259,   260,   258,   251,     0,   252,   216,   285,
       0,     0,     0,     0,    95,     0,   636,     0,    43,     0,
     184,     0,   183,   261,    43,    98,     0,     0,   106,   730,
     581,     0,    43,   137,    56,     0,   383,   687,    43,   288,
     131,     0,     0,   235,   228,     0,     0,     0,   240,   242,
       0,     0,   247,   254,   255,   253,     0,     0,   206,     0,
       0,     0,     0,   638,     0,   403,   558,     0,   185,     0,
     182,     0,    43,   100,     0,   589,     0,     0,     0,   218,
      32,     0,   219,   220,     0,     0,   234,   237,   421,   422,
       0,   229,   256,   257,   249,   250,   248,   286,   283,   210,
     208,   287,     0,   404,   557,     0,   376,     0,   187,   262,
       0,   102,     0,   140,    89,     0,   268,   236,   239,     0,
     601,   212,     0,   401,   399,   186,   378,   138,    93,   225,
       0,   267,   284,   166,     0,   558,   279,   601,   402,     0,
      92,    75,     0,     0,   224,   730,   601,   165,   280,   281,
     282,   753,   278,     0,     0,     0,   223,     0,   164,   279,
       0,   730,     0,   222,   263,    43,   211,   753,     0,   168,
       0,     0,     0,     0,   169,     0,   213,     0,   264,     0,
     172,     0,   171,    43,   173,     0,   170,     0,     0,   175,
      88,   174
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   103,   656,   463,   158,   239,   240,
     105,   106,   107,   108,   109,   110,   283,   482,   483,   407,
     211,  1120,   413,  1054,  1348,   783,  1130,  1361,   299,   159,
     484,   685,   827,   946,   485,   500,   702,   447,   700,   486,
     468,   701,   301,   255,   272,   116,   687,   659,   642,   792,
    1069,   873,   748,  1254,  1123,   594,   754,   412,   602,   756,
     979,   589,   739,   742,   864,  1354,  1355,   819,   820,   494,
     495,   244,   245,   249,   908,  1007,  1087,  1232,  1340,  1357,
    1261,  1302,  1303,  1304,  1075,  1076,  1077,  1262,  1268,  1311,
    1080,  1081,  1085,  1225,  1226,  1227,  1245,  1386,  1008,  1009,
     160,   118,  1371,  1372,  1230,  1011,   119,   205,   408,   409,
     120,   121,   122,   123,   124,   125,   126,   127,   684,   826,
     472,   473,   895,   474,   896,   128,   129,   130,   621,   131,
     132,  1103,  1286,   133,   469,  1095,   470,   805,   664,   924,
     921,  1218,  1219,   134,   135,   136,   199,   206,   286,   395,
     137,   771,   625,   138,   772,   389,   682,   773,   726,   845,
     847,   848,   849,   728,   958,   959,   729,   570,   380,   168,
     169,   139,   822,   363,   364,   675,   140,   200,   162,   142,
     143,   144,   145,   146,   147,   148,   530,   149,   202,   203,
     450,   191,   192,   533,   534,   900,   901,   264,   265,   650,
     150,   442,   151,   477,   152,   230,   256,   294,   422,   767,
    1024,   640,   605,   606,   607,   231,   232,   935
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1127
static const yytype_int16 yypact[] =
{
   -1127,   111, -1127, -1127,  3748,  9580,  9580,   -58,  9580,  9580,
    9580, -1127,  9580,  9580,  9580,  9580,  9580,  9580,  9580,  9580,
    9580,  9580,  9580,  9580,  2120,  2120,  7660,  9580,  2212,   -56,
     -50, -1127, -1127, -1127, -1127, -1127, -1127, -1127,  9580, -1127,
     -50,   -42,   -40,   -37,   -50,  7818,  1339,  7976, -1127,  1382,
    7344,   -35,  9580,  1151,   -20, -1127, -1127, -1127,   169,   228,
      18,   -22,    11,   177,   179, -1127,  1339,   182,   194, -1127,
   -1127, -1127, -1127, -1127,    25,   813, -1127, -1127,  1339,  8134,
   -1127, -1127, -1127, -1127, -1127, -1127,  1339, -1127,   274,   257,
    1339,  1339, -1127,  9580, -1127, -1127,  9580, -1127, -1127,   247,
     270,   333,   333, -1127,   426,   293,   -36, -1127,   280, -1127,
      56, -1127,   437, -1127, -1127, -1127,   305,   522, -1127, -1127,
     297,   299,   304,   306,   315,   337, 10263, -1127, -1127,   465,
   -1127,   476,   484, -1127,   119,   339,   393, -1127, -1127,  1545,
      -1,  2815,   135,   379,   137,   139,   381,    54, -1127,   124,
   -1127,   497, -1127, -1127, -1127,   428,   404,   438, -1127,   437,
     522, 10903,  2940, 10903,  9580, 10903, 10903, 10618,   545,  1339,
   -1127, -1127,   538, -1127, -1127, -1127, -1127, -1127, -1127, -1127,
   -1127, -1127, -1127,   682,   429, -1127,   450,   475,   475,  2120,
   10550,   415,   599, -1127,   428,   682,   429,   468,   481,   440,
     142, -1127,   504,   135,  8292, -1127, -1127,  9580,  6142,    63,
   10903,  7028, -1127,  9580,  9580,  1339, -1127, -1127, 10304,   449,
   -1127, 10345,  1382,  1382,   473, -1127,   459,   947,   625, -1127,
     630, -1127,  1339, -1127,   470, 10386, -1127, 10427,  1339,    64,
   -1127,   231, -1127,  1810,    65, -1127, -1127, -1127,   632,    69,
    2120,  2120,  2120,   482,   507, -1127, -1127,  1596,  7660,    38,
     411, -1127,  9738,  2120,   367, -1127,  1339, -1127,   331,   293,
     493, 10591, -1127, -1127, -1127,   592,   659,   584, 10903,   499,
   10903,   500,   667,  3906,  9580,   373,   512,   454,   373,    40,
     234, -1127,  1339,  1382,   503,  8450,  1382, -1127, -1127,   747,
   -1127, -1127, -1127, -1127,   437, -1127, -1127, -1127, -1127, -1127,
   -1127, -1127,  9580,  9580,  9580,  8632,  9580,  9580,  9580,  9580,
    9580,  9580,  9580,  9580,  9580,  9580,  9580,  9580,  9580,  9580,
    9580,  9580,  9580,  9580,  9580,  9580,  9580,  2212, -1127,  9580,
    9580,  9580,   629,  1339,  1339,   305,   596,   868,  7186,  9580,
    9580,  9580,  9580,  9580,  9580,  9580,  9580,  9580,  9580,  9580,
   -1127, -1127,   311, -1127,   145,  9580,  9580, -1127,  8450,  9580,
    9580,   247,   173,  1596,   521,   437,  8790, 10468, -1127,   526,
     695,   682,   532,    13,   629,   475,  8948, -1127,  9106, -1127,
     533,   205, -1127,   254,  8450, -1127,  1010, -1127,   238, -1127,
   -1127, 10509, -1127, -1127,  9580, -1127,   624,  6324,   702,   539,
   10796,   704,    51,   104, -1127, -1127, -1127, -1127, -1127,  1382,
     639,   544,   712, -1127,  2183, -1127, -1127, -1127,  4064, -1127,
     252,  1151, -1127,  1339,  9580,   475,   -20, -1127,  2183,   649,
   -1127,   475,    53,    59,   239,   546,  1339,   605,   550,   475,
      62,   552,  1185,  1339, -1127, -1127,   668,  2941,   -13, -1127,
   -1127, -1127,   293, -1127, -1127, -1127, -1127,  9580,   611,   572,
       6, -1127,   613,   730,   565,  1382,  1382,   734,    36,   687,
     123, -1127, -1127, -1127, -1127, -1127, -1127,  3133, -1127, -1127,
   -1127, -1127,    85,  2120,   567,   744, 10903,   745, -1127, -1127,
     644,  1137, 10943, 10980, 10618,  9580, 10862,  2988,  2680,  7236,
    7394,  3281,  7550,  7550,  7550,  7550,  2127,  2127,  2127,  2127,
    1062,  1062,   603,   603,   603,   538,   538,   538, -1127, 10903,
     582,   583, 10659,   588,   760,   -39,   595,   173, -1127, -1127,
   -1127,   437,  1320,  9580, -1127, -1127, 10618, 10618, 10618, 10618,
   10618, 10618, 10618, 10618, 10618, 10618, 10618, 10618,  9580,   -39,
     598,   589,  3238,   600,   601,  3346,    73,   604, -1127,   864,
   -1127,  1339, -1127,   499,    36,   429,  2120, 10903,  2120, 10700,
     223,   250, -1127,   608,  9580, -1127, -1127, -1127,  5960,    79,
   10903,   -50, -1127, -1127, -1127,  9580,  1066,  2183,  1339,  6506,
     610,   612, -1127,    88,   652, -1127,   777,   614,  1456,  1382,
    2183,  2183,  2183,   616,     7,   654,   617,   620,   -38, -1127,
     657, -1127,   621, -1127, -1127, -1127,   -10,  1339, -1127, -1127,
   10017, -1127, -1127,   788,  2120,   623, -1127, -1127, -1127,   715,
      92,  1615,   635,  1596,  1958,   789, -1127, -1127, -1127, -1127,
     628, -1127,  9580, -1127, -1127,  3432, -1127, 10903,  1615,   637,
   -1127, -1127, -1127, -1127,   803,  9580,   592, -1127, -1127,   640,
   -1127,  1382,  1101, -1127,   255, -1127, -1127,  1382, -1127,   475,
   -1127,  9264, -1127,  2183,    34,   645,  1615,   611, -1127, -1127,
    3011,  9580, -1127, -1127,  9580, -1127,  9580, -1127,   646,  8450,
     605,   611,   644,  1339,  2212,   475, 10058,   647,  8450, -1127,
   -1127,   261, -1127, -1127,   816,   425,   425,   864, -1127, -1127,
   -1127,   661,    44,   663,   664, -1127, -1127, -1127,   833,   669,
     526,   475,   475,  9422, -1127,   263, -1127, -1127, 10099,   421,
     -50,  7028, -1127,   670,  4222,   671,  2120,   674,   722,   475,
   -1127,   840, -1127, -1127, -1127, -1127,   509, -1127,   235,  1382,
   -1127,  1382,   639, -1127, -1127, -1127,   851,   684,   690, -1127,
   -1127,   741,   686,   865,  2183,   735,  1339,   592,  2183,  1339,
    2183,   699,   698, -1127, -1127, -1127,  2183,   475, -1127,  1382,
    1339, -1127,   871, -1127, -1127,    99,   711,   475,  7502, -1127,
    2437, -1127,  3590,   871, -1127,   -31,   190, 10903,   765, -1127,
     713,  9580,   -39,   714, -1127,  2120, 10903, -1127, -1127,   716,
     880, -1127,  1382,    34, -1127,   723,    34,   718,  3011, 10903,
   10755,  8450,   726,   731,   732,   611, -1127,   440,   729,  8450,
     736,  9580, -1127, -1127, -1127, -1127, -1127,   786,   737,   903,
     864,   791, -1127,   592,   864,   864, -1127, -1127, -1127,  2120,
   10903, -1127,   -50,   897,   856,  7028, -1127, -1127,   749,  9580,
     475,  1596,  1066,   753,  2183,  4380,   520,   759,  9580,    24,
     256, -1127,   774, -1127, -1127,  1491,   913, -1127,  2183, -1127,
    2183, -1127,   764, -1127,   819,   934,   768,   769, -1127,   824,
     771,   944,  1615, -1127, -1127, -1127,   873,  1615,   432, -1127,
    1596, -1127, -1127, 10618,   775, -1127,  1264, -1127,    49,  9580,
   -1127, -1127, -1127,  9580, -1127,  9580, -1127, 10140,   794,  8450,
     475,   957,   130, -1127, -1127,    83,   808, -1127,   809,    34,
    9580,   810, -1127, -1127, -1127,   792,   814, -1127,  8450,   818,
   -1127,   864, -1127,   864, -1127,   821, -1127,   877,   826,   982,
     827, -1127,   475,   969, -1127,   828, -1127, -1127,   830,   101,
   -1127, -1127, -1127,   832,   834, -1127,  2721, -1127, -1127, -1127,
   -1127, -1127, -1127,  1382, -1127,   882, -1127,  2183,   592, -1127,
   -1127, -1127,  2183, -1127,  2183, -1127,   937,  4538,  1382, -1127,
    1382,  1615, -1127,  1750,   870,  1030, -1127, -1127, -1127,   596,
     807,    70,   868,   102, -1127, -1127,   875, 10181, 10222, 10903,
     848,  8450,   849,  1382,   920, -1127,  1382,   953,  1014,   957,
    1604,   957,   853, 10903, -1127,  1686,  1700, -1127,    34,   855,
   -1127, -1127,   910, -1127,   864, -1127,   592, -1127, -1127, -1127,
    5960, -1127, -1127, -1127,  6688, -1127, -1127, -1127,  5960,   857,
    2183, -1127,   914, -1127,   915,   859, -1127, -1127,  1026,    47,
   -1127, -1127, -1127,    75,   866,    80, -1127,  9866, -1127, -1127,
      81, -1127, -1127,  1274, -1127,   874, -1127,   970,   437, -1127,
    1382, -1127,   596, -1127, -1127, -1127, -1127, -1127,  8450,   872,
   -1127, -1127,   867,   869,   110,  1040,  2183,   323, -1127,   920,
     957, -1127, -1127,  1866,   879, -1127,   864, -1127,   941,  5960,
    6870, -1127, -1127, -1127,  5960, -1127, -1127,  2183,  2183,   878,
   -1127,  2183,  1615, -1127, -1127,  2532,  1750, -1127, -1127, -1127,
   -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127,
   -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127,
   -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127,
   -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127,
   -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127,
   -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127,
   -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127,
   -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127,   456, -1127,
     870, -1127, -1127, -1127, -1127, -1127,    39,   530, -1127,  1052,
      86,  1339,   970,  1053,   437,   891, -1127,   288, -1127,   996,
    1057,  2183, -1127, -1127, -1127, -1127,   896,   323, -1127,   957,
   -1127,   864, -1127, -1127, -1127,  4696, -1127, -1127, -1127, -1127,
   -1127,   372,    43, -1127, -1127,  2183,  9866,  9866,  1024, -1127,
    1274,  1274,   597, -1127, -1127, -1127,  2183,  1003, -1127,   902,
      93,  2183,  1339, -1127,  1009, -1127,  1076,  4854,  1072,  2183,
   -1127,  5012, -1127, -1127,   323, -1127,  5170,   912,  5328, -1127,
     997,   950, -1127, -1127,  1002,  2532, -1127, -1127, -1127, -1127,
     961, -1127,  1065, -1127, -1127, -1127, -1127, -1127,  1090, -1127,
   -1127, -1127,   942, -1127,   316,   948, -1127,  2183, -1127, -1127,
    5486, -1127,   946, -1127, -1127,  1339,   868, -1127, -1127,  2183,
     132, -1127,  1045, -1127, -1127, -1127, -1127, -1127,    26,   964,
    1339,  1368, -1127, -1127,   951,  1120,   549,   132, -1127,   956,
   -1127, -1127,  1615,   959, -1127,   957,   134, -1127, -1127, -1127,
   -1127,  1382, -1127,   966,  1615,    97, -1127,   352, -1127,   549,
     322,   957,  1060, -1127, -1127, -1127, -1127,  1382,  1067,  1130,
     352,   971,  5644,   340,  1135,  2183, -1127,   972, -1127,  1078,
    1136,  2183, -1127, -1127,  1140,  2183, -1127,  5802,  2183, -1127,
   -1127, -1127
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1127, -1127, -1127,  -417, -1127, -1127, -1127,    -4, -1127,   725,
     -12,   943,  1818, -1127,  1401, -1127,  -230, -1127,     5, -1127,
   -1127, -1127, -1127, -1127, -1127,  -194, -1127, -1127,  -156,    52,
       4, -1127, -1127, -1127,     9, -1127, -1127, -1127, -1127,    12,
   -1127, -1127,   812,   817,   820,  1028,   466,  -550,   469,   517,
    -182, -1127,   313, -1127, -1127, -1127, -1127, -1127, -1127,  -474,
     211, -1127, -1127, -1127, -1127,  -166, -1127,  -769, -1127,  -329,
   -1127, -1127,   756, -1127,  -735, -1127, -1127, -1127, -1127, -1127,
   -1127, -1127, -1127, -1127, -1127,    60, -1127, -1127, -1127, -1127,
   -1127,   -23, -1127,   193,  -710, -1127, -1126,  -191, -1127,  -135,
      22,  -116,  -177, -1127,   -19, -1127,   -63,   -25,  1171,  -555,
    -320, -1127, -1127,   -48, -1127, -1127,  2531,  1124, -1127, -1127,
    -623, -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127, -1127,
   -1127,   107, -1127,   416, -1127, -1127, -1127, -1127, -1127, -1127,
   -1127, -1127,  -703, -1127,  1246,    -7,  -307, -1127, -1127,   386,
     787,  2021, -1127, -1127,  -419,  -342,  -793, -1127, -1127,   508,
    -546,  -487, -1127, -1127, -1127, -1127, -1127,   496, -1127, -1127,
   -1127,  -636,  -896,  -153,  -144,  -113, -1127, -1127,    10, -1127,
   -1127, -1127, -1127,    -6,  -125, -1127,   150, -1127, -1127, -1127,
    -353,   973, -1127, -1127, -1127, -1127, -1127,   561,   472, -1127,
   -1127,   978, -1127, -1127, -1127,  -286,   -79,  -171,  -262, -1127,
    -955, -1127,   471, -1127, -1127, -1127,  -143,  -994
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -724
static const yytype_int16 yytable[] =
{
     104,   305,   234,   374,   346,   208,   428,   273,   113,   111,
     497,   276,   277,   114,   141,   212,   115,   186,   186,   216,
     566,   198,   201,   727,   372,   392,   117,   933,   544,   367,
     528,   492,  1104,   279,   187,   188,  1026,   302,   572,   563,
     745,   241,   219,   808,   305,   228,   655,  1270,   821,   397,
     977,  1305,   661,   242,   936,  1132,   112,   938,   398,   598,
    -625,   634,   254,   268,   296,   583,   269,   634,  1271,   916,
     644,   404,   431,   436,  1107,    11,  1109,   439,  1090,   417,
     418,   644,   254,  -230,   423,   775,   254,   254,  1136,  1220,
     399,   248,   781,   677,  1277,  1016,   759,   365,   259,   452,
     790,  1277,  -723,   260,  -723,  1132,   282,   644,   362,   644,
     644,     3,   254,   259,  1027,   369,   917,   164,   459,   204,
     382,  1293,   851,   293,   293,   207,   293,   758,  1359,   918,
     600,   667,   390,   213,   362,   214,  -723,   825,   215,  -723,
    -723,  1239,  -723,   236,   261,   740,   741,   919,   501,   782,
     423,   834,   662,   250,   894,  1247,  1028,   672,  -507,   453,
     243,   347,   261,   262,   263,   379,   654,   663,  1331,   304,
    1032,    11,   588,    11,  -631,    11,   186,  -625,   262,   263,
    -622,   852,   186,  1240,  -629,   782,   251,   821,   186,   480,
     821,   818,   697,   383,  -508,   274,   365,    35,  -623,   385,
    -624,   978,  1030,  -659,   104,   391,  -626,   104,  1035,  1036,
    -179,   411,   375,   403,   366,  1272,   406,   537,   141,   567,
     499,   141,  1306,   489,  1133,  1134,   430,   599,   425,   635,
     957,   305,   370,   297,   369,   636,   186,   537,   645,   802,
     405,   432,   437,   186,   186,   186,   440,  1091,   678,   714,
     186,   760,  -230,   435,   462,   791,   186,  1137,  1221,   537,
     441,   441,   444,  1278,   763,   273,   302,   449,   537,  1114,
    1320,   537,  1012,   458,  1383,   909,   603,  1053,  1093,   104,
    1012,   876,   601,   880,   365,   945,   246,  1025,   491,  1353,
     795,  1378,   228,   141,  1294,   254,   821,  -622,  -632,  -660,
     545,  -629,  1113,   821,   371,   117,  -557,   259,  -167,   961,
    -557,  -662,   459,   366,   968,  -623,  -627,  -624,   573,  1284,
    -659,   433,  -628,  -626,  -661,   673,   362,    35,   878,   879,
     198,   201,   669,   670,   674,   112,   536,   922,   535,   254,
     254,   254,   627,   259,   396,   247,   766,  1342,   285,   878,
     879,   370,   252,  1388,   253,   892,   560,   257,   559,   897,
     362,  1285,   744,   955,   698,  1062,   186,   960,   923,   258,
     832,  1399,   262,   263,   186,   292,    33,  1380,   536,   840,
     575,   581,    33,   449,    35,   703,   856,   582,   707,  1343,
     586,   383,   585,  1393,  1012,  1389,   292,   837,   541,  1012,
    1012,   366,   821,   104,   698,  1041,   259,  1042,   262,   263,
    1377,   288,   593,  1400,   881,   637,  -660,   141,   490,   241,
     274,   673,   689,  1118,   104,   284,  1390,   734,  -662,   629,
     674,   891,   275,  -627,    99,   980,   735,   292,   141,  -628,
     259,  -661,   639,    33,  1356,   459,   259,   392,   649,   651,
     117,   300,   291,    81,    82,   295,    83,    84,    85,    81,
      82,  1356,    83,    84,    85,   423,   768,   736,   298,  1265,
    1379,   262,   263,    11,   306,   292,   307,  1012,   911,    95,
     112,   308,  1266,   309,   259,    95,   186,   862,   863,   558,
     531,    99,   310,  1325,   842,   843,    33,   254,  1117,  1267,
    1243,  1244,   941,   679,   460,   262,   263,   954,   464,   465,
     949,   262,   263,   157,   311,   561,    78,   341,   969,   564,
      81,    82,   875,    83,    84,    85,  -396,   259,   810,  1384,
    1385,  1000,   459,   342,   814,   186,  1001,   339,    55,    56,
      57,   153,   154,   303,  1002,   340,    95,   537,   454,   262,
     263,  1299,   705,   989,   368,  1273,  -630,  1013,  -397,   995,
    1312,  1313,  1367,  1308,  1309,   725,   743,   730,  -507,   186,
    1250,   186,  1274,    81,    82,  1275,    83,    84,    85,   373,
    1003,  1004,   266,  1005,   104,   378,   731,   337,   732,   186,
     384,   293,   262,   263,   751,   104,   362,   387,   141,    95,
    1022,   877,   878,   879,   753,    94,   749,   388,  -506,   141,
     117,  1006,   974,   878,   879,   394,   882,  1047,   883,  1039,
      48,   393,  1314,   784,   836,   396,   415,   186,    55,    56,
      57,   153,   154,   303,   420,   419,   186,   186,  -718,  1315,
     112,   928,  1316,   424,   787,   438,   905,   426,   334,   335,
     336,   104,   337,   449,   797,  1368,  1369,  1370,   445,   113,
     111,   287,   289,   290,   114,   141,   813,   115,   812,   446,
     466,   471,   475,   997,   476,   478,   479,   117,   -38,   934,
     376,   349,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,  1099,   488,    48,    94,   498,   198,   201,   254,
      33,   569,    35,   571,   821,  1295,   591,   112,   574,   580,
     404,   844,   844,   725,  1068,   865,   595,   597,   604,   608,
     609,   821,   633,   638,   641,   643,   360,   361,   646,   652,
     821,   658,   455,   660,  1050,   665,   461,   104,   666,   186,
     104,   668,   814,   680,    27,    28,   866,   671,  -398,  1058,
     912,   141,   681,    33,   141,    35,   870,   455,   683,   461,
     455,   461,   461,   686,   692,   693,   117,   695,   696,  1235,
     699,   709,   893,   708,   761,   898,   711,    81,    82,   688,
      83,    84,    85,   712,   737,   762,   906,   755,   362,   757,
     764,   774,   777,   182,   776,   778,   112,   779,   104,   780,
     788,   786,   798,    95,  1233,   789,   113,   111,   186,    99,
     799,   114,   141,   794,   115,   804,   806,   809,    33,  1119,
     823,   831,   839,   157,   117,   930,    78,  1124,    80,   841,
      81,    82,   222,    83,    84,    85,   850,   963,   853,   854,
    1059,   855,    88,   480,   872,   857,   725,   867,   869,   871,
     725,   725,   186,   874,   112,  1067,    95,   381,   223,   885,
     886,   104,    99,   888,   186,   186,   887,  1089,   889,   962,
     966,   104,   454,   890,   902,   141,   903,  1092,    33,   907,
    1101,   449,   749,   934,    33,   141,   910,   925,   932,   929,
     926,  1255,   931,   939,   305,    81,    82,   117,    83,    84,
      85,   937,   942,   186,   948,  -267,   715,   716,   951,   943,
     944,   953,   950,    55,    56,    57,   153,   154,   303,   952,
     449,    95,   498,   964,   965,   717,   967,   112,   956,   971,
    1010,   224,  1231,   718,   719,    33,   975,   981,  1010,   983,
     986,   987,   988,   720,   990,   991,   992,   725,   157,   725,
     993,    78,   994,   225,  1014,    81,    82,   266,    83,    84,
      85,    81,    82,   998,    83,    84,    85,   184,   184,  1021,
    1037,   196,   222,   226,    55,    56,    57,   153,   154,   303,
      94,    95,   227,  1023,  1029,  1031,  1034,    95,   721,  1038,
    1046,   267,   196,   104,  1040,  1049,   228,  1043,   223,  1044,
     722,  1084,  1045,  1048,  1060,  1051,  1052,   141,  1287,  1055,
    1065,  1056,    81,    82,  1291,    83,    84,    85,    33,   117,
    1079,  1094,  1296,  1098,  1102,  1100,  1105,  1106,  1298,  1110,
     723,  1115,  1116,  1125,   347,  1129,  1127,  1128,   724,  1131,
     725,    94,  1237,  1229,  1135,   421,   104,  1238,  1236,   112,
     104,  1228,  1010,  1241,   104,  1249,  1258,  1010,  1010,  1122,
     141,  1088,  1330,  1251,   141,  1276,  1281,  1283,   141,  1288,
    1289,   224,   117,  1217,  1292,  1310,  1318,  1319,  1282,  1224,
     117,    33,  1323,    35,  1324,  1327,   228,  -226,   157,  1333,
    1335,    78,  1336,   225,  1271,    81,    82,   746,    83,    84,
      85,    33,   112,  1339,   331,   332,   333,   334,   335,   336,
     112,   337,   725,   226,  1338,   104,   104,  1341,  1358,  1362,
     104,    95,   227,  1347,  1344,  1253,   184,  1365,  1366,   141,
     141,  1374,   184,  1391,   141,  1010,  1376,    33,   184,    35,
    1394,   117,  1381,  1395,  1234,  1082,   117,  1397,  1401,  1405,
    1403,  1404,  1279,  1408,  1360,  1392,   628,   540,    81,    82,
     538,    83,    84,    85,   539,   196,   196,   345,   835,   833,
     196,   112,    33,  1407,    35,   803,   112,   182,    81,    82,
    1375,    83,    84,    85,    95,   970,   184,  1057,   584,   747,
      99,  1373,   631,   184,   184,   184,  1264,  1269,  1086,  1396,
     184,  1350,  1387,  1322,    95,  1083,   184,   157,    33,   209,
      78,   623,    80,  1280,    81,    82,  1246,    83,    84,    85,
     281,   920,    33,   947,   846,   623,   858,   254,   934,   443,
       0,   451,     0,   884,     0,   305,   196,     0,     0,   196,
      95,   183,     0,     0,   934,     0,    99,   725,     0,    81,
      82,   104,    83,    84,    85,     0,    33,  1300,   647,   648,
       0,     0,  1217,  1217,     0,   141,  1224,  1224,     0,     0,
     185,   185,     0,     0,   197,    95,     0,   117,   254,   811,
     196,    99,     0,   104,     0,    81,    82,   104,    83,    84,
      85,     0,   104,     0,   104,   238,     0,   141,     0,    81,
      82,   141,    83,    84,    85,    11,   141,   112,   141,   117,
       0,    95,   688,   117,     0,     0,   184,     0,   117,     0,
     117,     0,     0,     0,   184,    95,   104,     0,     0,     0,
       0,  1349,     0,    81,    82,     0,    83,    84,    85,   112,
     141,     0,     0,   112,     0,    33,  1363,     0,   112,     0,
     112,     0,   117,     0,     0,     0,     0,     0,  1351,    95,
       0,     0,   196,  1000,     0,     0,     0,   618,  1001,     0,
      55,    56,    57,   153,   154,   303,  1002,     0,     0,     0,
       0,   618,   112,   704,   623,     0,     0,     0,   104,     0,
       0,    33,     0,    35,     0,     0,     0,   623,   623,   623,
       0,     0,   141,   104,     0,     0,     0,   222,     0,     0,
      33,     0,  1003,  1004,   117,  1005,     0,   141,   196,   196,
    1222,     0,    81,    82,  1223,    83,    84,    85,     0,   117,
       0,   182,     0,   223,     0,   185,   184,    94,     0,     0,
       0,     0,     0,  1015,   112,     0,     0,     0,    95,  1083,
     229,     0,     0,    33,     0,     0,     0,     0,     0,   112,
       0,   157,     0,     0,    78,     0,    80,     0,    81,    82,
     623,    83,    84,    85,    55,    56,    57,   153,   154,   303,
       0,   222,     0,     0,     0,   184,     0,    81,    82,   185,
      83,    84,    85,     0,    95,   183,   185,   185,   185,     0,
      99,     0,     0,   185,     0,     0,   224,   223,     0,   185,
       0,     0,     0,    95,     0,     0,   222,     0,     0,   184,
       0,   184,     0,   157,     0,     0,    78,    33,   225,     0,
      81,    82,     0,    83,    84,    85,     0,     0,     0,   184,
     618,    94,   223,     0,     0,  1364,     0,     0,   226,     0,
       0,   196,   196,   618,   618,   618,    95,   227,     0,     0,
       0,   623,    33,     0,     0,   623,     0,   623,     0,     0,
       0,     0,     0,   623,     0,     0,     0,   184,     0,     0,
     224,     0,     0,   197,   196,     0,   184,   184,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   157,     0,     0,
      78,   196,   225,     0,    81,    82,     0,    83,    84,    85,
       0,     0,     0,   765,   196,   224,     0,     0,     0,   185,
     196,     0,   226,   229,   229,     0,   618,     0,   229,   196,
      95,   227,   157,     0,     0,    78,     0,   225,     0,    81,
      82,     0,    83,    84,    85,    11,     0,   196,   982,     0,
       0,    55,    56,    57,    58,    59,   303,   226,     0,     0,
       0,   623,    65,   343,     0,    95,   227,    33,     0,    35,
     622,     0,     0,     0,     0,   623,     0,   623,     0,     0,
       0,     0,     0,     0,   622,     0,    33,     0,     0,   184,
       0,     0,     0,     0,   229,     0,     0,   229,     0,   344,
       0,     0,   196,  1000,   196,     0,     0,   182,  1001,     0,
      55,    56,    57,   153,   154,   303,  1002,   618,    94,   448,
       0,   618,     0,   618,     0,     0,     0,    11,     0,   618,
       0,     0,   196,     0,     0,     0,     0,   157,     0,   185,
      78,    11,    80,     0,    81,    82,     0,    83,    84,    85,
       0,     0,  1003,  1004,     0,  1005,   157,     0,   184,    78,
       0,    80,     0,    81,    82,   196,    83,    84,    85,     0,
      95,   183,     0,     0,   623,     0,    99,    94,     0,   623,
       0,   623,     0,  1108,     0,  1000,     0,     0,   185,    95,
    1001,     0,    55,    56,    57,   153,   154,   303,  1002,  1000,
       0,     0,   184,     0,  1001,     0,    55,    56,    57,   153,
     154,   303,  1002,     0,   184,   184,     0,   618,     0,     0,
     229,    33,   185,     0,   185,   620,     0,     0,   196,     0,
       0,   618,     0,   618,  1003,  1004,     0,  1005,     0,   620,
       0,     0,   185,   622,     0,   196,     0,   623,  1003,  1004,
     196,  1005,     0,   184,     0,     0,   622,   622,   622,    94,
       0,     0,  1071,     0,     0,  1111,     0,     0,     0,     0,
       0,     0,     0,    94,  1072,     0,   229,   229,     0,  1112,
     185,    33,     0,    35,     0,     0,     0,   793,     0,   185,
     185,   157,     0,   623,    78,     0,  1073,     0,    81,    82,
       0,    83,  1074,    85,   793,     0,     0,    11,     0,     0,
       0,     0,     0,     0,   623,   623,     0,     0,   623,     0,
       0,   182,  1263,     0,    95,     0,   196,     0,     0,   622,
     618,     0,   824,     0,     0,   618,     0,   618,     0,     0,
       0,   196,     0,   196,   196,     0,   196,     0,     0,     0,
     197,   157,     0,   196,    78,     0,    80,     0,    81,    82,
       0,    83,    84,    85,     0,  1000,   196,     0,     0,   196,
    1001,     0,    55,    56,    57,   153,   154,   303,  1002,     0,
       0,     0,     0,     0,    95,   183,     0,     0,   434,     0,
      99,     0,   185,     0,     0,     0,     0,     0,   620,     0,
       0,     0,     0,   618,     0,     0,     0,     0,     0,   229,
     229,   620,   620,   620,  1003,  1004,     0,  1005,     0,     0,
     622,     0,     0,     0,   622,     0,   622,     0,   623,    33,
       0,    35,   622,   196,     0,     0,     0,     0,     0,    94,
       0,     0,     0,     0,     0,  1248,     0,     0,     0,   618,
       0,     0,   623,     0,     0,     0,     0,     0,     0,     0,
       0,   185,     0,   623,     0,     0,     0,     0,   623,   182,
     618,   618,   229,     0,   618,   196,   623,     0,   229,   196,
       0,   796,     0,     0,   620,     0,     0,     0,     0,     0,
       0,     0,  1337,     0,     0,     0,     0,     0,     0,   157,
       0,     0,    78,     0,    80,   185,    81,    82,     0,    83,
      84,    85,     0,     0,   623,     0,     0,   185,   185,     0,
     622,     0,     0,     0,     0,     0,   623,     0,     0,     0,
       0,     0,    95,   183,   622,     0,   622,     0,    99,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   996,     0,
       0,     0,     0,   999,     0,     0,   185,     0,     0,     0,
     229,     0,   229,  -724,  -724,  -724,  -724,   329,   330,   331,
     332,   333,   334,   335,   336,   620,   337,     0,     0,   620,
       0,   620,   623,     0,   618,     0,     0,   620,   623,     0,
     229,    33,   623,    35,     0,   623,     0,     0,     0,     0,
       0,     0,     0,     0,   196,     0,     0,     0,   618,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   618,
       0,     0,     0,   229,   618,   610,   611,     0,     0,     0,
       0,   182,   618,   622,     0,     0,     0,     0,   622,     0,
     622,     0,   619,     0,   612,     0,     0,  1070,     0,  1078,
       0,     0,    31,    32,    33,     0,   619,     0,     0,     0,
       0,   157,    37,     0,    78,     0,    80,     0,    81,    82,
     618,    83,    84,    85,     0,   620,     0,     0,     0,     0,
       0,     0,   618,    33,     0,    35,   229,     0,     0,   620,
       0,   620,     0,     0,    95,   183,     0,     0,     0,     0,
      99,     0,     0,     0,     0,   196,   622,   613,    69,    70,
      71,    72,    73,     0,   196,     0,     0,   196,     0,   614,
       0,     0,     0,   194,   157,    76,    77,    78,     0,   615,
     196,    81,    82,     0,    83,    84,    85,     0,   618,    87,
       0,     0,     0,     0,   618,     0,     0,     0,   618,   616,
       0,   618,   622,   157,    92,     0,    78,   617,    80,     0,
      81,    82,     0,    83,    84,    85,     0,     0,     0,     0,
       0,     0,     0,   622,   622,     0,     0,   622,  1260,     0,
       0,     0,  1078,     0,   229,     0,    95,   195,   620,     0,
       0,     0,    99,   620,     0,   620,     0,     0,     0,   229,
       0,   229,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   229,     0,     0,     0,   619,     0,     0,     0,     0,
       0,     0,     0,     0,   229,     0,     0,   229,   619,   619,
     619,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   624,   312,   313,   314,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   632,
       0,   620,   315,     0,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,     0,   337,   622,     0,     0,
       0,   229,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   619,     0,     0,     0,     0,     0,   620,     0,     0,
       0,   622,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   622,     0,     0,     0,     0,   622,   620,   620,
       0,     0,   620,     0,     0,   622,   161,   163,     0,   165,
     166,   167,     0,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,     0,     0,   190,   193,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   210,
       0,     0,     0,   622,     0,     0,   218,     0,   221,     0,
       0,   235,     0,   237,     0,   622,     0,     0,     0,     0,
       0,     0,   619,     0,     0,     0,   619,     0,   619,     0,
       0,    31,    32,     0,   619,     0,     0,     0,  1070,     0,
     271,    37,     0,     0,     0,     0,     0,     0,   750,   914,
    1382,     0,     0,     0,   278,     0,     0,   280,     0,     0,
       0,   769,   770,     0,     0,     0,     0,     0,     0,     0,
       0,   622,   620,     0,     0,     0,     0,   622,     0,     0,
       0,   622,     0,     0,   622,     0,     0,    69,    70,    71,
      72,    73,  1301,     0,     0,     0,   620,     0,   614,     0,
       0,     0,     0,     0,    76,    77,     0,   620,     0,     0,
       0,     0,   620,     0,     0,     0,     0,     0,    87,     0,
     620,     0,   619,     0,     0,   377,     0,     0,     0,     0,
       0,     0,     0,    92,   817,     0,   619,     0,   619,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   620,   337,
     312,   313,   314,     0,     0,   401,     0,     0,   401,     0,
     620,     0,     0,     0,   210,   410,   315,   977,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,     0,
     337,     0,   229,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   229,   190,
       0,     0,     0,   457,     0,     0,   620,     0,     0,     0,
       0,   899,   620,     0,     0,   619,   620,   904,     0,   620,
     619,     0,   619,     0,     0,   487,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   496,     0,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
       0,     0,     0,   502,   503,   504,   506,   507,   508,   509,
     510,   511,   512,   513,   514,   515,   516,   517,   518,   519,
     520,   521,   522,   523,   524,   525,   526,   527,     0,     0,
     529,   529,   532,     0,   360,   361,     0,     0,   619,   546,
     547,   548,   549,   550,   551,   552,   553,   554,   555,   556,
     557,     0,     0,     0,     0,   972,   529,   562,   978,   496,
     529,   565,     0,     0,     0,     0,     0,   546,     0,   984,
       0,   985,     0,     0,     0,     0,     0,   577,     0,   579,
       0,     0,     0,     0,   619,   496,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   590,   362,     0,     0,     0,
       0,     0,     0,     0,     0,   619,   619,     0,     0,   619,
     312,   313,   314,   376,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   630,   315,     0,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,     0,
     337,     0,     0,     0,     0,     0,     0,     0,   657,   360,
     361,     0,     0,     0,     0,     0,     0,     0,  1061,     0,
       0,     0,     0,  1063,     0,  1064,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,   336,   690,   337,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,   619,
     337,   362,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   271,     0,     0,     0,     0,     0,
       0,  1126,     0,   619,     0,     0,     0,     0,     0,   706,
       0,     0,     0,     0,   619,     0,     0,     0,     0,   619,
       0,     0,     0,     0,     0,     0,     0,   619,     0,     0,
       0,     0,     0,     0,     0,   738,     0,     0,     0,     0,
     653,     0,     0,     0,     0,     0,   210,  1242,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   312,   313,   314,   619,     0,     0,  1256,  1257,
       0,     0,  1259,     0,     0,     0,     0,   619,   315,     0,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,     0,   337,   800,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   807,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   816,   619,     0,     0,     0,     0,     0,   619,
       0,     0,   828,   619,     0,   829,   619,   830,     0,     0,
     496,     0,     0,     0,     0,     0,     0,     0,     0,   496,
       0,     0,     0,     0,     0,     0,     0,   312,   313,   314,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1290,   315,   860,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,   336,  1307,   337,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1317,     0,     0,
       0,     0,  1321,     0,     0,     0,     0,     0,     0,     0,
    1328,     0,   676,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,   913,
     337,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   927,     0,     0,     0,     0,     0,  1345,     0,
       0,     0,     0,     0,     0,   312,   313,   314,     0,     0,
    1352,     0,   496,     0,     0,     0,     0,     0,     0,     0,
     496,   315,   913,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,     0,   337,     0,     0,     0,     0,
     210,     0,     0,     0,     0,     0,     0,     0,     0,   976,
       0,     0,     0,     0,     0,     0,  1402,   710,     0,     0,
       0,     0,  1406,     0,     0,     0,  1409,     0,     0,  1411,
       0,     0,     0,     0,     0,     5,     6,     7,     8,     9,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
    1017,     0,     0,     0,  1018,     0,  1019,     0,     0,     0,
     496,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1033,     0,    11,    12,    13,     0,     0,     0,   496,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,     0,    41,     0,    42,     0,
      43,     0,     0,    44,     0,   713,     0,    45,    46,    47,
      48,    49,    50,    51,     0,    52,    53,    54,    55,    56,
      57,    58,    59,    60,     0,    61,    62,    63,    64,    65,
      66,     0,   496,     0,     0,    67,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,    74,     0,
       0,     0,     0,    75,    76,    77,    78,    79,    80,     0,
      81,    82,     0,    83,    84,    85,    86,     0,    87,     0,
       0,     0,    88,     5,     6,     7,     8,     9,    89,    90,
       0,    91,    10,    92,    93,    94,    95,    96,     0,    97,
      98,   801,    99,   100,     0,   101,   102,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   496,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,     0,    41,     0,    42,     0,    43,     0,
       0,    44,     0,     0,     0,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,    55,    56,    57,    58,
      59,    60,     0,    61,    62,    63,    64,    65,    66,     0,
       0,     0,     0,    67,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,    75,    76,    77,    78,    79,    80,     0,    81,    82,
       0,    83,    84,    85,    86,     0,    87,     0,     0,     0,
      88,     5,     6,     7,     8,     9,    89,    90,     0,    91,
      10,    92,    93,    94,    95,    96,     0,    97,    98,   915,
      99,   100,     0,   101,   102,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,     0,    41,     0,    42,     0,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,    49,    50,    51,
       0,    52,    53,    54,    55,    56,    57,    58,    59,    60,
       0,    61,    62,    63,    64,    65,    66,     0,     0,     0,
       0,    67,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,    74,     0,     0,     0,     0,    75,
      76,    77,    78,    79,    80,     0,    81,    82,     0,    83,
      84,    85,    86,     0,    87,     0,     0,     0,    88,     5,
       6,     7,     8,     9,    89,    90,     0,    91,    10,    92,
      93,    94,    95,    96,     0,    97,    98,     0,    99,   100,
       0,   101,   102,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
       0,    36,     0,     0,     0,    37,    38,    39,    40,     0,
      41,     0,    42,     0,    43,     0,     0,    44,     0,     0,
       0,    45,    46,    47,    48,     0,    50,    51,     0,    52,
       0,    54,    55,    56,    57,    58,    59,    60,     0,    61,
      62,    63,     0,    65,    66,     0,     0,     0,     0,    67,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,    74,     0,     0,     0,     0,   157,    76,    77,
      78,    79,    80,     0,    81,    82,     0,    83,    84,    85,
      86,     0,    87,     0,     0,     0,    88,     5,     6,     7,
       8,     9,    89,     0,     0,     0,    10,    92,    93,    94,
      95,    96,     0,    97,    98,   481,    99,   100,     0,   101,
     102,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,     0,    36,
       0,     0,     0,    37,    38,    39,    40,     0,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,     0,    50,    51,     0,    52,     0,    54,
      55,    56,    57,    58,    59,    60,     0,    61,    62,    63,
       0,    65,    66,     0,     0,     0,     0,    67,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
      74,     0,     0,     0,     0,   157,    76,    77,    78,    79,
      80,     0,    81,    82,     0,    83,    84,    85,    86,     0,
      87,     0,     0,     0,    88,     5,     6,     7,     8,     9,
      89,     0,     0,     0,    10,    92,    93,    94,    95,    96,
       0,    97,    98,   626,    99,   100,     0,   101,   102,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,   868,    41,     0,    42,     0,
      43,     0,     0,    44,     0,     0,     0,    45,    46,    47,
      48,     0,    50,    51,     0,    52,     0,    54,    55,    56,
      57,    58,    59,    60,     0,    61,    62,    63,     0,    65,
      66,     0,     0,     0,     0,    67,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,    74,     0,
       0,     0,     0,   157,    76,    77,    78,    79,    80,     0,
      81,    82,     0,    83,    84,    85,    86,     0,    87,     0,
       0,     0,    88,     5,     6,     7,     8,     9,    89,     0,
       0,     0,    10,    92,    93,    94,    95,    96,     0,    97,
      98,     0,    99,   100,     0,   101,   102,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,     0,    41,     0,    42,     0,    43,   973,
       0,    44,     0,     0,     0,    45,    46,    47,    48,     0,
      50,    51,     0,    52,     0,    54,    55,    56,    57,    58,
      59,    60,     0,    61,    62,    63,     0,    65,    66,     0,
       0,     0,     0,    67,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,   157,    76,    77,    78,    79,    80,     0,    81,    82,
       0,    83,    84,    85,    86,     0,    87,     0,     0,     0,
      88,     5,     6,     7,     8,     9,    89,     0,     0,     0,
      10,    92,    93,    94,    95,    96,     0,    97,    98,     0,
      99,   100,     0,   101,   102,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,     0,    41,     0,    42,     0,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,     0,    50,    51,
       0,    52,     0,    54,    55,    56,    57,    58,    59,    60,
       0,    61,    62,    63,     0,    65,    66,     0,     0,     0,
       0,    67,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,    74,     0,     0,     0,     0,   157,
      76,    77,    78,    79,    80,     0,    81,    82,     0,    83,
      84,    85,    86,     0,    87,     0,     0,     0,    88,     5,
       6,     7,     8,     9,    89,     0,     0,     0,    10,    92,
      93,    94,    95,    96,     0,    97,    98,  1066,    99,   100,
       0,   101,   102,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
       0,    36,     0,     0,     0,    37,    38,    39,    40,     0,
      41,     0,    42,  1297,    43,     0,     0,    44,     0,     0,
       0,    45,    46,    47,    48,     0,    50,    51,     0,    52,
       0,    54,    55,    56,    57,    58,    59,    60,     0,    61,
      62,    63,     0,    65,    66,     0,     0,     0,     0,    67,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,    74,     0,     0,     0,     0,   157,    76,    77,
      78,    79,    80,     0,    81,    82,     0,    83,    84,    85,
      86,     0,    87,     0,     0,     0,    88,     5,     6,     7,
       8,     9,    89,     0,     0,     0,    10,    92,    93,    94,
      95,    96,     0,    97,    98,     0,    99,   100,     0,   101,
     102,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,     0,    36,
       0,     0,     0,    37,    38,    39,    40,     0,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,     0,    50,    51,     0,    52,     0,    54,
      55,    56,    57,    58,    59,    60,     0,    61,    62,    63,
       0,    65,    66,     0,     0,     0,     0,    67,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
      74,     0,     0,     0,     0,   157,    76,    77,    78,    79,
      80,     0,    81,    82,     0,    83,    84,    85,    86,     0,
      87,     0,     0,     0,    88,     5,     6,     7,     8,     9,
      89,     0,     0,     0,    10,    92,    93,    94,    95,    96,
       0,    97,    98,  1326,    99,   100,     0,   101,   102,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,     0,    41,     0,    42,     0,
      43,     0,     0,    44,     0,     0,     0,    45,    46,    47,
      48,     0,    50,    51,     0,    52,     0,    54,    55,    56,
      57,    58,    59,    60,     0,    61,    62,    63,     0,    65,
      66,     0,     0,     0,     0,    67,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,    74,     0,
       0,     0,     0,   157,    76,    77,    78,    79,    80,     0,
      81,    82,     0,    83,    84,    85,    86,     0,    87,     0,
       0,     0,    88,     5,     6,     7,     8,     9,    89,     0,
       0,     0,    10,    92,    93,    94,    95,    96,     0,    97,
      98,  1329,    99,   100,     0,   101,   102,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,     0,    41,  1332,    42,     0,    43,     0,
       0,    44,     0,     0,     0,    45,    46,    47,    48,     0,
      50,    51,     0,    52,     0,    54,    55,    56,    57,    58,
      59,    60,     0,    61,    62,    63,     0,    65,    66,     0,
       0,     0,     0,    67,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,   157,    76,    77,    78,    79,    80,     0,    81,    82,
       0,    83,    84,    85,    86,     0,    87,     0,     0,     0,
      88,     5,     6,     7,     8,     9,    89,     0,     0,     0,
      10,    92,    93,    94,    95,    96,     0,    97,    98,     0,
      99,   100,     0,   101,   102,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,     0,    41,     0,    42,     0,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,     0,    50,    51,
       0,    52,     0,    54,    55,    56,    57,    58,    59,    60,
       0,    61,    62,    63,     0,    65,    66,     0,     0,     0,
       0,    67,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,    74,     0,     0,     0,     0,   157,
      76,    77,    78,    79,    80,     0,    81,    82,     0,    83,
      84,    85,    86,     0,    87,     0,     0,     0,    88,     5,
       6,     7,     8,     9,    89,     0,     0,     0,    10,    92,
      93,    94,    95,    96,     0,    97,    98,  1334,    99,   100,
       0,   101,   102,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
       0,    36,     0,     0,     0,    37,    38,    39,    40,     0,
      41,     0,    42,     0,    43,     0,     0,    44,     0,     0,
       0,    45,    46,    47,    48,     0,    50,    51,     0,    52,
       0,    54,    55,    56,    57,    58,    59,    60,     0,    61,
      62,    63,     0,    65,    66,     0,     0,     0,     0,    67,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,    74,     0,     0,     0,     0,   157,    76,    77,
      78,    79,    80,     0,    81,    82,     0,    83,    84,    85,
      86,     0,    87,     0,     0,     0,    88,     5,     6,     7,
       8,     9,    89,     0,     0,     0,    10,    92,    93,    94,
      95,    96,     0,    97,    98,  1346,    99,   100,     0,   101,
     102,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,     0,    36,
       0,     0,     0,    37,    38,    39,    40,     0,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,     0,    50,    51,     0,    52,     0,    54,
      55,    56,    57,    58,    59,    60,     0,    61,    62,    63,
       0,    65,    66,     0,     0,     0,     0,    67,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
      74,     0,     0,     0,     0,   157,    76,    77,    78,    79,
      80,     0,    81,    82,     0,    83,    84,    85,    86,     0,
      87,     0,     0,     0,    88,     5,     6,     7,     8,     9,
      89,     0,     0,     0,    10,    92,    93,    94,    95,    96,
       0,    97,    98,  1398,    99,   100,     0,   101,   102,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,     0,    41,     0,    42,     0,
      43,     0,     0,    44,     0,     0,     0,    45,    46,    47,
      48,     0,    50,    51,     0,    52,     0,    54,    55,    56,
      57,    58,    59,    60,     0,    61,    62,    63,     0,    65,
      66,     0,     0,     0,     0,    67,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,    74,     0,
       0,     0,     0,   157,    76,    77,    78,    79,    80,     0,
      81,    82,     0,    83,    84,    85,    86,     0,    87,     0,
       0,     0,    88,     5,     6,     7,     8,     9,    89,     0,
       0,     0,    10,    92,    93,    94,    95,    96,     0,    97,
      98,  1410,    99,   100,     0,   101,   102,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,     0,    41,     0,    42,     0,    43,     0,
       0,    44,     0,     0,     0,    45,    46,    47,    48,     0,
      50,    51,     0,    52,     0,    54,    55,    56,    57,    58,
      59,    60,     0,    61,    62,    63,     0,    65,    66,     0,
       0,     0,     0,    67,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,   157,    76,    77,    78,    79,    80,     0,    81,    82,
       0,    83,    84,    85,    86,     0,    87,     0,     0,     0,
      88,     0,     0,     0,     0,     0,    89,     0,     0,     0,
       0,    92,    93,    94,    95,    96,     0,    97,    98,     0,
      99,   100,     0,   101,   102,     5,     6,     7,     8,     9,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   402,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,     0,    41,     0,    42,     0,
      43,     0,     0,    44,     0,     0,     0,    45,    46,    47,
      48,     0,    50,    51,     0,    52,     0,    54,    55,    56,
      57,   153,   154,    60,     0,    61,    62,    63,     0,     0,
       0,     0,     0,     0,     0,    67,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,    74,     0,
       0,     0,     0,   157,    76,    77,    78,    79,    80,     0,
      81,    82,     0,    83,    84,    85,     0,     0,    87,     0,
       0,     0,    88,     0,     0,     0,     0,     0,    89,     0,
       0,     0,     0,    92,    93,    94,    95,    96,     0,    97,
      98,     0,    99,   100,     0,   101,   102,     5,     6,     7,
       8,     9,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     592,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,     0,    36,
       0,     0,     0,    37,    38,    39,    40,     0,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,     0,    50,    51,     0,    52,     0,    54,
      55,    56,    57,   153,   154,    60,     0,    61,    62,    63,
       0,     0,     0,     0,     0,     0,     0,    67,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
      74,     0,     0,     0,     0,   157,    76,    77,    78,    79,
      80,     0,    81,    82,     0,    83,    84,    85,     0,     0,
      87,     0,     0,     0,    88,     0,     0,     0,     0,     0,
      89,     0,     0,     0,     0,    92,    93,    94,    95,    96,
       0,    97,    98,     0,    99,   100,     0,   101,   102,     5,
       6,     7,     8,     9,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   752,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
       0,    36,     0,     0,     0,    37,    38,    39,    40,     0,
      41,     0,    42,     0,    43,     0,     0,    44,     0,     0,
       0,    45,    46,    47,    48,     0,    50,    51,     0,    52,
       0,    54,    55,    56,    57,   153,   154,    60,     0,    61,
      62,    63,     0,     0,     0,     0,     0,     0,     0,    67,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,    74,     0,     0,     0,     0,   157,    76,    77,
      78,    79,    80,     0,    81,    82,     0,    83,    84,    85,
       0,     0,    87,     0,     0,     0,    88,     0,     0,     0,
       0,     0,    89,     0,     0,     0,     0,    92,    93,    94,
      95,    96,     0,    97,    98,     0,    99,   100,     0,   101,
     102,     5,     6,     7,     8,     9,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1121,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,     0,    41,     0,    42,     0,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,     0,    50,    51,
       0,    52,     0,    54,    55,    56,    57,   153,   154,    60,
       0,    61,    62,    63,     0,     0,     0,     0,     0,     0,
       0,    67,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,    74,     0,     0,     0,     0,   157,
      76,    77,    78,    79,    80,     0,    81,    82,     0,    83,
      84,    85,     0,     0,    87,     0,     0,     0,    88,     0,
       0,     0,     0,     0,    89,     0,     0,     0,     0,    92,
      93,    94,    95,    96,     0,    97,    98,     0,    99,   100,
       0,   101,   102,     5,     6,     7,     8,     9,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1252,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,     0,    41,     0,    42,     0,    43,     0,
       0,    44,     0,     0,     0,    45,    46,    47,    48,     0,
      50,    51,     0,    52,     0,    54,    55,    56,    57,   153,
     154,    60,     0,    61,    62,    63,     0,     0,     0,     0,
       0,     0,     0,    67,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,   157,    76,    77,    78,    79,    80,     0,    81,    82,
       0,    83,    84,    85,     0,     0,    87,     0,     0,     0,
      88,     5,     6,     7,     8,     9,    89,     0,     0,     0,
      10,    92,    93,    94,    95,    96,     0,    97,    98,     0,
      99,   100,     0,   101,   102,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,     0,    41,     0,    42,     0,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,     0,    50,    51,
       0,    52,     0,    54,    55,    56,    57,   153,   154,    60,
       0,    61,    62,    63,     0,     0,     0,     0,     0,     0,
       0,    67,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,    74,     0,     0,     0,     0,   157,
      76,    77,    78,    79,    80,     0,    81,    82,     0,    83,
      84,    85,     0,     0,    87,     0,     0,     0,    88,     5,
       6,     7,     8,     9,    89,     0,     0,     0,    10,    92,
      93,    94,    95,    96,     0,    97,    98,     0,    99,   100,
       0,   101,   102,     0,     0,     0,     0,   542,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
       0,     0,     0,     0,     0,    37,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,    48,   337,     0,     0,     0,     0,
       0,     0,    55,    56,    57,   153,   154,   155,     0,     0,
      62,    63,     0,     0,     0,     0,     0,     0,     0,   156,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,    74,     0,     0,     0,     0,   157,    76,    77,
      78,   543,    80,     0,    81,    82,     0,    83,    84,    85,
       0,     0,    87,     0,     0,     0,    88,     5,     6,     7,
       8,     9,    89,     0,     0,     0,    10,    92,    93,    94,
      95,    96,     0,     0,     0,     0,    99,   100,     0,   101,
     102,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,    33,    34,    35,     0,     0,
       0,     0,     0,    37,     0,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,    48,   337,     0,     0,     0,     0,     0,     0,
      55,    56,    57,   153,   154,   155,     0,     0,    62,    63,
       0,     0,     0,     0,     0,     0,     0,   156,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
      74,     0,     0,     0,     0,   157,    76,    77,    78,     0,
      80,     0,    81,    82,     0,    83,    84,    85,     0,     0,
      87,     0,     0,     0,    88,     5,     6,     7,     8,     9,
      89,     0,     0,     0,    10,    92,    93,    94,    95,    96,
       0,   233,     0,     0,    99,   100,     0,   101,   102,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,    35,     0,     0,     0,     0,
       0,    37,  -724,  -724,  -724,  -724,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,     0,   337,
      48,     0,     0,     0,     0,     0,     0,     0,    55,    56,
      57,   153,   154,   155,     0,     0,    62,    63,     0,     0,
       0,     0,     0,     0,     0,   156,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,    74,     0,
       0,     0,     0,   157,    76,    77,    78,   543,    80,     0,
      81,    82,     0,    83,    84,    85,     0,     0,    87,     0,
       0,     0,    88,     5,     6,     7,     8,     9,    89,     0,
       0,     0,    10,    92,    93,    94,    95,    96,     0,     0,
       0,     0,    99,   100,     0,   101,   102,     0,     0,     0,
       0,   189,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,    35,     0,     0,     0,     0,     0,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    55,    56,    57,   153,
     154,   155,     0,     0,    62,    63,     0,     0,     0,     0,
       0,     0,     0,   156,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,   157,    76,    77,    78,     0,    80,     0,    81,    82,
       0,    83,    84,    85,     0,     0,    87,     0,     0,     0,
      88,     5,     6,     7,     8,     9,    89,     0,     0,     0,
      10,    92,     0,    94,    95,    96,     0,     0,     0,     0,
      99,   100,     0,   101,   102,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,    35,     0,     0,     0,     0,     0,    37,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    55,    56,    57,   153,   154,   155,
       0,     0,    62,    63,     0,     0,     0,     0,     0,     0,
       0,   156,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,    74,     0,     0,     0,     0,   157,
      76,    77,    78,     0,    80,     0,    81,    82,     0,    83,
      84,    85,     0,     0,    87,     0,     0,     0,    88,     5,
       6,     7,     8,     9,    89,     0,     0,     0,    10,    92,
       0,    94,    95,    96,     0,   217,     0,     0,    99,   100,
       0,   101,   102,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
       0,     0,     0,     0,     0,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    55,    56,    57,   153,   154,   155,     0,     0,
      62,    63,     0,     0,     0,     0,     0,     0,     0,   156,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,    74,     0,     0,     0,     0,   157,    76,    77,
      78,     0,    80,     0,    81,    82,     0,    83,    84,    85,
       0,     0,    87,     0,     0,     0,    88,     5,     6,     7,
       8,     9,    89,     0,     0,     0,    10,    92,     0,    94,
      95,    96,     0,   220,     0,     0,    99,   100,     0,   101,
     102,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,    33,    34,    35,     0,     0,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   270,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      55,    56,    57,   153,   154,   155,     0,     0,    62,    63,
       0,     0,     0,     0,     0,     0,     0,   156,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
      74,     0,     0,     0,     0,   157,    76,    77,    78,     0,
      80,     0,    81,    82,     0,    83,    84,    85,     0,     0,
      87,     0,     0,     0,    88,     5,     6,     7,     8,     9,
      89,     0,     0,     0,    10,    92,     0,    94,    95,    96,
       0,     0,     0,     0,    99,   100,     0,   101,   102,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,    35,     0,     0,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    55,    56,
      57,   153,   154,   155,     0,     0,    62,    63,     0,     0,
       0,     0,     0,     0,     0,   156,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,    74,     0,
       0,     0,     0,   157,    76,    77,    78,     0,    80,     0,
      81,    82,     0,    83,    84,    85,     0,     0,    87,     0,
       0,     0,    88,     5,     6,     7,     8,     9,    89,     0,
       0,     0,    10,    92,     0,    94,    95,    96,   400,     0,
       0,     0,    99,   100,     0,   101,   102,     0,     0,     0,
       0,   493,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,    35,     0,     0,     0,     0,     0,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    55,    56,    57,   153,
     154,   155,     0,     0,    62,    63,     0,     0,     0,     0,
       0,     0,     0,   156,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,   157,    76,    77,    78,     0,    80,     0,    81,    82,
       0,    83,    84,    85,     0,     0,    87,     0,     0,     0,
      88,     0,     0,     0,     0,     0,    89,     0,     0,     0,
       0,    92,     0,    94,    95,    96,     0,     0,     0,     0,
      99,   100,     0,   101,   102,     5,     6,     7,     8,     9,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   505,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,    35,     0,     0,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    55,    56,
      57,   153,   154,   155,     0,     0,    62,    63,     0,     0,
       0,     0,     0,     0,     0,   156,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,    74,     0,
       0,     0,     0,   157,    76,    77,    78,     0,    80,     0,
      81,    82,     0,    83,    84,    85,     0,     0,    87,     0,
       0,     0,    88,     5,     6,     7,     8,     9,    89,     0,
       0,     0,    10,    92,     0,    94,    95,    96,     0,     0,
       0,     0,    99,   100,     0,   101,   102,     0,     0,     0,
       0,   542,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,    35,     0,     0,     0,     0,     0,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    55,    56,    57,   153,
     154,   155,     0,     0,    62,    63,     0,     0,     0,     0,
       0,     0,     0,   156,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,   157,    76,    77,    78,     0,    80,     0,    81,    82,
       0,    83,    84,    85,     0,     0,    87,     0,     0,     0,
      88,     5,     6,     7,     8,     9,    89,     0,     0,     0,
      10,    92,     0,    94,    95,    96,     0,     0,     0,     0,
      99,   100,     0,   101,   102,     0,     0,     0,     0,   576,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,    35,     0,     0,     0,     0,     0,    37,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    55,    56,    57,   153,   154,   155,
       0,     0,    62,    63,     0,     0,     0,     0,     0,     0,
       0,   156,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,    74,     0,     0,     0,     0,   157,
      76,    77,    78,     0,    80,     0,    81,    82,     0,    83,
      84,    85,     0,     0,    87,     0,     0,     0,    88,     5,
       6,     7,     8,     9,    89,     0,     0,     0,    10,    92,
       0,    94,    95,    96,     0,     0,     0,     0,    99,   100,
       0,   101,   102,     0,     0,     0,     0,   578,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
       0,     0,     0,     0,     0,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    55,    56,    57,   153,   154,   155,     0,     0,
      62,    63,     0,     0,     0,     0,     0,     0,     0,   156,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,    74,     0,     0,     0,     0,   157,    76,    77,
      78,     0,    80,     0,    81,    82,     0,    83,    84,    85,
       0,     0,    87,     0,     0,     0,    88,     5,     6,     7,
       8,     9,    89,     0,     0,     0,    10,    92,     0,    94,
      95,    96,     0,     0,     0,     0,    99,   100,     0,   101,
     102,     0,     0,     0,     0,   815,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,    33,    34,    35,     0,     0,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      55,    56,    57,   153,   154,   155,     0,     0,    62,    63,
       0,     0,     0,     0,     0,     0,     0,   156,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
      74,     0,     0,     0,     0,   157,    76,    77,    78,     0,
      80,     0,    81,    82,     0,    83,    84,    85,     0,     0,
      87,     0,     0,     0,    88,     5,     6,     7,     8,     9,
      89,     0,     0,     0,    10,    92,     0,    94,    95,    96,
       0,     0,     0,     0,    99,   100,     0,   101,   102,     0,
       0,     0,     0,   859,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,    35,     0,     0,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    55,    56,
      57,   153,   154,   155,     0,     0,    62,    63,     0,     0,
       0,     0,     0,     0,     0,   156,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,    74,     0,
       0,     0,     0,   157,    76,    77,    78,     0,    80,     0,
      81,    82,     0,    83,    84,    85,     0,     0,    87,     0,
       0,     0,    88,     5,     6,     7,     8,     9,    89,     0,
       0,     0,    10,    92,     0,    94,    95,    96,     0,     0,
       0,     0,    99,   100,     0,   101,   102,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,    35,     0,     0,     0,     0,     0,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    55,    56,    57,   153,
     154,   155,     0,     0,    62,    63,     0,     0,     0,     0,
       0,     0,     0,   156,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,   157,    76,    77,    78,     0,    80,     0,    81,    82,
       0,    83,    84,    85,     0,     0,    87,     0,     0,     0,
      88,     5,     6,     7,     8,     9,    89,     0,     0,     0,
      10,    92,     0,    94,    95,    96,     0,     0,     0,     0,
      99,   100,     0,   101,   102,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
     456,    35,     0,     0,     0,     0,     0,    37,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    55,    56,    57,   153,   154,   155,
       0,     0,    62,    63,     0,     0,     0,     0,     0,     0,
       0,   156,    68,    69,    70,    71,    72,    73,     0,  1138,
    1139,  1140,  1141,  1142,    74,  1143,  1144,  1145,  1146,   157,
      76,    77,    78,     0,    80,     0,    81,    82,     0,    83,
      84,    85,     0,     0,    87,     0,     0,     0,    88,     0,
       0,     0,     0,     0,    89,     0,     0,     0,     0,    92,
       0,    94,    95,    96,     0,  1147,     0,     0,    99,   100,
       0,   101,   102,     0,     0,     0,     0,     0,  1148,  1149,
    1150,  1151,  1152,  1153,  1154,     0,     0,    33,     0,     0,
       0,     0,     0,     0,     0,     0,  1155,  1156,  1157,  1158,
    1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,
    1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,  1178,
    1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,  1187,  1188,
    1189,  1190,  1191,  1192,  1193,  1194,  1195,     0,     0,  1196,
    1197,  1198,  1199,  1200,  1201,  1202,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1203,  1204,  1205,
       0,  1206,     0,     0,    81,    82,     0,    83,    84,    85,
    1207,  1208,  1209,     0,     0,  1210,   312,   313,   314,     0,
       0,     0,  1211,  1212,     0,  1213,     0,  1214,  1215,  1216,
      95,     0,   315,     0,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,     0,   337,   312,   313,   314,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   315,     0,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,   336,     0,   337,   312,   313,
     314,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   315,     0,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,     0,   337,   312,
     313,   314,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   315,     0,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,     0,   337,
     312,   313,   314,     0,     0,     0,   785,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   315,     0,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,     0,
     337,   312,   313,   314,     0,     0,     0,   838,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   315,     0,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
       0,   337,   312,   313,   314,     0,     0,     0,   861,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   315,     0,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,     0,   337,   312,   313,   314,     0,     0,     0,  1020,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   315,
       0,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,     0,   337,   312,   313,   314,     0,     0,     0,
    1096,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     315,     0,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,     0,   337,   312,   313,   314,     0,     0,
       0,  1097,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   315,     0,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,     0,   337,   312,   313,   314,     0,
     338,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   315,     0,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,     0,   337,   312,   313,   314,
       0,   414,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   315,     0,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,   336,     0,   337,   312,   313,
     314,     0,   416,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   315,     0,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,     0,   337,   312,
     313,   314,     0,   427,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   315,     0,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,     0,   337,
     312,   313,   314,     0,   429,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   315,     0,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,     0,
     337,     0,     0,   315,   568,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,   336,     0,   337,   312,   313,
     314,     0,   386,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   315,   587,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,     0,   337,   312,
     313,   314,     0,   467,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   315,     0,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,     0,   337,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   312,   313,   314,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     315,   694,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,     0,   337,   312,   313,   314,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   315,   733,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,     0,   337,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   312,   313,   314,     0,     0,     0,   940,     0,     0,
       0,     0,     0,     0,     0,     0,   596,   315,   691,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
       0,   337,   312,   313,   314,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   315,     0,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,     0,   337,   313,   314,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   315,     0,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   314,   337,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   315,     0,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,     0,   337
};

static const yytype_int16 yycheck[] =
{
       4,   117,    50,   159,   139,    30,   236,    86,     4,     4,
     296,    90,    91,     4,     4,    40,     4,    24,    25,    44,
     373,    28,    28,   569,   149,   196,     4,   820,   348,   142,
     337,   293,  1026,    96,    24,    25,   932,   116,   380,   368,
     595,    53,    46,   666,   160,    49,   463,     8,   684,   202,
      26,     8,    46,    73,   823,     8,     4,   826,   202,     8,
      61,     8,    66,    75,     8,   394,    78,     8,    29,   804,
       8,     8,     8,     8,  1029,    41,  1031,     8,     8,   222,
     223,     8,    86,     8,   227,    78,    90,    91,     8,     8,
     203,    73,   102,     8,     8,    46,     8,    61,    73,    61,
       8,     8,   140,    78,   140,     8,    96,     8,   121,     8,
       8,     0,   116,    73,    31,    61,   147,   175,    78,   175,
     183,  1247,    78,   162,   162,   175,   162,   601,   102,   160,
      26,   473,   195,   175,   121,   175,   175,   687,   175,   175,
     178,    31,   178,   178,   137,    66,    67,   178,   304,   159,
     293,   701,   146,   175,   777,  1110,    73,   121,   140,   121,
     180,   139,   137,   138,   139,   169,   179,   161,  1294,   117,
     939,    41,   402,    41,   175,    41,   183,   178,   138,   139,
      61,   137,   189,    73,    61,   159,   175,   823,   195,   176,
     826,   157,   534,   183,   140,   146,    61,    73,    61,   189,
      61,   177,   937,    61,   208,   195,    61,   211,   943,   944,
     176,   215,   160,   208,   178,   176,   211,   342,   208,   375,
     299,   211,   179,   183,   177,   178,   238,   176,   232,   176,
     853,   347,   178,   177,    61,   176,   243,   362,   176,   656,
     177,   177,   177,   250,   251,   252,   177,   177,   163,   176,
     257,   163,   177,   243,   266,   163,   263,   177,   177,   384,
     250,   251,   252,   177,   606,   344,   345,   257,   393,  1038,
     177,   396,   908,   263,   177,   176,   419,   176,   176,   283,
     916,   755,   178,   757,    61,   835,   117,   157,   292,   157,
     643,   157,   296,   283,  1249,   299,   932,   178,   175,    61,
     348,   178,  1037,   939,   180,   283,   176,    73,   176,   855,
     176,    61,    78,   178,   869,   178,    61,   178,   381,    31,
     178,    90,    61,   178,    61,   478,   121,    73,    93,    94,
     337,   337,   475,   476,   478,   283,   342,   147,   342,   343,
     344,   345,    90,    73,   121,   117,   608,    31,    78,    93,
      94,   178,   175,    31,   175,   774,   362,   175,   362,   778,
     121,    73,   592,   850,   535,   988,   373,   854,   178,   175,
     699,    31,   138,   139,   381,   144,    71,  1371,   384,   708,
     384,   176,    71,   373,    73,   541,   728,   393,   559,    73,
     396,   381,   396,  1387,  1030,    73,   144,   704,   346,  1035,
    1036,   178,  1038,   407,   575,   951,    73,   953,   138,   139,
    1365,    78,   407,    73,   179,   176,   178,   407,   184,   431,
     146,   574,   501,  1046,   428,   178,  1381,   580,   178,   433,
     574,   773,   175,   178,   180,   179,   580,   144,   428,   178,
      73,   178,   446,    71,  1340,    78,    73,   618,   452,   453,
     428,   146,    26,   148,   149,   175,   151,   152,   153,   148,
     149,  1357,   151,   152,   153,   608,   609,   580,    31,    13,
    1366,   138,   139,    41,   177,   144,   177,  1113,   798,   174,
     428,   177,    26,   177,    73,   174,   493,    66,    67,   178,
     340,   180,   177,  1286,    69,    70,    71,   501,  1044,    43,
     177,   178,   831,   493,   137,   138,   139,   849,   177,   178,
     839,   138,   139,   141,   177,   365,   144,   178,   871,   369,
     148,   149,   752,   151,   152,   153,    61,    73,   671,   177,
     178,    99,    78,   140,   677,   542,   104,    61,   106,   107,
     108,   109,   110,   111,   112,    61,   174,   672,   137,   138,
     139,   179,   542,   895,   175,    25,   175,   910,    61,   901,
    1270,  1271,  1355,  1266,  1267,   569,   591,   571,   140,   576,
    1116,   578,    42,   148,   149,    45,   151,   152,   153,   175,
     148,   149,   144,   151,   588,    40,   576,    49,   578,   596,
     140,   162,   138,   139,   598,   599,   121,   182,   588,   174,
     929,    92,    93,    94,   599,   173,   596,     8,   140,   599,
     588,   179,    92,    93,    94,   175,   759,   959,   761,   948,
      98,   140,    25,   627,   703,   121,   177,   634,   106,   107,
     108,   109,   110,   111,   175,   162,   643,   644,    13,    42,
     588,   812,    45,    13,   634,    13,   789,   177,    45,    46,
      47,   655,    49,   643,   644,   106,   107,   108,   176,   655,
     655,   100,   101,   102,   655,   655,   672,   655,   672,   162,
     177,    79,    13,   903,    90,   176,   176,   655,   175,   822,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,  1021,   181,    98,   173,   175,   704,   704,   703,
      71,   175,    73,     8,  1340,  1251,    82,   655,   176,   176,
       8,   715,   716,   717,  1000,   740,   177,    13,    79,   175,
       8,  1357,    73,   177,   119,   175,    59,    60,   176,    61,
    1366,   120,   260,   161,   964,   122,   264,   741,     8,   746,
     744,   176,   885,   176,    62,    63,   741,    13,    61,   979,
     798,   741,     8,    71,   744,    73,   746,   285,    13,   287,
     288,   289,   290,   119,   182,   182,   744,   179,     8,  1098,
     175,   182,   776,   175,   122,   779,   176,   148,   149,   175,
     151,   152,   153,   182,   176,     8,   790,   177,   121,   177,
     176,   175,   175,   111,   140,   175,   744,   140,   802,   178,
     177,    13,    13,   174,  1090,    90,   802,   802,   815,   180,
     182,   802,   802,   178,   802,   178,    13,   177,    71,  1049,
     175,   175,   175,   141,   802,   815,   144,  1057,   146,    13,
     148,   149,    25,   151,   152,   153,   175,   862,   175,   175,
     983,     8,   160,   176,   122,   176,   850,   177,   177,   175,
     854,   855,   859,    13,   802,   998,   174,   175,    51,     8,
     176,   865,   180,   122,   871,   872,   176,  1010,   182,   859,
     865,   875,   137,     8,   175,   865,   178,  1012,    71,     8,
    1023,   871,   872,  1026,    71,   875,   175,   122,     8,   175,
     177,  1121,   176,   175,  1010,   148,   149,   875,   151,   152,
     153,   178,   176,   910,   175,    98,    42,    43,   122,   178,
     178,     8,   176,   106,   107,   108,   109,   110,   111,   182,
     910,   174,   175,    26,    68,    61,   177,   875,   137,   176,
     908,   124,  1088,    69,    70,    71,   177,   163,   916,    26,
     176,   122,     8,    79,   176,   176,   122,   951,   141,   953,
     179,   144,     8,   146,   179,   148,   149,   144,   151,   152,
     153,   148,   149,    90,   151,   152,   153,    24,    25,   175,
     178,    28,    25,   166,   106,   107,   108,   109,   110,   111,
     173,   174,   175,    26,   176,   176,   176,   174,   124,   175,
       8,   178,    49,   997,   176,    26,  1000,   176,    51,   122,
     136,  1005,   176,   176,   122,   177,   176,   997,  1238,   177,
      73,   177,   148,   149,  1244,   151,   152,   153,    71,   997,
     150,   146,  1252,   175,   104,   176,    73,    13,  1258,   176,
     166,   176,   122,   176,  1012,   176,   122,   122,   174,    13,
    1044,   173,   175,    73,   178,    98,  1050,   178,   176,   997,
    1054,   177,  1030,    13,  1058,   176,   178,  1035,  1036,  1054,
    1050,  1009,  1292,   122,  1054,    13,    13,   176,  1058,    73,
      13,   124,  1050,  1077,   178,    51,    73,   175,  1234,  1083,
    1058,    71,    73,    73,     8,    13,  1090,    90,   141,   177,
     140,   144,    90,   146,    29,   148,   149,    31,   151,   152,
     153,    71,  1050,    13,    42,    43,    44,    45,    46,    47,
    1058,    49,  1116,   166,   153,  1119,  1120,   175,    73,   155,
    1124,   174,   175,   177,   176,  1120,   183,   176,     8,  1119,
    1120,   175,   189,    73,  1124,  1113,   177,    71,   195,    73,
      73,  1119,   176,    13,  1092,   115,  1124,   176,    13,    13,
     178,    73,  1231,    13,  1348,  1385,   431,   345,   148,   149,
     343,   151,   152,   153,   344,   222,   223,   139,   702,   700,
     227,  1119,    71,  1403,    73,   658,  1124,   111,   148,   149,
    1362,   151,   152,   153,   174,   872,   243,   976,   178,   123,
     180,  1357,   436,   250,   251,   252,  1136,  1220,  1005,  1390,
     257,  1336,  1379,  1282,   174,   175,   263,   141,    71,    38,
     144,   424,   146,  1232,   148,   149,  1109,   151,   152,   153,
      96,   805,    71,   837,   716,   438,   730,  1231,  1371,   251,
      -1,   258,    -1,   762,    -1,  1351,   293,    -1,    -1,   296,
     174,   175,    -1,    -1,  1387,    -1,   180,  1251,    -1,   148,
     149,  1255,   151,   152,   153,    -1,    71,  1261,    73,    74,
      -1,    -1,  1266,  1267,    -1,  1255,  1270,  1271,    -1,    -1,
      24,    25,    -1,    -1,    28,   174,    -1,  1255,  1282,   178,
     337,   180,    -1,  1287,    -1,   148,   149,  1291,   151,   152,
     153,    -1,  1296,    -1,  1298,   144,    -1,  1287,    -1,   148,
     149,  1291,   151,   152,   153,    41,  1296,  1255,  1298,  1287,
      -1,   174,   175,  1291,    -1,    -1,   373,    -1,  1296,    -1,
    1298,    -1,    -1,    -1,   381,   174,  1330,    -1,    -1,    -1,
      -1,  1335,    -1,   148,   149,    -1,   151,   152,   153,  1287,
    1330,    -1,    -1,  1291,    -1,    71,  1350,    -1,  1296,    -1,
    1298,    -1,  1330,    -1,    -1,    -1,    -1,    -1,  1336,   174,
      -1,    -1,   419,    99,    -1,    -1,    -1,   424,   104,    -1,
     106,   107,   108,   109,   110,   111,   112,    -1,    -1,    -1,
      -1,   438,  1330,    63,   597,    -1,    -1,    -1,  1392,    -1,
      -1,    71,    -1,    73,    -1,    -1,    -1,   610,   611,   612,
      -1,    -1,  1392,  1407,    -1,    -1,    -1,    25,    -1,    -1,
      71,    -1,   148,   149,  1392,   151,    -1,  1407,   475,   476,
     146,    -1,   148,   149,   150,   151,   152,   153,    -1,  1407,
      -1,   111,    -1,    51,    -1,   189,   493,   173,    -1,    -1,
      -1,    -1,    -1,   179,  1392,    -1,    -1,    -1,   174,   175,
      49,    -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,  1407,
      -1,   141,    -1,    -1,   144,    -1,   146,    -1,   148,   149,
     683,   151,   152,   153,   106,   107,   108,   109,   110,   111,
      -1,    25,    -1,    -1,    -1,   542,    -1,   148,   149,   243,
     151,   152,   153,    -1,   174,   175,   250,   251,   252,    -1,
     180,    -1,    -1,   257,    -1,    -1,   124,    51,    -1,   263,
      -1,    -1,    -1,   174,    -1,    -1,    25,    -1,    -1,   576,
      -1,   578,    -1,   141,    -1,    -1,   144,    71,   146,    -1,
     148,   149,    -1,   151,   152,   153,    -1,    -1,    -1,   596,
     597,   173,    51,    -1,    -1,   177,    -1,    -1,   166,    -1,
      -1,   608,   609,   610,   611,   612,   174,   175,    -1,    -1,
      -1,   774,    71,    -1,    -1,   778,    -1,   780,    -1,    -1,
      -1,    -1,    -1,   786,    -1,    -1,    -1,   634,    -1,    -1,
     124,    -1,    -1,   337,   641,    -1,   643,   644,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,    -1,    -1,
     144,   658,   146,    -1,   148,   149,    -1,   151,   152,   153,
      -1,    -1,    -1,   157,   671,   124,    -1,    -1,    -1,   373,
     677,    -1,   166,   222,   223,    -1,   683,    -1,   227,   686,
     174,   175,   141,    -1,    -1,   144,    -1,   146,    -1,   148,
     149,    -1,   151,   152,   153,    41,    -1,   704,   157,    -1,
      -1,   106,   107,   108,   109,   110,   111,   166,    -1,    -1,
      -1,   874,   117,   118,    -1,   174,   175,    71,    -1,    73,
     424,    -1,    -1,    -1,    -1,   888,    -1,   890,    -1,    -1,
      -1,    -1,    -1,    -1,   438,    -1,    71,    -1,    -1,   746,
      -1,    -1,    -1,    -1,   293,    -1,    -1,   296,    -1,   154,
      -1,    -1,   759,    99,   761,    -1,    -1,   111,   104,    -1,
     106,   107,   108,   109,   110,   111,   112,   774,   173,   123,
      -1,   778,    -1,   780,    -1,    -1,    -1,    41,    -1,   786,
      -1,    -1,   789,    -1,    -1,    -1,    -1,   141,    -1,   493,
     144,    41,   146,    -1,   148,   149,    -1,   151,   152,   153,
      -1,    -1,   148,   149,    -1,   151,   141,    -1,   815,   144,
      -1,   146,    -1,   148,   149,   822,   151,   152,   153,    -1,
     174,   175,    -1,    -1,   987,    -1,   180,   173,    -1,   992,
      -1,   994,    -1,   179,    -1,    99,    -1,    -1,   542,   174,
     104,    -1,   106,   107,   108,   109,   110,   111,   112,    99,
      -1,    -1,   859,    -1,   104,    -1,   106,   107,   108,   109,
     110,   111,   112,    -1,   871,   872,    -1,   874,    -1,    -1,
     419,    71,   576,    -1,   578,   424,    -1,    -1,   885,    -1,
      -1,   888,    -1,   890,   148,   149,    -1,   151,    -1,   438,
      -1,    -1,   596,   597,    -1,   902,    -1,  1060,   148,   149,
     907,   151,    -1,   910,    -1,    -1,   610,   611,   612,   173,
      -1,    -1,   112,    -1,    -1,   179,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   173,   124,    -1,   475,   476,    -1,   179,
     634,    71,    -1,    73,    -1,    -1,    -1,   641,    -1,   643,
     644,   141,    -1,  1106,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,   658,    -1,    -1,    41,    -1,    -1,
      -1,    -1,    -1,    -1,  1127,  1128,    -1,    -1,  1131,    -1,
      -1,   111,  1135,    -1,   174,    -1,   983,    -1,    -1,   683,
     987,    -1,   686,    -1,    -1,   992,    -1,   994,    -1,    -1,
      -1,   998,    -1,  1000,  1001,    -1,  1003,    -1,    -1,    -1,
     704,   141,    -1,  1010,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    99,  1023,    -1,    -1,  1026,
     104,    -1,   106,   107,   108,   109,   110,   111,   112,    -1,
      -1,    -1,    -1,    -1,   174,   175,    -1,    -1,   178,    -1,
     180,    -1,   746,    -1,    -1,    -1,    -1,    -1,   597,    -1,
      -1,    -1,    -1,  1060,    -1,    -1,    -1,    -1,    -1,   608,
     609,   610,   611,   612,   148,   149,    -1,   151,    -1,    -1,
     774,    -1,    -1,    -1,   778,    -1,   780,    -1,  1241,    71,
      -1,    73,   786,  1090,    -1,    -1,    -1,    -1,    -1,   173,
      -1,    -1,    -1,    -1,    -1,   179,    -1,    -1,    -1,  1106,
      -1,    -1,  1265,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   815,    -1,  1276,    -1,    -1,    -1,    -1,  1281,   111,
    1127,  1128,   671,    -1,  1131,  1132,  1289,    -1,   677,  1136,
      -1,   123,    -1,    -1,   683,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1305,    -1,    -1,    -1,    -1,    -1,    -1,   141,
      -1,    -1,   144,    -1,   146,   859,   148,   149,    -1,   151,
     152,   153,    -1,    -1,  1327,    -1,    -1,   871,   872,    -1,
     874,    -1,    -1,    -1,    -1,    -1,  1339,    -1,    -1,    -1,
      -1,    -1,   174,   175,   888,    -1,   890,    -1,   180,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   902,    -1,
      -1,    -1,    -1,   907,    -1,    -1,   910,    -1,    -1,    -1,
     759,    -1,   761,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,   774,    49,    -1,    -1,   778,
      -1,   780,  1395,    -1,  1241,    -1,    -1,   786,  1401,    -1,
     789,    71,  1405,    73,    -1,  1408,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1261,    -1,    -1,    -1,  1265,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1276,
      -1,    -1,    -1,   822,  1281,    42,    43,    -1,    -1,    -1,
      -1,   111,  1289,   987,    -1,    -1,    -1,    -1,   992,    -1,
     994,    -1,   424,    -1,    61,    -1,    -1,  1001,    -1,  1003,
      -1,    -1,    69,    70,    71,    -1,   438,    -1,    -1,    -1,
      -1,   141,    79,    -1,   144,    -1,   146,    -1,   148,   149,
    1327,   151,   152,   153,    -1,   874,    -1,    -1,    -1,    -1,
      -1,    -1,  1339,    71,    -1,    73,   885,    -1,    -1,   888,
      -1,   890,    -1,    -1,   174,   175,    -1,    -1,    -1,    -1,
     180,    -1,    -1,    -1,    -1,  1362,  1060,   124,   125,   126,
     127,   128,   129,    -1,  1371,    -1,    -1,  1374,    -1,   136,
      -1,    -1,    -1,   111,   141,   142,   143,   144,    -1,   146,
    1387,   148,   149,    -1,   151,   152,   153,    -1,  1395,   156,
      -1,    -1,    -1,    -1,  1401,    -1,    -1,    -1,  1405,   166,
      -1,  1408,  1106,   141,   171,    -1,   144,   174,   146,    -1,
     148,   149,    -1,   151,   152,   153,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1127,  1128,    -1,    -1,  1131,  1132,    -1,
      -1,    -1,  1136,    -1,   983,    -1,   174,   175,   987,    -1,
      -1,    -1,   180,   992,    -1,   994,    -1,    -1,    -1,   998,
      -1,  1000,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1010,    -1,    -1,    -1,   597,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1023,    -1,    -1,  1026,   610,   611,
     612,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   424,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   438,
      -1,  1060,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,  1241,    -1,    -1,
      -1,  1090,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   683,    -1,    -1,    -1,    -1,    -1,  1106,    -1,    -1,
      -1,  1265,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1276,    -1,    -1,    -1,    -1,  1281,  1127,  1128,
      -1,    -1,  1131,    -1,    -1,  1289,     5,     6,    -1,     8,
       9,    10,    -1,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    -1,    -1,    26,    27,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,
      -1,    -1,    -1,  1327,    -1,    -1,    45,    -1,    47,    -1,
      -1,    50,    -1,    52,    -1,  1339,    -1,    -1,    -1,    -1,
      -1,    -1,   774,    -1,    -1,    -1,   778,    -1,   780,    -1,
      -1,    69,    70,    -1,   786,    -1,    -1,    -1,  1362,    -1,
      79,    79,    -1,    -1,    -1,    -1,    -1,    -1,   597,   182,
    1374,    -1,    -1,    -1,    93,    -1,    -1,    96,    -1,    -1,
      -1,   610,   611,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1395,  1241,    -1,    -1,    -1,    -1,  1401,    -1,    -1,
      -1,  1405,    -1,    -1,  1408,    -1,    -1,   125,   126,   127,
     128,   129,  1261,    -1,    -1,    -1,  1265,    -1,   136,    -1,
      -1,    -1,    -1,    -1,   142,   143,    -1,  1276,    -1,    -1,
      -1,    -1,  1281,    -1,    -1,    -1,    -1,    -1,   156,    -1,
    1289,    -1,   874,    -1,    -1,   164,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   171,   683,    -1,   888,    -1,   890,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,  1327,    49,
       9,    10,    11,    -1,    -1,   204,    -1,    -1,   207,    -1,
    1339,    -1,    -1,    -1,   213,   214,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,  1371,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1387,   258,
      -1,    -1,    -1,   262,    -1,    -1,  1395,    -1,    -1,    -1,
      -1,   780,  1401,    -1,    -1,   987,  1405,   786,    -1,  1408,
     992,    -1,   994,    -1,    -1,   284,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   295,    -1,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      -1,    -1,    -1,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,    -1,    -1,
     339,   340,   341,    -1,    59,    60,    -1,    -1,  1060,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,    -1,    -1,    -1,    -1,   874,   365,   366,   177,   368,
     369,   370,    -1,    -1,    -1,    -1,    -1,   376,    -1,   888,
      -1,   890,    -1,    -1,    -1,    -1,    -1,   386,    -1,   388,
      -1,    -1,    -1,    -1,  1106,   394,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   404,   121,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1127,  1128,    -1,    -1,  1131,
       9,    10,    11,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,   434,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   467,    59,
      60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   987,    -1,
      -1,    -1,    -1,   992,    -1,   994,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,   505,    49,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,  1241,
      49,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   543,    -1,    -1,    -1,    -1,    -1,
      -1,  1060,    -1,  1265,    -1,    -1,    -1,    -1,    -1,   558,
      -1,    -1,    -1,    -1,  1276,    -1,    -1,    -1,    -1,  1281,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1289,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   584,    -1,    -1,    -1,    -1,
     179,    -1,    -1,    -1,    -1,    -1,   595,  1106,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     9,    10,    11,  1327,    -1,    -1,  1127,  1128,
      -1,    -1,  1131,    -1,    -1,    -1,    -1,  1339,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,   652,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   665,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   681,  1395,    -1,    -1,    -1,    -1,    -1,  1401,
      -1,    -1,   691,  1405,    -1,   694,  1408,   696,    -1,    -1,
     699,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   708,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1241,    25,   733,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,  1265,    49,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1276,    -1,    -1,
      -1,    -1,  1281,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1289,    -1,   179,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,   798,
      49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   811,    -1,    -1,    -1,    -1,    -1,  1327,    -1,
      -1,    -1,    -1,    -1,    -1,     9,    10,    11,    -1,    -1,
    1339,    -1,   831,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     839,    25,   841,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,
     869,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   878,
      -1,    -1,    -1,    -1,    -1,    -1,  1395,   179,    -1,    -1,
      -1,    -1,  1401,    -1,    -1,    -1,  1405,    -1,    -1,  1408,
      -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,
     919,    -1,    -1,    -1,   923,    -1,   925,    -1,    -1,    -1,
     929,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   940,    -1,    41,    42,    43,    -1,    -1,    -1,   948,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    -1,    86,    -1,
      88,    -1,    -1,    91,    -1,   179,    -1,    95,    96,    97,
      98,    99,   100,   101,    -1,   103,   104,   105,   106,   107,
     108,   109,   110,   111,    -1,   113,   114,   115,   116,   117,
     118,    -1,  1021,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
     148,   149,    -1,   151,   152,   153,   154,    -1,   156,    -1,
      -1,    -1,   160,     3,     4,     5,     6,     7,   166,   167,
      -1,   169,    12,   171,   172,   173,   174,   175,    -1,   177,
     178,   179,   180,   181,    -1,   183,   184,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1098,
      -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,    99,
     100,   101,    -1,   103,   104,   105,   106,   107,   108,   109,
     110,   111,    -1,   113,   114,   115,   116,   117,   118,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,   146,    -1,   148,   149,
      -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,
     160,     3,     4,     5,     6,     7,   166,   167,    -1,   169,
      12,   171,   172,   173,   174,   175,    -1,   177,   178,   179,
     180,   181,    -1,   183,   184,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,
      82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,
      -1,    -1,    -1,    95,    96,    97,    98,    99,   100,   101,
      -1,   103,   104,   105,   106,   107,   108,   109,   110,   111,
      -1,   113,   114,   115,   116,   117,   118,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,   146,    -1,   148,   149,    -1,   151,
     152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,     3,
       4,     5,     6,     7,   166,   167,    -1,   169,    12,   171,
     172,   173,   174,   175,    -1,   177,   178,    -1,   180,   181,
      -1,   183,   184,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,
      84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,
      -1,    95,    96,    97,    98,    -1,   100,   101,    -1,   103,
      -1,   105,   106,   107,   108,   109,   110,   111,    -1,   113,
     114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,   146,    -1,   148,   149,    -1,   151,   152,   153,
     154,    -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,
       6,     7,   166,    -1,    -1,    -1,    12,   171,   172,   173,
     174,   175,    -1,   177,   178,   179,   180,   181,    -1,   183,
     184,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,    75,
      -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,
      86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,
      96,    97,    98,    -1,   100,   101,    -1,   103,    -1,   105,
     106,   107,   108,   109,   110,   111,    -1,   113,   114,   115,
      -1,   117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
     146,    -1,   148,   149,    -1,   151,   152,   153,   154,    -1,
     156,    -1,    -1,    -1,   160,     3,     4,     5,     6,     7,
     166,    -1,    -1,    -1,    12,   171,   172,   173,   174,   175,
      -1,   177,   178,   179,   180,   181,    -1,   183,   184,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    83,    84,    -1,    86,    -1,
      88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    -1,   100,   101,    -1,   103,    -1,   105,   106,   107,
     108,   109,   110,   111,    -1,   113,   114,   115,    -1,   117,
     118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
     148,   149,    -1,   151,   152,   153,   154,    -1,   156,    -1,
      -1,    -1,   160,     3,     4,     5,     6,     7,   166,    -1,
      -1,    -1,    12,   171,   172,   173,   174,   175,    -1,   177,
     178,    -1,   180,   181,    -1,   183,   184,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    -1,    86,    -1,    88,    89,
      -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,
     100,   101,    -1,   103,    -1,   105,   106,   107,   108,   109,
     110,   111,    -1,   113,   114,   115,    -1,   117,   118,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,   146,    -1,   148,   149,
      -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,
     160,     3,     4,     5,     6,     7,   166,    -1,    -1,    -1,
      12,   171,   172,   173,   174,   175,    -1,   177,   178,    -1,
     180,   181,    -1,   183,   184,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,
      82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,
      -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,   101,
      -1,   103,    -1,   105,   106,   107,   108,   109,   110,   111,
      -1,   113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,   146,    -1,   148,   149,    -1,   151,
     152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,     3,
       4,     5,     6,     7,   166,    -1,    -1,    -1,    12,   171,
     172,   173,   174,   175,    -1,   177,   178,   179,   180,   181,
      -1,   183,   184,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,
      84,    -1,    86,    87,    88,    -1,    -1,    91,    -1,    -1,
      -1,    95,    96,    97,    98,    -1,   100,   101,    -1,   103,
      -1,   105,   106,   107,   108,   109,   110,   111,    -1,   113,
     114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,   146,    -1,   148,   149,    -1,   151,   152,   153,
     154,    -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,
       6,     7,   166,    -1,    -1,    -1,    12,   171,   172,   173,
     174,   175,    -1,   177,   178,    -1,   180,   181,    -1,   183,
     184,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,    75,
      -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,
      86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,
      96,    97,    98,    -1,   100,   101,    -1,   103,    -1,   105,
     106,   107,   108,   109,   110,   111,    -1,   113,   114,   115,
      -1,   117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
     146,    -1,   148,   149,    -1,   151,   152,   153,   154,    -1,
     156,    -1,    -1,    -1,   160,     3,     4,     5,     6,     7,
     166,    -1,    -1,    -1,    12,   171,   172,   173,   174,   175,
      -1,   177,   178,   179,   180,   181,    -1,   183,   184,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    -1,    86,    -1,
      88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    -1,   100,   101,    -1,   103,    -1,   105,   106,   107,
     108,   109,   110,   111,    -1,   113,   114,   115,    -1,   117,
     118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
     148,   149,    -1,   151,   152,   153,   154,    -1,   156,    -1,
      -1,    -1,   160,     3,     4,     5,     6,     7,   166,    -1,
      -1,    -1,    12,   171,   172,   173,   174,   175,    -1,   177,
     178,   179,   180,   181,    -1,   183,   184,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    85,    86,    -1,    88,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,
     100,   101,    -1,   103,    -1,   105,   106,   107,   108,   109,
     110,   111,    -1,   113,   114,   115,    -1,   117,   118,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,   146,    -1,   148,   149,
      -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,
     160,     3,     4,     5,     6,     7,   166,    -1,    -1,    -1,
      12,   171,   172,   173,   174,   175,    -1,   177,   178,    -1,
     180,   181,    -1,   183,   184,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,
      82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,
      -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,   101,
      -1,   103,    -1,   105,   106,   107,   108,   109,   110,   111,
      -1,   113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,   146,    -1,   148,   149,    -1,   151,
     152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,     3,
       4,     5,     6,     7,   166,    -1,    -1,    -1,    12,   171,
     172,   173,   174,   175,    -1,   177,   178,   179,   180,   181,
      -1,   183,   184,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,
      84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,
      -1,    95,    96,    97,    98,    -1,   100,   101,    -1,   103,
      -1,   105,   106,   107,   108,   109,   110,   111,    -1,   113,
     114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,   146,    -1,   148,   149,    -1,   151,   152,   153,
     154,    -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,
       6,     7,   166,    -1,    -1,    -1,    12,   171,   172,   173,
     174,   175,    -1,   177,   178,   179,   180,   181,    -1,   183,
     184,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,    75,
      -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,
      86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,
      96,    97,    98,    -1,   100,   101,    -1,   103,    -1,   105,
     106,   107,   108,   109,   110,   111,    -1,   113,   114,   115,
      -1,   117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
     146,    -1,   148,   149,    -1,   151,   152,   153,   154,    -1,
     156,    -1,    -1,    -1,   160,     3,     4,     5,     6,     7,
     166,    -1,    -1,    -1,    12,   171,   172,   173,   174,   175,
      -1,   177,   178,   179,   180,   181,    -1,   183,   184,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    -1,    86,    -1,
      88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    -1,   100,   101,    -1,   103,    -1,   105,   106,   107,
     108,   109,   110,   111,    -1,   113,   114,   115,    -1,   117,
     118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
     148,   149,    -1,   151,   152,   153,   154,    -1,   156,    -1,
      -1,    -1,   160,     3,     4,     5,     6,     7,   166,    -1,
      -1,    -1,    12,   171,   172,   173,   174,   175,    -1,   177,
     178,   179,   180,   181,    -1,   183,   184,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,
     100,   101,    -1,   103,    -1,   105,   106,   107,   108,   109,
     110,   111,    -1,   113,   114,   115,    -1,   117,   118,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,   146,    -1,   148,   149,
      -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,
     160,    -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,    -1,
      -1,   171,   172,   173,   174,   175,    -1,   177,   178,    -1,
     180,   181,    -1,   183,   184,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    -1,    86,    -1,
      88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    -1,   100,   101,    -1,   103,    -1,   105,   106,   107,
     108,   109,   110,   111,    -1,   113,   114,   115,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
     148,   149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,
      -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,   166,    -1,
      -1,    -1,    -1,   171,   172,   173,   174,   175,    -1,   177,
     178,    -1,   180,   181,    -1,   183,   184,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,    75,
      -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,
      86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,
      96,    97,    98,    -1,   100,   101,    -1,   103,    -1,   105,
     106,   107,   108,   109,   110,   111,    -1,   113,   114,   115,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
     146,    -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,
     156,    -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,
     166,    -1,    -1,    -1,    -1,   171,   172,   173,   174,   175,
      -1,   177,   178,    -1,   180,   181,    -1,   183,   184,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,
      84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,
      -1,    95,    96,    97,    98,    -1,   100,   101,    -1,   103,
      -1,   105,   106,   107,   108,   109,   110,   111,    -1,   113,
     114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,   146,    -1,   148,   149,    -1,   151,   152,   153,
      -1,    -1,   156,    -1,    -1,    -1,   160,    -1,    -1,    -1,
      -1,    -1,   166,    -1,    -1,    -1,    -1,   171,   172,   173,
     174,   175,    -1,   177,   178,    -1,   180,   181,    -1,   183,
     184,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,
      82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,
      -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,   101,
      -1,   103,    -1,   105,   106,   107,   108,   109,   110,   111,
      -1,   113,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,    -1,
      -1,    -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,   171,
     172,   173,   174,   175,    -1,   177,   178,    -1,   180,   181,
      -1,   183,   184,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,
     100,   101,    -1,   103,    -1,   105,   106,   107,   108,   109,
     110,   111,    -1,   113,   114,   115,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,
     160,     3,     4,     5,     6,     7,   166,    -1,    -1,    -1,
      12,   171,   172,   173,   174,   175,    -1,   177,   178,    -1,
     180,   181,    -1,   183,   184,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,
      82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,
      -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,   101,
      -1,   103,    -1,   105,   106,   107,   108,   109,   110,   111,
      -1,   113,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,
       4,     5,     6,     7,   166,    -1,    -1,    -1,    12,   171,
     172,   173,   174,   175,    -1,   177,   178,    -1,   180,   181,
      -1,   183,   184,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    -1,    79,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    98,    49,    -1,    -1,    -1,    -1,
      -1,    -1,   106,   107,   108,   109,   110,   111,    -1,    -1,
     114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,   146,    -1,   148,   149,    -1,   151,   152,   153,
      -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,
       6,     7,   166,    -1,    -1,    -1,    12,   171,   172,   173,
     174,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    -1,    79,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    98,    49,    -1,    -1,    -1,    -1,    -1,    -1,
     106,   107,   108,   109,   110,   111,    -1,    -1,   114,   115,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,
     146,    -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,
     156,    -1,    -1,    -1,   160,     3,     4,     5,     6,     7,
     166,    -1,    -1,    -1,    12,   171,   172,   173,   174,   175,
      -1,   177,    -1,    -1,   180,   181,    -1,   183,   184,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      -1,    79,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,
     108,   109,   110,   111,    -1,    -1,   114,   115,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
     148,   149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,
      -1,    -1,   160,     3,     4,     5,     6,     7,   166,    -1,
      -1,    -1,    12,   171,   172,   173,   174,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,   109,
     110,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,
     160,     3,     4,     5,     6,     7,   166,    -1,    -1,    -1,
      12,   171,    -1,   173,   174,   175,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,   184,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,   107,   108,   109,   110,   111,
      -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,    -1,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,
       4,     5,     6,     7,   166,    -1,    -1,    -1,    12,   171,
      -1,   173,   174,   175,    -1,   177,    -1,    -1,   180,   181,
      -1,   183,   184,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   106,   107,   108,   109,   110,   111,    -1,    -1,
     114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,    -1,   146,    -1,   148,   149,    -1,   151,   152,   153,
      -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,
       6,     7,   166,    -1,    -1,    -1,    12,   171,    -1,   173,
     174,   175,    -1,   177,    -1,    -1,   180,   181,    -1,   183,
     184,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    95,
      -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     106,   107,   108,   109,   110,   111,    -1,    -1,   114,   115,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,
     146,    -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,
     156,    -1,    -1,    -1,   160,     3,     4,     5,     6,     7,
     166,    -1,    -1,    -1,    12,   171,    -1,   173,   174,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,
     108,   109,   110,   111,    -1,    -1,   114,   115,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,
     148,   149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,
      -1,    -1,   160,     3,     4,     5,     6,     7,   166,    -1,
      -1,    -1,    12,   171,    -1,   173,   174,   175,   176,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,   109,
     110,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,
     160,    -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,    -1,
      -1,   171,    -1,   173,   174,   175,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,   184,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,
     108,   109,   110,   111,    -1,    -1,   114,   115,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,
     148,   149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,
      -1,    -1,   160,     3,     4,     5,     6,     7,   166,    -1,
      -1,    -1,    12,   171,    -1,   173,   174,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,   109,
     110,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,
     160,     3,     4,     5,     6,     7,   166,    -1,    -1,    -1,
      12,   171,    -1,   173,   174,   175,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,   184,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,   107,   108,   109,   110,   111,
      -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,    -1,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,
       4,     5,     6,     7,   166,    -1,    -1,    -1,    12,   171,
      -1,   173,   174,   175,    -1,    -1,    -1,    -1,   180,   181,
      -1,   183,   184,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   106,   107,   108,   109,   110,   111,    -1,    -1,
     114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,    -1,   146,    -1,   148,   149,    -1,   151,   152,   153,
      -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,
       6,     7,   166,    -1,    -1,    -1,    12,   171,    -1,   173,
     174,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,
     184,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     106,   107,   108,   109,   110,   111,    -1,    -1,   114,   115,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,
     146,    -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,
     156,    -1,    -1,    -1,   160,     3,     4,     5,     6,     7,
     166,    -1,    -1,    -1,    12,   171,    -1,   173,   174,   175,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,
     108,   109,   110,   111,    -1,    -1,   114,   115,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,
     148,   149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,
      -1,    -1,   160,     3,     4,     5,     6,     7,   166,    -1,
      -1,    -1,    12,   171,    -1,   173,   174,   175,    -1,    -1,
      -1,    -1,   180,   181,    -1,   183,   184,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,   109,
     110,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,
     160,     3,     4,     5,     6,     7,   166,    -1,    -1,    -1,
      12,   171,    -1,   173,   174,   175,    -1,    -1,    -1,    -1,
     180,   181,    -1,   183,   184,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,   107,   108,   109,   110,   111,
      -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,     3,
       4,     5,     6,     7,   136,     9,    10,    11,    12,   141,
     142,   143,   144,    -1,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,    -1,
      -1,    -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,   171,
      -1,   173,   174,   175,    -1,    49,    -1,    -1,   180,   181,
      -1,   183,   184,    -1,    -1,    -1,    -1,    -1,    62,    63,
      64,    65,    66,    67,    68,    -1,    -1,    71,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,
      -1,   145,    -1,    -1,   148,   149,    -1,   151,   152,   153,
     154,   155,   156,    -1,    -1,   159,     9,    10,    11,    -1,
      -1,    -1,   166,   167,    -1,   169,    -1,   171,   172,   173,
     174,    -1,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
       9,    10,    11,    -1,    -1,    -1,   179,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,     9,    10,    11,    -1,    -1,    -1,   179,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,     9,    10,    11,    -1,    -1,    -1,   179,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,     9,    10,    11,    -1,    -1,    -1,   179,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,     9,    10,    11,    -1,    -1,    -1,
     179,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,     9,    10,    11,    -1,    -1,
      -1,   179,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,     9,    10,    11,    -1,
     177,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,     9,    10,    11,
      -1,   177,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,     9,    10,
      11,    -1,   177,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,     9,
      10,    11,    -1,   177,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
       9,    10,    11,    -1,   177,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    25,   176,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,     9,    10,
      11,    -1,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,   176,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,     9,
      10,    11,    -1,   122,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,   122,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,   122,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     9,    10,    11,    -1,    -1,    -1,   122,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    90,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    11,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   186,   187,     0,   188,     3,     4,     5,     6,     7,
      12,    41,    42,    43,    48,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    69,    70,    71,    72,    73,    75,    79,    80,    81,
      82,    84,    86,    88,    91,    95,    96,    97,    98,    99,
     100,   101,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   113,   114,   115,   116,   117,   118,   123,   124,   125,
     126,   127,   128,   129,   136,   141,   142,   143,   144,   145,
     146,   148,   149,   151,   152,   153,   154,   156,   160,   166,
     167,   169,   171,   172,   173,   174,   175,   177,   178,   180,
     181,   183,   184,   189,   192,   195,   196,   197,   198,   199,
     200,   203,   214,   215,   219,   224,   230,   285,   286,   291,
     295,   296,   297,   298,   299,   300,   301,   302,   310,   311,
     312,   314,   315,   318,   328,   329,   330,   335,   338,   356,
     361,   363,   364,   365,   366,   367,   368,   369,   370,   372,
     385,   387,   389,   109,   110,   111,   123,   141,   192,   214,
     285,   301,   363,   301,   175,   301,   301,   301,   354,   355,
     301,   301,   301,   301,   301,   301,   301,   301,   301,   301,
     301,   301,   111,   175,   196,   329,   330,   363,   363,    31,
     301,   376,   377,   301,   111,   175,   196,   329,   330,   331,
     362,   368,   373,   374,   175,   292,   332,   175,   292,   293,
     301,   205,   292,   175,   175,   175,   292,   177,   301,   192,
     177,   301,    25,    51,   124,   146,   166,   175,   192,   199,
     390,   400,   401,   177,   298,   301,   178,   301,   144,   193,
     194,   195,    73,   180,   256,   257,   117,   117,    73,   258,
     175,   175,   175,   175,   192,   228,   391,   175,   175,    73,
      78,   137,   138,   139,   382,   383,   144,   178,   195,   195,
      95,   301,   229,   391,   146,   175,   391,   391,   301,   291,
     301,   302,   363,   201,   178,    78,   333,   382,    78,   382,
     382,    26,   144,   162,   392,   175,     8,   177,    31,   213,
     146,   227,   391,   111,   214,   286,   177,   177,   177,   177,
     177,   177,     9,    10,    11,    25,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    49,   177,    61,
      61,   178,   140,   118,   154,   230,   284,   285,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      59,    60,   121,   358,   359,    61,   178,   360,   175,    61,
     178,   180,   369,   175,   213,   214,    13,   301,    40,   192,
     353,   175,   291,   363,   140,   363,   122,   182,     8,   340,
     291,   363,   392,   140,   175,   334,   121,   358,   359,   360,
     176,   301,    26,   203,     8,   177,   203,   204,   293,   294,
     301,   192,   242,   207,   177,   177,   177,   401,   401,   162,
     175,    98,   393,   401,    13,   192,   177,   177,   201,   177,
     195,     8,   177,    90,   178,   363,     8,   177,    13,     8,
     177,   363,   386,   386,   363,   176,   162,   222,   123,   363,
     375,   376,    61,   121,   137,   383,    72,   301,   363,    78,
     137,   383,   195,   191,   177,   178,   177,   122,   225,   319,
     321,    79,   305,   306,   308,    13,    90,   388,   176,   176,
     176,   179,   202,   203,   215,   219,   224,   301,   181,   183,
     184,   192,   393,    31,   254,   255,   301,   390,   175,   391,
     220,   213,   301,   301,   301,    26,   301,   301,   301,   301,
     301,   301,   301,   301,   301,   301,   301,   301,   301,   301,
     301,   301,   301,   301,   301,   301,   301,   301,   331,   301,
     371,   371,   301,   378,   379,   192,   368,   369,   228,   229,
     227,   214,    31,   145,   295,   298,   301,   301,   301,   301,
     301,   301,   301,   301,   301,   301,   301,   301,   178,   192,
     368,   371,   301,   254,   371,   301,   375,   213,   176,   175,
     352,     8,   340,   291,   176,   192,    31,   301,    31,   301,
     176,   176,   368,   254,   178,   192,   368,   176,   201,   246,
     301,    82,    26,   203,   240,   177,    90,    13,     8,   176,
      26,   178,   243,   401,    79,   397,   398,   399,   175,     8,
      42,    43,    61,   124,   136,   146,   166,   174,   196,   197,
     199,   313,   329,   335,   336,   337,   179,    90,   194,   192,
     301,   257,   336,    73,     8,   176,   176,   176,   177,   192,
     396,   119,   233,   175,     8,   176,   176,    73,    74,   192,
     384,   192,    61,   179,   179,   188,   190,   301,   120,   232,
     161,    46,   146,   161,   323,   122,     8,   340,   176,   401,
     401,    13,   121,   358,   359,   360,   179,     8,   163,   363,
     176,     8,   341,    13,   303,   216,   119,   231,   175,   391,
     301,    26,   182,   182,   122,   179,     8,   340,   392,   175,
     223,   226,   221,   213,    63,   363,   301,   392,   175,   182,
     179,   176,   182,   179,   176,    42,    43,    61,    69,    70,
      79,   124,   136,   166,   174,   192,   343,   345,   348,   351,
     192,   363,   363,   122,   358,   359,   360,   176,   301,   247,
      66,    67,   248,   292,   201,   294,    31,   123,   237,   363,
     336,   192,    26,   203,   241,   177,   244,   177,   244,     8,
     163,   122,     8,   340,   176,   157,   393,   394,   401,   336,
     336,   336,   339,   342,   175,    78,   140,   175,   175,   140,
     178,   102,   159,   210,   192,   179,    13,   363,   177,    90,
       8,   163,   234,   329,   178,   375,   123,   363,    13,   182,
     301,   179,   188,   234,   178,   322,    13,   301,   305,   177,
     401,   178,   192,   368,   401,    31,   301,   336,   157,   252,
     253,   356,   357,   175,   329,   232,   304,   217,   301,   301,
     301,   175,   254,   233,   232,   231,   391,   331,   179,   175,
     254,    13,    69,    70,   192,   344,   344,   345,   346,   347,
     175,    78,   137,   175,   175,     8,   340,   176,   352,    31,
     301,   179,    66,    67,   249,   292,   203,   177,    83,   177,
     363,   175,   122,   236,    13,   201,   244,    92,    93,    94,
     244,   179,   401,   401,   397,     8,   176,   176,   122,   182,
       8,   340,   339,   192,   305,   307,   309,   339,   192,   336,
     380,   381,   175,   178,   336,   401,   192,     8,   259,   176,
     175,   295,   298,   301,   182,   179,   259,   147,   160,   178,
     318,   325,   147,   178,   324,   122,   177,   301,   392,   175,
     363,   176,     8,   341,   401,   402,   252,   178,   252,   175,
     122,   254,   176,   178,   178,   232,   218,   334,   175,   254,
     176,   122,   182,     8,   340,   346,   137,   305,   349,   350,
     346,   345,   363,   292,    26,    68,   203,   177,   294,   375,
     237,   176,   336,    89,    92,   177,   301,    26,   177,   245,
     179,   163,   157,    26,   336,   336,   176,   122,     8,   340,
     176,   176,   122,   179,     8,   340,   329,   201,    90,   329,
      99,   104,   112,   148,   149,   151,   179,   260,   283,   284,
     285,   290,   356,   375,   179,   179,    46,   301,   301,   301,
     179,   175,   254,    26,   395,   157,   357,    31,    73,   176,
     259,   176,   252,   301,   176,   259,   259,   178,   175,   254,
     176,   345,   345,   176,   122,   176,     8,   340,   176,    26,
     201,   177,   176,   176,   208,   177,   177,   245,   201,   401,
     122,   336,   305,   336,   336,    73,   179,   401,   390,   235,
     329,   112,   124,   146,   152,   269,   270,   271,   329,   150,
     275,   276,   115,   175,   192,   277,   278,   261,   214,   401,
       8,   177,   284,   176,   146,   320,   179,   179,   175,   254,
     176,   401,   104,   316,   402,    73,    13,   395,   179,   395,
     176,   179,   179,   259,   252,   176,   122,   345,   305,   201,
     206,    26,   203,   239,   201,   176,   336,   122,   122,   176,
     211,    13,     8,   177,   178,   178,     8,   177,     3,     4,
       5,     6,     7,     9,    10,    11,    12,    49,    62,    63,
      64,    65,    66,    67,    68,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   123,   124,   125,   126,
     127,   128,   129,   141,   142,   143,   145,   154,   155,   156,
     159,   166,   167,   169,   171,   172,   173,   192,   326,   327,
       8,   177,   146,   150,   192,   278,   279,   280,   177,    73,
     289,   213,   262,   390,   214,   254,   176,   175,   178,    31,
      73,    13,   336,   177,   178,   281,   316,   395,   179,   176,
     345,   122,    26,   203,   238,   201,   336,   336,   178,   336,
     329,   265,   272,   335,   270,    13,    26,    43,   273,   276,
       8,    29,   176,    25,    42,    45,    13,     8,   177,   391,
     289,    13,   213,   176,    31,    73,   317,   201,    73,    13,
     336,   201,   178,   281,   395,   345,   201,    87,   201,   179,
     192,   199,   266,   267,   268,     8,   179,   336,   327,   327,
      51,   274,   279,   279,    25,    42,    45,   336,    73,   175,
     177,   336,   391,    73,     8,   341,   179,    13,   336,   179,
     201,   281,    85,   177,   179,   140,    90,   335,   153,    13,
     263,   175,    31,    73,   176,   336,   179,   177,   209,   192,
     284,   285,   336,   157,   250,   251,   357,   264,    73,   102,
     210,   212,   155,   192,   177,   176,     8,   341,   106,   107,
     108,   287,   288,   250,   175,   235,   177,   395,   157,   357,
     402,   176,   329,   177,   177,   178,   282,   287,    31,    73,
     395,    73,   201,   402,    73,    13,   282,   176,   179,    31,
      73,    13,   336,   178,    73,    13,   336,   201,    13,   336,
     179,   336
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
#line 707 "hphp.y"
    { _p->initParseTree(); ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 707 "hphp.y"
    { _p->popLabelInfo();
                                                  _p->finiParseTree();;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 713 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 714 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 717 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 718 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 719 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 720 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 721 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 722 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 725 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 727 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 728 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 729 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 730 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 731 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 732 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 737 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 738 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 739 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 740 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 741 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 742 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 743 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 748 "hphp.y"
    { ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 749 "hphp.y"
    { ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 752 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 753 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 754 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 756 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 760 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 762 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 765 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 767 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 768 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 771 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 778 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 785 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 793 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 796 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 802 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 803 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 806 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 807 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 812 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 816 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 821 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 822 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 823 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 826 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 828 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 831 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 832 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 834 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 835 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 837 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 838 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 839 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 840 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 841 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 842 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 843 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 844 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 845 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 846 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 847 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 848 "hphp.y"
    { (yyval).reset(); (yyval) = ';';}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 849 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 852 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 853 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]));;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 856 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 864 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(3) - (14)]),(yyvsp[(7) - (14)]),(yyvsp[(8) - (14)]),(yyvsp[(11) - (14)]),(yyvsp[(13) - (14)]),(yyvsp[(14) - (14)]));;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 867 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 868 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 869 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval)); ;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 873 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 874 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 875 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 876 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 877 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 878 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 879 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 880 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 881 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval)); ;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 893 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 894 "hphp.y"
    { (yyval).reset();;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 899 "hphp.y"
    { _p->onFinally((yyval), (yyvsp[(4) - (4)]));;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 900 "hphp.y"
    { finally_statement(_p);;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 905 "hphp.y"
    { (yyval).reset();;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 909 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 910 "hphp.y"
    { (yyval).reset();;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 914 "hphp.y"
    { _p->pushFuncLocation();;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 919 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 924 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 929 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 934 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 939 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 944 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 951 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 954 "hphp.y"
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

  case 105:

/* Line 1455 of yacc.c  */
#line 969 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 972 "hphp.y"
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

  case 107:

/* Line 1455 of yacc.c  */
#line 986 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 989 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 994 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 997 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 1004 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { Token t_ext;
                                         t_ext.reset(); 
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 1015 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 1018 "hphp.y"
    { Token t_ext;
                                         t_ext.reset(); 
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 1026 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 1027 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 1031 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 1034 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 1037 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 1038 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 1039 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 1043 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 1044 "hphp.y"
    { (yyval).reset();;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1047 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1048 "hphp.y"
    { (yyval).reset();;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1051 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1052 "hphp.y"
    { (yyval).reset();;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1055 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1057 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1060 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1062 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1066 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1067 "hphp.y"
    { (yyval).reset();;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1070 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1071 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1072 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1076 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1078 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1081 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1083 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1086 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1088 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1091 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1093 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1103 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1104 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1105 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1106 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1111 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1113 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1114 "hphp.y"
    { (yyval).reset();;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1117 "hphp.y"
    { (yyval).reset();;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1118 "hphp.y"
    { (yyval).reset();;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1123 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1124 "hphp.y"
    { (yyval).reset();;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1129 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1130 "hphp.y"
    { (yyval).reset();;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1133 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1134 "hphp.y"
    { (yyval).reset();;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1137 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1138 "hphp.y"
    { (yyval).reset();;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1143 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]);;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1145 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1146 "hphp.y"
    { (yyval).reset();;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1147 "hphp.y"
    { (yyval).reset();;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1153 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1157 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1162 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1167 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1172 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1177 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1183 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1189 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1195 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]);;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1197 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1198 "hphp.y"
    { (yyval).reset();;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1199 "hphp.y"
    { (yyval).reset();;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1204 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),0,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL);;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1207 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),1,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL);;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1211 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),1,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL);;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1215 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),0,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL);;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1219 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),0,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL);;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1223 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),1,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL);;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1228 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),1,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL);;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1233 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),0,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL);;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1239 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1240 "hphp.y"
    { (yyval).reset();;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1243 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),0);;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1244 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),1);;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1246 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1248 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1252 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1253 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1256 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1257 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1258 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1262 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1264 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1265 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1266 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1271 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1272 "hphp.y"
    { (yyval).reset();;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1275 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1276 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1279 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1280 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1282 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1286 "hphp.y"
    { _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1291 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1297 "hphp.y"
    { _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1302 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1308 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1310 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1311 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1314 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1317 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1318 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1319 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1325 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1329 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1332 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1339 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1340 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1345 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1348 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1355 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1357 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1361 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1362 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1368 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1370 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1374 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1376 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1380 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1381 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1385 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1386 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1390 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1393 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1398 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1403 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1404 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1406 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1410 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1411 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1412 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1413 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1417 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1418 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1419 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1420 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1421 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1423 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1425 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1429 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1432 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1433 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1437 "hphp.y"
    { (yyval).reset();;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1438 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1442 "hphp.y"
    { (yyval).reset();;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1443 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1446 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1447 "hphp.y"
    { (yyval).reset();;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1450 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1451 "hphp.y"
    { (yyval).reset();;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1454 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1456 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1459 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1460 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1461 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1462 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1463 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1464 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1465 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1469 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1470 "hphp.y"
    { (yyval).reset();;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1473 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1474 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1475 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1479 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1481 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1482 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1483 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1487 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1488 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1492 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1494 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1495 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1497 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1500 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1504 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1505 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1510 "hphp.y"
    { (yyval).reset();;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1514 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1515 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1519 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1524 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1528 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1537 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1542 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1543 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1547 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1552 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1553 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1555 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1556 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1557 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1558 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1559 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1560 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1563 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1566 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1567 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1568 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1573 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1575 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1578 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1580 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1581 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1582 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1584 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1585 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1586 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1587 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1588 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1589 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1590 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1591 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1592 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1593 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1595 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1596 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1600 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1601 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1603 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1604 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1605 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1606 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1607 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1608 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1610 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1615 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1617 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1619 "hphp.y"
    { Token t; _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1623 "hphp.y"
    { Token u; u.reset();
                                         _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         _p->onClosure((yyval),0,u,(yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1628 "hphp.y"
    { Token t; _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1632 "hphp.y"
    { Token u; u.reset();
                                         _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         _p->onClosure((yyval),&(yyvsp[(1) - (12)]),u,(yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1636 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1640 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1647 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1650 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1657 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1660 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1665 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1666 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1671 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1672 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1676 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1680 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1681 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1686 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1693 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1700 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1702 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1708 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1713 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1714 "hphp.y"
    { (yyval).reset();;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1718 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1719 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1721 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1728 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1731 "hphp.y"
    { Token t1; _p->onArray(t1,(yyvsp[(1) - (2)]));
                                         Token t2; _p->onArray(t2,(yyvsp[(2) - (2)]));
                                         Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onCallParam((yyvsp[(1) - (2)]),NULL,t1,0);
                                         _p->onCallParam((yyval), &(yyvsp[(1) - (2)]),t2,0);
                                         _p->onCallParam((yyvsp[(1) - (2)]), &(yyvsp[(1) - (2)]),file,0);
                                         _p->onCallParam((yyvsp[(1) - (2)]), &(yyvsp[(1) - (2)]),line,0);
                                         (yyval).setText("");;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onArray((yyvsp[(4) - (6)]),(yyvsp[(1) - (6)]));
                                         _p->onArray((yyvsp[(5) - (6)]),(yyvsp[(3) - (6)]));
                                         _p->onCallParam((yyvsp[(2) - (6)]),NULL,(yyvsp[(4) - (6)]),0);
                                         _p->onCallParam((yyval), &(yyvsp[(2) - (6)]),(yyvsp[(5) - (6)]),0);
                                         _p->onCallParam((yyvsp[(2) - (6)]), &(yyvsp[(2) - (6)]),file,0);
                                         _p->onCallParam((yyvsp[(2) - (6)]), &(yyvsp[(2) - (6)]),line,0);
                                         (yyval).setText((yyvsp[(6) - (6)]).text());;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { (yyval).reset();;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { (yyval).reset();;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1852 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1858 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1864 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1867 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1868 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1869 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1871 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1872 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1873 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1874 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1875 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1880 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1884 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1888 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1890 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1895 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { (yyval).reset();;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { (yyval).reset();;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { (yyval).reset();;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { (yyval).reset();;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 1941 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 1966 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 1967 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 1968 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 1970 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 1975 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
    { (yyval).reset();;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 1980 "hphp.y"
    { (yyval).reset();;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 1981 "hphp.y"
    { (yyval).reset();;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 1984 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 1985 "hphp.y"
    { (yyval).reset();;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 1991 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 1993 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 1995 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 1996 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2000 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2001 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2002 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2006 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2008 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2011 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2012 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2013 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2016 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2017 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2018 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2019 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2021 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2023 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2024 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2026 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2031 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2032 "hphp.y"
    { (yyval).reset();;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2037 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2039 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2041 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2042 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2046 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2047 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2052 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2053 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2058 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2061 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2066 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2067 "hphp.y"
    { (yyval).reset();;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2070 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2071 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2078 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2080 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2083 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2085 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2088 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2091 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2092 "hphp.y"
    { (yyval).reset();;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2096 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2098 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2102 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2103 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2107 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2108 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2112 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2114 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2121 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2125 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2126 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2127 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2128 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2129 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2132 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2135 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2137 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2142 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2147 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2149 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2151 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2156 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2164 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2167 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2170 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2174 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2178 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2182 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2193 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2199 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2204 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2205 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2210 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2213 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2214 "hphp.y"
    { (yyval).reset();;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2219 "hphp.y"
    { (yyval)++;;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2227 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { (yyval).reset();;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2331 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2332 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2337 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2341 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2391 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2412 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); ;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2413 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); ;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); ;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    {;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2452 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2453 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2474 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (5)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 2482 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 10837 "hphp.tab.cpp"
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
#line 2486 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

