
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
     T_COMPILER_HALT_OFFSET = 407
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
#line 850 "new_hphp.tab.cpp"

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
#define YYLAST   10888

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  182
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  213
/* YYNRULES -- Number of rules.  */
#define YYNRULES  736
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1379

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   407

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    48,   180,     2,   177,    47,    31,   181,
     172,   173,    45,    42,     8,    43,    44,    46,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    26,   174,
      36,    13,    37,    25,    51,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    61,     2,   179,    30,     2,   178,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   175,    29,   176,    50,     2,     2,     2,
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
     164,   165,   166,   167,   168,   169,   170,   171
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
     280,   283,   286,   289,   299,   300,   301,   307,   309,   310,
     312,   313,   315,   316,   328,   329,   342,   343,   352,   353,
     363,   364,   372,   373,   382,   383,   391,   392,   401,   403,
     405,   407,   409,   411,   414,   417,   420,   421,   424,   425,
     428,   429,   431,   435,   437,   441,   444,   445,   447,   450,
     455,   457,   462,   464,   469,   471,   476,   478,   483,   487,
     493,   497,   502,   507,   513,   519,   524,   525,   527,   529,
     534,   535,   541,   542,   545,   546,   550,   551,   555,   558,
     560,   561,   566,   572,   580,   587,   594,   602,   612,   621,
     625,   628,   630,   631,   635,   640,   647,   653,   659,   666,
     675,   683,   686,   687,   689,   692,   696,   701,   705,   707,
     709,   712,   717,   721,   727,   729,   733,   736,   737,   738,
     743,   744,   750,   753,   754,   765,   766,   778,   782,   786,
     790,   794,   800,   803,   806,   807,   814,   820,   825,   829,
     831,   833,   837,   842,   844,   846,   848,   850,   855,   857,
     861,   864,   865,   868,   869,   871,   875,   877,   879,   881,
     883,   887,   892,   897,   902,   904,   906,   909,   912,   915,
     919,   923,   925,   927,   929,   931,   935,   937,   939,   941,
     942,   944,   947,   949,   951,   953,   955,   957,   959,   961,
     962,   964,   966,   968,   972,   978,   980,   984,   990,   995,
     999,  1003,  1006,  1008,  1012,  1016,  1018,  1020,  1021,  1024,
    1029,  1033,  1040,  1042,  1044,  1046,  1053,  1057,  1062,  1069,
    1073,  1077,  1081,  1085,  1089,  1093,  1097,  1101,  1105,  1109,
    1113,  1116,  1119,  1122,  1125,  1129,  1133,  1137,  1141,  1145,
    1149,  1153,  1157,  1161,  1165,  1169,  1173,  1177,  1181,  1185,
    1189,  1192,  1195,  1198,  1201,  1205,  1209,  1213,  1217,  1221,
    1225,  1229,  1233,  1237,  1241,  1247,  1252,  1254,  1257,  1260,
    1263,  1266,  1269,  1272,  1275,  1278,  1281,  1283,  1285,  1287,
    1291,  1294,  1295,  1307,  1308,  1321,  1323,  1325,  1327,  1333,
    1337,  1343,  1347,  1350,  1351,  1354,  1355,  1360,  1365,  1369,
    1374,  1379,  1384,  1389,  1391,  1393,  1397,  1403,  1404,  1408,
    1413,  1415,  1418,  1423,  1426,  1433,  1434,  1436,  1441,  1442,
    1445,  1446,  1448,  1450,  1454,  1456,  1460,  1462,  1464,  1468,
    1472,  1474,  1476,  1478,  1480,  1482,  1484,  1486,  1488,  1490,
    1492,  1494,  1496,  1498,  1500,  1502,  1504,  1506,  1508,  1510,
    1512,  1514,  1516,  1518,  1520,  1522,  1524,  1526,  1528,  1530,
    1532,  1534,  1536,  1538,  1540,  1542,  1544,  1546,  1548,  1550,
    1552,  1554,  1556,  1558,  1560,  1562,  1564,  1566,  1568,  1570,
    1572,  1574,  1576,  1578,  1580,  1582,  1584,  1586,  1588,  1590,
    1592,  1594,  1596,  1598,  1600,  1602,  1604,  1606,  1608,  1610,
    1612,  1614,  1616,  1618,  1620,  1622,  1624,  1626,  1628,  1633,
    1635,  1637,  1639,  1641,  1643,  1645,  1647,  1649,  1652,  1654,
    1655,  1656,  1658,  1660,  1664,  1665,  1667,  1669,  1671,  1673,
    1675,  1677,  1679,  1681,  1683,  1685,  1687,  1689,  1693,  1696,
    1698,  1700,  1703,  1706,  1711,  1715,  1720,  1722,  1724,  1728,
    1732,  1734,  1736,  1738,  1740,  1744,  1748,  1752,  1755,  1756,
    1758,  1759,  1761,  1762,  1768,  1772,  1776,  1778,  1780,  1782,
    1784,  1788,  1791,  1793,  1795,  1797,  1799,  1801,  1804,  1807,
    1812,  1816,  1821,  1824,  1825,  1831,  1835,  1839,  1841,  1845,
    1847,  1850,  1851,  1857,  1861,  1864,  1865,  1869,  1870,  1875,
    1878,  1879,  1883,  1887,  1889,  1890,  1892,  1895,  1898,  1903,
    1907,  1911,  1914,  1919,  1922,  1927,  1929,  1931,  1933,  1935,
    1937,  1940,  1945,  1949,  1954,  1958,  1960,  1962,  1964,  1966,
    1969,  1974,  1979,  1983,  1985,  1987,  1991,  1999,  2006,  2015,
    2025,  2034,  2045,  2053,  2060,  2062,  2065,  2070,  2075,  2077,
    2079,  2084,  2086,  2087,  2089,  2092,  2094,  2096,  2099,  2104,
    2108,  2112,  2113,  2115,  2118,  2123,  2127,  2130,  2134,  2141,
    2142,  2144,  2149,  2152,  2153,  2159,  2163,  2167,  2169,  2176,
    2181,  2186,  2189,  2192,  2193,  2199,  2203,  2207,  2209,  2212,
    2213,  2219,  2223,  2227,  2229,  2232,  2235,  2237,  2240,  2242,
    2247,  2251,  2255,  2262,  2266,  2268,  2270,  2272,  2277,  2282,
    2285,  2288,  2293,  2296,  2299,  2301,  2305,  2309,  2310,  2313,
    2319,  2326,  2328,  2331,  2333,  2338,  2342,  2343,  2345,  2349,
    2353,  2355,  2357,  2358,  2359,  2362,  2366,  2368,  2374,  2378,
    2382,  2386,  2388,  2391,  2392,  2397,  2400,  2403,  2405,  2407,
    2409,  2414,  2421,  2423,  2432,  2438,  2440
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     183,     0,    -1,    -1,   184,   185,    -1,   185,   186,    -1,
      -1,   200,    -1,   212,    -1,   215,    -1,   220,    -1,   381,
      -1,   116,   172,   173,   174,    -1,   141,   192,   174,    -1,
      -1,   141,   192,   175,   187,   185,   176,    -1,    -1,   141,
     175,   188,   185,   176,    -1,   104,   190,   174,    -1,   197,
     174,    -1,    71,    -1,   148,    -1,   149,    -1,   151,    -1,
     153,    -1,   152,    -1,   190,     8,   191,    -1,   191,    -1,
     192,    -1,   144,   192,    -1,   192,    90,   189,    -1,   144,
     192,    90,   189,    -1,   189,    -1,   192,   144,   189,    -1,
     192,    -1,   141,   144,   192,    -1,   144,   192,    -1,   193,
      -1,   193,   384,    -1,   193,   384,    -1,   197,     8,   382,
      13,   328,    -1,    99,   382,    13,   328,    -1,   198,   199,
      -1,    -1,   200,    -1,   212,    -1,   215,    -1,   220,    -1,
     175,   198,   176,    -1,    65,   287,   200,   242,   244,    -1,
      65,   287,    26,   198,   243,   245,    68,   174,    -1,    -1,
      82,   287,   201,   236,    -1,    -1,    81,   202,   200,    82,
     287,   174,    -1,    -1,    84,   172,   289,   174,   289,   174,
     289,   173,   203,   234,    -1,    -1,    91,   287,   204,   239,
      -1,    95,   174,    -1,    95,   293,   174,    -1,    97,   174,
      -1,    97,   293,   174,    -1,   100,   174,    -1,   100,   293,
     174,    -1,   145,    95,   174,    -1,   105,   252,   174,    -1,
     111,   254,   174,    -1,    80,   288,   174,    -1,   113,   172,
     378,   173,   174,    -1,   174,    -1,    75,    -1,    -1,    86,
     172,   293,    90,   233,   232,   173,   205,   235,    -1,    88,
     172,   238,   173,   237,    -1,   101,   175,   198,   176,   102,
     172,   321,    73,   173,   175,   198,   176,   206,   209,    -1,
     101,   175,   198,   176,   207,    -1,   103,   293,   174,    -1,
      96,   189,   174,    -1,   293,   174,    -1,   290,   174,    -1,
     291,   174,    -1,   292,   174,    -1,   189,    26,    -1,   206,
     102,   172,   321,    73,   173,   175,   198,   176,    -1,    -1,
      -1,   208,   159,   175,   198,   176,    -1,   207,    -1,    -1,
      31,    -1,    -1,    98,    -1,    -1,   211,   210,   383,   213,
     172,   248,   173,   387,   175,   198,   176,    -1,    -1,   348,
     211,   210,   383,   214,   172,   248,   173,   387,   175,   198,
     176,    -1,    -1,   226,   223,   216,   227,   228,   175,   255,
     176,    -1,    -1,   348,   226,   223,   217,   227,   228,   175,
     255,   176,    -1,    -1,   118,   224,   218,   229,   175,   255,
     176,    -1,    -1,   348,   118,   224,   219,   229,   175,   255,
     176,    -1,    -1,   154,   225,   221,   228,   175,   255,   176,
      -1,    -1,   348,   154,   225,   222,   228,   175,   255,   176,
      -1,   383,    -1,   146,    -1,   383,    -1,   383,    -1,   117,
      -1,   110,   117,    -1,   109,   117,    -1,   119,   321,    -1,
      -1,   120,   230,    -1,    -1,   119,   230,    -1,    -1,   321,
      -1,   230,     8,   321,    -1,   321,    -1,   231,     8,   321,
      -1,   122,   233,    -1,    -1,   355,    -1,    31,   355,    -1,
     123,   172,   367,   173,    -1,   200,    -1,    26,   198,    85,
     174,    -1,   200,    -1,    26,   198,    87,   174,    -1,   200,
      -1,    26,   198,    83,   174,    -1,   200,    -1,    26,   198,
      89,   174,    -1,   189,    13,   328,    -1,   238,     8,   189,
      13,   328,    -1,   175,   240,   176,    -1,   175,   174,   240,
     176,    -1,    26,   240,    92,   174,    -1,    26,   174,   240,
      92,   174,    -1,   240,    93,   293,   241,   198,    -1,   240,
      94,   241,   198,    -1,    -1,    26,    -1,   174,    -1,   242,
      66,   287,   200,    -1,    -1,   243,    66,   287,    26,   198,
      -1,    -1,    67,   200,    -1,    -1,    67,    26,   198,    -1,
      -1,   247,     8,   157,    -1,   247,   333,    -1,   157,    -1,
      -1,   349,   282,   394,    73,    -1,   349,   282,   394,    31,
      73,    -1,   349,   282,   394,    31,    73,    13,   328,    -1,
     349,   282,   394,    73,    13,   328,    -1,   247,     8,   282,
     349,   394,    73,    -1,   247,     8,   282,   349,   394,    31,
      73,    -1,   247,     8,   282,   349,   394,    31,    73,    13,
     328,    -1,   247,     8,   282,   349,   394,    73,    13,   328,
      -1,   249,     8,   157,    -1,   249,   333,    -1,   157,    -1,
      -1,   349,   394,    73,    -1,   349,   394,    31,    73,    -1,
     349,   394,    31,    73,    13,   328,    -1,   349,   394,    73,
      13,   328,    -1,   249,     8,   349,   394,    73,    -1,   249,
       8,   349,   394,    31,    73,    -1,   249,     8,   349,   394,
      31,    73,    13,   328,    -1,   249,     8,   349,   394,    73,
      13,   328,    -1,   251,   333,    -1,    -1,   293,    -1,    31,
     355,    -1,   251,     8,   293,    -1,   251,     8,    31,   355,
      -1,   252,     8,   253,    -1,   253,    -1,    73,    -1,   177,
     355,    -1,   177,   175,   293,   176,    -1,   254,     8,    73,
      -1,   254,     8,    73,    13,   328,    -1,    73,    -1,    73,
      13,   328,    -1,   255,   256,    -1,    -1,    -1,   278,   257,
     284,   174,    -1,    -1,   280,   393,   258,   284,   174,    -1,
     285,   174,    -1,    -1,   279,   211,   210,   383,   172,   259,
     246,   173,   387,   277,    -1,    -1,   348,   279,   211,   210,
     383,   172,   260,   246,   173,   387,   277,    -1,   148,   265,
     174,    -1,   149,   271,   174,    -1,   151,   273,   174,    -1,
     104,   231,   174,    -1,   104,   231,   175,   261,   176,    -1,
     261,   262,    -1,   261,   263,    -1,    -1,   196,   140,   189,
     155,   231,   174,    -1,   264,    90,   279,   189,   174,    -1,
     264,    90,   280,   174,    -1,   196,   140,   189,    -1,   189,
      -1,   266,    -1,   265,     8,   266,    -1,   267,   318,   269,
     270,    -1,   146,    -1,   124,    -1,   321,    -1,   112,    -1,
     152,   175,   268,   176,    -1,   327,    -1,   268,     8,   327,
      -1,    13,   328,    -1,    -1,    51,   153,    -1,    -1,   272,
      -1,   271,     8,   272,    -1,   150,    -1,   274,    -1,   189,
      -1,   115,    -1,   172,   275,   173,    -1,   172,   275,   173,
      45,    -1,   172,   275,   173,    25,    -1,   172,   275,   173,
      42,    -1,   274,    -1,   276,    -1,   276,    45,    -1,   276,
      25,    -1,   276,    42,    -1,   275,     8,   275,    -1,   275,
      29,   275,    -1,   189,    -1,   146,    -1,   150,    -1,   174,
      -1,   175,   198,   176,    -1,   280,    -1,   112,    -1,   280,
      -1,    -1,   281,    -1,   280,   281,    -1,   106,    -1,   107,
      -1,   108,    -1,   111,    -1,   110,    -1,   109,    -1,   283,
      -1,    -1,   106,    -1,   107,    -1,   108,    -1,   284,     8,
      73,    -1,   284,     8,    73,    13,   328,    -1,    73,    -1,
      73,    13,   328,    -1,   285,     8,   382,    13,   328,    -1,
      99,   382,    13,   328,    -1,   172,   286,   173,    -1,    63,
     323,   326,    -1,    62,   293,    -1,   310,    -1,   172,   293,
     173,    -1,   288,     8,   293,    -1,   293,    -1,   288,    -1,
      -1,   145,   293,    -1,   145,   293,   122,   293,    -1,   355,
      13,   290,    -1,   123,   172,   367,   173,    13,   290,    -1,
     294,    -1,   355,    -1,   286,    -1,   123,   172,   367,   173,
      13,   293,    -1,   355,    13,   293,    -1,   355,    13,    31,
     355,    -1,   355,    13,    31,    63,   323,   326,    -1,   355,
      24,   293,    -1,   355,    23,   293,    -1,   355,    22,   293,
      -1,   355,    21,   293,    -1,   355,    20,   293,    -1,   355,
      19,   293,    -1,   355,    18,   293,    -1,   355,    17,   293,
      -1,   355,    16,   293,    -1,   355,    15,   293,    -1,   355,
      14,   293,    -1,   355,    60,    -1,    60,   355,    -1,   355,
      59,    -1,    59,   355,    -1,   293,    27,   293,    -1,   293,
      28,   293,    -1,   293,     9,   293,    -1,   293,    11,   293,
      -1,   293,    10,   293,    -1,   293,    29,   293,    -1,   293,
      31,   293,    -1,   293,    30,   293,    -1,   293,    44,   293,
      -1,   293,    42,   293,    -1,   293,    43,   293,    -1,   293,
      45,   293,    -1,   293,    46,   293,    -1,   293,    47,   293,
      -1,   293,    41,   293,    -1,   293,    40,   293,    -1,    42,
     293,    -1,    43,   293,    -1,    48,   293,    -1,    50,   293,
      -1,   293,    33,   293,    -1,   293,    32,   293,    -1,   293,
      35,   293,    -1,   293,    34,   293,    -1,   293,    36,   293,
      -1,   293,    39,   293,    -1,   293,    37,   293,    -1,   293,
      38,   293,    -1,   293,    49,   323,    -1,   172,   294,   173,
      -1,   293,    25,   293,    26,   293,    -1,   293,    25,    26,
     293,    -1,   377,    -1,    58,   293,    -1,    57,   293,    -1,
      56,   293,    -1,    55,   293,    -1,    54,   293,    -1,    53,
     293,    -1,    52,   293,    -1,    64,   324,    -1,    51,   293,
      -1,   330,    -1,   303,    -1,   302,    -1,   178,   325,   178,
      -1,    12,   293,    -1,    -1,   211,   210,   172,   295,   248,
     173,   387,   308,   175,   198,   176,    -1,    -1,   280,   211,
     210,   172,   296,   248,   173,   387,   308,   175,   198,   176,
      -1,   306,    -1,   304,    -1,    79,    -1,   298,     8,   297,
     122,   293,    -1,   297,   122,   293,    -1,   299,     8,   297,
     122,   328,    -1,   297,   122,   328,    -1,   298,   332,    -1,
      -1,   299,   332,    -1,    -1,   166,   172,   300,   173,    -1,
     124,   172,   368,   173,    -1,    61,   368,   179,    -1,   321,
     175,   370,   176,    -1,   321,   175,   372,   176,    -1,   306,
      61,   363,   179,    -1,   307,    61,   363,   179,    -1,   303,
      -1,   379,    -1,   172,   294,   173,    -1,   104,   172,   309,
     333,   173,    -1,    -1,   309,     8,    73,    -1,   309,     8,
      31,    73,    -1,    73,    -1,    31,    73,    -1,   160,   146,
     311,   161,    -1,   313,    46,    -1,   313,   161,   314,   160,
      46,   312,    -1,    -1,   146,    -1,   313,   315,    13,   316,
      -1,    -1,   314,   317,    -1,    -1,   146,    -1,   147,    -1,
     175,   293,   176,    -1,   147,    -1,   175,   293,   176,    -1,
     310,    -1,   319,    -1,   318,    26,   319,    -1,   318,    43,
     319,    -1,   189,    -1,    64,    -1,    98,    -1,    99,    -1,
     100,    -1,   145,    -1,   101,    -1,   102,    -1,   159,    -1,
     103,    -1,    65,    -1,    66,    -1,    68,    -1,    67,    -1,
      82,    -1,    83,    -1,    81,    -1,    84,    -1,    85,    -1,
      86,    -1,    87,    -1,    88,    -1,    89,    -1,    49,    -1,
      90,    -1,    91,    -1,    92,    -1,    93,    -1,    94,    -1,
      95,    -1,    97,    -1,    96,    -1,    80,    -1,    12,    -1,
     117,    -1,   118,    -1,   119,    -1,   120,    -1,    63,    -1,
      62,    -1,   112,    -1,     5,    -1,     7,    -1,     6,    -1,
       4,    -1,     3,    -1,   141,    -1,   104,    -1,   105,    -1,
     114,    -1,   115,    -1,   116,    -1,   111,    -1,   110,    -1,
     109,    -1,   108,    -1,   107,    -1,   106,    -1,   113,    -1,
     123,    -1,   124,    -1,     9,    -1,    11,    -1,    10,    -1,
     125,    -1,   127,    -1,   126,    -1,   128,    -1,   129,    -1,
     143,    -1,   142,    -1,   171,    -1,   154,    -1,   156,    -1,
     155,    -1,   167,    -1,   169,    -1,   166,    -1,   195,   172,
     250,   173,    -1,   196,    -1,   146,    -1,   321,    -1,   111,
      -1,   361,    -1,   321,    -1,   111,    -1,   365,    -1,   172,
     173,    -1,   287,    -1,    -1,    -1,    78,    -1,   374,    -1,
     172,   250,   173,    -1,    -1,    69,    -1,    70,    -1,    79,
      -1,   128,    -1,   129,    -1,   143,    -1,   125,    -1,   156,
      -1,   126,    -1,   127,    -1,   142,    -1,   171,    -1,   136,
      78,   137,    -1,   136,   137,    -1,   327,    -1,   194,    -1,
      42,   328,    -1,    43,   328,    -1,   124,   172,   331,   173,
      -1,    61,   331,   179,    -1,   166,   172,   301,   173,    -1,
     329,    -1,   305,    -1,   196,   140,   189,    -1,   146,   140,
     189,    -1,   194,    -1,    72,    -1,   379,    -1,   327,    -1,
     180,   374,   180,    -1,   181,   374,   181,    -1,   136,   374,
     137,    -1,   334,   332,    -1,    -1,     8,    -1,    -1,     8,
      -1,    -1,   334,     8,   328,   122,   328,    -1,   334,     8,
     328,    -1,   328,   122,   328,    -1,   328,    -1,    69,    -1,
      70,    -1,    79,    -1,   136,    78,   137,    -1,   136,   137,
      -1,    69,    -1,    70,    -1,   189,    -1,   335,    -1,   189,
      -1,    42,   336,    -1,    43,   336,    -1,   124,   172,   338,
     173,    -1,    61,   338,   179,    -1,   166,   172,   341,   173,
      -1,   339,   332,    -1,    -1,   339,     8,   337,   122,   337,
      -1,   339,     8,   337,    -1,   337,   122,   337,    -1,   337,
      -1,   340,     8,   337,    -1,   337,    -1,   342,   332,    -1,
      -1,   342,     8,   297,   122,   337,    -1,   297,   122,   337,
      -1,   340,   332,    -1,    -1,   172,   343,   173,    -1,    -1,
     345,     8,   189,   344,    -1,   189,   344,    -1,    -1,   347,
     345,   332,    -1,    41,   346,    40,    -1,   348,    -1,    -1,
     351,    -1,   121,   360,    -1,   121,   189,    -1,   121,   175,
     293,   176,    -1,    61,   363,   179,    -1,   175,   293,   176,
      -1,   356,   352,    -1,   172,   286,   173,   352,    -1,   366,
     352,    -1,   172,   286,   173,   352,    -1,   360,    -1,   320,
      -1,   358,    -1,   359,    -1,   353,    -1,   355,   350,    -1,
     172,   286,   173,   350,    -1,   322,   140,   360,    -1,   357,
     172,   250,   173,    -1,   172,   355,   173,    -1,   320,    -1,
     358,    -1,   359,    -1,   353,    -1,   355,   351,    -1,   172,
     286,   173,   351,    -1,   357,   172,   250,   173,    -1,   172,
     355,   173,    -1,   360,    -1,   353,    -1,   172,   355,   173,
      -1,   355,   121,   189,   384,   172,   250,   173,    -1,   355,
     121,   360,   172,   250,   173,    -1,   355,   121,   175,   293,
     176,   172,   250,   173,    -1,   172,   286,   173,   121,   189,
     384,   172,   250,   173,    -1,   172,   286,   173,   121,   360,
     172,   250,   173,    -1,   172,   286,   173,   121,   175,   293,
     176,   172,   250,   173,    -1,   322,   140,   189,   384,   172,
     250,   173,    -1,   322,   140,   360,   172,   250,   173,    -1,
     361,    -1,   364,   361,    -1,   361,    61,   363,   179,    -1,
     361,   175,   293,   176,    -1,   362,    -1,    73,    -1,   177,
     175,   293,   176,    -1,   293,    -1,    -1,   177,    -1,   364,
     177,    -1,   360,    -1,   354,    -1,   365,   350,    -1,   172,
     286,   173,   350,    -1,   322,   140,   360,    -1,   172,   355,
     173,    -1,    -1,   354,    -1,   365,   351,    -1,   172,   286,
     173,   351,    -1,   172,   355,   173,    -1,   367,     8,    -1,
     367,     8,   355,    -1,   367,     8,   123,   172,   367,   173,
      -1,    -1,   355,    -1,   123,   172,   367,   173,    -1,   369,
     332,    -1,    -1,   369,     8,   293,   122,   293,    -1,   369,
       8,   293,    -1,   293,   122,   293,    -1,   293,    -1,   369,
       8,   293,   122,    31,   355,    -1,   369,     8,    31,   355,
      -1,   293,   122,    31,   355,    -1,    31,   355,    -1,   371,
     332,    -1,    -1,   371,     8,   293,   122,   293,    -1,   371,
       8,   293,    -1,   293,   122,   293,    -1,   293,    -1,   373,
     332,    -1,    -1,   373,     8,   328,   122,   328,    -1,   373,
       8,   328,    -1,   328,   122,   328,    -1,   328,    -1,   374,
     375,    -1,   374,    78,    -1,   375,    -1,    78,   375,    -1,
      73,    -1,    73,    61,   376,   179,    -1,    73,   121,   189,
      -1,   138,   293,   176,    -1,   138,    72,    61,   293,   179,
     176,    -1,   139,   355,   176,    -1,   189,    -1,    74,    -1,
      73,    -1,   114,   172,   378,   173,    -1,   115,   172,   355,
     173,    -1,     7,   293,    -1,     6,   293,    -1,     5,   172,
     293,   173,    -1,     4,   293,    -1,     3,   293,    -1,   355,
      -1,   378,     8,   355,    -1,   322,   140,   189,    -1,    -1,
      90,   393,    -1,   167,   383,    13,   393,   174,    -1,   169,
     383,   380,    13,   393,   174,    -1,   189,    -1,   393,   189,
      -1,   189,    -1,   189,   162,   388,   163,    -1,   162,   385,
     163,    -1,    -1,   393,    -1,   385,     8,   393,    -1,   385,
       8,   157,    -1,   385,    -1,   157,    -1,    -1,    -1,    26,
     393,    -1,   388,     8,   189,    -1,   189,    -1,   388,     8,
     189,    90,   393,    -1,   189,    90,   393,    -1,    79,   122,
     393,    -1,   390,     8,   389,    -1,   389,    -1,   390,   332,
      -1,    -1,   166,   172,   391,   173,    -1,    25,   393,    -1,
      51,   393,    -1,   196,    -1,   124,    -1,   392,    -1,   124,
     162,   393,   163,    -1,   124,   162,   393,     8,   393,   163,
      -1,   146,    -1,   172,    98,   172,   386,   173,    26,   393,
     173,    -1,   172,   385,     8,   393,   173,    -1,   393,    -1,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   704,   704,   704,   709,   711,   714,   715,   716,   717,
     718,   719,   722,   724,   724,   726,   726,   728,   729,   734,
     735,   736,   737,   738,   739,   743,   745,   748,   749,   750,
     751,   756,   757,   761,   762,   764,   767,   773,   780,   787,
     791,   797,   799,   802,   803,   804,   805,   808,   809,   813,
     818,   818,   822,   822,   827,   826,   830,   830,   833,   834,
     835,   836,   837,   838,   839,   840,   841,   842,   843,   844,
     845,   848,   846,   851,   853,   861,   864,   865,   869,   870,
     871,   872,   873,   880,   886,   890,   890,   896,   897,   901,
     902,   906,   911,   910,   920,   919,   932,   931,   950,   948,
     967,   966,   975,   973,   985,   984,   996,   994,  1007,  1008,
    1012,  1015,  1018,  1019,  1020,  1023,  1025,  1028,  1029,  1032,
    1033,  1036,  1037,  1041,  1042,  1047,  1048,  1051,  1052,  1053,
    1057,  1058,  1062,  1063,  1067,  1068,  1072,  1073,  1078,  1079,
    1084,  1085,  1086,  1087,  1090,  1093,  1095,  1098,  1099,  1103,
    1105,  1108,  1111,  1114,  1115,  1118,  1119,  1123,  1125,  1127,
    1128,  1132,  1136,  1140,  1145,  1150,  1155,  1160,  1166,  1175,
    1177,  1179,  1180,  1184,  1187,  1190,  1194,  1198,  1202,  1206,
    1211,  1219,  1221,  1224,  1225,  1226,  1228,  1233,  1234,  1237,
    1238,  1239,  1243,  1244,  1246,  1247,  1251,  1253,  1256,  1256,
    1260,  1259,  1263,  1267,  1265,  1278,  1275,  1286,  1288,  1290,
    1292,  1294,  1298,  1299,  1300,  1303,  1309,  1312,  1318,  1321,
    1326,  1328,  1333,  1338,  1342,  1343,  1349,  1350,  1355,  1356,
    1361,  1362,  1366,  1367,  1371,  1373,  1379,  1384,  1385,  1387,
    1391,  1392,  1393,  1394,  1398,  1399,  1400,  1401,  1402,  1403,
    1405,  1410,  1413,  1414,  1418,  1419,  1422,  1423,  1426,  1427,
    1430,  1431,  1435,  1436,  1437,  1438,  1439,  1440,  1443,  1444,
    1447,  1448,  1449,  1452,  1454,  1456,  1457,  1460,  1462,  1466,
    1467,  1469,  1470,  1473,  1477,  1478,  1482,  1483,  1487,  1488,
    1492,  1496,  1501,  1502,  1503,  1506,  1508,  1509,  1510,  1513,
    1514,  1515,  1516,  1517,  1518,  1519,  1520,  1521,  1522,  1523,
    1524,  1525,  1526,  1527,  1528,  1529,  1530,  1531,  1532,  1533,
    1534,  1535,  1536,  1537,  1538,  1539,  1540,  1541,  1542,  1543,
    1544,  1545,  1546,  1547,  1548,  1549,  1550,  1551,  1552,  1553,
    1555,  1556,  1558,  1560,  1561,  1562,  1563,  1564,  1565,  1566,
    1567,  1568,  1569,  1570,  1571,  1572,  1573,  1574,  1575,  1576,
    1577,  1579,  1578,  1587,  1586,  1594,  1595,  1599,  1603,  1607,
    1613,  1617,  1623,  1625,  1629,  1631,  1635,  1639,  1640,  1644,
    1651,  1658,  1660,  1665,  1666,  1667,  1671,  1673,  1677,  1678,
    1679,  1680,  1684,  1690,  1699,  1712,  1713,  1716,  1719,  1722,
    1723,  1726,  1730,  1733,  1736,  1743,  1744,  1748,  1749,  1751,
    1755,  1756,  1757,  1758,  1759,  1760,  1761,  1762,  1763,  1764,
    1765,  1766,  1767,  1768,  1769,  1770,  1771,  1772,  1773,  1774,
    1775,  1776,  1777,  1778,  1779,  1780,  1781,  1782,  1783,  1784,
    1785,  1786,  1787,  1788,  1789,  1790,  1791,  1792,  1793,  1794,
    1795,  1796,  1797,  1798,  1799,  1800,  1801,  1802,  1803,  1804,
    1805,  1806,  1807,  1808,  1809,  1810,  1811,  1812,  1813,  1814,
    1815,  1816,  1817,  1818,  1819,  1820,  1821,  1822,  1823,  1824,
    1825,  1826,  1827,  1828,  1829,  1830,  1831,  1832,  1836,  1841,
    1842,  1845,  1846,  1847,  1851,  1852,  1853,  1857,  1858,  1859,
    1863,  1864,  1865,  1868,  1870,  1874,  1875,  1876,  1878,  1879,
    1880,  1881,  1882,  1883,  1884,  1885,  1886,  1887,  1890,  1895,
    1896,  1897,  1898,  1899,  1901,  1902,  1904,  1905,  1909,  1912,
    1918,  1919,  1920,  1921,  1922,  1923,  1924,  1929,  1931,  1935,
    1936,  1939,  1940,  1944,  1947,  1949,  1951,  1955,  1956,  1957,
    1959,  1962,  1966,  1967,  1968,  1971,  1972,  1973,  1974,  1975,
    1977,  1978,  1983,  1985,  1988,  1991,  1993,  1995,  1998,  2000,
    2004,  2006,  2009,  2012,  2018,  2020,  2023,  2024,  2029,  2032,
    2036,  2036,  2041,  2044,  2045,  2049,  2050,  2055,  2056,  2060,
    2061,  2065,  2066,  2071,  2073,  2078,  2079,  2080,  2081,  2082,
    2083,  2084,  2086,  2089,  2091,  2095,  2096,  2097,  2098,  2099,
    2101,  2103,  2105,  2109,  2110,  2111,  2115,  2118,  2121,  2124,
    2128,  2132,  2139,  2143,  2150,  2151,  2156,  2158,  2159,  2162,
    2163,  2166,  2167,  2171,  2172,  2176,  2177,  2178,  2179,  2181,
    2184,  2187,  2188,  2189,  2191,  2193,  2197,  2198,  2199,  2201,
    2202,  2203,  2207,  2209,  2212,  2214,  2215,  2216,  2217,  2220,
    2222,  2223,  2227,  2229,  2232,  2234,  2235,  2236,  2240,  2242,
    2245,  2248,  2250,  2252,  2256,  2257,  2259,  2260,  2266,  2267,
    2269,  2271,  2273,  2275,  2278,  2279,  2280,  2284,  2285,  2286,
    2287,  2288,  2289,  2290,  2294,  2295,  2299,  2307,  2309,  2313,
    2316,  2322,  2323,  2329,  2330,  2337,  2340,  2344,  2347,  2352,
    2353,  2354,  2355,  2359,  2360,  2364,  2366,  2367,  2369,  2373,
    2379,  2381,  2385,  2388,  2391,  2399,  2402,  2405,  2406,  2409,
    2410,  2413,  2417,  2421,  2427,  2435,  2436
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
  "T_COMPILER_HALT_OFFSET", "'('", "')'", "';'", "'{'", "'}'", "'$'",
  "'`'", "']'", "'\"'", "'\\''", "$accept", "start", "$@1",
  "top_statement_list", "top_statement", "$@2", "$@3", "ident",
  "use_declarations", "use_declaration", "namespace_name",
  "namespace_string_base", "namespace_string", "namespace_string_typeargs",
  "class_namespace_string_typeargs", "constant_declaration",
  "inner_statement_list", "inner_statement", "statement", "$@4", "$@5",
  "$@6", "$@7", "$@8", "additional_catches", "finally", "$@9",
  "optional_finally", "is_reference", "function_loc",
  "function_declaration_statement", "$@10", "$@11",
  "class_declaration_statement", "$@12", "$@13", "$@14", "$@15",
  "trait_declaration_statement", "$@16", "$@17", "class_decl_name",
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
  "class_statement_list", "class_statement", "$@18", "$@19", "$@20",
  "$@21", "trait_rules", "trait_precedence_rule", "trait_alias_rule",
  "trait_alias_rule_method", "xhp_attribute_stmt", "xhp_attribute_decl",
  "xhp_attribute_decl_type", "xhp_attribute_enum", "xhp_attribute_default",
  "xhp_attribute_is_required", "xhp_category_stmt", "xhp_category_decl",
  "xhp_children_stmt", "xhp_children_paren_expr", "xhp_children_decl_expr",
  "xhp_children_decl_tag", "method_body", "variable_modifiers",
  "method_modifiers", "non_empty_member_modifiers", "member_modifier",
  "parameter_modifiers", "parameter_modifier",
  "class_variable_declaration", "class_constant_declaration",
  "expr_with_parens", "parenthesis_expr", "expr_list", "for_expr",
  "yield_expr", "yield_assign_expr", "yield_list_assign_expr", "expr",
  "expr_no_variable", "$@22", "$@23", "shape_keyname",
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
  "user_attribute_list", "$@24", "non_empty_user_attributes",
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
     406,   407,    40,    41,    59,   123,   125,    36,    96,    93,
      34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   182,   184,   183,   185,   185,   186,   186,   186,   186,
     186,   186,   186,   187,   186,   188,   186,   186,   186,   189,
     189,   189,   189,   189,   189,   190,   190,   191,   191,   191,
     191,   192,   192,   193,   193,   193,   194,   195,   196,   197,
     197,   198,   198,   199,   199,   199,   199,   200,   200,   200,
     201,   200,   202,   200,   203,   200,   204,   200,   200,   200,
     200,   200,   200,   200,   200,   200,   200,   200,   200,   200,
     200,   205,   200,   200,   200,   200,   200,   200,   200,   200,
     200,   200,   200,   206,   206,   208,   207,   209,   209,   210,
     210,   211,   213,   212,   214,   212,   216,   215,   217,   215,
     218,   215,   219,   215,   221,   220,   222,   220,   223,   223,
     224,   225,   226,   226,   226,   227,   227,   228,   228,   229,
     229,   230,   230,   231,   231,   232,   232,   233,   233,   233,
     234,   234,   235,   235,   236,   236,   237,   237,   238,   238,
     239,   239,   239,   239,   240,   240,   240,   241,   241,   242,
     242,   243,   243,   244,   244,   245,   245,   246,   246,   246,
     246,   247,   247,   247,   247,   247,   247,   247,   247,   248,
     248,   248,   248,   249,   249,   249,   249,   249,   249,   249,
     249,   250,   250,   251,   251,   251,   251,   252,   252,   253,
     253,   253,   254,   254,   254,   254,   255,   255,   257,   256,
     258,   256,   256,   259,   256,   260,   256,   256,   256,   256,
     256,   256,   261,   261,   261,   262,   263,   263,   264,   264,
     265,   265,   266,   266,   267,   267,   267,   267,   268,   268,
     269,   269,   270,   270,   271,   271,   272,   273,   273,   273,
     274,   274,   274,   274,   275,   275,   275,   275,   275,   275,
     275,   276,   276,   276,   277,   277,   278,   278,   279,   279,
     280,   280,   281,   281,   281,   281,   281,   281,   282,   282,
     283,   283,   283,   284,   284,   284,   284,   285,   285,   286,
     286,   286,   286,   287,   288,   288,   289,   289,   290,   290,
     291,   292,   293,   293,   293,   294,   294,   294,   294,   294,
     294,   294,   294,   294,   294,   294,   294,   294,   294,   294,
     294,   294,   294,   294,   294,   294,   294,   294,   294,   294,
     294,   294,   294,   294,   294,   294,   294,   294,   294,   294,
     294,   294,   294,   294,   294,   294,   294,   294,   294,   294,
     294,   294,   294,   294,   294,   294,   294,   294,   294,   294,
     294,   294,   294,   294,   294,   294,   294,   294,   294,   294,
     294,   295,   294,   296,   294,   294,   294,   297,   298,   298,
     299,   299,   300,   300,   301,   301,   302,   303,   303,   304,
     305,   306,   306,   307,   307,   307,   308,   308,   309,   309,
     309,   309,   310,   311,   311,   312,   312,   313,   313,   314,
     314,   315,   316,   316,   317,   317,   317,   318,   318,   318,
     319,   319,   319,   319,   319,   319,   319,   319,   319,   319,
     319,   319,   319,   319,   319,   319,   319,   319,   319,   319,
     319,   319,   319,   319,   319,   319,   319,   319,   319,   319,
     319,   319,   319,   319,   319,   319,   319,   319,   319,   319,
     319,   319,   319,   319,   319,   319,   319,   319,   319,   319,
     319,   319,   319,   319,   319,   319,   319,   319,   319,   319,
     319,   319,   319,   319,   319,   319,   319,   319,   319,   319,
     319,   319,   319,   319,   319,   319,   319,   319,   320,   321,
     321,   322,   322,   322,   323,   323,   323,   324,   324,   324,
     325,   325,   325,   326,   326,   327,   327,   327,   327,   327,
     327,   327,   327,   327,   327,   327,   327,   327,   327,   328,
     328,   328,   328,   328,   328,   328,   328,   328,   329,   329,
     330,   330,   330,   330,   330,   330,   330,   331,   331,   332,
     332,   333,   333,   334,   334,   334,   334,   335,   335,   335,
     335,   335,   336,   336,   336,   337,   337,   337,   337,   337,
     337,   337,   338,   338,   339,   339,   339,   339,   340,   340,
     341,   341,   342,   342,   343,   343,   344,   344,   345,   345,
     347,   346,   348,   349,   349,   350,   350,   351,   351,   352,
     352,   353,   353,   354,   354,   355,   355,   355,   355,   355,
     355,   355,   355,   355,   355,   356,   356,   356,   356,   356,
     356,   356,   356,   357,   357,   357,   358,   358,   358,   358,
     358,   358,   359,   359,   360,   360,   361,   361,   361,   362,
     362,   363,   363,   364,   364,   365,   365,   365,   365,   365,
     365,   366,   366,   366,   366,   366,   367,   367,   367,   367,
     367,   367,   368,   368,   369,   369,   369,   369,   369,   369,
     369,   369,   370,   370,   371,   371,   371,   371,   372,   372,
     373,   373,   373,   373,   374,   374,   374,   374,   375,   375,
     375,   375,   375,   375,   376,   376,   376,   377,   377,   377,
     377,   377,   377,   377,   378,   378,   379,   380,   380,   381,
     381,   382,   382,   383,   383,   384,   384,   385,   385,   386,
     386,   386,   386,   387,   387,   388,   388,   388,   388,   389,
     390,   390,   391,   391,   392,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   394,   394
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
       2,     2,     2,     9,     0,     0,     5,     1,     0,     1,
       0,     1,     0,    11,     0,    12,     0,     8,     0,     9,
       0,     7,     0,     8,     0,     7,     0,     8,     1,     1,
       1,     1,     1,     2,     2,     2,     0,     2,     0,     2,
       0,     1,     3,     1,     3,     2,     0,     1,     2,     4,
       1,     4,     1,     4,     1,     4,     1,     4,     3,     5,
       3,     4,     4,     5,     5,     4,     0,     1,     1,     4,
       0,     5,     0,     2,     0,     3,     0,     3,     2,     1,
       0,     4,     5,     7,     6,     6,     7,     9,     8,     3,
       2,     1,     0,     3,     4,     6,     5,     5,     6,     8,
       7,     2,     0,     1,     2,     3,     4,     3,     1,     1,
       2,     4,     3,     5,     1,     3,     2,     0,     0,     4,
       0,     5,     2,     0,    10,     0,    11,     3,     3,     3,
       3,     5,     2,     2,     0,     6,     5,     4,     3,     1,
       1,     3,     4,     1,     1,     1,     1,     4,     1,     3,
       2,     0,     2,     0,     1,     3,     1,     1,     1,     1,
       3,     4,     4,     4,     1,     1,     2,     2,     2,     3,
       3,     1,     1,     1,     1,     3,     1,     1,     1,     0,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     0,
       1,     1,     1,     3,     5,     1,     3,     5,     4,     3,
       3,     2,     1,     3,     3,     1,     1,     0,     2,     4,
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
       1,     1,     1,     1,     1,     1,     1,     1,     4,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     1,     0,
       0,     1,     1,     3,     0,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     2,     1,
       1,     2,     2,     4,     3,     4,     1,     1,     3,     3,
       1,     1,     1,     1,     3,     3,     3,     2,     0,     1,
       0,     1,     0,     5,     3,     3,     1,     1,     1,     1,
       3,     2,     1,     1,     1,     1,     1,     2,     2,     4,
       3,     4,     2,     0,     5,     3,     3,     1,     3,     1,
       2,     0,     5,     3,     2,     0,     3,     0,     4,     2,
       0,     3,     3,     1,     0,     1,     2,     2,     4,     3,
       3,     2,     4,     2,     4,     1,     1,     1,     1,     1,
       2,     4,     3,     4,     3,     1,     1,     1,     1,     2,
       4,     4,     3,     1,     1,     3,     7,     6,     8,     9,
       8,    10,     7,     6,     1,     2,     4,     4,     1,     1,
       4,     1,     0,     1,     2,     1,     1,     2,     4,     3,
       3,     0,     1,     2,     4,     3,     2,     3,     6,     0,
       1,     4,     2,     0,     5,     3,     3,     1,     6,     4,
       4,     2,     2,     0,     5,     3,     3,     1,     2,     0,
       5,     3,     3,     1,     2,     2,     1,     2,     1,     4,
       3,     3,     6,     3,     1,     1,     1,     4,     4,     2,
       2,     4,     2,     2,     1,     3,     3,     0,     2,     5,
       6,     1,     2,     1,     4,     3,     0,     1,     3,     3,
       1,     1,     0,     0,     2,     3,     1,     5,     3,     3,
       3,     1,     2,     0,     4,     2,     2,     1,     1,     1,
       4,     6,     1,     8,     5,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   580,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   653,     0,   641,   499,
       0,   505,   506,    19,   531,   629,    70,   507,     0,    52,
       0,     0,     0,     0,     0,     0,     0,     0,    91,     0,
       0,     0,     0,     0,     0,   262,   263,   264,   267,   266,
     265,     0,     0,     0,     0,   112,     0,     0,     0,   511,
     513,   514,   508,   509,     0,     0,   515,   510,     0,     0,
     490,    20,    21,    22,    24,    23,     0,   512,     0,     0,
       0,     0,   516,     0,    69,    42,   633,   500,     0,     0,
       4,    31,    33,    36,   530,     0,   489,     0,     6,    90,
       7,     8,     9,     0,     0,   260,   294,     0,     0,     0,
       0,   292,   358,   357,   366,   365,     0,   282,   596,   491,
       0,   533,   356,     0,   599,   293,     0,     0,   597,   598,
     595,   624,   628,     0,   346,   532,    10,   267,   266,   265,
       0,     0,    31,    90,   693,   293,   692,     0,   690,   689,
     360,     0,     0,   330,   331,   332,   333,   355,   353,   352,
     351,   350,   349,   348,   347,   492,     0,   706,   491,     0,
     313,   311,     0,   657,     0,   540,   281,   495,     0,   706,
     494,     0,   504,   636,   635,   496,     0,     0,   498,   354,
       0,     0,     0,   285,     0,    50,   287,     0,     0,    56,
      58,     0,     0,    60,     0,     0,     0,   728,   732,     0,
       0,    31,   727,     0,   729,     0,    62,     0,    42,     0,
       0,     0,    26,    27,   189,     0,     0,   188,   114,   113,
     194,     0,     0,     0,     0,     0,   703,   100,   110,   649,
     653,   678,     0,   518,     0,     0,     0,   676,     0,    15,
       0,    35,     0,   288,   104,   111,   398,   373,     0,   697,
     294,     0,   292,   293,     0,     0,   501,     0,   502,     0,
       0,     0,    82,     0,     0,    38,   182,     0,    18,    89,
       0,   109,    96,   108,   265,    90,   261,    79,    80,    81,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   641,    78,   632,   632,   663,
       0,     0,     0,     0,     0,    90,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   312,
     310,     0,   600,   585,   632,     0,   591,   182,   632,     0,
     634,   625,   649,     0,     0,     0,   582,   577,   540,     0,
       0,     0,     0,   661,     0,   378,   539,   652,     0,     0,
      38,     0,   182,   280,     0,   637,   585,   593,   497,     0,
      42,   150,     0,    67,     0,     0,   286,     0,     0,     0,
       0,     0,    59,    77,    61,   725,   726,     0,   723,     0,
       0,   707,     0,   702,    63,     0,    76,    28,     0,    17,
       0,     0,   190,     0,    65,     0,     0,    66,   694,     0,
       0,     0,     0,     0,   120,     0,   650,     0,     0,     0,
       0,   517,   677,   531,     0,     0,   675,   536,   674,    34,
       5,    12,    13,    64,     0,   118,     0,     0,   367,     0,
     540,     0,     0,     0,     0,   279,   343,   604,    47,    41,
      43,    44,    45,    46,     0,   359,   534,   535,    32,     0,
       0,     0,   542,   183,     0,   361,    92,   116,     0,   316,
     318,   317,     0,     0,   314,   315,   319,   321,   320,   335,
     334,   337,   336,   338,   340,   341,   339,   329,   328,   323,
     324,   322,   325,   326,   327,   342,   631,     0,     0,   667,
       0,   540,   696,   602,   624,   102,   106,     0,    98,     0,
       0,   290,   296,   309,   308,   307,   306,   305,   304,   303,
     302,   301,   300,   299,     0,   587,   586,     0,     0,     0,
       0,     0,     0,   691,   575,   579,   539,   581,     0,     0,
     706,     0,   656,     0,   655,     0,   640,   639,     0,     0,
     587,   586,   283,   152,   154,   284,     0,    42,   134,    51,
     287,     0,     0,     0,     0,   146,   146,    57,     0,     0,
     721,   540,     0,   712,     0,     0,     0,   538,     0,     0,
     490,     0,    36,   520,   489,   527,     0,   519,    40,   526,
      85,     0,    25,    29,     0,   187,   195,   192,     0,     0,
     687,   688,    11,   716,     0,     0,     0,   649,   646,     0,
     377,   686,   685,   684,     0,   680,     0,   681,   683,     0,
       5,   289,     0,     0,   392,   393,   401,   400,     0,     0,
     539,   372,   376,     0,   698,     0,     0,   601,   585,   592,
     630,     0,   705,   184,   488,   541,   181,     0,   584,     0,
       0,   118,   363,   345,     0,   381,   382,     0,   379,   539,
     662,     0,   182,   120,   118,    94,   116,   641,   297,     0,
       0,   182,   589,   590,   603,   626,   627,     0,     0,     0,
     563,   547,   548,   549,     0,     0,     0,   556,   555,   569,
     540,     0,   577,   660,   659,     0,   638,   585,   594,   503,
       0,   156,     0,     0,    48,     0,     0,     0,     0,     0,
     126,   127,   138,     0,    42,   136,    73,   146,     0,   146,
       0,     0,   730,     0,   539,   722,   724,   711,   710,     0,
     708,   521,   522,   546,     0,   540,   538,     0,     0,   375,
       0,   669,     0,    75,     0,    30,   191,     0,   695,    68,
       0,     0,   704,   119,   121,   197,     0,     0,   647,     0,
     679,     0,    16,     0,   117,   197,     0,     0,   369,     0,
     699,     0,     0,   587,   586,   708,     0,   185,    39,   171,
       0,   542,   583,   736,   584,   115,     0,   584,   344,   666,
     665,   182,     0,     0,     0,     0,   118,   504,   588,   182,
       0,     0,   552,   553,   554,   557,   558,   567,     0,   540,
     563,     0,   551,   571,   539,   574,   576,   578,     0,   654,
     588,     0,     0,     0,     0,   153,    53,     0,   287,   128,
     649,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     140,     0,   719,   720,     0,     0,   734,     0,   524,   539,
     537,     0,   529,     0,   540,     0,   528,   673,     0,   540,
       0,     0,   193,   718,   715,     0,   259,   651,   649,   291,
     295,     0,    14,   259,   404,     0,     0,   406,   399,   402,
       0,   397,     0,   700,     0,     0,   182,   186,   713,   584,
     170,   735,     0,     0,   197,     0,     0,     0,   623,   197,
     197,   584,     0,   298,   182,     0,   617,     0,   560,   539,
     562,     0,   550,     0,     0,   540,   568,   658,     0,    42,
       0,   149,   135,     0,     0,   125,    71,   139,     0,     0,
     142,     0,   147,   148,    42,   141,   731,   709,     0,   545,
     544,   523,     0,   539,   374,   525,     0,   380,   539,   668,
       0,    42,     0,   122,     0,     0,   257,     0,     0,     0,
     101,   196,   198,     0,   256,     0,   259,     0,   682,   105,
     395,     0,     0,   368,   588,   182,     0,     0,   387,   169,
     736,     0,   173,   713,   259,   713,   664,   622,   259,   259,
       0,   197,     0,   616,   566,   565,   559,     0,   561,   539,
     570,    42,   155,    49,    54,   129,     0,   137,   143,    42,
     145,     0,     0,   371,     0,   672,   671,     0,     0,   717,
       0,     0,   123,   226,   224,   490,    24,     0,   220,     0,
     225,   236,     0,   234,   239,     0,   238,     0,   237,     0,
      90,   200,     0,   202,     0,   258,   648,   396,   394,   405,
     403,   182,     0,   620,   714,     0,     0,     0,   174,     0,
       0,    97,   387,   103,   107,   713,   259,   618,     0,   573,
       0,   151,     0,    42,   132,    72,   144,   733,   543,     0,
       0,     0,    86,     0,     0,   210,   214,     0,     0,   207,
     455,   454,   451,   453,   452,   471,   473,   472,   443,   433,
     449,   448,   411,   420,   421,   423,   422,   442,   426,   424,
     425,   427,   428,   429,   430,   431,   432,   434,   435,   436,
     437,   438,   439,   441,   440,   412,   413,   414,   416,   417,
     419,   457,   458,   467,   466,   465,   464,   463,   462,   450,
     468,   459,   460,   461,   444,   445,   446,   447,   469,   470,
     474,   476,   475,   477,   478,   456,   480,   479,   415,   482,
     484,   483,   418,   487,   485,   486,   481,   410,   231,   407,
       0,   208,   252,   253,   251,   244,     0,   245,   209,   275,
       0,     0,     0,     0,    90,     0,   619,     0,    42,     0,
     177,     0,   176,    42,     0,     0,    99,   564,     0,    42,
     130,    55,     0,   370,   670,    42,   278,   124,     0,     0,
     228,   221,     0,     0,     0,   233,   235,     0,     0,   240,
     247,   248,   246,     0,     0,   199,     0,     0,     0,     0,
     621,     0,   390,   542,     0,   178,     0,   175,     0,    42,
      42,   572,     0,     0,     0,   211,    31,     0,   212,   213,
       0,     0,   227,   230,   408,   409,     0,   222,   249,   250,
     242,   243,   241,   276,   273,   203,   201,   277,     0,   391,
     541,     0,   362,     0,   180,    93,     0,     0,     0,   133,
      84,     0,   259,   229,   232,     0,   584,   205,     0,   388,
     386,   179,   364,    95,   131,    88,   218,     0,   258,   274,
     159,     0,   542,   269,   584,   389,     0,    87,    74,     0,
       0,   217,   713,   269,   158,   270,   271,   272,   736,   268,
       0,     0,     0,   216,     0,   157,   584,     0,   713,     0,
     215,   254,    42,   204,   736,     0,   161,     0,     0,     0,
       0,   162,     0,   206,     0,   255,     0,   165,     0,   164,
      42,   166,     0,   163,     0,     0,   168,    83,   167
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   100,   640,   450,   152,   231,   232,
     102,   103,   104,   105,   106,   107,   274,   469,   470,   395,
     204,  1092,   401,  1026,  1315,   763,   764,  1328,   290,   153,
     471,   669,   815,   472,   487,   686,   434,   683,   473,   455,
     684,   292,   247,   264,   113,   671,   643,   626,   773,  1041,
     852,   730,  1221,  1095,   579,   736,   400,   587,   738,   954,
     574,   721,   724,   843,  1321,  1322,   800,   801,   481,   482,
     236,   237,   241,   886,   981,  1059,  1202,  1306,  1324,  1228,
    1268,  1269,  1270,  1047,  1048,  1049,  1229,  1235,  1277,  1052,
    1053,  1057,  1195,  1196,  1197,  1353,   982,   983,   114,   115,
    1338,  1339,  1200,   985,   116,   198,   396,   397,   117,   118,
     119,   120,   121,   668,   807,   459,   460,   874,   461,   875,
     122,   123,   124,   605,   125,   126,  1076,  1253,   127,   456,
    1068,   457,   786,   648,   901,   898,  1188,  1189,   128,   129,
     130,   192,   199,   277,   383,   131,   753,   609,   132,   754,
     377,   666,   755,   708,   825,   827,   828,   829,   710,   934,
     935,   711,   555,   368,   161,   162,   133,   803,   352,   353,
     659,   134,   193,   155,   136,   137,   138,   139,   140,   141,
     142,   517,   143,   195,   196,   437,   184,   185,   520,   521,
     878,   879,   256,   257,   634,   144,   429,   145,   464,   146,
     223,   248,   285,   410,   749,   998,   624,   590,   591,   592,
     224,   225,   912
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1108
static const yytype_int16 yypact[] =
{
   -1108,   105, -1108, -1108,  3919,  9449,  9449,   -58,  9449,  9449,
    9449, -1108,  9449,  9449,  9449,  9449,  9449,  9449,  9449,  9449,
    9449,  9449,  9449,  9449,  2024,  2024,  3365,  9449,  2348,   -56,
     -48, -1108, -1108, -1108, -1108, -1108, -1108, -1108,  9449, -1108,
     -48,   -37,   -29,   -26,   -48,  7395,  1033,  7553, -1108,  1567,
    7711,   -27,  9449,   978,    18, -1108, -1108, -1108,   202,   214,
      17,   172,   174,   178,   225, -1108,  1033,   252,   258, -1108,
   -1108, -1108, -1108, -1108,   614,   845, -1108, -1108,  1033,  7869,
   -1108, -1108, -1108, -1108, -1108, -1108,  1033, -1108,   224,   281,
    1033,  1033, -1108,  9449, -1108, -1108,   248,   406,   662,   662,
   -1108,   414,   310,   272, -1108,   284, -1108,    48, -1108,   428,
   -1108, -1108, -1108,  1178,  1280, -1108, -1108,   289,   293,   294,
   10162, -1108, -1108,   415, -1108,   416,   422, -1108,   120,   318,
     355, -1108, -1108,  1056,   121,  2220,   132,   330,   138,   215,
     332,    35, -1108,    24, -1108,   446, -1108, -1108, -1108,   369,
     340,   376, -1108,   428, 10816,  2254, 10816,  9449, 10816, 10816,
    9764,   485,  1033, -1108, -1108,   477, -1108, -1108, -1108, -1108,
   -1108, -1108, -1108, -1108, -1108, -1108,  1748,   373, -1108,   402,
     426,   426,  2024, 10455,   377,   547, -1108,   369,  1748,   373,
     423,   430,   396,   239, -1108,   453,   132,  8027, -1108, -1108,
    9449,  6447,    49, 10816,  7237, -1108,  9449,  9449,  1033, -1108,
   -1108, 10203,   398, -1108, 10244,  1567,  1567,   419, -1108,   405,
     565,   574, -1108,   575, -1108,  1033, -1108, 10288, -1108, 10329,
    1033,    51, -1108,   195, -1108,   513,    52, -1108, -1108, -1108,
     580,    53,  2024,  2024,  2024,   432,   439, -1108, -1108,  2228,
    3365,    12,   311, -1108,  9607,  2024,   731, -1108,  1033, -1108,
     181,   310,   404, 10517, -1108, -1108, -1108,   527,   596,   524,
     444, 10816,   447,   148,  4077,  9449,   199,   454,   707,   199,
     379,   349, -1108,  1033,  1567,   461,  8185,  1567, -1108, -1108,
     322, -1108, -1108, -1108, -1108,   428, -1108, -1108, -1108, -1108,
    9449,  9449,  9449,  8343,  9449,  9449,  9449,  9449,  9449,  9449,
    9449,  9449,  9449,  9449,  9449,  9449,  9449,  9449,  9449,  9449,
    9449,  9449,  9449,  9449,  9449,  2348, -1108,  9449,  9449,  9449,
     470,   202,   214,  1033,  1033,   428,  1178,  3086,  9449,  9449,
    9449,  9449,  9449,  9449,  9449,  9449,  9449,  9449,  9449, -1108,
   -1108,   478, -1108,   242,  9449,  9449, -1108,  8185,  9449,  9449,
     248,   251,  2228,   468,  8501, 10370, -1108,   476,   642,  1748,
     452,   176,   470,   426,  8659, -1108,  8817, -1108,   487,   201,
   -1108,    25,  8185, -1108,   597, -1108,   253, -1108, -1108, 10411,
   -1108, -1108,  9449, -1108,   569,  6605,   650,   493, 10709,   660,
      43,    40, -1108, -1108, -1108, -1108, -1108,  1567,   595,   504,
     670, -1108,  1873, -1108, -1108,  4235, -1108,   198,   978, -1108,
    1033,  9449,   426,    18, -1108,  1873,   611, -1108,   426,    66,
      68,   212,   517,  1033,   579,   529,   426,    70,   532,  1168,
    1033, -1108, -1108,   641,  1459,   -22, -1108, -1108, -1108,   310,
   -1108, -1108, -1108, -1108,  9449,   587,   549,   128, -1108,   586,
     704,   548,  1567,  1567,   714,   220,   659,   133, -1108, -1108,
   -1108, -1108, -1108, -1108,  1619, -1108, -1108, -1108, -1108,    50,
    2024,   557,   734, 10816,   726, -1108, -1108,   628,   583,  3524,
    3750,  9764,  9449, 10775,  2538,  3572,  3039,  3069,  7229,  7387,
    7387,  7387,  7387,  2798,  2798,  2798,  2798,  1382,  1382,   515,
     515,   515,   477,   477,   477, -1108, 10816,   578,   585, 10558,
     589,   754,   -50,   601,   251, -1108, -1108,  1033, -1108,  1530,
    9449, -1108,  9764,  9764,  9764,  9764,  9764,  9764,  9764,  9764,
    9764,  9764,  9764,  9764,  9449,   -50,   603,   599,  1836,   608,
     605,  2041,    71, -1108,  2058, -1108,  1033, -1108,   444,   220,
     373,  2024, 10816,  2024, 10613,   240,   256, -1108,   609,  9449,
   -1108, -1108, -1108,  6289,   268, 10816,   -48, -1108, -1108, -1108,
    9449,  1614,  1873,  1033,  6763,   602,   618, -1108,    84,   685,
   -1108,   802,   638,   362,  1567,  1873,  1873,  1873,   640,    16,
     673,   646,   365, -1108,   674, -1108,   644, -1108, -1108, -1108,
     720,  1033, -1108, -1108,  3264, -1108, -1108,   812,  2024,   653,
   -1108, -1108, -1108,   738,    93,   451,   676,  2228,  2292,   839,
   -1108, -1108, -1108, -1108,   677, -1108,  9449, -1108, -1108,  3582,
   -1108, 10816,   451,   678, -1108, -1108, -1108, -1108,   842,  9449,
     527, -1108, -1108,   684, -1108,  1567,   744, -1108,   257, -1108,
   -1108,  1567, -1108,   426, -1108,  8975, -1108,  1873,    30,   687,
     451,   587, -1108, 10839,  9449, -1108, -1108,  9449, -1108,  9449,
   -1108,   691,  8185,   579,   587, -1108,   628,  2348,   426,  9916,
     694,  8185, -1108, -1108,   262, -1108, -1108,   862,   690,   690,
    2058, -1108, -1108, -1108,   705,    47,   709, -1108, -1108, -1108,
     871,   711,   476,   426,   426,  9133, -1108,   263, -1108, -1108,
    9957,   315,   -48,  7237, -1108,   708,  4393,   735,  2024,   727,
     778,   426, -1108,   895, -1108, -1108, -1108, -1108,   519, -1108,
      14,  1567, -1108,  1567,   595, -1108, -1108, -1108,   903,   740,
     741, -1108, -1108,   793,   739,   909,  1873,   785,  1033,   527,
    1033,  1873,   752, -1108,   767, -1108, -1108,  1873,   426, -1108,
    1567,  1033, -1108,   921, -1108, -1108,    73,   759,   426,  9291,
   -1108,  1412, -1108,  3761,   921, -1108,   -41,   -52, 10816,   811,
   -1108,   760,  9449,   -50,   763, -1108,  2024, 10816, -1108, -1108,
     765,   931, -1108,  1567,    30, -1108,   766,    30, 10839, 10816,
   10654,  8185,   770,   771,   775,   780,   587,   396,   791,  8185,
     792,  9449, -1108, -1108, -1108, -1108, -1108,   829,   790,   964,
    2058,   827, -1108,   527,  2058, -1108, -1108, -1108,  2024, 10816,
   -1108,   -48,   948,   908,  7237, -1108, -1108,   803,  9449,   426,
    2228,  1614,   805,  1873,  4551,   588,   809,  9449,    89,   282,
   -1108,   816, -1108, -1108,  1138,   959, -1108,  1873, -1108,  1873,
   -1108,   814, -1108,   866,   982,   818, -1108,   870,   823,   993,
     451,   828, -1108, -1108,   912,   451,   725, -1108,  2228, -1108,
    9764,   831, -1108,   779, -1108,   260,  9449, -1108, -1108, -1108,
    9449, -1108,  9449, -1108,  9998,   836,  8185,   426,   983,    85,
   -1108, -1108,    69,   838, -1108,   841,  9449,   843, -1108, -1108,
   -1108,    30,   840, -1108,  8185,   844, -1108,  2058, -1108,  2058,
   -1108,   846, -1108,   899,   850,  1016, -1108,   426,  1002, -1108,
     853, -1108, -1108,   856,    75, -1108, -1108, -1108,   861,   863,
   -1108, 10121, -1108, -1108, -1108, -1108, -1108, -1108,  1567, -1108,
     916, -1108,  1873,   527, -1108, -1108,  1873, -1108,  1873, -1108,
     957, -1108,  1567, -1108,  1567,   451, -1108,  2364,   889,  1064,
   -1108, -1108, -1108,   945,   796,    56,  1265,    77, -1108, -1108,
     898, 10039, 10080, 10816,   878,  8185,   879,  1567,   949, -1108,
    1567,   992,  1042,   983,   969,   983, 10816, -1108,  1001,  1234,
     883, -1108,   896, -1108, -1108,   952, -1108,  2058, -1108,   527,
   -1108, -1108,  6289, -1108, -1108, -1108,  6921, -1108, -1108, -1108,
    6289,   913,  1873, -1108,   960, -1108,   963,   917,  4709, -1108,
    1058,    44, -1108, -1108, -1108,    57,   914,    59, -1108,  9765,
   -1108, -1108,    60, -1108, -1108,   911, -1108,   919, -1108,  1018,
     428, -1108,  1567, -1108,   945,  1265, -1108, -1108, -1108, -1108,
   -1108,  8185,   922, -1108, -1108,   924,   923,    87,  1090,  1873,
     939, -1108,   949, -1108, -1108,   983,  1419, -1108,  2058, -1108,
     997,  6289,  7079, -1108, -1108, -1108,  6289, -1108, -1108,  1873,
    1873,   940, -1108,  1873,   451, -1108, -1108,  1672,  2364, -1108,
   -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108,
   -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108,
   -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108,
   -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108,
   -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108,
   -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108,
   -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108,
   -1108, -1108, -1108, -1108, -1108, -1108, -1108, -1108,   438, -1108,
     889, -1108, -1108, -1108, -1108, -1108,    55,   303, -1108,  1108,
      61,  1033,  1018,  1110,   428,   951, -1108,   106, -1108,  1052,
    1115,  1873, -1108, -1108,   958,   962, -1108, -1108,  2058, -1108,
   -1108, -1108,  4867, -1108, -1108, -1108, -1108, -1108,   348,    41,
   -1108, -1108,  1873,  9765,  9765,  1083, -1108,   911,   911,   512,
   -1108, -1108, -1108,  1873,  1067, -1108,   970,    62,  1873,  1033,
   -1108,  1068, -1108,  1139,  5025,  1133,  1873, -1108,  5183, -1108,
   -1108, -1108,  5341,   974,  5499, -1108,  1061,  1013, -1108, -1108,
    1065,  1672, -1108, -1108, -1108, -1108,  1003, -1108,  1129, -1108,
   -1108, -1108, -1108, -1108,  1147, -1108, -1108, -1108,   989, -1108,
     107,   994, -1108,  1873, -1108, -1108,  5657,  5815,   988, -1108,
   -1108,  1033,  1265, -1108, -1108,  1873,   114, -1108,  1095, -1108,
   -1108, -1108, -1108, -1108, -1108,    19,  1014,  1033,   688, -1108,
   -1108,  1005,  1162,   617,   114, -1108,  1008, -1108, -1108,   451,
    1019, -1108,   983,   425, -1108, -1108, -1108, -1108,  1567, -1108,
    1015,   451,    64, -1108,   -64, -1108,  1134,   118,   983,  1114,
   -1108, -1108, -1108, -1108,  1567,  1117,  1179,   -64,  1021,  5973,
     280,  1182,  1873, -1108,  1026, -1108,  1123,  1189,  1873, -1108,
   -1108,  1195,  1873, -1108,  6131,  1873, -1108, -1108, -1108
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1108, -1108, -1108,  -403, -1108, -1108, -1108,    -4, -1108,   801,
       2,   956,  1556, -1108,  1565, -1108,  -213, -1108,     5, -1108,
   -1108, -1108, -1108, -1108, -1108,   -95, -1108, -1108,  -150,     3,
       0, -1108, -1108,     4, -1108, -1108, -1108, -1108,     7, -1108,
   -1108,   885,   890,   888,  1093,   541,  -596,   545,   593,   -98,
   -1108,   386, -1108, -1108, -1108, -1108, -1108, -1108,  -473,   287,
   -1108, -1108, -1108, -1108,   -80, -1108,  -648, -1108,  -339, -1108,
   -1108,   824, -1108,  -731, -1108, -1108, -1108, -1108, -1108, -1108,
   -1108, -1108, -1108, -1108,   140, -1108, -1108, -1108, -1108, -1108,
      63, -1108,   267, -1107, -1108,  -106, -1108,  -940,  -857,  -112,
     -81, -1108,    54, -1108,   -49,   -18,  1217,  -532,  -314, -1108,
   -1108,  2676,  1166, -1108, -1108,  -609, -1108, -1108, -1108, -1108,
   -1108, -1108, -1108, -1108, -1108, -1108,   184, -1108,   483, -1108,
   -1108, -1108, -1108, -1108, -1108, -1108, -1108,  -826, -1108,  1378,
     116,  -298, -1108, -1108,   456,  1525,   104, -1108, -1108,   507,
    -264,  -784, -1108, -1108,   571,  -535,   441, -1108, -1108, -1108,
   -1108, -1108,   564, -1108, -1108, -1108,  -631,  -885,  -163,  -156,
    -103, -1108, -1108,    10, -1108, -1108, -1108, -1108,   -15,   -90,
   -1108,  -226, -1108, -1108, -1108,  -352,  1027, -1108, -1108, -1108,
   -1108, -1108,   671,   680, -1108, -1108,  1035, -1108, -1108, -1108,
    -271,   -85,  -159,  -253, -1108,  -965, -1108,   536, -1108, -1108,
   -1108,  -195,  -950
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -707
static const yytype_int16 yytable[] =
{
     101,   265,   296,   363,   110,   268,   269,   109,   111,   108,
     552,   112,   201,   194,   135,   415,   484,   910,   549,   709,
     405,   406,   205,   531,  1000,   411,   209,   515,   293,   984,
     380,   479,   385,   356,   180,   181,   984,   802,  1080,   386,
    1082,   789,   212,   568,   270,   221,  1064,   639,   727,  1271,
    1077,   583,  1104,   361,   893,   233,   287,   392,   661,   418,
     423,   426,   246,  1237,  1062,  -223,   585,  1108,  1190,  1244,
    1244,    11,  1104,   439,   618,   806,   618,   260,   628,   628,
     261,   628,   246,   628,  1238,   628,   246,   246,   814,   411,
     240,   234,   741,   387,   757,   899,   358,    35,    35,   351,
    1001,   771,   518,   273,   557,     3,   894,   857,   858,   246,
    1351,  1352,   284,   740,   157,   952,   197,   295,  1209,   895,
    1215,  1326,  -706,   900,   200,   831,    11,   370,   547,  1065,
    1278,  1279,   550,   440,   896,   206,   335,  1251,  1308,   378,
     179,   179,  1002,   207,   191,   488,   208,   984,   228,  1355,
     873,   984,   984,   253,   638,    11,   913,  -492,   367,   915,
    1210,   364,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   802,   645,  -493,   802,   573,   -85,  1252,
    1309,  -605,  -608,  1004,   832,   527,   371,   799,  1008,  1009,
     860,  1356,   373,   354,  -612,   235,   651,   101,   379,  -606,
     101,   360,    96,  -172,   399,   486,   391,   349,   350,   394,
     359,   135,   588,   662,   135,   586,   584,  1272,  1105,  1106,
     922,   413,   288,   393,   933,   419,   424,   427,  1239,   984,
    1063,  -223,   417,  1109,  1191,  1245,  1286,   783,  1350,   619,
     524,   620,   999,   629,   697,   422,   887,   742,  1025,   265,
    1066,   293,   428,   428,   431,   986,   772,   680,  -541,   436,
     449,   524,   986,   953,   855,   445,   859,   653,   654,   351,
     101,  1320,   251,  1010,   646,   776,  -607,   109,   802,   478,
    1086,   354,   524,   221,   135,   420,   246,  -160,   611,   647,
     802,   524,   179,  -614,   524,  -605,  -608,   351,   179,   936,
    -642,   354,   657,  -609,   179,  -615,   990,   355,  -612,   658,
     194,  1366,   358,  -606,  -643,   523,   943,  -645,  -610,   238,
     558,   467,   351,  -611,  -644,   283,   522,   745,  1240,   246,
     246,   239,   246,   351,   722,   723,   546,   254,   255,   283,
     748,   656,   283,   812,   242,  1241,   243,   545,  1242,   467,
     244,   179,   820,  1367,  1034,   451,   452,   523,   179,   179,
     179,   384,  1317,   681,   726,   179,   567,  1344,   560,   571,
     266,   179,   436,   986,   566,   857,   858,   986,   986,   371,
     570,   841,   842,  1357,   251,   621,   690,   215,  1347,   817,
    -607,   101,  1014,    33,  1015,   355,   657,   245,   411,   750,
     578,   681,   716,   658,  1360,   135,   266,  1274,  1275,   717,
    1090,   101,  -706,   216,  -642,   355,   613,  -609,   109,    33,
     233,  1323,   251,   275,   249,   135,   359,   446,  -643,   623,
     250,  -645,  -610,    33,   284,   633,   635,  -611,  -644,  1323,
     282,   191,   685,   380,  -706,  1318,   835,  -706,   441,   254,
     255,  1232,   251,   267,   283,   986,   286,   446,   955,   289,
     791,  1354,   718,   297,  1233,   889,   795,   298,   299,  1291,
      81,    82,   917,    83,    84,    85,  -383,   327,   179,   251,
     925,  1234,  1089,   328,   276,   179,   217,   254,   255,   151,
     663,   870,    78,   329,   485,   330,    81,    82,   944,    83,
      84,    85,   357,   151,  -613,  -706,    78,  -384,   218,  -492,
      81,    82,   362,    83,    84,    85,   608,   254,   255,   747,
     258,   854,    33,   246,  1265,   366,   325,   284,   219,   616,
     477,  1335,  1336,  1337,   220,   284,   987,  1280,  1334,   688,
    -706,    33,   372,    35,   254,   255,   861,   351,   862,    33,
     707,    35,   712,  1217,  1281,   376,   375,  1282,   725,   476,
     322,   323,   324,  -491,   325,   930,   524,   996,   382,   101,
     381,   713,   403,   714,   384,   883,   109,   408,   453,   733,
     101,   407,  1345,   135,    33,  1012,    35,  -701,   412,   735,
     215,   731,   151,   425,   135,    78,   179,    80,  -541,    81,
      82,   433,    83,    84,    85,   432,   458,   765,   911,   462,
     964,   856,   857,   858,   463,   969,   216,   465,    81,    82,
     466,    83,    84,    85,   175,   559,    81,    82,   768,    83,
      84,    85,   475,   -37,   905,   101,    33,   436,   778,   110,
     485,   794,   109,   111,   108,   179,   112,    96,   554,   135,
     556,   576,   793,   544,   151,    96,  1072,    78,   392,    80,
     565,    81,    82,   409,    83,    84,    85,   580,    33,   795,
      35,  1020,   194,   582,   589,   802,   593,   179,   594,   179,
     949,   857,   858,  1261,   617,   176,   732,   251,   421,   217,
      96,   622,   252,   802,   824,   824,   707,   179,   625,   751,
     752,   627,   636,  1040,   844,   630,   151,   642,   649,    78,
     644,   218,   650,    81,    82,   802,    83,    84,    85,   101,
    -385,   652,   101,  1335,  1336,  1337,  1022,   655,   845,   109,
     664,   219,  1205,   135,   179,   251,   135,   220,   849,   667,
     279,  1030,   665,   179,   179,    81,    82,   670,    83,    84,
      85,   253,   254,   255,   872,   672,   876,   675,  1038,   822,
     823,    33,   679,  1031,   676,   678,    11,   884,   278,   280,
     281,   798,   569,   682,    96,   691,   737,  1039,   692,   101,
     251,   694,   719,   110,   695,   446,   109,   111,   108,  1061,
     112,  1203,   739,   135,    55,    56,    57,   147,   148,   294,
     254,   255,  1074,   191,   251,   911,   907,   743,  1091,   446,
     744,   746,   756,   758,   760,    33,  1096,    35,   759,   761,
      11,   215,   762,   938,   974,   767,   707,   769,   770,   975,
     707,    55,    56,    57,   147,   148,   294,   976,    81,    82,
     101,    83,    84,    85,   179,   254,   255,   216,   937,   941,
     101,   775,   779,   785,   135,   787,   780,   109,   790,   804,
     436,   731,  1331,   811,   135,   877,   819,    33,   447,   254,
     255,   882,   296,   977,   978,   821,   979,   830,   974,   834,
    1222,   833,   846,   975,   836,    55,    56,    57,   147,   148,
     294,   976,    81,    82,  -258,    83,    84,    85,   436,   850,
     851,   980,    55,    56,    57,   147,   148,   294,   853,   848,
    1201,   864,   179,   865,   866,   867,    33,   869,   868,   792,
     217,    96,   441,   707,   880,   707,   881,   977,   978,   885,
     979,   888,   442,   902,   903,   906,   448,   151,   908,   909,
      78,   914,   218,   918,    81,    82,   919,    83,    84,    85,
     920,   927,   921,   296,   179,   989,   442,   947,   448,   442,
     448,   448,   219,   924,   932,   926,   179,   179,   220,   928,
     221,   959,   929,   960,   939,  1056,   940,   942,   946,   956,
     177,   177,    33,   950,   189,   958,  1060,   961,   962,   258,
     963,   965,   966,    81,    82,  1254,    83,    84,    85,   967,
    1258,   968,   972,   971,   179,   189,  1262,   988,   995,   997,
      11,  1003,  1264,   707,  1005,  1011,  1007,  1013,   101,  1016,
     259,  1017,   101,  1018,  1019,   109,   101,  1023,  1021,  1024,
    1037,  1094,   135,   109,   101,  1027,   135,  1028,  1032,  1051,
     135,   109,    11,    48,  1067,  1187,  1296,  1297,   135,    33,
    1071,  1194,  1073,  1075,  1249,  1079,  1085,  1192,   221,    81,
      82,  1193,    83,    84,    85,  1078,  1033,  1204,   974,  1087,
    1035,  1103,  1036,   975,  1088,    55,    56,    57,   147,   148,
     294,   976,  1099,  1055,   707,  1100,  1097,   101,   101,  1107,
    1101,  1199,   101,  1198,   109,  1206,  1207,  1220,  1208,   109,
     974,   135,   135,  1211,    33,   975,   135,    55,    56,    57,
     147,   148,   294,   976,  1213,  1225,  1246,   977,   978,  1218,
     979,  1243,   230,  1248,  1250,  1255,    81,    82,  1256,    83,
      84,    85,   177,  1259,  1276,    33,  1098,  1260,   177,  1359,
    1284,  1289,  1285,   911,   177,  1081,  1293,  1290,  1299,   977,
     978,  -219,   979,  1301,    48,  1302,  1304,  1374,  1238,   911,
    1305,  1307,  1314,   215,  1288,   331,   332,  1310,  1325,  1329,
    1333,   189,   189,    65,   333,    11,   189,  1083,  1332,  1054,
    1341,    81,    82,  1212,    83,    84,    85,  1358,  1348,   216,
    1361,   177,  1362,  1343,  1364,  1368,  1371,   246,   177,   177,
     177,  1370,  1372,  1223,  1224,   177,   296,  1226,  1375,    33,
     334,   177,    81,    82,   707,    83,    84,    85,   101,   612,
    1327,   528,   526,   525,  1266,   109,   336,   816,   813,  1187,
    1187,  1342,   135,  1194,  1194,   784,  1055,   945,  1029,    33,
     189,   631,   632,   189,  1340,   246,  1058,   615,  1231,    33,
     101,  1363,  1346,  1236,   101,   202,  1247,   109,   101,   272,
     101,   109,   217,   871,   135,   109,  1214,   109,   135,   897,
     826,   931,   135,   923,   135,    11,   837,   438,   430,   151,
     863,   189,    78,     0,   218,     0,    81,    82,     0,    83,
      84,    85,   101,   101,     0,   957,     0,  1316,     0,   109,
     109,     0,     0,     0,   219,     0,   135,   135,     0,     0,
     220,     0,     0,  1330,     0,  1257,    81,    82,   177,    83,
      84,    85,     0,     0,   291,   177,    81,    82,     0,    83,
      84,    85,     0,   974,     0,     0,  1273,     0,   975,     0,
      55,    56,    57,   147,   148,   294,   976,  1283,     0,     0,
       0,     0,  1287,     0,     0,   101,     0,     0,     0,     0,
    1294,     0,   109,   189,     0,     0,     0,     0,   602,   135,
     101,    55,    56,    57,   147,   148,   294,   109,    48,     0,
       0,   602,   977,   978,   135,   979,    55,    56,    57,   147,
     148,   294,     0,     0,     0,     0,     0,  1311,     0,     0,
       0,     0,   178,   178,     0,     0,   190,     0,     0,  1319,
    1084,     0,     0,     0,     0,     0,     0,     0,   189,   189,
       0,   300,   301,   302,   319,   320,   321,   322,   323,   324,
       0,   325,     0,     0,     0,     0,   177,   303,     0,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
      11,   325,     0,     0,     0,     0,  1369,     0,   300,   301,
     302,     0,  1373,     0,     0,     0,  1376,     0,     0,  1378,
       0,     0,     0,     0,   303,   177,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,     0,   325,     0,
       0,     0,     0,     0,     0,     0,     0,   177,   974,   177,
       0,     0,     0,   975,     0,    55,    56,    57,   147,   148,
     294,   976,     0,     0,     0,     0,     0,   177,   602,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   189,
     189,   602,   602,   602,   178,     0,     0,     0,     0,     0,
     178,     0,     0,     0,     0,     0,   178,   977,   978,     0,
     979,     0,     0,     0,   177,     0,     0,     0,     0,     0,
       0,   189,     0,   177,   177,     0,     0,     0,     0,     0,
       0,   891,   215,   687,     0,  1216,     0,     0,   189,     0,
       0,    33,     0,    35,     0,     0,     0,     0,     0,     0,
       0,   189,     0,   178,   222,     0,     0,   189,   216,     0,
     178,   178,   178,   602,     0,     0,   189,   178,   300,   301,
     302,     0,     0,   178,     0,   637,     0,     0,    33,     0,
       0,   175,     0,   189,   303,   728,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,     0,   325,     0,
       0,   151,     0,     0,    78,     0,    80,     0,    81,    82,
       0,    83,    84,    85,   177,    33,     0,    35,     0,     0,
       0,   217,     0,     0,     0,     0,     0,   189,     0,   189,
       0,     0,   176,   190,     0,     0,     0,    96,   151,     0,
       0,    78,   602,   218,     0,    81,    82,   602,    83,    84,
      85,     0,     0,   602,     0,   175,   189,     0,     0,     0,
       0,     0,     0,   219,     0,     0,     0,   729,     0,   220,
     178,    31,    32,     0,     0,     0,     0,   178,     0,     0,
       0,    37,   177,     0,     0,   151,     0,     0,    78,   189,
      80,     0,    81,    82,     0,    83,    84,    85,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     222,   222,     0,     0,     0,   222,   176,     0,     0,     0,
     606,    96,     0,     0,   177,   660,     0,    69,    70,    71,
      72,    73,     0,   606,     0,     0,   177,   177,   599,   602,
      27,    28,     0,     0,    76,    77,     0,     0,     0,    33,
     189,    35,     0,   602,     0,   602,     0,     0,    87,     0,
       0,     0,     0,     0,     0,     0,   189,     0,     0,     0,
       0,   189,     0,    92,   177,   300,   301,   302,     0,   222,
       0,     0,   222,     0,     0,     0,     0,     0,   178,   175,
       0,   303,     0,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,     0,   325,     0,     0,     0,   151,
       0,     0,    78,     0,    80,     0,    81,    82,     0,    83,
      84,    85,     0,     0,     0,     0,     0,   178,    88,     0,
       0,     0,     0,     0,   189,   595,   596,     0,   602,     0,
     369,     0,   602,     0,   602,    96,     0,     0,   189,     0,
     189,   189,     0,   189,   597,     0,     0,   607,     0,   178,
     189,   178,    31,    32,    33,     0,     0,     0,     0,     0,
     607,     0,    37,   189,     0,     0,   189,     0,     0,   178,
     606,     0,     0,     0,     0,     0,     0,     0,   603,     0,
       0,     0,   222,   606,   606,   606,     0,   604,     0,     0,
       0,   603,     0,     0,     0,     0,     0,     0,   602,     0,
     604,     0,     0,     0,     0,     0,   178,   598,    69,    70,
      71,    72,    73,   774,     0,   178,   178,     0,     0,   599,
       0,     0,   693,     0,   151,    76,    77,    78,   189,   600,
     774,    81,    82,     0,    83,    84,    85,   222,   222,    87,
       0,     0,     0,     0,     0,   602,     0,     0,     0,   601,
       0,     0,     0,     0,    92,   606,     0,     0,   805,     0,
     300,   301,   302,     0,     0,   602,   602,     0,     0,   602,
     189,     0,     0,     0,   189,   190,   303,     0,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,     0,
     325,     0,     0,     0,     0,    33,     0,    35,     0,     0,
     698,   699,     0,     0,     0,     0,   178,   607,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   700,
     607,   607,   607,     0,     0,     0,     0,   701,   702,    33,
       0,     0,     0,     0,   606,   175,     0,   703,   603,   606,
       0,     0,     0,     0,     0,   606,     0,   604,     0,     0,
       0,   603,   603,   603,     0,     0,     0,     0,   222,   222,
     604,   604,   604,     0,     0,   151,     0,   602,    78,     0,
      80,     0,    81,    82,   178,    83,    84,    85,     0,     0,
       0,     0,   704,     0,   189,     0,     0,     0,   602,     0,
       0,     0,   607,     0,   705,     0,   176,     0,     0,   602,
       0,    96,     0,     0,   602,     0,    81,    82,     0,    83,
      84,    85,   602,     0,     0,     0,   178,   696,     0,     0,
     222,     0,     0,   603,   706,     0,   222,     0,   178,   178,
       0,   606,   604,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   606,     0,   606,     0,   602,
       0,     0,     0,     0,     0,     0,     0,     0,   970,     0,
       0,   602,     0,   973,     0,     0,   178,   364,   338,   339,
     340,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   607,     0,     0,     0,   189,   607,     0,     0,     0,
       0,     0,   607,     0,   189,     0,     0,   189,     0,    33,
       0,    35,     0,     0,     0,     0,   222,     0,   222,     0,
     189,     0,   603,   349,   350,     0,     0,   603,   602,     0,
       0,   604,     0,   603,   602,     0,   604,     0,   602,     0,
       0,   602,   604,     0,     0,   222,     0,     0,     0,   175,
     606,   351,     0,     0,   606,     0,   606,     0,     0,     0,
       0,   435,     0,  1042,     0,  1050,     0,     0,     0,     0,
       0,     0,     0,    33,     0,    35,     0,     0,   222,   151,
       0,     0,    78,     0,    80,   351,    81,    82,   607,    83,
      84,    85,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   607,     0,   607,     0,     0,     0,     0,     0,
     176,     0,     0,   175,     0,    96,     0,     0,     0,   603,
     606,     0,     0,     0,     0,   777,     0,     0,   604,    33,
       0,    35,     0,   603,     0,   603,     0,     0,     0,   222,
       0,     0,   604,   151,   604,    33,    78,     0,    80,     0,
      81,    82,     0,    83,    84,    85,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   606,     0,   187,
       0,     0,     0,     0,   176,     0,     0,     0,     0,    96,
       0,     0,     0,     0,     0,     0,  1043,   606,   606,     0,
       0,   606,  1227,     0,     0,     0,  1050,   607,  1044,   151,
       0,   607,    78,   607,    80,     0,    81,    82,     0,    83,
      84,    85,     0,     0,     0,   151,     0,     0,    78,     0,
    1045,     0,    81,    82,     0,    83,  1046,    85,   603,     0,
     188,     0,   603,   222,   603,    96,     0,   604,     0,     0,
       0,   604,     0,   604,     0,     0,     0,   222,     0,   222,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   222,
       0,     0,     0,     0,     0,     0,     0,   607,     0,     0,
       0,     0,   222,     0,     0,   222,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,     0,   325,   603,   606,
       0,     0,     0,     0,     0,     0,     0,   604,     0,     0,
       0,     0,     0,     0,   607,     0,     0,     0,     0,     0,
     606,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   606,     0,     0,   607,   607,   606,   222,   607,     0,
       0,     0,  1230,     0,   606,   603,     0,     0,     0,     0,
       0,     0,     0,     0,   604,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   603,   603,     0,     0,   603,
       0,     0,     0,     0,   604,   604,     0,     0,   604,     0,
       0,   606,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   154,   156,   606,   158,   159,   160,     0,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
       0,     0,   183,   186,     0,     0,     0,  1042,     0,     0,
       0,     0,     0,     0,   203,     0,     0,     0,     0,  1349,
       0,   211,     0,   214,     0,     0,   227,     0,   229,     0,
       0,     0,     0,     0,     0,     0,   607,     0,     0,     0,
     606,     0,     0,     0,     0,     0,   606,     0,     0,     0,
     606,     0,     0,   606,     0,   263,     0,   607,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   603,   607,   271,
       0,     0,     0,   607,     0,     0,   604,     0,     0,     0,
       0,   607,     0,     0,     0,     0,     0,     0,   603,     0,
       0,     0,     0,  1267,     0,     0,  1303,   604,     0,   603,
       0,     0,     0,     0,   603,     0,     0,     0,   604,     0,
       0,     0,   603,   604,     0,     0,     0,     0,   607,     0,
       0,   604,     0,     0,     0,     0,     0,     0,     0,     0,
     607,     0,     0,   365,  -707,  -707,  -707,  -707,   317,   318,
     319,   320,   321,   322,   323,   324,     0,   325,     0,   603,
       0,     0,     0,     0,     0,     0,     0,     0,   604,     0,
       0,   603,     0,     0,     0,     0,     0,     0,     0,     0,
     604,     0,     0,   389,     0,     0,   389,     0,     0,     0,
       0,     0,   203,   398,     0,     0,     0,   607,     0,     0,
       0,     0,     0,   607,     0,     0,     0,   607,     0,     0,
     607,     0,     0,   222,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   603,   222,
       0,     0,     0,     0,   603,     0,   183,   604,   603,     0,
     444,   603,     0,   604,     0,     0,     0,   604,     0,     0,
     604,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   474,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   483,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   489,   490,   491,   493,
     494,   495,   496,   497,   498,   499,   500,   501,   502,   503,
     504,   505,   506,   507,   508,   509,   510,   511,   512,   513,
     514,     0,     0,   516,   516,   519,     0,     0,     0,     0,
       0,     0,     0,   532,   533,   534,   535,   536,   537,   538,
     539,   540,   541,   542,   543,     0,     0,     0,     0,     0,
     516,   548,     0,   483,   516,   551,     0,     0,     0,     0,
     532,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     562,     0,   564,     0,     0,     0,     0,     0,   483,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   575,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,     0,   325,     5,
       6,     7,     8,     9,     0,     0,     0,   614,    10,     0,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   529,   325,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
     641,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
       0,     0,     0,     0,     0,    37,     0,     0,   673,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,    55,    56,    57,   147,   148,   149,     0,     0,
      62,    63,     0,     0,     0,     0,   263,     0,     0,   150,
      68,    69,    70,    71,    72,    73,     0,     0,     0,     0,
     689,     0,    74,     0,     0,     0,     0,   151,    76,    77,
      78,   530,    80,     0,    81,    82,     0,    83,    84,    85,
       0,     0,    87,     0,     0,   720,    88,     0,     0,     0,
       0,     0,    89,     0,     0,     0,   203,    92,    93,     0,
       0,     0,     0,    96,    97,     0,    98,    99,     0,     0,
       0,     0,     0,   300,   301,   302,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   303,
       0,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   781,   325,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   788,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   797,     0,     0,     0,     0,     0,     0,     0,     0,
     808,     0,     0,   809,     0,   810,     0,     0,   483,     0,
       0,     0,     0,     0,     0,     0,     0,   483,     5,     6,
       7,     8,     9,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   839,     0,     0,     0,     0,   182,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,    35,     0,
     766,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   890,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,   904,     0,
       0,    55,    56,    57,   147,   148,   149,     0,     0,    62,
      63,     0,     0,     0,     0,     0,     0,   483,   150,    68,
      69,    70,    71,    72,    73,   483,     0,   890,     0,     0,
       0,    74,     0,     0,     0,     0,   151,    76,    77,    78,
       0,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,    87,     0,     0,   203,    88,     0,     0,     0,     0,
       0,    89,     0,   951,   301,   302,    92,    93,     0,     0,
       0,     0,    96,    97,     0,    98,    99,     0,     0,   303,
       0,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   991,   325,     0,     0,   992,     0,   993,     0,
       0,     0,   483,     0,     0,     5,     6,     7,     8,     9,
       0,     0,  1006,     0,    10,     0,     0,     0,     0,     0,
     483,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
       0,   325,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,     0,    41,     0,    42,     0,
      43,   483,     0,    44,     0,     0,     0,    45,    46,    47,
      48,    49,    50,    51,     0,    52,    53,    54,    55,    56,
      57,    58,    59,    60,     0,    61,    62,    63,    64,    65,
      66,     0,     0,     0,     0,    67,    68,    69,    70,    71,
      72,    73,     0,     0,     0,     0,     0,     0,    74,     0,
       0,     0,     0,    75,    76,    77,    78,    79,    80,     0,
      81,    82,     0,    83,    84,    85,    86,     0,    87,     0,
       0,     0,    88,     0,     0,     0,     0,   483,    89,    90,
       0,    91,     0,    92,    93,     0,    94,    95,   782,    96,
      97,   302,    98,    99,     5,     6,     7,     8,     9,     0,
       0,     0,     0,    10,     0,   303,     0,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,     0,   325,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
      49,    50,    51,     0,    52,    53,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,    64,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,    75,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     5,     6,     7,     8,     9,    89,    90,     0,
      91,    10,    92,    93,     0,    94,    95,   892,    96,    97,
       0,    98,    99,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,    49,    50,
      51,     0,    52,    53,    54,    55,    56,    57,    58,    59,
      60,     0,    61,    62,    63,    64,    65,    66,     0,     0,
       0,     0,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
      75,    76,    77,    78,    79,    80,     0,    81,    82,     0,
      83,    84,    85,    86,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,    90,     0,    91,    10,
      92,    93,     0,    94,    95,     0,    96,    97,     0,    98,
      99,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,     0,    43,     0,     0,    44,     0,
       0,     0,    45,    46,    47,    48,     0,    50,    51,     0,
      52,     0,    54,    55,    56,    57,    58,    59,    60,     0,
      61,    62,    63,     0,    65,    66,     0,     0,     0,     0,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   151,    76,
      77,    78,    79,    80,     0,    81,    82,     0,    83,    84,
      85,    86,     0,    87,     0,     0,     0,    88,     5,     6,
       7,     8,     9,    89,     0,     0,     0,    10,    92,    93,
       0,    94,    95,   468,    96,    97,     0,    98,    99,     0,
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
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   151,    76,    77,    78,
      79,    80,     0,    81,    82,     0,    83,    84,    85,    86,
       0,    87,     0,     0,     0,    88,     5,     6,     7,     8,
       9,    89,     0,     0,     0,    10,    92,    93,     0,    94,
      95,   610,    96,    97,     0,    98,    99,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,   847,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,     0,    50,    51,     0,    52,     0,    54,    55,
      56,    57,    58,    59,    60,     0,    61,    62,    63,     0,
      65,    66,     0,     0,     0,     0,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   151,    76,    77,    78,    79,    80,
       0,    81,    82,     0,    83,    84,    85,    86,     0,    87,
       0,     0,     0,    88,     5,     6,     7,     8,     9,    89,
       0,     0,     0,    10,    92,    93,     0,    94,    95,     0,
      96,    97,     0,    98,    99,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
     948,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   151,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     5,     6,     7,     8,     9,    89,     0,     0,
       0,    10,    92,    93,     0,    94,    95,     0,    96,    97,
       0,    98,    99,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,     0,    50,
      51,     0,    52,     0,    54,    55,    56,    57,    58,    59,
      60,     0,    61,    62,    63,     0,    65,    66,     0,     0,
       0,     0,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     151,    76,    77,    78,    79,    80,     0,    81,    82,     0,
      83,    84,    85,    86,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,     0,     0,     0,    10,
      92,    93,     0,    94,    95,  1102,    96,    97,     0,    98,
      99,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,  1263,    43,     0,     0,    44,     0,
       0,     0,    45,    46,    47,    48,     0,    50,    51,     0,
      52,     0,    54,    55,    56,    57,    58,    59,    60,     0,
      61,    62,    63,     0,    65,    66,     0,     0,     0,     0,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   151,    76,
      77,    78,    79,    80,     0,    81,    82,     0,    83,    84,
      85,    86,     0,    87,     0,     0,     0,    88,     5,     6,
       7,     8,     9,    89,     0,     0,     0,    10,    92,    93,
       0,    94,    95,     0,    96,    97,     0,    98,    99,     0,
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
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   151,    76,    77,    78,
      79,    80,     0,    81,    82,     0,    83,    84,    85,    86,
       0,    87,     0,     0,     0,    88,     5,     6,     7,     8,
       9,    89,     0,     0,     0,    10,    92,    93,     0,    94,
      95,  1292,    96,    97,     0,    98,    99,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,     0,    50,    51,     0,    52,     0,    54,    55,
      56,    57,    58,    59,    60,     0,    61,    62,    63,     0,
      65,    66,     0,     0,     0,     0,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   151,    76,    77,    78,    79,    80,
       0,    81,    82,     0,    83,    84,    85,    86,     0,    87,
       0,     0,     0,    88,     5,     6,     7,     8,     9,    89,
       0,     0,     0,    10,    92,    93,     0,    94,    95,  1295,
      96,    97,     0,    98,    99,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,  1298,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   151,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     5,     6,     7,     8,     9,    89,     0,     0,
       0,    10,    92,    93,     0,    94,    95,     0,    96,    97,
       0,    98,    99,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,     0,    50,
      51,     0,    52,     0,    54,    55,    56,    57,    58,    59,
      60,     0,    61,    62,    63,     0,    65,    66,     0,     0,
       0,     0,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     151,    76,    77,    78,    79,    80,     0,    81,    82,     0,
      83,    84,    85,    86,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,     0,     0,     0,    10,
      92,    93,     0,    94,    95,  1300,    96,    97,     0,    98,
      99,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,     0,    43,     0,     0,    44,     0,
       0,     0,    45,    46,    47,    48,     0,    50,    51,     0,
      52,     0,    54,    55,    56,    57,    58,    59,    60,     0,
      61,    62,    63,     0,    65,    66,     0,     0,     0,     0,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   151,    76,
      77,    78,    79,    80,     0,    81,    82,     0,    83,    84,
      85,    86,     0,    87,     0,     0,     0,    88,     5,     6,
       7,     8,     9,    89,     0,     0,     0,    10,    92,    93,
       0,    94,    95,  1312,    96,    97,     0,    98,    99,     0,
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
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   151,    76,    77,    78,
      79,    80,     0,    81,    82,     0,    83,    84,    85,    86,
       0,    87,     0,     0,     0,    88,     5,     6,     7,     8,
       9,    89,     0,     0,     0,    10,    92,    93,     0,    94,
      95,  1313,    96,    97,     0,    98,    99,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,     0,    50,    51,     0,    52,     0,    54,    55,
      56,    57,    58,    59,    60,     0,    61,    62,    63,     0,
      65,    66,     0,     0,     0,     0,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   151,    76,    77,    78,    79,    80,
       0,    81,    82,     0,    83,    84,    85,    86,     0,    87,
       0,     0,     0,    88,     5,     6,     7,     8,     9,    89,
       0,     0,     0,    10,    92,    93,     0,    94,    95,  1365,
      96,    97,     0,    98,    99,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   151,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     5,     6,     7,     8,     9,    89,     0,     0,
       0,    10,    92,    93,     0,    94,    95,  1377,    96,    97,
       0,    98,    99,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,     0,    50,
      51,     0,    52,     0,    54,    55,    56,    57,    58,    59,
      60,     0,    61,    62,    63,     0,    65,    66,     0,     0,
       0,     0,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     151,    76,    77,    78,    79,    80,     0,    81,    82,     0,
      83,    84,    85,    86,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,     0,     0,     0,    10,
      92,    93,     0,    94,    95,     0,    96,    97,     0,    98,
      99,     0,     0,   390,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,     0,    43,     0,     0,    44,     0,
       0,     0,    45,    46,    47,    48,     0,    50,    51,     0,
      52,     0,    54,    55,    56,    57,   147,   148,    60,     0,
      61,    62,    63,     0,     0,     0,     0,     0,     0,     0,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   151,    76,
      77,    78,    79,    80,     0,    81,    82,     0,    83,    84,
      85,     0,     0,    87,     0,     0,     0,    88,     5,     6,
       7,     8,     9,    89,     0,     0,     0,    10,    92,    93,
       0,    94,    95,     0,    96,    97,     0,    98,    99,     0,
       0,   577,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,     0,     0,     0,    37,    38,    39,    40,     0,    41,
       0,    42,     0,    43,     0,     0,    44,     0,     0,     0,
      45,    46,    47,    48,     0,    50,    51,     0,    52,     0,
      54,    55,    56,    57,   147,   148,    60,     0,    61,    62,
      63,     0,     0,     0,     0,     0,     0,     0,    67,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   151,    76,    77,    78,
      79,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,    87,     0,     0,     0,    88,     5,     6,     7,     8,
       9,    89,     0,     0,     0,    10,    92,    93,     0,    94,
      95,     0,    96,    97,     0,    98,    99,     0,     0,   734,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,     0,    50,    51,     0,    52,     0,    54,    55,
      56,    57,   147,   148,    60,     0,    61,    62,    63,     0,
       0,     0,     0,     0,     0,     0,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   151,    76,    77,    78,    79,    80,
       0,    81,    82,     0,    83,    84,    85,     0,     0,    87,
       0,     0,     0,    88,     5,     6,     7,     8,     9,    89,
       0,     0,     0,    10,    92,    93,     0,    94,    95,     0,
      96,    97,     0,    98,    99,     0,     0,  1093,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
     147,   148,    60,     0,    61,    62,    63,     0,     0,     0,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   151,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     5,     6,     7,     8,     9,    89,     0,     0,
       0,    10,    92,    93,     0,    94,    95,     0,    96,    97,
       0,    98,    99,     0,     0,  1219,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,     0,    50,
      51,     0,    52,     0,    54,    55,    56,    57,   147,   148,
      60,     0,    61,    62,    63,     0,     0,     0,     0,     0,
       0,     0,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     151,    76,    77,    78,    79,    80,     0,    81,    82,     0,
      83,    84,    85,     0,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,     0,     0,     0,    10,
      92,    93,     0,    94,    95,     0,    96,    97,     0,    98,
      99,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,     0,   325,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,     0,    43,     0,     0,    44,     0,
       0,     0,    45,    46,    47,    48,     0,    50,    51,     0,
      52,     0,    54,    55,    56,    57,   147,   148,    60,     0,
      61,    62,    63,     0,     0,     0,     0,     0,     0,     0,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   151,    76,
      77,    78,    79,    80,     0,    81,    82,     0,    83,    84,
      85,     0,     0,    87,     0,     0,     0,    88,     5,     6,
       7,     8,     9,    89,     0,     0,     0,    10,    92,    93,
       0,    94,    95,     0,    96,    97,     0,    98,    99,  -707,
    -707,  -707,  -707,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,     0,   325,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,    35,     0,
       0,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    55,    56,    57,   147,   148,   149,     0,     0,    62,
      63,     0,     0,     0,     0,     0,     0,     0,   150,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   151,    76,    77,    78,
       0,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,    87,     0,     0,     0,    88,     5,     6,     7,     8,
       9,    89,     0,     0,     0,    10,    92,    93,     0,   210,
       0,     0,    96,    97,     0,    98,    99,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,    34,    35,     0,     0,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    55,
      56,    57,   147,   148,   149,     0,     0,    62,    63,     0,
       0,     0,     0,     0,     0,     0,   150,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   151,    76,    77,    78,     0,    80,
       0,    81,    82,     0,    83,    84,    85,     0,     0,    87,
       0,     0,     0,    88,     5,     6,     7,     8,     9,    89,
       0,     0,     0,    10,    92,    93,     0,   213,     0,     0,
      96,    97,     0,    98,    99,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     147,   148,   149,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   150,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   151,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     5,     6,     7,     8,     9,    89,     0,     0,
       0,    10,    92,    93,     0,   226,     0,     0,    96,    97,
       0,    98,    99,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   262,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    55,    56,    57,   147,   148,
     149,     0,     0,    62,    63,     0,     0,     0,     0,     0,
       0,     0,   150,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     151,    76,    77,    78,     0,    80,     0,    81,    82,     0,
      83,    84,    85,     0,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,     0,     0,     0,    10,
      92,    93,     0,     0,     0,     0,    96,    97,     0,    98,
      99,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,     0,     0,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    55,    56,    57,   147,   148,   149,     0,
       0,    62,    63,     0,     0,     0,     0,     0,     0,     0,
     150,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   151,    76,
      77,    78,     0,    80,     0,    81,    82,     0,    83,    84,
      85,     0,     0,    87,     0,     0,     0,    88,     5,     6,
       7,     8,     9,    89,     0,     0,     0,    10,    92,    93,
     388,     0,     0,     0,    96,    97,     0,    98,    99,     0,
       0,     0,     0,     0,     0,     0,   480,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,    35,     0,
       0,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    55,    56,    57,   147,   148,   149,     0,     0,    62,
      63,     0,     0,     0,     0,     0,     0,     0,   150,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   151,    76,    77,    78,
       0,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,    87,     0,     0,     0,    88,     5,     6,     7,     8,
       9,    89,     0,     0,     0,    10,    92,    93,     0,     0,
       0,     0,    96,    97,     0,    98,    99,     0,     0,   492,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,    34,    35,     0,     0,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    55,
      56,    57,   147,   148,   149,     0,     0,    62,    63,     0,
       0,     0,     0,     0,     0,     0,   150,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   151,    76,    77,    78,     0,    80,
       0,    81,    82,     0,    83,    84,    85,     0,     0,    87,
       0,     0,     0,    88,     5,     6,     7,     8,     9,    89,
       0,     0,     0,    10,    92,    93,     0,     0,     0,     0,
      96,    97,     0,    98,    99,     0,     0,     0,     0,     0,
       0,     0,   529,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     147,   148,   149,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   150,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   151,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     5,     6,     7,     8,     9,    89,     0,     0,
       0,    10,    92,    93,     0,     0,     0,     0,    96,    97,
       0,    98,    99,     0,     0,     0,     0,     0,     0,     0,
     561,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    55,    56,    57,   147,   148,
     149,     0,     0,    62,    63,     0,     0,     0,     0,     0,
       0,     0,   150,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     151,    76,    77,    78,     0,    80,     0,    81,    82,     0,
      83,    84,    85,     0,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,     0,     0,     0,    10,
      92,    93,     0,     0,     0,     0,    96,    97,     0,    98,
      99,     0,     0,     0,     0,     0,     0,     0,   563,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,     0,     0,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    55,    56,    57,   147,   148,   149,     0,
       0,    62,    63,     0,     0,     0,     0,     0,     0,     0,
     150,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   151,    76,
      77,    78,     0,    80,     0,    81,    82,     0,    83,    84,
      85,     0,     0,    87,     0,     0,     0,    88,     5,     6,
       7,     8,     9,    89,     0,     0,     0,    10,    92,    93,
       0,     0,     0,     0,    96,    97,     0,    98,    99,     0,
       0,     0,     0,     0,     0,     0,   796,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,    35,     0,
       0,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    55,    56,    57,   147,   148,   149,     0,     0,    62,
      63,     0,     0,     0,     0,     0,     0,     0,   150,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   151,    76,    77,    78,
       0,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,    87,     0,     0,     0,    88,     5,     6,     7,     8,
       9,    89,     0,     0,     0,    10,    92,    93,     0,     0,
       0,     0,    96,    97,     0,    98,    99,     0,     0,     0,
       0,     0,     0,     0,   838,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,    34,    35,     0,     0,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    55,
      56,    57,   147,   148,   149,     0,     0,    62,    63,     0,
       0,     0,     0,     0,     0,     0,   150,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   151,    76,    77,    78,     0,    80,
       0,    81,    82,     0,    83,    84,    85,     0,     0,    87,
       0,     0,     0,    88,     5,     6,     7,     8,     9,    89,
       0,     0,     0,    10,    92,    93,     0,     0,     0,     0,
      96,    97,     0,    98,    99,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     147,   148,   149,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   150,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   151,    76,    77,    78,   530,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     5,     6,     7,     8,     9,    89,     0,     0,
       0,    10,    92,    93,     0,     0,     0,     0,    96,    97,
       0,    98,    99,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    55,    56,    57,   147,   148,
     149,     0,     0,    62,    63,     0,     0,     0,     0,     0,
       0,     0,   150,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     151,    76,    77,    78,     0,    80,     0,    81,    82,     0,
      83,    84,    85,     0,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,     0,     0,     0,    10,
      92,    93,     0,     0,     0,     0,    96,    97,     0,    98,
      99,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,   443,
      35,     0,     0,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    55,    56,    57,   147,   148,   149,     0,
       0,    62,    63,     0,     0,     0,     0,     0,     0,     0,
     150,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   151,    76,
      77,    78,     0,    80,     0,    81,    82,     0,    83,    84,
      85,     0,     0,    87,     0,     0,     0,    88,  1110,  1111,
    1112,  1113,  1114,    89,  1115,  1116,  1117,  1118,    92,    93,
       0,     0,     0,     0,    96,    97,     0,    98,    99,   303,
       0,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,     0,   325,  1119,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1120,  1121,  1122,
    1123,  1124,  1125,  1126,     0,     0,    33,     0,     0,     0,
       0,     0,     0,     0,     0,  1127,  1128,  1129,  1130,  1131,
    1132,  1133,  1134,  1135,  1136,  1137,  1138,  1139,  1140,  1141,
    1142,  1143,  1144,  1145,  1146,  1147,  1148,  1149,  1150,  1151,
    1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,
    1162,  1163,  1164,  1165,  1166,  1167,     0,     0,  1168,  1169,
    1170,  1171,  1172,  1173,  1174,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1175,  1176,  1177,     0,
    1178,     0,     0,    81,    82,     0,    83,    84,    85,  1179,
    1180,  1181,     0,     0,  1182,   300,   301,   302,     0,     0,
       0,  1183,  1184,     0,  1185,     0,  1186,     0,     0,     0,
       0,   303,     0,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,     0,   325,   300,   301,   302,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   303,     0,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,     0,   325,   300,   301,   302,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   303,     0,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,     0,   325,   300,   301,
     302,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   303,     0,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,     0,   325,   300,
     301,   302,   818,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   303,     0,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,     0,   325,
     300,   301,   302,   840,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   303,   952,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,     0,
     325,   300,   301,   302,   994,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   303,     0,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
       0,   325,   300,   301,   302,  1069,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   303,     0,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,     0,   325,   300,   301,   302,  1070,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   303,
       0,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,     0,   325,     0,   953,     0,   300,   301,   302,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   303,     0,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   326,   325,   300,   301,
     302,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   303,     0,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   402,   325,   300,
     301,   302,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   303,     0,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   404,   325,
     300,   301,   302,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   303,     0,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,     0,
     325,     0,   414,     0,   300,   301,   302,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     303,     0,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   416,   325,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   300,   301,   302,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   303,   553,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,     0,   325,   300,   301,   302,
       0,     0,     0,     0,     0,     0,     0,   374,     0,     0,
       0,     0,     0,   303,   572,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,     0,   325,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   300,   301,   302,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   303,   454,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,     0,   325,   300,   301,   302,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   303,
     677,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,     0,   325,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   300,   301,
     302,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   303,   715,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,     0,   325,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   916,     0,     0,     0,
       0,     0,     0,     0,   300,   301,   302,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   581,
     303,   674,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,     0,   325,   300,   301,   302,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   303,     0,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,     0,   325,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,     0,   325
};

static const yytype_int16 yycheck[] =
{
       4,    86,   114,   153,     4,    90,    91,     4,     4,     4,
     362,     4,    30,    28,     4,   228,   287,   801,   357,   554,
     215,   216,    40,   337,   909,   220,    44,   325,   113,   886,
     189,   284,   195,   136,    24,    25,   893,   668,  1003,   195,
    1005,   650,    46,   382,    93,    49,   986,   450,   580,     8,
    1000,     8,     8,   143,   785,    53,     8,     8,     8,     8,
       8,     8,    66,     8,     8,     8,    26,     8,     8,     8,
       8,    41,     8,    61,     8,   671,     8,    75,     8,     8,
      78,     8,    86,     8,    29,     8,    90,    91,   684,   284,
      73,    73,     8,   196,    78,   147,    61,    73,    73,   121,
      31,     8,   328,    93,   368,     0,   147,    93,    94,   113,
     174,   175,   162,   586,   172,    26,   172,   114,    31,   160,
    1085,   102,   172,   175,   172,    78,    41,   176,   354,   986,
    1237,  1238,   358,   121,   175,   172,   133,    31,    31,   188,
      24,    25,    73,   172,    28,   295,   172,  1004,   175,    31,
     759,  1008,  1009,   137,   176,    41,   804,   140,   162,   807,
      73,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,   804,    46,   140,   807,   390,   159,    73,
      73,    61,    61,   914,   137,   335,   176,   157,   919,   920,
     176,    73,   182,    61,    61,   177,   460,   201,   188,    61,
     204,   177,   177,   173,   208,   290,   201,    59,    60,   204,
     175,   201,   407,   163,   204,   175,   173,   176,   174,   175,
     816,   225,   174,   174,   833,   174,   174,   174,   173,  1086,
     174,   174,   230,   174,   174,   174,   174,   640,   174,   173,
     330,   173,   157,   173,   173,   235,   173,   163,   173,   334,
     173,   336,   242,   243,   244,   886,   163,   521,   173,   249,
     258,   351,   893,   174,   737,   255,   739,   462,   463,   121,
     274,   157,    73,   921,   146,   627,    61,   274,   909,   283,
    1011,    61,   372,   287,   274,    90,   290,   173,    90,   161,
     921,   381,   176,   172,   384,   175,   175,   121,   182,   834,
      61,    61,   465,    61,   188,   172,    46,   175,   175,   465,
     325,    31,    61,   175,    61,   330,   848,    61,    61,   117,
     369,   173,   121,    61,    61,   144,   330,   591,    25,   333,
     334,   117,   336,   121,    66,    67,   351,   138,   139,   144,
     593,   121,   144,   682,   172,    42,   172,   351,    45,   173,
     172,   235,   691,    73,   963,   174,   175,   372,   242,   243,
     244,   121,  1302,   522,   577,   249,   381,  1332,   372,   384,
     146,   255,   362,  1004,   173,    93,    94,  1008,  1009,   369,
     384,    66,    67,  1348,    73,   173,   545,    25,  1338,   687,
     175,   395,   927,    71,   929,   175,   559,   172,   593,   594,
     395,   560,   565,   559,  1354,   395,   146,  1233,  1234,   565,
    1019,   415,   140,    51,   175,   175,   420,   175,   415,    71,
     418,  1306,    73,   175,   172,   415,   175,    78,   175,   433,
     172,   175,   175,    71,   162,   439,   440,   175,   175,  1324,
      26,   325,   527,   602,   172,  1302,   710,   175,   137,   138,
     139,    13,    73,   172,   144,  1086,   172,    78,   176,    31,
     655,  1346,   565,   174,    26,   779,   661,   174,   174,  1253,
     148,   149,   811,   151,   152,   153,    61,    61,   362,    73,
     819,    43,  1017,    61,    78,   369,   124,   138,   139,   141,
     480,   755,   144,   175,   172,   140,   148,   149,   850,   151,
     152,   153,   172,   141,   172,   140,   144,    61,   146,   140,
     148,   149,   172,   151,   152,   153,   412,   138,   139,   157,
     144,   734,    71,   527,   176,    40,    49,   162,   166,   425,
     181,   106,   107,   108,   172,   162,   888,    25,  1322,   529,
     175,    71,   140,    73,   138,   139,   741,   121,   743,    71,
     554,    73,   556,  1088,    42,     8,   179,    45,   576,   180,
      45,    46,    47,   140,    49,   829,   656,   906,   172,   573,
     140,   561,   174,   563,   121,   770,   573,   172,   174,   583,
     584,   162,   157,   573,    71,   924,    73,    13,    13,   584,
      25,   581,   141,    13,   584,   144,   480,   146,   173,   148,
     149,   162,   151,   152,   153,   173,    79,   611,   803,    13,
     874,    92,    93,    94,    90,   879,    51,   173,   148,   149,
     173,   151,   152,   153,   111,   173,   148,   149,   618,   151,
     152,   153,   178,   172,   793,   639,    71,   627,   628,   639,
     172,   656,   639,   639,   639,   529,   639,   177,   172,   639,
       8,    82,   656,   175,   141,   177,   995,   144,     8,   146,
     173,   148,   149,    98,   151,   152,   153,   174,    71,   864,
      73,   935,   687,    13,    79,  1306,   172,   561,     8,   563,
      92,    93,    94,  1218,    73,   172,   582,    73,   175,   124,
     177,   174,    78,  1324,   698,   699,   700,   581,   119,   595,
     596,   172,    61,   974,   722,   173,   141,   120,   122,   144,
     161,   146,     8,   148,   149,  1346,   151,   152,   153,   723,
      61,   173,   726,   106,   107,   108,   939,    13,   723,   726,
     173,   166,  1071,   723,   618,    73,   726,   172,   728,    13,
      78,   954,     8,   627,   628,   148,   149,   119,   151,   152,
     153,   137,   138,   139,   758,   172,   760,   179,   971,    69,
      70,    71,     8,   958,   179,   176,    41,   771,    97,    98,
      99,   667,   175,   172,   177,   172,   174,   972,   179,   783,
      73,   173,   173,   783,   179,    78,   783,   783,   783,   984,
     783,  1062,   174,   783,   106,   107,   108,   109,   110,   111,
     138,   139,   997,   687,    73,  1000,   796,   122,  1021,    78,
       8,   173,   172,   140,   140,    71,  1029,    73,   172,   175,
      41,    25,   102,   841,    99,    13,   830,   174,    90,   104,
     834,   106,   107,   108,   109,   110,   111,   112,   148,   149,
     844,   151,   152,   153,   728,   138,   139,    51,   838,   844,
     854,   175,    13,   175,   844,    13,   179,   854,   174,   172,
     850,   851,   174,   172,   854,   761,   172,    71,   137,   138,
     139,   767,   984,   148,   149,    13,   151,   172,    99,     8,
    1093,   172,   174,   104,   173,   106,   107,   108,   109,   110,
     111,   112,   148,   149,    98,   151,   152,   153,   888,   172,
     122,   176,   106,   107,   108,   109,   110,   111,    13,   174,
    1060,     8,   796,   173,   173,   122,    71,     8,   179,   175,
     124,   177,   137,   927,   172,   929,   159,   148,   149,     8,
     151,   172,   252,   122,   174,   172,   256,   141,   173,     8,
     144,   175,   146,   173,   148,   149,   175,   151,   152,   153,
     175,   122,   172,  1065,   838,   176,   276,   853,   278,   279,
     280,   281,   166,   172,   137,   173,   850,   851,   172,   179,
     974,   867,     8,   869,    26,   979,    68,   174,   173,   163,
      24,    25,    71,   174,    28,    26,   983,   173,   122,   144,
       8,   173,   122,   148,   149,  1208,   151,   152,   153,   176,
    1213,     8,    90,   175,   888,    49,  1219,   176,   172,    26,
      41,   173,  1225,  1017,   173,   175,   173,   173,  1022,   173,
     175,   122,  1026,   173,     8,  1022,  1030,   174,    26,   173,
      73,  1026,  1022,  1030,  1038,   174,  1026,   174,   122,   150,
    1030,  1038,    41,    98,   146,  1049,  1259,  1260,  1038,    71,
     172,  1055,   173,   104,  1204,    13,   173,   146,  1062,   148,
     149,   150,   151,   152,   153,    73,   962,  1064,    99,   173,
     966,    13,   968,   104,   122,   106,   107,   108,   109,   110,
     111,   112,   122,   172,  1088,   122,   173,  1091,  1092,   175,
     173,    73,  1096,   174,  1091,   173,   172,  1092,   175,  1096,
      99,  1091,  1092,    13,    71,   104,  1096,   106,   107,   108,
     109,   110,   111,   112,   175,   175,  1201,   148,   149,   122,
     151,    13,   144,    13,   173,    73,   148,   149,    13,   151,
     152,   153,   176,   175,    51,    71,  1032,   175,   182,  1352,
      73,    73,   172,  1338,   188,   176,    13,     8,   174,   148,
     149,    90,   151,   140,    98,    90,   153,  1370,    29,  1354,
      13,   172,   174,    25,  1249,   109,   110,   173,    73,   155,
       8,   215,   216,   117,   118,    41,   220,   176,   173,   115,
     172,   148,   149,  1079,   151,   152,   153,    73,   173,    51,
      73,   235,    13,   174,   173,    13,    73,  1201,   242,   243,
     244,   175,    13,  1099,  1100,   249,  1318,  1103,    13,    71,
     154,   255,   148,   149,  1218,   151,   152,   153,  1222,   418,
    1315,   336,   334,   333,  1228,  1222,   133,   686,   683,  1233,
    1234,  1329,  1222,  1237,  1238,   642,   172,   851,   951,    71,
     284,    73,    74,   287,  1324,  1249,   979,   423,  1108,    71,
    1254,  1357,  1333,  1190,  1258,    38,  1202,  1254,  1262,    93,
    1264,  1258,   124,   756,  1254,  1262,  1082,  1264,  1258,   786,
     699,   830,  1262,   817,  1264,    41,   712,   250,   243,   141,
     744,   325,   144,    -1,   146,    -1,   148,   149,    -1,   151,
     152,   153,  1296,  1297,    -1,   157,    -1,  1301,    -1,  1296,
    1297,    -1,    -1,    -1,   166,    -1,  1296,  1297,    -1,    -1,
     172,    -1,    -1,  1317,    -1,  1211,   148,   149,   362,   151,
     152,   153,    -1,    -1,   146,   369,   148,   149,    -1,   151,
     152,   153,    -1,    99,    -1,    -1,  1232,    -1,   104,    -1,
     106,   107,   108,   109,   110,   111,   112,  1243,    -1,    -1,
      -1,    -1,  1248,    -1,    -1,  1359,    -1,    -1,    -1,    -1,
    1256,    -1,  1359,   407,    -1,    -1,    -1,    -1,   412,  1359,
    1374,   106,   107,   108,   109,   110,   111,  1374,    98,    -1,
      -1,   425,   148,   149,  1374,   151,   106,   107,   108,   109,
     110,   111,    -1,    -1,    -1,    -1,    -1,  1293,    -1,    -1,
      -1,    -1,    24,    25,    -1,    -1,    28,    -1,    -1,  1305,
     176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   462,   463,
      -1,     9,    10,    11,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,    -1,    -1,   480,    25,    -1,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      41,    49,    -1,    -1,    -1,    -1,  1362,    -1,     9,    10,
      11,    -1,  1368,    -1,    -1,    -1,  1372,    -1,    -1,  1375,
      -1,    -1,    -1,    -1,    25,   529,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   561,    99,   563,
      -1,    -1,    -1,   104,    -1,   106,   107,   108,   109,   110,
     111,   112,    -1,    -1,    -1,    -1,    -1,   581,   582,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   593,
     594,   595,   596,   597,   176,    -1,    -1,    -1,    -1,    -1,
     182,    -1,    -1,    -1,    -1,    -1,   188,   148,   149,    -1,
     151,    -1,    -1,    -1,   618,    -1,    -1,    -1,    -1,    -1,
      -1,   625,    -1,   627,   628,    -1,    -1,    -1,    -1,    -1,
      -1,   179,    25,    63,    -1,   176,    -1,    -1,   642,    -1,
      -1,    71,    -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   655,    -1,   235,    49,    -1,    -1,   661,    51,    -1,
     242,   243,   244,   667,    -1,    -1,   670,   249,     9,    10,
      11,    -1,    -1,   255,    -1,   176,    -1,    -1,    71,    -1,
      -1,   111,    -1,   687,    25,    31,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,   141,    -1,    -1,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,   728,    71,    -1,    73,    -1,    -1,
      -1,   124,    -1,    -1,    -1,    -1,    -1,   741,    -1,   743,
      -1,    -1,   172,   325,    -1,    -1,    -1,   177,   141,    -1,
      -1,   144,   756,   146,    -1,   148,   149,   761,   151,   152,
     153,    -1,    -1,   767,    -1,   111,   770,    -1,    -1,    -1,
      -1,    -1,    -1,   166,    -1,    -1,    -1,   123,    -1,   172,
     362,    69,    70,    -1,    -1,    -1,    -1,   369,    -1,    -1,
      -1,    79,   796,    -1,    -1,   141,    -1,    -1,   144,   803,
     146,    -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     215,   216,    -1,    -1,    -1,   220,   172,    -1,    -1,    -1,
     412,   177,    -1,    -1,   838,   176,    -1,   125,   126,   127,
     128,   129,    -1,   425,    -1,    -1,   850,   851,   136,   853,
      62,    63,    -1,    -1,   142,   143,    -1,    -1,    -1,    71,
     864,    73,    -1,   867,    -1,   869,    -1,    -1,   156,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   880,    -1,    -1,    -1,
      -1,   885,    -1,   171,   888,     9,    10,    11,    -1,   284,
      -1,    -1,   287,    -1,    -1,    -1,    -1,    -1,   480,   111,
      -1,    25,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,   141,
      -1,    -1,   144,    -1,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,    -1,    -1,    -1,    -1,   529,   160,    -1,
      -1,    -1,    -1,    -1,   958,    42,    43,    -1,   962,    -1,
     172,    -1,   966,    -1,   968,   177,    -1,    -1,   972,    -1,
     974,   975,    -1,   977,    61,    -1,    -1,   412,    -1,   561,
     984,   563,    69,    70,    71,    -1,    -1,    -1,    -1,    -1,
     425,    -1,    79,   997,    -1,    -1,  1000,    -1,    -1,   581,
     582,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   412,    -1,
      -1,    -1,   407,   595,   596,   597,    -1,   412,    -1,    -1,
      -1,   425,    -1,    -1,    -1,    -1,    -1,    -1,  1032,    -1,
     425,    -1,    -1,    -1,    -1,    -1,   618,   124,   125,   126,
     127,   128,   129,   625,    -1,   627,   628,    -1,    -1,   136,
      -1,    -1,   176,    -1,   141,   142,   143,   144,  1062,   146,
     642,   148,   149,    -1,   151,   152,   153,   462,   463,   156,
      -1,    -1,    -1,    -1,    -1,  1079,    -1,    -1,    -1,   166,
      -1,    -1,    -1,    -1,   171,   667,    -1,    -1,   670,    -1,
       9,    10,    11,    -1,    -1,  1099,  1100,    -1,    -1,  1103,
    1104,    -1,    -1,    -1,  1108,   687,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    -1,    -1,    71,    -1,    73,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,   728,   582,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    61,
     595,   596,   597,    -1,    -1,    -1,    -1,    69,    70,    71,
      -1,    -1,    -1,    -1,   756,   111,    -1,    79,   582,   761,
      -1,    -1,    -1,    -1,    -1,   767,    -1,   582,    -1,    -1,
      -1,   595,   596,   597,    -1,    -1,    -1,    -1,   593,   594,
     595,   596,   597,    -1,    -1,   141,    -1,  1211,   144,    -1,
     146,    -1,   148,   149,   796,   151,   152,   153,    -1,    -1,
      -1,    -1,   124,    -1,  1228,    -1,    -1,    -1,  1232,    -1,
      -1,    -1,   667,    -1,   136,    -1,   172,    -1,    -1,  1243,
      -1,   177,    -1,    -1,  1248,    -1,   148,   149,    -1,   151,
     152,   153,  1256,    -1,    -1,    -1,   838,   176,    -1,    -1,
     655,    -1,    -1,   667,   166,    -1,   661,    -1,   850,   851,
      -1,   853,   667,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,   867,    -1,   869,    -1,  1293,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   880,    -1,
      -1,  1305,    -1,   885,    -1,    -1,   888,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    59,
      60,   756,    -1,    -1,    -1,  1329,   761,    -1,    -1,    -1,
      -1,    -1,   767,    -1,  1338,    -1,    -1,  1341,    -1,    71,
      -1,    73,    -1,    -1,    -1,    -1,   741,    -1,   743,    -1,
    1354,    -1,   756,    59,    60,    -1,    -1,   761,  1362,    -1,
      -1,   756,    -1,   767,  1368,    -1,   761,    -1,  1372,    -1,
      -1,  1375,   767,    -1,    -1,   770,    -1,    -1,    -1,   111,
     962,   121,    -1,    -1,   966,    -1,   968,    -1,    -1,    -1,
      -1,   123,    -1,   975,    -1,   977,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    -1,    73,    -1,    -1,   803,   141,
      -1,    -1,   144,    -1,   146,   121,   148,   149,   853,   151,
     152,   153,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   867,    -1,   869,    -1,    -1,    -1,    -1,    -1,
     172,    -1,    -1,   111,    -1,   177,    -1,    -1,    -1,   853,
    1032,    -1,    -1,    -1,    -1,   123,    -1,    -1,   853,    71,
      -1,    73,    -1,   867,    -1,   869,    -1,    -1,    -1,   864,
      -1,    -1,   867,   141,   869,    71,   144,    -1,   146,    -1,
     148,   149,    -1,   151,   152,   153,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1079,    -1,   111,
      -1,    -1,    -1,    -1,   172,    -1,    -1,    -1,    -1,   177,
      -1,    -1,    -1,    -1,    -1,    -1,   112,  1099,  1100,    -1,
      -1,  1103,  1104,    -1,    -1,    -1,  1108,   962,   124,   141,
      -1,   966,   144,   968,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,    -1,    -1,   141,    -1,    -1,   144,    -1,
     146,    -1,   148,   149,    -1,   151,   152,   153,   962,    -1,
     172,    -1,   966,   958,   968,   177,    -1,   962,    -1,    -1,
      -1,   966,    -1,   968,    -1,    -1,    -1,   972,    -1,   974,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   984,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1032,    -1,    -1,
      -1,    -1,   997,    -1,    -1,  1000,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,  1032,  1211,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1032,    -1,    -1,
      -1,    -1,    -1,    -1,  1079,    -1,    -1,    -1,    -1,    -1,
    1232,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1243,    -1,    -1,  1099,  1100,  1248,  1062,  1103,    -1,
      -1,    -1,  1107,    -1,  1256,  1079,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1079,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1099,  1100,    -1,    -1,  1103,
      -1,    -1,    -1,    -1,  1099,  1100,    -1,    -1,  1103,    -1,
      -1,  1293,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     5,     6,  1305,     8,     9,    10,    -1,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      -1,    -1,    26,    27,    -1,    -1,    -1,  1329,    -1,    -1,
      -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,  1341,
      -1,    45,    -1,    47,    -1,    -1,    50,    -1,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1211,    -1,    -1,    -1,
    1362,    -1,    -1,    -1,    -1,    -1,  1368,    -1,    -1,    -1,
    1372,    -1,    -1,  1375,    -1,    79,    -1,  1232,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1211,  1243,    93,
      -1,    -1,    -1,  1248,    -1,    -1,  1211,    -1,    -1,    -1,
      -1,  1256,    -1,    -1,    -1,    -1,    -1,    -1,  1232,    -1,
      -1,    -1,    -1,  1228,    -1,    -1,  1271,  1232,    -1,  1243,
      -1,    -1,    -1,    -1,  1248,    -1,    -1,    -1,  1243,    -1,
      -1,    -1,  1256,  1248,    -1,    -1,    -1,    -1,  1293,    -1,
      -1,  1256,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1305,    -1,    -1,   157,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,  1293,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1293,    -1,
      -1,  1305,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1305,    -1,    -1,   197,    -1,    -1,   200,    -1,    -1,    -1,
      -1,    -1,   206,   207,    -1,    -1,    -1,  1362,    -1,    -1,
      -1,    -1,    -1,  1368,    -1,    -1,    -1,  1372,    -1,    -1,
    1375,    -1,    -1,  1338,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1362,  1354,
      -1,    -1,    -1,    -1,  1368,    -1,   250,  1362,  1372,    -1,
     254,  1375,    -1,  1368,    -1,    -1,    -1,  1372,    -1,    -1,
    1375,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   275,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   286,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,    -1,    -1,   327,   328,   329,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,    -1,    -1,    -1,    -1,    -1,
     354,   355,    -1,   357,   358,   359,    -1,    -1,    -1,    -1,
     364,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     374,    -1,   376,    -1,    -1,    -1,    -1,    -1,   382,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   392,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,     3,
       4,     5,     6,     7,    -1,    -1,    -1,   421,    12,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    31,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,
     454,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,   492,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   106,   107,   108,   109,   110,   111,    -1,    -1,
     114,   115,    -1,    -1,    -1,    -1,   530,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
     544,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,   146,    -1,   148,   149,    -1,   151,   152,   153,
      -1,    -1,   156,    -1,    -1,   569,   160,    -1,    -1,    -1,
      -1,    -1,   166,    -1,    -1,    -1,   580,   171,   172,    -1,
      -1,    -1,    -1,   177,   178,    -1,   180,   181,    -1,    -1,
      -1,    -1,    -1,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,   636,    49,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   649,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   665,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     674,    -1,    -1,   677,    -1,   679,    -1,    -1,   682,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   691,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   715,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
     176,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   779,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,   792,    -1,
      -1,   106,   107,   108,   109,   110,   111,    -1,    -1,   114,
     115,    -1,    -1,    -1,    -1,    -1,    -1,   811,   123,   124,
     125,   126,   127,   128,   129,   819,    -1,   821,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,   156,    -1,    -1,   848,   160,    -1,    -1,    -1,    -1,
      -1,   166,    -1,   857,    10,    11,   171,   172,    -1,    -1,
      -1,    -1,   177,   178,    -1,   180,   181,    -1,    -1,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,   896,    49,    -1,    -1,   900,    -1,   902,    -1,
      -1,    -1,   906,    -1,    -1,     3,     4,     5,     6,     7,
      -1,    -1,   916,    -1,    12,    -1,    -1,    -1,    -1,    -1,
     924,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    -1,    86,    -1,
      88,   995,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    99,   100,   101,    -1,   103,   104,   105,   106,   107,
     108,   109,   110,   111,    -1,   113,   114,   115,   116,   117,
     118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
     148,   149,    -1,   151,   152,   153,   154,    -1,   156,    -1,
      -1,    -1,   160,    -1,    -1,    -1,    -1,  1071,   166,   167,
      -1,   169,    -1,   171,   172,    -1,   174,   175,   176,   177,
     178,    11,   180,   181,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    12,    -1,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    -1,    86,    -1,    88,
      -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,
      99,   100,   101,    -1,   103,   104,   105,   106,   107,   108,
     109,   110,   111,    -1,   113,   114,   115,   116,   117,   118,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,   167,    -1,
     169,    12,   171,   172,    -1,   174,   175,   176,   177,   178,
      -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,
      81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,
      91,    -1,    -1,    -1,    95,    96,    97,    98,    99,   100,
     101,    -1,   103,   104,   105,   106,   107,   108,   109,   110,
     111,    -1,   113,   114,   115,   116,   117,   118,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,   167,    -1,   169,    12,
     171,   172,    -1,   174,   175,    -1,   177,   178,    -1,   180,
     181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    -1,   100,   101,    -1,
     103,    -1,   105,   106,   107,   108,   109,   110,   111,    -1,
     113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,   154,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,   172,
      -1,   174,   175,   176,   177,   178,    -1,   180,   181,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,
      -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,
      95,    96,    97,    98,    -1,   100,   101,    -1,   103,    -1,
     105,   106,   107,   108,   109,   110,   111,    -1,   113,   114,
     115,    -1,   117,   118,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,   146,    -1,   148,   149,    -1,   151,   152,   153,   154,
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,   174,
     175,   176,   177,   178,    -1,   180,   181,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    75,    -1,
      -1,    -1,    79,    80,    81,    82,    83,    84,    -1,    86,
      -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,
      97,    98,    -1,   100,   101,    -1,   103,    -1,   105,   106,
     107,   108,   109,   110,   111,    -1,   113,   114,   115,    -1,
     117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,   148,   149,    -1,   151,   152,   153,   154,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,    -1,    12,   171,   172,    -1,   174,   175,    -1,
     177,   178,    -1,   180,   181,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    -1,    86,    -1,    88,
      89,    -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,
      -1,   100,   101,    -1,   103,    -1,   105,   106,   107,   108,
     109,   110,   111,    -1,   113,   114,   115,    -1,   117,   118,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
      -1,    12,   171,   172,    -1,   174,   175,    -1,   177,   178,
      -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,
      81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,
      91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,
     101,    -1,   103,    -1,   105,   106,   107,   108,   109,   110,
     111,    -1,   113,   114,   115,    -1,   117,   118,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,    -1,   174,   175,   176,   177,   178,    -1,   180,
     181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    87,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    -1,   100,   101,    -1,
     103,    -1,   105,   106,   107,   108,   109,   110,   111,    -1,
     113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,   154,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,   172,
      -1,   174,   175,    -1,   177,   178,    -1,   180,   181,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,
      -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,
      95,    96,    97,    98,    -1,   100,   101,    -1,   103,    -1,
     105,   106,   107,   108,   109,   110,   111,    -1,   113,   114,
     115,    -1,   117,   118,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,   146,    -1,   148,   149,    -1,   151,   152,   153,   154,
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,   174,
     175,   176,   177,   178,    -1,   180,   181,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    75,    -1,
      -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,    86,
      -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,
      97,    98,    -1,   100,   101,    -1,   103,    -1,   105,   106,
     107,   108,   109,   110,   111,    -1,   113,   114,   115,    -1,
     117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,   148,   149,    -1,   151,   152,   153,   154,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,    -1,    12,   171,   172,    -1,   174,   175,   176,
     177,   178,    -1,   180,   181,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    85,    86,    -1,    88,
      -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,
      -1,   100,   101,    -1,   103,    -1,   105,   106,   107,   108,
     109,   110,   111,    -1,   113,   114,   115,    -1,   117,   118,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
      -1,    12,   171,   172,    -1,   174,   175,    -1,   177,   178,
      -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,
      81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,
      91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,
     101,    -1,   103,    -1,   105,   106,   107,   108,   109,   110,
     111,    -1,   113,   114,   115,    -1,   117,   118,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,    -1,   174,   175,   176,   177,   178,    -1,   180,
     181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    -1,   100,   101,    -1,
     103,    -1,   105,   106,   107,   108,   109,   110,   111,    -1,
     113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,   154,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,   172,
      -1,   174,   175,   176,   177,   178,    -1,   180,   181,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,
      -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,
      95,    96,    97,    98,    -1,   100,   101,    -1,   103,    -1,
     105,   106,   107,   108,   109,   110,   111,    -1,   113,   114,
     115,    -1,   117,   118,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,   146,    -1,   148,   149,    -1,   151,   152,   153,   154,
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,   174,
     175,   176,   177,   178,    -1,   180,   181,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    75,    -1,
      -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,    86,
      -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,
      97,    98,    -1,   100,   101,    -1,   103,    -1,   105,   106,
     107,   108,   109,   110,   111,    -1,   113,   114,   115,    -1,
     117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,   148,   149,    -1,   151,   152,   153,   154,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,    -1,    12,   171,   172,    -1,   174,   175,   176,
     177,   178,    -1,   180,   181,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    -1,    86,    -1,    88,
      -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,
      -1,   100,   101,    -1,   103,    -1,   105,   106,   107,   108,
     109,   110,   111,    -1,   113,   114,   115,    -1,   117,   118,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
      -1,    12,   171,   172,    -1,   174,   175,   176,   177,   178,
      -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,
      81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,
      91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,
     101,    -1,   103,    -1,   105,   106,   107,   108,   109,   110,
     111,    -1,   113,   114,   115,    -1,   117,   118,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,    -1,   174,   175,    -1,   177,   178,    -1,   180,
     181,    -1,    -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    -1,   100,   101,    -1,
     103,    -1,   105,   106,   107,   108,   109,   110,   111,    -1,
     113,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,   172,
      -1,   174,   175,    -1,   177,   178,    -1,   180,   181,    -1,
      -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,
      -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,
      95,    96,    97,    98,    -1,   100,   101,    -1,   103,    -1,
     105,   106,   107,   108,   109,   110,   111,    -1,   113,   114,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,   174,
     175,    -1,   177,   178,    -1,   180,   181,    -1,    -1,    26,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    75,    -1,
      -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,    86,
      -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,
      97,    98,    -1,   100,   101,    -1,   103,    -1,   105,   106,
     107,   108,   109,   110,   111,    -1,   113,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,    -1,    12,   171,   172,    -1,   174,   175,    -1,
     177,   178,    -1,   180,   181,    -1,    -1,    26,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    -1,    86,    -1,    88,
      -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,
      -1,   100,   101,    -1,   103,    -1,   105,   106,   107,   108,
     109,   110,   111,    -1,   113,   114,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
      -1,    12,   171,   172,    -1,   174,   175,    -1,   177,   178,
      -1,   180,   181,    -1,    -1,    26,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,
      81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,
      91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,
     101,    -1,   103,    -1,   105,   106,   107,   108,   109,   110,
     111,    -1,   113,   114,   115,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,    -1,   174,   175,    -1,   177,   178,    -1,   180,
     181,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    -1,   100,   101,    -1,
     103,    -1,   105,   106,   107,   108,   109,   110,   111,    -1,
     113,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,   172,
      -1,   174,   175,    -1,   177,   178,    -1,   180,   181,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,   107,   108,   109,   110,   111,    -1,    -1,   114,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,   174,
      -1,    -1,   177,   178,    -1,   180,   181,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
     107,   108,   109,   110,   111,    -1,    -1,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,    -1,    12,   171,   172,    -1,   174,    -1,    -1,
     177,   178,    -1,   180,   181,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,
     109,   110,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
      -1,    12,   171,   172,    -1,   174,    -1,    -1,   177,   178,
      -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    95,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   106,   107,   108,   109,   110,
     111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,    -1,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,    -1,    -1,    -1,    -1,   177,   178,    -1,   180,
     181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,   107,   108,   109,   110,   111,    -1,
      -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,    -1,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,   172,
     173,    -1,    -1,    -1,   177,   178,    -1,   180,   181,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,   107,   108,   109,   110,   111,    -1,    -1,   114,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,    -1,
      -1,    -1,   177,   178,    -1,   180,   181,    -1,    -1,    26,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
     107,   108,   109,   110,   111,    -1,    -1,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,    -1,    12,   171,   172,    -1,    -1,    -1,    -1,
     177,   178,    -1,   180,   181,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,
     109,   110,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
      -1,    12,   171,   172,    -1,    -1,    -1,    -1,   177,   178,
      -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   106,   107,   108,   109,   110,
     111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,    -1,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,    -1,    -1,    -1,    -1,   177,   178,    -1,   180,
     181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,   107,   108,   109,   110,   111,    -1,
      -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,    -1,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,   172,
      -1,    -1,    -1,    -1,   177,   178,    -1,   180,   181,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,   107,   108,   109,   110,   111,    -1,    -1,   114,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,    -1,
      -1,    -1,   177,   178,    -1,   180,   181,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
     107,   108,   109,   110,   111,    -1,    -1,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,    -1,    12,   171,   172,    -1,    -1,    -1,    -1,
     177,   178,    -1,   180,   181,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,
     109,   110,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
      -1,    12,   171,   172,    -1,    -1,    -1,    -1,   177,   178,
      -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   106,   107,   108,   109,   110,
     111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,    -1,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,    -1,    -1,    -1,    -1,   177,   178,    -1,   180,
     181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,   107,   108,   109,   110,   111,    -1,
      -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,    -1,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,     9,    10,    11,    12,   171,   172,
      -1,    -1,    -1,    -1,   177,   178,    -1,   180,   181,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    49,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    63,    64,
      65,    66,    67,    68,    -1,    -1,    71,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,    -1,
     145,    -1,    -1,   148,   149,    -1,   151,   152,   153,   154,
     155,   156,    -1,    -1,   159,     9,    10,    11,    -1,    -1,
      -1,   166,   167,    -1,   169,    -1,   171,    -1,    -1,    -1,
      -1,    25,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    -1,    27,    28,    29,    30,    31,    32,
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
      10,    11,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
       9,    10,    11,   176,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,     9,    10,    11,   176,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,     9,    10,    11,   176,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,     9,    10,    11,   176,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,   174,    -1,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,   174,    49,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,   174,    49,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,   174,    49,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,   174,    -1,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,   174,    49,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,   173,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   122,    -1,    -1,
      -1,    -1,    -1,    25,   173,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,   122,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
     122,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,   122,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   122,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   183,   184,     0,   185,     3,     4,     5,     6,     7,
      12,    41,    42,    43,    48,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    69,    70,    71,    72,    73,    75,    79,    80,    81,
      82,    84,    86,    88,    91,    95,    96,    97,    98,    99,
     100,   101,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   113,   114,   115,   116,   117,   118,   123,   124,   125,
     126,   127,   128,   129,   136,   141,   142,   143,   144,   145,
     146,   148,   149,   151,   152,   153,   154,   156,   160,   166,
     167,   169,   171,   172,   174,   175,   177,   178,   180,   181,
     186,   189,   192,   193,   194,   195,   196,   197,   200,   211,
     212,   215,   220,   226,   280,   281,   286,   290,   291,   292,
     293,   294,   302,   303,   304,   306,   307,   310,   320,   321,
     322,   327,   330,   348,   353,   355,   356,   357,   358,   359,
     360,   361,   362,   364,   377,   379,   381,   109,   110,   111,
     123,   141,   189,   211,   293,   355,   293,   172,   293,   293,
     293,   346,   347,   293,   293,   293,   293,   293,   293,   293,
     293,   293,   293,   293,   293,   111,   172,   193,   321,   322,
     355,   355,    31,   293,   368,   369,   293,   111,   172,   193,
     321,   322,   323,   354,   360,   365,   366,   172,   287,   324,
     172,   287,   288,   293,   202,   287,   172,   172,   172,   287,
     174,   293,   189,   174,   293,    25,    51,   124,   146,   166,
     172,   189,   196,   382,   392,   393,   174,   293,   175,   293,
     144,   190,   191,   192,    73,   177,   252,   253,   117,   117,
      73,   254,   172,   172,   172,   172,   189,   224,   383,   172,
     172,    73,    78,   137,   138,   139,   374,   375,   144,   175,
     192,   192,    95,   293,   225,   383,   146,   172,   383,   383,
     286,   293,   294,   355,   198,   175,    78,   325,   374,    78,
     374,   374,    26,   144,   162,   384,   172,     8,   174,    31,
     210,   146,   223,   383,   111,   211,   281,   174,   174,   174,
       9,    10,    11,    25,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    49,   174,    61,    61,   175,
     140,   109,   110,   118,   154,   211,   226,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    59,
      60,   121,   350,   351,    61,   175,   352,   172,    61,   175,
     177,   361,   172,   210,    13,   293,    40,   189,   345,   172,
     286,   355,   140,   355,   122,   179,     8,   332,   286,   355,
     384,   140,   172,   326,   121,   350,   351,   352,   173,   293,
      26,   200,     8,   174,   200,   201,   288,   289,   293,   189,
     238,   204,   174,   174,   174,   393,   393,   162,   172,    98,
     385,   393,    13,   189,   174,   198,   174,   192,     8,   174,
      90,   175,   355,     8,   174,    13,     8,   174,   355,   378,
     378,   355,   173,   162,   218,   123,   355,   367,   368,    61,
     121,   137,   375,    72,   293,   355,    78,   137,   375,   192,
     188,   174,   175,   174,   122,   221,   311,   313,    79,   297,
     298,   300,    13,    90,   380,   173,   173,   173,   176,   199,
     200,   212,   215,   220,   293,   178,   180,   181,   189,   385,
      31,   250,   251,   293,   382,   172,   383,   216,   210,   293,
     293,   293,    26,   293,   293,   293,   293,   293,   293,   293,
     293,   293,   293,   293,   293,   293,   293,   293,   293,   293,
     293,   293,   293,   293,   293,   323,   293,   363,   363,   293,
     370,   371,   189,   360,   361,   224,   225,   210,   223,    31,
     145,   290,   293,   293,   293,   293,   293,   293,   293,   293,
     293,   293,   293,   293,   175,   189,   360,   363,   293,   250,
     363,   293,   367,   173,   172,   344,     8,   332,   286,   173,
     189,    31,   293,    31,   293,   173,   173,   360,   250,   175,
     189,   360,   173,   198,   242,   293,    82,    26,   200,   236,
     174,    90,    13,     8,   173,    26,   175,   239,   393,    79,
     389,   390,   391,   172,     8,    42,    43,    61,   124,   136,
     146,   166,   193,   194,   196,   305,   321,   327,   328,   329,
     176,    90,   191,   189,   293,   253,   328,    73,     8,   173,
     173,   173,   174,   189,   388,   119,   229,   172,     8,   173,
     173,    73,    74,   189,   376,   189,    61,   176,   176,   185,
     187,   293,   120,   228,   161,    46,   146,   161,   315,   122,
       8,   332,   173,   393,   393,    13,   121,   350,   351,   352,
     176,     8,   163,   355,   173,     8,   333,    13,   295,   213,
     119,   227,   172,   293,    26,   179,   179,   122,   176,     8,
     332,   384,   172,   219,   222,   383,   217,    63,   355,   293,
     384,   172,   179,   176,   173,   179,   176,   173,    42,    43,
      61,    69,    70,    79,   124,   136,   166,   189,   335,   337,
     340,   343,   189,   355,   355,   122,   350,   351,   352,   173,
     293,   243,    66,    67,   244,   287,   198,   289,    31,   123,
     233,   355,   328,   189,    26,   200,   237,   174,   240,   174,
     240,     8,   163,   122,     8,   332,   173,   157,   385,   386,
     393,   328,   328,   328,   331,   334,   172,    78,   140,   172,
     140,   175,   102,   207,   208,   189,   176,    13,   355,   174,
      90,     8,   163,   230,   321,   175,   367,   123,   355,    13,
     179,   293,   176,   185,   230,   175,   314,    13,   293,   297,
     174,   393,   175,   189,   360,   393,    31,   293,   328,   157,
     248,   249,   348,   349,   172,   321,   228,   296,   293,   293,
     293,   172,   250,   229,   228,   214,   227,   323,   176,   172,
     250,    13,    69,    70,   189,   336,   336,   337,   338,   339,
     172,    78,   137,   172,     8,   332,   173,   344,    31,   293,
     176,    66,    67,   245,   287,   200,   174,    83,   174,   355,
     172,   122,   232,    13,   198,   240,    92,    93,    94,   240,
     176,   393,   393,   389,     8,   173,   173,   122,   179,     8,
     332,   331,   189,   297,   299,   301,   189,   328,   372,   373,
     172,   159,   328,   393,   189,     8,   255,   173,   172,   290,
     293,   179,   176,   255,   147,   160,   175,   310,   317,   147,
     175,   316,   122,   174,   293,   384,   172,   355,   173,     8,
     333,   393,   394,   248,   175,   248,   122,   250,   173,   175,
     175,   172,   228,   326,   172,   250,   173,   122,   179,     8,
     332,   338,   137,   297,   341,   342,   337,   355,   287,    26,
      68,   200,   174,   289,   367,   233,   173,   328,    89,    92,
     174,   293,    26,   174,   241,   176,   163,   157,    26,   328,
     328,   173,   122,     8,   332,   173,   122,   176,     8,   332,
     321,   175,    90,   321,    99,   104,   112,   148,   149,   151,
     176,   256,   278,   279,   280,   285,   348,   367,   176,   176,
      46,   293,   293,   293,   176,   172,   250,    26,   387,   157,
     349,    31,    73,   173,   255,   173,   293,   173,   255,   255,
     248,   175,   250,   173,   337,   337,   173,   122,   173,     8,
     332,    26,   198,   174,   173,   173,   205,   174,   174,   241,
     198,   393,   122,   328,   297,   328,   328,    73,   198,   393,
     382,   231,   321,   112,   124,   146,   152,   265,   266,   267,
     321,   150,   271,   272,   115,   172,   189,   273,   274,   257,
     211,   393,     8,   174,   279,   280,   173,   146,   312,   176,
     176,   172,   250,   173,   393,   104,   308,   394,    73,    13,
     387,   176,   387,   176,   176,   173,   255,   173,   122,   337,
     297,   198,   203,    26,   200,   235,   198,   173,   328,   122,
     122,   173,   176,    13,     8,   174,   175,   175,     8,   174,
       3,     4,     5,     6,     7,     9,    10,    11,    12,    49,
      62,    63,    64,    65,    66,    67,    68,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   123,   124,
     125,   126,   127,   128,   129,   141,   142,   143,   145,   154,
     155,   156,   159,   166,   167,   169,   171,   189,   318,   319,
       8,   174,   146,   150,   189,   274,   275,   276,   174,    73,
     284,   210,   258,   382,   211,   250,   173,   172,   175,    31,
      73,    13,   328,   175,   308,   387,   176,   337,   122,    26,
     200,   234,   198,   328,   328,   175,   328,   321,   261,   268,
     327,   266,    13,    26,    43,   269,   272,     8,    29,   173,
      25,    42,    45,    13,     8,   174,   383,   284,    13,   210,
     173,    31,    73,   309,   198,    73,    13,   328,   198,   175,
     175,   337,   198,    87,   198,   176,   189,   196,   262,   263,
     264,     8,   176,   328,   319,   319,    51,   270,   275,   275,
      25,    42,    45,   328,    73,   172,   174,   328,   383,    73,
       8,   333,   176,    13,   328,   176,   198,   198,    85,   174,
     176,   140,    90,   327,   153,    13,   259,   172,    31,    73,
     173,   328,   176,   176,   174,   206,   189,   279,   280,   328,
     157,   246,   247,   349,   260,    73,   102,   207,   209,   155,
     189,   174,   173,     8,   333,   106,   107,   108,   282,   283,
     246,   172,   231,   174,   387,   157,   282,   394,   173,   321,
     174,   174,   175,   277,   349,    31,    73,   387,    73,   198,
     394,    73,    13,   277,   173,   176,    31,    73,    13,   328,
     175,    73,    13,   328,   198,    13,   328,   176,   328
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
#line 704 "hphp.y"
    { _p->initParseTree(); ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 704 "hphp.y"
    { _p->popLabelInfo();
                                                  _p->finiParseTree();;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 710 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 711 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 714 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 715 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 716 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 717 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 718 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 719 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 722 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 724 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 725 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 726 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 727 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 728 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 729 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 734 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 735 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 736 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 737 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 738 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 739 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 744 "hphp.y"
    { ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 745 "hphp.y"
    { ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 748 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 749 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 750 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 752 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 756 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 758 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 761 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 763 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 764 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 767 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 774 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 781 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 789 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 792 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 798 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 799 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 802 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 803 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 804 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 808 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 812 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 817 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 818 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 819 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 822 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 824 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 827 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 828 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 830 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 831 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 833 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 834 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 835 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 836 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 837 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 838 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 839 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 840 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 841 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 842 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 843 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 844 "hphp.y"
    { (yyval).reset(); (yyval) = ';';}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 845 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 848 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 849 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]));;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 852 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 860 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(3) - (14)]),(yyvsp[(7) - (14)]),(yyvsp[(8) - (14)]),(yyvsp[(11) - (14)]),(yyvsp[(13) - (14)]),(yyvsp[(14) - (14)]));;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 863 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 864 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 865 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval)); ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 869 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 870 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 871 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 872 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 873 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval)); ;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 885 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 886 "hphp.y"
    { (yyval).reset();;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 890 "hphp.y"
    { finally_statement(_p);;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 892 "hphp.y"
    { _p->onFinally((yyval), (yyvsp[(4) - (5)]));;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 897 "hphp.y"
    { (yyval).reset();;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 901 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 902 "hphp.y"
    { (yyval).reset();;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 906 "hphp.y"
    { _p->pushFuncLocation();;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 911 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 916 "hphp.y"
    { _p->onFunction((yyval),0,(yyvsp[(8) - (11)]),(yyvsp[(2) - (11)]),(yyvsp[(3) - (11)]),(yyvsp[(6) - (11)]),(yyvsp[(10) - (11)]),0,false);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 920 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 925 "hphp.y"
    { _p->onFunction((yyval),0,(yyvsp[(9) - (12)]),(yyvsp[(3) - (12)]),(yyvsp[(4) - (12)]),(yyvsp[(7) - (12)]),(yyvsp[(11) - (12)]),&(yyvsp[(1) - (12)]),false);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 935 "hphp.y"
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

  case 98:

/* Line 1455 of yacc.c  */
#line 950 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 953 "hphp.y"
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

  case 100:

/* Line 1455 of yacc.c  */
#line 967 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 970 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 978 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 985 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 988 "hphp.y"
    { Token t_ext, t_imp;
                                         t_ext.reset(); t_imp.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,t_imp,
                                                     (yyvsp[(6) - (7)]), 0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 996 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 999 "hphp.y"
    { Token t_ext, t_imp;
                                         t_ext.reset(); t_imp.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,t_imp,
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 1008 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 1015 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 1018 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 1019 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 1020 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 1024 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 1025 "hphp.y"
    { (yyval).reset();;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 1028 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 1029 "hphp.y"
    { (yyval).reset();;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 1032 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 1033 "hphp.y"
    { (yyval).reset();;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 1036 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 1038 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 1041 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1043 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1047 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1048 "hphp.y"
    { (yyval).reset();;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1051 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1052 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1053 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1057 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1059 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1062 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1064 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1067 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1069 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1072 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1074 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1084 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1085 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1086 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1087 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1092 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1094 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1095 "hphp.y"
    { (yyval).reset();;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1098 "hphp.y"
    { (yyval).reset();;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1099 "hphp.y"
    { (yyval).reset();;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1104 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1105 "hphp.y"
    { (yyval).reset();;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1110 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1111 "hphp.y"
    { (yyval).reset();;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1114 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1115 "hphp.y"
    { (yyval).reset();;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1118 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1119 "hphp.y"
    { (yyval).reset();;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1124 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1126 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1127 "hphp.y"
    { (yyval).reset();;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1128 "hphp.y"
    { (yyval).reset();;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1134 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1138 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1143 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1148 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1153 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(4) - (6)]),&(yyvsp[(3) - (6)]));;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1158 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(4) - (7)]),&(yyvsp[(3) - (7)]));;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1164 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(4) - (9)]),&(yyvsp[(3) - (9)]));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1170 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(4) - (8)]),&(yyvsp[(3) - (8)]));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1176 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]);;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1178 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1179 "hphp.y"
    { (yyval).reset();;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1180 "hphp.y"
    { (yyval).reset();;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1185 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),0,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL);;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1188 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),1,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL);;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1192 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),1,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL);;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1196 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),0,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL);;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1200 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),0,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL);;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1204 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),1,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL);;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1209 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),1,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL);;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1214 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),0,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL);;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1220 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1221 "hphp.y"
    { (yyval).reset();;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1224 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),0);;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1225 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),1);;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1227 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1229 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1233 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1234 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1237 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1238 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1239 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1243 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1245 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1246 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1247 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1252 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1253 "hphp.y"
    { (yyval).reset();;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1256 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1257 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1260 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1261 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1263 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1267 "hphp.y"
    { _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1272 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),0);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1278 "hphp.y"
    { _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1283 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1287 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1289 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1291 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1292 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1295 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1298 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1299 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1300 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1310 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1313 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1320 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1321 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1326 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1329 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1336 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1338 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1342 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1343 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1349 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1351 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1355 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1357 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1361 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1362 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1366 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1367 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1371 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1374 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1379 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1384 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1385 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1387 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1391 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1392 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1393 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1394 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1398 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1399 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1400 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1401 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1402 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1404 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1406 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1410 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1413 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1414 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1418 "hphp.y"
    { (yyval).reset();;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1419 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1422 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1423 "hphp.y"
    { (yyval).reset();;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1426 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1427 "hphp.y"
    { (yyval).reset();;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1430 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1432 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1435 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1436 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1437 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1438 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1439 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1440 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1443 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1444 "hphp.y"
    { (yyval).reset();;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1447 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1448 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1449 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1453 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1455 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1456 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1457 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1461 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1462 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1466 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1468 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1469 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1470 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1473 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1477 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1478 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1482 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1483 "hphp.y"
    { (yyval).reset();;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1487 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1488 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1492 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1497 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1501 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1502 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1503 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1507 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1508 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1512 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1513 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1514 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1515 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1516 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1517 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1518 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1519 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1520 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1521 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1522 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1523 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1524 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1525 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1526 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1527 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1528 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1529 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1530 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1531 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1533 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1534 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1535 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1537 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1538 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1539 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1540 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1542 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1543 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1545 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1546 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1547 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1550 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1552 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1553 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1555 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1556 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1559 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1560 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1563 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1566 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1567 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1568 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1573 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1575 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { Token t; _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { Token u; u.reset();
                                         _p->onClosure((yyval),0,u,(yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1587 "hphp.y"
    { Token t; _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1591 "hphp.y"
    { Token u; u.reset();
                                         _p->onClosure((yyval),&(yyvsp[(1) - (12)]),u,(yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1594 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1595 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1606 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1619 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1624 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1625 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1630 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1631 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1635 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1639 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1640 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1645 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1652 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1659 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1661 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1665 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1666 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1667 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1672 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1673 "hphp.y"
    { (yyval).reset();;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1677 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1678 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1680 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1687 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1690 "hphp.y"
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

  case 394:

/* Line 1455 of yacc.c  */
#line 1701 "hphp.y"
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

  case 395:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1713 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1718 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1719 "hphp.y"
    { (yyval).reset();;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1722 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1723 "hphp.y"
    { (yyval).reset();;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1726 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1730 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1736 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1744 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1748 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1752 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1761 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1763 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1852 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { (yyval).reset();;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1858 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { (yyval).reset();;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { (yyval).reset();;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1864 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1869 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { (yyval).reset();;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1874 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1875 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1876 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1878 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1879 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1880 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1884 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 1886 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 1891 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 1895 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 1897 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { (yyval).reset();;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { (yyval).reset();;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { (yyval).reset();;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { (yyval).reset();;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 1966 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 1967 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 1968 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 1971 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 1974 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 1977 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 1979 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 1984 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 1985 "hphp.y"
    { (yyval).reset();;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 1990 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 1992 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 1994 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 1995 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2000 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2005 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2006 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2011 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2014 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2019 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2020 "hphp.y"
    { (yyval).reset();;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2023 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2024 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2031 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2033 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2036 "hphp.y"
    { only_in_hh_syntax(_p);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2038 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2041 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2044 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2045 "hphp.y"
    { (yyval).reset();;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2049 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2051 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2055 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2056 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2060 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2061 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2065 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2067 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2072 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2074 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2078 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2079 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2080 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2081 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2082 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2083 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2085 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2088 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2090 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2091 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2095 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2096 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2097 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2098 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2100 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2102 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2104 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2105 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2109 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2110 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2111 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2117 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2120 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2123 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2127 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2135 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2142 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2146 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2150 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2159 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2162 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2163 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2166 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2167 "hphp.y"
    { (yyval).reset();;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2171 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
    { (yyval)++;;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2176 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2177 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2178 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2183 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2184 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2190 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2193 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2200 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2201 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { (yyval).reset();;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2213 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2214 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2215 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2216 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2219 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2221 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2222 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2236 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); ;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2344 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2352 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2353 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2354 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2355 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2359 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); ;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); ;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); ;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    {;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2406 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2423 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2427 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (5)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 10653 "new_hphp.tab.cpp"
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
#line 2439 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

