
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
#line 643 "new_hphp.tab.cpp"

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
#line 853 "new_hphp.tab.cpp"

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
#define YYLAST   11331

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  185
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  217
/* YYNRULES -- Number of rules.  */
#define YYNRULES  751
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1414

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
     315,   316,   322,   324,   325,   327,   328,   330,   331,   343,
     344,   357,   358,   372,   373,   382,   383,   393,   394,   402,
     403,   412,   413,   421,   422,   431,   433,   435,   437,   439,
     441,   444,   447,   450,   451,   454,   455,   458,   459,   461,
     465,   467,   471,   474,   475,   477,   480,   485,   487,   492,
     494,   499,   501,   506,   508,   513,   517,   523,   527,   532,
     537,   543,   549,   554,   555,   557,   559,   564,   565,   571,
     572,   575,   576,   580,   581,   585,   588,   590,   591,   596,
     602,   610,   617,   624,   632,   642,   651,   655,   658,   660,
     661,   665,   670,   677,   683,   689,   696,   705,   713,   716,
     717,   719,   722,   726,   731,   735,   737,   739,   742,   747,
     751,   757,   759,   763,   766,   767,   768,   773,   774,   780,
     783,   784,   795,   796,   808,   812,   816,   820,   824,   830,
     833,   836,   837,   844,   850,   855,   859,   861,   863,   867,
     872,   874,   876,   878,   880,   885,   887,   891,   894,   895,
     898,   899,   901,   905,   907,   909,   911,   913,   917,   922,
     927,   932,   934,   936,   939,   942,   945,   949,   953,   955,
     957,   959,   961,   965,   967,   969,   971,   972,   974,   977,
     979,   981,   983,   985,   987,   989,   991,   993,   994,   996,
     998,  1000,  1004,  1010,  1012,  1016,  1022,  1027,  1031,  1035,
    1038,  1040,  1042,  1046,  1050,  1052,  1054,  1055,  1058,  1063,
    1067,  1074,  1077,  1081,  1088,  1090,  1092,  1094,  1101,  1105,
    1110,  1117,  1121,  1125,  1129,  1133,  1137,  1141,  1145,  1149,
    1153,  1157,  1161,  1164,  1167,  1170,  1173,  1177,  1181,  1185,
    1189,  1193,  1197,  1201,  1205,  1209,  1213,  1217,  1221,  1225,
    1229,  1233,  1237,  1240,  1243,  1246,  1249,  1253,  1257,  1261,
    1265,  1269,  1273,  1277,  1281,  1285,  1289,  1295,  1300,  1302,
    1305,  1308,  1311,  1314,  1317,  1320,  1323,  1326,  1329,  1331,
    1333,  1335,  1339,  1342,  1343,  1355,  1356,  1369,  1371,  1373,
    1379,  1383,  1389,  1393,  1396,  1397,  1400,  1401,  1406,  1411,
    1415,  1420,  1425,  1430,  1435,  1437,  1439,  1443,  1449,  1450,
    1454,  1459,  1461,  1464,  1469,  1472,  1479,  1480,  1482,  1487,
    1488,  1491,  1492,  1494,  1496,  1500,  1502,  1506,  1508,  1510,
    1514,  1518,  1520,  1522,  1524,  1526,  1528,  1530,  1532,  1534,
    1536,  1538,  1540,  1542,  1544,  1546,  1548,  1550,  1552,  1554,
    1556,  1558,  1560,  1562,  1564,  1566,  1568,  1570,  1572,  1574,
    1576,  1578,  1580,  1582,  1584,  1586,  1588,  1590,  1592,  1594,
    1596,  1598,  1600,  1602,  1604,  1606,  1608,  1610,  1612,  1614,
    1616,  1618,  1620,  1622,  1624,  1626,  1628,  1630,  1632,  1634,
    1636,  1638,  1640,  1642,  1644,  1646,  1648,  1650,  1652,  1654,
    1656,  1658,  1660,  1662,  1664,  1666,  1668,  1670,  1672,  1674,
    1676,  1678,  1683,  1685,  1687,  1689,  1691,  1693,  1695,  1697,
    1699,  1702,  1704,  1705,  1706,  1708,  1710,  1714,  1715,  1717,
    1719,  1721,  1723,  1725,  1727,  1729,  1731,  1733,  1735,  1737,
    1739,  1743,  1746,  1748,  1750,  1753,  1756,  1761,  1766,  1770,
    1775,  1777,  1779,  1783,  1787,  1789,  1791,  1793,  1795,  1799,
    1803,  1807,  1810,  1811,  1813,  1814,  1816,  1817,  1823,  1827,
    1831,  1833,  1835,  1837,  1839,  1843,  1846,  1848,  1850,  1852,
    1854,  1856,  1859,  1862,  1867,  1872,  1876,  1881,  1884,  1885,
    1891,  1895,  1899,  1901,  1905,  1907,  1910,  1911,  1917,  1921,
    1924,  1925,  1929,  1930,  1935,  1938,  1939,  1943,  1947,  1949,
    1950,  1952,  1955,  1958,  1963,  1967,  1971,  1974,  1979,  1982,
    1987,  1989,  1991,  1993,  1995,  1997,  2000,  2005,  2009,  2014,
    2018,  2020,  2022,  2024,  2026,  2029,  2034,  2039,  2043,  2045,
    2047,  2051,  2059,  2066,  2075,  2085,  2094,  2105,  2113,  2120,
    2122,  2125,  2130,  2135,  2137,  2139,  2144,  2146,  2147,  2149,
    2152,  2154,  2156,  2159,  2164,  2168,  2172,  2173,  2175,  2178,
    2183,  2187,  2190,  2194,  2201,  2202,  2204,  2209,  2212,  2213,
    2219,  2223,  2227,  2229,  2236,  2241,  2246,  2249,  2252,  2253,
    2259,  2263,  2267,  2269,  2272,  2273,  2279,  2283,  2287,  2289,
    2292,  2295,  2297,  2300,  2302,  2307,  2311,  2315,  2322,  2326,
    2328,  2330,  2332,  2337,  2342,  2345,  2348,  2353,  2356,  2359,
    2361,  2365,  2369,  2370,  2373,  2379,  2386,  2388,  2391,  2393,
    2398,  2402,  2403,  2405,  2409,  2413,  2415,  2417,  2418,  2419,
    2422,  2426,  2428,  2434,  2438,  2442,  2446,  2448,  2451,  2452,
    2457,  2460,  2463,  2465,  2467,  2469,  2474,  2481,  2483,  2492,
    2498,  2500
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     186,     0,    -1,    -1,   187,   188,    -1,   188,   189,    -1,
      -1,   203,    -1,   215,    -1,   219,    -1,   224,    -1,   388,
      -1,   116,   175,   176,   177,    -1,   141,   195,   177,    -1,
      -1,   141,   195,   178,   190,   188,   179,    -1,    -1,   141,
     178,   191,   188,   179,    -1,   104,   193,   177,    -1,   200,
     177,    -1,    71,    -1,   148,    -1,   149,    -1,   151,    -1,
     153,    -1,   152,    -1,   174,    -1,   193,     8,   194,    -1,
     194,    -1,   195,    -1,   144,   195,    -1,   195,    90,   192,
      -1,   144,   195,    90,   192,    -1,   192,    -1,   195,   144,
     192,    -1,   195,    -1,   141,   144,   195,    -1,   144,   195,
      -1,   196,    -1,   196,   391,    -1,   196,   391,    -1,   200,
       8,   389,    13,   335,    -1,    99,   389,    13,   335,    -1,
     201,   202,    -1,    -1,   203,    -1,   215,    -1,   219,    -1,
     224,    -1,   178,   201,   179,    -1,    65,   291,   203,   246,
     248,    -1,    65,   291,    26,   201,   247,   249,    68,   177,
      -1,    -1,    82,   291,   204,   240,    -1,    -1,    81,   205,
     203,    82,   291,   177,    -1,    -1,    84,   175,   293,   177,
     293,   177,   293,   176,   206,   238,    -1,    -1,    91,   291,
     207,   243,    -1,    95,   177,    -1,    95,   300,   177,    -1,
      97,   177,    -1,    97,   300,   177,    -1,   100,   177,    -1,
     100,   300,   177,    -1,   145,    95,   177,    -1,   105,   256,
     177,    -1,   111,   258,   177,    -1,    80,   292,   177,    -1,
     113,   175,   385,   176,   177,    -1,   177,    -1,    75,    -1,
      -1,    86,   175,   300,    90,   237,   236,   176,   208,   239,
      -1,    88,   175,   242,   176,   241,    -1,   101,   178,   201,
     179,   102,   175,   328,    73,   176,   178,   201,   179,   209,
     212,    -1,   101,   178,   201,   179,   210,    -1,   103,   300,
     177,    -1,    96,   192,   177,    -1,   300,   177,    -1,   294,
     177,    -1,   295,   177,    -1,   296,   177,    -1,   297,   177,
      -1,   298,   177,    -1,   100,   297,   177,    -1,   299,   177,
      -1,   192,    26,    -1,   209,   102,   175,   328,    73,   176,
     178,   201,   179,    -1,    -1,    -1,   159,   178,   201,   179,
     211,    -1,   210,    -1,    -1,    31,    -1,    -1,    98,    -1,
      -1,   214,   213,   390,   216,   175,   252,   176,   394,   178,
     201,   179,    -1,    -1,   284,   214,   213,   390,   217,   175,
     252,   176,   394,   178,   201,   179,    -1,    -1,   355,   283,
     214,   213,   390,   218,   175,   252,   176,   394,   178,   201,
     179,    -1,    -1,   230,   227,   220,   231,   232,   178,   259,
     179,    -1,    -1,   355,   230,   227,   221,   231,   232,   178,
     259,   179,    -1,    -1,   118,   228,   222,   233,   178,   259,
     179,    -1,    -1,   355,   118,   228,   223,   233,   178,   259,
     179,    -1,    -1,   154,   229,   225,   232,   178,   259,   179,
      -1,    -1,   355,   154,   229,   226,   232,   178,   259,   179,
      -1,   390,    -1,   146,    -1,   390,    -1,   390,    -1,   117,
      -1,   110,   117,    -1,   109,   117,    -1,   119,   328,    -1,
      -1,   120,   234,    -1,    -1,   119,   234,    -1,    -1,   328,
      -1,   234,     8,   328,    -1,   328,    -1,   235,     8,   328,
      -1,   122,   237,    -1,    -1,   362,    -1,    31,   362,    -1,
     123,   175,   374,   176,    -1,   203,    -1,    26,   201,    85,
     177,    -1,   203,    -1,    26,   201,    87,   177,    -1,   203,
      -1,    26,   201,    83,   177,    -1,   203,    -1,    26,   201,
      89,   177,    -1,   192,    13,   335,    -1,   242,     8,   192,
      13,   335,    -1,   178,   244,   179,    -1,   178,   177,   244,
     179,    -1,    26,   244,    92,   177,    -1,    26,   177,   244,
      92,   177,    -1,   244,    93,   300,   245,   201,    -1,   244,
      94,   245,   201,    -1,    -1,    26,    -1,   177,    -1,   246,
      66,   291,   203,    -1,    -1,   247,    66,   291,    26,   201,
      -1,    -1,    67,   203,    -1,    -1,    67,    26,   201,    -1,
      -1,   251,     8,   157,    -1,   251,   340,    -1,   157,    -1,
      -1,   356,   286,   401,    73,    -1,   356,   286,   401,    31,
      73,    -1,   356,   286,   401,    31,    73,    13,   335,    -1,
     356,   286,   401,    73,    13,   335,    -1,   251,     8,   286,
     356,   401,    73,    -1,   251,     8,   286,   356,   401,    31,
      73,    -1,   251,     8,   286,   356,   401,    31,    73,    13,
     335,    -1,   251,     8,   286,   356,   401,    73,    13,   335,
      -1,   253,     8,   157,    -1,   253,   340,    -1,   157,    -1,
      -1,   356,   401,    73,    -1,   356,   401,    31,    73,    -1,
     356,   401,    31,    73,    13,   335,    -1,   356,   401,    73,
      13,   335,    -1,   253,     8,   356,   401,    73,    -1,   253,
       8,   356,   401,    31,    73,    -1,   253,     8,   356,   401,
      31,    73,    13,   335,    -1,   253,     8,   356,   401,    73,
      13,   335,    -1,   255,   340,    -1,    -1,   300,    -1,    31,
     362,    -1,   255,     8,   300,    -1,   255,     8,    31,   362,
      -1,   256,     8,   257,    -1,   257,    -1,    73,    -1,   180,
     362,    -1,   180,   178,   300,   179,    -1,   258,     8,    73,
      -1,   258,     8,    73,    13,   335,    -1,    73,    -1,    73,
      13,   335,    -1,   259,   260,    -1,    -1,    -1,   282,   261,
     288,   177,    -1,    -1,   284,   400,   262,   288,   177,    -1,
     289,   177,    -1,    -1,   283,   214,   213,   390,   175,   263,
     250,   176,   394,   281,    -1,    -1,   355,   283,   214,   213,
     390,   175,   264,   250,   176,   394,   281,    -1,   148,   269,
     177,    -1,   149,   275,   177,    -1,   151,   277,   177,    -1,
     104,   235,   177,    -1,   104,   235,   178,   265,   179,    -1,
     265,   266,    -1,   265,   267,    -1,    -1,   199,   140,   192,
     155,   235,   177,    -1,   268,    90,   283,   192,   177,    -1,
     268,    90,   284,   177,    -1,   199,   140,   192,    -1,   192,
      -1,   270,    -1,   269,     8,   270,    -1,   271,   325,   273,
     274,    -1,   146,    -1,   124,    -1,   328,    -1,   112,    -1,
     152,   178,   272,   179,    -1,   334,    -1,   272,     8,   334,
      -1,    13,   335,    -1,    -1,    51,   153,    -1,    -1,   276,
      -1,   275,     8,   276,    -1,   150,    -1,   278,    -1,   192,
      -1,   115,    -1,   175,   279,   176,    -1,   175,   279,   176,
      45,    -1,   175,   279,   176,    25,    -1,   175,   279,   176,
      42,    -1,   278,    -1,   280,    -1,   280,    45,    -1,   280,
      25,    -1,   280,    42,    -1,   279,     8,   279,    -1,   279,
      29,   279,    -1,   192,    -1,   146,    -1,   150,    -1,   177,
      -1,   178,   201,   179,    -1,   284,    -1,   112,    -1,   284,
      -1,    -1,   285,    -1,   284,   285,    -1,   106,    -1,   107,
      -1,   108,    -1,   111,    -1,   110,    -1,   109,    -1,   173,
      -1,   287,    -1,    -1,   106,    -1,   107,    -1,   108,    -1,
     288,     8,    73,    -1,   288,     8,    73,    13,   335,    -1,
      73,    -1,    73,    13,   335,    -1,   289,     8,   389,    13,
     335,    -1,    99,   389,    13,   335,    -1,   175,   290,   176,
      -1,    63,   330,   333,    -1,    62,   300,    -1,   317,    -1,
     311,    -1,   175,   300,   176,    -1,   292,     8,   300,    -1,
     300,    -1,   292,    -1,    -1,   145,   300,    -1,   145,   300,
     122,   300,    -1,   362,    13,   294,    -1,   123,   175,   374,
     176,    13,   294,    -1,   172,   300,    -1,   362,    13,   297,
      -1,   123,   175,   374,   176,    13,   297,    -1,   301,    -1,
     362,    -1,   290,    -1,   123,   175,   374,   176,    13,   300,
      -1,   362,    13,   300,    -1,   362,    13,    31,   362,    -1,
     362,    13,    31,    63,   330,   333,    -1,   362,    24,   300,
      -1,   362,    23,   300,    -1,   362,    22,   300,    -1,   362,
      21,   300,    -1,   362,    20,   300,    -1,   362,    19,   300,
      -1,   362,    18,   300,    -1,   362,    17,   300,    -1,   362,
      16,   300,    -1,   362,    15,   300,    -1,   362,    14,   300,
      -1,   362,    60,    -1,    60,   362,    -1,   362,    59,    -1,
      59,   362,    -1,   300,    27,   300,    -1,   300,    28,   300,
      -1,   300,     9,   300,    -1,   300,    11,   300,    -1,   300,
      10,   300,    -1,   300,    29,   300,    -1,   300,    31,   300,
      -1,   300,    30,   300,    -1,   300,    44,   300,    -1,   300,
      42,   300,    -1,   300,    43,   300,    -1,   300,    45,   300,
      -1,   300,    46,   300,    -1,   300,    47,   300,    -1,   300,
      41,   300,    -1,   300,    40,   300,    -1,    42,   300,    -1,
      43,   300,    -1,    48,   300,    -1,    50,   300,    -1,   300,
      33,   300,    -1,   300,    32,   300,    -1,   300,    35,   300,
      -1,   300,    34,   300,    -1,   300,    36,   300,    -1,   300,
      39,   300,    -1,   300,    37,   300,    -1,   300,    38,   300,
      -1,   300,    49,   330,    -1,   175,   301,   176,    -1,   300,
      25,   300,    26,   300,    -1,   300,    25,    26,   300,    -1,
     384,    -1,    58,   300,    -1,    57,   300,    -1,    56,   300,
      -1,    55,   300,    -1,    54,   300,    -1,    53,   300,    -1,
      52,   300,    -1,    64,   331,    -1,    51,   300,    -1,   337,
      -1,   310,    -1,   309,    -1,   181,   332,   181,    -1,    12,
     300,    -1,    -1,   214,   213,   175,   302,   252,   176,   394,
     315,   178,   201,   179,    -1,    -1,   284,   214,   213,   175,
     303,   252,   176,   394,   315,   178,   201,   179,    -1,   313,
      -1,    79,    -1,   305,     8,   304,   122,   300,    -1,   304,
     122,   300,    -1,   306,     8,   304,   122,   335,    -1,   304,
     122,   335,    -1,   305,   339,    -1,    -1,   306,   339,    -1,
      -1,   166,   175,   307,   176,    -1,   124,   175,   375,   176,
      -1,    61,   375,   182,    -1,   328,   178,   377,   179,    -1,
     328,   178,   379,   179,    -1,   313,    61,   370,   182,    -1,
     314,    61,   370,   182,    -1,   310,    -1,   386,    -1,   175,
     301,   176,    -1,   104,   175,   316,   340,   176,    -1,    -1,
     316,     8,    73,    -1,   316,     8,    31,    73,    -1,    73,
      -1,    31,    73,    -1,   160,   146,   318,   161,    -1,   320,
      46,    -1,   320,   161,   321,   160,    46,   319,    -1,    -1,
     146,    -1,   320,   322,    13,   323,    -1,    -1,   321,   324,
      -1,    -1,   146,    -1,   147,    -1,   178,   300,   179,    -1,
     147,    -1,   178,   300,   179,    -1,   317,    -1,   326,    -1,
     325,    26,   326,    -1,   325,    43,   326,    -1,   192,    -1,
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
     254,   176,    -1,   199,    -1,   146,    -1,   328,    -1,   111,
      -1,   368,    -1,   328,    -1,   111,    -1,   372,    -1,   175,
     176,    -1,   291,    -1,    -1,    -1,    78,    -1,   381,    -1,
     175,   254,   176,    -1,    -1,    69,    -1,    70,    -1,    79,
      -1,   128,    -1,   129,    -1,   143,    -1,   125,    -1,   156,
      -1,   126,    -1,   127,    -1,   142,    -1,   171,    -1,   136,
      78,   137,    -1,   136,   137,    -1,   334,    -1,   197,    -1,
      42,   335,    -1,    43,   335,    -1,   124,   175,   338,   176,
      -1,   174,   175,   338,   176,    -1,    61,   338,   182,    -1,
     166,   175,   308,   176,    -1,   336,    -1,   312,    -1,   199,
     140,   192,    -1,   146,   140,   192,    -1,   197,    -1,    72,
      -1,   386,    -1,   334,    -1,   183,   381,   183,    -1,   184,
     381,   184,    -1,   136,   381,   137,    -1,   341,   339,    -1,
      -1,     8,    -1,    -1,     8,    -1,    -1,   341,     8,   335,
     122,   335,    -1,   341,     8,   335,    -1,   335,   122,   335,
      -1,   335,    -1,    69,    -1,    70,    -1,    79,    -1,   136,
      78,   137,    -1,   136,   137,    -1,    69,    -1,    70,    -1,
     192,    -1,   342,    -1,   192,    -1,    42,   343,    -1,    43,
     343,    -1,   124,   175,   345,   176,    -1,   174,   175,   345,
     176,    -1,    61,   345,   182,    -1,   166,   175,   348,   176,
      -1,   346,   339,    -1,    -1,   346,     8,   344,   122,   344,
      -1,   346,     8,   344,    -1,   344,   122,   344,    -1,   344,
      -1,   347,     8,   344,    -1,   344,    -1,   349,   339,    -1,
      -1,   349,     8,   304,   122,   344,    -1,   304,   122,   344,
      -1,   347,   339,    -1,    -1,   175,   350,   176,    -1,    -1,
     352,     8,   192,   351,    -1,   192,   351,    -1,    -1,   354,
     352,   339,    -1,    41,   353,    40,    -1,   355,    -1,    -1,
     358,    -1,   121,   367,    -1,   121,   192,    -1,   121,   178,
     300,   179,    -1,    61,   370,   182,    -1,   178,   300,   179,
      -1,   363,   359,    -1,   175,   290,   176,   359,    -1,   373,
     359,    -1,   175,   290,   176,   359,    -1,   367,    -1,   327,
      -1,   365,    -1,   366,    -1,   360,    -1,   362,   357,    -1,
     175,   290,   176,   357,    -1,   329,   140,   367,    -1,   364,
     175,   254,   176,    -1,   175,   362,   176,    -1,   327,    -1,
     365,    -1,   366,    -1,   360,    -1,   362,   358,    -1,   175,
     290,   176,   358,    -1,   364,   175,   254,   176,    -1,   175,
     362,   176,    -1,   367,    -1,   360,    -1,   175,   362,   176,
      -1,   362,   121,   192,   391,   175,   254,   176,    -1,   362,
     121,   367,   175,   254,   176,    -1,   362,   121,   178,   300,
     179,   175,   254,   176,    -1,   175,   290,   176,   121,   192,
     391,   175,   254,   176,    -1,   175,   290,   176,   121,   367,
     175,   254,   176,    -1,   175,   290,   176,   121,   178,   300,
     179,   175,   254,   176,    -1,   329,   140,   192,   391,   175,
     254,   176,    -1,   329,   140,   367,   175,   254,   176,    -1,
     368,    -1,   371,   368,    -1,   368,    61,   370,   182,    -1,
     368,   178,   300,   179,    -1,   369,    -1,    73,    -1,   180,
     178,   300,   179,    -1,   300,    -1,    -1,   180,    -1,   371,
     180,    -1,   367,    -1,   361,    -1,   372,   357,    -1,   175,
     290,   176,   357,    -1,   329,   140,   367,    -1,   175,   362,
     176,    -1,    -1,   361,    -1,   372,   358,    -1,   175,   290,
     176,   358,    -1,   175,   362,   176,    -1,   374,     8,    -1,
     374,     8,   362,    -1,   374,     8,   123,   175,   374,   176,
      -1,    -1,   362,    -1,   123,   175,   374,   176,    -1,   376,
     339,    -1,    -1,   376,     8,   300,   122,   300,    -1,   376,
       8,   300,    -1,   300,   122,   300,    -1,   300,    -1,   376,
       8,   300,   122,    31,   362,    -1,   376,     8,    31,   362,
      -1,   300,   122,    31,   362,    -1,    31,   362,    -1,   378,
     339,    -1,    -1,   378,     8,   300,   122,   300,    -1,   378,
       8,   300,    -1,   300,   122,   300,    -1,   300,    -1,   380,
     339,    -1,    -1,   380,     8,   335,   122,   335,    -1,   380,
       8,   335,    -1,   335,   122,   335,    -1,   335,    -1,   381,
     382,    -1,   381,    78,    -1,   382,    -1,    78,   382,    -1,
      73,    -1,    73,    61,   383,   182,    -1,    73,   121,   192,
      -1,   138,   300,   179,    -1,   138,    72,    61,   300,   182,
     179,    -1,   139,   362,   179,    -1,   192,    -1,    74,    -1,
      73,    -1,   114,   175,   385,   176,    -1,   115,   175,   362,
     176,    -1,     7,   300,    -1,     6,   300,    -1,     5,   175,
     300,   176,    -1,     4,   300,    -1,     3,   300,    -1,   362,
      -1,   385,     8,   362,    -1,   329,   140,   192,    -1,    -1,
      90,   400,    -1,   167,   390,    13,   400,   177,    -1,   169,
     390,   387,    13,   400,   177,    -1,   192,    -1,   400,   192,
      -1,   192,    -1,   192,   162,   395,   163,    -1,   162,   392,
     163,    -1,    -1,   400,    -1,   392,     8,   400,    -1,   392,
       8,   157,    -1,   392,    -1,   157,    -1,    -1,    -1,    26,
     400,    -1,   395,     8,   192,    -1,   192,    -1,   395,     8,
     192,    90,   400,    -1,   192,    90,   400,    -1,    79,   122,
     400,    -1,   397,     8,   396,    -1,   396,    -1,   397,   339,
      -1,    -1,   166,   175,   398,   176,    -1,    25,   400,    -1,
      51,   400,    -1,   199,    -1,   124,    -1,   399,    -1,   124,
     162,   400,   163,    -1,   124,   162,   400,     8,   400,   163,
      -1,   146,    -1,   175,    98,   175,   393,   176,    26,   400,
     176,    -1,   175,   392,     8,   400,   176,    -1,   400,    -1,
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
    1433,  1437,  1438,  1441,  1442,  1445,  1446,  1449,  1450,  1454,
    1455,  1456,  1457,  1458,  1459,  1460,  1464,  1465,  1468,  1469,
    1470,  1473,  1475,  1477,  1478,  1481,  1483,  1487,  1488,  1490,
    1491,  1492,  1495,  1499,  1500,  1504,  1505,  1509,  1510,  1514,
    1518,  1523,  1527,  1531,  1536,  1537,  1538,  1541,  1543,  1544,
    1545,  1548,  1549,  1550,  1551,  1552,  1553,  1554,  1555,  1556,
    1557,  1558,  1559,  1560,  1561,  1562,  1563,  1564,  1565,  1566,
    1567,  1568,  1569,  1570,  1571,  1572,  1573,  1574,  1575,  1576,
    1577,  1578,  1579,  1580,  1581,  1582,  1583,  1584,  1585,  1586,
    1587,  1588,  1590,  1591,  1593,  1595,  1596,  1597,  1598,  1599,
    1600,  1601,  1602,  1603,  1604,  1605,  1606,  1607,  1608,  1609,
    1610,  1611,  1612,  1614,  1613,  1622,  1621,  1629,  1633,  1637,
    1641,  1647,  1651,  1657,  1659,  1663,  1665,  1669,  1673,  1674,
    1678,  1685,  1692,  1694,  1699,  1700,  1701,  1705,  1707,  1711,
    1712,  1713,  1714,  1718,  1724,  1733,  1746,  1747,  1750,  1753,
    1756,  1757,  1760,  1764,  1767,  1770,  1777,  1778,  1782,  1783,
    1785,  1789,  1790,  1791,  1792,  1793,  1794,  1795,  1796,  1797,
    1798,  1799,  1800,  1801,  1802,  1803,  1804,  1805,  1806,  1807,
    1808,  1809,  1810,  1811,  1812,  1813,  1814,  1815,  1816,  1817,
    1818,  1819,  1820,  1821,  1822,  1823,  1824,  1825,  1826,  1827,
    1828,  1829,  1830,  1831,  1832,  1833,  1834,  1835,  1836,  1837,
    1838,  1839,  1840,  1841,  1842,  1843,  1844,  1845,  1846,  1847,
    1848,  1849,  1850,  1851,  1852,  1853,  1854,  1855,  1856,  1857,
    1858,  1859,  1860,  1861,  1862,  1863,  1864,  1865,  1866,  1867,
    1868,  1872,  1877,  1878,  1881,  1882,  1883,  1887,  1888,  1889,
    1893,  1894,  1895,  1899,  1900,  1901,  1904,  1906,  1910,  1911,
    1912,  1914,  1915,  1916,  1917,  1918,  1919,  1920,  1921,  1922,
    1923,  1926,  1931,  1932,  1933,  1934,  1935,  1937,  1939,  1940,
    1942,  1943,  1947,  1950,  1956,  1957,  1958,  1959,  1960,  1961,
    1962,  1967,  1969,  1973,  1974,  1977,  1978,  1982,  1985,  1987,
    1989,  1993,  1994,  1995,  1997,  2000,  2004,  2005,  2006,  2009,
    2010,  2011,  2012,  2013,  2015,  2017,  2018,  2023,  2025,  2028,
    2031,  2033,  2035,  2038,  2040,  2044,  2046,  2049,  2052,  2058,
    2060,  2063,  2064,  2069,  2072,  2076,  2076,  2081,  2084,  2085,
    2089,  2090,  2095,  2096,  2100,  2101,  2105,  2106,  2111,  2113,
    2118,  2119,  2120,  2121,  2122,  2123,  2124,  2126,  2129,  2131,
    2135,  2136,  2137,  2138,  2139,  2141,  2143,  2145,  2149,  2150,
    2151,  2155,  2158,  2161,  2164,  2168,  2172,  2179,  2183,  2190,
    2191,  2196,  2198,  2199,  2202,  2203,  2206,  2207,  2211,  2212,
    2216,  2217,  2218,  2219,  2221,  2224,  2227,  2228,  2229,  2231,
    2233,  2237,  2238,  2239,  2241,  2242,  2243,  2247,  2249,  2252,
    2254,  2255,  2256,  2257,  2260,  2262,  2263,  2267,  2269,  2272,
    2274,  2275,  2276,  2280,  2282,  2285,  2288,  2290,  2292,  2296,
    2297,  2299,  2300,  2306,  2307,  2309,  2311,  2313,  2315,  2318,
    2319,  2320,  2324,  2325,  2326,  2327,  2328,  2329,  2330,  2334,
    2335,  2339,  2347,  2349,  2353,  2356,  2362,  2363,  2369,  2370,
    2377,  2380,  2384,  2387,  2392,  2393,  2394,  2395,  2399,  2400,
    2404,  2406,  2407,  2409,  2413,  2419,  2421,  2425,  2428,  2431,
    2439,  2442,  2445,  2446,  2449,  2450,  2453,  2457,  2461,  2467,
    2475,  2476
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
  "xhp_children_decl_tag", "method_body", "variable_modifiers",
  "method_modifiers", "non_empty_member_modifiers", "member_modifier",
  "parameter_modifiers", "parameter_modifier",
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
     285,   285,   285,   285,   285,   285,   286,   286,   287,   287,
     287,   288,   288,   288,   288,   289,   289,   290,   290,   290,
     290,   290,   291,   292,   292,   293,   293,   294,   294,   295,
     296,   297,   298,   299,   300,   300,   300,   301,   301,   301,
     301,   301,   301,   301,   301,   301,   301,   301,   301,   301,
     301,   301,   301,   301,   301,   301,   301,   301,   301,   301,
     301,   301,   301,   301,   301,   301,   301,   301,   301,   301,
     301,   301,   301,   301,   301,   301,   301,   301,   301,   301,
     301,   301,   301,   301,   301,   301,   301,   301,   301,   301,
     301,   301,   301,   301,   301,   301,   301,   301,   301,   301,
     301,   301,   301,   302,   301,   303,   301,   301,   304,   305,
     305,   306,   306,   307,   307,   308,   308,   309,   310,   310,
     311,   312,   313,   313,   314,   314,   314,   315,   315,   316,
     316,   316,   316,   317,   318,   318,   319,   319,   320,   320,
     321,   321,   322,   323,   323,   324,   324,   324,   325,   325,
     325,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   327,   328,   328,   329,   329,   329,   330,   330,   330,
     331,   331,   331,   332,   332,   332,   333,   333,   334,   334,
     334,   334,   334,   334,   334,   334,   334,   334,   334,   334,
     334,   334,   335,   335,   335,   335,   335,   335,   335,   335,
     335,   335,   336,   336,   337,   337,   337,   337,   337,   337,
     337,   338,   338,   339,   339,   340,   340,   341,   341,   341,
     341,   342,   342,   342,   342,   342,   343,   343,   343,   344,
     344,   344,   344,   344,   344,   344,   344,   345,   345,   346,
     346,   346,   346,   347,   347,   348,   348,   349,   349,   350,
     350,   351,   351,   352,   352,   354,   353,   355,   356,   356,
     357,   357,   358,   358,   359,   359,   360,   360,   361,   361,
     362,   362,   362,   362,   362,   362,   362,   362,   362,   362,
     363,   363,   363,   363,   363,   363,   363,   363,   364,   364,
     364,   365,   365,   365,   365,   365,   365,   366,   366,   367,
     367,   368,   368,   368,   369,   369,   370,   370,   371,   371,
     372,   372,   372,   372,   372,   372,   373,   373,   373,   373,
     373,   374,   374,   374,   374,   374,   374,   375,   375,   376,
     376,   376,   376,   376,   376,   376,   376,   377,   377,   378,
     378,   378,   378,   379,   379,   380,   380,   380,   380,   381,
     381,   381,   381,   382,   382,   382,   382,   382,   382,   383,
     383,   383,   384,   384,   384,   384,   384,   384,   384,   385,
     385,   386,   387,   387,   388,   388,   389,   389,   390,   390,
     391,   391,   392,   392,   393,   393,   393,   393,   394,   394,
     395,   395,   395,   395,   396,   397,   397,   398,   398,   399,
     400,   400,   400,   400,   400,   400,   400,   400,   400,   400,
     401,   401
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
       0,     5,     1,     0,     1,     0,     1,     0,    11,     0,
      12,     0,    13,     0,     8,     0,     9,     0,     7,     0,
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
       1,     1,     3,     1,     1,     1,     0,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     1,     1,
       1,     3,     5,     1,     3,     5,     4,     3,     3,     2,
       1,     1,     3,     3,     1,     1,     0,     2,     4,     3,
       6,     2,     3,     6,     1,     1,     1,     6,     3,     4,
       6,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     2,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     5,     4,     1,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     1,     1,
       1,     3,     2,     0,    11,     0,    12,     1,     1,     5,
       3,     5,     3,     2,     0,     2,     0,     4,     4,     3,
       4,     4,     4,     4,     1,     1,     3,     5,     0,     3,
       4,     1,     2,     4,     2,     6,     0,     1,     4,     0,
       2,     0,     1,     1,     3,     1,     3,     1,     1,     3,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     4,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     1,     0,     0,     1,     1,     3,     0,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     2,     1,     1,     2,     2,     4,     4,     3,     4,
       1,     1,     3,     3,     1,     1,     1,     1,     3,     3,
       3,     2,     0,     1,     0,     1,     0,     5,     3,     3,
       1,     1,     1,     1,     3,     2,     1,     1,     1,     1,
       1,     2,     2,     4,     4,     3,     4,     2,     0,     5,
       3,     3,     1,     3,     1,     2,     0,     5,     3,     2,
       0,     3,     0,     4,     2,     0,     3,     3,     1,     0,
       1,     2,     2,     4,     3,     3,     2,     4,     2,     4,
       1,     1,     1,     1,     1,     2,     4,     3,     4,     3,
       1,     1,     1,     1,     2,     4,     4,     3,     1,     1,
       3,     7,     6,     8,     9,     8,    10,     7,     6,     1,
       2,     4,     4,     1,     1,     4,     1,     0,     1,     2,
       1,     1,     2,     4,     3,     3,     0,     1,     2,     4,
       3,     2,     3,     6,     0,     1,     4,     2,     0,     5,
       3,     3,     1,     6,     4,     4,     2,     2,     0,     5,
       3,     3,     1,     2,     0,     5,     3,     3,     1,     2,
       2,     1,     2,     1,     4,     3,     3,     6,     3,     1,
       1,     1,     4,     4,     2,     2,     4,     2,     2,     1,
       3,     3,     0,     2,     5,     6,     1,     2,     1,     4,
       3,     0,     1,     3,     3,     1,     1,     0,     0,     2,
       3,     1,     5,     3,     3,     3,     1,     2,     0,     4,
       2,     2,     1,     1,     1,     4,     6,     1,     8,     5,
       1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   595,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   668,     0,   656,   512,
       0,   518,   519,    19,   545,   644,    71,   520,     0,    53,
       0,     0,     0,     0,     0,     0,     0,     0,    96,     0,
       0,     0,     0,     0,     0,   269,   270,   271,   274,   273,
     272,     0,     0,     0,     0,   119,     0,     0,     0,   524,
     526,   527,   521,   522,     0,     0,   528,   523,     0,     0,
     503,    20,    21,    22,    24,    23,     0,   525,     0,     0,
       0,     0,   529,     0,   275,    25,     0,    70,    43,   648,
     513,     0,     0,     4,    32,    34,    37,   544,     0,   502,
       0,     6,    95,     7,     8,     9,     0,     0,   267,   306,
       0,     0,     0,     0,     0,     0,     0,   304,   370,   369,
     291,   377,     0,   290,   611,   504,     0,   547,   368,   266,
     614,   305,     0,     0,   612,   613,   610,   639,   643,     0,
     358,   546,    10,   274,   273,   272,     0,     0,    32,    95,
       0,   708,   305,   707,     0,   705,   704,   372,     0,     0,
     342,   343,   344,   345,   367,   365,   364,   363,   362,   361,
     360,   359,   505,     0,   721,   504,     0,   325,   323,     0,
     672,     0,   554,   289,   508,     0,   721,   507,     0,   517,
     651,   650,   509,     0,     0,   511,   366,     0,     0,     0,
     294,     0,    51,   296,     0,     0,    57,    59,     0,     0,
      61,     0,     0,     0,   743,   747,     0,     0,    32,   742,
       0,   744,     0,    63,     0,     0,    43,     0,     0,     0,
      27,    28,   196,     0,     0,   195,   121,   120,   201,     0,
       0,     0,     0,     0,   718,   107,   117,   664,   668,   693,
       0,   531,     0,     0,     0,   691,     0,    15,     0,    36,
       0,   297,   111,   118,   409,   384,     0,   712,   301,   306,
       0,   304,   305,     0,     0,   514,     0,   515,     0,     0,
       0,    87,     0,     0,    39,   189,     0,    18,    94,     0,
     116,   103,   115,   272,    95,   268,    80,    81,    82,    83,
      84,    86,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   656,    79,   647,
     647,   678,     0,     0,     0,     0,     0,   265,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     324,   322,     0,   615,   600,   647,     0,   606,   189,   647,
       0,   649,   640,   664,     0,    95,     0,     0,   597,   592,
     554,     0,     0,     0,     0,   676,     0,   389,   553,   667,
       0,     0,    39,     0,   189,   288,     0,   652,   600,   608,
     510,     0,    43,   157,     0,    68,     0,     0,   295,     0,
       0,     0,     0,     0,    60,    78,    62,   740,   741,     0,
     738,     0,     0,   722,     0,   717,    85,    64,     0,    77,
      29,     0,    17,     0,     0,   197,     0,    66,     0,     0,
      67,   709,     0,     0,     0,     0,     0,   127,     0,   665,
       0,     0,     0,     0,   530,   692,   545,     0,     0,   690,
     550,   689,    35,     5,    12,    13,    65,     0,   125,     0,
       0,   378,     0,   554,     0,     0,     0,     0,   287,   355,
     619,    48,    42,    44,    45,    46,    47,     0,   371,   548,
     549,    33,     0,     0,     0,   556,   190,     0,   373,    97,
     123,     0,   328,   330,   329,     0,     0,   326,   327,   331,
     333,   332,   347,   346,   349,   348,   350,   352,   353,   351,
     341,   340,   335,   336,   334,   337,   338,   339,   354,   646,
       0,     0,   682,     0,   554,   711,   617,   639,   109,   113,
     105,    95,     0,     0,   299,   302,   308,   321,   320,   319,
     318,   317,   316,   315,   314,   313,   312,   311,     0,   602,
     601,     0,     0,     0,     0,     0,     0,     0,   706,   590,
     594,   553,   596,     0,     0,   721,     0,   671,     0,   670,
       0,   655,   654,     0,     0,   602,   601,   292,   159,   161,
     293,     0,    43,   141,    52,   296,     0,     0,     0,     0,
     153,   153,    58,     0,     0,   736,   554,     0,   727,     0,
       0,     0,   552,     0,     0,   503,     0,    25,    37,   533,
     502,   541,     0,   532,    41,   540,     0,     0,    26,    30,
       0,   194,   202,   199,     0,     0,   702,   703,    11,   731,
       0,     0,     0,   664,   661,     0,   388,   701,   700,   699,
       0,   695,     0,   696,   698,     0,     5,   298,     0,     0,
     403,   404,   412,   411,     0,     0,   553,   383,   387,     0,
     713,     0,     0,   616,   600,   607,   645,     0,   720,   191,
     501,   555,   188,     0,   599,     0,     0,   125,   375,    99,
     357,     0,   392,   393,     0,   390,   553,   677,     0,   189,
     127,   125,   123,     0,   656,   309,     0,     0,   189,   604,
     605,   618,   641,   642,     0,     0,     0,   578,   561,   562,
     563,     0,     0,     0,    25,   570,   569,   584,   554,     0,
     592,   675,   674,     0,   653,   600,   609,   516,     0,   163,
       0,     0,    49,     0,     0,     0,     0,     0,   133,   134,
     145,     0,    43,   143,    74,   153,     0,   153,     0,     0,
     745,     0,   553,   737,   739,   726,   725,     0,   723,   534,
     535,   560,     0,   554,   552,     0,     0,   386,   552,     0,
     684,     0,     0,    76,    31,   198,     0,   710,    69,     0,
       0,   719,   126,   128,   204,     0,     0,   662,     0,   694,
       0,    16,     0,   124,   204,     0,     0,   380,     0,   714,
       0,     0,   602,   601,   723,     0,   192,    40,   178,     0,
     556,   598,   751,   599,   122,     0,   599,     0,   356,   681,
     680,   189,     0,     0,     0,   125,   101,   517,   603,   189,
       0,     0,   566,   567,   568,   571,   572,   582,     0,   554,
     578,     0,   565,   586,   578,   553,   589,   591,   593,     0,
     669,   603,     0,     0,     0,     0,   160,    54,     0,   296,
     135,   664,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   147,     0,   734,   735,     0,     0,   749,     0,   538,
     553,   551,     0,   543,     0,   554,     0,     0,   542,   688,
       0,   554,     0,    43,   200,   733,   730,     0,   266,   666,
     664,   300,   303,   307,     0,    14,   266,   415,     0,     0,
     417,   410,   413,     0,   408,     0,   715,     0,     0,   189,
     193,   728,   599,   177,   750,     0,     0,   204,     0,   599,
       0,     0,   638,   204,   204,     0,     0,   310,   189,     0,
     632,     0,   575,   553,   577,     0,   564,     0,     0,   554,
       0,   583,   673,     0,    43,     0,   156,   142,     0,     0,
     132,    72,   146,     0,     0,   149,     0,   154,   155,    43,
     148,   746,   724,     0,   559,   558,   536,     0,   553,   385,
     539,   537,     0,   391,   553,   683,     0,     0,     0,   129,
       0,     0,   264,     0,     0,     0,   108,   203,   205,     0,
     263,     0,   266,     0,   697,   112,   406,     0,     0,   379,
     603,   189,     0,     0,   398,   176,   751,     0,   180,   728,
     266,   728,     0,   679,   637,   266,   266,   204,   599,     0,
     631,   581,   580,   573,     0,   576,   553,   585,   574,    43,
     162,    50,    55,   136,     0,   144,   150,    43,   152,     0,
       0,   382,     0,   687,   686,     0,    90,   732,     0,     0,
     130,   233,   231,   503,    24,     0,   227,     0,   232,   243,
       0,   241,   246,     0,   245,     0,   244,     0,    95,   207,
       0,   209,     0,   663,   407,   405,   416,   414,   189,     0,
     635,   729,     0,     0,     0,   181,     0,     0,   104,   398,
     728,   110,   114,   266,     0,   633,     0,   588,     0,   158,
       0,    43,   139,    73,   151,   748,   557,     0,     0,     0,
      91,     0,     0,   217,   221,     0,     0,   214,   467,   466,
     463,   465,   464,   484,   486,   485,   455,   445,   461,   460,
     422,   432,   433,   435,   434,   454,   438,   436,   437,   439,
     440,   441,   442,   443,   444,   446,   447,   448,   449,   450,
     451,   453,   452,   423,   424,   425,   428,   429,   431,   469,
     470,   479,   478,   477,   476,   475,   474,   462,   481,   471,
     472,   473,   456,   457,   458,   459,   482,   483,   487,   489,
     488,   490,   491,   468,   493,   492,   426,   495,   497,   496,
     430,   500,   498,   499,   494,   427,   480,   421,   238,   418,
       0,   215,   259,   260,   258,   251,     0,   252,   216,   283,
       0,     0,     0,     0,    95,     0,   634,     0,    43,     0,
     184,     0,   183,    43,     0,     0,   106,   728,   579,     0,
      43,   137,    56,     0,   381,   685,    43,   286,   131,     0,
       0,   235,   228,     0,     0,     0,   240,   242,     0,     0,
     247,   254,   255,   253,     0,     0,   206,     0,     0,     0,
       0,   636,     0,   401,   556,     0,   185,     0,   182,     0,
      43,    43,     0,   587,     0,     0,     0,   218,    32,     0,
     219,   220,     0,     0,   234,   237,   419,   420,     0,   229,
     256,   257,   249,   250,   248,   284,   281,   210,   208,   285,
       0,   402,   555,     0,   374,     0,   187,    98,     0,     0,
      43,     0,   140,    89,     0,   266,   236,   239,     0,   599,
     212,     0,   399,   397,   186,   376,   100,     0,   138,    93,
     225,     0,   265,   282,   166,     0,   556,   277,   599,   400,
     102,     0,    92,    75,     0,     0,   224,   728,   277,   165,
     278,   279,   280,   751,   276,     0,     0,     0,   223,     0,
     164,   599,     0,   728,     0,   222,   261,    43,   211,   751,
       0,   168,     0,     0,     0,     0,   169,     0,   213,     0,
     262,     0,   172,     0,   171,    43,   173,     0,   170,     0,
       0,   175,    88,   174
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   103,   656,   463,   158,   239,   240,
     105,   106,   107,   108,   109,   110,   283,   482,   483,   407,
     211,  1120,   413,  1054,  1349,   783,  1130,  1363,   299,   159,
     484,   685,   827,   946,   485,   500,   702,   447,   700,   486,
     468,   701,   301,   255,   272,   116,   687,   659,   642,   792,
    1069,   873,   748,  1252,  1123,   594,   754,   412,   602,   756,
     979,   589,   739,   742,   864,  1355,  1356,   819,   820,   494,
     495,   244,   245,   249,   908,  1007,  1087,  1232,  1339,  1358,
    1259,  1300,  1301,  1302,  1075,  1076,  1077,  1260,  1266,  1309,
    1080,  1081,  1085,  1225,  1226,  1227,  1388,  1008,  1009,   160,
     118,  1373,  1374,  1230,  1011,   119,   205,   408,   409,   120,
     121,   122,   123,   124,   125,   126,   127,   684,   826,   472,
     473,   895,   474,   896,   128,   129,   130,   621,   131,   132,
    1103,  1284,   133,   469,  1095,   470,   805,   664,   924,   921,
    1218,  1219,   134,   135,   136,   199,   206,   286,   395,   137,
     771,   625,   138,   772,   389,   682,   773,   726,   845,   847,
     848,   849,   728,   958,   959,   729,   570,   380,   168,   169,
     139,   822,   363,   364,   675,   140,   200,   162,   142,   143,
     144,   145,   146,   147,   148,   530,   149,   202,   203,   450,
     191,   192,   533,   534,   900,   901,   264,   265,   650,   150,
     442,   151,   477,   152,   230,   256,   294,   422,   767,  1024,
     640,   605,   606,   607,   231,   232,   935
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -978
static const yytype_int16 yypact[] =
{
    -978,   101,  -978,  -978,  3712,  9860,  9860,   -42,  9860,  9860,
    9860,  -978,  9860,  9860,  9860,  9860,  9860,  9860,  9860,  9860,
    9860,  9860,  9860,  9860,  2386,  2386,  7940,  9860,  2625,   -33,
     -27,  -978,  -978,  -978,  -978,  -978,  -978,  -978,  9860,  -978,
     -27,   -25,   -19,   144,   -27,  8098,  1162,  8256,  -978,  1628,
    7624,    17,  9860,  1077,   102,  -978,  -978,  -978,   225,   237,
     -10,   181,   193,   200,   202,  -978,  1162,   245,   252,  -978,
    -978,  -978,  -978,  -978,   338,   536,  -978,  -978,  1162,  8414,
    -978,  -978,  -978,  -978,  -978,  -978,  1162,  -978,   217,   266,
    1162,  1162,  -978,  9860,  -978,  -978,  9860,  -978,  -978,   257,
      20,   410,   410,  -978,   417,   313,   -23,  -978,   270,  -978,
      42,  -978,   429,  -978,  -978,  -978,  1029,   682,  -978,  -978,
     285,   287,   292,   294,   295,   304, 10543,  -978,  -978,   443,
    -978,   444,   446,  -978,    22,   312,   368,  -978,  -978,  1259,
      -1,  1819,    44,   336,    48,    76,   339,    23,  -978,   177,
    -978,   452,  -978,  -978,  -978,   376,   342,   377,  -978,   429,
     682, 11183,  2904, 11183,  9860, 11183, 11183, 10898,   480,  1162,
    -978,  -978,   475,  -978,  -978,  -978,  -978,  -978,  -978,  -978,
    -978,  -978,  -978,  1838,   363,  -978,   387,   407,   407,  2386,
   10830,   350,   529,  -978,   376,  1838,   363,   396,   404,   370,
      90,  -978,   426,    44,  8572,  -978,  -978,  9860,  6422,    50,
   11183,  7308,  -978,  9860,  9860,  1162,  -978,  -978, 10584,   373,
    -978, 10625,  1628,  1628,   389,  -978,   378,  1460,   547,  -978,
     553,  -978,  1162,  -978,   392, 10666,  -978, 10707,  1162,    58,
    -978,    36,  -978,  2303,    59,  -978,  -978,  -978,   559,    60,
    2386,  2386,  2386,   399,   421,  -978,  -978,  2497,  7940,    55,
      -3,  -978, 10018,  2386,   348,  -978,  1162,  -978,   288,   313,
     405, 10871,  -978,  -978,  -978,   508,   576,   500, 11183,   423,
   11183,   424,   710,  3870,  9860,   261,   415,   420,   261,   301,
     250,  -978,  1162,  1628,   422,  8730,  1628,  -978,  -978,   806,
    -978,  -978,  -978,  -978,   429,  -978,  -978,  -978,  -978,  -978,
    -978,  -978,  9860,  9860,  9860,  8912,  9860,  9860,  9860,  9860,
    9860,  9860,  9860,  9860,  9860,  9860,  9860,  9860,  9860,  9860,
    9860,  9860,  9860,  9860,  9860,  9860,  9860,  2625,  -978,  9860,
    9860,  9860,   846,  1162,  1162,  1029,   507,  1343,  7466,  9860,
    9860,  9860,  9860,  9860,  9860,  9860,  9860,  9860,  9860,  9860,
    -978,  -978,   390,  -978,   106,  9860,  9860,  -978,  8730,  9860,
    9860,   257,   111,  2497,   435,   429,  9070, 10748,  -978,   436,
     605,  1838,   438,   183,   846,   407,  9228,  -978,  9386,  -978,
     440,   188,  -978,   226,  8730,  -978,   686,  -978,   112,  -978,
    -978, 10789,  -978,  -978,  9860,  -978,   535,  6604,   614,   448,
   11076,   613,    47,    39,  -978,  -978,  -978,  -978,  -978,  1628,
     558,   483,   631,  -978,  2997,  -978,  -978,  -978,  4028,  -978,
     253,  1077,  -978,  1162,  9860,   407,   102,  -978,  2997,   586,
    -978,   407,    80,    82,   210,   485,  1162,   544,   492,   407,
      87,   493,  1315,  1162,  -978,  -978,   611,  2228,   -11,  -978,
    -978,  -978,   313,  -978,  -978,  -978,  -978,  9860,   563,   518,
     199,  -978,   564,   683,   514,  1628,  1628,   680,    24,   633,
     123,  -978,  -978,  -978,  -978,  -978,  -978,  2905,  -978,  -978,
    -978,  -978,    43,  2386,   532,   693, 11183,   690,  -978,  -978,
     585,   918, 11223, 11260, 10898,  9860, 11142, 11282,  3175,  2971,
    7516,  3197,  7672,  7672,  7672,  7672,  2685,  2685,  2685,  2685,
     603,   603,   719,   719,   719,   475,   475,   475,  -978, 11183,
     523,   534, 10939,   539,   711,   191,   560,   111,  -978,  -978,
    -978,   429,  2289,  9860,  -978,  -978, 10898, 10898, 10898, 10898,
   10898, 10898, 10898, 10898, 10898, 10898, 10898, 10898,  9860,   191,
     561,   540,  2950,   562,   557,  3243,    88,   566,  -978,  1066,
    -978,  1162,  -978,   423,    24,   363,  2386, 11183,  2386, 10980,
      77,   125,  -978,   572,  9860,  -978,  -978,  -978,  6240,   249,
   11183,   -27,  -978,  -978,  -978,  9860,  1783,  2997,  1162,  6786,
     575,   581,  -978,    51,   627,  -978,   752,   591,  1282,  1628,
    2997,  2997,  2997,   598,    41,   621,   601,   606,   296,  -978,
     638,  -978,   604,  -978,  -978,  -978,     1,  1162,  -978,  -978,
   10297,  -978,  -978,   770,  2386,   607,  -978,  -978,  -978,   695,
      84,  1388,   609,  2497,  2540,   782,  -978,  -978,  -978,  -978,
     618,  -978,  9860,  -978,  -978,  3396,  -978, 11183,  1388,   623,
    -978,  -978,  -978,  -978,   790,  9860,   508,  -978,  -978,   628,
    -978,  1628,   744,  -978,   133,  -978,  -978,  1628,  -978,   407,
    -978,  9544,  -978,  2997,    40,   634,  1388,   563,  -978,  -978,
   11099,  9860,  -978,  -978,  9860,  -978,  9860,  -978,   635,  8730,
     544,   563,   585,  1162,  2625,   407, 10338,   639,  8730,  -978,
    -978,   135,  -978,  -978,   795,   522,   522,  1066,  -978,  -978,
    -978,   641,    45,   643,   645,  -978,  -978,  -978,   813,   648,
     436,   407,   407,  9702,  -978,   168,  -978,  -978, 10379,   327,
     -27,  7308,  -978,   650,  4186,   651,  2386,   655,   714,   407,
    -978,   819,  -978,  -978,  -978,  -978,   527,  -978,   228,  1628,
    -978,  1628,   558,  -978,  -978,  -978,   825,   664,   665,  -978,
    -978,   721,   662,   837,  2997,   712,  1162,   508,  2997,  1162,
    2997,   672,   670,  -978,  -978,  -978,  2997,   407,  -978,  1628,
    1162,  -978,   844,  -978,  -978,    94,   678,   407,  7782,  -978,
    2028,  -978,  3554,   844,  -978,   255,   189, 11183,   734,  -978,
     681,  9860,   191,   684,  -978,  2386, 11183,  -978,  -978,   687,
     849,  -978,  1628,    40,  -978,   694,    40,   698, 11099, 11183,
   11035,  8730,   702,   696,   701,   563,  -978,   370,   705,  8730,
     708,  9860,  -978,  -978,  -978,  -978,  -978,   740,   706,   881,
    1066,   753,  -978,   508,  1066,  1066,  -978,  -978,  -978,  2386,
   11183,  -978,   -27,   865,   830,  7308,  -978,  -978,   722,  9860,
     407,  2497,  1783,   724,  2997,  4344,   549,   728,  9860,    26,
     258,  -978,   738,  -978,  -978,  1573,   883,  -978,  2997,  -978,
    2997,  -978,   735,  -978,   788,   904,   739,   745,  -978,   792,
     737,   915,  1388,  -978,  -978,  -978,   835,  1388,  1396,  -978,
    2497,  -978,  -978, 10898,   747,  -978,  1562,  -978,    95,  9860,
    -978,  -978,  -978,  9860,  -978,  9860,  -978, 10420,   756,  8730,
     407,   902,   120,  -978,  -978,    73,   760,  -978,   763,    40,
    9860,   766,  -978,  -978,  -978,   754,   771,  -978,  8730,   769,
    -978,  1066,  -978,  1066,  -978,   772,  -978,   816,   774,   943,
     776,  -978,   407,   930,  -978,   785,  -978,  -978,   787,    99,
    -978,  -978,  -978,   789,   797,  -978,  3312,  -978,  -978,  -978,
    -978,  -978,  -978,  1628,  -978,   842,  -978,  2997,   508,  -978,
    -978,  -978,  2997,  -978,  2997,  -978,   894,  4502,  1628,  -978,
    1628,  1388,  -978,  1126,   826,   755,  -978,  -978,  -978,   507,
     862,    61,  1343,   100,  -978,  -978,   829, 10461, 10502, 11183,
     802,  8730,   803,  1628,   878,  -978,  1628,   910,   971,   902,
    1701,   902,   809, 11183,  -978,  2035,  2136,  -978,    40,   811,
    -978,  -978,   866,  -978,  1066,  -978,   508,  -978,  -978,  -978,
    6240,  -978,  -978,  -978,  6968,  -978,  -978,  -978,  6240,   824,
    2997,  -978,   870,  -978,   880,   828,  -978,  -978,   992,    56,
    -978,  -978,  -978,    63,   831,    66,  -978, 10146,  -978,  -978,
      67,  -978,  -978,   428,  -978,   839,  -978,   939,   429,  -978,
    1628,  -978,   507,  -978,  -978,  -978,  -978,  -978,  8730,   841,
    -978,  -978,   843,   845,    98,  1006,  2997,   852,  -978,   878,
     902,  -978,  -978,  2192,   851,  -978,  1066,  -978,   899,  6240,
    7150,  -978,  -978,  -978,  6240,  -978,  -978,  2997,  2997,   853,
    -978,  2997,  1388,  -978,  -978,  2025,  1126,  -978,  -978,  -978,
    -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,
    -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,
    -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,
    -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,
    -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,
    -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,
    -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,
    -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,   469,  -978,
     826,  -978,  -978,  -978,  -978,  -978,    32,   464,  -978,  1009,
      69,  1162,   939,  1019,   429,   857,  -978,   118,  -978,   961,
    1025,  2997,  -978,  -978,   863,   864,  -978,   902,  -978,  1066,
    -978,  -978,  -978,  4660,  -978,  -978,  -978,  -978,  -978,   382,
      30,  -978,  -978,  2997, 10146, 10146,   993,  -978,   428,   428,
     477,  -978,  -978,  -978,  2997,   970,  -978,   872,    71,  2997,
    1162,  -978,   972,  -978,  1040,  4818,  1038,  2997,  -978,  4976,
    -978,  -978,   874,  -978,  5134,   876,  5292,  -978,   965,   919,
    -978,  -978,   968,  2025,  -978,  -978,  -978,  -978,   912,  -978,
    1043,  -978,  -978,  -978,  -978,  -978,  1061,  -978,  -978,  -978,
     900,  -978,   260,   905,  -978,  2997,  -978,  -978,  5450,  5608,
    -978,   901,  -978,  -978,  1162,  1343,  -978,  -978,  2997,   121,
    -978,  1003,  -978,  -978,  -978,  -978,  -978,  5766,  -978,   178,
     925,  1162,   525,  -978,  -978,   906,  1076,   570,   121,  -978,
    -978,   913,  -978,  -978,  1388,   914,  -978,   902,     7,  -978,
    -978,  -978,  -978,  1628,  -978,   911,  1388,    72,  -978,   -56,
    -978,  1048,   274,   902,  1017,  -978,  -978,  -978,  -978,  1628,
    1021,  1082,   -56,   920,  5924,   276,  1084,  2997,  -978,   923,
    -978,  1030,  1085,  2997,  -978,  -978,  1091,  2997,  -978,  6082,
    2997,  -978,  -978,  -978
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -978,  -978,  -978,  -431,  -978,  -978,  -978,    -4,  -978,   675,
      19,   916,  1982,  -978,  1344,  -978,   -96,  -978,     2,  -978,
    -978,  -978,  -978,  -978,  -978,  -242,  -978,  -978,  -151,    52,
       0,  -978,  -978,  -978,     9,  -978,  -978,  -978,  -978,    11,
    -978,  -978,   768,   775,   773,   975,   419,  -576,   425,   461,
    -240,  -978,   251,  -978,  -978,  -978,  -978,  -978,  -978,  -565,
     150,  -978,  -978,  -978,  -978,  -230,  -978,  -769,  -978,  -321,
    -978,  -978,   697,  -978,  -756,  -978,  -978,  -978,  -978,  -978,
    -978,  -978,  -978,  -978,  -978,    -5,  -978,  -978,  -978,  -978,
    -978,   -88,  -978,   136,  -942,  -978,  -243,  -978,  -136,    27,
    -116,  -218,  -978,   -77,  -978,   -63,   -20,  1115,  -549,  -339,
    -978,  -978,   -48,  -978,  -978,  2495,  1060,  -978,  -978,  -623,
    -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,  -978,
      49,  -978,   352,  -978,  -978,  -978,  -978,  -978,  -978,  -978,
    -978,  -786,  -978,  1219,   119,  -309,  -978,  -978,   324,  1778,
     762,  -978,  -978,  -647,  -355,  -793,  -978,  -978,   447,  -543,
    -665,  -978,  -978,  -978,  -978,  -978,   432,  -978,  -978,  -978,
    -584,  -911,  -161,  -149,  -112,  -978,  -978,    10,  -978,  -978,
    -978,  -978,   -12,  -110,  -978,  -241,  -978,  -978,  -978,  -356,
     907,  -978,  -978,  -978,  -978,  -978,   528,   457,  -978,  -978,
     921,  -978,  -978,  -978,  -291,   -79,  -174,  -264,  -978,  -953,
    -978,   402,  -978,  -978,  -978,  -204,  -977
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -722
static const yytype_int16 yytable[] =
{
     104,   305,   234,   346,   113,   497,   111,   273,   374,   544,
     208,   276,   277,   114,   141,   115,   201,   566,   417,   418,
     212,  1026,   392,   423,   216,   572,   727,   933,   528,   492,
     367,   117,   655,   279,   187,   188,   758,   302,  1303,   372,
    1268,   397,   219,   808,   305,   228,   745,   563,   916,  1104,
     296,   677,   977,   398,   936,   598,   112,   938,   404,   759,
    -623,  1269,   254,   248,  1132,   600,   431,   436,   439,  1090,
     259,  -230,   241,   583,  1136,  1220,  1107,  1275,  1109,  1275,
    1132,    11,   254,  -620,   369,   365,   254,   254,   634,   423,
     634,   399,   790,   259,   268,   644,   644,   269,   285,   531,
     821,     3,   644,   781,  1027,   365,   282,   644,   644,  -621,
     362,   825,   254,  1370,  1371,  1372,   452,  -721,   667,   775,
     382,  1386,  1387,   851,   561,   834,   433,   892,   564,  1239,
    -505,   897,   390,   164,   454,   262,   263,  -622,   365,   293,
     428,  1016,   204,   186,   186,   672,  1028,   198,   207,  1282,
     213,  -657,  -721,   501,   894,  -721,   214,  1245,   262,   263,
     782,    11,    11,  -506,  1380,   379,   347,  -624,   654,   304,
    1032,  1240,   369,  -658,  -629,   242,   453,  -623,   261,   697,
     292,  1030,   852,  -555,  -627,   955,  -660,  1035,  1036,   960,
     876,  1283,   880,   383,  -625,   236,  -626,   818,   396,   385,
    -620,   370,   366,   978,   104,   391,   678,   104,  1270,  1304,
     403,   411,   375,   406,   760,   603,  -179,   601,   141,   297,
     499,   141,   366,   599,   567,   802,  -621,   405,   425,  -659,
     957,   305,   537,  1133,  1134,   432,   437,   440,  1091,   821,
    -230,   274,   821,  1137,  1221,   661,  1276,   791,  1318,  1385,
      35,   763,   537,   435,  -622,   366,   635,   430,   636,   945,
     441,   441,   444,   645,   714,   273,   302,   449,  -657,  1114,
     909,   669,   670,   458,   537,  1053,  1093,  1025,  1354,   104,
    1361,  1113,   243,   537,  -624,   462,   537,   795,   491,   370,
    -658,  1341,   228,   141,  1292,   254,  -555,  -167,  -630,    35,
     545,  -627,   186,  -660,   362,  1390,   588,  1401,   186,   362,
     117,  -625,   961,  -626,   186,   740,   741,   673,   573,   215,
     968,   878,   879,   259,  1012,   201,  1310,  1311,   459,   674,
     536,   362,  1012,  1342,   259,   112,   922,   782,   535,   254,
     254,   254,   246,   627,   766,   662,  -659,  1391,   821,  1402,
     560,   878,   879,   293,   247,   821,   250,   371,   559,   480,
     663,   698,   186,   274,   581,  1062,  -721,   923,   251,   186,
     186,   186,   536,   856,   259,   252,   186,   253,   832,   459,
     575,   582,   186,   449,   586,   707,   637,   840,   262,   263,
     703,   383,   585,   862,   863,   837,  1382,   292,   541,   262,
     263,   698,   917,   104,   423,   768,    99,   881,  1041,   593,
    1042,   259,  1395,   673,  1379,   918,   260,   141,   891,   734,
     257,   259,   689,  1118,   104,   674,   459,   258,  1357,   629,
    1392,   735,   292,   919,   490,   284,  -721,   980,   141,   262,
     263,   275,   639,   291,   392,   295,  1012,  1357,   649,   651,
     241,  1012,  1012,    33,   821,   117,   198,   292,   293,   911,
     298,    33,   306,    35,   307,   464,   465,   810,   736,   308,
    1389,   309,   310,   814,  -721,   261,   262,   263,  1306,  1307,
     112,   311,  1263,   259,   489,   460,   262,   263,   288,  1271,
     341,  1323,   186,   259,   954,  1264,   744,   254,   459,    33,
     186,  1117,  1312,   679,  -394,   339,  1272,   340,   342,  1273,
     941,   368,  1265,  -395,  -628,   969,  -505,   373,   949,  1313,
     378,   266,  1314,   157,   337,   293,    78,   384,   362,  1012,
      81,    82,   387,    83,    84,    85,  -504,   388,    81,    82,
     989,    83,    84,    85,   393,   394,   995,   396,   262,   263,
     415,   419,   705,   420,  1013,   882,    95,   883,   262,   263,
    -716,  1297,   537,  1369,    95,   725,   424,   730,   558,   426,
      99,   743,   438,  1248,  1222,   445,    81,    82,  1223,    83,
      84,    85,   466,   446,   104,   905,   731,   471,   732,   475,
     476,   842,   843,    33,   751,   104,   488,   -38,   141,   478,
     479,   753,    95,  1083,  1047,    48,   749,    33,  1022,   141,
     498,   569,   186,   571,   574,   117,   580,   591,   934,   877,
     878,   879,   404,   784,   836,   595,   597,  1039,   287,   289,
     290,    55,    56,    57,   153,   154,   303,   604,   928,   609,
     112,   974,   878,   879,   787,   331,   332,   333,   334,   335,
     336,   104,   337,   449,   797,   113,   875,   111,   608,   633,
     813,   186,   638,   641,   114,   141,   115,   643,   812,   646,
      81,    82,   652,    83,    84,    85,  1370,  1371,  1372,   660,
     266,   814,   117,   658,    81,    82,   665,    83,    84,    85,
     668,   666,   201,   671,  -396,   186,    95,   186,    94,   254,
    1099,   681,  1366,   683,   686,   692,  1293,   112,   680,  1068,
      95,   844,   844,   725,   267,   186,   693,   455,   695,   696,
     865,   461,   709,   376,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   699,   708,   104,   711,   712,
     104,   688,   455,   866,   461,   455,   461,   461,   737,   761,
     912,   141,   755,   186,   141,   821,   870,    33,   757,    35,
     762,   776,   186,   186,   334,   335,   336,   764,   337,   360,
     361,   117,   893,   774,   821,   898,   777,  1235,   779,  1059,
      48,   778,   780,   786,   788,   789,   906,   794,    55,    56,
      57,   153,   154,   303,  1067,   798,   112,   821,   104,  1233,
     799,   804,   113,   806,   111,   809,  1089,   997,   841,   823,
     831,   114,   141,   115,   839,    33,   850,    35,   853,  1101,
     854,   855,   934,   198,   857,   930,    33,   867,   869,   117,
     871,   362,   874,   885,    81,    82,   872,    83,    84,    85,
     886,   887,   963,   888,   889,   890,   725,   902,   903,   454,
     725,   725,   907,   910,   112,    94,   925,   932,   926,   929,
      95,   104,   951,   931,   584,   186,    99,   966,  1050,   962,
    1082,   104,   937,   939,   943,   141,  1092,    33,   942,   944,
     948,   449,   749,  1058,   950,   141,   480,   222,   952,   953,
     956,   964,    81,    82,   305,    83,    84,    85,   965,   967,
     971,   981,   117,    81,    82,   975,    83,    84,    85,   983,
     987,   986,   988,   223,   992,   990,   993,    33,    95,    35,
     449,   991,   811,   994,    99,   998,  1014,   112,  1023,    95,
    1083,  1021,  1037,    33,   186,  1010,  1029,  1231,  1044,  1031,
     184,   184,  1034,  1010,   196,  1040,  1038,   725,  1043,   725,
    1045,  1046,  1048,  1119,    81,    82,  1049,    83,    84,    85,
    -265,  1124,  1051,  1052,  1060,   196,  1055,  1065,    55,    56,
      57,   153,   154,   303,  1056,  1094,  1079,  1098,   186,  1100,
      95,   498,  1102,  1105,  1106,  1110,   224,  1115,  1116,    33,
     186,   186,  1127,   104,    81,    82,   228,    83,    84,    85,
    1125,  1084,  1128,   157,  1129,  1131,    78,   141,   225,  1135,
      81,    82,  1229,    83,    84,    85,  1228,  1236,  1237,  1241,
      95,  1249,  1274,  1238,   117,  1253,    99,  1247,   226,   186,
    1243,  1256,  1279,  1281,  1286,    94,    95,   227,  1287,   347,
     725,  1290,  1291,  1316,  1308,  1321,   104,  1317,  1322,   112,
     104,  1325,  1330,  1332,   104,  -226,  1122,  1010,  1335,  1334,
     141,  1088,  1010,  1010,   141,  1337,    81,    82,   141,    83,
      84,    85,  1269,  1217,  1338,  1340,  1359,   117,  1348,  1224,
    1364,  1343,  1367,  1280,  1368,   117,   228,  1383,  1376,    11,
    1393,  1378,    95,   688,  1396,  1397,  1399,  1403,  1407,   184,
      33,  1405,   112,  1406,  1410,   184,   628,  1362,   715,   716,
     112,   184,   725,   540,   345,   104,   104,   539,   538,   803,
     104,   835,  1251,   970,  1377,   833,  1057,   717,  1375,   141,
     141,  1262,  1267,   631,   141,   718,   719,    33,   196,   196,
    1010,  1086,  1285,   196,  1234,   720,   117,  1289,    33,  1398,
    1381,   117,  1277,   209,  1294,  1278,   281,   920,  1244,   184,
    1296,   947,   858,   846,   884,   451,   184,   184,   184,   934,
       0,   112,   443,   184,     0,   300,   112,    81,    82,   184,
      83,    84,    85,     0,     0,   934,   624,     0,     0,     0,
     721,     0,     0,     0,  1328,  1329,     0,    33,     0,  1351,
     632,  1320,   722,    95,     0,     0,     0,     0,     0,   196,
       0,     0,   196,     0,    81,    82,     0,    83,    84,    85,
       0,   238,     0,     0,     0,    81,    82,   254,    83,    84,
      85,     0,   723,    33,  1347,     0,   305,     0,  1071,     0,
     724,     0,     0,   185,   185,   725,     0,   197,     0,   104,
    1072,    95,     0,   196,     0,  1298,     0,     0,     0,     0,
    1217,  1217,     0,   141,  1224,  1224,     0,   157,     0,     0,
      78,     0,  1073,     0,    81,    82,   254,    83,  1074,    85,
     117,   104,     0,     0,     0,   104,     0,     0,     0,   184,
     104,  1394,   104,     0,     0,   141,     0,   184,     0,   141,
      95,     0,     0,     0,   141,   112,   141,   222,     0,  1409,
      81,    82,   117,    83,    84,    85,   117,     0,     0,     0,
       0,   117,     0,   117,   104,   104,     0,     0,     0,     0,
    1350,     0,     0,   223,     0,   196,    95,   112,   141,   141,
     618,   112,     0,   104,     0,     0,   112,  1365,   112,     0,
       0,     0,     0,    33,   618,   117,   117,   141,     0,   750,
       0,     0,  1352,     0,     0,    55,    56,    57,    58,    59,
     303,     0,   769,   770,   117,     0,    65,   343,     0,     0,
     112,   112,     0,     0,     0,     0,    33,     0,   647,   648,
     104,   196,   196,   229,     0,     0,     0,     0,     0,   112,
       0,     0,     0,     0,   141,   104,   224,     0,   185,   184,
       0,     0,     0,   344,     0,     0,     0,     0,     0,   141,
       0,   117,     0,   157,     0,     0,    78,     0,   225,     0,
      81,    82,    94,    83,    84,    85,   117,    11,     0,   765,
       0,     0,     0,     0,     0,   817,   112,     0,   226,    55,
      56,    57,   153,   154,   303,     0,    95,   227,   184,    33,
       0,   112,   185,    81,    82,     0,    83,    84,    85,   185,
     185,   185,     0,     0,     0,     0,   185,     0,     0,     0,
       0,     0,   185,     0,     0,   222,     0,     0,     0,    95,
       0,     0,   184,     0,   184,  1000,     0,     0,     0,     0,
    1001,     0,    55,    56,    57,   153,   154,   303,  1002,     0,
       0,   223,   184,   618,     0,     0,    94,     0,     0,     0,
       0,     0,     0,     0,   196,   196,   618,   618,   618,   157,
       0,    33,    78,     0,    80,     0,    81,    82,     0,    83,
      84,    85,   899,     0,  1003,  1004,     0,  1005,   904,     0,
     184,     0,     0,     0,     0,     0,   197,   196,   421,   184,
     184,     0,    95,     0,     0,     0,   229,   229,     0,    94,
       0,   229,     0,     0,   196,  1006,     0,     0,     0,     0,
       0,     0,     0,     0,   224,     0,     0,   196,     0,     0,
       0,     0,   185,   196,     0,     0,     0,     0,   222,   618,
       0,   157,   196,    11,    78,     0,   225,     0,    81,    82,
       0,    83,    84,    85,     0,     0,     0,     0,     0,     0,
     196,     0,     0,     0,   223,     0,   226,     0,     0,     0,
       0,     0,     0,     0,    95,   227,   972,   229,     0,     0,
     229,     0,     0,   622,    33,     0,     0,     0,     0,     0,
     984,     0,   985,   222,     0,     0,     0,   622,     0,     0,
       0,  1000,   184,     0,     0,     0,  1001,     0,    55,    56,
      57,   153,   154,   303,  1002,   196,     0,   196,     0,   223,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     618,     0,     0,     0,   618,     0,   618,   224,     0,    33,
       0,     0,   618,     0,     0,   196,     0,     0,     0,     0,
    1003,  1004,   185,  1005,   157,     0,     0,    78,     0,   225,
       0,    81,    82,     0,    83,    84,    85,     0,     0,     0,
     982,   184,     0,     0,     0,    94,     0,     0,   196,   226,
       0,  1015,    11,     0,     0,     0,     0,    95,   227,  1061,
       0,     0,   224,     0,  1063,     0,  1064,     0,     0,     0,
       0,   185,     0,   229,     0,     0,     0,     0,   620,   157,
       0,     0,    78,     0,   225,   184,    81,    82,     0,    83,
      84,    85,   620,     0,     0,     0,     0,   184,   184,     0,
     618,     0,     0,     0,   226,   185,     0,   185,     0,     0,
    1000,   196,    95,   227,   618,  1001,   618,    55,    56,    57,
     153,   154,   303,  1002,   746,   185,   622,     0,   196,   229,
     229,     0,  1126,   196,     0,     0,   184,     0,     0,   622,
     622,   622,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,     0,     0,     0,     0,     0,  1003,
    1004,     0,  1005,   185,    33,     0,    35,     0,     0,     0,
     793,     0,   185,   185,     0,     0,     0,     0,  1242,     0,
       0,     0,     0,     0,    94,     0,     0,   793,   360,   361,
    1108,     0,     0,     0,     0,     0,     0,     0,     0,  1254,
    1255,     0,     0,  1257,   182,     0,     0,     0,     0,   196,
      27,    28,   622,   618,     0,   824,   747,     0,   618,    33,
     618,    35,     0,     0,   196,     0,   196,   196,     0,   196,
       0,     0,     0,   197,   157,     0,   196,    78,     0,    80,
       0,    81,    82,     0,    83,    84,    85,     0,     0,   196,
     362,   620,   196,     0,     0,     0,     0,     0,     0,   182,
       0,     0,   229,   229,   620,   620,   620,    95,   183,     0,
       0,     0,     0,    99,     0,   185,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   618,     0,     0,   157,
       0,     0,    78,     0,    80,     0,    81,    82,     0,    83,
      84,    85,     0,   622,     0,     0,     0,   622,    88,   622,
       0,     0,     0,  1288,     0,   622,   196,     0,     0,     0,
       0,     0,    95,   381,     0,   229,     0,     0,    99,     0,
       0,   229,   618,     0,     0,  1305,     0,   620,     0,     0,
       0,     0,     0,     0,   185,     0,  1315,   312,   313,   314,
       0,  1319,     0,   618,   618,     0,     0,   618,   196,  1326,
       0,     0,   196,   315,     0,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,   336,    11,   337,   185,     0,
       0,     0,     0,     0,     0,     0,     0,  1344,     0,     0,
     185,   185,     0,   622,    31,    32,     0,     0,     0,     0,
    1353,     0,     0,   229,    37,   229,     0,   622,     0,   622,
       0,     0,     0,     0,     0,     0,     0,     0,   620,     0,
       0,   996,   620,     0,   620,     0,   999,     0,     0,   185,
     620,     0,     0,   229,  1000,     0,     0,     0,     0,  1001,
       0,    55,    56,    57,   153,   154,   303,  1002,     0,     0,
      69,    70,    71,    72,    73,     0,     0,   618,     0,  1404,
       0,   614,     0,     0,     0,  1408,   229,    76,    77,  1411,
       0,     0,  1413,     0,     0,   196,     0,    11,     0,   618,
       0,    87,     0,  1003,  1004,     0,  1005,     0,     0,     0,
     618,     0,     0,     0,     0,   618,    92,     0,     0,     0,
       0,     0,   623,   618,     0,     0,   622,     0,    94,     0,
     914,   622,     0,   622,  1111,     0,   623,     0,   620,     0,
    1070,     0,  1078,     0,     0,     0,     0,     0,     0,   229,
       0,     0,   620,    11,   620,  1000,     0,   312,   313,   314,
    1001,   618,    55,    56,    57,   153,   154,   303,  1002,     0,
       0,     0,     0,   315,   618,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,   336,     0,   337,     0,   622,
     196,     0,     0,     0,  1003,  1004,     0,  1005,     0,   196,
       0,  1000,   196,     0,     0,     0,  1001,     0,    55,    56,
      57,   153,   154,   303,  1002,   196,     0,     0,     0,    94,
       0,     0,     0,   618,     0,  1112,     0,     0,     0,   618,
       0,     0,     0,   618,     0,   622,   618,   229,     0,     0,
       0,   620,     0,     0,     0,     0,   620,     0,   620,     0,
    1003,  1004,   229,  1005,   229,     0,   622,   622,     0,     0,
     622,  1258,   704,     0,   229,  1078,     0,     0,     0,     0,
      33,     0,    35,     0,     0,    94,     0,   229,     0,     0,
     229,  1246,     0,     0,    33,   623,    35,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   623,   623,
     623,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     182,     0,     0,     0,   620,     0,   619,   653,     0,     0,
       0,     0,     0,     0,   182,     0,     0,     0,     0,     0,
     619,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     157,     0,     0,    78,   229,    80,     0,    81,    82,     0,
      83,    84,    85,     0,   157,     0,     0,    78,     0,    80,
     620,    81,    82,     0,    83,    84,    85,    33,     0,    35,
     622,   623,     0,    95,   183,     0,     0,     0,     0,    99,
       0,   620,   620,     0,     0,   620,     0,    95,   183,     0,
       0,   434,   622,    99,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   622,     0,     0,     0,   182,   622,     0,
     161,   163,     0,   165,   166,   167,   622,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,     0,
       0,   190,   193,     0,     0,     0,     0,   157,     0,     0,
      78,     0,    80,   210,    81,    82,     0,    83,    84,    85,
     218,     0,   221,     0,   622,   235,     0,   237,     0,     0,
       0,     0,   623,     0,     0,     0,   623,   622,   623,     0,
      95,   183,     0,     0,   623,     0,    99,     0,    33,     0,
      35,     0,     0,     0,   271,     0,     0,     0,     0,   619,
       0,     0,     0,  1070,     0,   620,     0,     0,   278,     0,
       0,   280,   619,   619,   619,  1384,     0,     0,     0,     0,
       0,     0,     0,  1299,     0,     0,     0,   620,   182,     0,
       0,    33,     0,    35,     0,     0,   622,     0,   620,     0,
     448,     0,   622,   620,     0,     0,   622,     0,     0,   622,
       0,   620,     0,     0,     0,     0,     0,     0,   157,     0,
       0,    78,     0,    80,     0,    81,    82,     0,    83,    84,
      85,   182,   623,     0,     0,     0,     0,     0,     0,   377,
       0,     0,     0,   796,     0,   619,   623,     0,   623,   620,
       0,    95,   183,     0,     0,     0,     0,    99,     0,     0,
       0,   157,   620,     0,    78,     0,    80,     0,    81,    82,
       0,    83,    84,    85,     0,     0,    33,     0,    35,   401,
       0,     0,   401,     0,     0,     0,     0,     0,   210,   410,
       0,     0,     0,     0,    95,   183,     0,   229,     0,     0,
      99,  -722,  -722,  -722,  -722,   329,   330,   331,   332,   333,
     334,   335,   336,   229,   337,     0,   194,     0,     0,     0,
       0,   620,     0,     0,     0,     0,     0,   620,     0,     0,
       0,   620,     0,   190,   620,     0,   619,   457,     0,     0,
     619,     0,   619,     0,     0,   623,   157,     0,   619,    78,
     623,    80,   623,    81,    82,     0,    83,    84,    85,   487,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     496,     0,     0,     0,     0,     0,     0,     0,     0,    95,
     195,     0,     0,     0,     0,    99,     0,   502,   503,   504,
     506,   507,   508,   509,   510,   511,   512,   513,   514,   515,
     516,   517,   518,   519,   520,   521,   522,   523,   524,   525,
     526,   527,     0,     0,   529,   529,   532,     0,   623,     0,
       0,     0,     0,   546,   547,   548,   549,   550,   551,   552,
     553,   554,   555,   556,   557,     0,   619,     0,     0,     0,
     529,   562,     0,   496,   529,   565,     0,     0,     0,     0,
     619,   546,   619,     0,     0,     0,     0,     0,     0,     0,
       0,   577,     0,   579,   623,     0,     0,     0,     0,   496,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   590,
       0,     0,     0,     0,     0,   623,   623,     0,     0,   623,
       0,     0,     0,  1261,   312,   313,   314,   376,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   630,
     315,     0,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,     0,   337,     0,     0,     0,     0,   312,
     313,   314,   657,   360,   361,     0,     0,     0,     0,   619,
       0,     0,     0,     0,   619,   315,   619,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,     0,   337,
     690,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,   623,
     337,     0,     0,     0,     0,   362,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   271,   610,
     611,   623,   619,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   623,   706,     0,     0,     0,   623,   612,     0,
       0,     0,     0,     0,     0,   623,    31,    32,    33,     0,
       0,     0,     0,     0,     0,     0,    37,     0,     0,   738,
       0,  1336,     0,     0,   676,     0,     0,     0,   619,     0,
     210,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   623,     0,     0,     0,     0,     0,   619,
     619,     0,     0,   619,     0,     0,   623,     0,     0,     0,
       0,   613,    69,    70,    71,    72,    73,     0,     0,   710,
       0,     0,     0,   614,     0,     0,     0,     0,   157,    76,
      77,    78,     0,   615,     0,    81,    82,   800,    83,    84,
      85,     0,     0,    87,     0,     0,     0,     0,     0,     0,
     807,     0,     0,   616,     0,     0,     0,     0,    92,     0,
       0,   617,     0,     0,     0,   623,   816,     0,     0,     0,
       0,   623,     0,     0,     0,   623,   828,     0,   623,   829,
       0,   830,     0,     0,   496,     0,     0,     0,     0,     0,
       0,     0,     0,   496,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   619,   337,     0,     0,     0,   860,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,   619,   337,     0,     0,     0,
       0,     0,   312,   313,   314,     0,   619,     0,     0,     0,
       0,   619,     0,     0,     0,     0,     0,     0,   315,   619,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,     0,   337,   913,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   927,   619,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     619,   312,   313,   314,     0,     0,   496,     0,     0,     0,
       0,     0,     0,     0,   496,     0,   913,   315,   977,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
       0,   337,     0,     0,   210,     0,     0,     0,     0,     0,
       0,     0,     0,   976,     0,     0,     0,     0,     0,   619,
       0,     0,     0,     0,     0,   619,     0,     0,     0,   619,
       0,     0,   619,     0,     0,     0,     0,     0,     0,     5,
       6,     7,     8,     9,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,  1017,     0,     0,     0,  1018,     0,
    1019,     0,   713,     0,   496,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1033,     0,    11,    12,    13,
       0,     0,     0,   496,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
       0,    36,     0,     0,     0,    37,    38,    39,    40,     0,
      41,     0,    42,     0,    43,     0,     0,    44,     0,   978,
       0,    45,    46,    47,    48,    49,    50,    51,     0,    52,
      53,    54,    55,    56,    57,    58,    59,    60,     0,    61,
      62,    63,    64,    65,    66,     0,   496,     0,     0,    67,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,    74,     0,     0,     0,     0,    75,    76,    77,
      78,    79,    80,     0,    81,    82,     0,    83,    84,    85,
      86,     0,    87,     0,     0,     0,    88,     5,     6,     7,
       8,     9,    89,    90,     0,    91,    10,    92,    93,    94,
      95,    96,     0,    97,    98,   801,    99,   100,     0,   101,
     102,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   496,     0,    11,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,     0,    36,
       0,     0,     0,    37,    38,    39,    40,     0,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,    49,    50,    51,     0,    52,    53,    54,
      55,    56,    57,    58,    59,    60,     0,    61,    62,    63,
      64,    65,    66,     0,     0,     0,     0,    67,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
      74,     0,     0,     0,     0,    75,    76,    77,    78,    79,
      80,     0,    81,    82,     0,    83,    84,    85,    86,     0,
      87,     0,     0,     0,    88,     5,     6,     7,     8,     9,
      89,    90,     0,    91,    10,    92,    93,    94,    95,    96,
       0,    97,    98,   915,    99,   100,     0,   101,   102,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,     0,    41,     0,    42,     0,
      43,     0,     0,    44,     0,     0,     0,    45,    46,    47,
      48,    49,    50,    51,     0,    52,    53,    54,    55,    56,
      57,    58,    59,    60,     0,    61,    62,    63,    64,    65,
      66,     0,     0,     0,     0,    67,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,    74,     0,
       0,     0,     0,    75,    76,    77,    78,    79,    80,     0,
      81,    82,     0,    83,    84,    85,    86,     0,    87,     0,
       0,     0,    88,     5,     6,     7,     8,     9,    89,    90,
       0,    91,    10,    92,    93,    94,    95,    96,     0,    97,
      98,     0,    99,   100,     0,   101,   102,     0,     0,     0,
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
      88,     5,     6,     7,     8,     9,    89,     0,     0,     0,
      10,    92,    93,    94,    95,    96,     0,    97,    98,   481,
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
      93,    94,    95,    96,     0,    97,    98,   626,    99,   100,
       0,   101,   102,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
       0,    36,     0,     0,     0,    37,    38,    39,    40,   868,
      41,     0,    42,     0,    43,     0,     0,    44,     0,     0,
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
      42,     0,    43,   973,     0,    44,     0,     0,     0,    45,
      46,    47,    48,     0,    50,    51,     0,    52,     0,    54,
      55,    56,    57,    58,    59,    60,     0,    61,    62,    63,
       0,    65,    66,     0,     0,     0,     0,    67,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
      74,     0,     0,     0,     0,   157,    76,    77,    78,    79,
      80,     0,    81,    82,     0,    83,    84,    85,    86,     0,
      87,     0,     0,     0,    88,     5,     6,     7,     8,     9,
      89,     0,     0,     0,    10,    92,    93,    94,    95,    96,
       0,    97,    98,     0,    99,   100,     0,   101,   102,     0,
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
      98,  1066,    99,   100,     0,   101,   102,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,     0,    41,     0,    42,  1295,    43,     0,
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
      93,    94,    95,    96,     0,    97,    98,  1324,    99,   100,
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
      95,    96,     0,    97,    98,  1327,    99,   100,     0,   101,
     102,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,     0,    36,
       0,     0,     0,    37,    38,    39,    40,     0,    41,  1331,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,     0,    50,    51,     0,    52,     0,    54,
      55,    56,    57,    58,    59,    60,     0,    61,    62,    63,
       0,    65,    66,     0,     0,     0,     0,    67,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
      74,     0,     0,     0,     0,   157,    76,    77,    78,    79,
      80,     0,    81,    82,     0,    83,    84,    85,    86,     0,
      87,     0,     0,     0,    88,     5,     6,     7,     8,     9,
      89,     0,     0,     0,    10,    92,    93,    94,    95,    96,
       0,    97,    98,     0,    99,   100,     0,   101,   102,     0,
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
      98,  1333,    99,   100,     0,   101,   102,     0,     0,     0,
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
      88,     5,     6,     7,     8,     9,    89,     0,     0,     0,
      10,    92,    93,    94,    95,    96,     0,    97,    98,  1345,
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
      93,    94,    95,    96,     0,    97,    98,  1346,    99,   100,
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
      95,    96,     0,    97,    98,  1360,    99,   100,     0,   101,
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
       0,    97,    98,  1400,    99,   100,     0,   101,   102,     0,
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
      98,  1412,    99,   100,     0,   101,   102,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,  1250,     0,     0,     0,
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
       0,     0,     0,     0,     0,    37,     0,   320,   321,   322,
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
       0,     0,     0,    37,  -722,  -722,  -722,  -722,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
       0,   337,    48,     0,     0,     0,     0,     0,     0,     0,
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
       0,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
     333,   334,   335,   336,     0,   337,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,     0,   337,     0,
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
     329,   330,   331,   332,   333,   334,   335,   336,     0,   337,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
       0,   337
};

static const yytype_int16 yycheck[] =
{
       4,   117,    50,   139,     4,   296,     4,    86,   159,   348,
      30,    90,    91,     4,     4,     4,    28,   373,   222,   223,
      40,   932,   196,   227,    44,   380,   569,   820,   337,   293,
     142,     4,   463,    96,    24,    25,   601,   116,     8,   149,
       8,   202,    46,   666,   160,    49,   595,   368,   804,  1026,
       8,     8,    26,   202,   823,     8,     4,   826,     8,     8,
      61,    29,    66,    73,     8,    26,     8,     8,     8,     8,
      73,     8,    53,   394,     8,     8,  1029,     8,  1031,     8,
       8,    41,    86,    61,    61,    61,    90,    91,     8,   293,
       8,   203,     8,    73,    75,     8,     8,    78,    78,   340,
     684,     0,     8,   102,    31,    61,    96,     8,     8,    61,
     121,   687,   116,   106,   107,   108,    61,   140,   473,    78,
     183,   177,   178,    78,   365,   701,    90,   774,   369,    31,
     140,   778,   195,   175,   137,   138,   139,    61,    61,   162,
     236,    46,   175,    24,    25,   121,    73,    28,   175,    31,
     175,    61,   175,   304,   777,   178,   175,  1110,   138,   139,
     159,    41,    41,   140,   157,   169,   139,    61,   179,   117,
     939,    73,    61,    61,   175,    73,   121,   178,   137,   534,
     144,   937,   137,   176,    61,   850,    61,   943,   944,   854,
     755,    73,   757,   183,    61,   178,    61,   157,   121,   189,
     178,   178,   178,   177,   208,   195,   163,   211,   176,   179,
     208,   215,   160,   211,   163,   419,   176,   178,   208,   177,
     299,   211,   178,   176,   375,   656,   178,   177,   232,    61,
     853,   347,   342,   177,   178,   177,   177,   177,   177,   823,
     177,   146,   826,   177,   177,    46,   177,   163,   177,   177,
      73,   606,   362,   243,   178,   178,   176,   238,   176,   835,
     250,   251,   252,   176,   176,   344,   345,   257,   178,  1038,
     176,   475,   476,   263,   384,   176,   176,   157,   157,   283,
     102,  1037,   180,   393,   178,   266,   396,   643,   292,   178,
     178,    31,   296,   283,  1247,   299,   176,   176,   175,    73,
     348,   178,   183,   178,   121,    31,   402,    31,   189,   121,
     283,   178,   855,   178,   195,    66,    67,   478,   381,   175,
     869,    93,    94,    73,   908,   337,  1268,  1269,    78,   478,
     342,   121,   916,    73,    73,   283,   147,   159,   342,   343,
     344,   345,   117,    90,   608,   146,   178,    73,   932,    73,
     362,    93,    94,   162,   117,   939,   175,   180,   362,   176,
     161,   535,   243,   146,   176,   988,   175,   178,   175,   250,
     251,   252,   384,   728,    73,   175,   257,   175,   699,    78,
     384,   393,   263,   373,   396,   559,   176,   708,   138,   139,
     541,   381,   396,    66,    67,   704,  1373,   144,   346,   138,
     139,   575,   147,   407,   608,   609,   180,   179,   951,   407,
     953,    73,  1389,   574,  1367,   160,    78,   407,   773,   580,
     175,    73,   501,  1046,   428,   574,    78,   175,  1339,   433,
    1383,   580,   144,   178,   184,   178,   140,   179,   428,   138,
     139,   175,   446,    26,   618,   175,  1030,  1358,   452,   453,
     431,  1035,  1036,    71,  1038,   428,   337,   144,   162,   798,
      31,    71,   177,    73,   177,   177,   178,   671,   580,   177,
    1381,   177,   177,   677,   178,   137,   138,   139,  1264,  1265,
     428,   177,    13,    73,   183,   137,   138,   139,    78,    25,
     178,  1284,   373,    73,   849,    26,   592,   501,    78,    71,
     381,  1044,    25,   493,    61,    61,    42,    61,   140,    45,
     831,   175,    43,    61,   175,   871,   140,   175,   839,    42,
      40,   144,    45,   141,    49,   162,   144,   140,   121,  1113,
     148,   149,   182,   151,   152,   153,   140,     8,   148,   149,
     895,   151,   152,   153,   140,   175,   901,   121,   138,   139,
     177,   162,   542,   175,   910,   759,   174,   761,   138,   139,
      13,   179,   672,  1356,   174,   569,    13,   571,   178,   177,
     180,   591,    13,  1116,   146,   176,   148,   149,   150,   151,
     152,   153,   177,   162,   588,   789,   576,    79,   578,    13,
      90,    69,    70,    71,   598,   599,   181,   175,   588,   176,
     176,   599,   174,   175,   959,    98,   596,    71,   929,   599,
     175,   175,   493,     8,   176,   588,   176,    82,   822,    92,
      93,    94,     8,   627,   703,   177,    13,   948,   100,   101,
     102,   106,   107,   108,   109,   110,   111,    79,   812,     8,
     588,    92,    93,    94,   634,    42,    43,    44,    45,    46,
      47,   655,    49,   643,   644,   655,   752,   655,   175,    73,
     672,   542,   177,   119,   655,   655,   655,   175,   672,   176,
     148,   149,    61,   151,   152,   153,   106,   107,   108,   161,
     144,   885,   655,   120,   148,   149,   122,   151,   152,   153,
     176,     8,   704,    13,    61,   576,   174,   578,   173,   703,
    1021,     8,   177,    13,   119,   182,  1249,   655,   176,  1000,
     174,   715,   716,   717,   178,   596,   182,   260,   179,     8,
     740,   264,   182,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,   175,   175,   741,   176,   182,
     744,   175,   285,   741,   287,   288,   289,   290,   176,   122,
     798,   741,   177,   634,   744,  1339,   746,    71,   177,    73,
       8,   140,   643,   644,    45,    46,    47,   176,    49,    59,
      60,   744,   776,   175,  1358,   779,   175,  1098,   140,   983,
      98,   175,   178,    13,   177,    90,   790,   178,   106,   107,
     108,   109,   110,   111,   998,    13,   744,  1381,   802,  1090,
     182,   178,   802,    13,   802,   177,  1010,   903,    13,   175,
     175,   802,   802,   802,   175,    71,   175,    73,   175,  1023,
     175,     8,  1026,   704,   176,   815,    71,   177,   177,   802,
     175,   121,    13,     8,   148,   149,   122,   151,   152,   153,
     176,   176,   862,   122,   182,     8,   850,   175,   178,   137,
     854,   855,     8,   175,   802,   173,   122,     8,   177,   175,
     174,   865,   122,   176,   178,   746,   180,   865,   964,   859,
     115,   875,   178,   175,   178,   865,  1012,    71,   176,   178,
     175,   871,   872,   979,   176,   875,   176,    25,   182,     8,
     137,    26,   148,   149,  1010,   151,   152,   153,    68,   177,
     176,   163,   875,   148,   149,   177,   151,   152,   153,    26,
     122,   176,     8,    51,   122,   176,   179,    71,   174,    73,
     910,   176,   178,     8,   180,    90,   179,   875,    26,   174,
     175,   175,   178,    71,   815,   908,   176,  1088,   122,   176,
      24,    25,   176,   916,    28,   176,   175,   951,   176,   953,
     176,     8,   176,  1049,   148,   149,    26,   151,   152,   153,
      98,  1057,   177,   176,   122,    49,   177,    73,   106,   107,
     108,   109,   110,   111,   177,   146,   150,   175,   859,   176,
     174,   175,   104,    73,    13,   176,   124,   176,   122,    71,
     871,   872,   122,   997,   148,   149,  1000,   151,   152,   153,
     176,  1005,   122,   141,   176,    13,   144,   997,   146,   178,
     148,   149,    73,   151,   152,   153,   177,   176,   175,    13,
     174,   122,    13,   178,   997,  1121,   180,   176,   166,   910,
     178,   178,    13,   176,    73,   173,   174,   175,    13,  1012,
    1044,   178,   178,    73,    51,    73,  1050,   175,     8,   997,
    1054,    13,   178,   177,  1058,    90,  1054,  1030,    90,   140,
    1050,  1009,  1035,  1036,  1054,   153,   148,   149,  1058,   151,
     152,   153,    29,  1077,    13,   175,    73,  1050,   177,  1083,
     155,   176,   176,  1234,     8,  1058,  1090,   176,   175,    41,
      73,   177,   174,   175,    73,    13,   176,    13,    13,   183,
      71,   178,  1050,    73,    13,   189,   431,  1349,    42,    43,
    1058,   195,  1116,   345,   139,  1119,  1120,   344,   343,   658,
    1124,   702,  1120,   872,  1364,   700,   976,    61,  1358,  1119,
    1120,  1136,  1220,   436,  1124,    69,    70,    71,   222,   223,
    1113,  1005,  1238,   227,  1092,    79,  1119,  1243,    71,  1392,
    1368,  1124,  1231,    38,  1250,  1232,    96,   805,  1109,   243,
    1256,   837,   730,   716,   762,   258,   250,   251,   252,  1373,
      -1,  1119,   251,   257,    -1,   146,  1124,   148,   149,   263,
     151,   152,   153,    -1,    -1,  1389,   424,    -1,    -1,    -1,
     124,    -1,    -1,    -1,  1290,  1291,    -1,    71,    -1,  1335,
     438,  1280,   136,   174,    -1,    -1,    -1,    -1,    -1,   293,
      -1,    -1,   296,    -1,   148,   149,    -1,   151,   152,   153,
      -1,   144,    -1,    -1,    -1,   148,   149,  1231,   151,   152,
     153,    -1,   166,    71,  1330,    -1,  1352,    -1,   112,    -1,
     174,    -1,    -1,    24,    25,  1249,    -1,    28,    -1,  1253,
     124,   174,    -1,   337,    -1,  1259,    -1,    -1,    -1,    -1,
    1264,  1265,    -1,  1253,  1268,  1269,    -1,   141,    -1,    -1,
     144,    -1,   146,    -1,   148,   149,  1280,   151,   152,   153,
    1253,  1285,    -1,    -1,    -1,  1289,    -1,    -1,    -1,   373,
    1294,  1387,  1296,    -1,    -1,  1285,    -1,   381,    -1,  1289,
     174,    -1,    -1,    -1,  1294,  1253,  1296,    25,    -1,  1405,
     148,   149,  1285,   151,   152,   153,  1289,    -1,    -1,    -1,
      -1,  1294,    -1,  1296,  1328,  1329,    -1,    -1,    -1,    -1,
    1334,    -1,    -1,    51,    -1,   419,   174,  1285,  1328,  1329,
     424,  1289,    -1,  1347,    -1,    -1,  1294,  1351,  1296,    -1,
      -1,    -1,    -1,    71,   438,  1328,  1329,  1347,    -1,   597,
      -1,    -1,  1335,    -1,    -1,   106,   107,   108,   109,   110,
     111,    -1,   610,   611,  1347,    -1,   117,   118,    -1,    -1,
    1328,  1329,    -1,    -1,    -1,    -1,    71,    -1,    73,    74,
    1394,   475,   476,    49,    -1,    -1,    -1,    -1,    -1,  1347,
      -1,    -1,    -1,    -1,  1394,  1409,   124,    -1,   189,   493,
      -1,    -1,    -1,   154,    -1,    -1,    -1,    -1,    -1,  1409,
      -1,  1394,    -1,   141,    -1,    -1,   144,    -1,   146,    -1,
     148,   149,   173,   151,   152,   153,  1409,    41,    -1,   157,
      -1,    -1,    -1,    -1,    -1,   683,  1394,    -1,   166,   106,
     107,   108,   109,   110,   111,    -1,   174,   175,   542,    71,
      -1,  1409,   243,   148,   149,    -1,   151,   152,   153,   250,
     251,   252,    -1,    -1,    -1,    -1,   257,    -1,    -1,    -1,
      -1,    -1,   263,    -1,    -1,    25,    -1,    -1,    -1,   174,
      -1,    -1,   576,    -1,   578,    99,    -1,    -1,    -1,    -1,
     104,    -1,   106,   107,   108,   109,   110,   111,   112,    -1,
      -1,    51,   596,   597,    -1,    -1,   173,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   608,   609,   610,   611,   612,   141,
      -1,    71,   144,    -1,   146,    -1,   148,   149,    -1,   151,
     152,   153,   780,    -1,   148,   149,    -1,   151,   786,    -1,
     634,    -1,    -1,    -1,    -1,    -1,   337,   641,    98,   643,
     644,    -1,   174,    -1,    -1,    -1,   222,   223,    -1,   173,
      -1,   227,    -1,    -1,   658,   179,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,    -1,    -1,   671,    -1,    -1,
      -1,    -1,   373,   677,    -1,    -1,    -1,    -1,    25,   683,
      -1,   141,   686,    41,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,    -1,    -1,    -1,    -1,
     704,    -1,    -1,    -1,    51,    -1,   166,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   174,   175,   874,   293,    -1,    -1,
     296,    -1,    -1,   424,    71,    -1,    -1,    -1,    -1,    -1,
     888,    -1,   890,    25,    -1,    -1,    -1,   438,    -1,    -1,
      -1,    99,   746,    -1,    -1,    -1,   104,    -1,   106,   107,
     108,   109,   110,   111,   112,   759,    -1,   761,    -1,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     774,    -1,    -1,    -1,   778,    -1,   780,   124,    -1,    71,
      -1,    -1,   786,    -1,    -1,   789,    -1,    -1,    -1,    -1,
     148,   149,   493,   151,   141,    -1,    -1,   144,    -1,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,    -1,
     157,   815,    -1,    -1,    -1,   173,    -1,    -1,   822,   166,
      -1,   179,    41,    -1,    -1,    -1,    -1,   174,   175,   987,
      -1,    -1,   124,    -1,   992,    -1,   994,    -1,    -1,    -1,
      -1,   542,    -1,   419,    -1,    -1,    -1,    -1,   424,   141,
      -1,    -1,   144,    -1,   146,   859,   148,   149,    -1,   151,
     152,   153,   438,    -1,    -1,    -1,    -1,   871,   872,    -1,
     874,    -1,    -1,    -1,   166,   576,    -1,   578,    -1,    -1,
      99,   885,   174,   175,   888,   104,   890,   106,   107,   108,
     109,   110,   111,   112,    31,   596,   597,    -1,   902,   475,
     476,    -1,  1060,   907,    -1,    -1,   910,    -1,    -1,   610,
     611,   612,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    -1,    -1,    -1,    -1,    -1,   148,
     149,    -1,   151,   634,    71,    -1,    73,    -1,    -1,    -1,
     641,    -1,   643,   644,    -1,    -1,    -1,    -1,  1106,    -1,
      -1,    -1,    -1,    -1,   173,    -1,    -1,   658,    59,    60,
     179,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1127,
    1128,    -1,    -1,  1131,   111,    -1,    -1,    -1,    -1,   983,
      62,    63,   683,   987,    -1,   686,   123,    -1,   992,    71,
     994,    73,    -1,    -1,   998,    -1,  1000,  1001,    -1,  1003,
      -1,    -1,    -1,   704,   141,    -1,  1010,   144,    -1,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,  1023,
     121,   597,  1026,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,   608,   609,   610,   611,   612,   174,   175,    -1,
      -1,    -1,    -1,   180,    -1,   746,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1060,    -1,    -1,   141,
      -1,    -1,   144,    -1,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,   774,    -1,    -1,    -1,   778,   160,   780,
      -1,    -1,    -1,  1241,    -1,   786,  1090,    -1,    -1,    -1,
      -1,    -1,   174,   175,    -1,   671,    -1,    -1,   180,    -1,
      -1,   677,  1106,    -1,    -1,  1263,    -1,   683,    -1,    -1,
      -1,    -1,    -1,    -1,   815,    -1,  1274,     9,    10,    11,
      -1,  1279,    -1,  1127,  1128,    -1,    -1,  1131,  1132,  1287,
      -1,    -1,  1136,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    41,    49,   859,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1325,    -1,    -1,
     871,   872,    -1,   874,    69,    70,    -1,    -1,    -1,    -1,
    1338,    -1,    -1,   759,    79,   761,    -1,   888,    -1,   890,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   774,    -1,
      -1,   902,   778,    -1,   780,    -1,   907,    -1,    -1,   910,
     786,    -1,    -1,   789,    99,    -1,    -1,    -1,    -1,   104,
      -1,   106,   107,   108,   109,   110,   111,   112,    -1,    -1,
     125,   126,   127,   128,   129,    -1,    -1,  1241,    -1,  1397,
      -1,   136,    -1,    -1,    -1,  1403,   822,   142,   143,  1407,
      -1,    -1,  1410,    -1,    -1,  1259,    -1,    41,    -1,  1263,
      -1,   156,    -1,   148,   149,    -1,   151,    -1,    -1,    -1,
    1274,    -1,    -1,    -1,    -1,  1279,   171,    -1,    -1,    -1,
      -1,    -1,   424,  1287,    -1,    -1,   987,    -1,   173,    -1,
     182,   992,    -1,   994,   179,    -1,   438,    -1,   874,    -1,
    1001,    -1,  1003,    -1,    -1,    -1,    -1,    -1,    -1,   885,
      -1,    -1,   888,    41,   890,    99,    -1,     9,    10,    11,
     104,  1325,   106,   107,   108,   109,   110,   111,   112,    -1,
      -1,    -1,    -1,    25,  1338,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,  1060,
    1364,    -1,    -1,    -1,   148,   149,    -1,   151,    -1,  1373,
      -1,    99,  1376,    -1,    -1,    -1,   104,    -1,   106,   107,
     108,   109,   110,   111,   112,  1389,    -1,    -1,    -1,   173,
      -1,    -1,    -1,  1397,    -1,   179,    -1,    -1,    -1,  1403,
      -1,    -1,    -1,  1407,    -1,  1106,  1410,   983,    -1,    -1,
      -1,   987,    -1,    -1,    -1,    -1,   992,    -1,   994,    -1,
     148,   149,   998,   151,  1000,    -1,  1127,  1128,    -1,    -1,
    1131,  1132,    63,    -1,  1010,  1136,    -1,    -1,    -1,    -1,
      71,    -1,    73,    -1,    -1,   173,    -1,  1023,    -1,    -1,
    1026,   179,    -1,    -1,    71,   597,    73,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   610,   611,
     612,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,  1060,    -1,   424,   179,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
     438,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     141,    -1,    -1,   144,  1090,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,   141,    -1,    -1,   144,    -1,   146,
    1106,   148,   149,    -1,   151,   152,   153,    71,    -1,    73,
    1241,   683,    -1,   174,   175,    -1,    -1,    -1,    -1,   180,
      -1,  1127,  1128,    -1,    -1,  1131,    -1,   174,   175,    -1,
      -1,   178,  1263,   180,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1274,    -1,    -1,    -1,   111,  1279,    -1,
       5,     6,    -1,     8,     9,    10,  1287,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    -1,
      -1,    26,    27,    -1,    -1,    -1,    -1,   141,    -1,    -1,
     144,    -1,   146,    38,   148,   149,    -1,   151,   152,   153,
      45,    -1,    47,    -1,  1325,    50,    -1,    52,    -1,    -1,
      -1,    -1,   774,    -1,    -1,    -1,   778,  1338,   780,    -1,
     174,   175,    -1,    -1,   786,    -1,   180,    -1,    71,    -1,
      73,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,   597,
      -1,    -1,    -1,  1364,    -1,  1241,    -1,    -1,    93,    -1,
      -1,    96,   610,   611,   612,  1376,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1259,    -1,    -1,    -1,  1263,   111,    -1,
      -1,    71,    -1,    73,    -1,    -1,  1397,    -1,  1274,    -1,
     123,    -1,  1403,  1279,    -1,    -1,  1407,    -1,    -1,  1410,
      -1,  1287,    -1,    -1,    -1,    -1,    -1,    -1,   141,    -1,
      -1,   144,    -1,   146,    -1,   148,   149,    -1,   151,   152,
     153,   111,   874,    -1,    -1,    -1,    -1,    -1,    -1,   164,
      -1,    -1,    -1,   123,    -1,   683,   888,    -1,   890,  1325,
      -1,   174,   175,    -1,    -1,    -1,    -1,   180,    -1,    -1,
      -1,   141,  1338,    -1,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,    71,    -1,    73,   204,
      -1,    -1,   207,    -1,    -1,    -1,    -1,    -1,   213,   214,
      -1,    -1,    -1,    -1,   174,   175,    -1,  1373,    -1,    -1,
     180,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,  1389,    49,    -1,   111,    -1,    -1,    -1,
      -1,  1397,    -1,    -1,    -1,    -1,    -1,  1403,    -1,    -1,
      -1,  1407,    -1,   258,  1410,    -1,   774,   262,    -1,    -1,
     778,    -1,   780,    -1,    -1,   987,   141,    -1,   786,   144,
     992,   146,   994,   148,   149,    -1,   151,   152,   153,   284,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     295,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,
     175,    -1,    -1,    -1,    -1,   180,    -1,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,    -1,    -1,   339,   340,   341,    -1,  1060,    -1,
      -1,    -1,    -1,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,    -1,   874,    -1,    -1,    -1,
     365,   366,    -1,   368,   369,   370,    -1,    -1,    -1,    -1,
     888,   376,   890,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   386,    -1,   388,  1106,    -1,    -1,    -1,    -1,   394,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   404,
      -1,    -1,    -1,    -1,    -1,  1127,  1128,    -1,    -1,  1131,
      -1,    -1,    -1,  1135,     9,    10,    11,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,   434,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,     9,
      10,    11,   467,    59,    60,    -1,    -1,    -1,    -1,   987,
      -1,    -1,    -1,    -1,   992,    25,   994,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
     505,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,  1241,
      49,    -1,    -1,    -1,    -1,   121,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   543,    42,
      43,  1263,  1060,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1274,   558,    -1,    -1,    -1,  1279,    61,    -1,
      -1,    -1,    -1,    -1,    -1,  1287,    69,    70,    71,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,   584,
      -1,  1303,    -1,    -1,   179,    -1,    -1,    -1,  1106,    -1,
     595,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1325,    -1,    -1,    -1,    -1,    -1,  1127,
    1128,    -1,    -1,  1131,    -1,    -1,  1338,    -1,    -1,    -1,
      -1,   124,   125,   126,   127,   128,   129,    -1,    -1,   179,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,    -1,   146,    -1,   148,   149,   652,   151,   152,
     153,    -1,    -1,   156,    -1,    -1,    -1,    -1,    -1,    -1,
     665,    -1,    -1,   166,    -1,    -1,    -1,    -1,   171,    -1,
      -1,   174,    -1,    -1,    -1,  1397,   681,    -1,    -1,    -1,
      -1,  1403,    -1,    -1,    -1,  1407,   691,    -1,  1410,   694,
      -1,   696,    -1,    -1,   699,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   708,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,  1241,    49,    -1,    -1,    -1,   733,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,  1263,    49,    -1,    -1,    -1,
      -1,    -1,     9,    10,    11,    -1,  1274,    -1,    -1,    -1,
      -1,  1279,    -1,    -1,    -1,    -1,    -1,    -1,    25,  1287,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,   798,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   811,  1325,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1338,     9,    10,    11,    -1,    -1,   831,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   839,    -1,   841,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,   869,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   878,    -1,    -1,    -1,    -1,    -1,  1397,
      -1,    -1,    -1,    -1,    -1,  1403,    -1,    -1,    -1,  1407,
      -1,    -1,  1410,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,    -1,
      -1,    -1,    -1,    -1,   919,    -1,    -1,    -1,   923,    -1,
     925,    -1,   179,    -1,   929,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   940,    -1,    41,    42,    43,
      -1,    -1,    -1,   948,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,
      84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,   177,
      -1,    95,    96,    97,    98,    99,   100,   101,    -1,   103,
     104,   105,   106,   107,   108,   109,   110,   111,    -1,   113,
     114,   115,   116,   117,   118,    -1,  1021,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,   146,    -1,   148,   149,    -1,   151,   152,   153,
     154,    -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,
       6,     7,   166,   167,    -1,   169,    12,   171,   172,   173,
     174,   175,    -1,   177,   178,   179,   180,   181,    -1,   183,
     184,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1098,    -1,    41,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,    75,
      -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,
      86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,
      96,    97,    98,    99,   100,   101,    -1,   103,   104,   105,
     106,   107,   108,   109,   110,   111,    -1,   113,   114,   115,
     116,   117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
     146,    -1,   148,   149,    -1,   151,   152,   153,   154,    -1,
     156,    -1,    -1,    -1,   160,     3,     4,     5,     6,     7,
     166,   167,    -1,   169,    12,   171,   172,   173,   174,   175,
      -1,   177,   178,   179,   180,   181,    -1,   183,   184,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    -1,    86,    -1,
      88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    99,   100,   101,    -1,   103,   104,   105,   106,   107,
     108,   109,   110,   111,    -1,   113,   114,   115,   116,   117,
     118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
     148,   149,    -1,   151,   152,   153,   154,    -1,   156,    -1,
      -1,    -1,   160,     3,     4,     5,     6,     7,   166,   167,
      -1,   169,    12,   171,   172,   173,   174,   175,    -1,   177,
     178,    -1,   180,   181,    -1,   183,   184,    -1,    -1,    -1,
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
     160,     3,     4,     5,     6,     7,   166,    -1,    -1,    -1,
      12,   171,   172,   173,   174,   175,    -1,   177,   178,   179,
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
      -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,    83,
      84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,
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
      86,    -1,    88,    89,    -1,    91,    -1,    -1,    -1,    95,
      96,    97,    98,    -1,   100,   101,    -1,   103,    -1,   105,
     106,   107,   108,   109,   110,   111,    -1,   113,   114,   115,
      -1,   117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
     146,    -1,   148,   149,    -1,   151,   152,   153,   154,    -1,
     156,    -1,    -1,    -1,   160,     3,     4,     5,     6,     7,
     166,    -1,    -1,    -1,    12,   171,   172,   173,   174,   175,
      -1,   177,   178,    -1,   180,   181,    -1,   183,   184,    -1,
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
      80,    81,    82,    -1,    84,    -1,    86,    87,    88,    -1,
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
      -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,    85,
      86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,
      96,    97,    98,    -1,   100,   101,    -1,   103,    -1,   105,
     106,   107,   108,   109,   110,   111,    -1,   113,   114,   115,
      -1,   117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
     146,    -1,   148,   149,    -1,   151,   152,   153,   154,    -1,
     156,    -1,    -1,    -1,   160,     3,     4,     5,     6,     7,
     166,    -1,    -1,    -1,    12,   171,   172,   173,   174,   175,
      -1,   177,   178,    -1,   180,   181,    -1,   183,   184,    -1,
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
     160,     3,     4,     5,     6,     7,   166,    -1,    -1,    -1,
      12,   171,   172,   173,   174,   175,    -1,   177,   178,   179,
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
      -1,    -1,    -1,    -1,    -1,    79,    -1,    31,    32,    33,
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
      -1,    -1,    -1,    79,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      44,    45,    46,    47,    -1,    49,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
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
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49
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
     200,   203,   214,   215,   219,   224,   230,   284,   285,   290,
     294,   295,   296,   297,   298,   299,   300,   301,   309,   310,
     311,   313,   314,   317,   327,   328,   329,   334,   337,   355,
     360,   362,   363,   364,   365,   366,   367,   368,   369,   371,
     384,   386,   388,   109,   110,   111,   123,   141,   192,   214,
     284,   300,   362,   300,   175,   300,   300,   300,   353,   354,
     300,   300,   300,   300,   300,   300,   300,   300,   300,   300,
     300,   300,   111,   175,   196,   328,   329,   362,   362,    31,
     300,   375,   376,   300,   111,   175,   196,   328,   329,   330,
     361,   367,   372,   373,   175,   291,   331,   175,   291,   292,
     300,   205,   291,   175,   175,   175,   291,   177,   300,   192,
     177,   300,    25,    51,   124,   146,   166,   175,   192,   199,
     389,   399,   400,   177,   297,   300,   178,   300,   144,   193,
     194,   195,    73,   180,   256,   257,   117,   117,    73,   258,
     175,   175,   175,   175,   192,   228,   390,   175,   175,    73,
      78,   137,   138,   139,   381,   382,   144,   178,   195,   195,
      95,   300,   229,   390,   146,   175,   390,   390,   300,   290,
     300,   301,   362,   201,   178,    78,   332,   381,    78,   381,
     381,    26,   144,   162,   391,   175,     8,   177,    31,   213,
     146,   227,   390,   111,   214,   285,   177,   177,   177,   177,
     177,   177,     9,    10,    11,    25,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    49,   177,    61,
      61,   178,   140,   118,   154,   230,   283,   284,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      59,    60,   121,   357,   358,    61,   178,   359,   175,    61,
     178,   180,   368,   175,   213,   214,    13,   300,    40,   192,
     352,   175,   290,   362,   140,   362,   122,   182,     8,   339,
     290,   362,   391,   140,   175,   333,   121,   357,   358,   359,
     176,   300,    26,   203,     8,   177,   203,   204,   292,   293,
     300,   192,   242,   207,   177,   177,   177,   400,   400,   162,
     175,    98,   392,   400,    13,   192,   177,   177,   201,   177,
     195,     8,   177,    90,   178,   362,     8,   177,    13,     8,
     177,   362,   385,   385,   362,   176,   162,   222,   123,   362,
     374,   375,    61,   121,   137,   382,    72,   300,   362,    78,
     137,   382,   195,   191,   177,   178,   177,   122,   225,   318,
     320,    79,   304,   305,   307,    13,    90,   387,   176,   176,
     176,   179,   202,   203,   215,   219,   224,   300,   181,   183,
     184,   192,   392,    31,   254,   255,   300,   389,   175,   390,
     220,   213,   300,   300,   300,    26,   300,   300,   300,   300,
     300,   300,   300,   300,   300,   300,   300,   300,   300,   300,
     300,   300,   300,   300,   300,   300,   300,   300,   330,   300,
     370,   370,   300,   377,   378,   192,   367,   368,   228,   229,
     227,   214,    31,   145,   294,   297,   300,   300,   300,   300,
     300,   300,   300,   300,   300,   300,   300,   300,   178,   192,
     367,   370,   300,   254,   370,   300,   374,   213,   176,   175,
     351,     8,   339,   290,   176,   192,    31,   300,    31,   300,
     176,   176,   367,   254,   178,   192,   367,   176,   201,   246,
     300,    82,    26,   203,   240,   177,    90,    13,     8,   176,
      26,   178,   243,   400,    79,   396,   397,   398,   175,     8,
      42,    43,    61,   124,   136,   146,   166,   174,   196,   197,
     199,   312,   328,   334,   335,   336,   179,    90,   194,   192,
     300,   257,   335,    73,     8,   176,   176,   176,   177,   192,
     395,   119,   233,   175,     8,   176,   176,    73,    74,   192,
     383,   192,    61,   179,   179,   188,   190,   300,   120,   232,
     161,    46,   146,   161,   322,   122,     8,   339,   176,   400,
     400,    13,   121,   357,   358,   359,   179,     8,   163,   362,
     176,     8,   340,    13,   302,   216,   119,   231,   175,   390,
     300,    26,   182,   182,   122,   179,     8,   339,   391,   175,
     223,   226,   221,   213,    63,   362,   300,   391,   175,   182,
     179,   176,   182,   179,   176,    42,    43,    61,    69,    70,
      79,   124,   136,   166,   174,   192,   342,   344,   347,   350,
     192,   362,   362,   122,   357,   358,   359,   176,   300,   247,
      66,    67,   248,   291,   201,   293,    31,   123,   237,   362,
     335,   192,    26,   203,   241,   177,   244,   177,   244,     8,
     163,   122,     8,   339,   176,   157,   392,   393,   400,   335,
     335,   335,   338,   341,   175,    78,   140,   175,   175,   140,
     178,   102,   159,   210,   192,   179,    13,   362,   177,    90,
       8,   163,   234,   328,   178,   374,   123,   362,    13,   182,
     300,   179,   188,   234,   178,   321,    13,   300,   304,   177,
     400,   178,   192,   367,   400,    31,   300,   335,   157,   252,
     253,   355,   356,   175,   328,   232,   303,   217,   300,   300,
     300,   175,   254,   233,   232,   231,   390,   330,   179,   175,
     254,    13,    69,    70,   192,   343,   343,   344,   345,   346,
     175,    78,   137,   175,   175,     8,   339,   176,   351,    31,
     300,   179,    66,    67,   249,   291,   203,   177,    83,   177,
     362,   175,   122,   236,    13,   201,   244,    92,    93,    94,
     244,   179,   400,   400,   396,     8,   176,   176,   122,   182,
       8,   339,   338,   192,   304,   306,   308,   338,   192,   335,
     379,   380,   175,   178,   335,   400,   192,     8,   259,   176,
     175,   294,   297,   300,   182,   179,   259,   147,   160,   178,
     317,   324,   147,   178,   323,   122,   177,   300,   391,   175,
     362,   176,     8,   340,   400,   401,   252,   178,   252,   175,
     122,   254,   176,   178,   178,   232,   218,   333,   175,   254,
     176,   122,   182,     8,   339,   345,   137,   304,   348,   349,
     345,   344,   362,   291,    26,    68,   203,   177,   293,   374,
     237,   176,   335,    89,    92,   177,   300,    26,   177,   245,
     179,   163,   157,    26,   335,   335,   176,   122,     8,   339,
     176,   176,   122,   179,     8,   339,   328,   201,    90,   328,
      99,   104,   112,   148,   149,   151,   179,   260,   282,   283,
     284,   289,   355,   374,   179,   179,    46,   300,   300,   300,
     179,   175,   254,    26,   394,   157,   356,    31,    73,   176,
     259,   176,   252,   300,   176,   259,   259,   178,   175,   254,
     176,   344,   344,   176,   122,   176,     8,   339,   176,    26,
     201,   177,   176,   176,   208,   177,   177,   245,   201,   400,
     122,   335,   304,   335,   335,    73,   179,   400,   389,   235,
     328,   112,   124,   146,   152,   269,   270,   271,   328,   150,
     275,   276,   115,   175,   192,   277,   278,   261,   214,   400,
       8,   177,   283,   176,   146,   319,   179,   179,   175,   254,
     176,   400,   104,   315,   401,    73,    13,   394,   179,   394,
     176,   179,   179,   259,   252,   176,   122,   344,   304,   201,
     206,    26,   203,   239,   201,   176,   335,   122,   122,   176,
     211,    13,     8,   177,   178,   178,     8,   177,     3,     4,
       5,     6,     7,     9,    10,    11,    12,    49,    62,    63,
      64,    65,    66,    67,    68,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   123,   124,   125,   126,
     127,   128,   129,   141,   142,   143,   145,   154,   155,   156,
     159,   166,   167,   169,   171,   172,   173,   192,   325,   326,
       8,   177,   146,   150,   192,   278,   279,   280,   177,    73,
     288,   213,   262,   389,   214,   254,   176,   175,   178,    31,
      73,    13,   335,   178,   315,   394,   179,   176,   344,   122,
      26,   203,   238,   201,   335,   335,   178,   335,   328,   265,
     272,   334,   270,    13,    26,    43,   273,   276,     8,    29,
     176,    25,    42,    45,    13,     8,   177,   390,   288,    13,
     213,   176,    31,    73,   316,   201,    73,    13,   335,   201,
     178,   178,   394,   344,   201,    87,   201,   179,   192,   199,
     266,   267,   268,     8,   179,   335,   326,   326,    51,   274,
     279,   279,    25,    42,    45,   335,    73,   175,   177,   335,
     390,    73,     8,   340,   179,    13,   335,   179,   201,   201,
     178,    85,   177,   179,   140,    90,   334,   153,    13,   263,
     175,    31,    73,   176,   335,   179,   179,   201,   177,   209,
     192,   283,   284,   335,   157,   250,   251,   356,   264,    73,
     179,   102,   210,   212,   155,   192,   177,   176,     8,   340,
     106,   107,   108,   286,   287,   250,   175,   235,   177,   394,
     157,   286,   401,   176,   328,   177,   177,   178,   281,   356,
      31,    73,   394,    73,   201,   401,    73,    13,   281,   176,
     179,    31,    73,    13,   335,   178,    73,    13,   335,   201,
      13,   335,   179,   335
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
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
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
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (11)]),(yyvsp[(2) - (11)]),(yyvsp[(3) - (11)]),(yyvsp[(6) - (11)]),(yyvsp[(10) - (11)]),nullptr);
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
    { _p->onFunction((yyval),&(yyvsp[(1) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(3) - (12)]),(yyvsp[(4) - (12)]),(yyvsp[(7) - (12)]),(yyvsp[(11) - (12)]),nullptr);
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
    { _p->onFunction((yyval),&(yyvsp[(2) - (13)]),(yyvsp[(10) - (13)]),(yyvsp[(4) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(8) - (13)]),(yyvsp[(12) - (13)]),&(yyvsp[(1) - (13)]));
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
    { Token t_ext, t_imp;
                                         t_ext.reset(); t_imp.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,t_imp,
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
    { Token t_ext, t_imp;
                                         t_ext.reset(); t_imp.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,t_imp,
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
                                                     NULL,&(yyvsp[(4) - (6)]),&(yyvsp[(3) - (6)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1177 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(4) - (7)]),&(yyvsp[(3) - (7)]));;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1183 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(4) - (9)]),&(yyvsp[(3) - (9)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1189 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(4) - (8)]),&(yyvsp[(3) - (8)]));;}
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
#line 1441 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1442 "hphp.y"
    { (yyval).reset();;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1445 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1446 "hphp.y"
    { (yyval).reset();;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1449 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1451 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1454 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1455 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1456 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1457 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1458 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1459 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1460 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1464 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1465 "hphp.y"
    { (yyval).reset();;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1468 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1469 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1470 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1474 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1476 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1477 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1478 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1482 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1483 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1487 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1489 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1490 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1491 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1492 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1495 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1499 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1500 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1504 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1505 "hphp.y"
    { (yyval).reset();;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1510 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1514 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1519 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1523 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1527 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1537 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1538 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1542 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1543 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1547 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1550 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1552 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1553 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1555 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1556 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1557 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1558 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1559 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1560 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1563 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1566 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1567 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1568 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1573 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1575 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1578 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1580 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1581 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1582 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1584 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1585 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1586 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1587 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1588 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1590 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1591 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1594 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1595 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1596 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1597 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1598 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1600 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1601 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1603 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1604 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1605 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1606 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1607 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1608 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1610 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { Token t; _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1618 "hphp.y"
    { Token u; u.reset();
                                         _p->onClosure((yyval),0,u,(yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1622 "hphp.y"
    { Token t; _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1626 "hphp.y"
    { Token u; u.reset();
                                         _p->onClosure((yyval),&(yyvsp[(1) - (12)]),u,(yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1629 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1633 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1640 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1643 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1650 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1658 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1659 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1664 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1665 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1669 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1673 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1686 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1693 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1695 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1699 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1700 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1701 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { (yyval).reset();;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1713 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1714 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1721 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1724 "hphp.y"
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

  case 405:

/* Line 1455 of yacc.c  */
#line 1735 "hphp.y"
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

  case 406:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1752 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { (yyval).reset();;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { (yyval).reset();;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1852 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1858 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1864 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1867 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1868 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1873 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1877 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1878 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1888 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1893 "hphp.y"
    { (yyval).reset();;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1895 "hphp.y"
    { (yyval).reset();;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1899 "hphp.y"
    { (yyval).reset();;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { (yyval).reset();;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 1941 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 1968 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 1969 "hphp.y"
    { (yyval).reset();;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { (yyval).reset();;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 1974 "hphp.y"
    { (yyval).reset();;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 1977 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 1978 "hphp.y"
    { (yyval).reset();;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 1984 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 1986 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 1988 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 1989 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 1993 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 1994 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 1995 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2001 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2004 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2005 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2006 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2009 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2010 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2011 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2012 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2014 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2016 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2017 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2019 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2024 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2025 "hphp.y"
    { (yyval).reset();;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2030 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2032 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2034 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2035 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2039 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2040 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2045 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2046 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2051 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2054 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2059 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2060 "hphp.y"
    { (yyval).reset();;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2063 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2064 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2071 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2073 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2076 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2078 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2081 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2084 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2085 "hphp.y"
    { (yyval).reset();;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2089 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2091 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2095 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2096 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2100 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2101 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2105 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2107 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
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
#line 2118 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2120 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2121 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2123 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2125 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2128 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2135 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2136 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2137 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2140 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2142 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2149 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2150 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2151 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2160 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2163 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2167 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2171 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2175 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2182 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2186 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2190 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2199 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2207 "hphp.y"
    { (yyval).reset();;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2211 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { (yyval)++;;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2216 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { (yyval).reset();;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); ;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); ;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); ;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    {;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2463 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2467 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (5)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2475 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 2476 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 10883 "new_hphp.tab.cpp"
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
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 2479 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

