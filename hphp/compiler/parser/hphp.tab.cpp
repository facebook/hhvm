
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
#include "hphp/util/parser/xhpast2/parser.h"
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
    _p->onMethod(out, modifiers, ret, ref, fname, params, stmt, 0, false);
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
    _p->onMethod(out, modifiers, ret, ref, fname, params, stmt, 0, false);
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
    _p->onMethod(out, modifiers, ret, ref, fname, params, stmt, 0, false);
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
     T_ASYNC = 409
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
#line 852 "new_hphp.tab.cpp"

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
#define YYLAST   11192

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  184
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  217
/* YYNRULES -- Number of rules.  */
#define YYNRULES  748
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1405

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   409

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    48,   182,     2,   179,    47,    31,   183,
     174,   175,    45,    42,     8,    43,    44,    46,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    26,   176,
      36,    13,    37,    25,    51,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    61,     2,   181,    30,     2,   180,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   177,    29,   178,    50,     2,     2,     2,
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
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    11,    13,    15,    17,
      19,    21,    26,    30,    31,    38,    39,    45,    49,    52,
      54,    56,    58,    60,    62,    64,    68,    70,    72,    75,
      79,    84,    86,    90,    92,    96,    99,   101,   104,   107,
     113,   118,   121,   122,   124,   126,   128,   130,   134,   140,
     149,   150,   155,   156,   163,   164,   175,   176,   181,   184,
     188,   191,   195,   198,   202,   206,   210,   214,   218,   224,
     226,   228,   229,   239,   245,   260,   266,   270,   274,   277,
     280,   283,   286,   289,   292,   296,   299,   302,   312,   313,
     314,   320,   322,   323,   325,   326,   328,   329,   341,   342,
     355,   356,   370,   371,   380,   381,   391,   392,   400,   401,
     410,   411,   419,   420,   429,   431,   433,   435,   437,   439,
     442,   445,   448,   449,   452,   453,   456,   457,   459,   463,
     465,   469,   472,   473,   475,   478,   483,   485,   490,   492,
     497,   499,   504,   506,   511,   515,   521,   525,   530,   535,
     541,   547,   552,   553,   555,   557,   562,   563,   569,   570,
     573,   574,   578,   579,   583,   586,   588,   589,   594,   600,
     608,   615,   622,   630,   640,   649,   653,   656,   658,   659,
     663,   668,   675,   681,   687,   694,   703,   711,   714,   715,
     717,   720,   724,   729,   733,   735,   737,   740,   745,   749,
     755,   757,   761,   764,   765,   766,   771,   772,   778,   781,
     782,   793,   794,   806,   810,   814,   818,   822,   828,   831,
     834,   835,   842,   848,   853,   857,   859,   861,   865,   870,
     872,   874,   876,   878,   883,   885,   889,   892,   893,   896,
     897,   899,   903,   905,   907,   909,   911,   915,   920,   925,
     930,   932,   934,   937,   940,   943,   947,   951,   953,   955,
     957,   959,   963,   965,   967,   969,   970,   972,   975,   977,
     979,   981,   983,   985,   987,   989,   991,   992,   994,   996,
     998,  1002,  1008,  1010,  1014,  1020,  1025,  1029,  1033,  1036,
    1038,  1042,  1046,  1048,  1050,  1051,  1054,  1059,  1063,  1070,
    1073,  1077,  1084,  1086,  1088,  1090,  1097,  1101,  1106,  1113,
    1117,  1121,  1125,  1129,  1133,  1137,  1141,  1145,  1149,  1153,
    1157,  1160,  1163,  1166,  1169,  1173,  1177,  1181,  1185,  1189,
    1193,  1197,  1201,  1205,  1209,  1213,  1217,  1221,  1225,  1229,
    1233,  1236,  1239,  1242,  1245,  1249,  1253,  1257,  1261,  1265,
    1269,  1273,  1277,  1281,  1285,  1291,  1296,  1298,  1301,  1304,
    1307,  1310,  1313,  1316,  1319,  1322,  1325,  1327,  1329,  1331,
    1335,  1338,  1339,  1351,  1352,  1365,  1367,  1369,  1371,  1377,
    1381,  1387,  1391,  1394,  1395,  1398,  1399,  1404,  1409,  1413,
    1418,  1423,  1428,  1433,  1435,  1437,  1441,  1447,  1448,  1452,
    1457,  1459,  1462,  1467,  1470,  1477,  1478,  1480,  1485,  1486,
    1489,  1490,  1492,  1494,  1498,  1500,  1504,  1506,  1508,  1512,
    1516,  1518,  1520,  1522,  1524,  1526,  1528,  1530,  1532,  1534,
    1536,  1538,  1540,  1542,  1544,  1546,  1548,  1550,  1552,  1554,
    1556,  1558,  1560,  1562,  1564,  1566,  1568,  1570,  1572,  1574,
    1576,  1578,  1580,  1582,  1584,  1586,  1588,  1590,  1592,  1594,
    1596,  1598,  1600,  1602,  1604,  1606,  1608,  1610,  1612,  1614,
    1616,  1618,  1620,  1622,  1624,  1626,  1628,  1630,  1632,  1634,
    1636,  1638,  1640,  1642,  1644,  1646,  1648,  1650,  1652,  1654,
    1656,  1658,  1660,  1662,  1664,  1666,  1668,  1670,  1672,  1674,
    1676,  1681,  1683,  1685,  1687,  1689,  1691,  1693,  1695,  1697,
    1700,  1702,  1703,  1704,  1706,  1708,  1712,  1713,  1715,  1717,
    1719,  1721,  1723,  1725,  1727,  1729,  1731,  1733,  1735,  1737,
    1741,  1744,  1746,  1748,  1751,  1754,  1759,  1763,  1768,  1770,
    1772,  1776,  1780,  1782,  1784,  1786,  1788,  1792,  1796,  1800,
    1803,  1804,  1806,  1807,  1809,  1810,  1816,  1820,  1824,  1826,
    1828,  1830,  1832,  1836,  1839,  1841,  1843,  1845,  1847,  1849,
    1852,  1855,  1860,  1864,  1869,  1872,  1873,  1879,  1883,  1887,
    1889,  1893,  1895,  1898,  1899,  1905,  1909,  1912,  1913,  1917,
    1918,  1923,  1926,  1927,  1931,  1935,  1937,  1938,  1940,  1943,
    1946,  1951,  1955,  1959,  1962,  1967,  1970,  1975,  1977,  1979,
    1981,  1983,  1985,  1988,  1993,  1997,  2002,  2006,  2008,  2010,
    2012,  2014,  2017,  2022,  2027,  2031,  2033,  2035,  2039,  2047,
    2054,  2063,  2073,  2082,  2093,  2101,  2108,  2110,  2113,  2118,
    2123,  2125,  2127,  2132,  2134,  2135,  2137,  2140,  2142,  2144,
    2147,  2152,  2156,  2160,  2161,  2163,  2166,  2171,  2175,  2178,
    2182,  2189,  2190,  2192,  2197,  2200,  2201,  2207,  2211,  2215,
    2217,  2224,  2229,  2234,  2237,  2240,  2241,  2247,  2251,  2255,
    2257,  2260,  2261,  2267,  2271,  2275,  2277,  2280,  2283,  2285,
    2288,  2290,  2295,  2299,  2303,  2310,  2314,  2316,  2318,  2320,
    2325,  2330,  2333,  2336,  2341,  2344,  2347,  2349,  2353,  2357,
    2358,  2361,  2367,  2374,  2376,  2379,  2381,  2386,  2390,  2391,
    2393,  2397,  2401,  2403,  2405,  2406,  2407,  2410,  2414,  2416,
    2422,  2426,  2430,  2434,  2436,  2439,  2440,  2445,  2448,  2451,
    2453,  2455,  2457,  2462,  2469,  2471,  2480,  2486,  2488
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     185,     0,    -1,    -1,   186,   187,    -1,   187,   188,    -1,
      -1,   202,    -1,   214,    -1,   218,    -1,   223,    -1,   387,
      -1,   116,   174,   175,   176,    -1,   141,   194,   176,    -1,
      -1,   141,   194,   177,   189,   187,   178,    -1,    -1,   141,
     177,   190,   187,   178,    -1,   104,   192,   176,    -1,   199,
     176,    -1,    71,    -1,   148,    -1,   149,    -1,   151,    -1,
     153,    -1,   152,    -1,   192,     8,   193,    -1,   193,    -1,
     194,    -1,   144,   194,    -1,   194,    90,   191,    -1,   144,
     194,    90,   191,    -1,   191,    -1,   194,   144,   191,    -1,
     194,    -1,   141,   144,   194,    -1,   144,   194,    -1,   195,
      -1,   195,   390,    -1,   195,   390,    -1,   199,     8,   388,
      13,   334,    -1,    99,   388,    13,   334,    -1,   200,   201,
      -1,    -1,   202,    -1,   214,    -1,   218,    -1,   223,    -1,
     177,   200,   178,    -1,    65,   290,   202,   245,   247,    -1,
      65,   290,    26,   200,   246,   248,    68,   176,    -1,    -1,
      82,   290,   203,   239,    -1,    -1,    81,   204,   202,    82,
     290,   176,    -1,    -1,    84,   174,   292,   176,   292,   176,
     292,   175,   205,   237,    -1,    -1,    91,   290,   206,   242,
      -1,    95,   176,    -1,    95,   299,   176,    -1,    97,   176,
      -1,    97,   299,   176,    -1,   100,   176,    -1,   100,   299,
     176,    -1,   145,    95,   176,    -1,   105,   255,   176,    -1,
     111,   257,   176,    -1,    80,   291,   176,    -1,   113,   174,
     384,   175,   176,    -1,   176,    -1,    75,    -1,    -1,    86,
     174,   299,    90,   236,   235,   175,   207,   238,    -1,    88,
     174,   241,   175,   240,    -1,   101,   177,   200,   178,   102,
     174,   327,    73,   175,   177,   200,   178,   208,   211,    -1,
     101,   177,   200,   178,   209,    -1,   103,   299,   176,    -1,
      96,   191,   176,    -1,   299,   176,    -1,   293,   176,    -1,
     294,   176,    -1,   295,   176,    -1,   296,   176,    -1,   297,
     176,    -1,   100,   296,   176,    -1,   298,   176,    -1,   191,
      26,    -1,   208,   102,   174,   327,    73,   175,   177,   200,
     178,    -1,    -1,    -1,   210,   159,   177,   200,   178,    -1,
     209,    -1,    -1,    31,    -1,    -1,    98,    -1,    -1,   213,
     212,   389,   215,   174,   251,   175,   393,   177,   200,   178,
      -1,    -1,   283,   213,   212,   389,   216,   174,   251,   175,
     393,   177,   200,   178,    -1,    -1,   354,   282,   213,   212,
     389,   217,   174,   251,   175,   393,   177,   200,   178,    -1,
      -1,   229,   226,   219,   230,   231,   177,   258,   178,    -1,
      -1,   354,   229,   226,   220,   230,   231,   177,   258,   178,
      -1,    -1,   118,   227,   221,   232,   177,   258,   178,    -1,
      -1,   354,   118,   227,   222,   232,   177,   258,   178,    -1,
      -1,   154,   228,   224,   231,   177,   258,   178,    -1,    -1,
     354,   154,   228,   225,   231,   177,   258,   178,    -1,   389,
      -1,   146,    -1,   389,    -1,   389,    -1,   117,    -1,   110,
     117,    -1,   109,   117,    -1,   119,   327,    -1,    -1,   120,
     233,    -1,    -1,   119,   233,    -1,    -1,   327,    -1,   233,
       8,   327,    -1,   327,    -1,   234,     8,   327,    -1,   122,
     236,    -1,    -1,   361,    -1,    31,   361,    -1,   123,   174,
     373,   175,    -1,   202,    -1,    26,   200,    85,   176,    -1,
     202,    -1,    26,   200,    87,   176,    -1,   202,    -1,    26,
     200,    83,   176,    -1,   202,    -1,    26,   200,    89,   176,
      -1,   191,    13,   334,    -1,   241,     8,   191,    13,   334,
      -1,   177,   243,   178,    -1,   177,   176,   243,   178,    -1,
      26,   243,    92,   176,    -1,    26,   176,   243,    92,   176,
      -1,   243,    93,   299,   244,   200,    -1,   243,    94,   244,
     200,    -1,    -1,    26,    -1,   176,    -1,   245,    66,   290,
     202,    -1,    -1,   246,    66,   290,    26,   200,    -1,    -1,
      67,   202,    -1,    -1,    67,    26,   200,    -1,    -1,   250,
       8,   157,    -1,   250,   339,    -1,   157,    -1,    -1,   355,
     285,   400,    73,    -1,   355,   285,   400,    31,    73,    -1,
     355,   285,   400,    31,    73,    13,   334,    -1,   355,   285,
     400,    73,    13,   334,    -1,   250,     8,   285,   355,   400,
      73,    -1,   250,     8,   285,   355,   400,    31,    73,    -1,
     250,     8,   285,   355,   400,    31,    73,    13,   334,    -1,
     250,     8,   285,   355,   400,    73,    13,   334,    -1,   252,
       8,   157,    -1,   252,   339,    -1,   157,    -1,    -1,   355,
     400,    73,    -1,   355,   400,    31,    73,    -1,   355,   400,
      31,    73,    13,   334,    -1,   355,   400,    73,    13,   334,
      -1,   252,     8,   355,   400,    73,    -1,   252,     8,   355,
     400,    31,    73,    -1,   252,     8,   355,   400,    31,    73,
      13,   334,    -1,   252,     8,   355,   400,    73,    13,   334,
      -1,   254,   339,    -1,    -1,   299,    -1,    31,   361,    -1,
     254,     8,   299,    -1,   254,     8,    31,   361,    -1,   255,
       8,   256,    -1,   256,    -1,    73,    -1,   179,   361,    -1,
     179,   177,   299,   178,    -1,   257,     8,    73,    -1,   257,
       8,    73,    13,   334,    -1,    73,    -1,    73,    13,   334,
      -1,   258,   259,    -1,    -1,    -1,   281,   260,   287,   176,
      -1,    -1,   283,   399,   261,   287,   176,    -1,   288,   176,
      -1,    -1,   282,   213,   212,   389,   174,   262,   249,   175,
     393,   280,    -1,    -1,   354,   282,   213,   212,   389,   174,
     263,   249,   175,   393,   280,    -1,   148,   268,   176,    -1,
     149,   274,   176,    -1,   151,   276,   176,    -1,   104,   234,
     176,    -1,   104,   234,   177,   264,   178,    -1,   264,   265,
      -1,   264,   266,    -1,    -1,   198,   140,   191,   155,   234,
     176,    -1,   267,    90,   282,   191,   176,    -1,   267,    90,
     283,   176,    -1,   198,   140,   191,    -1,   191,    -1,   269,
      -1,   268,     8,   269,    -1,   270,   324,   272,   273,    -1,
     146,    -1,   124,    -1,   327,    -1,   112,    -1,   152,   177,
     271,   178,    -1,   333,    -1,   271,     8,   333,    -1,    13,
     334,    -1,    -1,    51,   153,    -1,    -1,   275,    -1,   274,
       8,   275,    -1,   150,    -1,   277,    -1,   191,    -1,   115,
      -1,   174,   278,   175,    -1,   174,   278,   175,    45,    -1,
     174,   278,   175,    25,    -1,   174,   278,   175,    42,    -1,
     277,    -1,   279,    -1,   279,    45,    -1,   279,    25,    -1,
     279,    42,    -1,   278,     8,   278,    -1,   278,    29,   278,
      -1,   191,    -1,   146,    -1,   150,    -1,   176,    -1,   177,
     200,   178,    -1,   283,    -1,   112,    -1,   283,    -1,    -1,
     284,    -1,   283,   284,    -1,   106,    -1,   107,    -1,   108,
      -1,   111,    -1,   110,    -1,   109,    -1,   173,    -1,   286,
      -1,    -1,   106,    -1,   107,    -1,   108,    -1,   287,     8,
      73,    -1,   287,     8,    73,    13,   334,    -1,    73,    -1,
      73,    13,   334,    -1,   288,     8,   388,    13,   334,    -1,
      99,   388,    13,   334,    -1,   174,   289,   175,    -1,    63,
     329,   332,    -1,    62,   299,    -1,   316,    -1,   174,   299,
     175,    -1,   291,     8,   299,    -1,   299,    -1,   291,    -1,
      -1,   145,   299,    -1,   145,   299,   122,   299,    -1,   361,
      13,   293,    -1,   123,   174,   373,   175,    13,   293,    -1,
     172,   299,    -1,   361,    13,   296,    -1,   123,   174,   373,
     175,    13,   296,    -1,   300,    -1,   361,    -1,   289,    -1,
     123,   174,   373,   175,    13,   299,    -1,   361,    13,   299,
      -1,   361,    13,    31,   361,    -1,   361,    13,    31,    63,
     329,   332,    -1,   361,    24,   299,    -1,   361,    23,   299,
      -1,   361,    22,   299,    -1,   361,    21,   299,    -1,   361,
      20,   299,    -1,   361,    19,   299,    -1,   361,    18,   299,
      -1,   361,    17,   299,    -1,   361,    16,   299,    -1,   361,
      15,   299,    -1,   361,    14,   299,    -1,   361,    60,    -1,
      60,   361,    -1,   361,    59,    -1,    59,   361,    -1,   299,
      27,   299,    -1,   299,    28,   299,    -1,   299,     9,   299,
      -1,   299,    11,   299,    -1,   299,    10,   299,    -1,   299,
      29,   299,    -1,   299,    31,   299,    -1,   299,    30,   299,
      -1,   299,    44,   299,    -1,   299,    42,   299,    -1,   299,
      43,   299,    -1,   299,    45,   299,    -1,   299,    46,   299,
      -1,   299,    47,   299,    -1,   299,    41,   299,    -1,   299,
      40,   299,    -1,    42,   299,    -1,    43,   299,    -1,    48,
     299,    -1,    50,   299,    -1,   299,    33,   299,    -1,   299,
      32,   299,    -1,   299,    35,   299,    -1,   299,    34,   299,
      -1,   299,    36,   299,    -1,   299,    39,   299,    -1,   299,
      37,   299,    -1,   299,    38,   299,    -1,   299,    49,   329,
      -1,   174,   300,   175,    -1,   299,    25,   299,    26,   299,
      -1,   299,    25,    26,   299,    -1,   383,    -1,    58,   299,
      -1,    57,   299,    -1,    56,   299,    -1,    55,   299,    -1,
      54,   299,    -1,    53,   299,    -1,    52,   299,    -1,    64,
     330,    -1,    51,   299,    -1,   336,    -1,   309,    -1,   308,
      -1,   180,   331,   180,    -1,    12,   299,    -1,    -1,   213,
     212,   174,   301,   251,   175,   393,   314,   177,   200,   178,
      -1,    -1,   283,   213,   212,   174,   302,   251,   175,   393,
     314,   177,   200,   178,    -1,   312,    -1,   310,    -1,    79,
      -1,   304,     8,   303,   122,   299,    -1,   303,   122,   299,
      -1,   305,     8,   303,   122,   334,    -1,   303,   122,   334,
      -1,   304,   338,    -1,    -1,   305,   338,    -1,    -1,   166,
     174,   306,   175,    -1,   124,   174,   374,   175,    -1,    61,
     374,   181,    -1,   327,   177,   376,   178,    -1,   327,   177,
     378,   178,    -1,   312,    61,   369,   181,    -1,   313,    61,
     369,   181,    -1,   309,    -1,   385,    -1,   174,   300,   175,
      -1,   104,   174,   315,   339,   175,    -1,    -1,   315,     8,
      73,    -1,   315,     8,    31,    73,    -1,    73,    -1,    31,
      73,    -1,   160,   146,   317,   161,    -1,   319,    46,    -1,
     319,   161,   320,   160,    46,   318,    -1,    -1,   146,    -1,
     319,   321,    13,   322,    -1,    -1,   320,   323,    -1,    -1,
     146,    -1,   147,    -1,   177,   299,   178,    -1,   147,    -1,
     177,   299,   178,    -1,   316,    -1,   325,    -1,   324,    26,
     325,    -1,   324,    43,   325,    -1,   191,    -1,    64,    -1,
      98,    -1,    99,    -1,   100,    -1,   145,    -1,   172,    -1,
     101,    -1,   102,    -1,   159,    -1,   103,    -1,    65,    -1,
      66,    -1,    68,    -1,    67,    -1,    82,    -1,    83,    -1,
      81,    -1,    84,    -1,    85,    -1,    86,    -1,    87,    -1,
      88,    -1,    89,    -1,    49,    -1,    90,    -1,    91,    -1,
      92,    -1,    93,    -1,    94,    -1,    95,    -1,    97,    -1,
      96,    -1,    80,    -1,    12,    -1,   117,    -1,   118,    -1,
     119,    -1,   120,    -1,    63,    -1,    62,    -1,   112,    -1,
       5,    -1,     7,    -1,     6,    -1,     4,    -1,     3,    -1,
     141,    -1,   104,    -1,   105,    -1,   114,    -1,   115,    -1,
     116,    -1,   111,    -1,   110,    -1,   109,    -1,   108,    -1,
     107,    -1,   106,    -1,   173,    -1,   113,    -1,   123,    -1,
     124,    -1,     9,    -1,    11,    -1,    10,    -1,   125,    -1,
     127,    -1,   126,    -1,   128,    -1,   129,    -1,   143,    -1,
     142,    -1,   171,    -1,   154,    -1,   156,    -1,   155,    -1,
     167,    -1,   169,    -1,   166,    -1,   197,   174,   253,   175,
      -1,   198,    -1,   146,    -1,   327,    -1,   111,    -1,   367,
      -1,   327,    -1,   111,    -1,   371,    -1,   174,   175,    -1,
     290,    -1,    -1,    -1,    78,    -1,   380,    -1,   174,   253,
     175,    -1,    -1,    69,    -1,    70,    -1,    79,    -1,   128,
      -1,   129,    -1,   143,    -1,   125,    -1,   156,    -1,   126,
      -1,   127,    -1,   142,    -1,   171,    -1,   136,    78,   137,
      -1,   136,   137,    -1,   333,    -1,   196,    -1,    42,   334,
      -1,    43,   334,    -1,   124,   174,   337,   175,    -1,    61,
     337,   181,    -1,   166,   174,   307,   175,    -1,   335,    -1,
     311,    -1,   198,   140,   191,    -1,   146,   140,   191,    -1,
     196,    -1,    72,    -1,   385,    -1,   333,    -1,   182,   380,
     182,    -1,   183,   380,   183,    -1,   136,   380,   137,    -1,
     340,   338,    -1,    -1,     8,    -1,    -1,     8,    -1,    -1,
     340,     8,   334,   122,   334,    -1,   340,     8,   334,    -1,
     334,   122,   334,    -1,   334,    -1,    69,    -1,    70,    -1,
      79,    -1,   136,    78,   137,    -1,   136,   137,    -1,    69,
      -1,    70,    -1,   191,    -1,   341,    -1,   191,    -1,    42,
     342,    -1,    43,   342,    -1,   124,   174,   344,   175,    -1,
      61,   344,   181,    -1,   166,   174,   347,   175,    -1,   345,
     338,    -1,    -1,   345,     8,   343,   122,   343,    -1,   345,
       8,   343,    -1,   343,   122,   343,    -1,   343,    -1,   346,
       8,   343,    -1,   343,    -1,   348,   338,    -1,    -1,   348,
       8,   303,   122,   343,    -1,   303,   122,   343,    -1,   346,
     338,    -1,    -1,   174,   349,   175,    -1,    -1,   351,     8,
     191,   350,    -1,   191,   350,    -1,    -1,   353,   351,   338,
      -1,    41,   352,    40,    -1,   354,    -1,    -1,   357,    -1,
     121,   366,    -1,   121,   191,    -1,   121,   177,   299,   178,
      -1,    61,   369,   181,    -1,   177,   299,   178,    -1,   362,
     358,    -1,   174,   289,   175,   358,    -1,   372,   358,    -1,
     174,   289,   175,   358,    -1,   366,    -1,   326,    -1,   364,
      -1,   365,    -1,   359,    -1,   361,   356,    -1,   174,   289,
     175,   356,    -1,   328,   140,   366,    -1,   363,   174,   253,
     175,    -1,   174,   361,   175,    -1,   326,    -1,   364,    -1,
     365,    -1,   359,    -1,   361,   357,    -1,   174,   289,   175,
     357,    -1,   363,   174,   253,   175,    -1,   174,   361,   175,
      -1,   366,    -1,   359,    -1,   174,   361,   175,    -1,   361,
     121,   191,   390,   174,   253,   175,    -1,   361,   121,   366,
     174,   253,   175,    -1,   361,   121,   177,   299,   178,   174,
     253,   175,    -1,   174,   289,   175,   121,   191,   390,   174,
     253,   175,    -1,   174,   289,   175,   121,   366,   174,   253,
     175,    -1,   174,   289,   175,   121,   177,   299,   178,   174,
     253,   175,    -1,   328,   140,   191,   390,   174,   253,   175,
      -1,   328,   140,   366,   174,   253,   175,    -1,   367,    -1,
     370,   367,    -1,   367,    61,   369,   181,    -1,   367,   177,
     299,   178,    -1,   368,    -1,    73,    -1,   179,   177,   299,
     178,    -1,   299,    -1,    -1,   179,    -1,   370,   179,    -1,
     366,    -1,   360,    -1,   371,   356,    -1,   174,   289,   175,
     356,    -1,   328,   140,   366,    -1,   174,   361,   175,    -1,
      -1,   360,    -1,   371,   357,    -1,   174,   289,   175,   357,
      -1,   174,   361,   175,    -1,   373,     8,    -1,   373,     8,
     361,    -1,   373,     8,   123,   174,   373,   175,    -1,    -1,
     361,    -1,   123,   174,   373,   175,    -1,   375,   338,    -1,
      -1,   375,     8,   299,   122,   299,    -1,   375,     8,   299,
      -1,   299,   122,   299,    -1,   299,    -1,   375,     8,   299,
     122,    31,   361,    -1,   375,     8,    31,   361,    -1,   299,
     122,    31,   361,    -1,    31,   361,    -1,   377,   338,    -1,
      -1,   377,     8,   299,   122,   299,    -1,   377,     8,   299,
      -1,   299,   122,   299,    -1,   299,    -1,   379,   338,    -1,
      -1,   379,     8,   334,   122,   334,    -1,   379,     8,   334,
      -1,   334,   122,   334,    -1,   334,    -1,   380,   381,    -1,
     380,    78,    -1,   381,    -1,    78,   381,    -1,    73,    -1,
      73,    61,   382,   181,    -1,    73,   121,   191,    -1,   138,
     299,   178,    -1,   138,    72,    61,   299,   181,   178,    -1,
     139,   361,   178,    -1,   191,    -1,    74,    -1,    73,    -1,
     114,   174,   384,   175,    -1,   115,   174,   361,   175,    -1,
       7,   299,    -1,     6,   299,    -1,     5,   174,   299,   175,
      -1,     4,   299,    -1,     3,   299,    -1,   361,    -1,   384,
       8,   361,    -1,   328,   140,   191,    -1,    -1,    90,   399,
      -1,   167,   389,    13,   399,   176,    -1,   169,   389,   386,
      13,   399,   176,    -1,   191,    -1,   399,   191,    -1,   191,
      -1,   191,   162,   394,   163,    -1,   162,   391,   163,    -1,
      -1,   399,    -1,   391,     8,   399,    -1,   391,     8,   157,
      -1,   391,    -1,   157,    -1,    -1,    -1,    26,   399,    -1,
     394,     8,   191,    -1,   191,    -1,   394,     8,   191,    90,
     399,    -1,   191,    90,   399,    -1,    79,   122,   399,    -1,
     396,     8,   395,    -1,   395,    -1,   396,   338,    -1,    -1,
     166,   174,   397,   175,    -1,    25,   399,    -1,    51,   399,
      -1,   198,    -1,   124,    -1,   398,    -1,   124,   162,   399,
     163,    -1,   124,   162,   399,     8,   399,   163,    -1,   146,
      -1,   174,    98,   174,   392,   175,    26,   399,   175,    -1,
     174,   391,     8,   399,   175,    -1,   399,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   706,   706,   706,   711,   713,   716,   717,   718,   719,
     720,   721,   724,   726,   726,   728,   728,   730,   731,   736,
     737,   738,   739,   740,   741,   745,   747,   750,   751,   752,
     753,   758,   759,   763,   764,   766,   769,   775,   782,   789,
     793,   799,   801,   804,   805,   806,   807,   810,   811,   815,
     820,   820,   824,   824,   829,   828,   832,   832,   835,   836,
     837,   838,   839,   840,   841,   842,   843,   844,   845,   846,
     847,   850,   848,   853,   855,   863,   866,   867,   871,   872,
     873,   874,   875,   876,   877,   878,   879,   886,   892,   896,
     896,   902,   903,   907,   908,   912,   917,   916,   927,   925,
     937,   935,   949,   948,   967,   965,   984,   983,   992,   990,
    1002,  1001,  1013,  1011,  1024,  1025,  1029,  1032,  1035,  1036,
    1037,  1040,  1042,  1045,  1046,  1049,  1050,  1053,  1054,  1058,
    1059,  1064,  1065,  1068,  1069,  1070,  1074,  1075,  1079,  1080,
    1084,  1085,  1089,  1090,  1095,  1096,  1101,  1102,  1103,  1104,
    1107,  1110,  1112,  1115,  1116,  1120,  1122,  1125,  1128,  1131,
    1132,  1135,  1136,  1140,  1142,  1144,  1145,  1149,  1153,  1157,
    1162,  1167,  1172,  1177,  1183,  1192,  1194,  1196,  1197,  1201,
    1204,  1207,  1211,  1215,  1219,  1223,  1228,  1236,  1238,  1241,
    1242,  1243,  1245,  1250,  1251,  1254,  1255,  1256,  1260,  1261,
    1263,  1264,  1268,  1270,  1273,  1273,  1277,  1276,  1280,  1284,
    1282,  1295,  1292,  1303,  1305,  1307,  1309,  1311,  1315,  1316,
    1317,  1320,  1326,  1329,  1335,  1338,  1343,  1345,  1350,  1355,
    1359,  1360,  1366,  1367,  1372,  1373,  1378,  1379,  1383,  1384,
    1388,  1390,  1396,  1401,  1402,  1404,  1408,  1409,  1410,  1411,
    1415,  1416,  1417,  1418,  1419,  1420,  1422,  1427,  1430,  1431,
    1435,  1436,  1439,  1440,  1443,  1444,  1447,  1448,  1452,  1453,
    1454,  1455,  1456,  1457,  1458,  1462,  1463,  1466,  1467,  1468,
    1471,  1473,  1475,  1476,  1479,  1481,  1485,  1486,  1488,  1489,
    1492,  1496,  1497,  1501,  1502,  1506,  1507,  1511,  1515,  1520,
    1524,  1528,  1533,  1534,  1535,  1538,  1540,  1541,  1542,  1545,
    1546,  1547,  1548,  1549,  1550,  1551,  1552,  1553,  1554,  1555,
    1556,  1557,  1558,  1559,  1560,  1561,  1562,  1563,  1564,  1565,
    1566,  1567,  1568,  1569,  1570,  1571,  1572,  1573,  1574,  1575,
    1576,  1577,  1578,  1579,  1580,  1581,  1582,  1583,  1584,  1585,
    1587,  1588,  1590,  1592,  1593,  1594,  1595,  1596,  1597,  1598,
    1599,  1600,  1601,  1602,  1603,  1604,  1605,  1606,  1607,  1608,
    1609,  1611,  1610,  1619,  1618,  1626,  1627,  1631,  1635,  1639,
    1645,  1649,  1655,  1657,  1661,  1663,  1667,  1671,  1672,  1676,
    1683,  1690,  1692,  1697,  1698,  1699,  1703,  1705,  1709,  1710,
    1711,  1712,  1716,  1722,  1731,  1744,  1745,  1748,  1751,  1754,
    1755,  1758,  1762,  1765,  1768,  1775,  1776,  1780,  1781,  1783,
    1787,  1788,  1789,  1790,  1791,  1792,  1793,  1794,  1795,  1796,
    1797,  1798,  1799,  1800,  1801,  1802,  1803,  1804,  1805,  1806,
    1807,  1808,  1809,  1810,  1811,  1812,  1813,  1814,  1815,  1816,
    1817,  1818,  1819,  1820,  1821,  1822,  1823,  1824,  1825,  1826,
    1827,  1828,  1829,  1830,  1831,  1832,  1833,  1834,  1835,  1836,
    1837,  1838,  1839,  1840,  1841,  1842,  1843,  1844,  1845,  1846,
    1847,  1848,  1849,  1850,  1851,  1852,  1853,  1854,  1855,  1856,
    1857,  1858,  1859,  1860,  1861,  1862,  1863,  1864,  1865,  1866,
    1870,  1875,  1876,  1879,  1880,  1881,  1885,  1886,  1887,  1891,
    1892,  1893,  1897,  1898,  1899,  1902,  1904,  1908,  1909,  1910,
    1912,  1913,  1914,  1915,  1916,  1917,  1918,  1919,  1920,  1921,
    1924,  1929,  1930,  1931,  1932,  1933,  1935,  1936,  1938,  1939,
    1943,  1946,  1952,  1953,  1954,  1955,  1956,  1957,  1958,  1963,
    1965,  1969,  1970,  1973,  1974,  1978,  1981,  1983,  1985,  1989,
    1990,  1991,  1993,  1996,  2000,  2001,  2002,  2005,  2006,  2007,
    2008,  2009,  2011,  2012,  2017,  2019,  2022,  2025,  2027,  2029,
    2032,  2034,  2038,  2040,  2043,  2046,  2052,  2054,  2057,  2058,
    2063,  2066,  2070,  2070,  2075,  2078,  2079,  2083,  2084,  2089,
    2090,  2094,  2095,  2099,  2100,  2105,  2107,  2112,  2113,  2114,
    2115,  2116,  2117,  2118,  2120,  2123,  2125,  2129,  2130,  2131,
    2132,  2133,  2135,  2137,  2139,  2143,  2144,  2145,  2149,  2152,
    2155,  2158,  2162,  2166,  2173,  2177,  2184,  2185,  2190,  2192,
    2193,  2196,  2197,  2200,  2201,  2205,  2206,  2210,  2211,  2212,
    2213,  2215,  2218,  2221,  2222,  2223,  2225,  2227,  2231,  2232,
    2233,  2235,  2236,  2237,  2241,  2243,  2246,  2248,  2249,  2250,
    2251,  2254,  2256,  2257,  2261,  2263,  2266,  2268,  2269,  2270,
    2274,  2276,  2279,  2282,  2284,  2286,  2290,  2291,  2293,  2294,
    2300,  2301,  2303,  2305,  2307,  2309,  2312,  2313,  2314,  2318,
    2319,  2320,  2321,  2322,  2323,  2324,  2328,  2329,  2333,  2341,
    2343,  2347,  2350,  2356,  2357,  2363,  2364,  2371,  2374,  2378,
    2381,  2386,  2387,  2388,  2389,  2393,  2394,  2398,  2400,  2401,
    2403,  2407,  2413,  2415,  2419,  2422,  2425,  2433,  2436,  2439,
    2440,  2443,  2444,  2447,  2451,  2455,  2461,  2469,  2470
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
  "T_COMPILER_HALT_OFFSET", "T_AWAIT", "T_ASYNC", "'('", "')'", "';'",
  "'{'", "'}'", "'$'", "'`'", "']'", "'\"'", "'\\''", "$accept", "start",
  "$@1", "top_statement_list", "top_statement", "$@2", "$@3", "ident",
  "use_declarations", "use_declaration", "namespace_name",
  "namespace_string_base", "namespace_string", "namespace_string_typeargs",
  "class_namespace_string_typeargs", "constant_declaration",
  "inner_statement_list", "inner_statement", "statement", "$@4", "$@5",
  "$@6", "$@7", "$@8", "additional_catches", "finally", "$@9",
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
     406,   407,   408,   409,    40,    41,    59,   123,   125,    36,
      96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   184,   186,   185,   187,   187,   188,   188,   188,   188,
     188,   188,   188,   189,   188,   190,   188,   188,   188,   191,
     191,   191,   191,   191,   191,   192,   192,   193,   193,   193,
     193,   194,   194,   195,   195,   195,   196,   197,   198,   199,
     199,   200,   200,   201,   201,   201,   201,   202,   202,   202,
     203,   202,   204,   202,   205,   202,   206,   202,   202,   202,
     202,   202,   202,   202,   202,   202,   202,   202,   202,   202,
     202,   207,   202,   202,   202,   202,   202,   202,   202,   202,
     202,   202,   202,   202,   202,   202,   202,   208,   208,   210,
     209,   211,   211,   212,   212,   213,   215,   214,   216,   214,
     217,   214,   219,   218,   220,   218,   221,   218,   222,   218,
     224,   223,   225,   223,   226,   226,   227,   228,   229,   229,
     229,   230,   230,   231,   231,   232,   232,   233,   233,   234,
     234,   235,   235,   236,   236,   236,   237,   237,   238,   238,
     239,   239,   240,   240,   241,   241,   242,   242,   242,   242,
     243,   243,   243,   244,   244,   245,   245,   246,   246,   247,
     247,   248,   248,   249,   249,   249,   249,   250,   250,   250,
     250,   250,   250,   250,   250,   251,   251,   251,   251,   252,
     252,   252,   252,   252,   252,   252,   252,   253,   253,   254,
     254,   254,   254,   255,   255,   256,   256,   256,   257,   257,
     257,   257,   258,   258,   260,   259,   261,   259,   259,   262,
     259,   263,   259,   259,   259,   259,   259,   259,   264,   264,
     264,   265,   266,   266,   267,   267,   268,   268,   269,   269,
     270,   270,   270,   270,   271,   271,   272,   272,   273,   273,
     274,   274,   275,   276,   276,   276,   277,   277,   277,   277,
     278,   278,   278,   278,   278,   278,   278,   279,   279,   279,
     280,   280,   281,   281,   282,   282,   283,   283,   284,   284,
     284,   284,   284,   284,   284,   285,   285,   286,   286,   286,
     287,   287,   287,   287,   288,   288,   289,   289,   289,   289,
     290,   291,   291,   292,   292,   293,   293,   294,   295,   296,
     297,   298,   299,   299,   299,   300,   300,   300,   300,   300,
     300,   300,   300,   300,   300,   300,   300,   300,   300,   300,
     300,   300,   300,   300,   300,   300,   300,   300,   300,   300,
     300,   300,   300,   300,   300,   300,   300,   300,   300,   300,
     300,   300,   300,   300,   300,   300,   300,   300,   300,   300,
     300,   300,   300,   300,   300,   300,   300,   300,   300,   300,
     300,   300,   300,   300,   300,   300,   300,   300,   300,   300,
     300,   301,   300,   302,   300,   300,   300,   303,   304,   304,
     305,   305,   306,   306,   307,   307,   308,   309,   309,   310,
     311,   312,   312,   313,   313,   313,   314,   314,   315,   315,
     315,   315,   316,   317,   317,   318,   318,   319,   319,   320,
     320,   321,   322,   322,   323,   323,   323,   324,   324,   324,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     326,   327,   327,   328,   328,   328,   329,   329,   329,   330,
     330,   330,   331,   331,   331,   332,   332,   333,   333,   333,
     333,   333,   333,   333,   333,   333,   333,   333,   333,   333,
     333,   334,   334,   334,   334,   334,   334,   334,   334,   334,
     335,   335,   336,   336,   336,   336,   336,   336,   336,   337,
     337,   338,   338,   339,   339,   340,   340,   340,   340,   341,
     341,   341,   341,   341,   342,   342,   342,   343,   343,   343,
     343,   343,   343,   343,   344,   344,   345,   345,   345,   345,
     346,   346,   347,   347,   348,   348,   349,   349,   350,   350,
     351,   351,   353,   352,   354,   355,   355,   356,   356,   357,
     357,   358,   358,   359,   359,   360,   360,   361,   361,   361,
     361,   361,   361,   361,   361,   361,   361,   362,   362,   362,
     362,   362,   362,   362,   362,   363,   363,   363,   364,   364,
     364,   364,   364,   364,   365,   365,   366,   366,   367,   367,
     367,   368,   368,   369,   369,   370,   370,   371,   371,   371,
     371,   371,   371,   372,   372,   372,   372,   372,   373,   373,
     373,   373,   373,   373,   374,   374,   375,   375,   375,   375,
     375,   375,   375,   375,   376,   376,   377,   377,   377,   377,
     378,   378,   379,   379,   379,   379,   380,   380,   380,   380,
     381,   381,   381,   381,   381,   381,   382,   382,   382,   383,
     383,   383,   383,   383,   383,   383,   384,   384,   385,   386,
     386,   387,   387,   388,   388,   389,   389,   390,   390,   391,
     391,   392,   392,   392,   392,   393,   393,   394,   394,   394,
     394,   395,   396,   396,   397,   397,   398,   399,   399,   399,
     399,   399,   399,   399,   399,   399,   399,   400,   400
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     4,     3,     0,     6,     0,     5,     3,     2,     1,
       1,     1,     1,     1,     1,     3,     1,     1,     2,     3,
       4,     1,     3,     1,     3,     2,     1,     2,     2,     5,
       4,     2,     0,     1,     1,     1,     1,     3,     5,     8,
       0,     4,     0,     6,     0,    10,     0,     4,     2,     3,
       2,     3,     2,     3,     3,     3,     3,     3,     5,     1,
       1,     0,     9,     5,    14,     5,     3,     3,     2,     2,
       2,     2,     2,     2,     3,     2,     2,     9,     0,     0,
       5,     1,     0,     1,     0,     1,     0,    11,     0,    12,
       0,    13,     0,     8,     0,     9,     0,     7,     0,     8,
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
      10,     0,    11,     3,     3,     3,     3,     5,     2,     2,
       0,     6,     5,     4,     3,     1,     1,     3,     4,     1,
       1,     1,     1,     4,     1,     3,     2,     0,     2,     0,
       1,     3,     1,     1,     1,     1,     3,     4,     4,     4,
       1,     1,     2,     2,     2,     3,     3,     1,     1,     1,
       1,     3,     1,     1,     1,     0,     1,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     0,     1,     1,     1,
       3,     5,     1,     3,     5,     4,     3,     3,     2,     1,
       3,     3,     1,     1,     0,     2,     4,     3,     6,     2,
       3,     6,     1,     1,     1,     6,     3,     4,     6,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     5,     4,     1,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     1,     1,     3,
       2,     0,    11,     0,    12,     1,     1,     1,     5,     3,
       5,     3,     2,     0,     2,     0,     4,     4,     3,     4,
       4,     4,     4,     1,     1,     3,     5,     0,     3,     4,
       1,     2,     4,     2,     6,     0,     1,     4,     0,     2,
       0,     1,     1,     3,     1,     3,     1,     1,     3,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       4,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       1,     0,     0,     1,     1,     3,     0,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       2,     1,     1,     2,     2,     4,     3,     4,     1,     1,
       3,     3,     1,     1,     1,     1,     3,     3,     3,     2,
       0,     1,     0,     1,     0,     5,     3,     3,     1,     1,
       1,     1,     3,     2,     1,     1,     1,     1,     1,     2,
       2,     4,     3,     4,     2,     0,     5,     3,     3,     1,
       3,     1,     2,     0,     5,     3,     2,     0,     3,     0,
       4,     2,     0,     3,     3,     1,     0,     1,     2,     2,
       4,     3,     3,     2,     4,     2,     4,     1,     1,     1,
       1,     1,     2,     4,     3,     4,     3,     1,     1,     1,
       1,     2,     4,     4,     3,     1,     1,     3,     7,     6,
       8,     9,     8,    10,     7,     6,     1,     2,     4,     4,
       1,     1,     4,     1,     0,     1,     2,     1,     1,     2,
       4,     3,     3,     0,     1,     2,     4,     3,     2,     3,
       6,     0,     1,     4,     2,     0,     5,     3,     3,     1,
       6,     4,     4,     2,     2,     0,     5,     3,     3,     1,
       2,     0,     5,     3,     3,     1,     2,     2,     1,     2,
       1,     4,     3,     3,     6,     3,     1,     1,     1,     4,
       4,     2,     2,     4,     2,     2,     1,     3,     3,     0,
       2,     5,     6,     1,     2,     1,     4,     3,     0,     1,
       3,     3,     1,     1,     0,     0,     2,     3,     1,     5,
       3,     3,     3,     1,     2,     0,     4,     2,     2,     1,
       1,     1,     4,     6,     1,     8,     5,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   592,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   665,     0,   653,   511,
       0,   517,   518,    19,   543,   641,    70,   519,     0,    52,
       0,     0,     0,     0,     0,     0,     0,     0,    95,     0,
       0,     0,     0,     0,     0,   268,   269,   270,   273,   272,
     271,     0,     0,     0,     0,   118,     0,     0,     0,   523,
     525,   526,   520,   521,     0,     0,   527,   522,     0,     0,
     502,    20,    21,    22,    24,    23,     0,   524,     0,     0,
       0,     0,   528,     0,   274,     0,    69,    42,   645,   512,
       0,     0,     4,    31,    33,    36,   542,     0,   501,     0,
       6,    94,     7,     8,     9,     0,     0,   266,   304,     0,
       0,     0,     0,     0,     0,     0,   302,   368,   367,   376,
     375,     0,   289,   608,   503,     0,   545,   366,   265,   611,
     303,     0,     0,   609,   610,   607,   636,   640,     0,   356,
     544,    10,   273,   272,   271,     0,     0,    31,    94,     0,
     705,   303,   704,     0,   702,   701,   370,     0,     0,   340,
     341,   342,   343,   365,   363,   362,   361,   360,   359,   358,
     357,   504,     0,   718,   503,     0,   323,   321,     0,   669,
       0,   552,   288,   507,     0,   718,   506,     0,   516,   648,
     647,   508,     0,     0,   510,   364,     0,     0,     0,   292,
       0,    50,   294,     0,     0,    56,    58,     0,     0,    60,
       0,     0,     0,   740,   744,     0,     0,    31,   739,     0,
     741,     0,    62,     0,     0,    42,     0,     0,     0,    26,
      27,   195,     0,     0,   194,   120,   119,   200,     0,     0,
       0,     0,     0,   715,   106,   116,   661,   665,   690,     0,
     530,     0,     0,     0,   688,     0,    15,     0,    35,     0,
     295,   110,   117,   408,   383,     0,   709,   299,   304,     0,
     302,   303,     0,     0,   513,     0,   514,     0,     0,     0,
      86,     0,     0,    38,   188,     0,    18,    93,     0,   115,
     102,   114,   271,    94,   267,    79,    80,    81,    82,    83,
      85,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   653,    78,   644,   644,
     675,     0,     0,     0,     0,     0,   264,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   322,
     320,     0,   612,   597,   644,     0,   603,   188,   644,     0,
     646,   637,   661,     0,    94,     0,     0,   594,   589,   552,
       0,     0,     0,     0,   673,     0,   388,   551,   664,     0,
       0,    38,     0,   188,   287,     0,   649,   597,   605,   509,
       0,    42,   156,     0,    67,     0,     0,   293,     0,     0,
       0,     0,     0,    59,    77,    61,   737,   738,     0,   735,
       0,     0,   719,     0,   714,    84,    63,     0,    76,    28,
       0,    17,     0,     0,   196,     0,    65,     0,     0,    66,
     706,     0,     0,     0,     0,     0,   126,     0,   662,     0,
       0,     0,     0,   529,   689,   543,     0,     0,   687,   548,
     686,    34,     5,    12,    13,    64,     0,   124,     0,     0,
     377,     0,   552,     0,     0,     0,     0,   286,   353,   616,
      47,    41,    43,    44,    45,    46,     0,   369,   546,   547,
      32,     0,     0,     0,   554,   189,     0,   371,    96,   122,
       0,   326,   328,   327,     0,     0,   324,   325,   329,   331,
     330,   345,   344,   347,   346,   348,   350,   351,   349,   339,
     338,   333,   334,   332,   335,   336,   337,   352,   643,     0,
       0,   679,     0,   552,   708,   614,   636,   108,   112,   104,
      94,     0,     0,   297,   300,   306,   319,   318,   317,   316,
     315,   314,   313,   312,   311,   310,   309,     0,   599,   598,
       0,     0,     0,     0,     0,     0,     0,   703,   587,   591,
     551,   593,     0,     0,   718,     0,   668,     0,   667,     0,
     652,   651,     0,     0,   599,   598,   290,   158,   160,   291,
       0,    42,   140,    51,   294,     0,     0,     0,     0,   152,
     152,    57,     0,     0,   733,   552,     0,   724,     0,     0,
       0,   550,     0,     0,   502,     0,    36,   532,   501,   539,
       0,   531,    40,   538,    89,     0,    25,    29,     0,   193,
     201,   198,     0,     0,   699,   700,    11,   728,     0,     0,
       0,   661,   658,     0,   387,   698,   697,   696,     0,   692,
       0,   693,   695,     0,     5,   296,     0,     0,   402,   403,
     411,   410,     0,     0,   551,   382,   386,     0,   710,     0,
       0,   613,   597,   604,   642,     0,   717,   190,   500,   553,
     187,     0,   596,     0,     0,   124,   373,    98,   355,     0,
     391,   392,     0,   389,   551,   674,     0,   188,   126,   124,
     122,     0,   653,   307,     0,     0,   188,   601,   602,   615,
     638,   639,     0,     0,     0,   575,   559,   560,   561,     0,
       0,     0,   568,   567,   581,   552,     0,   589,   672,   671,
       0,   650,   597,   606,   515,     0,   162,     0,     0,    48,
       0,     0,     0,     0,     0,   132,   133,   144,     0,    42,
     142,    73,   152,     0,   152,     0,     0,   742,     0,   551,
     734,   736,   723,   722,     0,   720,   533,   534,   558,     0,
     552,   550,     0,     0,   385,     0,   681,     0,    75,     0,
      30,   197,     0,   707,    68,     0,     0,   716,   125,   127,
     203,     0,     0,   659,     0,   691,     0,    16,     0,   123,
     203,     0,     0,   379,     0,   711,     0,     0,   599,   598,
     720,     0,   191,    39,   177,     0,   554,   595,   748,   596,
     121,     0,   596,     0,   354,   678,   677,   188,     0,     0,
       0,   124,   100,   516,   600,   188,     0,     0,   564,   565,
     566,   569,   570,   579,     0,   552,   575,     0,   563,   583,
     551,   586,   588,   590,     0,   666,   600,     0,     0,     0,
       0,   159,    53,     0,   294,   134,   661,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   146,     0,   731,   732,
       0,     0,   746,     0,   536,   551,   549,     0,   541,     0,
     552,     0,   540,   685,     0,   552,     0,     0,   199,   730,
     727,     0,   265,   663,   661,   298,   301,   305,     0,    14,
     265,   414,     0,     0,   416,   409,   412,     0,   407,     0,
     712,     0,     0,   188,   192,   725,   596,   176,   747,     0,
       0,   203,     0,   596,     0,     0,   635,   203,   203,     0,
       0,   308,   188,     0,   629,     0,   572,   551,   574,     0,
     562,     0,     0,   552,   580,   670,     0,    42,     0,   155,
     141,     0,     0,   131,    71,   145,     0,     0,   148,     0,
     153,   154,    42,   147,   743,   721,     0,   557,   556,   535,
       0,   551,   384,   537,     0,   390,   551,   680,     0,    42,
       0,   128,     0,     0,   263,     0,     0,     0,   107,   202,
     204,     0,   262,     0,   265,     0,   694,   111,   405,     0,
       0,   378,   600,   188,     0,     0,   397,   175,   748,     0,
     179,   725,   265,   725,     0,   676,   634,   265,   265,   203,
     596,     0,   628,   578,   577,   571,     0,   573,   551,   582,
      42,   161,    49,    54,   135,     0,   143,   149,    42,   151,
       0,     0,   381,     0,   684,   683,     0,     0,   729,     0,
       0,   129,   232,   230,   502,    24,     0,   226,     0,   231,
     242,     0,   240,   245,     0,   244,     0,   243,     0,    94,
     206,     0,   208,     0,   660,   406,   404,   415,   413,   188,
       0,   632,   726,     0,     0,     0,   180,     0,     0,   103,
     397,   725,   109,   113,   265,     0,   630,     0,   585,     0,
     157,     0,    42,   138,    72,   150,   745,   555,     0,     0,
       0,    90,     0,     0,   216,   220,     0,     0,   213,   466,
     465,   462,   464,   463,   483,   485,   484,   454,   444,   460,
     459,   421,   431,   432,   434,   433,   453,   437,   435,   436,
     438,   439,   440,   441,   442,   443,   445,   446,   447,   448,
     449,   450,   452,   451,   422,   423,   424,   427,   428,   430,
     468,   469,   478,   477,   476,   475,   474,   473,   461,   480,
     470,   471,   472,   455,   456,   457,   458,   481,   482,   486,
     488,   487,   489,   490,   467,   492,   491,   425,   494,   496,
     495,   429,   499,   497,   498,   493,   426,   479,   420,   237,
     417,     0,   214,   258,   259,   257,   250,     0,   251,   215,
     282,     0,     0,     0,     0,    94,     0,   631,     0,    42,
       0,   183,     0,   182,    42,     0,     0,   105,   725,   576,
       0,    42,   136,    55,     0,   380,   682,    42,   285,   130,
       0,     0,   234,   227,     0,     0,     0,   239,   241,     0,
       0,   246,   253,   254,   252,     0,     0,   205,     0,     0,
       0,     0,   633,     0,   400,   554,     0,   184,     0,   181,
       0,    42,    42,     0,   584,     0,     0,     0,   217,    31,
       0,   218,   219,     0,     0,   233,   236,   418,   419,     0,
     228,   255,   256,   248,   249,   247,   283,   280,   209,   207,
     284,     0,   401,   553,     0,   372,     0,   186,    97,     0,
       0,    42,     0,   139,    88,     0,   265,   235,   238,     0,
     596,   211,     0,   398,   396,   185,   374,    99,     0,   137,
      92,   224,     0,   264,   281,   165,     0,   554,   276,   596,
     399,   101,     0,    91,    74,     0,     0,   223,   725,   276,
     164,   277,   278,   279,   748,   275,     0,     0,     0,   222,
       0,   163,   596,     0,   725,     0,   221,   260,    42,   210,
     748,     0,   167,     0,     0,     0,     0,   168,     0,   212,
       0,   261,     0,   171,     0,   170,    42,   172,     0,   169,
       0,     0,   174,    87,   173
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   102,   654,   462,   157,   238,   239,
     104,   105,   106,   107,   108,   109,   282,   481,   482,   406,
     210,  1111,   412,  1045,  1340,   778,   779,  1354,   298,   158,
     483,   683,   823,   940,   484,   499,   700,   446,   698,   485,
     467,   699,   300,   254,   271,   115,   685,   657,   640,   788,
    1060,   868,   745,  1243,  1114,   593,   751,   411,   601,   753,
     972,   588,   736,   739,   859,  1346,  1347,   815,   816,   493,
     494,   243,   244,   248,   902,   999,  1078,  1223,  1330,  1349,
    1250,  1291,  1292,  1293,  1066,  1067,  1068,  1251,  1257,  1300,
    1071,  1072,  1076,  1216,  1217,  1218,  1379,  1000,  1001,   159,
     117,  1364,  1365,  1221,  1003,   118,   204,   407,   408,   119,
     120,   121,   122,   123,   124,   125,   126,   682,   822,   471,
     472,   890,   473,   891,   127,   128,   129,   619,   130,   131,
    1094,  1275,   132,   468,  1086,   469,   801,   662,   918,   915,
    1209,  1210,   133,   134,   135,   198,   205,   285,   394,   136,
     768,   623,   137,   769,   388,   680,   770,   723,   841,   843,
     844,   845,   725,   952,   953,   726,   569,   379,   167,   168,
     138,   818,   362,   363,   673,   139,   199,   161,   141,   142,
     143,   144,   145,   146,   147,   529,   148,   201,   202,   449,
     190,   191,   532,   533,   894,   895,   263,   264,   648,   149,
     441,   150,   476,   151,   229,   255,   293,   421,   764,  1016,
     638,   604,   605,   606,   230,   231,   929
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -980
static const yytype_int16 yypact[] =
{
    -980,   113,  -980,  -980,  3856,  9702,  9702,   -51,  9702,  9702,
    9702,  -980,  9702,  9702,  9702,  9702,  9702,  9702,  9702,  9702,
    9702,  9702,  9702,  9702,  1697,  1697,  3041,  9702,  2265,   -43,
     -41,  -980,  -980,  -980,  -980,  -980,  -980,  -980,  9702,  -980,
     -41,   -29,   -26,   -13,   -41,  7964,   696,  8122,  -980,  1292,
    7648,    11,  9702,  1240,    -6,  -980,  -980,  -980,    85,   208,
      26,   158,   173,   180,   206,  -980,   696,   222,   230,  -980,
    -980,  -980,  -980,  -980,   362,   570,  -980,  -980,   696,  8280,
    -980,  -980,  -980,  -980,  -980,  -980,   696,  -980,   262,   236,
     696,   696,  -980,  9702,  -980,  9702,  -980,  -980,   238,   383,
     463,   463,  -980,   387,   292,   -30,  -980,   265,  -980,    45,
    -980,   414,  -980,  -980,  -980,   171,   893,  -980,  -980,   266,
     279,   289,   291,   343,   347, 10262,  -980,  -980,   401,  -980,
     465,   467,  -980,   104,   361,   384,  -980,  -980,  1945,     7,
    2542,   118,   368,   128,   130,   375,    18,  -980,    95,  -980,
     483,  -980,  -980,  -980,   410,   378,   413,  -980,   414,   893,
   10977,  2839, 10977,  9702, 10977, 10977, 11077,   523,   696,  -980,
    -980,   516,  -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,
    -980,  -980,   747,   427,  -980,   432,   452,   452,  1697, 10576,
     409,   583,  -980,   410,   747,   427,   478,   480,   424,   135,
    -980,   471,   118,  8438,  -980,  -980,  9702,  6542,    47, 10977,
    7332,  -980,  9702,  9702,   696,  -980,  -980, 10303,   449,  -980,
   10344,  1292,  1292,   441,  -980,   457,  1053,   614,  -980,   615,
    -980,   696,  -980,   456, 10385,  -980, 10426,   696,    49,  -980,
       2,  -980,  1581,    56,  -980,  -980,  -980,   620,    57,  1697,
    1697,  1697,   459,   474,  -980,  -980,  1846,  3041,    39,   333,
    -980,  9860,  1697,   546,  -980,   696,  -980,   276,   292,   462,
   10678,  -980,  -980,  -980,   561,   633,   557, 10977,   473, 10977,
     475,   961,  4014,  9702,   260,   476,   506,   260,   286,   356,
    -980,   696,  1292,   486,  8596,  1292,  -980,  -980,   965,  -980,
    -980,  -980,  -980,   414,  -980,  -980,  -980,  -980,  -980,  -980,
    -980,  9702,  9702,  9702,  8754,  9702,  9702,  9702,  9702,  9702,
    9702,  9702,  9702,  9702,  9702,  9702,  9702,  9702,  9702,  9702,
    9702,  9702,  9702,  9702,  9702,  9702,  2265,  -980,  9702,  9702,
    9702,   458,   696,   696,   171,   559,   404,  7490,  9702,  9702,
    9702,  9702,  9702,  9702,  9702,  9702,  9702,  9702,  9702,  -980,
    -980,   628,  -980,   138,  9702,  9702,  -980,  8596,  9702,  9702,
     238,   151,  1846,   487,   414,  8912, 10467,  -980,   488,   647,
     747,   489,     8,   458,   452,  9070,  -980,  9228,  -980,   490,
     200,  -980,   121,  8596,  -980,   767,  -980,   189,  -980,  -980,
   10508,  -980,  -980,  9702,  -980,   585,  6700,   660,   496, 10870,
     661,    53,    23,  -980,  -980,  -980,  -980,  -980,  1292,   596,
     502,   670,  -980, 10520,  -980,  -980,  -980,  4172,  -980,   237,
    1240,  -980,   696,  9702,   452,    -6,  -980, 10520,   609,  -980,
     452,    73,    80,   225,   510,   696,   569,   519,   452,    82,
     515,  1390,   696,  -980,  -980,   634,  1905,   -19,  -980,  -980,
    -980,   292,  -980,  -980,  -980,  -980,  9702,   576,   537,    24,
    -980,   580,   704,   541,  1292,  1292,   707,    28,   664,   106,
    -980,  -980,  -980,  -980,  -980,  -980,  2356,  -980,  -980,  -980,
    -980,    44,  1697,   552,   720, 10977,   717,  -980,  -980,   612,
    1034, 11017, 11054, 11077,  9702, 10936, 11122, 11143,  7540,  3531,
    7696,  7854,  7854,  7854,  7854,  2757,  2757,  2757,  2757,  1379,
    1379,   306,   306,   306,   516,   516,   516,  -980, 10977,   564,
     568, 10719,   572,   725,   -38,   578,   151,  -980,  -980,  -980,
     414,  1383,  9702,  -980,  -980, 11077, 11077, 11077, 11077, 11077,
   11077, 11077, 11077, 11077, 11077, 11077, 11077,  9702,   -38,   581,
     573,  2477,   589,   591,  3207,    87,   592,  -980,   464,  -980,
     696,  -980,   473,    28,   427,  1697, 10977,  1697, 10774,    32,
     193,  -980,   598,  9702,  -980,  -980,  -980,  6384,   415, 10977,
     -41,  -980,  -980,  -980,  9702,   760, 10520,   696,  6858,   599,
     602,  -980,    63,   652,  -980,   775,   611,  1422,  1292, 10520,
   10520, 10520,   618,    13,   645,   621,    -5,  -980,   657,  -980,
     623,  -980,  -980,  -980,   700,   696,  -980,  -980,  3323,  -980,
    -980,   793,  1697,   638,  -980,  -980,  -980,   726,    77,  1361,
     640,  1846,  2297,   809,  -980,  -980,  -980,  -980,   642,  -980,
    9702,  -980,  -980,  3540,  -980, 10977,  1361,   650,  -980,  -980,
    -980,  -980,   811,  9702,   561,  -980,  -980,   654,  -980,  1292,
    1062,  -980,   212,  -980,  -980,  1292,  -980,   452,  -980,  9386,
    -980, 10520,   133,   658,  1361,   576,  -980,  -980, 11100,  9702,
    -980,  -980,  9702,  -980,  9702,  -980,   662,  8596,   569,   576,
     612,   696,  2265,   452,  3430,   665,  8596,  -980,  -980,   245,
    -980,  -980,   815,  1109,  1109,   464,  -980,  -980,  -980,   667,
      50,   669,  -980,  -980,  -980,   826,   676,   488,   452,   452,
    9544,  -980,   250,  -980,  -980,  3484,   418,   -41,  7332,  -980,
     677,  4330,   678,  1697,   681,   730,   452,  -980,   844,  -980,
    -980,  -980,  -980,   268,  -980,    15,  1292,  -980,  1292,   596,
    -980,  -980,  -980,   851,   685,   687,  -980,  -980,   741,   684,
     859, 10520,   732,   696,   561,   696, 10520,   698,  -980,   714,
    -980,  -980, 10520,   452,  -980,  1292,   696,  -980,   866,  -980,
    -980,    88,   701,   452,  7806,  -980,  1584,  -980,  3698,   866,
    -980,   370,   -21, 10977,   756,  -980,   703,  9702,   -38,   708,
    -980,  1697, 10977,  -980,  -980,   706,   876,  -980,  1292,   133,
    -980,   709,   133,   711, 11100, 10977, 10815,  8596,   715,   712,
     733,   576,  -980,   424,   718,  8596,   719,  9702,  -980,  -980,
    -980,  -980,  -980,   780,   722,   909,   464,   786,  -980,   561,
     464,  -980,  -980,  -980,  1697, 10977,  -980,   -41,   898,   860,
    7332,  -980,  -980,   753,  9702,   452,  1846,   760,   755, 10520,
    4488,   411,   761,  9702,    25,    46,  -980,   768,  -980,  -980,
    1529,   912,  -980, 10520,  -980, 10520,  -980,   770,  -980,   820,
     941,   776,  -980,   828,   774,   945,  1361,   779,  -980,  -980,
     869,  1361,  1598,  -980,  1846,  -980,  -980, 11077,   787,  -980,
    1674,  -980,    30,  9702,  -980,  -980,  -980,  9702,  -980,  9702,
    -980, 10139,   790,  8596,   452,   940,   139,  -980,  -980,    81,
     792,  -980,   794,   133,  9702,   795,  -980,  -980,  -980,   791,
     799,  -980,  8596,   814,  -980,   464,  -980,   464,  -980,   817,
    -980,   868,   819,   978,  -980,   452,   969,  -980,   821,  -980,
    -980,   831,    93,  -980,  -980,  -980,   834,   835,  -980,  3365,
    -980,  -980,  -980,  -980,  -980,  -980,  1292,  -980,   874,  -980,
   10520,   561,  -980,  -980, 10520,  -980, 10520,  -980,   934,  -980,
    1292,  -980,  1292,  1361,  -980,  1952,   875,   555,  -980,  -980,
    -980,   559,  2667,    58,   404,    96,  -980,  -980,   872, 10180,
   10221, 10977,   840,  8596,   849,  1292,   915,  -980,  1292,   953,
    1014,   940,  1707,   940,   854, 10977,  -980,  1866,  2039,  -980,
     133,   855,  -980,  -980,   911,  -980,   464,  -980,   561,  -980,
    -980,  6384,  -980,  -980,  -980,  7016,  -980,  -980,  -980,  6384,
     863, 10520,  -980,   913,  -980,   917,   865,  4646,  -980,  1021,
      38,  -980,  -980,  -980,    61,   867,    65,  -980,  9988,  -980,
    -980,    67,  -980,  -980,   610,  -980,   871,  -980,   970,   414,
    -980,  1292,  -980,   559,  -980,  -980,  -980,  -980,  -980,  8596,
     873,  -980,  -980,   878,   877,    89,  1036, 10520,   880,  -980,
     915,   940,  -980,  -980,  2092,   883,  -980,   464,  -980,   928,
    6384,  7174,  -980,  -980,  -980,  6384,  -980,  -980, 10520, 10520,
     884,  -980, 10520,  1361,  -980,  -980,  1231,  1952,  -980,  -980,
    -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,
    -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,
    -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,
    -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,
    -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,
    -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,
    -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,
    -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,   527,
    -980,   875,  -980,  -980,  -980,  -980,  -980,    55,    92,  -980,
    1047,    69,   696,   970,  1049,   414,   888,  -980,   124,  -980,
     992,  1056, 10520,  -980,  -980,   894,   895,  -980,   940,  -980,
     464,  -980,  -980,  -980,  4804,  -980,  -980,  -980,  -980,  -980,
     864,    35,  -980,  -980, 10520,  9988,  9988,  1024,  -980,   610,
     610,   323,  -980,  -980,  -980, 10520,  1010,  -980,   914,    70,
   10520,   696,  -980,  1012,  -980,  1078,  4962,  1076, 10520,  -980,
    5120,  -980,  -980,   920,  -980,  5278,   916,  5436,  -980,  1008,
     959,  -980,  -980,  1018,  1231,  -980,  -980,  -980,  -980,   947,
    -980,  1086,  -980,  -980,  -980,  -980,  -980,  1106,  -980,  -980,
    -980,   948,  -980,   298,   951,  -980, 10520,  -980,  -980,  5594,
    5752,  -980,   952,  -980,  -980,   696,   404,  -980,  -980, 10520,
     141,  -980,  1050,  -980,  -980,  -980,  -980,  -980,  5910,  -980,
       4,   972,   696,   382,  -980,  -980,   956,  1124,   453,   141,
    -980,  -980,   964,  -980,  -980,  1361,   968,  -980,   940,   352,
    -980,  -980,  -980,  -980,  1292,  -980,   966,  1361,    75,  -980,
     -35,  -980,  1099,   300,   940,  1072,  -980,  -980,  -980,  -980,
    1292,  1073,  1134,   -35,   973,  6068,   304,  1136, 10520,  -980,
     975,  -980,  1080,  1141, 10520,  -980,  -980,  1142, 10520,  -980,
    6226, 10520,  -980,  -980,  -980
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -980,  -980,  -980,  -415,  -980,  -980,  -980,    -4,  -980,   731,
      19,   908,  1476,  -980,  1450,  -980,   -17,  -980,     1,  -980,
    -980,  -980,  -980,  -980,  -980,  -180,  -980,  -980,  -152,    27,
       0,  -980,  -980,  -980,     5,  -980,  -980,  -980,  -980,     6,
    -980,  -980,   822,   823,   825,  1025,   469,  -578,   491,   517,
    -184,  -980,   305,  -980,  -980,  -980,  -980,  -980,  -980,  -475,
     205,  -980,  -980,  -980,  -980,  -173,  -980,  -781,  -980,  -319,
    -980,  -980,   746,  -980,  -760,  -980,  -980,  -980,  -980,  -980,
    -980,  -980,  -980,  -980,  -980,    66,  -980,  -980,  -980,  -980,
    -980,   -27,  -980,   191,  -782,  -980,  -188,  -980,  -136,    52,
    -115,  -163,  -980,   -25,  -980,   -67,   -22,  1169,  -565,  -331,
    -980,  -980,   -37,  -980,  -980,  2619,  1114,  -980,  -980,  -605,
    -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,  -980,
     116,  -980,   419,  -980,  -980,  -980,  -980,  -980,  -980,  -980,
    -980,  -758,  -980,  1225,    94,  -316,  -980,  -980,   388,  -307,
    1941,  -980,  -980,   451,  -358,  -799,  -980,  -980,   509,  -553,
     379,  -980,  -980,  -980,  -980,  -980,   499,  -980,  -980,  -980,
    -584,  -899,  -176,  -147,  -122,  -980,  -980,    10,  -980,  -980,
    -980,  -980,    22,  -125,  -980,   207,  -980,  -980,  -980,  -348,
     974,  -980,  -980,  -980,  -980,  -980,   481,   454,  -980,  -980,
     979,  -980,  -980,  -980,  -292,   -79,  -165,  -266,  -980,  -963,
    -980,   479,  -980,  -980,  -980,  -189,  -979
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -719
static const yytype_int16 yytable[] =
{
     103,   304,   345,   496,   112,   110,   373,   272,   207,   113,
     114,   275,   276,   233,   140,   724,   543,   927,   211,   366,
     527,   571,   215,   371,   565,   396,   491,  1018,   278,   742,
     391,   111,   416,   417,   186,   187,   301,   422,   930,  1095,
     910,   932,   218,  1294,   304,   227,  1123,   653,   562,   599,
     200,   970,   675,   295,   397,   403,   116,   430,  1098,   804,
    1100,   597,   253,  1259,   435,   438,  1081,   241,  -620,  -229,
     659,   756,   240,  1127,   582,  1211,  1008,  1266,  1266,   368,
     398,   632,   253,  1123,  1260,   786,   253,   253,   632,   364,
     642,   772,   432,   364,   267,   642,   642,   268,   817,   247,
     451,   642,   361,   422,   642,   281,  1352,   821,   873,   874,
    -718,   253,  1019,     3,   665,   381,   621,  1262,   185,   185,
    1230,   830,   197,   163,   292,   755,   916,   389,   847,   361,
     621,   203,   292,   206,  1263,  -718,  -718,  1264,  1236,   873,
     874,  1377,  1378,   303,  -718,   212,   291,  -718,   213,   670,
     260,   500,  1024,   395,  1020,  1273,   917,   292,  -505,   652,
     452,   214,  1231,   -89,   378,  -617,  -504,  -624,    35,   889,
     660,  1022,  -718,   242,    11,   695,   273,  1027,  1028,   364,
      11,  -626,    11,   479,  -620,   661,   374,   848,   235,  -618,
     346,  -619,   382,   876,    35,   369,  -654,  1274,   384,  -621,
     600,   971,   245,   103,   390,   365,   103,   676,   402,   365,
     410,   405,   368,  1295,  1124,  1125,   536,   140,   427,   498,
     140,   296,   566,   404,   973,   431,   757,   424,   598,   602,
    1261,   304,   436,   439,  1082,   817,   536,  -229,   817,   798,
     787,  1128,    33,  1212,   951,  1267,  1309,   760,   633,  1105,
    -655,  1376,   434,   939,  -657,   634,   429,   643,   536,   440,
     440,   443,   712,   903,   272,   301,   448,   536,  1044,  1104,
     536,  1084,   457,  -622,   370,  1283,   185,   871,   103,   875,
    -627,  -617,   185,  -624,   461,   667,   668,   490,   185,   621,
     814,   227,   140,   791,   253,   365,  1017,   954,  1345,   961,
      98,   671,   621,   621,   621,  -618,  -623,  -619,  -178,   111,
     544,  -656,  -654,   572,  -553,  -621,  -166,   299,  1004,    81,
      82,   361,    83,    84,    85,   246,  1004,   625,   369,  1332,
     672,  1381,   249,   258,   116,  1392,   185,   534,   253,   253,
     253,   763,   817,   185,   185,   185,   361,   250,  1303,   817,
     185,   333,   334,   335,   251,   336,   185,   558,   200,   258,
     872,   873,   874,   535,   458,  1304,  -655,   851,  1305,   696,
    -657,  1333,   540,  1382,   621,   580,  1053,  1393,   828,   574,
     252,   291,   448,   559,   587,  1373,   833,   836,   701,  -622,
     382,   584,  1033,   705,  1034,  1370,   256,   671,   261,   262,
     635,  1386,   103,   731,   257,   535,   258,   592,   273,   696,
     274,  1383,   886,   290,   581,   283,   140,   585,   422,   765,
     291,   687,  -623,   103,   261,   262,   672,  -656,   627,   258,
     197,  1348,   732,  1109,   458,   258,   291,   140,  1004,   294,
     259,   637,   305,  1004,  1004,   297,   817,   647,   649,   240,
    1348,   391,   463,   464,   111,   306,   258,   733,  1361,  1362,
    1363,   284,  -393,   905,   621,   307,   185,   308,   488,   621,
     453,   261,   262,  1380,   185,   621,  1314,  1301,  1302,   116,
     806,   737,   738,  1108,   857,   858,   810,   948,    55,    56,
      57,   152,   153,   302,   261,   262,   253,  1297,  1298,   260,
     261,   262,   677,   967,   873,   874,   713,   714,   935,  1371,
      55,    56,    57,   152,   153,   302,   943,   911,   962,   309,
    1004,   261,   262,   310,   341,   715,   338,  -553,   339,    33,
     912,    35,   982,   716,   717,    33,   258,   987,   340,   489,
    1254,   287,   367,   718,  -394,   536,   530,   913,  1360,  -625,
    -504,   703,   372,  1255,  1239,    94,  1005,   265,  1357,  1361,
    1362,  1363,   621,   377,   722,   336,   727,   877,   740,   878,
    1256,   560,   383,   361,   741,   563,   621,    94,   621,   258,
     286,   288,   289,   103,   458,   728,   185,   729,   719,   292,
     386,   387,   395,   748,   103,  1039,   899,   140,   393,   750,
     720,   261,   262,   418,  1014,   746,    81,    82,   140,    83,
      84,    85,    81,    82,   111,    83,    84,    85,  -503,   258,
     392,   780,   832,  1031,   458,   414,    33,  -713,   423,   928,
     721,   419,   425,   437,   444,   185,   445,    98,   465,   116,
     470,    33,   783,   922,   261,   262,   474,   475,   477,   103,
     478,   448,   793,   112,   110,   570,   487,    48,   113,   114,
     -37,   497,   568,   140,   573,   579,   808,   590,   403,   185,
    1073,   185,   594,   621,   596,   603,   607,   621,   608,   621,
     111,    33,   631,   459,   261,   262,   636,  1284,   639,   185,
     644,   810,   809,   641,  1090,   650,   656,   253,   658,    33,
    1059,    35,   663,    81,    82,   116,    83,    84,    85,   840,
     840,   722,   664,   454,   265,   860,   666,   460,    81,    82,
     669,    83,    84,    85,   200,  -395,   185,   678,   679,  1074,
     681,   684,   870,   694,   103,   185,   185,   103,   454,   861,
     460,   454,   460,   460,   621,   690,   817,   266,   140,   691,
     693,   140,   697,   865,   707,   706,  1213,   906,    81,    82,
    1214,    83,    84,    85,   709,   817,   686,    33,   111,   888,
    1226,   892,   710,   734,   758,   752,    81,    82,   754,    83,
      84,    85,   900,   759,  1074,   773,   761,  1050,   817,  1224,
     621,   743,   771,   116,   103,   774,   197,   775,   112,   110,
     776,  1058,   777,   113,   114,   557,   782,    98,   140,    27,
      28,   621,   621,  1080,   784,   621,   785,   790,    33,  1252,
      35,   924,   794,   795,   802,   111,  1092,   800,   837,   928,
     805,    33,   819,    35,   850,   956,   827,   185,    33,   835,
      35,   846,   722,   849,    81,    82,   722,    83,    84,    85,
     116,   852,   867,   862,   864,   866,   103,   869,   181,   880,
     881,   959,   882,   883,   955,   884,   103,   885,  1083,   453,
     140,   181,   896,   897,   901,   904,   448,   746,   919,   920,
     140,   925,   923,   744,   926,   933,   931,   304,   156,   937,
     936,    78,   942,    80,   944,    81,    82,   111,    83,    84,
      85,   156,   945,   946,    78,   185,    80,    88,    81,    82,
     938,    83,    84,    85,   448,    81,    82,   947,    83,    84,
      85,   380,   116,   950,   957,   621,    98,  1222,   958,   960,
     964,   974,   183,   183,   182,    33,   195,   968,   976,    98,
    1041,   722,   980,   722,   583,   979,    98,   621,   185,   981,
     984,   983,   985,   986,  1002,  1049,   989,   195,   621,   990,
     185,   185,  1002,   621,  1013,  1006,  1015,  1021,  1029,  1023,
    1026,   621,  1057,  1030,   375,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,  1038,  1327,   227,  1032,
    1036,    48,  1035,  1075,  1037,  1040,  1051,  1042,   185,    55,
      56,    57,   152,   153,   302,   156,  1043,  1056,    78,   621,
    1046,  1047,    81,    82,  1089,    83,    84,    85,  1085,  1093,
     359,   360,   621,  1110,  1091,  1070,  1096,  1097,  1079,  1101,
    1106,  1115,   722,  1107,  1122,  1118,    33,   103,  1116,  1119,
    1120,   103,  1288,  1220,  1126,   103,  1113,  1219,  1227,  1232,
    1240,   140,  1228,   103,  1229,   140,   346,  1234,  1238,   140,
    1265,  1247,  1270,  1272,  1208,  1277,    94,   140,   111,  1278,
    1215,  1281,  1282,  1271,  1002,  1299,   111,   227,   221,  1002,
    1002,   621,   361,  1307,   111,  1312,  1313,   621,  1308,  1316,
     183,   621,  1323,   116,   621,  1244,   183,  1321,  -225,  1325,
    1328,   116,   183,   722,   222,    33,   103,   103,  1326,   116,
    1225,   103,  1242,    81,    82,  1260,    83,    84,    85,  1329,
     140,   140,  1331,  1350,    33,   140,  1334,  1355,  1339,   195,
     195,  1358,  1359,    33,   195,    35,   479,   111,  1367,   497,
      11,  1374,   111,  1268,  1369,  1384,  1387,  1388,  1390,  1394,
     183,   420,  1396,  1397,  1398,  1401,  1002,   183,   183,   183,
    1353,   626,   116,   344,   183,   537,   539,   116,   538,   831,
     183,  1368,   963,   799,  1048,   928,  1366,   223,   838,   839,
      33,   629,    81,    82,  1258,    83,    84,    85,  1077,   829,
    1342,   928,  1311,  1253,   156,  1389,  1372,    78,  1269,   224,
     195,    81,    82,   195,    83,    84,    85,   208,   686,   280,
      81,    82,  1276,    83,    84,    85,  1235,  1280,   253,   225,
     914,   941,   887,   842,  1285,   949,   853,   226,   304,   442,
    1287,   450,     0,     0,     0,     0,   722,     0,   879,   807,
     103,    98,     0,     0,   195,     0,  1289,     0,     0,   184,
     184,  1208,  1208,   196,   140,  1215,  1215,    81,    82,     0,
      83,    84,    85,     0,  1319,  1320,     0,   253,     0,     0,
       0,   111,   103,     0,     0,     0,   103,     0,     0,     0,
     183,   103,     0,   103,     0,     0,   140,     0,   183,     0,
     140,     0,     0,     0,     0,   140,   116,   140,     0,     0,
      31,    32,     0,   111,  1338,     0,     0,   111,     0,     0,
      37,    33,   111,     0,   111,   103,   103,   221,     0,     0,
       0,  1341,     0,     0,     0,     0,   195,     0,   116,   140,
     140,   616,   116,     0,   103,     0,     0,   116,  1356,   116,
       0,     0,     0,   222,     0,   616,   111,   111,   140,     0,
       0,     0,     0,     0,     0,     0,    69,    70,    71,    72,
      73,  1385,     0,    33,     0,   111,     0,   613,     0,     0,
       0,   116,   116,    76,    77,     0,     0,     0,  1343,  1400,
       0,   103,   195,   195,   237,     0,     0,    87,    81,    82,
     116,    83,    84,    85,     0,   140,   103,     0,     0,     0,
     183,     0,    92,     0,     0,     0,     0,   184,     0,     0,
     140,     0,   111,   184,     0,     0,   223,     0,     0,   184,
       0,   330,   331,   332,   333,   334,   335,   111,   336,     0,
       0,     0,    33,   156,     0,     0,    78,   116,   224,     0,
      81,    82,     0,    83,    84,    85,   702,   221,     0,   183,
       0,     0,   116,     0,    33,     0,    35,     0,   225,     0,
       0,    33,     0,   645,   646,     0,   226,   184,     0,     0,
       0,     0,     0,   222,   184,   184,   184,     0,     0,     0,
       0,   184,     0,   183,     0,   183,     0,   184,     0,     0,
       0,     0,     0,    33,   181,     0,     0,     0,     0,   228,
       0,     0,   156,   183,   616,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,   195,   195,   616,   616,   616,
       0,     0,     0,     0,   156,     0,     0,    78,     0,    80,
       0,    81,    82,     0,    83,    84,    85,     0,    81,    82,
     183,    83,    84,    85,     0,     0,   223,   195,     0,   183,
     183,     0,     0,     0,   221,     0,     0,   182,     0,     0,
       0,   196,    98,   156,   195,     0,    78,     0,   224,     0,
      81,    82,     0,    83,    84,    85,     0,   195,     0,   762,
     222,     0,     0,   195,     0,     0,     0,     0,   225,   616,
       0,     0,   195,   311,   312,   313,   226,   184,     0,     0,
      33,     0,     0,     0,     0,   184,     0,     0,     0,   314,
     195,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,     0,   336,     0,     0,     0,     0,     0,    11,
       0,     0,     0,     0,     0,     0,     0,     0,   620,     0,
       0,   183,    33,   223,    35,     0,     0,     0,     0,     0,
       0,     0,   620,     0,   195,     0,   195,     0,     0,     0,
     156,   228,   228,    78,     0,   224,   228,    81,    82,   616,
      83,    84,    85,     0,   616,     0,   975,     0,     0,     0,
     616,     0,   181,   195,     0,   225,     0,   992,     0,     0,
       0,     0,   993,   226,    55,    56,    57,   152,   153,   302,
     994,     0,     0,     0,     0,    11,     0,   184,     0,   183,
       0,     0,   156,     0,     0,    78,   195,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,     0,     0,     0,
       0,     0,   228,     0,     0,   228,   995,   996,    11,   997,
       0,     0,     0,     0,     0,   182,     0,     0,   433,     0,
      98,     0,   183,     0,     0,   908,   184,     0,    33,     0,
      35,    94,     0,   992,   183,   183,   998,   616,   993,     0,
      55,    56,    57,   152,   153,   302,   994,     0,   195,     0,
       0,   616,     0,   616,     0,     0,     0,     0,     0,     0,
     184,     0,   184,     0,   195,     0,   992,     0,   181,   195,
       0,   993,   183,    55,    56,    57,   152,   153,   302,   994,
     184,   620,   995,   996,     0,   997,     0,     0,     0,     0,
       0,     0,     0,     0,   620,   620,   620,     0,   156,     0,
       0,    78,     0,    80,     0,    81,    82,    94,    83,    84,
      85,     0,  1007,     0,     0,   995,   996,   184,   997,     0,
       0,     0,     0,     0,   789,     0,   184,   184,   228,     0,
       0,   182,     0,   618,     0,     0,    98,     0,     0,     0,
      94,   789,     0,     0,   195,  1099,     0,   618,   616,     0,
       0,     0,   616,     0,   616,     0,     0,     0,   195,   617,
     195,   195,     0,   195,     0,     0,   620,    11,     0,   820,
     195,     0,     0,   617,   311,   312,   313,    33,     0,    35,
       0,     0,     0,   195,   228,   228,   195,   196,     0,     0,
     314,     0,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,   334,   335,     0,   336,     0,     0,   181,     0,   616,
       0,     0,     0,     0,     0,   992,     0,     0,   184,   447,
     993,     0,    55,    56,    57,   152,   153,   302,   994,     0,
       0,     0,     0,     0,     0,     0,     0,   156,     0,   195,
      78,     0,    80,     0,    81,    82,   620,    83,    84,    85,
       0,   620,     0,     0,     0,   616,     0,   620,     0,     0,
       0,     0,     0,     0,   995,   996,     0,   997,     0,     0,
     182,     0,     0,    33,     0,    98,   616,   616,     0,     0,
     616,   195,     0,     0,     0,   195,   184,     0,     0,    94,
       0,     0,     0,     0,  1102,     0,   618,     0,     0,     0,
       0,    55,    56,    57,    58,    59,   302,   228,   228,   618,
     618,   618,    65,   342,  1062,     0,     0,     0,     0,     0,
       0,     0,   617,     0,     0,     0,  1063,     0,     0,   184,
      11,     0,     0,   651,     0,   617,   617,   617,     0,     0,
       0,   184,   184,   156,   620,     0,    78,     0,  1064,   343,
      81,    82,     0,    83,  1065,    85,     0,     0,   620,     0,
     620,     0,     0,     0,     0,     0,     0,     0,    94,   228,
       0,   988,     0,     0,     0,   228,   991,     0,     0,   184,
       0,   618,     0,    11,     0,     0,     0,     0,   992,     0,
     616,     0,     0,   993,     0,    55,    56,    57,   152,   153,
     302,   994,     0,     0,     0,     0,     0,   617,   195,     0,
       0,     0,   616,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   616,     0,     0,     0,     0,   616,     0,
       0,     0,     0,     0,     0,     0,   616,   995,   996,     0,
     997,   992,     0,     0,     0,     0,   993,     0,    55,    56,
      57,   152,   153,   302,   994,   620,   228,     0,   228,   620,
       0,   620,    94,     0,     0,     0,     0,  1103,  1061,     0,
    1069,   618,     0,     0,   616,     0,   618,     0,     0,     0,
       0,     0,   618,     0,     0,   228,     0,   616,     0,     0,
     995,   996,     0,   997,     0,     0,     0,   617,     0,     0,
       0,     0,   617,     0,     0,     0,     0,     0,   617,     0,
       0,     0,     0,   195,     0,    94,     0,     0,   228,     0,
    1237,     0,   195,     0,     0,   195,   620,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   195,     0,
       0,     0,     0,     0,     0,     0,   616,     0,     0,     0,
       0,     0,   616,     0,     0,     0,   616,     0,     0,   616,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   618,
       0,     0,   620,     0,     0,     0,     0,     0,     0,     0,
     228,     0,     0,   618,     0,   618,    33,     0,    35,     0,
       0,     0,     0,   620,   620,   617,     0,   620,  1249,     0,
       0,     0,  1069,     0,     0,     0,     0,     0,     0,   617,
       0,   617,     0,     0,   622,   311,   312,   313,    33,     0,
      35,     0,     0,     0,     0,     0,   193,     0,   630,     0,
       0,   314,     0,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,     0,   336,   156,     0,   181,    78,
       0,    80,     0,    81,    82,     0,    83,    84,    85,     0,
     792,     0,     0,     0,     0,     0,   228,     0,     0,     0,
     618,     0,     0,     0,   618,     0,   618,     0,   156,   194,
     228,    78,   228,    80,    98,    81,    82,     0,    83,    84,
      85,     0,   228,     0,     0,     0,   617,   620,     0,     0,
     617,     0,   617,     0,     0,   228,     0,     0,   228,     0,
       0,   182,     0,     0,     0,     0,    98,     0,     0,   620,
       0,     0,     0,     0,     0,     0,   311,   312,   313,     0,
     620,     0,     0,     0,     0,   620,     0,     0,     0,     0,
       0,   618,   314,   620,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,     0,   336,   617,     0,     0,
       0,   228,     0,     0,   674,     0,     0,   747,     0,     0,
       0,   620,     0,     0,     0,     0,     0,   618,     0,     0,
     766,   767,     0,     0,   620,   347,   348,   349,   350,   351,
     352,   353,   354,   355,   356,   357,   358,     0,   618,   618,
       0,     0,   618,   617,     0,     0,     0,     0,     0,     0,
    1061,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1375,     0,   617,   617,     0,     0,   617,     0,
       0,   359,   360,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   620,     0,     0,     0,     0,     0,   620,
       0,     0,   813,   620,   160,   162,   620,   164,   165,   166,
       0,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,     0,     0,   189,   192,     0,     0,     0,
       0,     0,     0,     0,     0,   708,     0,   209,     0,     0,
       0,     0,     0,   361,   217,     0,   220,     0,     0,   234,
       0,   236,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   618,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   221,     0,     0,     0,     0,     0,   270,     0,
    1290,     0,     0,     0,   618,     0,     0,     0,   617,     0,
       0,     0,   277,     0,   279,   618,     0,   893,   222,     0,
     618,     0,     0,   898,     0,     0,     0,     0,   618,     0,
     617,     0,     0,     0,     0,     0,     0,     0,    33,     0,
       0,   617,     0,     0,     0,     0,   617,     0,     0,     0,
       0,     0,     0,     0,   617,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  -264,   618,     0,     0,     0,
       0,     0,     0,    55,    56,    57,   152,   153,   302,   618,
       0,     0,   376,     0,     0,     0,     0,     0,     0,     0,
       0,   223,   617,  -719,  -719,  -719,  -719,   328,   329,   330,
     331,   332,   333,   334,   335,   617,   336,     0,   156,     0,
     965,    78,     0,   224,   228,    81,    82,     0,    83,    84,
      85,     0,   400,     0,   977,   400,   978,     0,     0,     0,
     228,   209,   409,   225,     0,     0,     0,     0,   618,     0,
      94,   226,     0,     0,   618,     0,     0,     0,   618,     0,
       0,   618,   375,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   617,     0,     0,     0,     0,     0,
     617,     0,     0,     0,   617,     0,   189,   617,     0,     0,
     456,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   359,   360,
       0,     0,   486,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   495,     0,     0,     0,     0,     0,     0,
       0,  1052,     0,     0,     0,  1054,     0,  1055,     0,     0,
     501,   502,   503,   505,   506,   507,   508,   509,   510,   511,
     512,   513,   514,   515,   516,   517,   518,   519,   520,   521,
     522,   523,   524,   525,   526,     0,     0,   528,   528,   531,
     361,     0,     0,     0,     0,     0,   545,   546,   547,   548,
     549,   550,   551,   552,   553,   554,   555,   556,     0,     0,
       0,     0,     0,   528,   561,     0,   495,   528,   564,     0,
       0,     0,  1117,     0,   545,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   576,     0,   578,     0,     0,     0,
       0,     0,   495,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   589,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1233,     0,
       0,     0,     0,     0,     5,     6,     7,     8,     9,     0,
       0,     0,   628,    10,     0,     0,     0,     0,     0,  1245,
    1246,     0,     0,  1248,     0,     0,     0,     0,     0,     0,
       0,     0,   188,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,   655,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,   688,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     152,   153,   154,     0,     0,    62,    63,     0,     0,     0,
       0,   270,     0,     0,   155,    68,    69,    70,    71,    72,
      73,     0,     0,  1279,     0,     0,   704,    74,     0,     0,
       0,     0,   156,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,  1296,     0,    87,     0,     0,
       0,    88,   735,     0,     0,     0,  1306,    89,     0,     0,
       0,  1310,    92,   209,    94,    95,   311,   312,   313,  1317,
      98,    99,     0,   100,   101,     0,     0,     0,     0,     0,
       0,     0,   314,     0,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,     0,   336,  1335,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   796,
    1344,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   803,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   812,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   824,     0,
       0,   825,     0,   826,     0,     0,   495,     0,     0,     0,
       0,     0,     0,     0,     0,   495,     0,     0,     0,  1395,
       0,     0,   311,   312,   313,  1399,     0,     0,     0,  1402,
       0,     0,  1404,     0,     0,     0,     0,     0,   314,   855,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,     0,   336,     0,   311,   312,   313,     0,     0,     0,
       0,     0,     0,     0,     0,   711,     0,     0,     0,     0,
     314,   970,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   907,   336,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   921,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   311,
     312,   313,     0,     0,     0,     0,   495,     0,     0,     0,
       0,     0,     0,     0,   495,   314,   907,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,     0,   336,
       0,     0,     0,   209,     0,     0,     0,     0,     0,     0,
       0,     0,   969,   311,   312,   313,     0,     0,     0,     0,
       0,   781,     0,     0,     0,     0,     0,     0,     0,   314,
       0,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,  1009,   336,     0,     0,  1010,     0,  1011,     0,
       0,   971,   495,     5,     6,     7,     8,     9,     0,     0,
       0,     0,    10,  1025,     0,     0,     0,     0,     0,     0,
       0,   495,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,     0,
     336,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,   834,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,     0,    41,     0,    42,     0,    43,     0,
       0,    44,   495,     0,     0,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,    55,    56,    57,    58,
      59,    60,     0,    61,    62,    63,    64,    65,    66,     0,
       0,     0,   856,    67,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,    75,    76,    77,    78,    79,    80,     0,    81,    82,
       0,    83,    84,    85,    86,     0,    87,     0,     0,     0,
      88,     5,     6,     7,     8,     9,    89,    90,   495,    91,
      10,    92,    93,    94,    95,     0,    96,    97,   797,    98,
      99,     0,   100,   101,     0,     0,     0,     0,     0,     0,
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
      93,    94,    95,     0,    96,    97,   909,    98,    99,     0,
     100,   101,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
       0,    36,     0,     0,     0,    37,    38,    39,    40,     0,
      41,     0,    42,     0,    43,     0,     0,    44,     0,     0,
       0,    45,    46,    47,    48,    49,    50,    51,     0,    52,
      53,    54,    55,    56,    57,    58,    59,    60,     0,    61,
      62,    63,    64,    65,    66,     0,     0,     0,     0,    67,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,    74,     0,     0,     0,     0,    75,    76,    77,
      78,    79,    80,     0,    81,    82,     0,    83,    84,    85,
      86,     0,    87,     0,     0,     0,    88,     5,     6,     7,
       8,     9,    89,    90,     0,    91,    10,    92,    93,    94,
      95,     0,    96,    97,     0,    98,    99,     0,   100,   101,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
      74,     0,     0,     0,     0,   156,    76,    77,    78,    79,
      80,     0,    81,    82,     0,    83,    84,    85,    86,     0,
      87,     0,     0,     0,    88,     5,     6,     7,     8,     9,
      89,     0,     0,     0,    10,    92,    93,    94,    95,     0,
      96,    97,   480,    98,    99,     0,   100,   101,     0,     0,
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
       0,     0,     0,   156,    76,    77,    78,    79,    80,     0,
      81,    82,     0,    83,    84,    85,    86,     0,    87,     0,
       0,     0,    88,     5,     6,     7,     8,     9,    89,     0,
       0,     0,    10,    92,    93,    94,    95,     0,    96,    97,
     624,    98,    99,     0,   100,   101,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,   863,    41,     0,    42,     0,    43,     0,
       0,    44,     0,     0,     0,    45,    46,    47,    48,     0,
      50,    51,     0,    52,     0,    54,    55,    56,    57,    58,
      59,    60,     0,    61,    62,    63,     0,    65,    66,     0,
       0,     0,     0,    67,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,   156,    76,    77,    78,    79,    80,     0,    81,    82,
       0,    83,    84,    85,    86,     0,    87,     0,     0,     0,
      88,     5,     6,     7,     8,     9,    89,     0,     0,     0,
      10,    92,    93,    94,    95,     0,    96,    97,     0,    98,
      99,     0,   100,   101,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,     0,    41,     0,    42,     0,    43,   966,     0,    44,
       0,     0,     0,    45,    46,    47,    48,     0,    50,    51,
       0,    52,     0,    54,    55,    56,    57,    58,    59,    60,
       0,    61,    62,    63,     0,    65,    66,     0,     0,     0,
       0,    67,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,    74,     0,     0,     0,     0,   156,
      76,    77,    78,    79,    80,     0,    81,    82,     0,    83,
      84,    85,    86,     0,    87,     0,     0,     0,    88,     5,
       6,     7,     8,     9,    89,     0,     0,     0,    10,    92,
      93,    94,    95,     0,    96,    97,     0,    98,    99,     0,
     100,   101,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,    74,     0,     0,     0,     0,   156,    76,    77,
      78,    79,    80,     0,    81,    82,     0,    83,    84,    85,
      86,     0,    87,     0,     0,     0,    88,     5,     6,     7,
       8,     9,    89,     0,     0,     0,    10,    92,    93,    94,
      95,     0,    96,    97,  1121,    98,    99,     0,   100,   101,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,     0,    36,
       0,     0,     0,    37,    38,    39,    40,     0,    41,     0,
      42,  1286,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,     0,    50,    51,     0,    52,     0,    54,
      55,    56,    57,    58,    59,    60,     0,    61,    62,    63,
       0,    65,    66,     0,     0,     0,     0,    67,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
      74,     0,     0,     0,     0,   156,    76,    77,    78,    79,
      80,     0,    81,    82,     0,    83,    84,    85,    86,     0,
      87,     0,     0,     0,    88,     5,     6,     7,     8,     9,
      89,     0,     0,     0,    10,    92,    93,    94,    95,     0,
      96,    97,     0,    98,    99,     0,   100,   101,     0,     0,
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
       0,     0,     0,   156,    76,    77,    78,    79,    80,     0,
      81,    82,     0,    83,    84,    85,    86,     0,    87,     0,
       0,     0,    88,     5,     6,     7,     8,     9,    89,     0,
       0,     0,    10,    92,    93,    94,    95,     0,    96,    97,
    1315,    98,    99,     0,   100,   101,     0,     0,     0,     0,
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
       0,   156,    76,    77,    78,    79,    80,     0,    81,    82,
       0,    83,    84,    85,    86,     0,    87,     0,     0,     0,
      88,     5,     6,     7,     8,     9,    89,     0,     0,     0,
      10,    92,    93,    94,    95,     0,    96,    97,  1318,    98,
      99,     0,   100,   101,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,     0,    41,  1322,    42,     0,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,     0,    50,    51,
       0,    52,     0,    54,    55,    56,    57,    58,    59,    60,
       0,    61,    62,    63,     0,    65,    66,     0,     0,     0,
       0,    67,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,    74,     0,     0,     0,     0,   156,
      76,    77,    78,    79,    80,     0,    81,    82,     0,    83,
      84,    85,    86,     0,    87,     0,     0,     0,    88,     5,
       6,     7,     8,     9,    89,     0,     0,     0,    10,    92,
      93,    94,    95,     0,    96,    97,     0,    98,    99,     0,
     100,   101,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,    74,     0,     0,     0,     0,   156,    76,    77,
      78,    79,    80,     0,    81,    82,     0,    83,    84,    85,
      86,     0,    87,     0,     0,     0,    88,     5,     6,     7,
       8,     9,    89,     0,     0,     0,    10,    92,    93,    94,
      95,     0,    96,    97,  1324,    98,    99,     0,   100,   101,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
      74,     0,     0,     0,     0,   156,    76,    77,    78,    79,
      80,     0,    81,    82,     0,    83,    84,    85,    86,     0,
      87,     0,     0,     0,    88,     5,     6,     7,     8,     9,
      89,     0,     0,     0,    10,    92,    93,    94,    95,     0,
      96,    97,  1336,    98,    99,     0,   100,   101,     0,     0,
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
       0,     0,     0,   156,    76,    77,    78,    79,    80,     0,
      81,    82,     0,    83,    84,    85,    86,     0,    87,     0,
       0,     0,    88,     5,     6,     7,     8,     9,    89,     0,
       0,     0,    10,    92,    93,    94,    95,     0,    96,    97,
    1337,    98,    99,     0,   100,   101,     0,     0,     0,     0,
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
       0,   156,    76,    77,    78,    79,    80,     0,    81,    82,
       0,    83,    84,    85,    86,     0,    87,     0,     0,     0,
      88,     5,     6,     7,     8,     9,    89,     0,     0,     0,
      10,    92,    93,    94,    95,     0,    96,    97,  1351,    98,
      99,     0,   100,   101,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,    74,     0,     0,     0,     0,   156,
      76,    77,    78,    79,    80,     0,    81,    82,     0,    83,
      84,    85,    86,     0,    87,     0,     0,     0,    88,     5,
       6,     7,     8,     9,    89,     0,     0,     0,    10,    92,
      93,    94,    95,     0,    96,    97,  1391,    98,    99,     0,
     100,   101,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,    74,     0,     0,     0,     0,   156,    76,    77,
      78,    79,    80,     0,    81,    82,     0,    83,    84,    85,
      86,     0,    87,     0,     0,     0,    88,     5,     6,     7,
       8,     9,    89,     0,     0,     0,    10,    92,    93,    94,
      95,     0,    96,    97,  1403,    98,    99,     0,   100,   101,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
      74,     0,     0,     0,     0,   156,    76,    77,    78,    79,
      80,     0,    81,    82,     0,    83,    84,    85,    86,     0,
      87,     0,     0,     0,    88,     5,     6,     7,     8,     9,
      89,     0,     0,     0,    10,    92,    93,    94,    95,     0,
      96,    97,     0,    98,    99,     0,   100,   101,   401,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,     0,    41,     0,    42,     0,
      43,     0,     0,    44,     0,     0,     0,    45,    46,    47,
      48,     0,    50,    51,     0,    52,     0,    54,    55,    56,
      57,   152,   153,    60,     0,    61,    62,    63,     0,     0,
       0,     0,     0,     0,     0,    67,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,    74,     0,
       0,     0,     0,   156,    76,    77,    78,    79,    80,     0,
      81,    82,     0,    83,    84,    85,     0,     0,    87,     0,
       0,     0,    88,     5,     6,     7,     8,     9,    89,     0,
       0,     0,    10,    92,    93,    94,    95,     0,    96,    97,
       0,    98,    99,     0,   100,   101,   591,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,     0,    41,     0,    42,     0,    43,     0,
       0,    44,     0,     0,     0,    45,    46,    47,    48,     0,
      50,    51,     0,    52,     0,    54,    55,    56,    57,   152,
     153,    60,     0,    61,    62,    63,     0,     0,     0,     0,
       0,     0,     0,    67,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,   156,    76,    77,    78,    79,    80,     0,    81,    82,
       0,    83,    84,    85,     0,     0,    87,     0,     0,     0,
      88,     5,     6,     7,     8,     9,    89,     0,     0,     0,
      10,    92,    93,    94,    95,     0,    96,    97,     0,    98,
      99,     0,   100,   101,   749,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,     0,    41,     0,    42,     0,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,     0,    50,    51,
       0,    52,     0,    54,    55,    56,    57,   152,   153,    60,
       0,    61,    62,    63,     0,     0,     0,     0,     0,     0,
       0,    67,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,    74,     0,     0,     0,     0,   156,
      76,    77,    78,    79,    80,     0,    81,    82,     0,    83,
      84,    85,     0,     0,    87,     0,     0,     0,    88,     5,
       6,     7,     8,     9,    89,     0,     0,     0,    10,    92,
      93,    94,    95,     0,    96,    97,     0,    98,    99,     0,
     100,   101,  1112,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
       0,    36,     0,     0,     0,    37,    38,    39,    40,     0,
      41,     0,    42,     0,    43,     0,     0,    44,     0,     0,
       0,    45,    46,    47,    48,     0,    50,    51,     0,    52,
       0,    54,    55,    56,    57,   152,   153,    60,     0,    61,
      62,    63,     0,     0,     0,     0,     0,     0,     0,    67,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,    74,     0,     0,     0,     0,   156,    76,    77,
      78,    79,    80,     0,    81,    82,     0,    83,    84,    85,
       0,     0,    87,     0,     0,     0,    88,     5,     6,     7,
       8,     9,    89,     0,     0,     0,    10,    92,    93,    94,
      95,     0,    96,    97,     0,    98,    99,     0,   100,   101,
    1241,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,     0,    36,
       0,     0,     0,    37,    38,    39,    40,     0,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,     0,    50,    51,     0,    52,     0,    54,
      55,    56,    57,   152,   153,    60,     0,    61,    62,    63,
       0,     0,     0,     0,     0,     0,     0,    67,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
      74,     0,     0,     0,     0,   156,    76,    77,    78,    79,
      80,     0,    81,    82,     0,    83,    84,    85,     0,     0,
      87,     0,     0,     0,    88,     5,     6,     7,     8,     9,
      89,     0,     0,     0,    10,    92,    93,    94,    95,     0,
      96,    97,     0,    98,    99,     0,   100,   101,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,     0,    41,     0,    42,     0,
      43,     0,     0,    44,     0,     0,     0,    45,    46,    47,
      48,     0,    50,    51,     0,    52,     0,    54,    55,    56,
      57,   152,   153,    60,     0,    61,    62,    63,     0,     0,
       0,     0,     0,     0,     0,    67,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,    74,     0,
       0,     0,     0,   156,    76,    77,    78,    79,    80,     0,
      81,    82,     0,    83,    84,    85,     0,     0,    87,     0,
       0,     0,    88,     5,     6,     7,     8,     9,    89,     0,
       0,     0,    10,    92,    93,    94,    95,     0,    96,    97,
       0,    98,    99,     0,   100,   101,     0,     0,     0,     0,
       0,   541,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,    35,     0,     0,     0,     0,     0,    37,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,    48,   336,
       0,     0,     0,     0,     0,     0,    55,    56,    57,   152,
     153,   154,     0,     0,    62,    63,     0,     0,     0,     0,
       0,     0,     0,   155,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,   156,    76,    77,    78,   542,    80,     0,    81,    82,
       0,    83,    84,    85,     0,     0,    87,     0,     0,     0,
      88,     5,     6,     7,     8,     9,    89,     0,     0,     0,
      10,    92,    93,    94,    95,     0,     0,     0,     0,    98,
      99,     0,   100,   101,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,    35,     0,     0,     0,     0,     0,    37,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,     0,   336,    48,     0,     0,     0,
       0,     0,     0,     0,    55,    56,    57,   152,   153,   154,
       0,     0,    62,    63,     0,     0,     0,     0,     0,     0,
       0,   155,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,    74,     0,     0,     0,     0,   156,
      76,    77,    78,     0,    80,     0,    81,    82,     0,    83,
      84,    85,     0,     0,    87,     0,     0,     0,    88,     5,
       6,     7,     8,     9,    89,     0,     0,     0,    10,    92,
      93,    94,    95,     0,   232,     0,     0,    98,    99,     0,
     100,   101,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
       0,     0,     0,     0,     0,    37,  -719,  -719,  -719,  -719,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,     0,   336,    48,     0,     0,     0,     0,     0,
       0,     0,    55,    56,    57,   152,   153,   154,     0,     0,
      62,    63,     0,     0,     0,     0,     0,     0,     0,   155,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,    74,     0,     0,     0,     0,   156,    76,    77,
      78,   542,    80,     0,    81,    82,     0,    83,    84,    85,
       0,     0,    87,     0,     0,     0,    88,     5,     6,     7,
       8,     9,    89,     0,     0,     0,    10,    92,    93,    94,
      95,     0,     0,     0,     0,    98,    99,     0,   100,   101,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,    33,    34,    35,     0,     0,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      55,    56,    57,   152,   153,   154,     0,     0,    62,    63,
       0,     0,     0,     0,     0,     0,     0,   155,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
      74,     0,     0,     0,     0,   156,    76,    77,    78,     0,
      80,     0,    81,    82,     0,    83,    84,    85,     0,     0,
      87,     0,     0,     0,    88,     5,     6,     7,     8,     9,
      89,     0,     0,     0,    10,    92,     0,    94,    95,     0,
     216,     0,     0,    98,    99,     0,   100,   101,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,    35,     0,     0,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    55,    56,
      57,   152,   153,   154,     0,     0,    62,    63,     0,     0,
       0,     0,     0,     0,     0,   155,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,    74,     0,
       0,     0,     0,   156,    76,    77,    78,     0,    80,     0,
      81,    82,     0,    83,    84,    85,     0,     0,    87,     0,
       0,     0,    88,     5,     6,     7,     8,     9,    89,     0,
       0,     0,    10,    92,     0,    94,    95,     0,   219,     0,
       0,    98,    99,     0,   100,   101,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,    35,     0,     0,     0,     0,     0,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   269,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    55,    56,    57,   152,
     153,   154,     0,     0,    62,    63,     0,     0,     0,     0,
       0,     0,     0,   155,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,   156,    76,    77,    78,     0,    80,     0,    81,    82,
       0,    83,    84,    85,     0,     0,    87,     0,     0,     0,
      88,     5,     6,     7,     8,     9,    89,     0,     0,     0,
      10,    92,     0,    94,    95,     0,     0,     0,     0,    98,
      99,     0,   100,   101,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,    35,     0,     0,     0,     0,     0,    37,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    55,    56,    57,   152,   153,   154,
       0,     0,    62,    63,     0,     0,     0,     0,     0,     0,
       0,   155,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,    74,     0,     0,     0,     0,   156,
      76,    77,    78,     0,    80,     0,    81,    82,     0,    83,
      84,    85,     0,     0,    87,     0,     0,     0,    88,     5,
       6,     7,     8,     9,    89,     0,     0,     0,    10,    92,
       0,    94,    95,   399,     0,     0,     0,    98,    99,     0,
     100,   101,     0,     0,     0,     0,     0,   492,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
       0,     0,     0,     0,     0,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    55,    56,    57,   152,   153,   154,     0,     0,
      62,    63,     0,     0,     0,     0,     0,     0,     0,   155,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,    74,     0,     0,     0,     0,   156,    76,    77,
      78,     0,    80,     0,    81,    82,     0,    83,    84,    85,
       0,     0,    87,     0,     0,     0,    88,     5,     6,     7,
       8,     9,    89,     0,     0,     0,    10,    92,     0,    94,
      95,     0,     0,     0,     0,    98,    99,     0,   100,   101,
     504,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,    33,    34,    35,     0,     0,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      55,    56,    57,   152,   153,   154,     0,     0,    62,    63,
       0,     0,     0,     0,     0,     0,     0,   155,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
      74,     0,     0,     0,     0,   156,    76,    77,    78,     0,
      80,     0,    81,    82,     0,    83,    84,    85,     0,     0,
      87,     0,     0,     0,    88,     5,     6,     7,     8,     9,
      89,     0,     0,     0,    10,    92,     0,    94,    95,     0,
       0,     0,     0,    98,    99,     0,   100,   101,     0,     0,
       0,     0,     0,   541,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,    35,     0,     0,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    55,    56,
      57,   152,   153,   154,     0,     0,    62,    63,     0,     0,
       0,     0,     0,     0,     0,   155,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,    74,     0,
       0,     0,     0,   156,    76,    77,    78,     0,    80,     0,
      81,    82,     0,    83,    84,    85,     0,     0,    87,     0,
       0,     0,    88,     5,     6,     7,     8,     9,    89,     0,
       0,     0,    10,    92,     0,    94,    95,     0,     0,     0,
       0,    98,    99,     0,   100,   101,     0,     0,     0,     0,
       0,   575,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,    35,     0,     0,     0,     0,     0,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    55,    56,    57,   152,
     153,   154,     0,     0,    62,    63,     0,     0,     0,     0,
       0,     0,     0,   155,    68,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,    74,     0,     0,     0,
       0,   156,    76,    77,    78,     0,    80,     0,    81,    82,
       0,    83,    84,    85,     0,     0,    87,     0,     0,     0,
      88,     5,     6,     7,     8,     9,    89,     0,     0,     0,
      10,    92,     0,    94,    95,     0,     0,     0,     0,    98,
      99,     0,   100,   101,     0,     0,     0,     0,     0,   577,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,    35,     0,     0,     0,     0,     0,    37,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,    55,    56,    57,   152,   153,   154,
       0,     0,    62,    63,     0,     0,     0,     0,     0,     0,
       0,   155,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,    74,     0,     0,     0,     0,   156,
      76,    77,    78,     0,    80,     0,    81,    82,     0,    83,
      84,    85,     0,     0,    87,     0,     0,     0,    88,     5,
       6,     7,     8,     9,    89,     0,     0,     0,    10,    92,
       0,    94,    95,     0,     0,     0,     0,    98,    99,     0,
     100,   101,     0,     0,     0,     0,     0,   811,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
       0,     0,     0,     0,     0,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    55,    56,    57,   152,   153,   154,     0,     0,
      62,    63,     0,     0,     0,     0,     0,     0,     0,   155,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
       0,     0,    74,     0,     0,     0,     0,   156,    76,    77,
      78,     0,    80,     0,    81,    82,     0,    83,    84,    85,
       0,     0,    87,     0,     0,     0,    88,     5,     6,     7,
       8,     9,    89,     0,     0,     0,    10,    92,     0,    94,
      95,     0,     0,     0,     0,    98,    99,     0,   100,   101,
       0,     0,     0,     0,     0,   854,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,    33,    34,    35,     0,     0,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
      55,    56,    57,   152,   153,   154,     0,     0,    62,    63,
       0,     0,     0,     0,     0,     0,     0,   155,    68,    69,
      70,    71,    72,    73,     0,     0,     0,     0,     0,     0,
      74,     0,     0,     0,     0,   156,    76,    77,    78,     0,
      80,     0,    81,    82,     0,    83,    84,    85,     0,     0,
      87,     0,     0,     0,    88,     5,     6,     7,     8,     9,
      89,     0,     0,     0,    10,    92,     0,    94,    95,     0,
       0,     0,     0,    98,    99,     0,   100,   101,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,    35,     0,     0,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,    55,    56,
      57,   152,   153,   154,     0,     0,    62,    63,     0,     0,
       0,     0,     0,     0,     0,   155,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,    74,     0,
       0,     0,     0,   156,    76,    77,    78,     0,    80,     0,
      81,    82,     0,    83,    84,    85,     0,     0,    87,     0,
       0,     0,    88,     5,     6,     7,     8,     9,    89,     0,
       0,     0,    10,    92,     0,    94,    95,     0,     0,     0,
       0,    98,    99,     0,   100,   101,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,   455,    35,     0,     0,     0,     0,     0,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,    55,    56,    57,   152,
     153,   154,     0,     0,    62,    63,     0,     0,     0,     0,
       0,     0,     0,   155,    68,    69,    70,    71,    72,    73,
       0,  1129,  1130,  1131,  1132,  1133,    74,  1134,  1135,  1136,
    1137,   156,    76,    77,    78,     0,    80,     0,    81,    82,
       0,    83,    84,    85,     0,     0,    87,     0,     0,     0,
      88,     0,     0,     0,     0,     0,    89,     0,     0,     0,
       0,    92,     0,    94,    95,     0,     0,  1138,     0,    98,
      99,     0,   100,   101,     0,     0,     0,     0,     0,     0,
    1139,  1140,  1141,  1142,  1143,  1144,  1145,     0,     0,    33,
       0,     0,     0,     0,     0,     0,     0,     0,  1146,  1147,
    1148,  1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,
    1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1172,  1173,  1174,  1175,  1176,  1177,
    1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,     0,
       0,  1187,  1188,  1189,  1190,  1191,  1192,  1193,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1194,
    1195,  1196,     0,  1197,     0,     0,    81,    82,     0,    83,
      84,    85,  1198,  1199,  1200,     0,     0,  1201,   311,   312,
     313,     0,     0,     0,  1202,  1203,     0,  1204,     0,  1205,
    1206,  1207,     0,     0,   314,     0,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,     0,   336,   311,
     312,   313,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   314,     0,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,     0,   336,
     311,   312,   313,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   314,     0,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,     0,
     336,   311,   312,   313,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   314,     0,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
       0,   336,   311,   312,   313,     0,     0,  1012,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   314,     0,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,     0,   336,   311,   312,   313,     0,     0,  1087,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   314,
       0,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,     0,   336,   311,   312,   313,     0,     0,  1088,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     314,     0,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,   334,   335,     0,   336,   311,   312,   313,   337,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   314,     0,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,     0,   336,   311,   312,   313,   413,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   314,     0,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,     0,   336,   311,   312,   313,
     415,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   314,     0,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,     0,   336,     0,     0,
       0,   426,   609,   610,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   611,     0,     0,     0,   311,   312,   313,     0,    31,
      32,    33,     0,     0,     0,     0,     0,     0,     0,    37,
       0,   314,   428,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,     0,   336,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   567,     0,   612,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,   613,     0,     0,     0,
       0,   156,    76,    77,    78,     0,   614,     0,    81,    82,
       0,    83,    84,    85,     0,     0,    87,     0,     0,     0,
       0,     0,     0,   586,     0,     0,   615,   311,   312,   313,
       0,    92,     0,     0,     0,     0,     0,     0,   385,     0,
       0,     0,     0,   314,     0,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,     0,   336,   311,   312,
     313,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   314,     0,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,     0,   336,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   311,   312,   313,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   314,
     466,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,     0,   336,   311,   312,   313,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     314,   692,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,   334,   335,     0,   336,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   311,
     312,   313,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   314,   730,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,     0,   336,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   934,     0,     0,
       0,     0,     0,     0,     0,   311,   312,   313,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     595,   314,   689,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,     0,   336,   311,   312,   313,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   314,     0,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,     0,   336,   312,   313,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   314,     0,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,   313,   336,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   314,
       0,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   314,   336,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,     0,   336,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,     0,   336,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
       0,   336,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,     0,   336
};

static const yytype_int16 yycheck[] =
{
       4,   116,   138,   295,     4,     4,   158,    86,    30,     4,
       4,    90,    91,    50,     4,   568,   347,   816,    40,   141,
     336,   379,    44,   148,   372,   201,   292,   926,    95,   594,
     195,     4,   221,   222,    24,    25,   115,   226,   819,  1018,
     800,   822,    46,     8,   159,    49,     8,   462,   367,    26,
      28,    26,     8,     8,   201,     8,     4,     8,  1021,   664,
    1023,     8,    66,     8,     8,     8,     8,    73,    61,     8,
      46,     8,    53,     8,   393,     8,    46,     8,     8,    61,
     202,     8,    86,     8,    29,     8,    90,    91,     8,    61,
       8,    78,    90,    61,    75,     8,     8,    78,   682,    73,
      61,     8,   121,   292,     8,    95,   102,   685,    93,    94,
     140,   115,    31,     0,   472,   182,   423,    25,    24,    25,
      31,   699,    28,   174,   162,   600,   147,   194,    78,   121,
     437,   174,   162,   174,    42,   140,   174,    45,  1101,    93,
      94,   176,   177,   116,   174,   174,   144,   177,   174,   121,
     137,   303,   933,   121,    73,    31,   177,   162,   140,   178,
     121,   174,    73,   159,   168,    61,   140,    61,    73,   774,
     146,   931,   177,   179,    41,   533,   146,   937,   938,    61,
      41,   174,    41,   175,   177,   161,   159,   137,   177,    61,
     138,    61,   182,   178,    73,   177,    61,    73,   188,    61,
     177,   176,   117,   207,   194,   177,   210,   163,   207,   177,
     214,   210,    61,   178,   176,   177,   341,   207,   235,   298,
     210,   176,   374,   176,   178,   176,   163,   231,   175,   418,
     175,   346,   176,   176,   176,   819,   361,   176,   822,   654,
     163,   176,    71,   176,   849,   176,   176,   605,   175,  1030,
      61,   176,   242,   831,    61,   175,   237,   175,   383,   249,
     250,   251,   175,   175,   343,   344,   256,   392,   175,  1029,
     395,   175,   262,    61,   179,  1238,   182,   752,   282,   754,
     174,   177,   188,   177,   265,   474,   475,   291,   194,   596,
     157,   295,   282,   641,   298,   177,   157,   850,   157,   864,
     179,   477,   609,   610,   611,   177,    61,   177,   175,   282,
     347,    61,   177,   380,   175,   177,   175,   146,   902,   148,
     149,   121,   151,   152,   153,   117,   910,    90,   177,    31,
     477,    31,   174,    73,   282,    31,   242,   341,   342,   343,
     344,   607,   926,   249,   250,   251,   121,   174,    25,   933,
     256,    45,    46,    47,   174,    49,   262,   361,   336,    73,
      92,    93,    94,   341,    78,    42,   177,   725,    45,   534,
     177,    73,   345,    73,   681,   175,   981,    73,   697,   383,
     174,   144,   372,   361,   401,  1364,   702,   706,   540,   177,
     380,   395,   945,   558,   947,  1358,   174,   573,   138,   139,
     175,  1380,   406,   579,   174,   383,    73,   406,   146,   574,
     174,  1374,   770,    26,   392,   177,   406,   395,   607,   608,
     144,   500,   177,   427,   138,   139,   573,   177,   432,    73,
     336,  1330,   579,  1038,    78,    73,   144,   427,  1022,   174,
      78,   445,   176,  1027,  1028,    31,  1030,   451,   452,   430,
    1349,   616,   176,   177,   427,   176,    73,   579,   106,   107,
     108,    78,    61,   794,   771,   176,   372,   176,   182,   776,
     137,   138,   139,  1372,   380,   782,  1275,  1259,  1260,   427,
     669,    66,    67,  1036,    66,    67,   675,   845,   106,   107,
     108,   109,   110,   111,   138,   139,   500,  1255,  1256,   137,
     138,   139,   492,    92,    93,    94,    42,    43,   827,   157,
     106,   107,   108,   109,   110,   111,   835,   147,   866,   176,
    1104,   138,   139,   176,   140,    61,    61,   175,    61,    71,
     160,    73,   890,    69,    70,    71,    73,   895,   177,   183,
      13,    78,   174,    79,    61,   670,   339,   177,  1347,   174,
     140,   541,   174,    26,  1107,   173,   904,   144,   176,   106,
     107,   108,   869,    40,   568,    49,   570,   756,   590,   758,
      43,   364,   140,   121,   591,   368,   883,   173,   885,    73,
      99,   100,   101,   587,    78,   575,   492,   577,   124,   162,
     181,     8,   121,   597,   598,   953,   785,   587,   174,   598,
     136,   138,   139,   162,   923,   595,   148,   149,   598,   151,
     152,   153,   148,   149,   587,   151,   152,   153,   140,    73,
     140,   625,   701,   942,    78,   176,    71,    13,    13,   818,
     166,   174,   176,    13,   175,   541,   162,   179,   176,   587,
      79,    71,   632,   808,   138,   139,    13,    90,   175,   653,
     175,   641,   642,   653,   653,     8,   180,    98,   653,   653,
     174,   174,   174,   653,   175,   175,   670,    82,     8,   575,
     115,   577,   176,   980,    13,    79,   174,   984,     8,   986,
     653,    71,    73,   137,   138,   139,   176,  1240,   119,   595,
     175,   880,   670,   174,  1013,    61,   120,   701,   161,    71,
     992,    73,   122,   148,   149,   653,   151,   152,   153,   713,
     714,   715,     8,   259,   144,   737,   175,   263,   148,   149,
      13,   151,   152,   153,   702,    61,   632,   175,     8,   174,
      13,   119,   749,     8,   738,   641,   642,   741,   284,   738,
     286,   287,   288,   289,  1051,   181,  1330,   177,   738,   181,
     178,   741,   174,   743,   181,   174,   146,   794,   148,   149,
     150,   151,   152,   153,   175,  1349,   174,    71,   741,   773,
    1089,   775,   181,   175,   122,   176,   148,   149,   176,   151,
     152,   153,   786,     8,   174,   140,   175,   976,  1372,  1081,
    1097,    31,   174,   741,   798,   174,   702,   140,   798,   798,
     177,   990,   102,   798,   798,   177,    13,   179,   798,    62,
      63,  1118,  1119,  1002,   176,  1122,    90,   177,    71,  1126,
      73,   811,    13,   181,    13,   798,  1015,   177,    13,  1018,
     176,    71,   174,    73,     8,   857,   174,   743,    71,   174,
      73,   174,   846,   174,   148,   149,   850,   151,   152,   153,
     798,   175,   122,   176,   176,   174,   860,    13,   111,     8,
     175,   860,   175,   122,   854,   181,   870,     8,  1004,   137,
     860,   111,   174,   159,     8,   174,   866,   867,   122,   176,
     870,   175,   174,   123,     8,   174,   177,  1002,   141,   177,
     175,   144,   174,   146,   175,   148,   149,   870,   151,   152,
     153,   141,   122,   181,   144,   811,   146,   160,   148,   149,
     177,   151,   152,   153,   904,   148,   149,     8,   151,   152,
     153,   174,   870,   137,    26,  1232,   179,  1079,    68,   176,
     175,   163,    24,    25,   174,    71,    28,   176,    26,   179,
     957,   945,   122,   947,   177,   175,   179,  1254,   854,     8,
     122,   175,   178,     8,   902,   972,   177,    49,  1265,    90,
     866,   867,   910,  1270,   174,   178,    26,   175,   177,   175,
     175,  1278,   989,   174,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,     8,  1294,   992,   175,
     122,    98,   175,   997,   175,    26,   122,   176,   904,   106,
     107,   108,   109,   110,   111,   141,   175,    73,   144,  1316,
     176,   176,   148,   149,   174,   151,   152,   153,   146,   104,
      59,    60,  1329,  1040,   175,   150,    73,    13,  1001,   175,
     175,  1048,  1036,   122,    13,   122,    71,  1041,   175,   122,
     175,  1045,   178,    73,   177,  1049,  1045,   176,   175,    13,
     122,  1041,   174,  1057,   177,  1045,  1004,   177,   175,  1049,
      13,   177,    13,   175,  1068,    73,   173,  1057,  1041,    13,
    1074,   177,   177,  1225,  1022,    51,  1049,  1081,    25,  1027,
    1028,  1388,   121,    73,  1057,    73,     8,  1394,   174,    13,
     182,  1398,   176,  1041,  1401,  1112,   188,   177,    90,   140,
     153,  1049,   194,  1107,    51,    71,  1110,  1111,    90,  1057,
    1083,  1115,  1111,   148,   149,    29,   151,   152,   153,    13,
    1110,  1111,   174,    73,    71,  1115,   175,   155,   176,   221,
     222,   175,     8,    71,   226,    73,   175,  1110,   174,   174,
      41,   175,  1115,  1222,   176,    73,    73,    13,   175,    13,
     242,    98,   177,    73,    13,    13,  1104,   249,   250,   251,
    1340,   430,  1110,   138,   256,   342,   344,  1115,   343,   700,
     262,  1355,   867,   656,   969,  1364,  1349,   124,    69,    70,
      71,   435,   148,   149,  1211,   151,   152,   153,   997,   698,
    1326,  1380,  1271,  1127,   141,  1383,  1359,   144,  1223,   146,
     292,   148,   149,   295,   151,   152,   153,    38,   174,    95,
     148,   149,  1229,   151,   152,   153,  1100,  1234,  1222,   166,
     801,   833,   771,   714,  1241,   846,   727,   174,  1343,   250,
    1247,   257,    -1,    -1,    -1,    -1,  1240,    -1,   759,   177,
    1244,   179,    -1,    -1,   336,    -1,  1250,    -1,    -1,    24,
      25,  1255,  1256,    28,  1244,  1259,  1260,   148,   149,    -1,
     151,   152,   153,    -1,  1281,  1282,    -1,  1271,    -1,    -1,
      -1,  1244,  1276,    -1,    -1,    -1,  1280,    -1,    -1,    -1,
     372,  1285,    -1,  1287,    -1,    -1,  1276,    -1,   380,    -1,
    1280,    -1,    -1,    -1,    -1,  1285,  1244,  1287,    -1,    -1,
      69,    70,    -1,  1276,  1321,    -1,    -1,  1280,    -1,    -1,
      79,    71,  1285,    -1,  1287,  1319,  1320,    25,    -1,    -1,
      -1,  1325,    -1,    -1,    -1,    -1,   418,    -1,  1276,  1319,
    1320,   423,  1280,    -1,  1338,    -1,    -1,  1285,  1342,  1287,
      -1,    -1,    -1,    51,    -1,   437,  1319,  1320,  1338,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   125,   126,   127,   128,
     129,  1378,    -1,    71,    -1,  1338,    -1,   136,    -1,    -1,
      -1,  1319,  1320,   142,   143,    -1,    -1,    -1,  1326,  1396,
      -1,  1385,   474,   475,   144,    -1,    -1,   156,   148,   149,
    1338,   151,   152,   153,    -1,  1385,  1400,    -1,    -1,    -1,
     492,    -1,   171,    -1,    -1,    -1,    -1,   182,    -1,    -1,
    1400,    -1,  1385,   188,    -1,    -1,   124,    -1,    -1,   194,
      -1,    42,    43,    44,    45,    46,    47,  1400,    49,    -1,
      -1,    -1,    71,   141,    -1,    -1,   144,  1385,   146,    -1,
     148,   149,    -1,   151,   152,   153,    63,    25,    -1,   541,
      -1,    -1,  1400,    -1,    71,    -1,    73,    -1,   166,    -1,
      -1,    71,    -1,    73,    74,    -1,   174,   242,    -1,    -1,
      -1,    -1,    -1,    51,   249,   250,   251,    -1,    -1,    -1,
      -1,   256,    -1,   575,    -1,   577,    -1,   262,    -1,    -1,
      -1,    -1,    -1,    71,   111,    -1,    -1,    -1,    -1,    49,
      -1,    -1,   141,   595,   596,   144,    -1,   146,    -1,   148,
     149,    -1,   151,   152,   153,   607,   608,   609,   610,   611,
      -1,    -1,    -1,    -1,   141,    -1,    -1,   144,    -1,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,   148,   149,
     632,   151,   152,   153,    -1,    -1,   124,   639,    -1,   641,
     642,    -1,    -1,    -1,    25,    -1,    -1,   174,    -1,    -1,
      -1,   336,   179,   141,   656,    -1,   144,    -1,   146,    -1,
     148,   149,    -1,   151,   152,   153,    -1,   669,    -1,   157,
      51,    -1,    -1,   675,    -1,    -1,    -1,    -1,   166,   681,
      -1,    -1,   684,     9,    10,    11,   174,   372,    -1,    -1,
      71,    -1,    -1,    -1,    -1,   380,    -1,    -1,    -1,    25,
     702,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    -1,    -1,    -1,    41,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   423,    -1,
      -1,   743,    71,   124,    73,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   437,    -1,   756,    -1,   758,    -1,    -1,    -1,
     141,   221,   222,   144,    -1,   146,   226,   148,   149,   771,
     151,   152,   153,    -1,   776,    -1,   157,    -1,    -1,    -1,
     782,    -1,   111,   785,    -1,   166,    -1,    99,    -1,    -1,
      -1,    -1,   104,   174,   106,   107,   108,   109,   110,   111,
     112,    -1,    -1,    -1,    -1,    41,    -1,   492,    -1,   811,
      -1,    -1,   141,    -1,    -1,   144,   818,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   292,    -1,    -1,   295,   148,   149,    41,   151,
      -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,   177,    -1,
     179,    -1,   854,    -1,    -1,   181,   541,    -1,    71,    -1,
      73,   173,    -1,    99,   866,   867,   178,   869,   104,    -1,
     106,   107,   108,   109,   110,   111,   112,    -1,   880,    -1,
      -1,   883,    -1,   885,    -1,    -1,    -1,    -1,    -1,    -1,
     575,    -1,   577,    -1,   896,    -1,    99,    -1,   111,   901,
      -1,   104,   904,   106,   107,   108,   109,   110,   111,   112,
     595,   596,   148,   149,    -1,   151,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   609,   610,   611,    -1,   141,    -1,
      -1,   144,    -1,   146,    -1,   148,   149,   173,   151,   152,
     153,    -1,   178,    -1,    -1,   148,   149,   632,   151,    -1,
      -1,    -1,    -1,    -1,   639,    -1,   641,   642,   418,    -1,
      -1,   174,    -1,   423,    -1,    -1,   179,    -1,    -1,    -1,
     173,   656,    -1,    -1,   976,   178,    -1,   437,   980,    -1,
      -1,    -1,   984,    -1,   986,    -1,    -1,    -1,   990,   423,
     992,   993,    -1,   995,    -1,    -1,   681,    41,    -1,   684,
    1002,    -1,    -1,   437,     9,    10,    11,    71,    -1,    73,
      -1,    -1,    -1,  1015,   474,   475,  1018,   702,    -1,    -1,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,   111,    -1,  1051,
      -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,   743,   123,
     104,    -1,   106,   107,   108,   109,   110,   111,   112,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,    -1,  1081,
     144,    -1,   146,    -1,   148,   149,   771,   151,   152,   153,
      -1,   776,    -1,    -1,    -1,  1097,    -1,   782,    -1,    -1,
      -1,    -1,    -1,    -1,   148,   149,    -1,   151,    -1,    -1,
     174,    -1,    -1,    71,    -1,   179,  1118,  1119,    -1,    -1,
    1122,  1123,    -1,    -1,    -1,  1127,   811,    -1,    -1,   173,
      -1,    -1,    -1,    -1,   178,    -1,   596,    -1,    -1,    -1,
      -1,   106,   107,   108,   109,   110,   111,   607,   608,   609,
     610,   611,   117,   118,   112,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   596,    -1,    -1,    -1,   124,    -1,    -1,   854,
      41,    -1,    -1,   178,    -1,   609,   610,   611,    -1,    -1,
      -1,   866,   867,   141,   869,    -1,   144,    -1,   146,   154,
     148,   149,    -1,   151,   152,   153,    -1,    -1,   883,    -1,
     885,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   173,   669,
      -1,   896,    -1,    -1,    -1,   675,   901,    -1,    -1,   904,
      -1,   681,    -1,    41,    -1,    -1,    -1,    -1,    99,    -1,
    1232,    -1,    -1,   104,    -1,   106,   107,   108,   109,   110,
     111,   112,    -1,    -1,    -1,    -1,    -1,   681,  1250,    -1,
      -1,    -1,  1254,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1265,    -1,    -1,    -1,    -1,  1270,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1278,   148,   149,    -1,
     151,    99,    -1,    -1,    -1,    -1,   104,    -1,   106,   107,
     108,   109,   110,   111,   112,   980,   756,    -1,   758,   984,
      -1,   986,   173,    -1,    -1,    -1,    -1,   178,   993,    -1,
     995,   771,    -1,    -1,  1316,    -1,   776,    -1,    -1,    -1,
      -1,    -1,   782,    -1,    -1,   785,    -1,  1329,    -1,    -1,
     148,   149,    -1,   151,    -1,    -1,    -1,   771,    -1,    -1,
      -1,    -1,   776,    -1,    -1,    -1,    -1,    -1,   782,    -1,
      -1,    -1,    -1,  1355,    -1,   173,    -1,    -1,   818,    -1,
     178,    -1,  1364,    -1,    -1,  1367,  1051,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1380,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1388,    -1,    -1,    -1,
      -1,    -1,  1394,    -1,    -1,    -1,  1398,    -1,    -1,  1401,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   869,
      -1,    -1,  1097,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     880,    -1,    -1,   883,    -1,   885,    71,    -1,    73,    -1,
      -1,    -1,    -1,  1118,  1119,   869,    -1,  1122,  1123,    -1,
      -1,    -1,  1127,    -1,    -1,    -1,    -1,    -1,    -1,   883,
      -1,   885,    -1,    -1,   423,     9,    10,    11,    71,    -1,
      73,    -1,    -1,    -1,    -1,    -1,   111,    -1,   437,    -1,
      -1,    25,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,   141,    -1,   111,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
     123,    -1,    -1,    -1,    -1,    -1,   976,    -1,    -1,    -1,
     980,    -1,    -1,    -1,   984,    -1,   986,    -1,   141,   174,
     990,   144,   992,   146,   179,   148,   149,    -1,   151,   152,
     153,    -1,  1002,    -1,    -1,    -1,   980,  1232,    -1,    -1,
     984,    -1,   986,    -1,    -1,  1015,    -1,    -1,  1018,    -1,
      -1,   174,    -1,    -1,    -1,    -1,   179,    -1,    -1,  1254,
      -1,    -1,    -1,    -1,    -1,    -1,     9,    10,    11,    -1,
    1265,    -1,    -1,    -1,    -1,  1270,    -1,    -1,    -1,    -1,
      -1,  1051,    25,  1278,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,  1051,    -1,    -1,
      -1,  1081,    -1,    -1,   178,    -1,    -1,   596,    -1,    -1,
      -1,  1316,    -1,    -1,    -1,    -1,    -1,  1097,    -1,    -1,
     609,   610,    -1,    -1,  1329,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    -1,  1118,  1119,
      -1,    -1,  1122,  1097,    -1,    -1,    -1,    -1,    -1,    -1,
    1355,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1367,    -1,  1118,  1119,    -1,    -1,  1122,    -1,
      -1,    59,    60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1388,    -1,    -1,    -1,    -1,    -1,  1394,
      -1,    -1,   681,  1398,     5,     6,  1401,     8,     9,    10,
      -1,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    -1,    -1,    26,    27,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   178,    -1,    38,    -1,    -1,
      -1,    -1,    -1,   121,    45,    -1,    47,    -1,    -1,    50,
      -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1232,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    -1,    -1,    -1,    -1,    -1,    79,    -1,
    1250,    -1,    -1,    -1,  1254,    -1,    -1,    -1,  1232,    -1,
      -1,    -1,    93,    -1,    95,  1265,    -1,   776,    51,    -1,
    1270,    -1,    -1,   782,    -1,    -1,    -1,    -1,  1278,    -1,
    1254,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,
      -1,  1265,    -1,    -1,    -1,    -1,  1270,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1278,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,  1316,    -1,    -1,    -1,
      -1,    -1,    -1,   106,   107,   108,   109,   110,   111,  1329,
      -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   124,  1316,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,  1329,    49,    -1,   141,    -1,
     869,   144,    -1,   146,  1364,   148,   149,    -1,   151,   152,
     153,    -1,   203,    -1,   883,   206,   885,    -1,    -1,    -1,
    1380,   212,   213,   166,    -1,    -1,    -1,    -1,  1388,    -1,
     173,   174,    -1,    -1,  1394,    -1,    -1,    -1,  1398,    -1,
      -1,  1401,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,  1388,    -1,    -1,    -1,    -1,    -1,
    1394,    -1,    -1,    -1,  1398,    -1,   257,  1401,    -1,    -1,
     261,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,
      -1,    -1,   283,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   294,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   980,    -1,    -1,    -1,   984,    -1,   986,    -1,    -1,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,    -1,    -1,   338,   339,   340,
     121,    -1,    -1,    -1,    -1,    -1,   347,   348,   349,   350,
     351,   352,   353,   354,   355,   356,   357,   358,    -1,    -1,
      -1,    -1,    -1,   364,   365,    -1,   367,   368,   369,    -1,
      -1,    -1,  1051,    -1,   375,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   385,    -1,   387,    -1,    -1,    -1,
      -1,    -1,   393,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   403,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1097,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,
      -1,    -1,   433,    12,    -1,    -1,    -1,    -1,    -1,  1118,
    1119,    -1,    -1,  1122,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,   466,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,
      79,    -1,    -1,   504,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,
     109,   110,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,
      -1,   542,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,  1232,    -1,    -1,   557,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,   148,
     149,    -1,   151,   152,   153,  1254,    -1,   156,    -1,    -1,
      -1,   160,   583,    -1,    -1,    -1,  1265,   166,    -1,    -1,
      -1,  1270,   171,   594,   173,   174,     9,    10,    11,  1278,
     179,   180,    -1,   182,   183,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,  1316,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   650,
    1329,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   663,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   679,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   689,    -1,
      -1,   692,    -1,   694,    -1,    -1,   697,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   706,    -1,    -1,    -1,  1388,
      -1,    -1,     9,    10,    11,  1394,    -1,    -1,    -1,  1398,
      -1,    -1,  1401,    -1,    -1,    -1,    -1,    -1,    25,   730,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   178,    -1,    -1,    -1,    -1,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,   794,    49,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   807,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,
      10,    11,    -1,    -1,    -1,    -1,   827,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   835,    25,   837,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    -1,   864,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   873,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,   178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,   913,    49,    -1,    -1,   917,    -1,   919,    -1,
      -1,   176,   923,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    12,   934,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   942,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,   178,    69,
      70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,
      -1,    91,  1013,    -1,    -1,    95,    96,    97,    98,    99,
     100,   101,    -1,   103,   104,   105,   106,   107,   108,   109,
     110,   111,    -1,   113,   114,   115,   116,   117,   118,    -1,
      -1,    -1,   178,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,   146,    -1,   148,   149,
      -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,
     160,     3,     4,     5,     6,     7,   166,   167,  1089,   169,
      12,   171,   172,   173,   174,    -1,   176,   177,   178,   179,
     180,    -1,   182,   183,    -1,    -1,    -1,    -1,    -1,    -1,
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
     172,   173,   174,    -1,   176,   177,   178,   179,   180,    -1,
     182,   183,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,
      84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,
      -1,    95,    96,    97,    98,    99,   100,   101,    -1,   103,
     104,   105,   106,   107,   108,   109,   110,   111,    -1,   113,
     114,   115,   116,   117,   118,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,   146,    -1,   148,   149,    -1,   151,   152,   153,
     154,    -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,
       6,     7,   166,   167,    -1,   169,    12,   171,   172,   173,
     174,    -1,   176,   177,    -1,   179,   180,    -1,   182,   183,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     166,    -1,    -1,    -1,    12,   171,   172,   173,   174,    -1,
     176,   177,   178,   179,   180,    -1,   182,   183,    -1,    -1,
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
      -1,    -1,    12,   171,   172,   173,   174,    -1,   176,   177,
     178,   179,   180,    -1,   182,   183,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,
      80,    81,    82,    83,    84,    -1,    86,    -1,    88,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,
     100,   101,    -1,   103,    -1,   105,   106,   107,   108,   109,
     110,   111,    -1,   113,   114,   115,    -1,   117,   118,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,   146,    -1,   148,   149,
      -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,
     160,     3,     4,     5,     6,     7,   166,    -1,    -1,    -1,
      12,   171,   172,   173,   174,    -1,   176,   177,    -1,   179,
     180,    -1,   182,   183,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,
      82,    -1,    84,    -1,    86,    -1,    88,    89,    -1,    91,
      -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,   101,
      -1,   103,    -1,   105,   106,   107,   108,   109,   110,   111,
      -1,   113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,   146,    -1,   148,   149,    -1,   151,
     152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,     3,
       4,     5,     6,     7,   166,    -1,    -1,    -1,    12,   171,
     172,   173,   174,    -1,   176,   177,    -1,   179,   180,    -1,
     182,   183,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     174,    -1,   176,   177,   178,   179,   180,    -1,   182,   183,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,    75,
      -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,
      86,    87,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,
      96,    97,    98,    -1,   100,   101,    -1,   103,    -1,   105,
     106,   107,   108,   109,   110,   111,    -1,   113,   114,   115,
      -1,   117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
     146,    -1,   148,   149,    -1,   151,   152,   153,   154,    -1,
     156,    -1,    -1,    -1,   160,     3,     4,     5,     6,     7,
     166,    -1,    -1,    -1,    12,   171,   172,   173,   174,    -1,
     176,   177,    -1,   179,   180,    -1,   182,   183,    -1,    -1,
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
      -1,    -1,    12,   171,   172,   173,   174,    -1,   176,   177,
     178,   179,   180,    -1,   182,   183,    -1,    -1,    -1,    -1,
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
      12,   171,   172,   173,   174,    -1,   176,   177,   178,   179,
     180,    -1,   182,   183,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,
      82,    -1,    84,    85,    86,    -1,    88,    -1,    -1,    91,
      -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,   101,
      -1,   103,    -1,   105,   106,   107,   108,   109,   110,   111,
      -1,   113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,   146,    -1,   148,   149,    -1,   151,
     152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,     3,
       4,     5,     6,     7,   166,    -1,    -1,    -1,    12,   171,
     172,   173,   174,    -1,   176,   177,    -1,   179,   180,    -1,
     182,   183,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     174,    -1,   176,   177,   178,   179,   180,    -1,   182,   183,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     166,    -1,    -1,    -1,    12,   171,   172,   173,   174,    -1,
     176,   177,   178,   179,   180,    -1,   182,   183,    -1,    -1,
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
      -1,    -1,    12,   171,   172,   173,   174,    -1,   176,   177,
     178,   179,   180,    -1,   182,   183,    -1,    -1,    -1,    -1,
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
      12,   171,   172,   173,   174,    -1,   176,   177,   178,   179,
     180,    -1,   182,   183,    -1,    -1,    -1,    -1,    -1,    -1,
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
     172,   173,   174,    -1,   176,   177,   178,   179,   180,    -1,
     182,   183,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     174,    -1,   176,   177,   178,   179,   180,    -1,   182,   183,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     166,    -1,    -1,    -1,    12,   171,   172,   173,   174,    -1,
     176,   177,    -1,   179,   180,    -1,   182,   183,    26,    -1,
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
      -1,    -1,   160,     3,     4,     5,     6,     7,   166,    -1,
      -1,    -1,    12,   171,   172,   173,   174,    -1,   176,   177,
      -1,   179,   180,    -1,   182,   183,    26,    -1,    -1,    -1,
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
      12,   171,   172,   173,   174,    -1,   176,   177,    -1,   179,
     180,    -1,   182,   183,    26,    -1,    -1,    -1,    -1,    -1,
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
     172,   173,   174,    -1,   176,   177,    -1,   179,   180,    -1,
     182,   183,    26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,
       6,     7,   166,    -1,    -1,    -1,    12,   171,   172,   173,
     174,    -1,   176,   177,    -1,   179,   180,    -1,   182,   183,
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
     156,    -1,    -1,    -1,   160,     3,     4,     5,     6,     7,
     166,    -1,    -1,    -1,    12,   171,   172,   173,   174,    -1,
     176,   177,    -1,   179,   180,    -1,   182,   183,    -1,    -1,
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
      -1,    -1,   160,     3,     4,     5,     6,     7,   166,    -1,
      -1,    -1,    12,   171,   172,   173,   174,    -1,   176,   177,
      -1,   179,   180,    -1,   182,   183,    -1,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    98,    49,
      -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,   109,
     110,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,
     160,     3,     4,     5,     6,     7,   166,    -1,    -1,    -1,
      12,   171,   172,   173,   174,    -1,    -1,    -1,    -1,   179,
     180,    -1,   182,   183,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    -1,    79,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,   107,   108,   109,   110,   111,
      -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,    -1,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,
       4,     5,     6,     7,   166,    -1,    -1,    -1,    12,   171,
     172,   173,   174,    -1,   176,    -1,    -1,   179,   180,    -1,
     182,   183,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    -1,    79,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   106,   107,   108,   109,   110,   111,    -1,    -1,
     114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,   146,    -1,   148,   149,    -1,   151,   152,   153,
      -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,
       6,     7,   166,    -1,    -1,    -1,    12,   171,   172,   173,
     174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     166,    -1,    -1,    -1,    12,   171,    -1,   173,   174,    -1,
     176,    -1,    -1,   179,   180,    -1,   182,   183,    -1,    -1,
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
      -1,    -1,    12,   171,    -1,   173,   174,    -1,   176,    -1,
      -1,   179,   180,    -1,   182,   183,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    95,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,   109,
     110,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,
     160,     3,     4,     5,     6,     7,   166,    -1,    -1,    -1,
      12,   171,    -1,   173,   174,    -1,    -1,    -1,    -1,   179,
     180,    -1,   182,   183,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   173,   174,   175,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,
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
     174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,
      26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     166,    -1,    -1,    -1,    12,   171,    -1,   173,   174,    -1,
      -1,    -1,    -1,   179,   180,    -1,   182,   183,    -1,    -1,
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
      -1,    -1,    12,   171,    -1,   173,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,    -1,    -1,    -1,    -1,
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
      12,   171,    -1,   173,   174,    -1,    -1,    -1,    -1,   179,
     180,    -1,   182,   183,    -1,    -1,    -1,    -1,    -1,    31,
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
      -1,   173,   174,    -1,    -1,    -1,    -1,   179,   180,    -1,
     182,   183,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,
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
     174,    -1,    -1,    -1,    -1,   179,   180,    -1,   182,   183,
      -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,
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
     166,    -1,    -1,    -1,    12,   171,    -1,   173,   174,    -1,
      -1,    -1,    -1,   179,   180,    -1,   182,   183,    -1,    -1,
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
      -1,    -1,    12,   171,    -1,   173,   174,    -1,    -1,    -1,
      -1,   179,   180,    -1,   182,   183,    -1,    -1,    -1,    -1,
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
      -1,     3,     4,     5,     6,     7,   136,     9,    10,    11,
      12,   141,   142,   143,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,
     160,    -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,    -1,
      -1,   171,    -1,   173,   174,    -1,    -1,    49,    -1,   179,
     180,    -1,   182,   183,    -1,    -1,    -1,    -1,    -1,    -1,
      62,    63,    64,    65,    66,    67,    68,    -1,    -1,    71,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,
     142,   143,    -1,   145,    -1,    -1,   148,   149,    -1,   151,
     152,   153,   154,   155,   156,    -1,    -1,   159,     9,    10,
      11,    -1,    -1,    -1,   166,   167,    -1,   169,    -1,   171,
     172,   173,    -1,    -1,    25,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,     9,    10,    11,    -1,    -1,   178,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,     9,    10,    11,    -1,    -1,   178,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,     9,    10,    11,    -1,    -1,   178,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,     9,    10,    11,   176,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,     9,    10,    11,   176,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,     9,    10,    11,
     176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      -1,   176,    42,    43,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    61,    -1,    -1,    -1,     9,    10,    11,    -1,    69,
      70,    71,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    79,
      -1,    25,   176,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   175,    -1,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,
      -1,    -1,    -1,   175,    -1,    -1,   166,     9,    10,    11,
      -1,   171,    -1,    -1,    -1,    -1,    -1,    -1,   122,    -1,
      -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
     122,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,   122,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,   122,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   122,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      90,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    11,    49,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    25,    49,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   185,   186,     0,   187,     3,     4,     5,     6,     7,
      12,    41,    42,    43,    48,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    69,    70,    71,    72,    73,    75,    79,    80,    81,
      82,    84,    86,    88,    91,    95,    96,    97,    98,    99,
     100,   101,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   113,   114,   115,   116,   117,   118,   123,   124,   125,
     126,   127,   128,   129,   136,   141,   142,   143,   144,   145,
     146,   148,   149,   151,   152,   153,   154,   156,   160,   166,
     167,   169,   171,   172,   173,   174,   176,   177,   179,   180,
     182,   183,   188,   191,   194,   195,   196,   197,   198,   199,
     202,   213,   214,   218,   223,   229,   283,   284,   289,   293,
     294,   295,   296,   297,   298,   299,   300,   308,   309,   310,
     312,   313,   316,   326,   327,   328,   333,   336,   354,   359,
     361,   362,   363,   364,   365,   366,   367,   368,   370,   383,
     385,   387,   109,   110,   111,   123,   141,   191,   213,   283,
     299,   361,   299,   174,   299,   299,   299,   352,   353,   299,
     299,   299,   299,   299,   299,   299,   299,   299,   299,   299,
     299,   111,   174,   195,   327,   328,   361,   361,    31,   299,
     374,   375,   299,   111,   174,   195,   327,   328,   329,   360,
     366,   371,   372,   174,   290,   330,   174,   290,   291,   299,
     204,   290,   174,   174,   174,   290,   176,   299,   191,   176,
     299,    25,    51,   124,   146,   166,   174,   191,   198,   388,
     398,   399,   176,   296,   299,   177,   299,   144,   192,   193,
     194,    73,   179,   255,   256,   117,   117,    73,   257,   174,
     174,   174,   174,   191,   227,   389,   174,   174,    73,    78,
     137,   138,   139,   380,   381,   144,   177,   194,   194,    95,
     299,   228,   389,   146,   174,   389,   389,   299,   289,   299,
     300,   361,   200,   177,    78,   331,   380,    78,   380,   380,
      26,   144,   162,   390,   174,     8,   176,    31,   212,   146,
     226,   389,   111,   213,   284,   176,   176,   176,   176,   176,
     176,     9,    10,    11,    25,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    49,   176,    61,    61,
     177,   140,   118,   154,   229,   282,   283,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    59,
      60,   121,   356,   357,    61,   177,   358,   174,    61,   177,
     179,   367,   174,   212,   213,    13,   299,    40,   191,   351,
     174,   289,   361,   140,   361,   122,   181,     8,   338,   289,
     361,   390,   140,   174,   332,   121,   356,   357,   358,   175,
     299,    26,   202,     8,   176,   202,   203,   291,   292,   299,
     191,   241,   206,   176,   176,   176,   399,   399,   162,   174,
      98,   391,   399,    13,   191,   176,   176,   200,   176,   194,
       8,   176,    90,   177,   361,     8,   176,    13,     8,   176,
     361,   384,   384,   361,   175,   162,   221,   123,   361,   373,
     374,    61,   121,   137,   381,    72,   299,   361,    78,   137,
     381,   194,   190,   176,   177,   176,   122,   224,   317,   319,
      79,   303,   304,   306,    13,    90,   386,   175,   175,   175,
     178,   201,   202,   214,   218,   223,   299,   180,   182,   183,
     191,   391,    31,   253,   254,   299,   388,   174,   389,   219,
     212,   299,   299,   299,    26,   299,   299,   299,   299,   299,
     299,   299,   299,   299,   299,   299,   299,   299,   299,   299,
     299,   299,   299,   299,   299,   299,   299,   329,   299,   369,
     369,   299,   376,   377,   191,   366,   367,   227,   228,   226,
     213,    31,   145,   293,   296,   299,   299,   299,   299,   299,
     299,   299,   299,   299,   299,   299,   299,   177,   191,   366,
     369,   299,   253,   369,   299,   373,   212,   175,   174,   350,
       8,   338,   289,   175,   191,    31,   299,    31,   299,   175,
     175,   366,   253,   177,   191,   366,   175,   200,   245,   299,
      82,    26,   202,   239,   176,    90,    13,     8,   175,    26,
     177,   242,   399,    79,   395,   396,   397,   174,     8,    42,
      43,    61,   124,   136,   146,   166,   195,   196,   198,   311,
     327,   333,   334,   335,   178,    90,   193,   191,   299,   256,
     334,    73,     8,   175,   175,   175,   176,   191,   394,   119,
     232,   174,     8,   175,   175,    73,    74,   191,   382,   191,
      61,   178,   178,   187,   189,   299,   120,   231,   161,    46,
     146,   161,   321,   122,     8,   338,   175,   399,   399,    13,
     121,   356,   357,   358,   178,     8,   163,   361,   175,     8,
     339,    13,   301,   215,   119,   230,   174,   389,   299,    26,
     181,   181,   122,   178,     8,   338,   390,   174,   222,   225,
     220,   212,    63,   361,   299,   390,   174,   181,   178,   175,
     181,   178,   175,    42,    43,    61,    69,    70,    79,   124,
     136,   166,   191,   341,   343,   346,   349,   191,   361,   361,
     122,   356,   357,   358,   175,   299,   246,    66,    67,   247,
     290,   200,   292,    31,   123,   236,   361,   334,   191,    26,
     202,   240,   176,   243,   176,   243,     8,   163,   122,     8,
     338,   175,   157,   391,   392,   399,   334,   334,   334,   337,
     340,   174,    78,   140,   174,   140,   177,   102,   209,   210,
     191,   178,    13,   361,   176,    90,     8,   163,   233,   327,
     177,   373,   123,   361,    13,   181,   299,   178,   187,   233,
     177,   320,    13,   299,   303,   176,   399,   177,   191,   366,
     399,    31,   299,   334,   157,   251,   252,   354,   355,   174,
     327,   231,   302,   216,   299,   299,   299,   174,   253,   232,
     231,   230,   389,   329,   178,   174,   253,    13,    69,    70,
     191,   342,   342,   343,   344,   345,   174,    78,   137,   174,
       8,   338,   175,   350,    31,   299,   178,    66,    67,   248,
     290,   202,   176,    83,   176,   361,   174,   122,   235,    13,
     200,   243,    92,    93,    94,   243,   178,   399,   399,   395,
       8,   175,   175,   122,   181,     8,   338,   337,   191,   303,
     305,   307,   191,   334,   378,   379,   174,   159,   334,   399,
     191,     8,   258,   175,   174,   293,   296,   299,   181,   178,
     258,   147,   160,   177,   316,   323,   147,   177,   322,   122,
     176,   299,   390,   174,   361,   175,     8,   339,   399,   400,
     251,   177,   251,   174,   122,   253,   175,   177,   177,   231,
     217,   332,   174,   253,   175,   122,   181,     8,   338,   344,
     137,   303,   347,   348,   343,   361,   290,    26,    68,   202,
     176,   292,   373,   236,   175,   334,    89,    92,   176,   299,
      26,   176,   244,   178,   163,   157,    26,   334,   334,   175,
     122,     8,   338,   175,   122,   178,     8,   338,   327,   177,
      90,   327,    99,   104,   112,   148,   149,   151,   178,   259,
     281,   282,   283,   288,   354,   373,   178,   178,    46,   299,
     299,   299,   178,   174,   253,    26,   393,   157,   355,    31,
      73,   175,   258,   175,   251,   299,   175,   258,   258,   177,
     174,   253,   175,   343,   343,   175,   122,   175,     8,   338,
      26,   200,   176,   175,   175,   207,   176,   176,   244,   200,
     399,   122,   334,   303,   334,   334,    73,   200,   399,   388,
     234,   327,   112,   124,   146,   152,   268,   269,   270,   327,
     150,   274,   275,   115,   174,   191,   276,   277,   260,   213,
     399,     8,   176,   282,   175,   146,   318,   178,   178,   174,
     253,   175,   399,   104,   314,   400,    73,    13,   393,   178,
     393,   175,   178,   178,   258,   251,   175,   122,   343,   303,
     200,   205,    26,   202,   238,   200,   175,   334,   122,   122,
     175,   178,    13,     8,   176,   177,   177,     8,   176,     3,
       4,     5,     6,     7,     9,    10,    11,    12,    49,    62,
      63,    64,    65,    66,    67,    68,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   123,   124,   125,
     126,   127,   128,   129,   141,   142,   143,   145,   154,   155,
     156,   159,   166,   167,   169,   171,   172,   173,   191,   324,
     325,     8,   176,   146,   150,   191,   277,   278,   279,   176,
      73,   287,   212,   261,   388,   213,   253,   175,   174,   177,
      31,    73,    13,   334,   177,   314,   393,   178,   175,   343,
     122,    26,   202,   237,   200,   334,   334,   177,   334,   327,
     264,   271,   333,   269,    13,    26,    43,   272,   275,     8,
      29,   175,    25,    42,    45,    13,     8,   176,   389,   287,
      13,   212,   175,    31,    73,   315,   200,    73,    13,   334,
     200,   177,   177,   393,   343,   200,    87,   200,   178,   191,
     198,   265,   266,   267,     8,   178,   334,   325,   325,    51,
     273,   278,   278,    25,    42,    45,   334,    73,   174,   176,
     334,   389,    73,     8,   339,   178,    13,   334,   178,   200,
     200,   177,    85,   176,   178,   140,    90,   333,   153,    13,
     262,   174,    31,    73,   175,   334,   178,   178,   200,   176,
     208,   191,   282,   283,   334,   157,   249,   250,   355,   263,
      73,   178,   102,   209,   211,   155,   191,   176,   175,     8,
     339,   106,   107,   108,   285,   286,   249,   174,   234,   176,
     393,   157,   285,   400,   175,   327,   176,   176,   177,   280,
     355,    31,    73,   393,    73,   200,   400,    73,    13,   280,
     175,   178,    31,    73,    13,   334,   177,    73,    13,   334,
     200,    13,   334,   178,   334
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
#line 706 "hphp.y"
    { _p->initParseTree(); ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 706 "hphp.y"
    { _p->popLabelInfo();
                                                  _p->finiParseTree();;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 712 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 713 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 716 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 717 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 718 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 719 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 720 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 721 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 724 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 726 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 727 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 728 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 729 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 730 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 731 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 736 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 737 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 738 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 739 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 740 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 741 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 746 "hphp.y"
    { ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 747 "hphp.y"
    { ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 750 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 751 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 752 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 754 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 758 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 760 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 763 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 765 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 766 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 769 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 776 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 783 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 791 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 794 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 800 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 801 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 804 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 806 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 807 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 810 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 814 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 819 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 820 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 821 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 824 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 826 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 829 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 830 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 832 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 833 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 835 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 836 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 837 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 838 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 839 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 840 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 841 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 842 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 843 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 844 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 845 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 846 "hphp.y"
    { (yyval).reset(); (yyval) = ';';}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 847 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 850 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 851 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]));;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 854 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 862 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(3) - (14)]),(yyvsp[(7) - (14)]),(yyvsp[(8) - (14)]),(yyvsp[(11) - (14)]),(yyvsp[(13) - (14)]),(yyvsp[(14) - (14)]));;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 865 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 866 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 867 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval)); ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 871 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 872 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 873 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 874 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 875 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 876 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 877 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); ;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 878 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 879 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval)); ;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 891 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 892 "hphp.y"
    { (yyval).reset();;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 896 "hphp.y"
    { finally_statement(_p);;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 898 "hphp.y"
    { _p->onFinally((yyval), (yyvsp[(4) - (5)]));;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 903 "hphp.y"
    { (yyval).reset();;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 907 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 908 "hphp.y"
    { (yyval).reset();;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 912 "hphp.y"
    { _p->pushFuncLocation();;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 917 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 922 "hphp.y"
    { _p->onFunction((yyval),0,(yyvsp[(8) - (11)]),(yyvsp[(2) - (11)]),(yyvsp[(3) - (11)]),(yyvsp[(6) - (11)]),(yyvsp[(10) - (11)]),0,false);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(3) - (12)]),(yyvsp[(4) - (12)]),(yyvsp[(7) - (12)]),(yyvsp[(11) - (12)]),0,false);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 937 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 942 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (13)]),(yyvsp[(10) - (13)]),(yyvsp[(4) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(8) - (13)]),(yyvsp[(12) - (13)]),&(yyvsp[(1) - (13)]),false);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 949 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 952 "hphp.y"
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

  case 104:

/* Line 1455 of yacc.c  */
#line 967 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 970 "hphp.y"
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

  case 106:

/* Line 1455 of yacc.c  */
#line 984 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 987 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 992 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 995 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 1005 "hphp.y"
    { Token t_ext, t_imp;
                                         t_ext.reset(); t_imp.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,t_imp,
                                                     (yyvsp[(6) - (7)]), 0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 1013 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 1016 "hphp.y"
    { Token t_ext, t_imp;
                                         t_ext.reset(); t_imp.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,t_imp,
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 1024 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 1025 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 1029 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 1032 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 1035 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 1036 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 1037 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 1041 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 1042 "hphp.y"
    { (yyval).reset();;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 1045 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1046 "hphp.y"
    { (yyval).reset();;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1049 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1050 "hphp.y"
    { (yyval).reset();;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1053 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1055 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1058 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1060 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1064 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1065 "hphp.y"
    { (yyval).reset();;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1068 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1069 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1070 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1074 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1076 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1079 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1081 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1084 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1086 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1089 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1091 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1101 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1102 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1103 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1104 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1109 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1111 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1112 "hphp.y"
    { (yyval).reset();;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1115 "hphp.y"
    { (yyval).reset();;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1116 "hphp.y"
    { (yyval).reset();;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1121 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1122 "hphp.y"
    { (yyval).reset();;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1127 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1128 "hphp.y"
    { (yyval).reset();;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1131 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1132 "hphp.y"
    { (yyval).reset();;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1135 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1136 "hphp.y"
    { (yyval).reset();;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1141 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]);;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1143 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1144 "hphp.y"
    { (yyval).reset();;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1145 "hphp.y"
    { (yyval).reset();;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1151 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1155 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1160 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1165 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1170 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(4) - (6)]),&(yyvsp[(3) - (6)]));;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1175 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(4) - (7)]),&(yyvsp[(3) - (7)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1181 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(4) - (9)]),&(yyvsp[(3) - (9)]));;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1187 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(4) - (8)]),&(yyvsp[(3) - (8)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1193 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]);;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1195 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1196 "hphp.y"
    { (yyval).reset();;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1197 "hphp.y"
    { (yyval).reset();;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1202 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),0,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL);;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1205 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),1,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL);;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1209 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),1,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL);;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1213 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),0,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL);;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1217 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),0,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL);;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1221 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),1,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL);;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1226 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),1,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL);;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1231 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),0,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL);;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1237 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1238 "hphp.y"
    { (yyval).reset();;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1241 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),0);;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1242 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),1);;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1244 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1246 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1250 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1251 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1254 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1255 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1256 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1260 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1262 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1263 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1264 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1269 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1270 "hphp.y"
    { (yyval).reset();;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1274 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1277 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1278 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1280 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1284 "hphp.y"
    { _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1289 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),0);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1295 "hphp.y"
    { _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1300 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1304 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1308 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1309 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1312 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1315 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1316 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1317 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1323 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1327 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1330 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1337 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1338 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1343 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1346 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1353 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1355 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1359 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1360 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1366 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1368 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1372 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1374 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1378 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1379 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1383 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1384 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1388 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1391 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1396 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1401 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1402 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1404 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1408 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1409 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1410 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1411 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1415 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1416 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1417 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1418 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1419 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1421 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1423 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1427 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1430 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1431 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1435 "hphp.y"
    { (yyval).reset();;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1436 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1439 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1440 "hphp.y"
    { (yyval).reset();;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1443 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1444 "hphp.y"
    { (yyval).reset();;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1447 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1449 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1452 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1453 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1454 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1455 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1456 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1457 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1458 "hphp.y"
    { (yyval) = T_ASYNC;;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1462 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1463 "hphp.y"
    { (yyval).reset();;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1466 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1467 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1468 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1472 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1474 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1475 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1476 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1480 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1481 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1485 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1487 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1488 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1489 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1492 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1497 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1501 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1502 "hphp.y"
    { (yyval).reset();;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1506 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1507 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1511 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1516 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1520 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); ;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1524 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1529 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1533 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1534 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1535 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1539 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1540 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1545 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1546 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1547 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1550 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1552 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1553 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1555 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1556 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1557 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1558 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1559 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1560 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1563 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1566 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1567 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1568 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1573 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1575 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1578 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1580 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1581 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1582 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1584 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1585 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1587 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1588 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1591 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1592 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1593 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1594 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1595 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1596 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1597 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1598 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1600 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1601 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1603 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1604 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1605 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1606 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1607 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1608 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { Token t; _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1615 "hphp.y"
    { Token u; u.reset();
                                         _p->onClosure((yyval),0,u,(yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1619 "hphp.y"
    { Token t; _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1623 "hphp.y"
    { Token u; u.reset();
                                         _p->onClosure((yyval),&(yyvsp[(1) - (12)]),u,(yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1626 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1627 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1631 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1638 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1641 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1651 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1657 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1662 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1663 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1667 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1671 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1672 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1677 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1684 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1693 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1697 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1698 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1699 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1704 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { (yyval).reset();;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1709 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1710 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1719 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1722 "hphp.y"
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

  case 404:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
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

  case 405:

/* Line 1455 of yacc.c  */
#line 1744 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1745 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1751 "hphp.y"
    { (yyval).reset();;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { (yyval).reset();;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1852 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1858 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1864 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1871 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1875 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1876 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1879 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1880 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1886 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1891 "hphp.y"
    { (yyval).reset();;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1892 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1893 "hphp.y"
    { (yyval).reset();;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1897 "hphp.y"
    { (yyval).reset();;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1899 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { (yyval).reset();;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { (yyval).reset();;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 1969 "hphp.y"
    { (yyval).reset();;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 1970 "hphp.y"
    { (yyval).reset();;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 1974 "hphp.y"
    { (yyval).reset();;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 1980 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 1982 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 1984 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 1985 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 1989 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 1990 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 1991 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 1995 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 1997 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2000 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2001 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2002 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2005 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2006 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2007 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2008 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2010 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2011 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2013 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2018 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2019 "hphp.y"
    { (yyval).reset();;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2024 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2026 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2028 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2029 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2033 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2034 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2039 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2040 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2045 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2048 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2053 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2054 "hphp.y"
    { (yyval).reset();;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2057 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2058 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2065 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2067 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2070 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2072 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2075 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2078 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2079 "hphp.y"
    { (yyval).reset();;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2083 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2085 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2089 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2090 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2094 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2095 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2099 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2101 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2106 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2108 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2112 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2114 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2115 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2116 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2117 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2124 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2125 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2129 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2132 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2136 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2139 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2151 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2154 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2161 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2165 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2169 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2176 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2184 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2186 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2191 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2193 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2200 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2201 "hphp.y"
    { (yyval).reset();;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2205 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { (yyval)++;;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2210 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2211 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2214 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2222 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2227 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2236 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { (yyval).reset();;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2348 "hphp.y"
    { _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); ;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); ;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); ;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    {;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 2443 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2448 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2451 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 2461 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (5)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 10823 "new_hphp.tab.cpp"
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
#line 2473 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

